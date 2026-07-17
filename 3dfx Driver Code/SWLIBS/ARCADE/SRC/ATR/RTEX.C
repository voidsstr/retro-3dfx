/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 4$ 
** $Date: 10/11/00 7:34:19 PM$ 
**
*/

#include <glide.h>
#include "atrender.h"
#include "fxatr.h"

#ifdef AT_STATISTICS
void _atrStatsIncTexMem( AtrTexelFx tmu, FxU32 numBytes );
void _atrStatsDecTexMem( AtrTexelFx tmu, FxU32 numBytes );
#endif

/* information common to all ATB texture handles */

_AtrTexEntry *(_inCache[2]);
_AtrTexEntry *(_outOfCache[2]);
_AtrTexEntry *(_unused[2]);

const static FxU32 TEX_GRANULARITY = 512;

AtrTexelFx  _atrTexTmuFromHandle( AtrTexHandle t ) {
    _AtrTexEntry *entry;
    entry = (_AtrTexEntry*)t;
    return entry->atrInfo.tmu;
}

/*-------------------------------------------------------------------
  Function: _texEntryInit
  Date: 6/30
  Implementor(s): jdt
  Library: AT Render
  Description:
  private function to Initialize a structures
  Arguments:
  entry    - entry to initialize
  Return:
  none
  -------------------------------------------------------------------*/

static void _texEntryInit( _AtrTexEntry *entry ) {
    entry->state = ATR_TEXENT_FREE;
    entry->next  = 0;
    entry->prev  = 0;
    entry->other = 0;
    entry->dirty = FXFALSE;
    DRV_FUNC(TexEntryInit)(entry);
}

/*-------------------------------------------------------------------
  Function: atrTexNewHandle
  Date: 6/30/96 
  Implementor(s): jdt 
  Library: AT Render
  Description:
  Allocate a new texture handle.
  Arguments:
  tmu - which tmu's memory pool to allocate handle from
  Return:
  a new texture handle
  
  -------------------------------------------------------------------*/

AtrTexHandle atrTexNewHandle( AtrTexelFx tmu ) {
    _AtrTexEntry *newEntry;

#ifdef AT_DEBUGGING
    if ( tmu > ATR_TEXELFX_BOTH || 
         ( _atrDriver->caps.numTex == 1 && tmu > ATR_TEXELFX_0  ) ) 
        atuError( FXTRUE, "atrTexAllocate: invalid tmu specified" );
#endif

    /*---------------------------------------------------------------
      If Depositing a texture on 2 tmus for lodBlending then call 
      atrTexAllocate twice to create to handles and hide the odd
      handle in the even one.
      ---------------------------------------------------------------*/
    if ( tmu == ATR_TEXELFX_BOTH ) {
        AtrTexHandle even;
        AtrTexHandle odd;
        even = atrTexNewHandle( ATR_TEXELFX_0 );
        odd  = atrTexNewHandle( ATR_TEXELFX_1 );
        ((_AtrTexEntry*)even)->other = odd;
        return even;
    }

    /*---------------------------------------------------------------
      If the _unused entry list is exhausted, allocate more entries
      ---------------------------------------------------------------*/
    if ( !_unused[tmu] ) {
        _AtrTexEntry *track;
        FxU32 entry;
        track = _unused[tmu] = atuMemCalloc( _atrDriver->texEntrySize, 1 );
        _texEntryInit( track );
        for( entry = 0; entry < TEX_GRANULARITY-1; entry++ ) {
            track->next = atuMemCalloc( _atrDriver->texEntrySize, 1);
            track = track->next;
            _texEntryInit( track );
        }
    }

    /*---------------------------------------------------------------
      Grab an _unused entry from the head of the _unused list
      ---------------------------------------------------------------*/
    newEntry     = _unused[tmu];
    _unused[tmu] = _unused[tmu]->next;

    /*---------------------------------------------------------------
      Tack the new entry onto the head of the out_of_cache list
      ---------------------------------------------------------------*/
    newEntry->next   = _outOfCache[tmu];
    if ( _outOfCache[tmu] ) _outOfCache[tmu]->prev = newEntry;
    _outOfCache[tmu] = newEntry;

    newEntry->atrInfo.tmu = tmu;
	newEntry->state = ATR_TEXENT_ALLOCATED;

    return (AtrTexHandle)newEntry;
}

/*-------------------------------------------------------------------
  Function: atrTexDeleteHandle
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
  Free storage ( both system and texram ) associated with a 
  texture handle
  Arguments:
  handle - handle to deallocate
  Return:
  none
  -------------------------------------------------------------------*/

void atrTexDeleteHandle( AtrTexHandle handle ) {
    _AtrTexEntry *deadEntry;
    FxU32 tmu;
#ifdef  AT_DEBUGGING
    _AtrTexEntry *track;
    FxBool looking = FXTRUE;
    if ( !handle )
      atuError( FXTRUE, "atrTexDeallocate: null handle.\n" );

    /*--------------------------------------------------------------
      Walk all of the memory pools looking for this handle
      --------------------------------------------------------------*/
    track = _outOfCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _outOfCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    if ( looking ) 
      atuError( FXTRUE, "atrTexDeallocate: tried to deallocate an"
                " unknown image handle" );
#endif

    deadEntry = (_AtrTexEntry *)handle;

    /*--------------------------------------------------------------
      If in both tmus, separate and deallocate texture handles singly.
      --------------------------------------------------------------*/
    if ( deadEntry->other ) {
        AtrTexHandle other = deadEntry->other;
        deadEntry->other = 0;
        atrTexDeleteHandle( other );
        atrTexDeleteHandle( handle );
        return;
    }

    /*--------------------------------------------------------------
      Divine the tmu
      --------------------------------------------------------------*/
    tmu = deadEntry->atrInfo.tmu;

    /*--------------------------------------------------------------
      Update the bookkeeping
      --------------------------------------------------------------*/

#ifdef AT_STATISTICS
    if ( deadEntry->atrInfo.img != NULL ) {
        _atrStatsDecTexMem( tmu, deadEntry->atrInfo.sizeInBytes );
    }
#endif

    /*--------------------------------------------------------------
      Close up the hole
      --------------------------------------------------------------*/
    if ( deadEntry->next ) deadEntry->next->prev = deadEntry->prev;
    if ( deadEntry->prev ) {
        deadEntry->prev->next = deadEntry->next;
    } else { /* head of some list */
        if ( deadEntry->state == ATR_TEXENT_ALLOCATED )
          _outOfCache[tmu] = deadEntry->next;
        else 
          _inCache[tmu] = deadEntry->next;
    }

    /*--------------------------------------------------------------
      Clear the important values in the dead entry
      --------------------------------------------------------------*/
    deadEntry->prev = deadEntry->next = 0;
    deadEntry->atrInfo.img = 0;
	deadEntry->state = ATR_TEXENT_FREE;

    DRV_FUNC(TexDeleteHandle)( deadEntry );

    /*--------------------------------------------------------------
      Append to the head of the unused list
      --------------------------------------------------------------*/
    deadEntry->next = _unused[tmu];
    _unused[tmu] = deadEntry;

    return;
}

/*-------------------------------------------------------------------
  Function: _atrTexPunt
  Date: 7/1
  Implementor(s): jdt
  Library: AT Render
  Description:
  Eject a texture from texture ram.  Called because space must be 
  freed for download, or because source image has been replaced 
  by an image that no longer fits in the old hole.
  Arguments:
  handle - handle of texture to punt
  Return:
  none
  -------------------------------------------------------------------*/

void _atrTexPunt( _AtrTexEntry *pigskin ) {
    FxU32 tmu;

    tmu = pigskin->atrInfo.tmu;
    
    /*-----------------------------------------------------------
      Close up hole
      -----------------------------------------------------------*/
    if ( pigskin->next ) pigskin->next->prev = pigskin->prev;
    if ( pigskin->prev ) {
        pigskin->prev->next = pigskin->next;
    } else { /* head of some list */
        _inCache[tmu] = pigskin->next;
    }
    
    /*-----------------------------------------------------------
      Clear critical data
      -----------------------------------------------------------*/
    pigskin->prev = 0;
    pigskin->state = ATR_TEXENT_ALLOCATED;

    /*-----------------------------------------------------------
      Attach to outOfCacheList
      -----------------------------------------------------------*/
    pigskin->next = _outOfCache[tmu];
    if ( _outOfCache[tmu] ) _outOfCache[tmu]->prev = pigskin;
    _outOfCache[tmu] = pigskin;
    
    return;
}

/*-------------------------------------------------------------------
  Function: atrTexAssociate
  Date: 4/30
  Implementor(s): jdt
  Library: AT Render
  Description:
  Associate an image with a texture handle.  If the entry is already
  in texture ram, then dirty and and evaluate if the entry needs to
  be punted from texture ram.
  Arguments:
  handle - valid texture handle
  img - new image to associate with texture handle
  Return:
  none
  -------------------------------------------------------------------*/

FxBool atrTexAssociate( AtrTexHandle handle, 
                      AtrImg       *img ) {
    _AtrTexEntry    *entry = (_AtrTexEntry *)handle;
    FxBool result;
#ifdef AT_DEBUGGING
    _AtrTexEntry *track;
    FxBool looking = FXTRUE;
    if ( !img )
      atuError( FXTRUE, "atrTexAssociate: null img pointer.\n" );

    if ( !handle )
      atuError( FXTRUE, "atrTexAssociate: null tex handle.\n" );
    /*--------------------------------------------------------------
      Walk all of the memory pools looking for this handle
      --------------------------------------------------------------*/
    track = _outOfCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _outOfCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    if ( looking ) 
      atuError( FXTRUE, "atrTexAssociate: tried to reference an"
                " unknown image handle" );
#endif

#ifdef AT_STATISTICS
    if ( entry->atrInfo.img != NULL ) {
        _atrStatsDecTexMem( entry->atrInfo.tmu, entry->atrInfo.sizeInBytes );
        if ( entry->other ) {
            _AtrTexEntry *other = entry->other;

            _atrStatsDecTexMem( other->atrInfo.tmu, 
                                other->atrInfo.sizeInBytes );
        }
    }
#endif
 
    entry->atrInfo.img = img;

    if ( entry->other ) {
       ((_AtrTexEntry *)entry->other)->atrInfo.img = img;
    }

    result = ( DRV_FUNC(RealizeImg)( img ) && 
             DRV_FUNC(TexAssociate)(handle, img));

#ifdef AT_STATISTICS
     if ( result ) {
        _atrStatsIncTexMem( entry->atrInfo.tmu, entry->atrInfo.sizeInBytes );
        if ( entry->other ) {
            _AtrTexEntry *other = entry->other;

            _atrStatsIncTexMem( other->atrInfo.tmu, 
                                other->atrInfo.sizeInBytes );
        }
     }
#endif

    return result;
}

void _atrTexSource( _AtrTexEntry *entry, FxU32 mask ) {
    FxU32 tmu = entry->atrInfo.tmu;

    /*-----------------------------------------------------------
      Ensure that entry has a slot in tex ram
      -----------------------------------------------------------*/
    if ( entry->state == ATR_TEXENT_ALLOCATED ) {
        _AtrTexEntry *last;

#ifdef AT_STATISTICS
        _atrStatsTexMemoryCacheMiss(tmu);
#endif

        /*----------------------------------------------------------
          Remove Entry From Out Of Cache List 
          ---------------------------------------------------------*/
        if ( entry->next ) entry->next->prev = entry->prev;
        if ( entry->prev ) 
          entry->prev->next = entry->next;
        else /* head of oocache list */
          _outOfCache[tmu] = entry->next;
        entry->prev = 0;
        entry->next = 0;

        /*----------------------------------------------------------
          Calculate a new starting address for this texture
          ---------------------------------------------------------*/

        DRV_FUNC(TramAllocate)(entry);
        entry->dirty = FXTRUE;

        /*----------------------------------------------------------
          Add To End of InCache list
          ---------------------------------------------------------*/
        if ( !_inCache[tmu] ) {
            _inCache[tmu] = entry;
        } else {
            last = _inCache[tmu];
            while( last->next ) last = last->next;
            last->next = entry;
            entry->prev = last;
        } 
    }
    DRV_FUNC(TexSource)( entry, mask );
}

/*-------------------------------------------------------------------
  Function: atrTexSource
  Date: 7/1
  Implementor(s): jdt
  Library: AT Render
  Description:
  Set the texture associated with the provided texture handle as
  current.
  Arguments:
  handle - handle of texture to set current.
  Return:
  none
  -------------------------------------------------------------------*/
void atrTexSource( AtrTexHandle handle ) {
    _AtrTexEntry *entry = (_AtrTexEntry*)handle;

#ifdef AT_DEBUGGING
    _AtrTexEntry *track;
    FxBool looking = FXTRUE;
    if ( !handle )
      atuError( FXTRUE, "atrTexSource: null handle.\n" );

    /*--------------------------------------------------------------
      Walk all of the memory pools looking for this handle
      --------------------------------------------------------------*/
    track = _outOfCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _outOfCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    if ( looking ) 
      atuError( FXTRUE, "atrTexSource: tried to operate on an"
                " unknown texture handle (0x%x)\n\n", handle );
#endif

    if ( entry->other ) {
        _atrTexSource(entry, 
                      GR_MIPMAPLEVELMASK_EVEN );
        _atrTexSource((_AtrTexEntry*)entry->other, 
                      GR_MIPMAPLEVELMASK_ODD );
    } else {
        _atrTexSource( entry, GR_MIPMAPLEVELMASK_BOTH );
    }
    
    entry->state = ATR_TEXENT_CACHED;
    return;
}

/*-------------------------------------------------------------------
  Function: atrTexDirty
  Date: 7/2
  Implementor(s): jdt
  Library: AT Render
  Description:
  Mark a texture as dirty.  Forcing it to be downloaded to texram on 
  next texsource
  Arguments:
  handle - handle of texture
  Return:
  none
  -------------------------------------------------------------------*/
void atrTexDirty( AtrTexHandle handle ) {
    _AtrTexEntry *entry;
#ifdef AT_DEBUGGING
    _AtrTexEntry *track;
    FxBool looking = FXTRUE;
    if ( !handle )
      atuError( FXTRUE, "atrTexDirty: null handle.\n" );

    /*--------------------------------------------------------------
      Walk all of the memory pools looking for this handle
      --------------------------------------------------------------*/
    track = _outOfCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _outOfCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    if ( looking ) 
      atuError( FXTRUE, "atrTexDirty: tried to operate on an"
                " unknown image handle" );
#endif
    entry = (_AtrTexEntry*)handle;
    if ( entry->state != ATR_TEXENT_FREE ) entry->dirty = FXTRUE;
    return;
}

/*-------------------------------------------------------------------
  Function: atrTexInfo
  Date: 7/2
  Implementor(s): jdt
  Library: AT Render
  Description:
  Return the texture information for a given texture handle
  Arguments:
  handle - handle of texture to query 
  info - pointer to memory inwhich to store an AtrTexInfo structure
  Return:
  none
  -------------------------------------------------------------------*/
void atrTexInfo( AtrTexHandle handle, AtrTexInfo *info ) {
    _AtrTexEntry *entry;
#ifdef AT_DEBUGGING
    _AtrTexEntry *track;
    FxBool looking = FXTRUE;

    if ( !info ) 
      atuError( FXTRUE, "atrTexInfo: null info pointer.\n" );

    if ( !handle )
      atuError( FXTRUE, "atrTexInfo: null handle.\n" );

    /*--------------------------------------------------------------
      Walk all of the memory pools looking for this handle
      --------------------------------------------------------------*/
    track = _outOfCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[0];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _outOfCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    track = _inCache[1];
    while( looking && track ) {
        if ( track == (_AtrTexEntry*)handle ) looking = FXFALSE;
        track = track->next;
    }
    if ( looking ) 
      atuError( FXTRUE, "atrTexAttribs: tried to operate on an"
                " unknown image handle" );
#endif
    entry = (_AtrTexEntry*)handle;
    *info = entry->atrInfo;
    return;
}

/*-------------------------------------------------------------------
  Function: atrTexMemFlush
  Date: 7/1
  Implementor(s): jdt
  Library: AT Render
  Description:
  Flush all of texture memory for a given tmu.  This will force a 
  re-download of all textures.
  Arguments:
  tmu - which tmu to flush
  Return:
  none
  -------------------------------------------------------------------*/
void atrTexMemFlush( FxU32 tmu ) {
#ifdef AT_DEBUGGING
    if ( tmu > ATR_TEXELFX_BOTH || 
         ( _atrDriver->caps.numTex == 1 && tmu > ATR_TEXELFX_0  ) ) 
        atuError( FXTRUE, "atrTexMemFlush: invalid tmu specified" );
#endif
    while( _inCache[tmu] )
      _atrTexPunt( _inCache[tmu] );
    return;    
}

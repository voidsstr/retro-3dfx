/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 7:34:09 PM$ 
**
*/

#include "atrender.h"
#include <string.h>

/*
**-----------------------------------------------------------------
** File Scope Data
**-----------------------------------------------------------------
*/

static const char  _dataType[]     = "atrLight";
static const FxU32 _binaryRevision = 0;


/*-------------------------------------------------------------------
  Function: atrLightAllocate
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate and initialize a Light
  Arguments:
    num - number of lights to allocate and assign to the pointer
  Return:
    new array of lights
  -------------------------------------------------------------------*/
AtrLight *atrLightAllocate( FxU32 num ) {
    AtrLight *l;
    FxU32 index;
#ifdef AT_DEBUGGING
    if ( num == 0 ) 
        atuError( FXTRUE, "atrLightAllocate(): Invalid parameter.\n" );
#endif
    l = ( AtrLight * ) atuMemCalloc( sizeof( AtrLight ), num );
    for ( index = 0; index < num; index++ ) {
        atrLightDefault( l+index );
    }
    return l;
}

/*-------------------------------------------------------------------
  Function: atrLightDeallocate
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Safely deallocate arrays of atrLights.
  Arguments:
    l - pointer to lights
  Return:
    none
  -------------------------------------------------------------------*/
void atrLightDeallocate( AtrLight l[] ) {

#ifdef AT_DEBUGGING
    if ( !l )
        atuError( FXTRUE, "atrLightDeallocate(): Invalid parameter.\n" );
#endif

    atuMemFree( l );
}


/*-------------------------------------------------------------------
  Function: AtrLightDefault
  Date: 4/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Initialize data structure to default values

    For Light
        Infinite directed white light that points Z+
  Arguments:
    l - light to initialize
  Return:
    none
  -------------------------------------------------------------------*/
void atrLightDefault( AtrLight *l ) {
#ifdef AT_DEBUGGING
    if ( !l )
        atuError( FXTRUE, "atrLightDefault(): Invalid parameter.\n" );
#endif
    l->flags          = ATR_LIGHT_DIRECTED;
    l->color.r        = 1.0f;
    l->color.g        = 1.0f;
    l->color.b        = 1.0f;
    l->attenuation[0] = 0.0f;
    l->attenuation[0] = 0.0f;
    l->attenuation[0] = 0.0f;
    l->project.nearClip = 1.0f;
    l->project.fovInX   = 60.0f * ATM_DEGREE;
    l->project.fovInY   = 60.0f * ATM_DEGREE;
    atrXformSetIdentity( &l->lcsToWCS );
    return;
}


/*-------------------------------------------------------------------
  Function: atrLightStore
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Store a light out to a file stream.
  Arguments:
    stream - file stream
    l - light to store
  Return:
    FXTRUE - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrLightStore( AtrLight *l, FILE *stream ) {
    FxU32 count;
#ifdef AT_DEBUGGING
    if ( !l || !stream ) 
        atuError( FXTRUE, "atrLightStore(): Invalid parameter.\n" );
#endif    
    count = fwrite( &_binaryRevision, 
                    sizeof( _binaryRevision ), 
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;
    count = fwrite( _dataType,
                    sizeof( _dataType ), 
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    count = fwrite( l,
                    sizeof( AtrLight ),
                    1,
                    stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrLightLoad
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Load a light from a file stream.
  Arguments:
    stream - file stream
    l - memory into which to load light
  Return:
    FXTRUE  - success
    FXFALSE - failure
  -------------------------------------------------------------------*/
FxBool atrLightLoad( AtrLight *l, FILE *stream ) {
    FxU32 rev;
    char  type[64];
    FxU32 count;

#ifdef AT_DEBUGGING
    if ( !l || !stream ) 
        atuError( FXTRUE, "atrLightLoad(): Invalid parameter.\n" );
#endif    

    count = fread( &rev, sizeof( _binaryRevision ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( rev != _binaryRevision ) return FXFALSE;
    count = fread( type, sizeof( _dataType ), 1, stream );
    if ( count != 1 ) return FXFALSE;
    if ( strcmp( type, _dataType ) ) return FXFALSE;
    count = fread( l, sizeof( AtrLight ), 1, stream );
    if ( count != 1 ) return FXFALSE;

    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: atrLightAssign
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Assign one light to another.
  Arguments:
    src - source light
    dest - destination light
  Return:
    none
  -------------------------------------------------------------------*/
void atrLightAssign( AtrLight *dest, AtrLight *src ) {
#ifdef AT_DEBUGGING
    if ( !dest || !src )
        atuError( FXTRUE, "atrLightAssign(): Invalid paramter.\n" );
#endif
    *dest = *src;
    return;
}

/*-------------------------------------------------------------------
  Function: atrLightClone
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Allocate a new light and initialize it from the provided
    light.
  Arguments:
    src - light to be cloned
  Return:
    clone of src
  -------------------------------------------------------------------*/
AtrLight *atrLightClone( AtrLight *src ) {
    AtrLight *tmp;

#ifdef AT_DEBUGGING
    if ( !src )
        atuError( FXTRUE, "atrLightClone(): Invalid paramter.\n" );
#endif

    tmp = atrLightAllocate( 1 );
    *tmp = *src;
    return tmp;
}

/*-------------------------------------------------------------------
  Function: atrLightPrint
  Date: 3/19/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Print a formatted description of a light
  Arguments:
    indent - spaces to append to all output
    stream - output stream
    l - light to print
  Return:
    none
  -------------------------------------------------------------------*/
void atrLightPrint( AtrLight *l,
                    FILE *stream, 
                    FxU32 indent ) {
    
#ifdef AT_DEBUGGING
    if ( !l || !stream )
        atuError( FXTRUE, "atrLightPrint(): Invalid paramter.\n" );
#endif

    fprintf( stream, "%*sLight: r:%f g:%f b:%f flags 0x%x\nlcsToWCS:\n", 
             indent, " ", l->color.r, l->color.g, l->color.b, l->flags );
    atrXformPrint( &l->lcsToWCS, stream, indent );
    return;
}



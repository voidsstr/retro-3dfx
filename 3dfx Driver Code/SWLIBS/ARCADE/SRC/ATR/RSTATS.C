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
** $Date: 10/11/00 7:34:17 PM$ 
**
*/

#include "atrender.h"
#include "fxatr.h"

#include <string.h>
#include <time.h>

#define GLIDE_LIB
#define GLIDE_HARDWARE
#include <glide.h>

/*
** Require access to register data that glide doesn't 
** yet expose.  
*/
#undef HW_ACCESS_HACK

#ifdef HW_ACCESS_HACK
#include "..\..\..\sst1\glide\dos\src\fxglide.h"
#endif

AtrStats _atrCurrentStats;
static FxU32    texMem[2];
static FxU32    cacheMisses[2];
static FxU32    texBytesUsed[2];

/*-------------------------------------------------------------------
  Function: atrStatsReset
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Reset all statistics.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void atrStatsReset( void ) {
    memset( &_atrCurrentStats, 0, sizeof( _atrCurrentStats ) );
    _atrCurrentStats.elapsedTime = (float)clock() / (float)CLOCKS_PER_SEC; 
    cacheMisses[0] = 0;
    cacheMisses[1] = 0;
    DRV_FUNC(ResetPerfStats)();
}

/*-------------------------------------------------------------------
  Function: atrStatsIncTexMem
  Date: 11/26/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Increment total amount of texture memory required
  Arguments:
    tmu      - TMU to which texture is being assigned
    numBytes - number of bytes of texture memory being used
  Return:
    none
  -------------------------------------------------------------------*/

void _atrStatsIncTexMem( AtrTexelFx tmu, FxU32 numBytes ) {
    texBytesUsed[tmu] += numBytes;
}

/*-------------------------------------------------------------------
  Function: atrStatsDecTexMem
  Date: 11/26/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Decrement total amount of texture memory required
  Arguments:
    tmu      - TMU from which texture is being freed
    numBytes - number of bytes of texture memory being freed
  Return:
    none
  -------------------------------------------------------------------*/

void _atrStatsDecTexMem( AtrTexelFx tmu, FxU32 numBytes ) {
    if ( texBytesUsed[tmu] < numBytes ) {
        atuError(FXTRUE, "atrStatsDecTexMem: Error in texture accounting\n");
        return;
    }
    texBytesUsed[tmu] -= numBytes;
}

/*-------------------------------------------------------------------
  Function: atrStatsRetrieve
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Retrieve a copy of the current stats.
  Arguments:
    s - pointer to storage for an AtrStats structure
  Return:
    none
  -------------------------------------------------------------------*/
void atrStatsRetrieve( AtrStats *s ) {
    float oldTime;
    GrSstPerfStats_t gStats;
#if 0
    atuTscRead( &_atrCurrentStats.lowTSC,
                &_atrCurrentStats.highTSC );
#endif
    oldTime = _atrCurrentStats.elapsedTime;
    _atrCurrentStats.elapsedTime = ((float)clock()/(float)CLOCKS_PER_SEC)-
                               _atrCurrentStats.elapsedTime;


     /* DRV_FUNC(Idle)(); */
     DRV_FUNC(GetPerfStats)( &gStats );

    _atrCurrentStats.pixelsIn  = gStats.pixelsIn;
    _atrCurrentStats.pixelsOut = gStats.pixelsOut;
    _atrCurrentStats.texBytesUsed[0] = texBytesUsed[0];
    _atrCurrentStats.texBytesUsed[1] = texBytesUsed[1];

    *s = _atrCurrentStats;
    _atrCurrentStats.elapsedTime = oldTime;
    return;
}

/*-------------------------------------------------------------------
  Function: _atrStatsIncFrame
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Increment frame number and do frame-end calcs.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void _atrStatsIncFrame(void) {
    _atrCurrentStats.frames++;
    if ( texMem[0] > _atrCurrentStats.peakTexMemoryInFrame[0] ) 
        _atrCurrentStats.peakTexMemoryInFrame[0] = texMem[0];
    if ( texMem[1] > _atrCurrentStats.peakTexMemoryInFrame[1] ) 
        _atrCurrentStats.peakTexMemoryInFrame[1] = texMem[1];
    if ( cacheMisses[0] > _atrCurrentStats.peakTexCacheMissesInFrame[0] ) 
        _atrCurrentStats.peakTexCacheMissesInFrame[0] = cacheMisses[0];
    if ( cacheMisses[1] > _atrCurrentStats.peakTexCacheMissesInFrame[1] ) 
        _atrCurrentStats.peakTexCacheMissesInFrame[1] = cacheMisses[1];
}

/*-------------------------------------------------------------------
  Function: _atrStatsIncTris
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Increment triangle count.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void _atrStatsIncTris(FxU32 num) {
    _atrCurrentStats.totalTris+=num;
}

/*-------------------------------------------------------------------
  Function: _atrStatsIncClippedTris
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Incremente clipped tri count.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void _atrStatsIncClippedTris(FxU32 num) {
    _atrCurrentStats.clippedTris+=num;
}

/*-------------------------------------------------------------------
  Function: _atrStatsIncFFPixels
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Keep track of pixels drawn with Fast Fill.  
  Arguments:
    pixels - number of pixels for operation ( probably screen clear )
  Return:
    none
  -------------------------------------------------------------------*/
void _atrStatsIncFFPixels( FxU32 pixels ) { 
    _atrCurrentStats.ffPixelsIn += pixels;
}

/*-------------------------------------------------------------------
  Function: _atrStatsIncTexDownloads
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Increment statistics download count.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void _atrStatsIncTexDownloads( AtrTexelFx tmu ) {
    _atrCurrentStats.textureDownloads[tmu]++;
}

/*-------------------------------------------------------------------
  Function: _atrStatsTexMemoryDownload
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Track memory downloaded to texture memory this frame
  Arguments:
    size - number of bytes allocated
  Return:
    none
  -------------------------------------------------------------------*/
void _atrStatsTexMemoryDownload( AtrTexelFx tmu, FxU32 size ) {
    texMem[tmu] += size; 
    _atrStatsIncTexDownloads(tmu);
}

/*-------------------------------------------------------------------
  Function: _atrStatsTexMemoryCacheMiss
  Date: 3/24/96
  Implementor(s):jdt
  Library: AT Render
  Description:
    Increment per frame cache miss counter.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void _atrStatsTexMemoryCacheMiss( AtrTexelFx tmu ) {
    cacheMisses[tmu]++;
}

/*-------------------------------------------------------------------
  Function: _atrStatsIncVerts
  Date: 3/26/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Increment vertex processed counter.
  Arguments:
    none
  Return:
    none
  -------------------------------------------------------------------*/
void _atrStatsIncVerts(FxU32 num) {
    _atrCurrentStats.totalVerts += num;
}

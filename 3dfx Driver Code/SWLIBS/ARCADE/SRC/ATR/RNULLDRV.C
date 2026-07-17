/*
** Copyright (c) 1995,1996 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/

/* ATB: skeleton driver
*/

#include <math.h>
#include <stdio.h>
#include <3dfx.h>
#include <glide.h>
#include <atrender.h>
#include "fxatr.h"

#if macintosh
#pragma warn_unusedarg off
#endif

static void _atrXXXInitDispatchTable(AtrDriver *ctx);

static AtrDriver _atrNullDriver;

/*-------------------------------------------------------------------
  Function: _atrXXXBeginScene
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Prepare a context for rendering. Check if any surfaces have been
    lost and if so restore them. Do any device specific actions and
    get the current buffer size.
  Arguments:
    viewWidth  - returns the current width of the drawing surface
    viewHeight - returns the current height of the drawing surface
  Return:
    FXTRUE if the context is ready for rendering, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool
_atrXXXBeginScene(FxU32 *viewWidth, FxU32 *viewHeight) {
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrXXXEndScene
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Flush any commands in the execute buffer, perform any device specific
    actions, determine area of surface which needs updating
  Arguments:
    None
  Return:
    FXTRUE if succesful, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool
_atrXXXEndScene(void) {
     return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrXXXSplash
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render the splash screen
  Arguments:
    None
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrXXXSplash( void ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXRenderImg
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render an image at the specified location
  Arguments:
    i       - the image to display
    screenX - location at which to display image
    screenY 
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

static void 
_atrXXXRenderImg( AtrImg *i, FxU32 screenX, FxU32 screenY ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXRenderImg
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render an image at the specified location
  Arguments:
    i       - the image to display
    screenX - location at which to display image
    screenY 
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

static void 
_atrXXXRenderImgBuffer( AtrImg *i, FxU32 screenX, FxU32 screenY , FxU32 buffer ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXGrabImg
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Grab a portion of the display surface
  Arguments:
    dst     - where to put the captured image
    screenX - location at which to start capture
    screenY 
    buf     - capture front or back buffer?
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

static void 
_atrXXXGrabImg( AtrImg    *dst,
                 FxU32     screenX,
                 FxU32     screenY,
                 AtrBuffer buf) {
}

/*-------------------------------------------------------------------
  Function: _atrDrawPoint
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Draw a point
  Arguments:
    p - point to draw
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrXXXDrawPoint( const GrVertex *p ) {
}

/*-------------------------------------------------------------------
  Function: _atrDrawLine
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Draw a line
  Arguments:
    a - vertices defining line
    b
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrXXXDrawLine( const GrVertex *a, const GrVertex *b ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXDrawTriangle
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Draw a triangle
  Arguments:
    a  - vertices defining triangle
    b  
    c  
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrXXXDrawTriangle(const GrVertex *a, const GrVertex *b,
                            const GrVertex *c) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXClearCanvas
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Clear the current canvas to the specified color and z value
  Arguments:
    r  - color to clear to
    g
    b 
    z  - z value to clear to
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrXXXClearCanvas( float r, float g, float b, FxU16 za ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXSwapBuffer
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Swaap front and back buffers
  Arguments:
    sync - sync to screen refresh
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrXXXSwapBuffer( FxU32 sync ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXGetPerfStats
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Get performance statistics
  Arguments:
    pStats - where to put statistics
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrXXXGetPerfStats(GrSstPerfStats_t *pStats) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXResetPerfStats
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Reset performance statistics
  Arguments:
    None
  Return:
    None
  -------------------------------------------------------------------*/

static void 
_atrXXXResetPerfStats(void) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXClipWindow
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Set current clipping window
  Arguments:
    minx - the new region to clip to
    miny
    maxx
    maxy
  Return:
    None
  -------------------------------------------------------------------*/

static void
_atrXXXClipWindow( int minx, int miny, int maxx, int maxy ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXTexEntryInit
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Device specific initialization of a texture handle
  Arguments:
      e - texture handle to initialize
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrXXXTexEntryInit( _AtrTexEntry *entry ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXRealizeImg
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Convert the image data into a form it can be processed by hardware
  Arguments:
      i        - pointer to image structure
  Return:
      FXTRUE on success, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrXXXRealizeImg( AtrImg *i ) {
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrXXXUnrealizeImg
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Release the device dependent image data
  Arguments:
      i        - pointer to image structure
  Return:
      FXTRUE on success, FXFALSE otherwise
  -------------------------------------------------------------------*/

static void 
_atrXXXUnrealizeImg( AtrImg *i ) {
    i->devPrivate = NULL;
}

/*-------------------------------------------------------------------
  Function: _atrXXXCloneImg
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Make a new copy of an image
  Arguments:
      dst        - pointer to target image structure
      src        - pointer to source image structure
  Return:
      FXTRUE on success, FXFALSE otherwise
  TBD:
      Implement
  -------------------------------------------------------------------*/

static FxBool _atrXXXCloneImg( AtrImg *dst, const AtrImg *src ) {
    return FXFALSE;
}

/*-------------------------------------------------------------------
  Function: _atrXXXTexAssociate
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Associate a texture with an image
  Arguments:
      handle   - handle to texture
      img      - image to associate with handle
  Return:
  FXTRUE on success, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrXXXTexAssociate( AtrTexHandle handle, AtrImg *img ) {
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrXXXTexSource
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Make texture current
  Arguments:
      entry    - handle to texture
      mask     - which mip maps to download
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrXXXTexSource( _AtrTexEntry *entry, FxU32 mask ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXTramAllocate
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
     Allocate texture memory for this object
  Arguments:
      entry    - handle to desired texture
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrXXXTramAllocate( _AtrTexEntry *entry ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXTexPunt
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
     Free texture cache resources used by this texture
  Arguments:
      entry    - handle to desired texture
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrXXXTexPunt( _AtrTexEntry *entry ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXTexDeleteHandle
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
     Delete the resources used by this texture handle
  Arguments:
      entry    - handle to desired texture
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrXXXTexDeleteHandle( _AtrTexEntry *entry) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXUpdateEnv
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Set the current environment
  Arguments:
      e        - new environment
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrXXXUpdateEnv( AtrEnv *e ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXRenderBuffer
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Specify which buffer to render into (fron or back)
  Arguments:
    buf - buffer to be used for rendering
  Return:
    None
  -------------------------------------------------------------------*/

static void 
_atrXXXRenderBuffer( AtrBuffer buf ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXUpdateMaterial
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Set the current material
  Arguments:
      m        - new material
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrXXXUpdateMaterial( AtrMaterial *m ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXShutdown
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Free resources used by this driver
  Arguments:
      None
  Return:
      Nothing
  -------------------------------------------------------------------*/

static void 
_atrXXXShutdown( void ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXInit
  Date: 10/6
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Create all DirectDraw and Direct3D objects necessary to begin rendering.
    and Initialize the XXX ATB driver state including the capabilities 
    structure
  Arguments:
      driver info - descibing specific configaration
  Return:
      Initialized driver
  -------------------------------------------------------------------*/

AtrDriver *
_atrXXXInit( AtrDriverInfo *driverInfo ) {
    AtrDriverCaps *caps = &_atrNullDriver.caps;

    _atrSnapBias = ATR_SNAP_BIAS;
    caps->width  = 640;
    caps->height = 480;
    caps->bpp  = 16;
    caps->pfxRev = 0;
    caps->pfxMem = 0;
    caps->numTex = (FxU32)driverInfo->info; /* default 1 texel unit */
    caps->tfxConfig[0].tfxRev = 0;
    caps->tfxConfig[0].tfxMem = 0;
    caps->fullScreen = FXTRUE;
    caps->fullScreen = FXTRUE;
    caps->sli    = 0;
    _atrNullDriver.texEntrySize = sizeof(_AtrTexEntry);
    _atrXXXInitDispatchTable(&_atrNullDriver);

	return &_atrNullDriver;
}

/*-------------------------------------------------------------------
  Function: _atrIsDrawable
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Determine if a context can currently be drawn to
  Arguments:
    ctx - the rendering context
  Return:
    FXTRUE if the context can currently be drawn to, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrXXXIsDrawable( AtrContext ctx ) {
    return FXFALSE;
}

/*-------------------------------------------------------------------
  Function: _atrXXXResize
  Date: 10/11/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Resizes all the buffers and re-creates device if necessary.
    A new viewport will definitely be needed, but the
    device and buffers will only be re-created if they have gotten bigger
    or change size by a very large amount.
  Arguments:
    ctx - the rendering context
    w   - new width
    h   - new height
  Return:
    FXTRUE if the resize was succesful, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool
_atrXXXResize(AtrContext ctx, int w, int h) {
    return FXFALSE;
}

/*-------------------------------------------------------------------
  Function: _atrXXXPause
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Switch between main screen and graphics screen when in pass 
    through mode
  Arguments:
    flag - 1 if application is pausing, 0 if restarting
  Return:
    FXTRUE if switch was successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

static FxBool 
_atrXXXPause(FxBool flag) {
    return FXTRUE;
}

/*-------------------------------------------------------------------
  Function: _atrXXXIdle
  Date: 10/9/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Wait until the hardware has finsihed rendering
  Arguments:
    None
  Return:
    None
  -------------------------------------------------------------------*/

static void _atrXXXIdle(void) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXTransformVertices
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Transform a vertex pool from object space to view space, clip test
    it, and project them ( if unclipped ).
  Arguments:
    dest - destination vertex pool
    src - src vertex pool
    end - done transforming vertices when src == end
  Return:
    none
  -------------------------------------------------------------------*/
FxU32 _atrXXXTransformVertices( AtrDstVertex *dest,
                            AtrVertex *src,
                            AtrVertex *end ) {
    return 0;
}

/*-------------------------------------------------------------------
  Function: _atrXXXTransformVertices2D
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Transform a vertex pool from object space to view space, clip test
    it, and project them ( if unclipped ).
  Arguments:
    dest - destination vertex pool
    src - src vertex pool
    end - done transforming vertices when src == end
  Return:
    none
  -------------------------------------------------------------------*/
FxU32 _atrXXXTransformVertices2D( AtrDstVertex *dest,
                            AtrVertex *src,
                            AtrVertex *end ) {
    return 0;
}
/*-------------------------------------------------------------------
  Function: _atrXXXSpecialTransformVertices
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Transform a vertex pool from object space to view space, clip test
    it, and project them ( if unclipped ).
  Arguments:
    dest - destination vertex pool
    src - src vertex pool
    end - done transforming vertices when src == end
  Return:
    none
  -------------------------------------------------------------------*/
FxU32 _atrXXXSpecialTransformVertices( AtrDstVertex *dest,
                            AtrVertex *src,
                            AtrVertex *end ) {
    return 0;
}

/*----------------------------------------------------------------
  Function: _atrXXXRenderTri
  Date: 1/7/97
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render a single triangle.
  Arguments:
    a, b, c - AtrVertices specifying specifying corners of triangl
  Return:
    none
  ----------------------------------------------------------------*/
void _atrXXXRenderTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ){
}

/*----------------------------------------------------------------
  Function: _atrXXXRenderTriWF
  Date: 1/7/97
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render a single triangle.
  Arguments:
    a, b, c - AtrVertices specifying specifying corners of triangl
  Return:
    none
  ---------------------------------------------------------------*/
void _atrXXXRenderTriWF( AtrVertex *a, AtrVertex *b, AtrVertex *c ){
}

/*-------------------------------------------------------------------
  Function: _atrXXXRenderTriSet
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render a tri set
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXRenderTriSet( AtrTriSet *t ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXRenderTriSetWF
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render a tri set
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXRenderTriSetWF( AtrTriSet *t ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXRenderOpenTriSet
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render a tri set
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXRenderOpenTriSet( AtrOpenTriSet *t ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXRenderOpenTriSetWF
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render a tri set
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXRenderOpenTriSetWF( AtrOpenTriSet *t ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXSpecialRenderTriSet
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render a tri set
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXSpecialRenderTriSet( AtrTriSet *t ) {
}

/*-------------------------------------------------------------------
  Function: atrTransformOpenTriSet
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Transform all of the vertices in an opentriset from the srcvertex
    pool to the destination vertex pool.  Vertices that are outside
    of the clip volume will be flagged and left in homogeneous coords.
    All lighting and assignment of texture vertices
  Arguments:
    t - triset
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXTransformOpenTriSet( AtrOpenTriSet *t ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXpecialTransformOpenTriSet
  Date: 1/7/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Transform all of the vertices in an opentriset from the srcvertex
    pool to the destination vertex pool.  Vertices that are outside
    of the clip volume will be flagged and left in homogeneous coords.
    All lighting and assignment of texture vertices
  Arguments:
    t - triset
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXSpecialTransformOpenTriSet( AtrOpenTriSet *t ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXRenderSegment
  Date: 7/1/97
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Render a clipped line segment
  Arguments:
    a - start point for line segment
    b - end point for line segment
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXRenderSegment( AtrVertex *a, AtrVertex *b ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXRender2DTri
  Date: 1/7/97
  Implementor(s): mlwp
  Library: AT Render
  Description:
  Render a two dimensional triangle to the screen in the current material.
  Computed lighting and texture processing functions are ignored.
  Arguments:
  a, b, c - vertices of triangle
  Return:
  none
  -------------------------------------------------------------------*/
void _atrXXXRender2DTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXClipAndRenderSegment
  Date: 1/7/97
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Clip a line segment recursively and draw it at the bottom.  Can be
    extended to generalized plane clipping by extending clip codes.
  Arguments:
    a, b    - AtrDestVertices representing endpoints of line segment
    clipCode - clip code for current clipping plane - to be right-
               shifted to next clipping plane at each recursion.
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXClipAndRenderSegment(AtrDstVertex *a, AtrDstVertex *b,
                                 FxU32 clipCode ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXClipAndRenderTri
  Date: 7/1/97
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Clip a triangle recursively and draw it at the bottom.  Can be
    extended to generalized plane clipping by extending clip codes.
  Arguments:
    a, b, c - AtrDestVertices representing corners of triangle
    clipCode - clip code for current clipping plane - to be right-
               shifted to next clipping plane at each recursion.
  Return:
    none
  -------------------------------------------------------------------*/
void _atrXXXClipAndRenderTri( AtrDstVertex *a, AtrDstVertex *b,
                           AtrDstVertex *c, FxU32 clipCode ) {
}

/*-------------------------------------------------------------------
  Function: _atrXXXInitDispatchTable
  Date: 10/12/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Initialize the dispatch table for the XXX ATB driver
  Arguments:
    ctx - the rendering context to initialize
  Return:
    Nothing
  -------------------------------------------------------------------*/

static void
_atrXXXInitDispatchTable(AtrDriver *ctx) {
    ctx->RealizeImg = _atrXXXRealizeImg ;
    ctx->UnrealizeImg = _atrXXXUnrealizeImg ;
    ctx->CloneImg = _atrXXXCloneImg ;
    ctx->ClearCanvas = _atrXXXClearCanvas ;
    ctx->SwapBuffer = _atrXXXSwapBuffer ;
    ctx->BeginScene = _atrXXXBeginScene ;
    ctx->EndScene = _atrXXXEndScene ;
    ctx->RenderImg = _atrXXXRenderImg ;
    ctx->RenderImgBuffer = _atrXXXRenderImgBuffer ;
    ctx->GrabImg = _atrXXXGrabImg ;
    ctx->TexEntryInit = _atrXXXTexEntryInit ;
    ctx->TexDeleteHandle = _atrXXXTexDeleteHandle ;
    ctx->TexPunt = _atrXXXTexPunt ;
    ctx->TexAssociate = _atrXXXTexAssociate ;
    ctx->TexSource = _atrXXXTexSource ;
    ctx->TramAllocate = _atrXXXTramAllocate ;
    ctx->UpdateEnv = _atrXXXUpdateEnv ;
    ctx->UpdateMaterial = _atrXXXUpdateMaterial ;
    ctx->Shutdown = _atrXXXShutdown ;
    ctx->Idle = _atrXXXIdle ;
    ctx->IsDrawable = _atrXXXIsDrawable ;
    ctx->Pause = _atrXXXPause ;
    ctx->Resize = _atrXXXResize ;
    ctx->RenderBuffer = _atrXXXRenderBuffer ;
    ctx->DrawLine = _atrXXXDrawLine ;
    ctx->DrawPoint = _atrXXXDrawPoint ;
    ctx->DrawTriangle = _atrXXXDrawTriangle ;
    ctx->Splash = _atrXXXSplash ;
    ctx->GetPerfStats = _atrXXXGetPerfStats ;
    ctx->ResetPerfStats = _atrXXXResetPerfStats ;
    ctx->ClipWindow = _atrXXXClipWindow ;
    ctx->TransformVertices = _atrXXXTransformVertices;
    ctx->SpecialTransformVertices = _atrXXXSpecialTransformVertices;
    ctx->TransformVertices2D = _atrXXXTransformVertices2D;
    ctx->RenderTri = _atrXXXRenderTri;
    ctx->RenderTriSet = _atrXXXRenderTriSet;
    ctx->SpecialRenderTriSet = _atrXXXSpecialRenderTriSet;
    ctx->RenderTriWF = _atrXXXRenderTriWF;
    ctx->RenderTriSetWF = _atrXXXRenderTriSetWF;
    ctx->TransformOpenTriSet = _atrXXXTransformOpenTriSet;
    ctx->SpecialTransformOpenTriSet = _atrXXXSpecialTransformOpenTriSet;
    ctx->RenderOpenTriSet = _atrXXXRenderOpenTriSet;
    ctx->RenderOpenTriSetWF = _atrXXXRenderOpenTriSetWF;
    ctx->RenderSegment = _atrXXXRenderSegment;
    ctx->Render2DTri = _atrXXXRender2DTri;
    ctx->ClipAndRenderTri = _atrXXXClipAndRenderTri;
    ctx->ClipAndRenderSegment = _atrXXXClipAndRenderSegment;
}

#if macintosh
#pragma warn_unusedarg reset
#endif

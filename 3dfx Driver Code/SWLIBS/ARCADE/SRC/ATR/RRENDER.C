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
** $Date: 10/11/00 7:34:15 PM$
**
*/

#include "atrender.h"
#include "fxatr.h"
#define  GLIDE_HARDWARE
#include <glide.h>


/* The current material being applied in multi-pass rendering ops */
/* This is global so that it may be accessed by lighting functions */
AtrMaterial *_atrRenderMaterial;
extern AtrEnv   *_atrCurrentEnv;

AtrVertexFunc *_atrSpecialVertexCallback;

/*-------------------------------------------------------------------
  Function: atrSetSpecialCallback
  Date: 7/12
  Implementor(s): jdt
  Library: AT Render
  Description:
  set up a vertex function to be applied to clip space vertices when
  calling the "special" render api
  Arguments:
  callback - callback function
  Return:
  none
  -------------------------------------------------------------------*/
void atrSetSpecialCallback( AtrVertexFunc *callback ) {
    _atrSpecialVertexCallback = callback;
    return;
}

/*-------------------------------------------------------------------
  Function: _atrCheckTri
  Date: 3/25/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Check the boundaries of a triangle, debugging only.
  Arguments:    
    a, b, c - vertices
  Return:   
    FXTRUE - in
    FXFALSE - out
  -------------------------------------------------------------------*/
static FxBool _atrCheckTri( AtrDstVertex *a,
                     AtrDstVertex *b,
                     AtrDstVertex *c ) {
    FxBool retval = FXTRUE;

    if ( (FxU32)a->x > 
         _atrCurrentCanvas->xMax ||
         (FxU32)a->x <
         _atrCurrentCanvas->xMin ||
         (FxU32)a->y > 
         _atrCurrentCanvas->yMax ||
         (FxU32)a->y <
         _atrCurrentCanvas->yMin ) {
        atuError( FXFALSE, "_atrCheckTri(): Out of bounds vertex (%f, %f).\n",
                  a->x, a->y );
        retval = FXFALSE;
    }

    if ( (FxU32)b->x > 
         _atrCurrentCanvas->xMax ||
         (FxU32)b->x <
         _atrCurrentCanvas->xMin ||
         (FxU32)b->y > 
         _atrCurrentCanvas->yMax ||
         (FxU32)b->y <
         _atrCurrentCanvas->yMin ) {
        atuError( FXFALSE, "_atrCheckTri(): Out of bounds vertex (%f, %f).\n",
                  b->x, b->y );
        retval = FXFALSE;
    }

    if ( (FxU32)c->x > 
         _atrCurrentCanvas->xMax ||
         (FxU32)c->x <
         _atrCurrentCanvas->xMin ||
         (FxU32)c->y > 
         _atrCurrentCanvas->yMax ||
         (FxU32)c->y <
         _atrCurrentCanvas->yMin ) {
        atuError( FXFALSE, "_atrCheckTri(): Out of bounds vertex (%f, %f).\n",
                  c->x, c->y );
        retval = FXFALSE;
    }
    return retval;
}

/*-------------------------------------------------------------------
  Function: atrClearCanvas
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Clear current canvas to a specified color/za value.
  Arguments:
    r, g, b - floating point color
    za - sixteen bit integer value to clear depth/alpha buffer to
         ATR_WBUFFER_CLEAR - clear the wbuffer to farthest value
         ATR_WBUFFER_SET   - set wbuffer to nearest value
  Return:
    none
  -------------------------------------------------------------------*/

void atrClearCanvas( float r, float g, float b, FxU16 za ) {
#   ifdef AT_STATISTICS
    _atrStatsIncFFPixels( (_atrCurrentCanvas->xMax - _atrCurrentCanvas->xMin) *
                          (_atrCurrentCanvas->yMax - _atrCurrentCanvas->yMin)); 
#   endif
    DRV_FUNC(ClearCanvas)( r, g, b, za );
    return;
}

/*-------------------------------------------------------------------
  Function: atrSwapBuffer
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Exchange the back and front buffers.  And syncronize swap to some
    interval.  This function will stall the application until there 
    are no more than 1 pending swapbuffer in the fifo.  This prevents
    the laggy case where the CPU is many frames ahead of the rasterization
    engine.  This only happens with very simple geometry and very large
    triangles.
  Arguments:
    sync - swapinterval - 0 indicates not to even wait for vertical
           retrace.  >0 indicates a number of VRTCs to wait until executing.
  Return:
    none
  -------------------------------------------------------------------*/

void atrSwapBuffer( FxU32 sync ) {
#   ifdef AT_STATISTICS
    _atrStatsIncFrame();
#   endif
    DRV_FUNC(SwapBuffer)( sync ) ;
}

/*-------------------------------------------------------------------
  Function: atrBeginScene
  Date: 10/14/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Prepare a context for rendering. Check if any surfaces have been
    lost and if so restore them. Do any device specific actions and
    get the current display surface size.
  Arguments:
    viewWidth  - returns the current width of the drawing surface
    viewHeight - returns the current height of the drawing surface
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool atrBeginScene(FxU32 *viewWidth, FxU32 *viewHeight) {
    return DRV_FUNC(BeginScene)(viewWidth, viewHeight);
}

/*-------------------------------------------------------------------
  Function: atrEndScene
  Date: 10/14/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Flush any outstanding rendering commands, perform any device specific
    actions, determine area of surface which needs updating
  Arguments:
    None
  Return:
    FXTRUE if successful, FXFALSE otherwise
  -------------------------------------------------------------------*/

FxBool atrEndScene(void) {
    return DRV_FUNC(EndScene)();
}

/*-------------------------------------------------------------------
  Function: _atrTransformVertices
  Date: 3/24/96
  Implementor(s): jdt
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
FxBool CheckVertex( const GrVertex *c);

FxU32 _atrTransformVertices( AtrDstVertex *dest,
                            AtrVertex *src,
                            AtrVertex *end ) {
    return DRV_FUNC(TransformVertices)(dest, src, end);
}

/*-------------------------------------------------------------------
  Function: _atrSpecialTransformVertices
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Transform a vertex pool from object space to view space, clip test
    it, and project them ( if unclipped ).

    This is a special one-off function that allows for 
    clip-space manipulation of vertices prior to clip testing

  Arguments:    
    dest - destination vertex pool
    src - src vertex pool
    end - done transforming vertices when src == end
  Return:
    none
  -------------------------------------------------------------------*/

void _atrSpecialTransformVertices( AtrDstVertex *dest,
                            AtrVertex *src,
                            AtrVertex *end ) {
    DRV_FUNC(SpecialTransformVertices)(dest, src, end);
}


/*-------------------------------------------------------------------
  Function: atrRenderTri
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Render a single triangle.  
  Arguments:
    a, b, c - AtrVertices specifying specifying corners of triangle.
  Return:
    none
  -------------------------------------------------------------------*/

void atrRenderTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
    DRV_FUNC(RenderTri)( a, b, c );
}

/*-------------------------------------------------------------------
  Function: atrRenderTriSet
  Date: 4/6/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Render a tri set
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/

void atrRenderTriSet( AtrTriSet *t ) {
    DRV_FUNC(RenderTriSet)(t);
}

/*-------------------------------------------------------------------
  Function: atrSpecialRenderTriSet
  Date: 4/6/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Render a tri set
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/

void atrSpecialRenderTriSet( AtrTriSet *t ) {
    DRV_FUNC(SpecialRenderTriSet)( t );
}

/*-------------------------------------------------------------------
  Function: atrRenderTriWF
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Render a single triangle in wireframe
  Arguments:
    a, b, c - AtrVertices specifying specifying corners of triangle.
  Return:
    none
  -------------------------------------------------------------------*/

void atrRenderTriWF( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
     DRV_FUNC(RenderTriWF)( a, b, c ) ;
}

/*-------------------------------------------------------------------
  Function: atrRenderTriSetWF
  Date: 4/6/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Render a tri set as a wireframe
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/

void atrRenderTriSetWF( AtrTriSet *t ) {
    DRV_FUNC(RenderTriSetWF)( t );
}

/*-------------------------------------------------------------------
  Function: atrTransformOpenTriSet
  Date: 6/16
  Implementor(s): jdt
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

void atrTransformOpenTriSet( AtrOpenTriSet *t ) {
    DRV_FUNC(TransformOpenTriSet)( t );
}

/*-------------------------------------------------------------------
  Function: atrSpecialTransformOpenTriSet
  Date: 6/16
  Implementor(s): jdt
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

void atrSpecialTransformOpenTriSet( AtrOpenTriSet *t ) {
    DRV_FUNC(SpecialTransformOpenTriSet)( t );
}

/*-------------------------------------------------------------------
  Function: atrApplyMaterialToOpenTriSet
  Date: 6/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Light and do texture processing for an open tri set based on the
    current material.  Multi-pass with open tri sets is completely 
    explicit and successor pointers are ignored.  
  Arguments:
    t - triset
  Return:
    none
  -------------------------------------------------------------------*/
void atrApplyMaterialToOpenTriSet( AtrOpenTriSet *t ) {
    AtrVertex    *src  = t->srcVertices;
    AtrDstVertex *dest = t->dstVertices;
    AtrVertex    *end  = src + t->numVertices;
    
    _atrRenderMaterial = _atrCurrentMaterial;

    /* Compute Lighting And Texture Coords */
    if ( _atrRenderCache->irgbSrcFunc ) 
       _atrRenderCache->irgbSrcFunc( dest, src, end );
    if ( _atrRenderCache->iaSrcFunc ) 
       _atrRenderCache->iaSrcFunc( dest, src, end );
    if ( _atrRenderCache->texCoordSrcFunc[0] ) 
        _atrRenderCache->texCoordSrcFunc[0]( dest, src, end );
    if ( _atrRenderCache->texCoordSrcFunc[1] ) 
       _atrRenderCache->texCoordSrcFunc[1]( dest, src, end );

    return;
}

/*-------------------------------------------------------------------
  Function: atrRenderOpenTriSet
  Date: 6/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Clip and render an atrOpenTriSet that has been previously
    processed by atrTransformOpenTriSte
  Arguments:
    t - triset
  Return:
    none
  -------------------------------------------------------------------*/

void atrRenderOpenTriSet( AtrOpenTriSet *t ) {
    DRV_FUNC(RenderOpenTriSet)( t );
}

/*-------------------------------------------------------------------
  Function: atrRenderOpenTriSetWF
  Date: 6/16
  Implementor(s): jdt
  Library: AT Render
  Description:
    Clip and render an atrOpenTriSet that has been previously
    processed by atrTransformOpenTriSet.  Render as wireframe
  Arguments:
    t - triset
  Return:
    none
  -------------------------------------------------------------------*/

void atrRenderOpenTriSetWF( AtrOpenTriSet *t ) {
    DRV_FUNC(RenderOpenTriSetWF)( t );
}

/*-------------------------------------------------------------------
  Function: atrRenderImg
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    Render an image into the current canvas and render
    buffer at the given x and y coordinate.  If mipmapped,
    then the largest LOD is displayed.
  Arguments:
    i - image structure
    winX - canvas-relative starting x coordinate
    winY - canvas-relative starting y coordinate
  Return:
  -------------------------------------------------------------------*/

void atrRenderImg( AtrImg         *i, 
                   FxU32           winX,
                   FxU32           winY ) {
    int screenX = winX + _atrCurrentCanvas->xMin;
    int screenY = winY + _atrCurrentCanvas->yMin;

#ifdef AT_DEBUGGING
    if ( !i )
      atuError( FXTRUE, "atrRenderImg: invalid parameter.\n" );
#endif

    DRV_FUNC(RenderImg)( i,  screenX, screenY);
}


/*-------------------------------------------------------------------
  Function: atrRenderImg
  Date: 6/26
  Implementor(s): jdt
  Library: AT Render
  Description:
    Render an image into the current canvas and render
    buffer at the given x and y coordinate.  If mipmapped,
    then the largest LOD is displayed.
  Arguments:
    i - image structure
    winX - canvas-relative starting x coordinate
    winY - canvas-relative starting y coordinate
  Return:
  -------------------------------------------------------------------*/

void atrRenderImgBuffer( AtrImg         *i,
                   FxU32           winX,
                   FxU32           winY, 
				   FxU32		   buffer ) {
    int screenX = winX + _atrCurrentCanvas->xMin;
    int screenY = winY + _atrCurrentCanvas->yMin;

#ifdef AT_DEBUGGING
    if ( !i )
      atuError( FXTRUE, "atrRenderImg: invalid parameter.\n" );
#endif

    DRV_FUNC(RenderImgBuffer)( i,  screenX, screenY, buffer );
}


/*-------------------------------------------------------------------
  Function: atrGrabImg
  Date: 4/26/96
  Implementor(s): jdt
  Library: AT Render
  Description: 
    Grab an image from the from buffer into an AtrImg.  The
    must be initialized to the desired size of the region to 
    be grabbed, including allocation of data space.  All 
    grabbed data will be in RGB_565 format.  If reading
    from the aux buffer, 16-bit depth buffer information
    will be retrieved into the destination image area
  Arguments:
    dst - initialized image storage to receive frame grab
    winX - x coordinate in the current canvas from which to 
           grab
    winy - y coordinate in the current canvas from which to 
           grab
    buf - which buffer to grab from valid values are:
          ATR_BUFFER_FRONTBUFFER
          ATR_BUFFER_BACKBUFFER
          ATR_BUFFER_AUXBUFFER
  Return:
     none
  -------------------------------------------------------------------*/

void atrGrabImg( AtrImg    *dst,
                 FxU32     winX,
                 FxU32     winY,
                 AtrBuffer buf) {
    FxU32 screenX = winX + _atrCurrentCanvas->xMin;
    FxU32 screenY = _atrDriver->caps.height - 
                    (_atrCurrentCanvas->yMin+winY+dst->height);

#ifdef AT_DEBUGGING
    if ( !dst ) 
      atuError( FXTRUE, "atrGrabImg: invalid parameter.\n" );
    if ( screenX + dst->width - 1 > _atrCurrentCanvas->xMax )
      atuError( FXTRUE, "atrGrabImg: out of bound read - x.\n" );
    if ( (_atrDriver->caps.height - screenY) - 1 > _atrCurrentCanvas->yMax )
      atuError( FXTRUE, "atrGrabImg: out of bounds write - y.\n" );
    if ( buf > ATR_BUFFER_AUXBUFFER )
      atuError( FXTRUE, "atrGrabImg: invalid buffer specified.\n" );
    if ( dst->format < ATR_IMGFMT_16_BIT || dst->format == ATR_IMGFMT_32_BIT )
      atuError( FXTRUE, "atrGrabImg: invalid destination color format.\n" );
#endif
    DRV_FUNC(GrabImg)( dst, screenX, screenY, buf);
}

/*-------------------------------------------------------------------
  Function: atrRenderSegment
  Date: 3/22/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Render a clipped line segment
  Arguments:
    a - start point for line segment
    b - end point for line segment
  Return:
    none
  -------------------------------------------------------------------*/

void atrRenderSegment( AtrVertex *a, AtrVertex *b ) {
     DRV_FUNC(RenderSegment)( a, b );
}

/*-------------------------------------------------------------------
  Function: _atrTransformVertices2D
  Date: 3/24/96
  Implementor(s): jdt
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

static void _atrTransformVertices2D( AtrDstVertex *dest,
                            AtrVertex *src,
                            AtrVertex *end ) {
    DRV_FUNC(TransformVertices2D)( dest, src, end );
}

/*-------------------------------------------------------------------
  Function: atrRender2DTri
  Date: 7/13
  Implementor(s): jdt
  Library: AT Render
  Description:
  Render a two dimensional triangle to the screen in the current material.
  Computed lighting and texture processing functions are ignored.
  Arguments:
  a, b, c - vertices of triangle
  Return:
  none
  -------------------------------------------------------------------*/

void atrRender2DTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
    DRV_FUNC(Render2DTri)( a, b, c );
}

void atrDrawLine( const AtrDstVertex *a, const  AtrDstVertex *b ) {
    DRV_FUNC(DrawLine)( (const GrVertex *)a, (const GrVertex *)b);
}

void atrDrawPoint( const AtrDstVertex *p ) {
    DRV_FUNC(DrawPoint)( (const GrVertex *)p);
}

void atrDrawTriangle(const AtrDstVertex *a, const AtrDstVertex *b, 
                     const AtrDstVertex *c) {
    DRV_FUNC(DrawTriangle)( (const GrVertex *)a, (const GrVertex *)b,
                        (const GrVertex *)c);
}

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
** $Date: 10/11/00 7:35:33 PM$
**
*/

/****************************************************************************
 * rave_render.c : this file is based on d3drender.c, and contains callbacks*
 *		used to massage triangle data before it goes on to the final        *
 *		rendering calls.                                                    *
 ****************************************************************************
 * written by : Mike (or rather copied and pasted and tweaked by Mike)      *
 ****************************************************************************/
#if _WINDOWS	
#include <windows.h>
#include <windowsx.h>
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "atrender.h"
#include "fxatr.h"
#include "rave.h"
#include "rave_system.h"
#include "atb_rave.h"
#include <glide.h>

/* The current material being applied in multi-pass rendering ops */
/* This is global so that it may be accessed by lighting functions */
extern AtrMaterial *_atrRenderMaterial;

extern AtrVertexFunc *_atrSpecialVertexCallback;
extern short Num_triangles;

float _atrRAV_ZSCALE=(float)(1.0/65535.0);

extern int gPassNumber;		//for testing 1-TMU rendering


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

void _atrRAVTransformVertices( AtrDstVertex *dest,
                            AtrVertex *src,
                            AtrVertex *end ) {

    while( src < end ) {

        /* Transform To CCS */
        dest->x   = src->x * _atrRenderCache->lcsToCCS.data[0]  +
                    src->y * _atrRenderCache->lcsToCCS.data[4]  +
                    src->z * _atrRenderCache->lcsToCCS.data[8] +
                    _atrRenderCache->lcsToCCS.data[12];

        dest->y   = src->x * _atrRenderCache->lcsToCCS.data[1]  +
                    src->y * _atrRenderCache->lcsToCCS.data[5]  +
                    src->z * _atrRenderCache->lcsToCCS.data[9] +
                    _atrRenderCache->lcsToCCS.data[13];

        dest->oow = src->x * _atrRenderCache->lcsToCCS.data[2]  +
                    src->y * _atrRenderCache->lcsToCCS.data[6]  +
                    src->z * _atrRenderCache->lcsToCCS.data[10] +
                    _atrRenderCache->lcsToCCS.data[14];

        /* Clip Check ->oow temporarily stores w value */
        dest->flags = 0;
        if ( dest->oow < _atrRenderCache->nearClip ) {
            dest->flags |= ATR_CC_BEFORE;
            dest++;
            src++;
            continue;
        }
        if ( dest->oow > _atrRenderCache->farClip )  
            dest->flags |= ATR_CC_BEHIND;
        if ( dest->x >  dest->oow )                  
            dest->flags |= ATR_CC_RIGHT;
        if ( dest->y >  dest->oow )                  
            dest->flags |= ATR_CC_ABOVE;
        if ( dest->x < -dest->oow )                  
            dest->flags |= ATR_CC_LEFT;
        if ( dest->y < -dest->oow )                  
            dest->flags |= ATR_CC_BELOW;

        if ( dest->flags ) {
            dest++;
            src++;
            continue;
        } else {
            float prevx = dest->x, prevy = dest->y;

            dest->oow = 1.0f / dest->oow;
            dest->x = (( dest->x * _atrRenderCache->xScale * dest->oow ) +
                       _atrRenderCache->xOffset) - _atrSnapBias;
            dest->y = (( dest->y * _atrRenderCache->yScale * dest->oow ) +
                      _atrRenderCache->yOffset) - _atrSnapBias;
        }
        dest++;
        src++;
    }
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
void _atrRAVSpecialTransformVertices( AtrDstVertex *dest,
                                  AtrVertex *src,
                                  AtrVertex *end ) {
    AtrDstVertex *pDest;
    AtrVertex     *pSrc;

    pDest = dest;
    pSrc = src;
    while( pSrc < end ) {

        /* Transform To CCS */
        pDest->x   = pSrc->x * _atrRenderCache->lcsToCCS.data[0]  +
                    pSrc->y * _atrRenderCache->lcsToCCS.data[4]  +
                    pSrc->z * _atrRenderCache->lcsToCCS.data[8] +
                    _atrRenderCache->lcsToCCS.data[12];

        pDest->y   = pSrc->x * _atrRenderCache->lcsToCCS.data[1]  +
                    pSrc->y * _atrRenderCache->lcsToCCS.data[5]  +
                    pSrc->z * _atrRenderCache->lcsToCCS.data[9] +
                    _atrRenderCache->lcsToCCS.data[13];

        pDest->oow = pSrc->x * _atrRenderCache->lcsToCCS.data[2]  +
                    pSrc->y * _atrRenderCache->lcsToCCS.data[6]  +
                    pSrc->z * _atrRenderCache->lcsToCCS.data[10] +
                    _atrRenderCache->lcsToCCS.data[14];
        pDest++;
        pSrc++;
    }

    if ( _atrSpecialVertexCallback )
      _atrSpecialVertexCallback( dest, src, end );

    pDest = dest;
    pSrc = src;
    while( pSrc < end ) {
        /* Clip Check ->oow temporarily stores w value */
        pDest->flags = 0;
        if ( pDest->oow < _atrRenderCache->nearClip ) {
            pDest->flags |= ATR_CC_BEFORE;
            pDest++;
            pSrc++;
            continue;
        }
        if ( pDest->oow > _atrRenderCache->farClip )  
            pDest->flags |= ATR_CC_BEHIND;
        if ( pDest->x >  pDest->oow )                  
            pDest->flags |= ATR_CC_RIGHT;
        if ( pDest->y >  pDest->oow )                  
            pDest->flags |= ATR_CC_ABOVE;
        if ( pDest->x < -pDest->oow )                  
            pDest->flags |= ATR_CC_LEFT;
        if ( pDest->y < -pDest->oow )                  
            pDest->flags |= ATR_CC_BELOW;

        if ( pDest->flags ) {
            pDest++;
            pSrc++;
            continue;
        } else {
            pDest->oow = 1.0f / pDest->oow;
            pDest->x = (( pDest->x * _atrRenderCache->xScale * pDest->oow ) +
                       _atrRenderCache->xOffset) - _atrSnapBias;
            pDest->y = (( pDest->y * _atrRenderCache->yScale * pDest->oow ) +
                      _atrRenderCache->yOffset) - _atrSnapBias;
        }
        pDest++;
        pSrc++;
    }
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
void atrRAVRenderTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ) 
{
    AtrVertex   list[3];

    _atrRenderMaterial = _atrCurrentMaterial;

	 gPassNumber=2;

    list[0] = *a;
    list[1] = *b;
    list[2] = *c;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( 3 );
#   endif

    DRV_FUNC(TransformVertices)( _atrRenderCache->destVertex, list, list + 3 );

    while( _atrRenderMaterial ) {
#       ifdef AT_STATISTICS
        _atrStatsIncTris(1);
#       endif

        if ( _atrRenderCache->irgbSrcFunc )
            _atrRenderCache->irgbSrcFunc( &_atrRenderCache->destVertex[0],
                                          list,
                                          list+3 );
        if ( _atrRenderCache->iaSrcFunc )
            _atrRenderCache->iaSrcFunc( &_atrRenderCache->destVertex[0],
                                        list,
                                        list+3 );
        if ( _atrRenderCache->texCoordSrcFunc[0] ) 
            _atrRenderCache->texCoordSrcFunc[0]( &_atrRenderCache->destVertex[0],
                                                 list,
                                                 list+3 );
        if ( _atrRenderCache->texCoordSrcFunc[1] ) 
            _atrRenderCache->texCoordSrcFunc[1]( &_atrRenderCache->destVertex[0],
                                                 list,
                                                 list+3 );

        if ( _atrRenderCache->destVertex[0].flags |
             _atrRenderCache->destVertex[1].flags |
             _atrRenderCache->destVertex[2].flags ) {
#           ifdef AT_STATISTICS
             _atrStatsIncClippedTris(1);
#           endif

             /* Trivial Reject */
             if ( _atrRenderCache->destVertex[0].flags &
                  _atrRenderCache->destVertex[1].flags &
                  _atrRenderCache->destVertex[2].flags )
                    return;
             DRV_FUNC(ClipAndRenderTri)( &_atrRenderCache->destVertex[0],
                                   &_atrRenderCache->destVertex[1],
                                   &_atrRenderCache->destVertex[2],
                                   ATR_CC_BEFORE );
        } else {
#           ifdef AT_CHECK_TRIS
            if ( !_atrCheckTri( &_atrRenderCache->destVertex[0],
                                &_atrRenderCache->destVertex[1],
                                &_atrRenderCache->destVertex[2] ) ) {
                atuError( FXTRUE, "atrRenderTri(): clipping failed.\n" );
            }
#           endif
            DRV_FUNC(DrawTriangle)( (GrVertex*) &_atrRenderCache->destVertex[0],
                            (GrVertex*) &_atrRenderCache->destVertex[1],
                            (GrVertex*) &_atrRenderCache->destVertex[2] );
        }

        _atrRenderMaterial = _atrRenderMaterial->successor;
        if ( _atrRenderMaterial )
		  { 
				gPassNumber=2;
            DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
		  }
        else if ( _atrCurrentMaterial->successor )
		  {
		  		gPassNumber=1;
            DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
		  }
    }
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
void atrRAVRenderTriSet( AtrTriSet *t ) {
    _AtrTriSetNode *n = t->nodes;

    /* For Each Chunk */
	gPassNumber=1;

    while( n ) {

        AtrVertex *src = n->vertices;
        AtrDstVertex *dest = _atrRenderCache->destVertex;
        AtrVertex *end = src + n->numVertices;

        _atrRenderMaterial = _atrCurrentMaterial;

#       ifdef AT_STATISTICS
        _atrStatsIncVerts( n->numVertices );
#       endif

        /* Transform All Vertices */
        DRV_FUNC(TransformVertices)( dest, src, end );

        /* For Each Material */
        while( _atrRenderMaterial ) {
            FxU32        *connectivity;
            AtrTexHandle *lmap;

/*
 if ( _atrCurrentMaterial->successor ) {
    DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial->successor );
 }
 */


            /* Compute Lighting And Texture Coords */
            if ( _atrRenderCache->irgbSrcFunc ) 
                _atrRenderCache->irgbSrcFunc( dest, src, end );
            if ( _atrRenderCache->iaSrcFunc ) 
                _atrRenderCache->iaSrcFunc( dest, src, end );
            if ( _atrRenderCache->texCoordSrcFunc[0] ) 
                _atrRenderCache->texCoordSrcFunc[0]( dest, src, end );
            if ( _atrRenderCache->texCoordSrcFunc[1] ) 
                _atrRenderCache->texCoordSrcFunc[1]( dest, src, end );

            /* For Each Triangle */
            connectivity = n->connectivity;
            lmap = n->lightMaps;
            while( *connectivity ) {
                AtrDstVertex *a = 
                    _atrRenderCache->destVertex+(((*connectivity)>>16)&0xFF);
                AtrDstVertex *b = 
                    _atrRenderCache->destVertex+(((*connectivity)>>8)&0xFF);
                AtrDstVertex *c = 
                    _atrRenderCache->destVertex+((*connectivity)&0xFF);
#               ifdef AT_STATISTICS
                _atrStatsIncTris(1);
#               endif

                /* This is an unsightly hack, but most everything about
                   lighting maps as we have used them thusfar is */
                if ( (_atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK ) 
                     == ATR_TEXSRC_LMAP ) {
#                   ifdef AT_DEBUGGING
                    if ( !*lmap ) 
                        atuError( FXTRUE, "atrTriSetRender(): Lighting map "
                        "specified in material, not present in model.\n" );
#                   endif
                    atrTexSource( *lmap );
                    if ( a->flags ) {
                        a->s0 = _atrRenderMaterial->lmCoords[0][0] * 
                            _atrTwoFiftyFive;
                        a->t0 = _atrRenderMaterial->lmCoords[0][1] * 
                            _atrTwoFiftyFive;
                    } else {
                        a->oow0 = a->oow;
                        a->s0 = _atrRenderMaterial->lmCoords[0][0] * a->oow *
                            _atrTwoFiftyFive;
                        a->t0 = _atrRenderMaterial->lmCoords[0][1] * a->oow *
                            _atrTwoFiftyFive;
                    }
                    if ( b->flags ) {
                        b->s0 = _atrRenderMaterial->lmCoords[1][0] * 
                            _atrTwoFiftyFive;
                        b->t0 = _atrRenderMaterial->lmCoords[1][1] * 
                            _atrTwoFiftyFive;
                    } else {
                        b->oow0 = b->oow;
                        b->s0 = _atrRenderMaterial->lmCoords[1][0] * b->oow *
                            _atrTwoFiftyFive;
                        b->t0 = _atrRenderMaterial->lmCoords[1][1] * b->oow *
                            _atrTwoFiftyFive;
                    }
                    if ( c->flags ) {
                        c->s0 = _atrRenderMaterial->lmCoords[2][0] * 
                            _atrTwoFiftyFive;
                        c->t0 = _atrRenderMaterial->lmCoords[2][1] * 
                            _atrTwoFiftyFive;
                    } else {
                        c->oow0 = c->oow;
                        c->s0 = _atrRenderMaterial->lmCoords[2][0] * c->oow *
                            _atrTwoFiftyFive;
                        c->t0 = _atrRenderMaterial->lmCoords[2][1] * c->oow *
                            _atrTwoFiftyFive;
                    }
                }

                /* Check Clip Flags */
                if ( a->flags | b->flags | c->flags ) {
#                    ifdef AT_STATISTICS
                     _atrStatsIncClippedTris(1);
#                    endif
                     /* Trivial Reject */
                     if ( !(a->flags & b->flags & c->flags) ) {
                         DRV_FUNC(ClipAndRenderTri)( a, b, c, ATR_CC_BEFORE );
                     }
                } else {
#                   ifdef AT_CHECK_TRIS
                    if ( !_atrCheckTri( a, b, c ) ) 
                        atuError( FXTRUE, "atrRenderTri(): clipping failed.\n");
#                   endif
                    DRV_FUNC(DrawTriangle)((GrVertex*)a,
                                   (GrVertex*)b, 
                                   (GrVertex*)c);

                }
                connectivity++;
                lmap++;
            }
            
            /* TBD is this a bug with _atrCurrentMaterial */
            _atrRenderMaterial = _atrRenderMaterial->successor;
            if ( _atrRenderMaterial ) 
				{
					 gPassNumber=2;
                DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
            }
            else if ( _atrCurrentMaterial->successor ) 
				{
					 gPassNumber=1;
                DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
            }
        }
        n = n->next;
    }
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
void atrRAVSpecialRenderTriSet( AtrTriSet *t ) 
{
    _AtrTriSetNode *n = t->nodes;

	  gPassNumber=1;

    /* For Each Chunk */
    while( n ) {

        AtrVertex *src = n->vertices;
        AtrDstVertex *dest = _atrRenderCache->destVertex;
        AtrVertex *end = src + n->numVertices;

        _atrRenderMaterial = _atrCurrentMaterial;

#       ifdef AT_STATISTICS
        _atrStatsIncVerts( n->numVertices );
#       endif

        /* Transform All Vertices */
        DRV_FUNC(SpecialTransformVertices)( dest, src, end );

        /* For Each Material */
        while( _atrRenderMaterial ) {
            FxU32        *connectivity;
            AtrTexHandle *lmap;

            /* Compute Lighting And Texture Coords */
            if ( _atrRenderCache->irgbSrcFunc ) 
                _atrRenderCache->irgbSrcFunc( dest, src, end );
            if ( _atrRenderCache->iaSrcFunc ) 
                _atrRenderCache->iaSrcFunc( dest, src, end );
            if ( _atrRenderCache->texCoordSrcFunc[0] ) 
                _atrRenderCache->texCoordSrcFunc[0]( dest, src, end );
            if ( _atrRenderCache->texCoordSrcFunc[1] ) 
                _atrRenderCache->texCoordSrcFunc[1]( dest, src, end );

            /* For Each Triangle */
            connectivity = n->connectivity;
            lmap = n->lightMaps;
            while( *connectivity ) {
                AtrDstVertex *a = 
                    _atrRenderCache->destVertex+(((*connectivity)>>16)&0xFF);
                AtrDstVertex *b = 
                    _atrRenderCache->destVertex+(((*connectivity)>>8)&0xFF);
                AtrDstVertex *c = 
                    _atrRenderCache->destVertex+((*connectivity)&0xFF);
#               ifdef AT_STATISTICS
                _atrStatsIncTris(1);
#               endif

                /* This is an unsightly hack, but most everything about
                   lighting maps as we have used them thusfar is */
                if ( (_atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK ) 
                     == ATR_TEXSRC_LMAP ) {
#                   ifdef AT_DEBUGGING
                    if ( !*lmap ) 
                        atuError( FXTRUE, "atrTriSetRender(): Lighting map "
                        "specified in material, not present in model.\n" );
#                   endif
                    atrTexSource( *lmap );
                    if ( a->flags ) {
                        a->s0 = _atrRenderMaterial->lmCoords[0][0] * 
                            _atrTwoFiftyFive;
                        a->t0 = _atrRenderMaterial->lmCoords[0][1] * 
                            _atrTwoFiftyFive;
                    } else {
                        a->oow0 = a->oow;
                        a->s0 = _atrRenderMaterial->lmCoords[0][0] * a->oow *
                            _atrTwoFiftyFive;
                        a->t0 = _atrRenderMaterial->lmCoords[0][1] * a->oow *
                            _atrTwoFiftyFive;
                    }
                    if ( b->flags ) {
                        b->s0 = _atrRenderMaterial->lmCoords[1][0] * 
                            _atrTwoFiftyFive;
                        b->t0 = _atrRenderMaterial->lmCoords[1][1] * 
                            _atrTwoFiftyFive;
                    } else {
                        b->oow0 = b->oow;
                        b->s0 = _atrRenderMaterial->lmCoords[1][0] * b->oow *
                            _atrTwoFiftyFive;
                        b->t0 = _atrRenderMaterial->lmCoords[1][1] * b->oow *
                            _atrTwoFiftyFive;
                    }
                    if ( c->flags ) {
                        c->s0 = _atrRenderMaterial->lmCoords[2][0] * 
                            _atrTwoFiftyFive;
                        c->t0 = _atrRenderMaterial->lmCoords[2][1] * 
                            _atrTwoFiftyFive;
                    } else {
                        c->oow0 = c->oow;
                        c->s0 = _atrRenderMaterial->lmCoords[2][0] * c->oow *
                            _atrTwoFiftyFive;
                        c->t0 = _atrRenderMaterial->lmCoords[2][1] * c->oow *
                            _atrTwoFiftyFive;
                    }
                }

                /* Check Clip Flags */
                if ( a->flags | b->flags | c->flags ) {
#                    ifdef AT_STATISTICS
                     _atrStatsIncClippedTris(1);
#                    endif
                     /* Trivial Reject */
                     if ( !(a->flags & b->flags & c->flags) ) {
                         DRV_FUNC(ClipAndRenderTri)( a, b, c, ATR_CC_BEFORE );
                     }
                } else {
#                   ifdef AT_CHECK_TRIS
                    if ( !_atrCheckTri( a, b, c ) ) 
                        atuError( FXTRUE, "atrRenderTri(): clipping failed.\n");
#                   endif
                    DRV_FUNC(DrawTriangle)((GrVertex*)a,
                                   (GrVertex*)b, 
                                   (GrVertex*)c);

                }
                connectivity++;
                lmap++;
            }
            
            _atrRenderMaterial = _atrRenderMaterial->successor;
            if ( _atrRenderMaterial ) 
				{
					 gPassNumber=2;
                DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
            }
            else if ( _atrCurrentMaterial->successor ) 
				{
		  			 gPassNumber=1;
                DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
            }
        }
        n = n->next;
    }
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
void atrRAVRenderTriWF( AtrVertex *a, AtrVertex *b, AtrVertex *c ) 
{
    AtrVertex   list[3];

    _atrRenderMaterial = _atrCurrentMaterial;

    gPassNumber=1;

    list[0] = *a;
    list[1] = *b;
    list[2] = *c;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( 3 );
#   endif

    DRV_FUNC(TransformVertices)( _atrRenderCache->destVertex, list, list + 3 );

    while( _atrRenderMaterial ) {
#       ifdef AT_STATISTICS
        _atrStatsIncTris(6);
#       endif

        if ( _atrRenderCache->irgbSrcFunc )
            _atrRenderCache->irgbSrcFunc( &_atrRenderCache->destVertex[0],
                                          list,
                                          list+3 );
        if ( _atrRenderCache->iaSrcFunc )
            _atrRenderCache->iaSrcFunc( &_atrRenderCache->destVertex[0],
                                          list,
                                          list+3 );
        if ( _atrRenderCache->texCoordSrcFunc[0] )
            _atrRenderCache->texCoordSrcFunc[0]( &_atrRenderCache->destVertex[0],
                                              list,
                                              list+3 );
        if ( _atrRenderCache->texCoordSrcFunc[1] )
            _atrRenderCache->texCoordSrcFunc[1]( &_atrRenderCache->destVertex[0],
                                              list,
                                              list+3 );

        if ( _atrRenderCache->destVertex[0].flags |
             _atrRenderCache->destVertex[1].flags |
             _atrRenderCache->destVertex[2].flags ) {
#           ifdef AT_STATISTICS
             _atrStatsIncClippedTris(6);
#           endif

             /* Trivial Reject */
             if ( _atrRenderCache->destVertex[0].flags &
                  _atrRenderCache->destVertex[1].flags &
                  _atrRenderCache->destVertex[2].flags )
                    return;
             _atrRAVClipAndRenderTriWF( &_atrRenderCache->destVertex[0],
                                     &_atrRenderCache->destVertex[1],
                                     &_atrRenderCache->destVertex[2],
                                     ATR_CC_BEFORE );
        } else {
#           ifdef AT_CHECK_TRIS
            if ( !_atrCheckTri( &_atrRenderCache->destVertex[0],
                                &_atrRenderCache->destVertex[1],
                                &_atrRenderCache->destVertex[2] ) ) {
                atuError( FXTRUE, "atrRenderTri(): clipping failed.\n" );
            }
#           endif
            DRV_FUNC(DrawLine)( (GrVertex*) &_atrRenderCache->destVertex[0],
                        (GrVertex*) &_atrRenderCache->destVertex[1] );
            DRV_FUNC(DrawLine)( (GrVertex*) &_atrRenderCache->destVertex[1],
                        (GrVertex*) &_atrRenderCache->destVertex[2] );
            DRV_FUNC(DrawLine)( (GrVertex*) &_atrRenderCache->destVertex[2],
                        (GrVertex*) &_atrRenderCache->destVertex[0] );

        }

        _atrRenderMaterial = _atrRenderMaterial->successor;
        if ( _atrRenderMaterial ) 
		  {
		  		gPassNumber=2;
            DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
		  }
        else if ( _atrCurrentMaterial->successor )
		  {
		  		gPassNumber=1;
            DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
		  }
    }
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
void atrRAVRenderTriSetWF( AtrTriSet *t ) {
    _AtrTriSetNode *n = t->nodes;

	 gPassNumber=1;

    /* For Each Chunk */
    while( n ) {

        AtrVertex *src = n->vertices;
        AtrDstVertex *dest = _atrRenderCache->destVertex;
        AtrVertex *end = src + n->numVertices;

#       ifdef AT_STATISTICS
        _atrStatsIncVerts( n->numVertices );
#       endif

        _atrRenderMaterial = _atrCurrentMaterial;

        /* Transform All Vertices */
        DRV_FUNC(TransformVertices)( dest, src, end );

        /* For Each Material */
        while( _atrRenderMaterial ) {
            FxU32        *connectivity;
            AtrTexHandle *lmap;

            /* Compute Lighting And Texture Coords */
            if ( _atrRenderCache->irgbSrcFunc ) 
                _atrRenderCache->irgbSrcFunc( dest, src, end );
            if ( _atrRenderCache->iaSrcFunc ) 
                _atrRenderCache->iaSrcFunc( dest, src, end );
            if ( _atrRenderCache->texCoordSrcFunc[0] ) 
                _atrRenderCache->texCoordSrcFunc[0]( dest, src, end );
            if ( _atrRenderCache->texCoordSrcFunc[1] ) 
                _atrRenderCache->texCoordSrcFunc[1]( dest, src, end );

            /* For Each Triangle */
            connectivity = n->connectivity;
            lmap = n->lightMaps;
            while( *connectivity ) {
                AtrDstVertex *a = 
                    _atrRenderCache->destVertex+(((*connectivity)>>16)&0xFF);
                AtrDstVertex *b = 
                    _atrRenderCache->destVertex+(((*connectivity)>>8)&0xFF);
                AtrDstVertex *c = 
                    _atrRenderCache->destVertex+((*connectivity)&0xFF);
#               ifdef AT_STATISTICS
                _atrStatsIncTris(1);
#               endif

                /* This is an unsightly hack, but most everything about
                   lighting maps as we have used them thusfar is */
                if ( (_atrRenderMaterial->sysFlags & ATR_TEXSRC_MASK ) 
                     == ATR_TEXSRC_LMAP ) {
#                   ifdef AT_DEBUGGING
                    if ( !*lmap ) 
                        atuError( FXTRUE, "atrTriSetRender(): Lighting map "
                        "specified in material, not present in model.\n" );
#                   endif
                    atrTexSource( *lmap );
                    if ( a->flags ) {
                        a->s0 = _atrRenderMaterial->lmCoords[0][0] * 
                            _atrTwoFiftyFive;
                        a->t0 = _atrRenderMaterial->lmCoords[0][1] * 
                            _atrTwoFiftyFive;
                    } else {
                        a->oow0 = a->oow;
                        a->s0 = _atrRenderMaterial->lmCoords[0][0] * a->oow *
                            _atrTwoFiftyFive;
                        a->t0 = _atrRenderMaterial->lmCoords[0][1] * a->oow *
                            _atrTwoFiftyFive;
                    }
                    if ( b->flags ) {
                        b->s0 = _atrRenderMaterial->lmCoords[1][0] * 
                            _atrTwoFiftyFive;
                        b->t0 = _atrRenderMaterial->lmCoords[1][1] * 
                            _atrTwoFiftyFive;
                    } else {
                        b->oow0 = b->oow;
                        b->s0 = _atrRenderMaterial->lmCoords[1][0] * b->oow *
                            _atrTwoFiftyFive;
                        b->t0 = _atrRenderMaterial->lmCoords[1][1] * b->oow *
                            _atrTwoFiftyFive;
                    }
                    if ( c->flags ) {
                        c->s0 = _atrRenderMaterial->lmCoords[2][0] * 
                            _atrTwoFiftyFive;
                        c->t0 = _atrRenderMaterial->lmCoords[2][1] * 
                            _atrTwoFiftyFive;
                    } else {
                        c->oow0 = c->oow;
                        c->s0 = _atrRenderMaterial->lmCoords[2][0] * c->oow *
                            _atrTwoFiftyFive;
                        c->t0 = _atrRenderMaterial->lmCoords[2][1] * c->oow *
                            _atrTwoFiftyFive;
                    }
                }

                /* Check Clip Flags */
                if ( a->flags | b->flags | c->flags ) {
#                    ifdef AT_STATISTICS
                     _atrStatsIncClippedTris(6);
#                    endif
                     /* Trivial Reject */
                     if ( !(a->flags & b->flags & c->flags) ) {
                         _atrRAVClipAndRenderTriWF( a, b, c, ATR_CC_BEFORE );
                     }
                } else {
#                   ifdef AT_CHECK_TRIS
                    if ( !_atrCheckTri( a, b, c ) ) 
                        atuError( FXTRUE, "atrRenderTriWF(): clipping failed.\n");
#                   endif
                    DRV_FUNC(DrawLine)((GrVertex*)a,
                               (GrVertex*)b );
                    DRV_FUNC(DrawLine)((GrVertex*)b,
                               (GrVertex*)c );
                    DRV_FUNC(DrawLine)((GrVertex*)c,
                               (GrVertex*)a );

                }
                connectivity++;
                lmap++;
            }
            
            _atrRenderMaterial = _atrRenderMaterial->successor;
            if ( _atrRenderMaterial )
				{
 		  			 gPassNumber=2;
                DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
			   }
            else if ( _atrCurrentMaterial->successor )
				{
					 gPassNumber=1;
                DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
				}
        }
        n = n->next;
    }
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
void atrRAVTransformOpenTriSet( AtrOpenTriSet *t ) {
    AtrVertex    *src  = t->srcVertices;
    AtrDstVertex *dest = t->dstVertices;
    AtrVertex    *end  = src + t->numVertices;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( t->numVertices );
#   endif

    /* Transform All Vertices */
    DRV_FUNC(TransformVertices)( dest, src, end );
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
void atrRAVSpecialTransformOpenTriSet( AtrOpenTriSet *t ) {
    AtrVertex    *src  = t->srcVertices;
    AtrDstVertex *dest = t->dstVertices;
    AtrVertex    *end  = src + t->numVertices;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( t->numVertices );
#   endif

    DRV_FUNC(SpecialTransformVertices)( dest, src, end );
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
void atrRAVRenderOpenTriSet( AtrOpenTriSet *t ) {
    FxU32 (*connectivity)[3] = t->connectivity;
    FxU32 (*last)[3] = t->connectivity + t->numTriangles;
#   ifdef AT_STATISTICS
    _atrStatsIncTris( t->numTriangles );
#   endif

    while( connectivity < last ) {
        AtrDstVertex *a, *b, *c;
        a = t->dstVertices+(*connectivity)[0];
        b = t->dstVertices+(*connectivity)[1];
        c = t->dstVertices+(*connectivity)[2];
    
        if ( a->flags | b->flags | c->flags ) {
#           ifdef AT_STATISTICS
            _atrStatsIncClippedTris(1);
#           endif
            /* Trivial Reject */
            if ( !(a->flags & b->flags & c->flags) ) {
                DRV_FUNC(ClipAndRenderTri)( a, b, c, ATR_CC_BEFORE );
            }
        } else {
#           ifdef AT_CHECK_TRIS
            if ( !_atrCheckTri( a, b, c ) ) 
                atuError( FXTRUE, "atrRenderTriWF(): clipping failed.\n");
#           endif
            DRV_FUNC(DrawTriangle)( (GrVertex*)a,
                            (GrVertex*)b,
                            (GrVertex*)c );  
        }
        connectivity++;
    }
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
void atrRAVRenderOpenTriSetWF( AtrOpenTriSet *t ) {
    FxU32 (*connectivity)[3] = t->connectivity;
    FxU32 (*last)[3] = t->connectivity + t->numTriangles;
#   ifdef AT_STATISTICS
    _atrStatsIncTris( t->numTriangles * 6 );
#   endif

    while( connectivity < last ) {
        AtrDstVertex *a, *b, *c;
        a = t->dstVertices+(*connectivity)[0];
        b = t->dstVertices+(*connectivity)[1];
        c = t->dstVertices+(*connectivity)[2];
    
        if ( a->flags | b->flags | c->flags ) {
#           ifdef AT_STATISTICS
            _atrStatsIncClippedTris(6);
#           endif
            /* Trivial Reject */
            if ( !(a->flags & b->flags & c->flags) ) {
                _atrRAVClipAndRenderTriWF( a, b, c, ATR_CC_BEFORE );
            }
        } else {
#           ifdef AT_CHECK_TRIS
            if ( !_atrCheckTri( a, b, c ) ) 
                atuError( FXTRUE, "atrRenderTriWF(): clipping failed.\n");
#           endif
            DRV_FUNC(DrawLine)( (GrVertex*)a,
                        (GrVertex*)b );
            DRV_FUNC(DrawLine)( (GrVertex*)b,
                        (GrVertex*)c );
            DRV_FUNC(DrawLine)( (GrVertex*)c,
                        (GrVertex*)a );
        }
        connectivity++;
    }
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
void atrRAVRenderSegment( AtrVertex *a, AtrVertex *b ) 
{
    AtrVertex   list[2];
    AtrVertex   *end = list + 2;

#ifdef AT_DEBUGGING
    if ( !a || !b )
      atuError( FXTRUE, "atrRenderSegment: invalid parameter.\n" );
#endif
    
	 gPassNumber=1;

    _atrRenderMaterial = _atrCurrentMaterial;
    
    list[0] = *a;
    list[1] = *b;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( 2 );
#   endif

    DRV_FUNC(TransformVertices)( _atrRenderCache->destVertex, list, end );

    while( _atrRenderMaterial ) {
#       ifdef AT_STATISTICS
        _atrStatsIncTris(1);
#       endif
        
        if ( _atrRenderCache->irgbSrcFunc )
          _atrRenderCache->irgbSrcFunc( &_atrRenderCache->destVertex[0],
                                       list,
                                       end );
        if ( _atrRenderCache->iaSrcFunc )
          _atrRenderCache->iaSrcFunc( &_atrRenderCache->destVertex[0],
                                       list,
                                       end );
        if ( _atrRenderCache->texCoordSrcFunc[0] ) 
          _atrRenderCache->texCoordSrcFunc[0]( &_atrRenderCache->destVertex[0],
                                              list,
                                              end );
        if ( _atrRenderCache->texCoordSrcFunc[1] ) 
          _atrRenderCache->texCoordSrcFunc[1]( &_atrRenderCache->destVertex[0],
                                              list,
                                              end );
        
        if (_atrRenderCache->destVertex[0].flags |
            _atrRenderCache->destVertex[1].flags ) {
#           ifdef AT_STATISTICS
            _atrStatsIncClippedTris(1);
#           endif
            /* Trivial Reject */
            if ( !(_atrRenderCache->destVertex[0].flags &
                   _atrRenderCache->destVertex[1].flags) ) {
                DRV_FUNC(ClipAndRenderSegment)(&_atrRenderCache->destVertex[0],
                                         &_atrRenderCache->destVertex[1],
                                         ATR_CC_BEFORE );
            }
        } else {
            DRV_FUNC(DrawLine)( (GrVertex*) &_atrRenderCache->destVertex[0],
                       (GrVertex*) &_atrRenderCache->destVertex[1] );
        }
        
        _atrRenderMaterial = _atrRenderMaterial->successor;

        if ( _atrRenderMaterial )
		  {
		  		gPassNumber=2;
            DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
		  }
        else if ( _atrCurrentMaterial->successor )
		  {
		  		gPassNumber=1;
            DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
		  }
    }
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
void _atrRAVTransformVertices2D( AtrDstVertex *dest,
                            AtrVertex *src,
                            AtrVertex *end ) {

    while( src < end ) {

        dest->x = src->x;
        dest->y = src->y;
        dest->oow = 1.0f;
        dest->flags = 0;

        dest++;
        src++;
    }
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
void atrRAVRender2DTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
    AtrVertex   list[3];

    _atrRenderMaterial = _atrCurrentMaterial;

	  gPassNumber=1;

    list[0] = *a;
    list[1] = *b;
    list[2] = *c;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( 3 );
#   endif

    _atrRAVTransformVertices2D( _atrRenderCache->destVertex, list, list + 3 );

    while( _atrRenderMaterial ) {
#       ifdef AT_STATISTICS
        _atrStatsIncTris(1);
#       endif

        _atrIRGBSRC_STATIC( &_atrRenderCache->destVertex[0], list, list+3 );
        _atrTCSRC0_2DTC0( &_atrRenderCache->destVertex[0], list, list+3 );

        DRV_FUNC(DrawTriangle)( (GrVertex*) &_atrRenderCache->destVertex[0],
                       (GrVertex*) &_atrRenderCache->destVertex[1],
                       (GrVertex*) &_atrRenderCache->destVertex[2] );

        _atrRenderMaterial = _atrRenderMaterial->successor;
        if ( _atrRenderMaterial )
		  {
		  		gPassNumber=2;
				DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
		  }
        else if ( _atrCurrentMaterial->successor )
		  {
		  		gPassNumber=1;
            DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
		  }
    }
}

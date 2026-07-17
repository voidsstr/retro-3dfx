/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
** All Rights Reserved.
**                                                      7** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
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
** $Date: 10/11/00 7:35:05 PM$
**
*/

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ddraw.h>
#include <d3d.h>
#include "atrender.h"
#include "fxatr.h"
#include "atd3d.h"
#include <glide.h>


void RenderNode( _AtrTriSetNode *n );
void render_node_asm(_AtrTriSetNode *n);

/* The current material being applied in multi-pass rendering ops */
/* This is global so that it may be accessed by lighting functions */
AtrMaterial *_atrRenderMaterial;

float one_over_far_minus_near;
FxU32 _atr_src_size=64;
FxU32 _atr_src_xyz_offset=0;
FxU32 _atr_dst_size=64;
FxU32 _atr_dst_x_offset=0;
FxU32 _atr_dst_y_offset=4;
FxU32 _atr_dst_z_offset=24;
FxU32 _atr_dst_oow_offset=32;
FxU32 _atr_dst_flags_offset=8;
FxU32 _ONE=0x3f800000;
FxU32 _atrTempVertSpace[2000];


extern AtrVertexFunc *_atrSpecialVertexCallback;

void _atrD3DClipAndRenderTriWF( AtrDstVertex *a, AtrDstVertex *b,
                             AtrDstVertex *c, FxU32 clipCode );

/*-------------------------------------------------------------------
  Function: _atrD3DTransformVertices
  Date: 10/17/96
  Implementor(s): jdt, mlwp
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

/*-------------------------------------------------------------------
  Function: _atrD3DSpecialTransformVertices
  Date: 10/17/96
  Implementor(s): jdt, mlwp
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
void _atrD3DSpecialTransformVertices( AtrDstVertex *dest,
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

        pDest->ooz = (pDest->oow-_atrRenderCache->nearClip)*one_over_far_minus_near;

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
                       _atrRenderCache->xOffset) ;
            pDest->y = (( pDest->y * _atrRenderCache->yScale * pDest->oow ) +
                      _atrRenderCache->yOffset) ;
        }
        pDest++;
        pSrc++;
    }
}

/*-------------------------------------------------------------------
  Function: atrD3DRenderTri
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
    Render a single triangle.  
  Arguments:
    a, b, c - AtrVertices specifying specifying corners of triangle.
  Return:
    none
  -------------------------------------------------------------------*/
void atrD3DRenderTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
    AtrVertex   list[3];

    _atrRenderMaterial = _atrCurrentMaterial;

    list[0] = *a;
    list[1] = *b;
    list[2] = *c;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( 3 );
#   endif

    atr_d3d_transform_verts_asm(_atrRenderCache->destVertex, list, list + 3);

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
            DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
        else if ( _atrCurrentMaterial->successor )
            DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
    }
}

/*-------------------------------------------------------------------
  Function: atrD3DRenderTriSet
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
    Render a tri set
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/
#ifdef C_CODE_ONLY
void RenderNode( _AtrTriSetNode *n )
{
    FxU32   *connectivity;
    FxU16    numTri = 0;
    FxU32 i;
    WORD *idx;
    WORD idx_buffer[400];
    D3DTLVERTEX verts[64];
    
    idx=idx_buffer;
    /* TBD: figure out a better z scaling factor. this may be trouble */
    for ( i = 0; i <  n->numVertices; i++)
    {
        AtrDstVertex *a = _atrRenderCache->destVertex+i;

        verts[i].sx=a->x;
        verts[i].sy=d3dContext->any.caps.height - a->y;
        verts[i].sz=a->ooz;
        verts[i].dvRHW=a->oow0;
        verts[i].color=RGBA_MAKE((int)a->r,(int)a->g,(int)a->b, 0xff);
        verts[i].specular=0;
        verts[i].tu=a->s0;
        verts[i].tv=a->t0;
    }

    for ( connectivity = n->connectivity; *connectivity; connectivity++ )
    {
        *idx++=(WORD)(((FxU8 *)connectivity)[2]);
        *idx++=(WORD)(((FxU8 *)connectivity)[1]);
        *idx++=(WORD)(((FxU8 *)connectivity)[0]);
        numTri++;
    }
    d3dContext->lpD3DDevice2->lpVtbl->DrawIndexedPrimitive(d3dContext->lpD3DDevice2, 
               D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, 
               (LPVOID)verts, n->numVertices, 
               (LPWORD)(idx_buffer), (numTri)*3,D3DDP_DONOTCLIP|D3DDP_DONOTUPDATEEXTENTS);
#       ifdef AT_STATISTICS
        _atrStatsIncTris(numTri);
#       endif
}
#endif

void RenderNodeLM( _AtrTriSetNode *n )
{
    FxU32   *connectivity;
    AtrTexHandle *lmap;
    D3DTLVERTEX *verts;
    WORD *idx;
    FxU16 numTri = 0;
    FxU32 i;
    
    /* count number of triangles */

    for ( connectivity = n->connectivity; *connectivity; connectivity++ ) {
        numTri++;
    }
  
    _atrD3DCheckGeom(D3DPT_TRIANGLELIST, n->numVertices, 3*numTri);
    verts = _atD3DVerts + _atd3dCurVert;
    idx = _atD3DIdx+_atd3dCurIdx;
    *idx = _atd3dCurVert;

    /* TBD: figure out a better z scaling factor. this may be trouble */

    lmap = n->lightMaps;

    for ( i = 0; i <  n->numVertices; i++) {
        AtrDstVertex *a = _atrRenderCache->destVertex+i;

        verts[i].sx=a->x;
        verts[i].sy=d3dContext->any.caps.height - a->y;
        verts[i].sz=a->ooz;
        verts[i].dvRHW=a->oow0;
        verts[i].color=RGBA_MAKE((int)a->r,(int)a->g,(int)a->b, 0xFF);
        verts[i].specular=0;
        verts[i].tu=a->s0;
        verts[i].tv=a->t0;
    }

    for ( connectivity = n->connectivity; *connectivity; connectivity++ ) {
        *idx++=(WORD)(_atd3dCurVert+((*connectivity)>>16)&0xFF);
        *idx++=(WORD)(_atd3dCurVert+((*connectivity)>>8)&0xFF);
        *idx++=(WORD)(_atd3dCurVert+(*connectivity)&0xFF);
    }

#ifdef notdef
    d3dContext->lpD3DDevice2->lpVtbl->DrawIndexedPrimitive(d3dContext->lpD3DDevice2, 
               D3DPT_TRIANGLELIST, D3DVT_TLVERTEX, 
               (LPVOID)verts, n->numVertices, 
               (LPWORD)idxBase, numTri*3, 0);
#endif
#ifdef AT_STATISTICS
   _atrStatsIncTris(numTri);
#endif
}

void atrD3DRenderTriSet( AtrTriSet *t ) {
    _AtrTriSetNode *n = t->nodes;

    /* set scale for z, ensure we never exceed 1.0f */

    one_over_far_minus_near=0.98f/(_atrRenderCache->farClip - _atrRenderCache->nearClip);

    /* For Each Chunk */
    while( n ) {

        AtrVertex *src = n->vertices;
        AtrDstVertex *dest = _atrRenderCache->destVertex;
        AtrVertex *end = src + n->numVertices;
        FxU32 flags ;

        _atrRenderMaterial = _atrCurrentMaterial;

#       ifdef AT_STATISTICS
        _atrStatsIncVerts( n->numVertices );
#       endif

        /* Transform All Vertices */
        flags = atr_d3d_transform_verts_asm(dest,src,end);
        if (flags>=0x80000000)
        {
           n=n->next;
           continue;
        }

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

            if (( flags == 0 ) &&
                ( (_atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK ) 
                     != ATR_TEXSRC_LMAP ))
            {
                #ifdef C_CODE_ONLY
                RenderNode(n);
                #else
                render_node_asm(n);
                #endif
            } else {
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
                    a->s0 = _atrRenderMaterial->lmCoords[0][0];
                    a->t0 = _atrRenderMaterial->lmCoords[0][1];
                    b->s0 = _atrRenderMaterial->lmCoords[1][0];
                    b->t0 = _atrRenderMaterial->lmCoords[1][1];
                    c->s0 = _atrRenderMaterial->lmCoords[2][0];
                    c->t0 = _atrRenderMaterial->lmCoords[2][1];
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
            }
            
            /* TBD is this a bug with _atrCurrentMaterial */
            _atrRenderMaterial = _atrRenderMaterial->successor;
            if ( _atrRenderMaterial ) {
                DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
            }
            else if ( _atrCurrentMaterial->successor ) {
                DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
            }
        }
        n = n->next;
    }
}

/*-------------------------------------------------------------------
  Function: atrD3DSpecialRenderTriSet
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
    Render a tri set
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/
void atrD3DSpecialRenderTriSet( AtrTriSet *t ) {
    _AtrTriSetNode *n = t->nodes;

    /* set scale for z, ensure we never exceed 1.0f */

    one_over_far_minus_near=0.98f/(_atrRenderCache->farClip - _atrRenderCache->nearClip);

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
                    a->s0 = _atrRenderMaterial->lmCoords[0][0];
                    a->t0 = _atrRenderMaterial->lmCoords[0][1]; 
                    b->s0 = _atrRenderMaterial->lmCoords[1][0];
                    b->t0 = _atrRenderMaterial->lmCoords[1][1];
                    c->s0 = _atrRenderMaterial->lmCoords[2][0];
                    c->t0 = _atrRenderMaterial->lmCoords[2][1];
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
            if ( _atrRenderMaterial ) {
                DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
            }
            else if ( _atrCurrentMaterial->successor ) {
                DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
            }
        }
        n = n->next;
    }
}

/*-------------------------------------------------------------------
  Function: atrD3DRenderTriWF
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
    Render a single triangle in wireframe
  Arguments:
    a, b, c - AtrVertices specifying specifying corners of triangle.
  Return:
    none
  -------------------------------------------------------------------*/
void atrD3DRenderTriWF( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
    AtrVertex   list[3];

    _atrRenderMaterial = _atrCurrentMaterial;

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
             _atrD3DClipAndRenderTriWF( &_atrRenderCache->destVertex[0],
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
            DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
        else if ( _atrCurrentMaterial->successor )
            DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
    }
}

/*-------------------------------------------------------------------
  Function: atrD3DRenderTriSetWF
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
    Render a tri set as a wireframe
  Arguments:
    t - tri set to render
  Return:
    none
  -------------------------------------------------------------------*/
void atrD3DRenderTriSetWF( AtrTriSet *t ) {
    _AtrTriSetNode *n = t->nodes;

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
                    a->s0 = _atrRenderMaterial->lmCoords[0][0];
                    a->t0 = _atrRenderMaterial->lmCoords[0][1];
                    b->s0 = _atrRenderMaterial->lmCoords[1][0];
                    b->t0 = _atrRenderMaterial->lmCoords[1][1];
                    c->s0 = _atrRenderMaterial->lmCoords[2][0];
                    c->t0 = _atrRenderMaterial->lmCoords[2][1];
                }

                /* Check Clip Flags */
                if ( a->flags | b->flags | c->flags ) {
#                    ifdef AT_STATISTICS
                     _atrStatsIncClippedTris(6);
#                    endif
                     /* Trivial Reject */
                     if ( !(a->flags & b->flags & c->flags) ) {
                         _atrD3DClipAndRenderTriWF( a, b, c, ATR_CC_BEFORE );
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
                DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
            else if ( _atrCurrentMaterial->successor )
                DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
        }
        n = n->next;
    }
}

/*-------------------------------------------------------------------
  Function: atrD3DTransformOpenTriSet
  Date: 10/17/96
  Implementor(s): jdt, mlwp
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
void atrD3DTransformOpenTriSet( AtrOpenTriSet *t ) {
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
  Function: atrD3DSpecialTransformOpenTriSet
  Date: 10/17/96
  Implementor(s): jdt, mlwp
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
void atrD3DSpecialTransformOpenTriSet( AtrOpenTriSet *t ) {
    AtrVertex    *src  = t->srcVertices;
    AtrDstVertex *dest = t->dstVertices;
    AtrVertex    *end  = src + t->numVertices;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( t->numVertices );
#   endif

    DRV_FUNC(SpecialTransformVertices)( dest, src, end );
}

/*-------------------------------------------------------------------
  Function: atrD3DRenderOpenTriSet
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
    Clip and render an atrOpenTriSet that has been previously
    processed by atrTransformOpenTriSte
  Arguments:
    t - triset
  Return:
    none
  -------------------------------------------------------------------*/
void atrD3DRenderOpenTriSet( AtrOpenTriSet *t ) {
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
  Function: atrD3DRenderOpenTriSetWF
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
    Clip and render an atrOpenTriSet that has been previously
    processed by atrTransformOpenTriSet.  Render as wireframe
  Arguments:
    t - triset
  Return:
    none
  -------------------------------------------------------------------*/
void atrD3DRenderOpenTriSetWF( AtrOpenTriSet *t ) {
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
                _atrD3DClipAndRenderTriWF( a, b, c, ATR_CC_BEFORE );
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
  Function: atrD3DRenderSegment
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
    Render a clipped line segment
  Arguments:
    a - start point for line segment
    b - end point for line segment
  Return:
    none
  -------------------------------------------------------------------*/
void atrD3DRenderSegment( AtrVertex *a, AtrVertex *b ) {
    AtrVertex   list[2];
    AtrVertex   *end = list + 2;

#ifdef AT_DEBUGGING
    if ( !a || !b )
      atuError( FXTRUE, "atrRenderSegment: invalid parameter.\n" );
#endif
    
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
            DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
        else if ( _atrCurrentMaterial->successor )
            DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
    }
}

/*-------------------------------------------------------------------
  Function: _atrD3DTransformVertices2D
  Date: 10/17/96
  Implementor(s): jdt, mlwp
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
void _atrD3DTransformVertices2D( AtrDstVertex *dest,
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
  Function: atrD3DRender2DTri
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
  Render a two dimensional triangle to the screen in the current material.
  Computed lighting and texture processing functions are ignored.
  Arguments:
  a, b, c - vertices of triangle
  Return:
  none
  -------------------------------------------------------------------*/
void atrD3DRender2DTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
    AtrVertex   list[3];

    _atrRenderMaterial = _atrCurrentMaterial;

    list[0] = *a;
    list[1] = *b;
    list[2] = *c;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( 3 );
#   endif

    DRV_FUNC(TransformVertices2D)(_atrRenderCache->destVertex, list, list + 3 );

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
            DRV_FUNC(UpdateMaterial)( _atrRenderMaterial );
        else if ( _atrCurrentMaterial->successor )
            DRV_FUNC(UpdateMaterial)( _atrCurrentMaterial );
    }
}

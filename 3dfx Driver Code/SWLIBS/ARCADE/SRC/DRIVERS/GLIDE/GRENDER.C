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
** $Date: 10/11/00 7:35:25 PM$
**
*/

#include "atrender.h"
#include "fxatr.h"
#define  GLIDE_HARDWARE
#include <glide.h>
#include "rglide.h"

FxU32 _asm_TransformVertices(AtrDstVertex *dest,AtrVertex *src,AtrVertex *end);
//FxU32 __stdcall _asm_TransformVertices(AtrDstVertex *dest,AtrVertex *src,AtrVertex *end);
#define AT_PENTIUM_ASSEMBLY

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
  Function: _atrGlideTransformVertices
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

FxU32 CTransformVertices( AtrDstVertex *dest,
                            AtrVertex *src,
                            AtrVertex *end ){
    FxU32 value=0;

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
            value+=1;
            dest++;
            src++;
            continue;
        }
        if ( dest->oow > _atrRenderCache->farClip )
        {  
            dest->flags |= ATR_CC_BEHIND;
            value+=0x1000000;
        }
        if ( dest->x >  dest->oow )
        {                  
            dest->flags |= ATR_CC_RIGHT;
            value+=0x100;
        }
        if ( dest->x < -dest->oow )
        {                  
            dest->flags |= ATR_CC_LEFT;
            value+=0x10000;
        }
        if ( dest->y >  dest->oow )                  
            dest->flags |= ATR_CC_ABOVE;
        if ( dest->y < -dest->oow )                  
            dest->flags |= ATR_CC_BELOW;

        if ( dest->flags )
        {
            dest++;
            src++;
            value|=0x80000000;
            continue;
        } else {
            float prevx = dest->x, prevy = dest->y;

            dest->oow = 1.0f / dest->oow;
            dest->x = (( dest->x * _atrRenderCache->xScale * dest->oow ) +
                       _atrRenderCache->xOffset) - ATR_SNAP_BIAS;
            dest->y = (( dest->y * _atrRenderCache->yScale * dest->oow ) +
                      _atrRenderCache->yOffset) - ATR_SNAP_BIAS;
        }
        dest++;
        src++;
    }
    return value;
}

FxU32 _atrGlideTransformVertices( AtrDstVertex *dest,
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
                       _atrRenderCache->xOffset) - ATR_SNAP_BIAS;
            dest->y = (( dest->y * _atrRenderCache->yScale * dest->oow ) +
                      _atrRenderCache->yOffset) - ATR_SNAP_BIAS;
        }
        dest++;
        src++;
    }
    /* !! JDT - shoulde return munged condition code !! */
    return 0;
}

/*-------------------------------------------------------------------
  Function: _atrGlideSpecialTransformVertices
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
void _atrGlideSpecialTransformVertices( AtrDstVertex *dest,
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
                       _atrRenderCache->xOffset) - ATR_SNAP_BIAS;
            pDest->y = (( pDest->y * _atrRenderCache->yScale * pDest->oow ) +
                      _atrRenderCache->yOffset) - ATR_SNAP_BIAS;
        }
        pDest++;
        pSrc++;
    }
}

/*-------------------------------------------------------------------
  Function: _atrGlideRenderTri
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
void _atrGlideRenderTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
    AtrVertex   list[3];

    _atrRenderMaterial = _atrCurrentMaterial;

    list[0] = *a;
    list[1] = *b;
    list[2] = *c;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( 3 );
#   endif

#   ifdef AT_PENTIUM_ASSEMBLY
//    a_atrTransformVertices( _atrRenderCache->destVertex, list, list + 3 );
    _asm_TransformVertices(_atrRenderCache->destVertex, list, list + 3);
#   else
    CTransformVertices( _atrRenderCache->destVertex, list, list + 3 );
#   endif

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
             _atrGlideClipAndRenderTri( &_atrRenderCache->destVertex[0],
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
            grDrawTriangle( (GrVertex*) &_atrRenderCache->destVertex[0],
                            (GrVertex*) &_atrRenderCache->destVertex[1],
                            (GrVertex*) &_atrRenderCache->destVertex[2] );
        }

        _atrRenderMaterial = _atrRenderMaterial->successor;
        if ( _atrRenderMaterial ) 
            _atrGlideUpdateMaterial( _atrRenderMaterial );
        else if ( _atrCurrentMaterial->successor )
            _atrGlideUpdateMaterial( _atrCurrentMaterial );
    }
}

float _one_=1.0f;
float _bias_=(float)(1<<19);

/*-------------------------------------------------------------------
  Function: _atrGlideRenderTriSet
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
void _atrGlideRenderTriSet( AtrTriSet *t ) {
    _AtrTriSetNode *n = t->nodes;
    FxU32 flags;
    FxU32 num_tris=0;

#ifdef CHECK_HARSH
    AtrDstVertex     check_vert[64];
    FxU32 i;
#endif
    /* For Each Chunk */
    while( n )
    {

        AtrVertex *src = n->vertices;
        AtrDstVertex *dest = _atrRenderCache->destVertex;
        AtrVertex *end = src + n->numVertices;

        _atrRenderMaterial = _atrCurrentMaterial;

#       ifdef AT_STATISTICS
        _atrStatsIncVerts( n->numVertices );
#       endif

        /* Transform All Vertices */
#       ifdef AT_PENTIUM_ASSEMBLY
        flags=_asm_TransformVertices( dest, src, end );
        if (flags)
        {
           FxU8 *flgp=(FxU8 *)&flags;

           if (flgp[0]==n->numVertices)
           {
              n=n->next;
              continue;
           }
           if (flgp[1]==n->numVertices)
           {
              n=n->next;
              continue;
           }
           if (flgp[2]==n->numVertices)
           {
              n=n->next;
              continue;
           }
           if ((flgp[3]&0x7F)==n->numVertices)
           {
              n=n->next;
              continue;
           }
        }
#       else
        flags=CTransformVertices( dest, src, end );
        if (flags)
        {
           FxU8 *flgp=(FxU8 *)&flags;

           if (flgp[0]==n->numVertices)
           {
              n=n->next;
              continue;
           }
           if (flgp[1]==n->numVertices)
           {
              n=n->next;
              continue;
           }
           if (flgp[2]==n->numVertices)
           {
              n=n->next;
              continue;
           }
           if ((flgp[3]&0x7F)==n->numVertices)
           {
              n=n->next;
              continue;
           }
        }
#       endif


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

            connectivity = n->connectivity;
            if ((flags==0)&&((_atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK)
                     != ATR_TEXSRC_LMAP ))
            {
               while(*connectivity)
               {
                  AtrDstVertex *a,*b,*c;
                  
                  a=_atrRenderCache->destVertex+(((FxU8 *)connectivity)[2]);
                  b=_atrRenderCache->destVertex+(((FxU8 *)connectivity)[1]);
                  c=_atrRenderCache->destVertex+(((FxU8 *)connectivity)[0]);
                  grDrawTriangle((GrVertex *)a,(GrVertex *)b,(GrVertex *)c);
                  num_tris++;
                  connectivity++;
               }
            }
            else
            {
               /* For Each Triangle */
               lmap = n->lightMaps;
               while( *connectivity )
               {
                   AtrDstVertex *a,*b,*c;

                   a=_atrRenderCache->destVertex+(((FxU8 *)connectivity)[2]);
                   b=_atrRenderCache->destVertex+(((FxU8 *)connectivity)[1]);
                   c=_atrRenderCache->destVertex+(((FxU8 *)connectivity)[0]);

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
                   num_tris++;
                   /* Check Clip Flags */
                   if ( a->flags | b->flags | c->flags ) {
   #                    ifdef AT_STATISTICS
                        _atrStatsIncClippedTris(1);
   #                    endif
                        /* Trivial Reject */
                        if ( !(a->flags & b->flags & c->flags) ) {
                            _atrGlideClipAndRenderTri( a, b, c, ATR_CC_BEFORE );
                        }
                   } else {
   #                   ifdef AT_CHECK_TRIS
                       if ( !_atrCheckTri( a, b, c ) ) 
                           atuError( FXTRUE, "atrRenderTri(): clipping failed.\n");
   #                   endif
                       grDrawTriangle((GrVertex*)a,
                                      (GrVertex*)b, 
                                      (GrVertex*)c);

                   }
                   connectivity++;
                   lmap++;
               }
            }
            _atrRenderMaterial = _atrRenderMaterial->successor;
            if ( _atrRenderMaterial ) {
                _atrGlideUpdateMaterial( _atrRenderMaterial );
                if ( (_atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK ) 
                     == ATR_TEXSRC_LMAP ) 
					{
                    if (( _atrCurrentEnv->flags & ATR_FOG_ON_DEPTH_MP ) |
                        ( _atrCurrentEnv->flags & ATR_FOG_ON_DEPTH )) {
                         grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_PREFOG_COLOR,
                                               GR_BLEND_ONE, GR_BLEND_ZERO );
                    } 
                }
            } else if ( _atrCurrentMaterial->successor ) {
                _atrGlideUpdateMaterial( _atrCurrentMaterial );
            }
        }
        n = n->next;
    }
#ifdef AT_STATISTICS
   _atrStatsIncTris(num_tris);
#endif
}

/*-------------------------------------------------------------------
  Function: _atrGlideSpecialRenderTriSet
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
void _atrGlideSpecialRenderTriSet( AtrTriSet *t ) {
    _AtrTriSetNode *n = t->nodes;

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
        
        _atrGlideSpecialTransformVertices ( dest, src, end );

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
                         _atrGlideClipAndRenderTri( a, b, c, ATR_CC_BEFORE );
                     }
                } else {
#                   ifdef AT_CHECK_TRIS
                    if ( !_atrCheckTri( a, b, c ) ) 
                        atuError( FXTRUE, "atrRenderTri(): clipping failed.\n");
#                   endif
                    grDrawTriangle((GrVertex*)a,
                                   (GrVertex*)b, 
                                   (GrVertex*)c);

                }
                connectivity++;
                lmap++;
            }
            
            _atrRenderMaterial = _atrRenderMaterial->successor;
            if ( _atrRenderMaterial ) {
                _atrGlideUpdateMaterial( _atrRenderMaterial );
            }
            else if ( _atrCurrentMaterial->successor ) {
                _atrGlideUpdateMaterial( _atrCurrentMaterial );
            }
        }
        n = n->next;
    }
}

/*-------------------------------------------------------------------
  Function: _atrGlideRenderTriWF
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
void _atrGlideRenderTriWF( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
    AtrVertex   list[3];

    _atrRenderMaterial = _atrCurrentMaterial;

    list[0] = *a;
    list[1] = *b;
    list[2] = *c;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( 3 );
#   endif

#ifdef AT_PENTIUM_ASSEMBLY
    _asm_TransformVertices( _atrRenderCache->destVertex, list, list + 3 );
#else
    CTransformVertices( _atrRenderCache->destVertex, list, list + 3 );
#endif

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
             _atrGlideClipAndRenderTriWF( &_atrRenderCache->destVertex[0],
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
            grDrawLine( (GrVertex*) &_atrRenderCache->destVertex[0],
                        (GrVertex*) &_atrRenderCache->destVertex[1] );
            grDrawLine( (GrVertex*) &_atrRenderCache->destVertex[1],
                        (GrVertex*) &_atrRenderCache->destVertex[2] );
            grDrawLine( (GrVertex*) &_atrRenderCache->destVertex[2],
                        (GrVertex*) &_atrRenderCache->destVertex[0] );

        }

        _atrRenderMaterial = _atrRenderMaterial->successor;
        if ( _atrRenderMaterial ) 
            _atrGlideUpdateMaterial( _atrRenderMaterial );
        else if ( _atrCurrentMaterial->successor )
            _atrGlideUpdateMaterial( _atrCurrentMaterial );
    }
}

/*-------------------------------------------------------------------
  Function: _atrGlideRenderTriSetWF
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
void _atrGlideRenderTriSetWF( AtrTriSet *t ) {
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
#       ifdef AT_PENTIUM_ASSEMBLY
        _asm_TransformVertices( dest, src, end );
#       else
        CTransformVertices( dest, src, end );
#       endif

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
                         _atrGlideClipAndRenderTriWF( a, b, c, ATR_CC_BEFORE );
                     }
                } else {
#                   ifdef AT_CHECK_TRIS
                    if ( !_atrCheckTri( a, b, c ) ) 
                        atuError( FXTRUE, "atrRenderTriWF(): clipping failed.\n");
#                   endif
                    grDrawLine((GrVertex*)a,
                               (GrVertex*)b );
                    grDrawLine((GrVertex*)b,
                               (GrVertex*)c );
                    grDrawLine((GrVertex*)c,
                               (GrVertex*)a );

                }
                connectivity++;
                lmap++;
            }
            
            _atrRenderMaterial = _atrRenderMaterial->successor;
            if ( _atrRenderMaterial ) 
                _atrGlideUpdateMaterial( _atrRenderMaterial );
            else if ( _atrCurrentMaterial->successor )
                _atrGlideUpdateMaterial( _atrCurrentMaterial );
        }
        n = n->next;
    }
}

/*-------------------------------------------------------------------
  Function: _atrGlideTransformOpenTriSet
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
void _atrGlideTransformOpenTriSet( AtrOpenTriSet *t ) {
    AtrVertex    *src  = t->srcVertices;
    AtrDstVertex *dest = t->dstVertices;
    AtrVertex    *end  = src + t->numVertices;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( t->numVertices );
#   endif

    /* Transform All Vertices */
#   ifdef AT_PENTIUM_ASSEMBLY
    _asm_TransformVertices( dest, src, end );
#   else
    CTransformVertices( dest, src, end );
#   endif
}

/*-------------------------------------------------------------------
  Function: _atrGlideSpecialTransformOpenTriSet
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
void _atrGlideSpecialTransformOpenTriSet( AtrOpenTriSet *t ) {
    AtrVertex    *src  = t->srcVertices;
    AtrDstVertex *dest = t->dstVertices;
    AtrVertex    *end  = src + t->numVertices;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( t->numVertices );
#   endif

    _atrGlideSpecialTransformVertices( dest, src, end );
}

/*-------------------------------------------------------------------
  Function: _atrGlideRenderOpenTriSet
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
void _atrGlideRenderOpenTriSet( AtrOpenTriSet *t ) {
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
                _atrGlideClipAndRenderTri( a, b, c, ATR_CC_BEFORE );
            }
        } else {
#           ifdef AT_CHECK_TRIS
            if ( !_atrCheckTri( a, b, c ) ) 
                atuError( FXTRUE, "atrRenderTriWF(): clipping failed.\n");
#           endif
            grDrawTriangle ( (GrVertex*)a,
                            (GrVertex*)b,
                            (GrVertex*)c );  
        }
        connectivity++;
    }
}

/*-------------------------------------------------------------------
  Function: _atrGlideRenderOpenTriSetWF
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
void _atrGlideRenderOpenTriSetWF( AtrOpenTriSet *t ) {
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
                _atrGlideClipAndRenderTriWF( a, b, c, ATR_CC_BEFORE );
            }
        } else {
#           ifdef AT_CHECK_TRIS
            if ( !_atrCheckTri( a, b, c ) ) 
                atuError( FXTRUE, "atrRenderTriWF(): clipping failed.\n");
#           endif
            grDrawLine( (GrVertex*)a,
                        (GrVertex*)b );
            grDrawLine( (GrVertex*)b,
                        (GrVertex*)c );
            grDrawLine( (GrVertex*)c,
                        (GrVertex*)a );
        }
        connectivity++;
    }
}

/*-------------------------------------------------------------------
  Function: _atrGlideRenderSegment
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
void _atrGlideRenderSegment( AtrVertex *a, AtrVertex *b ) {
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

#   ifdef AT_PENTIUM_ASSEMBLY
    _asm_TransformVertices( _atrRenderCache->destVertex, list, end );
#   else
    CTransformVertices( _atrRenderCache->destVertex, list, end );
#   endif

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
                _atrGlideClipAndRenderSegment(&_atrRenderCache->destVertex[0],
                                         &_atrRenderCache->destVertex[1],
                                         ATR_CC_BEFORE );
            }
        } else {
            grDrawLine( (GrVertex*) &_atrRenderCache->destVertex[0],
                       (GrVertex*) &_atrRenderCache->destVertex[1] );
        }
        
        _atrRenderMaterial = _atrRenderMaterial->successor;
        if ( _atrRenderMaterial ) 
            _atrGlideUpdateMaterial( _atrRenderMaterial );
        else if ( _atrCurrentMaterial->successor )
            _atrGlideUpdateMaterial( _atrCurrentMaterial );
    }
}

/*-------------------------------------------------------------------
  Function: _atrGlideTransformVertices2D
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
void _atrGlideTransformVertices2D( AtrDstVertex *dest,
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
void _atrGlideRender2DTri( AtrVertex *a, AtrVertex *b, AtrVertex *c ) {
    AtrVertex   list[3];

    _atrRenderMaterial = _atrCurrentMaterial;

    list[0] = *a;
    list[1] = *b;
    list[2] = *c;

#   ifdef AT_STATISTICS
    _atrStatsIncVerts( 3 );
#   endif

    _atrGlideTransformVertices2D( _atrRenderCache->destVertex, list, list + 3 );

    while( _atrRenderMaterial ) {
#       ifdef AT_STATISTICS
        _atrStatsIncTris(1);
#       endif

        _atrIRGBSRC_STATIC( &_atrRenderCache->destVertex[0], list, list+3 );
        _atrTCSRC0_2DTC0( &_atrRenderCache->destVertex[0], list, list+3 );

        grDrawTriangle( (GrVertex*) &_atrRenderCache->destVertex[0],
                       (GrVertex*) &_atrRenderCache->destVertex[1],
                       (GrVertex*) &_atrRenderCache->destVertex[2] );

        _atrRenderMaterial = _atrRenderMaterial->successor;
        if ( _atrRenderMaterial ) 
            _atrGlideUpdateMaterial( _atrRenderMaterial );
        else if ( _atrCurrentMaterial->successor )
            _atrGlideUpdateMaterial( _atrCurrentMaterial );
    }
}

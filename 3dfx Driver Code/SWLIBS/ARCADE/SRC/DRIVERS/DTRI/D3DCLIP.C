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
** $Date: 10/11/00 7:35:15 PM$ 
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
#include "atd3d.h"
#include "fxatr.h"

extern float one_over_far_minus_near;


/*-------------------------------------------------------------------
  Function: _atrD3DunXform
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:  
    Untransform a vertex so that it may be clipped.
  Arguments:
    dest - destination vertex
    src - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrD3DunXform( AtrDstVertex *dest, AtrDstVertex *src ) {
    dest->oow  = 1.0f / src->oow;
    dest->r    = src->r;
    dest->g    = src->g;
    dest->b    = src->b;
    dest->a    = src->a;
    dest->x    = ( src->x - ( _atrRenderCache->xOffset ) ) /
                 _atrRenderCache->xScale  * dest->oow;
    dest->y    = ( src->y - ( _atrRenderCache->yOffset ) ) / 
                 _atrRenderCache->yScale  * dest->oow;
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK ) {    
        dest->oow0 = src->oow0 * dest->oow;
        if ( _atrRenderCache->texCoordSrcFunc[0] == _atrD3D_TCSRC0_PROJECTED ) {
            dest->s0   = src->s0 * dest->oow0 ;
            dest->t0   = src->t0 * dest->oow0 ; 
        } else {
            dest->s0   = src->s0 ;
            dest->t0   = src->t0 ; 
        }
    }
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC1_MASK ) {    
        dest->oow1 = src->oow1 * dest->oow;
        if ( _atrRenderCache->texCoordSrcFunc[1] == _atrD3D_TCSRC1_PROJECTED ) {
            dest->s1   = src->s1 * dest->oow1 ;
            dest->t1   = src->t1 * dest->oow1 ; 
        } else {
            dest->s1   = src->s1 ;
            dest->t1   = src->t1 ; 
        }
    }
    return;
}

/*-------------------------------------------------------------------
  Function: _atrD3DIntersect
  Date: 10/17/96
  Implementor(s): jdt, mlwp
  Library: AT Render
  Description:
    Intersection routine for clipping lines in homogeneous coords.
  Arguments:
    clipCode - flag describing which clipping plane is currently
               under consideration
    in - inside vertex
    out - outside vertex
    mid - new vertex that rests on clipping plane
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrD3DIntersect( FxU32 clipFlag, 
                AtrDstVertex *in,
                AtrDstVertex *out,
                AtrDstVertex *mid ) {
    AtrDstVertex _in;
    float din, dout, t, omt;

    if ( in->flags == 0 ) {
        _atrD3DunXform( &_in,in );
        in = &_in;
    }

    switch( clipFlag ) {
        case ATR_CC_BEFORE:
            din  = in->oow  - _atrRenderCache->nearClip;
            dout = out->oow - _atrRenderCache->nearClip;
            break;
        case ATR_CC_BEHIND:
            din  = in->oow  - _atrRenderCache->farClip;
            dout = out->oow - _atrRenderCache->farClip;
            break;
        case ATR_CC_LEFT:
            din  = in->oow  + in->x;
            dout = out->oow + out->x;
            break;
        case ATR_CC_RIGHT:
            din  = in->oow  - in->x;
            dout = out->oow - out->x;
            break;
        case ATR_CC_ABOVE:
            din  = in->oow  - in->y;
            dout = out->oow - out->y;
            break;
        case ATR_CC_BELOW:
            din  = in->oow  + in->y;
            dout = out->oow + out->y;
            break;
    }

    t = din / ( din - dout );
    omt = 1.0f - t; 

    mid->x   = t * out->x   + omt * in->x;
    mid->y   = t * out->y   + omt * in->y;
    mid->oow = t * out->oow + omt * in->oow;
    mid->r   = t * out->r   + omt * in->r;
    mid->g   = t * out->g   + omt * in->g;
    mid->b   = t * out->b   + omt * in->b;
    mid->a   = t * out->a   + omt * in->a;
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK ) {
        mid->s0   = t * out->s0   + omt * in->s0;
        mid->t0   = t * out->t0   + omt * in->t0;
        mid->oow0 = t * out->oow0 + omt * in->oow0; 
    }
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC1_MASK ) {
        mid->s1   = t * out->s1   + omt * in->s1;
        mid->t1   = t * out->t1   + omt * in->t1;
        mid->oow1 = t * out->oow1 + omt * in->oow1; 
    }    
    mid->flags = ATR_CC_CLIPPED;

    /* Intentional Fall-Through */
    switch( clipFlag ) {
        case ATR_CC_BEFORE:
            if ( mid->x >  mid->oow ) mid->flags |= ATR_CC_RIGHT;
        case ATR_CC_RIGHT:
            if ( mid->x < -mid->oow ) mid->flags |= ATR_CC_LEFT;
        case ATR_CC_LEFT:
            if ( mid->y >  mid->oow ) mid->flags |= ATR_CC_ABOVE;
        case ATR_CC_ABOVE:
            if ( mid->y < -mid->oow ) mid->flags |= ATR_CC_BELOW;
        case ATR_CC_BELOW:
            if ( mid->oow > _atrRenderCache->farClip )
                mid->flags |= ATR_CC_BEHIND;
        break;
    }

    return;
}

/*-------------------------------------------------------------------
  Function: _atrD3DClipProject
  Date: 10/17/96
  Implementor(s): mlwp
  Library: AT Render
  Description:
    Project an unprojected/clipped vertex
  Arguments:
    dest - destination vertex data
    src  - src vertex data
  Return:
    none
  -------------------------------------------------------------------*/
void _atrD3DClipProject( AtrDstVertex *dest, AtrDstVertex *src ) {

    dest->flags = 0;
    dest->ooz = (src->oow-_atrRenderCache->nearClip)*one_over_far_minus_near;
    dest->oow = 1.0f / src->oow;
    dest->x = (( src->x * _atrRenderCache->xScale * dest->oow ) +
               _atrRenderCache->xOffset) ;
    dest->y = (( src->y * _atrRenderCache->yScale * dest->oow ) +
               _atrRenderCache->yOffset) ;

    dest->r = src->r;
    dest->g = src->g;
    dest->b = src->b;
    dest->a = src->a;
    
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK ) {
        if ( _atrRenderCache->texCoordSrcFunc[0] == _atrD3D_TCSRC0_PROJECTED ) {
            float tmp = 1.0f / src->oow0;
            dest->s0   = tmp * src->s0 ;
            dest->t0   = tmp * src->t0 ; 
        } else {
            dest->s0   = src->s0 ;
            dest->t0   = src->t0 ; 
        }
        dest->oow0 = src->oow0 * dest->oow;
    }
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC1_MASK ) {
        if ( _atrRenderCache->texCoordSrcFunc[1] == _atrD3D_TCSRC1_PROJECTED ) {
            float tmp = 1.0f / src->oow1;
            dest->s1   = tmp * src->s1 ;
            dest->t1   = tmp * src->t1 ; 
        } else {
            dest->s1   = src->s1 ;
            dest->t1   = src->t1 ; 
        }
        dest->oow1 = src->oow1 * dest->oow;
    }
    return;
}
  
/*-------------------------------------------------------------------
  Function: _atrD3DClipAndRenderTri
  Date: 10/17/96
  Implementor(s): jdt, mlwp
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
void _atrD3DClipAndRenderTri( AtrDstVertex *a,
                              AtrDstVertex *b,
                              AtrDstVertex *c,
                              FxU32 clipCode ) {
    AtrDstVertex v1, v2;

    if ( clipCode == 0 ) {
        AtrDstVertex a_, b_, c_;
        if ( a->flags & ATR_CC_CLIPPED ) {
            _atrD3DClipProject( &a_, a );
            a = &a_;
        }
        if ( b->flags & ATR_CC_CLIPPED ) {
            _atrD3DClipProject( &b_, b );
            b = &b_;
        }
        if ( c->flags & ATR_CC_CLIPPED ) {
            _atrD3DClipProject( &c_, c );
            c = &c_;
        }

#ifdef AT_CHECK_TRIS
        if ( !_atrCheckTri( a, b, c ) ) {
            atuError( FXTRUE, "_atrD3DClipRenderTri(): clipping failed.\n" );
        }
#endif
        DRV_FUNC(DrawTriangle)( (GrVertex*)a, 
                        (GrVertex*)b, 
                        (GrVertex*)c );
    } else if ( a->flags & clipCode ) { /* out, ?, ? */
        if ( b->flags & clipCode ) { /* out, out, ? */
            if ( c->flags & clipCode ) { /* out, out, out */
                return;
            } else { /* out, out, in */
                _atrD3DIntersect( clipCode, c, a, &v1 );
                _atrD3DIntersect( clipCode, c, b, &v2 );
                _atrD3DClipAndRenderTri( c, &v1, &v2, clipCode >> 1 );
            }            
        } else { /* out, in, ? */
            if ( c->flags & clipCode ) { /* out, in, out */
                _atrD3DIntersect( clipCode, b, a, &v1 );
                _atrD3DIntersect( clipCode, b, c, &v2 );
                _atrD3DClipAndRenderTri( &v1, b, &v2, clipCode >> 1 );
            } else { /* out, in, in */
                _atrD3DIntersect( clipCode, b, a, &v1 );
                _atrD3DIntersect( clipCode, c, a, &v2 );
                _atrD3DClipAndRenderTri( &v1, b, c, clipCode >> 1 );
                _atrD3DClipAndRenderTri( &v1, c, &v2, clipCode >> 1 );
            }            
        }
    } else { /* in, ?, ? */
        if ( b->flags & clipCode ) { /* in, out, ? */
            if ( c->flags & clipCode ) { /* in, out, out */
                _atrD3DIntersect( clipCode, a, b, &v1 );
                _atrD3DIntersect( clipCode, a, c, &v2 );
                _atrD3DClipAndRenderTri( a, &v1, &v2, clipCode >> 1 );
            } else { /* in, out, in */
                _atrD3DIntersect( clipCode, a, b, &v1 );
                _atrD3DIntersect( clipCode, c, b, &v2 );
                _atrD3DClipAndRenderTri( a, &v1, c, clipCode >> 1 );
                _atrD3DClipAndRenderTri( &v1, &v2, c, clipCode >> 1 );
            }            
        } else { /* in, in */
            if ( c->flags & clipCode ) { /* in, in, out */
                _atrD3DIntersect( clipCode, a, c, &v1 );
                _atrD3DIntersect( clipCode, b, c, &v2 );
                _atrD3DClipAndRenderTri( &v1, a, b, clipCode >> 1 );
                _atrD3DClipAndRenderTri( &v1, b, &v2, clipCode >> 1 );
            } else { /* in, in, in */
                _atrD3DClipAndRenderTri( a, b, c, clipCode >> 1 );
            }            
        }
    }
    return;
}

/*-------------------------------------------------------------------
  Function: _atrD3DClipAndRenderSegment
  Date: 7/11
  Implementor(s): jdt
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
void _atrD3DClipAndRenderSegment(AtrDstVertex *a,
                              AtrDstVertex *b,
                              FxU32 clipCode ) {
    AtrDstVertex v1;

    if ( clipCode == 0 ) {
        AtrDstVertex a_, b_;
        if ( a->flags & ATR_CC_CLIPPED ) {
            _atrD3DClipProject( &a_, a );
            a = &a_;
        }
        if ( b->flags & ATR_CC_CLIPPED ) {
            _atrD3DClipProject( &b_, b );
            b = &b_;
        }
        DRV_FUNC(DrawLine)( (GrVertex*)a, (GrVertex*)b ); 
    } else if ( a->flags & clipCode ) { /* out, ? */
        if ( b->flags & clipCode ) {    /* out, out */
            return;
        } else {                        /* out, in */
            _atrD3DIntersect( clipCode, b, a, &v1 ); 
            _atrD3DClipAndRenderSegment( b, &v1, clipCode >> 1 );
        }
    } else {                             /* in, ? */
        if ( b->flags & clipCode ) {     /* in, out */
            _atrD3DIntersect( clipCode, a, b, &v1 );
            _atrD3DClipAndRenderSegment( a, &v1, clipCode >> 1 );
        } else {                         /* in, in */
            _atrD3DClipAndRenderSegment( a, b, clipCode >> 1 );
        }
    }
    return;
}

/*-------------------------------------------------------------------
  Function: _atrD3DClipAndRenderTriWF
  Date: 3/24/96
  Implementor(s): jdt
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
void _atrD3DClipAndRenderTriWF( AtrDstVertex *a,
                             AtrDstVertex *b,
                             AtrDstVertex *c,
                             FxU32 clipCode ) {

    _atrD3DClipAndRenderSegment( a, b, clipCode );
    _atrD3DClipAndRenderSegment( b, c, clipCode );
    _atrD3DClipAndRenderSegment( c, a, clipCode );
    return;
}

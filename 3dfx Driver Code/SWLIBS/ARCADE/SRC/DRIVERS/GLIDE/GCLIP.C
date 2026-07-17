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
** $Date: 10/11/00 7:35:23 PM$ 
**
*/

#include "atrender.h"
#include "fxatr.h"
#include "rglide.h"

/*-------------------------------------------------------------------
  Function: unXform
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:  
    Untransform a vertex so that it may be clipped.
  Arguments:
    dest - destination vertex
    src - source vertex
  Return:
    none
  -------------------------------------------------------------------*/
static void unXform( AtrDstVertex *dest, AtrDstVertex *src ) {
    dest->oow  = 1.0f / src->oow;
    dest->r    = src->r;
    dest->g    = src->g;
    dest->b    = src->b;
    dest->a    = src->a;
    dest->x    = ( src->x - ( _atrRenderCache->xOffset - ATR_SNAP_BIAS ) ) /
                 _atrRenderCache->xScale  * dest->oow;
    dest->y    = ( src->y - ( _atrRenderCache->yOffset - ATR_SNAP_BIAS ) ) / 
                 _atrRenderCache->yScale  * dest->oow;
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK ) {    
        dest->oow0 = src->oow0 * dest->oow;
        dest->s0   = src->s0   * dest->oow;
        dest->t0   = src->t0   * dest->oow; 
    }
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC1_MASK ) {    
        dest->oow1 = src->oow1 * dest->oow;
        dest->s1   = src->s1 * dest->oow;
        dest->t1   = src->t1 * dest->oow; 
    }
    return;
}

/*-------------------------------------------------------------------
  Function: intersect
  Date: 3/24/96
  Implementor(s): jdt
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
static void intersect( FxU32 clipFlag, 
                AtrDstVertex *in,
                AtrDstVertex *out,
                AtrDstVertex *mid ) {
    AtrDstVertex _in;
    float din, dout, t, omt;

    if ( in->flags == 0 ) {
        unXform( &_in,in );
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
  Function: _atrClipProject
  Date: 3/24/96
  Implementor(s): jdt
  Library: AT Render
  Description:
    Project an unprojected/clipped vertex
  Arguments:
    dest - destination vertex data
    src  - src vertex data
  Return:
    none
  -------------------------------------------------------------------*/
static void _atrClipProject( AtrDstVertex *dest, AtrDstVertex *src ) {
    dest->flags = 0;
    dest->oow = 1.0f / src->oow;
    dest->x = (( src->x * _atrRenderCache->xScale * dest->oow ) +
               _atrRenderCache->xOffset) - ATR_SNAP_BIAS;
    dest->y = (( src->y * _atrRenderCache->yScale * dest->oow ) +
               _atrRenderCache->yOffset) - ATR_SNAP_BIAS;

    dest->r = src->r;
    dest->g = src->g;
    dest->b = src->b;
    dest->a = src->a;
    
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC0_MASK ) {
        dest->s0   = src->s0 * dest->oow;
        dest->t0   = src->t0 * dest->oow;
        dest->oow0 = src->oow0 * dest->oow;
    }
    if ( _atrRenderMaterial->sysFlags & ATR_TEXSRC1_MASK ) {
        dest->s1   = src->s1 * dest->oow;
        dest->t1   = src->t1 * dest->oow;
        dest->oow1 = src->oow1 * dest->oow;
    }
    return;
}
  
/*-------------------------------------------------------------------
  Function: _atrGlideClipAndRenderTri
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
void _atrGlideClipAndRenderTri( AtrDstVertex *a,
                           AtrDstVertex *b,
                           AtrDstVertex *c,
                           FxU32 clipCode ) {
    AtrDstVertex v1, v2;

    if ( clipCode == 0 ) {
        AtrDstVertex a_, b_, c_;
        if ( a->flags & ATR_CC_CLIPPED ) {
            _atrClipProject( &a_, a );
            a = &a_;
        }
        if ( b->flags & ATR_CC_CLIPPED ) {
            _atrClipProject( &b_, b );
            b = &b_;
        }
        if ( c->flags & ATR_CC_CLIPPED ) {
            _atrClipProject( &c_, c );
            c = &c_;
        }

#ifdef AT_CHECK_TRIS
        if ( !_atrCheckTri( a, b, c ) ) {
            atuError( FXTRUE, "_atrClipRenderTri(): clipping failed.\n" );
        }
#endif
        grDrawTriangle( (GrVertex*)a, 
                        (GrVertex*)b, 
                        (GrVertex*)c );
    } else if ( a->flags & clipCode ) { /* out, ?, ? */
        if ( b->flags & clipCode ) { /* out, out, ? */
            if ( c->flags & clipCode ) { /* out, out, out */
                return;
            } else { /* out, out, in */
                intersect( clipCode, c, a, &v1 );
                intersect( clipCode, c, b, &v2 );
                _atrGlideClipAndRenderTri( c, &v1, &v2, clipCode >> 1 );
            }            
        } else { /* out, in, ? */
            if ( c->flags & clipCode ) { /* out, in, out */
                intersect( clipCode, b, a, &v1 );
                intersect( clipCode, b, c, &v2 );
                _atrGlideClipAndRenderTri( &v1, b, &v2, clipCode >> 1 );
            } else { /* out, in, in */
                intersect( clipCode, b, a, &v1 );
                intersect( clipCode, c, a, &v2 );
                _atrGlideClipAndRenderTri( &v1, b, c, clipCode >> 1 );
                _atrGlideClipAndRenderTri( &v1, c, &v2, clipCode >> 1 );
            }            
        }
    } else { /* in, ?, ? */
        if ( b->flags & clipCode ) { /* in, out, ? */
            if ( c->flags & clipCode ) { /* in, out, out */
                intersect( clipCode, a, b, &v1 );
                intersect( clipCode, a, c, &v2 );
                _atrGlideClipAndRenderTri( a, &v1, &v2, clipCode >> 1 );
            } else { /* in, out, in */
                intersect( clipCode, a, b, &v1 );
                intersect( clipCode, c, b, &v2 );
                _atrGlideClipAndRenderTri( a, &v1, c, clipCode >> 1 );
                _atrGlideClipAndRenderTri( &v1, &v2, c, clipCode >> 1 );
            }            
        } else { /* in, in */
            if ( c->flags & clipCode ) { /* in, in, out */
                intersect( clipCode, a, c, &v1 );
                intersect( clipCode, b, c, &v2 );
                _atrGlideClipAndRenderTri( &v1, a, b, clipCode >> 1 );
                _atrGlideClipAndRenderTri( &v1, b, &v2, clipCode >> 1 );
            } else { /* in, in, in */
                _atrGlideClipAndRenderTri( a, b, c, clipCode >> 1 );
            }            
        }
    }
    return;
}


/*-------------------------------------------------------------------
  Function: _atrGlideClipAndRenderTriWF
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
void _atrGlideClipAndRenderTriWF( AtrDstVertex *a,
                             AtrDstVertex *b,
                             AtrDstVertex *c,
                             FxU32 clipCode ) {

    _atrGlideClipAndRenderSegment( a, b, clipCode );
    _atrGlideClipAndRenderSegment( b, c, clipCode );
    _atrGlideClipAndRenderSegment( c, a, clipCode );
    return;
}


/*-------------------------------------------------------------------
  Function: _atrGlideClipAndRenderSegment
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
void _atrGlideClipAndRenderSegment(AtrDstVertex *a,
                              AtrDstVertex *b,
                              FxU32 clipCode ) {
    AtrDstVertex v1;

    if ( clipCode == 0 ) {
        AtrDstVertex a_, b_;
        if ( a->flags & ATR_CC_CLIPPED ) {
            _atrClipProject( &a_, a );
            a = &a_;
        }
        if ( b->flags & ATR_CC_CLIPPED ) {
            _atrClipProject( &b_, b );
            b = &b_;
        }
        grDrawLine( (GrVertex*)a, (GrVertex*)b ); 
    } else if ( a->flags & clipCode ) { /* out, ? */
        if ( b->flags & clipCode ) {    /* out, out */
            return;
        } else {                        /* out, in */
            intersect( clipCode, b, a, &v1 ); 
            _atrGlideClipAndRenderSegment( b, &v1, clipCode >> 1 );
        }
    } else {                             /* in, ? */
        if ( b->flags & clipCode ) {     /* in, out */
            intersect( clipCode, a, b, &v1 );
            _atrGlideClipAndRenderSegment( a, &v1, clipCode >> 1 );
        } else {                         /* in, in */
            _atrGlideClipAndRenderSegment( a, b, clipCode >> 1 );
        }
    }
    return;
}

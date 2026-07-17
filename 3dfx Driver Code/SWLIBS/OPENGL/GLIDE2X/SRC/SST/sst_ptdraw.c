/*
** Copyright 1991-1997, Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 7:54:24 PM$
*/
#include <windows.h>
#include "render.h"
#include "context.h"
#include "global.h"
#include "imports.h"
#include <glide.h>
#include "sst_globals.h"

void __glSSTRenderAliasedPoint1_NoTex(__GLcontext *gc, __GLvertex *vx)
{
    GrVertex vtx;

    vtx.x = vx->window.x;
    vtx.y = vx->window.y;
    vtx.ooz = vx->window.z;
    vtx.r = vx->color->r;
    vtx.g = vx->color->g;
    vtx.b = vx->color->b;
    vtx.a = vx->color->a;

    grDrawPoint(&vtx);
}

void __glSSTRenderAntiAliasedPoint1_NoTex(__GLcontext *gc, __GLvertex *vx)
{
    GrVertex vtx;

    vtx.x = vx->window.x;
    vtx.y = vx->window.y;
    vtx.ooz = vx->window.z;
    vtx.r = vx->color->r;
    vtx.g = vx->color->g;
    vtx.b = vx->color->b;
    vtx.a = vx->color->a;

    grAADrawPoint(&vtx);
}

void __glSSTRenderAliasedPoint1(__GLcontext *gc, __GLvertex *vx)
{
    GrVertex vtx;

    vtx.x = vx->window.x;
    vtx.y = vx->window.y;
    vtx.ooz = vx->window.z;
    vtx.oow = vx->texture[0].w;
    vtx.r = vx->color->r;
    vtx.g = vx->color->g;
    vtx.b = vx->color->b;
    vtx.a = vx->color->a;
    vtx.tmuvtx[0].sow = vx->texture[0].x;
    vtx.tmuvtx[0].tow = vx->texture[0].y;

    grDrawPoint(&vtx);
}

void __glSSTRenderAntiAliasedPoint1(__GLcontext *gc, __GLvertex *vx)
{
    GrVertex vtx;

    vtx.x = vx->window.x;
    vtx.y = vx->window.y;
    vtx.ooz = vx->window.z;
    vtx.oow = vx->texture[0].w;
    vtx.r = vx->color->r;
    vtx.g = vx->color->g;
    vtx.b = vx->color->b;
    vtx.a = vx->color->a;
    vtx.tmuvtx[0].sow = vx->texture[0].x;
    vtx.tmuvtx[0].tow = vx->texture[0].y;

    grAADrawPoint(&vtx);
}

void __glSSTRenderAliasedPointN(__GLcontext *gc, __GLvertex *vx)
{
    GLint pointSize, pointSizeHalf, xLeft, xRight, yBottom, yTop;
    GLint cullBit, frontBit;
    GrVertex vtxA, vtxB, vtxC, vtxD;

    /*
    ** Compute the x and y starting coordinates for rendering the square.
    */
    pointSize = gc->state.point.aliasedSize;
    pointSizeHalf = pointSize >> 1;
    if (pointSize & 1) {
        /* odd point size */
        xLeft = (((GLint)(vx->window.x)) - pointSizeHalf);
        yBottom = (((GLint)(vx->window.y)) - pointSizeHalf);
    } else {
        /* even point size */
        xLeft = (((GLint)(vx->window.x+__glHalf)) - pointSizeHalf);
        yBottom = (((GLint)(vx->window.y+__glHalf)) - pointSizeHalf);
    }
    xRight = xLeft + pointSize;
    yTop = yBottom + pointSize;

    vtxA.r = vtxB.r = vtxC.r = vtxD.r = vx->color->r;
    vtxA.g = vtxB.g = vtxC.g = vtxD.g = vx->color->g;
    vtxA.b = vtxB.b = vtxC.b = vtxD.b = vx->color->b;
    vtxA.a = vtxB.a = vtxC.a = vtxD.a = vx->color->a;
    vtxA.ooz = vtxB.ooz = vtxC.ooz = vtxD.ooz = vx->window.z;
    vtxA.oow = vtxB.oow = vtxC.oow = vtxD.oow = vx->texture[0].w;
    vtxA.tmuvtx[0].sow = vtxB.tmuvtx[0].sow =
        vtxC.tmuvtx[0].sow = vtxD.tmuvtx[0].sow = vx->texture[0].x;
    vtxA.tmuvtx[0].tow = vtxB.tmuvtx[0].tow =
        vtxC.tmuvtx[0].tow = vtxD.tmuvtx[0].tow = vx->texture[0].y;

    vtxA.x = xLeft;
    vtxA.y = yBottom;
    vtxB.x = xLeft;
    vtxB.y = yTop;
    vtxC.x = xRight;
    vtxC.y = yBottom;
    vtxD.x = xRight;
    vtxD.y = yTop;
    
    /* use a winding that we know won't get culled */
    cullBit = (gc->state.polygon.cull == GL_FRONT);
    frontBit = (gc->state.polygon.frontFaceDirection == GL_CW);
    if (frontBit ^ cullBit) {
        /* positive area may be culled; draw cw */
        grDrawTriangle( &vtxC, &vtxA, &vtxB );
        grDrawTriangle( &vtxB, &vtxD, &vtxC );
    } else {
        /* negative area may be culled; draw ccw */
        grDrawTriangle( &vtxB, &vtxA, &vtxC );
        grDrawTriangle( &vtxC, &vtxD, &vtxB );
    }
}

void __glSSTRenderAntiAliasedPointN(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat radius, ang;
    __GLfloat x, y, prevx, prevy;
    GLint i, n, ccw, cullBit, frontBit;
    GrVertex vtxA, vtxB, vtxC;

    radius = gc->state.point.smoothSize / 2.0f;

    vtxA.r = vtxB.r = vtxC.r = vx->color->r;
    vtxA.g = vtxB.g = vtxC.g = vx->color->g;
    vtxA.b = vtxB.b = vtxC.b = vx->color->b;
    vtxA.a = vtxB.a = vtxC.a = vx->color->a;
    vtxA.ooz = vtxB.ooz = vtxC.ooz = vx->window.z;
    vtxA.oow = vtxB.oow = vtxC.oow = vx->texture[0].w;
    vtxA.tmuvtx[0].sow = vtxB.tmuvtx[0].sow = vtxC.tmuvtx[0].sow =
        vx->texture[0].x;
    vtxA.tmuvtx[0].tow = vtxB.tmuvtx[0].tow = vtxC.tmuvtx[0].tow =
        vx->texture[0].y;

    prevx = prevy = 0.0f;
    vtxA.x = vx->window.x;
    vtxA.y = vx->window.y;

    /* 
    ** XXXshui This is almost certainly wrong. We draw a number of 
    ** triangles with an antialiased outer edge.  Choose radius * 4
    ** because hopefully that covers every pixel along the perimeter
    ** of the circle.  Also need to honor gc.constants.pointSizeGranularity.
    */
    n = (int)(radius * 4 + __glHalf);

    /* use a winding that we know won't get culled */
    cullBit = (gc->state.polygon.cull == GL_FRONT);
    frontBit = (gc->state.polygon.frontFaceDirection == GL_CW);
    if (frontBit ^ cullBit) {
        /* positive area may be culled; draw cw */
        ccw = GL_FALSE;
    } else {
        /* negative area may be culled; draw ccw */
        ccw = GL_TRUE;
    }
    
    for (i=0; i <= n; i++) {
        ang = i / (float) n * 2.0 * __glPi;
        x = vx->window.x + radius * __GL_COSF(ang);
        y = vx->window.y + radius * __GL_SINF(ang);
        vtxB.x = prevx;
        vtxB.y = prevy;
        vtxC.x = x;
        vtxC.y = y;
        prevx = x;
        prevy = y;
        if (i == 0) continue;
        if (ccw) {
            grAADrawTriangle( &vtxA, &vtxB, &vtxC, FXFALSE, FXTRUE, FXFALSE);
        } else {
            grAADrawTriangle( &vtxA, &vtxC, &vtxB, FXFALSE, FXTRUE, FXFALSE);
        }
        }
}

float atmOOSqrt( float number ) {
    long i;
    float x2, y;
    float threehalfs = 1.5F;

    x2 = number * (float)0.5F;
    y = number;
    i = *(long *) &y;
    i = 0x5f3759df - (i>>1);
    y = *(float *)&i;

    y = y * (threehalfs - (x2 * y * y));
    y = y * (threehalfs - (x2 * y * y));
    return y;
}

void __glSSTComputePointSize(__GLcontext *gc, __GLvertex *vx) {
        __GLfloat dist, size;
        __GLfloat a, b, c, min, max, threshold;
        __GLtransform *tr = gc->transform.modelView;
        __GLfloat smoothSize;
        GLint aliasedSize;

        a = gc->state.point.a;
        b = gc->state.point.b;
        c = gc->state.point.c;
        min = gc->state.point.min;
        max = gc->state.point.max;
        threshold = gc->state.point.fadeThreshold;
        /* We need eye coordinates to do point parameters */
        (*tr->matrix.xf4)(&vx->eye, &vx->obj.x, &tr->matrix);

#if 1
        dist = __GL_SQRTF(vx->eye.x * vx->eye.x + vx->eye.y * vx->eye.y + vx->eye.z * vx->eye.z);
#else
        dist = -vx->eye.z;
#endif

#if 0
        dist = __GL_SQRTF(1.0f / (a + b * dist + c * dist * dist));
#else
        dist = atmOOSqrt(a + b * dist + c * dist * dist);
#endif

        size = dist * gc->state.point.smoothSize;

        if (size < threshold) {
                __GLfloat alpha;
                alpha = size/threshold;
                size = threshold;
                vx->color->a *= alpha*alpha;

        }
        if (size < min) {
                size = min;
        } else if (size > max) {
                size = max;
        }

        aliasedSize = gc->state.point.aliasedSize;
        smoothSize = gc->state.point.smoothSize;
        gc->state.point.aliasedSize = size;
        gc->state.point.smoothSize = size;

        gc->procs.renderPoint2(gc, vx);

        gc->state.point.aliasedSize = aliasedSize;
        gc->state.point.smoothSize = smoothSize;
}




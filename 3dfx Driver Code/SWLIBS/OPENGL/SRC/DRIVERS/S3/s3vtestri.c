/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
*/

#include "s3vcontext.h"
#include "global.h"
#include "xform.h"
#include "string.h"
#include "fr_modes.h"
#include "fr_tri.h"
#include "imports.h" /* for __GL_ABSF */
#include "s3vutil.h"
#include "s3virge.h"

#define __DEBUG_PRINT
#include "dbg.h"




/* Triangle-filling parameters and states: */
struct TessInfoRec {
    GLfloat halfArea;
    GLfloat invHalfArea;

    GLfloat uvScale;
    GLfloat dScale;
    GLfloat wScale;
    GLfloat uBaseScale;
    GLfloat vBaseScale;
    GLfloat ufix;
    GLfloat vfix;

    GLfloat uBase;
    GLfloat vBase;

    GLfloat dxAC;
    GLfloat dxAB;
    GLfloat dxBC;
    GLfloat dyAC;
    GLfloat dyAB;
    GLfloat dyBC;
    GLfloat invdyAC;
};





#define S3V_MAX_TEXELS 128


#define WSCALE 2048.0
#define __S3V_FLOAT_LTZ(flt) (*(GLint *)&(flt) < 0)
#define FLT_TO_FIX_SCALE(value_in, scale) ((GLuint)((GLfloat)(value_in) * scale))


static GLfloat  powersOfFour[10] = {(GLfloat)1.0,
                          (GLfloat)4.0,
                          (GLfloat)16.0,
                          (GLfloat)64.0,
                          (GLfloat)256.0,
                          (GLfloat)1024.0,
                          (GLfloat)4096.0,
                          (GLfloat)16384.0,
                          (GLfloat)65536.0,
                          (GLfloat)262144.0 };

static GLfloat negPowerOfTwo[32] ={(GLfloat) 1.0,
                          (GLfloat) 0.5,
                          (GLfloat) 0.25,
                          (GLfloat) 0.125,
                          (GLfloat) 0.0625,
                          (GLfloat) 0.03125,
                          (GLfloat) 0.015625 ,
                          (GLfloat) 0.0078125 ,
                          (GLfloat) 0.00390625,
                          (GLfloat) 0.001953125,
                          (GLfloat) 0.0009765625,
                          (GLfloat) 0.00048828125,
                          (GLfloat) 0.000244140625,
                          (GLfloat) 0.0001220703125,
                          (GLfloat) 0.00006103515625,
                          (GLfloat) 0.000030517578125,
                          (GLfloat) 1.52587890625e-5,
                          (GLfloat) 7.62939453125e-6,
                          (GLfloat) 3.814697265625e-6,
                          (GLfloat) 1.907348632813e-6,
                          (GLfloat) 9.536743164063e-7,
                          (GLfloat) 4.768371582031e-7,
                          (GLfloat) 2.384185791016e-7,
                          (GLfloat) 1.192092895508e-7,
                          (GLfloat) 5.960464477539e-8,
                          (GLfloat) 2.98023223877e-8,
                          (GLfloat) 1.490116119385e-8,
                          (GLfloat) 7.450580596924e-9,
                          (GLfloat) 3.725290298462e-9,
                          (GLfloat) 1.862645149231e-9,
                          (GLfloat) 9.313225746155e-10,
                          (GLfloat) 4.656612873077e-10};


#define S3VDIVEPS  1.0e-4
#define S3VUVEPSILON ((float)(1.0e-10))

// Calculate the new internal properties (x,y,z,w,a,r,g,b) of a vertex
// created in some place along the edge v0-v1 in UV space

#define NEWPROP(VN, V0, V1, PXY, PROP)                  \
            VN->PROP = V0->PROP + PXY * (V1->PROP - V0->PROP)


__inline GLint ICLAMP(GLint l)
{
    if (l < 0L)
        return 0L;
    else if (l > 0x7FFF)
        return 0x7FFF;
    else
        return l;
}

__inline GLint __S3V_CEIL(float x)
{
    if (x - (GLint)__GL_ABSF(x) > 0.0)
        return (GLint)__GL_ABSF(x) + 1;
    else
        return (GLint)__GL_ABSF(x);
}

//*****************************************************************************
//
// void FillSubTriangle
//
// HW Render a triangle (no left window edge clipping nor tesselation necessary)
//
// Vertices are ordered from highest y-value (a) to smallest y-value (c)
//
// bTrust indicates if this  triangle was generated from our tesselation code or
// from the left edge window clipping code and therefore we can't trust the
// deltas and area calculated in previuos stages.
//
//*****************************************************************************

static void FillSubTriangle(__GLcontext *gc, struct TessInfoRec *pInfo, 
                          __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, 
                          GLboolean notcw)
{
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    volatile __glS3VTriEngineRegisters *triEngine;
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap1, ysnap2;
    int ccw, reversed = 0;
    float xsnap1, invc;
    float p0, dp0, dp1, dpdx, dpdy;
    float ftmp;
    double dtmp;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    float FR_COLOR_ROUND = COLOR_ROUND_DITHER;
    __GLtri tr;

    if (*((int *)&v1->window.y) < *((int *)&v0->window.y)) {
	__GLvertex *tmp = v1; v1 = v0; v0 = tmp;
	reversed ^= 1;
    }
    if (*((int *)&v2->window.y) < *((int *)&v0->window.y)) {
	__GLvertex *tmp = v2; v2 = v0; v0 = tmp;
	reversed ^= 1;
    }
    if (*((int *)&v1->window.y) < *((int *)&v2->window.y)) {
	__GLvertex *tmp = v2; v2 = v1; v1 = tmp;
	reversed ^= 1;
    }

    xsnap1 = (float) ((int)(v1->window.x) + 1) - v1->window.x;
    dx0 = v1->window.x - v0->window.x;
    dy0 = v1->window.y - v0->window.y;

    dx1 = v2->window.x - v0->window.x;
    dy1 = v2->window.y - v0->window.y;

    dx2 = v1->window.x - v2->window.x;
    dy2 = v1->window.y - v2->window.y;

    c = -dx2*-dy0 - -dx0*-dy2;

    if ((*(int *)&c << 1) == 0) {
	return;
    }

    ccw = (1 ^ ((unsigned int) FloatBits(c) >> 31));
    tr.dx = 1 - (ccw << 1);

    dxdy0 = dy0 ? dx0 / dy0 : 0.0F;
    dxdy1 = dy1 ? dx1 / dy1 : 0.0F;
    dxdy2 = dy2 ? dx2 / dy2 : 0.0F;

    tr.y0Int = BiasedFloatToInt(v0->window.y, XY_FRAC_BITS);
    tr.y1Int = BiasedFloatToInt(v1->window.y, XY_FRAC_BITS);
    tr.y2Int = BiasedFloatToInt(v2->window.y, XY_FRAC_BITS);

    if ((tr.y0Int == tr.y1Int) &&
	(tr.y1Int == tr.y2Int))
	return;

    ysnap0 = (1.0f - ((float) (tr.y0Int + XY_BIAS + 1) - v0->window.y));
    ysnap1 = (1.0f - ((float) (tr.y1Int + XY_BIAS + 1) - v1->window.y));
    ysnap2 = (1.0f - ((float) (tr.y2Int + XY_BIAS + 1) - v2->window.y));


    // BEGIN MUTEX

    triEngine = hwcx->triEngine;

    WaitForQueue(7);

    x = v1->window.x + -dxdy0 * ysnap1;
    triEngine->XACStart = 
	BiasedFloatToFixed(x) << (20 - XY_FRAC_BITS);
    triEngine->dXAC = FloatToFixed(-dxdy0, 20);

    x = v1->window.x + -dxdy2 * ysnap1;
    triEngine->XABStart = 
	BiasedFloatToFixed(x) << (20 - XY_FRAC_BITS);
    triEngine->dXAB = FloatToFixed(-dxdy2, 20);

    x = v2->window.x + -dxdy1 * ysnap2;
    triEngine->XBCStart = 
	BiasedFloatToFixed(x) << (20 - XY_FRAC_BITS);
    triEngine->dXBC = FloatToFixed(-dxdy1, 20);

    triEngine->AY = tr.y1Int;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;
    dx2 *= invc;
    dy2 *= invc;

    { /* For Texture */
	__GLtexture *current = gc->texture.currentTexture;
	__GLmipMapLevel *lp = current->level[0];
        float vBaseHw, uBaseHw;
        float uMin, vMin;
        float p0, p1, p2;

	tr.tp            = (GLubyte *) lp->buffer;
	tr.texWidth      = lp->width;
	tr.texWidthLog2  = lp->widthLog2;
	tr.texHeight     = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

        if (modeFlags & __GL_SHADE_TEXTURE_MIPMAP) {
 
               /* We need rho registers */
        }

        uMin = v0->texture.x;
        if (uMin > v1->texture.x) uMin = v1->texture.x;
        if (uMin > v2->texture.x) uMin = v2->texture.x;

        vMin = v0->texture.y;
        if (vMin > v1->texture.y) vMin = v1->texture.y;
        if (vMin > v2->texture.y) vMin = v2->texture.y;

        // u and v base register can only take care of integer part of u and
        // v value.
        if (uMin < 0.0)
            pInfo->uBase = (float)__GL_ABSF(uMin - 1.0);
        else
            pInfo->uBase = (float)__GL_ABSF(uMin);
        if (vMin < 0.0)
            pInfo->vBase = (float)__GL_ABSF(vMin - 1.0);
        else
            pInfo->vBase = (float)__GL_ABSF(vMin);

        WaitForQueue(6);

        uBaseHw = pInfo->uBase + pInfo->ufix;
        vBaseHw = pInfo->vBase + pInfo->vfix;
        while (uBaseHw < 0.0) uBaseHw += tr.texWidth;
        while (vBaseHw < 0.0) vBaseHw += tr.texWidth;

        triEngine->texelUBase = (GLint)(uBaseHw * pInfo->uBaseScale);
        triEngine->texelVBase = (GLint)(vBaseHw * pInfo->vBaseScale);

	p0  = (v1->texture.x - pInfo->uBase)*v1->texture.w;
        p1  = (v0->texture.x - pInfo->uBase)*v0->texture.w;
        p2  = (v2->texture.x - pInfo->uBase)*v2->texture.w;

	dp0 = p1 - p0;
	dp1 = p2 - p0;

        /* For now, scale the tcoords in the triangle. We should be able to wrap 
           this into the texture transform later */

	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(3);
	triEngine->UStart = 
	    FloatToFixed((p0 + dpdy * ysnap1 + dpdx * xsnap1), /* 12 */ 12);
	triEngine->dUX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dUY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);

	p0  = (v1->texture.y - pInfo->vBase)*v1->texture.w;
        p1  = (v0->texture.y - pInfo->vBase)*v0->texture.w;
        p2  = (v2->texture.y - pInfo->vBase)*v2->texture.w;

	dp0 = p1 - p0;
	dp1 = p2 - p0;

	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(3);
	triEngine->VStart = 
	    FloatToFixed((p0 + dpdy * ysnap1 + dpdx * xsnap1), 12);
	triEngine->dVX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dVY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);

	p0  = v1->texture.w;
	dp0 = v0->texture.w - p0;
	dp1 = v2->texture.w - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(3);
	triEngine->WStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 19);
	triEngine->dWX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 19);
	triEngine->dWY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 19);
    }

    { /* For Smooth Shade */
	p0 = v1->color->r;
	dp0 = v0->color->r - p0;
	dp1 = v2->color->r - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);
	tr.r0 = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1 + FR_COLOR_ROUND,
			 COLOR_FRAC_BITS);
	tr.drdy0[0] = FloatToFixed(dpdy + (dxdy0 * -dpdx), COLOR_FRAC_BITS);
	tr.drdx = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), COLOR_FRAC_BITS);

	p0 = v1->color->g;
	dp0 = v0->color->g - p0;
	dp1 = v2->color->g - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);
	tr.g0 = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1 + FR_COLOR_ROUND,
			 COLOR_FRAC_BITS);
	tr.dgdy0[0] = FloatToFixed(dpdy + (dxdy0 * -dpdx), COLOR_FRAC_BITS);
	tr.dgdx = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), COLOR_FRAC_BITS);

	p0 = v1->color->b;
	dp0 = v0->color->b - p0;
	dp1 = v2->color->b - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);
	tr.b0 =
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1 + FR_COLOR_ROUND,
			 COLOR_FRAC_BITS);
	tr.dbdy0[0] = FloatToFixed(dpdy + (dxdy0 * -dpdx), COLOR_FRAC_BITS);
	tr.dbdx = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), COLOR_FRAC_BITS);

        WaitForQueue(6);
	triEngine->ARStart = 0x7fff0000 | (tr.r0 >> 6);
	triEngine->dARY = (tr.drdy0[0] >> 6) & 0xffff;
	triEngine->dARX = (tr.drdx >> 6) & 0xffff;

	triEngine->BGStart = (((tr.g0 >> 6) & 0xffff) << 16) | (tr.b0 >> 6);
	triEngine->dBGY = (((tr.dgdy0[0] >> 6) & 0xffff) << 16) | (tr.dbdy0[0] >> 6) & 0xffff;
	triEngine->dBGX = (((tr.dgdx >> 6) & 0xffff) << 16) | (tr.dbdx >> 6) & 0xffff;
    }

    { /* For Depth Test */

	p0 = v1->window.z;
	dp0 = v0->window.z - p0;
	dp1 = v2->window.z - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);
	tr.z0 =
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 0) >> 1;
        WaitForQueue(4);
	triEngine->ZStart = tr.z0;
	tr.dzdx = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 0) >> 1;
	triEngine->dZX = tr.dzdx;
	tr.dzdy0[0] = FloatToFixed(dpdy + (dxdy0 * -dpdx), 0) >> 1;
	triEngine->dZY = tr.dzdy0[0];
   }

    triEngine->Heights =
	((tr.dx > 0) ? 0x80000000 : 0x00000000) | 
	(tr.y2Int - tr.y0Int) |
	((tr.y1Int - tr.y2Int) << 16);


    /* set up the command register with the value calculated at pick time */

    triEngine->cmd = hwcx->hwCmdMask;

    // END MUTEX
}




//**************************************************************************
// float UVSlope
//
// Return appropriate slope to use if we are going to travel mainly along
// U or V for a specific edge v0-v1
//
//**************************************************************************
static float UVSlope(__GLvertex *v0, __GLvertex *v1, GLboolean uGTv)
{
    if (uGTv)
        // dy/dx
        return (v0->texture.y - v1->texture.y) /
               (v0->texture.x - v1->texture.x);
    else
        // dx/dy
        return (v0->texture.x - v1->texture.x) /
               (v0->texture.y - v1->texture.y);
}

//**************************************************************************
// void __S3VNew_UV_VertexProps
//
// Calculate the properties of a new vertex based on UV subdivision
//
//**************************************************************************
static void New_UV_VertexProps(__GLcontext *gc,
                             __GLvertex *vNew,
                             __GLvertex *v0, __GLvertex *v1, GLboolean uGTv)
{
    float perspUVSpace, perspXYSpace, s0, s1, sn;
    GLint modeFlags = gc->polygon.shader.modeFlags;

    // calculate new w coordinate
    if (uGTv) {
        s0 = v0->texture.x;
        s1 = v1->texture.x;
        sn = vNew->texture.x;
    } else {
        s0 = v0->texture.y;
        s1 = v1->texture.y;
        sn = vNew->texture.y;
    }

    if (__GL_ABSF(s1 - s0) > S3VUVEPSILON)
        perspUVSpace = (sn - s0) / (s1 - s0);
    else
        perspUVSpace = 0.0;

    vNew->window.w = (v1->window.w * v0->window.w) /
                     (perspUVSpace * v0->window.w +
                     (1.0 - perspUVSpace) * v1->window.w);

    // calculate perspective correction factor in XY space
    if (uGTv) {
        s0 = v0->texture.x * v0->window.w;
        s1 = v1->texture.x * v1->window.w;
        sn = vNew->texture.x * vNew->window.w;
    } else {
        s0 = v0->texture.y * v0->window.w;
        s1 = v1->texture.y * v1->window.w;
        sn = vNew->texture.y * vNew->window.w;
    }

    if (__GL_ABSF(s1 - s0) > S3VUVEPSILON)
        perspXYSpace = (sn - s0) / (s1 - s0);
    else
        perspXYSpace = 0.0;

    NEWPROP(vNew, v0, v1, perspXYSpace, texture.w);

    // calculate new spatial coordinates
    NEWPROP(vNew, v0, v1, perspXYSpace, window.x);
    NEWPROP(vNew, v0, v1, perspXYSpace, window.y);
    NEWPROP(vNew, v0, v1, perspXYSpace, window.z);

    // calculate new color coordinates, if neccesary
    if (modeFlags & __GL_SHADE_SMOOTH) {
        NEWPROP(vNew, v0, v1, perspXYSpace, colors[0].r);
        NEWPROP(vNew, v0, v1, perspXYSpace, colors[0].g);
        NEWPROP(vNew, v0, v1, perspXYSpace, colors[0].b);
        NEWPROP(vNew, v0, v1, perspXYSpace, colors[0].a);
        vNew->color = &vNew->colors[0];
    } 
}

//**************************************************************************
// GLboolean __S3V_SplitUVEdge
//
// Calculates a new vertex (vNew) along the edge between v0 and v1 and the next
// in sequence according to vNew's old value , calculate the new properties of
// this point and updates the basevertex for the next calculations!
// Returns TRUE if a new vertex was created,FALSE if v1 has already been reached
//
//**************************************************************************
static GLboolean 
SplitUVEdge(__GLcontext *gc, 
                  __GLvertex *vNew, 
                       __GLvertex *voldnew, __GLvertex *v0, __GLvertex *v1, 
                       GLboolean uGTv, float derivate,
                       float uvIncr, __GLvertex *basevertex)
{
    float delta;

#ifdef DEBUG1
    ErrorF("TEX PERSP PART:S3VUVSplitUVEdge");
#endif
    if (uGTv) { // if delta_u >> delta_v
        if (__GL_ABSF(voldnew->texture.x - v1->texture.x) < S3VUVEPSILON) {
            *vNew = *v1;
            return GL_FALSE; // nothing to do, there is no new vertex to create!
        }
        if (v0->texture.x < v1->texture.x) {
            if ((vNew->texture.x = basevertex->texture.x + uvIncr) > v1->texture.x)
                *vNew = *v1;
            else {
                delta = uvIncr - (voldnew->texture.x - basevertex->texture.x);
                vNew->texture.y = voldnew->texture.y + delta * derivate;
                // calculate new properties (w,r,g,b,a,x,y,z)
                New_UV_VertexProps(gc, vNew, v0, v1, uGTv);
            }

        } 
        else {
            if ((vNew->texture.x = basevertex->texture.x - uvIncr) < v1->texture.x)
                *vNew = *v1;
            else {
                delta =-uvIncr + (basevertex->texture.x - voldnew->texture.x);
                vNew->texture.y = voldnew->texture.y + delta * derivate;
                // calculate new properties (w,r,g,b,a,x,y,z)
                New_UV_VertexProps(gc, vNew, v0, v1, uGTv);
            }
        }
    } else { // if delta_v > delta_u
        if (__GL_ABSF(voldnew->texture.y - v1->texture.y) < S3VUVEPSILON) {
            *vNew = *v1;
            return GL_FALSE; // nothing to do, there is no new vertex to create!
        }
        if (v0->texture.y < v1->texture.y) {
            if ((vNew->texture.y = basevertex->texture.y + uvIncr) >
                v1->texture.y)
                *vNew = *v1;
            else {
                delta = uvIncr - (voldnew->texture.y - basevertex->texture.y);
                vNew->texture.x = voldnew->texture.x + delta * derivate;
                // calculate new properties (w,r,g,b,a,x,y,z)
                New_UV_VertexProps(gc, vNew, v0, v1, uGTv);
            }
        } else {
            if ((vNew->texture.y = basevertex->texture.y - uvIncr) <
                v1->texture.y)
                *vNew = *v1;
            else {
                delta = -uvIncr + (basevertex->texture.y - voldnew->texture.y);
                vNew->texture.x = voldnew->texture.x + delta * derivate;
                // calculate new properties (w,r,g,b,a,x,y,z)
                New_UV_VertexProps(gc, vNew, v0, v1, uGTv);
            }
        }
    }
    return GL_TRUE; // a new vertex was created
}

//**************************************************************************
// void __S3VUVSplitTrapezoid
//
// Split the trapezoid formed by the base edge b and the top edge a with start
// at the v?0 vertexes and ending at the v?f vertexes. Care must be given if
// the trapezoid is degenerated (vA0 == vAf in values or vB0 == vBf in values)
//
//**************************************************************************
static void UVSplitTrapezoid(__GLcontext *gc, struct TessInfoRec *pInfo,
                           __GLvertex *vA0, 
                           __GLvertex *vAf, __GLvertex *vB0,__GLvertex *vBf,
                           float uvMaxLen, float uvIncr, float uvIncrInv)
{
    float      derEdgeA, derEdgeB;
    __GLvertex vBaseA, vBaseB, vNewTestA, vNewTestB;
    GLboolean       edgeXLargeA, edgeXLargeB, incrA, incrB, lastSplitEdgeWasA;
    GLint       piecesEdgeA, piecesEdgeB, totalPieces, iTriCnt;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;

#ifdef DEBUG1
    ErrorF("TEX PERSP PART:S3VUVSplitTrapezoid");
#endif

    // Edge a properties
    if (__GL_ABSF(vAf->texture.x - vA0->texture.x) >
        __GL_ABSF(vAf->texture.y - vA0->texture.y)) {
        // trapezoid basis is u-large
        edgeXLargeA = GL_TRUE;
        piecesEdgeA =
            __S3V_CEIL(__GL_ABSF(vAf->texture.x - vA0->texture.x) * uvIncrInv);
    } else {
        // trapezoid basis is v-large
        edgeXLargeA = GL_FALSE;
        piecesEdgeA =
            __S3V_CEIL(__GL_ABSF(vAf->texture.y - vA0->texture.y) * uvIncrInv);
    }
    //beware if degenerate
    if (piecesEdgeA) derEdgeA = UVSlope(vAf, vA0, edgeXLargeA);

    // Edge b properties
    if (__GL_ABSF(vBf->texture.x - vB0->texture.x) >
        __GL_ABSF(vBf->texture.y - vB0->texture.y)) {
        // trapezoid basis is u-large
        edgeXLargeB = GL_TRUE;
        piecesEdgeB =
            __S3V_CEIL(__GL_ABSF(vBf->texture.x - vB0->texture.x) * uvIncrInv);
    } else {
        // trapezoid basis is v-large
        edgeXLargeB = GL_FALSE;
        piecesEdgeB =
            __S3V_CEIL(__GL_ABSF(vBf->texture.y - vB0->texture.y) * uvIncrInv);
    }
    //beware if degenerate
    if (piecesEdgeB) derEdgeB = UVSlope(vBf, vB0, edgeXLargeB);


    // establish base vertexes to start stripe tesselation
    vBaseA =  *vA0;
    vBaseB =  *vB0;
    // we set the limit of pieces created beforehand
    totalPieces = piecesEdgeA + piecesEdgeB;

    lastSplitEdgeWasA = GL_FALSE;

    for (iTriCnt=0;iTriCnt < totalPieces ;iTriCnt++) {

        if (piecesEdgeA) { // we can still create new pieces along edge a
            SplitUVEdge(gc, &vNewTestA, &vBaseA, vA0, vAf,
                              edgeXLargeA, derEdgeA, uvIncr, &vBaseA);
            //check if new diagonal meets constraints
            incrA =(__GL_ABSF(vNewTestA.texture.x - vBaseB.texture.x) <= uvMaxLen)&&
                   (__GL_ABSF(vNewTestA.texture.y - vBaseB.texture.y) <= uvMaxLen);
        } else
            incrA = GL_FALSE;

        if (piecesEdgeB) { // we can still create new pieces along edge b
            SplitUVEdge(gc, &vNewTestB, &vBaseB, vB0, vBf,
                              edgeXLargeB, derEdgeB, uvIncr, &vBaseB);
            //check if new diagonal meets constraints
            incrB =(__GL_ABSF(vNewTestB.texture.x - vBaseA.texture.x) <= uvMaxLen)&&
                   (__GL_ABSF(vNewTestB.texture.y - vBaseA.texture.y) <= uvMaxLen);
        } else
            incrB = GL_FALSE;

        if (incrA && (!incrB || !lastSplitEdgeWasA)) {
            FillSubTriangle(gc, pInfo, &vNewTestA, &vBaseA, &vBaseB, GL_FALSE);
            lastSplitEdgeWasA = GL_TRUE;
            vBaseA = vNewTestA;
            piecesEdgeA--;
        } else if (incrB && (!incrA || lastSplitEdgeWasA)) {
            FillSubTriangle(gc, pInfo, &vNewTestB, &vBaseB, &vBaseA, GL_FALSE);
            lastSplitEdgeWasA = GL_FALSE;
            vBaseB = vNewTestB;
            piecesEdgeB--;
        } else {
            // This sould not happen but just in case we handle it.
            ErrorF("Could not continue processing Trapezoid stripe!");
        }
    }
}


//**************************************************************************
// void  __S3VUVSplitTriangle
//
// Checks if a triangle requires tesselation in order not to span the
// maximum of texels the S3Virge allows us for a single textured triangle.
// If it does require tesselation, it goes on to do it according to the
// number of edges which violate the constraint.
//
//**************************************************************************
void __glS3VTexPerspTessellatedTriangle(__GLcontext *gc, __GLvertex *a, 
					__GLvertex *b, __GLvertex *c)
{
    __GLcoord aTexOld, bTexOld, cTexOld;
    GLboolean ccw;
    float uvMaxLen, uvIncr, uvIncrInv, uLen[3], vLen[3], derEdge1, derEdge2;
    __GLvertex *v[3], vNew, vNew1, vNew2, vBase;
    __GLvertex vLast1, vLast2, vNewApex1, vNewApex2;
    GLboolean     uGTv[3], vGTu[3];
    GLboolean     edgeXLarge1, edgeXLarge2, bvNew1, bvNew2;
    GLint     vLargeSideI, vLargeSideF, vSmallSideI, vSmallSideF;
    GLint     vOppositeLarge, vOppositeSmall;
    GLint     bigSidesCount, vInd, vInd1, vInd2;
    GLint     bLeftToRight, bBottomToTop, bHorizontal, bVertical, bMixed;
    GLint modeFlags = gc->polygon.shader.modeFlags;
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    GLint  reversed;
    __GLvertex *temp;
    int texWidth      = gc->texture.currentTexture->level[0]->width;
    int texWidthLog2  = gc->texture.currentTexture->level[0]->widthLog2;
    struct TessInfoRec tessInfo;

    tessInfo.uBaseScale = (GLfloat)(1 << (16 - texWidthLog2));
    tessInfo.vBaseScale = (GLfloat)(1 << (16 - texWidthLog2));
    tessInfo.wScale     = (GLfloat)(1 << 19);

    tessInfo.uvScale    = (GLfloat)(1 << 12);
    tessInfo.ufix = (GLfloat)(-0.5f) * negPowerOfTwo[texWidthLog2];
    if (texWidthLog2 >= 8) {
        tessInfo.vfix = (GLfloat)(-0.5f)*negPowerOfTwo[texWidthLog2];
    }
    else {
        tessInfo.vfix = (GLfloat)(-0.496093f)*negPowerOfTwo[texWidthLog2];
    }

    reversed = 0;                                                           
    if (*((GLint *)&(c)->window.y) <= *((GLint *)&(b)->window.y)) {           
        if (*((GLint *)&(b)->window.y) <= *((GLint *)&(a)->window.y)) {       
            /* Already sorted */                                            
        } else {                                                            
            if (*((GLint *)&(c)->window.y) <= *((GLint *)&(a)->window.y)) {   
                temp = (b); (b) = (a); (a) = temp;                          
                reversed = 1;                                               
            } else {                                                        
                temp = (c); (c) = (a); (a) = (b); (b) = temp;               
            }                                                               
        }                                                                   
    } else {                                                                
        if (*((GLint *)&(b)->window.y) <= *((GLint *)&(a)->window.y)) {       
            if (*((GLint *)&(c)->window.y) <= *((GLint *)&(a)->window.y)) {   
                temp = (c); (c) = (b); (b) = temp;                          
                reversed = 1;                                               
            } else {                                                        
                temp = (c); (c) = (b); (b) = (a); (a) = temp;               
            }                                                               
        } else {                                                            
            temp = (c); (c) = (a); (a) = temp;                              
            reversed = 1;                                                   
        }                                                                   
    }                                                                       
                                                                            
    // Later the inverse of dyAC will be needed so start computing it
    tessInfo.dyAC = c->window.y - a->window.y;
    tessInfo.dxAC = c->window.x - a->window.x;
    tessInfo.dxBC = c->window.x - b->window.x;
    tessInfo.dxAB = b->window.x - a->window.x;
    tessInfo.dyBC = c->window.y - b->window.y;
    tessInfo.dyAB = b->window.y - a->window.y;

    tessInfo.halfArea = tessInfo.dxAB * tessInfo.dyAC - tessInfo.dxAC * tessInfo.dyAB;
    ccw = !__S3V_FLOAT_LTZ(tessInfo.halfArea);
                                                                            
#ifdef DEBUG1
    ErrorF("Entry to S3VUVSplit Triangle");
    ErrorF("Vertex 1 u=%i  v=%i", 
                     (long)(a->texture.x * 1000.0), 
                     (long)(a->texture.y*1000.0));
    ErrorF("Vertex 2 u=%i  v=%i", 
                     (long)(b->texture.x * 1000.0),
                     (long)(b->texture.y*1000.0));
    ErrorF("Vertex 3 u=%i  v=%i", 
                     (long)(c->texture.x * 1000.0),
                     (long)(c->texture.y*1000.0));
#endif

    // Save the original texture coordinates.
    if ((modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) ||
        !(modeFlags & __GL_SHADE_TEXTURE_UVSCALED))
    {
        aTexOld.x = a->texture.x;
        aTexOld.y = a->texture.y;
        bTexOld.x = b->texture.x;
        bTexOld.y = b->texture.y;
        cTexOld.x = c->texture.x;
        cTexOld.y = c->texture.y;
    }

    // Need to divide by window.w if doing the perspective correction.
    if (modeFlags & __GL_SHADE_TEXTURE_PROJSCALED)
    {
        a->texture.x /= a->texture.w;
        a->texture.y /= a->texture.w;
        b->texture.x /= b->texture.w;
        b->texture.y /= b->texture.w;
        c->texture.x /= c->texture.w;
        c->texture.y /= c->texture.w;
    }

    // If the texture coordinates are not scaled, scale them.
    if (!(modeFlags & __GL_SHADE_TEXTURE_UVSCALED))
    {
        a->texture.x *= texWidth;
        a->texture.y *= texWidth;
        b->texture.x *= texWidth;
        b->texture.y *= texWidth;
        c->texture.x *= texWidth;
        c->texture.y *= texWidth;
    }

    // For mip mapping we should change this somehow
    uvMaxLen = S3V_MAX_TEXELS;

    bigSidesCount = 0;
    v[0] = a;
    v[1] = b;
    v[2] = c;

    for(vInd=0; vInd < 3; vInd++) {
        vInd1 = (vInd + 1) % 3;
        vInd2 = (vInd + 2) % 3;
        uLen[vInd] = __GL_ABSF(v[vInd1]->texture.x - v[vInd2]->texture.x);
        vLen[vInd] = __GL_ABSF(v[vInd1]->texture.y - v[vInd2]->texture.y);
        if ((uLen[vInd] > uvMaxLen) || (vLen[vInd] > uvMaxLen)) {
            bigSidesCount++;       // This branch is taken only once for
            vLargeSideI = vInd1;   // triangles which have 1 edge
            vLargeSideF = vInd2;   // exceeding the UV constraint
            vOppositeLarge  = vInd;
            uGTv[vInd] = uLen[vInd] > vLen[vInd];
            vGTu[vInd] = vLen[vInd] > uLen[vInd];
        } else {
            vSmallSideI     = vInd1;   // This branch is taken only once for
            vSmallSideF     = vInd2;   // triangles which have 2 edges
            vOppositeSmall  = vInd;    // exceeding the UV constraint
        }
    }

    switch(bigSidesCount) {
        case 0:
            // Trivial case, the triangle doesnt exceed UV space constraints
#ifdef DEBUG1
            ErrorF("TEX PERSP PART(0):No splitting");
#endif
            FillSubTriangle(gc, &tessInfo, a, b, c, ccw);
            break;
        case 1:
            // Only one side exceeds constraints, it will be midpoint
            // subdivided and two new triangles created which are guaranteed
            // to satisfy constraints
            ErrorF("TEX PERSP PART(1):Start edge splitting");

            vNew.texture.x = 0.5 * (v[vLargeSideI]->texture.x +
                             v[vLargeSideF]->texture.x);
            vNew.texture.y = 0.5 * (v[vLargeSideI]->texture.y +
                             v[vLargeSideF]->texture.y);
            New_UV_VertexProps(gc, &vNew, v[vLargeSideI],
                                    v[vLargeSideF],uGTv[vOppositeLarge]);
            // Must recalculate area & bCCW for new two subtriangles
            FillSubTriangle(gc, &tessInfo, v[vLargeSideI], &vNew, 
			    v[vOppositeLarge], GL_FALSE);
            FillSubTriangle(gc, &tessInfo, &vNew, v[vLargeSideF], 
			    v[vOppositeLarge], GL_FALSE);
            break;
        case 2:
            // Two edges exceed constraints, we will first cut a triangle out
            // of the apex (vOppositeSmall) and then create pairs of triangles
            // until we get to the opposite edge to the apex vertex.

            ErrorF("TEX PERSP PART(2):Start edge splitting");

            // Establish first the base vertex of our virtual square equal
            // to the apex vertex
            vLast1 = vLast2 = vBase = *(v[vOppositeSmall]);
            edgeXLarge1 = uGTv[vSmallSideF];
            edgeXLarge2 = uGTv[vSmallSideI];

            derEdge1 = UVSlope(v[vOppositeSmall], v[vSmallSideI], edgeXLarge1);
            derEdge2 = UVSlope(v[vOppositeSmall], v[vSmallSideF], edgeXLarge2);

            // Determine if both edges which exceed the constraint run
            // horizontal or vertically. If they run in mixed mode then
            // we need to handle the base vertex to build valid subtriangles
            // which don't exceed the constraints. Only one of bHorizontal,
            // bVertical and bMixed is going to be GL_TRUE.

            bHorizontal = edgeXLarge1 && edgeXLarge2;
            bVertical   = (!edgeXLarge1) && (!edgeXLarge2);
            bMixed      = (!bHorizontal) && (!bVertical);

            if (bHorizontal)
                bLeftToRight =
                    (v[vOppositeSmall]->texture.x < v[vSmallSideI]->texture.x);

            if (bVertical)
                bBottomToTop =
                    (v[vOppositeSmall]->texture.y < v[vSmallSideI]->texture.y);

            // Do the first splitting of the edges creating an apex subtriangle

            SplitUVEdge(gc, &vNew1, &vLast1,
                              v[vOppositeSmall], v[vSmallSideI],
                              edgeXLarge1, derEdge1, uvMaxLen, &vBase);
            SplitUVEdge(gc, &vNew2, &vLast2,
                              v[vOppositeSmall], v[vSmallSideF],
                              edgeXLarge2, derEdge2, uvMaxLen, &vBase);

            // Must recalculate area & bCCW for our first triangle
            FillSubTriangle(gc, &tessInfo, v[vOppositeSmall], &vNew1, 
                             &vNew2, GL_FALSE);

            // Now we partition the remaining  trapezoid until we reach the
            // small edge. This partitioning must be done at least once,
            // otherwise we would have not fallen in this particular case
            // the trapezoid will be tesselated in stripes, each stripe
            // containing 2 or less triangles

            do {
                // update working vertexes
                vLast1 = vNew1;
                vLast2 = vNew2;

                // Update the base vertex from which we are going to build up
                // the next virtual square to contain our sub triangles

                if (bHorizontal)
                    if ( bLeftToRight )
                        vBase.texture.x =
                            max(vNew1.texture.x, vNew2.texture.x);
                    else
                        vBase.texture.x =
                            min(vNew1.texture.x, vNew2.texture.x);

                if (bVertical)
                    if ( bBottomToTop )
                        vBase.texture.y =
                            max(vNew1.texture.y, vNew2.texture.y);
                    else
                        vBase.texture.y =
                            min(vNew1.texture.y, vNew2.texture.y);

                if (bMixed)
                    if (edgeXLarge2) {
                            vBase.texture.x = vNew1.texture.x;
                            vBase.texture.y = vNew2.texture.y;
                    } else {
                            vBase.texture.x = vNew2.texture.x;
                            vBase.texture.y = vNew1.texture.y;
                    }

                // Calculate new vertexes!
                bvNew1 = SplitUVEdge(gc, &vNew1, &vLast1,
                                           v[vOppositeSmall], v[vSmallSideI],
                                           edgeXLarge1, derEdge1, uvMaxLen,
                                           &vBase);
                bvNew2 = SplitUVEdge(gc, &vNew2, &vLast2,
                                           v[vOppositeSmall], v[vSmallSideF],
                                           edgeXLarge2, derEdge2, uvMaxLen,
                                           &vBase);

                // If vNew1 is indeed new then create a subtriangle
                // with vLast1,vNew1 and vLast2 and send it to hw rendering
                if (bvNew1)
                    FillSubTriangle(gc, &tessInfo, &vLast1, &vNew1, 
				    &vLast2, GL_FALSE);

                // if vNew2 is indeed new and vNew1 is also new then create a
                // subtriangle with vLast2,vNew2 and vNew1 , otherwise do it
                // with vLast2,vNew2 and vLast1. Send it to hw rendering
                if (bvNew2)
                    if (bvNew1)
                        FillSubTriangle(gc, &tessInfo, &vLast2, &vNew2, 
					&vNew1, GL_FALSE);
                    else
                        FillSubTriangle(gc, &tessInfo, &vLast2, &vNew2, 
					&vLast1, GL_FALSE);

            } while (bvNew1 || bvNew2);

            break;
        case 3:
            // All three edges exceed the constraints. This is a generalized
            // case of the above case, in which in each stripe we create not
            // 2 but maybe more subtriangles, according to the length of each
            // stripes side.

            // Choose first a starting vertex, choose the one opposite to
            // the smallest side using x2+y2 metric (instead of max(x,y))

            vOppositeSmall = 0;

            if (( uLen[vOppositeSmall] * uLen[vOppositeSmall] +
                  vLen[vOppositeSmall] * vLen[vOppositeSmall] ) >
                ( uLen[1] * uLen[1] + vLen[1] * vLen[1]) )
                vOppositeSmall = 1;

            if (( uLen[vOppositeSmall] * uLen[vOppositeSmall] +
                  vLen[vOppositeSmall] * vLen[vOppositeSmall] ) >
                ( uLen[2] * uLen[2] + vLen[2] * vLen[2]) )
                vOppositeSmall = 2;

            vSmallSideI = (vOppositeSmall + 1) % 3;
            vSmallSideF = (vOppositeSmall + 2) % 3;


            // The base vertex of each edge is going to be equal to new vertex
            vLast1 = vLast2 = *(v[vOppositeSmall]);
            edgeXLarge1 = uGTv[vSmallSideF];
            edgeXLarge2 = uGTv[vSmallSideI];

            derEdge1 = UVSlope(v[vOppositeSmall], v[vSmallSideI], edgeXLarge1);
            derEdge2 = UVSlope(v[vOppositeSmall], v[vSmallSideF], edgeXLarge2);

            if ((edgeXLarge1 ^ edgeXLarge2) &&
               (vGTu[vSmallSideF] ^ vGTu[vSmallSideI]))
                // large in opposite directions
                uvIncr = uvMaxLen * 0.5;
            else
                // both large in same direction
                uvIncr = uvMaxLen;

            uvIncrInv = 1.0 / uvIncr;

            // Calculate new vertexes!
            bvNew1 = SplitUVEdge(gc, &vNew1, &vLast1,
                                       v[vOppositeSmall], v[vSmallSideI],
                                       edgeXLarge1, derEdge1, uvIncr,
                                       &vLast1);
            bvNew2 = SplitUVEdge(gc, &vNew2, &vLast2,
                                       v[vOppositeSmall], v[vSmallSideF],
                                       edgeXLarge2, derEdge2, uvIncr,
                                       &vLast2);

            // We create triangle stripes until we hit the end of at least
            // one the edges
#ifdef DEBUG1
            ErrorF("TEX PERSP PART(3):Start first edge splitting");
#endif
            while (bvNew1 && bvNew2) {

                // Now we have to partition the trapezoid formed by vLast1,
                // vLast2, vNew1 and vNew2 into triangles
                UVSplitTrapezoid(gc, &tessInfo, &vNew1, &vNew2, &vLast1, &vLast2,
                                      uvMaxLen, uvIncr, uvIncrInv);

                // update working vertexes
                vLast1 = vNew1;
                vLast2 = vNew2;

                // Calculate new vertexes!
                bvNew1 = SplitUVEdge(gc, &vNew1, &vLast1,
                                           v[vOppositeSmall], v[vSmallSideI],
                                           edgeXLarge1, derEdge1, uvIncr,
                                           &vLast1);
                bvNew2 = SplitUVEdge(gc, &vNew2, &vLast2,
                                           v[vOppositeSmall], v[vSmallSideF],
                                           edgeXLarge2, derEdge2, uvIncr,
                                           &vLast2);
            }

            // Now we must see if we are done or if we need to switch
            // to our last edge

            if ((!bvNew1) && (!bvNew2))
            {
                if ((modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) ||
                    !(modeFlags & __GL_SHADE_TEXTURE_UVSCALED))
                {
                    a->texture.x = aTexOld.x;
                    a->texture.y = aTexOld.y;
                    b->texture.x = bTexOld.x;
                    b->texture.y = bTexOld.y;
                    c->texture.x = cTexOld.x;
                    c->texture.y = cTexOld.y;
                }
                return; //we are done
            }

            vNewApex1 = vNewApex2 = *v[vOppositeSmall];

            // Update the finished edge (1) with the last remaining edge
            if (!bvNew1) {

                vLast1 = vNewApex1 = *v[vSmallSideI];
                vSmallSideI = vSmallSideF; //Don't miss this

                // The base vertex of each edge is going
                // to be equal to the new vertex
                edgeXLarge1 =  ( __GL_ABSF(vNewApex1.texture.x -
                                       v[vSmallSideI]->texture.x) >
                                  __GL_ABSF(vNewApex1.texture.y -
                                       v[vSmallSideI]->texture.y));

                derEdge1 = UVSlope(&vNewApex1, v[vSmallSideF], edgeXLarge1);

                // Calculate new vertex!
                bvNew1 = SplitUVEdge(gc, &vNew1, &vLast1,
                                           &vNewApex1, v[vSmallSideI],
                                           edgeXLarge1, derEdge1, uvIncr,
                                           &vLast1);
            }

            // Update the finished edge (2) with the last remaining edge
            if (!bvNew2) {
                vLast2 = vNewApex2 = *v[vSmallSideF];
                vSmallSideF = vSmallSideI; // Don't miss this

                // The base vertex of each edge is going
                // to be equal to the new vertex
                edgeXLarge2 =  ( __GL_ABSF(vNewApex2.texture.x -
                                       v[vSmallSideF]->texture.x) >
                                  __GL_ABSF(vNewApex2.texture.y -
                                       v[vSmallSideF]->texture.y));


                derEdge2 = UVSlope(&vNewApex2, v[vSmallSideI],
                                        edgeXLarge2);

                // Calculate new vertex!
                bvNew2 = SplitUVEdge(gc, &vNew2, &vLast2,
                                           &vNewApex2, v[vSmallSideF],
                                           edgeXLarge2, derEdge2, uvIncr,
                                           &vLast2);
            }

            // We create the second set of triangle stripes until we hit the
            // end of at least one the edges (which really is going to be
            // the end of both!)
#ifdef DEBUG1
            ErrorF("TEX PERSP PART(3):Starting second "
                                 "edge splitting");
#endif

            while ( bvNew1 || bvNew2) {

                // Now we have to partition the trapezoid formed by vLast1,
                // vLast2, vNew1 and vNew2 into triangles
                UVSplitTrapezoid(gc, &tessInfo, &vNew1, &vNew2,
                                      &vLast1, &vLast2,
                                      uvMaxLen, uvIncr, uvIncrInv);

                // Calculate new vertexes!
                if (bvNew1) {
                    vLast1 = vNew1;
                    bvNew1 = SplitUVEdge(gc, &vNew1, &vLast1,
                                               &vNewApex1, v[vSmallSideI],
                                               edgeXLarge1, derEdge1, uvIncr,
                                               &vLast1);
                }

                if (bvNew2) {
                    vLast2 = vNew2;
                    bvNew2 = SplitUVEdge(gc, &vNew2, &vLast2,
                                               &vNewApex2, v[vSmallSideF],
                                               edgeXLarge2, derEdge2, uvIncr,
                                               &vLast2);
                }
            }

            break;
        default:
#ifdef DEBUG1
            ErrorF(
            "TEX PERSP PART:Cannot have more than 3 sides on a triangle for UV space partitioning!");
#endif
            break;
    }

    if ((modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) ||
        !(modeFlags & __GL_SHADE_TEXTURE_UVSCALED))
    {
        a->texture.x = aTexOld.x;
        a->texture.y = aTexOld.y;
        b->texture.x = bTexOld.x;
        b->texture.y = bTexOld.y;
        c->texture.x = cTexOld.x;
        c->texture.y = cTexOld.y;
    }
}



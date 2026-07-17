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
#include "s3virge.h"


#define __DEBUG_PRINT
#include "dbg.h"


#define VERY_SMALL              0.00000190771250009538
#define NEGATIVE_VERY_SMALL     0.00000190771250009538


/*
 * Initially, no mip maps, and we assume that everybody is 
 * texture op MODULATE, and that the texture is AFFINE
 * 
 */

void  
__glS3VTexFlatDepthTriangle(__GLcontext *gc, __GLvertex *v0,
			__GLvertex *v1, __GLvertex *v2)
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

    dx0 = v1->window.x - v0->window.x;
    dy0 = v1->window.y - v0->window.y;

    dx1 = v2->window.x - v0->window.x;
    dy1 = v2->window.y - v0->window.y;

    dx2 = v1->window.x - v2->window.x;
    dy2 = v1->window.y - v2->window.y;
    xsnap1 = (float) ((int)(v1->window.x) + 1) - v1->window.x;

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

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;
    dx2 *= invc;
    dy2 *= invc;

    // BEGIN MUTEX

    triEngine = hwcx->triEngine;

    { /* For Texture */
	__GLtexture *current = gc->texture.currentTexture;
	__GLmipMapLevel *lp = current->level[0];

	tr.tp            = (GLubyte *) lp->buffer;
	tr.texWidth      = lp->width;
	tr.texWidthLog2  = lp->widthLog2;
	tr.texHeight     = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

        if (modeFlags & __GL_SHADE_TEXTURE_MIPMAP) {
 
               /* We need rho */
        }

	p0  = v1->texture.x;
	dp0 = v0->texture.x - p0;
	dp1 = v2->texture.x - p0;
 
        /* For now, scale the tcoords in the triangle. We should be able to wrap 
           this into the texture transform later */

        p0  *= tr.texWidth;    /* width MUST equal height, and must be a power of 2 */
        dp0 *= tr.texWidth; 
        dp1 *= tr.texWidth; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(6);
	triEngine->UStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 20);
	triEngine->dUX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 20);
	triEngine->dUY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 20);

	p0 = v1->texture.y;
	dp0 = v0->texture.y - p0;
	dp1 = v2->texture.y - p0;
        p0  *= tr.texHeight;
        dp0 *= tr.texHeight; 
        dp1 *= tr.texHeight; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

	triEngine->VStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 20);
	triEngine->dVX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 20);
	triEngine->dVY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 20);
    }

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

    {
        __GLcolor *flatColor = gc->vertex.provoking->color;
        __GLcolorBuffer *cfb = gc->drawBuffer;

        tr.r = FloatToFixed(flatColor->r + FR_COLOR_ROUND, COLOR_FRAC_BITS);
        tr.g = FloatToFixed(flatColor->g + FR_COLOR_ROUND, COLOR_FRAC_BITS);
        tr.b = FloatToFixed(flatColor->b + FR_COLOR_ROUND, COLOR_FRAC_BITS);
        tr.a = FloatToFixed(flatColor->a + FR_COLOR_ROUND, COLOR_FRAC_BITS);

        WaitForQueue(6);
        triEngine->ARStart =
            0x7fff0000 | (tr.r >> 6);
        triEngine->BGStart =
            (tr.b >> 6) |
            ((tr.g >> 6) << 16);

    }

    { /* For Depth Test */

	p0 = v1->window.z;
	dp0 = v0->window.z - p0;
	dp1 = v2->window.z - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);
	tr.z0 =
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 0) >> 1;
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


void  
__glS3VTexFlatTriangle(__GLcontext *gc, __GLvertex *v0,
		       __GLvertex *v1, __GLvertex *v2)
{
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    volatile __glS3VTriEngineRegisters *triEngine;
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap1, ysnap2;
    int ccw, reversed = 0;
    float xsnap1;
    float p0, dp0, dp1, dpdx, dpdy;
    float ftmp;
    double dtmp;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    float FR_COLOR_ROUND = COLOR_ROUND_DITHER;
    __GLtri tr;		/* Must be last auto for assembler access */

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

    dx0 = v1->window.x - v0->window.x;
    dy0 = v1->window.y - v0->window.y;

    dx1 = v2->window.x - v0->window.x;
    dy1 = v2->window.y - v0->window.y;

    dx2 = v1->window.x - v2->window.x;
    dy2 = v1->window.y - v2->window.y;
    xsnap1 = (float) ((int)(v1->window.x) + 1) - v1->window.x;

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

    { /* For Texture */
	__GLtexture *current = gc->texture.currentTexture;
	__GLmipMapLevel *lp = current->level[0];

	tr.tp            = (GLubyte *) lp->buffer;
	tr.texWidth      = lp->width;
	tr.texWidthLog2  = lp->widthLog2;
	tr.texHeight     = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

        if (modeFlags & __GL_SHADE_TEXTURE_MIPMAP) {
 
               /* We need rho */
        }

	p0  = v1->texture.x;
	dp0 = v0->texture.x - p0;
	dp1 = v2->texture.x - p0;
 
        /* For now, scale the tcoords in the triangle. We should be able to wrap 
           this into the texture transform later */

        p0  *= tr.texWidth;    /* width MUST equal height, and must be a power of 2 */
        dp0 *= tr.texWidth; 
        dp1 *= tr.texWidth; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(6);
	triEngine->UStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 20);
	triEngine->dUX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 20);
	triEngine->dUY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 20);

	p0 = v1->texture.y;
	dp0 = v0->texture.y - p0;
	dp1 = v2->texture.y - p0;
        p0  *= tr.texHeight;
        dp0 *= tr.texHeight; 
        dp1 *= tr.texHeight; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

	triEngine->VStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 20);
	triEngine->dVX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 20);
	triEngine->dVY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 20);
    }

    WaitForQueue(10);
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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;
	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + FR_COLOR_ROUND, COLOR_FRAC_BITS);
	tr.g = FloatToFixed(flatColor->g + FR_COLOR_ROUND, COLOR_FRAC_BITS);
	tr.b = FloatToFixed(flatColor->b + FR_COLOR_ROUND, COLOR_FRAC_BITS);
	tr.a = FloatToFixed(flatColor->a + FR_COLOR_ROUND, COLOR_FRAC_BITS);

	triEngine->ARStart = 
	    0x7fff0000 | (tr.r >> 6);
	triEngine->BGStart = 
	    (tr.b >> 6) |
	    ((tr.g >> 6) << 16);
    }

    triEngine->Heights =
	((tr.dx > 0) ? 0x80000000 : 0x00000000) | 
	(tr.y2Int - tr.y0Int) |
	((tr.y1Int - tr.y2Int) << 16);

    /* set up the command register with the value calculated at pick time */

    triEngine->cmd = hwcx->hwCmdMask;

    // END MUTEX
}


void  
__glS3VTexSmoothDepthTriangle(__GLcontext *gc, __GLvertex *v0,
			      __GLvertex *v1, __GLvertex *v2)
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

    dx0 = v1->window.x - v0->window.x;
    dy0 = v1->window.y - v0->window.y;

    dx1 = v2->window.x - v0->window.x;
    dy1 = v2->window.y - v0->window.y;

    dx2 = v1->window.x - v2->window.x;
    dy2 = v1->window.y - v2->window.y;
    xsnap1 = (float) ((int)(v1->window.x) + 1) - v1->window.x;

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

	tr.tp            = (GLubyte *) lp->buffer;
	tr.texWidth      = lp->width;
	tr.texWidthLog2  = lp->widthLog2;
	tr.texHeight     = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

        if (modeFlags & __GL_SHADE_TEXTURE_MIPMAP) {
 
               /* We need rho */
        }

	p0  = v1->texture.x;
	dp0 = v0->texture.x - p0;
	dp1 = v2->texture.x - p0;
 
        /* For now, scale the tcoords in the triangle. We should be able to wrap 
           this into the texture transform later */

        p0  *= tr.texWidth;    /* width MUST equal height, and must be a power of 2 */
        dp0 *= tr.texWidth; 
        dp1 *= tr.texWidth; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(6);
	triEngine->UStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 20);
	triEngine->dUX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 20);
	triEngine->dUY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 20);

	p0 = v1->texture.y;
	dp0 = v0->texture.y - p0;
	dp1 = v2->texture.y - p0;
        p0  *= tr.texHeight;
        dp0 *= tr.texHeight; 
        dp1 *= tr.texHeight; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

	triEngine->VStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 20);
	triEngine->dVX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 20);
	triEngine->dVY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 20);
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
        WaitForQueue(3);
	triEngine->ARStart = 0x7fff0000 | (tr.r0 >> 6);
	triEngine->dARY = (tr.drdy0[0] >> 6) & 0xffff;
	triEngine->dARX = (tr.drdx >> 6) & 0xffff;

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

        WaitForQueue(3);
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


void  
__glS3VTextureSmoothTriangle(__GLcontext *gc, __GLvertex *v0,
			     __GLvertex *v1, __GLvertex *v2)
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

    dx0 = v1->window.x - v0->window.x;
    dy0 = v1->window.y - v0->window.y;

    dx1 = v2->window.x - v0->window.x;
    dy1 = v2->window.y - v0->window.y;

    dx2 = v1->window.x - v2->window.x;
    dy2 = v1->window.y - v2->window.y;
    xsnap1 = (float) ((int)(v1->window.x) + 1) - v1->window.x;

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

	tr.tp            = (GLubyte *) lp->buffer;
	tr.texWidth      = lp->width;
	tr.texWidthLog2  = lp->widthLog2;
	tr.texHeight     = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

        if (modeFlags & __GL_SHADE_TEXTURE_MIPMAP) {
 
               /* We need rho */
        }

	p0  = v1->texture.x;
	dp0 = v0->texture.x - p0;
	dp1 = v2->texture.x - p0;
 
        /* For now, scale the tcoords in the triangle. We should be able to wrap 
           this into the texture transform later */

        p0  *= tr.texWidth;    /* width MUST equal height, and must be a power of 2 */
        dp0 *= tr.texWidth; 
        dp1 *= tr.texWidth; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(6);
	triEngine->UStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 20);
	triEngine->dUX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 20);
	triEngine->dUY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 20);

	p0 = v1->texture.y;
	dp0 = v0->texture.y - p0;
	dp1 = v2->texture.y - p0;
        p0  *= tr.texHeight;
        dp0 *= tr.texHeight; 
        dp1 *= tr.texHeight; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

	triEngine->VStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 20);
	triEngine->dVX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 20);
	triEngine->dVY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 20);
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
        WaitForQueue(3);
	triEngine->ARStart = 0x7fff0000 | (tr.r0 >> 6);
	triEngine->dARY = (tr.drdy0[0] >> 6) & 0xffff;
	triEngine->dARX = (tr.drdx >> 6) & 0xffff;

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

        WaitForQueue(4);
	triEngine->BGStart = (((tr.g0 >> 6) & 0xffff) << 16) | (tr.b0 >> 6);
	triEngine->dBGY = (((tr.dgdy0[0] >> 6) & 0xffff) << 16) | (tr.dbdy0[0] >> 6) & 0xffff;
	triEngine->dBGX = (((tr.dgdx >> 6) & 0xffff) << 16) | (tr.dbdx >> 6) & 0xffff;
    }

    triEngine->Heights =
	((tr.dx > 0) ? 0x80000000 : 0x00000000) | 
	(tr.y2Int - tr.y0Int) |
	((tr.y1Int - tr.y2Int) << 16);

    /* set up the command register with the value calculated at pick time */

    triEngine->cmd = hwcx->hwCmdMask;

    // END MUTEX
}


/*----------------------------------------------
 *
 * Perspective Triangles 
 *
 -----------------------------------------------*/

void  
__glS3VTexPerspectiveFlatDepthTriangle(__GLcontext *gc, __GLvertex *v0,
				       __GLvertex *v1, __GLvertex *v2)
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

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;
    dx2 *= invc;
    dy2 *= invc;

    // BEGIN MUTEX

    triEngine = hwcx->triEngine;

    { /* For Texture */
	__GLtexture *current = gc->texture.currentTexture;
	__GLmipMapLevel *lp = current->level[0];

	tr.tp            = (GLubyte *) lp->buffer;
	tr.texWidth      = lp->width;
	tr.texWidthLog2  = lp->widthLog2;
	tr.texHeight     = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

        if (modeFlags & __GL_SHADE_TEXTURE_MIPMAP) {
 
               /* We need rho */
        }

	p0  = v1->texture.x;
	dp0 = v0->texture.x - p0;
	dp1 = v2->texture.x - p0;
 
        /* For now, scale the tcoords in the triangle. We should be able to wrap 
           this into the texture transform later */

        p0  *= tr.texWidth;    /* width MUST equal height, and must be a power of 2 */
        dp0 *= tr.texWidth; 
        dp1 *= tr.texWidth; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(6);
	triEngine->UStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 12);
	triEngine->dUX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dUY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);

	p0 = v1->texture.y;
	dp0 = v0->texture.y - p0;
	dp1 = v2->texture.y - p0;
        p0  *= tr.texHeight;
        dp0 *= tr.texHeight; 
        dp1 *= tr.texHeight; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

	triEngine->VStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 12);
	triEngine->dVX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dVY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);

	p0 = v1->texture.w;
	dp0 = v0->texture.w - p0;
	dp1 = v2->texture.w - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

	triEngine->WStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 19);
	triEngine->dWX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 19);
	triEngine->dWY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 19);
    }

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


    {
        __GLcolor *flatColor = gc->vertex.provoking->color;
        __GLcolorBuffer *cfb = gc->drawBuffer;

        tr.r = FloatToFixed(flatColor->r + FR_COLOR_ROUND, COLOR_FRAC_BITS);
        tr.g = FloatToFixed(flatColor->g + FR_COLOR_ROUND, COLOR_FRAC_BITS);
        tr.b = FloatToFixed(flatColor->b + FR_COLOR_ROUND, COLOR_FRAC_BITS);
        tr.a = FloatToFixed(flatColor->a + FR_COLOR_ROUND, COLOR_FRAC_BITS);

        WaitForQueue(6);
        triEngine->ARStart =
            0x7fff0000 | (tr.r >> 6);
        triEngine->BGStart =
            (tr.b >> 6) |
            ((tr.g >> 6) << 16);

    }

    { /* For Depth Test */

	p0 = v1->window.z;
	dp0 = v0->window.z - p0;
	dp1 = v2->window.z - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);
	tr.z0 =
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 0) >> 1;
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


void  
__glS3VTexPerspectiveFlatTriangle(__GLcontext *gc, __GLvertex *v0,
				  __GLvertex *v1, __GLvertex *v2)
{
    __GLS3Vcontext *hwcx = (__GLS3Vcontext *)gc;
    volatile __glS3VTriEngineRegisters *triEngine;
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap1, ysnap2;
    int ccw, reversed = 0;
    float xsnap1;
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

    { /* For Texture */
	__GLtexture *current = gc->texture.currentTexture;
	__GLmipMapLevel *lp = current->level[0];

	tr.tp            = (GLubyte *) lp->buffer;
	tr.texWidth      = lp->width;
	tr.texWidthLog2  = lp->widthLog2;
	tr.texHeight     = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

        if (modeFlags & __GL_SHADE_TEXTURE_MIPMAP) {
 
               /* We need rho */
        }

	p0  = v1->texture.x;
	dp0 = v0->texture.x - p0;
	dp1 = v2->texture.x - p0;
 
        /* For now, scale the tcoords in the triangle. We should be able to wrap 
           this into the texture transform later */

        p0  *= tr.texWidth;    /* width MUST equal height, and must be a power of 2 */
        dp0 *= tr.texWidth; 
        dp1 *= tr.texWidth; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(6);
	triEngine->UStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 12);
	triEngine->dUX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dUY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);

	p0 = v1->texture.y;
	dp0 = v0->texture.y - p0;
	dp1 = v2->texture.y - p0;
        p0  *= tr.texHeight;
        dp0 *= tr.texHeight; 
        dp1 *= tr.texHeight; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

	triEngine->VStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 12);
	triEngine->dVX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dVY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);

	p0 = v1->texture.w;
	dp0 = v0->texture.w - p0;
	dp1 = v2->texture.w - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

	triEngine->WStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 12);
	triEngine->dWX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dWY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;
	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + FR_COLOR_ROUND, COLOR_FRAC_BITS);
	tr.g = FloatToFixed(flatColor->g + FR_COLOR_ROUND, COLOR_FRAC_BITS);
	tr.b = FloatToFixed(flatColor->b + FR_COLOR_ROUND, COLOR_FRAC_BITS);
	tr.a = FloatToFixed(flatColor->a + FR_COLOR_ROUND, COLOR_FRAC_BITS);

        WaitForQueue(3);
	triEngine->ARStart = 
	    0x7fff0000 | (tr.r >> 6);
	triEngine->BGStart = 
	    (tr.b >> 6) |
	    ((tr.g >> 6) << 16);
    }

    triEngine->Heights =
	((tr.dx > 0) ? 0x80000000 : 0x00000000) | 
	(tr.y2Int - tr.y0Int) |
	((tr.y1Int - tr.y2Int) << 16);

    /* set up the command register with the value calculated at pick time */

    triEngine->cmd = hwcx->hwCmdMask;

    // END MUTEX
}


void  
__glS3VTexPerspectiveSmoothDepthTriangle(__GLcontext *gc, __GLvertex *v0,
					 __GLvertex *v1, __GLvertex *v2)
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

	tr.tp            = (GLubyte *) lp->buffer;
	tr.texWidth      = lp->width;
	tr.texWidthLog2  = lp->widthLog2;
	tr.texHeight     = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

        if (modeFlags & __GL_SHADE_TEXTURE_MIPMAP) {
 
               /* We need rho registers */
        }

	p0  = v1->texture.x;
	dp0 = v0->texture.x - p0;
	dp1 = v2->texture.x - p0;

        /* For now, scale the tcoords in the triangle. We should be able to wrap 
           this into the texture transform later */

        p0  *= tr.texWidth;    /* width MUST equal height, and must be a power of 2 */
        dp0 *= tr.texWidth; 
        dp1 *= tr.texWidth; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(3);
	triEngine->UStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, /* 12 */ 12);
	triEngine->dUX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dUY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);

	p0  = v1->texture.y;
	dp0 = v0->texture.y - p0;
	dp1 = v2->texture.y - p0;
        p0  *= tr.texHeight;
        dp0 *= tr.texHeight; 
        dp1 *= tr.texHeight; 

	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(3);
	triEngine->VStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 12);
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


void  
__glS3VTexPerspectiveSmoothTriangle(__GLcontext *gc, __GLvertex *v0,
				    __GLvertex *v1, __GLvertex *v2)
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

    WaitForQueue(15);

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

	tr.tp            = (GLubyte *) lp->buffer;
	tr.texWidth      = lp->width;
	tr.texWidthLog2  = lp->widthLog2;
	tr.texHeight     = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

        if (modeFlags & __GL_SHADE_TEXTURE_MIPMAP) {
 
               /* We need rho */
        }

	p0  = v1->texture.x;
	dp0 = v0->texture.x - p0;
	dp1 = v2->texture.x - p0;
 
        /* For now, scale the tcoords in the triangle. We should be able to wrap 
           this into the texture transform later */

        p0  *= tr.texWidth;    /* width MUST equal height, and must be a power of 2 */
        dp0 *= tr.texWidth; 
        dp1 *= tr.texWidth; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

        WaitForQueue(6);
	triEngine->UStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 12);
	triEngine->dUX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dUY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);

	p0 = v1->texture.y;
	dp0 = v0->texture.y - p0;
	dp1 = v2->texture.y - p0;
        p0  *= tr.texWidth;
        dp0 *= tr.texWidth; 
        dp1 *= tr.texWidth; 
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

	triEngine->VStart = 
	    FloatToFixed(p0 + dpdy * ysnap1 + dpdx * xsnap1, 12);
	triEngine->dVX = FloatToFixed(((tr.dx < 0) ? -dpdx : dpdx), 12);
	triEngine->dVY = FloatToFixed(dpdy + (dxdy0 * -dpdx), 12);

	p0 = v1->texture.w;
	dp0 = v0->texture.w - p0;
	dp1 = v2->texture.w - p0;
	dpdx = (dy2*dp0 - dy0*dp1);
	dpdy = (dp0*dx2 - dp1*dx0);

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

	triEngine->ARStart = 0x7fff0000 | (tr.r0 >> 6);
	triEngine->dARY = (tr.drdy0[0] >> 6) & 0xffff;
	triEngine->dARX = (tr.drdx >> 6) & 0xffff;

	triEngine->BGStart = (((tr.g0 >> 6) & 0xffff) << 16) | (tr.b0 >> 6);
	triEngine->dBGY = (((tr.dgdy0[0] >> 6) & 0xffff) << 16) | (tr.dbdy0[0] >> 6) & 0xffff;
	triEngine->dBGX = (((tr.dgdx >> 6) & 0xffff) << 16) | (tr.dbdx >> 6) & 0xffff;
    }

    triEngine->Heights =
	((tr.dx > 0) ? 0x80000000 : 0x00000000) | 
	(tr.y2Int - tr.y0Int) |
	((tr.y1Int - tr.y2Int) << 16);

    /* set up the command register with the value calculated at pick time */

    triEngine->cmd = hwcx->hwCmdMask;

    // END MUTEX
}

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

#ifdef __GL_PC_RAST
#include "context.h"
#include "global.h"
#include "xform.h"
#include "string.h"
#include "fr_modes.h"
#include "fr_tri.h"
#include "imports.h" /* for __GL_ABSF */

/* The conditionals which use FR_RGBA_COLOR are all hard-coded to TRUE
 * for simplicity, but are left in place for purposes of documenting
 * what can be removed as an optimization when alpha is not required.
 */
#define FR_RGBA_COLOR GL_TRUE

/* For now, we force perspective correction if needRho */
#define AFFINE_RHO 0

/* XXX HACK!!!
 * The following function was changed to work around a bug in the
 * assembly coded setup.  Until it is fixed, we must emulate the
 * bug so that the right thing will happen in the C version.
 */
#define RHO_BUG 1

extern
void __glFRSetupRho(__GLcontext *gc, __GLtri *tr,
		    __GLvertex *v0, __GLvertex *v1, __GLvertex *v2,
		    float dx0, float dy0, float dx1, float dy1, float dxdy0,
		    float xsnap0, float ysnap0);

void  __glFRRenderTriangle(__GLcontext *gc, __GLvertex *v0,
			   __GLvertex *v1, __GLvertex *v2)
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;
    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;
    float ftmp;
    double dtmp;
    GLint texNeedRho;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLboolean doubleStore = gc->buffers.doubleStore;
    float FR_COLOR_ROUND = (modeFlags & __GL_SHADE_DITHER) ?
			    COLOR_ROUND_DITHER : COLOR_ROUND_NO_DITHER;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;
#if 0
    __GLcolor tmpColor;
#endif
    __GLtri tr;		/* Must be last auto for assembler access */

    __GL_ADJUST_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    *((int *)&v0->window.x) &= XY_QUANTIZE_MASK;
    *((int *)&v0->window.y) &= XY_QUANTIZE_MASK;
    *((int *)&v1->window.x) &= XY_QUANTIZE_MASK;
    *((int *)&v1->window.y) &= XY_QUANTIZE_MASK;
    *((int *)&v2->window.x) &= XY_QUANTIZE_MASK;
    *((int *)&v2->window.y) &= XY_QUANTIZE_MASK;

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

    c = dx1*dy0 - dx0*dy1;

    if ((*(int *)&c << 1) == 0) {
	__GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);
	return;
    }

    ccw = (unsigned int) FloatBits(c) >> 31;
    tr.dx = 1 - (ccw << 1);

#if 0
    {
	GLuint needs, faceNeeds;
	GLint face, colorFace;

	face = gc->polygon.face[ccw ^ reversed];
	if (face == gc->polygon.cullFace) {
	    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);
	    return;
	}

	if (modeFlags & __GL_SHADE_TWOSIDED) {
	    colorFace = face;
	    faceNeeds = gc->vertex.faceNeeds[face];
	} else {
	    colorFace = __GL_FRONTFACE;
	    faceNeeds = gc->vertex.faceNeeds[__GL_FRONTFACE];
	}

	needs = gc->vertex.needs | faceNeeds;
	if (modeFlags & __GL_SHADE_SMOOTH)
	{
	    v0->color = &v0->colors[colorFace];
	    v1->color = &v1->colors[colorFace];
	    v2->color = &v2->colors[colorFace];

	    if (~v0->hasAndClipCode & needs) DO_VALIDATE(gc, v0, needs);
	    if (~v1->hasAndClipCode & needs) DO_VALIDATE(gc, v1, needs);
	    if (~v2->hasAndClipCode & needs) DO_VALIDATE(gc, v2, needs);

	    if (!(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
		__GLvertex *pv = gc->vertex.provoking;

		pv->color = &pv->colors[colorFace];
		if (~pv->hasAndClipCode & needs) DO_VALIDATE(gc, pv, needs);

		if (modeFlags & __GL_SHADE_CHEAP_FOG) {
		    __GLcolor *pvColor = pv->color;

		    pv->color = &tmpColor;
		    (*gc->procs.fogColor)(gc, v0->color, pvColor, v0->fog);
		    (*gc->procs.fogColor)(gc, v1->color, pvColor, v1->fog);
		    (*gc->procs.fogColor)(gc, v2->color, pvColor, v2->fog);
		}

		if (pv != v0 && pv != v1 && pv != v2) {
		    pv->color = &pv->colors[__GL_FRONTFACE];
		}
	    }
	}
	else
	{
	    __GLvertex *pv = gc->vertex.provoking;

	    pv->color = &pv->colors[colorFace];
	    if (~pv->hasAndClipCode & needs) DO_VALIDATE(gc, pv, needs);
	    if (~v0->hasAndClipCode & needs) DO_VALIDATE(gc, v0, needs);
	    if (~v1->hasAndClipCode & needs) DO_VALIDATE(gc, v1, needs);
	    if (~v2->hasAndClipCode & needs) DO_VALIDATE(gc, v2, needs);
	    v0->color = v1->color = v2->color = pv->color;
	}
    }
#endif

    dxdy0 = dy0 ? dx0 / dy0 : 0.0F;
    dxdy1 = dy1 ? dx1 / dy1 : 0.0F;
    dxdy2 = dy2 ? dx2 / dy2 : 0.0F;

    tr.y0Int = BiasedFloatToInt(v0->window.y, XY_FRAC_BITS);
    tr.y1Int = BiasedFloatToInt(v1->window.y, XY_FRAC_BITS);
    tr.y2Int = BiasedFloatToInt(v2->window.y, XY_FRAC_BITS);

    ysnap0 = (float) (tr.y0Int + XY_BIAS + 1) - v0->window.y;
    ysnap2 = (float) (tr.y2Int + XY_BIAS + 1) - v2->window.y;

    x = v0->window.x + dxdy0 * ysnap0;
    tr.x0Int = BiasedFloatToInt(x, XY_FRAC_BITS);
    tr.x0Frac = FloatToFrac(x - ((float) tr.x0Int + XY_BIAS));
    tr.dxdy0Int[0] = FloatToInt(dxdy0, XY_FRAC_BITS);
    tr.dxdy0Int[1] = tr.dxdy0Int[0] + (dxdy0 < 0.0F ? -1 : 1);
    tr.dxdy0Frac = FloatToFrac(dxdy0 - (float) tr.dxdy0Int[0]);

    x = v0->window.x + dxdy1 * ysnap0;
    tr.x1Int = BiasedFloatToInt(x, XY_FRAC_BITS);
    tr.x1Frac = FloatToFrac(x - ((float) tr.x1Int + XY_BIAS));
    tr.dxdy1Int[0] = FloatToInt(dxdy1, XY_FRAC_BITS);
    tr.dxdy1Int[1] = tr.dxdy1Int[0] + (dxdy1 < 0.0F ? -1 : 1);
    tr.dxdy1Frac = FloatToFrac(dxdy1 - (float) tr.dxdy1Int[0]);

    x = v2->window.x + dxdy2 * ysnap2;
    tr.x2Int = BiasedFloatToInt(x, XY_FRAC_BITS);
    tr.x2Frac = FloatToFrac(x - ((float) tr.x2Int + XY_BIAS));
    tr.dxdy2Int[0] = FloatToInt(dxdy2, XY_FRAC_BITS);
    tr.dxdy2Int[1] = tr.dxdy2Int[0] + (dxdy2 < 0.0F ? -1 : 1);
    tr.dxdy2Frac = FloatToFrac(dxdy2 - (float) tr.dxdy2Int[0]);

    if (tr.dx < 0) {
	tr.x0Int -= 1; tr.x1Int -= 1; tr.x2Int -= 1;
    }

    __GL_LOCK_RENDER_BUFFERS(gc);

    {
	/* We used to shift by elementSizeLog2 instead of multiplying
	 * by elementSize, but this doesn't work in 24-bit buffers.
	 */
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLuint elementSize = cfb->buf.elementSize;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int * elementSize);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] * elementSize);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] * elementSize);
    }

    if (doubleStore)
    {
	__GLcolorBuffer *cfb = gc->front;
	GLint byteWidth = cfb->buf.byteWidth;
	GLuint elementSize = cfb->buf.elementSize;

	/* We assume that the "primary" drawBuffer is the back buffer,
	 * and here we set up the front buffer.  We also assume that
	 * the front and back buffers are the same pixel format.
	 */
	assert(gc->drawBuffer == gc->back);
	assert(elementSize == gc->drawBuffer->buf.elementSize);

	tr.cp20 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int * elementSize);
	tr.dcp2dx = cfb->buf.elementSize;
	tr.dcp2dy0[0] = byteWidth + (tr.dxdy0Int[0] * elementSize);
	tr.dcp2dy0[1] = byteWidth + (tr.dxdy0Int[1] * elementSize);
    }

    if (modeFlags & __GL_SHADE_STENCIL_TEST)
    {
	__GLstencilBuffer *sfb = &gc->stencilBuffer;
	GLint byteWidth = sfb->buf.byteWidth;
	GLint elementSizeLog2 = sfb->buf.elementSizeLog2;

	tr.sp0 = ((GLubyte *) sfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dspdx = sfb->buf.elementSize;
	tr.dspdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dspdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    if (modeFlags & __GL_SHADE_DEPTH_TEST)
    {
	__GLdepthBuffer *dfb = &gc->depthBuffer;
	GLint byteWidth = dfb->buf.byteWidth;
	GLint elementSizeLog2 = dfb->buf.elementSizeLog2;

	tr.zp0 = ((GLubyte *) dfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dzpdx = dfb->buf.elementSize;
	tr.dzpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dzpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    if (0 == (modeFlags & __GL_SHADE_SMOOTH))
    {
	__GLcolor *flatColor = gc->vertex.provoking->color;
	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + FR_COLOR_ROUND, COLOR_FRAC_BITS);
	if (modeFlags & __GL_SHADE_RGB) {
	    tr.g = FloatToFixed(flatColor->g + FR_COLOR_ROUND, COLOR_FRAC_BITS);
	    tr.b = FloatToFixed(flatColor->b + FR_COLOR_ROUND, COLOR_FRAC_BITS);
#if FR_RGBA_COLOR
	    tr.a = FloatToFixed(flatColor->a + FR_COLOR_ROUND, COLOR_FRAC_BITS);
#endif
	}

	if (0 == (modeFlags & __GL_SHADE_DITHER)) {
	    if (0 == (modeFlags & __GL_SHADE_RGB)) {
		tr.r0 = CoordToInt(tr.r);
	    }
	    else {
		tr.r0 =
#if FR_RGBA_COLOR
		(CoordToInt(tr.a) << cfb->alphaShift) |
#endif
		(CoordToInt(tr.r) << cfb->redShift)   |
		    (CoordToInt(tr.g) << cfb->greenShift) |
		    (CoordToInt(tr.b) << cfb->blueShift);
	    }
	}
    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
	p0 = v0->window.z;
	dp0 = v1->window.z - p0;
	dp1 = v2->window.z - p0;
	dpdx = dy0*dp1 - dy1*dp0;
	dpdy = dp0*dx1 - dp1*dx0;

	if (modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
	    /* find the maximum Z slope with respect to X and Y */
	    float dzdx = __GL_ABSF(dpdx);
	    float dzdy = __GL_ABSF(dpdy);
	    float maxdZ = (dzdx > dzdy) ? dzdx : dzdy;
	    float offset =
		gc->state.polygon.factor * maxdZ +
		gc->state.polygon.units * gc->depthBuffer.minResolution;

	    tr.depthOffset = FloatToFixed(offset, Z_FRAC_BITS);
	}

	tr.z0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + 0.5F,
			     Z_FRAC_BITS);
	tr.dzdx = FloatToFixed(dpdx, Z_FRAC_BITS);
	tr.dzdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], Z_FRAC_BITS);
	tr.dzdy0[1] = tr.dzdy0[0] + (dxdy0 < 0.0F ? -tr.dzdx : tr.dzdx);
    }

    if (modeFlags & __GL_SHADE_SMOOTH) {

	p0 = v0->color->r;
	dp0 = v1->color->r - p0;
	dp1 = v2->color->r - p0;
	dpdx = dy0*dp1 - dy1*dp0;
	dpdy = dp0*dx1 - dp1*dx0;
	tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 +
			     FR_COLOR_ROUND, COLOR_FRAC_BITS);
	tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
	tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0],
				   COLOR_FRAC_BITS);
	tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

	if (modeFlags & __GL_SHADE_RGB) {

	    p0 = v0->color->g;
	    dp0 = v1->color->g - p0;
	    dp1 = v2->color->g - p0;
	    dpdx = dy0*dp1 - dy1*dp0;
	    dpdy = dp0*dx1 - dp1*dx0;
	    tr.g0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 +
				 FR_COLOR_ROUND, COLOR_FRAC_BITS);
	    tr.dgdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
	    tr.dgdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0],
				       COLOR_FRAC_BITS);
	    tr.dgdy0[1] = tr.dgdy0[0] + (dxdy0 < 0.0F ? -tr.dgdx : tr.dgdx);

	    p0 = v0->color->b;
	    dp0 = v1->color->b - p0;
	    dp1 = v2->color->b - p0;
	    dpdx = dy0*dp1 - dy1*dp0;
	    dpdy = dp0*dx1 - dp1*dx0;
	    tr.b0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 +
				 FR_COLOR_ROUND, COLOR_FRAC_BITS);
	    tr.dbdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
	    tr.dbdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0],
				       COLOR_FRAC_BITS);
	    tr.dbdy0[1] = tr.dbdy0[0] + (dxdy0 < 0.0F ? -tr.dbdx : tr.dbdx);

#if FR_RGBA_COLOR
	    p0 = v0->color->a;
	    dp0 = v1->color->a - p0;
	    dp1 = v2->color->a - p0;
	    dpdx = dy0*dp1 - dy1*dp0;
	    dpdy = dp0*dx1 - dp1*dx0;
	    tr.a0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 +
				 FR_COLOR_ROUND, COLOR_FRAC_BITS);
	    tr.dadx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
	    tr.dady0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0],
				       COLOR_FRAC_BITS);
	    tr.dady0[1] = tr.dady0[0] + (dxdy0 < 0.0F ? -tr.dadx : tr.dadx);
#endif
	}
    }

    if (modeFlags & __GL_SHADE_TEXTURE)
    {
	__GLtexture *current = gc->texture.currentTexture;
	__GLmipMapLevel *lp = current->level[0];

	/* If minFilter != magFilter we need to interpolate rho */
	texNeedRho = current->params.minFilter != current->params.magFilter;

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;

	/* NOTE - some of the conditionals involving texNeedRho are left in
	 * place as documentation but commented out for performance (i.e. its
	 * faster to do the operation than to branch).
	 */
	if (
#if !AFFINE_RHO
	    texNeedRho ||
#endif
	    (modeFlags & __GL_SHADE_TEXTURE_PERSP)) {

	    if (modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
		p0 = v0->texture.x;
		dp0 = v1->texture.x - p0;
		dp1 = v2->texture.x - p0;
	    } else {
		p0 = v0->texture.x*v0->window.w;
		dp0 = v1->texture.x*v1->window.w - p0;
		dp1 = v2->texture.x*v2->window.w - p0;
	    }
	    dpdx = dy0*dp1 - dy1*dp0;
	    dpdy = dp0*dx1 - dp1*dx0;
	    tr.fsw0 = p0 + dpdy * ysnap0 + dpdx * xsnap0;
	    tr.fdswdx = dpdx;
	    /* if (texNeedRho) */
	    {
		tr.fdswdy = dpdy;
	    }
	    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
	    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

	    if (modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
		p0 = v0->texture.y;
		dp0 = v1->texture.y - p0;
		dp1 = v2->texture.y - p0;
	    } else {
		p0 = v0->texture.y*v0->window.w;
		dp0 = v1->texture.y*v1->window.w - p0;
		dp1 = v2->texture.y*v2->window.w - p0;
	    }
	    dpdx = dy0*dp1 - dy1*dp0;
	    dpdy = dp0*dx1 - dp1*dx0;
	    tr.ftw0 = p0 + dpdy * ysnap0 + dpdx * xsnap0;
	    tr.fdtwdx = dpdx;
	    /* if (texNeedRho) */
	    {
		tr.fdtwdy = dpdy;
	    }
	    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
	    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

	    if (modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
		p0 = v0->texture.w;
		dp0 = v1->texture.w - p0;
		dp1 = v2->texture.w - p0;
	    } else {
		p0 = v0->texture.w*v0->window.w;
		dp0 = v1->texture.w*v1->window.w - p0;
		dp1 = v2->texture.w*v2->window.w - p0;
	    }
	    dpdx = dy0*dp1 - dy1*dp0;
	    dpdy = dp0*dx1 - dp1*dx0;
	    tr.fqw0 = p0 + dpdy * ysnap0 + dpdx * xsnap0;
	    tr.fdqwdx = dpdx;
	    /* if (texNeedRho) */
	    {
		tr.fdqwdy = dpdy;
	    }
	    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
	    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

	} else {

	    p0 = v0->texture.x;
	    dp0 = v1->texture.x - p0;
	    dp1 = v2->texture.x - p0;
	    dpdx = dy0*dp1 - dy1*dp0;
	    dpdy = dp0*dx1 - dp1*dx0;
#if AFFINE_RHO
	    /* if (texNeedRho) */
	    {
		tr.fdswdx = dpdx;
		tr.fdswdy = dpdy;
	    }
#endif
	    tr.s0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0, STRQ_FRAC_BITS);
	    tr.dsdx = FloatToFixed(dpdx, STRQ_FRAC_BITS);
	    tr.dsdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], STRQ_FRAC_BITS);
	    tr.dsdy0[1] = tr.dsdy0[0] + (dxdy0 < 0.0F ? -tr.dsdx : tr.dsdx);

	    p0 = v0->texture.y;
	    dp0 = v1->texture.y - p0;
	    dp1 = v2->texture.y - p0;
	    dpdx = dy0*dp1 - dy1*dp0;
	    dpdy = dp0*dx1 - dp1*dx0;
#if AFFINE_RHO
	    /* if (texNeedRho) */
	    {
		tr.fdtwdx = dpdx;
		tr.fdtwdy = dpdy;
	    }
#endif
	    tr.t0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0, STRQ_FRAC_BITS);
	    tr.dtdx = FloatToFixed(dpdx, STRQ_FRAC_BITS);
	    tr.dtdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], STRQ_FRAC_BITS);
	    tr.dtdy0[1] = tr.dtdy0[0] + (dxdy0 < 0.0F ? -tr.dtdx : tr.dtdx);

#if AFFINE_RHO
	    if (texNeedRho) {
		p0 = v0->texture.w;
		dp0 = v1->texture.w - p0;
		dp1 = v2->texture.w - p0;
		dpdx = dy0*dp1 - dy1*dp0;
		dpdy = dp0*dx1 - dp1*dx0;
		tr.fdqwdx = dpdx;
		tr.fdqwdy = dpdy;
	    }
#endif
	}

	/* calculate rho/w at the vertices */
	if (texNeedRho) {
#ifdef RHO_BUG
	    if (tr.dx < 0) tr.dxdy0Int[0] = -tr.dxdy0Int[0];
#endif
	    __glFRSetupRho(gc, &tr, v0, v1, v2, dx0, dy0, dx1, dy1, dxdy0,
			   xsnap0, ysnap0);
#ifdef RHO_BUG
	    if (tr.dx < 0) tr.dxdy0Int[0] = -tr.dxdy0Int[0];
#endif
	}
    }

    if (modeFlags & __GL_SHADE_SLOW_FOG) {

	if (gc->state.hints.fog == GL_NICEST) {
	    /* Use eyeZ for interpolation value */
	    p0 = v0->eye.z;
	    dp0 = v1->eye.z - p0;
	    dp1 = v2->eye.z - p0;
	} else {
	    /* Use fog(eyeZ) for interpolation value */
	    p0 = v0->fog;
	    dp0 = v1->fog - p0;
	    dp1 = v2->fog - p0;

	    /* HACK - can we move this to the fog calculation? */
	    p0 *= 255.0f;
	    dp0 *= 255.0f;
	    dp1 *= 255.0f;
	}

	dpdx = dy0*dp1 - dy1*dp0;
	dpdy = dp0*dx1 - dp1*dx0;

	tr.f0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 +
			     COLOR_ROUND_NO_DITHER, COLOR_FRAC_BITS);
	tr.dfdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
	tr.dfdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0],
				   COLOR_FRAC_BITS);
	tr.dfdy0[1] = tr.dfdy0[0] + (dxdy0 < 0.0F ? -tr.dfdx : tr.dfdx);

    }

#if 0
    v0->color = &v0->colors[__GL_FRONTFACE];
    v1->color = &v1->colors[__GL_FRONTFACE];
    v2->color = &v2->colors[__GL_FRONTFACE];
    if ((modeFlags & __GL_SHADE_SMOOTH) == 0) {
	gc->vertex.provoking->color = &gc->vertex.provoking->colors[__GL_FRONTFACE];
    }
#endif

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;
	if (doubleStore) {
	    tr.dcp2dx = -tr.dcp2dx;
	}
	if (modeFlags & __GL_SHADE_STENCIL_TEST) {
	    tr.dspdx = -tr.dspdx;
	}
	if (modeFlags & __GL_SHADE_DEPTH_TEST) {
	    tr.dzpdx = -tr.dzpdx;
	    tr.dzdx = -tr.dzdx;
	}
	if (modeFlags & __GL_SHADE_SMOOTH) {
	    tr.drdx = -tr.drdx;
	    if (modeFlags & __GL_SHADE_RGB) {
		tr.dgdx = -tr.dgdx; tr.dbdx = -tr.dbdx;
#if FR_RGBA_COLOR
		tr.dadx = -tr.dadx;
#endif
	    }
	}
	if (modeFlags & __GL_SHADE_TEXTURE) {
	    if (
#if !AFFINE_RHO
		texNeedRho ||
#endif
		(modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
		tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
		tr.fdqwdx = -tr.fdqwdx;
		/* if (texNeedRho) */
		{
		    tr.fdrhowdx = -tr.fdrhowdx;
		}
	    } else {
		tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;
#if AFFINE_RHO
		/* if (texNeedRho) */
		{
		    tr.drhodx = -tr.drhodx;
		}
#endif
	    }
	}
	if (modeFlags & __GL_SHADE_SLOW_FOG) {
	    tr.dfdx = -tr.dfdx;
	}
    }

    tr.dcpdxSpan = tr.dcpdx << 5;
    if (doubleStore) {
	tr.dcp2dxSpan = tr.dcp2dx << 5;
    }
    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
	tr.dspdxSpan = tr.dspdx << 5;
    }
    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
	tr.dzpdxSpan = tr.dzpdx << 5;
	tr.dzdxSpan = tr.dzdx << 5;
    }
    if (modeFlags & __GL_SHADE_SMOOTH) {
	tr.drdxSpan = tr.drdx << 5;
	if (modeFlags & __GL_SHADE_RGB) {
	    tr.dgdxSpan = tr.dgdx << 5;
	    tr.dbdxSpan = tr.dbdx << 5;
#if FR_RGBA_COLOR
	    tr.dadxSpan = tr.dadx << 5;
#endif
	}
    }
    if (modeFlags & __GL_SHADE_TEXTURE) {
	if (
#if !AFFINE_RHO
	    texNeedRho ||
#endif
	    (modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
	    tr.fdswdxSpan = tr.fdswdx * 32.0F;
	    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
	    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
	    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
	    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
	    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
	    /* if (texNeedRho) */
	    {
		tr.fdrhowdxSpan = tr.fdrhowdx * 32.0F;
		tr.fdrhowdxPWL = tr.fdrhowdx * __FR_TEXEL_PWL_SPAN_WIDTH;
	    }
	} else {
	    tr.dsdxSpan = tr.dsdx << 5;
	    tr.dtdxSpan = tr.dtdx << 5;
#if AFFINE_RHO
	    /* if (texNeedRho) */
	    {
		tr.drhodxSpan = tr.drhodx * 32.0F;
	    }
#endif
	}
    }
    if (modeFlags & __GL_SHADE_SLOW_FOG) {
	tr.dfdxSpan = tr.dfdx << 5;
    }

    if (modeFlags & __GL_SHADE_DITHER) {
	tr.ditherTable = (tr.dx < 0) ? __glFRDitherTableRtoL : __glFRDitherTable;
    }

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;
	    if (modeFlags & __GL_SHADE_DITHER) {
		tr.dither = &tr.ditherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];
	    }
	    tr.cp = tr.cp0;
	    if (doubleStore) {
		tr.cp2 = tr.cp20;
	    }
	    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
		tr.sp = tr.sp0;
	    }
	    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
		tr.zp = tr.zp0;
		tr.z = tr.z0;
	    }
	    if (modeFlags & __GL_SHADE_SMOOTH) {
		tr.r = tr.r0;
		if (modeFlags & __GL_SHADE_RGB) {
		    tr.g = tr.g0; tr.b = tr.b0;
#if FR_RGBA_COLOR
		    tr.a = tr.a0;
#endif
		}
	    }
	    if (modeFlags & __GL_SHADE_TEXTURE) {
		if (
#if !AFFINE_RHO
		    texNeedRho ||
#endif
		    (modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
		    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
		    tr.fqw = tr.fqw0;
		    /* if (texNeedRho) */
		    {
			tr.frhow = tr.frhow0;
		    }
		} else {
		    tr.s = tr.s0; tr.t = tr.t0;
#if AFFINE_RHO
		    /* if (texNeedRho) */
		    {
			tr.rho = tr.rho0;
		    }
#endif
		}
	    }
	    if (modeFlags & __GL_SHADE_SLOW_FOG) {
		tr.f = tr.f0;
	    }

	    while (w > 0) {
		GLuint mask;

		if (w < 32) {
		    mask = __glFRMaskTable[w];
		} else {
		    mask = ~0;
		}

		(*gc->procs.renderSpan)(mask, &tr);

		w -= 32;
		if (w <= 0) break;

		tr.x += 32;
		if (modeFlags & __GL_SHADE_DITHER) {
		    tr.dither = &tr.ditherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];
		}
		tr.cp += tr.dcpdxSpan;
		if (doubleStore) {
		    tr.cp2 += tr.dcp2dxSpan;
		}
		if (modeFlags & __GL_SHADE_STENCIL_TEST) {
		    tr.sp += tr.dspdxSpan;
		}
		if (modeFlags & __GL_SHADE_DEPTH_TEST) {
		    tr.zp += tr.dzpdxSpan;
		    tr.z += tr.dzdxSpan;
		}
		if (modeFlags & __GL_SHADE_SMOOTH) {
		    tr.r += tr.drdxSpan;
		    if (modeFlags & __GL_SHADE_RGB) {
			tr.g += tr.dgdxSpan; tr.b += tr.dbdxSpan;
#if FR_RGBA_COLOR
			tr.a += tr.dadxSpan;
#endif
		    }
		}
		if (modeFlags & __GL_SHADE_TEXTURE) {
		    if (
#if !AFFINE_RHO
			texNeedRho ||
#endif
			(modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
			tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
			tr.fqw += tr.fdqwdxSpan;
			/* if (texNeedRho) */
			{
			    tr.frhow += tr.fdrhowdxSpan;
			}
		    } else {
			tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;
#if AFFINE_RHO
			/* if (texNeedRho) */
			{
			    tr.rho += tr.drhodxSpan;
			}
#endif
		    }
		}
		if (modeFlags & __GL_SHADE_SLOW_FOG) {
		    tr.f += tr.dfdxSpan;
		}
	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];
	    if (doubleStore) {
		tr.cp20 += tr.dcp2dy0[step];
	    }
	    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
		tr.sp0 += tr.dspdy0[step];
	    }
	    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
		tr.zp0 += tr.dzpdy0[step];
		tr.z0 += tr.dzdy0[step];
	    }
	    if (modeFlags & __GL_SHADE_SMOOTH) {
		tr.r0 += tr.drdy0[step];
		if (modeFlags & __GL_SHADE_RGB) {
		    tr.g0 += tr.dgdy0[step]; tr.b0 += tr.dbdy0[step];
#if FR_RGBA_COLOR
		    tr.a0 += tr.dady0[step];
#endif
		}
	    }
	    if (modeFlags & __GL_SHADE_TEXTURE) {
		if (
#if !AFFINE_RHO
		    texNeedRho ||
#endif
		    (modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
		    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
		    tr.fqw0 += tr.fdqwdy0[step];
		    /* if (texNeedRho) */
		    {
			tr.frhow0 += tr.fdrhowdy0[step];
		    }
		} else {
		    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];
#if AFFINE_RHO
		    /* if (texNeedRho) */
		    {
			tr.rho0 += tr.drhody0[step];
		    }
#endif
		}
	    }
	    if (modeFlags & __GL_SHADE_SLOW_FOG) {
		tr.f0 += tr.dfdy0[step];
	    }
	    tr.x1Frac += tr.dxdy1Frac;
	    step = (unsigned) tr.x1Frac >> 31;
	    tr.x1Frac &= ~0x80000000UL;
	    tr.x1Int += tr.dxdy1Int[step];

	    tr.y += 1;
	}

	if (tr.y1Int == tr.y2Int) break;

	tr.y0Int = tr.y2Int; tr.y2Int = tr.y1Int;
	tr.x1Int = tr.x2Int;
	tr.dxdy1Int[0] = tr.dxdy2Int[0]; tr.dxdy1Int[1] = tr.dxdy2Int[1];
	tr.x1Frac = tr.x2Frac;
	tr.dxdy1Frac = tr.dxdy2Frac;
    }

    __GL_UNLOCK_RENDER_BUFFERS(gc);
}


/* Compute the "rho" (level of detail) parameter used by the texturing
 * code.  Instead of fully computing the derivatives compute nearby
 * texture coordinates and discover the derivative.  The incoming s &
 * t arguments have not been divided by winv yet.
 */

static __GLfloat
__glFRComputePolygonRho(__GLcontext *gc, const __GLtri *tr,
			__GLfloat s, __GLfloat t, __GLfloat qw)
{
    const GLuint modeFlags = gc->polygon.shader.modeFlags;
    const __GLtexture *tex = gc->texture.currentTexture;
    __GLfloat qw0, qw1, p0, p1;
    __GLfloat pupx, pupy, pvpx, pvpy;
    __GLfloat px, py;

    /* Compute partial of u with respect to x */
    qw0 = 1.0f / (qw - tr->fdqwdx);
    qw1 = 1.0f / (qw + tr->fdqwdx);
    p0 = (s - tr->fdswdx) * qw0;
    p1 = (s + tr->fdswdx) * qw1;
    pupx = p1 - p0;
    if (!(modeFlags & __GL_SHADE_TEXTURE_UVSCALED)) {
	pupx *= tex->level[0]->width2f;
    }

    /* Compute partial of v with respect to x */
    p0 = (t - tr->fdtwdx) * qw0;
    p1 = (t + tr->fdtwdx) * qw1;
    pvpx = p1 - p0;
    if (!(modeFlags & __GL_SHADE_TEXTURE_UVSCALED)) {
	pvpx *= tex->level[0]->height2f;
    }

    /* Compute partial of u with respect to y */
    qw0 = 1.0f / (qw - tr->fdqwdy);
    qw1 = 1.0f / (qw + tr->fdqwdy);
    p0 = (s - tr->fdswdy) * qw0;
    p1 = (s + tr->fdswdy) * qw1;
    pupy = p1 - p0;
    if (!(modeFlags & __GL_SHADE_TEXTURE_UVSCALED)) {
	pupy *= tex->level[0]->width2f;
    }

    /* Compute partial of v with respect to y */
    p0 = (t - tr->fdtwdy) * qw0;
    p1 = (t + tr->fdtwdy) * qw1;
    pvpy = p1 - p0;
    if (!(modeFlags & __GL_SHADE_TEXTURE_UVSCALED)) {
	pvpy *= tex->level[0]->height2f;
    }

    /* Finally, figure sum of squares */
    px = pupx * pupx + pvpx * pvpx;
    py = pupy * pupy + pvpy * pvpy;

    /* Return largest value as the level of detail */
    if (px > py) {
	return px * ((__GLfloat) 0.25);
    } else {
	return py * ((__GLfloat) 0.25);
    }
}

void __glFRSetupRho(__GLcontext *gc, __GLtri *tr,
		    __GLvertex *v0, __GLvertex *v1, __GLvertex *v2,
		    float dx0, float dy0, float dx1, float dy1, float dxdy0,
		    float xsnap0, float ysnap0)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    float s0, t0, q0, s1, t1, q1, s2, t2, q2, rho0, rho1, rho2;
    float p0, dp0, dp1, dpdx, dpdy;

    s0 = v0->texture.x; t0 = v0->texture.y; q0 = v0->texture.w;
    s1 = v1->texture.x; t1 = v1->texture.y; q1 = v1->texture.w;
    s2 = v2->texture.x; t2 = v2->texture.y; q2 = v2->texture.w;

    if ((modeFlags * __GL_SHADE_TEXTURE_PERSP) &&
        !(modeFlags & __GL_SHADE_TEXTURE_PROJSCALED))
    {
	s0 *= v0->window.w; t0 *= v0->window.w; q0 *= v0->window.w;
	s1 *= v1->window.w; t1 *= v1->window.w; q1 *= v1->window.w;
	s2 *= v2->window.w; t2 *= v2->window.w; q2 *= v2->window.w;
    }

    rho0 = __glFRComputePolygonRho(gc, tr, s0, t0, q0) * q0;
    rho1 = __glFRComputePolygonRho(gc, tr, s1, t1, q1) * q1;
    rho2 = __glFRComputePolygonRho(gc, tr, s2, t2, q2) * q2;

    p0 = rho0;
    dp0 = rho1 - p0;
    dp1 = rho2 - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;

#if AFFINE_RHO
    if (modeFlags & __GL_SHADE_TEXTURE_PERSP)
#endif
    {
	tr->frhow0 = p0 + dpdy * ysnap0 + dpdx * xsnap0;
	tr->fdrhowdx = dpdx;
#ifdef RHO_BUG
	tr->fdrhowdy0[0] = dpdy + dpdx *
	    (tr->dx < 0 ? -tr->dxdy0Int[0] : tr->dxdy0Int[0]);
#else
	tr->fdrhowdy0[0] = dpdy + dpdx * tr->dxdy0Int[0];
#endif
	tr->fdrhowdy0[1] = tr->fdrhowdy0[0] +
	    (dxdy0 < 0.0F ? -tr->fdrhowdx : tr->fdrhowdx);
    }
#if AFFINE_RHO
    else {
	tr->rho0 = p0 + dpdy * ysnap0 + dpdx * xsnap0;
	tr->drhodx = dpdx;
	tr->drhody0[0] = dpdy + dpdx * tr->dxdy0Int[0];
	tr->drhody0[1] = tr->drhody0[0] +
	    (dxdy0 < 0.0F ? -tr->drhodx : tr->drhodx);
    }
#endif

}

#endif /* __GL_PC_RAST */

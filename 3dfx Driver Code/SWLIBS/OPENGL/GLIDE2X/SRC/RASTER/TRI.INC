#if 0 /* Perl cannot handle C-style comments */
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
#endif
FR_TRI_RASTERIZER_PROTOTYPE
{
\#ifndef __GL_CODEGEN
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;
#if FR_INTERPOLANTS
    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;
#endif
    float ftmp;
    double dtmp;
#if FR_SMOOTH_SHADING
    __GLcolor tmpColor;
#endif
    __GLtri tr;		/* Must be last auto for assembler access */

    *((int *)&v0->window.x) &= 0xffffff80;
    *((int *)&v0->window.y) &= 0xffffff80;
    *((int *)&v1->window.x) &= 0xffffff80;
    *((int *)&v1->window.y) &= 0xffffff80;
    *((int *)&v2->window.x) &= 0xffffff80;
    *((int *)&v2->window.y) &= 0xffffff80;

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
    if (c == 0.0F) return;

    ccw = (unsigned int) FloatBits(c) >> 31;
    tr.dx = 1 - (ccw << 1);

    {
	GLuint modeFlags = gc->polygon.shader.modeFlags;
	GLuint needs, faceNeeds;
	GLint face, colorFace;

	face = gc->polygon.face[ccw ^ reversed];
	if (face == gc->polygon.cullFace) {
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
#if FR_SMOOTH_SHADING
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
#endif

#if FR_FLAT_SHADING
	{
	    __GLvertex *pv = gc->vertex.provoking;

	    pv->color = &pv->colors[colorFace];
	    if (~pv->hasAndClipCode & needs) DO_VALIDATE(gc, pv, needs);
	    if (~v0->hasAndClipCode & needs) DO_VALIDATE(gc, v0, needs);
	    if (~v1->hasAndClipCode & needs) DO_VALIDATE(gc, v1, needs);
	    if (~v2->hasAndClipCode & needs) DO_VALIDATE(gc, v2, needs);
	}
#endif
    }

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

    __GL_LOCK_BUFFERS(gc);

    {
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }
#if FR_STENCIL_BUFFER
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
#endif
#if FR_DEPTH_BUFFER
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
#endif

#if FR_FLAT_SHADING
    {
	__GLcolor *flatColor = gc->vertex.provoking->color;
#if !FR_DITHER
	__GLcolorBuffer *cfb = gc->drawBuffer;
#endif

	tr.r = FloatToFixed(flatColor->r + FR_COLOR_ROUND, COLOR_FRAC_BITS);
#if FR_RGB_COLOR
	tr.g = FloatToFixed(flatColor->g + FR_COLOR_ROUND, COLOR_FRAC_BITS);
	tr.b = FloatToFixed(flatColor->b + FR_COLOR_ROUND, COLOR_FRAC_BITS);
#if FR_RGBA_COLOR
	tr.a = FloatToFixed(flatColor->a + FR_COLOR_ROUND, COLOR_FRAC_BITS);
#endif /* FR_RGBA_COLOR */
#endif /* FR_RGB_COLOR */

#if !FR_DITHER
#if FR_CI_COLOR
	tr.r0 = CoordToInt(tr.r);
#endif /* FR_CI_COLOR */
#if FR_RGB_COLOR
	tr.r0 =
#if FR_RGBA_COLOR
	    (CoordToInt(tr.a) << cfb->alphaShift) |
#endif /* FR_RGBA_COLOR */
	    (CoordToInt(tr.r) << cfb->redShift)   |
	    (CoordToInt(tr.g) << cfb->greenShift) |
	    (CoordToInt(tr.b) << cfb->blueShift);
#endif /* FR_RGB_COLOR */
#endif /* !FR_DITHER */
    }
#endif /* FR_FLAT_SHADING */

#if FR_INTERPOLANTS
    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;
#endif
#if FR_DEPTH_BUFFER
    p0 = v0->window.z;
    dp0 = v1->window.z - p0;
    dp1 = v2->window.z - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.z0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + 0.5F,
							Z_FRAC_BITS);
    tr.dzdx = FloatToFixed(dpdx, Z_FRAC_BITS);
    tr.dzdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], Z_FRAC_BITS);
    tr.dzdy0[1] = tr.dzdy0[0] + (dxdy0 < 0.0F ? -tr.dzdx : tr.dzdx);
#endif
#if FR_SMOOTH_SHADING
    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + FR_COLOR_ROUND,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);
#endif
#if FR_SMOOTH_SHADING && FR_RGB_COLOR
    p0 = v0->color->g;
    dp0 = v1->color->g - p0;
    dp1 = v2->color->g - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.g0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + FR_COLOR_ROUND,
							COLOR_FRAC_BITS);
    tr.dgdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.dgdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.dgdy0[1] = tr.dgdy0[0] + (dxdy0 < 0.0F ? -tr.dgdx : tr.dgdx);

    p0 = v0->color->b;
    dp0 = v1->color->b - p0;
    dp1 = v2->color->b - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.b0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + FR_COLOR_ROUND,
							COLOR_FRAC_BITS);
    tr.dbdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.dbdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.dbdy0[1] = tr.dbdy0[0] + (dxdy0 < 0.0F ? -tr.dbdx : tr.dbdx);
#if FR_SMOOTH_SHADING && FR_RGBA_COLOR
    p0 = v0->color->a;
    dp0 = v1->color->a - p0;
    dp1 = v2->color->a - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.a0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + FR_COLOR_ROUND,
							COLOR_FRAC_BITS);
    tr.dadx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.dady0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.dady0[1] = tr.dady0[0] + (dxdy0 < 0.0F ? -tr.dadx : tr.dadx);
#endif
#endif
#if FR_TEXTURING
    {
	__GLmipMapLevel *lp = &gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }
#endif
#if FR_TEXTURING && FR_AFFINE_TEXTURING
    p0 = v0->texture.x;
    dp0 = v1->texture.x - p0;
    dp1 = v2->texture.x - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.s0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0, STRQ_FRAC_BITS);
    tr.dsdx = FloatToFixed(dpdx, STRQ_FRAC_BITS);
    tr.dsdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], STRQ_FRAC_BITS);
    tr.dsdy0[1] = tr.dsdy0[0] + (dxdy0 < 0.0F ? -tr.dsdx : tr.dsdx);

    p0 = v0->texture.y;
    dp0 = v1->texture.y - p0;
    dp1 = v2->texture.y - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.t0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0, STRQ_FRAC_BITS);
    tr.dtdx = FloatToFixed(dpdx, STRQ_FRAC_BITS);
    tr.dtdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], STRQ_FRAC_BITS);
    tr.dtdy0[1] = tr.dtdy0[0] + (dxdy0 < 0.0F ? -tr.dtdx : tr.dtdx);
#endif
#if FR_TEXTURING && FR_PERSPECTIVE_TEXTURING
    p0 = v0->texture.x;
    dp0 = v1->texture.x - p0;
    dp1 = v2->texture.x - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.fsw0 = p0 + dpdy * ysnap0 + dpdx * xsnap0;
    tr.fdswdx = dpdx;
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    p0 = v0->texture.y;
    dp0 = v1->texture.y - p0;
    dp1 = v2->texture.y - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.ftw0 = p0 + dpdy * ysnap0 + dpdx * xsnap0;
    tr.fdtwdx = dpdx;
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    p0 = v0->texture.w;
    dp0 = v1->texture.w - p0;
    dp1 = v2->texture.w - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.fqw0 = p0 + dpdy * ysnap0 + dpdx * xsnap0;
    tr.fdqwdx = dpdx;
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);
#endif

#if FR_SMOOTH_SHADING
    v0->color = &v0->colors[__GL_FRONTFACE];
    v1->color = &v1->colors[__GL_FRONTFACE];
    v2->color = &v2->colors[__GL_FRONTFACE];
#endif
#if FR_FLAT_SHADING
    gc->vertex.provoking->color = &gc->vertex.provoking->colors[__GL_FRONTFACE];
#endif

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;
#if FR_STENCIL_BUFFER
	tr.dspdx = -tr.dspdx;
#endif
#if FR_DEPTH_BUFFER
	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;
#endif
#if FR_SMOOTH_SHADING
	tr.drdx = -tr.drdx;
#endif
#if FR_SMOOTH_SHADING && FR_RGB_COLOR
	tr.dgdx = -tr.dgdx; tr.dbdx = -tr.dbdx;
#if FR_RGBA_COLOR
	tr.dadx = -tr.dadx;
#endif
#endif
#if FR_TEXTURING && FR_AFFINE_TEXTURING
	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;
#endif
#if FR_TEXTURING && FR_PERSPECTIVE_TEXTURING
	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;
#endif
    }

    tr.dcpdxSpan = tr.dcpdx << 5;
#if FR_STENCIL_BUFFER
    tr.dspdxSpan = tr.dspdx << 5;
#endif
#if FR_DEPTH_BUFFER
    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;
#endif
#if FR_SMOOTH_SHADING
    tr.drdxSpan = tr.drdx << 5;
#endif
#if FR_SMOOTH_SHADING && FR_RGB_COLOR
    tr.dgdxSpan = tr.dgdx << 5;
    tr.dbdxSpan = tr.dbdx << 5;
#if FR_RGBA_COLOR
    tr.dadxSpan = tr.dadx << 5;
#endif
#endif
#if FR_TEXTURING && FR_AFFINE_TEXTURING
    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;
#endif
#if FR_TEXTURING && FR_PERSPECTIVE_TEXTURING
    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
#endif

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;
#if FR_DITHER
	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];
#endif
	    tr.cp = tr.cp0;
#if FR_STENCIL_BUFFER
	    tr.sp = tr.sp0;
#endif
#if FR_DEPTH_BUFFER
	    tr.zp = tr.zp0;
	    tr.z = tr.z0;
#endif
#if FR_SMOOTH_SHADING
	    tr.r = tr.r0;
#endif
#if FR_SMOOTH_SHADING && FR_RGB_COLOR
	    tr.g = tr.g0; tr.b = tr.b0;
#if FR_RGBA_COLOR
	    tr.a = tr.a0;
#endif
#endif
#if FR_TEXTURING && FR_AFFINE_TEXTURING
	    tr.s = tr.s0; tr.t = tr.t0;
#endif
#if FR_TEXTURING && FR_PERSPECTIVE_TEXTURING
	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;
#endif
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
#if FR_DITHER
		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];
#endif
		tr.cp += tr.dcpdxSpan;
#if FR_STENCIL_BUFFER
		tr.sp += tr.dspdxSpan;
#endif
#if FR_DEPTH_BUFFER
		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;
#endif
#if FR_SMOOTH_SHADING
		tr.r += tr.drdxSpan;
#endif
#if FR_SMOOTH_SHADING && FR_RGB_COLOR
		tr.g += tr.dgdxSpan; tr.b += tr.dbdxSpan;
#if FR_RGBA_COLOR
		tr.a += tr.dadxSpan;
#endif
#endif
#if FR_TEXTURING && FR_AFFINE_TEXTURING
		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;
#endif
#if FR_TEXTURING && FR_PERSPECTIVE_TEXTURING
		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;
#endif
	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];
#if FR_STENCIL_BUFFER
	    tr.sp0 += tr.dspdy0[step];
#endif
#if FR_DEPTH_BUFFER
	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];
#endif
#if FR_SMOOTH_SHADING
	    tr.r0 += tr.drdy0[step];
#endif
#if FR_SMOOTH_SHADING && FR_RGB_COLOR
	    tr.g0 += tr.dgdy0[step]; tr.b0 += tr.dbdy0[step];
#if FR_RGBA_COLOR
	    tr.a0 += tr.dady0[step];
#endif
#endif
#if FR_TEXTURING && FR_AFFINE_TEXTURING
	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];
#endif
#if FR_TEXTURING && FR_PERSPECTIVE_TEXTURING
	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];
#endif
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
    __GL_UNLOCK_BUFFERS(gc);
\#endif
}

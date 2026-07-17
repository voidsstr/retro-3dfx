#if defined(__GL_PC_RAST) && !defined(__GL_CODEGEN)
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
#include "context.h"
#include "global.h"
#include "xform.h"
#include "string.h"
#include "fr_modes.h"
#include "fr_tri.h"

/*
 * #define FR_INTERPOLANTS 0
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_0(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

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

		tr.cp += tr.dcpdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

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

/*
 * #define FR_INTERPOLANTS 0
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_1(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_2(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.drdx = -tr.drdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.r = tr.r0;

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

		tr.cp += tr.dcpdxSpan;

		tr.r += tr.drdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.r0 += tr.drdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_3(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.drdx = -tr.drdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.r = tr.r0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.r += tr.drdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.r0 += tr.drdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_4(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.cp += tr.dcpdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_5(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_6(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.drdx = -tr.drdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.r = tr.r0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.cp += tr.dcpdxSpan;

		tr.r += tr.drdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_7(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.drdx = -tr.drdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.r = tr.r0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.r += tr.drdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_C(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.cp += tr.dcpdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_D(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_E(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.drdx = -tr.drdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.r = tr.r0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.cp += tr.dcpdxSpan;

		tr.r += tr.drdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_F(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.drdx = -tr.drdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.r = tr.r0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.r += tr.drdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_10(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

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

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_11(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_12(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

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

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_13(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_14(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_15(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_16(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_17(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_1C(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_1D(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_1E(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 0
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_1F(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 0
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_20(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

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

/*
 * #define FR_INTERPOLANTS 0
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_21(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_22(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.drdx = -tr.drdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.r = tr.r0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.r += tr.drdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.r0 += tr.drdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_23(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.drdx = -tr.drdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.r = tr.r0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.r += tr.drdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.r0 += tr.drdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_24(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_25(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_26(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.drdx = -tr.drdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.r = tr.r0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.r += tr.drdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_27(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.drdx = -tr.drdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.r = tr.r0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.r += tr.drdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_2C(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_2D(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_2E(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.drdx = -tr.drdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.r = tr.r0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.r += tr.drdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 0
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_2F(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.drdx = -tr.drdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.r = tr.r0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.r += tr.drdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_30(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_31(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_32(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_33(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_34(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_35(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_36(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 1
 * #define FR_PERSPECTIVE_TEXTURING 0
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_37(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

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

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

	tr.dsdx = -tr.dsdx; tr.dtdx = -tr.dtdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.dsdxSpan = tr.dsdx << 5;
    tr.dtdxSpan = tr.dtdx << 5;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

	    tr.s = tr.s0; tr.t = tr.t0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

		tr.s += tr.dsdxSpan; tr.t += tr.dtdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.s0 += tr.dsdy0[step]; tr.t0 += tr.dtdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_3C(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	__GLcolorBuffer *cfb = gc->drawBuffer;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_NO_DITHER , COLOR_FRAC_BITS);

	tr.r0 = CoordToInt(tr.r);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_3D(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    {
	__GLcolor *flatColor = gc->vertex.provoking->color;

	tr.r = FloatToFixed(flatColor->r + COLOR_ROUND_DITHER , COLOR_FRAC_BITS);

    }

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 0
 * #define FR_COLOR_ROUND COLOR_ROUND_NO_DITHER
 */

void  __glRenderTriangleCI_3E(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_NO_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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

/*
 * #define FR_INTERPOLANTS 1
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_AFFINE_TEXTURING 0
 * #define FR_PERSPECTIVE_TEXTURING 1
 * #define FR_DEPTH_BUFFER 1
 * #define FR_STENCIL_BUFFER 1
 * #define FR_DITHER 1
 * #define FR_COLOR_ROUND COLOR_ROUND_DITHER
 */

void  __glRenderTriangleCI_3F(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) 
{
    float dx0, dx1, dx2, dy0, dy1, dy2, x, c;
    float dxdy0, dxdy1, dxdy2;
    float ysnap0, ysnap2;
    int ccw, reversed = 0;

    float xsnap0, invc;
    float p0, dp0, dp1, dpdx, dpdy;

    float ftmp;
    double dtmp;
    __GLvertex *ov0 = v0, *ov1 = v1, *ov2 = v2;

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
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSizeLog2 = cfb->buf.elementSizeLog2;

	tr.cp0 = ((GLubyte *) cfb->buf.base) +
			(tr.y0Int * byteWidth) + (tr.x0Int << elementSizeLog2);
	tr.dcpdx = cfb->buf.elementSize;
	tr.dcpdy0[0] = byteWidth + (tr.dxdy0Int[0] << elementSizeLog2);
	tr.dcpdy0[1] = byteWidth + (tr.dxdy0Int[1] << elementSizeLog2);
    }

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

    xsnap0 = (float) (tr.x0Int + XY_BIAS + 1) - v0->window.x;

    invc = 1.0F / c;
    dx0 *= invc;
    dy0 *= invc;
    dx1 *= invc;
    dy1 *= invc;

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

    p0 = v0->color->r;
    dp0 = v1->color->r - p0;
    dp1 = v2->color->r - p0;
    dpdx = dy0*dp1 - dy1*dp0;
    dpdy = dp0*dx1 - dp1*dx0;
    tr.r0 = FloatToFixed(p0 + dpdy * ysnap0 + dpdx * xsnap0 + COLOR_ROUND_DITHER ,
							COLOR_FRAC_BITS);
    tr.drdx = FloatToFixed(dpdx, COLOR_FRAC_BITS);
    tr.drdy0[0] = FloatToFixed(dpdy + dpdx * tr.dxdy0Int[0], COLOR_FRAC_BITS);
    tr.drdy0[1] = tr.drdy0[0] + (dxdy0 < 0.0F ? -tr.drdx : tr.drdx);

    {
	__GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

	tr.tp = (GLubyte *) lp->buffer;
	tr.texWidth = lp->width;
	tr.texWidthLog2 = lp->widthLog2;
	tr.texHeight = lp->height;
	tr.texHeightLog2 = lp->heightLog2;
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdswdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdswdy0[1] = tr.fdswdy0[0] + (dxdy0 < 0.0F ? -tr.fdswdx : tr.fdswdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdtwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdtwdy0[1] = tr.fdtwdy0[0] + (dxdy0 < 0.0F ? -tr.fdtwdx : tr.fdtwdx);

    if (gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
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
    tr.fdqwdy0[0] = dpdy + dpdx * tr.dxdy0Int[0];
    tr.fdqwdy0[1] = tr.fdqwdy0[0] + (dxdy0 < 0.0F ? -tr.fdqwdx : tr.fdqwdx);

    __GL_RESTORE_TRIANGLE_VERTS(gc, ov0, ov1, ov2, gc->vertex.provoking);

    if (tr.dx < 0) {
	tr.x0Int = -tr.x0Int; tr.x1Int = -tr.x1Int; tr.x2Int = -tr.x2Int;
	tr.dxdy0Int[0] = -tr.dxdy0Int[0]; tr.dxdy0Int[1] = -tr.dxdy0Int[1];
	tr.dxdy1Int[0] = -tr.dxdy1Int[0]; tr.dxdy1Int[1] = -tr.dxdy1Int[1];
	tr.dxdy2Int[0] = -tr.dxdy2Int[0]; tr.dxdy2Int[1] = -tr.dxdy2Int[1];

	tr.dcpdx = -tr.dcpdx;

	tr.dspdx = -tr.dspdx;

	tr.dzpdx = -tr.dzpdx;
	tr.dzdx = -tr.dzdx;

	tr.drdx = -tr.drdx;

	tr.fdswdx = -tr.fdswdx; tr.fdtwdx = -tr.fdtwdx;
	tr.fdqwdx = -tr.fdqwdx;

    }

    tr.dcpdxSpan = tr.dcpdx << 5;

    tr.dspdxSpan = tr.dspdx << 5;

    tr.dzpdxSpan = tr.dzpdx << 5;
    tr.dzdxSpan = tr.dzdx << 5;

    tr.drdxSpan = tr.drdx << 5;

    tr.fdswdxSpan = tr.fdswdx * 32.0F;
    tr.fdtwdxSpan = tr.fdtwdx * 32.0F;
    tr.fdqwdxSpan = tr.fdqwdx * 32.0F;
    tr.fdswdxPWL = tr.fdswdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdtwdxPWL = tr.fdtwdx * __FR_TEXEL_PWL_SPAN_WIDTH;
    tr.fdqwdxPWL = tr.fdqwdx * __FR_TEXEL_PWL_SPAN_WIDTH;

    tr.gc = gc;

    while (1) {
	int h = tr.y2Int - tr.y0Int;

	tr.y = tr.y0Int;

	while (h-- > 0) {
	    int w = tr.x1Int - tr.x0Int;
	    int step;

	    tr.x = tr.x0Int;

	    tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

	    tr.cp = tr.cp0;

	    tr.sp = tr.sp0;

	    tr.zp = tr.zp0;
	    tr.z = tr.z0;

	    tr.r = tr.r0;

	    tr.fsw = tr.fsw0; tr.ftw = tr.ftw0;
	    tr.fqw = tr.fqw0;

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

		tr.dither = &__glFRDitherTable[tr.y&DITHER_MASK][tr.x&DITHER_MASK];

		tr.cp += tr.dcpdxSpan;

		tr.sp += tr.dspdxSpan;

		tr.zp += tr.dzpdxSpan;
		tr.z += tr.dzdxSpan;

		tr.r += tr.drdxSpan;

		tr.fsw += tr.fdswdxSpan; tr.ftw += tr.fdtwdxSpan;
		tr.fqw += tr.fdqwdxSpan;

	    }

	    tr.x0Frac += tr.dxdy0Frac;
	    step = (unsigned) tr.x0Frac >> 31;
	    tr.x0Frac &= ~0x80000000UL;
	    tr.x0Int += tr.dxdy0Int[step];

	    tr.cp0 += tr.dcpdy0[step];

	    tr.sp0 += tr.dspdy0[step];

	    tr.zp0 += tr.dzpdy0[step];
	    tr.z0 += tr.dzdy0[step];

	    tr.r0 += tr.drdy0[step];

	    tr.fsw0 += tr.fdswdy0[step]; tr.ftw0 += tr.fdtwdy0[step];
	    tr.fqw0 += tr.fdqwdy0[step];

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
void ( *__fr_ci_tri_rast_table[64])(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2) =
{
    __glRenderTriangleCI_0,
    __glRenderTriangleCI_1,
    __glRenderTriangleCI_2,
    __glRenderTriangleCI_3,
    __glRenderTriangleCI_4,
    __glRenderTriangleCI_5,
    __glRenderTriangleCI_6,
    __glRenderTriangleCI_7,
    0,
    0,
    0,
    0,
    __glRenderTriangleCI_C,
    __glRenderTriangleCI_D,
    __glRenderTriangleCI_E,
    __glRenderTriangleCI_F,
    __glRenderTriangleCI_10,
    __glRenderTriangleCI_11,
    __glRenderTriangleCI_12,
    __glRenderTriangleCI_13,
    __glRenderTriangleCI_14,
    __glRenderTriangleCI_15,
    __glRenderTriangleCI_16,
    __glRenderTriangleCI_17,
    0,
    0,
    0,
    0,
    __glRenderTriangleCI_1C,
    __glRenderTriangleCI_1D,
    __glRenderTriangleCI_1E,
    __glRenderTriangleCI_1F,
    __glRenderTriangleCI_20,
    __glRenderTriangleCI_21,
    __glRenderTriangleCI_22,
    __glRenderTriangleCI_23,
    __glRenderTriangleCI_24,
    __glRenderTriangleCI_25,
    __glRenderTriangleCI_26,
    __glRenderTriangleCI_27,
    0,
    0,
    0,
    0,
    __glRenderTriangleCI_2C,
    __glRenderTriangleCI_2D,
    __glRenderTriangleCI_2E,
    __glRenderTriangleCI_2F,
    __glRenderTriangleCI_30,
    __glRenderTriangleCI_31,
    __glRenderTriangleCI_32,
    __glRenderTriangleCI_33,
    __glRenderTriangleCI_34,
    __glRenderTriangleCI_35,
    __glRenderTriangleCI_36,
    __glRenderTriangleCI_37,
    0,
    0,
    0,
    0,
    __glRenderTriangleCI_3C,
    __glRenderTriangleCI_3D,
    __glRenderTriangleCI_3E,
    __glRenderTriangleCI_3F,
};

#endif /* __GL_PC_RAST && !__GL_CODEGEN */

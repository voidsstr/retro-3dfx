#ifdef __GL_PC_RAST
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
#include "fr_fbtype.h"

#define PIXEL_TYPE 10
#include "fr_fbconf.h"

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_0(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result = tr->r0;

	    /* Step interpolants */

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_1(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		(CoordToInt(a + dither) << ASHIFT) |
#endif /* ASIZE */
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);

	    /* Step interpolants */

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_2(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		(CoordToInt(a) << ASHIFT) |
#endif /* ASIZE */
		(CoordToInt(r) << RSHIFT) |
		(CoordToInt(g) << GSHIFT) |
		(CoordToInt(b) << BSHIFT);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_3(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		(CoordToInt(a + dither) << ASHIFT) |
#endif /* ASIZE */
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_4(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    /* Eventually to be replaced by pixel-format texels */
	    result =
#if ASIZE
		((tp[3]>>AFRAC) << ASHIFT) |
#endif /* ASIZE */
		((tp[0]>>RFRAC) << RSHIFT) |
		((tp[1]>>GFRAC) << GSHIFT) |
		((tp[2]>>BFRAC) << BSHIFT);

	    /* Step interpolants */

	    tp += 3;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_5(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLuint rr, rg, rb;
#if ASIZE
    GLuint ra;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    rr = CoordToInt((tp[0]<<RUB2C) + dither);
#if RSIZE < 8
	    rr -= (rr >> RSIZE);
#endif
	    rg = CoordToInt((tp[1]<<GUB2C) + dither);
#if GSIZE < 8
	    rg -= (rg >> GSIZE);
#endif
	    rb = CoordToInt((tp[2]<<BUB2C) + dither);
#if BSIZE < 8
	    rb -= (rb >> BSIZE);
#endif
#if ASIZE
	    ra = CoordToInt((tp[3]<<AUB2C) + dither);
#if ASIZE < 8
	    ra -= (ra >> ASIZE);
#endif
#endif /* ASIZE */
	    result =
#if ASIZE
		(ra << ASHIFT) |
#endif /* ASIZE */
		(rr << RSHIFT) |
		(rg << GSHIFT) |
		(rb << BSHIFT);

	    /* Step interpolants */

	    tp += 3;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_6(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    /* Eventually to be replaced by pixel-format texels */
	    result =
#if ASIZE
		((tp[3]>>AFRAC) << ASHIFT) |
#endif /* ASIZE */
		((tp[0]>>RFRAC) << RSHIFT) |
		((tp[1]>>GFRAC) << GSHIFT) |
		((tp[2]>>BFRAC) << BSHIFT);

	    /* Step interpolants */

	    tp += 3;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_7(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLuint rr, rg, rb;
#if ASIZE
    GLuint ra;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    rr = CoordToInt((tp[0]<<RUB2C) + dither);
#if RSIZE < 8
	    rr -= (rr >> RSIZE);
#endif
	    rg = CoordToInt((tp[1]<<GUB2C) + dither);
#if GSIZE < 8
	    rg -= (rg >> GSIZE);
#endif
	    rb = CoordToInt((tp[2]<<BUB2C) + dither);
#if BSIZE < 8
	    rb -= (rb >> BSIZE);
#endif
#if ASIZE
	    ra = CoordToInt((tp[3]<<AUB2C) + dither);
#if ASIZE < 8
	    ra -= (ra >> ASIZE);
#endif
#endif /* ASIZE */
	    result =
#if ASIZE
		(ra << ASHIFT) |
#endif /* ASIZE */
		(rr << RSHIFT) |
		(rg << GSHIFT) |
		(rb << BSHIFT);

	    /* Step interpolants */

	    tp += 3;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_C(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		MODULATE_A(tp[3], a) |
#endif /* ASIZE */
		MODULATE_R(tp[0], r) |
		MODULATE_G(tp[1], g) |
		MODULATE_B(tp[2], b);

	    /* Step interpolants */

	    tp += 3;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_D(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |
#endif /* ASIZE */
		MODULATE_R_D(tp[0], r, RCoordToUByte(dither)) |
		MODULATE_G_D(tp[1], g, GCoordToUByte(dither)) |
		MODULATE_B_D(tp[2], b, BCoordToUByte(dither));

	    /* Step interpolants */

	    tp += 3;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_E(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		MODULATE_A(tp[3], a) |
#endif /* ASIZE */
		MODULATE_R(tp[0], r) |
		MODULATE_G(tp[1], g) |
		MODULATE_B(tp[2], b);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 3;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_F(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |
#endif /* ASIZE */
		MODULATE_R_D(tp[0], r, RCoordToUByte(dither)) |
		MODULATE_G_D(tp[1], g, GCoordToUByte(dither)) |
		MODULATE_B_D(tp[2], b, BCoordToUByte(dither));

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 3;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_10(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE

		(CoordToInt(a) << ASHIFT) |

#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);

	    /* Step interpolants */

	    tp += 3;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_11(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE

		(CoordToInt(a + dither) << ASHIFT) |

#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));

	    /* Step interpolants */

	    tp += 3;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_12(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE

		(CoordToInt(a) << ASHIFT) |

#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 3;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_13(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE

		(CoordToInt(a + dither) << ASHIFT) |

#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 3;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_14(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		TBLEND_A(tp[3], a, ba) |
#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);

	    /* Step interpolants */

	    tp += 3;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_15(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		TBLEND_A_D(tp[3], a, ba, ACoordToUByte(dither)) |
#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));

	    /* Step interpolants */

	    tp += 3;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_16(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		TBLEND_A(tp[3], a, ba) |
#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 3;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_17(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		TBLEND_A_D(tp[3], a, ba, ACoordToUByte(dither)) |
#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 3;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_20(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result = tr->r0;

	    /* Step interpolants */

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_21(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		(CoordToInt(a + dither) << ASHIFT) |
#endif /* ASIZE */
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);

	    /* Step interpolants */

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_22(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		(CoordToInt(a) << ASHIFT) |
#endif /* ASIZE */
		(CoordToInt(r) << RSHIFT) |
		(CoordToInt(g) << GSHIFT) |
		(CoordToInt(b) << BSHIFT);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenPixelsRGBa_23(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		(CoordToInt(a + dither) << ASHIFT) |
#endif /* ASIZE */
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_24(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    /* Eventually to be replaced by pixel-format texels */
	    result =
#if ASIZE
		((tp[3]>>AFRAC) << ASHIFT) |
#endif /* ASIZE */
		((tp[0]>>RFRAC) << RSHIFT) |
		((tp[1]>>GFRAC) << GSHIFT) |
		((tp[2]>>BFRAC) << BSHIFT);

	    /* Step interpolants */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_25(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLuint rr, rg, rb;
#if ASIZE
    GLuint ra;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    rr = CoordToInt((tp[0]<<RUB2C) + dither);
#if RSIZE < 8
	    rr -= (rr >> RSIZE);
#endif
	    rg = CoordToInt((tp[1]<<GUB2C) + dither);
#if GSIZE < 8
	    rg -= (rg >> GSIZE);
#endif
	    rb = CoordToInt((tp[2]<<BUB2C) + dither);
#if BSIZE < 8
	    rb -= (rb >> BSIZE);
#endif
#if ASIZE
	    ra = CoordToInt((tp[3]<<AUB2C) + dither);
#if ASIZE < 8
	    ra -= (ra >> ASIZE);
#endif
#endif /* ASIZE */
	    result =
#if ASIZE
		(ra << ASHIFT) |
#endif /* ASIZE */
		(rr << RSHIFT) |
		(rg << GSHIFT) |
		(rb << BSHIFT);

	    /* Step interpolants */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_26(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    /* Eventually to be replaced by pixel-format texels */
	    result =
#if ASIZE
		((tp[3]>>AFRAC) << ASHIFT) |
#endif /* ASIZE */
		((tp[0]>>RFRAC) << RSHIFT) |
		((tp[1]>>GFRAC) << GSHIFT) |
		((tp[2]>>BFRAC) << BSHIFT);

	    /* Step interpolants */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_27(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLuint rr, rg, rb;
#if ASIZE
    GLuint ra;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    rr = CoordToInt((tp[0]<<RUB2C) + dither);
#if RSIZE < 8
	    rr -= (rr >> RSIZE);
#endif
	    rg = CoordToInt((tp[1]<<GUB2C) + dither);
#if GSIZE < 8
	    rg -= (rg >> GSIZE);
#endif
	    rb = CoordToInt((tp[2]<<BUB2C) + dither);
#if BSIZE < 8
	    rb -= (rb >> BSIZE);
#endif
#if ASIZE
	    ra = CoordToInt((tp[3]<<AUB2C) + dither);
#if ASIZE < 8
	    ra -= (ra >> ASIZE);
#endif
#endif /* ASIZE */
	    result =
#if ASIZE
		(ra << ASHIFT) |
#endif /* ASIZE */
		(rr << RSHIFT) |
		(rg << GSHIFT) |
		(rb << BSHIFT);

	    /* Step interpolants */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 1
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_28(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		(CoordToInt(a) << ASHIFT) |
#endif /* ASIZE */
		DECAL_R(tp[0], tp[3], r) |
		DECAL_G(tp[1], tp[3], g) |
		DECAL_B(tp[2], tp[3], b);

	    /* Step interpolants */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 1
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_29(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		(CoordToInt(a + dither) << ASHIFT) |
#endif /* ASIZE */
		DECAL_R_D(tp[0], tp[3], r, RCoordToUByte(dither)) |
		DECAL_G_D(tp[1], tp[3], g, GCoordToUByte(dither)) |
		DECAL_B_D(tp[2], tp[3], b, BCoordToUByte(dither));

	    /* Step interpolants */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 1
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_2A(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		(CoordToInt(a) << ASHIFT) |
#endif /* ASIZE */
		DECAL_R(tp[0], tp[3], r) |
		DECAL_G(tp[1], tp[3], g) |
		DECAL_B(tp[2], tp[3], b);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 1
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_2B(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		(CoordToInt(a + dither) << ASHIFT) |
#endif /* ASIZE */
		DECAL_R_D(tp[0], tp[3], r, RCoordToUByte(dither)) |
		DECAL_G_D(tp[1], tp[3], g, GCoordToUByte(dither)) |
		DECAL_B_D(tp[2], tp[3], b, BCoordToUByte(dither));

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_2C(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		MODULATE_A(tp[3], a) |
#endif /* ASIZE */
		MODULATE_R(tp[0], r) |
		MODULATE_G(tp[1], g) |
		MODULATE_B(tp[2], b);

	    /* Step interpolants */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_2D(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |
#endif /* ASIZE */
		MODULATE_R_D(tp[0], r, RCoordToUByte(dither)) |
		MODULATE_G_D(tp[1], g, GCoordToUByte(dither)) |
		MODULATE_B_D(tp[2], b, BCoordToUByte(dither));

	    /* Step interpolants */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_2E(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		MODULATE_A(tp[3], a) |
#endif /* ASIZE */
		MODULATE_R(tp[0], r) |
		MODULATE_G(tp[1], g) |
		MODULATE_B(tp[2], b);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_2F(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |
#endif /* ASIZE */
		MODULATE_R_D(tp[0], r, RCoordToUByte(dither)) |
		MODULATE_G_D(tp[1], g, GCoordToUByte(dither)) |
		MODULATE_B_D(tp[2], b, BCoordToUByte(dither));

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_30(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE

		MODULATE_A(tp[3], a) |

#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);

	    /* Step interpolants */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_31(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE

		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |

#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));

	    /* Step interpolants */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_32(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE

		MODULATE_A(tp[3], a) |

#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_33(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE

		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |

#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_34(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		TBLEND_A(tp[3], a, ba) |
#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);

	    /* Step interpolants */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_35(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		TBLEND_A_D(tp[3], a, ba, ACoordToUByte(dither)) |
#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));

	    /* Step interpolants */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_36(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
#if ASIZE
		TBLEND_A(tp[3], a, ba) |
#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 4;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_37(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
#if ASIZE
		TBLEND_A_D(tp[3], a, ba, ACoordToUByte(dither)) |
#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 4;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 1
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_38(unsigned int mask, __GLtri *tr) 
{

#if ASIZE

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
		((tp[0]>>AFRAC) << ASHIFT) |
		(CoordToInt(r) << RSHIFT) |
		(CoordToInt(g) << GSHIFT) |
		(CoordToInt(b) << BSHIFT);

	    /* Step interpolants */

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

#endif

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 1
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_39(unsigned int mask, __GLtri *tr) 
{

#if ASIZE

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLuint ra;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    ra = CoordToInt((tp[0]<<AUB2C) + dither);
#if ASIZE < 8
	    ra -= (ra >> ASIZE);
#endif
	    result =
		(ra << ASHIFT) |
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);

	    /* Step interpolants */

	    tp += 1;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

#endif

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 1
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_3A(unsigned int mask, __GLtri *tr) 
{

#if ASIZE

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
		((tp[0]>>AFRAC) << ASHIFT) |
		(CoordToInt(r) << RSHIFT) |
		(CoordToInt(g) << GSHIFT) |
		(CoordToInt(b) << BSHIFT);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

#endif

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 1
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_3B(unsigned int mask, __GLtri *tr) 
{

#if ASIZE

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLuint ra;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    ra = CoordToInt((tp[0]<<AUB2C) + dither);
#if ASIZE < 8
	    ra -= (ra >> ASIZE);
#endif
	    result =
		(ra << ASHIFT) |
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 1;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

#endif

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 1
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_3C(unsigned int mask, __GLtri *tr) 
{

#if ASIZE

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
		MODULATE_A(tp[0], a) |
		(CoordToInt(r) << RSHIFT) |
		(CoordToInt(g) << GSHIFT) |
		(CoordToInt(b) << BSHIFT);

	    /* Step interpolants */

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

#endif

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 1
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_3D(unsigned int mask, __GLtri *tr) 
{

#if ASIZE

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);

	    /* Step interpolants */

	    tp += 1;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

#endif

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 1
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_3E(unsigned int mask, __GLtri *tr) 
{

#if ASIZE

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result =
		MODULATE_A(tp[0], a) |
		(CoordToInt(r) << RSHIFT) |
		(CoordToInt(g) << GSHIFT) |
		(CoordToInt(b) << BSHIFT);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	fp += tr->dx;
	mask <<= 1;
    }

#endif

}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 1
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenPixelsRGBa_3F(unsigned int mask, __GLtri *tr) 
{

#if ASIZE

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#if ASIZE
    unsigned int a = tr->a;
#endif /* ASIZE */

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result =
		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#if ASIZE
	    a += tr->dadx;
#endif /* ASIZE */

	    tp += 1;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#if ASIZE
	a += tr->dadx;
#endif /* ASIZE */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

#endif

}
/*
 *
 */
void (__fastcall *__fr_rgb_pix_a[64])(unsigned int mask, __GLtri *tr) =
{
    __glGenPixelsRGBa_0,
    __glGenPixelsRGBa_1,
    __glGenPixelsRGBa_2,
    __glGenPixelsRGBa_3,
    __glGenPixelsRGBa_4,
    __glGenPixelsRGBa_5,
    __glGenPixelsRGBa_6,
    __glGenPixelsRGBa_7,
    0,
    0,
    0,
    0,
    __glGenPixelsRGBa_C,
    __glGenPixelsRGBa_D,
    __glGenPixelsRGBa_E,
    __glGenPixelsRGBa_F,
    __glGenPixelsRGBa_10,
    __glGenPixelsRGBa_11,
    __glGenPixelsRGBa_12,
    __glGenPixelsRGBa_13,
    __glGenPixelsRGBa_14,
    __glGenPixelsRGBa_15,
    __glGenPixelsRGBa_16,
    __glGenPixelsRGBa_17,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    __glGenPixelsRGBa_20,
    __glGenPixelsRGBa_21,
    __glGenPixelsRGBa_22,
    __glGenPixelsRGBa_23,
    __glGenPixelsRGBa_24,
    __glGenPixelsRGBa_25,
    __glGenPixelsRGBa_26,
    __glGenPixelsRGBa_27,
    __glGenPixelsRGBa_28,
    __glGenPixelsRGBa_29,
    __glGenPixelsRGBa_2A,
    __glGenPixelsRGBa_2B,
    __glGenPixelsRGBa_2C,
    __glGenPixelsRGBa_2D,
    __glGenPixelsRGBa_2E,
    __glGenPixelsRGBa_2F,
    __glGenPixelsRGBa_30,
    __glGenPixelsRGBa_31,
    __glGenPixelsRGBa_32,
    __glGenPixelsRGBa_33,
    __glGenPixelsRGBa_34,
    __glGenPixelsRGBa_35,
    __glGenPixelsRGBa_36,
    __glGenPixelsRGBa_37,
    __glGenPixelsRGBa_38,
    __glGenPixelsRGBa_39,
    __glGenPixelsRGBa_3A,
    __glGenPixelsRGBa_3B,
    __glGenPixelsRGBa_3C,
    __glGenPixelsRGBa_3D,
    __glGenPixelsRGBa_3E,
    __glGenPixelsRGBa_3F,
};

#endif /* __GL_PC_RAST */

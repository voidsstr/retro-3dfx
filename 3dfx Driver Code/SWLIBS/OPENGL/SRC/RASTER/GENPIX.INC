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
FR_GENPIXELS_PROTOTYPE
{
#if FR_REPLACE_ALPHA || FR_MODULATE_ALPHA
\#if ASIZE
#endif
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;
#if !FR_REPLACE
#if FR_CI_COLOR
    unsigned int c = tr->r;
#else /* FR_CI_COLOR */
    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
\#if ASIZE
    unsigned int a = tr->a;
\#endif /* ASIZE */
#endif /* FR_CI_COLOR */
#endif /* !FR_REPLACE */
#if FR_RGB_COLOR && FR_REPLACE && FR_DITHER
    GLuint rr, rg, rb;
\#if ASIZE
    GLuint ra;
\#endif /* ASIZE */
#endif /* FR_RGB_COLOR && FR_REPLACE && FR_DITHER */
#if FR_REPLACE_ALPHA && FR_DITHER
    GLuint ra;
#endif
#if FR_TEXTURING
    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;
#if FR_BLEND | FR_BLEND_INTENSITY
    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];
#endif
#if FR_BLEND_INTENSITY
    GLuint ba = gc->texture.envColor[3];
#endif

    (*gc->procs.extractTexels)(mask, tr);
#endif /* FR_TEXTURING */

    while (1) {
	while (((int)mask) < 0) {
#if (FR_FLAT_SHADING | FR_SMOOTH_SHADING)
#if FR_DITHER
#if !(FR_CI_COLOR && FR_REPLACE)
	    /* Dithered */
	    int dither = *tr->dither;
#endif /* !(FR_CI_COLOR && FR_REPLACE) */
#if FR_CI_COLOR
#if FR_REPLACE
	    result = *tp;
#elif FR_ADD
	    result = *tp + CoordToInt(c + dither);
#else /* TEXENV */
	    result = CoordToInt(c + dither);
#endif /* TEXENV */
#elif FR_RGB_COLOR
#if FR_MODULATE
	    result =
\#if ASIZE
		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |
\#endif /* ASIZE */
		MODULATE_R_D(tp[0], r, RCoordToUByte(dither)) |
		MODULATE_G_D(tp[1], g, GCoordToUByte(dither)) |
		MODULATE_B_D(tp[2], b, BCoordToUByte(dither));
#elif FR_DECAL
	    result =
\#if ASIZE
		(CoordToInt(a + dither) << ASHIFT) |
\#endif /* ASIZE */
		DECAL_R_D(tp[0], tp[3], r, RCoordToUByte(dither)) |
		DECAL_G_D(tp[1], tp[3], g, GCoordToUByte(dither)) |
		DECAL_B_D(tp[2], tp[3], b, BCoordToUByte(dither));
#elif FR_REPLACE
	    rr = CoordToInt((tp[0]<<RUB2C) + dither);
\#if RSIZE < 8
	    rr -= (rr >> RSIZE);
\#endif
	    rg = CoordToInt((tp[1]<<GUB2C) + dither);
\#if GSIZE < 8
	    rg -= (rg >> GSIZE);
\#endif
	    rb = CoordToInt((tp[2]<<BUB2C) + dither);
\#if BSIZE < 8
	    rb -= (rb >> BSIZE);
\#endif
\#if ASIZE
	    ra = CoordToInt((tp[3]<<AUB2C) + dither);
\#if ASIZE < 8
	    ra -= (ra >> ASIZE);
\#endif
\#endif /* ASIZE */
	    result =
\#if ASIZE
		(ra << ASHIFT) |
\#endif /* ASIZE */
		(rr << RSHIFT) |
		(rg << GSHIFT) |
		(rb << BSHIFT);
#elif FR_BLEND
	    result =
\#if ASIZE
#if FR_TEXALPHA
		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |
#else
		(CoordToInt(a + dither) << ASHIFT) |
#endif
\#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));
#elif FR_BLEND_INTENSITY
	    result =
\#if ASIZE
		TBLEND_A_D(tp[3], a, ba, ACoordToUByte(dither)) |
\#endif /* ASIZE */
		TBLEND_R_D(tp[0], r, br, RCoordToUByte(dither)) |
		TBLEND_G_D(tp[1], g, bg, GCoordToUByte(dither)) |
		TBLEND_B_D(tp[2], b, bb, BCoordToUByte(dither));
#elif FR_REPLACE_ALPHA
	    ra = CoordToInt((tp[0]<<AUB2C) + dither);
\#if ASIZE < 8
	    ra -= (ra >> ASIZE);
\#endif
	    result =
		(ra << ASHIFT) |
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);
#elif FR_MODULATE_ALPHA
	    result =
		MODULATE_A_D(tp[3], a, ACoordToUByte(dither)) |
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);
#else /* TEXENV */
	    result =
\#if ASIZE
		(CoordToInt(a + dither) << ASHIFT) |
\#endif /* ASIZE */
		(CoordToInt(r + dither) << RSHIFT) |
		(CoordToInt(g + dither) << GSHIFT) |
		(CoordToInt(b + dither) << BSHIFT);
#endif /* TEXENV */
#endif /* FR_COLOR */
#else /* FR_DITHER */
	/* No dithering */
#if FR_FLAT_SHADING && !FR_TEXTURING
	    result = tr->r0;
#else
#if FR_CI_COLOR
#if FR_REPLACE
	    result = *tp;
#elif FR_ADD
	    result = *tp + CoordToInt(c);
#else
	    result = CoordToInt(c);
#endif
#elif FR_RGB_COLOR
#if FR_MODULATE
	    result =
\#if ASIZE
		MODULATE_A(tp[3], a) |
\#endif /* ASIZE */
		MODULATE_R(tp[0], r) |
		MODULATE_G(tp[1], g) |
		MODULATE_B(tp[2], b);
#elif FR_DECAL
	    result =
\#if ASIZE
		(CoordToInt(a) << ASHIFT) |
\#endif /* ASIZE */
		DECAL_R(tp[0], tp[3], r) |
		DECAL_G(tp[1], tp[3], g) |
		DECAL_B(tp[2], tp[3], b);
#elif FR_REPLACE
	    /* Eventually to be replaced by pixel-format texels */
	    result =
\#if ASIZE
		((tp[3]>>AFRAC) << ASHIFT) |
\#endif /* ASIZE */
		((tp[0]>>RFRAC) << RSHIFT) |
		((tp[1]>>GFRAC) << GSHIFT) |
		((tp[2]>>BFRAC) << BSHIFT);
#elif FR_BLEND
	    result =
\#if ASIZE
#if FR_TEXALPHA
		MODULATE_A(tp[3], a) |
#else
		(CoordToInt(a) << ASHIFT) |
#endif
\#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);
#elif FR_BLEND_INTENSITY
	    result =
\#if ASIZE
		TBLEND_A(tp[3], a, ba) |
\#endif /* ASIZE */
		TBLEND_R(tp[0], r, br) |
		TBLEND_G(tp[1], g, bg) |
		TBLEND_B(tp[2], b, bb);
#elif FR_REPLACE_ALPHA
	    result =
		((tp[0]>>AFRAC) << ASHIFT) |
		(CoordToInt(r) << RSHIFT) |
		(CoordToInt(g) << GSHIFT) |
		(CoordToInt(b) << BSHIFT);
#elif FR_MODULATE_ALPHA
	    result =
		MODULATE_A(tp[0], a) |
		(CoordToInt(r) << RSHIFT) |
		(CoordToInt(g) << GSHIFT) |
		(CoordToInt(b) << BSHIFT);
#else /* TEXENV */
	    result =
\#if ASIZE
		(CoordToInt(a) << ASHIFT) |
\#endif /* ASIZE */
		(CoordToInt(r) << RSHIFT) |
		(CoordToInt(g) << GSHIFT) |
		(CoordToInt(b) << BSHIFT);
#endif /* TEXTURE */
#endif /* COLOR */
#endif /* FR_FLAT_SHADING */
#endif /* FR_DITHER */
#endif /* FR_FLAT_SHADING | FR_SMOOTH_SHADING */

	    /* Step interpolants */
#if FR_SMOOTH_SHADING && !FR_REPLACE
#if FR_CI_COLOR
	    c += tr->drdx;
#elif FR_RGB_COLOR
	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
\#if ASIZE
	    a += tr->dadx;
\#endif /* ASIZE */
#endif
#endif
#if FR_TEXTURING
#if FR_CI_COLOR || FR_REPLACE_ALPHA || FR_MODULATE_ALPHA
	    tp += 1;
#elif !FR_TEXALPHA
	    tp += 3;
#else /* FR_TEXALPHA */
	    tp += 4;
#endif
#endif
#if FR_DITHER
#if !(FR_CI_COLOR && FR_REPLACE)
	    tr->dither++;
#endif /* !(FR_CI_COLOR && FR_REPLACE) */
#endif
	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */
#if FR_SMOOTH_SHADING && !FR_REPLACE
#if FR_CI_COLOR
	c += tr->drdx;
#elif FR_RGB_COLOR
	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
\#if ASIZE
	a += tr->dadx;
\#endif /* ASIZE */
#endif
#endif
#if FR_DITHER
#if !(FR_CI_COLOR && FR_REPLACE)
	tr->dither++;
#endif /* !(FR_CI_COLOR && FR_REPLACE) */
#endif
	fp += tr->dx;
	mask <<= 1;
    }
#if FR_REPLACE_ALPHA || FR_MODULATE_ALPHA
\#endif
#endif
}

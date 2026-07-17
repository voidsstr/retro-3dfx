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
FR_GENCOLOR_PROTOTYPE
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;
#if FR_CI_COLOR
    GLuint *cp = (GLuint *) tr->colorBuf;
#else
    GLubyte *cp = (GLubyte *) tr->colorBuf;
#endif
#if !FR_REPLACE
#if FR_CI_COLOR
    unsigned int c = tr->r;
#elif FR_RGB_COLOR || FR_REPLACE_ALPHA
    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
#elif FR_RGBA_COLOR
    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;
#endif /* COLOR */
#elif FR_RGBA_COLOR && FR_TEXTURING && !FR_TEXALPHA
    unsigned int a = tr->a;
#endif /* !FR_REPLACE*/
#if FR_TEXTURING
    GLubyte *tp = (GLubyte *) tr->texelBuf;
#if FR_BLEND | FR_BLEND_INTENSITY
    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];
#endif
#if FR_BLEND_INTENSITY
    GLuint ba = gc->texture.envColor[3];
#endif

    (*gc->procs.extractTexels)(mask, tr);
#endif

    while (1) {
	while (((int)mask) < 0) {
#if FR_CI_COLOR
#if FR_REPLACE
	    cp[0] = *tp << COLOR_FRAC_BITS;
#elif FR_ADD
	    cp[0] = (*tp << COLOR_FRAC_BITS) + c;
#else
	    cp[0] = c;
#endif
#elif FR_RGB_COLOR
#if FR_MODULATE
	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);
#elif FR_DECAL
#if FR_TEXALPHA
	    cp[0] = RCLAMP_UB(DECAL_R_UB(tp[0], tp[3], r));
	    cp[1] = GCLAMP_UB(DECAL_G_UB(tp[1], tp[3], g));
	    cp[2] = BCLAMP_UB(DECAL_B_UB(tp[2], tp[3], b));
#else
	    cp[0] = tp[0];
	    cp[1] = tp[1];
	    cp[2] = tp[2];
#endif
#elif FR_REPLACE
	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);
#elif FR_BLEND
	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);
#else
	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
#endif
#elif FR_RGBA_COLOR
#if FR_MODULATE
	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);
#if FR_TEXALPHA
	    cp[3] = MODULATE_A_UB(tp[3], a);
#else
	    cp[3] = ACoordToUByte(a);
#endif
#elif FR_DECAL
#if FR_TEXALPHA
	    cp[0] = RCLAMP_UB(DECAL_R_UB(tp[0], tp[3], r));
	    cp[1] = GCLAMP_UB(DECAL_G_UB(tp[1], tp[3], g));
	    cp[2] = BCLAMP_UB(DECAL_B_UB(tp[2], tp[3], b));
	    cp[3] = ACoordToUByte(a);
#else
	    cp[0] = tp[0];
	    cp[1] = tp[1];
	    cp[2] = tp[2];
	    cp[3] = ACoordToUByte(a);
#endif
#elif FR_REPLACE
	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);
#if FR_TEXALPHA
	    cp[3] = ACLAMP_UB(tp[3]);
#else
	    cp[3] = ACoordToUByte(a);
#endif
#elif FR_BLEND
	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);
#if FR_TEXALPHA
	    cp[3] = MODULATE_A_UB(tp[3], a);
#else
	    cp[3] = (CoordToInt(a) << ASHIFT);
#endif
#elif FR_BLEND_INTENSITY
	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);
	    cp[3] = TBLEND_A_UB(tp[3], a, ba);
#elif FR_REPLACE_ALPHA
	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
	    cp[3] = ACLAMP_UB(tp[3]);
#elif FR_MODULATE_ALPHA
	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
	    cp[3] = MODULATE_A_UB(tp[0], a);
#else
	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
	    cp[3] = ACoordToUByte(a);
#endif /* TEXTURE */
#endif /* COLOR */

	    /* Step interpolants */
#if FR_SMOOTH_SHADING
#if !FR_REPLACE
#if FR_CI_COLOR
	    c += tr->drdx;
#elif FR_RGB_COLOR || FR_REPLACE_ALPHA
	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
#elif FR_RGBA_COLOR
	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;
#endif /* COLOR */
#elif FR_TEXTURING && FR_RGBA_COLOR && !FR_TEXALPHA
	    a += tr->dadx;
#endif /* !FR_REPLACE */
#endif /* FR_SMOOTH_SHADING */
#if FR_TEXTURING
#if FR_CI_COLOR || FR_REPLACE_ALPHA || FR_MODULATE_ALPHA
	    tp += 1;
#elif !FR_TEXALPHA
	    tp += 3;
#else /* FR_TEXALPHA */
	    tp += 4;
#endif
#endif
#if FR_CI_COLOR
	    cp += 1;
#elif FR_RGB_COLOR || FR_RGBA_COLOR
	    cp += 4;
#endif
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */
#if FR_SMOOTH_SHADING
#if !FR_REPLACE
#if FR_CI_COLOR
	c += tr->drdx;
#elif FR_RGB_COLOR || FR_REPLACE_ALPHA
	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
#elif FR_RGBA_COLOR
	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;
#endif /* COLOR */
#elif FR_TEXTURING && FR_RGBA_COLOR && !FR_TEXALPHA
	a += tr->dadx;
#endif /* !FR_REPLACE */
#endif /* FR_SMOOTH_SHADING */
#if FR_CI_COLOR
	cp += 1;
#elif FR_RGB_COLOR || FR_RGBA_COLOR
	cp += 4;
#endif
	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

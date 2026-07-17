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
FR_COMBINE_PROTOTYPE
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
#if FR_NEED_SRC
#if FR_RGB_MODE
    GLubyte *cp = tr->colorBuf;
#else
    GLuint *cp = (GLuint *) tr->colorBuf;
#endif
#if FR_RGB_MODE
    GLuint sr, sg, sb;
\#if ASIZE
    GLuint sa;
\#endif /* ASIZE */
#if FR_BLEND
    GLuint sf;
#endif /* FR_BLEND */
#endif /* FR_RGB_MODE */
#endif /* FR_NEED_SRC */
    GLuint result;
#if FR_NEED_DST
    GLuint fbcolor;
#endif /* FR_NEED_DST */
#if FR_DST_COMPS
    GLuint dr, dg, db;
\#if ASIZE
    GLuint da;
\#endif /* ASIZE */
#endif /* FR_DST_COMPS */
#if FR_BLEND
#if (FR_DST_BLEND_FUNC == __FR_BLEND_DST_SA) || \
    (FR_DST_BLEND_FUNC == __FR_BLEND_DST_MSA)
    GLuint df;
#endif /* FR_DST_BLEND_FUNC */
    __GLcontext *gc = tr->gc;

#endif /* FR_BLEND */

    while (1) {
	while (((int)mask) < 0) {
#if FR_DITHER && FR_NEED_SRC
	    int dither = *tr->dither;
#endif /* FR_DITHER */
#if FR_NEED_DST
	    fbcolor = *fp;
#if FR_DST_COMPS
	    dr = ((fbcolor & RMASK) >> RSHIFT) << RFRAC;
	    dg = ((fbcolor & GMASK) >> GSHIFT) << GFRAC;
	    db = ((fbcolor & BMASK) >> BSHIFT) << BFRAC;
\#if ASIZE
	    da = ((fbcolor & AMASK) >> ASHIFT) << AFRAC;
\#endif /* ASIZE */
#endif /* FR_DST_COMPS */
#endif /* FR_NEED_DST */

#if FR_NEED_SRC
#if FR_BLEND
#if FR_SRC_BLEND_FUNC == __FR_BLEND_SRC_SA
	    sf = cp[3];
#elif FR_SRC_BLEND_FUNC == __FR_BLEND_SRC_MSA
	    sf = 255-cp[3];
#elif FR_SRC_BLEND_FUNC == __FR_BLEND_SRC_SAS
\#if ASIZE
	    sf = cp[3] < (255-da) ? cp[3] : (255-da);
\#else /* ASIZE */
	    sf = 0;
\#endif /* ASIZE */
#endif /* FR_SRC_BLEND_FUNC */

#if FR_DITHER
	    sr = BLEND(sf, cp[0]) + (dither >> RUB2C);
	    sg = BLEND(sf, cp[1]) + (dither >> GUB2C);
	    sb = BLEND(sf, cp[2]) + (dither >> BUB2C);
\#if ASIZE
	    sa = BLEND(sf, cp[3]) + (dither >> AUB2C);
\#endif /* ASIZE */
#else
	    sr = BLEND(sf, cp[0]);
	    sg = BLEND(sf, cp[1]);
	    sb = BLEND(sf, cp[2]);
\#if ASIZE
	    sa = BLEND(sf, cp[3]);
\#endif /* ASIZE */
#endif

#if FR_DST_BLEND_FUNC == __FR_BLEND_DST_SA
	    df = cp[3];
	    dr = BLEND(df, dr);
	    dg = BLEND(df, dg);
	    db = BLEND(df, db);
\#if ASIZE
	    da = BLEND(df, da);
\#endif /* ASIZE */
#elif FR_DST_BLEND_FUNC == __FR_BLEND_DST_MSA
	    df = 255-cp[3];
	    dr = BLEND(df, dr);
	    dg = BLEND(df, dg);
	    db = BLEND(df, db);
\#if ASIZE
	    da = BLEND(df, da);
\#endif /* ASIZE */
#endif /* FR_DST_BLEND_FUNC */

#else /* FR_BLEND */

#if FR_DITHER
#if FR_RGB_MODE
	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
\#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
\#endif /* ASIZE */
#else /* FR_RGB_MODE */
	    result = CoordToInt(*cp + dither);
#endif /* FR_RGB_MODE */
#else /* FR_DITHER */
#if FR_RGB_MODE
	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
\#if ASIZE
	    sa = cp[3];
\#endif /* ASIZE */
#else /* FR_RGB_MODE */
	    result = CoordToInt(*cp);
#endif /* FR_RGB_MODE */
#endif /* FR_DITHER */
#endif /* FR_BLEND */

#if FR_DST_COMPS
	    dr = (dr + sr) >> RFRAC;
	    dg = (dg + sg) >> GFRAC;
	    db = (db + sb) >> BFRAC;
\#if ASIZE
	    da = (da + sa) >> AFRAC;
\#endif /* ASIZE */

#if FR_CLAMP
	    if (dr > MAX_R) dr = MAX_R;
	    if (dg > MAX_G) dg = MAX_G;
	    if (db > MAX_B) db = MAX_B;
\#if ASIZE
	    if (da > MAX_A) da = MAX_A;
\#endif /* ASIZE */
#endif /* FR_CLAMP */

\#if ASIZE
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT) |
		(da << ASHIFT);
\#else /* ASIZE */
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT);
\#endif /* ASIZE */
#else /* FR_DST_COMPS */
#if FR_RGB_MODE
\#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
\#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
\#endif /* ASIZE */
#endif /* FR_RGB_MODE */
#endif /* FR_DST_COMPS */
#endif /* FR_NEED_SRC */

#if FR_LOGICOP == __FR_LO_CLEAR
	    result = 0;
#elif FR_LOGICOP == __FR_LO_AND
	    result = result & fbcolor;
#elif FR_LOGICOP == __FR_LO_AND_REVERSE
	    result = result & (~fbcolor);
#elif FR_LOGICOP == __FR_LO_COPY
	    result = result;
#elif FR_LOGICOP == __FR_LO_AND_INVERTED
	    result = (~result) & fbcolor;
#elif FR_LOGICOP == __FR_LO_NOOP
	    result = fbcolor;
#elif FR_LOGICOP == __FR_LO_XOR
	    result = result ^ fbcolor;
#elif FR_LOGICOP == __FR_LO_OR
	    result = result | fbcolor;
#elif FR_LOGICOP == __FR_LO_NOR
	    result = ~(result | fbcolor);
#elif FR_LOGICOP == __FR_LO_EQUIV
	    result = ~(result ^ fbcolor);
#elif FR_LOGICOP == __FR_LO_INVERT
	    result = ~fbcolor;
#elif FR_LOGICOP == __FR_LO_OR_REVERSE
	    result = result | (~fbcolor);
#elif FR_LOGICOP == __FR_LO_COPY_INVERTED
	    result = ~result;
#elif FR_LOGICOP == __FR_LO_OR_INVERTED
	    result = (~result) | fbcolor;
#elif FR_LOGICOP == __FR_LO_NAND
	    result = ~(result & fbcolor);
#elif FR_LOGICOP == __FR_LO_SET
	    result = ~0;
#endif

	    *fp = result;

#if FR_DITHER && FR_NEED_SRC
	    tr->dither++;
#endif /* FR_DITHER */
	    mask <<= 1;
	    fp += tr->dx;
#if FR_NEED_SRC
#if FR_RGB_MODE
	    cp += 4;
#else /* FR_RGB_MODE */
	    cp += 1;
#endif /* FR_RGB_MODE */
#endif /* FR_NEED_SRC */
	}

	if (mask == 0) break;

#if FR_DITHER && FR_NEED_SRC
	tr->dither++;
#endif /* FR_DITHER */
	mask <<= 1;
	fp += tr->dx;
#if FR_NEED_SRC
#if FR_RGB_MODE
	cp += 4;
#else /* FR_RGB_MODE */
	cp += 1;
#endif /* FR_RGB_MODE */
#endif /* FR_NEED_SRC */

    }
}

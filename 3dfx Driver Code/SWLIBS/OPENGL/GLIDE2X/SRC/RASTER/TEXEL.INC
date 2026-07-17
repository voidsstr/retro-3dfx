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
FR_EXTRACT_TEXELS_PROTOTYPE
{
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    GLubyte *tbase;
    unsigned int s, t;
#if FR_TEX_RGBMODE
    GLuint offset;
    GLubyte *tp;
#endif
#if FR_TEX_PERSPECTIVE
    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float next_s, next_t, ds, dt;
    double dtmp;
    GLbitfield mask2;
#endif

#if FR_TEX_TABLE
    __GLcontext *gc = tr->gc;
    /* Move this to fr_data in GC at pick time? */
    tbase = gc->texture.currentTexture->CT.table;
#else
    tbase = ((GLubyte*)tr->tp);
#endif

#if !FR_TEX_PERSPECTIVE
    s = tr->s;
    t = tr->t;
#endif

#if FR_TEX_PERSPECTIVE && (__FR_TEXEL_PWL_SPAN_WIDTH < 32)
    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {
#endif

#if FR_TEX_PERSPECTIVE
	qw += tr->fdqwdxPWL;
\#ifdef PWL_OVERLAP
	/* begin the next FDIV so that it overlaps */
	__asm fld  qw
	__asm fld1
	__asm fdivrp st(1), st
\#else
	invqw = 1.0F / qw;
\#endif /* PWL_OVERLAP */
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;

\#ifdef PWL_OVERLAP
	/* fetch invqw from FPU */
	__asm fstp invqw
\#endif /* PWL_OVERLAP */

	/* compute next texture parameters */
	next_s = sw * invqw;
	next_t = tw * invqw;

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
#endif /* FR_TEX_PERSPECTIVE */

	while (((int)mask) < 0) {
#if !FR_TEX_RGBMODE
	    *texelBuf++ = tbase[ComputeTextureOffset(s, t)];
#elif FR_TEX_TABLE
	    offset = ((GLubyte*)tr->tp)[ComputeTextureOffset(s, t)];
#else
	    offset = ComputeTextureOffset(s, t);
#endif

#if FR_TEX_LUMINANCE
	    tp = tbase + offset;
	    texelBuf[0] = tp[0];
	    texelBuf[1] = tp[0];
	    texelBuf[2] = tp[0];
#elif FR_TEX_LUMINANCE_ALPHA
	    tp = tbase + (offset<<1);
	    texelBuf[0] = tp[0];
	    texelBuf[1] = tp[0];
	    texelBuf[2] = tp[0];
#if FR_TEX_NEED_ALPHA
	    texelBuf[3] = tp[1];
#endif
#elif FR_TEX_RGB
	    tp = tbase + (offset<<1) + offset;
	    texelBuf[0] = tp[0];
	    texelBuf[1] = tp[1];
	    texelBuf[2] = tp[2];
#elif FR_TEX_RGBA
	    tp = tbase + (offset<<2);
	    texelBuf[0] = tp[0];
	    texelBuf[1] = tp[1];
	    texelBuf[2] = tp[2];
#if FR_TEX_NEED_ALPHA
	    texelBuf[3] = tp[3];
#endif
#endif /* FR_TEX_FORMAT */

#if FR_TEX_RGBMODE
#if !FR_TEX_NEED_ALPHA
	    texelBuf += 3;
#else
	    texelBuf += 4;
#endif
#endif
	    s += tr->dsdx;
	    t += tr->dtdx;
	    mask <<= 1;
	}

	while (mask) {

	    s += tr->dsdx;
	    t += tr->dtdx;
	    mask <<= 1;

	    if (((int)mask) < 0) {

#if !FR_TEX_RGBMODE
		*texelBuf++ = tbase[ComputeTextureOffset(s, t)];
#elif FR_TEX_TABLE
		offset = ((GLubyte*)tr->tp)[ComputeTextureOffset(s, t)];
#else
		offset = ComputeTextureOffset(s, t);
#endif

#if FR_TEX_LUMINANCE
		tp = tbase + offset;
		texelBuf[0] = tp[0];
		texelBuf[1] = tp[0];
		texelBuf[2] = tp[0];
#elif FR_TEX_LUMINANCE_ALPHA
		tp = tbase + (offset<<1);
		texelBuf[0] = tp[0];
		texelBuf[1] = tp[0];
		texelBuf[2] = tp[0];
#if FR_TEX_NEED_ALPHA
		texelBuf[3] = tp[1];
#endif
#elif FR_TEX_RGB
		tp = tbase + (offset<<1) + offset;
		texelBuf[0] = tp[0];
		texelBuf[1] = tp[1];
		texelBuf[2] = tp[2];
#elif FR_TEX_RGBA
		tp = tbase + (offset<<2);
		texelBuf[0] = tp[0];
		texelBuf[1] = tp[1];
		texelBuf[2] = tp[2];
#if FR_TEX_NEED_ALPHA
		texelBuf[3] = tp[3];
#endif
#endif /* FR_TEX_FORMAT */
#if FR_TEX_RGBMODE
#if !FR_TEX_NEED_ALPHA
		texelBuf += 3;
#else
		texelBuf += 4;
#endif
#endif
	    }
	}

#if FR_TEX_PERSPECTIVE && (__FR_TEXEL_PWL_SPAN_WIDTH < 32)
	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
    }
#endif
}

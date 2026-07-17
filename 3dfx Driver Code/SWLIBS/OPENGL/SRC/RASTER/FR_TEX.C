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
#include "imports.h"
#include "global.h"
#include "g_imfncs.h"
#include "types.h"
#include "namesint.h"
#include "pixel.h"
#include "image.h"
#include "glmath.h"
#include <memory.h>
#include "fr_tri.h"
#include "fr_tex.h"
#include "fr_modes.h"

/* Fixed point contants */
#define STRQ_ONE  (1<<STRQ_FRAC_BITS)
#define STRQ_HALF (STRQ_ONE>>1)

/* Smallest float for which 1/f is representable */
/*#define MIN_FLOAT (5.87747e-039)*/
#define MIN_FLOAT_HEX (0x003fffff)

#define ExtractBits(f)		(*(int*)&f)
#define ExtractExponent(f)	(((ExtractBits(f)&0x7f800000)>>23)-127)
#define ExtractMantissa(f)	(ExtractBits(f)&0x007fffff)

#define ExtractLOD(rho) ((ExtractExponent(rho)<<(STRQ_FRAC_BITS-1))|\
			 (ExtractMantissa(rho)>>(23-STRQ_FRAC_BITS+1)))

/* For now, we force perspective correction if needRho */
#define AFFINE_RHO 0

/**************************************************************************
 * Filter procs
 **************************************************************************/

/* When these are called directly for non-mipmapped cases, lod is zero.
 * When called from a mipmap filter, lod is an integer.
 */

void __glFRNearestFilterUVScaled(__GLtexture *tex, GLuint lod,
				 GLint fs, GLint ft, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp = tex->level[lod];
    GLint row = TruncFixed(ft, STRQ_FRAC_BITS);
    GLint col = TruncFixed(fs, STRQ_FRAC_BITS);
    GLint w2 = lp->width2;
    GLint h2 = lp->height2;

    /* Find texel column address */
    if (tex->params.sWrapMode == GL_REPEAT) {
	col &= w2-1;
    } else {
	if (col < 0) col = 0;
	else if (col >= w2) col = w2 - 1;
    }

    /* Find texel row address */
    if (tex->params.tWrapMode == GL_REPEAT) {
	row &= h2-1;
    } else {
	if (row < 0) row = 0;
	else if (row >= h2) row = h2 - 1;
    }

    /* Lookup texel */
    (*lp->extract)(lp, tex, 0, row, col, result);
}


void __glFRLinearFilterUVScaled(__GLtexture *tex, GLuint lod,
				GLint fs, GLint ft, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp = tex->level[lod];
    GLuint half, alpha, beta;
    GLuint col0, row0, col1, row1;
    GLint w2mask, h2mask;
    GLuint omalpha, ombeta, m00, m01, m10, m11;
    __GLtexel t00, t01, t10, t11;
    GLuint r, g, b, a;

    half = STRQ_HALF;

    /* Find col0, col1 */
    if (tex->params.sWrapMode == GL_REPEAT) {
	w2mask = lp->width2 - 1;
	fs -= half;
	col0 = TruncFixed(fs, STRQ_FRAC_BITS) & w2mask;
	col1 = (col0 + 1) & w2mask;
    } else {
	if (fs < 0)
	    fs = 0;
	else if (fs > (lp->width2<<STRQ_FRAC_BITS))
	    fs = (lp->width2<<STRQ_FRAC_BITS);
	fs -= half;
	col0 = TruncFixed(fs, STRQ_FRAC_BITS);
	col1 = col0 + 1;
    }

    /* Find row0, row1 */
    if (tex->params.tWrapMode == GL_REPEAT) {
	h2mask = lp->height2 - 1;
	ft -= half;
	row0 = TruncFixed(ft, STRQ_FRAC_BITS) & h2mask;
	row1 = (row0 + 1) & h2mask;
    } else {
	if (ft < 0)
	    ft = 0;
	else if (ft > (lp->height2<<STRQ_FRAC_BITS))
	    ft = (lp->height2<<STRQ_FRAC_BITS);
	ft -= half;
	row0 = TruncFixed(ft, STRQ_FRAC_BITS);
	row1 = row0 + 1;
    }

    /* Compute alpha and beta */
    alpha = ((fs & 0xffff) >> 8);
    beta  = ((ft & 0xffff) >> 8);

    /* Calculate the final texel value as a combination of the square chosen */
    (*lp->extract)(lp, tex, 0, row0, col0, &t00);
    (*lp->extract)(lp, tex, 0, row0, col1, &t10);
    (*lp->extract)(lp, tex, 0, row1, col0, &t01);
    (*lp->extract)(lp, tex, 0, row1, col1, &t11);

    omalpha = 255 - alpha;
    ombeta  = 255 - beta;

    m00 = BLEND(omalpha, ombeta);
    m01 = BLEND(omalpha, beta);
    m10 = BLEND(alpha, ombeta);
    m11 = BLEND(alpha, beta);

    /* Why the interleaving?  I'm trying to minimize cache thrashing. */
    a = BLEND(m00,t00.a);
    r = BLEND(m00,t00.r);
    g = BLEND(m00,t00.g);
    b = BLEND(m00,t00.b);

    a += BLEND(m01,t01.a);
    r += BLEND(m01,t01.r);
    g += BLEND(m01,t01.g);
    b += BLEND(m01,t01.b);

    a += BLEND(m10,t10.a);
    r += BLEND(m10,t10.r);
    g += BLEND(m10,t10.g);
    b += BLEND(m10,t10.b);

    a += BLEND(m11,t11.a);
    r += BLEND(m11,t11.r);
    g += BLEND(m11,t11.g);
    b += BLEND(m11,t11.b);

    result->r = r;
    result->g = g;
    result->b = b;
    result->a = a;
}

/* Magnification filters: lod is always zero */

void __glFRNearestFilter(__GLtexture *tex, GLuint lod,
			 GLint fs, GLint ft, __GLtexel *result)
{
    __GLmipMapLevel *lp = tex->level[0];

    assert(0 == lod);

    fs <<= lp->widthLog2;
    ft <<= lp->heightLog2;
    __glFRNearestFilterUVScaled(tex, 0, fs, ft, result);
}

void __glFRLinearFilter(__GLtexture *tex, GLuint lod,
			GLint fs, GLint ft, __GLtexel *result)
{
    __GLmipMapLevel *lp = tex->level[0];

    assert(0 == lod);

    fs <<= lp->widthLog2;
    ft <<= lp->heightLog2;
    __glFRLinearFilterUVScaled(tex, 0, fs, ft, result);
}

/* Minification filters: lod is fixed point */

void __glFR_NMNFilter(__GLtexture *tex, GLuint lod,
		      GLint fs, GLint ft, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLuint d, p;

    d = TruncFixed(lod + STRQ_HALF, STRQ_FRAC_BITS);
    p = (GLuint) tex->p;
    if (d > p) {
	d = p;
    }
    lp = tex->level[d];
    fs <<= lp->widthLog2;
    ft <<= lp->heightLog2;
    __glFRNearestFilterUVScaled(tex, d, fs, ft, result);
}

/* NOTE: lod is fixed point */
void __glFR_LMNFilter(__GLtexture *tex, GLuint lod,
		      GLint fs, GLint ft, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLuint d, p;

    d = TruncFixed(lod + STRQ_HALF, STRQ_FRAC_BITS);
    p = (GLuint) tex->p;
    if (d > p) {
	d = p;
    }
    lp = tex->level[d];
    fs <<= lp->widthLog2;
    ft <<= lp->heightLog2;
    __glFRLinearFilterUVScaled(tex, d, fs, ft, result);
}


void __glFR_NMLFilter(__GLtexture *tex, GLuint lod,
		      GLint fs, GLint ft, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLuint p, d, f;
    __GLtexel td, td1;

    d = TruncFixed(lod + STRQ_ONE, STRQ_FRAC_BITS);
    p = (GLuint) tex->p;
    if (d > p) {
	/* Clamp d to last available mipmap */
	lp = tex->level[p];
	fs <<= lp->widthLog2;
	ft <<= lp->heightLog2;
	__glFRNearestFilterUVScaled(tex, p, fs, ft, result);
    } else {
	GLint s1, t1;

	lp = tex->level[d];
	s1 = fs << lp->widthLog2;
	t1 = ft << lp->heightLog2;
	__glFRNearestFilterUVScaled(tex, d, s1, t1, &td);

	lp = tex->level[d-1];
	s1 = fs << lp->widthLog2;
	t1 = ft << lp->heightLog2;
	__glFRNearestFilterUVScaled(tex, d-1, s1, t1, &td1);

	f = (lod & (STRQ_ONE-1)) >> (STRQ_FRAC_BITS-8);

	result->r = SA_MSA(td.r, f, td1.r);
	result->g = SA_MSA(td.g, f, td1.g);
	result->b = SA_MSA(td.b, f, td1.b);
	result->a = SA_MSA(td.a, f, td1.a);
    }
}

void __glFR_LMLFilter(__GLtexture *tex, GLuint lod,
		      GLint fs, GLint ft, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLuint p, d, f;
    __GLtexel td, td1;

    d = TruncFixed(lod + STRQ_ONE, STRQ_FRAC_BITS);
    p = (GLuint) tex->p;
    if (d > p) {
	/* Clamp d to last available mipmap */
	lp = tex->level[p];
	fs <<= lp->widthLog2;
	ft <<= lp->heightLog2;
	__glFRLinearFilterUVScaled(tex, p, fs, ft, result);
    } else {
	GLint s1, t1;

	lp = tex->level[d];
	s1 = fs << lp->widthLog2;
	t1 = ft << lp->heightLog2;
	__glFRLinearFilterUVScaled(tex, d, s1, t1, &td);

	lp = tex->level[d-1];
	s1 = fs << lp->widthLog2;
	t1 = ft << lp->heightLog2;
	__glFRLinearFilterUVScaled(tex, d-1, s1, t1, &td1);

	f = (lod & (STRQ_ONE-1)) >> (STRQ_FRAC_BITS-8);

	result->r = SA_MSA(td.r, f, td1.r);
	result->g = SA_MSA(td.g, f, td1.g);
	result->b = SA_MSA(td.b, f, td1.b);
	result->a = SA_MSA(td.a, f, td1.a);
    }
}

/**************************************************************************
 * Spanlet procs
 **************************************************************************/

void __fastcall __glGenericExtractTexels_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    GLint s, t;

    s = tr->s;
    t = tr->t;

    while (1) {

	while (((int)mask) < 0) {
	    (*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);

	    texelBuf[0] = texel.r;
	    texelBuf++;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	s += tr->dsdx;
	t += tr->dtdx;
	mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexels_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    GLint s, t;

    s = tr->s;
    t = tr->t;

    while (1) {

	while (((int)mask) < 0) {
	    (*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);

	    texelBuf[0] = texel.r;
	    texelBuf[1] = texel.g;
	    texelBuf[2] = texel.b;
	    texelBuf += 3;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	s += tr->dsdx;
	t += tr->dtdx;
	mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexels_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    GLint s, t;

    s = tr->s;
    t = tr->t;

    while (1) {

	while (((int)mask) < 0) {
	    (*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);

	    texelBuf[0] = texel.r;
	    texelBuf[1] = texel.g;
	    texelBuf[2] = texel.b;
	    texelBuf[3] = texel.a;
	    texelBuf += 4;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	s += tr->dsdx;
	t += tr->dtdx;
	mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsPC_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float next_s, next_t, ds, dt;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

	qw += tr->fdqwdxPWL;
#ifdef PWL_OVERLAP
	/* begin the next FDIV so that it overlaps */
	__asm fld  qw
	__asm fld1
	__asm fdivrp st(1), st
#else
	invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;

#ifdef PWL_OVERLAP
	/* fetch invqw from FPU */
	__asm fstp invqw
#endif /* PWL_OVERLAP */

	/* compute next texture parameters */
	next_s = sw * invqw;
	next_t = tw * invqw;

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);

	while (1) {
	    while (((int)mask) < 0) {
		(*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);

		texelBuf[0] = texel.r;
		texelBuf++;

		s += tr->dsdx;
		t += tr->dtdx;

		mask <<= 1;
	    }

	    if (mask == 0) break;

	    s += tr->dsdx;
	    t += tr->dtdx;

	    mask <<= 1;
	}

	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
    }
}

void __fastcall __glGenericExtractTexelsPC_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float next_s, next_t, ds, dt;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

	qw += tr->fdqwdxPWL;
#ifdef PWL_OVERLAP
	/* begin the next FDIV so that it overlaps */
	__asm fld  qw
	__asm fld1
	__asm fdivrp st(1), st
#else
	invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;

#ifdef PWL_OVERLAP
	/* fetch invqw from FPU */
	__asm fstp invqw
#endif /* PWL_OVERLAP */

	/* compute next texture parameters */
	next_s = sw * invqw;
	next_t = tw * invqw;

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);

	while (1) {
	    while (((int)mask) < 0) {
		(*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);

		texelBuf[0] = texel.r;
		texelBuf[1] = texel.g;
		texelBuf[2] = texel.b;
		texelBuf += 3;

		s += tr->dsdx;
		t += tr->dtdx;

		mask <<= 1;
	    }

	    if (mask == 0) break;

	    s += tr->dsdx;
	    t += tr->dtdx;

	    mask <<= 1;
	}

	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
    }
}

void __fastcall __glGenericExtractTexelsPC_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float next_s, next_t, ds, dt;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

	qw += tr->fdqwdxPWL;
#ifdef PWL_OVERLAP
	/* begin the next FDIV so that it overlaps */
	__asm fld  qw
	__asm fld1
	__asm fdivrp st(1), st
#else
	invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;

#ifdef PWL_OVERLAP
	/* fetch invqw from FPU */
	__asm fstp invqw
#endif /* PWL_OVERLAP */

	/* compute next texture parameters */
	next_s = sw * invqw;
	next_t = tw * invqw;

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);

	while (1) {
	    while (((int)mask) < 0) {
		(*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);

		texelBuf[0] = texel.r;
		texelBuf[1] = texel.g;
		texelBuf[2] = texel.b;
		texelBuf[3] = texel.a;
		texelBuf += 4;

		s += tr->dsdx;
		t += tr->dtdx;

		mask <<= 1;
	    }

	    if (mask == 0) break;

	    s += tr->dsdx;
	    t += tr->dtdx;

	    mask <<= 1;
	}

	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
    }
}

/**************************************************************************
 * Rho-interpolant procs
 **************************************************************************/

#if AFFINE_RHO

void __fastcall __glGenericExtractTexelsRho_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

	while (((int)mask) < 0) {
	    if (rho <= tex->c) {
		(*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
	    } else {
		(*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);
	    }

	    texelBuf[0] = texel.r;
	    texelBuf += 1;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	s += tr->dsdx;
	t += tr->dtdx;
	rho += tr->drhodx;
	mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsRho_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

	while (((int)mask) < 0) {
	    if (rho <= tex->c) {
		(*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
	    } else {
		(*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);
	    }

	    texelBuf[0] = texel.r;
	    texelBuf[1] = texel.g;
	    texelBuf[2] = texel.b;
	    texelBuf += 3;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	s += tr->dsdx;
	t += tr->dtdx;
	rho += tr->drhodx;
	mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsRho_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

	while (((int)mask) < 0) {
	    if (rho <= tex->c) {
		(*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
	    } else {
		(*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);
	    }

	    texelBuf[0] = texel.r;
	    texelBuf[1] = texel.g;
	    texelBuf[2] = texel.b;
	    texelBuf[3] = texel.a;
	    texelBuf += 4;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	s += tr->dsdx;
	t += tr->dtdx;
	rho += tr->drhodx;
	mask <<= 1;
    }
}

#endif /* AFFINE_RHO */

void __fastcall __glGenericExtractTexelsPCRho_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

	qw += tr->fdqwdxPWL;
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;
	rhow += tr->fdrhowdxPWL;

	/* compute next texture parameters */
	if (ExtractBits(qw) < MIN_FLOAT_HEX) {

	    next_s = 0.0f;
	    next_t = 0.0f;
	    next_rho = 0.0f;

	} else {
	    invqw = 1.0F / qw;

	    next_s = sw * invqw;
	    next_t = tw * invqw;
	    next_rho = rhow * invqw;
	}

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
	tr->drhodx = drho;

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
	rho = prev_rho;

	while (1) {
	    while (((int)mask) < 0) {
		if (rho <= tex->c) {
		    (*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
		} else {
		    (*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);
		}

		texelBuf[0] = texel.r;
		texelBuf += 1;

		s += tr->dsdx;
		t += tr->dtdx;
		rho += tr->drhodx;

		mask <<= 1;
	    }

	    if (mask == 0) break;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;

	    mask <<= 1;
	}

	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
	prev_rho = next_rho;
    }
}

void __fastcall __glGenericExtractTexelsPCRho_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

	qw += tr->fdqwdxPWL;
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;
	rhow += tr->fdrhowdxPWL;

	/* compute next texture parameters */
	if (ExtractBits(qw) < MIN_FLOAT_HEX) {

	    next_s = 0.0f;
	    next_t = 0.0f;
	    next_rho = 0.0f;

	} else {
	    invqw = 1.0F / qw;

	    next_s = sw * invqw;
	    next_t = tw * invqw;
	    next_rho = rhow * invqw;
	}

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
	tr->drhodx = drho;

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
	rho = prev_rho;

	while (1) {
	    while (((int)mask) < 0) {
		if (rho <= tex->c) {
		    (*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
		} else {
		    (*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);
		}

		texelBuf[0] = texel.r;
		texelBuf[1] = texel.g;
		texelBuf[2] = texel.b;
		texelBuf += 3;

		s += tr->dsdx;
		t += tr->dtdx;
		rho += tr->drhodx;

		mask <<= 1;
	    }

	    if (mask == 0) break;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;

	    mask <<= 1;
	}

	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
	prev_rho = next_rho;
    }
}

void __fastcall __glGenericExtractTexelsPCRho_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

	qw += tr->fdqwdxPWL;
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;
	rhow += tr->fdrhowdxPWL;

	/* compute next texture parameters */
	if (ExtractBits(qw) < MIN_FLOAT_HEX) {

	    next_s = 0.0f;
	    next_t = 0.0f;
	    next_rho = 0.0f;

	} else {
	    invqw = 1.0F / qw;

	    next_s = sw * invqw;
	    next_t = tw * invqw;
	    next_rho = rhow * invqw;
	}

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
	tr->drhodx = drho;

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
	rho = prev_rho;

	while (1) {
	    while (((int)mask) < 0) {
		if (rho <= tex->c) {
		    (*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
		} else {
		    (*gc->procs.slowMinFilter)(tex, 0, s, t, &texel);
		}

		texelBuf[0] = texel.r;
		texelBuf[1] = texel.g;
		texelBuf[2] = texel.b;
		texelBuf[3] = texel.a;
		texelBuf += 4;

		s += tr->dsdx;
		t += tr->dtdx;
		rho += tr->drhodx;

		mask <<= 1;
	    }

	    if (mask == 0) break;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;

	    mask <<= 1;
	}

	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
	prev_rho = next_rho;
    }
}

/**************************************************************************
 * Mipmap procs
 **************************************************************************/

#if AFFINE_RHO

void __fastcall __glGenericExtractTexelsMipmap_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

	while (((int)mask) < 0) {
	    if (rho <= tex->c) {
		(*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
	    } else {
		GLuint lod = ExtractLOD(rho);
		(*gc->procs.slowMinFilter)(tex, lod, s, t, &texel);
	    }

	    texelBuf[0] = texel.r;
	    texelBuf += 1;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	s += tr->dsdx;
	t += tr->dtdx;
	rho += tr->drhodx;
	mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsMipmap_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

	while (((int)mask) < 0) {
	    if (rho <= tex->c) {
		(*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
	    } else {
		GLuint lod = ExtractLOD(rho);
		(*gc->procs.slowMinFilter)(tex, lod, s, t, &texel);
	    }

	    texelBuf[0] = texel.r;
	    texelBuf[1] = texel.g;
	    texelBuf[2] = texel.b;
	    texelBuf += 3;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	s += tr->dsdx;
	t += tr->dtdx;
	rho += tr->drhodx;
	mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsMipmap_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

	while (((int)mask) < 0) {
	    if (rho <= tex->c) {
		(*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
	    } else {
		GLuint lod = ExtractLOD(rho);
		(*gc->procs.slowMinFilter)(tex, lod, s, t, &texel);
	    }

	    texelBuf[0] = texel.r;
	    texelBuf[1] = texel.g;
	    texelBuf[2] = texel.b;
	    texelBuf[3] = texel.a;
	    texelBuf += 4;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	s += tr->dsdx;
	t += tr->dtdx;
	rho += tr->drhodx;
	mask <<= 1;
    }
}

#endif /* AFFINE_RHO */

void __fastcall
__glGenericExtractTexelsPCMipmap_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

	qw += tr->fdqwdxPWL;
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;
	rhow += tr->fdrhowdxPWL;

	/* compute next texture parameters */
	if (ExtractBits(qw) < MIN_FLOAT_HEX) {

	    next_s = 0.0f;
	    next_t = 0.0f;
	    next_rho = 0.0f;

	} else {
	    invqw = 1.0F / qw;

	    next_s = sw * invqw;
	    next_t = tw * invqw;
	    next_rho = rhow * invqw;
	}

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
	tr->drhodx = drho;

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
	rho = prev_rho;

	while (1) {
	    while (((int)mask) < 0) {
		if (rho <= tex->c) {
		    (*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
		} else {
		    GLuint lod = ExtractLOD(rho);
		    (*gc->procs.slowMinFilter)(tex, lod, s, t, &texel);
		}

		texelBuf[0] = texel.r;
		texelBuf += 1;

		s += tr->dsdx;
		t += tr->dtdx;
		rho += tr->drhodx;

		mask <<= 1;
	    }

	    if (mask == 0) break;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;

	    mask <<= 1;
	}

	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
	prev_rho = next_rho;
    }
}

void __fastcall
__glGenericExtractTexelsPCMipmap_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

	qw += tr->fdqwdxPWL;
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;
	rhow += tr->fdrhowdxPWL;

	/* compute next texture parameters */
	if (ExtractBits(qw) < MIN_FLOAT_HEX) {

	    next_s = 0.0f;
	    next_t = 0.0f;
	    next_rho = 0.0f;

	} else {
	    invqw = 1.0F / qw;

	    next_s = sw * invqw;
	    next_t = tw * invqw;
	    next_rho = rhow * invqw;
	}

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
	tr->drhodx = drho;

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
	rho = prev_rho;

	while (1) {
	    while (((int)mask) < 0) {
		if (rho <= tex->c) {
		    (*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
		} else {
		    GLuint lod = ExtractLOD(rho);
		    (*gc->procs.slowMinFilter)(tex, lod, s, t, &texel);
		}

		texelBuf[0] = texel.r;
		texelBuf[1] = texel.g;
		texelBuf[2] = texel.b;
		texelBuf += 3;

		s += tr->dsdx;
		t += tr->dtdx;
		rho += tr->drhodx;

		mask <<= 1;
	    }

	    if (mask == 0) break;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;

	    mask <<= 1;
	}

	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
	prev_rho = next_rho;
    }
}

void __fastcall
__glGenericExtractTexelsPCMipmap_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexel texel;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw	= 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

	qw += tr->fdqwdxPWL;
	sw += tr->fdswdxPWL;
	tw += tr->fdtwdxPWL;
	rhow += tr->fdrhowdxPWL;

	/* compute next texture parameters */
	if (ExtractBits(qw) < MIN_FLOAT_HEX) {

	    next_s = 0.0f;
	    next_t = 0.0f;
	    next_rho = 0.0f;

	} else {
	    invqw = 1.0F / qw;

	    next_s = sw * invqw;
	    next_t = tw * invqw;
	    next_rho = rhow * invqw;
	}

	ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
	drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

	tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
	tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
	tr->drhodx = drho;

	s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
	t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
	rho = prev_rho;

	while (1) {
	    while (((int)mask) < 0) {
		if (rho <= tex->c) {
		    (*gc->procs.slowMagFilter)(tex, 0, s, t, &texel);
		} else {
		    GLuint lod = ExtractLOD(rho);
		    (*gc->procs.slowMinFilter)(tex, lod, s, t, &texel);
		}

		texelBuf[0] = texel.r;
		texelBuf[1] = texel.g;
		texelBuf[2] = texel.b;
		texelBuf[3] = texel.a;
		texelBuf += 4;

		s += tr->dsdx;
		t += tr->dtdx;
		rho += tr->drhodx;

		mask <<= 1;
	    }

	    if (mask == 0) break;

	    s += tr->dsdx;
	    t += tr->dtdx;
	    rho += tr->drhodx;

	    mask <<= 1;
	}

	if (mask2 == 0) break;

	mask = mask2 << 16;
	mask2 = 0;

	prev_s = next_s;
	prev_t = next_t;
	prev_rho = next_rho;
    }
}

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
#ifdef __GL_CODEGEN
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>

#include "context.h"
#include "render.h"
#include "global.h"
#include "fr_modes.h"
#include "fr_tri.h"
#include "fr_fbtype.h"

#include "og.h"
#include "ras_og.h"

#undef trs

#define MMX 1
#define STATIC static

#define DISPOF(k)	((k)->spec.mem.disp->spec.value)
#define TRBASE		tr->spec.mem.disp->spec.value
#define	tri(M)		__glNSWhereIs(gc->tr, offsetof(M, __GLtri), ssizeof(M, __GLtri))
#define tr(M)		mem2(gc, value(gc, TRBASE + tri(M)), esp)
#define tr2(M, N)	mem2(gc, value(gc, TRBASE + __glNSWhereIs(gc->tr, offsetof(M, __GLtri), ssizeof(M, __GLtri)) + N), esp)

/*
 * trs(M) and trsq(M) (the MMX equivalent) access one of the Y-step values
 * according to ebx.  So trs(drdy) is roughly equivalent to C 'tr->drdy0[ebx]'
 * When we're MMXing, there are eight bytes between each each because the
 * interpolants are paired.
 */
#define trs(M)		\
(__glNSWhereIs(gc->tr, offsetof(M[0], __GLtri), ssizeof(M[0], __GLtri)), \
 __glNSWhereIs(gc->tr, offsetof(M[1], __GLtri), ssizeof(M[1], __GLtri)), \
 mem(gc, value(gc, TRBASE + __glNSWhereIs(gc->tr, offsetof(M, __GLtri), ssizeof(M, __GLtri))), esp, ebx, 4))
#define trsq(M)		\
(__glNSWhereIs(gc->tr, offsetof(M[0], __GLtri), ssizeof(M[0], __GLtri)), \
 __glNSWhereIs(gc->tr, offsetof(M[1], __GLtri), ssizeof(M[1], __GLtri)), \
 mem(gc, value(gc, TRBASE + __glNSWhereIs(gc->tr, offsetof(M, __GLtri), ssizeof(M, __GLtri))), esp, ebx, 8))

/* vxi() and vx() give position of member with the __GLvertex struct.
 * vxi() just gives the offset, vx() gives the same as a value.
 */
#define vxi(M) \
 __glNSWhereIs(&gc->vx_ns, offsetof(M, __GLvertex), ssizeof(M, __GLvertex))
#define vx(M)  value(gc, vxi(M))

/*
 * Setting DEBUG to 1 enables various run-time tests for internal
 * consistency that trip int3's when they fail.  It's basically like
 * assembler assert()s.
 *
 * Defining __GL_CODEGEN_DESCRIBERS enables the describer scheme.
 * This writes a log file on termination in "./areas.log" that 
 * gives a breakdown of all rendering by quality.  See dump_areas()
 * for details of what gets recorded.  WARNING: right now describers
 * are NOT thread-safe: don't use them to analyse multi-threaded apps.
 *
 * Defining __GL_CODEGEN_PMON enables instrumentation code for
 * use with Intel's excellent pmon utility.  It's very useful for finding
 * stalls, cache thrashing and other performance hits.
 *
 * Setting SOURCE_TRAIL to 1 causes an asm source file to be generated
 * for the setup code.  It really serves as an example of og_stream()
 * in use.  Handy if you want to take a look at generated code
 * in a slightly friendlier format than disassembly.
 */

#define DEBUG				0
#define __GL_CODEGEN_DESCRIBERS		0
#define __GL_CODEGEN_PMON		0
#define SOURCE_TRAIL			0

#if __GL_CODEGEN_PMON
#include "pmonstat.h"
#endif /* __GL_CODEGEN_PMON */

#define ALPHA_MAX 256

/************************************************************************/

/* fptr (Floating-Point TRiangle) holds memory variables used 
 * used by the floating-point rasterizer.
 * (There is a much larger MMX equivalent, 'mmxtr').
 */

struct __GLfptri {
    struct celem *dstdx;
    struct celem *dstdx2;

    struct celem *qis;
    struct celem *qit;
    struct celem *st;
    struct celem *ist;
    struct celem *is;
    struct celem *it;
    struct celem *prev_is;
    struct celem *prev_it;
    struct celem *converters[16];
};

STATIC void
this_warmup(__GLcontext *gc,
	    struct __GLfptri *fptr,
	    float *data)
{
    GLuint i;

    for (i = 0; i < 16; i++) {
	fptr->converters[i] = fpk(gc, ((float)(1 << 20) * (float)(3 << (15 + (i)))));
    }

    i = 0;
    fptr->qis = mem2(gc, value(gc, (int)&(data[i++])), NULL); /* allocate qword */
    i++;
    fptr->qit = mem2(gc, value(gc, (int)&(data[i++])), NULL); /* allocate qword */
    i++;

    fptr->st = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->ist = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->dstdx = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->dstdx2 = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->is = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->it = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->prev_is = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->prev_it = mem2(gc, value(gc, (int)&(data[i++])), NULL);
}

/************************************************************************/

/* Simple macro to move 'src' to 'dst'
 */
STATIC unsigned char*
memov(__GLcontext *gc,
      unsigned char *cb,
      struct celem *dst,
      struct celem *src)
{
cb = emit_code(gc, cb, mov(gc, eax, src));
cb = emit_code(gc, cb, mov(gc, dst, eax));

  return cb;
}

STATIC unsigned char*
addesp(__GLcontext *gc,
       unsigned char* cb,
       struct celem *count)
{
    int cc = count->spec.value;
  
    if (cc <= 16) {
        while (8 <= cc) {
cb = emit_code(gc, cb, pop(gc, eax));
cb = emit_code(gc, cb, pop(gc, edx));
 	    cc -= 8;
        }
        if (cc)
cb = emit_code(gc, cb, pop(gc, eax));
    } else {
cb = emit_code(gc, cb, add(gc, esp, count));
    }
   
    return cb;
}

/* Handle the per-span step for an interpolator.  Used in the spanlet path
 * only.
 */
STATIC unsigned char*
ds(__GLcontext *gc,
   unsigned char *cb,
   struct celem *i,
   struct celem *dids)
{
cb = emit_code(gc, cb, mov(gc, eax, i));
cb = emit_code(gc, cb, mov(gc, ecx, dids));
cb = emit_code(gc, cb, add(gc, ecx, eax));
cb = emit_code(gc, cb, mov(gc, i, ecx));

  return cb;
}

/* Handle the Y-step for an interpolator */
STATIC unsigned char* 
dy(__GLcontext *gc,
   unsigned char *cb,
   struct celem *i,
   struct celem *didy)
{

cb = emit_code(gc, cb, mov(gc, eax, i));
cb = emit_code(gc, cb, mov(gc, ecx, didy));
cb = emit_code(gc, cb, add(gc, ecx, eax));
cb = emit_code(gc, cb, mov(gc, i, ecx));

  return cb;
}

/* Handle the Y-step for an interpolator, but use the specified work
 * registers 'r1' and 'r2'.
 */
STATIC unsigned char*
dyv(__GLcontext *gc,
    unsigned char *cb,
    struct celem *i,
    struct celem *didy,
    struct celem *r1,
    struct celem *r2)
{

cb = emit_code(gc, cb, mov(gc, r1, i));
cb = emit_code(gc, cb, mov(gc, r2, didy));
cb = emit_code(gc, cb, add(gc, r1, r2));

  return cb;
}

/* Given
 *  	1/q	q	s	t
 * on the stack, calculates new s and t, stores them in is and it,
 * and fixed-point versions in qis and qit.
 * Increments the three interpolants, and leaves the stack:
 *  	q	q	s	t
 */

STATIC unsigned char*
pcstep(__GLcontext *gc, 
       unsigned char *cb, 
       struct celem *tr,
       int wl2,
       int hl2,
       struct celem* len,
       struct __GLfptri *fptr)
{
cb = emit_code(gc, cb, fld(gc, st3));
    
cb = emit_code(gc, cb, fmul(gc, st1));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fmul(gc, st3));
cb = emit_code(gc, cb, fst(gc, fptr->is));
cb = emit_code(gc, cb, fadd(gc, fptr->converters[wl2]));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fst(gc, fptr->it));
cb = emit_code(gc, cb, fadd(gc, fptr->converters[hl2]));
    
    /* Scale all 3 steps (fd.wdx) by span length */
cb = emit_code(gc, cb, fild(gc, len));
cb = emit_code(gc, cb, fstp(gc, fptr->qit));
    
cb = emit_code(gc, cb, fld(gc, tr(fdqwdx)));
cb = emit_code(gc, cb, fmul(gc, fptr->qit));
cb = emit_code(gc, cb, fld(gc, tr(fdswdx)));
cb = emit_code(gc, cb, fmul(gc, fptr->qit));
cb = emit_code(gc, cb, fld(gc, tr(fdtwdx)));
cb = emit_code(gc, cb, fmul(gc, fptr->qit));
cb = emit_code(gc, cb, fxch(gc, st2));
    
cb = emit_code(gc, cb, faddp(gc, st5, st0));
cb = emit_code(gc, cb, faddp(gc, st5, st0));
cb = emit_code(gc, cb, faddp(gc, st5, st0));
    
cb = emit_code(gc, cb, fstpq(gc, fptr->qit));
cb = emit_code(gc, cb, fstpq(gc, fptr->qis));
cb = emit_code(gc, cb, fld(gc, st0));

    return cb;
}

/*
 * Reads is,it and prev_is,prev_it.  Calculates dstdx, trashing qis,qit.
 */

STATIC unsigned char*
pcstepnext(__GLcontext *gc,
	   unsigned char *cb,
	   struct celem *tr,
	   int wl2,
	   int hl2,
	   struct __GLfptri *fptr)
{
    static const float reciprocals[17] = {
	0.0f,
	1.0f,
	1.0f / 2.0f,
	1.0f / 3.0f,
	1.0f / 4.0f,
	1.0f / 5.0f,
	1.0f / 6.0f,
	1.0f / 7.0f,
	1.0f / 8.0f,
	1.0f / 9.0f,
	1.0f / 10.0f,
	1.0f / 11.0f,
	1.0f / 12.0f,
	1.0f / 13.0f,
	1.0f / 14.0f,
	1.0f / 15.0f,
	1.0f / 16.0f
    };

cb = emit_code(gc, cb, fld(gc, fptr->is));
cb = emit_code(gc, cb, fsub(gc, fptr->prev_is));
cb = emit_code(gc, cb, fld(gc, fptr->it));
cb = emit_code(gc, cb, fsub(gc, fptr->prev_it));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&reciprocals), NULL, ebx, 4)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&reciprocals), NULL, ebx, 4)));
cb = emit_code(gc, cb, fxch(gc, st1));
	    
cb = emit_code(gc, cb, fadd(gc, fptr->converters[wl2]));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fadd(gc, fptr->converters[hl2]));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, mov(gc, eax, fptr->is));
cb = emit_code(gc, cb, mov(gc, fptr->prev_is, eax));
cb = emit_code(gc, cb, mov(gc, eax, fptr->it));
cb = emit_code(gc, cb, mov(gc, fptr->prev_it, eax));

cb = emit_code(gc, cb, fstpq(gc, fptr->qis));
cb = emit_code(gc, cb, fstpq(gc, fptr->qit));

cb = emit_code(gc, cb, mov(gc, eax, fptr->qit));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, eax, fptr->qis));
cb = emit_code(gc, cb, mov(gc, fptr->dstdx, eax));

    return cb;
}

STATIC int
log2(unsigned int n)
{
  if (n >= 0x100)
    return (8 + log2(n >> 8));
  if (n >= 0x80)
    return 7;
  if (n >= 0x40)
    return 6;
  if (n >= 0x20)
    return 5;
  if (n >= 0x10)
    return 4;
  if (n >= 0x08)
    return 3;
  if (n >= 0x04)
    return 2;
  if (n >= 0x02)
    return 1;
  return 0;
}

unsigned char* gobit(__GLcontext *gc,
		     unsigned char* cb,
		     struct celem* r,
		     int from, int to)
{
  if (from < to)
cb = emit_code(gc, cb, shl(gc, r, value(gc, (to - from))));
  else
cb = emit_code(gc, cb, shr(gc, r, value(gc, (from - to))));

  return cb;
}

unsigned char* clamp(__GLcontext *gc,
		     unsigned char* cb,
		     struct celem* r,
		     int bound)
{
  struct celem *ok = label(gc);

cb = emit_code(gc, cb, cmp(gc, r, value(gc, bound)));
cb = emit_code(gc, cb, jl(gc, ok));
cb = emit_code(gc, cb, mov(gc, r, value(gc, (bound - 1))));
ok->spec.label.where = cb; resolve(gc, ok, cb);

  return cb;
}

#if __GL_CODEGEN_DESCRIBERS
struct describer {
    char name[200];
    unsigned long pixels;
    unsigned long spans;
    unsigned long tot_tris;
    unsigned long drawn_tris;
    unsigned long easy_tris;
    unsigned long cyc[2];
    double cycles;
    struct describer* prev;
};

static struct describer* last_describer = NULL;
#endif /* __GL_CODEGEN_DESCRIBERS */

/*
 * Render a gouraud span
 */

#define DUP2(x)	(((x) << 16) | (x))

STATIC unsigned char*
smoothspan(__GLcontext *gc,
	   unsigned char* cb,
	   struct celem* tr,
	   struct celem* drdxH,
	   struct celem* dgdxH,
	   struct celem* dbdxH,
	   struct celem* drdxP,
	   struct celem* dgdxP,
	   struct celem* dbdxP,
	   struct celem* last,
	   int fbsize)
{
  __GLcolorBuffer *cfb = gc->drawBuffer;
  int redSize, greenSize, blueSize;
  struct celem* perpixelpair = label(gc);
  struct celem* done = label(gc);

  redSize = log2(cfb->redMax);
  greenSize = log2(cfb->greenMax);
  blueSize = log2(cfb->blueMax);

cb = emit_code(gc, cb, shr(gc, ebx, value(gc, 1)));
cb = emit_code(gc, cb, test(gc, ebx, ebx));
cb = emit_code(gc, cb, je(gc, done));

cb = emit_code(gc, cb, mov(gc, edx, tr(dcpdx)));
cb = emit_code(gc, cb, cmp(gc, edx, value(gc, 0)));
cb = emit_code(gc, cb, jle(gc, done));

	/* Extend color interpolators to 32 bits, and
	 * step the later half.
	 */

cb = emit_code(gc, cb, mov(gc, edi, ebx));

cb = emit_code(gc, cb, mov(gc, eax, tr(r0)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(g0)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(b0)));

cb = emit_code(gc, cb, shr(gc, eax, value(gc, redSize + 1)));
cb = emit_code(gc, cb, shr(gc, ebx, value(gc, greenSize + 1)));
cb = emit_code(gc, cb, shr(gc, ecx, value(gc, blueSize + 1)));

cb = emit_code(gc, cb, mov(gc, edx, eax));
cb = emit_code(gc, cb, mov(gc, ebp, ebx));
cb = emit_code(gc, cb, mov(gc, esi, ecx));

cb = emit_code(gc, cb, shl(gc, edx, value(gc, 16)));
cb = emit_code(gc, cb, shl(gc, ebp, value(gc, 16)));
cb = emit_code(gc, cb, shl(gc, esi, value(gc, 16)));

cb = emit_code(gc, cb, or(gc, eax, edx));
cb = emit_code(gc, cb, or(gc, ebx, ebp));
cb = emit_code(gc, cb, or(gc, ecx, esi));

cb = emit_code(gc, cb, mov(gc, edx, drdxH));
cb = emit_code(gc, cb, mov(gc, ebp, dgdxH));
cb = emit_code(gc, cb, mov(gc, esi, dbdxH));

cb = emit_code(gc, cb, and(gc, edx, value(gc, 0xffff0000)));
cb = emit_code(gc, cb, and(gc, ebp, value(gc, 0xffff0000)));
cb = emit_code(gc, cb, and(gc, esi, value(gc, 0xffff0000)));

cb = emit_code(gc, cb, add(gc, eax, edx));
cb = emit_code(gc, cb, add(gc, ebx, ebp));
cb = emit_code(gc, cb, add(gc, ecx, esi));

	/* Now figure where the run will end, set up
	 * 'last' and edi
	 */

cb = emit_code(gc, cb, mov(gc, edx, tr(cp0)));
cb = emit_code(gc, cb, mov(gc, esi, edi));

cb = emit_code(gc, cb, lea(gc, edx, mem(gc, NULL, edx, esi, 4)));
cb = emit_code(gc, cb, xor(gc, edi, edi));

cb = emit_code(gc, cb, and(gc, edx, value(gc, ~3)));
cb = emit_code(gc, cb, mov(gc, last, edx));

cb = emit_code(gc, cb, sub(gc, edi, esi));

cb = emit_code(gc, cb, shl(gc, edi, value(gc, 2)));

perpixelpair->spec.label.where = cb; resolve(gc, perpixelpair, cb);

cb = emit_code(gc, cb, mov(gc, edx, eax));
cb = emit_code(gc, cb, mov(gc, ebp, ebx));

cb = emit_code(gc, cb, shr(gc, edx, value(gc, (15 - redSize) - cfb->redShift)));
cb = emit_code(gc, cb, and(gc, ebp, value(gc, DUP2(cfb->greenMax << (15 - greenSize)))));

cb = emit_code(gc, cb, shr(gc, ebp, value(gc, (15 - greenSize) - cfb->greenShift)));
cb = emit_code(gc, cb, and(gc, edx, value(gc, DUP2(cfb->redMax << cfb->redShift))));

cb = emit_code(gc, cb, mov(gc, esi, ecx));
cb = emit_code(gc, cb, or(gc, edx, ebp));

cb = emit_code(gc, cb, shr(gc, esi, value(gc, (15 - blueSize) - cfb->blueShift)));
cb = emit_code(gc, cb, mov(gc, ebp, drdxP));

cb = emit_code(gc, cb, and(gc, esi, value(gc, DUP2(cfb->blueMax << cfb->blueShift))));
cb = emit_code(gc, cb, add(gc, eax, ebp));

cb = emit_code(gc, cb, or(gc, edx, esi));
cb = emit_code(gc, cb, mov(gc, esi, last));

cb = emit_code(gc, cb, nop(gc));
cb = emit_code(gc, cb, mov(gc, ebp, tr(dcpdx)));

cb = emit_code(gc, cb, add(gc, ebx, dgdxP));
cb = emit_code(gc, cb, add(gc, ecx, dbdxP));

cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, esi, edi, 1), edx));
cb = emit_code(gc, cb, add(gc, edi, value(gc, 4)));

cb = emit_code(gc, cb, jne(gc, perpixelpair));

done->spec.label.where = cb; resolve(gc, done, cb);

  return cb;
}

/*
 * Render a flat dithered span.  Do do this, we write out 4 pixels
 * at a time; in 8-bit we write dwords, in 16-bit we have to use fistp.
 */

STATIC unsigned char*
flatspan(__GLcontext *gc,
	 unsigned char* cb,
	 struct celem* tr,
	 struct celem* ditherline,		/* Pointer to dither pattern */
	 struct celem* w,
	 int fbsize)
{
    struct celem *try_d, *do_d, *try1, *do1;
    struct celem *reverse, *done;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

/*
 *	eax	ebx	ecx	edx	ebp	esi	edi
 *		w	dxcp	r0			cp
 */

    try1 = label(gc);
    try_d = label(gc);
    do1 = label(gc);
    do_d = label(gc);
    reverse = label(gc);
    done = label(gc);

cb = emit_code(gc, cb, mov(gc, ecx, tr(dcpdx)));

    if (fbsize == 1) {
        if (modeFlags & __GL_SHADE_DITHER) {
cb = emit_code(gc, cb, mov(gc, edx, ditherline));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, NULL, edx, NULL, 1)));
	} else {
cb = emit_code(gc, cb, mov(gc, edx, tr(r0)));
cb = emit_code(gc, cb, mov(gc, dh, dl));
cb = emit_code(gc, cb, mov(gc, eax, edx));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, or(gc, edx, eax));
	}
    } else if (fbsize == 2) {
	if (modeFlags & __GL_SHADE_DITHER) {
cb = emit_code(gc, cb, mov(gc, edx, ditherline));
cb = emit_code(gc, cb, fildq(gc, mem(gc, NULL, edx, NULL, 1)));
	} else {
cb = emit_code(gc, cb, mov(gc, edx, tr(r0)));
cb = emit_code(gc, cb, mov(gc, eax, edx));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, or(gc, edx, eax));
cb = emit_code(gc, cb, push(gc, edx));
cb = emit_code(gc, cb, push(gc, edx));
cb = emit_code(gc, cb, fildq(gc, mem(gc, value(gc, 0), esp, NULL, 1)));
cb = emit_code(gc, cb, add(gc, esp, value(gc, 8)));
	}
    } else {
	assert(4 == fbsize);

cb = emit_code(gc, cb, mov(gc, edx, tr(r0)));
cb = emit_code(gc, cb, push(gc, edx));
cb = emit_code(gc, cb, push(gc, edx));
cb = emit_code(gc, cb, fildq(gc, mem(gc, value(gc, 0), esp, NULL, 1)));
cb = emit_code(gc, cb, add(gc, esp, value(gc, 8)));
    }

cb = emit_code(gc, cb, mov(gc, edi, tr(cp0)));
    
cb = emit_code(gc, cb, cmp(gc, ecx, value(gc, 0)));
cb = emit_code(gc, cb, jg(gc, reverse));
    if (fbsize == 1) {
cb = emit_code(gc, cb, sub(gc, edi, ebx));
cb = emit_code(gc, cb, inc(gc, edi));
    } else if (fbsize == 2) {
cb = emit_code(gc, cb, sub(gc, edi, ebx));
cb = emit_code(gc, cb, sub(gc, edi, ebx));
cb = emit_code(gc, cb, add(gc, edi, value(gc, fbsize)));
    } else {
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, 0), NULL, ebx, 4)));
cb = emit_code(gc, cb, sub(gc, edi, eax));
cb = emit_code(gc, cb, add(gc, edi, value(gc, fbsize)));
    }
reverse->spec.label.where = cb; resolve(gc, reverse, cb);

cb = emit_code(gc, cb, jmp(gc, try_d));
do_d->spec.label.where = cb; resolve(gc, do_d, cb);
    if (fbsize == 1) {
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
    } else if (fbsize == 2) {
cb = emit_code(gc, cb, fld(gc, st0));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fistpq(gc, mem(gc, NULL, edi, NULL, 1)));
    } else {
cb = emit_code(gc, cb, fld(gc, st0));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fistpq(gc, mem(gc, NULL, edi, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, st0));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fistpq(gc, mem(gc, value(gc, 8), edi, NULL, 1)));
    }
cb = emit_code(gc, cb, add(gc, edi, value(gc, 4 * fbsize)));
cb = emit_code(gc, cb, sub(gc, ebx, value(gc, 4)));
cb = emit_code(gc, cb, cmp(gc, ebx, value(gc, 4)));
cb = emit_code(gc, cb, jge(gc, do_d));
try_d->spec.label.where = cb; resolve(gc, try_d, cb);
cb = emit_code(gc, cb, cmp(gc, ebx, value(gc, 4)));
cb = emit_code(gc, cb, jl(gc, try1));
cb = emit_code(gc, cb, test(gc, edi, value(gc, (fbsize == 1) ? 3 : 7)));
cb = emit_code(gc, cb, je(gc, do_d));

try1->spec.label.where = cb; resolve(gc, try1, cb);
cb = emit_code(gc, cb, cmp(gc, ebx, value(gc, 1)));
cb = emit_code(gc, cb, jl(gc, done));
do1->spec.label.where = cb; resolve(gc, do1, cb);
    if (modeFlags & __GL_SHADE_DITHER) {
cb = emit_code(gc, cb, mov(gc, eax, edi));
	
cb = emit_code(gc, cb, and(gc, eax, value(gc, 3)));
	
	if (fbsize == 2)
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 1)));
	
cb = emit_code(gc, cb, add(gc, eax, ditherline));
	
	if (fbsize == 1) {
cb = emit_code(gc, cb, mov(gc, al, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, dec(gc, ebx));
	
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), al));
	} else {
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, NULL, eax, NULL, 1)));
	
cb = emit_code(gc, cb, dec(gc, ebx));
	
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
	}
cb = emit_code(gc, cb, add(gc, edi, value(gc, fbsize)));
    } else {
cb = emit_code(gc, cb, dec(gc, ebx));
	if (fbsize == 1) {
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), dl));
	} else if (fbsize == 2) {
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
	} else {
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
	}
cb = emit_code(gc, cb, add(gc, edi, value(gc, fbsize)));
    }

cb = emit_code(gc, cb, test(gc, edi, value(gc, (fbsize == 1) ? 3 : 7)));
cb = emit_code(gc, cb, je(gc, try_d));

cb = emit_code(gc, cb, test(gc, ebx, ebx));
cb = emit_code(gc, cb, jne(gc, do1));

done->spec.label.where = cb; resolve(gc, done, cb);

    if (fbsize != 1) {
cb = emit_code(gc, cb, fstp(gc, st0));
    }
    return cb;
}

/*
 * Render a span
 *
 */

STATIC unsigned char*
span(__GLcontext *gc, 
     unsigned char *cb,
     GLuint modeFlags,
     struct celem* tr,
     struct celem* truew,
     struct celem* w,
     struct celem* dither_index,
     struct __GLfptri *fptr,
     struct celem* rptr,	/* Pointers to lighting tables for flat-modulate */
     struct celem* gptr,
     struct celem* bptr,
     int interpolate_color,
     int interpolate_z,
     int fbsize,
     GLenum texenv)
{
  struct celem
    *perpixel,
    *perpixel_reload,
    *zfail,
    *allover,
    *adjuster;
  struct celem *r = NULL, *g = NULL, *b = NULL;
  __GLcolorBuffer *cfb = gc->drawBuffer;
  __GLtexture *current;
  __GLmipMapLevel *lp;
  GLuint wl2, hl2;
  enum { hi16, lo8, loN } colorformat;
  GLenum baseFormat;
  int redSize, greenSize, blueSize;
  int spill_rgb;	/* Set to 1 if RGB (eax,ebx,ecx) must be spilt in the loop */
  int spill_hiset;	/* set to 1 if spilling (ebp,esi,edi) in the loop */
  GLboolean cooked = GL_FALSE;
  struct celem* final_pixel;

  redSize = log2(cfb->redMax);
  greenSize = log2(cfb->greenMax);
  blueSize = log2(cfb->blueMax);

  perpixel = label(gc);
  perpixel_reload = label(gc);
  zfail = label(gc);
  allover = label(gc);
  adjuster = label(gc);

  if (modeFlags & __GL_SHADE_TEXTURE) {
    current = gc->texture.currentTexture;
    lp = current->level[0];
    wl2 = lp->widthLog2;
    hl2 = lp->heightLog2;

    switch (texenv) {
    case GL_REPLACE:
	if (!(modeFlags & __GL_SHADE_DITHER) && lp->pixelBuffer)
	    cooked = GL_TRUE;
	break;
    }
  }

  /*
   * Texture setup: load ecx with starting s,t, dstdx
   * with change per pixel.
   */

  if (modeFlags & __GL_SHADE_TEXTURE) {
cb = emit_code(gc, cb, mov(gc, truew, ebx));
    {
      struct celem* no = label(gc);

cb = emit_code(gc, cb, cmp(gc, ebx, value(gc, 16)));
cb = emit_code(gc, cb, jle(gc, no));
cb = emit_code(gc, cb, mov(gc, ebx, value(gc, 16)));
no->spec.label.where = cb; resolve(gc, no, cb);
cb = emit_code(gc, cb, mov(gc, w, ebx));
    }

    if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
cb = emit_code(gc, cb, fld(gc, tr(ftw0)));
cb = emit_code(gc, cb, fld(gc, tr(fsw0)));
cb = emit_code(gc, cb, fld(gc, tr(fqw0)));

cb = emit_code(gc, cb, fld(gc, st0));

cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));

cb = pcstep(gc, cb, tr, wl2, hl2, w, fptr);

cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));

cb = emit_code(gc, cb, mov(gc, eax, fptr->qit));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, eax, fptr->qis));
cb = emit_code(gc, cb, mov(gc, fptr->ist, eax));

cb = emit_code(gc, cb, mov(gc, eax, fptr->is));
cb = emit_code(gc, cb, mov(gc, fptr->prev_is, eax));
cb = emit_code(gc, cb, mov(gc, eax, fptr->it));
cb = emit_code(gc, cb, mov(gc, fptr->prev_it, eax));

		/* Length of this span is more complicated: it's
		 * min(truew - 16, 16)
		 */
		{
		    struct celem *ok = near_label(gc);

cb = emit_code(gc, cb, mov(gc, eax, truew));
cb = emit_code(gc, cb, sub(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, jle(gc, ok));
cb = emit_code(gc, cb, mov(gc, eax, value(gc, 16)));
ok->spec.label.where = cb; resolve(gc, ok, cb);
cb = emit_code(gc, cb, mov(gc, fptr->qit, eax));
		}

cb = pcstep(gc, cb, tr, wl2, hl2, fptr->qit, fptr);

		/* Now need to calculate ist for the second span */
cb = emit_code(gc, cb, mov(gc, eax, fptr->qit));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, eax, fptr->qis));

cb = emit_code(gc, cb, mov(gc, ecx, fptr->ist));
cb = emit_code(gc, cb, mov(gc, fptr->ist, eax));
cb = emit_code(gc, cb, mov(gc, fptr->st, ecx));

cb = pcstepnext(gc, cb, tr, wl2, hl2, fptr);

cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));
    } else {
cb = emit_code(gc, cb, mov(gc, ecx, tr(t0)));
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, 16 - hl2)));
cb = emit_code(gc, cb, mov(gc, eax, tr(s0)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, wl2)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, ecx, eax));
cb = emit_code(gc, cb, mov(gc, fptr->st, ecx));
    }
  } else {
cb = emit_code(gc, cb, mov(gc, w, ebx));
  }

cb = memov(gc, cb, tr(cp), tr(cp0));

  if (interpolate_z) {
cb = emit_code(gc, cb, mov(gc, eax, tr(z0)));
cb = emit_code(gc, cb, mov(gc, tr(z), eax));
cb = emit_code(gc, cb, mov(gc, eax, tr(zp0)));
cb = emit_code(gc, cb, mov(gc, tr(zp), eax));
  }

  if (modeFlags & __GL_SHADE_DITHER) {
cb = emit_code(gc, cb, xor(gc, eax, eax));
cb = emit_code(gc, cb, mov(gc, dither_index, eax));
  }

  spill_rgb =
      (modeFlags & __GL_SHADE_TEXTURE);
  spill_hiset = (interpolate_z) ||
    (modeFlags & __GL_SHADE_DITHER) ||
    ((modeFlags & __GL_SHADE_RGB) && !cooked);

    if (modeFlags & __GL_SHADE_SMOOTH) {
cb = emit_code(gc, cb, mov(gc, eax, tr(r0)));
      if (modeFlags & __GL_SHADE_RGB) {
cb = emit_code(gc, cb, mov(gc, ebx, tr(g0)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(b0)));
      }

cb = emit_code(gc, cb, mov(gc, tr(r), eax));
      if (modeFlags & __GL_SHADE_RGB) {
cb = emit_code(gc, cb, mov(gc, tr(g), ebx));
cb = emit_code(gc, cb, mov(gc, tr(b), ecx));
      }
    } else if (!spill_rgb) {
cb = emit_code(gc, cb, mov(gc, eax, tr(r)));
      if (modeFlags & __GL_SHADE_RGB) {
cb = emit_code(gc, cb, mov(gc, ebx, tr(g)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(b)));
      }
      r = eax;
      g = ebx;
      b = ecx;
    }

    if (!spill_hiset) {
cb = emit_code(gc, cb, mov(gc, ebp, tr(dcpdx)));
cb = emit_code(gc, cb, mov(gc, edi, tr(cp)));
perpixel_reload->spec.label.where = cb; resolve(gc, perpixel_reload, cb);
cb = emit_code(gc, cb, mov(gc, esi, w));
    } else {
perpixel_reload->spec.label.where = cb; resolve(gc, perpixel_reload, cb);
    }

perpixel->spec.label.where = cb; resolve(gc, perpixel, cb);
  /* We have:
   *	eax	ebx	ecx	edx	ebp	esi	edi
   *				dxzp			
   */

  if (interpolate_z) {
cb = emit_code(gc, cb, mov(gc, ebp, tr(dzpdx)));
cb = emit_code(gc, cb, mov(gc, edi, tr(zp)));

cb = emit_code(gc, cb, mov(gc, edx, tr(dzdx)));
cb = emit_code(gc, cb, mov(gc, esi, tr(z)));

cb = emit_code(gc, cb, add(gc, edx, esi));
cb = emit_code(gc, cb, add(gc, ebp, edi));

    if (1 /*gc->state.depth.testFunc != GL_ALWAYS*/) {
        if (gc->depthBuffer.buf.depth == 16) {
cb = emit_code(gc, cb, shr(gc, esi, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
        }
cb = emit_code(gc, cb, cmp(gc, esi, mem(gc, NULL, edi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, tr(zp), ebp));
cb = emit_code(gc, cb, mov(gc, tr(z), edx));
        switch (gc->state.depth.testFunc) {
        case GL_NEVER:
cb = emit_code(gc, cb, jmp(gc, zfail));
          break;
        case GL_LESS:
cb = emit_code(gc, cb, jae(gc, zfail));
          break;
        case GL_EQUAL:
cb = emit_code(gc, cb, jne(gc, zfail));
          break;
        case GL_LEQUAL:
cb = emit_code(gc, cb, ja(gc, zfail));
          break;
        case GL_GREATER:
cb = emit_code(gc, cb, jbe(gc, zfail));
          break;
        case GL_NOTEQUAL:
cb = emit_code(gc, cb, je(gc, zfail));
          break;
        case GL_GEQUAL:
cb = emit_code(gc, cb, jb(gc, zfail));
          break;
        case GL_ALWAYS:
	  break;
      }

      if (gc->state.depth.writeEnable) {
        if (gc->depthBuffer.buf.depth == 16)
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), esi));
      }
    } else {
	/* We're running ALWAYS.  Just write out that Z */
        if (gc->depthBuffer.buf.depth == 16) {
cb = emit_code(gc, cb, shr(gc, esi, value(gc, 16)));
        }
cb = emit_code(gc, cb, mov(gc, tr(zp), ebp));
cb = emit_code(gc, cb, mov(gc, tr(z), edx));
      if (gc->state.depth.writeEnable) {
        if (gc->depthBuffer.buf.depth == 16)
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), esi));
      }
    }
    /* If we're masking, we never want to write out the pixel anyway,
     * so we might as well fail now.
     */
    if (modeFlags & __GL_SHADE_MASK) {
cb = emit_code(gc, cb, jmp(gc, zfail));
      }
  }

  if (modeFlags & __GL_SHADE_TEXTURE) {

	if (wl2 <= 8) {
cb = emit_code(gc, cb, mov(gc, ecx, fptr->st));
cb = emit_code(gc, cb, mov(gc, ebx, fptr->dstdx));

cb = emit_code(gc, cb, mov(gc, eax, ecx));
cb = emit_code(gc, cb, add(gc, ebx, ecx));

cb = emit_code(gc, cb, shr(gc, eax, value(gc, 24 - hl2)));

cb = emit_code(gc, cb, mov(gc, al, ch));

	    	if (wl2 < 8)
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 8 - wl2)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(tp)));

	} else {
	    	/* Got to do the more expensive 16-bit version */
cb = emit_code(gc, cb, mov(gc, ecx, fptr->st));
cb = emit_code(gc, cb, mov(gc, ebx, fptr->dstdx));

cb = emit_code(gc, cb, mov(gc, eax, ecx));
cb = emit_code(gc, cb, add(gc, ebx, ecx));

	    	if (hl2 < 16)
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16 - hl2)));

cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, eax, ecx));
	    	if (wl2 < 16)
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16 - wl2)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(tp)));
	}
	/* EAX now holds (unscaled) offset into texture */

        baseFormat = current->texelFormat;

	if ((modeFlags & __GL_SHADE_RGB) && baseFormat == GL_COLOR_INDEX) {
	  /* Indirect, turn it into offset in palette (which looks like
	   * a 256x1 texture).
	   */
cb = emit_code(gc, cb, mov(gc, ecx, tr(tp)));
cb = emit_code(gc, cb, add(gc, ecx, eax));
cb = emit_code(gc, cb, xor(gc, eax, eax));
cb = emit_code(gc, cb, mov(gc, al, mem(gc, NULL, ecx, NULL, 1)));
	    	/* EAX now holds (unscaled) offset into palette */
	    	baseFormat = current->CT.baseFormat;
	}

	if (cooked) {
	    switch (fbsize) {
	    case 1:
cb = emit_code(gc, cb, mov(gc, dl, mem(gc, NULL, ecx, eax, 1)));
		break;
	    case 2:
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, NULL, ecx, eax, 2)));
		break;
	    case 3:
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, NULL, ecx, eax, 2)));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, NULL, ecx, eax, 1)));
		break;
	    case 4:
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, NULL, ecx, eax, 4)));
		break;
	    }
	} else {
	  switch (baseFormat) {
	  case GL_COLOR_INDEX:
cb = emit_code(gc, cb, mov(gc, edx, eax));
          r =	mem(gc, NULL, ecx, edx, 1);
	  break;
	case GL_LUMINANCE:
	case GL_INTENSITY:
cb = emit_code(gc, cb, mov(gc, edx, eax));
cb = emit_code(gc, cb, mov(gc, esi, ecx));
          colorformat = lo8;
          r =	mem(gc, NULL, esi, edx, 1);
          g =	mem(gc, NULL, esi, edx, 1);
	  b =	mem(gc, NULL, esi, edx, 1);
	  break;
	case GL_LUMINANCE_ALPHA:
          colorformat = lo8;
cb = emit_code(gc, cb, mov(gc, edx, eax));
cb = emit_code(gc, cb, mov(gc, esi, ecx));
          r =	mem(gc, NULL, esi, edx, 2);
          g =	mem(gc, NULL, esi, edx, 2);
          b =	mem(gc, NULL, esi, edx, 2);
	  break;
	case GL_RGB:
cb = emit_code(gc, cb, lea(gc, edx, mem(gc, NULL, eax, eax, 2)));
cb = emit_code(gc, cb, mov(gc, esi, ecx));
          colorformat = lo8;
          r =	mem(gc, NULL, esi, edx, 1);
          g =	mem(gc, value(gc, 1), esi, edx, 1);
	  b =	mem(gc, value(gc, 2), esi, edx, 1);
	  break;
	case GL_RGBA:
          colorformat = lo8;
cb = emit_code(gc, cb, mov(gc, edx, eax));
cb = emit_code(gc, cb, mov(gc, esi, ecx));
          r =	mem(gc, NULL, esi, edx, 4);
          g =	mem(gc, value(gc, 1), esi, edx, 4);
	  b =	mem(gc, value(gc, 2), esi, edx, 4);
	  break;
        }
     }
  } else if (interpolate_color) {
    if (spill_rgb) {
cb = emit_code(gc, cb, mov(gc, eax, tr(r)));
      if (modeFlags & __GL_SHADE_RGB) {
cb = emit_code(gc, cb, mov(gc, ebx, tr(g)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(b)));
      }
    }
    colorformat = hi16;
    r = eax;
    g = ebx;
    b = ecx;
  }

  if (modeFlags & __GL_SHADE_TEXTURE) {
      switch (texenv) {
      case GL_MODULATE: {
	int redshift, greenshift, blueshift;

	if (!(modeFlags & __GL_SHADE_DITHER)) {

	    redshift = 
		blueshift = 
		greenshift = 16;

	    colorformat = loN;
	} else {
	    redshift = redSize + 1 + 8;
	    greenshift = greenSize + 1 + 8;
	    blueshift = blueSize + 1 + 8;
	    colorformat = lo8;
	}

#define sqtab	value(gc, (int)(&__glSquareTable2[256]))

	if (modeFlags & __GL_SHADE_SMOOTH) {
cb = emit_code(gc, cb, xor(gc, eax, eax));
cb = emit_code(gc, cb, mov(gc, fptr->st, ebx));

cb = emit_code(gc, cb, mov(gc, al, r));
cb = emit_code(gc, cb, xor(gc, ebx, ebx));

cb = emit_code(gc, cb, mov(gc, bl, g));
cb = emit_code(gc, cb, xor(gc, ecx, ecx));

cb = emit_code(gc, cb, mov(gc, cl, b));
cb = emit_code(gc, cb, mov(gc, ebp, tr(r)));

cb = emit_code(gc, cb, shr(gc, ebp, value(gc, redshift)));
cb = emit_code(gc, cb, mov(gc, edi, tr(g)));

cb = emit_code(gc, cb, shr(gc, edi, value(gc, greenshift)));
cb = emit_code(gc, cb, mov(gc, esi, tr(b)));

cb = emit_code(gc, cb, shr(gc, esi, value(gc, blueshift)));
cb = emit_code(gc, cb, push(gc, ecx));

cb = emit_code(gc, cb, mov(gc, cl, mem(gc, sqtab, ebp, NULL, 1)));
cb = emit_code(gc, cb, sub(gc, ebp, eax));

cb = emit_code(gc, cb, mov(gc, dl, mem(gc, sqtab, edi, NULL, 1)));
cb = emit_code(gc, cb, sub(gc, edi, ebx));

cb = emit_code(gc, cb, add(gc, cl, mem(gc, sqtab, eax, NULL, 1)));
cb = emit_code(gc, cb, add(gc, dl, mem(gc, sqtab, ebx, NULL, 1)));

cb = emit_code(gc, cb, xor(gc, ebx, ebx));
cb = emit_code(gc, cb, pop(gc, eax));

cb = emit_code(gc, cb, sub(gc, cl, mem(gc, sqtab, ebp, NULL, 1)));
cb = emit_code(gc, cb, sub(gc, dl, mem(gc, sqtab, edi, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, bl, mem(gc, sqtab, eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, bh, mem(gc, sqtab, esi, NULL, 1)));

cb = emit_code(gc, cb, sub(gc, esi, eax));
cb = emit_code(gc, cb, xor(gc, eax, eax));

cb = emit_code(gc, cb, mov(gc, al, cl));
cb = emit_code(gc, cb, add(gc, bl, bh));

cb = emit_code(gc, cb, mov(gc, bh, mem(gc, sqtab, esi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, cl, bl));

cb = emit_code(gc, cb, sub(gc, cl, bh));
cb = emit_code(gc, cb, xor(gc, ebx, ebx));

cb = emit_code(gc, cb, mov(gc, bl, dl));
        } else {
		/* we must be flat */
cb = emit_code(gc, cb, xor(gc, eax, eax));
cb = emit_code(gc, cb, mov(gc, fptr->st, ebx));

cb = emit_code(gc, cb, mov(gc, al, r));
cb = emit_code(gc, cb, mov(gc, edi, rptr));

cb = emit_code(gc, cb, xor(gc, ebx, ebx));
cb = emit_code(gc, cb, mov(gc, ebp, gptr));

cb = emit_code(gc, cb, xor(gc, ecx, ecx));
cb = emit_code(gc, cb, mov(gc, bl, g));

cb = emit_code(gc, cb, mov(gc, cl, b));
cb = emit_code(gc, cb, mov(gc, edx, bptr));

cb = emit_code(gc, cb, mov(gc, al, mem(gc, NULL, edi, eax, 1)));
cb = emit_code(gc, cb, mov(gc, bl, mem(gc, value(gc, 0), ebx, ebp, 1)));

cb = emit_code(gc, cb, mov(gc, cl, mem(gc, NULL, edx, ecx, 1)));
	}

        r = eax;
        g = ebx;
        b = ecx;
	}
	break;
    case GL_ADD:
	if (!(modeFlags & __GL_SHADE_DITHER)) {
cb = emit_code(gc, cb, mov(gc, al, tr2(r,2)));
cb = emit_code(gc, cb, mov(gc, cl, r));

cb = emit_code(gc, cb, add(gc, al, cl));
cb = emit_code(gc, cb, mov(gc, fptr->st, ebx));
            colorformat = lo8;
	} else {
cb = emit_code(gc, cb, xor(gc, eax, eax));
cb = emit_code(gc, cb, mov(gc, al, r));
cb = emit_code(gc, cb, mov(gc, edx, tr(r)));

cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, add(gc, eax, edx));

cb = emit_code(gc, cb, mov(gc, fptr->st, ebx));

            colorformat = hi16;
	}
	r = eax;
	break;
    default:
cb = emit_code(gc, cb, mov(gc, fptr->st, ebx));
	break;

  }
}

/* At this point r, g, b are ready for dithering */
    if (modeFlags & __GL_SHADE_DITHER) {

      if (r && r->what == MEM) {
	/* The color triple is in memory: load into registers */
cb = emit_code(gc, cb, xor(gc, eax, eax));
cb = emit_code(gc, cb, mov(gc, al, r));
      if (modeFlags & __GL_SHADE_RGB) {
cb = emit_code(gc, cb, xor(gc, ebx, ebx));
cb = emit_code(gc, cb, mov(gc, bl, g));
cb = emit_code(gc, cb, xor(gc, ecx, ecx));
cb = emit_code(gc, cb, mov(gc, cl, b));
      }

      colorformat = lo8;

      } if (r == NULL) {
cb = emit_code(gc, cb, mov(gc, eax, tr(r)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(g)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(b)));

	colorformat = hi16;
      }

      r = eax;
      g = ebx;
      b = ecx;

      if ((modeFlags & __GL_SHADE_TEXTURE) && 
	  (modeFlags & __GL_SHADE_RGB) &&
	  (texenv == GL_REPLACE)) {
	  /* Texture replace + dithered, we have to scale down texture,
	   * so we multiply each component by max/max+1.  This
	   * is about the same as subtracting 1/max.  For example,
	   * in 5-bit color, max is 31, so we subtract 1/32 * x.  Hence
	   * 0xff gets scaled down to (0xff - 0x7) = 0xf8.
	   */

	  assert(colorformat == lo8);

cb = emit_code(gc, cb, mov(gc, edx, eax));
cb = emit_code(gc, cb, shr(gc, edx, value(gc, redSize + 1)));
cb = emit_code(gc, cb, mov(gc, esi, ebx));

cb = emit_code(gc, cb, shr(gc, esi, value(gc, greenSize + 1)));
cb = emit_code(gc, cb, mov(gc, edi, ecx));

cb = emit_code(gc, cb, shr(gc, edi, value(gc, blueSize + 1)));
cb = emit_code(gc, cb, sub(gc, eax, edx));

cb = emit_code(gc, cb, sub(gc, ebx, esi));
cb = emit_code(gc, cb, sub(gc, ecx, edi));

      }

      switch (colorformat) {
      case lo8:
cb = emit_code(gc, cb, shl(gc, r, value(gc, (8 + redSize + 1))));
	if (modeFlags & __GL_SHADE_RGB) {
cb = emit_code(gc, cb, shl(gc, g, value(gc, (8 + greenSize + 1))));
cb = emit_code(gc, cb, shl(gc, b, value(gc, (8 + blueSize + 1))));
	}
        break;
      case loN:
	assert(0);
      }

      colorformat = hi16;
      r = eax;
      g = ebx;
      b = ecx;

cb = emit_code(gc, cb, mov(gc, esi, tr(dither)));
cb = emit_code(gc, cb, mov(gc, ebp, dither_index));

cb = emit_code(gc, cb, mov(gc, edx, ebp));
cb = emit_code(gc, cb, inc(gc, ebp));

cb = emit_code(gc, cb, and(gc, ebp, value(gc, 3)));

cb = emit_code(gc, cb, mov(gc, dither_index, ebp));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, NULL, esi, edx, 4)));

    }

  final_pixel = (fbsize == 1) ? dl : edx;
  if (cooked) {
	/* edx already hold pixel to write */
  } else if ((modeFlags & (__GL_SHADE_SMOOTH | __GL_SHADE_DITHER | __GL_SHADE_TEXTURE)) == 0) {
cb = emit_code(gc, cb, mov(gc, edx, tr(r0)));
  } else {
      if (modeFlags & __GL_SHADE_RGB) {
    switch (colorformat) {
    case hi16:

cb = emit_code(gc, cb, mov(gc, edx, r));
cb = emit_code(gc, cb, mov(gc, ebp, b));

      if (!(modeFlags & __GL_SHADE_DITHER)) {
cb = emit_code(gc, cb, shr(gc, edx, value(gc, 16 - cfb->redShift)));
cb = emit_code(gc, cb, mov(gc, esi, g));
      } else {
cb = emit_code(gc, cb, add(gc, edx, esi));
cb = emit_code(gc, cb, add(gc, ebp, esi));

cb = emit_code(gc, cb, shr(gc, edx, value(gc, 16 - cfb->redShift)));
cb = emit_code(gc, cb, add(gc, esi, g));
      }

cb = emit_code(gc, cb, shr(gc, ebp, value(gc, 16 - cfb->blueShift)));
cb = emit_code(gc, cb, and(gc, edx, value(gc, (cfb->redMax << cfb->redShift))));

cb = emit_code(gc, cb, shr(gc, esi, value(gc, 16 - cfb->greenShift)));
cb = emit_code(gc, cb, and(gc, ebp, value(gc, (cfb->blueMax << cfb->blueShift))));

cb = emit_code(gc, cb, and(gc, esi, value(gc, (cfb->greenMax << cfb->greenShift))));
cb = emit_code(gc, cb, or(gc, edx, ebp));

cb = emit_code(gc, cb, or(gc, edx, esi));
      break;

    case lo8:
    {
cb = emit_code(gc, cb, mov(gc, al, r));
cb = emit_code(gc, cb, mov(gc, bl, g));
cb = emit_code(gc, cb, mov(gc, cl, b));

cb = emit_code(gc, cb, and(gc, eax, value(gc, (cfb->redMax << (7 - redSize)))));

cb = gobit(gc, cb, eax, (7 - redSize), cfb->redShift);
cb = emit_code(gc, cb, and(gc, ebx, value(gc, (cfb->greenMax << (7 - greenSize)))));

cb = gobit(gc, cb, ebx, (7 - greenSize), cfb->greenShift);
cb = emit_code(gc, cb, mov(gc, edx, eax));

cb = emit_code(gc, cb, or(gc, edx, ebx));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, (cfb->blueMax << (7 - blueSize)))));

cb = gobit(gc, cb, ecx, (7 - blueSize), cfb->blueShift);

cb = emit_code(gc, cb, or(gc, edx, ecx));
    }
      break;
    case loN:
      if (cfb->blueShift != 0)
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, cfb->blueShift)));

cb = emit_code(gc, cb, shl(gc, eax, value(gc, cfb->redShift)));
cb = emit_code(gc, cb, mov(gc, edx, ecx));

cb = emit_code(gc, cb, shl(gc, ebx, value(gc, cfb->greenShift)));
cb = emit_code(gc, cb, or(gc, edx, eax));

cb = emit_code(gc, cb, or(gc, edx, ebx));
      break;
    }
  } else {
      switch (colorformat) {
      case hi16:
cb = emit_code(gc, cb, mov(gc, edx, eax));
	if (modeFlags & __GL_SHADE_DITHER)
cb = emit_code(gc, cb, add(gc, edx, esi));
cb = emit_code(gc, cb, shr(gc, edx, value(gc, 16)));
        final_pixel = dl;
	break;
      case lo8:
        final_pixel = al;
	break;
      }
  }

    if (interpolate_color && (modeFlags & __GL_SHADE_SMOOTH)) 
	if (modeFlags & __GL_SHADE_RGB) {
	if (spill_rgb) {
cb = emit_code(gc, cb, mov(gc, eax, tr(r)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(g)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(b)));
	}

cb = emit_code(gc, cb, mov(gc, esi, tr(drdx)));

cb = emit_code(gc, cb, add(gc, ebx, tr(dgdx)));
cb = emit_code(gc, cb, add(gc, ecx, tr(dbdx)));

cb = emit_code(gc, cb, add(gc, eax, esi));

	if (spill_rgb) {
cb = emit_code(gc, cb, mov(gc, tr(r), eax));
cb = emit_code(gc, cb, mov(gc, tr(g), ebx));
cb = emit_code(gc, cb, mov(gc, tr(b), ecx));
	}
    } else {
	if (final_pixel == al) {
cb = emit_code(gc, cb, mov(gc, dl, al));
	    final_pixel = dl;
	}
	if (spill_rgb) {
cb = emit_code(gc, cb, mov(gc, eax, tr(r)));
	}
cb = emit_code(gc, cb, mov(gc, ebx, tr(drdx)));
cb = emit_code(gc, cb, add(gc, eax, ebx));
	if (spill_rgb) {
cb = emit_code(gc, cb, mov(gc, tr(r), eax));
	}
    }
  }

  /* 
   * Now 'final_pixel' holds the pixel value to write to the framebuffer
   * Take care not to stall while the write buffer is busy
   */

  if (spill_hiset) {
cb = emit_code(gc, cb, mov(gc, ebp, tr(dcpdx)));
cb = emit_code(gc, cb, mov(gc, edi, tr(cp)));

cb = emit_code(gc, cb, mov(gc, esi, w));
cb = emit_code(gc, cb, add(gc, ebp, edi));

cb = emit_code(gc, cb, dec(gc, esi));
cb = emit_code(gc, cb, mov(gc, tr(cp), ebp));

cb = emit_code(gc, cb, mov(gc, w, esi));
	if (!(modeFlags & __GL_SHADE_MASK)) {
  	    switch (fbsize) {
  	    case 1:
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), final_pixel));
  	      break;
  	    case 2:
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
  	      break;
  	    case 3:
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
cb = emit_code(gc, cb, shr(gc, edx, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), dl));
  	      break;
  	    case 4:
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
  	      break;
            }
        }

cb = emit_code(gc, cb, jne(gc, perpixel));
  } else {
	if (!(modeFlags & __GL_SHADE_MASK)) {
  	    switch (fbsize) {
  	    case 1:
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), final_pixel));
  	      break;
  	    case 2:
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
  	      break;
  	    case 3:
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
cb = emit_code(gc, cb, shr(gc, edx, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), dl));
  	      break;
  	    case 4:
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
  	      break;
  	    }
	}         
cb = emit_code(gc, cb, add(gc, edi, ebp));

cb = emit_code(gc, cb, dec(gc, esi));
cb = emit_code(gc, cb, jne(gc, perpixel));

  }

  if (modeFlags & __GL_SHADE_TEXTURE) {
adjuster->spec.label.where = cb; resolve(gc, adjuster, cb);

cb = emit_code(gc, cb, mov(gc, ebx, truew));
cb = emit_code(gc, cb, sub(gc, ebx, value(gc, 16)));
cb = emit_code(gc, cb, jle(gc, allover));
cb = emit_code(gc, cb, mov(gc, truew, ebx));

      {
          struct celem* no = near_label(gc);
cb = emit_code(gc, cb, cmp(gc, ebx, value(gc, 16)));
cb = emit_code(gc, cb, jle(gc, no));
cb = emit_code(gc, cb, mov(gc, ebx, value(gc, 16)));
no->spec.label.where = cb; resolve(gc, no, cb);
cb = emit_code(gc, cb, mov(gc, w, ebx));
      }

      if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
cb = emit_code(gc, cb, mov(gc, eax, fptr->is));
cb = emit_code(gc, cb, mov(gc, fptr->prev_is, eax));
cb = emit_code(gc, cb, mov(gc, eax, fptr->it));
cb = emit_code(gc, cb, mov(gc, fptr->prev_it, eax));

	  /* Length of this span is more complicated: it's
	   * min(truew - 16, 16)
	   */
	  {
	      struct celem *ok = near_label(gc);
	  
cb = emit_code(gc, cb, mov(gc, eax, truew));
cb = emit_code(gc, cb, sub(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, jle(gc, ok));
cb = emit_code(gc, cb, mov(gc, eax, value(gc, 16)));
ok->spec.label.where = cb; resolve(gc, ok, cb);
cb = emit_code(gc, cb, mov(gc, fptr->qit, eax));
	  }
cb = pcstep(gc, cb, tr, wl2, hl2, fptr->qit, fptr);

cb = emit_code(gc, cb, mov(gc, eax, fptr->ist));
cb = emit_code(gc, cb, mov(gc, fptr->st, eax));
cb = emit_code(gc, cb, mov(gc, eax, fptr->qit));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, eax, fptr->qis));
cb = emit_code(gc, cb, mov(gc, fptr->ist, eax));

cb = pcstepnext(gc, cb, tr, wl2, hl2, fptr);

cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));
      } else {
cb = emit_code(gc, cb, mov(gc, eax, fptr->st));
cb = emit_code(gc, cb, mov(gc, ebx, fptr->dstdx2));
cb = emit_code(gc, cb, add(gc, eax, ebx));
cb = emit_code(gc, cb, mov(gc, fptr->st, eax));
      }
cb = emit_code(gc, cb, jmp(gc, perpixel_reload));
    }

  if (interpolate_z) {
cb = emit_code(gc, cb, jmp(gc, allover));
zfail->spec.label.where = cb; resolve(gc, zfail, cb);
    if (interpolate_color && (modeFlags & __GL_SHADE_SMOOTH)) {
      if (spill_rgb) {
cb = emit_code(gc, cb, mov(gc, eax, tr(r)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(g)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(b)));
      }

cb = emit_code(gc, cb, add(gc, eax, tr(drdx)));
cb = emit_code(gc, cb, add(gc, ebx, tr(dgdx)));
cb = emit_code(gc, cb, add(gc, ecx, tr(dbdx)));

      if (spill_rgb) {
cb = emit_code(gc, cb, mov(gc, tr(r), eax));
cb = emit_code(gc, cb, mov(gc, tr(g), ebx));
cb = emit_code(gc, cb, mov(gc, tr(b), ecx));
      }
    }

  if (modeFlags & __GL_SHADE_TEXTURE) {
cb = emit_code(gc, cb, mov(gc, edx, fptr->st));
cb = emit_code(gc, cb, mov(gc, ebp, fptr->dstdx));
cb = emit_code(gc, cb, add(gc, edx, ebp));
cb = emit_code(gc, cb, mov(gc, fptr->st, edx));
  }

    if (modeFlags & __GL_SHADE_DITHER) {
cb = emit_code(gc, cb, mov(gc, edi, dither_index));
cb = emit_code(gc, cb, inc(gc, edi));
cb = emit_code(gc, cb, and(gc, edi, value(gc, 3)));
cb = emit_code(gc, cb, mov(gc, dither_index, edi));
    }

cb = emit_code(gc, cb, mov(gc, edi, tr(cp)));
cb = emit_code(gc, cb, add(gc, edi, tr(dcpdx)));
cb = emit_code(gc, cb, mov(gc, tr(cp), edi));

cb = emit_code(gc, cb, dec(gc, w));
cb = emit_code(gc, cb, jne(gc, perpixel));
    if (modeFlags & __GL_SHADE_TEXTURE)
cb = emit_code(gc, cb, jmp(gc, adjuster));

  }

allover->spec.label.where = cb; resolve(gc, allover, cb);

  if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fstp(gc, st0));
  }

  return cb;
}

STATIC unsigned char*
set_dither(__GLcontext* gc,
	   unsigned char* cb,
	   struct celem* tr,
	   struct celem* key,
	   int spanlet)
{
cb = emit_code(gc, cb, mov(gc, eax, key));
cb = emit_code(gc, cb, mov(gc, ecx, tr(y)));

cb = emit_code(gc, cb, and(gc, eax, value(gc, DITHER_MASK)));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, DITHER_MASK)));

  /* ebx *= 35, or 0x23 */
cb = emit_code(gc, cb, mov(gc, edx, ecx));

cb = emit_code(gc, cb, add(gc, edx, edx));
cb = emit_code(gc, cb, add(gc, edx, ecx));
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, 5)));

cb = emit_code(gc, cb, add(gc, ecx, edx));
cb = emit_code(gc, cb, add(gc, ecx, eax));
  if (spanlet) {
cb = emit_code(gc, cb, mov(gc, edx, tr(ditherTable)));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, NULL, edx, ecx, 4)));
  } else {
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, (int)__glFRDitherTable), NULL, ecx, 4)));
  }
cb = emit_code(gc, cb, mov(gc, tr(dither), eax));

  return cb;
}

/********************************************
**  Rasterizer Cache
*********************************************/

/* mode bits for cache signature */
#define __OG_RASTER_SPANLETS			(1<<0)
#define __OG_RASTER_TEX_ENV_SHIFT		1	/* 3 bits */
#define __OG_RASTER_TEX_WIDTH_SHIFT		4	/* 4 bits */
#define __OG_RASTER_TEX_HEIGHT_SHIFT		8	/* 4 bits */
#define __OG_RASTER_TEX_INDEX			(1<<12)
#define __OG_RASTER_TEX_FORMAT_SHIFT		13	/* 3 bits */
#define __OG_RASTER_TEX_S_CLAMP			(1<<16)
#define __OG_RASTER_TEX_T_CLAMP			(1<<17)
#define __OG_RASTER_TEX_BORDER			(1<<18)
#define __OG_RASTER_TEX_MIN_LINEAR		(1<<19)
#define __OG_RASTER_TEX_MAX_FILTER_SHIFT	20	/* 3 bits */
#define __OG_RASTER_CLIP_SCISSOR		(1<<23)
#define __OG_RASTER_DEPTH_FUNC_SHIFT		24	/* 3 bits */
#define __OG_RASTER_DEPTH_SIZE32		(1<<27)
#define __OG_RASTER_DEPTH_WRITE			(1<<28)
#define __OG_RASTER_DOUBLESTORE			(1<<29)
#define __OG_RASTER_RHO				(1<<30)

#define MODEFLAGS_RASTERIZER_CARES_ABOUT \
    ~(__GL_SHADE_STIPPLE | __GL_SHADE_TWOSIDED | __GL_SHADE_CULL_FACE)

#define FORCE_DIRECT !MMX

/*
 * Input is a dword-aligned pointer [edi], and a count, in dwords.
 * Fill value is in eax.
 * 
 * edx, ebp, esi are unchanged.
 * 
 */
void __glGenerateMMXFill(__GLcontext *gc, unsigned char* cb)
{
    struct celem* even;
    struct celem* loop;
    struct celem* done;
    struct celem* done2;

    og_warmup(gc);

    even = near_label(gc);
    loop = near_label(gc);
    done = near_label(gc);
    done2 = near_label(gc);

cb = emit_code(gc, cb, test(gc, edi, value(gc, 7)));
cb = emit_code(gc, cb, je(gc, even));
 
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
cb = emit_code(gc, cb, add(gc, edi, value(gc, 4)));
cb = emit_code(gc, cb, dec(gc, ecx));
 
even->spec.label.where = cb; resolve(gc, even, cb);
cb = emit_code(gc, cb, mov(gc, ebx, ecx));
cb = emit_code(gc, cb, shr(gc, ecx, value(gc, 1)));
cb = emit_code(gc, cb, je(gc, done));
 
cb = emit_code(gc, cb, movd(gc, mm0, eax));
cb = emit_code(gc, cb, punpckldq(gc, mm0, mm0));
loop->spec.label.where = cb; resolve(gc, loop, cb);
cb = emit_code(gc, cb, movq(gc, mem(gc, NULL, edi, NULL, 1), mm0));
cb = emit_code(gc, cb, add(gc, edi, value(gc, 8)));
cb = emit_code(gc, cb, dec(gc, ecx));
cb = emit_code(gc, cb, jne(gc, loop));
 
done->spec.label.where = cb; resolve(gc, done, cb);
cb = emit_code(gc, cb, test(gc, ebx, value(gc, 1)));
cb = emit_code(gc, cb, je(gc, done2));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
cb = emit_code(gc, cb, add(gc, edi, value(gc, 4)));

done2->spec.label.where = cb; resolve(gc, done2, cb);
cb = emit_code(gc, cb, ret(gc));
}

#if MMX

/* 
 * This needn't go into the gc because MMX is available
 * across all contexts.  This won't hold if
 * people build mixed MMX/non-MMX multi-processor machines
 * and run multi-thread/multi-context apps.
 */

int __glIsMmx(void)
{
    static int cold = 1;
    static int r;

    if (cold) {
	int d = 0;
	char *force;

	cold = 0;

	force = getenv("__GL_FORCE_MMX");
	if (force) {
	    r = atoi(force);
	} else { 
	    __asm    push    eax
	    __asm    push    ebx
	    __asm    push    ecx
	    __asm    push    edx

	    __asm    pushfd
	    __asm    pop     eax
	    __asm    mov     ebx, eax
	    __asm    xor     eax, 200000h
	    __asm    push    eax
	    __asm    popfd

	    __asm    pushfd
	    __asm    pop     eax
	    __asm    xor     eax, ebx
	    __asm    je      no_cpuid

	    __asm    push    ebx
	    __asm    popfd

	    __asm    mov eax,1
            __asm    __emit 0x0f  /* cpuid */
            __asm    __emit 0xa2  /* cpuid */
            __asm    mov d,edx

no_cpuid:

	    __asm    pop     edx
	    __asm    pop     ecx
	    __asm    pop     ebx
	    __asm    pop     eax

	    if (d & 0x00800000)
		r = 1;
	    else
		r = 0;
        }
    }
    return r;
}

/* Determines if this machine is capable of using MMX with the gc's
 * pixel format.  Uses only state that is available during early
 * context initialization.  This assumes that gc->drawBuffer and
 * gc->front are comparable formats.
 */
int __glCanMmx(__GLcontext* gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    return (__glIsMmx() &&
	    gc->modes.rgbMode &&
	    (gc->front->buf.depth <= 16));
}

STATIC
struct celem* mxk(__GLcontext *gc, GLint v)
{
    return mxk4(gc, v, v, v, v);
}

unsigned char* mmxgobit(__GLcontext *gc,
			unsigned char* cb,
			struct celem* r,
			int from, int to)
{
    if (from < to)
cb = emit_code(gc, cb, psllw(gc, r, value(gc, (to - from))));
    else if (from > to)
cb = emit_code(gc, cb, psrlw(gc, r, value(gc, (from - to))));

    return cb;
}

struct mmxinterp {
    struct celem *didx, *istagger;
};

struct mmxcolor {
    struct celem *r;
    struct celem *g;
    struct celem *b;
    struct celem *a;
};

struct __GLmmxtri {
    struct mmxinterp r, g, b, a, s, t, z;
    struct celem *zvalue, *zresult;
    struct celem *z_skew;
    struct celem* tex_s_shift;
    struct celem* tex_t_shift;
    struct celem* tex_s_kerrect;
    struct celem* tex_t_kerrect;
    struct celem* tex_t_mask;
    struct celem* spill_z;
    struct celem* spill_r;
    struct celem* spill_g;
    struct celem* spill_b;
    struct celem* spill_a;
    struct celem* spill_1;
    struct celem* dithers[4];
    struct celem* tex_a;
    struct celem* pixcopy; /* Sometimes blending needs a copy of
			    * destination pixels. */
    struct mmxcolor src, dst;
    struct mmxcolor sf, df;
    struct celem* prev_st;
    struct celem* next_pc;
    struct celem *indexTextureBase;
};

STATIC unsigned char* 
genread(__GLcontext *gc,
	unsigned char* cb,
	struct celem* target,
	GLenum baseFormat)
{
    switch (baseFormat) {
    case GL_RGB:
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, NULL, eax, eax, 2)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, NULL, ecx, ecx, 2)));

cb = emit_code(gc, cb, movd(gc, target, mem(gc, NULL, ebp, eax, 1)));
cb = emit_code(gc, cb, punpckldq(gc, target, mem(gc, NULL, ebp, ecx, 1)));

        break;
    case GL_RGBA:
cb = emit_code(gc, cb, movd(gc, target, mem(gc, NULL, ebp, eax, 4)));
cb = emit_code(gc, cb, punpckldq(gc, target, mem(gc, NULL, ebp, ecx, 4)));
        break;
    default:
        assert(0);
    }

    return cb;
}

/* Given a blend function 'func', set 'factor' appropriately.
 * This may be just setting the variables in factor to the appropriate
 * terms, or it may involve generating a little code.
 */
STATIC unsigned char*
calc_blend_factor(__GLcontext *gc,
		  unsigned char* cb,
		  struct mmxcolor *factor,
		  struct __GLmmxtri *mmxtr,
		  GLenum func)
{
    GLboolean must_save = GL_FALSE;
    GLboolean copy_red = GL_FALSE;
    struct celem* alpha_one = mxk(gc, 0x4000);
    struct celem* alpha_oneX2 = mxk(gc, 0x8000);

    switch (func) {
    case GL_ZERO:
	factor->r = mxk(gc, 0);
	factor->g = mxk(gc, 0);
	factor->b = mxk(gc, 0);
	break;
    case GL_ONE:
	factor->r = alpha_one;
	factor->g = alpha_one;
	factor->b = alpha_one;
	break;
    case GL_DST_COLOR:
cb = emit_code(gc, cb, movq(gc, mm0, mmxtr->dst.r));
cb = emit_code(gc, cb, movq(gc, mm1, mmxtr->dst.g));
cb = emit_code(gc, cb, movq(gc, mm2, mmxtr->dst.b));

cb = emit_code(gc, cb, psrlw(gc, mm0, value(gc, 1)));
cb = emit_code(gc, cb, psrlw(gc, mm1, value(gc, 1)));
cb = emit_code(gc, cb, psrlw(gc, mm2, value(gc, 1)));

	must_save = GL_TRUE;
	break;
    case GL_SRC_COLOR:
cb = emit_code(gc, cb, movq(gc, mm0, mmxtr->src.r));
cb = emit_code(gc, cb, movq(gc, mm1, mmxtr->src.g));
cb = emit_code(gc, cb, movq(gc, mm2, mmxtr->src.b));

cb = emit_code(gc, cb, psrlw(gc, mm0, value(gc, 1)));
cb = emit_code(gc, cb, psrlw(gc, mm1, value(gc, 1)));
cb = emit_code(gc, cb, psrlw(gc, mm2, value(gc, 1)));

	must_save = GL_TRUE;
	break;
    case GL_ONE_MINUS_DST_COLOR:
cb = emit_code(gc, cb, movq(gc, mm0, alpha_oneX2));
cb = emit_code(gc, cb, movq(gc, mm1, mm0));
cb = emit_code(gc, cb, movq(gc, mm2, mm0));

cb = emit_code(gc, cb, psubw(gc, mm0, mmxtr->dst.r));
cb = emit_code(gc, cb, psubw(gc, mm1, mmxtr->dst.g));
cb = emit_code(gc, cb, psubw(gc, mm2, mmxtr->dst.b));

cb = emit_code(gc, cb, psrlw(gc, mm0, value(gc, 1)));
cb = emit_code(gc, cb, psrlw(gc, mm1, value(gc, 1)));
cb = emit_code(gc, cb, psrlw(gc, mm2, value(gc, 1)));

	must_save = GL_TRUE;
	break;
    case GL_ONE_MINUS_SRC_COLOR:
cb = emit_code(gc, cb, movq(gc, mm0, alpha_oneX2));
cb = emit_code(gc, cb, movq(gc, mm1, mm0));
cb = emit_code(gc, cb, movq(gc, mm2, mm0));

cb = emit_code(gc, cb, psubw(gc, mm0, mmxtr->src.r));
cb = emit_code(gc, cb, psubw(gc, mm1, mmxtr->src.g));
cb = emit_code(gc, cb, psubw(gc, mm2, mmxtr->src.b));

cb = emit_code(gc, cb, psrlw(gc, mm0, value(gc, 1)));
cb = emit_code(gc, cb, psrlw(gc, mm1, value(gc, 1)));
cb = emit_code(gc, cb, psrlw(gc, mm2, value(gc, 1)));

	must_save = GL_TRUE;
	break;
    case GL_SRC_ALPHA:
	factor->r = mmxtr->src.a;
	copy_red = GL_TRUE;
	break;
    case GL_ONE_MINUS_SRC_ALPHA:
cb = emit_code(gc, cb, movq(gc, mm0, alpha_one));
cb = emit_code(gc, cb, psubw(gc, mm0, mmxtr->src.a));
cb = emit_code(gc, cb, movq(gc, factor->r, mm0));
	copy_red = GL_TRUE;
	break;
    case GL_DST_ALPHA:
	factor->r = mmxtr->dst.a;
	copy_red = GL_TRUE;
	break;
    case GL_ONE_MINUS_DST_ALPHA:
cb = emit_code(gc, cb, movq(gc, mm0, alpha_one));
cb = emit_code(gc, cb, psubw(gc, mm0, mmxtr->dst.a));
cb = emit_code(gc, cb, movq(gc, factor->r, mm0));
	copy_red = GL_TRUE;
	break;
    case GL_SRC_ALPHA_SATURATE:

	/* load mm0 with 1-<dst alpha>
	 *      mm1 with <src alpha>
	 */

cb = emit_code(gc, cb, movq(gc, mm0, alpha_one));
cb = emit_code(gc, cb, psubw(gc, mm0, mmxtr->dst.a));
cb = emit_code(gc, cb, movq(gc, mm1, mmxtr->src.a));
cb = emit_code(gc, cb, movq(gc, mm2, mm0));
cb = emit_code(gc, cb, pcmpgtw(gc, mm0, mm1));
cb = emit_code(gc, cb, pand(gc, mm1, mm0));
cb = emit_code(gc, cb, pandn(gc, mm0, mm2));
cb = emit_code(gc, cb, por(gc, mm0, mm1));
cb = emit_code(gc, cb, movq(gc, factor->r, mm0));
        copy_red = GL_TRUE;
    }

    if (copy_red) {
	factor->g = factor->b = factor->r;
    }
    if (must_save) {
	/* factor.[rgb] point to some memory by default.
	 * save mm[012] there.
	 */
cb = emit_code(gc, cb, movq(gc, factor->r, mm0));
cb = emit_code(gc, cb, movq(gc, factor->g, mm1));
cb = emit_code(gc, cb, movq(gc, factor->b, mm2));
    }

    return cb;
}


/* 
 *  Predicate to identify common situations where
 *  we don't want to clamp.  This is true
 *  (1) when one term is GL_ZERO
 *  (2) when the 2 terms are complementary: ALPHA, 1-ALPHA.
 */
STATIC GLboolean
clamp_blend_factor(GLenum src, GLenum dst)
{
    if ((src == GL_ZERO) || (dst = GL_ZERO))
	return GL_FALSE;

    if ((src == GL_SRC_ALPHA) &&
	(dst == GL_ONE_MINUS_SRC_ALPHA))
	return GL_FALSE;

    if ((src == GL_ONE_MINUS_SRC_ALPHA) &&
	(dst == GL_SRC_ALPHA))
	return GL_FALSE;

    return GL_TRUE;
}

STATIC unsigned char*
genblend(__GLcontext* gc,
	 unsigned char* cb,
	 struct __GLmmxtri *mmxtr,
	 GLint c_stride,
	 struct celem* cp)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcolorBuffer* fb = &gc->frontBuffer;
    GLuint redSize = log2(fb->redMax);
    GLuint greenSize = log2(fb->greenMax);
    GLuint blueSize = log2(fb->blueMax);
    GLuint hibit;

#if DEBUG
    {
        struct celem *bad = near_label(gc);
        struct celem *good = near_label(gc);

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.raster.blendSrc), NULL, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, gc->state.raster.blendSrc)));
cb = emit_code(gc, cb, jne(gc, bad));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.raster.blendDst), NULL, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, gc->state.raster.blendDst)));
cb = emit_code(gc, cb, je(gc, good));
bad->spec.label.where = cb; resolve(gc, bad, cb);
cb = emit_code(gc, cb, int3(gc));
good->spec.label.where = cb; resolve(gc, good, cb);
    }
#endif /* DEBUG */

    /* Load up destination.  We don't currently support destination alpha,
     * so it's RGB only here.
     */
    if (c_stride == 8) {
cb = emit_code(gc, cb, movq(gc, mm0, cp));
    } else {
cb = emit_code(gc, cb, movd(gc, mm0, cp));
cb = emit_code(gc, cb, pxor(gc, mm1, mm0));
cb = emit_code(gc, cb, punpcklbw(gc, mm0, mm1));
    }

cb = emit_code(gc, cb, movq(gc, mm1, mm0));
cb = emit_code(gc, cb, movq(gc, mm2, mm0));

cb = mmxgobit(gc, cb, mm0, fb->redShift + redSize, (14));
cb = mmxgobit(gc, cb, mm1, fb->greenShift + greenSize, (14));
cb = mmxgobit(gc, cb, mm2, fb->blueShift + blueSize, (14));

cb = emit_code(gc, cb, pand(gc, mm0, mxk(gc, fb->redMax << (13 - (redSize - 1)))));
cb = emit_code(gc, cb, pand(gc, mm1, mxk(gc, fb->greenMax << (13 - (greenSize - 1)))));
cb = emit_code(gc, cb, pand(gc, mm2, mxk(gc, fb->blueMax << (13 - (blueSize - 1)))));

    if ((gc->state.raster.blendSrc == GL_ONE) &&
	(gc->state.raster.blendDst == GL_ONE)) {

cb = emit_code(gc, cb, paddsw(gc, mm0, mmxtr->src.r));
cb = emit_code(gc, cb, paddsw(gc, mm1, mmxtr->src.g));
cb = emit_code(gc, cb, paddsw(gc, mm2, mmxtr->src.b));

	hibit = 14;

    } else {
cb = emit_code(gc, cb, movq(gc, mmxtr->dst.r, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr->dst.g, mm1));
cb = emit_code(gc, cb, movq(gc, mmxtr->dst.b, mm2));
    
    	/* Now we have source and destination, calculate blend factors */
cb = calc_blend_factor(gc, cb, &mmxtr->sf, mmxtr, gc->state.raster.blendSrc);
cb = calc_blend_factor(gc, cb, &mmxtr->df, mmxtr, gc->state.raster.blendDst);
    
    	if (gc->state.raster.blendSrc == GL_ZERO) {
cb = emit_code(gc, cb, movq(gc, mm0, mmxtr->dst.r));
cb = emit_code(gc, cb, pmulhw(gc, mm0, mmxtr->df.r));
    	
cb = emit_code(gc, cb, movq(gc, mm1, mmxtr->dst.g));
cb = emit_code(gc, cb, pmulhw(gc, mm1, mmxtr->df.g));
    	
cb = emit_code(gc, cb, movq(gc, mm2, mmxtr->dst.b));
cb = emit_code(gc, cb, pmulhw(gc, mm2, mmxtr->df.b));
    
	    hibit = 12;
    	} else {
cb = emit_code(gc, cb, movq(gc, mm0, mmxtr->src.r));
cb = emit_code(gc, cb, pmulhw(gc, mm0, mmxtr->sf.r));
cb = emit_code(gc, cb, movq(gc, mm1, mmxtr->dst.r));
cb = emit_code(gc, cb, pmulhw(gc, mm1, mmxtr->df.r));
cb = emit_code(gc, cb, paddsw(gc, mm0, mm1));
    	
cb = emit_code(gc, cb, movq(gc, mm1, mmxtr->src.g));
cb = emit_code(gc, cb, pmulhw(gc, mm1, mmxtr->sf.g));
cb = emit_code(gc, cb, movq(gc, mm2, mmxtr->dst.g));
cb = emit_code(gc, cb, pmulhw(gc, mm2, mmxtr->df.g));
cb = emit_code(gc, cb, paddsw(gc, mm1, mm2));
    	
cb = emit_code(gc, cb, movq(gc, mm2, mmxtr->src.b));
cb = emit_code(gc, cb, pmulhw(gc, mm2, mmxtr->sf.b));
cb = emit_code(gc, cb, movq(gc, mm3, mmxtr->dst.b));
cb = emit_code(gc, cb, pmulhw(gc, mm3, mmxtr->df.b));
cb = emit_code(gc, cb, paddsw(gc, mm2, mm3));
    
	    hibit = 12;
    	}
    
    	/* Now we might have to clamp.
    	   This isn't free, so there are certain common situations where
    	   we don't want to clamp.
    	   (1) When one term is GL_ZERO
    	   (2) When the 2 terms are complementary: ALPHA, 1-ALPHA
    	   */
    
    	if (clamp_blend_factor(gc->state.raster.blendSrc, 
				gc->state.raster.blendDst)) {
    	    while (hibit < 14) {
cb = emit_code(gc, cb, paddsw(gc, mm0, mm0));
cb = emit_code(gc, cb, paddsw(gc, mm1, mm1));
cb = emit_code(gc, cb, paddsw(gc, mm2, mm2));
    
    		hibit++;
    	    }
    	}   
    }
	
    if (modeFlags & __GL_SHADE_DITHER) {
cb = mmxgobit(gc, cb, mm0, hibit, (14));
    	
cb = emit_code(gc, cb, paddsw(gc, mm0, mmxtr->dithers[0]));
cb = mmxgobit(gc, cb, mm1, hibit, (14));
    	
cb = emit_code(gc, cb, paddsw(gc, mm1, mmxtr->dithers[1]));
cb = mmxgobit(gc, cb, mm2, hibit, (14));
    	
cb = emit_code(gc, cb, paddsw(gc, mm2, mmxtr->dithers[2]));
    	
    	hibit = 14;
    }

cb = mmxgobit(gc, cb, mm0, hibit, fb->redShift + redSize);
cb = emit_code(gc, cb, pand(gc, mm0, mxk(gc, fb->redMax << fb->redShift)));

cb = mmxgobit(gc, cb, mm1, hibit, fb->greenShift + greenSize);
cb = emit_code(gc, cb, pand(gc, mm1, mxk(gc, fb->greenMax << fb->greenShift)));

cb = mmxgobit(gc, cb, mm2, hibit, fb->blueShift + blueSize);

cb = emit_code(gc, cb, por(gc, mm0, mm1));
cb = emit_code(gc, cb, por(gc, mm0, mm2));

    return cb;
}

STATIC GLboolean
has_alpha(GLenum baseFormat)
{
    switch (baseFormat) {
    case GL_ALPHA:
    case GL_LUMINANCE_ALPHA:
    case GL_INTENSITY:
    case GL_RGBA:
	return GL_TRUE;
    case GL_LUMINANCE:
    case GL_RGB:
	return GL_FALSE;
    default:
	assert(0);
    }
}

STATIC unsigned char*
genpix(__GLcontext* gc,
       unsigned char* cb,
       struct celem** result,
       struct celem* tr,
       struct __GLmmxtri *mmxtr,
       GLint c_stride,
       struct celem* step,
       GLint dx,
       struct celem* cp,
       GLenum texenv)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcolorBuffer* fb = &gc->frontBuffer;
    struct celem
      *rstep,
      *gstep,
      *bstep,
      *astep;
    struct celem
      *sstep,
      *tstep;
    GLuint redSize = log2(fb->redMax);
    GLuint greenSize = log2(fb->greenMax);
    GLuint blueSize = log2(fb->blueMax);
    GLboolean do_blend;
    __GLtexture *current;
    GLenum baseFormat;
    GLboolean indexed = GL_FALSE;

    if (modeFlags & __GL_SHADE_TEXTURE) {
	current = gc->texture.currentTexture;
	baseFormat = current->texelFormat;
	if (baseFormat == GL_COLOR_INDEX) {
	    /* Index texture.  Set the flag, and find the real format.
	     */
	    indexed = GL_TRUE;
	    baseFormat = current->CT.baseFormat;
	}
    }

    do_blend = (modeFlags & __GL_SHADE_BLEND) ? GL_TRUE : GL_FALSE;

    if (modeFlags & __GL_SHADE_SMOOTH) {
	if (step) {
	    rstep = mmxtr->r.didx;
	    gstep = mmxtr->g.didx;
	    bstep = mmxtr->b.didx;
	    astep = mmxtr->a.didx;
	}
    }

    if (modeFlags & __GL_SHADE_TEXTURE) {
	GLboolean cookedTexture;

	if ((modeFlags & (__GL_SHADE_BLEND|__GL_SHADE_TEXTURE|__GL_SHADE_RGB|__GL_SHADE_DITHER)) ==
	    (__GL_SHADE_TEXTURE|__GL_SHADE_RGB)) {
	    cookedTexture =
		(texenv == GL_REPLACE) &&
		(current->texelFormat != GL_COLOR_INDEX);
	} else {
	    cookedTexture = GL_FALSE;
	}

	if (step) {
	    sstep = mmxtr->s.didx;
	    tstep = mmxtr->t.didx;
	}

	/* 
	 * Build 2 pairs of offsets in mm0,mm1.
	 */

cb = emit_code(gc, cb, movq(gc, mm3, mmxtr->tex_s_shift));
cb = emit_code(gc, cb, movq(gc, mm1, mm7));
	
cb = emit_code(gc, cb, psrlw(gc, mm1, mmxtr->tex_t_shift));
cb = emit_code(gc, cb, movq(gc, mm0, mm6));
	
cb = emit_code(gc, cb, movq(gc, mm2, mm6));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm1));
	
        if (step)
cb = emit_code(gc, cb, paddw(gc, mm6, sstep));
cb = emit_code(gc, cb, punpckhwd(gc, mm2, mm1));
	
        if (step)
cb = emit_code(gc, cb, paddw(gc, mm7, tstep));
cb = emit_code(gc, cb, psrld(gc, mm0, mm3));
	
cb = emit_code(gc, cb, psrld(gc, mm2, mm3));

        if (indexed) {
cb = emit_code(gc, cb, paddd(gc, mm0, mmxtr->indexTextureBase));
cb = emit_code(gc, cb, paddd(gc, mm2, mmxtr->indexTextureBase));
	}

        if (step == eax)
cb = emit_code(gc, cb, push(gc, eax));

        if (cookedTexture) {
	    GLint texsize = c_stride / 4;

	    assert(!do_blend);

cb = emit_code(gc, cb, movd(gc, eax, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, movd(gc, ecx, mm0));
cb = emit_code(gc, cb, movq(gc, mm0, mm2));

	    // stall

	    if (texsize == 2) {
cb = emit_code(gc, cb, mov(gc, dl, mem(gc, value(gc, 0), ebp, ecx, 2)));
    
cb = emit_code(gc, cb, mov(gc, dh, mem(gc, value(gc, 1), ebp, ecx, 2)));
cb = emit_code(gc, cb, mov(gc, cl, mem(gc, value(gc, 0), ebp, eax, 2)));
    
cb = emit_code(gc, cb, shl(gc, edx, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, ch, mem(gc, value(gc, 1), ebp, eax, 2)));
    
cb = emit_code(gc, cb, movd(gc, eax, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
    
cb = emit_code(gc, cb, or(gc, edx, ecx));

cb = emit_code(gc, cb, movd(gc, ecx, mm0));
    
cb = emit_code(gc, cb, movd(gc, mm0, edx));
    
cb = emit_code(gc, cb, mov(gc, dl, mem(gc, value(gc, 0), ebp, ecx, 2)));
    
cb = emit_code(gc, cb, mov(gc, dh, mem(gc, value(gc, 1), ebp, ecx, 2)));
cb = emit_code(gc, cb, mov(gc, cl, mem(gc, value(gc, 0), ebp, eax, 2)));
    
cb = emit_code(gc, cb, shl(gc, edx, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, ch, mem(gc, value(gc, 1), ebp, eax, 2)));

cb = emit_code(gc, cb, or(gc, edx, ecx));

cb = emit_code(gc, cb, movd(gc, mm2, edx));

cb = emit_code(gc, cb, punpckldq(gc, mm0, mm2));
	    } else {
cb = emit_code(gc, cb, mov(gc, dl, mem(gc, value(gc, 0), ebp, eax, 1)));

cb = emit_code(gc, cb, mov(gc, dh, mem(gc, value(gc, 0), ebp, ecx, 1)));

cb = emit_code(gc, cb, movd(gc, eax, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));

cb = emit_code(gc, cb, movd(gc, ecx, mm0));
cb = emit_code(gc, cb, pxor(gc, mm1, mm1));

cb = emit_code(gc, cb, mov(gc, al, mem(gc, value(gc, 0), ebp, eax, 1)));
cb = emit_code(gc, cb, and(gc, edx, value(gc, 0xffff)));

cb = emit_code(gc, cb, mov(gc, ah, mem(gc, value(gc, 0), ebp, ecx, 1)));

cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));

cb = emit_code(gc, cb, or(gc, edx, eax));

cb = emit_code(gc, cb, movd(gc, mm0, edx));

cb = emit_code(gc, cb, punpcklbw(gc, mm0, mm1));
	    }

            if (step == eax)
cb = emit_code(gc, cb, pop(gc, eax));
            else if (modeFlags & __GL_SHADE_TEXTURE_PERSP)
cb = emit_code(gc, cb, mov(gc, eax, mmxtr->next_pc));
        } else {
	    GLuint hibit;

	    /* Be warned, the rules for handling glTexEnv with the
	     * various texture formats are somewhat arcane...
	     */

	    if (modeFlags & __GL_SHADE_DEPTH_TEST)
cb = emit_code(gc, cb, movq(gc, mmxtr->spill_z, mm4));

cb = emit_code(gc, cb, movd(gc, eax, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, movd(gc, ecx, mm0));
cb = emit_code(gc, cb, movq(gc, mm0, mm2));

	    if (indexed) {
cb = emit_code(gc, cb, mov(gc, al, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, cl, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 255)));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, 255)));
	    }

            switch (baseFormat) {
	    case GL_LUMINANCE:
	    case GL_INTENSITY:
cb = emit_code(gc, cb, mov(gc, dl, mem(gc, NULL, ebp, eax, 1)));
cb = emit_code(gc, cb, mov(gc, dh, mem(gc, NULL, ebp, ecx, 1)));
cb = emit_code(gc, cb, shl(gc, edx, value(gc, 16)));

cb = emit_code(gc, cb, movd(gc, eax, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, movd(gc, ecx, mm0));

	    	if (indexed) {
cb = emit_code(gc, cb, mov(gc, al, mem(gc, NULL, edx, eax, 1)));
cb = emit_code(gc, cb, mov(gc, cl, mem(gc, NULL, edx, ecx, 1)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 255)));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, 255)));
	    	}   

cb = emit_code(gc, cb, mov(gc, dl, mem(gc, NULL, ebp, eax, 1)));
cb = emit_code(gc, cb, mov(gc, dh, mem(gc, NULL, ebp, ecx, 1)));
cb = emit_code(gc, cb, rol(gc, edx, value(gc, 16)));

cb = emit_code(gc, cb, movd(gc, mm0, edx));
cb = emit_code(gc, cb, punpcklbw(gc, mm0, mm0));
 		if (step == eax)
cb = emit_code(gc, cb, pop(gc, eax));
                else if (modeFlags & __GL_SHADE_TEXTURE_PERSP)
cb = emit_code(gc, cb, mov(gc, eax, mmxtr->next_pc));
cb = emit_code(gc, cb, psrlw(gc, mm0, value(gc, 1)));

cb = emit_code(gc, cb, movq(gc, mm1, mm0));
cb = emit_code(gc, cb, movq(gc, mm2, mm0));
		if (do_blend && has_alpha(baseFormat)) {
cb = emit_code(gc, cb, movq(gc, mm3, mm0));
cb = emit_code(gc, cb, psrlw(gc, mm3, value(gc, 1)));
cb = emit_code(gc, cb, movq(gc, mmxtr->tex_a, mm3));
		}

		break;
	    case GL_LUMINANCE_ALPHA:
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, NULL, ebp, ecx, 2)));
cb = emit_code(gc, cb, shl(gc, edx, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, NULL, ebp, eax, 2)));
cb = emit_code(gc, cb, movd(gc, mm3, edx));

cb = emit_code(gc, cb, movd(gc, eax, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, movd(gc, ecx, mm0));

	    	if (indexed) {
cb = emit_code(gc, cb, mov(gc, al, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, cl, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 255)));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, 255)));
	    	}   

cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, NULL, ebp, ecx, 2)));
cb = emit_code(gc, cb, shl(gc, edx, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, NULL, ebp, eax, 2)));
cb = emit_code(gc, cb, movd(gc, mm2, edx));

		/* Now we have luminance-alpha values in the low
		 * halves of mm2,mm3.  Luminance is in the low byte of
		 * each word, alpha is in the high byte. */

cb = emit_code(gc, cb, punpckldq(gc, mm3, mm2));
cb = emit_code(gc, cb, movq(gc, mm0, mm3));
cb = emit_code(gc, cb, psrlw(gc, mm3, value(gc, 2)));
cb = emit_code(gc, cb, psllw(gc, mm0, value(gc, 7)));
cb = emit_code(gc, cb, pand(gc, mm3, mxk(gc, 0x3fc0)));
cb = emit_code(gc, cb, pand(gc, mm0, mxk(gc, 0x7f80)));

 		if (step == eax)
cb = emit_code(gc, cb, pop(gc, eax));
                else if (modeFlags & __GL_SHADE_TEXTURE_PERSP)
cb = emit_code(gc, cb, mov(gc, eax, mmxtr->next_pc));

cb = emit_code(gc, cb, movq(gc, mm1, mm0));
cb = emit_code(gc, cb, movq(gc, mm2, mm0));
		if (do_blend) {
cb = emit_code(gc, cb, movq(gc, mmxtr->tex_a, mm3));
		}

		break;
            case GL_RGB:
            case GL_RGBA:
cb = genread(gc, cb, mm3, baseFormat);

cb = emit_code(gc, cb, movd(gc, eax, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, movd(gc, ecx, mm0));

	    	if (indexed) {
cb = emit_code(gc, cb, mov(gc, al, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, cl, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 255)));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, 255)));
	    	}   

cb = genread(gc, cb, mm4, baseFormat);
    
 		if (step == eax)
cb = emit_code(gc, cb, pop(gc, eax));
                else if (modeFlags & __GL_SHADE_TEXTURE_PERSP)
cb = emit_code(gc, cb, mov(gc, eax, mmxtr->next_pc));
    
cb = emit_code(gc, cb, lea(gc, ecx, mxk4(gc, 0x7f80, 0x0, 0x7f80, 0x0)));

		/* Extract alpha from texture */
		if (do_blend && has_alpha(baseFormat)) {
cb = emit_code(gc, cb, movq(gc, mm0, mm3));
cb = emit_code(gc, cb, movq(gc, mm1, mm4));
cb = emit_code(gc, cb, psrld(gc, mm0, value(gc, 18)));
cb = emit_code(gc, cb, psrld(gc, mm1, value(gc, 18)));
cb = emit_code(gc, cb, pand(gc, mm0, mxk4(gc, 0x3fc0, 0x0, 0x3fc0, 0x0)));
cb = emit_code(gc, cb, pand(gc, mm1, mxk4(gc, 0x3fc0, 0x0, 0x3fc0, 0x0)));
cb = emit_code(gc, cb, packssdw(gc, mm0, mm1));
cb = emit_code(gc, cb, movq(gc, mmxtr->tex_a, mm0));
		}

cb = emit_code(gc, cb, movq(gc, mm0, mm3));
cb = emit_code(gc, cb, movq(gc, mm1, mm4));
cb = emit_code(gc, cb, pslld(gc, mm0, value(gc, 7)));
cb = emit_code(gc, cb, pslld(gc, mm1, value(gc, 7)));
cb = emit_code(gc, cb, pand(gc, mm0, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, pand(gc, mm1, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, packssdw(gc, mm0, mm1));

cb = emit_code(gc, cb, movq(gc, mm1, mm3));
cb = emit_code(gc, cb, movq(gc, mm2, mm4));
cb = emit_code(gc, cb, psrld(gc, mm1, value(gc, 1)));
cb = emit_code(gc, cb, psrld(gc, mm2, value(gc, 1)));
cb = emit_code(gc, cb, pand(gc, mm1, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, pand(gc, mm2, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, packssdw(gc, mm1, mm2));

cb = emit_code(gc, cb, psrld(gc, mm3, value(gc, 9)));
cb = emit_code(gc, cb, psrld(gc, mm4, value(gc, 9)));
cb = emit_code(gc, cb, pand(gc, mm3, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, pand(gc, mm4, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, movq(gc, mm2, mm3));
cb = emit_code(gc, cb, packssdw(gc, mm2, mm4));
	        break;
	    default:
		assert(0);
            }

	    if (texenv == GL_MODULATE) {
		/* Texture with alpha component, modulate alpha with
                   fragment alpha */
		if (do_blend) {
		    if (has_alpha(baseFormat)) {
cb = emit_code(gc, cb, movq(gc, mm3, mmxtr->tex_a));
cb = emit_code(gc, cb, pmulhw(gc, mm3, mmxtr->spill_a));
cb = emit_code(gc, cb, psllw(gc, mm3, value(gc, 2)));
cb = emit_code(gc, cb, movq(gc, mmxtr->tex_a, mm3));
		    }
		}
                if (modeFlags & __GL_SHADE_SMOOTH) {
cb = emit_code(gc, cb, movq(gc, mm3, mmxtr->spill_g));
cb = emit_code(gc, cb, pmulhw(gc, mm0, mm5));
    
		    if (step)
cb = emit_code(gc, cb, paddw(gc, mm5, rstep));
cb = emit_code(gc, cb, pmulhw(gc, mm1, mm3));
    
cb = emit_code(gc, cb, movq(gc, mm4, mmxtr->spill_b));
cb = emit_code(gc, cb, pmulhw(gc, mm2, mm4));

 		    if (step) {
cb = emit_code(gc, cb, paddw(gc, mm3, gstep));
cb = emit_code(gc, cb, paddw(gc, mm4, bstep));
cb = emit_code(gc, cb, movq(gc, mmxtr->spill_g, mm3));
cb = emit_code(gc, cb, movq(gc, mmxtr->spill_b, mm4));
                    }
                } else {
cb = emit_code(gc, cb, pmulhw(gc, mm0, mmxtr->spill_r));
cb = emit_code(gc, cb, pmulhw(gc, mm1, mmxtr->spill_g));
cb = emit_code(gc, cb, pmulhw(gc, mm2, mmxtr->spill_b));
                }

                hibit = 13;
            } else {
		hibit = 14;
	    }

	    if (!do_blend) {
            	if (modeFlags & __GL_SHADE_DITHER) {
            	     if (hibit == 13) {
cb = emit_code(gc, cb, psllw(gc, mm0, value(gc, 1)));
    
cb = emit_code(gc, cb, paddsw(gc, mm0, mmxtr->dithers[0]));
cb = emit_code(gc, cb, psllw(gc, mm1, value(gc, 1)));
    
cb = emit_code(gc, cb, paddsw(gc, mm1, mmxtr->dithers[1]));
cb = emit_code(gc, cb, psllw(gc, mm2, value(gc, 1)));
    
cb = emit_code(gc, cb, paddsw(gc, mm2, mmxtr->dithers[2]));
    
			 hibit = 14;
		     } else {
cb = emit_code(gc, cb, paddsw(gc, mm0, mmxtr->dithers[0]));
cb = emit_code(gc, cb, paddsw(gc, mm1, mmxtr->dithers[1]));
cb = emit_code(gc, cb, paddsw(gc, mm2, mmxtr->dithers[2]));
		     }
	    	}
    
cb = emit_code(gc, cb, pand(gc, mm1, mxk(gc, fb->greenMax << (hibit - greenSize))));
cb = mmxgobit(gc, cb, mm0, hibit, fb->redShift + redSize);
    	
cb = emit_code(gc, cb, pand(gc, mm0, mxk(gc, fb->redMax << fb->redShift)));
cb = mmxgobit(gc, cb, mm2, hibit, fb->blueShift + blueSize);
    
cb = mmxgobit(gc, cb, mm1, hibit, fb->greenShift + greenSize);
cb = emit_code(gc, cb, por(gc, mm0, mm2));

cb = emit_code(gc, cb, por(gc, mm0, mm1));
	    } else {
cb = mmxgobit(gc, cb, mm0, hibit, (14));
cb = mmxgobit(gc, cb, mm1, hibit, (14));
cb = mmxgobit(gc, cb, mm2, hibit, (14));

cb = emit_code(gc, cb, movq(gc, mmxtr->src.r, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr->src.g, mm1));
cb = emit_code(gc, cb, movq(gc, mmxtr->src.b, mm2));

		/* If the texture format has alpha, use it; otherwise
		   take alpha from the fragment.  */

		if (has_alpha(baseFormat))
		    mmxtr->src.a = mmxtr->tex_a;     /* At */
		else
		    mmxtr->src.a = mmxtr->spill_a;   /* Af */

cb = genblend(gc, cb, mmxtr, c_stride, cp);
                if ((modeFlags & __GL_SHADE_SMOOTH) && step) {
cb = emit_code(gc, cb, movq(gc, mm3, mmxtr->spill_a));
cb = emit_code(gc, cb, paddw(gc, mm3, astep));
cb = emit_code(gc, cb, movq(gc, mmxtr->spill_a, mm3));
	        }
	    }

	    if (modeFlags & __GL_SHADE_DEPTH_TEST)
cb = emit_code(gc, cb, movq(gc, mm4, mmxtr->spill_z));
        }

	*result = mm0;
    } else {
	if ((modeFlags & __GL_SHADE_SMOOTH) == 0) {
	    if (!do_blend) {
cb = emit_code(gc, cb, movq(gc, mm0, mm7));
	    } else {
		/* The colors are already present in spill_[rgb] */
		mmxtr->src.r = mmxtr->spill_r;
		mmxtr->src.g = mmxtr->spill_g;
		mmxtr->src.b = mmxtr->spill_b;

		mmxtr->src.a = mmxtr->spill_a;

cb = genblend(gc, cb, mmxtr, c_stride, cp);
	    }

	    *result = mm0;
 	} else {
	    if (!do_blend) {
 	    	if (step)
cb = emit_code(gc, cb, movq(gc, mm3, gstep));
cb = emit_code(gc, cb, movq(gc, mm0, mm5));
    
cb = mmxgobit(gc, cb, mm0, (14), fb->redShift + redSize);
cb = emit_code(gc, cb, movq(gc, mm1, mm6));
    
cb = emit_code(gc, cb, pand(gc, mm0, mxk(gc, fb->redMax << fb->redShift)));
cb = mmxgobit(gc, cb, mm1, (14), fb->greenShift + greenSize);
    
cb = emit_code(gc, cb, pand(gc, mm1, mxk(gc, fb->greenMax << fb->greenShift)));
cb = emit_code(gc, cb, movq(gc, mm2, mm7));
    
cb = mmxgobit(gc, cb, mm2, (14), fb->blueShift + blueSize);
cb = emit_code(gc, cb, por(gc, mm0, mm1));
    
	    	if (step)
cb = emit_code(gc, cb, paddw(gc, mm5, rstep));
cb = emit_code(gc, cb, por(gc, mm0, mm2));
    
 	    	if (step) {
cb = emit_code(gc, cb, paddw(gc, mm7, bstep));
cb = emit_code(gc, cb, paddw(gc, mm6, mm3));
 	    	}
	    } else {
		mmxtr->src.r = mm5;
		mmxtr->src.g = mm6;
		mmxtr->src.b = mm7;
		mmxtr->src.a = mmxtr->spill_a;

cb = genblend(gc, cb, mmxtr, c_stride, cp);

	    	if (step) {
cb = emit_code(gc, cb, movq(gc, mm1, mmxtr->spill_a));
cb = emit_code(gc, cb, paddw(gc, mm1, astep));
cb = emit_code(gc, cb, paddw(gc, mm5, rstep));
cb = emit_code(gc, cb, paddw(gc, mm6, gstep));
cb = emit_code(gc, cb, paddw(gc, mm7, bstep));
cb = emit_code(gc, cb, movq(gc, mmxtr->spill_a, mm1));
		}
	    }
    
	    *result = mm0;
 	}
    }

    return cb;
}

STATIC unsigned char* 
genstep(__GLcontext* gc,
	unsigned char* cb,
	struct celem* tr,
	struct __GLmmxtri *mmxtr,
	struct celem* step,
	GLint dx,
	GLenum texenv)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcolorBuffer* fb = &gc->frontBuffer;
    struct celem
	*rstep,
	*gstep,
	*bstep,
	*astep,
	*sstep,
	*tstep;

    if (step) {
	rstep = mmxtr->r.didx;
	gstep = mmxtr->g.didx;
	bstep = mmxtr->b.didx;
	astep = mmxtr->a.didx;
	sstep = mmxtr->s.didx;
	tstep = mmxtr->t.didx;

    	if ((modeFlags & __GL_SHADE_SMOOTH) &&
	    !((modeFlags & __GL_SHADE_TEXTURE) &&
	      (texenv == GL_REPLACE))) {
cb = emit_code(gc, cb, paddw(gc, mm5, rstep));
	    if ((modeFlags & __GL_SHADE_TEXTURE) &&
		(texenv == GL_MODULATE)) {
cb = emit_code(gc, cb, movq(gc, mm0, mmxtr->spill_g));
cb = emit_code(gc, cb, movq(gc, mm1, mmxtr->spill_b));
cb = emit_code(gc, cb, paddw(gc, mm0, gstep));
cb = emit_code(gc, cb, paddw(gc, mm1, bstep));
cb = emit_code(gc, cb, movq(gc, mmxtr->spill_g, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr->spill_b, mm1));
    	    } else {
cb = emit_code(gc, cb, paddw(gc, mm6, gstep));
cb = emit_code(gc, cb, paddw(gc, mm7, bstep));
    	    }
    	}
	if ((modeFlags & __GL_SHADE_BLEND) &&
	    (modeFlags & __GL_SHADE_SMOOTH)) {
cb = emit_code(gc, cb, movq(gc, mm0, mmxtr->spill_a));
cb = emit_code(gc, cb, paddw(gc, mm0, astep));
cb = emit_code(gc, cb, movq(gc, mmxtr->spill_a, mm0));
	}

    	if (modeFlags & __GL_SHADE_TEXTURE) {
cb = emit_code(gc, cb, paddw(gc, mm6, sstep));
cb = emit_code(gc, cb, paddw(gc, mm7, tstep));
    	}
    }
    	    
    return cb;
}

/* 
 * Preshift is shift of inital Z value
 * alive is which results matter
 *
 * Writes Z value
 */

STATIC unsigned char*
genz(__GLcontext *gc,
     unsigned char* cb,
     struct celem* tr,
     struct __GLmmxtri *mmxtr,
     struct celem* cp,
     struct celem* cp_read,
     struct celem* zp,
     struct celem* step,
     GLint dx,
     struct celem* alive,
     struct celem* win,
     struct celem* lose,
     struct celem* mixpix,
     GLenum texenv)
{
    struct celem* dostep = near_label(gc);
    struct celem* zstep = NULL;

    if (step) {
	zstep = mmxtr->z.didx;
    }

    if (gc->depthBuffer.buf.depth == 16) {
cb = emit_code(gc, cb, movq(gc, mm0, mm4));
cb = emit_code(gc, cb, movq(gc, mm2, mm4));

	if (dx == 1)
cb = emit_code(gc, cb, paddd(gc, mm2, mmxtr->z_skew));
	else
cb = emit_code(gc, cb, paddd(gc, mm0, mmxtr->z_skew));

cb = emit_code(gc, cb, pand(gc, mm2, mxk4(gc, 0, ~0, 0, ~0)));
cb = emit_code(gc, cb, psrld(gc, mm0, value(gc, 16)));

 	if (gc->state.depth.testFunc != GL_ALWAYS)
cb = emit_code(gc, cb, movq(gc, mm1, zp));
cb = emit_code(gc, cb, por(gc, mm0, mm2));

 	/*
 	 * Input:
 	 *     mm0   Generated Z value
 	 *     mm1   Z-buffer value
 	 *     mm2   scratch
 	 *
 	 * Output:
 	 *     mm0   Generated Z value
 	 *     mm1   Success mask: 0x0000 means lose, 0xffff means win
 	 *     mm2   scratch
 	 */

 	switch (gc->state.depth.testFunc) {
 	case GL_NEVER:
cb = emit_code(gc, cb, pxor(gc, mm1, mm1));
 	    break;
 	case GL_LESS:
cb = emit_code(gc, cb, movq(gc, mm2, mm0));
cb = emit_code(gc, cb, movq(gc, mm3, mm0));
cb = emit_code(gc, cb, psubw(gc, mm2, mm1));
cb = emit_code(gc, cb, psubusw(gc, mm3, mm1));
cb = emit_code(gc, cb, pcmpeqw(gc, mm2, mm3));
cb = emit_code(gc, cb, pcmpeqw(gc, mm1, mm1));
cb = emit_code(gc, cb, pxor(gc, mm1, mm2));
 	    break;
 	case GL_EQUAL:
cb = emit_code(gc, cb, pcmpeqw(gc, mm1, mm0));
	    break;
 	case GL_LEQUAL:
cb = emit_code(gc, cb, movq(gc, mm2, mm1));
cb = emit_code(gc, cb, psubw(gc, mm1, mm0));
cb = emit_code(gc, cb, psubusw(gc, mm2, mm0));
cb = emit_code(gc, cb, pcmpeqw(gc, mm1, mm2));
 	    break;
 	case GL_GREATER:
cb = emit_code(gc, cb, movq(gc, mm2, mm1));
cb = emit_code(gc, cb, psubw(gc, mm1, mm0));
cb = emit_code(gc, cb, psubusw(gc, mm2, mm0));
cb = emit_code(gc, cb, pcmpeqw(gc, mm1, mm2));
cb = emit_code(gc, cb, pcmpeqw(gc, mm2, mm2));
cb = emit_code(gc, cb, pxor(gc, mm1, mm2));
 	    break;
 	case GL_NOTEQUAL:
cb = emit_code(gc, cb, pcmpeqw(gc, mm1, mm0));
cb = emit_code(gc, cb, pcmpeqw(gc, mm2, mm2));
cb = emit_code(gc, cb, pxor(gc, mm1, mm2));
 	    break;
 	case GL_GEQUAL:
cb = emit_code(gc, cb, movq(gc, mm2, mm0));
cb = emit_code(gc, cb, movq(gc, mm3, mm0));
cb = emit_code(gc, cb, psubw(gc, mm2, mm1));
cb = emit_code(gc, cb, psubusw(gc, mm3, mm1));
cb = emit_code(gc, cb, pcmpeqw(gc, mm2, mm3));
cb = emit_code(gc, cb, movq(gc, mm1, mm2));
 	    break;
 	}

 	/* Now 
 	 * mm0 is new Z
 	 * mm1 is result of comparison
 	 * [esi] is original Z
 	 */

 	/* 0x00 means lose, 0xff means win */

 	/* Mask to stop us winning over the edges */
 	if (alive) {
cb = emit_code(gc, cb, movd(gc, mm2, alive));
cb = emit_code(gc, cb, punpcklbw(gc, mm2, mm2));
 	    if (gc->state.depth.testFunc != GL_ALWAYS)
cb = emit_code(gc, cb, pand(gc, mm1, mm2));
	    else
cb = emit_code(gc, cb, movq(gc, mm1, mm2));
 	}

 	if ((gc->state.depth.testFunc != GL_ALWAYS) ||
	    alive) {
cb = emit_code(gc, cb, movq(gc, mm2, mm1));
cb = emit_code(gc, cb, packsswb(gc, mm1, mm1));

            if (gc->state.depth.writeEnable) {
cb = emit_code(gc, cb, pand(gc, mm0, mm2));
    
cb = emit_code(gc, cb, pandn(gc, mm2, zp));
	    }
    
cb = emit_code(gc, cb, movd(gc, ecx, mm1));
            if (gc->state.depth.writeEnable)
cb = emit_code(gc, cb, por(gc, mm0, mm2));

 	    if (step)
cb = emit_code(gc, cb, paddd(gc, mm4, zstep));

            if (gc->state.depth.writeEnable)
cb = emit_code(gc, cb, movq(gc, zp, mm0));

cb = emit_code(gc, cb, test(gc, ecx, ecx));
cb = emit_code(gc, cb, je(gc, dostep));
 	} else {
 	    if (step)
cb = emit_code(gc, cb, paddd(gc, mm4, zstep));

cb = emit_code(gc, cb, movq(gc, zp, mm0));
cb = emit_code(gc, cb, jmp(gc, win));
        }
    } else {
	struct celem* zp8;

	zp8 = mem(gc, value(gc, 8), zp->spec.mem.reg1, zp->spec.mem.reg2, zp->spec.mem.scale);

	/* Form a new quadruplet of Z-values in mm3,mm4 */

cb = emit_code(gc, cb, movq(gc, mm3, mm4));

	if (dx == -1)
cb = emit_code(gc, cb, paddd(gc, mm3, mmxtr->z_skew));
	else
cb = emit_code(gc, cb, paddd(gc, mm4, mmxtr->z_skew));

 	switch (gc->state.depth.testFunc) {
 	case GL_ALWAYS:
cb = emit_code(gc, cb, pcmpeqd(gc, mm0, mm0));
cb = emit_code(gc, cb, pcmpeqd(gc, mm1, mm1));
 	    break;

 	case GL_NEVER:
cb = emit_code(gc, cb, pxor(gc, mm0, mm0));
cb = emit_code(gc, cb, pxor(gc, mm1, mm1));
 	    break;

 	case GL_LESS:
cb = emit_code(gc, cb, movq(gc, mm0, zp));
cb = emit_code(gc, cb, pcmpgtd(gc, mm0, mm3));
cb = emit_code(gc, cb, movq(gc, mm1, zp8));
cb = emit_code(gc, cb, pcmpgtd(gc, mm1, mm4));
 	    break;

 	case GL_EQUAL:
cb = emit_code(gc, cb, movq(gc, mm0, zp));
cb = emit_code(gc, cb, pcmpeqd(gc, mm0, mm3));
cb = emit_code(gc, cb, movq(gc, mm1, zp8));
cb = emit_code(gc, cb, pcmpeqd(gc, mm1, mm4));
	    break;

 	case GL_LEQUAL:
cb = emit_code(gc, cb, movq(gc, mm0, mm3));
cb = emit_code(gc, cb, pcmpgtd(gc, mm0, zp));
cb = emit_code(gc, cb, movq(gc, mm1, mm4));
cb = emit_code(gc, cb, pcmpgtd(gc, mm1, zp8));

cb = emit_code(gc, cb, pcmpeqw(gc, mm2, mm2));
cb = emit_code(gc, cb, pxor(gc, mm0, mm2));
cb = emit_code(gc, cb, pxor(gc, mm1, mm2));

 	    break;
 	case GL_GREATER:
cb = emit_code(gc, cb, movq(gc, mm0, mm3));
cb = emit_code(gc, cb, pcmpgtd(gc, mm0, zp));
cb = emit_code(gc, cb, movq(gc, mm1, mm4));
cb = emit_code(gc, cb, pcmpgtd(gc, mm1, zp8));
 	    break;

 	case GL_NOTEQUAL:
cb = emit_code(gc, cb, movq(gc, mm0, zp));
cb = emit_code(gc, cb, pcmpeqd(gc, mm0, mm3));
cb = emit_code(gc, cb, movq(gc, mm1, zp8));
cb = emit_code(gc, cb, pcmpeqd(gc, mm1, mm4));

cb = emit_code(gc, cb, pcmpeqw(gc, mm2, mm2));
cb = emit_code(gc, cb, pxor(gc, mm0, mm2));
cb = emit_code(gc, cb, pxor(gc, mm1, mm2));
 	    break;

 	case GL_GEQUAL:
cb = emit_code(gc, cb, movq(gc, mm0, zp));
cb = emit_code(gc, cb, pcmpgtd(gc, mm0, mm3));
cb = emit_code(gc, cb, movq(gc, mm1, zp8));
cb = emit_code(gc, cb, pcmpgtd(gc, mm1, mm4));
cb = emit_code(gc, cb, pcmpeqw(gc, mm2, mm2));
cb = emit_code(gc, cb, pxor(gc, mm0, mm2));
cb = emit_code(gc, cb, pxor(gc, mm1, mm2));
 	    break;
 	}

cb = emit_code(gc, cb, movq(gc, mm2, mm0));
cb = emit_code(gc, cb, packssdw(gc, mm2, mm1));

 	/* Now 
 	 * mm3,4 is new Z
	 * mm0,mm1 are masks
 	 * mm2 is result of comparison
 	 * [esi] is original Z
 	 */

 	/* 0x00 means lose, 0xff means win */

 	/* Mask to stop us winning over the edges */
 	if (alive) {
cb = emit_code(gc, cb, movd(gc, mm1, alive));
cb = emit_code(gc, cb, punpcklbw(gc, mm1, mm1));
cb = emit_code(gc, cb, pand(gc, mm2, mm1));
 	}
cb = emit_code(gc, cb, movq(gc, mm1, mm2));
cb = emit_code(gc, cb, packsswb(gc, mm2, mm2));
cb = emit_code(gc, cb, movd(gc, ecx, mm2));

        if (gc->state.depth.writeEnable) {
cb = emit_code(gc, cb, movq(gc, mm0, mm1));
cb = emit_code(gc, cb, punpckhwd(gc, mm1, mm1));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));

cb = emit_code(gc, cb, pand(gc, mm3, mm0));
cb = emit_code(gc, cb, pandn(gc, mm0, zp));
cb = emit_code(gc, cb, por(gc, mm0, mm3));
cb = emit_code(gc, cb, movq(gc, zp, mm0));

cb = emit_code(gc, cb, movq(gc, mm2, mm4));
cb = emit_code(gc, cb, pand(gc, mm2, mm1));
cb = emit_code(gc, cb, pandn(gc, mm1, zp8));
cb = emit_code(gc, cb, por(gc, mm1, mm2));
cb = emit_code(gc, cb, movq(gc, zp8, mm1));
	}

 	if (step) {
	    if (dx == -1)
cb = emit_code(gc, cb, paddd(gc, mm4, mmxtr->z.didx));
	    else
cb = emit_code(gc, cb, paddd(gc, mm4, mmxtr->z_skew));
	}

cb = emit_code(gc, cb, test(gc, ecx, ecx));
cb = emit_code(gc, cb, je(gc, dostep));

cb = emit_code(gc, cb, movd(gc, mm1, ecx));
    }

    if (gc->polygon.shader.modeFlags & __GL_SHADE_MASK) {
	/* Do nothing, just fall through to dostep and the Z lose route. */
    } else {
    	 if (alive) {
cb = emit_code(gc, cb, mov(gc, edx, alive));
cb = emit_code(gc, cb, cmp(gc, ecx, edx));
cb = emit_code(gc, cb, je(gc, win));
    	 } else {
cb = emit_code(gc, cb, cmp(gc, ecx, value(gc, ~0)));
cb = emit_code(gc, cb, je(gc, win));
    	 }
     
    	  /* We're mixed */
    	  if (step == eax)
cb = emit_code(gc, cb, mov(gc, mmxtr->spill_1, eax));
     
	  /* 'mixpix' takes it color pointer in edi.  If we're
	   * blending, short spans have a different cp and cp_read: so
	   * mixpix takes a color source pointer in esi and
	   * destination in edi.
	   */

          if (gc->polygon.shader.modeFlags & __GL_SHADE_BLEND) {
cb = emit_code(gc, cb, push(gc, esi));
cb = emit_code(gc, cb, lea(gc, esi, cp_read));
cb = emit_code(gc, cb, push(gc, edi));
cb = emit_code(gc, cb, lea(gc, edi, cp));

cb = emit_code(gc, cb, call(gc, mixpix));

cb = emit_code(gc, cb, pop(gc, edi));
cb = emit_code(gc, cb, pop(gc, esi));
	  } else {
cb = emit_code(gc, cb, push(gc, edi));
cb = emit_code(gc, cb, lea(gc, edi, cp));
cb = emit_code(gc, cb, call(gc, mixpix));
cb = emit_code(gc, cb, pop(gc, edi));
	  }

    	  if (step == eax)
cb = emit_code(gc, cb, mov(gc, eax, mmxtr->spill_1));
    }

dostep->spec.label.where = cb; resolve(gc, dostep, cb);
     if (step)
cb = genstep(gc, cb, tr, mmxtr, step, dx, texenv);

    /* Need to load up eax if we're in the central section */
    if (!alive &&
	(gc->polygon.shader.modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = emit_code(gc, cb, mov(gc, eax, mmxtr->next_pc));
    }
cb = emit_code(gc, cb, jmp(gc, lose));

    return cb;
}

/* Round r towards zero, given that we intend to truncate 
 * afterwards and preserve 'keep' bits.
 */
STATIC unsigned char*
rtz(__GLcontext *gc,
    unsigned char* cb,
    struct celem* r,
    struct celem* keep)
{
cb = emit_code(gc, cb, movq(gc, mm7, r));
cb = emit_code(gc, cb, psrad(gc, mm7, value(gc, 31)));
cb = emit_code(gc, cb, psrld(gc, mm7, keep));
cb = emit_code(gc, cb, paddd(gc, r, mm7));

    return cb;
}

/* Round r towards zero, given that we intend to truncate 
 * afterwards and preserve 'keep' bits.
 */
STATIC unsigned char*
rtzw(__GLcontext *gc,
     unsigned char* cb,
     struct celem* r,
     struct celem* keep)
{
cb = emit_code(gc, cb, movq(gc, mm7, r));
cb = emit_code(gc, cb, psraw(gc, mm7, value(gc, 15)));
cb = emit_code(gc, cb, psrlw(gc, mm7, keep));
cb = emit_code(gc, cb, paddw(gc, r, mm7));

    return cb;
}

/*
 * Correct an interpolant.  Generally, we get interpolants in 16.16
 * precision.  We like to use small (16-bit) interpolators, so we have
 * so shift all values left by some amount ('shift').
 */
STATIC unsigned char*
kerrect(__GLcontext *gc,
	unsigned char* cb,
	struct celem* r0,
	struct celem* drdx,
	struct celem* drdy0,
	struct celem* drdy1,
	struct celem* shift)
{
cb = emit_code(gc, cb, movd(gc, mm0, r0));
cb = emit_code(gc, cb, pslld(gc, mm0, shift));
cb = emit_code(gc, cb, movd(gc, r0, mm0));

cb = emit_code(gc, cb, movd(gc, mm0, drdx));
cb = emit_code(gc, cb, pslld(gc, mm0, shift));
cb = emit_code(gc, cb, movd(gc, drdx, mm0));

cb = emit_code(gc, cb, movd(gc, mm0, drdy0));
cb = emit_code(gc, cb, pslld(gc, mm0, shift));
cb = emit_code(gc, cb, movd(gc, drdy0, mm0));

cb = emit_code(gc, cb, movd(gc, mm0, drdy1));
cb = emit_code(gc, cb, pslld(gc, mm0, shift));
cb = emit_code(gc, cb, movd(gc, drdy1, mm0));

     return cb;
}

STATIC unsigned char*
prepare(__GLcontext *gc,
	unsigned char* cb,
	struct celem* ending,
	struct celem* tail,
	int dx,
	GLint z_stride,
	GLint c_stride,
	GLboolean interpolate_z,
	struct __GLmmxtri* mmxtr,
	GLuint modeFlags)
{
    if (ending)
cb = emit_code(gc, cb, mov(gc, ending, ebx));

cb = emit_code(gc, cb, mov(gc, eax, ebx));

cb = emit_code(gc, cb, shr(gc, eax, value(gc, 2)));
cb = emit_code(gc, cb, xor(gc, ebx, ebx));

    if (dx == 1) {
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 0)));
cb = emit_code(gc, cb, je(gc, tail));

cb = emit_code(gc, cb, lea(gc, edi, mem(gc, NULL, edi, eax, c_stride)));
cb = emit_code(gc, cb, sub(gc, ebx, eax));

        if (interpolate_z) {
	    if (z_stride == 16)
cb = emit_code(gc, cb, add(gc, eax, eax));
cb = emit_code(gc, cb, lea(gc, esi, mem(gc, NULL, esi, eax, 8)));
        }
        if ((modeFlags & __GL_SHADE_TEXTURE) && 
	    (modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = emit_code(gc, cb, mov(gc, eax, ebx));
cb = emit_code(gc, cb, add(gc, eax, value(gc, 4)));
cb = emit_code(gc, cb, mov(gc, mmxtr->next_pc, eax));
        }
    } else {
cb = emit_code(gc, cb, sub(gc, ebx, eax));
cb = emit_code(gc, cb, lea(gc, edi, mem(gc, NULL, edi, ebx, c_stride)));
        if (interpolate_z) {
	    if (z_stride == 16)
cb = emit_code(gc, cb, add(gc, ebx, ebx));
cb = emit_code(gc, cb, lea(gc, esi, mem(gc, NULL, esi, ebx, 8)));
        }
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 0)));
cb = emit_code(gc, cb, je(gc, tail));
cb = emit_code(gc, cb, mov(gc, ebx, eax));
        if ((modeFlags & __GL_SHADE_TEXTURE) && 
	    (modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = emit_code(gc, cb, sub(gc, eax, value(gc, 4)));
cb = emit_code(gc, cb, mov(gc, mmxtr->next_pc, eax));
        }
    }

    return cb;
}

/*
 * Perspective-correct approximation in MMX.
 * 
 * Crucial to this process we need a reciprocal function: something
 * to give us 1/q, but without using floating-point.
 * 
 * First we normalize 'q'.  This is done in 'mknorm'; it builds
 * a normalization tree that binary chops through 'q' until it
 * finds the most-significant bit.  Then it shifts 'q' left and
 * records the exponent.
 *
 * Next, we look up 'q' by indexing via its high bits into a
 * reciprocal table.  Each entry contains not only (1/x), but the
 * slope of (1/x) at that point.  So we LERP between entries by
 * multiplying this slope by the fractional part of 'q'.
 * 
 * The function returns this value, which is basically a mantissa,
 * and the exponent, which is the normalization distance we found
 * in 'mknorm'.
 *
 * (If you don't know what LERP, mantissa, exponent, and normalization
 * mean, you should read up on floating-point and numerical
 * approximations.)
 */

#define MMX_TRUE_DIVIDE 0

#define L2_RECIP_SIZE 8
#define RECIP_SIZE (1 << L2_RECIP_SIZE) 

STATIC unsigned char*
mknorm(__GLcontext *gc,
       unsigned char * cb,
       GLint lo,
       GLint hi,
       struct celem* done)
{
    if (lo == hi) {
        if ((lo == 24) || (lo == 0)) {
cb = emit_code(gc, cb, mov(gc, ecx, value(gc, MMX_TRUE_DIVIDE ? 0xffffff : 0xffffffff)));
cb = emit_code(gc, cb, mov(gc, edx, value(gc, 1)));
	} else {
cb = gobit(gc, cb, ecx, (24 - lo), (MMX_TRUE_DIVIDE ? 23 : 32));
cb = emit_code(gc, cb, mov(gc, edx, value(gc, lo)));
        }
cb = emit_code(gc, cb, jmp(gc, done));
    } else {
	struct celem *less = label(gc);
	GLuint mid;

	mid = (hi + lo) / 2;
cb = emit_code(gc, cb, cmp(gc, ecx, value(gc, (0x1000000 >> mid))));
cb = emit_code(gc, cb, jl(gc, less));
cb = mknorm(gc, cb, lo, mid, done);
less->spec.label.where = cb; resolve(gc, less, cb);
cb = mknorm(gc, cb, mid + 1, hi, done);
    }

    return cb;
}

STATIC unsigned char*
reciprocal(__GLcontext *gc,
	   unsigned char * cb,
	   struct celem* q)
{
    static GLboolean cold = GL_TRUE;
    static GLuint recips[RECIP_SIZE];
    struct celem
	*ok = near_label(gc),
	*done = label(gc);

    if (cold) {
	GLint i;

	cold = GL_FALSE;
	for (i = 0 ; i < RECIP_SIZE; i++) {

#define RENT(i) ((0x8000 << L2_RECIP_SIZE) / (RECIP_SIZE + (i)))

	    recips[i] = (RENT(i) | ((2 * (RENT(i + 1) - RENT(i))) << 16));

#undef RENT

	}
    }

    if (q != ecx)
cb = emit_code(gc, cb, mov(gc, ecx, q));

cb = mknorm(gc, cb, (0), (24), done);
done->spec.label.where = cb; resolve(gc, done, cb);

#if MMX_TRUE_DIVIDE
cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, push(gc, ebx));

cb = emit_code(gc, cb, mov(gc, eax, ecx));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 0xff800000)));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 0x00800000)));
cb = emit_code(gc, cb, je(gc, ok));
cb = emit_code(gc, cb, int3(gc));
ok->spec.label.where = cb; resolve(gc, ok, cb);

cb = emit_code(gc, cb, movd(gc, mm2, edx));

cb = emit_code(gc, cb, mov(gc, edx, value(gc, 0x40)));
cb = emit_code(gc, cb, mov(gc, eax, value(gc, 0x00000000)));
cb = emit_code(gc, cb, idiv(gc, ecx));
cb = emit_code(gc, cb, movd(gc, mm0, eax));
cb = emit_code(gc, cb, packssdw(gc, mm0, mm0));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckldq(gc, mm0, mm0));

cb = emit_code(gc, cb, pop(gc, ebx));
cb = emit_code(gc, cb, pop(gc, eax));

#else
cb = emit_code(gc, cb, movd(gc, mm2, edx));

cb = emit_code(gc, cb, mov(gc, edx, ecx));

cb = emit_code(gc, cb, shr(gc, edx, value(gc, ((32 - L2_RECIP_SIZE) - 15))));

cb = emit_code(gc, cb, shr(gc, ecx, value(gc, (32 - L2_RECIP_SIZE))));
cb = emit_code(gc, cb, and(gc, edx, value(gc, 0x7fff)));

cb = emit_code(gc, cb, movd(gc, mm3, edx));

cb = emit_code(gc, cb, movd(gc, mm1, mem(gc, value(gc, (int)recips), NULL, ecx, 4)));

cb = emit_code(gc, cb, movq(gc, mm0, mm1));
cb = emit_code(gc, cb, psrld(gc, mm1, value(gc, 16)));

cb = emit_code(gc, cb, pmulhw(gc, mm1, mm3));

cb = emit_code(gc, cb, paddw(gc, mm0, mm1));

cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));

cb = emit_code(gc, cb, punpckldq(gc, mm0, mm0));

#endif

    return cb;
}

STATIC unsigned char*
mul32x16(__GLcontext *gc,
	 unsigned char * cb,
	 struct celem *dst32,
	 struct celem *src16,
	 struct celem *work1,
	 struct celem *work2)
{

cb = emit_code(gc, cb, movq(gc, work1, dst32));
cb = emit_code(gc, cb, psrlw(gc, work1, value(gc, 1)));

cb = emit_code(gc, cb, movq(gc, work2, src16));
cb = emit_code(gc, cb, pand(gc, work2, mxk4(gc, ~0, 0, ~0, 0)));

cb = emit_code(gc, cb, pand(gc, src16, mxk4(gc, 0, ~0, 0, ~0)));

cb = emit_code(gc, cb, pmaddwd(gc, dst32, src16));
cb = emit_code(gc, cb, pmaddwd(gc, work1, work2));
cb = emit_code(gc, cb, pslld(gc, dst32, value(gc, 1)));
cb = emit_code(gc, cb, psrld(gc, work1, value(gc, 14)));
cb = emit_code(gc, cb, paddd(gc, dst32, work1));

    return cb;
}

STATIC unsigned char*
mul32x16i(__GLcontext *gc,
	  unsigned char * cb,
	  struct celem *dst32,
	  struct celem *src16,
	  struct celem *work1,
	  struct celem *work2)
{
cb = emit_code(gc, cb, movq(gc, work1, dst32));
cb = emit_code(gc, cb, psrlw(gc, work1, value(gc, 1)));

cb = emit_code(gc, cb, movq(gc, work2, src16));
cb = emit_code(gc, cb, pand(gc, work2, mxk4(gc, ~0, 0, ~0, 0)));

cb = emit_code(gc, cb, pand(gc, src16, mxk4(gc, 0, ~0, 0, ~0)));

cb = emit_code(gc, cb, pmaddwd(gc, dst32, src16));
cb = emit_code(gc, cb, pmaddwd(gc, work1, work2));
cb = emit_code(gc, cb, pslld(gc, work1, value(gc, 1)));
cb = emit_code(gc, cb, pslld(gc, dst32, value(gc, 16)));
cb = emit_code(gc, cb, paddd(gc, dst32, work1));

    return cb;
}

STATIC unsigned char*
perspective(__GLcontext *gc,
	    unsigned char* cb,
	    struct celem *tc,
	    struct celem *q,
	    struct celem *work1,
	    struct celem *work2)
{
cb = mul32x16(gc, cb, tc, mm0, work1, work2);
cb = emit_code(gc, cb, pslld(gc, tc, mm2));

    return cb;
}

/* Returns true if the blend function uses alpha */
STATIC GLboolean
uses_alpha(GLenum bm)
{
    switch (bm) {
    case GL_ZERO:
    case GL_ONE:
    case GL_DST_COLOR:
    case GL_SRC_COLOR:
    case GL_ONE_MINUS_DST_COLOR:
    case GL_ONE_MINUS_SRC_COLOR:
	return GL_FALSE;

    case GL_SRC_ALPHA:
    case GL_ONE_MINUS_SRC_ALPHA:
    case GL_DST_ALPHA:
    case GL_ONE_MINUS_DST_ALPHA:
    case GL_SRC_ALPHA_SATURATE:
	return GL_TRUE;

    default:
	assert(0);
    }
}

STATIC unsigned char*
BuildMMXRasterizer(__GLcontext *gc, unsigned char* cb)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    struct celem
	*forwards = label(gc),
	*alldone = label(gc);
    double *data;
    GLuint offset = 0;
    struct celem* tr;
    struct celem* h;
    struct celem* ending;
    struct celem* headgate;	/* 4-way jump table for handling start-of-span... */
    struct celem* tailgate;	/* ... end-of-span... */
    struct celem* minigate;	/* ... short (1-4 pixel) spans */
    struct celem* pixval;	/* Pixel value we're going to generate.  mm0 */
    struct celem* readfrom;	/* Place to get pixel from when blending with short spans */
    struct celem* stagger_k;
    struct celem* mixpix = label(gc);
    struct celem* do_recip = label(gc);
    struct __GLmmxtri mmxtr;
    GLint dx;
    GLboolean interpolate_tex;
    GLboolean interpolate_color;
    GLboolean interpolate_z;
    GLboolean need_alpha;
    GLenum texenv;
    struct celem *ditherspace;
    double *dithertab;
    __GLcolorBuffer* fb = &gc->frontBuffer;
    GLint
	z_stride,		/* Depth 4-pixel stride (either 8 or 16) */
	c_stride;		/* Color 4-pixel stride (either 4 or 8) */
#if __GL_CODEGEN_PMON
    struct celem* startKernelIteration;
    struct celem* endKernelIteration;
#endif /* __GL_CODEGEN_PMON */

#if __GL_CODEGEN_PMON
    startKernelIteration = label(gc);
    startKernelIteration->spec.label.where = (unsigned char*)&StartKernelIteration;
    endKernelIteration = label(gc);
    endKernelIteration->spec.label.where = (unsigned char*)&EndKernelIteration;
#endif /* __GL_CODEGEN_PMON */

    if (modeFlags & __GL_SHADE_TEXTURE) {
	interpolate_tex = GL_TRUE;
	texenv = gc->state.texture.env[0].mode;
	if (texenv == GL_DECAL &&
	    gc->texture.currentTexture->texelFormat == GL_RGB)
	    texenv = GL_REPLACE;
    } else {
	interpolate_tex = GL_FALSE;
	texenv = 0;
    }

    interpolate_color = !(interpolate_tex && (texenv == GL_REPLACE)) &&
	(modeFlags & __GL_SHADE_SMOOTH);

    if (modeFlags & __GL_SHADE_BLEND) {
	if (uses_alpha(gc->state.raster.blendSrc) ||
	    uses_alpha(gc->state.raster.blendDst))
	    need_alpha = GL_TRUE;
    } else {
	need_alpha = GL_FALSE;
    }
    
    /* However, if we're blending with a GL_REPLACE texture that has no alpha
     * channel, we need alpha from the fragment, so we set interpolate_color.
     */
    if (need_alpha &&
	(interpolate_tex && (texenv == GL_REPLACE)) &&
	 !has_alpha(gc->texture.currentTexture->texelFormat) &&
	(modeFlags & __GL_SHADE_SMOOTH))
	interpolate_color = GL_TRUE;

    switch (gc->drawBuffer->fbtype) {
    case 0:
    case CINDEX:
    case RGB332:
    case RGB666:
	c_stride = 4;
	break;
    case RGB5:
    case RGB565:
    case A1RGB5:
	c_stride = 8;
	break;
    case RGB8:
    case BGR8:
    case RGBX8:
    case XRGB8:
    case XBGR8:
    case RGBA8:
    case ARGB8:
    case ABGR8:
	assert(0);
	break;
    }

    interpolate_z = (modeFlags & __GL_SHADE_DEPTH_TEST &&
		     (gc->state.depth.writeEnable ||
		      gc->state.depth.testFunc != GL_ALWAYS));

    /* 
     * z_stride is the size of a gang of 4 depth values.
     * For a 16-bit Z it's 8, for 32-bit Z it's 16.
     */
    z_stride = 4 * (gc->depthBuffer.buf.depth / 8);

    tr = mem2(gc, value(gc, 0), esp);

    data = (double*)((((int)cb - 2000) + 7) & ~7UL);

#define ILOCAL(name)	(name = mem2(gc, value(gc, (int)&data[offset]), NULL), offset++)
#define DLOCAL(name)	(name = mem2(gc, value(gc, (int)&data[offset]), NULL), offset++)
#define XLOCAL(name, sz)(name = mem2(gc, value(gc, (int)&data[offset]), NULL), offset += (sz / sizeof(double)))

    DLOCAL(mmxtr.r.didx);
    DLOCAL(mmxtr.g.didx);
    DLOCAL(mmxtr.b.didx);
    DLOCAL(mmxtr.a.didx);
    DLOCAL(mmxtr.s.didx);
    DLOCAL(mmxtr.t.didx);
    DLOCAL(mmxtr.z.didx);

    XLOCAL(mmxtr.r.istagger, 32);
    XLOCAL(mmxtr.g.istagger, 32);
    XLOCAL(mmxtr.b.istagger, 32);
    XLOCAL(mmxtr.a.istagger, 32);
    XLOCAL(mmxtr.s.istagger, 32);
    XLOCAL(mmxtr.t.istagger, 32);
    XLOCAL(mmxtr.z.istagger, 32);

    /* Dither table for 3 colors, for 4 lines, for 4 horizontal pixels,
     * using 16-bit (2 byte) values.
     */
    XLOCAL(ditherspace, (3 * 4 * 4 * 2));
    dithertab = (double*)DISPOF(ditherspace);

    DLOCAL(mmxtr.zvalue);
    DLOCAL(mmxtr.zresult);
    DLOCAL(mmxtr.z_skew);

    DLOCAL(mmxtr.spill_z);
    DLOCAL(mmxtr.spill_r);
    DLOCAL(mmxtr.spill_g);
    DLOCAL(mmxtr.spill_a);
    DLOCAL(mmxtr.tex_a);
    DLOCAL(mmxtr.spill_b);
    DLOCAL(mmxtr.spill_1);

    DLOCAL(mmxtr.dithers[0]);
    DLOCAL(mmxtr.dithers[1]);
    DLOCAL(mmxtr.dithers[2]);
    DLOCAL(mmxtr.dithers[3]);

    DLOCAL(mmxtr.src.r);
    DLOCAL(mmxtr.src.g);
    DLOCAL(mmxtr.src.b);
    DLOCAL(mmxtr.src.a);

    DLOCAL(mmxtr.dst.r);
    DLOCAL(mmxtr.dst.g);
    DLOCAL(mmxtr.dst.b);
    DLOCAL(mmxtr.dst.a);

    DLOCAL(mmxtr.df.r);
    DLOCAL(mmxtr.df.g);
    DLOCAL(mmxtr.df.b);
    DLOCAL(mmxtr.df.a);

    DLOCAL(mmxtr.sf.r);
    DLOCAL(mmxtr.sf.g);
    DLOCAL(mmxtr.sf.b);
    DLOCAL(mmxtr.sf.a);

    DLOCAL(mmxtr.pixcopy);
    DLOCAL(mmxtr.indexTextureBase);

    ILOCAL(mmxtr.prev_st);
    ILOCAL(mmxtr.next_pc);

    ILOCAL(h);
    ILOCAL(ending);

    {
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLuint shifts[4];
	int c, i, j;

	shifts[0] = log2(cfb->redMax) + 1;
	shifts[1] = log2(cfb->greenMax) + 1;
	shifts[2] = log2(cfb->blueMax) + 1;

	for (c = 0; c <= 2; c++)
	    for (i = 0; i < 4; i++) {
		short* ps;

		ps = (short*)&dithertab[(4 * c) + i];
		for (j = 0; j < 4; j++) {
		    *ps++ = __glFRDitherTable[i][j] >> (shifts[c] + 1);
		}
	    }
    }

#define MENTION(i)								\
    __glNSWhereIs(gc->tr, offsetof(i, __GLtri), sizeof(GLint));

    MENTION(y0Int);
    MENTION(y1Int);
    MENTION(y2Int);

    MENTION(x0Int);
    MENTION(x1Int);
    MENTION(x2Int);

    MENTION(dxdy0Int[0]);
    MENTION(dxdy0Int[1]);
    MENTION(dxdy1Int[0]);
    MENTION(dxdy1Int[1]);
    MENTION(dxdy2Int[0]);
    MENTION(dxdy2Int[1]);

    MENTION(x0Frac);
    MENTION(x1Frac);
    MENTION(x2Frac);

    MENTION(dxdy0Frac);
    MENTION(dxdy1Frac);
    MENTION(dxdy2Frac);

    MENTION(dx);

#define IPAIR(a, b)				\
    __glNSAlign(gc->tr, 8);			\
    MENTION(a);					\
    MENTION(b);					\
    MENTION(a##0);				\
    MENTION(b##0);				\
    MENTION(d##a##dx);				\
    MENTION(d##b##dx);				\
    MENTION(d##a##dy0[0]);			\
    MENTION(d##b##dy0[0]);			\
    MENTION(d##a##dy0[1]);			\
    MENTION(d##b##dy0[1]);

#if __GL_CODEGEN_DESCRIBERS
cb = emit_code(gc, cb, inc(gc, mem(gc, value(gc, (int)&last_describer->drawn_tris), NULL, NULL, 1)));
cb = emit_code(gc, cb, rdtsc(gc));
cb = emit_code(gc, cb, sub(gc, mem(gc, value(gc, (int)&last_describer->cyc[0]), NULL, NULL, 1), eax));
cb = emit_code(gc, cb, sbb(gc, mem(gc, value(gc, (int)&last_describer->cyc[1]), NULL, NULL, 1), edx));
#endif /* __GL_CODEGEN_DESCRIBERS */

    if (interpolate_color || interpolate_tex || interpolate_z) {
	stagger_k = mxk4(gc, 0, 1, 2, 3);

	mxk4(gc, 3, 2, 1, 0);

cb = emit_code(gc, cb, mov(gc, ebp, tr(dx)));
cb = emit_code(gc, cb, mov(gc, ebx, ebp));
cb = emit_code(gc, cb, sar(gc, ebp, value(gc, 31)));
cb = emit_code(gc, cb, and(gc, ebp, value(gc, 3)));
    }

    if (interpolate_color) {
	struct celem* again = label(gc);

	IPAIR(r, g);
	IPAIR(b, a);

cb = kerrect(gc, cb, tr(r0), tr(drdx), tr(drdy0[0]), tr(drdy0[1]), value(gc, 14 - log2(fb->redMax)));
cb = kerrect(gc, cb, tr(g0), tr(dgdx), tr(dgdy0[0]), tr(dgdy0[1]), value(gc, 14 - log2(fb->greenMax)));
cb = kerrect(gc, cb, tr(b0), tr(dbdx), tr(dbdy0[0]), tr(dbdy0[1]), value(gc, 14 - log2(fb->blueMax)));
	if (need_alpha)
cb = kerrect(gc, cb, tr(a0), tr(dadx), tr(dady0[0]), tr(dady0[1]), value(gc, 14 - log2(ALPHA_MAX)));

cb = emit_code(gc, cb, movq(gc, mm0, tr(drdx)));
cb = emit_code(gc, cb, movq(gc, mm2, tr(dbdx)));

cb = emit_code(gc, cb, paddd(gc, mm0, mm0));
cb = emit_code(gc, cb, paddd(gc, mm2, mm2));

cb = emit_code(gc, cb, paddd(gc, mm0, mm0));
cb = emit_code(gc, cb, paddd(gc, mm2, mm2));

cb = rtz(gc, cb, mm0, value(gc, 16));
cb = rtz(gc, cb, mm2, value(gc, 16));

cb = emit_code(gc, cb, movq(gc, mm1, mm0));
cb = emit_code(gc, cb, movq(gc, mm3, mm2));

cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpcklwd(gc, mm2, mm2));
cb = emit_code(gc, cb, punpckhwd(gc, mm1, mm1));
cb = emit_code(gc, cb, punpckhwd(gc, mm3, mm3));

cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm1, mm1));
cb = emit_code(gc, cb, punpckhdq(gc, mm2, mm2));
cb = emit_code(gc, cb, punpckhdq(gc, mm3, mm3));

cb = emit_code(gc, cb, movq(gc, mmxtr.r.didx, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.g.didx, mm1));
cb = emit_code(gc, cb, movq(gc, mmxtr.b.didx, mm2));
	if (need_alpha)
cb = emit_code(gc, cb, movq(gc, mmxtr.a.didx, mm3));

cb = emit_code(gc, cb, movq(gc, mm0, tr(drdx)));
cb = emit_code(gc, cb, movq(gc, mm2, tr(dbdx)));

cb = rtz(gc, cb, mm0, value(gc, 16));
cb = rtz(gc, cb, mm2, value(gc, 16));

cb = emit_code(gc, cb, movq(gc, mm1, mm0));
cb = emit_code(gc, cb, movq(gc, mm3, mm2));

cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpcklwd(gc, mm2, mm2));
cb = emit_code(gc, cb, punpckhwd(gc, mm1, mm1));
cb = emit_code(gc, cb, punpckhwd(gc, mm3, mm3));

cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm1, mm1));
cb = emit_code(gc, cb, punpckhdq(gc, mm2, mm2));
cb = emit_code(gc, cb, punpckhdq(gc, mm3, mm3));

cb = emit_code(gc, cb, movq(gc, mm4, mm0));
cb = emit_code(gc, cb, movq(gc, mm5, mm1));
cb = emit_code(gc, cb, movq(gc, mm6, mm2));
cb = emit_code(gc, cb, movq(gc, mm7, mm3));

cb = emit_code(gc, cb, mov(gc, eax, tr(dx)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 31)));

cb = emit_code(gc, cb, pmullw(gc, mm0, mem(gc, value(gc, DISPOF(stagger_k)), NULL, eax, 8)));
cb = emit_code(gc, cb, pmullw(gc, mm1, mem(gc, value(gc, DISPOF(stagger_k)), NULL, eax, 8)));
cb = emit_code(gc, cb, pmullw(gc, mm2, mem(gc, value(gc, DISPOF(stagger_k)), NULL, eax, 8)));
	if (need_alpha)
cb = emit_code(gc, cb, pmullw(gc, mm3, mem(gc, value(gc, DISPOF(stagger_k)), NULL, eax, 8)));

cb = emit_code(gc, cb, mov(gc, ecx, value(gc, 4)));
cb = emit_code(gc, cb, push(gc, ebp));

again->spec.label.where = cb; resolve(gc, again, cb);
cb = emit_code(gc, cb, movq(gc, mem(gc, value(gc, DISPOF(mmxtr.r.istagger)), NULL, ebp, 8), mm0));
cb = emit_code(gc, cb, movq(gc, mem(gc, value(gc, DISPOF(mmxtr.g.istagger)), NULL, ebp, 8), mm1));
cb = emit_code(gc, cb, movq(gc, mem(gc, value(gc, DISPOF(mmxtr.b.istagger)), NULL, ebp, 8), mm2));
	if (need_alpha)
cb = emit_code(gc, cb, movq(gc, mem(gc, value(gc, DISPOF(mmxtr.a.istagger)), NULL, ebp, 8), mm3));

cb = emit_code(gc, cb, psubw(gc, mm0, mm4));
cb = emit_code(gc, cb, psubw(gc, mm1, mm5));
cb = emit_code(gc, cb, psubw(gc, mm2, mm6));
cb = emit_code(gc, cb, psubw(gc, mm3, mm7));

cb = emit_code(gc, cb, add(gc, ebp, ebx));

cb = emit_code(gc, cb, dec(gc, ecx));
cb = emit_code(gc, cb, jne(gc, again));

cb = emit_code(gc, cb, pop(gc, ebp));
    }

    if (interpolate_z) {
	static const unsigned short masks[4] = { 
	    0x0000, 0xffff,
	    0xffff, 0x0000
	};
	struct celem* again = label(gc);
	IPAIR(z, zp);

cb = emit_code(gc, cb, movd(gc, mm0, tr(dzdx)));
cb = emit_code(gc, cb, punpckldq(gc, mm0, mm0));

	if (gc->depthBuffer.buf.depth == 16) {
cb = emit_code(gc, cb, movq(gc, mmxtr.z_skew, mm0));
cb = emit_code(gc, cb, paddd(gc, mm0, mm0));
	} else {
cb = emit_code(gc, cb, paddd(gc, mm0, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.z_skew, mm0));
	}

cb = emit_code(gc, cb, paddd(gc, mm0, mm0));

cb = emit_code(gc, cb, movq(gc, mmxtr.z.didx, mm0));

cb = emit_code(gc, cb, mov(gc, eax, tr(dx)));

cb = emit_code(gc, cb, shr(gc, eax, value(gc, 31)));

cb = rtz(gc, cb, mm0, value(gc, 30));

cb = emit_code(gc, cb, push(gc, ebp));
cb = emit_code(gc, cb, psrad(gc, mm0, value(gc, 2)));

cb = emit_code(gc, cb, movq(gc, mm1, mm0));

	if (gc->depthBuffer.buf.depth == 16)
cb = emit_code(gc, cb, pslld(gc, mm0, value(gc, 1)));

cb = emit_code(gc, cb, movq(gc, mm2, mem(gc, value(gc, (int)&masks), NULL, eax, 4)));

cb = emit_code(gc, cb, punpcklwd(gc, mm2, mm2));

cb = emit_code(gc, cb, pand(gc, mm0, mm2));

cb = emit_code(gc, cb, mov(gc, ecx, value(gc, 4)));
again->spec.label.where = cb; resolve(gc, again, cb);
cb = emit_code(gc, cb, movq(gc, mem(gc, value(gc, DISPOF(mmxtr.z.istagger)), NULL, ebp, 8), mm0));
    	
cb = emit_code(gc, cb, psubd(gc, mm0, mm1));
cb = emit_code(gc, cb, add(gc, ebp, ebx));
    	
cb = emit_code(gc, cb, dec(gc, ecx));
cb = emit_code(gc, cb, jne(gc, again));

cb = emit_code(gc, cb, pop(gc, ebp));

    }

    if (interpolate_tex) {
	struct celem* again = label(gc);

	IPAIR(s, t);
	if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
	    IPAIR(q, dummy);
	}

	DLOCAL(mmxtr.tex_s_shift);
	DLOCAL(mmxtr.tex_t_shift);
	DLOCAL(mmxtr.tex_s_kerrect);
	DLOCAL(mmxtr.tex_t_kerrect);
	DLOCAL(mmxtr.tex_t_mask);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, (int)&gc->texture.currentTexture), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, edx, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, offsetof(level, __GLtexture)), esi, NULL, 1)));
cb = emit_code(gc, cb, pxor(gc, mm0, mm0));

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ecx, edx));

cb = emit_code(gc, cb, movq(gc, mmxtr.tex_s_shift, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.tex_t_shift, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.tex_s_kerrect, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.tex_t_kerrect, mm0));

cb = emit_code(gc, cb, movd(gc, mm7, mem(gc, value(gc, offsetof(widthLog2, __GLmipMapLevel)), esi, NULL, 1)));
cb = emit_code(gc, cb, movd(gc, mm0, mem(gc, value(gc, offsetof(height2, __GLmipMapLevel)), esi, NULL, 1)));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckldq(gc, mm0, mm0));
cb = emit_code(gc, cb, psubw(gc, mm0, mxk(gc, 1)));
cb = emit_code(gc, cb, psllw(gc, mm0, mm7));
cb = emit_code(gc, cb, movq(gc, mmxtr.tex_t_mask, mm0));

/*
 * s_shift = (16 - wl2)
 * t_shift = (16 - wl2) - hl2;
 */

cb = emit_code(gc, cb, sub(gc, ecx, mem(gc, value(gc, offsetof(heightLog2, __GLmipMapLevel)), esi, NULL, 1)));
cb = emit_code(gc, cb, sub(gc, edx, mem(gc, value(gc, offsetof(widthLog2, __GLmipMapLevel)), esi, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, mmxtr.tex_t_kerrect, ecx));
cb = emit_code(gc, cb, mov(gc, mmxtr.tex_s_kerrect, edx));

cb = emit_code(gc, cb, mov(gc, mmxtr.tex_s_shift, edx));
cb = emit_code(gc, cb, mov(gc, mmxtr.tex_t_shift, ecx));

	if (!(modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = kerrect(gc, cb, tr(s0), tr(dsdx), tr(dsdy0[0]), tr(dsdy0[1]), mmxtr.tex_s_kerrect);
cb = kerrect(gc, cb, tr(t0), tr(dtdx), tr(dtdy0[0]), tr(dtdy0[1]), mmxtr.tex_t_kerrect);
        }
	if (!(modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = emit_code(gc, cb, movq(gc, mm0, tr(dsdx)));

cb = emit_code(gc, cb, paddd(gc, mm0, mm0));
cb = emit_code(gc, cb, paddd(gc, mm0, mm0));
cb = rtz(gc, cb, mm0, value(gc, 16));

cb = emit_code(gc, cb, movq(gc, mm1, mm0));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));

cb = emit_code(gc, cb, punpckhwd(gc, mm1, mm1));

cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm1, mm1));

cb = emit_code(gc, cb, movq(gc, mmxtr.s.didx, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.t.didx, mm1));

cb = rtzw(gc, cb, mm0, value(gc, 14));
cb = rtzw(gc, cb, mm1, value(gc, 14));
cb = emit_code(gc, cb, psraw(gc, mm0, value(gc, 2)));
cb = emit_code(gc, cb, psraw(gc, mm1, value(gc, 2)));

cb = emit_code(gc, cb, movq(gc, mm2, mm0));
cb = emit_code(gc, cb, movq(gc, mm3, mm1));

cb = emit_code(gc, cb, mov(gc, eax, tr(dx)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 31)));

cb = emit_code(gc, cb, pmullw(gc, mm0, mem(gc, value(gc, DISPOF(stagger_k)), NULL, eax, 8)));
cb = emit_code(gc, cb, pmullw(gc, mm1, mem(gc, value(gc, DISPOF(stagger_k)), NULL, eax, 8)));

cb = emit_code(gc, cb, mov(gc, ecx, value(gc, 4)));
again->spec.label.where = cb; resolve(gc, again, cb);
cb = emit_code(gc, cb, movq(gc, mem(gc, value(gc, DISPOF(mmxtr.s.istagger)), NULL, ebp, 8), mm0));
cb = emit_code(gc, cb, movq(gc, mem(gc, value(gc, DISPOF(mmxtr.t.istagger)), NULL, ebp, 8), mm1));

cb = emit_code(gc, cb, psubw(gc, mm0, mm2));
cb = emit_code(gc, cb, psubw(gc, mm1, mm3));
    
cb = emit_code(gc, cb, add(gc, ebp, ebx));
    
cb = emit_code(gc, cb, dec(gc, ecx));
cb = emit_code(gc, cb, jne(gc, again));
        }
    }

    if (modeFlags & __GL_SHADE_BLEND) {
	/* If we're blending with a texenv GL_REPLACE, and the texture
	 * has no alpha channel, assume alpha of 1.0 */
cb = emit_code(gc, cb, movq(gc, mm0, mxk(gc, 0x4000)));
cb = emit_code(gc, cb, movq(gc, mmxtr.src.a, mm0));

        /* Similarly, destination alpha is assumed to be 1.0 */
cb = emit_code(gc, cb, movq(gc, mmxtr.dst.a, mm0));
    }

    if (!(interpolate_tex && (texenv == GL_REPLACE)) &&
	!interpolate_color) {
cb = emit_code(gc, cb, movd(gc, mm0, tr(r)));
cb = emit_code(gc, cb, pslld(gc, mm0, value(gc, 14 - log2(fb->redMax))));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.spill_r, mm0));
 
cb = emit_code(gc, cb, movd(gc, mm0, tr(g)));
cb = emit_code(gc, cb, pslld(gc, mm0, value(gc, 14 - log2(fb->greenMax))));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.spill_g, mm0));
 
cb = emit_code(gc, cb, movd(gc, mm0, tr(b)));
cb = emit_code(gc, cb, pslld(gc, mm0, value(gc, 14 - log2(fb->blueMax))));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.spill_b, mm0));
    }
    if (need_alpha && !(modeFlags & __GL_SHADE_SMOOTH)) {
cb = emit_code(gc, cb, movd(gc, mm0, tr(a)));
cb = emit_code(gc, cb, pslld(gc, mm0, value(gc, 14 - log2(ALPHA_MAX))));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.spill_a, mm0));
    }

cb = emit_code(gc, cb, mov(gc, eax, tr(dcpdx)));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 0)));
cb = emit_code(gc, cb, jg(gc, forwards));

    for (dx = -1; dx <= 1; dx += 2) {
	struct celem
	    *trap = label(gc),
	    *perline = label(gc),
	    *next_line = label(gc),
	    *done_trap = label(gc);
	struct celem
	    *short_span = label(gc),
	    *no_head = label(gc),
	    *central = label(gc),
	    *central2 = label(gc),
	    *tail = label(gc);
	struct celem
	    *zwinh = label(gc),
	    *zloseh = label(gc),
	    *zwinc = label(gc),
	    *zlosec = label(gc),
	    *zwins = label(gc),
	    *zwint = label(gc);

	XLOCAL(headgate, 4 * sizeof(void*));
	XLOCAL(tailgate, 4 * sizeof(void*));
	XLOCAL(minigate, 4 * sizeof(void*));

	if (dx == 1)
forwards->spec.label.where = cb; resolve(gc, forwards, cb);

	if (interpolate_z) {
cb = emit_code(gc, cb, movq(gc, mm4, tr(z0)));
	}

	if (interpolate_color) {
cb = emit_code(gc, cb, movq(gc, mm5, tr(r0)));
cb = emit_code(gc, cb, movq(gc, mm7, tr(b0)));
	}

	if (interpolate_tex) {
cb = emit_code(gc, cb, movq(gc, mm6, tr(s0)));
	}

	if (!interpolate_color && !interpolate_tex) {
	    /* Must be flat untextured */
cb = emit_code(gc, cb, movq(gc, mm7, tr(r0)));
cb = emit_code(gc, cb, punpcklwd(gc, mm7, mm7));
cb = emit_code(gc, cb, punpckldq(gc, mm7, mm7));
	}

trap->spec.label.where = cb; resolve(gc, trap, cb);
cb = emit_code(gc, cb, mov(gc, eax, tr(y2Int)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(y0Int)));
cb = emit_code(gc, cb, sub(gc, eax, ebx));
cb = emit_code(gc, cb, mov(gc, tr(y), ebx));
cb = emit_code(gc, cb, je(gc, done_trap));


#if __GL_CODEGEN_PMON
cb = emit_code(gc, cb, push(gc, value(gc, 1)));
cb = emit_code(gc, cb, call(gc, startKernelIteration));
#endif /* __GL_CODEGEN_PMON */

perline->spec.label.where = cb; resolve(gc, perline, cb);

#if DEBUG
	if (interpolate_z) {
	    struct celem* ok = near_label(gc);

cb = emit_code(gc, cb, mov(gc, edi, tr(cp0)));
cb = emit_code(gc, cb, mov(gc, esi, tr(zp0)));

cb = emit_code(gc, cb, and(gc, edi, value(gc, (c_stride - 1))));
cb = emit_code(gc, cb, and(gc, esi, value(gc, (z_stride - 1))));

cb = emit_code(gc, cb, shr(gc, edi, value(gc, ((c_stride == 4) ? 0 : 1))));
cb = emit_code(gc, cb, shr(gc, esi, value(gc, ((z_stride == 8) ? 1 : 2))));

cb = emit_code(gc, cb, cmp(gc, edi, esi));
cb = emit_code(gc, cb, je(gc, ok));

cb = emit_code(gc, cb, int3(gc));

ok->spec.label.where = cb; resolve(gc, ok, cb);
	}
#endif

cb = emit_code(gc, cb, mov(gc, ecx, tr(x0Int)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(x1Int)));

cb = emit_code(gc, cb, sub(gc, ebx, ecx));
cb = emit_code(gc, cb, mov(gc, h, eax));

cb = emit_code(gc, cb, jle(gc, next_line));

#if __GL_CODEGEN_DESCRIBERS
cb = emit_code(gc, cb, add(gc, mem(gc, value(gc, (int)&last_describer->pixels), NULL, NULL, 1), ebx));
cb = emit_code(gc, cb, inc(gc, mem(gc, value(gc, (int)&last_describer->spans), NULL, NULL, 1)));
#endif /* __GL_CODEGEN_DESCRIBERS */

        /*
	 * Start of span.  Prepare interpolants for use
         */

	/* Do dither setup, unless we're in the very easy case, when we
	 * do dither setup when we expand the colors at the start of the line
	 */
        if (modeFlags & __GL_SHADE_DITHER) {
		GLuint dt = (int)dithertab;
    
cb = emit_code(gc, cb, mov(gc, ecx, tr(y)));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, DITHER_MASK)));
	    if (!interpolate_color &&
		!interpolate_tex &&
		!(modeFlags & __GL_SHADE_BLEND)) {
		GLuint redSize = log2(fb->redMax);
		GLuint greenSize = log2(fb->greenMax);
		GLuint blueSize = log2(fb->blueMax);

cb = emit_code(gc, cb, movq(gc, mm0, mmxtr.spill_r));
cb = emit_code(gc, cb, movq(gc, mm1, mmxtr.spill_g));
cb = emit_code(gc, cb, movq(gc, mm7, mmxtr.spill_b));
cb = emit_code(gc, cb, paddw(gc, mm0, mem(gc, value(gc, dt), NULL, ecx, 8)));

cb = emit_code(gc, cb, paddw(gc, mm1, mem(gc, value(gc, dt + 32), NULL, ecx, 8)));
cb = mmxgobit(gc, cb, mm0, (14), fb->redShift + redSize);

cb = emit_code(gc, cb, pand(gc, mm0, mxk(gc, fb->redMax << fb->redShift)));

		/* Mask green before shift.  To do this we have to 
		 * shift the mask (greenMax) so that its high bit is
		 * in bit-position 14.
	         */
cb = emit_code(gc, cb, pand(gc, mm1, mxk(gc, fb->greenMax << (14 - greenSize))));

cb = emit_code(gc, cb, paddw(gc, mm7, mem(gc, value(gc, dt + 64), NULL, ecx, 8)));
cb = mmxgobit(gc, cb, mm1, (14), fb->greenShift + greenSize);

cb = mmxgobit(gc, cb, mm7, (14), fb->blueShift + blueSize);
cb = emit_code(gc, cb, por(gc, mm1, mm0));

cb = emit_code(gc, cb, por(gc, mm7, mm1));

	    } else if (!(interpolate_color &&
		  !(modeFlags & (__GL_SHADE_TEXTURE | __GL_SHADE_BLEND)))) {
cb = emit_code(gc, cb, movq(gc, mm0, mem(gc, value(gc, dt), NULL, ecx, 8)));
cb = emit_code(gc, cb, movq(gc, mmxtr.dithers[0], mm0));
cb = emit_code(gc, cb, movq(gc, mm0, mem(gc, value(gc, dt + 32), NULL, ecx, 8)));
cb = emit_code(gc, cb, movq(gc, mmxtr.dithers[1], mm0));
cb = emit_code(gc, cb, movq(gc, mm0, mem(gc, value(gc, dt + 64), NULL, ecx, 8)));
cb = emit_code(gc, cb, movq(gc, mmxtr.dithers[2], mm0));
	    }
	}

	if (interpolate_color) {
	    struct celem* span_long = near_label(gc);
	    struct celem* do_stagger = near_label(gc);

	    if (need_alpha) {
cb = emit_code(gc, cb, movq(gc, mm0, mm7));
cb = emit_code(gc, cb, punpckhwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckhdq(gc, mm0, mm0));
	    }

cb = emit_code(gc, cb, movq(gc, mm6, mm5));
cb = emit_code(gc, cb, punpcklwd(gc, mm5, mm5));

cb = emit_code(gc, cb, punpckhwd(gc, mm6, mm6));
cb = emit_code(gc, cb, mov(gc, edi, tr(cp0)));

cb = emit_code(gc, cb, punpcklwd(gc, mm7, mm7));
cb = emit_code(gc, cb, mov(gc, eax, edi));

cb = emit_code(gc, cb, punpckhdq(gc, mm5, mm5));
cb = emit_code(gc, cb, and(gc, eax, value(gc, c_stride - 1)));

	    if (c_stride == 8)
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 1)));

cb = emit_code(gc, cb, punpckhdq(gc, mm6, mm6));
cb = emit_code(gc, cb, punpckhdq(gc, mm7, mm7));

cb = emit_code(gc, cb, cmp(gc, ebx, value(gc, 4)));
cb = emit_code(gc, cb, jg(gc, span_long));

cb = emit_code(gc, cb, mov(gc, eax, value(gc, (dx == -1) ? 3 : 0)));
cb = emit_code(gc, cb, jmp(gc, do_stagger));

span_long->spec.label.where = cb; resolve(gc, span_long, cb);

            if ((modeFlags & __GL_SHADE_DITHER) &&
		!(modeFlags & (__GL_SHADE_TEXTURE | __GL_SHADE_BLEND))) {
		GLuint dt = (int)dithertab;

cb = emit_code(gc, cb, mov(gc, ecx, tr(y)));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, DITHER_MASK)));
cb = emit_code(gc, cb, paddw(gc, mm5, mem(gc, value(gc, dt), NULL, ecx, 8)));
cb = emit_code(gc, cb, paddw(gc, mm6, mem(gc, value(gc, dt + 32), NULL, ecx, 8)));
cb = emit_code(gc, cb, paddw(gc, mm7, mem(gc, value(gc, dt + 64), NULL, ecx, 8)));
	    }
do_stagger->spec.label.where = cb; resolve(gc, do_stagger, cb);

cb = emit_code(gc, cb, paddw(gc, mm5, mem(gc, value(gc, DISPOF(mmxtr.r.istagger)), NULL, eax, 8)));

cb = emit_code(gc, cb, paddw(gc, mm6, mem(gc, value(gc, DISPOF(mmxtr.g.istagger)), NULL, eax, 8)));

cb = emit_code(gc, cb, paddw(gc, mm7, mem(gc, value(gc, DISPOF(mmxtr.b.istagger)), NULL, eax, 8)));

	    if (need_alpha) {
cb = emit_code(gc, cb, paddw(gc, mm0, mem(gc, value(gc, DISPOF(mmxtr.a.istagger)), NULL, eax, 8)));
cb = emit_code(gc, cb, movq(gc, mmxtr.spill_a, mm0));
	    }
	    if ((modeFlags & __GL_SHADE_TEXTURE) &&
		(texenv == GL_MODULATE)) {
cb = emit_code(gc, cb, movq(gc, mmxtr.spill_r, mm5));
cb = emit_code(gc, cb, movq(gc, mmxtr.spill_g, mm6));
cb = emit_code(gc, cb, movq(gc, mmxtr.spill_b, mm7));
	    }
	} else {
	    struct celem* span_long = near_label(gc);
	    struct celem* done = near_label(gc);

cb = emit_code(gc, cb, mov(gc, edi, tr(cp0)));

cb = emit_code(gc, cb, cmp(gc, ebx, value(gc, 4)));
cb = emit_code(gc, cb, jg(gc, span_long));

cb = emit_code(gc, cb, mov(gc, eax, value(gc, (dx == -1) ? 3 : 0)));
cb = emit_code(gc, cb, jmp(gc, done));

span_long->spec.label.where = cb; resolve(gc, span_long, cb);
cb = emit_code(gc, cb, mov(gc, eax, edi));
cb = emit_code(gc, cb, and(gc, eax, value(gc, c_stride - 1)));
            if (c_stride == 8)
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 1)));
done->spec.label.where = cb; resolve(gc, done, cb);
        }

	if (interpolate_tex) {
	    /* Color interpolation trashes s,t interpolant */
	    if (interpolate_color)
cb = emit_code(gc, cb, movq(gc, mm6, tr(s0)));

	    if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
               /* Starting point is (s,t) divided by w */
		static const unsigned long spanlen[2][4] = {
		    { 16, 19, 18, 17 },
		    { 17, 18, 19, 16 }
		};
		static const unsigned long fracs[] = {
		    0,
		    32767,
		    (32768 * 1) / 2,
		    (32768 * 1) / 3,
		    (32768 * 1) / 4,
		    (32768 * 1) / 5,
		    (32768 * 1) / 6,
		    (32768 * 1) / 7,
		    (32768 * 1) / 8,
		    (32768 * 1) / 9,
		    (32768 * 1) / 10,
		    (32768 * 1) / 11,
		    (32768 * 1) / 12,
		    (32768 * 1) / 13,
		    (32768 * 1) / 14,
		    (32768 * 1) / 15,
		    (32768 * 1) / 16,
		    (32768 * 1) / 17,
		    (32768 * 1) / 18,
		    (32768 * 1) / 19
		};
		static const unsigned long initials[2][4] = {
		    { 0,
		      -1,
		      -2,
		      -3 },
		    { -3,
		      -2,
		      -1,
		      0 }
		};

cb = emit_code(gc, cb, mov(gc, ecx, tr(q0)));
cb = emit_code(gc, cb, call(gc, do_recip));

cb = mul32x16(gc, cb, mm6, mm0, mm1, mm7);
cb = emit_code(gc, cb, pslld(gc, mm6, mm2));

		/* Now mm6 holds the affine start point. */

		/* Find the second point... */

		/* ebp, the span length, is the minimum of the total
		   span length and the computed 'pixels till we correct
		   again' in the spanlen[] table.
		   */
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, (int)(spanlen[dx == -1])), NULL, eax, 4)));
                {
		    struct celem *ok = near_label(gc);

		    /* ebp = min(ebp, ebx) */
cb = emit_code(gc, cb, cmp(gc, ebp, ebx));
cb = emit_code(gc, cb, jb(gc, ok));
cb = emit_code(gc, cb, mov(gc, ebp, ebx));
ok->spec.label.where = cb; resolve(gc, ok, cb);
	
		}

cb = emit_code(gc, cb, movd(gc, mm0, ebp));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckldq(gc, mm0, mm0));
cb = emit_code(gc, cb, movq(gc, mm3, mm0));

cb = emit_code(gc, cb, movq(gc, mm7, tr(dsdx)));
cb = mul32x16i(gc, cb, mm7, mm0, mm1, mm2);
cb = emit_code(gc, cb, paddd(gc, mm7, tr(s0)));

		/* mm7 holds the second point (pre divide) */

cb = emit_code(gc, cb, movq(gc, tr(s), mm7));

		/* Find second 'q' */
cb = emit_code(gc, cb, movq(gc, mm2, tr(dqdx)));
cb = mul32x16i(gc, cb, mm2, mm3, mm1, mm0);
cb = emit_code(gc, cb, movd(gc, ecx, mm2));
cb = emit_code(gc, cb, add(gc, ecx, tr(q0)));

cb = emit_code(gc, cb, mov(gc, tr(q), ecx));
cb = emit_code(gc, cb, call(gc, do_recip));
cb = mul32x16(gc, cb, mm7, mm0, mm1, mm3);
cb = emit_code(gc, cb, pslld(gc, mm7, mm2));

		/* mm7 holds affine second point */

cb = emit_code(gc, cb, movq(gc, mmxtr.prev_st, mm7));

cb = emit_code(gc, cb, psubd(gc, mm7, mm6));

	        /* mm7 is affine span.  Derive initial step, fourfold
		 * step, and stagger.  */

cb = emit_code(gc, cb, movd(gc, mm0, mem(gc, value(gc, (int)fracs), NULL, ebp, 4)));
cb = emit_code(gc, cb, punpcklwd(gc, mm0, mm0));
cb = emit_code(gc, cb, punpckldq(gc, mm0, mm0));
cb = mul32x16(gc, cb, mm7, mm0, mm2, mm3);

		/* mm7 is pixel step: derive the fourfold */
cb = emit_code(gc, cb, movq(gc, mm2, mm7));
cb = emit_code(gc, cb, movq(gc, mm3, mm7));
cb = emit_code(gc, cb, pslld(gc, mm2, mmxtr.tex_s_kerrect));
cb = emit_code(gc, cb, pslld(gc, mm3, mmxtr.tex_t_kerrect));
cb = emit_code(gc, cb, punpcklwd(gc, mm2, mm2));
cb = emit_code(gc, cb, punpckhwd(gc, mm3, mm3));
cb = emit_code(gc, cb, punpckhdq(gc, mm2, mm2));
cb = emit_code(gc, cb, punpckhdq(gc, mm3, mm3));
cb = emit_code(gc, cb, movq(gc, mm0, mm2));
cb = emit_code(gc, cb, movq(gc, mm1, mm3));
cb = emit_code(gc, cb, psllw(gc, mm0, value(gc, 2)));
cb = emit_code(gc, cb, psllw(gc, mm1, value(gc, 2)));
cb = emit_code(gc, cb, movq(gc, mmxtr.s.didx, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.t.didx, mm1));

cb = emit_code(gc, cb, movd(gc, mm7, mem(gc, value(gc, (int)(initials[dx == -1])), NULL, eax, 4)));
cb = emit_code(gc, cb, punpcklwd(gc, mm7, mm7));
cb = emit_code(gc, cb, punpckldq(gc, mm7, mm7));
cb = emit_code(gc, cb, movq(gc, mm0, mm2));
cb = emit_code(gc, cb, movq(gc, mm1, mm3));
cb = emit_code(gc, cb, pmullw(gc, mm0, mm7));
cb = emit_code(gc, cb, pmullw(gc, mm1, mm7));

cb = emit_code(gc, cb, mov(gc, edx, tr(dx)));
cb = emit_code(gc, cb, shr(gc, edx, value(gc, 31)));
cb = emit_code(gc, cb, pmullw(gc, mm2, mem(gc, value(gc, DISPOF(stagger_k)), NULL, edx, 8)));
cb = emit_code(gc, cb, pmullw(gc, mm3, mem(gc, value(gc, DISPOF(stagger_k)), NULL, edx, 8)));
cb = emit_code(gc, cb, paddw(gc, mm2, mm0));
cb = emit_code(gc, cb, paddw(gc, mm3, mm1));
            }

cb = emit_code(gc, cb, movq(gc, mm7, mm6));

	    if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
cb = emit_code(gc, cb, pslld(gc, mm6, mmxtr.tex_s_kerrect));
cb = emit_code(gc, cb, pslld(gc, mm7, mmxtr.tex_t_kerrect));
cb = emit_code(gc, cb, punpcklwd(gc, mm6, mm6));
cb = emit_code(gc, cb, punpckhwd(gc, mm7, mm7));
cb = emit_code(gc, cb, punpckhdq(gc, mm6, mm6));
cb = emit_code(gc, cb, punpckhdq(gc, mm7, mm7));
cb = emit_code(gc, cb, paddw(gc, mm6, mm2));
cb = emit_code(gc, cb, paddw(gc, mm7, mm3));
            } else {
cb = emit_code(gc, cb, punpcklwd(gc, mm6, mm6));
cb = emit_code(gc, cb, punpckhwd(gc, mm7, mm7));
cb = emit_code(gc, cb, punpckhdq(gc, mm6, mm6));
cb = emit_code(gc, cb, punpckhdq(gc, mm7, mm7));
cb = emit_code(gc, cb, paddw(gc, mm6, mem(gc, value(gc, DISPOF(mmxtr.s.istagger)), NULL, eax, 8)));
cb = emit_code(gc, cb, paddw(gc, mm7, mem(gc, value(gc, DISPOF(mmxtr.t.istagger)), NULL, eax, 8)));
            }

	    if (gc->texture.currentTexture->texelFormat != GL_COLOR_INDEX) {
cb = emit_code(gc, cb, mov(gc, ebp, tr(tp)));
	    } else {
cb = emit_code(gc, cb, movd(gc, mm0, tr(tp)));
cb = emit_code(gc, cb, punpckldq(gc, mm0, mm0));
cb = emit_code(gc, cb, movq(gc, mmxtr.indexTextureBase, mm0));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, (int)&gc->texture.currentTexture), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, offsetof(CT.table, __GLtexture)), ebp, NULL, 1)));
	    }
        }

        if (interpolate_z) {
cb = emit_code(gc, cb, punpckldq(gc, mm4, mm4));
cb = emit_code(gc, cb, paddd(gc, mm4, mem(gc, value(gc, DISPOF(mmxtr.z.istagger)), NULL, eax, 8)));
        }

cb = emit_code(gc, cb, cmp(gc, ebx, value(gc, 4)));
cb = emit_code(gc, cb, jle(gc, short_span));

        if (interpolate_z) {
cb = emit_code(gc, cb, mov(gc, esi, tr(zp0)));
cb = emit_code(gc, cb, and(gc, esi, value(gc, (~(z_stride - 1)))));
        }
cb = emit_code(gc, cb, and(gc, edi, value(gc, (~(c_stride - 1)))));

        if (interpolate_z) {
	    static const GLuint mask_M[] = { 0x000000ff,
					     0x0000ffff,
					     0x00ffffff,
					     0xffffffff};

	    static const GLuint mask_1[] = { 0xffffffff,
				       0xffffff00,
				       0xffff0000,
				       0xff000000 };
	    struct celem* x = near_label(gc);

cb = genz(gc, cb, tr, &mmxtr, mem(gc, NULL, edi, NULL, 1), mem(gc, NULL, edi, NULL, 1), mem(gc, NULL, esi, NULL, 1), eax, dx, mem(gc, value(gc, (int)((dx == -1) ? &mask_M : &mask_1)), NULL, eax, 4), zwinh, zloseh, mixpix, texenv);
zloseh->spec.label.where = cb; resolve(gc, zloseh, cb);
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, ((dx == -1) ? 3 : 0))));
cb = emit_code(gc, cb, jne(gc, x));
cb = prepare(gc, cb, ending, tail, dx, z_stride, c_stride, interpolate_z, &mmxtr, modeFlags);
	    if (modeFlags & __GL_SHADE_TEXTURE_PERSP)
cb = emit_code(gc, cb, mov(gc, eax, mmxtr.next_pc));
cb = emit_code(gc, cb, jmp(gc, zlosec));
		
            /*
	     * We have to decrement ebx by the number of pixels drawn.
	     * This is a chart; the code actually uses cute math.
	     *
	     * eax    dx     dx
	     *        -1      1
	     *
	     * 0       1      4
             * 1       2      3
	     * 2       3      2
	     * 3       4      1
	     */
x->spec.label.where = cb; resolve(gc, x, cb);
            if (dx == -1) {
cb = emit_code(gc, cb, sub(gc, edi, value(gc, c_stride)));
cb = emit_code(gc, cb, sub(gc, ebx, eax));
cb = emit_code(gc, cb, sub(gc, esi, value(gc, z_stride)));
cb = emit_code(gc, cb, dec(gc, ebx));
            } else {
cb = emit_code(gc, cb, add(gc, edi, value(gc, c_stride)));
cb = emit_code(gc, cb, add(gc, ebx, eax));
cb = emit_code(gc, cb, add(gc, esi, value(gc, z_stride)));
cb = emit_code(gc, cb, sub(gc, ebx, value(gc, 4)));
            }
cb = emit_code(gc, cb, mov(gc, ending, ebx));
cb = emit_code(gc, cb, jmp(gc, no_head));
        }

zwinh->spec.label.where = cb; resolve(gc, zwinh, cb);
cb = emit_code(gc, cb, push(gc, eax));
cb = genpix(gc, cb, &pixval, tr, &mmxtr, c_stride, value(gc, 0), dx, mem(gc, NULL, edi, NULL, 1), texenv);
cb = emit_code(gc, cb, pop(gc, eax));

cb = emit_code(gc, cb, jmpi(gc, mem(gc, value(gc, DISPOF(headgate)), NULL, eax, 4)));

        {
	    struct celem
		*c0 = label(gc), 
		*c1 = label(gc), 
		*c2 = label(gc),
		*c3 = label(gc);
	    unsigned char** targets = (unsigned char**)DISPOF(headgate);

	    if (dx == -1) {
c0->spec.label.where = cb; resolve(gc, c0, cb);
cb = emit_code(gc, cb, dec(gc, ebx));

cb = emit_code(gc, cb, mov(gc, ending, ebx));

cb = emit_code(gc, cb, movd(gc, eax, pixval));
                if (c_stride == 8) {
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, sub(gc, edi, value(gc, c_stride)));
                if (interpolate_z) {
cb = emit_code(gc, cb, add(gc, esi, value(gc, z_stride * dx)));
                }
cb = emit_code(gc, cb, jmp(gc, no_head));

c1->spec.label.where = cb; resolve(gc, c1, cb);
cb = emit_code(gc, cb, sub(gc, ebx, value(gc, 2)));
                if (c_stride == 8) {
cb = emit_code(gc, cb, mov(gc, ending, ebx));
cb = emit_code(gc, cb, movd(gc, mem(gc, NULL, edi, NULL, 1), pixval));
		} else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, mov(gc, ending, ebx));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
	        }
cb = emit_code(gc, cb, sub(gc, edi, value(gc, c_stride)));
                if (interpolate_z) {
cb = emit_code(gc, cb, add(gc, esi, value(gc, z_stride * dx)));
                }
cb = emit_code(gc, cb, jmp(gc, no_head));

c2->spec.label.where = cb; resolve(gc, c2, cb);
cb = emit_code(gc, cb, sub(gc, ebx, value(gc, 3)));
                if (c_stride == 8) {
cb = emit_code(gc, cb, mov(gc, ending, ebx));
cb = emit_code(gc, cb, movd(gc, mem(gc, NULL, edi, NULL, 1), pixval));
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 32)));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 4), edi, NULL, 1), eax));
                } else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, mov(gc, ending, ebx));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), al));
	        }
cb = emit_code(gc, cb, sub(gc, edi, value(gc, c_stride)));
                if (interpolate_z) {
cb = emit_code(gc, cb, add(gc, esi, value(gc, z_stride * dx)));
                }
cb = emit_code(gc, cb, jmp(gc, no_head));

c3->spec.label.where = cb; resolve(gc, c3, cb);
cb = prepare(gc, cb, ending, tail, dx, z_stride, c_stride, interpolate_z, &mmxtr, modeFlags);
cb = emit_code(gc, cb, jmp(gc, central2));

	    } else {
c3->spec.label.where = cb; resolve(gc, c3, cb);
cb = emit_code(gc, cb, dec(gc, ebx));
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 48)));
cb = emit_code(gc, cb, mov(gc, ending, ebx));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
                if (c_stride == 8) {
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 6), edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 3), edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, add(gc, edi, value(gc, c_stride)));
                if (interpolate_z) {
cb = emit_code(gc, cb, add(gc, esi, value(gc, z_stride * dx)));
                }
cb = emit_code(gc, cb, jmp(gc, no_head));

c2->spec.label.where = cb; resolve(gc, c2, cb);
cb = emit_code(gc, cb, sub(gc, ebx, value(gc, 2)));
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 32)));

cb = emit_code(gc, cb, mov(gc, ending, ebx));
                if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, mem(gc, value(gc, 4), edi, NULL, 1), pixval));
		} else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), eax));
		}
cb = emit_code(gc, cb, add(gc, edi, value(gc, c_stride)));
                if (interpolate_z) {
cb = emit_code(gc, cb, add(gc, esi, value(gc, z_stride * dx)));
                }
cb = emit_code(gc, cb, jmp(gc, no_head));

c1->spec.label.where = cb; resolve(gc, c1, cb);
cb = emit_code(gc, cb, sub(gc, ebx, value(gc, 3)));
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, ending, ebx));
                if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 16)));

cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), eax));

cb = emit_code(gc, cb, movd(gc, mem(gc, value(gc, 4), edi, NULL, 1), pixval));
		} else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 1), edi, NULL, 1), eax));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 3), edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, add(gc, edi, value(gc, c_stride)));
                if (interpolate_z) {
cb = emit_code(gc, cb, add(gc, esi, value(gc, z_stride * dx)));
                }
cb = emit_code(gc, cb, jmp(gc, no_head));

c0->spec.label.where = cb; resolve(gc, c0, cb);
cb = prepare(gc, cb, ending, tail, dx, z_stride, c_stride, interpolate_z, &mmxtr, modeFlags);
		if (modeFlags & __GL_SHADE_TEXTURE_PERSP)
cb = emit_code(gc, cb, mov(gc, eax, mmxtr.next_pc));
cb = emit_code(gc, cb, jmp(gc, central2));
	    }

            targets[0] = c0->spec.label.where;
            targets[1] = c1->spec.label.where;
            targets[2] = c2->spec.label.where;
            targets[3] = c3->spec.label.where;
        }

no_head->spec.label.where = cb; resolve(gc, no_head, cb);
cb = prepare(gc, cb, NULL, tail, dx, z_stride, c_stride, interpolate_z, &mmxtr, modeFlags);

#if 1
central->spec.label.where = cb; resolve(gc, central, cb);
        if (interpolate_z) {
	    if (z_stride == 8) {
cb = genz(gc, cb, tr, &mmxtr, mem(gc, NULL, edi, ebx, c_stride), mem(gc, NULL, edi, ebx, c_stride), mem(gc, NULL, esi, ebx, 8), value(gc, 0), dx, NULL, zwinc, zlosec, mixpix, texenv);
            } else {
cb = emit_code(gc, cb, mov(gc, eax, ebx));
cb = emit_code(gc, cb, add(gc, eax, eax));
cb = genz(gc, cb, tr, &mmxtr, mem(gc, NULL, edi, ebx, c_stride), mem(gc, NULL, edi, ebx, c_stride), mem(gc, NULL, esi, eax, 8), value(gc, 0), dx, NULL, zwinc, zlosec, mixpix, texenv);
	    }
        }
zwinc->spec.label.where = cb; resolve(gc, zwinc, cb);
//@ opt_begin
cb = genpix(gc, cb, &pixval, tr, &mmxtr, c_stride, value(gc, 0), dx, mem(gc, NULL, edi, ebx, c_stride), texenv);
//@ opt_end
central2->spec.label.where = cb; resolve(gc, central2, cb);
	if (c_stride == 8) {
cb = emit_code(gc, cb, movq(gc, mem(gc, NULL, edi, ebx, 8), pixval));
        } else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, mem(gc, NULL, edi, ebx, 4), pixval));
        }
zlosec->spec.label.where = cb; resolve(gc, zlosec, cb);
        if (dx == 1) {
cb = emit_code(gc, cb, inc(gc, ebx));
	} else {
cb = emit_code(gc, cb, dec(gc, ebx));
	}
	if ((modeFlags & __GL_SHADE_TEXTURE) && 
	    (modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = emit_code(gc, cb, je(gc, tail));
cb = emit_code(gc, cb, mov(gc, eax, mmxtr.next_pc));
cb = emit_code(gc, cb, cmp(gc, ebx, eax));
cb = emit_code(gc, cb, jne(gc, central));

cb = emit_code(gc, cb, mov(gc, eax, mmxtr.next_pc));
cb = emit_code(gc, cb, add(gc, eax, value(gc, (dx == 1) ? 4 : -4)));

cb = emit_code(gc, cb, movq(gc, mm6, mmxtr.prev_st));

cb = emit_code(gc, cb, movq(gc, mm7, tr(dsdx)));
cb = emit_code(gc, cb, pslld(gc, mm7, value(gc, 4)));
cb = emit_code(gc, cb, paddd(gc, mm7, tr(s)));

cb = emit_code(gc, cb, mov(gc, edx, tr(dqdx)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(q)));
cb = emit_code(gc, cb, shl(gc, edx, value(gc, 4)));
cb = emit_code(gc, cb, add(gc, ecx, edx));
cb = emit_code(gc, cb, mov(gc, tr(q), ecx));
cb = emit_code(gc, cb, movq(gc, tr(s), mm7));
cb = emit_code(gc, cb, mov(gc, mmxtr.next_pc, eax));

cb = emit_code(gc, cb, call(gc, do_recip));
cb = perspective(gc, cb, mm7, ecx, mm1, mm3);

cb = emit_code(gc, cb, movq(gc, mmxtr.prev_st, mm7));

cb = emit_code(gc, cb, psubd(gc, mm7, mm6));
	    
	    /* mm7 is affine span.  Derive initial step, fourfold
	     * step, and stagger.  */
	    
cb = emit_code(gc, cb, psrld(gc, mm7, value(gc, 2)));
	    
	    /* mm7 is fourfold step: derive the others */
cb = emit_code(gc, cb, movq(gc, mm2, mm7));

cb = emit_code(gc, cb, pslld(gc, mm2, mmxtr.tex_s_kerrect));
cb = emit_code(gc, cb, movq(gc, mm3, mm7));

cb = emit_code(gc, cb, pslld(gc, mm3, mmxtr.tex_t_kerrect));
cb = emit_code(gc, cb, punpcklwd(gc, mm2, mm2));
cb = emit_code(gc, cb, punpckhwd(gc, mm3, mm3));
cb = emit_code(gc, cb, punpckhdq(gc, mm2, mm2));
cb = emit_code(gc, cb, punpckhdq(gc, mm3, mm3));

cb = emit_code(gc, cb, movq(gc, mmxtr.s.didx, mm2));
cb = emit_code(gc, cb, psraw(gc, mm2, value(gc, 2)));
cb = emit_code(gc, cb, movq(gc, mmxtr.t.didx, mm3));
cb = emit_code(gc, cb, pmullw(gc, mm2, mem(gc, value(gc, DISPOF(stagger_k) + ((dx == -1) ? 8 : 0)), NULL, NULL, 1)));
cb = emit_code(gc, cb, psraw(gc, mm3, value(gc, 2)));
cb = emit_code(gc, cb, pmullw(gc, mm3, mem(gc, value(gc, DISPOF(stagger_k) + ((dx == -1) ? 8 : 0)), NULL, NULL, 1)));

cb = emit_code(gc, cb, movq(gc, mm7, mm6));
cb = emit_code(gc, cb, pslld(gc, mm6, mmxtr.tex_s_kerrect));
cb = emit_code(gc, cb, pslld(gc, mm7, mmxtr.tex_t_kerrect));
cb = emit_code(gc, cb, punpcklwd(gc, mm6, mm6));
cb = emit_code(gc, cb, punpckhwd(gc, mm7, mm7));
cb = emit_code(gc, cb, punpckhdq(gc, mm6, mm6));
cb = emit_code(gc, cb, punpckhdq(gc, mm7, mm7));
cb = emit_code(gc, cb, paddw(gc, mm6, mm2));
cb = emit_code(gc, cb, paddw(gc, mm7, mm3));
cb = emit_code(gc, cb, jmp(gc, central));
	} else {
cb = emit_code(gc, cb, jne(gc, central));
        }
#else
central->spec.label.where = cb; resolve(gc, central, cb);
central2->spec.label.where = cb; resolve(gc, central2, cb);
#endif
tail->spec.label.where = cb; resolve(gc, tail, cb);

cb = emit_code(gc, cb, mov(gc, ebx, ending));
cb = emit_code(gc, cb, and(gc, ebx, value(gc, 3)));

	/* Don't do a thing if there's no span left */

cb = emit_code(gc, cb, je(gc, next_line));

        if (interpolate_z) {
	    static const GLuint mask_1[] = { 0xffffffff,
				       0x000000ff,
				       0x0000ffff,
				       0x00ffffff };
	    static const GLuint mask_M[] = { 0xffffffff,
				       0xff000000,
				       0xffff0000,
				       0xffffff00 };

cb = genz(gc, cb, tr, &mmxtr, mem(gc, NULL, edi, NULL, 1), mem(gc, NULL, edi, NULL, 1), mem(gc, NULL, esi, NULL, 1), NULL, dx, mem(gc, value(gc, (int)((dx == -1) ? &mask_M : &mask_1)), NULL, ebx, 4), zwint, next_line, mixpix, texenv);
        }
zwint->spec.label.where = cb; resolve(gc, zwint, cb);

cb = genpix(gc, cb, &pixval, tr, &mmxtr, c_stride, NULL, dx, mem(gc, NULL, edi, NULL, 1), texenv);
cb = emit_code(gc, cb, jmpi(gc, mem(gc, value(gc, DISPOF(tailgate)), NULL, ebx, 4)));

        {
	    struct celem
		*c0 = label(gc), 
		*c1 = label(gc), 
		*c2 = label(gc),
		*c3 = label(gc);
            unsigned char** targets = (unsigned char**)DISPOF(tailgate);

	    if (dx == -1) {
c1->spec.label.where = cb; resolve(gc, c1, cb);
		if (c_stride == 8) {
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 48)));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 6), edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 48)));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 3), edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, jmp(gc, next_line));

c2->spec.label.where = cb; resolve(gc, c2, cb);
		if (c_stride == 8) {
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 32)));
cb = emit_code(gc, cb, movd(gc, mem(gc, value(gc, 4), edi, NULL, 1), pixval));
		} else {
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 32)));
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), eax));
		}
cb = emit_code(gc, cb, jmp(gc, next_line));

c3->spec.label.where = cb; resolve(gc, c3, cb);
		if (c_stride == 8) {
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 16)));
cb = emit_code(gc, cb, movd(gc, mem(gc, value(gc, 2), edi, NULL, 1), pixval));
cb = emit_code(gc, cb, punpckhdq(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 6), edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 16)));
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 1), edi, NULL, 1), eax));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 3), edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, jmp(gc, next_line));

c0->spec.label.where = cb; resolve(gc, c0, cb);
	    } else {
c1->spec.label.where = cb; resolve(gc, c1, cb);
		if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, jmp(gc, c0));
    	
c2->spec.label.where = cb; resolve(gc, c2, cb);
		if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, mem(gc, NULL, edi, NULL, 1), pixval));
cb = emit_code(gc, cb, jmp(gc, c0));
		} else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
cb = emit_code(gc, cb, jmp(gc, c0));
		}
    	
c3->spec.label.where = cb; resolve(gc, c3, cb);
		if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, mem(gc, NULL, edi, NULL, 1), pixval));
cb = emit_code(gc, cb, punpckhdq(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 4), edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), al));
		}

c0->spec.label.where = cb; resolve(gc, c0, cb);
	    }

            targets[0] = c0->spec.label.where;
            targets[1] = c1->spec.label.where;
            targets[2] = c2->spec.label.where;
            targets[3] = c3->spec.label.where;
        }

next_line->spec.label.where = cb; resolve(gc, next_line, cb);
cb = emit_code(gc, cb, mov(gc, eax, tr(x0Frac)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(dxdy0Frac)));

cb = emit_code(gc, cb, add(gc, eax, ebx));
cb = emit_code(gc, cb, mov(gc, ebp, tr(cp0)));

cb = emit_code(gc, cb, mov(gc, ebx, eax));
cb = emit_code(gc, cb, and(gc, eax, value(gc, ~0x80000000UL)));

cb = emit_code(gc, cb, shr(gc, ebx, value(gc, 31)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(x0Int)));

cb = emit_code(gc, cb, mov(gc, edx, trs(dcpdy0)));
cb = emit_code(gc, cb, mov(gc, edi, trs(dxdy0Int)));

cb = emit_code(gc, cb, add(gc, ecx, edi));
cb = emit_code(gc, cb, add(gc, ebp, edx));

        if (interpolate_z) {
cb = emit_code(gc, cb, movq(gc, mm4, tr(z0)));
cb = emit_code(gc, cb, paddd(gc, mm4, trsq(dzdy0)));
cb = emit_code(gc, cb, movq(gc, tr(z0), mm4));
        }

        if (interpolate_tex) {
cb = emit_code(gc, cb, movq(gc, mm6, tr(s0)));
cb = emit_code(gc, cb, paddd(gc, mm6, trsq(dsdy0)));
cb = emit_code(gc, cb, movq(gc, tr(s0), mm6));

	    if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
cb = emit_code(gc, cb, movq(gc, mm0, tr(q0)));
cb = emit_code(gc, cb, paddd(gc, mm0, trsq(dqdy0)));
cb = emit_code(gc, cb, movq(gc, tr(q0), mm0));
            }
        }

	if (interpolate_color) {
cb = emit_code(gc, cb, movq(gc, mm5, tr(r0)));
cb = emit_code(gc, cb, movq(gc, mm7, tr(b0)));
cb = emit_code(gc, cb, paddd(gc, mm5, trsq(drdy0)));
cb = emit_code(gc, cb, paddd(gc, mm7, trsq(dbdy0)));

cb = emit_code(gc, cb, movq(gc, tr(r0), mm5));
cb = emit_code(gc, cb, movq(gc, tr(b0), mm7));
	}

cb = emit_code(gc, cb, mov(gc, tr(x0Int), ecx));
cb = emit_code(gc, cb, mov(gc, tr(cp0), ebp));
cb = emit_code(gc, cb, mov(gc, tr(x0Frac), eax));

cb = emit_code(gc, cb, mov(gc, ecx, tr(x1Frac)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(dxdy1Frac)));

cb = emit_code(gc, cb, add(gc, ecx, ebx));
	if (modeFlags & __GL_SHADE_DITHER)
cb = emit_code(gc, cb, mov(gc, ebp, tr(y)));

cb = emit_code(gc, cb, mov(gc, ebx, ecx));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, ~0x80000000UL)));

cb = emit_code(gc, cb, shr(gc, ebx, value(gc, 31)));
cb = emit_code(gc, cb, mov(gc, tr(x1Frac), ecx));

cb = emit_code(gc, cb, mov(gc, eax, h));
cb = emit_code(gc, cb, mov(gc, ecx, tr(x1Int)));

	if (modeFlags & __GL_SHADE_DITHER)
cb = emit_code(gc, cb, inc(gc, ebp));
cb = emit_code(gc, cb, mov(gc, ebx, trs(dxdy1Int)));

cb = emit_code(gc, cb, add(gc, ecx, ebx));
	if (modeFlags & __GL_SHADE_DITHER)
cb = emit_code(gc, cb, mov(gc, tr(y), ebp));

cb = emit_code(gc, cb, mov(gc, tr(x1Int), ecx));
cb = emit_code(gc, cb, dec(gc, eax));

cb = emit_code(gc, cb, jne(gc, perline));

done_trap->spec.label.where = cb; resolve(gc, done_trap, cb);
cb = emit_code(gc, cb, mov(gc, eax, tr(y1Int)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(y2Int)));
cb = emit_code(gc, cb, cmp(gc, eax, ebx));
cb = emit_code(gc, cb, je(gc, alldone));
cb = emit_code(gc, cb, mov(gc, tr(y0Int), ebx));
cb = emit_code(gc, cb, mov(gc, tr(y2Int), eax));

cb = memov(gc, cb, tr(x1Int), tr(x2Int));
cb = memov(gc, cb, tr(dxdy1Int[0]), tr(dxdy2Int[0]));
cb = memov(gc, cb, tr(dxdy1Int[1]), tr(dxdy2Int[1]));
cb = memov(gc, cb, tr(x1Frac), tr(x2Frac));
cb = memov(gc, cb, tr(dxdy1Frac), tr(dxdy2Frac));

cb = emit_code(gc, cb, jmp(gc, trap));

short_span->spec.label.where = cb; resolve(gc, short_span, cb);
cb = emit_code(gc, cb, mov(gc, edi, tr(cp0)));

        if (interpolate_z)
cb = emit_code(gc, cb, mov(gc, esi, tr(zp0)));

	/* We have to be careful here... if edi is right at the start
	 * of the color buffer, then subtracting 6 or 3 might leave it pointing
	 * out of the area.  qword accesses (such as the color pointer read in
	 * blending) might give us GPFs.  So we copy as much as we need, and set
	 * 'readfrom' to point to the copy.
	 */
        readfrom = mem2(gc, NULL, edi);
        if (dx == -1) {
	    /* Decrease both pointers by 3/4 of stride */
cb = emit_code(gc, cb, sub(gc, edi, value(gc, (3 * c_stride) / 4)));
            if (interpolate_z)
cb = emit_code(gc, cb, sub(gc, esi, value(gc, (3 * z_stride) / 4)));

	    if (modeFlags & __GL_SHADE_BLEND) {
		struct celem *copy = near_label(gc);

cb = emit_code(gc, cb, mov(gc, eax, value(gc, 3)));
cb = emit_code(gc, cb, mov(gc, edx, ebx));
copy->spec.label.where = cb; resolve(gc, copy, cb);

		if (c_stride == 8) {
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, NULL, edi, eax, 2)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, DISPOF(mmxtr.pixcopy)), NULL, eax, 2), ecx));
		} else {
cb = emit_code(gc, cb, mov(gc, cl, mem(gc, NULL, edi, eax, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, DISPOF(mmxtr.pixcopy)), eax, NULL, 1), cl));
		}

cb = emit_code(gc, cb, dec(gc, eax));
cb = emit_code(gc, cb, dec(gc, edx));
cb = emit_code(gc, cb, jne(gc, copy));

		readfrom = mmxtr.pixcopy;
	    } else {

	    }
        }

cb = emit_code(gc, cb, and(gc, ebx, value(gc, 3)));

	if (modeFlags & __GL_SHADE_DITHER) {
	    GLuint dt = (int)dithertab;

	    /* Oh dear.  We're about to do a misaligned write, so we need
	     * to rotate the dither pattern sync up with the screen.
	     * When looking at the rotates, keep in mind that 'left' in
	     * intelspeak is right on the screen.  Good luck.
	     */
cb = emit_code(gc, cb, mov(gc, eax, edi));
cb = emit_code(gc, cb, and(gc, eax, value(gc, c_stride - 1)));
            if (c_stride == 8)
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 1)));

cb = emit_code(gc, cb, mov(gc, ecx, tr(y)));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, DITHER_MASK)));

cb = emit_code(gc, cb, movd(gc, mm0, eax));
cb = emit_code(gc, cb, pslld(gc, mm0, value(gc, 4)));
cb = emit_code(gc, cb, movq(gc, mm1, mxk4(gc, 64,0,0,0)));
cb = emit_code(gc, cb, psubd(gc, mm1, mm0));

	    if (!interpolate_color && !interpolate_tex) {
		/* We have 4 precalculated pixels sitting in mm7. */
cb = emit_code(gc, cb, movq(gc, mm3, mm7));
cb = emit_code(gc, cb, psllq(gc, mm7, mm1));
cb = emit_code(gc, cb, psrlq(gc, mm3, mm0));
cb = emit_code(gc, cb, por(gc, mm7, mm3));
	    } else {
cb = emit_code(gc, cb, movq(gc, mm2, mem(gc, value(gc, dt), NULL, ecx, 8)));
cb = emit_code(gc, cb, movq(gc, mm3, mm2));
cb = emit_code(gc, cb, psllq(gc, mm2, mm1));
cb = emit_code(gc, cb, psrlq(gc, mm3, mm0));
cb = emit_code(gc, cb, por(gc, mm2, mm3));
cb = emit_code(gc, cb, movq(gc, mmxtr.dithers[0], mm2));
    
cb = emit_code(gc, cb, movq(gc, mm2, mem(gc, value(gc, dt + 32), NULL, ecx, 8)));
cb = emit_code(gc, cb, movq(gc, mm3, mm2));
cb = emit_code(gc, cb, psllq(gc, mm2, mm1));
cb = emit_code(gc, cb, psrlq(gc, mm3, mm0));
cb = emit_code(gc, cb, por(gc, mm2, mm3));
cb = emit_code(gc, cb, movq(gc, mmxtr.dithers[1], mm2));
    
cb = emit_code(gc, cb, movq(gc, mm2, mem(gc, value(gc, dt + 64), NULL, ecx, 8)));
cb = emit_code(gc, cb, movq(gc, mm3, mm2));
cb = emit_code(gc, cb, psllq(gc, mm2, mm1));
cb = emit_code(gc, cb, psrlq(gc, mm3, mm0));
cb = emit_code(gc, cb, por(gc, mm2, mm3));
cb = emit_code(gc, cb, movq(gc, mmxtr.dithers[2], mm2));
	    }

            if (interpolate_color &&
		!(modeFlags & (__GL_SHADE_TEXTURE | __GL_SHADE_BLEND))) {
cb = emit_code(gc, cb, paddw(gc, mm5, mmxtr.dithers[0]));
cb = emit_code(gc, cb, paddw(gc, mm6, mmxtr.dithers[1]));
cb = emit_code(gc, cb, paddw(gc, mm7, mmxtr.dithers[2]));
	    }
	}

        if (interpolate_z) {
	    static const GLuint mask_1[] = { 0xffffffff,
				       0x000000ff,
				       0x0000ffff,
				       0x00ffffff };
	    static const GLuint mask_M[] = { 0xffffffff,
				       0xff000000,
				       0xffff0000,
				       0xffffff00 };

cb = genz(gc, cb, tr, &mmxtr, mem(gc, NULL, edi, NULL, 1), readfrom, mem(gc, NULL, esi, NULL, 1), NULL, dx, mem(gc, value(gc, (int)((dx == -1) ? &mask_M : &mask_1)), NULL, ebx, 4), zwins, next_line, mixpix, texenv);
        }
zwins->spec.label.where = cb; resolve(gc, zwins, cb);
cb = genpix(gc, cb, &pixval, tr, &mmxtr, c_stride, NULL, dx, readfrom, texenv);
cb = emit_code(gc, cb, jmpi(gc, mem(gc, value(gc, DISPOF(minigate)), NULL, ebx, 4)));
    
    	    {
    		struct celem
    		    *c0 = label(gc), 
    		    *c1 = label(gc), 
    		    *c2 = label(gc),
    		    *c3 = label(gc);
    		unsigned char** targets = (unsigned char**)DISPOF(minigate);

		if (dx == -1) {
c1->spec.label.where = cb; resolve(gc, c1, cb);
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 48)));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
		if (c_stride == 8) {
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 6), edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 3), edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, jmp(gc, next_line));

c2->spec.label.where = cb; resolve(gc, c2, cb);
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 32)));
		if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, mem(gc, value(gc, 4), edi, NULL, 1), pixval));
		} else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), eax));
		}
cb = emit_code(gc, cb, jmp(gc, next_line));

c3->spec.label.where = cb; resolve(gc, c3, cb);
cb = emit_code(gc, cb, psrlq(gc, pixval, value(gc, 16)));
		if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, mem(gc, value(gc, 2), edi, NULL, 1), pixval));
cb = emit_code(gc, cb, punpckhdq(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 6), edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 1), edi, NULL, 1), eax));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 3), edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, jmp(gc, next_line));
    
c0->spec.label.where = cb; resolve(gc, c0, cb);
		    if (c_stride == 8) {
cb = emit_code(gc, cb, movq(gc, mem(gc, NULL, edi, NULL, 1), pixval));
		    } else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, mem(gc, NULL, edi, NULL, 1), pixval));
		    }
cb = emit_code(gc, cb, jmp(gc, next_line));
		} else {
c1->spec.label.where = cb; resolve(gc, c1, cb);
		if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, jmp(gc, next_line));
    	
c2->spec.label.where = cb; resolve(gc, c2, cb);
		if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, mem(gc, NULL, edi, NULL, 1), pixval));
		} else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
		}
cb = emit_code(gc, cb, jmp(gc, next_line));
    	
c3->spec.label.where = cb; resolve(gc, c3, cb);
		if (c_stride == 8) {
cb = emit_code(gc, cb, movd(gc, mem(gc, NULL, edi, NULL, 1), pixval));
cb = emit_code(gc, cb, punpckhdq(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 4), edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, eax, pixval));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), eax));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), al));
		}
cb = emit_code(gc, cb, jmp(gc, next_line));
    
c0->spec.label.where = cb; resolve(gc, c0, cb);
		    if (c_stride == 8) {
cb = emit_code(gc, cb, movq(gc, mem(gc, NULL, edi, NULL, 1), pixval));
		    } else {
cb = emit_code(gc, cb, packuswb(gc, pixval, pixval));
cb = emit_code(gc, cb, movd(gc, mem(gc, NULL, edi, NULL, 1), pixval));
		    }
cb = emit_code(gc, cb, jmp(gc, next_line));
		}
    	
    		targets[0] = c0->spec.label.where;
    		targets[1] = c1->spec.label.where;
    		targets[2] = c2->spec.label.where;
    		targets[3] = c3->spec.label.where;
            }
    }

alldone->spec.label.where = cb; resolve(gc, alldone, cb);

   if (gc->buffers.lock.unlockRenderBuffers != NULL) {
	    struct celem* noneed = label(gc);

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, (int)&(gc)->buffers.lock.unlockRenderBuffers), NULL, NULL, 1)));
cb = emit_code(gc, cb, test(gc, edx, edx));
cb = emit_code(gc, cb, je(gc, noneed));

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, (int)&(gc)->buffers.lock.lockCnt), NULL, NULL, 1)));
cb = emit_code(gc, cb, dec(gc, edx));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&(gc)->buffers.lock.lockCnt), NULL, NULL, 1), edx));
cb = emit_code(gc, cb, cmp(gc, edx, value(gc, 0)));
cb = emit_code(gc, cb, jne(gc, noneed));
cb = emit_code(gc, cb, mov(gc, edx, value(gc, (int)gc)));
cb = emit_code(gc, cb, push(gc, edx));
cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)&(gc)->buffers.lock.unlockRenderBuffers), NULL, NULL, 1)));
cb = emit_code(gc, cb, add(gc, esp, value(gc, 4)));
noneed->spec.label.where = cb; resolve(gc, noneed, cb);
    }

cb = emit_code(gc, cb, emms(gc));

#if __GL_CODEGEN_DESCRIBERS
cb = emit_code(gc, cb, rdtsc(gc));
cb = emit_code(gc, cb, add(gc, mem(gc, value(gc, (int)&last_describer->cyc[0]), NULL, NULL, 1), eax));
cb = emit_code(gc, cb, adc(gc, mem(gc, value(gc, (int)&last_describer->cyc[1]), NULL, NULL, 1), edx));
#endif /* __GL_CODEGEN_DESCRIBERS */

#if __GL_CODEGEN_PMON
cb = emit_code(gc, cb, push(gc, value(gc, 1)));
cb = emit_code(gc, cb, call(gc, endKernelIteration));
#endif /* __GL_CODEGEN_PMON */

/* Really a return, this is initialised at the start of setup. */
cb = emit_code(gc, cb, mov(gc, eax, value(gc, 1)));
cb = emit_code(gc, cb, jmpi(gc, mem(gc, value(gc, (int)&gc->ogState.rsave), NULL, NULL, 1)));

    if (interpolate_z) {
	struct celem* result;

/* Ptr to color destination in edi.  If blending, color source is 
 * in esi.
 */
mixpix->spec.label.where = cb; resolve(gc, mixpix, cb);
cb = emit_code(gc, cb, movq(gc, mmxtr.zresult, mm1));
cb = genpix(gc, cb, &result, tr, &mmxtr, c_stride, NULL, (1), mem(gc, NULL, esi, NULL, 1), texenv);

cb = emit_code(gc, cb, mov(gc, ecx, mmxtr.zresult));
        if (c_stride != 8) {
cb = emit_code(gc, cb, packuswb(gc, mm0, mm0));
	}

	/* Do a selective write, using the mask in ecx */
	{
	    struct celem
		*loop = near_label(gc),
		*lose = near_label(gc);

cb = emit_code(gc, cb, mov(gc, edx, value(gc, 0)));
loop->spec.label.where = cb; resolve(gc, loop, cb);
cb = emit_code(gc, cb, test(gc, ecx, value(gc, 0xff)));
cb = emit_code(gc, cb, je(gc, lose));
cb = emit_code(gc, cb, movd(gc, eax, mm0));
            if (c_stride == 8) {
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, edx, 2), eax));
	    } else {
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, edx, 1), al));
	    }
lose->spec.label.where = cb; resolve(gc, lose, cb);
cb = emit_code(gc, cb, psrlq(gc, mm0, value(gc, 16)));
cb = emit_code(gc, cb, shr(gc, ecx, value(gc, 8)));
cb = emit_code(gc, cb, inc(gc, edx));
cb = emit_code(gc, cb, cmp(gc, edx, value(gc, 4)));
cb = emit_code(gc, cb, jne(gc, loop));
	}

cb = emit_code(gc, cb, ret(gc));
    }

do_recip->spec.label.where = cb; resolve(gc, do_recip, cb);
cb = reciprocal(gc, cb, ecx);
cb = emit_code(gc, cb, ret(gc));

    return cb;
}
#endif

/* e_to_str: Convert enum to printable string */

STATIC char*
e_to_str(GLenum e)
{

#define E2S(e) \
case e: return (#e)

    switch (e) {
	E2S(GL_ZERO);
	E2S(GL_ONE);
	E2S(GL_DST_COLOR);
	E2S(GL_SRC_COLOR);
	E2S(GL_ONE_MINUS_DST_COLOR);
	E2S(GL_ONE_MINUS_SRC_COLOR);
	E2S(GL_SRC_ALPHA);
	E2S(GL_ONE_MINUS_SRC_ALPHA);
	E2S(GL_DST_ALPHA);
	E2S(GL_ONE_MINUS_DST_ALPHA);
	E2S(GL_SRC_ALPHA_SATURATE);

    default: return "?";
    }

#undef E2S
}

STATIC GLuint
bf_to_index(GLenum x)
{
    switch (x) {
    case GL_ZERO:		return 0;
    case GL_ONE:		return 1;
    case GL_SRC_COLOR:		return 2;
    case GL_ONE_MINUS_SRC_COLOR:return 3;
    case GL_SRC_ALPHA:		return 4;
    case GL_ONE_MINUS_SRC_ALPHA:return 5;
    case GL_DST_ALPHA:		return 6;
    case GL_ONE_MINUS_DST_ALPHA:return 7;
    case GL_DST_COLOR:		return 8;
    case GL_ONE_MINUS_DST_COLOR:return 9;
    case GL_SRC_ALPHA_SATURATE: return 10;

    default:
	assert(0);
    }
}

rasterizer
GenerateRasterizer(__GLcontext *gc,
		   GLboolean spanlet)
{
  unsigned char *space;
  unsigned char *data;
  unsigned char *cb;
  struct __GLfptri fptr;
  struct celem
    *trap,
    *alldone,
    *canal,
    *nextline,
    *hzero;
  int locals = 0;
  struct celem *tr = NULL;
  struct celem *h = NULL;
  struct celem *w = NULL;
  struct celem *truew = NULL;
  struct celem *dither_index = NULL;
  struct celem *rptr = NULL, *gptr = NULL, *bptr = NULL;
  struct celem *dithertable = NULL;
  struct celem *dithervalue = NULL;
  struct celem
    *drdxH, *dgdxH, *dbdxH,	/* Used in smoothspan */
    *drdxP, *dgdxP, *dbdxP;
  GLuint modeFlags = gc->polygon.shader.modeFlags;
  __GLtexture *current;
  __GLmipMapLevel *lp;
  GLuint wl2, hl2;
  int fbsize;
  GLboolean useFlatspan;
  GLboolean useSmoothspan;
  unsigned int signature;
  int
    interpolate_color,
    interpolate_z;
  GLboolean texNeedRho;
  GLenum texenv;

  /* Create a unique cache signature that covers information not
   * included in modeFlags.
   */
  signature = 0;

  if (modeFlags & __GL_SHADE_TEXTURE) {
      __GLtexture *current = gc->texture.currentTexture;
      texNeedRho = current->params.minFilter != current->params.magFilter;
      texenv = gc->state.texture.env[0].mode;
      if (texenv == GL_DECAL &&
	  gc->texture.currentTexture->texelFormat == GL_RGB)
	  texenv = GL_REPLACE;
  } else {
      texNeedRho = GL_FALSE;
  }

  if (spanlet) {
      signature |= __OG_RASTER_SPANLETS;
  } else {

      if (modeFlags & __GL_SHADE_TEXTURE) {
	  __GLtexture *current = gc->texture.currentTexture;
	  int baseFormat;

	  /* Texture environment */
	  switch (texenv) {
	  case GL_MODULATE:
	      signature |= (0<<__OG_RASTER_TEX_ENV_SHIFT);
	      break;
	  case GL_DECAL:
	      signature |= (1<<__OG_RASTER_TEX_ENV_SHIFT);
	      break;
	  case GL_BLEND:
	      signature |= (2<<__OG_RASTER_TEX_ENV_SHIFT);
	      break;
	  case GL_REPLACE:
	      signature |= (3<<__OG_RASTER_TEX_ENV_SHIFT);
	      break;
	  case GL_ADD:
	      signature |= (4<<__OG_RASTER_TEX_ENV_SHIFT);
	      break;
	  }

	  /* Texture size */
	  assert(current->level[0]->widthLog2 < 16);
	  assert(current->level[0]->heightLog2 < 16);

	  /* MMX renderer doesn't care about texture width... */
	  /* ... but on MMX we use the bits for somethings else (see below) */
	  if (!MMX || !__glCanMmx(gc)) {
	      signature |=
		  (current->level[0]->widthLog2 << __OG_RASTER_TEX_WIDTH_SHIFT) |
		  (current->level[0]->heightLog2 << __OG_RASTER_TEX_HEIGHT_SHIFT);
	  }

	  /* Texture format */
	  baseFormat = current->texelFormat;
	  if (baseFormat == GL_COLOR_INDEX) {
	      signature |= __OG_RASTER_TEX_INDEX;
	      if (modeFlags & __GL_SHADE_RGB)
		  baseFormat = current->CT.baseFormat;
	  }

	  switch (baseFormat) {
	  case GL_LUMINANCE:
	      signature |= (0 << __OG_RASTER_TEX_FORMAT_SHIFT);
	      break;
	  case GL_LUMINANCE_ALPHA:
	      signature |= (1 << __OG_RASTER_TEX_FORMAT_SHIFT);
	      break;
	  case GL_RGB:
	      signature |= (2 << __OG_RASTER_TEX_FORMAT_SHIFT);
	      break;
	  case GL_RGBA:
	      signature |= (3 << __OG_RASTER_TEX_FORMAT_SHIFT);
	      break;
	  case GL_INTENSITY:
	      signature |= (4 << __OG_RASTER_TEX_FORMAT_SHIFT);
	      break;
	  case GL_ALPHA:
	      signature |= (5 << __OG_RASTER_TEX_FORMAT_SHIFT);
	      break;
	  }

#if 0 /* Currently this falls back to spanlet pipeline */
	  /* Texture wrap mode */
	  if (current->params.sWrapMode != GL_REPEAT)
	      signature |= __OG_RASTER_TEX_S_CLAMP;
	  if (current->params.tWrapMode != GL_REPEAT)
	      signature |= __OG_RASTER_TEX_S_CLAMP;

	  /* Texture border */
	  if (current->level[0]->border)
	      signature |= __OG_RASTER_TEX_BORDER;
	
	  /* Texture filter */
	  if (current->params.minFilter != GL_NEAREST)
	      signature |= __OG_RASTER_TEX_MIN_LINEAR;

	  switch (current->params.magFilter) {
	  case GL_NEAREST:
	      signature |= (0<<__OG_RASTER_TEX_MAX_FILTER_SHIFT);
	      break;
	  case GL_LINEAR:
	      signature |= (1<<__OG_RASTER_TEX_MAX_FILTER_SHIFT);
	      break;
	  case GL_NEAREST_MIPMAP_NEAREST:	
	      signature |= (2<<__OG_RASTER_TEX_MAX_FILTER_SHIFT);
	      break;
	  case GL_LINEAR_MIPMAP_NEAREST:
	      signature |= (3<<__OG_RASTER_TEX_MAX_FILTER_SHIFT);
	      break;
	  case GL_NEAREST_MIPMAP_LINEAR:
	      signature |= (4<<__OG_RASTER_TEX_MAX_FILTER_SHIFT);
	      break;
	  case GL_LINEAR_MIPMAP_LINEAR:
	      signature |= (5<<__OG_RASTER_TEX_MAX_FILTER_SHIFT);
	      break;
	  }
#endif
      }
      if (MMX && __glCanMmx(gc)) {
	  /* Use these bits to represent the blend func.
	   */
	  if (modeFlags & __GL_SHADE_BLEND) {
	      signature |=
		  ((bf_to_index(gc->state.raster.blendSrc) << 
		    __OG_RASTER_TEX_WIDTH_SHIFT) |
		   (bf_to_index(gc->state.raster.blendDst) << 
		    __OG_RASTER_TEX_HEIGHT_SHIFT));
	  }
      }

#if 0  /* Currently this falls back to spanlet pipeline */
      /* Scissor or front-buffer clipmask */
      if (!gc->transform.reasonableViewport) {
	  signature |= __OG_RASTER_CLIP_SCISSOR;
      }
#endif

      if (modeFlags & __GL_SHADE_DEPTH_TEST) {

	  signature |= (gc->state.depth.testFunc & 7) <<
	      __OG_RASTER_DEPTH_FUNC_SHIFT;

	  if (gc->depthBuffer.buf.depth > 16) {
	      signature |= __OG_RASTER_DEPTH_SIZE32;
	  }
	  if (gc->state.depth.writeEnable) {
	      signature |= __OG_RASTER_DEPTH_WRITE;
	  }
      }
  }
  if (gc->buffers.doubleStore) {
      signature |= __OG_RASTER_DOUBLESTORE;
  }
  if (texNeedRho) {
      signature |= __OG_RASTER_RHO;
  }

  if (__glAllocCodeSpace(&gc->triRasterCache, 
			 modeFlags & MODEFLAGS_RASTERIZER_CARES_ABOUT, 
			 signature, 
			 &gc->tr,
			 &space)) {
      /* Found existing rasterizer which satisfies request! */
      return (rasterizer) (space + 2000);
  }
  data = (unsigned char*)(((GLuint)space + 7) & ~7UL);
  space += 2000;

  cb = space;

#if __GL_CODEGEN_DESCRIBERS
  /* New rasterizer, make a new describer */
  {
    static const char* znames[] = {
      "GL_NEVER",
      "GL_LESS",
      "GL_EQUAL",
      "GL_LEQUAL",
      "GL_GREATER",
      "GL_NOTEQUAL",
      "GL_GEQUAL",
      "GL_ALWAYS"
    };
    struct describer* new = malloc(sizeof(*new));
    char *txt = new->name;

    sprintf(txt, "%08x ", modeFlags);
    txt += strlen(txt);

    if (spanlet) {
	int tm;
	static const char* texenvs[] = {
	    "GL_MODULATE",
	    "GL_DECAL",
	    "GL_BLEND",
	    "GL_REPLACE",
	    "GL_ADD"
	};

	tm = gc->state.texture.env[0].mode;
	sprintf(txt,
		"Spanlet, %s%s",
		(modeFlags & __GL_SHADE_TEXTURE) ? (
		    (tm == GL_MODULATE) ? "GL_MODULATE" :
		    (tm == GL_DECAL) ? "GL_DECAL" :
		    (tm == GL_BLEND) ? "GL_BLEND" :
		    (tm == GL_REPLACE) ? "GL_REPLACE" :
		    (tm == GL_ADD) ? "GL_ADD" : "no texenv") :
		"", 
		gc->transform.reasonableViewport ? "" : " unreasonable viewport");

    } else {
      if (modeFlags & __GL_SHADE_DEPTH_TEST &&
	  (gc->state.depth.writeEnable ||
	   gc->state.depth.testFunc != GL_ALWAYS)) {
	sprintf(txt,
		"Z %s %swrite %d bit, ",
		znames[gc->state.depth.testFunc - GL_NEVER],
		gc->state.depth.writeEnable ? "" : "no ",
		gc->depthBuffer.buf.depth);
      } else {
	sprintf(txt, "no Z, ");
      }
      txt += strlen(txt);
      
      sprintf(txt, "%sdither, ", (modeFlags & __GL_SHADE_DITHER) ? "" : "no ");
      txt += strlen(txt);
      
      sprintf(txt, "%s, ", (modeFlags & __GL_SHADE_SMOOTH) ? "smooth" : "flat");
      txt += strlen(txt);
      
      if (modeFlags & __GL_SHADE_TEXTURE) {
	__GLtexture *current;
	__GLmipMapLevel *lp;
	GLenum baseFormat;

	current = gc->texture.currentTexture;
	lp = current->level[0];

	sprintf(txt, "%s ", (modeFlags & __GL_SHADE_TEXTURE_PERSP) ? "PC" : "affine");
	txt += strlen(txt);
	sprintf(txt, "%dx%d ", lp->width, lp->height);
	txt += strlen(txt);
	sprintf(txt,
		"%s",
		(lp->pixelBuffer != NULL) ? "cooked " : "");
	txt += strlen(txt);

	baseFormat = current->texelFormat;
	if ((modeFlags & __GL_SHADE_RGB) && baseFormat == GL_COLOR_INDEX) {
	  sprintf(txt, "indexed ");
	  txt += strlen(txt);
	  baseFormat = current->CT.baseFormat;
	}
	switch (baseFormat) {
	case GL_COLOR_INDEX:	sprintf(txt, "GL_COLOR_INDEX "); break;
	case GL_LUMINANCE:	sprintf(txt, "GL_LUMINANCE "); break;
	case GL_INTENSITY:	sprintf(txt, "GL_INTENSITY "); break;
	case GL_LUMINANCE_ALPHA:	sprintf(txt, "GL_LUMINANCE_ALPHA "); break;
	case GL_RGB:	sprintf(txt, "GL_RGB "); break;
	case GL_RGBA:	sprintf(txt, "GL_RGBA "); break;
	default:	sprintf(txt, "? "); break;
	}
	txt += strlen(txt);

	sprintf(txt,
		"%s",
		(gc->state.texture.env[0].mode == GL_MODULATE) ? "modulate" :
		(gc->state.texture.env[0].mode == GL_DECAL) ? "decal" :
		(gc->state.texture.env[0].mode == GL_REPLACE) ? "replace":
		(gc->state.texture.env[0].mode == GL_ADD) ? "add" : "?");
   
      } else {
	sprintf(txt, "no texture");
      }
      if (modeFlags & __GL_SHADE_BLEND) {
	  txt += strlen(txt);
	  sprintf(txt, 
		  " blend(%s, %s)",
		  e_to_str(gc->state.raster.blendSrc),
		  e_to_str(gc->state.raster.blendDst));
      }
    }

    new->pixels = 0;
    new->spans = 0;
    new->tot_tris = 0;
    new->drawn_tris = 0;
    new->easy_tris = 0;
    new->cyc[0] = 0;
    new->cyc[1] = 0;

    new->prev = last_describer;
    last_describer = new;
  }
#endif /* __GL_CODEGEN_DESCRIBERS */

  __glNSOpenSpace(gc->tr);
  if (spanlet || FORCE_DIRECT)
    gc->tr->direct_map = GL_TRUE;

#if MMX
  if (!spanlet && __glCanMmx(gc)) {
      struct celem *scalar;
      unsigned char* end;
      GLboolean makeScalarToo;

      makeScalarToo =
	  !(modeFlags & __GL_SHADE_TEXTURE) &&
	  !(modeFlags & __GL_SHADE_BLEND);

      og_warmup(gc);

      MENTION(c);

      if (makeScalarToo) {
	  /* This early on, esp points to 'tr'
	   */
	  tr = mem2(gc, value(gc, 0), esp);
	  scalar = label(gc);

cb = emit_code(gc, cb, mov(gc, eax, tr(c)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 0x7fffffff)));
cb = emit_code(gc, cb, cmp(gc, eax, fpk(gc, 25.0f * 2.0f)));
cb = emit_code(gc, cb, jle(gc, scalar));
      }

      end = BuildMMXRasterizer(gc, cb);

      assert((end - space) < 0x3800);
      cb = end;

      if (!makeScalarToo) {
//	  __glNSCloseSpace(gc->tr);
	  return (rasterizer) space;
      } else {
scalar->spec.label.where = cb; resolve(gc, scalar, cb);
      }
  }
#endif

  interpolate_color = 0;
  if (modeFlags & __GL_SHADE_SMOOTH) {
    interpolate_color = 1;
  }
  if (modeFlags & __GL_SHADE_TEXTURE) {
    switch (texenv) {
    case GL_MODULATE:
    case GL_ADD:
      break;
    case GL_REPLACE:
      interpolate_color = 0;
      break;
    }
  }
  interpolate_z = (modeFlags & __GL_SHADE_DEPTH_TEST &&
		   (gc->state.depth.writeEnable ||
		    gc->state.depth.testFunc != GL_ALWAYS));


#define MENTION(i)								\
  __glNSWhereIs(gc->tr, offsetof(i, __GLtri), sizeof(GLint));

  MENTION(y0Int);
  MENTION(y1Int);
  MENTION(y2Int);

  MENTION(x0Int);
  MENTION(x1Int);
  MENTION(x2Int);

  MENTION(dxdy0Int[0]);
  MENTION(dxdy0Int[1]);
  MENTION(dxdy1Int[0]);
  MENTION(dxdy1Int[1]);
  MENTION(dxdy2Int[0]);
  MENTION(dxdy2Int[1]);

  MENTION(x0Frac);
  MENTION(x1Frac);
  MENTION(x2Frac);

  MENTION(dxdy0Frac);
  MENTION(dxdy1Frac);
  MENTION(dxdy2Frac);

  MENTION(dx);
  MENTION(c);

#define INTERPOLANT(i)								\
  MENTION(i);			\
  MENTION(i##0);		\
  MENTION(d##i##dx);		\
  MENTION(d##i##dy0[0]);	\
  MENTION(d##i##dy0[1]);

#define FPINTERPOLANT(i)	\
  MENTION(f##i##w);		\
  MENTION(f##i##w0);		\
  MENTION(fd##i##wdx);		\
  MENTION(fd##i##wdxPWL);	\
  MENTION(fd##i##wdy0[0]);	\
  MENTION(fd##i##wdy0[1]);  

  if (interpolate_color) {
    INTERPOLANT(r);
    if (modeFlags & __GL_SHADE_RGB) {
      INTERPOLANT(g);
      INTERPOLANT(b);
      if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_BLEND)) {
	INTERPOLANT(a);
      }
    }
  }
  if (interpolate_z ) {
    INTERPOLANT(z);
  }
  if (modeFlags & __GL_SHADE_TEXTURE) {
    if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
      FPINTERPOLANT(s);
      FPINTERPOLANT(t);
      FPINTERPOLANT(q);
    } else {
      INTERPOLANT(s);
      INTERPOLANT(t);
    }
  }

  og_warmup(gc);
  this_warmup(gc, &fptr, (void*)data);

  trap = label(gc);
  alldone = label(gc);
  canal = label(gc);
  nextline = label(gc);
  hzero = label(gc);

  switch (gc->drawBuffer->fbtype) {
  case 0:
  case CINDEX:
  case RGB332:
  case RGB666:
    fbsize = 1;
    break;
  case RGB5:
  case RGB565:
  case A1RGB5:
    fbsize = 2;
    break;
  case RGB8:
  case BGR8:
    fbsize = 3;
    break;
  case RGBX8:
  case XRGB8:
  case XBGR8:
  case RGBA8:
  case ARGB8:
  case ABGR8:
    fbsize = 4;
    break;
  }

  useFlatspan = !spanlet &&
                (fbsize != 3) &&
		(!(modeFlags & __GL_SHADE_TEXTURE) &&
		 !(modeFlags & __GL_SHADE_SMOOTH) &&
		 !(modeFlags & __GL_SHADE_MASK) &&
		 !((modeFlags & __GL_SHADE_DEPTH_TEST &&
		    (gc->state.depth.writeEnable ||
		     gc->state.depth.testFunc != GL_ALWAYS))));
  useSmoothspan = GL_FALSE;
#if 0
 (!(modeFlags & __GL_SHADE_TEXTURE) &&
		   (modeFlags & __GL_SHADE_SMOOTH) &&
		   !((modeFlags & __GL_SHADE_DEPTH_TEST &&
		      (gc->state.depth.writeEnable ||
		       gc->state.depth.testFunc != GL_ALWAYS))));
#endif

#define SLOCAL(type, name)	(name = mem2(gc, value(gc, locals), esp), locals += sizeof(type))

#if __GL_CODEGEN_DESCRIBERS
cb = emit_code(gc, cb, inc(gc, mem(gc, value(gc, (int)&last_describer->drawn_tris), NULL, NULL, 1)));
cb = emit_code(gc, cb, rdtsc(gc));
cb = emit_code(gc, cb, sub(gc, mem(gc, value(gc, (int)&last_describer->cyc[0]), NULL, NULL, 1), eax));
cb = emit_code(gc, cb, sbb(gc, mem(gc, value(gc, (int)&last_describer->cyc[1]), NULL, NULL, 1), edx));
#endif /* __GL_CODEGEN_DESCRIBERS */

#if DEBUG
	{
	struct celem* ok = near_label(gc);

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&(gc->polygon.shader.modeFlags)), NULL, NULL, 1)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, MODEFLAGS_RASTERIZER_CARES_ABOUT)));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, (modeFlags & MODEFLAGS_RASTERIZER_CARES_ABOUT))));
cb = emit_code(gc, cb, je(gc, ok));
cb = emit_code(gc, cb, int3(gc));
ok->spec.label.where = cb; resolve(gc, ok, cb);
	}
#endif

  SLOCAL(long, w);
  SLOCAL(long, truew);
  SLOCAL(long, h);
  if (modeFlags & __GL_SHADE_DITHER)
    SLOCAL(long, dither_index);
  if (modeFlags & __GL_SHADE_TEXTURE) {
    if ((texenv == GL_MODULATE) &&
	!(modeFlags & __GL_SHADE_SMOOTH)) {
      SLOCAL(long, rptr);
      SLOCAL(long, gptr);
      SLOCAL(long, bptr);
    }
  }

  if (useFlatspan) {
    SLOCAL(long, dithervalue);
    dithertable = mem2(gc, value(gc, locals), esp);
    locals += fbsize * 16;
  }

  if (useSmoothspan) {
    SLOCAL(long, drdxH);
    SLOCAL(long, dgdxH);
    SLOCAL(long, dbdxH);
    SLOCAL(long, drdxP);
    SLOCAL(long, dgdxP);
    SLOCAL(long, dbdxP);
  }

  tr = mem2(gc, value(gc, locals), esp);

cb = emit_code(gc, cb, sub(gc, esp, value(gc, locals)));

  if (modeFlags & __GL_SHADE_TEXTURE) {
    current = gc->texture.currentTexture;
    lp = current->level[0];

    wl2 = lp->widthLog2;
    hl2 = lp->heightLog2;
  }

  if ((modeFlags & __GL_SHADE_TEXTURE) &&
      (texenv == GL_MODULATE) &&
      !(modeFlags & __GL_SHADE_SMOOTH)) {
      __GLcolorBuffer *cfb = gc->drawBuffer;
      int redshift, greenshift, blueshift;

	if (!(modeFlags & __GL_SHADE_DITHER)) {
	    redshift = blueshift = greenshift = 8;
	} else {
	    redshift = log2(cfb->redMax) + 1;
	    greenshift = log2(cfb->greenMax) + 1;
	    blueshift = log2(cfb->blueMax) + 1;
	}

	/* Work out the color lookup lines */
cb = emit_code(gc, cb, mov(gc, eax, tr(r)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, redshift)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 0xff00)));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, (int)&__glBlendTab), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, rptr, eax));

cb = emit_code(gc, cb, mov(gc, eax, tr(g)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, greenshift)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 0xff00)));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, (int)&__glBlendTab), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, gptr, eax));

cb = emit_code(gc, cb, mov(gc, eax, tr(b)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, blueshift)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 0xff00)));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, (int)&__glBlendTab), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, bptr, eax));

  }

  if ((modeFlags & __GL_SHADE_TEXTURE) &&
      ((modeFlags & __GL_SHADE_TEXTURE_PERSP) == 0)) {

cb = emit_code(gc, cb, mov(gc, ecx, tr(dtdx)));
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, 16 - hl2)));

cb = emit_code(gc, cb, mov(gc, eax, tr(dsdx)));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16 - wl2)));

cb = emit_code(gc, cb, mov(gc, ebx, eax));
cb = emit_code(gc, cb, mov(gc, edx, ecx));

cb = emit_code(gc, cb, and(gc, ebx, value(gc, 0xf000)));
cb = emit_code(gc, cb, and(gc, edx, value(gc, 0xf000)));

cb = emit_code(gc, cb, shr(gc, ebx, value(gc, 12)));
cb = emit_code(gc, cb, shl(gc, edx, value(gc, 4)));

cb = emit_code(gc, cb, or(gc, ebx, edx));

cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16)));

cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, ecx, eax));

cb = emit_code(gc, cb, mov(gc, eax, ecx));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 0x8000)));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 1)));
cb = emit_code(gc, cb, sub(gc, ecx, eax));
cb = emit_code(gc, cb, mov(gc, fptr.dstdx, ecx));

cb = emit_code(gc, cb, mov(gc, eax, ebx));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 0x8000)));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 1)));
cb = emit_code(gc, cb, sub(gc, ebx, eax));
cb = emit_code(gc, cb, mov(gc, fptr.dstdx2, ebx));

  }

  if (useFlatspan) {
    __GLcolorBuffer *cfb = gc->drawBuffer;
    struct celem *perline, *perpixel;

    perline = label(gc);
    perpixel = label(gc);

    /* Set up the dither table */
cb = emit_code(gc, cb, lea(gc, ebp, dithertable));

cb = emit_code(gc, cb, mov(gc, esi, value(gc, 0)));
perline->spec.label.where = cb; resolve(gc, perline, cb);
cb = emit_code(gc, cb, mov(gc, edi, value(gc, 0)));

perpixel->spec.label.where = cb; resolve(gc, perpixel, cb);
	if (modeFlags & __GL_SHADE_RGB) {
cb = emit_code(gc, cb, mov(gc, eax, tr(r)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(g)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(b)));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, (int)__glFRDitherTable), esi, edi, 4)));
    
cb = emit_code(gc, cb, add(gc, eax, edx));
cb = emit_code(gc, cb, add(gc, ebx, edx));
cb = emit_code(gc, cb, add(gc, ecx, edx));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, cfb->redShift)));
cb = emit_code(gc, cb, shr(gc, ebx, value(gc, 16)));
cb = emit_code(gc, cb, shl(gc, ebx, value(gc, cfb->greenShift)));
cb = emit_code(gc, cb, shr(gc, ecx, value(gc, 16)));
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, cfb->blueShift)));
cb = emit_code(gc, cb, or(gc, eax, ebx));
cb = emit_code(gc, cb, or(gc, eax, ecx));
	} else {
cb = emit_code(gc, cb, mov(gc, eax, tr(r)));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, (int)__glFRDitherTable), esi, edi, 4)));
cb = emit_code(gc, cb, add(gc, eax, edx));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16)));
	}

	switch (fbsize) {
	case 1:
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 0), ebp, NULL, 1), al));
	    break;
	case 2:
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 0), ebp, NULL, 1), eax));
	    break;
	case 3:
	    /* unsupported */
	    assert(0);
	    break;
	case 4:
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 0), ebp, NULL, 1), eax));
	    break;
	}
cb = emit_code(gc, cb, add(gc, ebp, value(gc, fbsize)));

cb = emit_code(gc, cb, inc(gc, edi));
cb = emit_code(gc, cb, cmp(gc, edi, value(gc, 4)));
cb = emit_code(gc, cb, jne(gc, perpixel));

cb = emit_code(gc, cb, add(gc, esi, value(gc, (4 * 35))));
cb = emit_code(gc, cb, cmp(gc, esi, value(gc, (4 * 4 * 35))));
cb = emit_code(gc, cb, jne(gc, perline));
  }

  if (useSmoothspan) {
    __GLcolorBuffer *cfb = gc->drawBuffer;
    int redSize, greenSize, blueSize;
    struct celem* perpixelpair = label(gc);
    struct celem* done = label(gc);

    redSize = log2(cfb->redMax);
    greenSize = log2(cfb->greenMax);
    blueSize = log2(cfb->blueMax);

cb = emit_code(gc, cb, mov(gc, eax, tr(drdx)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(dgdx)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(dbdx)));

cb = emit_code(gc, cb, shl(gc, eax, value(gc, 1)));
cb = emit_code(gc, cb, shl(gc, ebx, value(gc, 1)));
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, 1)));

cb = emit_code(gc, cb, shr(gc, eax, value(gc, (15 - redSize) - cfb->redShift)));
cb = emit_code(gc, cb, shr(gc, ebx, value(gc, (15 - greenSize) - cfb->greenShift)));
cb = emit_code(gc, cb, shr(gc, ecx, value(gc, (15 - blueSize) - cfb->blueShift)));

cb = emit_code(gc, cb, mov(gc, edx, eax));
cb = emit_code(gc, cb, mov(gc, ebp, ebx));
cb = emit_code(gc, cb, mov(gc, esi, ecx));

cb = emit_code(gc, cb, shl(gc, edx, value(gc, 16)));
cb = emit_code(gc, cb, shl(gc, ebp, value(gc, 16)));
cb = emit_code(gc, cb, shl(gc, esi, value(gc, 16)));

cb = emit_code(gc, cb, or(gc, eax, edx));
cb = emit_code(gc, cb, or(gc, ebx, ebp));
cb = emit_code(gc, cb, or(gc, ecx, esi));
	
cb = emit_code(gc, cb, mov(gc, drdxP, eax));
cb = emit_code(gc, cb, mov(gc, dgdxP, ebx));
cb = emit_code(gc, cb, mov(gc, dbdxP, ecx));

cb = emit_code(gc, cb, xor(gc, eax, eax));
cb = emit_code(gc, cb, mov(gc, drdxH, eax));
cb = emit_code(gc, cb, mov(gc, dgdxH, eax));
cb = emit_code(gc, cb, mov(gc, dbdxH, eax));

  }

  if ((modeFlags & (__GL_SHADE_SMOOTH | __GL_SHADE_DITHER | __GL_SHADE_TEXTURE)) == 0) {
cb = emit_code(gc, cb, mov(gc, edx, tr(r0)));
  }

trap->spec.label.where = cb; resolve(gc, trap, cb);

cb = emit_code(gc, cb, mov(gc, eax, tr(y2Int)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(y0Int)));
cb = emit_code(gc, cb, sub(gc, eax, ebx));
cb = emit_code(gc, cb, mov(gc, tr(y), ebx));
cb = emit_code(gc, cb, je(gc, hzero));

canal->spec.label.where = cb; resolve(gc, canal, cb);
cb = emit_code(gc, cb, mov(gc, h, eax));

  if (modeFlags & __GL_SHADE_DITHER) {
    if (!useFlatspan) {
cb = set_dither(gc, cb, tr, tr(x0Int), spanlet);
    } else {

cb = emit_code(gc, cb, mov(gc, ecx, tr(y)));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, DITHER_MASK)));
cb = emit_code(gc, cb, lea(gc, edi, dithertable));
      if (fbsize == 1)
cb = emit_code(gc, cb, lea(gc, edi, mem(gc, NULL, edi, ecx, 4)));
      else
cb = emit_code(gc, cb, lea(gc, edi, mem(gc, NULL, edi, ecx, 8)));
cb = emit_code(gc, cb, mov(gc, dithervalue, edi));
    }
  }

cb = emit_code(gc, cb, mov(gc, eax, tr(x0Int)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(x1Int)));

cb = emit_code(gc, cb, sub(gc, ebx, eax));
cb = emit_code(gc, cb, jle(gc, nextline));

#if __GL_CODEGEN_DESCRIBERS
cb = emit_code(gc, cb, add(gc, mem(gc, value(gc, (int)&last_describer->pixels), NULL, NULL, 1), ebx));
cb = emit_code(gc, cb, inc(gc, mem(gc, value(gc, (int)&last_describer->spans), NULL, NULL, 1)));
#endif /* __GL_CODEGEN_DESCRIBERS */

  /* For spanlets, the only thing that we rely on is modeFlags.
   * So for example the depth-test clause just blindly does
   * its job: there's no attempt to recognize the 
   * (GL_ALWAYS && no-writes) case because this is controlled
   * by things other than modeFlags.
   */
  if (spanlet) {
    struct celem
      *f = label(gc),
      *s = label(gc);

cb = emit_code(gc, cb, mov(gc, w, ebx));

cb = memov(gc, cb, tr(cp), tr(cp0));
    if (gc->buffers.doubleStore)
cb = memov(gc, cb, tr(cp2), tr(cp20));
    
cb = memov(gc, cb, tr(x), tr(x0Int));
    if (modeFlags & __GL_SHADE_STENCIL_TEST)
cb = memov(gc, cb, tr(sp), tr(sp0));

    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
cb = memov(gc, cb, tr(zp), tr(zp0));
cb = memov(gc, cb, tr(z), tr(z0));
    }

    if (modeFlags & __GL_SHADE_SMOOTH) {
cb = memov(gc, cb, tr(r), tr(r0));
      if (modeFlags & __GL_SHADE_RGB) {
cb = memov(gc, cb, tr(g), tr(g0));
cb = memov(gc, cb, tr(b), tr(b0));

        if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_BLEND))
cb = memov(gc, cb, tr(a), tr(a0));
      }
    }

    if (modeFlags & __GL_SHADE_SLOW_FOG) {
cb = memov(gc, cb, tr(f), tr(f0));
    }

    if (modeFlags & __GL_SHADE_TEXTURE) {
      if (!(modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = memov(gc, cb, tr(s), tr(s0));
cb = memov(gc, cb, tr(t), tr(t0));
      } else {
cb = memov(gc, cb, tr(fsw), tr(fsw0));
cb = memov(gc, cb, tr(ftw), tr(ftw0));
cb = memov(gc, cb, tr(fqw), tr(fqw0));
        if (texNeedRho) {
cb = memov(gc, cb, tr(frhow), tr(frhow0));
	}
      }
    }

f->spec.label.where = cb; resolve(gc, f, cb);
cb = emit_code(gc, cb, mov(gc, ecx, value(gc, ~0)));
cb = emit_code(gc, cb, cmp(gc, ebx, value(gc, 32)));
cb = emit_code(gc, cb, jge(gc, s));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, (int)__glFRMaskTable), NULL, ebx, 4)));
s->spec.label.where = cb; resolve(gc, s, cb);
cb = emit_code(gc, cb, mov(gc, w, ebx));
cb = emit_code(gc, cb, lea(gc, edx, tr));
cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)(&gc->procs.renderSpan)), NULL, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, ebx, w));
cb = emit_code(gc, cb, sub(gc, ebx, value(gc, 32)));
cb = emit_code(gc, cb, jle(gc, nextline));

cb = emit_code(gc, cb, mov(gc, eax, tr(x)));
cb = emit_code(gc, cb, add(gc, eax, value(gc, 32)));
cb = emit_code(gc, cb, mov(gc, tr(x), eax));
cb = ds(gc, cb, tr(cp), tr(dcpdxSpan));
    if (gc->buffers.doubleStore)
cb = ds(gc, cb, tr(cp2), tr(dcp2dxSpan));

    if (modeFlags & __GL_SHADE_STENCIL_TEST)
cb = ds(gc, cb, tr(sp), tr(dspdxSpan));

    if (modeFlags & __GL_SHADE_DITHER)
cb = set_dither(gc, cb, tr, tr(x), spanlet);

    if (modeFlags & __GL_SHADE_DEPTH_TEST &&
	(gc->state.depth.writeEnable ||
	 gc->state.depth.testFunc != GL_ALWAYS)) {
cb = ds(gc, cb, tr(zp), tr(dzpdxSpan));
cb = ds(gc, cb, tr(z), tr(dzdxSpan));
    }

    if (modeFlags & __GL_SHADE_SMOOTH) {
cb = ds(gc, cb, tr(r), tr(drdxSpan));
cb = ds(gc, cb, tr(g), tr(dgdxSpan));
cb = ds(gc, cb, tr(b), tr(dbdxSpan));
      if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_BLEND))
cb = ds(gc, cb, tr(a), tr(dadxSpan));
    }

    if (modeFlags & __GL_SHADE_SLOW_FOG) {
cb = ds(gc, cb, tr(f), tr(dfdxSpan));
    }

    if (modeFlags & __GL_SHADE_TEXTURE) {
      if (!(modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = ds(gc, cb, tr(s), tr(dsdxSpan));
cb = ds(gc, cb, tr(t), tr(dtdxSpan));
      } else {
cb = emit_code(gc, cb, fld(gc, tr(fsw)));
cb = emit_code(gc, cb, fadd(gc, tr(fdswdxSpan)));
cb = emit_code(gc, cb, fstp(gc, tr(fsw)));

cb = emit_code(gc, cb, fld(gc, tr(ftw)));
cb = emit_code(gc, cb, fadd(gc, tr(fdtwdxSpan)));
cb = emit_code(gc, cb, fstp(gc, tr(ftw)));

cb = emit_code(gc, cb, fld(gc, tr(fqw)));
cb = emit_code(gc, cb, fadd(gc, tr(fdqwdxSpan)));
cb = emit_code(gc, cb, fstp(gc, tr(fqw)));

        if (texNeedRho) {
cb = emit_code(gc, cb, fld(gc, tr(frhow)));
cb = emit_code(gc, cb, fadd(gc, tr(fdrhowdxSpan)));
cb = emit_code(gc, cb, fstp(gc, tr(frhow)));
	}
      }
    }
  if (modeFlags & __GL_SHADE_DITHER)
cb = set_dither(gc, cb, tr, tr(x), spanlet);

cb = emit_code(gc, cb, jmp(gc, f));
  } else {
      if (useFlatspan) {
cb = flatspan(gc, cb, tr, dithervalue, w, fbsize);
      } else if (useSmoothspan) {
cb = smoothspan(gc, cb, tr, drdxH, dgdxH, dbdxH, drdxP, dgdxP, dbdxP, w, fbsize);
      } else {
cb = span(gc, cb, modeFlags, tr, truew, w, dither_index, &fptr, rptr, gptr, bptr, interpolate_color, interpolate_z, fbsize, texenv);
      }
  }

nextline->spec.label.where = cb; resolve(gc, nextline, cb);

cb = emit_code(gc, cb, mov(gc, eax, tr(x0Frac)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(dxdy0Frac)));
cb = emit_code(gc, cb, add(gc, eax, ebx));
cb = emit_code(gc, cb, mov(gc, ebx, eax));
cb = emit_code(gc, cb, and(gc, eax, value(gc, ~0x80000000UL)));
cb = emit_code(gc, cb, shr(gc, ebx, value(gc, 31)));
    	
cb = emit_code(gc, cb, mov(gc, ebp, trs(dxdy0Int)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(x0Int)));
cb = emit_code(gc, cb, add(gc, ecx, ebp));
    
cb = dyv(gc, cb, tr(cp0), trs(dcpdy0), edx, ebp);

cb = emit_code(gc, cb, mov(gc, tr(x0Int), ecx));
cb = emit_code(gc, cb, mov(gc, tr(x0Frac), eax));
cb = emit_code(gc, cb, mov(gc, tr(cp0), edx));

    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
cb = dy(gc, cb, tr(sp0), trs(dspdy0));
    }

    if (gc->buffers.doubleStore)
cb = dy(gc, cb, tr(cp20), trs(dcp2dy0));

    if (modeFlags & __GL_SHADE_SMOOTH) {
        if (spanlet || !__glCanMmx(gc)) {
cb = dy(gc, cb, tr(r0), trs(drdy0));
    	    if (modeFlags & __GL_SHADE_RGB) {
cb = dy(gc, cb, tr(g0), trs(dgdy0));
cb = dy(gc, cb, tr(b0), trs(dbdy0));
      	  
    		if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_BLEND))
cb = dy(gc, cb, tr(a0), trs(dady0));
    	    }
        } else {
            /* We must be the scalar fallback for an MMX renderer. */
cb = dy(gc, cb, tr(r0), trsq(drdy0));
cb = dy(gc, cb, tr(g0), trsq(dgdy0));
cb = dy(gc, cb, tr(b0), trsq(dbdy0));
        }
    }

  if (modeFlags & __GL_SHADE_SLOW_FOG) {
cb = dy(gc, cb, tr(f0), trs(dfdy0));
  }

  if (modeFlags & __GL_SHADE_DEPTH_TEST &&
      (gc->state.depth.writeEnable ||
       gc->state.depth.testFunc != GL_ALWAYS)) {
        if (spanlet || !__glCanMmx(gc)) {
cb = dy(gc, cb, tr(z0), trs(dzdy0));
cb = dy(gc, cb, tr(zp0), trs(dzpdy0));
        } else {
cb = dy(gc, cb, tr(z0), trsq(dzdy0));
cb = dy(gc, cb, tr(zp0), trsq(dzpdy0));
        }
  }

  if (modeFlags & __GL_SHADE_TEXTURE) {
    if (!(modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = dy(gc, cb, tr(s0), trs(dsdy0));
cb = dy(gc, cb, tr(t0), trs(dtdy0));
    } else {
cb = emit_code(gc, cb, fld(gc, tr(fsw0)));
cb = emit_code(gc, cb, fadd(gc, trs(fdswdy0)));

cb = emit_code(gc, cb, fld(gc, tr(ftw0)));
cb = emit_code(gc, cb, fadd(gc, trs(fdtwdy0)));

cb = emit_code(gc, cb, fld(gc, tr(fqw0)));
cb = emit_code(gc, cb, fadd(gc, trs(fdqwdy0)));
cb = emit_code(gc, cb, fxch(gc, st2));

cb = emit_code(gc, cb, fstp(gc, tr(fsw0)));
cb = emit_code(gc, cb, fstp(gc, tr(ftw0)));
cb = emit_code(gc, cb, fstp(gc, tr(fqw0)));

      if (texNeedRho) {
cb = emit_code(gc, cb, fld(gc, tr(frhow0)));
cb = emit_code(gc, cb, fadd(gc, trs(fdrhowdy0)));
cb = emit_code(gc, cb, fstp(gc, tr(frhow0)));
      }
    }
  }

cb = emit_code(gc, cb, mov(gc, eax, tr(x1Frac)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(dxdy1Frac)));
cb = emit_code(gc, cb, add(gc, eax, ebx));
cb = emit_code(gc, cb, mov(gc, ebx, eax));
cb = emit_code(gc, cb, and(gc, eax, value(gc, ~0x80000000UL)));
cb = emit_code(gc, cb, shr(gc, ebx, value(gc, 31)));
cb = emit_code(gc, cb, mov(gc, tr(x1Frac), eax));

cb = emit_code(gc, cb, mov(gc, eax, trs(dxdy1Int)));
cb = emit_code(gc, cb, mov(gc, ecx, tr(x1Int)));
cb = emit_code(gc, cb, add(gc, ecx, eax));
cb = emit_code(gc, cb, mov(gc, tr(x1Int), ecx));

  if ((modeFlags & __GL_SHADE_DITHER) || spanlet)
cb = emit_code(gc, cb, inc(gc, tr(y)));

cb = emit_code(gc, cb, mov(gc, eax, h));
cb = emit_code(gc, cb, dec(gc, eax));
cb = emit_code(gc, cb, jne(gc, canal));

hzero->spec.label.where = cb; resolve(gc, hzero, cb);

cb = emit_code(gc, cb, mov(gc, eax, tr(y1Int)));
cb = emit_code(gc, cb, mov(gc, ebx, tr(y2Int)));
cb = emit_code(gc, cb, cmp(gc, eax, ebx));
cb = emit_code(gc, cb, je(gc, alldone));
cb = emit_code(gc, cb, mov(gc, tr(y0Int), ebx));
cb = emit_code(gc, cb, mov(gc, tr(y2Int), eax));

cb = memov(gc, cb, tr(x1Int), tr(x2Int));
cb = memov(gc, cb, tr(dxdy1Int[0]), tr(dxdy2Int[0]));
cb = memov(gc, cb, tr(dxdy1Int[1]), tr(dxdy2Int[1]));
cb = memov(gc, cb, tr(x1Frac), tr(x2Frac));
cb = memov(gc, cb, tr(dxdy1Frac), tr(dxdy2Frac));

cb = emit_code(gc, cb, jmp(gc, trap));

alldone->spec.label.where = cb; resolve(gc, alldone, cb);

  /*if (gc->procs.unlockRenderBuffers != NULL)*/ {
      struct celem* noneed = label(gc);

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, (int)&(gc)->buffers.lock.unlockRenderBuffers), NULL, NULL, 1)));
cb = emit_code(gc, cb, test(gc, edx, edx));
cb = emit_code(gc, cb, je(gc, noneed));

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, (int)&(gc)->buffers.lock.lockCnt), NULL, NULL, 1)));
cb = emit_code(gc, cb, dec(gc, edx));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&(gc)->buffers.lock.lockCnt), NULL, NULL, 1), edx));
cb = emit_code(gc, cb, cmp(gc, edx, value(gc, 0)));
cb = emit_code(gc, cb, jne(gc, noneed));
cb = emit_code(gc, cb, mov(gc, edx, value(gc, (int)gc)));
cb = emit_code(gc, cb, push(gc, edx));
cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)&(gc)->buffers.lock.unlockRenderBuffers), NULL, NULL, 1)));
cb = emit_code(gc, cb, add(gc, esp, value(gc, 4)));
noneed->spec.label.where = cb; resolve(gc, noneed, cb);
  }

cb = emit_code(gc, cb, add(gc, esp, value(gc, locals)));

#if __GL_CODEGEN_DESCRIBERS
cb = emit_code(gc, cb, rdtsc(gc));
cb = emit_code(gc, cb, add(gc, mem(gc, value(gc, (int)&last_describer->cyc[0]), NULL, NULL, 1), eax));
cb = emit_code(gc, cb, adc(gc, mem(gc, value(gc, (int)&last_describer->cyc[1]), NULL, NULL, 1), edx));
#endif /* __GL_CODEGEN_DESCRIBERS */

/* Really a return, this is initialised at the start of setup. */
cb = emit_code(gc, cb, mov(gc, eax, value(gc, 1)));
cb = emit_code(gc, cb, jmpi(gc, mem(gc, value(gc, (int)&gc->ogState.rsave), NULL, NULL, 1)));

  assert((unsigned)space + gc->constants.triRasterCacheSize > (unsigned)cb);

//  __glNSCloseSpace(gc->tr);

  return (rasterizer)space;
}

/************************************************************************/

STATIC unsigned char*
sort2(__GLcontext *gc,
      unsigned char* cb, 
      struct celem* r0,
      struct celem* r1)
{
	struct celem* ok = near_label(gc);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(window.y), r0, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, vx(window.y), r1, NULL, 1)));

cb = emit_code(gc, cb, cmp(gc, esi, ebp));
cb = emit_code(gc, cb, jge(gc, ok));

cb = emit_code(gc, cb, mov(gc, esi, r0));
cb = emit_code(gc, cb, mov(gc, r0, r1));

cb = emit_code(gc, cb, mov(gc, r1, esi));
cb = emit_code(gc, cb, xor(gc, edi, edx));

ok->spec.label.where = cb; resolve(gc, ok, cb);

	return cb;
}

STATIC unsigned char*
scaleby(__GLcontext *gc,
	unsigned char* cb,
	struct celem* dst,
	struct celem* src,
	int N)
{
    int shifts;
    
    if (N == 1) {
cb = emit_code(gc, cb, mov(gc, dst, src));
    } else {

       for (shifts = 1; (N & (1 << shifts)) == 0; shifts++)
	   ;

cb = scaleby(gc, cb, dst, src, (N >> shifts));
       if (shifts == 1)
cb = emit_code(gc, cb, add(gc, dst, dst));
       else
cb = emit_code(gc, cb, shl(gc, dst, value(gc, shifts)));
       if (N & 1)
cb = emit_code(gc, cb, add(gc, dst, src));
    }

    return cb;
}

STATIC unsigned char*
buf(__GLcontext* gc,
    unsigned char* cb,
    struct celem* p0,
    struct celem* dpdx,
    struct celem* dpdxSpan,
    struct celem* dpdy0,
    struct celem* dpdy1,
    void* base,
    void* bw,
    int es,
    struct celem* dxneg,
    struct celem* tr,
    GLboolean spanlet)
{

cb = emit_code(gc, cb, mov(gc, ebp, tr(x0Int)));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, (int)bw), NULL, NULL, 1)));

cb = scaleby(gc, cb, edi, ebp, es);

cb = emit_code(gc, cb, add(gc, edi, mem(gc, value(gc, (int)base), NULL, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, ebp, tr(y0Int)));
cb = emit_code(gc, cb, imul(gc, ebp, edx));

cb = emit_code(gc, cb, add(gc, edi, ebp));

cb = emit_code(gc, cb, mov(gc, p0, edi));

cb = emit_code(gc, cb, mov(gc, esi, tr(dx)));
cb = scaleby(gc, cb, ebp, esi, es);
cb = emit_code(gc, cb, mov(gc, dpdx, ebp));
    if (spanlet) {
cb = emit_code(gc, cb, shl(gc, ebp, value(gc, 5)));
cb = emit_code(gc, cb, mov(gc, dpdxSpan, ebp));
    }

cb = emit_code(gc, cb, mov(gc, esi, tr(dxdy0Int[0])));
cb = scaleby(gc, cb, edi, esi, es);
cb = emit_code(gc, cb, add(gc, edi, edx));
cb = emit_code(gc, cb, mov(gc, dpdy0, edi));

cb = emit_code(gc, cb, mov(gc, esi, tr(dxdy0Int[1])));
cb = scaleby(gc, cb, edi, esi, es);
cb = emit_code(gc, cb, add(gc, edi, edx));
cb = emit_code(gc, cb, mov(gc, dpdy1, edi));

    return cb;
}

STATIC void
fogsetup(__GLcontext *gc, GLint colorFace, GLint needs, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2)
{
    static __GLcolor tmpColor;
    static __GLcolor subColors[3];	/* Substitute colors for the vertices */
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLvertex *pv = gc->vertex.provoking;

    pv->color = &pv->colors[colorFace];
    if (~pv->hasAndClipCode & needs) DO_VALIDATE(gc, pv, needs);

    if (modeFlags & __GL_SHADE_CHEAP_FOG) {
	__GLcolor *pvColor = pv->color;

	pv->color = &tmpColor;
	v0->color = &subColors[0];
	v1->color = &subColors[1];
	v2->color = &subColors[2];

	(*gc->procs.fogColor)(gc, v0->color, pvColor, v0->fog);
	(*gc->procs.fogColor)(gc, v1->color, pvColor, v1->fog);
	(*gc->procs.fogColor)(gc, v2->color, pvColor, v2->fog);
    }

    if (pv != v0 && pv != v1 && pv != v2) {
	pv->color = &pv->colors[__GL_FRONTFACE];
    }
}

STATIC unsigned char*
validate(__GLcontext *gc, unsigned char* cb, struct celem* v)
{
    struct celem* ok = near_label(gc);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(hasAndClipCode), v, NULL, 1)));
cb = emit_code(gc, cb, xor(gc, esi, value(gc, ~0)));
cb = emit_code(gc, cb, test(gc, ebp, esi));
cb = emit_code(gc, cb, je(gc, ok));
    
cb = emit_code(gc, cb, mov(gc, esi, value(gc, ((int)gc))));

cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, push(gc, edx));

cb = emit_code(gc, cb, push(gc, ebp));
cb = emit_code(gc, cb, push(gc, v));
cb = emit_code(gc, cb, push(gc, esi));

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, vx(hasAndClipCode), v, NULL, 1)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 10)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 3)));
cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)&gc->procs.validateVertex0), NULL, eax, 4)));

cb = emit_code(gc, cb, add(gc, esp, value(gc, 8)));
cb = emit_code(gc, cb, pop(gc, ebp));

cb = emit_code(gc, cb, pop(gc, edx));
cb = emit_code(gc, cb, pop(gc, ecx));
cb = emit_code(gc, cb, pop(gc, ebx));
cb = emit_code(gc, cb, pop(gc, eax));

ok->spec.label.where = cb; resolve(gc, ok, cb);

    return cb;
}

/********************************************
**  Triangle Setup Cache
*********************************************/

/* mode bits for cache signature */
#define __OG_SETUP_SPANLETS		(1<<0)
#define __OG_SETUP_COOKED_TEXTURE	(1<<1)
#define __OG_SETUP_REPLACE		(1<<2)
#define __OG_SETUP_DO_DEPTH		(1<<3)
#define __OG_SETUP_DOUBLESTORE          (1<<4)
#define __OG_SETUP_NEEDRHO	        (1<<5)
#define __OG_SETUP_CRC_POS	        6

#define CHECK_STACK	0	/* Set to 1 to enable check
				 * for floating-point stack balance */

#if __GL_CODEGEN_DESCRIBERS

/* 
 * This produces an area breakdown in the file "areas.log".  See
 * below for description of column headings.
 */

STATIC void
dump_areas(void)
{
    FILE* f;
    struct describer* pd, totals;

    f = fopen("areas.log", "wt");
    fprintf(f, "\n");

    /* Count totals */
    totals.pixels = 0;
    totals.spans = 0;
    totals.tot_tris = 0;
    for (pd = last_describer; pd; pd = pd->prev) {
	totals.pixels += pd->pixels;
	totals.spans += pd->spans;
	totals.tot_tris += pd->tot_tris;

	pd->cycles = (double)pd->cyc[0] + (pd->cyc[1] * (65536.0 * 65536.0));
    }

    fprintf(f,
	    "%5s %6s %8s %8s %8s %8s %8s %s\n",
	    "area",	/* % of total area (rough importance measure) */
	    "uncull",	/* % of all tris passing cull (EXCLUDES VX CULL!) */
	    "pix/tri",	/* Avg. pixels per triangle */
	    "pix/spn",	/* Avg. pixels per span */
	    "spn/tri",	/* Avg. spans per triangle */
	    "cyc/pix",	/* Avg. cycles per pixel */
	    "cyc/spn",	/* Avg. cycles per span */
	    "description");

    for (pd = last_describer; pd; pd = pd->prev) {
	if ((pd->pixels > 0) &&
	    (pd->drawn_tris > 0) &&
	    (pd->spans > 0))
	    fprintf(f, "%5.1f %6d %8d %8d %8d %8.1f %8.1f %s\n",
		    (100.0 * pd->pixels) / totals.pixels,
		    /* (100 * pd->drawn_tris) / pd->tot_tris*/ 0,
		    pd->pixels / pd->drawn_tris,
		    pd->pixels / pd->spans,
		    pd->spans / pd->drawn_tris,
		    pd->cycles / pd->pixels,
		    pd->cycles / pd->spans,
		    pd->name);
    }

    fclose(f);
}

#endif /* __GL_CODEGEN_DESCRIBERS */

STATIC GLboolean
need_color(GLuint modeFlags, GLuint texenv)
{
  if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_BLEND))
    return GL_TRUE;

  if ((modeFlags & __GL_SHADE_TEXTURE) &&
      ((texenv == GL_REPLACE)))
    return GL_FALSE;
  else
    return GL_TRUE;
}

STATIC unsigned char*
adjustTriangleVerts(__GLcontext *gc,
	unsigned char* cb,
	struct celem* v0, 
	struct celem* v1, 
	struct celem* v2, 
	struct celem* saved)
{
	struct celem* skip_pv = label(gc);
	struct celem* skip_a = label(gc);
	struct celem* skip_b = label(gc);
	struct celem* skip_c = label(gc);

#if !defined(__GL_HALF_PIXEL_OFFSET) || \
     defined(__GL_DEVICE_COLOR_SCALES) || \
     defined(__GL_DEVICE_DEPTH_SCALE)
cb = emit_code(gc, cb, lea(gc, edx, saved));

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 0), edx, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 4), edx, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 8), edx, NULL, 1), ecx));
#endif

#if !defined(__GL_HALF_PIXEL_OFFSET)
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.x), eax, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+0*sizeof(__GLcoord)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fadd(gc, fpk(gc, 0.5F)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.y), eax, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+0*sizeof(__GLcoord)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fadd(gc, fpk(gc, 0.5F)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.x), ebx, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+1*sizeof(__GLcoord)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fadd(gc, fpk(gc, 0.5F)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.y), ebx, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+1*sizeof(__GLcoord)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fadd(gc, fpk(gc, 0.5F)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.x), ecx, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+2*sizeof(__GLcoord)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fadd(gc, fpk(gc, 0.5F)));
cb = emit_code(gc, cb, fxch(gc, st4));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+2*sizeof(__GLcoord)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fadd(gc, fpk(gc, 0.5F)));
cb = emit_code(gc, cb, fxch(gc, st4));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.y), eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.x), eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.y), ebx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.x), ebx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.x), ecx, NULL, 1)));
#endif

#if defined(__GL_DEVICE_DEPTH_SCALE)
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.z), eax, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+0*sizeof(__GLcoord)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.depthRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.z), ebx, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+1*sizeof(__GLcoord)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.depthRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.z), ecx, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+2*sizeof(__GLcoord)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.depthRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.z), eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.z), ebx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.z), ecx, NULL, 1)));
#endif

#if defined(__GL_DEVICE_COLOR_SCALE)
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, (int)&gc->vertex.provoking), NULL, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, edi, value(gc, 0)));
cb = emit_code(gc, cb, je(gc, skip_pv));

cb = emit_code(gc, cb, mov(gc, edi, mem(gc, vx(color), edi, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 0), edi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+3*sizeof(__GLcolor)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.redRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), edi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+3*sizeof(__GLcolor)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.greenRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 8), edi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+3*sizeof(__GLcolor)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.blueRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12), edi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+3*sizeof(__GLcolor)+12), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.alphaRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), edi, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), edi, NULL, 1)));
skip_pv->spec.label.where = cb; resolve(gc, skip_pv, cb);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(color), eax, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, esi, edi));
cb = emit_code(gc, cb, je(gc, skip_a));

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+0*sizeof(__GLcolor)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.redRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+0*sizeof(__GLcolor)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.greenRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+0*sizeof(__GLcolor)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.blueRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+0*sizeof(__GLcolor)+12), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.alphaRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), esi, NULL, 1)));
skip_a->spec.label.where = cb; resolve(gc, skip_a, cb);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(color), ebx, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, esi, edi));
cb = emit_code(gc, cb, je(gc, skip_b));

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+1*sizeof(__GLcolor)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.redRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+1*sizeof(__GLcolor)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.greenRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+1*sizeof(__GLcolor)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.blueRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+1*sizeof(__GLcolor)+12), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.alphaRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), esi, NULL, 1)));
skip_b->spec.label.where = cb; resolve(gc, skip_b, cb);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(color), ecx, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, esi, edi));
cb = emit_code(gc, cb, je(gc, skip_c));

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+2*sizeof(__GLcolor)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.redRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+2*sizeof(__GLcolor)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.greenRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+2*sizeof(__GLcolor)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.blueRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12), esi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+2*sizeof(__GLcolor)+12), edx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&gc->constants.alphaRescale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), esi, NULL, 1)));
skip_c->spec.label.where = cb; resolve(gc, skip_c, cb);
#endif
	return cb;
}

STATIC unsigned char*
restoreTriangleVerts(__GLcontext *gc,
	unsigned char* cb,
	struct celem* saved)
{
	struct celem* skip_pv = label(gc);
	struct celem* skip_a = label(gc);
	struct celem* skip_b = label(gc);
	struct celem* skip_c = label(gc);

#if !defined(__GL_HALF_PIXEL_OFFSET) || \
     defined(__GL_DEVICE_COLOR_SCALES) || \
     defined(__GL_DEVICE_DEPTH_SCALE)
cb = emit_code(gc, cb, lea(gc, edx, saved));

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, 0), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 4), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, 8), edx, NULL, 1)));
#endif

#if !defined(__GL_HALF_PIXEL_OFFSET)
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, 12+0*sizeof(__GLcoord)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, 12+0*sizeof(__GLcoord)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.x), eax, NULL, 1), edi));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.y), eax, NULL, 1), esi));
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, 12+1*sizeof(__GLcoord)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, 12+1*sizeof(__GLcoord)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.x), ebx, NULL, 1), edi));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.y), ebx, NULL, 1), esi));
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, 12+2*sizeof(__GLcoord)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, 12+2*sizeof(__GLcoord)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.x), ecx, NULL, 1), edi));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.y), ecx, NULL, 1), esi));
#endif

#if defined(__GL_DEVICE_DEPTH_SCALE)
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, 12+0*sizeof(__GLcoord)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, 12+1*sizeof(__GLcoord)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.z), eax, NULL, 1), edi));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.z), ebx, NULL, 1), esi));
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, 12+2*sizeof(__GLcoord)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.z), ecx, NULL, 1), edi));
#endif

#if defined(__GL_DEVICE_COLOR_SCALE)
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, (int)&gc->vertex.provoking), NULL, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, edi, value(gc, 0)));
cb = emit_code(gc, cb, je(gc, skip_pv));

cb = emit_code(gc, cb, mov(gc, edi, mem(gc, vx(color), edi, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+3*sizeof(__GLcolor)+12), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+3*sizeof(__GLcolor)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+3*sizeof(__GLcolor)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+3*sizeof(__GLcolor)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), edi, NULL, 1)));
skip_pv->spec.label.where = cb; resolve(gc, skip_pv, cb);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(color), eax, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, esi, edi));
cb = emit_code(gc, cb, je(gc, skip_a));

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+0*sizeof(__GLcolor)+12), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+0*sizeof(__GLcolor)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+0*sizeof(__GLcolor)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+0*sizeof(__GLcolor)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), esi, NULL, 1)));
skip_a->spec.label.where = cb; resolve(gc, skip_a, cb);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(color), ebx, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, esi, edi));
cb = emit_code(gc, cb, je(gc, skip_b));

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+1*sizeof(__GLcolor)+12), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+1*sizeof(__GLcolor)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+1*sizeof(__GLcolor)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+1*sizeof(__GLcolor)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), esi, NULL, 1)));
skip_b->spec.label.where = cb; resolve(gc, skip_b, cb);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(color), ecx, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, esi, edi));
cb = emit_code(gc, cb, je(gc, skip_c));

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+2*sizeof(__GLcolor)+12), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+2*sizeof(__GLcolor)+8), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+2*sizeof(__GLcolor)+4), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12+3*sizeof(__GLcoord)+2*sizeof(__GLcolor)+0), edx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), esi, NULL, 1)));
skip_c->spec.label.where = cb; resolve(gc, skip_c, cb);
#endif
	return cb;
}

setterupper
GenerateSetup(__GLcontext *gc, GLboolean spanlet)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLboolean cookedTexture;
    GLenum texenv;
    unsigned char *data;
    unsigned char *code;
    unsigned char *cb;
    int offset;
    struct celem
	*fxinterp,
	*fpinterp,
	*cull,
	*cull3,
	*simpleRenderTriangle;
    struct celem
	*v0, *v1, *v2, *dx0, *dy0, *dx1, *dy1, *dx2, *dy2, *xsnap0, 
	*ysnap0, *ysnap1, *ysnap2, *dxdyInt0, *negdx, *negdxdy, *fnegdx, *fnegdxdy;
    struct celem
	*color_posns;
    GLuint *color_posnarray;
    struct celem
	*dxdy0, *dxdy1, *dxdy2;
    struct celem
	*i_gc, *i_v0, *i_v1, *i_v2;
    struct celem
	*q0, *qdx, *qdy;
    struct celem* tr;
    struct celem* fpcw;
    static const double alpha_bias[3] = {
	DoubleBias(COLOR_FRAC_BITS),
	DoubleBias(COLOR_FRAC_BITS),
	DoubleBias(COLOR_FRAC_BITS)
    };
    static const double color_no_dither_bias[3] = {
	COLOR_ROUND_NO_DITHER + DoubleBias(COLOR_FRAC_BITS),
	DoubleBias(COLOR_FRAC_BITS),
	DoubleBias(COLOR_FRAC_BITS)
    };
    static const double color_dither_bias[3] = {
	COLOR_ROUND_DITHER + DoubleBias(COLOR_FRAC_BITS),
	DoubleBias(COLOR_FRAC_BITS),
	DoubleBias(COLOR_FRAC_BITS)
    };
    static const double z_bias[3] = {
	0.5 + DoubleBias(Z_FRAC_BITS),
	DoubleBias(Z_FRAC_BITS),
	DoubleBias(Z_FRAC_BITS)
    };
    static const double affine_texture_bias[3] = {
	DoubleBias(STRQ_FRAC_BITS),
	DoubleBias(STRQ_FRAC_BITS),
	DoubleBias(STRQ_FRAC_BITS)	
    };
    static const double mmx_q_bias[3] = {
	DoubleBias(24),
	DoubleBias(24),
	DoubleBias(24)	
    };
    int interpolants = 1;
    unsigned int signature;
    static const float alpha_correction_factor = 256.0f / 255.0f;
    struct celem* alpha_correcter;
    GLboolean texNeedRho;
    struct celem* saved;

    /* Set up effective texenv: DECAL w/o alpha is the same as REPLACE */
    if (modeFlags & __GL_SHADE_TEXTURE) {
	texenv = gc->state.texture.env[0].mode;
	if (texenv == GL_DECAL &&
	    gc->texture.currentTexture->texelFormat == GL_RGB)
	    texenv = GL_REPLACE;
    }

    /* Set up cookedTexture flag: we use special texture format for
     * undithered replace in rgbMode only.
     */
    if (!spanlet &&
	(modeFlags & (__GL_SHADE_BLEND|__GL_SHADE_TEXTURE|__GL_SHADE_RGB|__GL_SHADE_DITHER)) ==
	(__GL_SHADE_TEXTURE|__GL_SHADE_RGB)) {
	cookedTexture = (texenv == GL_REPLACE) &&
	    gc->texture.currentTexture->level[0]->pixelBuffer;
    } else {
	cookedTexture = GL_FALSE;
    }

    if (modeFlags & __GL_SHADE_TEXTURE) {
	__GLtexture *current = gc->texture.currentTexture;
	texNeedRho = current->params.minFilter != current->params.magFilter;
    } else {
	texNeedRho = GL_FALSE;
    }

    /* Create a unique cache signature that covers information not
     * included in modeFlags.
     */
    signature = 0;

    if (spanlet)
	signature |= __OG_SETUP_SPANLETS;

    if (cookedTexture)
	signature |= __OG_SETUP_COOKED_TEXTURE;

    if ((modeFlags & __GL_SHADE_TEXTURE) && (texenv == GL_REPLACE))
        signature |= __OG_SETUP_REPLACE;

    /* Need to know this specifically because of the optimisation
     * for it in the rasterizers.
     */
    if ((modeFlags & __GL_SHADE_DEPTH_TEST) &&
	(gc->state.depth.writeEnable ||
	 gc->state.depth.testFunc != GL_ALWAYS))
	signature |= __OG_SETUP_DO_DEPTH;

    if (gc->buffers.doubleStore) {
	signature |= __OG_SETUP_DOUBLESTORE;
    }

    if (texNeedRho) {
	signature |= __OG_SETUP_NEEDRHO;
    }

    /* And something to make it _very_ unlikely that we will be
     * generating setup code for a rasterizer that has a different
     * idea of the layout of 'tr'.  Chance of error is much
     * less than 1 in 2^28.
     */
    signature ^= ((int)(gc->tr->crc) << __OG_SETUP_CRC_POS);

    data = (unsigned char*)(((int)gc->triSetupData + 7) & ~7);

    if (__glAllocCodeSpace(&gc->triSetupCache, modeFlags, signature, NULL, &code)) {
      return (setterupper) code;
    }

    cb = code;

    og_warmup(gc);
#if SOURCE_TRAIL
    og_stream(gc, "/tmp/setup.inc");
#endif
    
    alpha_correcter = mem2(gc, value(gc, (int)&alpha_correction_factor), 0);

    fxinterp = label(gc);
    fpinterp = label(gc);
    cull = label(gc);
    cull3 = label(gc);
    simpleRenderTriangle = label(gc);

cb = emit_code(gc, cb, push(gc, esi));
cb = emit_code(gc, cb, push(gc, edi));
cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, push(gc, ebp));

#if CHECK_STACK
cb = emit_code(gc, cb, fld(gc, fpk(gc, 947.947f)));
#endif

#undef ILOCAL
#undef DLOCAL
#undef XLOCAL

    offset = 0;
#define ILOCAL(name)	(name = mem2(gc, value(gc, (int)&data[offset]), NULL), offset += 4)
#define DLOCAL(name)	(name = mem2(gc, value(gc, (int)&data[offset]), NULL), offset += 8)
#define XLOCAL(name, N)	(name = mem2(gc, value(gc, (int)&data[offset]), NULL), offset += N)
    DLOCAL(q0);
    DLOCAL(qdx);
    DLOCAL(qdy);
    ILOCAL(v0);
    ILOCAL(v1);
    ILOCAL(v2);
    ILOCAL(dx0);
    ILOCAL(dy0);
    ILOCAL(dx1);
    ILOCAL(dy1);
    ILOCAL(dx2);
    ILOCAL(dy2);
    ILOCAL(dxdy0);
    ILOCAL(dxdy1);
    ILOCAL(dxdy2);
    ILOCAL(xsnap0);
    ILOCAL(ysnap0);
    ILOCAL(ysnap1);
    ILOCAL(ysnap2);
    ILOCAL(dxdyInt0);
    ILOCAL(negdx);
    ILOCAL(negdxdy);
    ILOCAL(fpcw);
    if ((modeFlags & __GL_SHADE_TEXTURE) && (modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
      ILOCAL(fnegdx);
      ILOCAL(fnegdxdy);
    }

    XLOCAL(saved, (3*sizeof(void*) + 3*(sizeof(__GLcoord) + 4*sizeof(__GLcolor))));

    color_posnarray = (GLint*)(code + 0xf00);
    color_posns = value(gc, (int)color_posnarray);

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&gc->ogState.spsave), NULL, NULL, 1), esp));
cb = emit_code(gc, cb, sub(gc, esp, value(gc, sizeof(__GLtri))));
cb = emit_code(gc, cb, mov(gc, ebp, esp));
cb = emit_code(gc, cb, and(gc, esp, value(gc, ~7)));

    tr = mem2(gc, value(gc, 0), esp);

    offset = 6 * sizeof(long) + sizeof(__GLtri);
#define PARM(name)	(name = mem2(gc, value(gc, offset), ebp), offset += sizeof(long))
    PARM(i_gc);
    PARM(i_v0);
    PARM(i_v1);
    PARM(i_v2);

/*
 *	eax	ebx	ecx	edx	ebp	esi	edi
 *	v0	v1	v2				reversed
 */

cb = emit_code(gc, cb, mov(gc, eax, i_v0));
cb = emit_code(gc, cb, mov(gc, ebx, i_v1));
cb = emit_code(gc, cb, mov(gc, ecx, i_v2));

cb = emit_code(gc, cb, call(gc, simpleRenderTriangle));

cb = emit_code(gc, cb, mov(gc, esp, mem(gc, value(gc, (int)&gc->ogState.spsave), NULL, NULL, 1)));

cb = emit_code(gc, cb, pop(gc, ebp));
cb = emit_code(gc, cb, pop(gc, ecx));
cb = emit_code(gc, cb, pop(gc, ebx));
cb = emit_code(gc, cb, pop(gc, edi));
cb = emit_code(gc, cb, pop(gc, esi));

cb = emit_code(gc, cb, ret(gc));

        gc->procs.simpleRenderTriangle = (void*) cb;
simpleRenderTriangle->spec.label.where = cb; resolve(gc, simpleRenderTriangle, cb);

#if __GL_CODEGEN_DESCRIBERS
cb = emit_code(gc, cb, inc(gc, mem(gc, value(gc, (int)&last_describer->tot_tris), NULL, NULL, 1)));
#endif /* __GL_CODEGEN_DESCRIBERS */

cb = emit_code(gc, cb, pop(gc, edx));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&gc->ogState.rsave), NULL, NULL, 1), edx));

cb = adjustTriangleVerts(gc, cb, eax, ebx, ecx, saved);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(window.x), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, vx(window.y), eax, NULL, 1)));
cb = emit_code(gc, cb, and(gc, esi, value(gc, XY_QUANTIZE_MASK)));
cb = emit_code(gc, cb, and(gc, ebp, value(gc, XY_QUANTIZE_MASK)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.x), eax, NULL, 1), esi));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.y), eax, NULL, 1), ebp));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(window.x), ebx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, vx(window.y), ebx, NULL, 1)));
cb = emit_code(gc, cb, and(gc, esi, value(gc, XY_QUANTIZE_MASK)));
cb = emit_code(gc, cb, and(gc, ebp, value(gc, XY_QUANTIZE_MASK)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.x), ebx, NULL, 1), esi));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.y), ebx, NULL, 1), ebp));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(window.x), ecx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, vx(window.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, and(gc, esi, value(gc, XY_QUANTIZE_MASK)));
cb = emit_code(gc, cb, and(gc, ebp, value(gc, XY_QUANTIZE_MASK)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.x), ecx, NULL, 1), esi));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.y), ecx, NULL, 1), ebp));

    {
cb = emit_code(gc, cb, xor(gc, edi, edi));
cb = emit_code(gc, cb, mov(gc, edx, value(gc, 1)));
cb = sort2(gc, cb, ebx, eax);
cb = sort2(gc, cb, ecx, eax);
cb = sort2(gc, cb, ebx, ecx);
    }

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.x), ebx, NULL, 1)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, vx(window.x), eax, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.y), ebx, NULL, 1)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, vx(window.y), eax, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.x), ecx, NULL, 1)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, vx(window.x), eax, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, vx(window.y), eax, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, edx, value(gc, DISPOF(q0))));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.x), ebx, NULL, 1)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, vx(window.x), ecx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(window.y), ebx, NULL, 1)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, vx(window.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st5));

#define QBASE(r,V) mem2(gc, value(gc, DISPOF(V) - DISPOF(q0)), r)

cb = emit_code(gc, cb, fld(gc, st0));
cb = emit_code(gc, cb, fmul(gc, st3));
cb = emit_code(gc, cb, fxch(gc, st6));
cb = emit_code(gc, cb, fld(gc, st4));
cb = emit_code(gc, cb, fmul(gc, st6));
cb = emit_code(gc, cb, fxch(gc, st2));

cb = emit_code(gc, cb, fstp(gc, QBASE(edx,dx0)));
cb = emit_code(gc, cb, fstp(gc, QBASE(edx,dy2)));
cb = emit_code(gc, cb, fsubrp(gc, st5, st0));

cb = emit_code(gc, cb, fstp(gc, QBASE(edx,dx2)));
cb = emit_code(gc, cb, fstp(gc, QBASE(edx,dy1)));
cb = emit_code(gc, cb, fstp(gc, QBASE(edx,dx1)));
cb = emit_code(gc, cb, fstp(gc, QBASE(edx,dy0)));
cb = emit_code(gc, cb, fstp(gc, tr(c)));

cb = emit_code(gc, cb, mov(gc, QBASE(edx,v0), eax));
cb = emit_code(gc, cb, mov(gc, QBASE(edx,v1), ebx));
cb = emit_code(gc, cb, mov(gc, QBASE(edx,v2), ecx));

cb = emit_code(gc, cb, mov(gc, edx, tr(c)));

cb = emit_code(gc, cb, test(gc, edx, value(gc, 0x7fffffff)));
cb = emit_code(gc, cb, je(gc, cull));

cb = emit_code(gc, cb, shr(gc, edx, value(gc, 31)));
cb = emit_code(gc, cb, mov(gc, ebp, value(gc, 1)));

cb = emit_code(gc, cb, mov(gc, esi, edx));

cb = emit_code(gc, cb, add(gc, esi, esi));

cb = emit_code(gc, cb, sub(gc, ebp, esi));

cb = emit_code(gc, cb, mov(gc, tr(dx), ebp));

#if 0
// Now do validation...

cb = emit_code(gc, cb, xor(gc, edi, edx));
cb = emit_code(gc, cb, xor(gc, edx, edx));

cb = emit_code(gc, cb, mov(gc, dl, mem(gc, value(gc, ((int)(gc->polygon.face))), edi, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, dl, mem(gc, value(gc, ((int)&gc->polygon.cullFace)), NULL, NULL, 1)));
cb = emit_code(gc, cb, je(gc, cull));

    if (modeFlags & __GL_SHADE_TWOSIDED) {
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, ((int)gc->vertex.faceNeeds)), NULL, edx, 4)));
    } else {
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, ((int)&gc->vertex.faceNeeds[__GL_FRONTFACE])), NULL, NULL, 1)));
    }

// if twosided, edx is colorface, ebp is faceneeds

cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, ((int)&gc->vertex.needs)), NULL, NULL, 1)));
cb = emit_code(gc, cb, or(gc, ebp, edi));

    /* need edx later for cheap fog */
    if (!(modeFlags & __GL_SHADE_SMOOTH_LIGHT))
cb = emit_code(gc, cb, mov(gc, q0, edx));

// edx is colorface, ebp is needs

    if (modeFlags & __GL_SHADE_SMOOTH) {
        struct celem* already = near_label(gc);

        if (modeFlags & __GL_SHADE_TWOSIDED)
cb = emit_code(gc, cb, shl(gc, edx, value(gc, 4)));
        else {

	/* Shortcut: test for no validation required at all */
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, vx(hasAndClipCode), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, vx(hasAndClipCode), ebx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, vx(hasAndClipCode), ecx, NULL, 1)));
cb = emit_code(gc, cb, and(gc, eax, ebx));
cb = emit_code(gc, cb, and(gc, eax, ecx));
cb = emit_code(gc, cb, xor(gc, eax, value(gc, ~0)));
cb = emit_code(gc, cb, and(gc, eax, ebp));
cb = emit_code(gc, cb, je(gc, already));
	}

cb = emit_code(gc, cb, xor(gc, eax, eax));
        {
	    struct celem* head = label(gc);
	    struct celem* ok = near_label(gc);
head->spec.label.where = cb; resolve(gc, head, cb);
cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, v0->spec.mem.disp->spec.value), NULL, eax, 4)));

	    if (modeFlags & __GL_SHADE_TWOSIDED) {
cb = emit_code(gc, cb, lea(gc, edi, mem(gc, vx(colors), eax, edx, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(color), eax, NULL, 1), edi));
	    }
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(hasAndClipCode), eax, NULL, 1)));
cb = emit_code(gc, cb, xor(gc, esi, value(gc, ~0)));

cb = emit_code(gc, cb, test(gc, ebp, esi));
cb = emit_code(gc, cb, je(gc, ok));

cb = emit_code(gc, cb, mov(gc, esi, value(gc, ((int)gc))));

	    if (modeFlags & __GL_SHADE_TWOSIDED)
cb = emit_code(gc, cb, push(gc, edx));

cb = emit_code(gc, cb, push(gc, ebp));
cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, push(gc, esi));

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, vx(hasAndClipCode), eax, NULL, 1)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 10)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 3)));
cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)&gc->procs.validateVertex0), NULL, eax, 4)));

cb = addesp(gc, cb, value(gc, 8));
cb = emit_code(gc, cb, pop(gc, ebp));

	    if (modeFlags & __GL_SHADE_TWOSIDED)
cb = emit_code(gc, cb, pop(gc, edx));

ok->spec.label.where = cb; resolve(gc, ok, cb);

cb = emit_code(gc, cb, pop(gc, eax));

cb = emit_code(gc, cb, inc(gc, eax));

cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 3)));
cb = emit_code(gc, cb, jne(gc, head));
	}

already->spec.label.where = cb; resolve(gc, already, cb);

	if (!(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
		struct celem*  f_fogsetup = label(gc);

		f_fogsetup->spec.label.where = (unsigned char*)&fogsetup;

cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));
cb = emit_code(gc, cb, mov(gc, edx, q0));

cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, eax));

cb = emit_code(gc, cb, push(gc, ebp));
cb = emit_code(gc, cb, push(gc, edx));
cb = emit_code(gc, cb, push(gc, value(gc, (int)gc)));

cb = emit_code(gc, cb, call(gc, f_fogsetup));

cb = addesp(gc, cb, value(gc, 12));
cb = emit_code(gc, cb, pop(gc, eax));
cb = emit_code(gc, cb, pop(gc, ebx));
cb = emit_code(gc, cb, pop(gc, ecx));
	}
    } else {
cb = emit_code(gc, cb, shl(gc, edx, value(gc, 4)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, ((int)&gc->vertex.provoking)), NULL, NULL, 1)));

	if (modeFlags & (__GL_SHADE_TWOSIDED | __GL_SHADE_CHEAP_FOG)) {
cb = emit_code(gc, cb, lea(gc, edi, mem(gc, vx(colors), esi, edx, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(color), esi, NULL, 1), edi));
	}

cb = emit_code(gc, cb, mov(gc, edx, esi));

cb = validate(gc, cb, edx);
cb = validate(gc, cb, eax);
cb = validate(gc, cb, ebx);
cb = validate(gc, cb, ecx);
    }
#endif

#if __GL_CODEGEN_DESCRIBERS
  {
    static int cold = 1;

    if (cold) {
	    atexit(dump_areas);
	    cold = 0;
    }
  }
#endif /* __GL_CODEGEN_DESCRIBERS */

cb = emit_code(gc, cb, mov(gc, esi, value(gc, DISPOF(q0))));

cb = emit_code(gc, cb, fstcw(gc, QBASE(esi,fpcw)));
cb = emit_code(gc, cb, mov(gc, edx, QBASE(esi,fpcw)));
cb = emit_code(gc, cb, or(gc, dh, value(gc, 3 << 2)));
cb = emit_code(gc, cb, and(gc, dh, value(gc, ~3)));
cb = emit_code(gc, cb, mov(gc, QBASE(esi,q0), edx));
cb = emit_code(gc, cb, fldcw(gc, QBASE(esi,q0)));

cb = emit_code(gc, cb, xor(gc, eax, eax));
    {
      struct celem* head = label(gc);
      struct celem* zero = near_label(gc);
      struct celem* done = near_label(gc);

head->spec.label.where = cb; resolve(gc, head, cb);
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, DISPOF(QBASE(esi,dy0))), esi, eax, 8)));

cb = emit_code(gc, cb, test(gc, edx, edx));
cb = emit_code(gc, cb, je(gc, zero));

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, DISPOF(QBASE(esi,dx0))), esi, eax, 8)));
cb = emit_code(gc, cb, fdiv(gc, mem(gc, value(gc, DISPOF(QBASE(esi,dy0))), esi, eax, 8)));
cb = emit_code(gc, cb, jmp(gc, done));

zero->spec.label.where = cb; resolve(gc, zero, cb);
cb = emit_code(gc, cb, fld(gc, fpk(gc, 0.0)));
done->spec.label.where = cb; resolve(gc, done, cb);
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, DISPOF(QBASE(esi,v0))), esi, eax, 4)));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, vx(window.y), edx, NULL, 1)));
cb = emit_code(gc, cb, and(gc, edx, value(gc, 0x7FFFFFL)));
cb = emit_code(gc, cb, sub(gc, edx, value(gc, 0x400000L)));
cb = emit_code(gc, cb, sar(gc, edx, value(gc, XY_FRAC_BITS)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, DISPOF(tr(y0Int))), esp, eax, 4), edx));

cb = emit_code(gc, cb, inc(gc, eax));

cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 3)));
cb = emit_code(gc, cb, jne(gc, head));

    }
	//	dxdy2	dxdy1	dxdy0

  // if ((y1Int - y0Int) <= 1) break;
#if 0
cb = emit_code(gc, cb, mov(gc, edx, tr(y1Int)));
cb = emit_code(gc, cb, sub(gc, edx, tr(y0Int)));
cb = emit_code(gc, cb, cmp(gc, edx, value(gc, 1)));
cb = emit_code(gc, cb, jle(gc, cull3));
#endif


    {
      /* y0Int and y2Int are vanilla integers now.  Turn them
       * into floats biased by (XY_BIAS + 1).
       */
      union {
	float f;
	unsigned long l;
      } u;

      u.f = (float)(XY_BIAS + 1);

cb = emit_code(gc, cb, mov(gc, edx, tr(y0Int)));

cb = emit_code(gc, cb, shl(gc, edx, value(gc, XY_FRAC_BITS)));
cb = emit_code(gc, cb, mov(gc, ebp, tr(y2Int)));

cb = emit_code(gc, cb, shl(gc, ebp, value(gc, XY_FRAC_BITS)));
cb = emit_code(gc, cb, add(gc, edx, value(gc, u.l)));

cb = emit_code(gc, cb, add(gc, ebp, value(gc, u.l)));
cb = emit_code(gc, cb, mov(gc, QBASE(esi,q0), edx));

cb = emit_code(gc, cb, mov(gc, QBASE(esi,qdx), ebp));
    }

cb = emit_code(gc, cb, mov(gc, edx, tr(dx)));
cb = emit_code(gc, cb, sar(gc, edx, value(gc, 31)));
cb = emit_code(gc, cb, mov(gc, QBASE(esi,negdx), edx));

cb = emit_code(gc, cb, mov(gc, eax, QBASE(esi,v0)));
cb = emit_code(gc, cb, mov(gc, ebx, QBASE(esi,v1)));
cb = emit_code(gc, cb, mov(gc, ecx, QBASE(esi,v2)));

cb = emit_code(gc, cb, fstp(gc, QBASE(esi,dxdy2)));
cb = emit_code(gc, cb, fstp(gc, QBASE(esi,dxdy1)));
cb = emit_code(gc, cb, fstp(gc, QBASE(esi,dxdy0)));

cb = emit_code(gc, cb, fld(gc, QBASE(esi,q0)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, vx(window.y), eax, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, QBASE(esi,qdx)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, vx(window.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, fstp(gc, QBASE(esi,ysnap0)));
cb = emit_code(gc, cb, fstp(gc, QBASE(esi,ysnap2)));

#if 0
cb = edge(gc, cb, ecx, dxdy2, ysnap2, tr(x2Int), tr(x2Frac), tr(dxdy2Int[0]), tr(dxdy2Int[1]), tr(dxdy2Frac), negdx, q0, fpcw);
cb = edge(gc, cb, eax, dxdy1, ysnap0, tr(x1Int), tr(x1Frac), tr(dxdy1Int[0]), tr(dxdy1Int[1]), tr(dxdy1Frac), negdx, q0, fpcw);
cb = edge(gc, cb, eax, dxdy0, ysnap0, tr(x0Int), tr(x0Frac), tr(dxdy0Int[0]), tr(dxdy0Int[1]), tr(dxdy0Frac), negdx, q0, fpcw);
#else
    {
      struct celem* head = label(gc);

cb = emit_code(gc, cb, mov(gc, ebp, value(gc, 2)));
cb = emit_code(gc, cb, mov(gc, edx, value(gc, 2)));

head->spec.label.where = cb; resolve(gc, head, cb);

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, DISPOF(v0)), NULL, edx, 4)));

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, DISPOF(dxdy0)), NULL, ebp, 4)));
cb = emit_code(gc, cb, fld(gc, st0));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, DISPOF(ysnap0)), NULL, edx, 4)));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, fist(gc, mem(gc, value(gc, DISPOF(tr(dxdy0Int[0]))), esp, ebp, 8)));

cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, fadd(gc, mem(gc, vx(window.x), esi, NULL, 1)));

cb = emit_code(gc, cb, fild(gc, mem(gc, value(gc, DISPOF(tr(dxdy0Int[0]))), esp, ebp, 8)));
					        // [12]	dxdy0Int	wx+dxdy0*ysnap0	dxdy0
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fadd(gc, fpk(gc, 3.0)));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, DISPOF(dxdy0)), NULL, ebp, 4)));
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, DISPOF(tr(dxdy0Int[0]))), esp, ebp, 8)));

cb = emit_code(gc, cb, fst(gc, q0));

cb = emit_code(gc, cb, sar(gc, esi, value(gc, 30)));
cb = emit_code(gc, cb, mov(gc, edx, q0));

cb = emit_code(gc, cb, and(gc, esi, value(gc, ~1)));
cb = emit_code(gc, cb, and(gc, edx, value(gc, 0x7FFFFFL)));

cb = emit_code(gc, cb, inc(gc, esi));
cb = emit_code(gc, cb, sub(gc, edx, value(gc, 0x400000L)));

cb = emit_code(gc, cb, sar(gc, edx, value(gc, XY_FRAC_BITS)));
cb = emit_code(gc, cb, add(gc, edi, esi));

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, DISPOF(tr(x0Int))), esp, ebp, 4), edx));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, DISPOF(tr(dxdy0Int[1]))), esp, ebp, 8), edi));

cb = emit_code(gc, cb, fild(gc, mem(gc, value(gc, DISPOF(tr(x0Int))), esp, ebp, 4)));
						//[22]	x0Int	x	dxdy0+3	dxdy0Int
cb = emit_code(gc, cb, fxch(gc, st2));

cb = emit_code(gc, cb, fsubrp(gc, st3, st0));

cb = emit_code(gc, cb, fadd(gc, fpk(gc, 3.0 - XY_BIAS)));
cb = emit_code(gc, cb, fxch(gc, st2));

cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, DISPOF(tr(dxdy0Frac))), esp, ebp, 4)));
						//[27]	x0Int	x
cb = emit_code(gc, cb, fsubp(gc, st1, st0));

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, DISPOF(tr(dxdy0Frac))), esp, ebp, 4)));

cb = emit_code(gc, cb, shl(gc, edx, value(gc, 9)));

cb = emit_code(gc, cb, xor(gc, edx, value(gc, 0x80000000UL)));

cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, DISPOF(tr(x0Frac))), esp, ebp, 4)));

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, DISPOF(tr(dxdy0Frac))), esp, ebp, 4), edx));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, DISPOF(tr(x0Frac))), esp, ebp, 4)));

cb = emit_code(gc, cb, shl(gc, edx, value(gc, 9)));

cb = emit_code(gc, cb, xor(gc, edx, value(gc, 0x80000000UL)));

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, DISPOF(tr(x0Frac))), esp, ebp, 4), edx));
cb = emit_code(gc, cb, xor(gc, edx, edx));

cb = emit_code(gc, cb, dec(gc, ebp));
cb = emit_code(gc, cb, jns(gc, head));

    }
#endif

cb = emit_code(gc, cb, mov(gc, ebp, value(gc, DISPOF(q0))));

cb = emit_code(gc, cb, fld(gc, tr(c)));
cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));

cb = emit_code(gc, cb, mov(gc, esi, QBASE(ebp,negdx)));
cb = emit_code(gc, cb, mov(gc, edx, QBASE(ebp,dxdy0)));
cb = emit_code(gc, cb, sar(gc, edx, value(gc, 31)));
cb = emit_code(gc, cb, xor(gc, edx, esi));
cb = emit_code(gc, cb, mov(gc, QBASE(ebp,negdxdy), edx));

    {
	struct celem* no = near_label(gc);

cb = emit_code(gc, cb, mov(gc, dl, tr2(dx,3)));
cb = emit_code(gc, cb, test(gc, dl, value(gc, 0x80)));
cb = emit_code(gc, cb, je(gc, no));

cb = emit_code(gc, cb, mov(gc, edx, tr(x0Int)));
cb = emit_code(gc, cb, mov(gc, ebp, tr(x1Int)));
cb = emit_code(gc, cb, mov(gc, edi, tr(x2Int)));

cb = emit_code(gc, cb, dec(gc, edx));
cb = emit_code(gc, cb, dec(gc, ebp));
cb = emit_code(gc, cb, dec(gc, edi));

cb = emit_code(gc, cb, mov(gc, tr(x0Int), edx));
cb = emit_code(gc, cb, mov(gc, tr(x1Int), ebp));
cb = emit_code(gc, cb, mov(gc, tr(x2Int), edi));

no->spec.label.where = cb; resolve(gc, no, cb);
    }

    /*if ((gc)->procs.lockRenderBuffers)*/ {
      struct celem* noneed = near_label(gc);

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, (int)&(gc)->buffers.lock.lockRenderBuffers), NULL, NULL, 1)));
cb = emit_code(gc, cb, test(gc, edx, edx));
cb = emit_code(gc, cb, je(gc, noneed));

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, (int)&(gc)->buffers.lock.lockCnt), NULL, NULL, 1)));
cb = emit_code(gc, cb, inc(gc, edx));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&(gc)->buffers.lock.lockCnt), NULL, NULL, 1), edx));
cb = emit_code(gc, cb, cmp(gc, edx, value(gc, 1)));
cb = emit_code(gc, cb, jne(gc, noneed));

cb = emit_code(gc, cb, mov(gc, edx, value(gc, (int)gc)));
cb = emit_code(gc, cb, push(gc, edx));
cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)&(gc)->buffers.lock.lockRenderBuffers), NULL, NULL, 1)));
cb = addesp(gc, cb, value(gc, 4));

cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));

noneed->spec.label.where = cb; resolve(gc, noneed, cb);
    }

    if (gc->buffers.doubleStore) {
	__GLcolorBuffer *cfb = gc->front;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSize = cfb->buf.elementSize;

cb = buf(gc, cb, tr(cp20), tr(dcp2dx), spanlet ? tr(dcp2dxSpan) : NULL, tr(dcp2dy0[0]), tr(dcp2dy0[1]), &cfb->buf.base, &cfb->buf.byteWidth, elementSize, negdx, tr, spanlet);
    }

    {
	__GLcolorBuffer *cfb = gc->drawBuffer;
	GLint byteWidth = cfb->buf.byteWidth;
	GLint elementSize = cfb->buf.elementSize;

cb = buf(gc, cb, tr(cp0), tr(dcpdx), spanlet ? tr(dcpdxSpan) : NULL, tr(dcpdy0[0]), tr(dcpdy0[1]), &cfb->buf.base, &cfb->buf.byteWidth, elementSize, negdx, tr, spanlet);
    }

    if (modeFlags & __GL_SHADE_STENCIL_TEST) {
	__GLstencilBuffer *sfb = &gc->stencilBuffer;
	GLint byteWidth = sfb->buf.byteWidth;
	GLint elementSize = sfb->buf.elementSize;

cb = buf(gc, cb, tr(sp0), tr(dspdx), spanlet ? tr(dspdxSpan) : NULL, tr(dspdy0[0]), tr(dspdy0[1]), &sfb->buf.base, &sfb->buf.byteWidth, elementSize, negdx, tr, spanlet);
    }

    if (modeFlags & __GL_SHADE_DEPTH_TEST &&
	(gc->state.depth.writeEnable ||
	 gc->state.depth.testFunc != GL_ALWAYS)) {
	__GLdepthBuffer *dfb = &gc->depthBuffer;
	GLint byteWidth = dfb->buf.byteWidth;
	GLint elementSize = dfb->buf.elementSize;

cb = buf(gc, cb, tr(zp0), tr(dzpdx), spanlet ? tr(dzpdxSpan) : NULL, tr(dzpdy0[0]), tr(dzpdy0[1]), &dfb->buf.base, &dfb->buf.byteWidth, elementSize, negdx, tr, spanlet);
    }

cb = emit_code(gc, cb, fldcw(gc, fpcw));

cb = emit_code(gc, cb, fild(gc, tr(dxdy0Int[0])));

    /* Until its use, esi holds original value of x0Int.
     * Watch carefully...
     */

    if (interpolants) {
cb = emit_code(gc, cb, mov(gc, esi, tr(x0Int)));
    }

    {
	struct celem* no = near_label(gc);
	struct celem* head = label(gc);

cb = emit_code(gc, cb, mov(gc, edx, tr(dx)));
cb = emit_code(gc, cb, cmp(gc, edx, value(gc, 0)));
cb = emit_code(gc, cb, jge(gc, no));

cb = emit_code(gc, cb, mov(gc, edx, value(gc, 8)));
head->spec.label.where = cb; resolve(gc, head, cb);
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, DISPOF(tr(x0Int))), esp, edx, 4)));
cb = emit_code(gc, cb, xor(gc, ebp, ebp));

cb = emit_code(gc, cb, sub(gc, ebp, esi));

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, DISPOF(tr(x0Int))), esp, edx, 4), ebp));
cb = emit_code(gc, cb, dec(gc, edx));

cb = emit_code(gc, cb, jns(gc, head));

no->spec.label.where = cb; resolve(gc, no, cb);
    }

cb = emit_code(gc, cb, mov(gc, ebp, value(gc, DISPOF(q0))));

cb = emit_code(gc, cb, fstp(gc, QBASE(ebp,dxdyInt0)));

    if (interpolants) {
      /* x0Int is a vanilla integer now.  Turn it
       * into a float biased by (XY_BIAS + 1).
       */
      union {
	float f;
	unsigned long l;
      } u;

      u.f = (float)(XY_BIAS) + 1.0F;

cb = emit_code(gc, cb, shl(gc, esi, value(gc, XY_FRAC_BITS)));
cb = emit_code(gc, cb, add(gc, esi, value(gc, u.l)));
cb = emit_code(gc, cb, mov(gc, QBASE(ebp,q0), esi));
cb = emit_code(gc, cb, fld(gc, QBASE(ebp,q0)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, vx(window.x), eax, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, fld(gc, st0));
cb = emit_code(gc, cb, fmul(gc, QBASE(ebp,dx0)));
cb = emit_code(gc, cb, fld(gc, st1));
cb = emit_code(gc, cb, fmul(gc, QBASE(ebp,dy0)));
cb = emit_code(gc, cb, fld(gc, st2));
cb = emit_code(gc, cb, fmul(gc, QBASE(ebp,dx1)));
cb = emit_code(gc, cb, fxch(gc, st3));

cb = emit_code(gc, cb, fmul(gc, QBASE(ebp,dy1)));
cb = emit_code(gc, cb, fxch(gc, st2));

cb = emit_code(gc, cb, fstp(gc, QBASE(ebp,dx0)));
cb = emit_code(gc, cb, fstp(gc, QBASE(ebp,dy0)));
cb = emit_code(gc, cb, fstp(gc, QBASE(ebp,dy1)));
cb = emit_code(gc, cb, fstp(gc, QBASE(ebp,dx1)));

cb = emit_code(gc, cb, fstp(gc, QBASE(ebp,xsnap0)));
    }

      if (modeFlags & __GL_SHADE_DEPTH_TEST &&
	  (gc->state.depth.writeEnable ||
	   gc->state.depth.testFunc != GL_ALWAYS)) {
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, vx(window.z), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, vx(window.z), ebx, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, vx(window.z), ecx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, esi, value(gc, (int)&z_bias)));
        assert(((int)&z_bias & 7) == 0);
cb = emit_code(gc, cb, lea(gc, edi, tr(z0)));
cb = emit_code(gc, cb, call(gc, fxinterp));
      }

      if (modeFlags & __GL_SHADE_SLOW_FOG) {
	    struct celem *scratch;

	    XLOCAL(scratch, 3 * 4);

cb = emit_code(gc, cb, mov(gc, eax, QBASE(ebp,v0)));
cb = emit_code(gc, cb, mov(gc, ebx, QBASE(ebp,v1)));
cb = emit_code(gc, cb, mov(gc, ecx, QBASE(ebp,v2)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(fog), eax, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, 255.0)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(fog), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, 255.0)));
cb = emit_code(gc, cb, fld(gc, mem(gc, vx(fog), ecx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, 255.0)));
cb = emit_code(gc, cb, fxch(gc, st2));

cb = emit_code(gc, cb, lea(gc, eax, scratch));

cb = emit_code(gc, cb, fstp(gc, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), eax, NULL, 1)));

cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, value(gc, 4), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, value(gc, 8), eax, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, esi, value(gc, (int)&color_no_dither_bias)));
cb = emit_code(gc, cb, lea(gc, edi, tr(f0)));
cb = emit_code(gc, cb, call(gc, fxinterp));
      }

    /* 
     * Don't do any color work at all if we're texturing
     * with GL_REPLACE
     */
    if (need_color(modeFlags, texenv))
      if (modeFlags & __GL_SHADE_SMOOTH) {

cb = emit_code(gc, cb, mov(gc, eax, QBASE(ebp,v0)));
cb = emit_code(gc, cb, mov(gc, ebx, QBASE(ebp,v1)));
cb = emit_code(gc, cb, mov(gc, ecx, QBASE(ebp,v2)));

	if ((modeFlags & (__GL_SHADE_TWOSIDED | __GL_SHADE_CHEAP_FOG)) ||
	    !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, vx(color), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, vx(color), ebx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, vx(color), ecx, NULL, 1)));
	} else {
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, vx(colors[0]), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, vx(colors[0]), ebx, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, vx(colors[0]), ecx, NULL, 1)));
	}

	if (modeFlags & __GL_SHADE_DITHER)
cb = emit_code(gc, cb, mov(gc, esi, value(gc, (int)&color_dither_bias)));
        else
cb = emit_code(gc, cb, mov(gc, esi, value(gc, (int)&color_no_dither_bias)));

    if (modeFlags & __GL_SHADE_RGB) {
cb = emit_code(gc, cb, mov(gc, edx, value(gc, 3)));
	color_posnarray[3] = tri(r0);
	color_posnarray[2] = tri(g0);
	color_posnarray[1] = tri(b0);
    } else {
cb = emit_code(gc, cb, mov(gc, edx, value(gc, 1)));
	color_posnarray[1] = tri(r0);
    }

        {
	  struct celem* head = label(gc);
head->spec.label.where = cb; resolve(gc, head, cb);
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, color_posns, NULL, edx, 4)));
cb = emit_code(gc, cb, lea(gc, edi, mem(gc, value(gc, TRBASE), esp, edi, 1)));

cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, call(gc, fxinterp));

cb = emit_code(gc, cb, pop(gc, ecx));
cb = emit_code(gc, cb, pop(gc, ebx));
cb = emit_code(gc, cb, pop(gc, eax));

cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, value(gc, 4), ecx, NULL, 1)));

cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, 4), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, value(gc, 4), ebx, NULL, 1)));

cb = emit_code(gc, cb, dec(gc, dl));
cb = emit_code(gc, cb, jne(gc, head));

	if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_BLEND)) {
cb = emit_code(gc, cb, mov(gc, esi, value(gc, (int)&alpha_bias)));

	    /* We have to scale alpha values by 256.0 / 255.0
	     */

	    if (!spanlet) {
	        struct celem *scratch;

	        XLOCAL(scratch, 3 * 4);

cb = emit_code(gc, cb, lea(gc, edi, scratch));

cb = emit_code(gc, cb, fld(gc, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, alpha_correcter));
cb = emit_code(gc, cb, fld(gc, mem(gc, NULL, ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, alpha_correcter));
cb = emit_code(gc, cb, fld(gc, mem(gc, NULL, ecx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, alpha_correcter));
cb = emit_code(gc, cb, fxch(gc, st2));

cb = emit_code(gc, cb, mov(gc, eax, edi));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, value(gc, 4), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, value(gc, 8), edi, NULL, 1)));

cb = emit_code(gc, cb, fstp(gc, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, NULL, ebx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, NULL, ecx, NULL, 1)));
	    }

cb = emit_code(gc, cb, lea(gc, edi, tr(a0)));
cb = emit_code(gc, cb, call(gc, fxinterp));
	}
      }

#if 0
      if (modeFlags & (__GL_SHADE_TWOSIDED | __GL_SHADE_CHEAP_FOG)) {
cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));
cb = emit_code(gc, cb, lea(gc, edx, mem(gc, vx(colors), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, edi, mem(gc, vx(colors), ebx, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, esi, mem(gc, vx(colors), ecx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(color), eax, NULL, 1), edx));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(color), ebx, NULL, 1), edi));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(color), ecx, NULL, 1), esi));
      }
#endif
  } else {
      if (modeFlags & __GL_SHADE_DITHER)
cb = emit_code(gc, cb, mov(gc, edx, value(gc, (int)&color_dither_bias)));
      else
cb = emit_code(gc, cb, mov(gc, edx, value(gc, (int)&color_no_dither_bias)));

cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, ((int)&gc->vertex.provoking)), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(color), esi, NULL, 1)));
        if (modeFlags & __GL_SHADE_RGB) {
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, offsetof(b, __GLcolor)), esi, NULL, 1)));
cb = emit_code(gc, cb, faddq(gc, mem(gc, NULL, edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, offsetof(g, __GLcolor)), esi, NULL, 1)));
cb = emit_code(gc, cb, faddq(gc, mem(gc, NULL, edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, offsetof(r, __GLcolor)), esi, NULL, 1)));
cb = emit_code(gc, cb, faddq(gc, mem(gc, NULL, edx, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st2));
    
		    //	b	g	r
cb = emit_code(gc, cb, fstpq(gc, q0));
cb = emit_code(gc, cb, mov(gc, edx, q0));
cb = emit_code(gc, cb, mov(gc, tr(b), edx));
    
cb = emit_code(gc, cb, fstpq(gc, q0));
cb = emit_code(gc, cb, mov(gc, edx, q0));
cb = emit_code(gc, cb, mov(gc, tr(g), edx));
    
cb = emit_code(gc, cb, fstpq(gc, q0));
cb = emit_code(gc, cb, mov(gc, edx, q0));
cb = emit_code(gc, cb, mov(gc, tr(r), edx));
        } else {
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, offsetof(r, __GLcolor)), esi, NULL, 1)));
cb = emit_code(gc, cb, faddq(gc, mem(gc, NULL, edx, NULL, 1)));
cb = emit_code(gc, cb, fstpq(gc, q0));
cb = emit_code(gc, cb, mov(gc, edx, q0));
cb = emit_code(gc, cb, mov(gc, tr(r), edx));
	}

    if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_BLEND)) {

cb = emit_code(gc, cb, mov(gc, edx, value(gc, (int)&alpha_bias)));

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, offsetof(a, __GLcolor)), esi, NULL, 1)));
        if (!spanlet)
cb = emit_code(gc, cb, fmul(gc, alpha_correcter));
cb = emit_code(gc, cb, faddq(gc, mem(gc, NULL, edx, NULL, 1)));
cb = emit_code(gc, cb, fstpq(gc, q0));
cb = emit_code(gc, cb, mov(gc, edx, q0));
cb = emit_code(gc, cb, mov(gc, tr(a), edx));
    }

    if (!(modeFlags & __GL_SHADE_TEXTURE))
      if (!(modeFlags & __GL_SHADE_DITHER)) {
	__GLcolorBuffer *cfb = gc->drawBuffer;

cb = emit_code(gc, cb, mov(gc, edx, tr(r)));
	if (!(modeFlags & __GL_SHADE_RGB)) {
cb = emit_code(gc, cb, shr(gc, edx, value(gc, 16)));
        } else {
cb = emit_code(gc, cb, mov(gc, edi, value(gc, 0x00ff0000)));
cb = emit_code(gc, cb, and(gc, edx, edi));
cb = gobit(gc, cb, edx, (16), cfb->redShift);

cb = emit_code(gc, cb, mov(gc, esi, tr(g)));
cb = emit_code(gc, cb, and(gc, esi, edi));
cb = gobit(gc, cb, esi, (16), cfb->greenShift);
cb = emit_code(gc, cb, or(gc, edx, esi));

cb = emit_code(gc, cb, mov(gc, esi, tr(b)));
cb = emit_code(gc, cb, and(gc, esi, edi));
cb = gobit(gc, cb, esi, (16), cfb->blueShift);
cb = emit_code(gc, cb, or(gc, edx, esi));

		if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_BLEND)) {
cb = emit_code(gc, cb, mov(gc, esi, tr(a)));
cb = emit_code(gc, cb, and(gc, esi, edi));
cb = gobit(gc, cb, esi, (16), cfb->alphaShift);
cb = emit_code(gc, cb, or(gc, edx, esi));
		}
        }
cb = emit_code(gc, cb, mov(gc, tr(r0), edx));
    }

#if 0
    if (modeFlags & __GL_SHADE_TWOSIDED) {
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, ((int)&gc->vertex.provoking)), NULL, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, edi, mem(gc, vx(colors), esi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(color), esi, NULL, 1), edi));
    }
#endif
  }
#if 1	/* JCB */

cb = restoreTriangleVerts(gc, cb, saved);

  if (modeFlags & __GL_SHADE_TEXTURE) {
      if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
	if ((!MMX || !__glCanMmx(gc)) || 
	    !(modeFlags & __GL_SHADE_RGB) || spanlet) {
        /* Need to set up fnegdx, fnegdxdy */

#define FPONE	0x3f800000

cb = emit_code(gc, cb, mov(gc, edx, tr(dx)));
cb = emit_code(gc, cb, and(gc, edx, value(gc, 0x80000000UL)));
cb = emit_code(gc, cb, or(gc, edx, value(gc, FPONE)));
cb = emit_code(gc, cb, mov(gc, fnegdx, edx));

cb = emit_code(gc, cb, mov(gc, edx, dxdy0));
cb = emit_code(gc, cb, and(gc, edx, value(gc, 0x80000000UL)));
cb = emit_code(gc, cb, or(gc, edx, value(gc, FPONE)));
cb = emit_code(gc, cb, mov(gc, fnegdxdy, edx));

	if (modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {
cb = emit_code(gc, cb, lea(gc, edi, tr(fsw0)));
cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, vx(texture.x), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, vx(texture.x), ebx, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, vx(texture.x), ecx, NULL, 1)));
cb = emit_code(gc, cb, call(gc, fpinterp));

cb = emit_code(gc, cb, lea(gc, edi, tr(ftw0)));
cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, vx(texture.y), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, vx(texture.y), ebx, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, vx(texture.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, call(gc, fpinterp));

cb = emit_code(gc, cb, lea(gc, edi, tr(fqw0)));
cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, vx(texture.w), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, vx(texture.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, vx(texture.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, call(gc, fpinterp));

	} else {
	struct celem *scratch;

	XLOCAL(scratch, 9 * 4);

cb = emit_code(gc, cb, lea(gc, edi, scratch));
cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.x), eax, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.y), eax, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.w), eax, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), edi, NULL, 1)));


cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.x), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.y), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 16), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 20), edi, NULL, 1)));


cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.x), ecx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 24), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 28), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 32), edi, NULL, 1)));


cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, 0), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, value(gc, 12), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, value(gc, 24), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, edi, tr(fsw0)));
cb = emit_code(gc, cb, call(gc, fpinterp));

cb = emit_code(gc, cb, lea(gc, edi, scratch));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, 4), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, value(gc, 16), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, value(gc, 28), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, edi, tr(ftw0)));
cb = emit_code(gc, cb, call(gc, fpinterp));

cb = emit_code(gc, cb, lea(gc, edi, scratch));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, 8), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, value(gc, 20), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, value(gc, 32), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, edi, tr(fqw0)));
cb = emit_code(gc, cb, call(gc, fpinterp));

	}

	if (texNeedRho) {
	    extern __glFRSetupRho(); /* Just so we know the address... */
	    struct celem* setup_rho = label(gc);

	    setup_rho->spec.label.where = (unsigned char*)&__glFRSetupRho;

cb = emit_code(gc, cb, lea(gc, eax, tr));
cb = emit_code(gc, cb, push(gc, ysnap0));
cb = emit_code(gc, cb, push(gc, xsnap0));
cb = emit_code(gc, cb, push(gc, dxdy0));
cb = emit_code(gc, cb, push(gc, dy1));
cb = emit_code(gc, cb, push(gc, dx1));
cb = emit_code(gc, cb, push(gc, dy0));
cb = emit_code(gc, cb, push(gc, dx0));
cb = emit_code(gc, cb, push(gc, v2));
cb = emit_code(gc, cb, push(gc, v1));
cb = emit_code(gc, cb, push(gc, v0));
cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, push(gc, value(gc, (int)gc)));

cb = emit_code(gc, cb, call(gc, setup_rho));

cb = emit_code(gc, cb, add(gc, esp, value(gc, 4 * 12)));

	    /* __glFRSetupRho(gc, &tr, v0, v1, v2, dx0, dy0, dx1, dy1, dxdy0,
			   xsnap0, ysnap0); */
	}

cb = emit_code(gc, cb, fld(gc, tr(fdswdx)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, __FR_TEXEL_PWL_SPAN_WIDTH)));
cb = emit_code(gc, cb, fld(gc, tr(fdtwdx)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, __FR_TEXEL_PWL_SPAN_WIDTH)));
cb = emit_code(gc, cb, fld(gc, tr(fdqwdx)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, __FR_TEXEL_PWL_SPAN_WIDTH)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, tr(fdswdxPWL)));
cb = emit_code(gc, cb, fstp(gc, tr(fdtwdxPWL)));
cb = emit_code(gc, cb, fstp(gc, tr(fdqwdxPWL)));

	if (texNeedRho) {
	    /* Negate rho if dx == -1. */
cb = emit_code(gc, cb, fild(gc, tr(dx)));
cb = emit_code(gc, cb, fmul(gc, tr(fdrhowdx)));
cb = emit_code(gc, cb, fst(gc, tr(fdrhowdx)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, __FR_TEXEL_PWL_SPAN_WIDTH)));
cb = emit_code(gc, cb, fstp(gc, tr(fdrhowdxPWL)));
	}
	if (spanlet) {
cb = emit_code(gc, cb, fld(gc, tr(fdswdx)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, 32.0)));
cb = emit_code(gc, cb, fld(gc, tr(fdtwdx)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, 32.0)));
cb = emit_code(gc, cb, fld(gc, tr(fdqwdx)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, 32.0)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, tr(fdswdxSpan)));
cb = emit_code(gc, cb, fstp(gc, tr(fdtwdxSpan)));
cb = emit_code(gc, cb, fstp(gc, tr(fdqwdxSpan)));

	        if (texNeedRho) {
cb = emit_code(gc, cb, fld(gc, tr(fdrhowdx)));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, 32.0)));
cb = emit_code(gc, cb, fstp(gc, tr(fdrhowdxSpan)));
	        }
	}
        } else {
	    struct celem
		*win1 = near_label(gc),
		*win2 = near_label(gc);
	    struct celem *scratch;

	    XLOCAL(scratch, 9 * 4);

	    /* MMX requires a fixed-point setup for PC */
	    /* (1) We divide through by the largest w */

cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));
    
	    if (modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, vx(texture.w), eax, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, edx, mem(gc, vx(texture.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, jge(gc, win1));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, vx(texture.w), ebx, NULL, 1)));
win1->spec.label.where = cb; resolve(gc, win1, cb);
cb = emit_code(gc, cb, cmp(gc, edx, mem(gc, vx(texture.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, jge(gc, win2));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, vx(texture.w), ecx, NULL, 1)));
win2->spec.label.where = cb; resolve(gc, win2, cb);

	    } else {

cb = emit_code(gc, cb, lea(gc, edi, scratch));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.w), eax, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 24), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 28), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 32), edi, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, 24), edi, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, edx, mem(gc, value(gc, 28), edi, NULL, 1)));
cb = emit_code(gc, cb, jge(gc, win1));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, 28), edi, NULL, 1)));
win1->spec.label.where = cb; resolve(gc, win1, cb);
cb = emit_code(gc, cb, cmp(gc, edx, mem(gc, value(gc, 32), edi, NULL, 1)));
cb = emit_code(gc, cb, jge(gc, win2));
cb = emit_code(gc, cb, mov(gc, edx, mem(gc, value(gc, 32), edi, NULL, 1)));
win2->spec.label.where = cb; resolve(gc, win2, cb);

	    }

/* 
 * We scale by 2.0 here to make largest 'q' (after scaling)
 * 0.5 - this prevents us from overflowing while interpolating
 * in the affine spans in the rasterizer.
 * JCB - we're more careful about not going over the edges
 * of the span, so this is no longer necessary.
 */

cb = emit_code(gc, cb, lea(gc, edi, scratch));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), edx));
cb = emit_code(gc, cb, fld(gc, mem(gc, NULL, edi, NULL, 1)));
//@           fmul    'fpk(gc, 2.0)'
cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));

	    if (modeFlags & __GL_SHADE_TEXTURE_PROJSCALED) {

cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 24), edi, NULL, 1)));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 28), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 32), edi, NULL, 1)));

	    } else {

cb = emit_code(gc, cb, fld(gc, st0));
cb = emit_code(gc, cb, fld(gc, st0));

cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), eax, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 24), edi, NULL, 1)));

cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 28), edi, NULL, 1)));

cb = emit_code(gc, cb, fmul(gc, mem(gc, vx(window.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 32), edi, NULL, 1)));

	    }


cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.x), eax, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 24), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.y), eax, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 24), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.w), eax, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 24), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 24), edi, NULL, 1)));


cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.x), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 28), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.y), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 28), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 16), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.w), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 28), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 28), edi, NULL, 1)));


cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.x), ecx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 32), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 32), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 20), edi, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, mem(gc, vx(texture.w), ecx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 32), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 32), edi, NULL, 1)));


cb = emit_code(gc, cb, mov(gc, esi, value(gc, (int)&affine_texture_bias)));

cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, 0), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, value(gc, 4), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, value(gc, 8), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, edi, tr(s0)));
cb = emit_code(gc, cb, call(gc, fxinterp));

cb = emit_code(gc, cb, lea(gc, edi, scratch));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, 12), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, value(gc, 16), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, value(gc, 20), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, edi, tr(t0)));
cb = emit_code(gc, cb, call(gc, fxinterp));

cb = emit_code(gc, cb, mov(gc, esi, value(gc, (int)&mmx_q_bias)));
cb = emit_code(gc, cb, lea(gc, edi, scratch));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, value(gc, 24), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, value(gc, 28), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, value(gc, 32), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, edi, tr(q0)));
cb = emit_code(gc, cb, call(gc, fxinterp));

	}
      } else {
cb = emit_code(gc, cb, mov(gc, esi, value(gc, (int)&affine_texture_bias)));

cb = emit_code(gc, cb, lea(gc, edi, tr(s0)));
cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, vx(texture.x), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, vx(texture.x), ebx, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, vx(texture.x), ecx, NULL, 1)));
cb = emit_code(gc, cb, call(gc, fxinterp));

cb = emit_code(gc, cb, lea(gc, edi, tr(t0)));
cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));
cb = emit_code(gc, cb, mov(gc, ecx, v2));
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, vx(texture.y), eax, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ebx, mem(gc, vx(texture.y), ebx, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, ecx, mem(gc, vx(texture.y), ecx, NULL, 1)));
cb = emit_code(gc, cb, call(gc, fxinterp));

      }
  }

  if (spanlet) {
cb = emit_code(gc, cb, mov(gc, tr(gc), value(gc, ((int)gc))));
    if (modeFlags & __GL_SHADE_DITHER) {
cb = emit_code(gc, cb, mov(gc, edi, tr(dx)));
cb = emit_code(gc, cb, shr(gc, edi, value(gc, 31)));
cb = emit_code(gc, cb, lea(gc, ebp, mem(gc, value(gc, (int)__glFRDitherTables), NULL, edi, 4)));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, 0), ebp, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, tr(ditherTable), ebp));
    }
  }

  if (modeFlags & __GL_SHADE_TEXTURE) {
      __GLtextureMachine *txm = &(gc->texture);
      __GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

cb = emit_code(gc, cb, mov(gc, ebp, value(gc, ((int)txm))));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, offsetof(currentTexture, __GLtextureMachine)), ebp, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, offsetof(level, __GLtexture)), ebp, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, 0), ebp, NULL, 1)));
      if (cookedTexture) {
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, offsetof(pixelBuffer, __GLmipMapLevel)), ebp, NULL, 1)));
      } else {
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, offsetof(buffer, __GLmipMapLevel)), ebp, NULL, 1)));
      }
cb = emit_code(gc, cb, mov(gc, tr(tp), edi));

      if (spanlet) {
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, offsetof(width, __GLmipMapLevel)), ebp, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, tr(texWidth), edi));

cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, offsetof(widthLog2, __GLmipMapLevel)), ebp, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, tr(texWidthLog2), edi));

cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, offsetof(height, __GLmipMapLevel)), ebp, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, tr(texHeight), edi));

cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, offsetof(heightLog2, __GLmipMapLevel)), ebp, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, tr(texHeightLog2), edi));
      }
  }

#if CHECK_STACK
      {
	struct celem *ok = near_label(gc);

cb = emit_code(gc, cb, fsub(gc, fpk(gc, 947.947f)));
cb = emit_code(gc, cb, fstp(gc, q0));
cb = emit_code(gc, cb, mov(gc, edx, q0));
cb = emit_code(gc, cb, cmp(gc, edx, value(gc, 0)));
cb = emit_code(gc, cb, je(gc, ok));
ok->spec.label.where = cb; resolve(gc, ok, cb);
      }
#endif

#endif /* JCB */

cb = emit_code(gc, cb, jmpi(gc, mem(gc, value(gc, (int)&gc->procs.renderTrap), NULL, NULL, 1)));

    {
	struct celem *v0p, *v1p, *v2p;
	struct celem *dxbias, *dybias, *p0bias;
	GLuint
	    pos_didx,
	    pos_didxSpan,
	    pos_didy0_0,
	    pos_didy0_1;
	
	v0p = mem2(gc, NULL, eax);
	v1p = mem2(gc, NULL, ebx);
	v2p = mem2(gc, NULL, ecx);

	p0bias = mem2(gc, NULL, esi);
	dxbias = mem2(gc, value(gc, 8), esi);
	dybias = mem2(gc, value(gc, 16), esi);

	if (spanlet ||
	    (modeFlags & __GL_SHADE_DEPTH_TEST &&
	     (gc->state.depth.writeEnable ||
	      gc->state.depth.testFunc != GL_ALWAYS))) {
	    pos_didx = tri(dzdx) - tri(z0);
	    if (spanlet)
		pos_didxSpan = tri(dzdxSpan) - tri(z0);
	    pos_didy0_0 = tri(dzdy0[0]) - tri(z0);
	    pos_didy0_1 = tri(dzdy0[1]) - tri(z0);
	} else if ((modeFlags & __GL_SHADE_TEXTURE) &&
		   (!(modeFlags & __GL_SHADE_TEXTURE_PERSP) ||
		    (MMX && __glCanMmx(gc)))) {
	    pos_didx = tri(dsdx) - tri(s0);
	    pos_didy0_0 = tri(dsdy0[0]) - tri(s0);
	    pos_didy0_1 = tri(dsdy0[1]) - tri(s0);
	} else if (need_color(modeFlags, texenv) &&
		   (modeFlags & __GL_SHADE_SMOOTH)) {
	    pos_didx = tri(drdx) - tri(r0);
	    pos_didy0_0 = tri(drdy0[0]) - tri(r0);
	    pos_didy0_1 = tri(drdy0[1]) - tri(r0);
	} 
	/* Otherwise there are no fixed-point interpolants,
	 so we don't need this stuff */

fxinterp->spec.label.where = cb; resolve(gc, fxinterp, cb);
cb = emit_code(gc, cb, fld(gc, v0p));
cb = emit_code(gc, cb, fld(gc, v1p));
cb = emit_code(gc, cb, fsub(gc, st1));
cb = emit_code(gc, cb, fld(gc, v2p));
cb = emit_code(gc, cb, fsub(gc, st2));
cb = emit_code(gc, cb, fld(gc, QBASE(ebp,dy1)));
cb = emit_code(gc, cb, fmul(gc, st2));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fld(gc, QBASE(ebp,dy0)));
cb = emit_code(gc, cb, fmul(gc, st2));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fmul(gc, QBASE(ebp,dx1)));
cb = emit_code(gc, cb, fxch(gc, st3));
		        		// 11	------------ STALL ------------
cb = emit_code(gc, cb, fsubp(gc, st1, st0));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fmul(gc, QBASE(ebp,dx0)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fldq(gc, dxbias));
cb = emit_code(gc, cb, fadd(gc, st1));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fsubp(gc, st3, st0));
cb = emit_code(gc, cb, fld(gc, st0));
cb = emit_code(gc, cb, fmul(gc, QBASE(ebp,xsnap0)));
cb = emit_code(gc, cb, fld(gc, QBASE(ebp,ysnap0)));
cb = emit_code(gc, cb, fmul(gc, st4));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fstpq(gc, QBASE(ebp,qdx)));
		        		// 22
cb = emit_code(gc, cb, faddp(gc, st2, st0));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, faddq(gc, p0bias));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fmul(gc, QBASE(ebp,dxdyInt0)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, faddq(gc, dybias));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, faddp(gc, st3, st0));

cb = emit_code(gc, cb, mov(gc, eax, QBASE(ebp,qdx)));
cb = emit_code(gc, cb, mov(gc, ebx, QBASE(ebp,negdx)));

cb = emit_code(gc, cb, xor(gc, eax, ebx));

cb = emit_code(gc, cb, sub(gc, eax, ebx));

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, pos_didx), edi, NULL, 1), eax));
	if (spanlet) {
cb = emit_code(gc, cb, mov(gc, ecx, eax));
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, 5)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, pos_didxSpan), edi, NULL, 1), ecx));
        }
cb = emit_code(gc, cb, mov(gc, ecx, QBASE(ebp,negdxdy)));

cb = emit_code(gc, cb, faddp(gc, st1, st0));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fstpq(gc, QBASE(ebp,q0)));
		       // 32
cb = emit_code(gc, cb, xor(gc, eax, ecx));
cb = emit_code(gc, cb, mov(gc, ebx, QBASE(ebp,q0)));

cb = emit_code(gc, cb, fstpq(gc, QBASE(ebp,qdy)));

cb = emit_code(gc, cb, sub(gc, eax, ecx));
cb = emit_code(gc, cb, mov(gc, ecx, QBASE(ebp,qdy)));

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, pos_didy0_0), edi, NULL, 1), ecx));
cb = emit_code(gc, cb, add(gc, ecx, eax));

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, pos_didy0_1), edi, NULL, 1), ecx));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), ebx));

cb = emit_code(gc, cb, ret(gc));

        if ((modeFlags & __GL_SHADE_TEXTURE) &&
	    (modeFlags & __GL_SHADE_TEXTURE_PERSP)) {

fpinterp->spec.label.where = cb; resolve(gc, fpinterp, cb);
cb = emit_code(gc, cb, fld(gc, v0p));
cb = emit_code(gc, cb, fld(gc, v1p));
cb = emit_code(gc, cb, fsub(gc, st1));
cb = emit_code(gc, cb, fld(gc, v2p));
cb = emit_code(gc, cb, fsub(gc, st2));
cb = emit_code(gc, cb, fld(gc, dy1));
cb = emit_code(gc, cb, fmul(gc, st2));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fld(gc, dy0));
cb = emit_code(gc, cb, fmul(gc, st2));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fmul(gc, dx1));
cb = emit_code(gc, cb, fxch(gc, st3));
		       // 11	------------ STALL ------------
cb = emit_code(gc, cb, fsubp(gc, st1, st0));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fmul(gc, dx0));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fld(gc, fnegdx));
cb = emit_code(gc, cb, fmul(gc, st1));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fsubp(gc, st3, st0));
cb = emit_code(gc, cb, fld(gc, st0));
cb = emit_code(gc, cb, fmul(gc, xsnap0));
cb = emit_code(gc, cb, fld(gc, ysnap0));
cb = emit_code(gc, cb, fmul(gc, st4));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), edi, NULL, 1)));
		       // 22
cb = emit_code(gc, cb, faddp(gc, st2, st0));
cb = emit_code(gc, cb, fst(gc, q0));
cb = emit_code(gc, cb, fmul(gc, dxdyInt0));
cb = emit_code(gc, cb, fxch(gc, st2));

        if (texNeedRho) {
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, 24), edi, NULL, 1)));
	}

cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, faddp(gc, st3, st0));

cb = emit_code(gc, cb, faddp(gc, st1, st0));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fstp(gc, mem(gc, NULL, edi, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, q0));
cb = emit_code(gc, cb, fmul(gc, fnegdxdy));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fst(gc, mem(gc, value(gc, (spanlet || FORCE_DIRECT) ? 16 : 12), edi, NULL, 1)));
cb = emit_code(gc, cb, faddp(gc, st1, st0));

cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, (spanlet || FORCE_DIRECT) ? 20 : 16), edi, NULL, 1)));

cb = emit_code(gc, cb, ret(gc));
        }
    }

cull3->spec.label.where = cb; resolve(gc, cull3, cb);
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fldcw(gc, fpcw));

cull->spec.label.where = cb; resolve(gc, cull, cb);
cb = restoreTriangleVerts(gc, cb, saved);
cb = emit_code(gc, cb, xor(gc, eax, eax));
/* Really a return, this is initialised at the start of setup. */
cb = emit_code(gc, cb, jmpi(gc, mem(gc, value(gc, (int)&gc->ogState.rsave), NULL, NULL, 1)));

    assert((unsigned)code + gc->constants.triSetupCacheSize > (unsigned)cb);
#if SOURCE_TRAIL
    og_stream(gc, NULL);
#endif

    return (setterupper)code;
}
#endif

/*
** Copyright 1991-1997 Silicon Graphics, Inc.
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
** $Revision: 4$
** $Date: 10/11/00 7:50:40 PM$
*/

#ifdef __GL_CODEGEN

#pragma optimize("", off)	/* MSVC 4.2 optmizer bugs...
				   silently generates bad code */

#include <stdio.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>

#include "context.h"
#include "render.h"
#include "global.h"
#include "og.h"

#include "geom_og.h"
#include "fr_tri.h"

#define EMIT(C, X)	emit_code(C, X)

#define STATIC static

#define FP_ZERO	0x00000000UL	/* IEEE 0.0f as a dword */
#define FP_ONE	0x3f800000UL	/* IEEE 1.0f as a dword */

#define vx(M)  \
 value(gc, __glNSWhereIs(&gc->vx_ns, offsetof(M, __GLvertex), ssizeof(M, __GLvertex)))

#define VX_SIZE (dynamic_vertex() ? __glNSSizeof(&gc->vx_ns) : sizeof(__GLvertex))

/* 
 * By default, the dynamic vertex feature is disabled.
 */

STATIC GLboolean
dynamic_vertex()
{
    static GLboolean cold = GL_TRUE;
    static GLboolean r;

    if (cold) {
	char *e;

	e = getenv("__GL_DISABLE_DYNAMIC_VERTEX");
	if (e != NULL) {
	    r = atoi(e) ? GL_FALSE : GL_TRUE;
	} else {
	    r = GL_FALSE;
	}
	cold = GL_FALSE;
    }
    return r;
}

/************************************************************************/

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

unsigned char* BuildDrawVertexes(unsigned char* cb, __GLcontext *gc)
{
    struct celem *glDrawVertexes = label(gc);
    struct celem *for_count = label(gc);
    struct celem *no_compile = label(gc);
    struct celem *not_first = label(gc);
    struct celem *endif1 = label(gc);
    struct celem *traverse = label(gc);
    struct celem *not_accept = label(gc);
    struct celem *next_tri = label(gc);
    struct celem *done = label(gc);
    struct celem
	*first,
	*count,
	*elements;
    struct celem *f_cliptriangle;
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    struct celem
	*v0,
	*v1,
	*v2,
	*batchFirst,
	*batchIndex,
	*batchCount,
	*batchOffset,
	*batchElements,
	*i,
	*i_end;
    GLsizei offset;
    GLboolean useElements = GL_TRUE;
    struct celem *tr;

    f_cliptriangle = label(gc);
    {
	extern void
	    __glDoClipTriangle(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2);

	f_cliptriangle->spec.label.where = (unsigned char*)&__glDoClipTriangle;
	f_cliptriangle->spec.label.name = "__glDoClipTriangle";
    }

    offset = 0;

#define SLOCAL(V, T) \
    (V = mem2(gc, value(gc, offset), esp),  offset += sizeof(T))
cb = emit_code(gc, cb, push(gc, esi));
cb = emit_code(gc, cb, push(gc, edi));
cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, push(gc, ebp));

    SLOCAL(tr, __GLtri);
    SLOCAL(v0, long);
    SLOCAL(v1, long);
    SLOCAL(v2, long);
    SLOCAL(batchFirst, long);
    SLOCAL(batchIndex, long);
    SLOCAL(batchCount, long);
    SLOCAL(batchOffset, long);
    SLOCAL(batchElements, long);
    SLOCAL(i, long);
    SLOCAL(i_end, long);
    SLOCAL(first, long);
    SLOCAL(count, long);
    SLOCAL(elements, long);

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, 28), esp, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 32), esp, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, 36), esp, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&gc->ogState.spsave2), NULL, NULL, 1), esp));
cb = emit_code(gc, cb, sub(gc, esp, value(gc, offset)));
cb = emit_code(gc, cb, and(gc, esp, value(gc, ~7)));

cb = emit_code(gc, cb, mov(gc, first, eax));
cb = emit_code(gc, cb, mov(gc, count, ebx));
cb = emit_code(gc, cb, mov(gc, elements, ecx));

glDrawVertexes->spec.label.where = cb; resolve(gc, glDrawVertexes, cb);

/* This costs almost nothing, and ensures that locals are in the cache... */
cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, batchCount));
cb = emit_code(gc, cb, mov(gc, eax, i_end));

cb = emit_code(gc, cb, mov(gc, eax, count));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 3)));
cb = emit_code(gc, cb, jl(gc, done));

cb = emit_code(gc, cb, mov(gc, batchCount, eax));

cb = emit_code(gc, cb, mov(gc, ecx, first));
cb = emit_code(gc, cb, mov(gc, batchIndex, ecx));
cb = emit_code(gc, cb, mov(gc, batchFirst, ecx));
cb = emit_code(gc, cb, mov(gc, batchOffset, value(gc, 0)));
cb = emit_code(gc, cb, mov(gc, edx, elements));
cb = emit_code(gc, cb, mov(gc, batchElements, edx));

for_count->spec.label.where = cb; resolve(gc, for_count, cb);

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->vertexArray.controlWord), NULL, NULL, 1)));
cb = emit_code(gc, cb, test(gc, eax, value(gc, VERTARRAY_CW_NEEDS_COMPILE)));
cb = emit_code(gc, cb, je(gc, no_compile));

cb = emit_code(gc, cb, mov(gc, eax, batchFirst));
cb = emit_code(gc, cb, mov(gc, ebx, batchIndex));

cb = emit_code(gc, cb, cmp(gc, eax, ebx));
cb = emit_code(gc, cb, jne(gc, not_first));

    /* first batch */
cb = emit_code(gc, cb, mov(gc, batchFirst, value(gc, 0)));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->vertexArray.batchSize), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, batchCount, eax));
cb = emit_code(gc, cb, mov(gc, batchElements, value(gc, 0)));
    /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
    {
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&gc->vertexArray.batchMode), NULL, NULL, 1), value(gc, GL_TRIANGLE_STRIP)));
cb = emit_code(gc, cb, mov(gc, eax, first));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&gc->vertexArray.batchFirst), NULL, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, ebx, elements));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&gc->vertexArray.batchElements), NULL, NULL, 1), ebx));
    }
cb = emit_code(gc, cb, jmp(gc, endif1));

not_first->spec.label.where = cb; resolve(gc, not_first, cb);
    /* subsequent batch */
cb = emit_code(gc, cb, mov(gc, edi, value(gc, (int)&vBuf[0])));

cb = emit_code(gc, cb, mov(gc, esi, value(gc, (int)vBuf)));
cb = emit_code(gc, cb, mov(gc, eax, batchOffset));
cb = emit_code(gc, cb, add(gc, eax, batchCount));
cb = emit_code(gc, cb, sub(gc, eax, value(gc, 2)));
cb = emit_code(gc, cb, imul(gc, eax, value(gc, VX_SIZE)));
cb = emit_code(gc, cb, add(gc, esi, eax));

cb = emit_code(gc, cb, mov(gc, ecx, value(gc, (2 * (VX_SIZE / 4)))));

cb = emit_code(gc, cb, rep(gc));
cb = emit_code(gc, cb, movsd(gc));

cb = emit_code(gc, cb, mov(gc, eax, value(gc, (int)&vBuf[0].colors[__GL_FRONTFACE])));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&vBuf[0].color), NULL, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, value(gc, (int)&vBuf[1].colors[__GL_FRONTFACE])));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&vBuf[1].color), NULL, NULL, 1), eax));

cb = emit_code(gc, cb, mov(gc, batchOffset, value(gc, 2)));

endif1->spec.label.where = cb; resolve(gc, endif1, cb);

    {
	struct celem* ok = near_label(gc);

cb = emit_code(gc, cb, mov(gc, eax, count));
cb = emit_code(gc, cb, mov(gc, ebx, batchCount));
cb = emit_code(gc, cb, cmp(gc, eax, ebx));
cb = emit_code(gc, cb, jg(gc, ok));
cb = emit_code(gc, cb, mov(gc, batchCount, eax));
ok->spec.label.where = cb; resolve(gc, ok, cb);
    }

cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, (int)&gc->vertexArray.continuation), NULL, NULL, 1)));
cb = emit_code(gc, cb, shl(gc, ebp, value(gc, 1)));

cb = emit_code(gc, cb, mov(gc, eax, elements));
cb = emit_code(gc, cb, mov(gc, ebx, batchCount));
cb = emit_code(gc, cb, sub(gc, ebx, ebp));
cb = emit_code(gc, cb, mov(gc, ecx, batchIndex));
cb = emit_code(gc, cb, mov(gc, edx, batchOffset));
cb = emit_code(gc, cb, add(gc, edx, ebp));

cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, push(gc, edx));
cb = emit_code(gc, cb, push(gc, value(gc, (int)gc)));

cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)&gc->vertexArray.compileElementsIndexed), NULL, NULL, 1)));
cb = emit_code(gc, cb, add(gc, esp, value(gc, (5 * sizeof(GLuint)))));

cb = emit_code(gc, cb, mov(gc, eax, batchIndex));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&gc->vertexArray.batchIndex), NULL, NULL, 1), eax));

no_compile->spec.label.where = cb; resolve(gc, no_compile, cb);

    {
	struct celem
	    *_else = near_label(gc), 
	    *_endif = near_label(gc);

cb = emit_code(gc, cb, mov(gc, eax, batchElements));
cb = emit_code(gc, cb, test(gc, eax, eax));
cb = emit_code(gc, cb, je(gc, _else));
    
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, 4), eax, NULL, 1)));
cb = emit_code(gc, cb, jmp(gc, _endif));

_else->spec.label.where = cb; resolve(gc, _else, cb);
cb = emit_code(gc, cb, mov(gc, ebx, batchFirst));
cb = emit_code(gc, cb, mov(gc, ecx, ebx));
cb = emit_code(gc, cb, inc(gc, ecx));

_endif->spec.label.where = cb; resolve(gc, _endif, cb);
cb = emit_code(gc, cb, imul(gc, ebx, value(gc, VX_SIZE)));
cb = emit_code(gc, cb, add(gc, ebx, value(gc, (int)vBuf)));
cb = emit_code(gc, cb, mov(gc, v0, ebx));
cb = emit_code(gc, cb, imul(gc, ecx, value(gc, VX_SIZE)));
cb = emit_code(gc, cb, add(gc, ecx, value(gc, (int)vBuf)));
cb = emit_code(gc, cb, mov(gc, v2, ecx));
    }

cb = emit_code(gc, cb, mov(gc, eax, batchFirst));
cb = emit_code(gc, cb, add(gc, eax, value(gc, 2)));

cb = emit_code(gc, cb, mov(gc, ebx, batchCount));
cb = emit_code(gc, cb, mov(gc, ecx, batchOffset));
cb = emit_code(gc, cb, add(gc, ebx, ecx));
cb = emit_code(gc, cb, mov(gc, i_end, ebx));

    if ((gc->state.polygon.frontMode != GL_FILL) ||
	 (gc->state.polygon.backMode != GL_FILL)) {
cb = emit_code(gc, cb, mov(gc, edx, v0));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(boundaryEdge), edx, NULL, 1), value(gc, GL_TRUE)));
cb = emit_code(gc, cb, mov(gc, edx, v2));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(boundaryEdge), edx, NULL, 1), value(gc, GL_TRUE)));
    }

traverse->spec.label.where = cb; resolve(gc, traverse, cb);
    /* eax is 'i' */
cb = emit_code(gc, cb, mov(gc, ebx, eax));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 1)));

cb = emit_code(gc, cb, xor(gc, eax, value(gc, 1)));
cb = emit_code(gc, cb, lea(gc, edi, v0));

cb = emit_code(gc, cb, mov(gc, edx, v2));
cb = emit_code(gc, cb, mov(gc, ecx, batchElements));

cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, eax, 4), edx));

    if (useElements) {
	struct celem
	    *_else = near_label(gc), 
	    *_endif = near_label(gc);

cb = emit_code(gc, cb, test(gc, ecx, ecx));
cb = emit_code(gc, cb, je(gc, _else));

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, NULL, ecx, ebx, 4)));
cb = emit_code(gc, cb, jmp(gc, _endif));

_else->spec.label.where = cb; resolve(gc, _else, cb);
cb = emit_code(gc, cb, mov(gc, eax, ebx));

_endif->spec.label.where = cb; resolve(gc, _endif, cb);
cb = scaleby(gc, cb, ecx, eax, VX_SIZE);
cb = emit_code(gc, cb, add(gc, ecx, value(gc, (int)vBuf)));
cb = emit_code(gc, cb, inc(gc, ebx));
cb = emit_code(gc, cb, mov(gc, i, ebx));
    } else {
	/* If we're not using elements, it's much easier: just increment
	 * v2 by one vertex.
	 */
cb = emit_code(gc, cb, inc(gc, ebx));
cb = emit_code(gc, cb, mov(gc, ecx, v2));

cb = emit_code(gc, cb, add(gc, ecx, value(gc, sizeof(vBuf[0]))));
cb = emit_code(gc, cb, mov(gc, i, ebx));
    }
cb = emit_code(gc, cb, mov(gc, v2, ecx));

    if ((gc->state.polygon.frontMode != GL_FILL) ||
	 (gc->state.polygon.backMode != GL_FILL)) {
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(boundaryEdge), ecx, NULL, 1), value(gc, GL_TRUE)));
    }

    if (!(gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH) ||
	(gc->polygon.shader.modeFlags & __GL_SHADE_CHEAP_FOG) ||
	!(gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT) ||
	(gc->state.polygon.frontMode != GL_FILL ||
	gc->state.polygon.backMode != GL_FILL)) {
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, (int)&gc->vertex.provoking), NULL, NULL, 1), ecx));
    }

    /* Now we have triangle in v0,v1,v2.
     * Do the clip check.
     */

cb = emit_code(gc, cb, mov(gc, eax, v0));
cb = emit_code(gc, cb, mov(gc, ebx, v1));

cb = emit_code(gc, cb, mov(gc, edi, mem(gc, vx(hasAndClipCode), ecx, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, edx, mem(gc, vx(hasAndClipCode), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, vx(hasAndClipCode), ebx, NULL, 1)));

    /* Check for trivial accept */
cb = emit_code(gc, cb, mov(gc, ebp, edx));
cb = emit_code(gc, cb, or(gc, ebp, esi));
cb = emit_code(gc, cb, or(gc, ebp, edi));
cb = emit_code(gc, cb, test(gc, ebp, value(gc, __GL_ALL_CLIP_MASK)));
cb = emit_code(gc, cb, jne(gc, not_accept));

#if 1
cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, push(gc, value(gc, (int)gc)));
cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)&(gc->procs.renderTriangle)), NULL, NULL, 1)));
cb = emit_code(gc, cb, add(gc, esp, value(gc, 16)));
#else
cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)&(gc->procs.simpleRenderTriangle)), NULL, NULL, 1)));
#endif

cb = emit_code(gc, cb, jmp(gc, next_tri));

not_accept->spec.label.where = cb; resolve(gc, not_accept, cb);
    /* Now check for trivial reject */
cb = emit_code(gc, cb, mov(gc, ebp, edx));
cb = emit_code(gc, cb, and(gc, ebp, esi));
cb = emit_code(gc, cb, and(gc, ebp, edi));
cb = emit_code(gc, cb, and(gc, ebp, value(gc, __GL_ALL_CLIP_MASK)));
cb = emit_code(gc, cb, jne(gc, next_tri));

    /* Must clip */
cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, push(gc, value(gc, (int)gc)));
cb = emit_code(gc, cb, call(gc, f_cliptriangle));
cb = emit_code(gc, cb, add(gc, esp, value(gc, 16)));

next_tri->spec.label.where = cb; resolve(gc, next_tri, cb);

cb = emit_code(gc, cb, mov(gc, eax, i));
cb = emit_code(gc, cb, mov(gc, ebx, i_end));
cb = emit_code(gc, cb, cmp(gc, eax, ebx));
cb = emit_code(gc, cb, jl(gc, traverse));

cb = emit_code(gc, cb, mov(gc, ebx, batchCount));

cb = emit_code(gc, cb, mov(gc, eax, count));
cb = emit_code(gc, cb, mov(gc, ecx, batchIndex));

cb = emit_code(gc, cb, add(gc, ecx, ebx));
cb = emit_code(gc, cb, sub(gc, eax, ebx));

cb = emit_code(gc, cb, mov(gc, count, eax));
cb = emit_code(gc, cb, mov(gc, batchIndex, ecx));

cb = emit_code(gc, cb, ja(gc, for_count));

done->spec.label.where = cb; resolve(gc, done, cb);

cb = emit_code(gc, cb, mov(gc, esp, mem(gc, value(gc, (int)&gc->ogState.spsave2), NULL, NULL, 1)));

cb = emit_code(gc, cb, pop(gc, ebp));
cb = emit_code(gc, cb, pop(gc, ecx));
cb = emit_code(gc, cb, pop(gc, ebx));
cb = emit_code(gc, cb, pop(gc, edi));
cb = emit_code(gc, cb, pop(gc, esi));

cb = emit_code(gc, cb, ret(gc));

    return cb;
}

/************************************************************************/

static unsigned char* clamp(__GLcontext *gc,
			    unsigned char* cb,
			    struct celem* r,
			    struct celem* bound)
{
  struct celem *ok = label(gc);

cb = emit_code(gc, cb, cmp(gc, r, bound));
cb = emit_code(gc, cb, jl(gc, ok));
cb = emit_code(gc, cb, mov(gc, r, bound));
ok->spec.label.where = cb; resolve(gc, ok, cb);

  return cb;
}

/*
 * src is incoming pointer
 * dst is pointer to vertex
 */

static unsigned char* do_load(__GLcontext *gc,
			      unsigned char *cb,
			      struct celem *src,
			      GLuint type,
			      struct celem *scratch)
{
  switch (type) {
  case GL_SHORT:
cb = emit_code(gc, cb, movsxw(gc, ebx, src));
cb = emit_code(gc, cb, mov(gc, scratch, ebx));
cb = emit_code(gc, cb, fild(gc, scratch));
     break;
  case GL_INT:
cb = emit_code(gc, cb, fild(gc, src));
     break;
  case GL_FLOAT:
cb = emit_code(gc, cb, fld(gc, src));
     break;
  case GL_DOUBLE:
cb = emit_code(gc, cb, fldq(gc, src));
     break;
  default:
     assert(0);
  }

  return cb;
}


/* FAC(N) is the conversion factor for an N-bit quantity,
 * as listed in the GL specificiation.
 * 1 / ((2 ^ N) - 1)
 */
/* NB: this works for N = 32 because ((1 << 32) - 1) == ~0 */

#define FAC(N)	((GLfloat)(1.0 / ((double)(unsigned long)((1UL << N) - 1UL))))

static unsigned char* conv_load(__GLcontext* gc,
				unsigned char* cb,
				struct celem* src,
				GLuint type,
				struct celem* work)
{
  switch (type) {
  case GL_UNSIGNED_BYTE:
cb = emit_code(gc, cb, mov(gc, bl, src));
cb = emit_code(gc, cb, mov(gc, work, ebx));
cb = emit_code(gc, cb, fild(gc, work));
      break;
  case GL_BYTE:
cb = emit_code(gc, cb, movsx(gc, ebx, src));
cb = emit_code(gc, cb, add(gc, ebx, ebx));
cb = emit_code(gc, cb, inc(gc, ebx));
cb = emit_code(gc, cb, mov(gc, work, ebx));
cb = emit_code(gc, cb, fild(gc, work));
      break;
  case GL_UNSIGNED_SHORT:
cb = emit_code(gc, cb, movzxw(gc, ebx, src));
cb = emit_code(gc, cb, mov(gc, work, ebx));
cb = emit_code(gc, cb, fild(gc, work));
      break;
  case GL_SHORT:
cb = emit_code(gc, cb, movsxw(gc, ebx, src));
cb = emit_code(gc, cb, add(gc, ebx, ebx));
cb = emit_code(gc, cb, inc(gc, ebx));
cb = emit_code(gc, cb, mov(gc, work, ebx));
cb = emit_code(gc, cb, fild(gc, work));
      break;
  case GL_UNSIGNED_INT:
cb = emit_code(gc, cb, lea(gc, ecx, work));
cb = emit_code(gc, cb, mov(gc, ebx, src));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, ecx, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 4), ecx, NULL, 1), value(gc, 0)));
cb = emit_code(gc, cb, fildq(gc, mem(gc, NULL, ecx, NULL, 1)));
      break;
  case GL_INT:
      /* This one starts off by doubling a 32-bit quantity, so
       * we need to do 64-bit arithmetic.
       * Just switch to floating point instead.  Could probably do
       * better in integer, at the expense of longer code.
       */
cb = emit_code(gc, cb, fild(gc, src));
cb = emit_code(gc, cb, fmul(gc, fpk(gc, 2.0)));
cb = emit_code(gc, cb, fadd(gc, fpk(gc, 1.0)));
      break;
  case GL_FLOAT:
cb = emit_code(gc, cb, fld(gc, src));
      break;
  case GL_DOUBLE:
cb = emit_code(gc, cb, fldq(gc, src));
     break;
  default:
     assert(0);
  }

  return cb;
}

unsigned char* do_normal_transform(__GLcontext *gc,
				   unsigned char* cb,
				   __GLtransform *tr,
				   struct celem* nx,
				   struct celem* ny,
				   struct celem* nz)
{
cb = emit_code(gc, cb, mov(gc, ecx, value(gc, (int)(tr->inverseTranspose.matrix))));

#define MAT(R,C)	mem2(gc, value(gc, (R) * 16 + (C) * 4), ecx)

cb = emit_code(gc, cb, fld(gc, mem(gc, NULL, esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, MAT(0,0)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, MAT(1,0)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, MAT(2,0)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
    
cb = emit_code(gc, cb, fld(gc, mem(gc, NULL, esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, MAT(0,1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, faddp(gc, st2, st0));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, MAT(1,1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, MAT(2,1)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, nx));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
    
cb = emit_code(gc, cb, fld(gc, mem(gc, NULL, esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, MAT(0,2)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, MAT(1,2)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, MAT(2,2)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, ny));
    
cb = emit_code(gc, cb, faddp(gc, st1, st0));
    
cb = emit_code(gc, cb, fstp(gc, nz));

    return cb;
}

/* Copy a coordinate from [eax] to the floating-point destination t0,1,2,3.
 * If the size is less than 4, set t3 to 1.0
 * If the size is less than 3, set t2 to 0.0
 * If the size is less than 2, set t1 to 0.0
 *
 * This routine is used to handle position and texture coordinates.
 */
static unsigned char* CopyCoord(__GLcontext* gc,
				unsigned char* cb,
				GLuint type,
				GLuint size,
				struct celem* t0,
				struct celem* t1,
				struct celem* t2,
				struct celem* t3)
{
    GLsizei isize;	/* How big is each input value */

    switch (type) {
    case GL_SHORT:      isize = 2;	break;
    case GL_INT:	isize = 4;	break;
    case GL_FLOAT:      isize = 4;	break;
    case GL_DOUBLE:     isize = 8;	break;
    }

    if (type == GL_FLOAT) {
	/* We can avoid doing any fp work */

	if (size >= 1) {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, t0, ebx));
	}

	if (size >= 2) {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 4), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, t1, ebx));
	} else {
cb = emit_code(gc, cb, mov(gc, t1, value(gc, FP_ZERO)));
	}

	if (size >= 3) {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 8), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, t2, ebx));
	} else {
cb = emit_code(gc, cb, mov(gc, t2, value(gc, FP_ZERO)));
	}

	if (size >= 4) {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 12), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, t3, ebx));
	} else {
cb = emit_code(gc, cb, mov(gc, t3, value(gc, FP_ONE)));
	}
    } else {
    	switch (size) {
    	case 1:
cb = do_load(gc, cb, mem(gc, NULL, eax, NULL, 1), type, t0);
cb = emit_code(gc, cb, mov(gc, ebx, value(gc, FP_ZERO)));
cb = emit_code(gc, cb, mov(gc, t3, value(gc, FP_ONE)));
cb = emit_code(gc, cb, mov(gc, t1, ebx));
cb = emit_code(gc, cb, mov(gc, t2, ebx));
cb = emit_code(gc, cb, fstp(gc, t0));
    	    break;
    	case 2:
cb = do_load(gc, cb, mem(gc, NULL, eax, NULL, 1), type, t0);
cb = do_load(gc, cb, mem(gc, value(gc, isize), eax, NULL, 1), type, t1);
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fstp(gc, t0));
cb = emit_code(gc, cb, fstp(gc, t1));
cb = emit_code(gc, cb, mov(gc, t2, value(gc, FP_ZERO)));
cb = emit_code(gc, cb, mov(gc, t3, value(gc, FP_ONE)));
    	    break;
    	case 3:
cb = do_load(gc, cb, mem(gc, NULL, eax, NULL, 1), type, t0);
cb = do_load(gc, cb, mem(gc, value(gc, isize), eax, NULL, 1), type, t1);
cb = do_load(gc, cb, mem(gc, value(gc, 2 * isize), eax, NULL, 1), type, t2);
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, t0));
cb = emit_code(gc, cb, fstp(gc, t1));
cb = emit_code(gc, cb, fstp(gc, t2));
cb = emit_code(gc, cb, mov(gc, t3, value(gc, FP_ONE)));
    	    break;
    	case 4:
cb = do_load(gc, cb, mem(gc, NULL, eax, NULL, 1), type, t0);
cb = do_load(gc, cb, mem(gc, value(gc, isize), eax, NULL, 1), type, t1);
cb = do_load(gc, cb, mem(gc, value(gc, 2 * isize), eax, NULL, 1), type, t2);
cb = do_load(gc, cb, mem(gc, value(gc, 3 * isize), eax, NULL, 1), type, t3);
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fstp(gc, t0));
cb = emit_code(gc, cb, fstp(gc, t2));
cb = emit_code(gc, cb, fstp(gc, t1));
cb = emit_code(gc, cb, fstp(gc, t3));
    	    break;
        }
    }

    return cb;
}

static void 
calc_size_and_scale(GLuint *isize,
		    GLfloat *scale,
		    GLenum type)
{
    switch (type) {
    case GL_BYTE:
	*isize = 1;
	*scale = FAC(8);
	break;
    case GL_UNSIGNED_BYTE:
	*isize = 1;
	*scale = FAC(8);
	break;
    case GL_SHORT:
	*isize = 2;	
	*scale = FAC(16);
	break;
    case GL_UNSIGNED_SHORT:     
	*isize = 2;	
	*scale = FAC(16);
	break;
    case GL_INT:		
	*isize = 4;	
	*scale = FAC(32);
	break;
    case GL_UNSIGNED_INT:	
	*isize = 4;	
	*scale = FAC(32);
	break;
    case GL_FLOAT:		
	*isize = 4;	
	*scale = 1.0f;
	break;
    case GL_DOUBLE:		
	*isize = 8;	
	*scale = 1.0f;
	break;
    default:
	assert(GL_FALSE);
    }
}

/*
 * Copy color, and do the same work as __glClampAndScaleColorf.
 * Also copies index in color index mode.
 * As ever, we're careful not to read from the destination buffer.
 */
static unsigned char* CopyColor(__GLcontext* gc,
				unsigned char* cb,
				GLuint type,
				GLuint size,
				struct celem* work,
				struct celem* rr,
				struct celem* gg,
				struct celem* bb,
				struct celem* aa)
{
    GLsizei isize;	/* How big is each array element */
    GLfloat red_s, green_s, blue_s, alpha_s;
    GLfloat scale;
    GLboolean must_clamp = ((type != GL_UNSIGNED_BYTE) && 
			    (type != GL_UNSIGNED_SHORT) &&
			    (type != GL_UNSIGNED_INT));

    /* If we're not color clamping, we can write results straight into
     * the vertex.
     */
    if (!must_clamp) {
	rr = mem2(gc, vx(colors[0].r), edi);
	gg = mem2(gc, vx(colors[0].g), edi);
	bb = mem2(gc, vx(colors[0].b), edi);
	aa = mem2(gc, vx(colors[0].a), edi);
    }

    if (gc->modes.colorIndexMode)
	size = 1;

    calc_size_and_scale(&isize, &scale, type);

    red_s = gc->frontBuffer.redScale * scale;
    green_s = gc->frontBuffer.greenScale * scale;
    blue_s = gc->frontBuffer.blueScale * scale;
    alpha_s = gc->frontBuffer.alphaScale * scale;

    if (type == GL_UNSIGNED_BYTE) {
cb = emit_code(gc, cb, xor(gc, ebx, ebx));
    }

    switch (size) {
    case 1:
	/* color index case is a straight copy */

cb = conv_load(gc, cb, mem(gc, NULL, eax, NULL, 1), type, work);
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(colors[0].r), edi, NULL, 1)));
        break;
    case 3:
	/* Load up colors, scale and store in scratch variables */

cb = conv_load(gc, cb, mem(gc, NULL, eax, NULL, 1), type, work);
cb = conv_load(gc, cb, mem(gc, value(gc, isize), eax, NULL, 1), type, work);
cb = conv_load(gc, cb, mem(gc, value(gc, 2 * isize), eax, NULL, 1), type, work);
cb = emit_code(gc, cb, fld(gc, fpk(gc, red_s)));
cb = emit_code(gc, cb, fmulp(gc, st3, st0));
cb = emit_code(gc, cb, fld(gc, fpk(gc, green_s)));
cb = emit_code(gc, cb, fmulp(gc, st2, st0));
cb = emit_code(gc, cb, fld(gc, fpk(gc, blue_s)));
cb = emit_code(gc, cb, fmulp(gc, st1, st0));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, rr));
cb = emit_code(gc, cb, fstp(gc, gg));
cb = emit_code(gc, cb, fstp(gc, bb));
	break;
    case 4:
cb = conv_load(gc, cb, mem(gc, NULL, eax, NULL, 1), type, work);
cb = emit_code(gc, cb, fmul(gc, fpk(gc, red_s)));
cb = conv_load(gc, cb, mem(gc, value(gc, isize), eax, NULL, 1), type, work);
cb = emit_code(gc, cb, fmul(gc, fpk(gc, green_s)));
cb = conv_load(gc, cb, mem(gc, value(gc, 2 * isize), eax, NULL, 1), type, work);
cb = emit_code(gc, cb, fmul(gc, fpk(gc, blue_s)));
cb = conv_load(gc, cb, mem(gc, value(gc, 3 * isize), eax, NULL, 1), type, work);
cb = emit_code(gc, cb, fmul(gc, fpk(gc, alpha_s)));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fstp(gc, rr));
cb = emit_code(gc, cb, fstp(gc, bb));
cb = emit_code(gc, cb, fstp(gc, gg));
cb = emit_code(gc, cb, fstp(gc, aa));
    }

    /* Now the color scratch-pad values 'rr', 'gg', 'bb' and maybe 'aa'
     * are scaled, we need to clamp them.
     * (If type was originally one of the unsigned integer forms,
     * we can't overflow, so we don't have to clamp.)
     */

    if ((size >= 3) &&
	must_clamp) {
	union {
	    float f;
	    int i;
	} u[4];

	u[0].f = gc->frontBuffer.redScale;
	u[1].f = gc->frontBuffer.greenScale;
	u[2].f = gc->frontBuffer.blueScale;
	u[3].f = gc->frontBuffer.alphaScale;
	
cb = emit_code(gc, cb, mov(gc, ebx, rr));
cb = clamp(gc, cb, ebx, value(gc, u[0].i));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].r), edi, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, ebx, gg));
cb = clamp(gc, cb, ebx, value(gc, u[1].i));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].g), edi, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, ebx, bb));
cb = clamp(gc, cb, ebx, value(gc, u[2].i));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].b), edi, NULL, 1), ebx));

        if (gc->polygon.shader.modeFlags & 
	    (__GL_SHADE_BLEND | __GL_SHADE_ALPHA_TEST)) {
	    if (size == 4) {
cb = emit_code(gc, cb, mov(gc, ebx, aa));
cb = clamp(gc, cb, ebx, value(gc, u[3].i));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].a), edi, NULL, 1), ebx));
            } else {
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].a), edi, NULL, 1), value(gc, FP_ONE)));
	    }
	}
    } else {
        if ((size == 3) &&
	    gc->polygon.shader.modeFlags & 
	    (__GL_SHADE_BLEND | __GL_SHADE_ALPHA_TEST)) {
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].a), edi, NULL, 1), value(gc, FP_ONE)));
	}
    }

    return cb;
}

static unsigned char* CopyNormal(__GLcontext* gc,
				 unsigned char* cb,
				 GLuint type,
				 struct celem* work)
{
    GLsizei isize;	/* How big is each array element */
    GLfloat scale;

    calc_size_and_scale(&isize, &scale, type);

    if (type == GL_FLOAT) {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, NULL, eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.x), edi, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 4), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.y), edi, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 8), eax, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.z), edi, NULL, 1), ebx));
    } else {
cb = conv_load(gc, cb, mem(gc, NULL, eax, NULL, 1), type, work);
cb = emit_code(gc, cb, fmul(gc, fpk(gc, scale)));
cb = conv_load(gc, cb, mem(gc, value(gc, isize), eax, NULL, 1), type, work);
cb = emit_code(gc, cb, fmul(gc, fpk(gc, scale)));
cb = conv_load(gc, cb, mem(gc, value(gc, 2 * isize), eax, NULL, 1), type, work);
cb = emit_code(gc, cb, fmul(gc, fpk(gc, scale)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(normal.x), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(normal.y), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(normal.z), edi, NULL, 1)));
    }

    return cb;
}


/*
 * do_light
 *
 * Do lighting calculations for 1 vertex
 *
 * esi	source normal
 * edi	destination vertex
 */

static unsigned char* do_light(__GLcontext *gc,
			       unsigned char* cb,
			       struct celem* nx,
			       struct celem* ny,
			       struct celem* nz,
			       struct celem* rr,
			       struct celem* gg,
			       struct celem* bb,
			       struct celem* d)
{
    struct celem* its_dark = label(gc);
    __GLtransform *tr = gc->transform.modelView;
    __GLlightSourceMachine *lsm;
    __GLmaterialMachine *msm;
    GLboolean xform_normal = GL_FALSE;	/* Set to GL_FALSE if we're doing model
					 * space lighting */
    GLboolean one_light;

    msm = &gc->light.front;
    lsm = gc->light.sources;

    one_light = (lsm != NULL) && (lsm->next == NULL);

    if (xform_normal) {
cb = do_normal_transform(gc, cb, tr, nx, ny, nz);
    } else {
	nx = mem2(gc, NULL, esi);
	ny = mem2(gc, value(gc, 4), esi);
	nz = mem2(gc, value(gc, 8), esi);
    }

    if (gc->modes.colorIndexMode) {
cb = emit_code(gc, cb, fld(gc, fpk(gc, 0.0)));
cb = emit_code(gc, cb, fld(gc, fpk(gc, 0.0)));
    } else {
	/* This color is computed in lightsToModel; it's the clamped sum of
	 * the sceneColor and all the ambient lights.
	 */
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&gc->light.baseColor.b), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&gc->light.baseColor.g), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&gc->light.baseColor.r), NULL, NULL, 1)));
    }

	{
	  while (lsm) {
	    struct celem* no_diffuse = label(gc);
	    struct celem* next_light = label(gc);
	    __GLlightSourcePerMaterialMachine *lspmm;

	    lspmm = &lsm->front + __GL_FRONTFACE;

	    {
		__GLcoord *light_dir;

		if (xform_normal)
		    light_dir = &lsm->unitVPpli;
		else
		    light_dir = &lsm->unitVPpliModel;

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&light_dir->x), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, nx));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&light_dir->y), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, ny));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&light_dir->z), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, nz));
	    }
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, faddp(gc, st1, st0));

cb = emit_code(gc, cb, faddp(gc, st1, st0));

	    /* writing 'd' will be a write hit, so do as much as possible
	     * before this happens. */

	    if (!gc->modes.colorIndexMode) {
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&lspmm->diffuse.g), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, st1));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&lspmm->diffuse.r), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, st2));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&lspmm->diffuse.b), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, st3));
cb = emit_code(gc, cb, fxch(gc, st3));
	    } else {
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&lsm->dli), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, st1));
cb = emit_code(gc, cb, fxch(gc, st1));
	    }

cb = emit_code(gc, cb, fstp(gc, d));

cb = emit_code(gc, cb, mov(gc, eax, d));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 0)));
	    if (one_light)
cb = emit_code(gc, cb, jle(gc, its_dark));
            else
cb = emit_code(gc, cb, jle(gc, no_diffuse));

	    if (!gc->modes.colorIndexMode) {
cb = emit_code(gc, cb, faddp(gc, st3, st0));
cb = emit_code(gc, cb, faddp(gc, st3, st0));
cb = emit_code(gc, cb, faddp(gc, st3, st0));
	    } else {
cb = emit_code(gc, cb, faddp(gc, st1, st0));
	    }

            if (1) {
	        struct celem* spec_ok = label(gc);
		__GLcoord *light_dir;

		if (xform_normal)
		    light_dir = &lsm->hHat;
		else
		    light_dir = &lsm->hHatModel;

cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&light_dir->x), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, nx));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&light_dir->y), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, ny));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fsub(gc, mem(gc, value(gc, (int)&msm->threshold), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&light_dir->z), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, nz));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
cb = emit_code(gc, cb, fstp(gc, d));
cb = emit_code(gc, cb, mov(gc, eax, d));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 0)));
cb = emit_code(gc, cb, jle(gc, next_light));

cb = emit_code(gc, cb, fld(gc, d));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&msm->scale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fadd(gc, fpk(gc, __glHalf)));
cb = emit_code(gc, cb, fistp(gc, d));
cb = emit_code(gc, cb, mov(gc, eax, d));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, __GL_SPEC_LOOKUP_TABLE_SIZE)));
cb = emit_code(gc, cb, jl(gc, spec_ok));
cb = emit_code(gc, cb, mov(gc, eax, value(gc, (__GL_SPEC_LOOKUP_TABLE_SIZE - 1))));
spec_ok->spec.label.where = cb; resolve(gc, spec_ok, cb);
		if (!gc->modes.colorIndexMode) {
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)msm->specTable), NULL, eax, 4)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&lspmm->specular.b), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)msm->specTable), NULL, eax, 4)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&lspmm->specular.g), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)msm->specTable), NULL, eax, 4)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&lspmm->specular.r), NULL, NULL, 1)));
cb = emit_code(gc, cb, faddp(gc, st3, st0));
cb = emit_code(gc, cb, faddp(gc, st3, st0));
cb = emit_code(gc, cb, faddp(gc, st3, st0));
		} else {
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)msm->specTable), NULL, eax, 4)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&lsm->sli), NULL, NULL, 1)));
cb = emit_code(gc, cb, faddp(gc, st2, st0));
		}
	    }
            if (!one_light) {
cb = emit_code(gc, cb, jmp(gc, next_light));

		/* The no-diffuse path leaves 3 items on top-of-stack */
no_diffuse->spec.label.where = cb; resolve(gc, no_diffuse, cb);
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fstp(gc, st0));

	    }
next_light->spec.label.where = cb; resolve(gc, next_light, cb);

            lsm = lsm->next;	/* process the next light, if any */
	  }

	  if (!gc->modes.colorIndexMode) {
	      union {
		float f;
		int i;
	      } u[3];

	      u[0].f = gc->frontBuffer.redScale;
	      u[1].f = gc->frontBuffer.greenScale;
	      u[2].f = gc->frontBuffer.blueScale;

cb = emit_code(gc, cb, fstp(gc, rr));
cb = emit_code(gc, cb, fstp(gc, gg));
cb = emit_code(gc, cb, fstp(gc, bb));

cb = emit_code(gc, cb, mov(gc, eax, rr));
cb = clamp(gc, cb, eax, value(gc, u[0].i));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].r), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, gg));
cb = clamp(gc, cb, eax, value(gc, u[1].i));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].g), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, bb));
cb = clamp(gc, cb, eax, value(gc, u[2].i));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].b), edi, NULL, 1), eax));

	      if (gc->polygon.shader.modeFlags & 
		  (__GL_SHADE_BLEND | __GL_SHADE_ALPHA_TEST)) {
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&msm->alpha), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].a), edi, NULL, 1), eax));
	      }
          } else {
		struct celem* ok = near_label(gc);
		__GLmaterialState *ms;

		ms = &gc->state.light.front;

cb = emit_code(gc, cb, fld(gc, fpk(gc, __glOne)));
cb = emit_code(gc, cb, fsub(gc, st2));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&msm->cmapd_minus_cmapa), NULL, NULL, 1)));
cb = emit_code(gc, cb, fmulp(gc, st1, st0));
cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, (int)&ms->cmapa), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&msm->cmaps_minus_cmapa), NULL, NULL, 1)));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
cb = emit_code(gc, cb, fstp(gc, rr));
cb = emit_code(gc, cb, mov(gc, eax, rr));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, (int)&ms->cmaps), NULL, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, eax, ecx));
cb = emit_code(gc, cb, jle(gc, ok));
cb = emit_code(gc, cb, mov(gc, eax, ecx));
ok->spec.label.where = cb; resolve(gc, ok, cb);
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].r), edi, NULL, 1), eax));
	  }
	}

    if (one_light)
    {
	struct celem *endif = label(gc);
	GLuint stack_depth;

cb = emit_code(gc, cb, jmp(gc, endif));

	/* There's only one light, and it's not illuminating this vertex.
	 * Forget the color we were hoping to accumulate, and write out
	 * the baseColor.
         */
its_dark->spec.label.where = cb; resolve(gc, its_dark, cb);

	if (gc->modes.colorIndexMode) {
	    __GLmaterialState *ms = &gc->state.light.front;
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&ms->cmapa), NULL, NULL, 1)));
	    for (stack_depth = 3; stack_depth; stack_depth--)
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].r), edi, NULL, 1), eax));
	} else {
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->light.baseColor.r), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].r), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->light.baseColor.g), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].g), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->light.baseColor.b), NULL, NULL, 1)));

            /* 2 writes pending, the third will block anyway, so now's
             * a good time to clear out the stack.
             */
            for (stack_depth = 6; stack_depth; stack_depth--)
cb = emit_code(gc, cb, fstp(gc, st0));

cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].b), edi, NULL, 1), eax));

            if (gc->polygon.shader.modeFlags & 
	        (__GL_SHADE_BLEND | __GL_SHADE_ALPHA_TEST)) {
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&msm->alpha), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].a), edi, NULL, 1), eax));
	    }
	}

endif->spec.label.where = cb; resolve(gc, endif, cb);
    }

	return cb;
}

#define IMPORTANT_MODEFLAGS (__GL_SHADE_TWOSIDED | \
			     __GL_SHADE_BLEND | \
			     __GL_SHADE_ALPHA_TEST | \
			     __GL_SHADE_SMOOTH | \
			     __GL_SHADE_CHEAP_FOG | \
			     __GL_SHADE_SMOOTH_LIGHT | \
			     __GL_SHADE_DEPTH_TEST | \
			     __GL_SHADE_TEXTURE | \
			     __GL_SHADE_TEXTURE_PERSP)

typedef GLsizei (*key_builder)(unsigned long* k,
			       __GLcontext *gc,
			       unsigned long flags,
			       GLuint vertex_size);

/* Build a variable length key that uniquely identifies
 * the front-end code we need.
 */
static GLsizei make_key(unsigned long* k,
			__GLcontext *gc,
			unsigned long flags,
			GLuint vertex_size)
{
    GLsizei i;
    GLuint needs;

    needs = (gc->vertex.faceNeeds[__GL_FRONTFACE] | gc->vertex.needs);

    i = 0;
    k[i++] = (unsigned long)flags;
    k[i++] = (unsigned long)gc->vertexArray.controlWord;
    k[i++] = (unsigned long)gc->vertexArray.vp_stride;
    k[i++] = (unsigned long)gc->transform.modelView;
    k[i++] = (unsigned long)gc->transform.modelView->mvp.matrixType;
    k[i++] = (unsigned long)vertex_size;
    k[i++] = (unsigned long)needs;
    k[i++] = (unsigned long)gc->state.enables.lights;
    k[i++] = (unsigned long)gc->state.enables.clipPlanes;
    k[i++] = (unsigned long)gc->vertexArray.varrayPtr;

    /* Things that matter if we are lighting */
    k[i++] = (unsigned long)gc->modes.colorIndexMode;
    k[i++] = (unsigned long)gc->polygon.shader.modeFlags;
    k[i++] = (unsigned long)gc->state.enables.general;
    k[i++] = (unsigned long)gc->light.sources;
    k[i++] = (unsigned long)((gc->state.polygon.frontMode != GL_FILL ||
			      gc->state.polygon.backMode != GL_FILL));
    {
	__GLVertArrayMachine *va = &gc->vertexArray;

	/* mask indicates active arrays */
	/* signature contains size & type info of all arrays */
	k[i++] = va->mask;
	k[i++] = va->signature;
	k[i++] = (unsigned long)va->vp_stride;
	k[i++] = (unsigned long)va->np_stride;
	if (gc->modes.colorIndexMode) {
	    k[i++] = (unsigned long)va->ip_stride;
	} else {
	    k[i++] = (unsigned long)va->cp_stride;
	}
	k[i++] = (unsigned long)va->tp_stride;
    }

    assert(i <= GEOM_OG_MAX_KEY_SIZE);

    return sizeof(k[0]) * i;
}

static struct task *
gimme_space(GLboolean *hit,
	    __GLcontext *gc,
	    unsigned long flags,
	    GLuint vertex_size,
	    key_builder make_key)
{
    __GLgeomOGData *god;	/* Geom OG Data */
    unsigned long key[GEOM_OG_MAX_KEY_SIZE];
    GLsizei keysize;
    GLuint i;

    god = &gc->geomOGdata;

    keysize = (*make_key)(key, gc, flags, vertex_size);

    if (god->last_hit &&
	(god->last_hit->keysize == keysize) &&
	(memcmp(god->last_hit->key, key, keysize) == 0)) {
	*hit = GL_TRUE;
	return god->last_hit;
    }

    for (i = 0; i < GEOM_OG_NUM_SLOTS; i++) {
	if ((god->slots[i].keysize == keysize) &&
	    (memcmp(god->slots[i].key, key, keysize) == 0)) {
	    *hit = GL_TRUE;
	    god->last_hit = &god->slots[i];
	    return &god->slots[i];
	}
    }

    *hit = GL_FALSE;
    i = rand() % GEOM_OG_NUM_SLOTS;
    god->slots[i].keysize = keysize;
    memcpy(god->slots[i].key, key, keysize);
    god->last_hit = &god->slots[i];

    return &god->slots[i];
}

/*
 * A little snippet of code we insert in the matrix sequence during
 * a latency.
 */
static unsigned char *set_esi(__GLcontext* gc,
			      unsigned char* cb,
			      GLuint needs)
{
cb = emit_code(gc, cb, push(gc, esi));
cb = emit_code(gc, cb, mov(gc, esi, value(gc, (gc->vertexArray.validateMask | (needs & (__GL_HAS_FRONT_COLOR | __GL_HAS_NORMAL))))));

    return cb;
}

/*
 * Given a matrix, generates code to push vertex at [edx] through it.
 * Stores [x,y,z] result in given locations, but leaves w stewing
 * on the floating-point top-of-stack.
 * vertex_size gives the size of the incoming vertex (i.e.
 * 2, 3, or 4D).  For the smaller sizes we assume z=0.0 and w=1.0
 * here.
 * During immediate-mode operation, __glProcessVertexCache sets
 * vertex_size to the the maximum size of any of the vertices
 * in the cache.
 */

static unsigned char* doMatrix(__GLcontext *gc,
			       unsigned char* cb, 
			       struct celem* outx,
			       struct celem* outy,
			       struct celem* outz,
			       GLenum matrixType,
			       GLfloat *mat,
			       GLuint vertex_size,
			       unsigned char* (*frag)(__GLcontext* gc,
						     unsigned char* cb,
						     GLuint needs),
			       GLuint parm1)
{
	struct celem
	    *x = mem2(gc, NULL, edx),
	    *y = mem2(gc, value(gc, 4), edx),
	    *z = mem2(gc, value(gc, 8), edx),
	    *w = mem2(gc, value(gc, 12), edx);

	/* Implied z and w for smaller vertices */
	switch (vertex_size) {
	case 2:
	    z = fpk(gc, 0.0f);
	case 3:
	    w = NULL;  /* We explicitly handle w throughout */
	}

cb = emit_code(gc, cb, mov(gc, ecx, value(gc, (int)mat)));

	switch (matrixType) {
	case __GL_MT_IS2DNR:
	case __GL_MT_IS2DNRSC:
	case __GL_MT_IDENTITY:
	    /* This corresponds to __glXForm{2,3,4}_2DNRW */
	    if (vertex_size == 2) {
cb = emit_code(gc, cb, fld(gc, x));
cb = emit_code(gc, cb, fmul(gc, MAT(0,0)));
cb = emit_code(gc, cb, fld(gc, y));
cb = emit_code(gc, cb, fmul(gc, MAT(1,1)));
cb = emit_code(gc, cb, fxch(gc, st1));
    	    	
cb = emit_code(gc, cb, fadd(gc, MAT(3,0)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fadd(gc, MAT(3,1)));
cb = emit_code(gc, cb, fxch(gc, st1));
    	    	
cb = emit_code(gc, cb, fstp(gc, outx));
    	    	
	    	if (frag)
cb = frag(gc, cb, parm1);
    	    	
cb = emit_code(gc, cb, fstp(gc, outy));
	    } else if (vertex_size == 3) {
cb = emit_code(gc, cb, fld(gc, x));
cb = emit_code(gc, cb, fmul(gc, MAT(0,0)));
cb = emit_code(gc, cb, fld(gc, y));
cb = emit_code(gc, cb, fmul(gc, MAT(1,1)));
cb = emit_code(gc, cb, fld(gc, z));
cb = emit_code(gc, cb, fmul(gc, MAT(2,2)));
cb = emit_code(gc, cb, fxch(gc, st2));
    	    	
cb = emit_code(gc, cb, fadd(gc, MAT(3,0)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fadd(gc, MAT(3,1)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fadd(gc, MAT(3,2)));
cb = emit_code(gc, cb, fxch(gc, st1));
    	    	
cb = emit_code(gc, cb, fstp(gc, outx));
    	    	
	    	if (frag)
cb = frag(gc, cb, parm1);
    	    	
cb = emit_code(gc, cb, fstp(gc, outz));
cb = emit_code(gc, cb, fstp(gc, outy));
	    } else {
cb = emit_code(gc, cb, fld(gc, x));
cb = emit_code(gc, cb, fmul(gc, MAT(0,0)));
cb = emit_code(gc, cb, fld(gc, y));
cb = emit_code(gc, cb, fmul(gc, MAT(1,1)));
cb = emit_code(gc, cb, fld(gc, z));
cb = emit_code(gc, cb, fmul(gc, MAT(2,2)));
cb = emit_code(gc, cb, fxch(gc, st2));
    	    	
cb = emit_code(gc, cb, fld(gc, w));
cb = emit_code(gc, cb, fmul(gc, MAT(3,0)));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
cb = emit_code(gc, cb, fld(gc, w));
cb = emit_code(gc, cb, fmul(gc, MAT(3,1)));
cb = emit_code(gc, cb, faddp(gc, st2, st0));
cb = emit_code(gc, cb, fld(gc, w));
cb = emit_code(gc, cb, fmul(gc, MAT(3,2)));
cb = emit_code(gc, cb, faddp(gc, st3, st0));
    	    	
cb = emit_code(gc, cb, fstp(gc, outx));
    	    	
	    	if (frag)
cb = frag(gc, cb, parm1);
    	    	
cb = emit_code(gc, cb, fstp(gc, outy));
cb = emit_code(gc, cb, fstp(gc, outz));
	    }
    	    
            /* Caller detects case where w is 1.0, and doesn't need it */
	    if (w)
cb = emit_code(gc, cb, fld(gc, w));
	    /* 18 cycles */
	    break;
	default:
cb = emit_code(gc, cb, fld(gc, x));
cb = emit_code(gc, cb, fmul(gc, MAT(0,0)));
cb = emit_code(gc, cb, fld(gc, y));
cb = emit_code(gc, cb, fmul(gc, MAT(1,0)));
cb = emit_code(gc, cb, fxch(gc, st1));
	    if (w == NULL) {
cb = emit_code(gc, cb, fadd(gc, MAT(3,0)));
	    } else {
cb = emit_code(gc, cb, fld(gc, w));
cb = emit_code(gc, cb, fmul(gc, MAT(3,0)));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
	    }
cb = emit_code(gc, cb, fld(gc, z));
cb = emit_code(gc, cb, fmul(gc, MAT(2,0)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, faddp(gc, st2, st0));

cb = emit_code(gc, cb, fld(gc, x));
cb = emit_code(gc, cb, fmul(gc, MAT(0,1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, faddp(gc, st2, st0));
cb = emit_code(gc, cb, fld(gc, y));
cb = emit_code(gc, cb, fmul(gc, MAT(1,1)));
cb = emit_code(gc, cb, fxch(gc, st1));
	    if (w == NULL) {
cb = emit_code(gc, cb, fadd(gc, MAT(3,1)));
	    } else {
cb = emit_code(gc, cb, fld(gc, w));
cb = emit_code(gc, cb, fmul(gc, MAT(3,1)));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
	    }
cb = emit_code(gc, cb, fld(gc, z));
cb = emit_code(gc, cb, fmul(gc, MAT(2,1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, faddp(gc, st2, st0));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, outx));
	    if (frag)
cb = frag(gc, cb, parm1);
cb = emit_code(gc, cb, faddp(gc, st1, st0));

cb = emit_code(gc, cb, fld(gc, x));
cb = emit_code(gc, cb, fmul(gc, MAT(0,2)));
cb = emit_code(gc, cb, fld(gc, y));
cb = emit_code(gc, cb, fmul(gc, MAT(1,2)));
cb = emit_code(gc, cb, fxch(gc, st1));
	    if (w == NULL) {
cb = emit_code(gc, cb, fadd(gc, MAT(3,2)));
	    } else {
cb = emit_code(gc, cb, fld(gc, w));
cb = emit_code(gc, cb, fmul(gc, MAT(3,2)));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
	    }
cb = emit_code(gc, cb, fld(gc, z));
cb = emit_code(gc, cb, fmul(gc, MAT(2,2)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, faddp(gc, st2, st0));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, outy));
cb = emit_code(gc, cb, faddp(gc, st1, st0));

cb = emit_code(gc, cb, fld(gc, x));
cb = emit_code(gc, cb, fmul(gc, MAT(0,3)));
cb = emit_code(gc, cb, fld(gc, y));
cb = emit_code(gc, cb, fmul(gc, MAT(1,3)));
cb = emit_code(gc, cb, fxch(gc, st1));
	    if (w == NULL) {
cb = emit_code(gc, cb, fadd(gc, MAT(3,3)));
	    } else {
cb = emit_code(gc, cb, fld(gc, w));
cb = emit_code(gc, cb, fmul(gc, MAT(3,3)));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
	    }
cb = emit_code(gc, cb, fld(gc, z));
cb = emit_code(gc, cb, fmul(gc, MAT(2,3)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, faddp(gc, st2, st0));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstp(gc, outz));
cb = emit_code(gc, cb, faddp(gc, st1, st0));
	    /* If __GL_MT_IS2D, the caller is not expecting w on the stack
	     * unless vertex_size == 4
	     */
	    if (w == NULL && matrixType == __GL_MT_IS2D) {
cb = emit_code(gc, cb, fstp(gc, st0));
	    }
	}

    return cb;
}

static GLuint what_needs(__GLcontext *gc)
{
    GLuint needs = gc->vertex.faceNeeds[__GL_FRONTFACE] | gc->vertex.needs;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    /* If we're doing twosided, disable lighting */
    if (needs & __GL_HAS_FRONT_COLOR) {
      GLboolean anySlow = GL_FALSE;
      __GLlightSourceMachine *lsm;

      for (lsm = gc->light.sources; lsm; lsm = lsm->next) {
	  if (lsm->slowPath) {
	      anySlow = GL_TRUE;
	      break;
	  }
      }

      if (anySlow ||
	  (modeFlags & __GL_SHADE_TWOSIDED) ||
	  !(gc->state.enables.general & __GL_LIGHTING_ENABLE) ||
	  (gc->state.enables.general & __GL_FOG_ENABLE) ||
	  (gc->transform.modelView->inverseTranspose.matrixType == __GL_MT_GENERAL) ||
	  (gc->state.enables.general & __GL_NORMALIZE_ENABLE))
	  needs = 0;
    }

    return needs;
}

void* GenerateCompile(__GLcontext *gc, GLuint vertex_size, GLuint flags)
{
  __GLtransform *tr = gc->transform.modelView;
  __GLviewport *viewport = &gc->state.viewport;
  GLuint modeFlags = gc->polygon.shader.modeFlags;
  GLboolean interpolate_z;
  GLboolean nocopy = (flags & __GL_GEOM_OG_NOCOPY) ? GL_TRUE : GL_FALSE;
  GLuint needs = what_needs(gc);
  GLuint np_stride, vp_stride;
  __GLVertArrayMachine *va = &gc->vertexArray;
  GLuint vaMask = va->compileMask;
  GLboolean current_color, current_texcoord, current_normal;
  GLuint alloc;
  GLboolean w_is_1;
  GLboolean z_is_c;

  struct celem* off;
  struct celem* offset;
  struct celem* first;
  struct celem* count;  

  float *work = (float*)(((int)gc->triSetupData + 7) & ~7);
  struct celem *fpcw;
  struct celem *d;
  struct celem *clipx, *clipy, *clipz, *clipw;
  struct celem *clip_z_flag;	/* Sometimes Z clip code is constant */
  struct celem *nx, *ny, *nz;
  struct celem *rr, *gg, *bb, *aa;
  struct celem *windoww;
  struct celem *np;
  struct celem *cp;
  struct celem *tp;
  struct celem *vp;
  struct celem *six;

  struct celem *lightsToModel;

  struct celem					        
    *compile,
    *culled,
    *culled2,
    *do_divide,
    *do_slow_clip_check,
    *done_clip_check,
    *skip_rest,
    *done;

  GLboolean hit;
  struct task *ptask;
  unsigned char *cb;

  interpolate_z = (modeFlags & __GL_SHADE_DEPTH_TEST &&
		   (gc->state.depth.writeEnable ||
		    gc->state.depth.testFunc != GL_ALWAYS));

  current_normal = !(va->compileMask & va->mask & VERTARRAY_N_MASK);
  current_color = !(va->compileMask & va->mask & VERTARRAY_C_MASK);
  current_texcoord = !(va->compileMask & va->mask & VERTARRAY_T_MASK);

  w_is_1 =
    (tr->mvp.matrixType >= __GL_MT_IS2D) &&
    (vertex_size < 4);
  z_is_c =
    (tr->mvp.matrixType >= __GL_MT_IS2D) &&
    (vertex_size < 3);

  if (!interpolate_z)
      flags |= __GL_GEOM_OG_NO_Z;

  ptask = gimme_space(&hit, gc, flags, vertex_size, make_key);
  if (hit) {
      gc->vertexArray.drawVertexes[GL_TRIANGLE_STRIP] = ptask->drawVertexes;
      return ptask->space;
  }

  cb = ptask->space;

  og_warmup(gc);

  /* Set up our parameters, given that we push 5 things
   * on to the stack, and that the return address is there
   * already.
   */
  off = value(gc, gc->vertexArray.vp_stride);
  offset = mem2(gc, value(gc, (6 + 1) * 4), esp);
  first = mem2(gc, value(gc, (6 + 2) * 4), esp); 
  count = mem2(gc, value(gc, (6 + 3) * 4), esp); 

  alloc = 0;

  fpcw = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  d = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  clipx = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  clipy = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  clipz = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  clipw = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  clip_z_flag = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  nx = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  ny = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  nz = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  rr = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  gg = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  bb = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  aa = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  windoww = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  vp = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  tp = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  cp = mem2(gc, value(gc, (int)&work[alloc++]), NULL);
  np = mem2(gc, value(gc, (int)&work[alloc++]), NULL);

  six = mem2(gc, value(gc, (int)&work[alloc]), NULL);

  compile = label(gc);
  done = label(gc);
  culled = label(gc);
  culled2 = label(gc);
  do_divide = label(gc);
  do_slow_clip_check = label(gc);
  done_clip_check = label(gc);
  skip_rest = label(gc);

  lightsToModel = label(gc);
  lightsToModel->spec.label.where = (unsigned char*)&__glLightsToModel;
  

/*
 * eax	
 * ebx
 * ecx
 * edx	vptr
 * ebp
 * esi  nptr, tptr, cptr
 * edi  outptr
 */

cb = emit_code(gc, cb, push(gc, esi));
cb = emit_code(gc, cb, push(gc, edi));
cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, push(gc, ebp));

	/* Ensure everything we write to is in the cache already */
cb = emit_code(gc, cb, mov(gc, eax, fpcw));
cb = emit_code(gc, cb, mov(gc, ebx, nx));
cb = emit_code(gc, cb, mov(gc, eax, vp));
cb = emit_code(gc, cb, mov(gc, ebx, np));
  {
    struct celem* no = label(gc);

cb = emit_code(gc, cb, xor(gc, eax, eax));
cb = emit_code(gc, cb, mov(gc, al, mem(gc, value(gc, (int)&tr->updateInverse), NULL, NULL, 1)));
cb = emit_code(gc, cb, test(gc, eax, eax));
cb = emit_code(gc, cb, je(gc, no));
cb = emit_code(gc, cb, push(gc, value(gc, (int)tr)));
cb = emit_code(gc, cb, push(gc, value(gc, (int)gc)));
cb = emit_code(gc, cb, call(gc, mem(gc, value(gc, (int)&gc->procs.computeInverseTranspose), NULL, NULL, 1)));
cb = emit_code(gc, cb, pop(gc, eax));
cb = emit_code(gc, cb, pop(gc, ebx));

no->spec.label.where = cb; resolve(gc, no, cb);

	if (gc->state.enables.general & __GL_LIGHTING_ENABLE) {
cb = emit_code(gc, cb, push(gc, value(gc, (int)gc)));
cb = emit_code(gc, cb, call(gc, lightsToModel));
cb = emit_code(gc, cb, pop(gc, eax));
	}
  }

        if (!w_is_1) {
	    /* Set the floating-point precision to 24-bit so
	     * we get a 19-cycle divide later.
	     */
cb = emit_code(gc, cb, fstcw(gc, fpcw));
cb = emit_code(gc, cb, mov(gc, edx, fpcw));
cb = emit_code(gc, cb, and(gc, dh, value(gc, ~3)));
cb = emit_code(gc, cb, mov(gc, d, edx));
cb = emit_code(gc, cb, fldcw(gc, d));
	}

        /* If z_is_c and w_is_1, then clip.z, clip.w and the Z clip codes are
	 * constant, and we can do them once here.
	 */
	if (z_is_c && w_is_1) {
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, (int)&(tr->mvp.matrix[3][2])), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, clipz, ecx));
cb = emit_code(gc, cb, mov(gc, eax, value(gc, FP_ONE)));

	    /* Generate the clip codes.  This code is taken from
	     * the main body of the loop.  It's explained in more detai
	     * there.
	     */
cb = emit_code(gc, cb, mov(gc, ebx, clipz));
    
cb = emit_code(gc, cb, mov(gc, ecx, ebx));
cb = emit_code(gc, cb, and(gc, ebx, value(gc, 0x7fffffff)));
    
cb = emit_code(gc, cb, sar(gc, ecx, value(gc, 31)));
cb = emit_code(gc, cb, cmp(gc, eax, ebx));
    
cb = emit_code(gc, cb, sbb(gc, ebx, ebx));
cb = emit_code(gc, cb, xor(gc, ecx, value(gc, __GL_CLIP_FAR)));
    
cb = emit_code(gc, cb, and(gc, ecx, value(gc, __GL_CLIP_FAR | __GL_CLIP_NEAR)));
    
cb = emit_code(gc, cb, and(gc, ebx, ecx));

cb = emit_code(gc, cb, mov(gc, clip_z_flag, ebx));
	}

        /* Initialise the pointers according to the
	 * current mode.
	 *
	 * edi	destination vertex
	 * esi  source normal
	 * edx  source position
	 *
	 * vp   vertex pointer
	 * np	normal pointer
	 * tp	texcoord pointer
	 * cp	color pointer
	 * 
	 * If the vertex or normal array has type GL_FLOAT, we don't
	 * use vp or np.  Instead, we have simpler code that uses register
	 * pointers esi and edx.
	 */
        if (nocopy) {
cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, (int)&gc->vertexArray.varrayPtr), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, eax, offset));
cb = emit_code(gc, cb, imul(gc, eax, value(gc, VX_SIZE)));
cb = emit_code(gc, cb, add(gc, edi, eax));

cb = emit_code(gc, cb, lea(gc, esi, mem(gc, value(gc, offsetof(normal, __GLvertex)), edi, NULL, 1)));
cb = emit_code(gc, cb, lea(gc, edx, mem(gc, value(gc, offsetof(obj, __GLvertex)), edi, NULL, 1)));
	    off = value(gc, VX_SIZE);
            vp_stride = VX_SIZE;
            np_stride = VX_SIZE;
	} else {
	    if ((vaMask & VERTARRAY_V_MASK) && !current_normal) {
cb = emit_code(gc, cb, mov(gc, eax, value(gc, gc->vertexArray.vp_stride)));
cb = emit_code(gc, cb, imul(gc, eax, first));
cb = emit_code(gc, cb, add(gc, eax, mem(gc, value(gc, (int)&gc->vertexArray.vertex_pointer), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, vp, eax));
	    }

	    if (vaMask & VERTARRAY_V_MASK) {
cb = emit_code(gc, cb, mov(gc, eax, value(gc, gc->vertexArray.vp_stride)));
cb = emit_code(gc, cb, imul(gc, eax, first));
cb = emit_code(gc, cb, add(gc, eax, mem(gc, value(gc, (int)&gc->vertexArray.vertex_pointer), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, vp, eax));
	    }

	    if ((vaMask & VERTARRAY_N_MASK) && !current_normal) {
cb = emit_code(gc, cb, mov(gc, eax, value(gc, gc->vertexArray.np_stride)));
cb = emit_code(gc, cb, imul(gc, eax, first));
cb = emit_code(gc, cb, add(gc, eax, mem(gc, value(gc, (int)&gc->vertexArray.normal_pointer), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, np, eax));
	    }

	    if ((vaMask & VERTARRAY_T_MASK) && !current_texcoord) {
cb = emit_code(gc, cb, mov(gc, eax, value(gc, gc->vertexArray.tp_stride)));
cb = emit_code(gc, cb, imul(gc, eax, first));
cb = emit_code(gc, cb, add(gc, eax, mem(gc, value(gc, (int)&gc->vertexArray.tex_coord_pointer), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, tp, eax));
	    }

	    if ((vaMask & (VERTARRAY_C_MASK | VERTARRAY_I_MASK)) &&
		!current_color) {
cb = emit_code(gc, cb, mov(gc, eax, value(gc, gc->vertexArray.cp_stride)));
cb = emit_code(gc, cb, imul(gc, eax, first));
cb = emit_code(gc, cb, add(gc, eax, mem(gc, value(gc, (int)&gc->vertexArray.color_pointer), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, cp, eax));
	    }

cb = emit_code(gc, cb, mov(gc, esi, value(gc, gc->vertexArray.np_stride)));
cb = emit_code(gc, cb, imul(gc, esi, first));
cb = emit_code(gc, cb, add(gc, esi, mem(gc, value(gc, (int)&gc->vertexArray.normal_pointer), NULL, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, edx, value(gc, gc->vertexArray.vp_stride)));
cb = emit_code(gc, cb, imul(gc, edx, first));
cb = emit_code(gc, cb, add(gc, edx, mem(gc, value(gc, (int)&gc->vertexArray.vertex_pointer), NULL, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, edi, mem(gc, value(gc, (int)&gc->vertexArray.varrayPtr), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, eax, offset));
cb = emit_code(gc, cb, imul(gc, eax, value(gc, VX_SIZE)));
cb = emit_code(gc, cb, add(gc, edi, eax));

            vp_stride = gc->vertexArray.vp_stride;
            np_stride = gc->vertexArray.np_stride;
	}

cb = emit_code(gc, cb, mov(gc, ebp, count));

compile->spec.label.where = cb; resolve(gc, compile, cb);
        if (!nocopy) {
	    if ((vaMask & VERTARRAY_V_MASK) && 
		(gc->vertexArray.vp_type != GL_FLOAT)) {
cb = emit_code(gc, cb, mov(gc, eax, vp));
cb = CopyCoord(gc, cb, gc->vertexArray.vp_type, gc->vertexArray.vp_size, mem(gc, vx(obj.x), edi, NULL, 1), mem(gc, vx(obj.y), edi, NULL, 1), mem(gc, vx(obj.z), edi, NULL, 1), mem(gc, vx(obj.w), edi, NULL, 1));
cb = emit_code(gc, cb, add(gc, eax, value(gc, gc->vertexArray.vp_stride)));
cb = emit_code(gc, cb, mov(gc, vp, eax));
cb = emit_code(gc, cb, lea(gc, edx, mem(gc, vx(obj.x), edi, NULL, 1)));
	    }
	    if ((vaMask & VERTARRAY_N_MASK) && 
		(gc->vertexArray.np_type != GL_FLOAT)) {
		if (current_normal) {
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.normal.x), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.x), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.normal.y), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.y), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.normal.z), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.z), edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, mov(gc, eax, np));
cb = CopyNormal(gc, cb, gc->vertexArray.np_type, six);
cb = emit_code(gc, cb, add(gc, eax, value(gc, gc->vertexArray.np_stride)));
cb = emit_code(gc, cb, mov(gc, np, eax));
		}
cb = emit_code(gc, cb, lea(gc, esi, mem(gc, vx(normal.x), edi, NULL, 1)));
	    }
	    if (vaMask & VERTARRAY_T_MASK) {
		if (current_texcoord) {
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.texture.x), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(texture.x), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.texture.y), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(texture.y), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.texture.z), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(texture.z), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.texture.w), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(texture.w), edi, NULL, 1), eax));
		} else {
cb = emit_code(gc, cb, mov(gc, eax, tp));
cb = CopyCoord(gc, cb, gc->vertexArray.tp_type, gc->vertexArray.tp_size, mem(gc, vx(texture.x), edi, NULL, 1), mem(gc, vx(texture.y), edi, NULL, 1), mem(gc, vx(texture.z), edi, NULL, 1), mem(gc, vx(texture.w), edi, NULL, 1));
cb = emit_code(gc, cb, add(gc, eax, value(gc, gc->vertexArray.tp_stride)));
cb = emit_code(gc, cb, mov(gc, tp, eax));
		}
	    }
	    if (vaMask & VERTARRAY_C_MASK) {
		if (current_color) {
		    if (gc->modes.colorIndexMode) {
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.userColorIndex), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].r), edi, NULL, 1), eax));
		    } else {
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.color.r), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].r), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.color.g), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].g), edi, NULL, 1), eax));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.color.b), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].b), edi, NULL, 1), eax));
        	        if (gc->polygon.shader.modeFlags & 
			    (__GL_SHADE_BLEND | __GL_SHADE_ALPHA_TEST)) {
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->state.current.color.a), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(colors[0].a), edi, NULL, 1), eax));
		        }
		    }
        	} else {
cb = emit_code(gc, cb, mov(gc, eax, cp));
cb = CopyColor(gc, cb, gc->vertexArray.cp_type, gc->vertexArray.cp_size, six, rr, gg, bb, aa);
		}
		if (!current_color) {
cb = emit_code(gc, cb, add(gc, eax, value(gc, gc->vertexArray.cp_stride)));
cb = emit_code(gc, cb, mov(gc, cp, eax));
		}
	    }
	}
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
	    if (!(gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL)) {
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&(gc->transform.cullEye.x)), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&(gc->transform.cullEye.y)), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&(gc->transform.cullEye.z)), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st2));
	    } else {
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&(gc->transform.cullEye.z)), NULL, NULL, 1)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, value(gc, 8), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&(gc->transform.cullEye.y)), NULL, NULL, 1)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, value(gc, 4), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, (int)&(gc->transform.cullEye.x)), NULL, NULL, 1)));
cb = emit_code(gc, cb, fsub(gc, mem(gc, value(gc, 0), edx, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st2));

cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, fmulp(gc, st2, st0));
cb = emit_code(gc, cb, fld(gc, mem(gc, NULL, esi, NULL, 1)));
cb = emit_code(gc, cb, fmulp(gc, st3, st0));
	    }

cb = emit_code(gc, cb, faddp(gc, st1, st0));

cb = emit_code(gc, cb, faddp(gc, st1, st0));

cb = emit_code(gc, cb, fstp(gc, d));

cb = emit_code(gc, cb, mov(gc, eax, d));

cb = emit_code(gc, cb, dec(gc, eax));

cb = emit_code(gc, cb, shr(gc, eax, value(gc, 31)));

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, (int)&gc->vertexArray.cullCode), NULL, eax, 4)));

cb = emit_code(gc, cb, test(gc, eax, eax));
cb = emit_code(gc, cb, jne(gc, culled));
	}

	/* Not culled: Transform and clipcheck */
cb = doMatrix(gc, cb, clipx, clipy, clipz, tr->mvp.matrixType, (float*)(tr->mvp.matrix), vertex_size, set_esi, needs);

/************************************************************************/

	if (!w_is_1) {
cb = emit_code(gc, cb, fst(gc, clipw));
	} else {
            clipw = value(gc, FP_ONE);
	}

	/* Now we have clip[x,y,z,w] calculated, start the perspective
	 * divide and then (if w is positive) do the clip-check in
	 * branchless integer.
	 */

cb = emit_code(gc, cb, mov(gc, eax, clipw));
cb = emit_code(gc, cb, mov(gc, ebx, clipx));

	if (!w_is_1) {
cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));

cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 0)));
cb = emit_code(gc, cb, jle(gc, do_slow_clip_check));
	}

	/* The sequence of operations for the clip-check:
	 *     
	 */

	if (z_is_c) {
cb = emit_code(gc, cb, mov(gc, ecx, clipy));
cb = emit_code(gc, cb, or(gc, ecx, ebx));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, 0x7fffffff)));
cb = emit_code(gc, cb, cmp(gc, ecx, value(gc, FP_ONE)));
cb = emit_code(gc, cb, jl(gc, done_clip_check));
       	}

cb = emit_code(gc, cb, mov(gc, ecx, ebx));
cb = emit_code(gc, cb, and(gc, ebx, value(gc, 0x7fffffff)));

cb = emit_code(gc, cb, sar(gc, ecx, value(gc, 31)));
cb = emit_code(gc, cb, cmp(gc, eax, ebx));

cb = emit_code(gc, cb, sbb(gc, ebx, ebx));
cb = emit_code(gc, cb, xor(gc, ecx, value(gc, __GL_CLIP_RIGHT)));

cb = emit_code(gc, cb, and(gc, ecx, value(gc, __GL_CLIP_RIGHT | __GL_CLIP_LEFT)));
				/* ecx is __GL_CLIP_RIGHT if x was +ve,
				 * or __GL_CLIP_LEFT if x was -ve...
				 */
cb = emit_code(gc, cb, and(gc, ebx, ecx));

cb = emit_code(gc, cb, or(gc, esi, ebx));

				/* Similarly for y and z */
cb = emit_code(gc, cb, mov(gc, ebx, clipy));

cb = emit_code(gc, cb, mov(gc, ecx, ebx));
cb = emit_code(gc, cb, and(gc, ebx, value(gc, 0x7fffffff)));

cb = emit_code(gc, cb, sar(gc, ecx, value(gc, 31)));
cb = emit_code(gc, cb, cmp(gc, eax, ebx));

cb = emit_code(gc, cb, sbb(gc, ebx, ebx));
cb = emit_code(gc, cb, xor(gc, ecx, value(gc, __GL_CLIP_TOP)));

cb = emit_code(gc, cb, and(gc, ecx, value(gc, __GL_CLIP_TOP | __GL_CLIP_BOTTOM)));

cb = emit_code(gc, cb, and(gc, ebx, ecx));

cb = emit_code(gc, cb, or(gc, esi, ebx));

	if (!z_is_c) {
cb = emit_code(gc, cb, mov(gc, ebx, clipz));
    
cb = emit_code(gc, cb, mov(gc, ecx, ebx));
cb = emit_code(gc, cb, and(gc, ebx, value(gc, 0x7fffffff)));
    
cb = emit_code(gc, cb, sar(gc, ecx, value(gc, 31)));
cb = emit_code(gc, cb, cmp(gc, eax, ebx));
    
cb = emit_code(gc, cb, sbb(gc, ebx, ebx));
cb = emit_code(gc, cb, xor(gc, ecx, value(gc, __GL_CLIP_FAR)));
    
cb = emit_code(gc, cb, and(gc, ecx, value(gc, __GL_CLIP_FAR | __GL_CLIP_NEAR)));
    
cb = emit_code(gc, cb, and(gc, ebx, ecx));
    
cb = emit_code(gc, cb, or(gc, esi, ebx));
	} else {
cb = emit_code(gc, cb, mov(gc, ebx, clip_z_flag));
cb = emit_code(gc, cb, or(gc, esi, ebx));
	}

done_clip_check->spec.label.where = cb; resolve(gc, done_clip_check, cb);

        if (gc->state.enables.clipPlanes) {
	    GLuint bit, clipPlanesMask;
	    __GLcoord *plane;

	    /* Transform to eye coords */
cb = emit_code(gc, cb, lea(gc, ebx, six));
cb = doMatrix(gc, cb, mem(gc, NULL, ebx, NULL, 1), mem(gc, value(gc, 4), ebx, NULL, 1), mem(gc, value(gc, 8), ebx, NULL, 1), __GL_MT_GENERAL, (float*)(tr->matrix.matrix), vertex_size, NULL, needs);

cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), ebx, NULL, 1)));

	    /* The following code is taken from __glClipCheckAll() */
	    clipPlanesMask = gc->state.enables.clipPlanes;

	    plane = &gc->state.transform.eyeClipPlanes[0];
	    bit = __GL_CLIP_USER0;
	    while (clipPlanesMask) {
		if (clipPlanesMask & 1) {
		    /*
		    ** Dot the vertex clip coordinate against the clip plane and see
		    ** if the sign is negative.  If so, then the point is out.
		    */
cb = emit_code(gc, cb, fld(gc, mem(gc, NULL, ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&plane->x), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 4), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&plane->y), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 8), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&plane->z), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, 12), ebx, NULL, 1)));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&plane->w), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, faddp(gc, st2, st0));
cb = emit_code(gc, cb, faddp(gc, st2, st0));

cb = emit_code(gc, cb, faddp(gc, st1, st0));

cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 16), ebx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, 16), ebx, NULL, 1)));
cb = emit_code(gc, cb, sar(gc, eax, value(gc, 31)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, bit)));
cb = emit_code(gc, cb, or(gc, esi, eax));
		}
		clipPlanesMask >>= 1;
		bit <<= 1;
		plane++;
	    }
	}

	if (!w_is_1) {
	    /* 19 cycles for the divide to run, + 1 cycle for the fp result
	     * to percolate through.  We're ready to write window.w
	     */
            if ((modeFlags & __GL_SHADE_TEXTURE) &&
		(modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = emit_code(gc, cb, fst(gc, windoww));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.w), edi, NULL, 1)));
	    } else {
cb = emit_code(gc, cb, fstp(gc, windoww));
	    }
	} else {
            if ((modeFlags & __GL_SHADE_TEXTURE) &&
		(modeFlags & __GL_SHADE_TEXTURE_PERSP)) {
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(window.w), edi, NULL, 1), value(gc, FP_ONE)));
	    }
	}

	if (nocopy)
cb = emit_code(gc, cb, or(gc, mem(gc, vx(hasAndClipCode), edi, NULL, 1), esi));
	else
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(hasAndClipCode), edi, NULL, 1), esi));
cb = emit_code(gc, cb, mov(gc, eax, esi));

	if (needs & __GL_HAS_FRONT_COLOR) {
	    if (flags & __GL_GEOM_OG_NOCOPY) {
cb = emit_code(gc, cb, lea(gc, esi, six));
    
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, vx(normal.x), edi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, esi, NULL, 1), ecx));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, vx(normal.y), edi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 4), esi, NULL, 1), ecx));
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, vx(normal.z), edi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 8), esi, NULL, 1), ecx));

cb = emit_code(gc, cb, push(gc, eax));
cb = do_light(gc, cb, nx, ny, nz, rr, gg, bb, d);
cb = emit_code(gc, cb, pop(gc, eax));
cb = emit_code(gc, cb, pop(gc, esi));
	    } else {
cb = emit_code(gc, cb, pop(gc, esi));
cb = emit_code(gc, cb, mov(gc, ebx, eax));
cb = do_light(gc, cb, nx, ny, nz, rr, gg, bb, d);
cb = emit_code(gc, cb, mov(gc, eax, ebx));
	    }
	} else {
cb = emit_code(gc, cb, pop(gc, esi));

	    /*
	     * If we're doing any lighting at all, then we're going to
	     * need at least the normal later on.  Maybe the position,
	     * too.  Fogging also needs the position in order to
	     * calculate eye coords later.
	     */
	    if (gc->state.enables.general & __GL_LIGHTING_ENABLE) {
		if (current_normal && !nocopy) {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, (int)&gc->state.current.normal.x), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.x), edi, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, (int)&gc->state.current.normal.y), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.y), edi, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, (int)&gc->state.current.normal.z), NULL, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.z), edi, NULL, 1), ebx));
		} else {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 0), esi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.x), edi, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 4), esi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.y), edi, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 8), esi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(normal.z), edi, NULL, 1), ebx));
		}
	    }

	    if ((gc->state.enables.general & __GL_LIGHTING_ENABLE) ||
		(gc->state.enables.general & __GL_FOG_ENABLE)) {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 0), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(obj.x), edi, NULL, 1), ebx));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 4), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(obj.y), edi, NULL, 1), ebx));
                if (vertex_size <= 2) {
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(obj.z), edi, NULL, 1), value(gc, FP_ZERO)));
                } else {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 8), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(obj.z), edi, NULL, 1), ebx));
		}
                if (vertex_size <= 3) {
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(obj.w), edi, NULL, 1), value(gc, FP_ONE)));
                } else {
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 12), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(obj.w), edi, NULL, 1), ebx));
		}
	    }
	}

cb = emit_code(gc, cb, test(gc, eax, value(gc, __GL_ALL_CLIP_MASK)));
cb = emit_code(gc, cb, jne(gc, culled2));

        /* Don't bother calculating window.z if the rasterizer isn't
	 * Z-buffering.
	 */
	if (interpolate_z) {
cb = emit_code(gc, cb, fld(gc, clipx));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&viewport->xScale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, clipy));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&viewport->yScale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, clipz));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&viewport->zScale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st2));

	    if (!w_is_1) {
cb = emit_code(gc, cb, fmul(gc, windoww));
cb = emit_code(gc, cb, fld(gc, windoww));
cb = emit_code(gc, cb, fmulp(gc, st2, st0));
cb = emit_code(gc, cb, fld(gc, windoww));
cb = emit_code(gc, cb, fmulp(gc, st3, st0));
	    }

cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, (int)&viewport->xCenter), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, (int)&viewport->yCenter), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, (int)&viewport->zCenter), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.x), edi, NULL, 1)));

cb = emit_code(gc, cb, add(gc, edx, off));

cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.z), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.y), edi, NULL, 1)));

	} else {

cb = emit_code(gc, cb, fld(gc, clipx));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&viewport->xScale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, clipy));
cb = emit_code(gc, cb, fmul(gc, mem(gc, value(gc, (int)&viewport->yScale), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));

	    if (!w_is_1) {
cb = emit_code(gc, cb, fmul(gc, windoww));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, fmul(gc, windoww));
cb = emit_code(gc, cb, fxch(gc, st1));
	    }

cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, (int)&viewport->xCenter), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, (int)&viewport->yCenter), NULL, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, add(gc, edx, value(gc, vp_stride)));

cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.x), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, vx(window.y), edi, NULL, 1)));
	}

cb = emit_code(gc, cb, add(gc, esi, value(gc, np_stride)));
cb = emit_code(gc, cb, add(gc, edi, value(gc, VX_SIZE)));

cb = emit_code(gc, cb, dec(gc, ebp));
cb = emit_code(gc, cb, jne(gc, compile));

cb = emit_code(gc, cb, jmp(gc, done));

culled->spec.label.where = cb; resolve(gc, culled, cb);
cb = emit_code(gc, cb, or(gc, eax, value(gc, gc->vertexArray.validateMask)));
	if (nocopy)
cb = emit_code(gc, cb, or(gc, mem(gc, vx(hasAndClipCode), edi, NULL, 1), eax));
	else
cb = emit_code(gc, cb, mov(gc, mem(gc, vx(hasAndClipCode), edi, NULL, 1), eax));

culled2->spec.label.where = cb; resolve(gc, culled2, cb);
cb = emit_code(gc, cb, add(gc, edx, off));
cb = emit_code(gc, cb, add(gc, esi, off));

cb = emit_code(gc, cb, add(gc, edi, value(gc, VX_SIZE)));

cb = emit_code(gc, cb, dec(gc, ebp));
cb = emit_code(gc, cb, jne(gc, compile));

done->spec.label.where = cb; resolve(gc, done, cb);

     if (!w_is_1) {
cb = emit_code(gc, cb, fldcw(gc, fpcw));
     }

cb = emit_code(gc, cb, pop(gc, ebp));
cb = emit_code(gc, cb, pop(gc, ecx));
cb = emit_code(gc, cb, pop(gc, ebx));
cb = emit_code(gc, cb, pop(gc, edi));
cb = emit_code(gc, cb, pop(gc, esi));

cb = emit_code(gc, cb, ret(gc));

     if (!w_is_1) {
/* 
 * Slow case clip-code generation
 *
 * if ( x < negW ) code |= __GL_CLIP_LEFT;
 * if (x > w) code |= __GL_CLIP_RIGHT;
 *
 * becomes:
 *
 * if ( (x - negw) < 0) code |= __GL_CLIP_LEFT;
 * if ( (w - x) < 0) code |= __GL_CLIP_RIGHT;
 *
 * becomes:
 *
 * if ( (x + w) < 0) code |= __GL_CLIP_LEFT;
 * if ( (w - x) < 0) code |= __GL_CLIP_RIGHT;
 */

do_slow_clip_check->spec.label.where = cb; resolve(gc, do_slow_clip_check, cb);
cb = emit_code(gc, cb, fld(gc, clipx));
cb = emit_code(gc, cb, fadd(gc, clipw));
cb = emit_code(gc, cb, fld(gc, clipw));
cb = emit_code(gc, cb, fsub(gc, clipx));

cb = emit_code(gc, cb, fld(gc, clipy));
cb = emit_code(gc, cb, fadd(gc, clipw));
cb = emit_code(gc, cb, fld(gc, clipw));
cb = emit_code(gc, cb, fsub(gc, clipy));

cb = emit_code(gc, cb, fld(gc, clipz));
cb = emit_code(gc, cb, fadd(gc, clipw));
cb = emit_code(gc, cb, fld(gc, clipw));
cb = emit_code(gc, cb, fsub(gc, clipz));
cb = emit_code(gc, cb, fxch(gc, st5));

cb = emit_code(gc, cb, push(gc, edi));
cb = emit_code(gc, cb, lea(gc, edi, six));

cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 0), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 4), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 8), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 12), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 16), edi, NULL, 1)));
cb = emit_code(gc, cb, fstp(gc, mem(gc, value(gc, 20), edi, NULL, 1)));

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, 0), edi, NULL, 1)));

cb = emit_code(gc, cb, sar(gc, eax, value(gc, 31)));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 4), edi, NULL, 1)));

cb = emit_code(gc, cb, sar(gc, ebx, value(gc, 31)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, __GL_CLIP_LEFT)));

cb = emit_code(gc, cb, or(gc, esi, eax));
cb = emit_code(gc, cb, and(gc, ebx, value(gc, __GL_CLIP_NEAR)));

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, 8), edi, NULL, 1)));
cb = emit_code(gc, cb, or(gc, esi, ebx));

cb = emit_code(gc, cb, sar(gc, eax, value(gc, 31)));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 12), edi, NULL, 1)));

cb = emit_code(gc, cb, sar(gc, ebx, value(gc, 31)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, __GL_CLIP_TOP)));

cb = emit_code(gc, cb, or(gc, esi, eax));
cb = emit_code(gc, cb, and(gc, ebx, value(gc, __GL_CLIP_BOTTOM)));

cb = emit_code(gc, cb, or(gc, esi, ebx));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, 16), edi, NULL, 1)));

cb = emit_code(gc, cb, sar(gc, eax, value(gc, 31)));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, 20), edi, NULL, 1)));

cb = emit_code(gc, cb, sar(gc, ebx, value(gc, 31)));
cb = emit_code(gc, cb, and(gc, eax, value(gc, __GL_CLIP_RIGHT)));

cb = emit_code(gc, cb, or(gc, esi, eax));
cb = emit_code(gc, cb, and(gc, ebx, value(gc, __GL_CLIP_FAR)));

cb = emit_code(gc, cb, or(gc, esi, ebx));

cb = emit_code(gc, cb, pop(gc, edi));
cb = emit_code(gc, cb, jmp(gc, done_clip_check));
	}

    gc->vertexArray.drawVertexes[GL_TRIANGLE_STRIP] = (void*) cb;
    ptask->drawVertexes = cb;
    cb = BuildDrawVertexes(cb, gc);

    return ptask->space;
}

void __glPickVertexShape(__GLcontext *gc)
{
    GLuint needs = what_needs(gc);

    if (!dynamic_vertex())
	gc->vx_ns.direct_map = GL_TRUE;

    /* XXXshui is the dynamic vertex stuff good for anything? */
    return;
    
    __glNSOpenSpace(&gc->vx_ns);

#define VXMENTION(i)								\
  __glNSWhereIs(&gc->vx_ns, offsetof(i, __GLvertex), sizeof(GLint));

    VXMENTION(hasAndClipCode);
    VXMENTION(window.x);
    VXMENTION(window.y);

    if (!(needs & __GL_HAS_FRONT_COLOR)) {
	VXMENTION(normal.x);
	VXMENTION(normal.y);
	VXMENTION(normal.z);

	VXMENTION(obj.x);
	VXMENTION(obj.y);
	VXMENTION(obj.z);
	VXMENTION(obj.w);
    }

    if (0) {
        VXMENTION(window.w);
    }

    if (0) {
	VXMENTION(color);
    }

    VXMENTION(colors[0].r);
    
    if (!gc->modes.colorIndexMode) {
	VXMENTION(colors[0].g);
	VXMENTION(colors[0].b);
    }

    if (0) {
	VXMENTION(colors[0].a);
    }

    __glNSCloseSpace(&gc->vx_ns);

}

#endif

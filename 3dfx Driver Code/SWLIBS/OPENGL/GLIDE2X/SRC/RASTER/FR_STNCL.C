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
#include "render.h"
#include "global.h"
#include "fr_tri.h"
#include "fr_modes.h"

void __fastcall __glFRStencilTest(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLuint smask = gc->state.stencil.mask;
    GLbitfield outMask = 0, sfail, bit = HIBIT;
    GLbitfield maskPreStencil = mask;
    GLubyte ref = gc->state.stencil.reference & smask;
    GLubyte *sp = tr->sp;

    while (mask) {
	if (((int)mask) < 0) {

	    switch (gc->state.stencil.testFunc) {
	    case GL_NEVER:
		break;
	    case GL_LESS:
		if (ref < (*sp & smask))
		    outMask |= bit;
		break;
	    case GL_EQUAL:
		if (ref == (*sp & smask))
		    outMask |= bit;
		break;
	    case GL_LEQUAL:
		if (ref <= (*sp & smask))
		    outMask |= bit;
		break;
	    case GL_GREATER:
		if (ref > (*sp & smask))
		    outMask |= bit;
		break;
	    case GL_NOTEQUAL:
		if (ref != (*sp & smask))
		    outMask |= bit;
		break;
	    case GL_GEQUAL:
		if (ref >= (*sp & smask))
		    outMask |= bit;
		break;
	    case GL_ALWAYS:
		outMask |= bit;
		break;
	    }
	}

	bit >>= 1;
	mask <<= 1;
	sp += tr->dx;
    }

    tr->maskPostStencil = outMask;

    
    /* Now we have enough to process OpSFail, comparing the incoming
     * and final mask.  If depth is disabled, we can also process the
     * final mask as OpZPass.  Otherwise we must call depth test next,
     * followed by a post-depth stencil cleanup phase, which processes
     * OpZFail and OpZPass and continues down the pipeline.
     */
    sfail = maskPreStencil - outMask;
    if (sfail)
	(*gc->procs.stencilOpSFail)(maskPreStencil - outMask, tr);

    if (outMask)
	(*gc->procs.afterStencilTest)(outMask, tr);
}

void __fastcall __glFRStencilPostDepthCleanup(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLbitfield zfail = tr->maskPostStencil - mask;

    if (zfail)
	(gc->procs.stencilOpZFail)(tr->maskPostStencil - mask, tr);

    if (mask) {
	(gc->procs.stencilOpZPass)(mask, tr);
	(gc->procs.afterStencilDepthTest)(mask, tr);
    }
}

void __fastcall __glFRStencilKeep(GLbitfield mask, __GLtri *tr)
{
    /* nop */
}

void __fastcall __glFRStencilZero(GLbitfield mask, __GLtri *tr)
{
    GLubyte *sp = tr->sp;
    GLubyte srcMask = tr->gc->state.stencil.writeMask;
    GLubyte dstMask = ~srcMask;

    while (mask) {
	if (((int)mask) < 0) {
	    *sp &= dstMask;
	}
	sp += tr->dx;
	mask <<= 1;
    }
}

void __fastcall __glFRStencilReplace(GLbitfield mask, __GLtri *tr)
{
    GLubyte *sp = tr->sp;
    GLubyte srcMask = tr->gc->state.stencil.writeMask;
    GLubyte dstMask = ~srcMask;
    GLubyte value = srcMask & tr->gc->state.stencil.reference;

    while (mask) {
	if (((int)mask) < 0) {
	    *sp = (dstMask & *sp) | value;
	}
	sp += tr->dx;
	mask <<= 1;
    }
}

void __fastcall __glFRStencilIncrement(GLbitfield mask, __GLtri *tr)
{
    GLubyte *sp = tr->sp;
    GLubyte srcMask = tr->gc->state.stencil.writeMask;
    GLubyte dstMask = ~srcMask;
    GLubyte value;

    while (mask) {
	if (((int)mask) < 0) {
	    value = *sp;
	    if (value < 255) {
		*sp = (value & dstMask) | ((value+1) & srcMask);
	    }
	}
	sp += tr->dx;
	mask <<= 1;
    }
}

void __fastcall __glFRStencilDecrement(GLbitfield mask, __GLtri *tr)
{
    GLubyte *sp = tr->sp;
    GLubyte srcMask = tr->gc->state.stencil.writeMask;
    GLubyte dstMask = ~srcMask;
    GLubyte value;

    while (mask) {
	if (((int)mask) < 0) {
	    value = *sp;
	    if (value > 0) {
		*sp = (value & dstMask) | ((value-1) & srcMask);
	    }
	}
	sp += tr->dx;
	mask <<= 1;
    }
}

void __fastcall __glFRStencilInvert(GLbitfield mask, __GLtri *tr)
{
    GLubyte *sp = tr->sp;
    GLubyte srcMask = tr->gc->state.stencil.writeMask;
    GLubyte dstMask = ~srcMask;
    GLubyte value;

    while (mask) {
	if (((int)mask) < 0) {
	    value = *sp;
	    *sp = (dstMask & value) | (srcMask & ~value);
	}
	sp += tr->dx;
	mask <<= 1;
    }
}

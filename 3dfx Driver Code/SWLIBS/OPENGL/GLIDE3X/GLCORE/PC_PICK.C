/*
** Copyright 1991-1997, Silicon Graphics, Inc.
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
*/
#include "render.h"
#include "context.h"
#include "global.h"
#include "pc_lnspan.h"

#define UNOPTIMIZED_LINE_MODES \
	( __GL_SHADE_LOGICOP |\
	  __GL_SHADE_STENCIL_TEST |\
	  __GL_SHADE_ALPHA_TEST |\
	  __GL_SHADE_INDEX_TEST |\
	  __GL_SHADE_TEXTURE |\
	  __GL_SHADE_SLOW_FOG |\
	  0 )

GLboolean __glPCPickLineProcs(__GLcontext *gc)
{
    GLuint modeFlags;
    __GLspanFunc *sp;
    __GLstippledSpanFunc *ssp;
    int spanCount;
    GLboolean wideLine;
    GLboolean replicateLine;

#if __GL_DISABLE_RASTER
    if (gc->vertexArray.controlWord & VERTARRAY_CW_DISABLE_RASTER) {
	gc->procs.renderLine = __glDontRenderLine;
	return GL_TRUE;
    }
#endif

    modeFlags = gc->polygon.shader.modeFlags;

    if (modeFlags & UNOPTIMIZED_LINE_MODES)
	return GL_FALSE;

    if (GL_RENDER != gc->renderMode)
	return GL_FALSE;

    if (0 != gc->drawBuffer->destMask)
	return GL_FALSE;

    /* Support only 8-bit CI or 16-bit RGB */
    if (gc->modes.colorIndexMode) {
	if (1 != gc->drawBuffer->buf.elementSize)
	    return GL_FALSE;
    } else {
	if (2 != gc->drawBuffer->buf.elementSize)
	    return GL_FALSE;
    }

    replicateLine = wideLine = GL_FALSE;

    if (gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE) {
	/* Anti-aliased lines */

#define REQUIRED_MODES (__GL_SHADE_RGB|__GL_SHADE_BLEND)
	/* All require modes must be set for this path */
	if (REQUIRED_MODES != (modeFlags & REQUIRED_MODES))
	    return GL_FALSE;
#undef  REQUIRED_MODES

	if (modeFlags & __GL_SHADE_DEPTH_TEST) {

	    /* Flat shaded only! */
	    if (modeFlags & __GL_SHADE_SMOOTH)
		return GL_FALSE;

	    if (GL_SRC_ALPHA != gc->state.raster.blendSrc ||
		GL_ONE_MINUS_SRC_ALPHA != gc->state.raster.blendDst)
		return GL_FALSE;

	    if(gc->state.depth.testFunc == GL_NEVER ) {
		/* Unexpected end of line routine picking! */
		gc->procs.line.processLine = (__GLspanFunc) __glNop;
		return GL_TRUE;
	    }

	    if (gc->state.depth.testFunc != GL_LESS)
		return GL_FALSE;

	    gc->procs.renderLine = __glRenderAntiAliasLine;

	    sp = gc->procs.line.lineFuncs;
	    ssp = gc->procs.line.stippledLineFuncs;

	    *sp++ = __glScissorLine;
	    *ssp++ = __glScissorStippledLine;

	    if (gc->depthBuffer.buf.elementSize == 2) {
		*sp++ = __glDrawAALine_RGB_16_Flat_LESS16_SA_MSA;
		*ssp++ = __glDrawAAStippledLine_RGB_16_Flat_LESS16_SA_MSA;
	    } else if (gc->depthBuffer.buf.elementSize == 4) {
		*sp++ = __glDrawAALine_RGB_16_Flat_LESS32_SA_MSA;
		*ssp++ = __glDrawAAStippledLine_RGB_16_Flat_LESS32_SA_MSA;
	    } else {
		return GL_FALSE;
	    }
	} else {
	    if (GL_SRC_ALPHA != gc->state.raster.blendSrc ||
		GL_ONE != gc->state.raster.blendDst)
		return GL_FALSE;

	    gc->procs.renderLine = __glRenderAntiAliasLine;

	    sp = gc->procs.line.lineFuncs;
	    ssp = gc->procs.line.stippledLineFuncs;

	    *sp++ = __glScissorLine;
	    *ssp++ = __glScissorStippledLine;

	    if (modeFlags & __GL_SHADE_SMOOTH) {
		*sp++ = __glDrawAALine_RGB_16_Smooth_SA_ONE;
		*ssp++ = __glDrawAAStippledLine_RGB_16_Smooth_SA_ONE;
	    } else {
		*sp++ = __glDrawAALine_RGB_16_Flat_SA_ONE;
		*ssp++ = __glDrawAAStippledLine_RGB_16_Flat_SA_ONE;
	    }
	}

	if (gc->buffers.doubleStore) {
	    replicateLine = GL_TRUE;
	}

	spanCount = sp - gc->procs.line.lineFuncs;
	gc->procs.line.n = spanCount;
	gc->procs.line.m = spanCount;
	gc->procs.line.l = spanCount;

	gc->procs.line.wideLineRep = (__GLspanFunc) __glNop;
	gc->procs.line.wideStippledLineRep = (__GLstippledSpanFunc) __glNop;

	assert(spanCount == 2);
	gc->procs.line.processLine = __glProcessLine2NW;

	if (gc->buffers.doubleStore) {
	    /* maybe should create processLine2 for this purpose? */
	    gc->procs.line.drawLine = gc->procs.line.processLine;
	    gc->procs.line.processLine = __glSlowDrawBothLine;
	}

	if ((modeFlags & __GL_SHADE_CHEAP_FOG) &&
	    !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
	    gc->procs.renderLine2 = gc->procs.renderLine;
	    gc->procs.renderLine = __glRenderFlatFogLine;
	}

    } else {
	/* Aliased lines */

	gc->procs.renderLine = __glRenderAliasLine;
	sp = gc->procs.line.lineFuncs;
	ssp = gc->procs.line.stippledLineFuncs;

	if (modeFlags & __GL_SHADE_LINE_STIPPLE) {
	    *sp++ = __glStippleLine;
	    *ssp++ = NULL;
	}

	if (gc->state.line.aliasedWidth > 1) {
	    wideLine = GL_TRUE;
	}
	spanCount = sp - gc->procs.line.lineFuncs;
	gc->procs.line.n = spanCount;

	*sp++ = __glScissorLine;
	*ssp++ = __glScissorStippledLine;

	if (modeFlags & __GL_SHADE_DEPTH_TEST) {
	    if (gc->state.depth.testFunc == GL_NEVER) {
		/* Unexpected end of line routine picking! */
		gc->procs.line.processLine = (__GLspanFunc) __glNop;
		return GL_TRUE;
	    } else {
		*sp++ = __glDepthTestLine;
		*ssp++ = __glDepthTestStippledLine;
	    }
	}

	if (gc->buffers.doubleStore) {
	    replicateLine = GL_TRUE;
	}
	spanCount = sp - gc->procs.line.lineFuncs;
	gc->procs.line.m = spanCount;

	if (gc->modes.colorIndexMode) {
	    /* 8-bit */
	    if (modeFlags & __GL_SHADE_DITHER) {
		if (modeFlags & __GL_SHADE_SMOOTH) {
		    *sp++ = __glDrawLine_CI8_Smooth_Dither;
		    *ssp++ = __glDrawStippledLine_CI8_Smooth_Dither;
		} else {
		    *sp++ = __glDrawLine_CI8_Flat_Dither;
		    *ssp++ = __glDrawStippledLine_CI8_Flat_Dither;
		}
	    } else {
		if (modeFlags & __GL_SHADE_SMOOTH) {
		    *sp++ = __glDrawLine_CI8_Smooth;
		    *ssp++ = __glDrawStippledLine_CI8_Smooth;
		} else {
		    *sp++ = __glDrawLine_CI8_Flat;
		    *ssp++ = __glDrawStippledLine_CI8_Flat;
		}
	    }
	} else { /* RGBA */
	    /* 16-bit */
	    if (modeFlags & __GL_SHADE_DITHER) {
		if (modeFlags & __GL_SHADE_SMOOTH) {
		    *sp++ = __glDrawLine_RGB16_Smooth_Dither;
		    *ssp++ = __glDrawStippledLine_RGB16_Smooth_Dither;
		} else {
		    *sp++ = __glDrawLine_RGB16_Flat_Dither;
		    *ssp++ = __glDrawStippledLine_RGB16_Flat_Dither;
		}
	    } else {
		if (modeFlags & __GL_SHADE_SMOOTH) {
		    *sp++ = __glDrawLine_RGB16_Smooth;
		    *ssp++ = __glDrawStippledLine_RGB16_Smooth;
		} else {
		    *sp++ = __glDrawLine_RGB16_Flat;
		    *ssp++ = __glDrawStippledLine_RGB16_Flat;
		}
	    }
	}

	spanCount = sp - gc->procs.line.lineFuncs;
	gc->procs.line.l = spanCount;

	sp = &gc->procs.line.wideLineRep;
	ssp = &gc->procs.line.wideStippledLineRep;
	if (wideLine) {
	    *sp = __glWideLineRep;
	    *ssp = __glWideStippleLineRep;
	    sp = &gc->procs.line.drawLine;
	    ssp = &gc->procs.line.drawStippledLine;
	} 
	if (replicateLine) {
	    *sp = __glDrawBothLine;
	    *ssp = __glDrawBothStippledLine;
	} else {
	    *sp = (__GLspanFunc) __glNop;
	    *ssp = (__GLstippledSpanFunc) __glNop;
	    gc->procs.line.m = gc->procs.line.l;
	}
	if (!wideLine) {
	    gc->procs.line.n = gc->procs.line.m;
	}

	if (!wideLine && !replicateLine) {
	    if (spanCount == 2) {
		gc->procs.line.processLine = __glProcessLine2NW;
	    } else if (spanCount = 3) {
		gc->procs.line.processLine = __glProcessLine3NW;
	    } else {
		gc->procs.line.processLine = __glProcessLine;
	    }
	} else {
	    gc->procs.line.processLine = __glProcessLine;
	}

	if ((modeFlags & __GL_SHADE_CHEAP_FOG) &&
	    !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
	    gc->procs.renderLine2 = gc->procs.renderLine;
	    gc->procs.renderLine = __glRenderFlatFogLine;
	}
    }

    return GL_TRUE;
}

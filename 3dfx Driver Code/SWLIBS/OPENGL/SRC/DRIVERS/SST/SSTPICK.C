/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 2$ 
** $Date: 10/11/00 8:03:07 PM$ 
**
*/

#include "sstcontext.h"

void __glSSTValidate(__GLcontext *gc)
{
    (*gc->procs.pickAllProcs)(gc);
    __glSSTGlideValidateHW(gc);

    /* copy the dispatch table into the ICD, if needed */
    __glCopyDispatch(&__gl_dispatch, &__gl_dispatch);
}

void __glSSTPickBufferProcs(__GLcontext *gc)
{
    __GLbufferMachine *buffers;
    GLuint renderBufferMask = 0;
    GLuint drawBufferMask, readBufferMask;

    buffers = &gc->buffers;
    buffers->doubleStore = GL_FALSE;

    gc->buffers.lock.bufferToLock = NULL;
    if (gc->frontBuffer.buf.drawableBuf) {
        gc->frontBuffer.buf.drawableBuf->readlock =  GL_FALSE;
        gc->frontBuffer.buf.drawableBuf->writelock =  GL_FALSE;
    }
    if (gc->backBuffer.buf.drawableBuf) {
        gc->backBuffer.buf.drawableBuf->readlock =  GL_FALSE;
        gc->backBuffer.buf.drawableBuf->writelock =  GL_FALSE;
    }

    /* Set draw buffer pointer */
    switch (gc->state.raster.drawBuffer) {
      case GL_FRONT:
	gc->drawBuffer = gc->front;
	drawBufferMask = __GL_FRONT_BUFFER_MASK;
	if (gc->front->buf.drawableBuf->lock &&
	    gc->front->buf.drawableBuf->unlock) {
	    renderBufferMask = __GL_FRONT_BUFFER_MASK;
	    gc->buffers.lock.bufferToLock = &gc->front->buf;
	}
	break;
      case GL_FRONT_AND_BACK:
	if (gc->modes.doubleBufferMode) {
	    gc->drawBuffer = gc->back;
	    buffers->doubleStore = GL_TRUE;
	    drawBufferMask = __GL_FRONT_BUFFER_MASK | __GL_BACK_BUFFER_MASK;
	    if (gc->front->buf.drawableBuf->lock &&
		gc->front->buf.drawableBuf->unlock) {
		renderBufferMask = __GL_FRONT_BUFFER_MASK;
	    }
	    if (gc->back->buf.drawableBuf->lock &&
		gc->back->buf.drawableBuf->unlock) {
		renderBufferMask |= __GL_BACK_BUFFER_MASK;
	    }
	} else {
	    gc->drawBuffer = gc->front;
	    drawBufferMask = __GL_FRONT_BUFFER_MASK;
	    if (gc->front->buf.drawableBuf->lock &&
		gc->front->buf.drawableBuf->unlock) {
		renderBufferMask = __GL_FRONT_BUFFER_MASK;
		gc->buffers.lock.bufferToLock = &gc->front->buf;
	    }
	}
	break;
      case GL_BACK:
	gc->drawBuffer = gc->back;
	drawBufferMask = __GL_BACK_BUFFER_MASK;
	if (gc->back->buf.drawableBuf->lock &&
	    gc->back->buf.drawableBuf->unlock) {
	    renderBufferMask = __GL_BACK_BUFFER_MASK;
	    gc->buffers.lock.bufferToLock = &gc->back->buf;
	}
	break;
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3: 
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
        {
	    GLint i = gc->state.raster.drawBuffer - GL_AUX0;
	    gc->drawBuffer = &gc->auxBuffer[i];
	    drawBufferMask = __GL_AUX_BUFFER_MASK(i);
	    if (gc->auxBuffer[i].buf.drawableBuf->lock &&
		gc->auxBuffer[i].buf.drawableBuf->unlock) {
		renderBufferMask = __GL_AUX_BUFFER_MASK(i);
		gc->buffers.bufferToLock = &gc->auxBuffer[i];
	    }
        }
#endif
	break;
      default:
	  break;
    }

    gc->drawBuffer->buf.drawableBuf->writelock = GL_TRUE;

    /* set read buffer pointer */
    switch (gc->state.pixel.readBuffer) {
      case GL_FRONT:
	  readBufferMask = __GL_FRONT_BUFFER_MASK;
	  gc->front->buf.drawableBuf->readlock = GL_TRUE;
	  break;
      case GL_BACK:
	  readBufferMask = __GL_BACK_BUFFER_MASK;
	  gc->back->buf.drawableBuf->readlock = GL_TRUE;
	  break;
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
	  {
	      GLint i = gc->state.raster.readBuffer = GL_AUX0;
	      readBufferMask |= __GL_AUX_BUFFER_MASK(i);
	  }
#endif
          break;
      default:
	  break;
    }

    /* figure out lock routines */
    gc->buffers.lock.lockBuffers = __glLockBuffers;
    gc->buffers.lock.unlockBuffers = __glUnlockBuffers;
    gc->buffers.lock.lockRenderBuffers = __glLockRenderBuffers;
    gc->buffers.lock.unlockRenderBuffers = __glUnlockRenderBuffers;

    if (gc->polygon.shader.modeFlags & __GL_SHADE_DEPTH_TEST) {
	renderBufferMask |= __GL_DEPTH_BUFFER_MASK;
    }
    if (gc->polygon.shader.modeFlags & __GL_SHADE_STENCIL_TEST) {
	renderBufferMask |= __GL_STENCIL_BUFFER_MASK;
    }

    /* set fast paths for buffer locking */
    if (((drawBufferMask | __GL_DEPTH_BUFFER_MASK) == renderBufferMask) &&
	(buffers->doubleStore == GL_FALSE)) {
	/* only one color buffer and depth buffer */
	gc->buffers.lock.lockRenderBuffers = __glLockColorBufferDepthBuffer;
	gc->buffers.lock.unlockRenderBuffers = __glUnlockColorBufferDepthBuffer;
    }
    if ((drawBufferMask == renderBufferMask) &&
	(buffers->doubleStore == GL_FALSE)) {
	/* only one color buffer */
	gc->buffers.lock.lockRenderBuffers = __glLockColorBuffer;
	gc->buffers.lock.unlockRenderBuffers = __glUnlockColorBuffer;
    }

    gc->buffers.lock.renderBufferMask = renderBufferMask;
    gc->buffers.lock.drawBufferMask = drawBufferMask;
    gc->buffers.lock.readBufferMask = readBufferMask;
}

void __glSSTPickLineProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLspanFunc *sp;
    __GLstippledSpanFunc *ssp;
    int spanCount;
    GLboolean wideLine;
    GLboolean replicateLine;
    GLuint aaline;

    if (gc->vertex.faceNeeds[__GL_FRONTFACE] == 0) {
	gc->procs.vertexLStrip = __glOtherLStripVertexFast;
    } else {
	gc->procs.vertexLStrip = __glOtherLStripVertex;
    }
    gc->procs.vertex2ndLines = __glSecondLinesVertex;
    if (gc->renderMode == GL_FEEDBACK) {
	gc->procs.renderLine = __glFeedbackLine;
    } else if (gc->renderMode == GL_SELECT) {
	gc->procs.renderLine = __glSelectLine;
    } else {
	aaline = gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE;
        if (modeFlags & (__GL_SHADE_MASK|__GL_SHADE_STENCIL_TEST|
			 __GL_SHADE_LINE_STIPPLE)) {
	    /* Software rasterization */
	    replicateLine = wideLine = GL_FALSE;

	    if (aaline) {
		gc->procs.renderLine = __glRenderAntiAliasLine;
	    } else {
		gc->procs.renderLine = __glRenderAliasLine;
	    }

	    sp = gc->procs.line.lineFuncs;
	    ssp = gc->procs.line.stippledLineFuncs;

	    if (!aaline && (modeFlags & __GL_SHADE_LINE_STIPPLE)) {
		*sp++ = __glStippleLine;
		*ssp++ = NULL;
	    }

	    if (!aaline && gc->state.line.aliasedWidth > 1) {
		wideLine = GL_TRUE;
	    }
	    spanCount = sp - gc->procs.line.lineFuncs;
	    gc->procs.line.n = spanCount;

	    *sp++ = __glScissorLine;
	    *ssp++ = __glScissorStippledLine;

	    if (!aaline) {
		if (modeFlags & __GL_SHADE_STENCIL_TEST) {
		    *sp++ = __glStencilTestLine;
		    *ssp++ = __glStencilTestStippledLine;
		    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
			*sp = __glDepthTestStencilLine;
			*ssp = __glDepthTestStencilStippledLine;
		    } else {
			*sp = __glDepthPassLine;
			*ssp = __glDepthPassStippledLine;
		    }
		    sp++;
		    ssp++;
		} else {
		    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
			if (gc->depthBuffer.testFunc == GL_NEVER) {
			    /* Unexpected end of line routine picking! */
			    gc->procs.line.processLine = (__GLspanFunc) __glNop;
			    return;
			} else {
#ifdef __GL_USE_MIPSASMCODE
			    *sp++ = __glDepthTestLine_asm;
#else
			    *sp++ = __glDepthTestLine;
#endif
			}
			*ssp++ = __glDepthTestStippledLine;
		    }
		}
	    }

	    /* Load phase three procs */
	    if (modeFlags & __GL_SHADE_RGB) {
		if (modeFlags & __GL_SHADE_SMOOTH) {
		    *sp = __glShadeRGBASpan;
		    *ssp = __glShadeRGBASpan;
		} else {
		    *sp = __glFlatRGBASpan;
		    *ssp = __glFlatRGBASpan;
		}
	    } else {
		if (modeFlags & __GL_SHADE_SMOOTH) {
		    *sp = __glShadeCISpan;
		    *ssp = __glShadeCISpan;
		} else {
		    *sp = __glFlatCISpan;
		    *ssp = __glFlatCISpan;
		}
	    }
	    sp++;
	    ssp++;
	    if (modeFlags & __GL_SHADE_TEXTURE) {
		*sp++ = __glTextureSpan;
		*ssp++ = __glTextureStippledSpan;
	    }
	    if (modeFlags & __GL_SHADE_SLOW_FOG) {
		if (gc->state.hints.fog == GL_NICEST) {
		    *sp = __glFogSpanSlow;
		    *ssp = __glFogStippledSpanSlow;
		} else {
		    *sp = __glFogSpan;
		    *ssp = __glFogStippledSpan;
		}
		sp++;
		ssp++;
	    }

	    if (aaline) {
		*sp++ = __glAntiAliasLine;
		*ssp++ = __glAntiAliasStippledLine;
	    }

	    if (aaline) {
		if (modeFlags & __GL_SHADE_STENCIL_TEST) {
		    *sp++ = __glStencilTestLine;
		    *ssp++ = __glStencilTestStippledLine;
		    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
			*sp = __glDepthTestStencilLine;
			*ssp = __glDepthTestStencilStippledLine;
		    } else {
			*sp = __glDepthPassLine;
			*ssp = __glDepthPassStippledLine;
		    }
		    sp++;
		    ssp++;
		} else {
		    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
			if( gc->depthBuffer.testFunc == GL_NEVER ) {
			    /* Unexpected end of line routine picking! */
			    gc->procs.line.processLine = (__GLspanFunc) __glNop;
			    return;
			} else {
#ifdef __GL_USE_MIPSASMCODE
			    *sp++ = __glDepthTestLine_asm;
#else
			    *sp++ = __glDepthTestLine;
#endif
			}
			*ssp++ = __glDepthTestStippledLine;
		    }
		}
	    }

	    if (modeFlags & __GL_SHADE_ALPHA_TEST) {
		*sp++ = __glAlphaTestSpan;
		*ssp++ = __glAlphaTestStippledSpan;
	    }

	    if (modeFlags & __GL_SHADE_INDEX_TEST) {
		*sp++ = __glIndexTestSpan;
		*ssp++ = __glIndexTestStippledSpan;
	    }
           
	    if (gc->buffers.doubleStore) {
		replicateLine = GL_TRUE;
	    }
	    spanCount = sp - gc->procs.line.lineFuncs;
	    gc->procs.line.m = spanCount;

	    if (0 == gc->drawBuffer->destMask &&
		0 == (modeFlags & (__GL_SHADE_LOGICOP|__GL_SHADE_BLEND|__GL_SHADE_OWNERSHIP_TEST))) {
		if (gc->modes.colorIndexMode) {
		    if (gc->drawBuffer->buf.elementSize == 1) {
			/* 8-bit */
			if (modeFlags & __GL_SHADE_DITHER) {
			    if (modeFlags & (__GL_SHADE_SMOOTH |
					     __GL_SHADE_TEXTURE |
					     __GL_SHADE_SLOW_FOG)) {
				*sp++ = __glStoreLine_CI_8_Smooth_Dither;
				*ssp++ = __glStoreStippledLine_CI_8_Smooth_Dither;
			    } else {
				*sp++ = __glStoreLine_CI_8_Flat_Dither;
				*ssp++ = __glStoreStippledLine_CI_8_Flat_Dither;
			    }
			} else {
			    if (modeFlags & (__GL_SHADE_SMOOTH |
					     __GL_SHADE_TEXTURE |
					     __GL_SHADE_SLOW_FOG)) {
				*sp++ = __glStoreLine_CI_8_Smooth;
				*ssp++ = __glStoreStippledLine_CI_8_Smooth;
			    } else {
				*sp++ = __glStoreLine_CI_8_Flat;
				*ssp++ = __glStoreStippledLine_CI_8_Flat;
			    }
			}
		    } else {
			/* Not fast path (yet) */
			*sp++ = __glStoreLine;
			*ssp++ = __glStoreStippledLine;
		    }
		} else { /* RGBA */
		    if (gc->drawBuffer->buf.elementSize == 2) {
			/* 16-bit */
			if (modeFlags & __GL_SHADE_DITHER) {
			    *sp++ = __glStoreLine_RGB_16_Dither;
			    *ssp++ = __glStoreStippledLine_RGB_16_Dither;
			} else {
			    if (modeFlags & (__GL_SHADE_SMOOTH |
					     __GL_SHADE_TEXTURE |
					     __GL_SHADE_SLOW_FOG)) {
				*sp++ = __glStoreLine_RGB_16_Smooth;
				*ssp++ = __glStoreStippledLine_RGB_16_Smooth;
			    } else {
				*sp++ = __glStoreLine_RGB_16_Flat;
				*ssp++ = __glStoreStippledLine_RGB_16_Flat;
			    }
			}
		    } else {
			/* Not fast path (yet) */
			*sp++ = __glStoreLine;
			*ssp++ = __glStoreStippledLine;
		    }
		}
	    } else {
		*sp++ = __glStoreLine;
		*ssp++ = __glStoreStippledLine;
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

	    if (!wideLine && !replicateLine && spanCount == 3) {
		gc->procs.line.processLine = __glProcessLine3NW;
	    } else {
		gc->procs.line.processLine = __glProcessLine;
	    }
	    if ((modeFlags & __GL_SHADE_CHEAP_FOG) &&
		!(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
		gc->procs.renderLine2 = gc->procs.renderLine;
		gc->procs.renderLine = __glRenderFlatFogLine;
	    }
	} else {
	    /* Hardware accelerated rasterization */
	    if (gc->state.line.requestedWidth == 1.0f) {
		gc->procs.renderLine = __glSSTRenderLine;
	    } else {
		/* Wide lines */
		if (aaline) {
		    gc->procs.renderLine = __glSSTRenderWideAALine;
		} else {
		    gc->procs.renderLine = __glSSTRenderWideLine;
		}
	    }
	}
    }
}

void __glSSTPickPointProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if (gc->vertex.faceNeeds[__GL_FRONTFACE] == 0) {
	gc->procs.vertexPoints = __glPointFast;
    } else {
	gc->procs.vertexPoints = __glPoint;
    }

    if (gc->renderMode == GL_FEEDBACK) {
	gc->procs.renderPoint = __glFeedbackPoint;
	return;
    } 
    if (gc->renderMode == GL_SELECT) {
	gc->procs.renderPoint = __glSelectPoint;
	return;
    } 
    if (modeFlags & (__GL_SHADE_MASK|__GL_SHADE_STENCIL_TEST)) {
	/* Software rasterization */
	if (gc->state.enables.general & __GL_POINT_SMOOTH_ENABLE) {
	    if (gc->modes.colorIndexMode) {
		gc->procs.renderPoint = __glRenderAntiAliasedCIPoint;
	    } else {
		gc->procs.renderPoint = __glRenderAntiAliasedRGBPoint;
	    }
	} else if (gc->state.point.aliasedSize != 1) {
	    gc->procs.renderPoint = __glRenderAliasedPointN;
	} else if (modeFlags & __GL_SHADE_TEXTURE) {
	    gc->procs.renderPoint = __glRenderAliasedPoint1;
	} else {
	    gc->procs.renderPoint = __glRenderAliasedPoint1_NoTex;
	}

	if (((modeFlags & __GL_SHADE_CHEAP_FOG) &&
	     !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) ||
	    (modeFlags & __GL_SHADE_SLOW_FOG)) {
	    gc->procs.renderPoint2 = gc->procs.renderPoint;
	    gc->procs.renderPoint = __glRenderFlatFogPoint;
	}
    } else {
	/* Hardware accelerate rasterization */
	if (gc->state.point.requestedSize == 1.0f) {
	    gc->procs.renderPoint = __glSSTRenderPoint;
	} else {
	    /* Wide lines */
	    if (gc->state.enables.general & __GL_POINT_SMOOTH_ENABLE) {
		gc->procs.renderPoint = __glSSTRenderWideAAPoint;
	    } else {
		gc->procs.renderPoint = __glSSTRenderWidePoint;
	    }
	}
    }
}

#ifdef __GL_PC_RAST
extern int __glPCPickTriangleProcs(__GLcontext *gc);
#endif

void __glSSTPickTriangleProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    /*
    ** Setup cullFace so that a single test will do the cull check.
    */
    if (modeFlags & __GL_SHADE_CULL_FACE) {
	switch (gc->state.polygon.cull) {
	  case GL_FRONT:
	    gc->polygon.cullFace = __GL_CULL_FLAG_FRONT;
	    break;
	  case GL_BACK:
	    gc->polygon.cullFace = __GL_CULL_FLAG_BACK;
	    break;
	  case GL_FRONT_AND_BACK:
	    gc->procs.renderTriangle = __glDontRenderTriangle;
	    gc->procs.fillTriangle = 0;		/* Done to find bugs */
	    return;
	}
    } else {
	gc->polygon.cullFace = __GL_CULL_FLAG_DONT;
    }

    /* Build lookup table for face direction */
    switch (gc->state.polygon.frontFaceDirection) {
      case GL_CW:
	if (gc->constants.yInverted) {
	    gc->polygon.face[__GL_CW] = __GL_BACKFACE;
	    gc->polygon.face[__GL_CCW] = __GL_FRONTFACE;
	} else {
	    gc->polygon.face[__GL_CW] = __GL_FRONTFACE;
	    gc->polygon.face[__GL_CCW] = __GL_BACKFACE;
	}
	break;
      case GL_CCW:
	if (gc->constants.yInverted) {
	    gc->polygon.face[__GL_CW] = __GL_FRONTFACE;
	    gc->polygon.face[__GL_CCW] = __GL_BACKFACE;
	} else {
	    gc->polygon.face[__GL_CW] = __GL_BACKFACE;
	    gc->polygon.face[__GL_CCW] = __GL_FRONTFACE;
	}
	break;
    }

    /* Make polygon mode indexable and zero based */
    gc->polygon.mode[__GL_FRONTFACE] =
	(GLubyte) (gc->state.polygon.frontMode & 0xf);
    gc->polygon.mode[__GL_BACKFACE] =
	(GLubyte) (gc->state.polygon.backMode & 0xf);
    
    if (gc->renderMode == GL_FEEDBACK) {
	gc->procs.renderTriangle = __glFeedbackTriangle;
	gc->procs.fillTriangle = 0;		/* Done to find bugs */
	return;
    }
    if (gc->renderMode == GL_SELECT) {
	gc->procs.renderTriangle = __glSelectTriangle;
	gc->procs.fillTriangle = 0;		/* Done to find bugs */
	return;
    }

    /* Invoke sw renderer for unsupported modes */
    if (modeFlags & (__GL_SHADE_STENCIL_TEST|__GL_SHADE_STIPPLE|
		     __GL_SHADE_MASK)) {
#ifdef __GL_PC_RAST
	if (__glPCPickTriangleProcs(gc))
	    return;
#endif
	if ((gc->state.polygon.frontMode == gc->state.polygon.backMode) &&
	    (gc->state.polygon.frontMode == GL_FILL)) {
	    if (modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
		gc->procs.renderTriangle = __glRenderSmoothTriangle;
	    } else {
		gc->procs.renderTriangle = __glRenderFlatTriangle;
	    }
	} else {
	    gc->procs.renderTriangle = __glRenderTriangle;
	}

	if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) {
	    gc->procs.fillTriangle = __glFillAntiAliasedTriangle;
	} else {
	    gc->procs.fillTriangle = __glFillTriangle;
	}
	if ((modeFlags & __GL_SHADE_CHEAP_FOG) &&
	    !(modeFlags & __GL_SHADE_SMOOTH_LIGHT)) {
	    gc->procs.fillTriangle2 = gc->procs.fillTriangle;
	    gc->procs.fillTriangle = __glFillFlatFogTriangle;
	}
	return;
    }

    /* XXXwheeler - flesh out more of these routines */
    gc->procs.renderTriangle = __glSSTRenderTriangle;
}

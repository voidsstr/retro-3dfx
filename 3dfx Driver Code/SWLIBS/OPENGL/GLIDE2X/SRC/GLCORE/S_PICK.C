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
#include "mips.h"

#include "geom_og.h"

#ifdef __GL_PC_RAST
extern int __glPCPickTriangleProcs(__GLcontext *gc);
#endif

#ifdef _GL_PC_FAST_RAST
extern int __glPCFastRastPickTriangleProcs(__GLcontext *gc);
#endif

/************************************************************************/

#define __GL_DISABLE_RASTER 1
#if __GL_DISABLE_RASTER
/*
** Useful for timing the geometry pipeline.
*/
void __glDontFillTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
                          __GLvertex *c, GLboolean ccw)
{
}

void __glDontRenderPoint(__GLcontext *gc, __GLvertex *v)
{
}

void __glDontRenderLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
}

#endif

/************************************************************************/

/* these are depth test routines for C.. */
GLboolean (*__glCDTPixel[32])(__GLzValue, __GLzValue *) = {
    /* unsigned ops, no mask */
    __glDT_NEVER16,
    __glDT_LESS16,
    __glDT_EQUAL16,
    __glDT_LEQUAL16,
    __glDT_GREATER16,
    __glDT_NOTEQUAL16,
    __glDT_GEQUAL16,
    __glDT_ALWAYS16,
    /* unsigned ops, mask */
    __glDT_NEVER16,
    __glDT_LESS16_M,
    __glDT_EQUAL16_M,
    __glDT_LEQUAL16_M,
    __glDT_GREATER16_M,
    __glDT_NOTEQUAL16_M,
    __glDT_GEQUAL16_M,
    __glDT_ALWAYS16_M,
    /* unsigned ops, no mask */
    __glDT_NEVER,
    __glDT_LESS,
    __glDT_EQUAL,
    __glDT_LEQUAL,
    __glDT_GREATER,
    __glDT_NOTEQUAL,
    __glDT_GEQUAL,
    __glDT_ALWAYS,
    /* unsigned ops, mask */
    __glDT_NEVER,
    __glDT_LESS_M,
    __glDT_EQUAL_M,
    __glDT_LEQUAL_M,
    __glDT_GREATER_M,
    __glDT_NOTEQUAL_M,
    __glDT_GEQUAL_M,
    __glDT_ALWAYS_M,
};

#ifdef __GL_USE_MIPSASMCODE
/*
** the following are depth testers for assemblerized
** depth test routines.  They have unique calling conventions
** and they should only be used in the routines that they are
** design for.
*/


/* for spans */
void (*__glSDepthTestPixel[32])(void) = {
    /* unsigned ops, no mask */
    __glDTS_NEVER16,
    __glDTS_LESS16,
    __glDTS_EQUAL16,
    __glDTS_LEQUAL16,
    __glDTS_GREATER16,
    __glDTS_NOTEQUAL16,
    __glDTS_GEQUAL16,
    __glDTS_ALWAYS16,
    /* unsigned ops, mask */
    __glDTS_NEVER16,
    __glDTS_LESS16_M,
    __glDTS_EQUAL16_M,
    __glDTS_LEQUAL16_M,
    __glDTS_GREATER16_M,
    __glDTS_NOTEQUAL16_M,
    __glDTS_GEQUAL16_M,
    __glDTS_ALWAYS16_M,
    /* unsigned ops, no mask */
    __glDTS_NEVER,
    __glDTS_LESS,
    __glDTS_EQUAL,
    __glDTS_LEQUAL,
    __glDTS_GREATER,
    __glDTS_NOTEQUAL,
    __glDTS_GEQUAL,
    __glDTS_ALWAYS,
    /* unsigned ops, mask */
    __glDTS_NEVER,
    __glDTS_LESS_M,
    __glDTS_EQUAL_M,
    __glDTS_LEQUAL_M,
    __glDTS_GREATER_M,
    __glDTS_NOTEQUAL_M,
    __glDTS_GEQUAL_M,
    __glDTS_ALWAYS_M,
};

/* for lines */
void (*__glPDepthTestPixel[32])(void) = {
    /* unsigned, no mask */
    __glDTP_NEVER16,
    __glDTP_LESS16,
    __glDTP_EQUAL16,
    __glDTP_LEQUAL16,
    __glDTP_GREATER16,
    __glDTP_NOTEQUAL16,
    __glDTP_GEQUAL16,
    __glDTP_ALWAYS16,
    /* unsigned, mask */
    __glDTP_NEVER16,
    __glDTP_LESS16_M,
    __glDTP_EQUAL16_M,
    __glDTP_LEQUAL16_M,
    __glDTP_GREATER16_M,
    __glDTP_NOTEQUAL16_M,
    __glDTP_GEQUAL16_M,
    __glDTP_ALWAYS16_M,
    /* unsigned, no mask */
    __glDTP_NEVER,
    __glDTP_LESS,
    __glDTP_EQUAL,
    __glDTP_LEQUAL,
    __glDTP_GREATER,
    __glDTP_NOTEQUAL,
    __glDTP_GEQUAL,
    __glDTP_ALWAYS,
    /* unsigned, mask */
    __glDTP_NEVER,
    __glDTP_LESS_M,
    __glDTP_EQUAL_M,
    __glDTP_LEQUAL_M,
    __glDTP_GREATER_M,
    __glDTP_NOTEQUAL_M,
    __glDTP_GEQUAL_M,
    __glDTP_ALWAYS_M,
};
#endif

void __glGenericPickSpanProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    __GLspanFunc *sp;
    __GLstippledSpanFunc *ssp;
    int spanCount;
    GLboolean replicateSpan;

    replicateSpan = GL_FALSE;
    sp = gc->procs.span.spanFuncs;
    ssp = gc->procs.span.stippledSpanFuncs;

    /* Load phase one procs */
    if (!gc->transform.reasonableViewport) {
        *sp++ = __glClipSpan;
        *ssp++ = NULL;
    }
    
    if (modeFlags & __GL_SHADE_STIPPLE) {
        *sp++ = __glStippleSpan;
        *ssp++ = __glStippleStippledSpan;
    }

    /* Load phase two procs */

    /* 
    ** If alpha or index test is on, stencil and depth have to be done after
    ** the alpha test.  If not, doing them before interpolating
    ** and texture mapping, etc. is an optimization.
    */
    if (!(modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_INDEX_TEST))) {
        if (modeFlags & __GL_SHADE_STENCIL_TEST) {
#ifdef __GL_USE_MIPSASMCODE
            *sp++ = __glStencilTestSpan_asm;
#else
            *sp++ = __glStencilTestSpan;
#endif
            *ssp++ = __glStencilTestStippledSpan;
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
#ifdef __GL_USE_MIPSASMCODE
                if(modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                  /* No asm version yet */
                  *sp = __glDepthTestStencilOffsetSpan;
                  *ssp = __glDepthTestStencilStippledOffsetSpan;
                } else {
                  *sp = __glDepthTestStencilSpan_asm;
                  *ssp = __glDepthTestStencilStippledSpan_asm;
                }
#else
                if(modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                  *sp = __glDepthTestStencilOffsetSpan;
                  *ssp = __glDepthTestStencilStippledOffsetSpan;
                } else {
                  *sp = __glDepthTestStencilSpan;
                  *ssp = __glDepthTestStencilStippledSpan;
                }
#endif
            } else {
                *sp = __glDepthPassSpan;
                *ssp = __glDepthPassStippledSpan;
            }
            sp++;
            ssp++;
        } else {
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                if (gc->depthBuffer.testFunc == GL_NEVER) {
                    gc->procs.span.processSpan = (__GLspanFunc) __glNop;
                    return;
                }
#ifdef __GL_USE_MIPSASMCODE
                if(modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                  /* No asm version yet */
                  *sp++ = __glDepthTestOffsetSpan;
                  *ssp++ = __glDepthTestStippledOffsetSpan;
                } else {
                  *sp++ = __glDepthTestSpan_asm;
                  *ssp++ = __glDepthTestStippledSpan_asm;
                }
#else
                if(modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                  *sp++ = __glDepthTestOffsetSpan;
                  *ssp++ = __glDepthTestStippledOffsetSpan;
                } else {
                  *sp++ = __glDepthTestSpan;
                  *ssp++ = __glDepthTestStippledSpan;
                }
#endif
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
        __GLtexture *current = gc->texture.currentTexture[0];

		if ( !current && ( gc->texture.currentTexture[1] ) ) {
			current = gc->texture.currentTexture[1];
		}

		if ( !current ) return;

        *sp = __glTextureSpan;
        *ssp = __glTextureStippledSpan;
        /* optimize some cases */
        if (current->dim == 2) {
            __GLtextureParamState *params = &current->params;
            if ((current->level[0].internalFormat == GL_RGB) &&
                (current->level[0].border == 0)) {
                if (params->minFilter == GL_NEAREST_MIPMAP_LINEAR &&
                    params->magFilter == GL_LINEAR) {
                    *sp = __glTextureRGB_L_NML_Span;
                    *ssp = __glTextureRGB_L_NML_StippledSpan;
                }
                if (params->minFilter == GL_LINEAR_MIPMAP_LINEAR &&
                    params->magFilter == GL_LINEAR) {
                    *sp = __glTextureRGB_L_LML_Span;
                    *ssp = __glTextureRGB_L_LML_StippledSpan;
                }
                if (params->minFilter == GL_NEAREST &&
                    params->magFilter == GL_NEAREST) {
                    *sp = __glTextureRGB_N_N_Span;
                    *ssp = __glTextureRGB_N_N_StippledSpan;
                }
                if (params->minFilter == GL_NEAREST_MIPMAP_NEAREST &&
                    params->magFilter == GL_NEAREST) {
                    *sp = __glTextureRGB_N_NMN_Span;
                    *ssp = __glTextureRGB_N_NMN_StippledSpan;
                }
            } else if ((current->level[0].internalFormat == GL_COLOR_INDEX8_EXT) &&
                       (current->level[0].border == 0)) {
                switch(current->CT.baseFormat) {
                case GL_RGB:
#if 0
                    if (params->minFilter == GL_NEAREST_MIPMAP_LINEAR &&
                        params->magFilter == GL_LINEAR) {
                        *sp = __glTextureRGB_L_NML_Span;
                        *ssp = __glTextureRGB_L_NML_StippledSpan;
                    }
                    if (params->minFilter == GL_LINEAR_MIPMAP_LINEAR &&
                        params->magFilter == GL_LINEAR) {
                        *sp = __glTextureRGB_L_LML_Span;
                        *ssp = __glTextureRGB_L_LML_StippledSpan;
                    }
#endif
                    if (params->minFilter == GL_NEAREST &&
                        params->magFilter == GL_NEAREST) {
                        *sp = __glTextureCI8_N_N_RGB_Span;
                        *ssp = __glTextureCI8_N_N_RGB_StippledSpan;
                    }
#if 0
                    if (params->minFilter == GL_NEAREST_MIPMAP_NEAREST &&
                        params->magFilter == GL_NEAREST) {
                        *sp = __glTextureRGB_N_NMN_Span;
                        *ssp = __glTextureRGB_N_NMN_StippledSpan;
                    }
#endif
                    break;
                case GL_RGBA:
#if 0
                    if (params->minFilter == GL_NEAREST_MIPMAP_LINEAR &&
                        params->magFilter == GL_LINEAR) {
                        *sp = __glTextureRGB_L_NML_Span;
                        *ssp = __glTextureRGB_L_NML_StippledSpan;
                    }
                    if (params->minFilter == GL_LINEAR_MIPMAP_LINEAR &&
                        params->magFilter == GL_LINEAR) {
                        *sp = __glTextureRGB_L_LML_Span;
                        *ssp = __glTextureRGB_L_LML_StippledSpan;
                    }
#endif
                    if (params->minFilter == GL_NEAREST &&
                        params->magFilter == GL_NEAREST) {
                        *sp = __glTextureCI8_N_N_RGBA_Span;
                        *ssp = __glTextureCI8_N_N_RGBA_StippledSpan;
                    }
#if 0
                    if (params->minFilter == GL_NEAREST_MIPMAP_NEAREST &&
                        params->magFilter == GL_NEAREST) {
                        *sp = __glTextureRGB_N_NMN_Span;
                        *ssp = __glTextureRGB_N_NMN_StippledSpan;
                    }
#endif
                    break;
                default:
                    break;
                }
            }
        }
        sp++;
        ssp++;
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

    if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_INDEX_TEST)) {
        if (modeFlags & __GL_SHADE_ALPHA_TEST) {
            *sp++ = __glAlphaTestSpan;
            *ssp++ = __glAlphaTestStippledSpan;
        } else {
            /* __GL_SHADE_INDEX_TEST must be set */
            *sp++ = __glIndexTestSpan;
            *ssp++ = __glIndexTestStippledSpan;
        }
        /* If alpha/index test is on, need to do the depth/stencil tests now. */
        if (modeFlags & __GL_SHADE_STENCIL_TEST) {
#ifdef __GL_USE_MIPSASMCODE
            *sp++ = __glStencilTestSpan_asm;
#else
            *sp++ = __glStencilTestSpan;
#endif
            *ssp++ = __glStencilTestStippledSpan;
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
#ifdef __GL_USE_MIPSASMCODE
                if(modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                  /* No asm version yet */
                  *sp = __glDepthTestStencilOffsetSpan;
                  *ssp = __glDepthTestStencilStippledOffsetSpan;
                } else {
                  *sp = __glDepthTestStencilSpan_asm;
                  *ssp = __glDepthTestStencilStippledSpan_asm;
                }
#else
                if(modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                  *sp = __glDepthTestStencilOffsetSpan;
                  *ssp = __glDepthTestStencilStippledOffsetSpan;
                } else {
                  *sp = __glDepthTestStencilSpan;
                  *ssp = __glDepthTestStencilStippledSpan;
                }
#endif
            } else {
                *sp = __glDepthPassSpan;
                *ssp = __glDepthPassStippledSpan;
            }
            sp++;
            ssp++;
        } else {
            if (modeFlags & __GL_SHADE_DEPTH_TEST) {
                if (gc->depthBuffer.testFunc == GL_NEVER) {
                    gc->procs.span.processSpan = (__GLspanFunc) __glNop;
                    return;
                }
#ifdef __GL_USE_MIPSASMCODE
                if(modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                  /* No asm version yet */
                  *sp++ = __glDepthTestOffsetSpan;
                  *ssp++ = __glDepthTestStippledOffsetSpan;
                } else {
                  *sp++ = __glDepthTestSpan_asm;
                  *ssp++ = __glDepthTestStippledSpan_asm;
                }
#else
                if(modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
                  *sp++ = __glDepthTestOffsetSpan;
                  *ssp++ = __glDepthTestStippledOffsetSpan;
                } else {
                  *sp++ = __glDepthTestSpan;
                  *ssp++ = __glDepthTestStippledSpan;
                }
#endif
            }
        }
    }

    if (gc->buffers.doubleStore) {
        spanCount = (int)(sp - gc->procs.span.spanFuncs);
        gc->procs.span.n = spanCount;
        replicateSpan = GL_TRUE;
    } 

    /* Load phase four procs */
    if (cfb->needColorFragmentOps) {
        if (modeFlags & (__GL_SHADE_LOGICOP | __GL_SHADE_MASK)) {
            *sp++ = cfb->fetchSpan;
            *ssp++ = cfb->fetchStippledSpan;
        } 
        if (modeFlags & __GL_SHADE_BLEND) {
            GLenum s = gc->state.raster.blendSrc;
            GLenum d = gc->state.raster.blendDst;

            if ((~modeFlags & __GL_SHADE_MASK) &&
                (d != GL_ZERO ||
                 s == GL_DST_COLOR || s == GL_ONE_MINUS_DST_COLOR ||
                 s == GL_DST_ALPHA || s == GL_ONE_MINUS_DST_ALPHA ||
                 s == GL_SRC_ALPHA_SATURATE)) {
                *sp++ = cfb->fetchSpan;
                *ssp++ = cfb->fetchStippledSpan;
            }
            if (s == GL_SRC_ALPHA) {
                if (d == GL_ONE_MINUS_SRC_ALPHA) {
                    *sp = __glBlendSpan_SA_MSA;
                } else if (d == GL_ONE) {
                    *sp = __glBlendSpan_SA_ONE;
                } else if (d == GL_ZERO) {
                    *sp = __glBlendSpan_SA_ZERO;
                } else {
                    *sp = __glBlendSpan;
                }
            } else if (s == GL_ONE_MINUS_SRC_ALPHA && d == GL_SRC_ALPHA) {
                *sp = __glBlendSpan_MSA_SA;
            } else {
                *sp = __glBlendSpan;
            }
            sp++;
            *ssp++ = __glBlendStippledSpan;
        }
        if (modeFlags & __GL_SHADE_DITHER) {
            if (modeFlags & __GL_SHADE_RGB) {
                *sp = __glDitherRGBASpan;
                *ssp = __glDitherRGBAStippledSpan;
            } else {
                *sp = __glDitherCISpan;
                *ssp = __glDitherCIStippledSpan;
            }
        } else {
            if (modeFlags & __GL_SHADE_RGB) {
                *sp = __glRoundRGBASpan;
                *ssp = __glRoundRGBAStippledSpan;
            } else {
                *sp = __glRoundCISpan;
                *ssp = __glRoundCIStippledSpan;
            }
        }
        sp++;
        ssp++;
        if (modeFlags & __GL_SHADE_LOGICOP) {
            *sp++ = __glLogicOpSpan;
            *ssp++ = __glLogicOpStippledSpan;
        }
        if (modeFlags & __GL_SHADE_MASK) {
            if (modeFlags & __GL_SHADE_RGB) {
                *sp = __glMaskRGBASpan;
                *ssp = __glMaskRGBASpan;
            } else {
                *sp = __glMaskCISpan;
                *ssp = __glMaskCISpan;
            }
            sp++;
            ssp++;
        }
    }

    /* Finally, copy over procs from drawBuffer */
    *sp++ = cfb->storeSpan;
    *ssp++ = cfb->storeStippledSpan;

    spanCount = (int)(sp - gc->procs.span.spanFuncs);
    gc->procs.span.m = spanCount;
    if (replicateSpan) {
        gc->procs.span.processSpan = __glProcessReplicateSpan;
    } else {
        gc->procs.span.processSpan = __glProcessSpan;
        gc->procs.span.n = spanCount;
    }
}

/************************************************************************/

void __glGenericPickPointProcs(__GLcontext *gc)
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
    if (gc->state.enables.general & __GL_POINT_SMOOTH_ENABLE) {
        if (gc->modes.colorIndexMode) {
            gc->procs.renderPoint = __glRenderAntiAliasedCIPoint;
        } else {
            gc->procs.renderPoint = __glRenderAntiAliasedRGBPoint;
        }
    } else if (gc->state.point.aliasedSize != 1) {
        gc->procs.renderPoint = __glRenderAliasedPointN;
    } else if (gc->texture.textureEnabled) {
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
    #if __GL_DISABLE_RASTER
        if (gc->vertexArray.controlWord & VERTARRAY_CW_DISABLE_RASTER) {
            gc->procs.renderPoint2 = __glDontRenderPoint;
            gc->procs.renderPoint = __glDontRenderPoint;
        }
    #endif
}

void __glGenericPickLineProcs(__GLcontext *gc)
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
        replicateLine = wideLine = GL_FALSE;

        aaline = gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE;
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
    }
    #if __GL_DISABLE_RASTER
        if (gc->vertexArray.controlWord & VERTARRAY_CW_DISABLE_RASTER) {
            gc->procs.renderLine2 = __glDontRenderLine;
            gc->procs.renderLine = __glDontRenderLine;
        }
    #endif
}

/*
** Pick the fastest triangle rendering implementation available based on
** the current mode set.  This implementation only has a few triangle
** procs, and falls back on the generic all purpose one when forced to.
*/
void __glGenericPickTriangleProcs(__GLcontext *gc)
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
            gc->procs.fillTriangle = 0;         /* Done to find bugs */
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
        gc->procs.fillTriangle = 0;             /* Done to find bugs */
        return;
    }
    if (gc->renderMode == GL_SELECT) {
        gc->procs.renderTriangle = __glSelectTriangle;
        gc->procs.fillTriangle = 0;             /* Done to find bugs */
        return;
    }

#ifdef __GL_PC_RAST
    if (__glPCPickTriangleProcs(gc))
        return;
#endif

#ifdef _GL_PC_FAST_RAST
    if (__glPCFastRastPickTriangleProcs(gc))
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

#if __GL_DISABLE_RASTER
    if (gc->vertexArray.controlWord & VERTARRAY_CW_DISABLE_RASTER) {
        gc->procs.fillTriangle = __glDontFillTriangle;
    }
#endif
}

void __glGenericPickRenderBitmapProcs(__GLcontext *gc)
{
    gc->procs.renderBitmap = __glRenderBitmap;
}

void __glGenericPickClipProcs(__GLcontext *gc)
{
    if (gc->state.light.shadingModel == GL_FLAT) {
        gc->procs.clipLine = __glFastClipFlatLine;
    } else {
        gc->procs.clipLine = __glFastClipSmoothLine;
    }
    gc->procs.clipTriangle = __glClipTriangle;
}

void __glGenericPickCalcTextureProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    /* Pick coordinate generation function */
    if ((gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_S_ENABLE) &&
        (gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_T_ENABLE) &&
        !(gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_R_ENABLE) &&
        !(gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_Q_ENABLE) &&
        (gc->state.texture[gc->texture.currentTexUnit].s.mode == gc->state.texture[gc->texture.currentTexUnit].t.mode)) {
        /* Use a special function when both modes are enabled and identical */
        switch (gc->state.texture[gc->texture.currentTexUnit].s.mode) {
          case GL_EYE_LINEAR:
            gc->procs.calcTexture = __glCalcEyeLinear;
            break;
          case GL_OBJECT_LINEAR:
            gc->procs.calcTexture = __glCalcObjectLinear;
            break;
          case GL_SPHERE_MAP:
            gc->procs.calcTexture = __glCalcSphereMap;
            break;
        }
    } else {
        if (!(gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_S_ENABLE) &&
            !(gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_T_ENABLE) &&
            !(gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_R_ENABLE) &&
            !(gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_Q_ENABLE)) {
            /* Use fast function when both are disabled */
            gc->procs.calcTexture = __glCalcTexture;
        } else {
            gc->procs.calcTexture = __glCalcMixedTexture;
        }
    }
    gc->procs.calcRasterTexture = gc->procs.calcTexture;

    if ((modeFlags & __GL_SHADE_TEXTURE_PERSP) &&
        (modeFlags & __GL_SHADE_TEXTURE_UVSCALED)) {
        gc->procs.calcTexture2 = gc->procs.calcTexture;
        if (gc->procs.calcTexture == __glCalcTexture) {
            gc->procs.calcTexture = __glFastCalcTexturePerspUVScale;
        } else {
            gc->procs.calcTexture = __glCalcTexturePerspUVScale;
        }
    } else if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
        gc->procs.calcTexture2 = gc->procs.calcTexture;
        if (gc->procs.calcTexture == __glCalcTexture) {
            gc->procs.calcTexture = __glFastCalcTexturePersp;
        } else {
            gc->procs.calcTexture = __glCalcTexturePersp;
        }
    } else if (modeFlags & __GL_SHADE_TEXTURE_UVSCALED) {
        gc->procs.calcTexture2 = gc->procs.calcTexture;
        if (gc->procs.calcTexture == __glCalcTexture) {
            gc->procs.calcTexture = __glFastCalcTextureUVScale;
        } else {
            gc->procs.calcTexture = __glCalcTextureUVScale;
        }
    }
}

void __glGenericPickTextureProcs(__GLcontext *gc)
{
    __GLtextureParamState *params;
    __GLtexture *current;

    gc->texture.currentTexture[gc->texture.currentTexUnit] = current = NULL;
    if (gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_2D_ENABLE) {
        __GLtexture *tex = __glLookUpTexture(gc, GL_TEXTURE_2D, gc->texture.currentTexUnit);

        if (__glIsTextureConsistent(gc, tex)) {
            gc->texture.currentTexture[gc->texture.currentTexUnit] = current = tex;
            params = __glLookUpTextureParams(gc, GL_TEXTURE_2D, gc->texture.currentTexUnit );
        }
    } else
    if (gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_1D_ENABLE) {
        __GLtexture *tex = __glLookUpTexture(gc, GL_TEXTURE_1D, gc->texture.currentTexUnit );

        if (__glIsTextureConsistent(gc, tex)) {
            gc->texture.currentTexture[gc->texture.currentTexUnit] = current = tex;
            params = __glLookUpTextureParams(gc, GL_TEXTURE_1D, gc->texture.currentTexUnit );
        }
    }

    /* Pick texturing function for the current texture */
    if (current) {
        GLenum baseFormat, mode;

/* XXX most of this should be bound into the texture param code, right? */
        current->params = *params;

        /*
        ** Figure out if mipmapping is being used.  If not, then the
        ** rho computations can be avoided as there is only one texture
        ** to choose from.
        */
        gc->procs.calcLineRho = __glComputeLineRho;
        gc->procs.calcPolygonRho = __glComputePolygonRho;
        if ((current->params.minFilter == GL_LINEAR)
            || (current->params.minFilter == GL_NEAREST)) {
            /* No mipmapping needed */
            if (current->params.minFilter == current->params.magFilter) {
                /* No rho needed as min/mag application is identical */
                current->textureFunc = __glFastTextureFragment;
                gc->procs.calcLineRho = __glNopLineRho;
                gc->procs.calcPolygonRho = __glNopPolygonRho;
            } else {
                current->textureFunc = __glTextureFragment;

                /*
                ** Pre-calculate min/mag switchover point.  The rho calculation
                ** doesn't perform a square root (ever).  Consequently, these
                ** constants are squared.
                */
                if ((current->params.magFilter == GL_LINEAR) &&
                    ((current->params.minFilter == GL_NEAREST_MIPMAP_NEAREST) ||
                     (current->params.minFilter == GL_LINEAR_MIPMAP_NEAREST))) {
                    current->c = ((__GLfloat) 2.0);
                } else {
                    current->c = __glOne;
                }
            }
        } else {
            current->textureFunc = __glMipMapFragment;

            /*
            ** Pre-calculate min/mag switchover point.  The rho
            ** calculation doesn't perform a square root (ever).
            ** Consequently, these constants are squared.
            */
            if ((current->params.magFilter == GL_LINEAR) &&
                ((current->params.minFilter == GL_NEAREST_MIPMAP_NEAREST) ||
                 (current->params.minFilter == GL_LINEAR_MIPMAP_NEAREST))) {
                current->c = ((__GLfloat) 2.0);
            } else {
                current->c = __glOne;
            }
        }

        /* Pick environment function according to base format and mode */
        baseFormat = current->texelFormat = current->level[0].baseFormat;
        mode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;
        /* release reference to any previous texture environment table */
        if (gc->texture.envTable) {
           gc->texture.envTable->refcount--;
        }
        gc->texture.envTable = __glCreateTextureEnvTable(mode, current);
        switch (mode) {
          case GL_MODULATE:
            switch (baseFormat) {
              case GL_LUMINANCE:
                  current->env = __glTextureModulateL;
                break;
              case GL_LUMINANCE_ALPHA:
                  current->env = __glTextureModulateLA;
                break;
              case GL_RGB:
                  current->env = __glTextureModulateRGB;
                break;
              case GL_RGBA:
                  current->env = __glTextureModulateRGBA;
                break;
              case GL_ALPHA:
                  current->env = __glTextureModulateA;
                break;
              case GL_INTENSITY:
                  current->env = __glTextureModulateI;
                break;
            case GL_COLOR_INDEX:
                current->env = __glTextureModulateCI;
                break;
            }
            break;
          case GL_DECAL:
            switch (baseFormat) {
              case GL_LUMINANCE:
                current->env = (void (*)(__GLcontext *gc, 
                        __GLcolor *frag, __GLtexel *texel)) __glNop; 
                break;
              case GL_LUMINANCE_ALPHA:
                current->env = (void (*)(__GLcontext *gc, 
                        __GLcolor *frag, __GLtexel *texel)) __glNop; 
                break;
              case GL_RGB:
                  current->env = __glTextureDecalRGB;
                break;
              case GL_RGBA:
                  current->env = __glTextureDecalRGBA;
                break;
              case GL_ALPHA:
                current->env = (void (*)(__GLcontext *gc, 
                        __GLcolor *frag, __GLtexel *texel)) __glNop; 
                break;
              case GL_INTENSITY:
                current->env = (void (*)(__GLcontext *gc, 
                        __GLcolor *frag, __GLtexel *texel)) __glNop; 
                break;
            }
            break;
          case GL_BLEND:
            switch (baseFormat) {
              case GL_LUMINANCE:
                  current->env = __glTextureBlendL;
                break;
              case GL_LUMINANCE_ALPHA:
                  current->env = __glTextureBlendLA;
                break;
              case GL_RGB:
                  current->env = __glTextureBlendRGB;
                break;
              case GL_RGBA:
                  current->env = __glTextureBlendRGBA;
                break;
              case GL_ALPHA:
                  current->env = __glTextureBlendA;
                break;
              case GL_INTENSITY:
                  current->env = __glTextureBlendI;
                break;
            }
            break;
          case GL_REPLACE:
            switch (baseFormat) {
              case GL_LUMINANCE:
                  current->env = __glTextureReplaceL;
                break;
              case GL_LUMINANCE_ALPHA:
                  current->env = __glTextureReplaceLA;
                break;
              case GL_RGB:
                  current->env = __glTextureReplaceRGB;
                break;
              case GL_RGBA:
                  current->env = __glTextureReplaceRGBA;
                break;
              case GL_ALPHA:
                  current->env = __glTextureReplaceA;
                break;
              case GL_INTENSITY:
                  current->env = __glTextureReplaceI;
                break;
              case GL_COLOR_INDEX:
                  current->env = __glTextureReplaceCI;
                break;
            }
            break;
          case GL_ADD:
            switch (baseFormat) {
              case GL_COLOR_INDEX:
                current->env = __glTextureAddCI;
                break;
            }
            break;
        }

        /* Pick mag/min functions */
        switch (current->dim) {
          case 1:
            current->nearest = __glNearestFilter1;
            current->linear = __glLinearFilter1;
            break;
          case 2:
            current->nearest = __glNearestFilter2;
            current->linear = __glLinearFilter2;
            break;
        }

        /* set min filter function */
        switch (current->params.minFilter) {
          case GL_LINEAR:
            gc->texture.interpUVScaled[gc->texture.currentTexUnit] = GL_TRUE;
            current->minnify = __glLinearFilterUVScaled;
            break;
          case GL_NEAREST:
            gc->texture.interpUVScaled[gc->texture.currentTexUnit] = GL_TRUE;
            current->minnify = __glNearestFilterUVScaled;
            break;
          case GL_NEAREST_MIPMAP_NEAREST:
            gc->texture.interpUVScaled[gc->texture.currentTexUnit] = GL_FALSE;
            current->minnify = __glNMNFilter;
            break;
          case GL_LINEAR_MIPMAP_NEAREST:
            gc->texture.interpUVScaled[gc->texture.currentTexUnit] = GL_FALSE;
            current->minnify = __glLMNFilter;
            break;
          case GL_NEAREST_MIPMAP_LINEAR:
            gc->texture.interpUVScaled[gc->texture.currentTexUnit] = GL_FALSE;
            current->minnify = __glNMLFilter;
            break;
          case GL_LINEAR_MIPMAP_LINEAR:
            gc->texture.interpUVScaled[gc->texture.currentTexUnit] = GL_FALSE;
            current->minnify = __glLMLFilter;
            break;
        }

        /* set mag filter function */
        switch (current->params.magFilter) {
          case GL_LINEAR:
            if (gc->texture.interpUVScaled[gc->texture.currentTexUnit]) {
                current->magnify = __glLinearFilterUVScaled;
            } else {
                current->magnify = __glLinearFilter;
            }
            break;
          case GL_NEAREST:
            if (gc->texture.interpUVScaled[gc->texture.currentTexUnit]) {
                current->magnify = __glNearestFilterUVScaled;
            } else {
                current->magnify = __glNearestFilter;
            }
            break;
        }

        gc->procs.texture = current->textureFunc;
        if (gc->texture.interpUVScaled[gc->texture.currentTexUnit]) {
            gc->procs.textureRaster = __glTextureFragmentUVScale;
        } else {
            gc->procs.textureRaster = current->textureFunc;
        }
    } else {
        gc->procs.texture = 0;
        gc->procs.textureRaster = 0;
    }
}

void __glGenericPickFogProcs(__GLcontext *gc)
{
    if (gc->state.enables.general & __GL_FOG_ENABLE) {
        if (gc->state.hints.fog == GL_NICEST) {
            gc->procs.fogVertex = 0;    /* Better not be called */
        } else {
            if (gc->state.fog.mode == GL_LINEAR) 
                gc->procs.fogVertex = __glFogVertexLinear;
            else
                gc->procs.fogVertex = __glFogVertex;
        }
        gc->procs.fogPoint = __glFogFragmentSlow;
        gc->procs.fogColor = __glFogColorSlow;
    } else {
        gc->procs.fogVertex = 0;
        gc->procs.fogPoint = 0;
        gc->procs.fogColor = 0;
    }
}

void __glGenericPickBufferProcs(__GLcontext *gc)
{
    __GLbufferMachine *buffers;

    buffers = &gc->buffers;
    buffers->readbuffer = NULL;
    buffers->writebuffer = NULL;
    buffers->depthbuffer = NULL;
    buffers->auxbuffer = NULL;

    /* Set draw buffer pointer */
    switch (gc->state.raster.drawBuffer) {
      case GL_FRONT:
        gc->drawBuffer = gc->front;
        buffers->writebuffer = &gc->front->buf;
        buffers->readbuffer = &gc->front->buf;
        break;
      case GL_FRONT_AND_BACK:
        if (gc->modes.doubleBufferMode) {
/*****  XXXXXXXXXXXXX

	We can't really do front and back rendering with our hardware. 
	If we decide that this is important, we need to snarf back these
	buffers to memory and operate on them.

            gc->drawBuffer = gc->back;
            buffers->writebuffer = &gc->back->buf;
            buffers->readbuffer = &gc->front->buf;
            buffers->doubleStore = GL_TRUE;
******/
        } else {
            gc->drawBuffer = gc->front;
            buffers->writebuffer = &gc->front->buf;
            buffers->readbuffer = &gc->front->buf;
        }
        break;
      case GL_BACK:
        gc->drawBuffer = gc->back;
        buffers->writebuffer = &gc->back->buf;
        buffers->readbuffer = &gc->back->buf;
        break;
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3: 
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
        {
            GLint i = gc->state.raster.drawBuffer - GL_AUX0;
            gc->drawBuffer = &gc->auxBuffer[i];
            buffers->auxbuffer = &gc->drawable->buf;
        }
#endif
        break;
    }

    if (gc->state.raster.drawBuffer != gc->state.pixel.readBuffer) {
        switch (gc->state.pixel.readBuffer) {
          case GL_FRONT:
            gc->readBuffer = gc->front;
            break;
	  case GL_BACK:
            gc->readBuffer = gc->back;
            break;
        } /* all the cases that we will support this release */
     }

#if 1
    if (gc->polygon.shader.modeFlags & __GL_SHADE_DEPTH_TEST) {
#else
	if (gc->modes.haveDepthBuffer) {
#endif
        
        buffers->depthbuffer = &gc->depthBuffer.buf;
    }

    /* Set lockBuffers proc and buffer pointer */

    if ((gc->front->buf.drawableBuf->lock) || 
                        (gc->back->buf.drawableBuf->lock)){
        gc->procs.lockBuffers = __glLockBuffer;
        gc->procs.unlockBuffers = __glUnlockBuffer;
    } else {
        gc->procs.lockBuffers = NULL;
        gc->procs.unlockBuffers = NULL;
        gc->buffers.readbuffer = NULL;
        gc->buffers.writebuffer = NULL;
        gc->buffers.depthbuffer = NULL;
        gc->buffers.auxbuffer = NULL;
    }
}

void __glGenericPickPixelProcs(__GLcontext *gc)
{
    __GLpixelTransferMode *tm;
    __GLpixelMachine *pm;
    GLboolean mapColor;
    GLfloat red, green, blue, alpha;
    GLint entry;
    GLuint enables = gc->state.enables.general;
    __GLpixelMapHead *pmap;

    /* Set read buffer pointer */
    switch (gc->state.pixel.readBuffer) {
      case GL_FRONT:
        gc->readBuffer = gc->front;
        break;
      case GL_BACK:
        gc->readBuffer = gc->back;
        break;
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
        {
            GLint i = gc->state.pixel.readBuffer - GL_AUX0;
            gc->readBuffer = &gc->auxBuffer[i];
        }
#endif
        break;
    }

    if (gc->texture.textureEnabled
            || (enables & __GL_FOG_ENABLE)) {
        gc->procs.pxStore = __glSlowDrawPixelsStore;
    } else {
        gc->procs.pxStore = gc->procs.store;
    }

    tm = &gc->state.pixel.transferMode;
    pm = &(gc->pixel);
    mapColor = tm->mapColor;
    if (mapColor || gc->modes.rgbMode || tm->indexShift || tm->indexOffset) {
        pm->iToICurrent = GL_FALSE;
        pm->iToRGBACurrent = GL_FALSE;
        pm->modifyCI = GL_TRUE;
    } else {
        pm->modifyCI = GL_FALSE;
    }
    if (tm->mapStencil || tm->indexShift || tm->indexOffset) {
        pm->modifyStencil = GL_TRUE;
    } else {
        pm->modifyStencil = GL_FALSE;
    }
    if (tm->d_scale != __glOne || tm->d_bias) {
        pm->modifyDepth = GL_TRUE;
    } else {
        pm->modifyDepth = GL_FALSE;
    }
    if (mapColor || tm->r_bias || tm->g_bias || tm->b_bias || tm->a_bias ||
        tm->r_scale != __glOne || tm->g_scale != __glOne ||
        tm->b_scale != __glOne || tm->a_scale != __glOne) {
        pm->modifyRGBA = GL_TRUE;
        pm->rgbaCurrent = GL_FALSE;
    } else {
        pm->modifyRGBA = GL_FALSE;
    }

    if (pm->modifyRGBA) {
        /* Compute default values for red, green, blue, alpha */
        red = gc->state.pixel.transferMode.r_bias;
        green = gc->state.pixel.transferMode.g_bias;
        blue = gc->state.pixel.transferMode.b_bias;
        alpha = gc->state.pixel.transferMode.a_scale +
            gc->state.pixel.transferMode.a_bias;
        if (mapColor) {
            pmap = 
                &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_R_TO_R];
            entry = (GLint)(red * (pmap->size-1) + __glHalf);
            if (entry < 0) entry = 0;
            else if (entry > pmap->size-1) entry = pmap->size-1;
            red = pmap->base.mapF[entry];

            pmap = 
                &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_G_TO_G];
            entry = (GLint)(green * (pmap->size-1) + __glHalf);
            if (entry < 0) entry = 0;
            else if (entry > pmap->size-1) entry = pmap->size-1;
            green = pmap->base.mapF[entry];

            pmap = 
                &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_B_TO_B];
            entry = (GLint)(blue * (pmap->size-1) + __glHalf);
            if (entry < 0) entry = 0;
            else if (entry > pmap->size-1) entry = pmap->size-1;
            blue = pmap->base.mapF[entry];

            pmap = 
                &gc->state.pixel.pixelMap[__GL_PIXEL_MAP_A_TO_A];
            entry = (GLint)(alpha * (pmap->size-1) + __glHalf);
            if (entry < 0) entry = 0;
            else if (entry > pmap->size-1) entry = pmap->size-1;
            alpha = pmap->base.mapF[entry];
        } else {
            if (red > __glOne) red = __glOne;
            else if (red < 0) red = 0;
            if (green > __glOne) green = __glOne;
            else if (green < 0) green = 0;
            if (blue > __glOne) blue = __glOne;
            else if (blue < 0) blue = 0;
            if (alpha > __glOne) alpha = __glOne;
            else if (alpha < 0) alpha = 0;
        }
        pm->red0Mod = red * gc->frontBuffer.redScale;
        pm->green0Mod = green * gc->frontBuffer.greenScale;
        pm->blue0Mod = blue * gc->frontBuffer.blueScale;
        pm->alpha1Mod = alpha * gc->frontBuffer.alphaScale;
    } else {
        pm->red0Mod = __glZero;
        pm->green0Mod = __glZero;
        pm->blue0Mod = __glZero;
        pm->alpha1Mod = gc->frontBuffer.alphaScale;
    }

    if ((enables & __GL_ALPHA_TEST_ENABLE) || 
        (enables & __GL_STENCIL_TEST_ENABLE) ||
        (enables & __GL_DEPTH_TEST_ENABLE) || 
        gc->state.raster.drawBuffer == GL_NONE ||
        gc->state.raster.drawBuffer == GL_FRONT_AND_BACK ||
        !(enables & __GL_DITHER_ENABLE) ||
        (enables & __GL_BLEND_ENABLE) ||
        gc->texture.textureEnabled ||
        (enables & __GL_FOG_ENABLE)) {
        pm->fastRGBA = GL_FALSE;
    } else {
        pm->fastRGBA = GL_TRUE;
    }

    gc->procs.drawPixels = __glOptPickDrawPixels;
    gc->procs.readPixels = __glSlowPickReadPixels;
    gc->procs.copyPixels = __glSlowPickCopyPixels;
}

void __glGenericPickTransformProcs(__GLcontext *gc)
{
    switch (gc->state.transform.matrixMode) {
      case GL_MODELVIEW:
        gc->procs.pushMatrix = __glPushModelViewMatrix;
        gc->procs.popMatrix = __glPopModelViewMatrix;
        gc->procs.loadIdentity = __glLoadIdentityModelViewMatrix;
        break;
      case GL_PROJECTION:
        gc->procs.pushMatrix = __glPushProjectionMatrix;
        gc->procs.popMatrix = __glPopProjectionMatrix;
        gc->procs.loadIdentity = __glLoadIdentityProjectionMatrix;
        break;
      case GL_TEXTURE:
        gc->procs.pushMatrix = __glPushTextureMatrix;
        gc->procs.popMatrix = __glPopTextureMatrix;
        gc->procs.loadIdentity = __glLoadIdentityTextureMatrix;
        break;
    }
    /* shui - this if test was not in original sgi code */
    if (gc->state.enables.general & __GL_CULL_VERTEX_ENABLE) {
        (gc->procs.pickCullVertexProcs)(gc, gc->transform.modelView);
    }
}

/*
** pick the depth function pointers
*/
int __glGenericPickDepthProcs(__GLcontext *gc)
{
    extern void __glValidateZCount(__GLdepthBuffer *);
    GLint depthIndex;

    __glValidateZCount(&gc->depthBuffer);

    depthIndex = gc->depthBuffer.testFunc;

    depthIndex -= GL_NEVER;

    if ( gc->state.depth.writeEnable == GL_FALSE ) {
        depthIndex += 8;
    }

    if ( gc->depthBuffer.buf.depth > 16 ) {
        depthIndex += 16;
    }

    if (gc->modes.haveDepthBuffer) {
        (*gc->depthBuffer.pick)(gc, &gc->depthBuffer, depthIndex);
    }

    gc->procs.DTPixel = __glCDTPixel[depthIndex];
#ifdef __GL_USE_MIPSASMCODE
    gc->procs.span.depthTestPixel = __glSDepthTestPixel[depthIndex];
    gc->procs.line.depthTestPixel = __glPDepthTestPixel[depthIndex];
#endif
    return depthIndex;
}


void __glGenericValidate(__GLcontext *gc)
{
    (*gc->procs.pickAllProcs)(gc);
}

void __glGenericPickAllProcs(__GLcontext *gc)
{
    GLuint enables = gc->state.enables.general;
    GLuint texenables = gc->state.enables.texture[gc->texture.currentTexUnit];
    GLuint modeFlags = 0;

    __glPickVertexShape(gc);

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_VERTARRAY)) {
        __glGenericPickVertexArrayEnables(gc);

        if ((gc->dirtyMask == __GL_DIRTY_VERTARRAY) && !gc->validateMask) {
            gc->dirtyMask = 0;
            return;
        }
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_LIGHTING)) {
        /* 
        ** Set textureEnabled flag early on, so we can set modeFlags
        ** based upon it.
        */
        (*gc->procs.pickTextureProcs)(gc);
        gc->texture.textureEnabled =
            (GLboolean) (gc->texture.currentTexture[0] != NULL) ||
                        (gc->texture.currentTexture[1] != NULL);
    }

    /* Compute shading mode flags before triangle, span, and line picker */
    if (gc->modes.rgbMode) {
        modeFlags |= __GL_SHADE_RGB;
        if (enables & __GL_COLOR_LOGIC_OP_ENABLE) {
            modeFlags |= __GL_SHADE_LOGICOP;
        } else if (enables & __GL_BLEND_ENABLE) {
            modeFlags |= __GL_SHADE_BLEND;
        }
        if (enables & __GL_ALPHA_TEST_ENABLE) {
            modeFlags |= __GL_SHADE_ALPHA_TEST;
        }
        if (!gc->state.raster.rMask ||
            !gc->state.raster.gMask ||
            !gc->state.raster.bMask ||
            (!gc->state.raster.aMask && gc->modes.alphaBits)) {
            modeFlags |= __GL_SHADE_MASK;
        }
    } else {
        if (enables & __GL_INDEX_LOGIC_OP_ENABLE) {
            modeFlags |= __GL_SHADE_LOGICOP;
        }
        if (gc->state.raster.writeMask != __GL_MASK_INDEXI(gc, ~0)) {
            modeFlags |= __GL_SHADE_MASK;
        }
        if (enables & __GL_INDEX_TEST_ENABLE) {
            modeFlags |= __GL_SHADE_INDEX_TEST;
        }
    }
    if (gc->texture.textureEnabled) {
        modeFlags |= __GL_SHADE_TEXTURE;
        if (gc->texture.interpUVScaled) {
            modeFlags |= __GL_SHADE_TEXTURE_UVSCALED;
        }
        /* XXX the triangle rasterization code assumes perspective
         * correction if rho interpolation is required.
         */
        if (gc->state.hints.perspectiveCorrection != GL_FASTEST ||
            gc->texture.currentTexture[gc->texture.currentTexUnit]->params.minFilter !=
            gc->texture.currentTexture[gc->texture.currentTexUnit]->params.magFilter) {
            modeFlags |= __GL_SHADE_TEXTURE_PERSP;
        }
    }
    if (gc->state.light.shadingModel == GL_SMOOTH) {
        modeFlags |= __GL_SHADE_SMOOTH | __GL_SHADE_SMOOTH_LIGHT;
    }
    if ((enables & __GL_DEPTH_TEST_ENABLE) && 
        gc->modes.haveDepthBuffer) {
            modeFlags |= ( __GL_SHADE_DEPTH_TEST |  __GL_SHADE_DEPTH_ITER );
            if((enables & __GL_POLYGON_OFFSET_FILL_ENABLE) &&
               (gc->state.polygon.factor != 0.0 ||
                gc->state.polygon.units != 0.0)) {
                   modeFlags |= __GL_SHADE_POLYGON_OFFSET_FILL;
            }
    }
    if (enables & __GL_CULL_FACE_ENABLE) {
        modeFlags |= __GL_SHADE_CULL_FACE;
    }
    /* Only turn on dither for 8- and 16-bit framebuffers */
    if (enables & __GL_DITHER_ENABLE && gc->front->buf.elementSize < 3) {
        modeFlags |= __GL_SHADE_DITHER;
    }
    if (enables & __GL_POLYGON_STIPPLE_ENABLE) {
        modeFlags |= __GL_SHADE_STIPPLE;
    }
    if (enables & __GL_LINE_STIPPLE_ENABLE) {
        modeFlags |= __GL_SHADE_LINE_STIPPLE;
    }
    if ((enables & __GL_STENCIL_TEST_ENABLE) && 
            gc->modes.haveStencilBuffer) {
        modeFlags |= __GL_SHADE_STENCIL_TEST;
    }
    if ((enables & __GL_LIGHTING_ENABLE) && 
            gc->state.light.model.twoSided) {
        modeFlags |= __GL_SHADE_TWOSIDED;
    }

    if (enables & __GL_FOG_ENABLE) {
        /* Figure out type of fogging to do.  Try to do cheap fog */
        if ((!(modeFlags & __GL_SHADE_TEXTURE) ||
             (gc->state.texture[gc->texture.currentTexUnit].env[0].mode == GL_ADD)) &&
                (gc->state.hints.fog != GL_NICEST)) {
            /*
            ** Cheap fog can be done.  Now figure out which kind we
            ** will do.  If smooth shading, its easy - just change
            ** the calcColor proc (let the color proc picker do it).
            ** Otherwise, set has flag later on to use smooth shading
            ** to do flat shaded fogging.
            */
            modeFlags |= __GL_SHADE_CHEAP_FOG | __GL_SHADE_SMOOTH;
        } else {
            /* Use slowest fog mode */
            modeFlags |= __GL_SHADE_SLOW_FOG;
        }
    }

    if ((gc->state.raster.drawBuffer == GL_FRONT ||
         gc->state.raster.drawBuffer == GL_FRONT_AND_BACK) &&
        (gc->drawablePrivate->ownershipBuffer.update != NULL)) {
        modeFlags |= __GL_SHADE_OWNERSHIP_TEST;
    }

    gc->polygon.shader.modeFlags = modeFlags;

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_LIGHTING)) {
        GLuint needs;
        GLuint faceNeeds;

        /* Compute needs mask */
        faceNeeds = needs = 0;
        if (gc->texture.textureEnabled) {
            needs |= __GL_HAS_TEXTURE;
            if ((texenables & __GL_TEXTURE_GEN_S_ENABLE)) {
                switch (gc->state.texture[gc->texture.currentTexUnit].s.mode) {
                  case GL_EYE_LINEAR:
                    needs |= __GL_HAS_EYE;
                    break;
                  case GL_SPHERE_MAP:
                    needs |= __GL_HAS_EYE | __GL_HAS_NORMAL;
                    break;
                }
            }
            if ((texenables & __GL_TEXTURE_GEN_T_ENABLE)) {
                switch (gc->state.texture[gc->texture.currentTexUnit].t.mode) {
                  case GL_EYE_LINEAR:
                    needs |= __GL_HAS_EYE;
                    break;
                  case GL_SPHERE_MAP:
                    needs |= __GL_HAS_EYE | __GL_HAS_NORMAL;
                    break;
                }
            }
            if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
                needs |= __GL_HAS_WINDOW_TEXTURE;
            }
        }
        if (enables & __GL_LIGHTING_ENABLE) {
            faceNeeds |= __GL_HAS_NORMAL;
            if (gc->state.light.model.localViewer) {
                faceNeeds |= __GL_HAS_EYE;
            } else {
                GLint i;
                __GLlightSourceState *lss = &gc->state.light.source[0];

                for (i = 0; i < gc->constants.numberOfLights; i++, lss++)
                    if ((gc->state.enables.lights & (1<<i)) && 
                        (lss->positionEye.w != __glZero))
                    {
                        /* local light source enabled */
                        faceNeeds |= __GL_HAS_EYE;
                        break;
                    }
            }
        }
        if (enables & __GL_FOG_ENABLE) {
            /* Need z in eye coordinates for fog */
            needs |= __GL_HAS_EYE;

            /* Need fog value at vertex if not nicest */
            if (gc->state.hints.fog != GL_NICEST)
                needs |= __GL_HAS_FOG;
        }
        if (gc->state.enables.clipPlanes) {
            /* Clip with user planes in eye space! */
            needs |= __GL_HAS_EYE;
        }
        gc->vertex.needs = needs;
        gc->vertex.faceNeeds[__GL_FRONTFACE] = faceNeeds | needs;
        gc->vertex.faceNeeds[__GL_BACKFACE] = faceNeeds | needs;
        if ((enables & __GL_LIGHTING_ENABLE) || 
                (modeFlags & 
                (__GL_SHADE_CHEAP_FOG | __GL_SHADE_SMOOTH_LIGHT))) {
            gc->vertex.faceNeeds[__GL_FRONTFACE] |= __GL_HAS_FRONT_COLOR;
            if (gc->state.light.model.twoSided) {
                gc->vertex.faceNeeds[__GL_BACKFACE] |= __GL_HAS_BACK_COLOR;
            } else {
                gc->vertex.faceNeeds[__GL_BACKFACE] = 
                        gc->vertex.faceNeeds[__GL_FRONTFACE];
            }
        } 
        if ((gc->state.light.shadingModel == GL_SMOOTH) ||
            gc->vertexCache.vertexCacheEnabled) {
            gc->vertex.materialNeeds = 
                    (gc->vertex.faceNeeds[__GL_FRONTFACE] | 
                     gc->vertex.faceNeeds[__GL_BACKFACE]) & ~__GL_HAS_TEXTURE;
        } else {
            /* Need nothing if only the provoking vertex needs to be lit! */
            gc->vertex.materialNeeds = 0;
        }

        (*gc->front->pick)(gc, gc->front);
        if (gc->modes.doubleBufferMode) {
            (*gc->back->pick)(gc, gc->back);
        }
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
        {
            GLint i;
            for (i = 0; i < gc->modes.numAuxBuffers; i++) {
                (*gc->auxBuffer[i].pick)(gc, &gc->auxBuffer[i]);
            }
        }
#endif
        if (gc->modes.haveStencilBuffer) {
            (*gc->stencilBuffer.pick)(gc, &gc->stencilBuffer);
        }
        (*gc->procs.pickBufferProcs)(gc);

        /* 
        ** Note: Must call gc->front->pick and gc->back->pick before calling
        ** pickStoreProcs.  This also must be called prior to line, point, 
        ** polygon, clipping, or bitmap pickers.  The LIGHT implementation
        ** depends upon it.
        */
        (*gc->procs.pickStoreProcs)(gc);

        (*gc->procs.pickTransformProcs)(gc);

        __glValidateLighting(gc);

        /*
        ** Note: pickColorMaterialProcs is called frequently outside of this
        ** generic picking routine.
        */
        (*gc->procs.pickColorMaterialProcs)(gc);

        (*gc->procs.pickCalcTextureProcs)(gc);

        (*gc->procs.pickBlendProcs)(gc);
        (*gc->procs.pickFogProcs)(gc);

        (*gc->procs.pickParameterClipProcs)(gc);
        (*gc->procs.pickClipProcs)(gc);

        /*
        ** Needs to be done after pickStoreProcs.
        */
        (*gc->procs.pickRenderBitmapProcs)(gc);

        if (gc->validateMask & __GL_VALIDATE_ALPHA_FUNC) {
            __glValidateAlphaTest(gc);
        }

        if (gc->validateMask & __GL_VALIDATE_INDEX_FUNC) {
            __glValidateIndexTest(gc);
        }

        (*gc->procs.computeClipBox)(gc);

    }

    if (gc->dirtyMask & __GL_DIRTY_POLYGON_STIPPLE) {
        /*
        ** Usually, the polygon stipple is converted immediately after
        ** it is changed.  However, if the polygon stipple was changed
        ** when this context was the destination of a CopyContext, then
        ** the polygon stipple will be converted here.
        */
        (*gc->procs.convertPolygonStipple)(gc);
    }

    if (gc->dirtyMask & __GL_DIRTY_DEPTH) {
        (*gc->procs.pickDepthProcs)(gc);
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_POLYGON | 
            __GL_DIRTY_LIGHTING | __GL_DIRTY_DEPTH)) {
        /* 
        ** May be used for picking Rect() procs, need to check polygon 
        ** bit.  Must also be called after gc->vertex.needs is set.
        ** Needs to be called prior to point, line, and triangle pickers.
        ** Also needs to be called after the store procs picker is called.
        */
        (*gc->procs.pickVertexProcs)(gc);

        (*gc->procs.pickSpanProcs)(gc);
        (*gc->procs.pickTriangleProcs)(gc);
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_POINT |
            __GL_DIRTY_LIGHTING)) {
        (*gc->procs.pickPointProcs)(gc);
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_LINE |
            __GL_DIRTY_LIGHTING)) {
        (*gc->procs.pickLineProcs)(gc);
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_PIXEL | 
            __GL_DIRTY_LIGHTING)) {
        (*gc->procs.pickPixelProcs)(gc);
    }

    if (gc->dirtyMask & (__GL_DIRTY_GENERIC | __GL_DIRTY_LIGHTING)) {
        (*gc->procs.pickVertexArrayProcs)(gc);
    }

    /*
    ** Vertex cache stuff
    */
    __glPickVcacheProcs(gc);

    gc->validateMask = 0;
    gc->dirtyMask = 0;
}

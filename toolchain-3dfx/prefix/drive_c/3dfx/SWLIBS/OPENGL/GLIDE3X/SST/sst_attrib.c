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
#include <windows.h>
#include "context.h"
#include "global.h"
#include "g_imfncs.h"
#include "lighting.h"
#include "imports.h"
#include "image.h"
#include <glide.h>
#include "sst_globals.h"

#include "glnew.h"

// XXXwheeler: special glide3 extension for opengl
#define GR_AA_ORDERED_OGL               0x00010000
#define GR_AA_ORDERED_POINTS_OGL        GR_AA_ORDERED_OGL+1
#define GR_AA_ORDERED_LINES_OGL         GR_AA_ORDERED_OGL+2
#define GR_AA_ORDERED_TRIANGLES_OGL     GR_AA_ORDERED_OGL+3

void APIENTRY __glsstim_AlphaFunc(GLenum af, GLfloat ref)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if ((af < GL_NEVER) || (af > GL_ALWAYS)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_API_STATE();

    if (ref < __glZero) ref = __glZero;
    if (ref > __glOne) ref = __glOne;

    gc->state.raster.alphaFunction = af;
    gc->state.raster.alphaReference = ref;

    GL_CLAMP_255(ref);
    grAlphaTestReferenceValue((GrAlpha_t)ref);
    if (gc->state.enables.general & __GL_ALPHA_TEST_ENABLE) {
        grAlphaTestFunction(af - GL_NEVER);
    } else {
        grAlphaTestFunction(GR_CMP_ALWAYS);
    }
    __GL_DELAY_VALIDATE(gc);
    gc->validateMask |= __GL_VALIDATE_ALPHA_FUNC;
}

void APIENTRY __glsstim_BlendFunc(GLenum sf, GLenum df)
{
    GrAlphaBlendFnc_t grsf, grdf;
    __GL_SETUP_NOT_IN_BEGIN();

    switch (sf) {
      case GL_ZERO:
          grsf = GR_BLEND_ZERO;
          break;
      case GL_ONE:
          grsf = GR_BLEND_ONE;
          break;
      case GL_DST_COLOR:
          grsf = GR_BLEND_DST_COLOR;
          break;
      case GL_ONE_MINUS_DST_COLOR:
          grsf = GR_BLEND_ONE_MINUS_DST_COLOR;
          break;
      case GL_SRC_ALPHA:
          grsf = GR_BLEND_SRC_ALPHA;
          break;
      case GL_ONE_MINUS_SRC_ALPHA:
          grsf = GR_BLEND_ONE_MINUS_SRC_ALPHA;
          break;
      case GL_DST_ALPHA:
          if (gc->modes.alphaBits) {
	      grsf = GR_BLEND_DST_ALPHA;
          } else {
              grsf = GR_BLEND_ONE;
          }
          break;
      case GL_ONE_MINUS_DST_ALPHA:
          if (gc->modes.alphaBits) {
	      grsf = GR_BLEND_ONE_MINUS_DST_ALPHA;
          } else {
              grsf = GR_BLEND_ZERO;
          }
          break;
      case GL_SRC_ALPHA_SATURATE:
          if (gc->modes.alphaBits) {
	      grsf = GR_BLEND_ALPHA_SATURATE;
          } else {
              grsf = GR_BLEND_ZERO;
          }
          break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    switch (df) {
      case GL_ZERO:
          grdf = GR_BLEND_ZERO;
          break;
      case GL_ONE:
          grdf = GR_BLEND_ONE;
          break;
      case GL_SRC_COLOR:
          grdf = GR_BLEND_SRC_COLOR;
          break;
      case GL_ONE_MINUS_SRC_COLOR:
          grdf = GR_BLEND_ONE_MINUS_SRC_COLOR;
          break;
      case GL_SRC_ALPHA:
          grdf = GR_BLEND_SRC_ALPHA;
          break;
      case GL_ONE_MINUS_SRC_ALPHA:
          grdf = GR_BLEND_ONE_MINUS_SRC_ALPHA;
          break;
      case GL_DST_ALPHA:
          if (gc->modes.alphaBits) {
	      grdf = GR_BLEND_DST_ALPHA;
          } else {
              grdf = GR_BLEND_ONE;
          }
          break;
      case GL_ONE_MINUS_DST_ALPHA:
          if (gc->modes.alphaBits) {
	      grdf = GR_BLEND_ONE_MINUS_DST_ALPHA;
          } else {
              grdf = GR_BLEND_ZERO;
          }
          break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __GL_API_STATE();

    gc->state.raster.blendSrc = sf;
    gc->state.raster.blendDst = df;

    gc->grBlendSrc = grsf;
    gc->grBlendDst = grdf;
    if (gc->state.enables.general & __GL_BLEND_ENABLE) {
      grAlphaBlendFunction(grsf, grdf, GR_BLEND_ONE, GR_BLEND_ZERO);
    } else {
      grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                           GR_BLEND_ONE, GR_BLEND_ZERO);
    }
}

/* XXXshui nothing done yet */
void APIENTRY __glsstim_ClearAccum(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    __GLfloat minusOne;
    __GLfloat one;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_BLAND();

    minusOne = __glMinusOne;
    one = __glOne;
    if (r < minusOne) r = minusOne;
    if (r > one) r = one;
    if (g < minusOne) g = minusOne;
    if (g > one) g = one;
    if (b < minusOne) b = minusOne;
    if (b > one) b = one;
    if (a < minusOne) a = minusOne;
    if (a > one) a = one;

    gc->state.accum.clear.r = r;
    gc->state.accum.clear.g = g;
    gc->state.accum.clear.b = b;
    gc->state.accum.clear.a = a;

    __GL_DELAY_VALIDATE(gc);
}

/* XXXshui nothing done yet */
void APIENTRY __glsstim_ClearColor(GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    __GLfloat zero;
    __GLfloat one;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_BLAND();

    zero = (__GLfloat)__glZero;
    one = (__GLfloat)__glOne;
    if (r < zero) r = zero;
    if (r > one) r = one;
    if (g < zero) g = zero;
    if (g > one) g = one;
    if (b < zero) b = zero;
    if (b > one) b = one;
    if (a < zero) a = zero;
    if (a > one) a = one;

    gc->state.raster.clear.r = r;
    gc->state.raster.clear.g = g;
    gc->state.raster.clear.b = b;
    gc->state.raster.clear.a = a;
}

/* XXXshui nothing done yet */
void APIENTRY __glsstim_ClearDepth(GLdouble z)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_BLAND();

    if (z < 0) z = 0;
    if (z > 1) z = 1;
    gc->state.depth.clear = z;
    __GL_DELAY_VALIDATE(gc);
}

/* XXXshui nothing done yet */
void APIENTRY __glsstim_ColorMask(GLboolean r, GLboolean g, GLboolean b, GLboolean a)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    if (r == g && g == b) {
        grColorMask(r, a);
        gc->slowPath &= ~__GL_COLORMASK_SLOWPATH;
    } else {
        grColorMask(FXTRUE, FXTRUE);
        gc->slowPath |= __GL_COLORMASK_SLOWPATH;
    }

    gc->state.raster.rMask = r;
    gc->state.raster.gMask = g;
    gc->state.raster.bMask = b;
    gc->state.raster.aMask = a;

    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_CullFace(GLenum cfm)
{
    __GL_SETUP_NOT_IN_BEGIN();

    switch (cfm) {
    case GL_FRONT:
    case GL_BACK:
        break;
    case GL_FRONT_AND_BACK:
        /* XXXshui don't support this yet */
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __GL_API_STATE();

    gc->state.polygon.cull = cfm;
    __glSSTValidateCull(gc);
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
}

void APIENTRY __glsstim_DepthFunc(GLenum zf)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    if ((zf < GL_NEVER) || (zf > GL_ALWAYS)) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    gc->state.depth.testFunc = zf;
    grDepthBufferFunction(zf - GL_NEVER);
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);
}

void APIENTRY __glsstim_DepthMask(GLboolean enabled)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    gc->state.depth.writeEnable = enabled;
    if (gc->state.enables.general & __GL_DEPTH_TEST_ENABLE) {
        grDepthMask(enabled);
    }
}

void APIENTRY __glsstim_DrawBuffer(GLenum mode)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch (mode) {
      case GL_NONE:
        gc->state.raster.drawBuffer = GL_NONE;
        break;
      case GL_FRONT_RIGHT:
      case GL_BACK_RIGHT:
      case GL_RIGHT:
      not_supported_in_this_implementation:
        __glSetError(GL_INVALID_OPERATION);
        return;
      case GL_FRONT:
      case GL_FRONT_LEFT:
        gc->state.raster.drawBuffer = GL_FRONT;
        grRenderBuffer(GR_BUFFER_FRONTBUFFER);
        break;
      case GL_FRONT_AND_BACK: /* XXXshui not supported in hw */
      case GL_LEFT:
        if (!gc->modes.doubleBufferMode) {
            gc->state.raster.drawBuffer = GL_FRONT;
        } else {
            gc->state.raster.drawBuffer = GL_FRONT_AND_BACK;
        }
        break;
      case GL_BACK:
      case GL_BACK_LEFT:
        if (!gc->modes.doubleBufferMode) {
            goto not_supported_in_this_implementation;
        }
        gc->state.raster.drawBuffer = GL_BACK;
        grRenderBuffer(GR_BUFFER_BACKBUFFER);
        break;
      case GL_AUX0:
      case GL_AUX1:
      case GL_AUX2:
      case GL_AUX3:
        {
            GLint i = mode - GL_AUX0;
            if (i >= gc->modes.numAuxBuffers) {
                goto not_supported_in_this_implementation;
            }
            gc->state.raster.drawBuffer = mode;
        }
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    gc->state.raster.drawBufferReturn = mode;
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_FrontFace(GLenum dir)
{
    __GL_SETUP_NOT_IN_BEGIN();

    switch (dir) {
    case GL_CW:
    case GL_CCW:
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __GL_API_STATE();

    gc->state.polygon.frontFaceDirection = dir;
    __glSSTValidateCull(gc);
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
}

/************************************************************************/

/* XXXshui nothing done yet */
void APIENTRY __glsstim_PushAttrib(GLuint mask)
{
    __GLattribute **spp;
    __GLattribute *sp;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_BLAND();

    spp = gc->attributes.stackPointer;
    if (spp < &gc->attributes.stack[gc->constants.maxAttribStackDepth]) {
        if (!(sp = *spp)) {
            sp = (__GLattribute*)
                (*gc->imports.calloc)(gc, 1, sizeof(__GLattribute));
            *spp = sp;
        }
        sp->mask = mask;
        sp->enables = gc->state.enables;        /* Always save enables */
        gc->attributes.stackPointer = spp + 1;

        if (mask & GL_ACCUM_BUFFER_BIT) {
            sp->accum = gc->state.accum;
        }
        if (mask & GL_COLOR_BUFFER_BIT) {
            sp->raster = gc->state.raster;
        }
        if (mask & GL_CURRENT_BIT) {
            sp->current = gc->state.current;
        }
        if (mask & GL_DEPTH_BUFFER_BIT) {
            sp->depth = gc->state.depth;
        }
        if (mask & GL_EVAL_BIT) {
            sp->evaluator = gc->state.evaluator;
        }
        if (mask & GL_FOG_BIT) {
            sp->fog = gc->state.fog;
        }
        if (mask & GL_HINT_BIT) {
            sp->hints = gc->state.hints;
        }
        if (mask & GL_LIGHTING_BIT) {
            size_t bytes = (size_t)
                (gc->constants.numberOfLights * sizeof(__GLlightSourceState));
            sp->light.colorMaterialFace = gc->state.light.colorMaterialFace;
            sp->light.colorMaterialParam = gc->state.light.colorMaterialParam;
            sp->light.indexMaterialFace = gc->state.light.indexMaterialFace;
            sp->light.indexMaterialParam = gc->state.light.indexMaterialParam;
            sp->light.shadingModel = gc->state.light.shadingModel;
            sp->light.model = gc->state.light.model;
            sp->light.front = gc->state.light.front;
            sp->light.back = gc->state.light.back;
            sp->light.source = (__GLlightSourceState*)
                (*gc->imports.malloc)(gc, bytes);
            __GL_MEMCOPY(sp->light.source, gc->state.light.source, bytes);
        }
        if (mask & GL_LINE_BIT) {
            sp->line = gc->state.line;
        }
        if (mask & GL_LIST_BIT) {
            sp->list = gc->state.list;
        }
        if (mask & GL_PIXEL_MODE_BIT) {
            sp->pixel.readBuffer = gc->state.pixel.readBuffer;
            sp->pixel.readBufferReturn = gc->state.pixel.readBufferReturn;
            sp->pixel.transferMode = gc->state.pixel.transferMode;
        }
        if (mask & GL_POINT_BIT) {
            sp->point = gc->state.point;
        }
        if (mask & GL_POLYGON_BIT) {
            sp->polygon = gc->state.polygon;
        }
        if (mask & GL_POLYGON_STIPPLE_BIT) {
            sp->polygonStipple = gc->state.polygonStipple;
        }
        if (mask & GL_SCISSOR_BIT) {
            sp->scissor = gc->state.scissor;
        }
        if (mask & GL_STENCIL_BUFFER_BIT) {
            sp->stencil = gc->state.stencil;
        }
        if (mask & GL_TEXTURE_BIT) {
            size_t texbytes = (size_t) (gc->constants.numberOfTextures
                                        * sizeof(__GLperTextureState));
            size_t envbytes = (size_t) (gc->constants.numberOfTextureEnvs
                                        * sizeof(__GLtextureEnvState));
            sp->texture[0].s = gc->state.texture[0].s;
            sp->texture[0].t = gc->state.texture[0].t;
            sp->texture[0].r = gc->state.texture[0].r;
            sp->texture[0].q = gc->state.texture[0].q;

            sp->texture[0].scale[0] = gc->state.texture[0].scale[0];
            sp->texture[0].scale[1] = gc->state.texture[0].scale[1];
            sp->texture[0].scale[2] = gc->state.texture[0].scale[2];
            sp->texture[0].scale[3] = gc->state.texture[0].scale[3];

            sp->texture[0].bias[0] = gc->state.texture[0].bias[0];
            sp->texture[0].bias[1] = gc->state.texture[0].bias[1];
            sp->texture[0].bias[2] = gc->state.texture[0].bias[2];
            sp->texture[0].bias[3] = gc->state.texture[0].bias[3];

            sp->texture[0].texture = (__GLperTextureState*)
                (*gc->imports.malloc)(gc, texbytes);
            __GL_MEMCOPY(sp->texture[0].texture, gc->state.texture[0].texture,
                         texbytes);
            sp->texture[0].env = (__GLtextureEnvState*)
                (*gc->imports.malloc)(gc, envbytes);
            __GL_MEMCOPY(sp->texture[0].env, gc->state.texture[0].env, envbytes);
        }
        if (mask & GL_TRANSFORM_BIT) {
            size_t bytes = (size_t)
                (gc->constants.numberOfClipPlanes * sizeof(__GLcoord));
            sp->transform.matrixMode = gc->state.transform.matrixMode;
            sp->transform.eyeClipPlanes = (__GLcoord*)
                (*gc->imports.malloc)(gc, bytes);
            __GL_MEMCOPY(sp->transform.eyeClipPlanes,
                         gc->state.transform.eyeClipPlanes, bytes);
        }
        if (mask & GL_VIEWPORT_BIT) {
            sp->viewport = gc->state.viewport;
        }
    } else {
        __glSetError(GL_STACK_OVERFLOW);
        return;
    }
}

/************************************************************************/

static void __glSSTSetSlowPath(__GLcontext *gc) {
    if (gc->state.enables.general & __GL_FOG_ENABLE) { 
	gc->slowPath |= __GL_FOG_SLOWPATH;

    } else {
	gc->slowPath &= ~__GL_FOG_SLOWPATH;
    }
    if (gc->state.enables.general & __GL_LIGHTING_ENABLE) { 
	gc->slowPath |= __GL_LIGHTING_SLOWPATH;
    } else {
	gc->slowPath &= ~__GL_LIGHTING_SLOWPATH;
    }
    if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) { 
	gc->slowPath |= __GL_SMOOTH_SLOWPATH;
    } else {
	gc->slowPath &= ~__GL_SMOOTH_SLOWPATH;
    }
    if (gc->state.enables.general & __GL_POLYGON_STIPPLE_ENABLE) { 
	gc->slowPath |= __GL_STIPPLE_SLOWPATH;
    } else {
	gc->slowPath &= ~__GL_STIPPLE_SLOWPATH;
    }
    if (gc->state.enables.general & __GL_STENCIL_TEST_ENABLE) { 
	gc->slowPath |= __GL_STENCIL_SLOWPATH;
    } else {
	gc->slowPath &= ~__GL_STENCIL_SLOWPATH;
    }
    if (gc->state.enables.clipPlanes) {
        gc->slowPath |= __GL_CLIP_SLOWPATH;
    } else {
        gc->slowPath &= ~__GL_CLIP_SLOWPATH;
    }
    if ((gc->state.polygon.frontMode == gc->state.polygon.backMode) &&
        (gc->state.polygon.frontMode == GL_FILL)) {
        gc->slowPath &= ~__GL_POLYMODE_SLOWPATH;
    } else {
        gc->slowPath |= __GL_POLYMODE_SLOWPATH;
    }
    if (gc->renderMode == GL_RENDER) {
        gc->slowPath &= ~__GL_RENDERMODE_SLOWPATH;
    } else {
        gc->slowPath |= __GL_RENDERMODE_SLOWPATH;
    }
    if (gc->state.enables.texture[0] & __GL_TEXGEN_ENABLES) {
	gc->slowPath |= __GL_TEXGEN_SLOWPATH_0;
    } else {
	gc->slowPath &= ~__GL_TEXGEN_SLOWPATH_0;
    }
    if (gc->transform.texture[0]->matrix.matrixType == __GL_MT_IDENTITY) {
        gc->slowPath &= ~__GL_TEXTURE_MATRIX_SLOWPATH_0;
    } else {
        gc->slowPath |= __GL_TEXTURE_MATRIX_SLOWPATH_0;
    }
    if (gc->state.raster.rMask == gc->state.raster.gMask &&
        gc->state.raster.gMask == gc->state.raster.bMask) {
        gc->slowPath &= ~__GL_COLORMASK_SLOWPATH;
    } else {
        gc->slowPath |= __GL_COLORMASK_SLOWPATH;
    }
    if (gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE) {
        grEnable(GR_AA_ORDERED_LINES_OGL);
	gc->glideLineFunc = gc->glideAALineFunc != NULL ? 
                                gc->glideAALineFunc : grDrawLine;
    } else {
        grDisable(GR_AA_ORDERED_LINES_OGL);
	gc->glideLineFunc = grDrawLine;
    }
    if (gc->state.enables.general & __GL_POINT_SMOOTH_ENABLE) {
        grEnable(GR_AA_ORDERED_POINTS_OGL);
    } else {
        grDisable(GR_AA_ORDERED_POINTS_OGL);
    }
}

/* XXXshui nothing done yet */
void APIENTRY __glsstim_PopAttrib(void)
{
    __GLattribute **spp;
    __GLattribute *sp;
    GLuint mask;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    spp = gc->attributes.stackPointer;
    if (spp > &gc->attributes.stack[0]) {
        --spp;
        sp = *spp;
        assert(sp != 0);
        mask = sp->mask;
        gc->attributes.stackPointer = spp;
        if (mask & GL_ACCUM_BUFFER_BIT) {
            gc->state.accum = sp->accum;
        }
        if (mask & GL_COLOR_BUFFER_BIT) {
            gc->state.raster = sp->raster;
            gc->state.enables.general &= ~__GL_COLOR_BUFFER_ENABLES;
            gc->state.enables.general |=
                sp->enables.general & __GL_COLOR_BUFFER_ENABLES;
            __glSSTValidateBlend(gc);
            __glSSTValidateAlphaTest(gc);
            __glSSTValidateAlphaFunc(gc);
        }
        if (mask & GL_CURRENT_BIT) {
            gc->state.current = sp->current;
        }
        if (mask & GL_DEPTH_BUFFER_BIT) {
            gc->state.depth = sp->depth;
            gc->state.enables.general &= ~__GL_DEPTH_TEST_ENABLE;
            gc->state.enables.general |=
                sp->enables.general & __GL_DEPTH_TEST_ENABLE;
            __glSSTValidateDepthTest(gc);
            __glSSTValidateDepthFunc(gc);
        }
        if (mask & GL_ENABLE_BIT) {
            gc->state.enables = sp->enables;
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LINE | __GL_DIRTY_POLYGON |
                    __GL_DIRTY_POINT | __GL_DIRTY_LIGHTING |
                    __GL_DIRTY_PIXEL | __GL_DIRTY_SCISSOR);
            (*gc->procs.pickColorMaterialProcs)(gc);
            (*gc->procs.applyColor)(gc);
            (*gc->procs.computeClipBox)(gc);
            (*gc->procs.applyScissor)(gc);
            __glSSTValidateDepthTest(gc);
            __glSSTValidateBlend(gc);
            __glSSTValidateAlphaTest(gc);
            __glSSTValidateCull(gc);
        }
        if (mask & GL_EVAL_BIT) {
            gc->state.evaluator = sp->evaluator;
            gc->state.enables.general &= ~__GL_AUTO_NORMAL_ENABLE;
            gc->state.enables.general |=
                sp->enables.general & __GL_AUTO_NORMAL_ENABLE;
            gc->state.enables.eval1 = sp->enables.eval1;
            gc->state.enables.eval2 = sp->enables.eval2;
        }
        if (mask & GL_FOG_BIT) {
            gc->state.fog = sp->fog;
            gc->state.enables.general &= ~__GL_FOG_ENABLE;
            gc->state.enables.general |=
                sp->enables.general & __GL_FOG_ENABLE;
        }
        if (mask & GL_HINT_BIT) {
            gc->state.hints = sp->hints;
        }
        if (mask & GL_LIGHTING_BIT) {
            gc->state.light.colorMaterialFace = sp->light.colorMaterialFace;
            gc->state.light.colorMaterialParam = sp->light.colorMaterialParam;
            gc->state.light.indexMaterialFace = sp->light.indexMaterialFace;
            gc->state.light.indexMaterialParam = sp->light.indexMaterialParam;
            gc->state.light.shadingModel = sp->light.shadingModel;
            gc->state.light.model = sp->light.model;
            gc->state.light.front = sp->light.front;
            gc->state.light.back = sp->light.back;
            __GL_MEMCOPY(gc->state.light.source, sp->light.source,
                         gc->constants.numberOfLights
                             * sizeof(__GLlightSourceState));
            (*gc->imports.free)(gc, sp->light.source);
            sp->light.source = 0;
            gc->state.enables.general &= ~__GL_LIGHTING_ENABLES;
            gc->state.enables.general |=
                sp->enables.general & __GL_LIGHTING_ENABLES;
            gc->state.enables.lights = sp->enables.lights;
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
        }
        if (mask & GL_LINE_BIT) {
            gc->state.line = sp->line;
            gc->state.enables.general &= ~__GL_LINE_ENABLES;
            gc->state.enables.general |=
                sp->enables.general & __GL_LINE_ENABLES;
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LINE);
        }
        if (mask & GL_LIST_BIT) {
            gc->state.list = sp->list;
        }
        if (mask & GL_PIXEL_MODE_BIT) {
            gc->state.pixel.transferMode = sp->pixel.transferMode;
            gc->state.pixel.readBufferReturn = sp->pixel.readBufferReturn;
            gc->state.pixel.readBuffer = sp->pixel.readBuffer;
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_PIXEL);
        }
        if (mask & GL_POINT_BIT) {
            gc->state.point = sp->point;
            gc->state.enables.general &= ~__GL_POINT_SMOOTH_ENABLE;
            gc->state.enables.general |=
                sp->enables.general & __GL_POINT_SMOOTH_ENABLE;
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POINT);
        }
        if (mask & GL_POLYGON_BIT) {
            gc->state.polygon = sp->polygon;
            gc->state.enables.general &= ~__GL_POLYGON_ENABLES;
            gc->state.enables.general |=
                sp->enables.general & __GL_POLYGON_ENABLES;
            __glSSTValidateCull(gc);
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
        }
        if (mask & GL_POLYGON_STIPPLE_BIT) {
            gc->state.polygonStipple = sp->polygonStipple;
            gc->state.enables.general &= ~__GL_POLYGON_STIPPLE_ENABLE;
            gc->state.enables.general |=
                sp->enables.general & __GL_POLYGON_STIPPLE_ENABLE;
            (*gc->procs.convertPolygonStipple)(gc);
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
        }
        if (mask & GL_SCISSOR_BIT) {
            gc->state.scissor = sp->scissor;
            gc->state.enables.general &= ~__GL_SCISSOR_TEST_ENABLE;
            gc->state.enables.general |=
                sp->enables.general & __GL_SCISSOR_TEST_ENABLE;
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_SCISSOR);
            (*gc->procs.computeClipBox)(gc);
            (*gc->procs.applyScissor)(gc);
        }
        if (mask & GL_STENCIL_BUFFER_BIT) {
            gc->state.stencil = sp->stencil;
            gc->state.enables.general &= ~__GL_STENCIL_TEST_ENABLE;
            gc->state.enables.general |=
                sp->enables.general & __GL_STENCIL_TEST_ENABLE;
            gc->validateMask |= __GL_VALIDATE_STENCIL_FUNC |
                __GL_VALIDATE_STENCIL_OP;/*XXX*/
        }
        if (mask & GL_TEXTURE_BIT) {
            GLuint numTextures = gc->constants.numberOfTextures;
            
            gc->state.texture[0].s = sp->texture[0].s;
            gc->state.texture[0].t = sp->texture[0].t;
            gc->state.texture[0].r = sp->texture[0].r;
            gc->state.texture[0].q = sp->texture[0].q;

            gc->state.texture[0].scale[0] = sp->texture[0].scale[0];
            gc->state.texture[0].scale[1] = sp->texture[0].scale[1];
            gc->state.texture[0].scale[2] = sp->texture[0].scale[2];
            gc->state.texture[0].scale[3] = sp->texture[0].scale[3];

            gc->state.texture[0].bias[0] = sp->texture[0].bias[0];
            gc->state.texture[0].bias[1] = sp->texture[0].bias[1];
            gc->state.texture[0].bias[2] = sp->texture[0].bias[2];
            gc->state.texture[0].bias[3] = sp->texture[0].bias[3];

            /*
            ** If the texture name is different, a new binding is
            ** called for.  Deferring the binding is dangerous, because
            ** the state before the pop has to be saved with the
            ** texture that is being unbound.  If we defer the binding,
            ** we need to watch out for cases like two pops in a row
            ** or a pop followed by a bind.
            */
            {
                GLuint targetIndex;
                __GLperTextureState *pts, *spPts;

                pts = gc->state.texture[0].texture;
                spPts = sp->texture[0].texture;
                for (targetIndex = 0; targetIndex < numTextures; 
                        targetIndex++, pts++, spPts++) {
                    if (pts->texobjs.name != spPts->texobjs.name) {
                        __glSSTBindTexture(gc, targetIndex, 
                                           spPts->texobjs.name, GL_FALSE);
                    }
                }
            }
            __GL_MEMCOPY(gc->state.texture[0].texture, sp->texture[0].texture,
                         numTextures * sizeof(__GLperTextureState));
            __GL_MEMCOPY(gc->state.texture[0].env, sp->texture[0].env,
                         gc->constants.numberOfTextureEnvs
                             * sizeof(__GLtextureEnvState));
            (*gc->imports.free)(gc, sp->texture[0].texture);
            sp->texture[0].texture = 0;
            (*gc->imports.free)(gc, sp->texture[0].env);
            sp->texture[0].env = 0;
            gc->state.enables.texture[0] &= ~__GL_TEXTURE_ENABLES;
            gc->state.enables.texture[0] |=
                sp->enables.texture[0] & __GL_TEXTURE_ENABLES;
            gc->validateTexture = 1;
        }
        if (mask & GL_TRANSFORM_BIT) {
            gc->state.transform.matrixMode = sp->transform.matrixMode;
            __GL_MEMCOPY(gc->state.transform.eyeClipPlanes,
                         sp->transform.eyeClipPlanes,
                         gc->constants.numberOfClipPlanes * sizeof(__GLcoord));
            (*gc->imports.free)(gc, sp->transform.eyeClipPlanes);
            sp->transform.eyeClipPlanes = 0;
            gc->state.enables.general &= ~__GL_NORMALIZE_ENABLE;
            gc->state.enables.general |=
                sp->enables.general & __GL_NORMALIZE_ENABLE;
            gc->state.enables.clipPlanes = sp->enables.clipPlanes;
        }
        if (mask & GL_VIEWPORT_BIT) {
            gc->state.viewport = sp->viewport;
            (*gc->procs.applyViewport)(gc);
            __glUpdateViewportTransform(gc);
        }

        /*
        ** Clear out mask so that any memory frees done above won't get
        ** re-done when the context is destroyed
        */
        sp->mask = 0;

        __GL_DELAY_VALIDATE(gc);
    } else {
        __glSetError(GL_STACK_UNDERFLOW);
        return;
    }
    __glSSTSetSlowPath(gc);
}

/*
** Free any attribute state left on the stack.  Stop at the first
** zero in the array.
*/
void __glSSTFreeAttributeState(__GLcontext *gc)
{
    __GLattribute *sp, **spp;

    /*
    ** Need to pop all pushed attributes to free storage.
    ** Then it will be safe to delete stack entries
    */
    for (spp = &gc->attributes.stack[0];
         spp < &gc->attributes.stack[gc->constants.maxAttribStackDepth];
         spp++) {
        if (sp = *spp) {
            if (sp->light.source) {
                (*gc->imports.free)(gc, sp->light.source);
            }
            if (sp->texture[0].texture) {
                (*gc->imports.free)(gc, sp->texture[0].texture);
            }
            if (sp->texture[0].env) {
                (*gc->imports.free)(gc, sp->texture[0].env);
            }
            if (sp->transform.eyeClipPlanes) {
                (*gc->imports.free)(gc, sp->transform.eyeClipPlanes);
            }
            (*gc->imports.free)(gc, sp);
        } else {
            break;
        }
    }
}

/************************************************************************/

void APIENTRY __glsstim_Enable(GLenum cap)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch (cap) {
      case GL_ALPHA_TEST:
        gc->state.enables.general |= __GL_ALPHA_TEST_ENABLE;
        grAlphaTestFunction(gc->state.raster.alphaFunction - GL_NEVER);
        break;
      case GL_BLEND:
        gc->state.enables.general |= __GL_BLEND_ENABLE;
        grAlphaBlendFunction(gc->grBlendSrc, gc->grBlendDst, 
                             GR_BLEND_ONE, GR_BLEND_ZERO);
        break;
      case GL_COLOR_MATERIAL:
        gc->state.enables.general |= __GL_COLOR_MATERIAL_ENABLE;
        (*gc->procs.pickColorMaterialProcs)(gc);
        (*gc->procs.applyColor)(gc);
        gc->dispatchState->color.Color3ub = __glim_Color3ub_CM;
        gc->dispatchState->color.Color3ubv = __glim_Color3ubv_CM;
        gc->dispatchState->color.Color3f = __glim_Color3f_CM;
        gc->dispatchState->color.Color3fv = __glim_Color3fv_CM;
        gc->dispatchState->color.Color4ub = __glim_Color4ub_CM;
        gc->dispatchState->color.Color4ubv = __glim_Color4ubv_CM;
        gc->dispatchState->color.Color4f = __glim_Color4f_CM;
        gc->dispatchState->color.Color4fv = __glim_Color4fv_CM;
        return;                         /* NOTE: return! */
      case GL_INDEX_MATERIAL_SGI:
        gc->state.enables.general |= __GL_INDEX_MATERIAL_ENABLE;
        break;
      case GL_CULL_FACE:
        if (gc->state.enables.general & __GL_CULL_FACE_ENABLE) return;
        gc->state.enables.general |= __GL_CULL_FACE_ENABLE;
        __glSSTValidateCull(gc);
        /* __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON); */
        return;
      case GL_DEPTH_TEST:
        gc->state.enables.general |= __GL_DEPTH_TEST_ENABLE;
        __glSSTValidateDepthTest(gc);
        break;
      case GL_DITHER:
        gc->state.enables.general |= __GL_DITHER_ENABLE;
        { extern int __r3d_nodither;
          grDitherMode(__r3d_nodither ? GR_DITHER_DISABLE : GR_DITHER_4x4); }
        break;
      case GL_FOG:
        gc->state.enables.general |= __GL_FOG_ENABLE;
        gc->ogKey |= __GL_GEOM_OG_KEY_FOG_ENABLE;       
    gc->slowPath |= __GL_FOG_SLOWPATH;
        break;
      case GL_INDEX_TEST_SGI:
        gc->state.enables.general |= __GL_INDEX_TEST_ENABLE;
        break;
      case GL_LIGHTING:
        gc->state.enables.general |= __GL_LIGHTING_ENABLE;
        gc->ogKey |= __GL_GEOM_OG_KEY_LIGHTING_ENABLE;  
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
        (*gc->procs.pickColorMaterialProcs)(gc);
        (*gc->procs.applyColor)(gc);
    gc->slowPath |= __GL_LIGHTING_SLOWPATH;
        return;
      case GL_LINE_SMOOTH:
        gc->state.enables.general |= __GL_LINE_SMOOTH_ENABLE;
        grEnable(GR_AA_ORDERED_LINES_OGL);
	gc->glideLineFunc = gc->glideAALineFunc != NULL ? gc->glideAALineFunc : grDrawLine;
        break;
      case GL_LINE_STIPPLE:
        gc->state.enables.general |= __GL_LINE_STIPPLE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LINE);
        return;
      case GL_INDEX_LOGIC_OP:
        gc->state.enables.general |= __GL_INDEX_LOGIC_OP_ENABLE;
        break;
      case GL_COLOR_LOGIC_OP:
        gc->state.enables.general |= __GL_COLOR_LOGIC_OP_ENABLE;
        break;
      case GL_NORMALIZE:
        gc->state.enables.general |= __GL_NORMALIZE_ENABLE;
        gc->ogKey |= __GL_GEOM_OG_KEY_NORMALIZE_ENABLE; 
        break;
      case GL_POINT_SMOOTH:
        gc->state.enables.general |= __GL_POINT_SMOOTH_ENABLE;
        grEnable(GR_AA_ORDERED_POINTS_OGL);
        break;
      case GL_POLYGON_SMOOTH:
        gc->state.enables.general |= __GL_POLYGON_SMOOTH_ENABLE;
    gc->slowPath |= __GL_SMOOTH_SLOWPATH;
        break;
      case GL_POLYGON_STIPPLE:
        gc->state.enables.general |= __GL_POLYGON_STIPPLE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
    gc->slowPath |= __GL_STIPPLE_SLOWPATH;
        return;
      case GL_SCISSOR_TEST:
        if (gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE) return;
        gc->state.enables.general |= __GL_SCISSOR_TEST_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_SCISSOR);
        (*gc->procs.computeClipBox)(gc);
        (*gc->procs.applyScissor)(gc);
        break;
      case GL_STENCIL_TEST:
        gc->state.enables.general |= __GL_STENCIL_TEST_ENABLE;
    gc->slowPath |= __GL_STENCIL_SLOWPATH;
        break;
      case GL_TEXTURE_1D:
        gc->state.enables.texture[gc->texture.currentTexUnit] |= __GL_TEXTURE_1D_ENABLE;
        break;
      case GL_TEXTURE_2D:
        gc->state.enables.texture[gc->texture.currentTexUnit] |= __GL_TEXTURE_2D_ENABLE;
        gc->validateTexture = 1;
        break;
      case GL_AUTO_NORMAL:
        gc->state.enables.general |= __GL_AUTO_NORMAL_ENABLE;
        break;
      case GL_TEXTURE_GEN_S:
        gc->state.enables.texture[gc->texture.currentTexUnit] |= __GL_TEXTURE_GEN_S_ENABLE;
    gc->slowPath |= __GL_TEXGEN_SLOWPATH_0;
        break;
      case GL_TEXTURE_GEN_T:
        gc->state.enables.texture[gc->texture.currentTexUnit] |= __GL_TEXTURE_GEN_T_ENABLE;
    gc->slowPath |= __GL_TEXGEN_SLOWPATH_0;
        break;
      case GL_TEXTURE_GEN_R:
        gc->state.enables.texture[gc->texture.currentTexUnit] |= __GL_TEXTURE_GEN_R_ENABLE;
    gc->slowPath |= __GL_TEXGEN_SLOWPATH_0;
        break;
      case GL_TEXTURE_GEN_Q:
        gc->state.enables.texture[gc->texture.currentTexUnit] |= __GL_TEXTURE_GEN_Q_ENABLE;
    gc->slowPath |= __GL_TEXGEN_SLOWPATH_0;
        break;

      case GL_CLIP_PLANE0: case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2: case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4: case GL_CLIP_PLANE5:
        cap -= GL_CLIP_PLANE0;
        gc->ogKey |= __GL_GEOM_OG_KEY_CLIPPLANE_ENABLE; 
        gc->state.enables.clipPlanes |= (1 << cap);
    gc->slowPath |= __GL_CLIP_SLOWPATH;
        break;
      case GL_LIGHT0: case GL_LIGHT1:
      case GL_LIGHT2: case GL_LIGHT3:
      case GL_LIGHT4: case GL_LIGHT5:
      case GL_LIGHT6: case GL_LIGHT7:
        cap -= GL_LIGHT0;
        gc->state.enables.lights |= (1 << cap);
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);

        return;
      case GL_MAP1_COLOR_4:
      case GL_MAP1_NORMAL:
      case GL_MAP1_INDEX:
      case GL_MAP1_TEXTURE_COORD_1: case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3: case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3: case GL_MAP1_VERTEX_4:
        cap = __GL_EVAL1D_INDEX(cap);
        gc->state.enables.eval1 |= (GLushort) (1 << cap);
        break;
      case GL_MAP2_COLOR_4:
      case GL_MAP2_NORMAL:
      case GL_MAP2_INDEX:
      case GL_MAP2_TEXTURE_COORD_1: case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3: case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3: case GL_MAP2_VERTEX_4:
        cap = __GL_EVAL2D_INDEX(cap);
        gc->state.enables.eval2 |= (GLushort) (1 << cap);
        break;
      case GL_POLYGON_OFFSET_POINT:
        gc->state.enables.general |= __GL_POLYGON_OFFSET_POINT_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POINT);
        break;
      case GL_POLYGON_OFFSET_LINE:
        gc->state.enables.general |= __GL_POLYGON_OFFSET_LINE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LINE);
        break;
      case GL_POLYGON_OFFSET_FILL:
        gc->state.enables.general |= __GL_POLYGON_OFFSET_FILL_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
        break;
      case GL_VERTEX_ARRAY_EXT:
        gc->ogKey |= __GL_GEOM_OG_KEY_VERTARRAY_ENABLE; 
        glEnableClientState(GL_VERTEX_ARRAY);
        return;
      case GL_NORMAL_ARRAY_EXT:
        glEnableClientState(GL_NORMAL_ARRAY);
        return;
      case GL_COLOR_ARRAY_EXT:
        glEnableClientState(GL_COLOR_ARRAY);
        return;
      case GL_INDEX_ARRAY_EXT:
        glEnableClientState(GL_INDEX_ARRAY);
        return;
      case GL_TEXTURE_COORD_ARRAY_EXT:
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        return;
      case GL_EDGE_FLAG_ARRAY_EXT:
        glEnableClientState(GL_EDGE_FLAG_ARRAY);
        return;
      case GL_CULL_VERTEX_SGI:
        gc->state.enables.general |= __GL_CULL_VERTEX_ENABLE;
        break;
          case GL_SHARED_TEXTURE_PALETTE_EXT:
    gc->state.enables.general |= __GL_SHARED_TEXTURE_PALETTE_EXT_ENABLE;
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_Disable(GLenum cap)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch (cap) {
      case GL_ALPHA_TEST:
        gc->state.enables.general &= ~__GL_ALPHA_TEST_ENABLE;
        grAlphaTestFunction(GR_CMP_ALWAYS);
        break;
      case GL_BLEND:
        gc->state.enables.general &= ~__GL_BLEND_ENABLE;
        grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, 
                             GR_BLEND_ONE, GR_BLEND_ZERO);
        break;
      case GL_COLOR_MATERIAL:
        gc->state.enables.general &= ~__GL_COLOR_MATERIAL_ENABLE;
        (*gc->procs.pickColorMaterialProcs)(gc);
        gc->dispatchState->color.Color3ub = __glim_Color3ub;
        gc->dispatchState->color.Color3ubv = __glim_Color3ubv;
        gc->dispatchState->color.Color3f = __glim_Color3f;
        gc->dispatchState->color.Color3fv = __glim_Color3fv;
        gc->dispatchState->color.Color4ub = __glim_Color4ub;
        gc->dispatchState->color.Color4ubv = __glim_Color4ubv;
        gc->dispatchState->color.Color4f = __glim_Color4f;
        gc->dispatchState->color.Color4fv = __glim_Color4fv;
        return;                         /* NOTE: return! */
      case GL_INDEX_MATERIAL_SGI:
        gc->state.enables.general &= ~__GL_INDEX_MATERIAL_ENABLE;
        break;
      case GL_CULL_FACE:
        if (!(gc->state.enables.general & __GL_CULL_FACE_ENABLE)) return;
        gc->state.enables.general &= ~__GL_CULL_FACE_ENABLE;
        __glSSTValidateCull(gc);
        /* __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON); */
        return;
      case GL_DEPTH_TEST:
        gc->state.enables.general &= ~__GL_DEPTH_TEST_ENABLE;
        grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
        grDepthMask(FXFALSE);
        break;
      case GL_DITHER:
        gc->state.enables.general &= ~__GL_DITHER_ENABLE;
        grDitherMode(GR_DITHER_DISABLE);
        break;
      case GL_FOG:
        gc->state.enables.general &= ~__GL_FOG_ENABLE;
        gc->ogKey &= ~__GL_GEOM_OG_KEY_FOG_ENABLE;      
    gc->slowPath &= ~__GL_FOG_SLOWPATH;
        break;
      case GL_INDEX_TEST_SGI:
        gc->state.enables.general &= ~__GL_INDEX_TEST_ENABLE;
        break;
      case GL_LIGHTING:
        gc->state.enables.general &= ~__GL_LIGHTING_ENABLE;
        gc->ogKey &= ~__GL_GEOM_OG_KEY_LIGHTING_ENABLE; 
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
        (*gc->procs.pickColorMaterialProcs)(gc);
        (*gc->procs.applyColor)(gc);
    gc->slowPath &= ~__GL_LIGHTING_SLOWPATH;
        return;
      case GL_LINE_SMOOTH:
        gc->state.enables.general &= ~__GL_LINE_SMOOTH_ENABLE;
        grDisable(GR_AA_ORDERED_LINES_OGL);
	gc->glideLineFunc = grDrawLine;
        break;
      case GL_LINE_STIPPLE:
        gc->state.enables.general &= ~__GL_LINE_STIPPLE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LINE);
        return;
      case GL_INDEX_LOGIC_OP:
        gc->state.enables.general &= ~__GL_INDEX_LOGIC_OP_ENABLE;
        break;
      case GL_COLOR_LOGIC_OP:
        gc->state.enables.general &= ~__GL_COLOR_LOGIC_OP_ENABLE;
        break;
      case GL_NORMALIZE:
        gc->state.enables.general &= ~__GL_NORMALIZE_ENABLE;
        gc->ogKey &= ~__GL_GEOM_OG_KEY_NORMALIZE_ENABLE;        
        break;
      case GL_POINT_SMOOTH:
        gc->state.enables.general &= ~__GL_POINT_SMOOTH_ENABLE;
        grDisable(GR_AA_ORDERED_POINTS_OGL);
        break;
      case GL_POLYGON_SMOOTH:
        gc->state.enables.general &= ~__GL_POLYGON_SMOOTH_ENABLE;
    gc->slowPath &= ~__GL_SMOOTH_SLOWPATH;
        break;
      case GL_POLYGON_STIPPLE:
        gc->state.enables.general &= ~__GL_POLYGON_STIPPLE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
    gc->slowPath &= ~__GL_STIPPLE_SLOWPATH;
        return;
      case GL_SCISSOR_TEST:
        gc->state.enables.general &= ~__GL_SCISSOR_TEST_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_SCISSOR);
        (*gc->procs.computeClipBox)(gc);
        (*gc->procs.applyScissor)(gc);
        break;
      case GL_STENCIL_TEST:
        gc->state.enables.general &= ~__GL_STENCIL_TEST_ENABLE;
    gc->slowPath &= ~__GL_STENCIL_SLOWPATH;
        break;
      case GL_TEXTURE_1D:
        gc->state.enables.texture[gc->texture.currentTexUnit] &= ~__GL_TEXTURE_1D_ENABLE;
        break;
      case GL_TEXTURE_2D:
        gc->state.enables.texture[gc->texture.currentTexUnit] &= ~__GL_TEXTURE_2D_ENABLE;
    gc->validateTexture = 1;
        break;
      case GL_AUTO_NORMAL:
        gc->state.enables.general &= ~__GL_AUTO_NORMAL_ENABLE;
        break;
      case GL_TEXTURE_GEN_S:
        gc->state.enables.texture[gc->texture.currentTexUnit] &= ~__GL_TEXTURE_GEN_S_ENABLE;
    if ((gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXGEN_ENABLES)==0)
        gc->slowPath &= ~__GL_TEXGEN_SLOWPATH_0;
        break;
      case GL_TEXTURE_GEN_T:
        gc->state.enables.texture[gc->texture.currentTexUnit] &= ~__GL_TEXTURE_GEN_T_ENABLE;
    if ((gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXGEN_ENABLES)==0)
        gc->slowPath &= ~__GL_TEXGEN_SLOWPATH_0;
        break;
      case GL_TEXTURE_GEN_R:
        gc->state.enables.texture[gc->texture.currentTexUnit] &= ~__GL_TEXTURE_GEN_R_ENABLE;
    if ((gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXGEN_ENABLES)==0)
        gc->slowPath &= ~__GL_TEXGEN_SLOWPATH_0;
        break;
      case GL_TEXTURE_GEN_Q:
        gc->state.enables.texture[gc->texture.currentTexUnit] &= ~__GL_TEXTURE_GEN_Q_ENABLE;
    if ((gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXGEN_ENABLES)==0)
        gc->slowPath &= ~__GL_TEXGEN_SLOWPATH_0;
        break;

      case GL_CLIP_PLANE0: case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2: case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4: case GL_CLIP_PLANE5:
        cap -= GL_CLIP_PLANE0;
        gc->state.enables.clipPlanes &= ~(1 << cap);
        if (!gc->state.enables.clipPlanes) {
            gc->ogKey &= ~__GL_GEOM_OG_KEY_CLIPPLANE_ENABLE;
        gc->slowPath &= ~__GL_CLIP_SLOWPATH;
        }
        break;
      case GL_LIGHT0: case GL_LIGHT1:
      case GL_LIGHT2: case GL_LIGHT3:
      case GL_LIGHT4: case GL_LIGHT5:
      case GL_LIGHT6: case GL_LIGHT7:
        cap -= GL_LIGHT0;
        gc->state.enables.lights &= ~(1 << cap);
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
        return;
      case GL_MAP1_COLOR_4:
      case GL_MAP1_NORMAL:
      case GL_MAP1_INDEX:
      case GL_MAP1_TEXTURE_COORD_1: case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3: case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3: case GL_MAP1_VERTEX_4:
        cap = __GL_EVAL1D_INDEX(cap);
        gc->state.enables.eval1 &= (GLushort) ~(1 << cap);
        break;
      case GL_MAP2_COLOR_4:
      case GL_MAP2_NORMAL:
      case GL_MAP2_INDEX:
      case GL_MAP2_TEXTURE_COORD_1: case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3: case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3: case GL_MAP2_VERTEX_4:
        cap = __GL_EVAL2D_INDEX(cap);
        gc->state.enables.eval2 &= (GLushort) ~(1 << cap);
        break;
      case GL_POLYGON_OFFSET_POINT:
        gc->state.enables.general &= ~__GL_POLYGON_OFFSET_POINT_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POINT);
        break;
      case GL_POLYGON_OFFSET_LINE:
        gc->state.enables.general &= ~__GL_POLYGON_OFFSET_LINE_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LINE);
        break;
      case GL_POLYGON_OFFSET_FILL:
        gc->state.enables.general &= ~__GL_POLYGON_OFFSET_FILL_ENABLE;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
        break;
      case GL_VERTEX_ARRAY_EXT:
        gc->ogKey &= ~__GL_GEOM_OG_KEY_VERTARRAY_ENABLE;        
        glDisableClientState(GL_VERTEX_ARRAY);
        return;
      case GL_NORMAL_ARRAY_EXT:
        glDisableClientState(GL_NORMAL_ARRAY);
        return;
      case GL_COLOR_ARRAY_EXT:
        glDisableClientState(GL_COLOR_ARRAY);
        return;
      case GL_INDEX_ARRAY_EXT:
        glDisableClientState(GL_INDEX_ARRAY);
        return;
      case GL_TEXTURE_COORD_ARRAY_EXT:
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        return;
      case GL_EDGE_FLAG_ARRAY_EXT:
        glDisableClientState(GL_EDGE_FLAG_ARRAY);
        return;
      case GL_CULL_VERTEX_SGI:
        gc->state.enables.general &= ~__GL_CULL_VERTEX_ENABLE;
        break;
          case GL_SHARED_TEXTURE_PALETTE_EXT:
    gc->state.enables.general &= ~__GL_SHARED_TEXTURE_PALETTE_EXT_ENABLE;
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __GL_DELAY_VALIDATE(gc);
}

GLboolean APIENTRY __glsstim_IsEnabled(GLenum cap)
{
    GLuint bit;
    __GL_SETUP_NOT_IN_BEGIN2();
    __GL_API_GET();

    switch (cap) {
      case GL_ALPHA_TEST:
        bit = gc->state.enables.general & __GL_ALPHA_TEST_ENABLE;
        break;
      case GL_BLEND:
        bit = gc->state.enables.general & __GL_BLEND_ENABLE;
        break;
      case GL_COLOR_MATERIAL:
        bit = gc->state.enables.general & __GL_COLOR_MATERIAL_ENABLE;
        break;
      case GL_INDEX_MATERIAL_SGI:
        bit = gc->state.enables.general & __GL_INDEX_MATERIAL_ENABLE;
        break;
      case GL_CULL_FACE:
        bit = gc->state.enables.general & __GL_CULL_FACE_ENABLE;
        break;
      case GL_DEPTH_TEST:
        bit = gc->state.enables.general & __GL_DEPTH_TEST_ENABLE;
        break;
      case GL_DITHER:
        bit = gc->state.enables.general & __GL_DITHER_ENABLE;
        break;
      case GL_FOG:
        bit = gc->state.enables.general & __GL_FOG_ENABLE;
        break;
      case GL_INDEX_TEST_SGI:
        bit = gc->state.enables.general & __GL_INDEX_TEST_ENABLE;
        break;
      case GL_LIGHTING:
        bit = gc->state.enables.general & __GL_LIGHTING_ENABLE;
        break;
      case GL_LINE_SMOOTH:

        bit = gc->state.enables.general & __GL_LINE_SMOOTH_ENABLE;
        break;
      case GL_LINE_STIPPLE:
        bit = gc->state.enables.general & __GL_LINE_STIPPLE_ENABLE;
        break;
      case GL_INDEX_LOGIC_OP:
        bit = gc->state.enables.general & __GL_INDEX_LOGIC_OP_ENABLE;
        break;
      case GL_COLOR_LOGIC_OP:
        bit = gc->state.enables.general & __GL_COLOR_LOGIC_OP_ENABLE;
        break;
      case GL_NORMALIZE:
        bit = gc->state.enables.general & __GL_NORMALIZE_ENABLE;
        break;
      case GL_POINT_SMOOTH:
        bit = gc->state.enables.general & __GL_POINT_SMOOTH_ENABLE;
        break;
      case GL_POLYGON_SMOOTH:
        bit = gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE;
        break;
      case GL_POLYGON_STIPPLE:
        bit = gc->state.enables.general & __GL_POLYGON_STIPPLE_ENABLE;
        break;
      case GL_SCISSOR_TEST:
        bit = gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE;
        break;
      case GL_STENCIL_TEST:
        bit = gc->state.enables.general & __GL_STENCIL_TEST_ENABLE;
        break;
      case GL_TEXTURE_1D:
        bit = gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_1D_ENABLE;
        break;
      case GL_TEXTURE_2D:
        bit = gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_2D_ENABLE;
        break;
      case GL_AUTO_NORMAL:
        bit = gc->state.enables.general & __GL_AUTO_NORMAL_ENABLE;
        break;
      case GL_TEXTURE_GEN_S:
        bit = gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_S_ENABLE;
        break;
      case GL_TEXTURE_GEN_T:
        bit = gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_T_ENABLE;
        break;
      case GL_TEXTURE_GEN_R:
        bit = gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_R_ENABLE;
        break;
      case GL_TEXTURE_GEN_Q:
        bit = gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_Q_ENABLE;
        break;

      case GL_CLIP_PLANE0: case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2: case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4: case GL_CLIP_PLANE5:
        cap -= GL_CLIP_PLANE0;
        bit = gc->state.enables.clipPlanes & (1 << cap);
        break;
      case GL_LIGHT0: case GL_LIGHT1:
      case GL_LIGHT2: case GL_LIGHT3:
      case GL_LIGHT4: case GL_LIGHT5:
      case GL_LIGHT6: case GL_LIGHT7:
        cap -= GL_LIGHT0;
        bit = gc->state.enables.lights & (1 << cap);
        break;
      case GL_MAP1_COLOR_4:
      case GL_MAP1_NORMAL:
      case GL_MAP1_INDEX:
      case GL_MAP1_TEXTURE_COORD_1: case GL_MAP1_TEXTURE_COORD_2:
      case GL_MAP1_TEXTURE_COORD_3: case GL_MAP1_TEXTURE_COORD_4:
      case GL_MAP1_VERTEX_3: case GL_MAP1_VERTEX_4:
        cap = __GL_EVAL1D_INDEX(cap);
        bit = gc->state.enables.eval1 & (1 << cap);
        break;
      case GL_MAP2_COLOR_4:
      case GL_MAP2_NORMAL:
      case GL_MAP2_INDEX:
      case GL_MAP2_TEXTURE_COORD_1: case GL_MAP2_TEXTURE_COORD_2:
      case GL_MAP2_TEXTURE_COORD_3: case GL_MAP2_TEXTURE_COORD_4:
      case GL_MAP2_VERTEX_3: case GL_MAP2_VERTEX_4:
        cap = __GL_EVAL2D_INDEX(cap);
        bit = gc->state.enables.eval2 & (1 << cap);
        break;
      case GL_POLYGON_OFFSET_POINT:
        bit = gc->state.enables.general & __GL_POLYGON_OFFSET_POINT_ENABLE;
        break;
      case GL_POLYGON_OFFSET_LINE:
        bit = gc->state.enables.general & __GL_POLYGON_OFFSET_LINE_ENABLE;
        break;
      case GL_POLYGON_OFFSET_FILL:
        bit = gc->state.enables.general & __GL_POLYGON_OFFSET_FILL_ENABLE;
        break;
      case GL_VERTEX_ARRAY:
        bit = gc->vertexArray.mask & VERTARRAY_V_MASK;
        break;
      case GL_NORMAL_ARRAY:
        bit = gc->vertexArray.mask & VERTARRAY_N_MASK;
        break;
      case GL_COLOR_ARRAY:
        bit = gc->vertexArray.mask & VERTARRAY_C_MASK;
        break;
      case GL_INDEX_ARRAY:
        bit = gc->vertexArray.mask & VERTARRAY_I_MASK;
        break;
      case GL_TEXTURE_COORD_ARRAY:
        bit = gc->vertexArray.mask & VERTARRAY_T_MASK;
        break;
      case GL_EDGE_FLAG_ARRAY:
        bit = gc->vertexArray.mask & VERTARRAY_E_MASK;
        break;
      case GL_CULL_VERTEX_SGI:
        bit = gc->state.enables.general & __GL_CULL_VERTEX_ENABLE;
        break;
          case GL_SHARED_TEXTURE_PALETTE_EXT:
    bit = gc->state.enables.general & __GL_SHARED_TEXTURE_PALETTE_EXT_ENABLE;
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return GL_FALSE;
    }
    return bit != 0;
}

void APIENTRY __glsstim_EnableClientState(GLenum mode)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch(mode) {
    case GL_VERTEX_ARRAY:
        gc->vertexArray.mask |= VERTARRAY_V_MASK;
        gc->vertexArray.index |= VERTARRAY_V_INDEX;
        gc->ogKey |= __GL_GEOM_OG_KEY_VERTARRAY_ENABLE; 
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_NORMAL_ARRAY:
        gc->vertexArray.mask |= VERTARRAY_N_MASK;
        gc->vertexArray.index |= VERTARRAY_N_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_COLOR_ARRAY:
        gc->vertexArray.mask |= VERTARRAY_C_MASK;
        gc->vertexArray.index |= VERTARRAY_C_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_INDEX_ARRAY:
        gc->vertexArray.mask |= VERTARRAY_I_MASK;
        gc->vertexArray.index |= VERTARRAY_I_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_TEXTURE_COORD_ARRAY:
        gc->vertexArray.mask |= VERTARRAY_T_MASK;
        gc->vertexArray.index |= VERTARRAY_T_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_EDGE_FLAG_ARRAY:
        gc->vertexArray.mask |= VERTARRAY_E_MASK;
        gc->vertexArray.index |= VERTARRAY_E_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_DisableClientState(GLenum mode) 
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch(mode) {
    case GL_VERTEX_ARRAY:
        gc->vertexArray.mask &= ~VERTARRAY_V_MASK;
        gc->vertexArray.index &= ~VERTARRAY_V_INDEX;
        gc->ogKey &= ~__GL_GEOM_OG_KEY_VERTARRAY_ENABLE;        
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_NORMAL_ARRAY:
        gc->vertexArray.mask &= ~VERTARRAY_N_MASK;
        gc->vertexArray.index &= ~VERTARRAY_N_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_COLOR_ARRAY:
        gc->vertexArray.mask &= ~VERTARRAY_C_MASK;
        gc->vertexArray.index &= ~VERTARRAY_C_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_INDEX_ARRAY:
        gc->vertexArray.mask &= ~VERTARRAY_I_MASK;
        gc->vertexArray.index &= ~VERTARRAY_I_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_TEXTURE_COORD_ARRAY:
        gc->vertexArray.mask &= ~VERTARRAY_T_MASK;
        gc->vertexArray.index &= ~VERTARRAY_T_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    case GL_EDGE_FLAG_ARRAY:
        gc->vertexArray.mask &= ~VERTARRAY_E_MASK;
        gc->vertexArray.index &= ~VERTARRAY_E_INDEX;
        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
        return;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_DELAY_VALIDATE(gc);
}


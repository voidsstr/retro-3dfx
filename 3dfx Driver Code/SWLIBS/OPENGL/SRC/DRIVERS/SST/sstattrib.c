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
** $Date: 10/11/00 8:03:00 PM$ 
**
*/

#include "sstcontext.h"
#include "global.h"
#include "g_imfncs.h"

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

    __GL_CLAMP_255(ref);
    __GL_GLIDE_SET(alphaTestReferenceValue, value, ref);
    if (gc->state.enables.general & __GL_ALPHA_TEST_ENABLE) {
        __GL_GLIDE_SET(alphaTestFunction, func, af - GL_NEVER);
    } else {
        __GL_GLIDE_SET(alphaTestFunction, func, GR_CMP_ALWAYS);
    }
    __GL_DELAY_VALIDATE(gc);
    gc->validateMask |= __GL_VALIDATE_ALPHA_FUNC;
}

void APIENTRY __glsstim_BlendFunc(GLenum sf, GLenum df)
{
    GrAlphaBlendFnc_t grsf, grdf;
    __GLSSTcontext *hwcx;
    __GL_SETUP_NOT_IN_BEGIN();

    hwcx = (__GLSSTcontext *)gc;

    /* Set Glide source function */
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

    /* Set Glide destination function */
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
    hwcx->glide.state.blendSrc = grsf;
    hwcx->glide.state.blendDst = grdf;

    if (gc->state.enables.general & __GL_BLEND_ENABLE) {
	__GL_GLIDE_SET(alphaBlendFunction, src, grsf);
	__GL_GLIDE_SET(alphaBlendFunction, dst, grdf);
    } else {
	__GL_GLIDE_SET( alphaBlendFunction, src, GR_BLEND_ONE );
	__GL_GLIDE_SET( alphaBlendFunction, dst, GR_BLEND_ZERO );
    }
}

void APIENTRY __glsstim_ColorMask(GLboolean r, GLboolean g,
				  GLboolean b, GLboolean a)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    /* XXXwheeler - Need to fallback to sw if r, g, and b are different */
    if (r == g && g == b) {
        __GL_GLIDE_SET(colorMask, rgb, r);
        __GL_GLIDE_SET(colorMask, a, a);
    } else {
        __GL_GLIDE_SET(colorMask, rgb, FXTRUE);
        __GL_GLIDE_SET(colorMask, a, FXTRUE);
    }

    gc->state.raster.rMask = r;
    gc->state.raster.gMask = g;
    gc->state.raster.bMask = b;
    gc->state.raster.aMask = a;
    __GL_DELAY_VALIDATE(gc);
}

void __glSSTValidateCull(__GLcontext *gc)
{
    GLuint cullBit, frontBit;
    
    if (gc->state.enables.general & __GL_CULL_FACE_ENABLE) {
        cullBit = (gc->state.polygon.cull == GL_FRONT);
        frontBit = (gc->state.polygon.frontFaceDirection == GL_CW);
        if (gc->constants.yInverted) {
	    if (frontBit ^ cullBit) {
                __GL_GLIDE_SET(cullMode, mode, GR_CULL_NEGATIVE);
	    } else {
                __GL_GLIDE_SET(cullMode, mode, GR_CULL_POSITIVE);
	    }
        } else {
	    if (frontBit ^ cullBit) {
                __GL_GLIDE_SET(cullMode, mode, GR_CULL_POSITIVE);
	    } else {
                __GL_GLIDE_SET(cullMode, mode, GR_CULL_NEGATIVE);
	    }
        }
    } else {
	__GL_GLIDE_SET(cullMode, mode, GR_CULL_DISABLE);
    }
}

void APIENTRY __glsstim_CullFace(GLenum cfm)
{
    __GL_SETUP_NOT_IN_BEGIN();

    switch (cfm) {
      case GL_FRONT:
      case GL_BACK:
      case GL_FRONT_AND_BACK:
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
    __GL_GLIDE_SET(depthBufferFunction, func, zf - GL_NEVER);

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);
}

void APIENTRY __glsstim_DepthMask(GLboolean enabled)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    gc->state.depth.writeEnable = enabled;
    __GL_GLIDE_SET(depthMask, mask, enabled);
    
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);
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
        __GL_GLIDE_SET(renderBuffer, buf, GR_BUFFER_FRONTBUFFER);
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
        __GL_GLIDE_SET(renderBuffer, buf, GR_BUFFER_BACKBUFFER);
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

/* XXXwheeler - not implemented yet */
void APIENTRY __glsstim_Fogfv(GLenum p, const GLfloat pv[])
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch (p) {
    case GL_FOG_COLOR:
	__glClampAndScaleColorf(gc, &gc->state.fog.color, pv);

	gc->state.fog.bufColor.r =
	    gc->state.fog.color.r * gc->constants.redRescale;
	gc->state.fog.bufColor.g =
	    gc->state.fog.color.g * gc->constants.greenRescale;
	gc->state.fog.bufColor.b =
	    gc->state.fog.color.b * gc->constants.blueRescale;

	gc->state.fog.r =
	    gc->state.fog.color.r * 255.0f * gc->constants.oneOverRedScale;
	gc->state.fog.g =
	    gc->state.fog.color.g * 255.0f * gc->constants.oneOverGreenScale;
	gc->state.fog.b =
	    gc->state.fog.color.b * 255.0f * gc->constants.oneOverBlueScale;
	break;
    case GL_FOG_DENSITY:
	if (pv[0] < 0) {
	    __glSetError(GL_INVALID_VALUE);
	    return;
	}
	gc->state.fog.density = pv[0];
	__GL_GLIDE_SET(fogParms, density, pv[0]);
	break;
    case GL_FOG_END:
	gc->state.fog.end = pv[0];
	__GL_GLIDE_SET(fogParms, end, pv[0]);
	break;
    case GL_FOG_START:
	gc->state.fog.start = pv[0];
	__GL_GLIDE_SET(fogParms, start, pv[0]);
	break;
    case GL_FOG_INDEX:
	{
	    GLint fogIndex, indexMask;

	    indexMask = (1 << gc->modes.indexBits) - 1;
	    fogIndex = (GLint)pv[0];
	    gc->state.fog.index = (__GLfloat)(fogIndex & indexMask);
	    gc->state.fog.i = fogIndex & indexMask;
	}
    break;
    case GL_FOG_MODE:
	switch ((GLenum) pv[0]) {
	case GL_EXP:
	case GL_EXP2:
	case GL_LINEAR:
	    gc->state.fog.mode = (GLenum) pv[0];
            __GL_GLIDE_SET(fogParms, mode, pv[0]);
	    break;
	default:
	    __glSetError(GL_INVALID_ENUM);
	    return;
	}
	break;
    default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }

    /*
    ** Recompute cached 1/(end - start) value for linear fogging.
    */
    if (gc->state.fog.mode == GL_LINEAR) {
	if (gc->state.fog.start != gc->state.fog.end) {
	    gc->state.fog.oneOverEMinusS =  
		__glOne / (gc->state.fog.end - gc->state.fog.start);
	} else {
	    /*
	    ** Use zero as the undefined value.
	    */
	    gc->state.fog.oneOverEMinusS = __glZero;
	}
    }

    __GL_DELAY_VALIDATE(gc);
}


void APIENTRY __glsstim_Fogf(GLenum p, GLfloat f)
{
    /* Accept only enumerants that correspond to single values */
    switch (p) {
      case GL_FOG_DENSITY:
      case GL_FOG_END:
      case GL_FOG_START:
      case GL_FOG_INDEX:
      case GL_FOG_MODE:
	__glsstim_Fogfv(p,&f);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}

void APIENTRY __glsstim_Fogiv(GLenum p, const GLint pv[])
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch (p) {
      case GL_FOG_COLOR:
	__glClampAndScaleColori(gc, &gc->state.fog.color, pv);

	gc->state.fog.bufColor.r =
	    gc->state.fog.color.r * gc->constants.redRescale;
	gc->state.fog.bufColor.g =
	    gc->state.fog.color.g * gc->constants.greenRescale;
	gc->state.fog.bufColor.b =
	    gc->state.fog.color.b * gc->constants.blueRescale;

	gc->state.fog.r =
	    gc->state.fog.color.r * 255.0f * gc->constants.oneOverRedScale;
	gc->state.fog.g =
	    gc->state.fog.color.g * 255.0f * gc->constants.oneOverGreenScale;
	gc->state.fog.b =
	    gc->state.fog.color.b * 255.0f * gc->constants.oneOverBlueScale;
	break;
      case GL_FOG_DENSITY:
	if (pv[0] < 0) {
	    __glSetError(GL_INVALID_VALUE);
	    return;
	}
	gc->state.fog.density = pv[0];
	__GL_GLIDE_SET(fogParms, density, pv[0]);
	break;
      case GL_FOG_END:
	gc->state.fog.end = pv[0];
	__GL_GLIDE_SET(fogParms, end, pv[0]);
	break;
      case GL_FOG_START:
	gc->state.fog.start = pv[0];
	__GL_GLIDE_SET(fogParms, start, pv[0]);
	break;
      case GL_FOG_INDEX:
	{
	    GLint fogIndex, indexMask;

	    indexMask = (1 << gc->modes.indexBits) - 1;
	    fogIndex = (GLint)pv[0];
	    gc->state.fog.index = (__GLfloat)(fogIndex & indexMask);
	    gc->state.fog.i = fogIndex & indexMask;
	}
	break;
      case GL_FOG_MODE:
	switch ((GLenum) pv[0]) {
	  case GL_EXP:
	  case GL_EXP2:
	  case GL_LINEAR:
	    gc->state.fog.mode = (GLenum) pv[0];
            __GL_GLIDE_SET(fogParms, mode, pv[0]);
	    break;
	  default:
	    __glSetError(GL_INVALID_ENUM);
	    return;
	}
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }

    /*
    ** Recompute cached 1/(end - start) value for linear fogging.
    */
    if (gc->state.fog.mode == GL_LINEAR) {
	if (gc->state.fog.start != gc->state.fog.end) {
	    gc->state.fog.oneOverEMinusS =  
		__glOne / (gc->state.fog.end - gc->state.fog.start);
	} else {
	    /*
	    ** Use zero as the undefined value.
	    */
	    gc->state.fog.oneOverEMinusS = __glZero;
	}
    }

    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_Fogi(GLenum p, GLint i)
{
    /* Accept only enumerants that correspond to single values */
    switch (p) {
      case GL_FOG_DENSITY:
      case GL_FOG_END:
      case GL_FOG_START:
      case GL_FOG_INDEX:
      case GL_FOG_MODE:
	__glim_Fogiv(p,&i);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
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

void APIENTRY __glsstim_Enable(GLenum cap)
{
    __GLSSTcontext *hwcx;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    hwcx = (__GLSSTcontext *)gc;

    switch (cap) {
      case GL_ALPHA_TEST:
	gc->state.enables.general |= __GL_ALPHA_TEST_ENABLE;
        __GL_GLIDE_SET(alphaTestFunction, func,
		       gc->state.raster.alphaFunction - GL_NEVER);
	break;
      case GL_BLEND:
	gc->state.enables.general |= __GL_BLEND_ENABLE;
	__GL_GLIDE_SET(alphaBlendFunction, src, hwcx->glide.state.blendSrc);
	__GL_GLIDE_SET(alphaBlendFunction, dst, hwcx->glide.state.blendDst);
	break;
      case GL_COLOR_MATERIAL:
	gc->state.enables.general |= __GL_COLOR_MATERIAL_ENABLE;
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
	(*gc->procs.pickColorMaterialProcs)(gc);
	(*gc->procs.applyColor)(gc);
	return;				/* NOTE: return! */
      case GL_INDEX_MATERIAL_SGI:
        gc->state.enables.general |= __GL_INDEX_MATERIAL_ENABLE;
        break;
      case GL_CULL_FACE:
	if (gc->state.enables.general & __GL_CULL_FACE_ENABLE) return;
	gc->state.enables.general |= __GL_CULL_FACE_ENABLE;
        __glSSTValidateCull(gc);
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
	return;
      case GL_DEPTH_TEST:
	gc->state.enables.general |= __GL_DEPTH_TEST_ENABLE;
	__GL_GLIDE_SET(depthBufferMode, mode, GR_DEPTHBUFFER_ZBUFFER);
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);
	break;
      case GL_DITHER:
	gc->state.enables.general |= __GL_DITHER_ENABLE;
        __GL_GLIDE_SET(ditherMode, mode, GR_DITHER_4x4);
	break;
      case GL_FOG:
	gc->state.enables.general |= __GL_FOG_ENABLE;
	/* XXXwheeler - not implemented in hw yet */
	break;
      case GL_INDEX_TEST_SGI:
        gc->state.enables.general |= __GL_INDEX_TEST_ENABLE;
        break;
      case GL_LIGHTING:
	gc->state.enables.general |= __GL_LIGHTING_ENABLE;
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
	(*gc->procs.pickColorMaterialProcs)(gc);
	(*gc->procs.applyColor)(gc);
	return;
      case GL_LINE_SMOOTH:
	gc->state.enables.general |= __GL_LINE_SMOOTH_ENABLE;
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
	break;
      case GL_POINT_SMOOTH:
	gc->state.enables.general |= __GL_POINT_SMOOTH_ENABLE;
	break;
      case GL_POLYGON_SMOOTH:
	gc->state.enables.general |= __GL_POLYGON_SMOOTH_ENABLE;
	break;
      case GL_POLYGON_STIPPLE:
	gc->state.enables.general |= __GL_POLYGON_STIPPLE_ENABLE;
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
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
	break;
      case GL_TEXTURE_1D:
	gc->state.enables.texture |= __GL_TEXTURE_1D_ENABLE;
	break;
      case GL_TEXTURE_2D:
	gc->state.enables.texture |= __GL_TEXTURE_2D_ENABLE;
	break;
      case GL_AUTO_NORMAL:
	gc->state.enables.general |= __GL_AUTO_NORMAL_ENABLE;
	break;
      case GL_TEXTURE_GEN_S:
	gc->state.enables.texture |= __GL_TEXTURE_GEN_S_ENABLE;
	break;
      case GL_TEXTURE_GEN_T:
	gc->state.enables.texture |= __GL_TEXTURE_GEN_T_ENABLE;
	break;
      case GL_TEXTURE_GEN_R:
	gc->state.enables.texture |= __GL_TEXTURE_GEN_R_ENABLE;
	break;
      case GL_TEXTURE_GEN_Q:
	gc->state.enables.texture |= __GL_TEXTURE_GEN_Q_ENABLE;
	break;

      case GL_CLIP_PLANE0: case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2: case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4: case GL_CLIP_PLANE5:
	cap -= GL_CLIP_PLANE0;
	gc->state.enables.clipPlanes |= (1 << cap);
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
        __GL_GLIDE_SET(alphaTestFunction, func, GR_CMP_ALWAYS);
	break;
      case GL_BLEND:
	gc->state.enables.general &= ~__GL_BLEND_ENABLE;
	__GL_GLIDE_SET(alphaBlendFunction, src, GR_BLEND_ONE);
	__GL_GLIDE_SET(alphaBlendFunction, dst, GR_BLEND_ZERO);
	break;
      case GL_COLOR_MATERIAL:
	gc->state.enables.general &= ~__GL_COLOR_MATERIAL_ENABLE;
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
	(*gc->procs.pickColorMaterialProcs)(gc);
	return;				/* NOTE: return! */
      case GL_INDEX_MATERIAL_SGI:
        gc->state.enables.general &= ~__GL_INDEX_MATERIAL_ENABLE;
        break;
      case GL_CULL_FACE:
	if (!(gc->state.enables.general & __GL_CULL_FACE_ENABLE)) return;
	gc->state.enables.general &= ~__GL_CULL_FACE_ENABLE;
	__glSSTValidateCull(gc);
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
	return;
      case GL_DEPTH_TEST:
	gc->state.enables.general &= ~__GL_DEPTH_TEST_ENABLE;
	__GL_GLIDE_SET(depthBufferMode, mode, GR_DEPTHBUFFER_DISABLE);
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);
	break;
      case GL_DITHER:
	gc->state.enables.general &= ~__GL_DITHER_ENABLE;
        __GL_GLIDE_SET(ditherMode, mode, GR_DITHER_DISABLE);
	break;
      case GL_FOG:
	gc->state.enables.general &= ~__GL_FOG_ENABLE;
	/* XXXwheeler - not implemented in hw yet */
	break;
      case GL_INDEX_TEST_SGI:
        gc->state.enables.general &= ~__GL_INDEX_TEST_ENABLE;
        break;
      case GL_LIGHTING:
	gc->state.enables.general &= ~__GL_LIGHTING_ENABLE;
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_LIGHTING);
	(*gc->procs.pickColorMaterialProcs)(gc);
	(*gc->procs.applyColor)(gc);
	return;
      case GL_LINE_SMOOTH:
	gc->state.enables.general &= ~__GL_LINE_SMOOTH_ENABLE;
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
	break;
      case GL_POINT_SMOOTH:
	gc->state.enables.general &= ~__GL_POINT_SMOOTH_ENABLE;
	break;
      case GL_POLYGON_SMOOTH:
	gc->state.enables.general &= ~__GL_POLYGON_SMOOTH_ENABLE;
	break;
      case GL_POLYGON_STIPPLE:
	gc->state.enables.general &= ~__GL_POLYGON_STIPPLE_ENABLE;
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_POLYGON);
	return;
      case GL_SCISSOR_TEST:
	gc->state.enables.general &= ~__GL_SCISSOR_TEST_ENABLE;
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_SCISSOR);
	(*gc->procs.computeClipBox)(gc);
	(*gc->procs.applyScissor)(gc);
	break;
      case GL_STENCIL_TEST:
	gc->state.enables.general &= ~__GL_STENCIL_TEST_ENABLE;
	break;
      case GL_TEXTURE_1D:
	gc->state.enables.texture &= ~__GL_TEXTURE_1D_ENABLE;
	break;
      case GL_TEXTURE_2D:
	gc->state.enables.texture &= ~__GL_TEXTURE_2D_ENABLE;
	break;
      case GL_AUTO_NORMAL:
	gc->state.enables.general &= ~__GL_AUTO_NORMAL_ENABLE;
	break;
      case GL_TEXTURE_GEN_S:
	gc->state.enables.texture &= ~__GL_TEXTURE_GEN_S_ENABLE;
	break;
      case GL_TEXTURE_GEN_T:
	gc->state.enables.texture &= ~__GL_TEXTURE_GEN_T_ENABLE;
	break;
      case GL_TEXTURE_GEN_R:
	gc->state.enables.texture &= ~__GL_TEXTURE_GEN_R_ENABLE;
	break;
      case GL_TEXTURE_GEN_Q:
	gc->state.enables.texture &= ~__GL_TEXTURE_GEN_Q_ENABLE;
	break;

      case GL_CLIP_PLANE0: case GL_CLIP_PLANE1:
      case GL_CLIP_PLANE2: case GL_CLIP_PLANE3:
      case GL_CLIP_PLANE4: case GL_CLIP_PLANE5:
	cap -= GL_CLIP_PLANE0;
	gc->state.enables.clipPlanes &= ~(1 << cap);
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
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    __GL_DELAY_VALIDATE(gc);
}

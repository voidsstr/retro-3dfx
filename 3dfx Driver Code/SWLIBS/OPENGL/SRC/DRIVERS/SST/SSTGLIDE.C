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
** $Date: 10/11/00 8:03:05 PM$ 
**
*/

#include "sstcontext.h"
#include <stddef.h>

#define __GL_GLIDE_CALLOC(variable, type) \
    if ((hwcx->glide.shadow.variable = \
         gc->imports.calloc(gc, sizeof(type), 1)) == NULL) { \
        return GL_FALSE; \
    } else

#define __GL_GLIDE_FREE(variable) \
    gc->imports.free(gc, hwcx->glide.shadow.variable)

int grGetInteger(int pname) {
   int value;
   grGet(pname, sizeof(value), &value);
   return value;
}

GLboolean __glSSTGlideCreateContext(__GLcontext *gc) {
    __GLSSTcontext *hwcx = (__GLSSTcontext *)gc;

    /* Initialize hardware configuration */
    hwcx->glide.config.numTMUs = grGetInteger(GR_NUM_TMU);

    __GL_GLIDE_CALLOC(alphaBlendFunction, __GlideAlphaBlendFunction);
    __GL_GLIDE_SET( alphaBlendFunction, src, GR_BLEND_ONE );
    __GL_GLIDE_SET( alphaBlendFunction, dst, GR_BLEND_ZERO );

    __GL_GLIDE_CALLOC(depthBufferFunction, __GlideDepthBufferFunction);
    __GL_GLIDE_SET( depthBufferFunction, func, GR_CMP_ALWAYS );

    __GL_GLIDE_CALLOC(depthBufferMode, __GlideDepthBufferMode);
    __GL_GLIDE_SET( depthBufferMode, mode, GR_DEPTHBUFFER_DISABLE );

    __GL_GLIDE_CALLOC(depthMask, __GlideDepthMask);
    __GL_GLIDE_SET( depthMask, mask, FXTRUE );

    __GL_GLIDE_CALLOC(texFilterMode0, __GlideTexFilterMode);
    __GL_GLIDE_SET( texFilterMode0, mag, GR_TEXTUREFILTER_BILINEAR );
    __GL_GLIDE_SET( texFilterMode0, min, GR_TEXTUREFILTER_BILINEAR );

    __GL_GLIDE_CALLOC(texClampMode0, __GlideTexClampMode);
    __GL_GLIDE_SET( texClampMode0, s, GR_TEXTURECLAMP_WRAP );
    __GL_GLIDE_SET( texClampMode0, t, GR_TEXTURECLAMP_WRAP );

    __GL_GLIDE_CALLOC(texMipMapMode0, __GlideTexMipMapMode);
    __GL_GLIDE_SET( texMipMapMode0, mode, GR_MIPMAP_NEAREST );

    __GL_GLIDE_CALLOC(texCombine0, __GlideTexCombine);
    __GL_GLIDE_SET( texCombine0, function, GR_COMBINE_FUNCTION_LOCAL );
    __GL_GLIDE_SET( texCombine0, factor,   GR_COMBINE_FACTOR_NONE );

    if ( hwcx->glide.config.numTMUs == 2 ) {
        __GL_GLIDE_CALLOC(texFilterMode1, __GlideTexFilterMode);
        __GL_GLIDE_SET( texFilterMode1, mag, GR_TEXTUREFILTER_BILINEAR );
        __GL_GLIDE_SET( texFilterMode1, min, GR_TEXTUREFILTER_BILINEAR );
        
	__GL_GLIDE_CALLOC(texClampMode1, __GlideTexClampMode);
	__GL_GLIDE_SET( texClampMode1, s, GR_TEXTURECLAMP_WRAP );
	__GL_GLIDE_SET( texClampMode1, t, GR_TEXTURECLAMP_WRAP );
        
	__GL_GLIDE_CALLOC(texMipMapMode1, __GlideTexMipMapMode);
	__GL_GLIDE_SET( texMipMapMode1, mode, GR_MIPMAP_NEAREST );
        
	__GL_GLIDE_CALLOC(texCombine1, __GlideTexCombine);
	__GL_GLIDE_SET( texCombine1, function, GR_COMBINE_FUNCTION_LOCAL );
	__GL_GLIDE_SET( texCombine1, factor,   GR_COMBINE_FACTOR_NONE );
    }

    __GL_GLIDE_CALLOC(colorCombine, __GlideColorCombine);
    __GL_GLIDE_SET( colorCombine, function, GR_COMBINE_FUNCTION_SCALE_OTHER );
    __GL_GLIDE_SET( colorCombine, factor,   GR_COMBINE_FACTOR_ONE );
    __GL_GLIDE_SET( colorCombine, local,    GR_COMBINE_LOCAL_NONE );
    __GL_GLIDE_SET( colorCombine, other,    GR_COMBINE_OTHER_ITERATED );

    __GL_GLIDE_CALLOC(alphaCombine, __GlideAlphaCombine);
    __GL_GLIDE_SET( alphaCombine, function, GR_COMBINE_FUNCTION_SCALE_OTHER );
    __GL_GLIDE_SET( alphaCombine, factor,   GR_COMBINE_FACTOR_ONE );
    __GL_GLIDE_SET( alphaCombine, local,    GR_COMBINE_LOCAL_NONE );
    __GL_GLIDE_SET( alphaCombine, other,    GR_COMBINE_OTHER_ITERATED );

    __GL_GLIDE_CALLOC(renderBuffer, __GlideRenderBuffer);
    __GL_GLIDE_SET( renderBuffer, buf, GR_BUFFER_BACKBUFFER );

    __GL_GLIDE_CALLOC(cullMode, __GlideCullMode);
    __GL_GLIDE_SET( cullMode, mode, GR_CULL_DISABLE );

    __GL_GLIDE_CALLOC(alphaTestFunction, __GlideAlphaTestFunction);
    __GL_GLIDE_SET( alphaTestFunction, func, GR_CMP_ALWAYS );
    
    __GL_GLIDE_CALLOC(alphaTestReferenceValue, __GlideAlphaTestReferenceValue);
    __GL_GLIDE_SET( alphaTestReferenceValue, value, 0x0 );

    __GL_GLIDE_CALLOC(depthBiasLevel, __GlideDepthBiasLevel);
    __GL_GLIDE_SET( depthBiasLevel, level, 0x0 );

    __GL_GLIDE_CALLOC(colorMask, __GlideColorMask);
    __GL_GLIDE_SET( colorMask, rgb, FXTRUE );
    __GL_GLIDE_SET( colorMask, a, FXTRUE );

    __GL_GLIDE_CALLOC(ditherMode, __GlideDitherMode);
    __GL_GLIDE_SET( ditherMode, mode, GR_DITHER_4x4 );

    __GL_GLIDE_CALLOC(fogColor, __GlideFogColor);
    __GL_GLIDE_SET( fogColor, value, 0x0 );

    __GL_GLIDE_CALLOC(fogParms, __GlideFogParms);
    __GL_GLIDE_SET( fogParms, mode, GL_EXP );
    __GL_GLIDE_SET( fogParms, density, 1.0f );
    __GL_GLIDE_SET( fogParms, start, 0.0f );
    __GL_GLIDE_SET( fogParms, end, 1.0f );

    // XXXwheeler: need to do something with fog
#ifdef GL_GLIDE3
  if (fogTable == NULL) {
                FxI32 fogTableSize;
                grGet ( GR_FOG_TABLE_ENTRIES, sizeof ( FxI32 ), &fogTableSize );
                fogTable = CALLOC ( fogTableSize, sizeof ( GrFog_t ) );
        }
#endif

    grReset(GR_VERTEX_PARAMETER);
    grVertexLayout(GR_PARAM_XY, offsetof(__GLvertex, window.x),
		   GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_RGB, offsetof(__GLvertex, colors[0].r),
		   GR_PARAM_ENABLE );
    grVertexLayout(GR_PARAM_Z,offsetof(__GLvertex, window.z),
		   GR_PARAM_ENABLE );
    grVertexLayout(GR_PARAM_A,offsetof(__GLvertex, colors[0].a),
		   GR_PARAM_ENABLE );
    
    return GL_TRUE;
}

void __glSSTGlideDestroyContext(__GLcontext *gc) {
    __GLSSTcontext *hwcx = (__GLSSTcontext *)gc;

    __GL_GLIDE_FREE(alphaBlendFunction);
    __GL_GLIDE_FREE(depthBufferFunction);
    __GL_GLIDE_FREE(depthBufferMode);
    __GL_GLIDE_FREE(depthMask);
    __GL_GLIDE_FREE(texFilterMode0);
    __GL_GLIDE_FREE(texClampMode0);
    __GL_GLIDE_FREE(texMipMapMode0);
    __GL_GLIDE_FREE(texCombine0);
    if ( hwcx->glide.config.numTMUs == 2 ) {
        __GL_GLIDE_FREE(texFilterMode1);
	__GL_GLIDE_FREE(texClampMode1);
	__GL_GLIDE_FREE(texMipMapMode1);
	__GL_GLIDE_FREE(texCombine1);
    }
    __GL_GLIDE_FREE(colorCombine);
    __GL_GLIDE_FREE(alphaCombine);
    __GL_GLIDE_FREE(renderBuffer);
    __GL_GLIDE_FREE(cullMode);
    __GL_GLIDE_FREE(alphaTestFunction);
    __GL_GLIDE_FREE(alphaTestReferenceValue);
    __GL_GLIDE_FREE(depthBiasLevel);
    __GL_GLIDE_FREE(colorMask);
    __GL_GLIDE_FREE(ditherMode);
    __GL_GLIDE_FREE(fogColor);
    __GL_GLIDE_FREE(fogParms);
    /* XXXwheeler: do something with fog table */
}

void __glSSTGlideValidateHW(__GLcontext *gc) {
    int function;
    __GLglideShadow *shadow = &((__GLSSTcontext *)gc)->glide.shadow;

    for( function = 0;  shadow->dirty; shadow->dirty >>= 1,function++ ) {
	if ( shadow->dirty & 0x1 ) {
	    switch( function ) {
	    case ALPHA_BLEND_FUNCTION:
		grAlphaBlendFunction( shadow->alphaBlendFunction->src,
				      shadow->alphaBlendFunction->dst,
				      GR_BLEND_ZERO, GR_BLEND_ZERO);
		break;
	    case DEPTH_BUFFER_FUNCTION:
		grDepthBufferFunction( shadow->depthBufferFunction->func );
		break;
	    case DEPTH_BUFFER_MODE:
		grDepthBufferMode( shadow->depthBufferMode->mode );
		break;
	    case DEPTH_MASK:
		grDepthMask( shadow->depthMask->mask );
		break;
	    case TEX_FILTER_MODE0:
		grTexFilterMode( GR_TMU0,
				 shadow->texFilterMode0->min,
				 shadow->texFilterMode0->mag );
		break;
	    case TEX_CLAMP_MODE0:
		grTexClampMode( GR_TMU0,
				shadow->texClampMode0->s,
				shadow->texClampMode0->t );
		break;
	    case TEX_MIPMAP_MODE0:
		grTexMipMapMode( GR_TMU0,
				 shadow->texMipMapMode0->mode,
				 FXFALSE );
		break;
	    case TEX_COMBINE0:
		grTexCombine( GR_TMU0,
			      shadow->texCombine0->function,
			      shadow->texCombine0->factor,
			      shadow->texCombine0->function,
			      shadow->texCombine0->factor,
			      FXFALSE,
			      FXFALSE );
		break;
	    case TEX_FILTER_MODE1:
		grTexFilterMode( GR_TMU1,
				 shadow->texFilterMode1->min,
				 shadow->texFilterMode1->mag );
		break;
	    case TEX_CLAMP_MODE1:
		grTexClampMode( GR_TMU1,
				shadow->texClampMode1->s,
				shadow->texClampMode1->t );
		break;
	    case TEX_MIPMAP_MODE1:
		grTexMipMapMode( GR_TMU1,
				 shadow->texMipMapMode1->mode,
				 FXFALSE );
		break;
	    case TEX_COMBINE1:
		grTexCombine( GR_TMU1,
			      shadow->texCombine1->function,
			      shadow->texCombine1->factor,
			      shadow->texCombine1->function,
			      shadow->texCombine1->factor,
			      FXFALSE,
			      FXFALSE );
		break;
	    case COLOR_COMBINE:
		grColorCombine( shadow->colorCombine->function,
				shadow->colorCombine->factor,
				shadow->colorCombine->local,
				shadow->colorCombine->other,
				FXFALSE );
		break;
	    case ALPHA_COMBINE:
		grAlphaCombine( shadow->alphaCombine->function,
				shadow->alphaCombine->factor,
				shadow->alphaCombine->local,
				shadow->alphaCombine->other,
				FXFALSE );
		break;
	    case RENDER_BUFFER:
		grRenderBuffer( shadow->renderBuffer->buf );
		break;
	    case CULL_MODE:
		grCullMode( shadow->cullMode->mode );
		break;
	    case ALPHA_TEST_FUNCTION:
		grAlphaTestFunction( shadow->alphaTestFunction->func );
		break;
	    case ALPHA_TEST_REFERENCE_VALUE:
		grAlphaTestReferenceValue( (FxU8)shadow->alphaTestReferenceValue->value );
		break;
	    case DEPTH_BIAS_LEVEL:
		grDepthBiasLevel( shadow->depthBiasLevel->level );
		break;
            case COLOR_MASK:
		grColorMask(shadow->colorMask->rgb, shadow->colorMask->a);
		break;
	    case DITHER_MODE:
	        grDitherMode(shadow->ditherMode->mode);
		break;
	    case FOG_COLOR:
		grFogColorValue( shadow->fogColor->value );
		break;
	    case FOG_PARMS:
		//XXXwheeler - do something with fog
#if 0
		switch (shadow->fogParms->mode) {
		case GL_LINEAR:
		    guFogGenerateLinear(fogTable, shadow->fogParms->start, 
					shadow->fogParms->end);
		    break;
		case GL_EXP:
		    guFogGenerateExp(fogTable, shadow->fogParms->density);
		    break;
		case GL_EXP2:
		    guFogGenerateExp2(fogTable, shadow->fogParms->density);
		    break;
		}
		grFogTable(fogTable);
#endif
		break;
	    default:
		break;
	    }
	}
    }
}

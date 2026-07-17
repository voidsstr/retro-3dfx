/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
**________________________________________________________________________________________
**
**  Description: 
**
** 
**
*/

#include <stdlib.h>

#include "glr.h"
#include "glr_glide.h"
#include "glr_drawing.h"

//#include "fxglide.h"
// these are from "fxglide.h"
// too big of a hassle to include the file
#define GR_AA_ORDERED_OGL               0x00010000
#define GR_AA_ORDERED_POINTS_OGL        GR_AA_ORDERED_OGL+1
#define GR_AA_ORDERED_LINES_OGL         GR_AA_ORDERED_OGL+2
#define GR_AA_ORDERED_TRIANGLES_OGL     GR_AA_ORDERED_OGL+3

#include <h3.h>

#include "GlideWrapper.h"

/*
________________________________________________________________________________________

      glrConfigureTMU
________________________________________________________________________________________

*/

static void glrConfigureTMU(FxU32 tmu, const GLDPerTextureState *tex_state)
{		
	FxBool lodBlend;
	GrMipMapMode_t mode;
	GrTextureFilterMode_t minfilter_mode, magfilter_mode;

	switch(tex_state->params.min_filter)
	{
		case GL_NEAREST:
			lodBlend = FXFALSE;
			mode = GR_MIPMAP_DISABLE;
			minfilter_mode = GR_TEXTUREFILTER_POINT_SAMPLED;
		break;
		case GL_NEAREST_MIPMAP_NEAREST:
			lodBlend = FXFALSE;
			mode = GR_MIPMAP_NEAREST;
			minfilter_mode = GR_TEXTUREFILTER_POINT_SAMPLED;
		break;
		case GL_NEAREST_MIPMAP_LINEAR:
			lodBlend = FXFALSE;
			mode = GR_MIPMAP_NEAREST;
			minfilter_mode = GR_TEXTUREFILTER_POINT_SAMPLED;
		break;
		case GL_LINEAR:
			lodBlend = FXFALSE;
			mode = GR_MIPMAP_DISABLE;
			minfilter_mode = GR_TEXTUREFILTER_BILINEAR;
		break;
		case GL_LINEAR_MIPMAP_NEAREST:
			lodBlend = FXFALSE;
			mode = GR_MIPMAP_NEAREST;
			minfilter_mode = GR_TEXTUREFILTER_BILINEAR;
		break;
		default: 
		case GL_LINEAR_MIPMAP_LINEAR:
			lodBlend = FXFALSE;
//			mode = GR_MIPMAP_NEAREST_DITHER;
			mode = GR_MIPMAP_NEAREST;
			minfilter_mode = GR_TEXTUREFILTER_BILINEAR;
		break;
	}
	
	switch(tex_state->params.mag_filter)
	{
		case GL_NEAREST:
			magfilter_mode = GR_TEXTUREFILTER_POINT_SAMPLED;
		break;
		default:
		case GL_LINEAR:
			magfilter_mode = GR_TEXTUREFILTER_BILINEAR;
		break;
	}
		
	grTexFilterMode(tmu, minfilter_mode, magfilter_mode);
//	grEnable(GR_ALLOW_MIPMAP_DITHER);
	grTexMipMapMode(tmu, mode, lodBlend);
		
	// Not supported : tex_state->params.border_color.r
	// Not supported : tex_state->params.border_color.g
	// Not supported : tex_state->params.border_color.b
	// Not supported : tex_state->params.border_color.a
	
	grTexClampMode(
		tmu,
		(tex_state->params.s_wrap_mode == GL_REPEAT ? GR_TEXTURECLAMP_WRAP : GR_TEXTURECLAMP_CLAMP),
		(tex_state->params.t_wrap_mode == GL_REPEAT ? GR_TEXTURECLAMP_WRAP : GR_TEXTURECLAMP_CLAMP)
	);
	
	//grTexMipMapMode(GR_TMU0, GR_MIPMAP_DISABLE, FXFALSE);
	//grTexClampMode(GR_TMU0,GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);
}

void glrConfigureCombineUnitsSST1(GLDContext ctx, GLuint activeTextureUnits)
{
	/* Do nasty texture/color combine stuff. */
	switch(activeTextureUnits)
	{
	  case GLR_TMU0_ACTIVE:
	  {
	    grTexCombine(GR_TMU0,
		             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			         FXFALSE,FXFALSE);

	    grTexCombine(GR_TMU1,
		             GR_COMBINE_FUNCTION_NONE,GR_COMBINE_FACTOR_NONE,
		             GR_COMBINE_FUNCTION_NONE,GR_COMBINE_FACTOR_NONE,
		             FXFALSE,FXFALSE);
	
		switch(ctx->state->texture_mode[0].env_mode)
		{
		  case GL_DECAL:
		    
			  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
			                 GR_COMBINE_FACTOR_LOCAL,
			                 GR_COMBINE_LOCAL_ITERATED,
			                 GR_COMBINE_OTHER_TEXTURE,
			                 FXFALSE);
			                 
			  grColorCombine(GR_COMBINE_FUNCTION_BLEND,
			                 GR_COMBINE_FACTOR_TEXTURE_ALPHA,
			                 GR_COMBINE_LOCAL_ITERATED,
			                 GR_COMBINE_OTHER_TEXTURE,
			                 FXFALSE);
		  break;
		  
		  case GL_REPLACE:
		    
		      if(ctx->hw_texture[0]->baseInternalFormat == GL_ALPHA){
				  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
				                 GR_COMBINE_FACTOR_NONE,
				                 GR_COMBINE_LOCAL_ITERATED,
				                 GR_COMBINE_OTHER_NONE,
				                 FXFALSE);				      
		      } else {
				  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
				                 GR_COMBINE_FACTOR_ONE,
				                 GR_COMBINE_LOCAL_NONE,
				                 GR_COMBINE_OTHER_TEXTURE,
				                 FXFALSE);				      
		      }
			  switch(ctx->hw_texture[0]->baseInternalFormat){
			  case GL_ALPHA:
			  case GL_LUMINANCE_ALPHA:
			  case GL_INTENSITY:
			  case GL_RGBA:
				  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
				                 GR_COMBINE_FACTOR_ONE,
				                 GR_COMBINE_LOCAL_NONE,
				                 GR_COMBINE_OTHER_TEXTURE,
				                 FXFALSE);
				break;
			  case GL_LUMINANCE:
			  case GL_RGB:
				  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
				                 GR_COMBINE_FACTOR_NONE,
				                 GR_COMBINE_LOCAL_ITERATED,
				                 GR_COMBINE_OTHER_NONE,
				                 FXFALSE);
			  	break;
			  }
			                 
		  break;
		  
		  case GL_BLEND:
			// Cant seem to do blend with voodoo3???
		  break;
		  
		  case GL_MODULATE:
	      {
		    
			  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			                 GR_COMBINE_FACTOR_LOCAL,
			                 GR_COMBINE_LOCAL_ITERATED,
			                 GR_COMBINE_OTHER_TEXTURE,
			                 FXFALSE);
	
	          if(ctx->hw_texture[0]->baseInternalFormat == GL_ALPHA)
	          {
				  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
				                 GR_COMBINE_FACTOR_NONE,
				                 GR_COMBINE_LOCAL_ITERATED,
				                 GR_COMBINE_OTHER_NONE,
				                 FXFALSE);
			  } 
			  else 
			  {
				  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
				                 GR_COMBINE_FACTOR_LOCAL,
				                 GR_COMBINE_LOCAL_ITERATED,
				                 GR_COMBINE_OTHER_TEXTURE,				  
				                 FXFALSE);
			  }
		  } 
		  break;
		      
		  }
		}
		break;
	  
	  
	  
	  /* Multitexture cases */
	  case GLR_TMU0_ACTIVE|GLR_TMU1_ACTIVE:
	  {
	    /* Would be nice to do this in some nicer way... */
	    if(ctx->state->texture_mode[0].env_mode == GL_REPLACE &&
	       ctx->state->texture_mode[1].env_mode == GL_MODULATE)
	    {
	      grTexCombine(GR_TMU0,
			   GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_LOCAL,
			   GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_LOCAL,
			   FXFALSE,FXFALSE);

	      grTexCombine(GR_TMU1,
			   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			   FXFALSE,FXTRUE);			  
	    
	    if(ctx->hw_texture[0]->baseInternalFormat == GL_RGB)
	      grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
			     GR_COMBINE_FACTOR_NONE,
			     GR_COMBINE_LOCAL_ITERATED,
			     GR_COMBINE_OTHER_NONE,
			     FXFALSE);
	    else
	      grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			     GR_COMBINE_FACTOR_ONE,
			     GR_COMBINE_LOCAL_ITERATED,
			     GR_COMBINE_OTHER_NONE,
			     FXFALSE);
	
	    /* I think this is wrong, and shouldn't be doing the multiply... */
	    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			   GR_COMBINE_FACTOR_ONE,
			   GR_COMBINE_LOCAL_ITERATED,
			   GR_COMBINE_OTHER_TEXTURE,
			   FXFALSE);
	    }
	    else if(ctx->state->texture_mode[0].env_mode == GL_MODULATE &&
	       ctx->state->texture_mode[1].env_mode == GL_MODULATE)
	    {
	      grTexCombine(GR_TMU1,
			   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			   FXFALSE,FXFALSE);
			  
	      grTexCombine(GR_TMU0,
			   GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_LOCAL,
			   GR_COMBINE_FUNCTION_BLEND_OTHER,GR_COMBINE_FACTOR_LOCAL,
			   FXFALSE,FXFALSE);
	    
	    if(ctx->hw_texture[0]->baseInternalFormat == GL_RGB)
	      grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
			     GR_COMBINE_FACTOR_NONE,
			     GR_COMBINE_LOCAL_ITERATED,
			     GR_COMBINE_OTHER_NONE,
			     FXFALSE);
	    else
	      grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			     GR_COMBINE_FACTOR_ONE,
			     GR_COMBINE_LOCAL_ITERATED,
			     GR_COMBINE_OTHER_NONE,
			     FXFALSE);
	
	
	    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			   GR_COMBINE_FACTOR_ONE,
			   GR_COMBINE_LOCAL_ITERATED,
			   GR_COMBINE_OTHER_TEXTURE,
			   FXFALSE);
	    } 
	    else if(ctx->state->texture_mode[0].env_mode == GL_MODULATE &&
	       ctx->state->texture_mode[1].env_mode == GL_ADD)
	    {
	      
	      grTexCombine(GR_TMU1,
			   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			   GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			   FXFALSE,FXFALSE);
			  
	      grTexCombine(GR_TMU0,
			   GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,GR_COMBINE_FACTOR_ONE,
			   GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,GR_COMBINE_FACTOR_ONE,
			   FXFALSE,FXFALSE);
	    
	    if(ctx->hw_texture[0]->baseInternalFormat == GL_RGB)
	      grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
			     GR_COMBINE_FACTOR_NONE,
			     GR_COMBINE_LOCAL_ITERATED,
			     GR_COMBINE_OTHER_NONE,
			     FXFALSE);
	    else
	      grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			     GR_COMBINE_FACTOR_ONE,
			     GR_COMBINE_LOCAL_ITERATED,
			     GR_COMBINE_OTHER_NONE,
			     FXFALSE);
	
	
	    grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			   GR_COMBINE_FACTOR_ONE,
			   GR_COMBINE_LOCAL_ITERATED,
			   GR_COMBINE_OTHER_TEXTURE,
			   FXFALSE);
	   } else {
	#if 0			   
	      glr_debug_printf("unimplemented multitexture ENV: %08lx %08lx\n",
	        ctx->state->texture_mode[0].env_mode,
	        ctx->state->texture_mode[1].env_mode);
	#endif			        
	    }			    
	  }
	  break;
	  
	  /* Illegal cases (in theory) */
	  case GLR_TMU1_ACTIVE:
	  default:
	    DEBUG_PRINTF("This should not happen!\n");
	    break;
	}
}

/* For reference, from the "Red Book" 
**
**
** Base Internal Format       GL_REPLACE         GL_MODULATE               GL_DECAL                          GL_BLEND                    GL_ADD
**   
** GL_ALPHA                     C = Cf             C = Cf                 undefined                           C = Cf                     C = Cf
**                              A = At             A = At                                                  A = Af * At                 A = Af * At
**
** GL_LUMINANCE                 C = Lt           C = Cf * Lt              undefined                 C = Cf * (1 - Lt) + Cc * Lt        C = Cf + Lt
**                              A = Af             A = Af                                                     A = Af                     A = Af
**
** GL_LUMINANCE_ALPHA           C = Lt           C = Cf * Lt              undefined                 C = Cf * (1 - Lt) + Cc * Lt        C = Cf + Lt
**                              A = At           A = Af * At                                                A = Af * At                A = Af * At
**
** GL_INTENSITY                 C = It           C = Cf * It              undefined                 C = Cf * (1 - It) + Cc * It        C = Cf + It
**                              A = It           A = Af * It                                        A = Af * (1 - It) + Ac * It        A = Af + At
**
** GL_RGB                       C = Ct           C = Cf * Ct               C = Ct                   C = Cf * (1 - Ct) + Cc * Ct        C = Cf + Ct
**                              A = Af             A = Af                  A = Af                             A = Af                     A = Af
**
** GL_RGBA                      C = Ct           C = Cf * Ct      C = Cf * (1 - At) + Ct * At       C = Cf * (1 - Ct) + Cc * Ct        C = Cf + Ct
**                              A = At           A = Af * At               A = Af                           A = Af * At                A = Af * At
**
*/

static void glrConfigureCombineUnitsInternal(GrChipID_t tmu,  GrTCCUColor_t fragmentColor, GrTACUColor_t fragmentAlpha, 
                                             GLuint mode, const GLTColor4 *color, GLuint format)
{
	



  switch(mode) {
   case GL_REPLACE:	  
   if(format == GL_ALPHA) {
     grTexColorCombineExt(tmu,
                          fragmentColor, GR_FUNC_MODE_X,
                          GR_CMBX_ZERO,  GR_FUNC_MODE_X,
                          GR_CMBX_ZERO,  FXTRUE,
                          GR_CMBX_ZERO,  FXFALSE, 0, FXFALSE);
   } else {
     grTexColorCombineExt(tmu,
                          GR_CMBX_LOCAL_TEXTURE_RGB, GR_FUNC_MODE_X,
                          GR_CMBX_ZERO,              GR_FUNC_MODE_X,
                          GR_CMBX_ZERO,              FXTRUE,
                          GR_CMBX_ZERO,              FXFALSE, 0, FXFALSE);
   }
   if(format == GL_LUMINANCE || format == GL_RGB) {
     grTexAlphaCombineExt(tmu,
                          fragmentAlpha, GR_FUNC_MODE_X,
                          GR_CMBX_ZERO,  GR_FUNC_MODE_X,
                          GR_CMBX_ZERO,  FXTRUE,
                          GR_CMBX_ZERO,  FXFALSE, 0, FXFALSE);
   } else {
     grTexAlphaCombineExt(tmu,
                          GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
                          GR_CMBX_ZERO,                GR_FUNC_MODE_X,
                          GR_CMBX_ZERO,                FXTRUE,
                          GR_CMBX_ZERO,                FXFALSE, 0, FXFALSE);
   }  
  break;
  
  case GL_MODULATE:
  if(format == GL_ALPHA) {
    grTexColorCombineExt(tmu,
                         fragmentColor, GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  FXTRUE,
                         GR_CMBX_ZERO,  FXFALSE, 0, FXFALSE);
  } else {
    grTexColorCombineExt(tmu,
                         fragmentColor, GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  GR_FUNC_MODE_X,
                         GR_CMBX_LOCAL_TEXTURE_RGB, FXFALSE,
                         GR_CMBX_ZERO,  FXFALSE, 0, FXFALSE);
  }
  if(format == GL_LUMINANCE || format == GL_RGB) {
    grTexAlphaCombineExt(tmu,
                         fragmentAlpha, GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  FXTRUE,
                         GR_CMBX_ZERO,  FXFALSE, 0, FXFALSE); 
  } else {
    grTexAlphaCombineExt(tmu,
                         fragmentAlpha,              GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,               GR_FUNC_MODE_X,
                         GR_CMBX_LOCAL_TEXTURE_ALPHA,FXFALSE,
                         GR_CMBX_ZERO,               FXFALSE,
                         0, FXFALSE); 
  }
  break;
  
  case GL_DECAL:
  grTexColorCombineExt(tmu,
                       GR_CMBX_LOCAL_TEXTURE_RGB,   GR_FUNC_MODE_X,
                       fragmentColor,               GR_FUNC_MODE_NEGATIVE_X,
                       GR_CMBX_LOCAL_TEXTURE_ALPHA, FXFALSE,
                       GR_CMBX_B,                   FXFALSE, 
                       0, FXFALSE);
  grTexAlphaCombineExt(tmu,
                       fragmentAlpha,               GR_FUNC_MODE_X,
                       GR_CMBX_ZERO,                GR_FUNC_MODE_X,
                       GR_CMBX_ZERO,                FXTRUE,
                       GR_CMBX_ZERO,                FXFALSE,
                       0, FXFALSE);
  break;
    
  case GL_BLEND:
  grConstantColorValueExt(tmu, packARGB(color->a, color->r, color->g, color->b));
  if(format == GL_ALPHA) {
    grTexColorCombineExt(tmu,
                         fragmentColor, GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  FXTRUE,
                         GR_CMBX_ZERO,  FXFALSE, 0, FXFALSE);
  } else {
    grTexColorCombineExt(tmu,
                         GR_CMBX_TMU_CCOLOR,        GR_FUNC_MODE_X,
                         fragmentColor,             GR_FUNC_MODE_NEGATIVE_X,
                         GR_CMBX_LOCAL_TEXTURE_RGB, FXFALSE,
                         GR_CMBX_B,                 FXFALSE, 0, FXFALSE);
  }
  if(format == GL_INTENSITY) {
    grTexAlphaCombineExt(tmu,
                         GR_CMBX_TMU_CALPHA,          GR_FUNC_MODE_X,
                         fragmentAlpha,               GR_FUNC_MODE_X,
                         GR_CMBX_LOCAL_TEXTURE_ALPHA, FXFALSE,
                         GR_CMBX_ZERO,                FXFALSE,
                         0, FXFALSE); 
  } else {
    grTexAlphaCombineExt(tmu,
                         fragmentAlpha,                GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,                 GR_FUNC_MODE_X,
                         GR_CMBX_LOCAL_TEXTURE_ALPHA,  FXFALSE,
                         GR_CMBX_ZERO,                 FXFALSE, 
                         0, FXFALSE); 
  }
  break;
    
  case GL_ADD:
  if(format == GL_ALPHA) {
    grTexColorCombineExt(tmu,
                         fragmentColor, GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  FXTRUE,
                         GR_CMBX_ZERO,  FXFALSE,
                         0, FXFALSE);
  } else {
    grTexColorCombineExt(tmu,
                         fragmentColor,              GR_FUNC_MODE_X,
                         GR_CMBX_LOCAL_TEXTURE_RGB,  GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,               FXTRUE,
                         GR_CMBX_ZERO,               FXFALSE,
                         0, FXFALSE);
  }
  if(format == GL_LUMINANCE || format == GL_RGB) {
    grTexAlphaCombineExt(tmu,
                         fragmentAlpha, GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,  FXTRUE,
                         GR_CMBX_ZERO,  FXFALSE,
                         0, FXFALSE);
  } else if(format == GL_INTENSITY) {
    grTexAlphaCombineExt(tmu,
                         fragmentAlpha,               GR_FUNC_MODE_X,
                         GR_CMBX_LOCAL_TEXTURE_ALPHA, GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,                FXTRUE,
                         GR_CMBX_ZERO,                FXFALSE,
                         0, FXFALSE);
  } else {
    grTexAlphaCombineExt(tmu,
                         fragmentAlpha,               GR_FUNC_MODE_X,
                         GR_CMBX_ZERO,                GR_FUNC_MODE_X,
                         GR_CMBX_LOCAL_TEXTURE_ALPHA, FXFALSE,
                         GR_CMBX_ZERO,                FXFALSE,
                         0, FXFALSE);
  }
  break;
  }
}

void glrConfigureCombineUnitsNapalm(GLDContext ctx, GLuint activeTextureUnits)
{
  /* On Napalm we simply put the color combine unit into passthrough mode and
     do everything we need with the texture combine units. */
{
	
	if(ctx->combineUnitState != 1)
	{
  grColorCombineExt(GR_CMBX_TEXTURE_RGB, GR_FUNC_MODE_X,
                    GR_CMBX_ZERO, GR_FUNC_MODE_X,
                    GR_CMBX_ZERO, FXTRUE,
                    GR_CMBX_ZERO, FXFALSE,
                    0, FXFALSE);
  grAlphaCombineExt(GR_CMBX_TEXTURE_ALPHA, GR_FUNC_MODE_X,
                    GR_CMBX_ZERO, GR_FUNC_MODE_X,
                    GR_CMBX_ZERO, FXTRUE,
                    GR_CMBX_ZERO, FXFALSE,
                    0, FXFALSE);
		ctx->combineUnitState = 1;
	}
}  
  /* Do nasty texture/color combine stuff. */
  switch(activeTextureUnits) {
    case GLR_TMU0_ACTIVE: 
    {
 	  glrConfigureCombineUnitsInternal(GR_TMU0, GR_CMBX_ITRGB, GR_CMBX_ITALPHA, 
	                                   ctx->state->texture_mode[0].env_mode, 
	                                   &ctx->state->texture_mode[0].env_color, 
	                                   ctx->hw_texture[0]->baseInternalFormat);
	}
	break;

    case GLR_TMU0_ACTIVE|GLR_TMU1_ACTIVE:
    {
	  glrConfigureCombineUnitsInternal(GR_TMU1, GR_CMBX_ITRGB, GR_CMBX_ITALPHA, 
	                                            ctx->state->texture_mode[0].env_mode,
	                                            &ctx->state->texture_mode[0].env_color, 
	                                            ctx->hw_texture[0]->baseInternalFormat);
	  glrConfigureCombineUnitsInternal(GR_TMU0, GR_CMBX_OTHER_TEXTURE_RGB, GR_CMBX_OTHER_TEXTURE_ALPHA, 
	                                            ctx->state->texture_mode[1].env_mode, 
	                                            &ctx->state->texture_mode[1].env_color, 
	                                            ctx->hw_texture[1]->baseInternalFormat);
    }
    break;

    case GLR_TMU1_ACTIVE:
    default:
	    glr_debug_printf("This should not happen!\n");
    break;
  }    
}  	 	    


static void glrTextureState(GLDContext ctx)
{
    FxU32 activeTextureUnits;

	activeTextureUnits = glrLoadCurrentTexture(ctx);
	
	if(activeTextureUnits)
	{
		/* First do easy per-TMU config stuff */
		if(ctx->hw_texture[0]) {
		  glrConfigureTMU(GR_TMU0, ctx->hw_texture[0]->state);
		  grVertexLayout(GR_PARAM_ST0, 40, GR_PARAM_ENABLE);
		  grVertexLayout(GR_PARAM_Q0,  48, GR_PARAM_ENABLE);
		} else {
		  grVertexLayout(GR_PARAM_ST0, 40, GR_PARAM_DISABLE);
		  grVertexLayout(GR_PARAM_Q0,  48, GR_PARAM_DISABLE);
		}
		  
		if(ctx->hw_texture[1]) {
		  glrConfigureTMU(GR_TMU1, ctx->hw_texture[1]->state);			
 		  grVertexLayout(GR_PARAM_ST1, 52, GR_PARAM_ENABLE);
		  grVertexLayout(GR_PARAM_Q1,  60, GR_PARAM_ENABLE);
		} else {
 		  grVertexLayout(GR_PARAM_ST1, 52, GR_PARAM_DISABLE);
		  grVertexLayout(GR_PARAM_Q1,  60, GR_PARAM_DISABLE);
		}			
		
		ctx->configureCombineUnits(ctx, activeTextureUnits);			
	}
	else
	{
		grVertexLayout(GR_PARAM_ST0, 40, GR_PARAM_DISABLE);
		grVertexLayout(GR_PARAM_Q0,  48, GR_PARAM_DISABLE);
		grVertexLayout(GR_PARAM_ST1, 52, GR_PARAM_DISABLE);
		grVertexLayout(GR_PARAM_Q1,  60, GR_PARAM_DISABLE);
		
		ctx->combineUnitState = 0;

		grColorCombine(
			GR_COMBINE_FUNCTION_LOCAL, 
			GR_COMBINE_FACTOR_NONE,
			GR_COMBINE_LOCAL_ITERATED, 
			GR_COMBINE_OTHER_NONE, 
			FXFALSE
			);
		grAlphaCombine(
			GR_COMBINE_FUNCTION_LOCAL,              // combine function
			GR_COMBINE_FACTOR_NONE,					// combine factor
			GR_COMBINE_LOCAL_ITERATED, 				// local color
			GR_COMBINE_OTHER_NONE, 				    // other color
			FXFALSE									// invert
			);
		grTexCombine(GR_TMU0,
		             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			         FXFALSE,FXFALSE);
		grTexCombine(GR_TMU1,
		             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		             FXFALSE,FXFALSE);
	}
			
	// TODO : ctx->state->texture_mode.state.env.mode		
}	



void glrUpdateHardwareState(GLDContext ctx, GLbitfield change_flags)
{
	DEBUG_ENTRY( glrUpdateHardwareState );
	
	DEBUG_VERBOSE( glrUpdateHardwareState, "change flags = 0x%08x\n", change_flags );
	//DEBUG_PRINT_GLDSTATE( glrUpdateHardwareState, ctx->state );
	
	
	/* Check for needed updates */
	if(ctx->context_reinit)
	{
		change_flags |= GLD_STATE_ALL;
		ctx->context_reinit = GL_FALSE;
	}
	
	if(change_flags == 0) return;
	
#if SHOW_OVERDRAW
    grDisableAllEffects();
#endif
    		
#if SHOW_OVERDRAW
	grAlphaTestFunction(GR_CMP_ALWAYS);
#else			
	/* Set glide hardware state */
	if(change_flags & GLD_STATE_ALPHA_TEST)
	{
		if(ctx->state->alpha_test.enable)
			grAlphaTestFunction(glrCompareFunc(ctx->state->alpha_test.func));
		else
			grAlphaTestFunction(GR_CMP_ALWAYS);
		grAlphaTestReferenceValue(ctx->state->alpha_test.ref * 255.9f);
	}
#endif

#if SHOW_OVERDRAW || 0
    grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ONE, GR_BLEND_ZERO);
#else				
	if(change_flags & GLD_STATE_BLEND_MODE)
	{
		if(ctx->state->blend_mode.enable)
		{
			GrAlphaBlendFnc_t rgb_sf;
			GrAlphaBlendFnc_t rgb_df;
			GrAlphaBlendFnc_t alpha_sf;
			GrAlphaBlendFnc_t alpha_df;	       	        
	        
			switch(ctx->state->blend_mode.src)
			{
				case GL_ZERO:
					rgb_sf = GR_BLEND_ZERO;
					alpha_sf = GR_BLEND_ZERO;
				break;
				case GL_ONE:
					rgb_sf = GR_BLEND_ONE;
					alpha_sf = GR_BLEND_ONE;
				break;
				case GL_DST_COLOR:
					rgb_sf = GR_BLEND_DST_COLOR;
					alpha_sf = GR_BLEND_DST_COLOR;
				break;
				case GL_ONE_MINUS_DST_COLOR:
					rgb_sf = GR_BLEND_ONE_MINUS_DST_COLOR;
					alpha_sf = GR_BLEND_ONE_MINUS_DST_COLOR;
				break;
				case GL_SRC_ALPHA_SATURATE:
					// Alpha dest is not support with depth buffer
					// rgb_sf = GR_BLEND_ALPHA_SATURATE;
					// alpha_sf = GR_BLEND_ALPHA_SATURATE;
					
					rgb_sf = GR_BLEND_ONE;
					alpha_sf = GR_BLEND_ONE;
				break;
				case GL_SRC_ALPHA:
					rgb_sf = GR_BLEND_SRC_ALPHA;
					alpha_sf = GR_BLEND_SRC_ALPHA;
				break;
				case GL_ONE_MINUS_SRC_ALPHA:
					rgb_sf = GR_BLEND_ONE_MINUS_SRC_ALPHA;
					alpha_sf = GR_BLEND_ONE_MINUS_SRC_ALPHA;
				break;
				case GL_DST_ALPHA:
					// Alpha dest is not support with depth buffer
					// rgb_sf = GR_BLEND_DST_ALPHA;
					// alpha_sf = GR_BLEND_DST_ALPHA;
					
					rgb_sf = GR_BLEND_ONE;
					alpha_sf = GR_BLEND_ONE;
				break;
				default: /*case GL_ONE_MINUS_DST_ALPHA:*/
					// Alpha dest is not support with depth buffer
					// rgb_sf = GR_BLEND_ONE_MINUS_DST_ALPHA;
					// alpha_sf = GR_BLEND_ONE_MINUS_DST_ALPHA;
					
					rgb_sf = GR_BLEND_ONE;
					alpha_sf = GR_BLEND_ONE;
				break;
			}
			
			switch(ctx->state->blend_mode.dst)
			{
				case GL_ZERO:
					rgb_df = GR_BLEND_ZERO;
					alpha_df = GR_BLEND_ZERO;
				break;
				case GL_ONE:
					rgb_df = GR_BLEND_ONE;
					alpha_df = GR_BLEND_ONE;
				break;
				case GL_SRC_COLOR:
					rgb_df = GR_BLEND_SRC_COLOR;
					alpha_df = GR_BLEND_SRC_COLOR;
				break;
				case GL_ONE_MINUS_SRC_COLOR:
					rgb_df = GR_BLEND_ONE_MINUS_SRC_COLOR;
					alpha_df = GR_BLEND_ONE_MINUS_SRC_COLOR;
				break;
				case GL_SRC_ALPHA:
					rgb_df = GR_BLEND_SRC_ALPHA;
					alpha_df = GR_BLEND_SRC_ALPHA;
				break;
				case GL_ONE_MINUS_SRC_ALPHA:
					rgb_df = GR_BLEND_ONE_MINUS_SRC_ALPHA;
					alpha_df = GR_BLEND_ONE_MINUS_SRC_ALPHA;
				break;
				case GL_DST_ALPHA:
					// Alpha dest is not support with depth buffer
					// rgb_df = GR_BLEND_DST_ALPHA;
					// alpha_df = GR_BLEND_DST_ALPHA;
					
					rgb_df = GR_BLEND_ZERO;
					alpha_df = GR_BLEND_ZERO;
				break;
				default: /*case GL_ONE_MINUS_DST_ALPHA:*/
					// Alpha dest is not support with depth buffer
					// rgb_df = GR_BLEND_ONE_MINUS_DST_ALPHA;
					// alpha_df = GR_BLEND_ONE_MINUS_DST_ALPHA;
					
					rgb_df = GR_BLEND_ZERO;
					alpha_df = GR_BLEND_ZERO;
				break;
			}
			
//			grAlphaBlendFunction(rgb_sf, rgb_df, alpha_sf, alpha_df);
			grAlphaBlendFunction(rgb_sf, rgb_df, GR_BLEND_ONE, GR_BLEND_ZERO);
		}
		else
		{
			grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO);
		}
	}
#endif
	
	if(change_flags & GLD_STATE_CLEAR_COLOR)
	{
		// This is all done in glClear
	}
	
	if(change_flags & GLD_STATE_COLOR_BUFFER)
	{
		if(ctx->gliPixelFormat.buffer_mode & GLI_DOUBLEBUFFER_BIT){
		switch(ctx->state->color_buffer.draw)
		{
				case GL_FRONT_AND_BACK:
			case GL_LEFT:
			case GL_RIGHT:
					// not really supported
					grRenderBuffer(GR_BUFFER_BACKBUFFER);
				break;
				case GL_BACK:
			case GL_BACK_LEFT:
			case GL_BACK_RIGHT:
					grRenderBuffer(GR_BUFFER_BACKBUFFER);
				break;
				case GL_FRONT:
			case GL_FRONT_LEFT:
			case GL_FRONT_RIGHT:
				grRenderBuffer(GR_BUFFER_FRONTBUFFER);
			break;
		}
		switch(ctx->state->color_buffer.read)
		{
			case GL_FRONT_AND_BACK:
			case GL_LEFT:
			case GL_RIGHT:
					// not really supported
					ctx->readBuffer = GR_BUFFER_BACKBUFFER;
				break;
				case GL_BACK:
			case GL_BACK_LEFT:
			case GL_BACK_RIGHT:
			        ctx->readBuffer = GR_BUFFER_BACKBUFFER;
				break;
				case GL_FRONT:
			case GL_FRONT_LEFT:
			case GL_FRONT_RIGHT:
			    ctx->readBuffer = GR_BUFFER_FRONTBUFFER;
			break;
		}
		} else {
			grRenderBuffer(GR_BUFFER_FRONTBUFFER);
			ctx->readBuffer = GR_BUFFER_FRONTBUFFER;
		}
		
	}
	
#if SHOW_OVERDRAW
	grDepthBufferFunction(GR_CMP_ALWAYS);
#else
	if(change_flags & GLD_STATE_DEPTH_TEST)
	{
		FxBool mask;
		
		mask = ((ctx->state->mask_mode.depth_mask  && ctx->state->depth_test.enable) ? FXTRUE : FXFALSE);
		
		grDepthMask(mask);

		if(ctx->state->depth_test.enable)
		{
			grDepthBufferFunction(ctx->state->depth_test.func - GL_NEVER);
		}
		else
		{
			grDepthBufferFunction(GR_CMP_ALWAYS);
		}
	}
#endif
	
	if(change_flags & GLD_STATE_DITHER_MODE)
	{
		grDitherMode((ctx->state->dither_mode.enable ? GR_DITHER_4x4 : GR_DITHER_DISABLE));
	}

#if SHOW_OVERDRAW
          grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
                         GR_COMBINE_FACTOR_ONE,
                         GR_COMBINE_LOCAL_CONSTANT,
                         GR_COMBINE_OTHER_NONE,
                         FXFALSE);
          grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                         GR_COMBINE_FACTOR_ONE,
                         GR_COMBINE_LOCAL_CONSTANT,
                         GR_COMBINE_OTHER_NONE,
                         FXFALSE);
		  grTexCombine(GR_TMU0,
			             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
				         FXFALSE,FXFALSE);
		  grTexCombine(GR_TMU1,
			             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			             GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
			             FXFALSE,FXFALSE);
          grConstantColorValue(0x20202020);                         
#else					
	if(change_flags & GLD_STATE_TEXTURE_MODE)
	{
		glrTextureState(ctx);
	}
#endif
	
#if !SHOW_OVERDRAW	
	if(change_flags & GLD_STATE_FOG_MODE)
	{
		FxI32     fogTableEntries;
		GrColor_t fogcolor;
		GrFog_t * ft;
		FxI32 i;
		
		if(ctx->state->fog_mode.enable)
		{
			// Q param is only used for fog
			grVertexLayout(GR_PARAM_Q,   12, GR_PARAM_ENABLE);
			
			// set up fog mode
			grFogMode(GR_FOG_WITH_TABLE_ON_W);

			// set up the fog table
			grGet(GR_FOG_TABLE_ENTRIES, sizeof fogTableEntries, &fogTableEntries);
			ft = glmMalloc(fogTableEntries * sizeof (GrFog_t));
			for(i = 0; i < fogTableEntries; i++){
				ft[i] = 255.0 - 255.0 / guFogTableIndexToW(i);
			}			
			grFogTable(ft);
			glmFree(ft);

			// setup the fog color
			fogcolor = (GLuint)( ctx->state->fog_mode.color.a * 255.99) << 24;
			fogcolor |= (GLuint)( ctx->state->fog_mode.color.r * 255.99) << 16;
			fogcolor |= (GLuint)( ctx->state->fog_mode.color.g * 255.99) << 8;
			fogcolor |= (GLuint)( ctx->state->fog_mode.color.b * 255.99) << 0;
			grFogColorValue(fogcolor);
		}
		else
		{
			// Q param is only used for fog
			grVertexLayout(GR_PARAM_Q,   12, GR_PARAM_DISABLE);
			grFogMode(GR_FOG_DISABLE);
		}

	}
#endif
	
	if(change_flags & GLD_STATE_HINT_MODE)
	{
		// TODO : grHints(GrHint_t hintType, FxU32 hintMask);
	}
	
	if(change_flags & GLD_STATE_LINE_MODE)
	{
		if(ctx->state->line_mode.smooth_enable){
			grEnable(GR_AA_ORDERED_LINES_OGL);
		} else {
			grDisable(GR_AA_ORDERED_LINES_OGL);
		}
		// Not supported : ctx->state->line_mode.stipple_enable
		// Not supported : ctx->state->line_mode.stipple_pattern
	}
	
	if(change_flags & GLD_STATE_LOGIC_OP)
	{
		// Not supported
	}
	
#if SHOW_OVERDRAW
        if(grColorMaskExt)
        grColorMaskExt(FXTRUE, FXTRUE, FXTRUE, FXFALSE);
        grColorMask(FXTRUE, FXFALSE);
		grDepthMask(FXTRUE);
#else
	if(change_flags & GLD_STATE_MASK_MODE)
	{
		FxBool rgb_mask;
		FxBool depth_mask;
		GLuint stencilMask;
		
		// FIXME - See if color masks actually work in non 8888 modes.
		if(ctx->glideExtensions.gls_pix_ext && ctx->grPixelFormat == GR_PIXFMT_ARGB_8888) {
		  grColorMaskExt(ctx->state->mask_mode.color_red_mask,
		                 ctx->state->mask_mode.color_green_mask,
		                 ctx->state->mask_mode.color_blue_mask,
		                 ctx->state->mask_mode.color_alpha_mask);
          stencilMask = ctx->state->stencil_test.enable ? ctx->state->mask_mode.stencil_mask : 0;
		  grStencilMaskExt(stencilMask);
		} else {
		  rgb_mask = (ctx->state->mask_mode.color_red_mask && ctx->state->mask_mode.color_green_mask && ctx->state->mask_mode.color_blue_mask ? FXTRUE : FXFALSE);
		  grColorMask(rgb_mask, FXFALSE);
		}

		depth_mask = ((ctx->state->mask_mode.depth_mask && ctx->state->depth_test.enable) ? FXTRUE : FXFALSE);
		grDepthMask(depth_mask);
	}
#endif
	
	if(change_flags & GLD_STATE_PIXEL_MODE)
	{
		// Not supported
	}
	
	if(change_flags & GLD_STATE_POINT_MODE)
	{
		if(ctx->state->point_mode.smooth_enable){
			grEnable(GR_AA_ORDERED_POINTS_OGL);
		} else {
			grDisable(GR_AA_ORDERED_POINTS_OGL);
		}
	}
	
#if SHOW_OVERDRAW
    grCullMode(GR_CULL_DISABLE);
#else    	
	if(change_flags & GLD_STATE_POLYGON_MODE)
	{
		#define GLR_DEPTH_MIN_RES    1.0f
		#define GLR_DEPTH_MAX_DZ     GLR_DEPTH_MIN_RES

		GLfloat z_offset = ctx->state->polygon_mode.offset_factor * GLR_DEPTH_MAX_DZ +
						   ctx->state->polygon_mode.offset_units  * GLR_DEPTH_MIN_RES;


        // This is basically 100% wrong and out of spec.  Fix me, damnit.					             
		if(ctx->state->polygon_mode.offset_fill_enable) {
		    grDepthBiasLevel((FxU32)((FxI32)z_offset));
		}
		else {
		    grDepthBiasLevel(0);
        }
        
        /* We have to do our own culling, otherwise we won't be on the fast path code. */
        if(ctx->state->polygon_mode.cull_face_enable) {
	        // Ugh. Do culling too.
	        if(ctx->state->polygon_mode.cull_face_mode == GL_FRONT) {
	          if(ctx->state->polygon_mode.front_face == GL_CCW) {
	            grCullMode(GR_CULL_NEGATIVE);
	          } else {
	            grCullMode(GR_CULL_POSITIVE);
	          }
	        } else if (ctx->state->polygon_mode.cull_face_mode == GL_BACK) {
	          if(ctx->state->polygon_mode.front_face == GL_CCW) {
	            grCullMode(GR_CULL_POSITIVE);
	          } else {
	            grCullMode(GR_CULL_NEGATIVE);
	          }
	        } else if(ctx->state->polygon_mode.cull_face_mode == GL_FRONT_AND_BACK) {
	          /* TBI.  This should set some flag that the facet rendering code will use, unless
	             the GLI layer is smart enough to do this for us (but I doubt it). */
	        }	        
        } else {
          grCullMode(GR_CULL_DISABLE);
        }
        
		if(ctx->state->polygon_mode.smooth_enable){
			grEnable(GR_AA_ORDERED_TRIANGLES_OGL);
		} else {
			grDisable(GR_AA_ORDERED_TRIANGLES_OGL);
		}
		
		// Not supported : ctx->state->polygon_mode.stipple_enable
		// Not supported : ctx->state->polygon_mode.stipple
	}
#endif	
	if(change_flags & GLD_STATE_SCISSOR_TEST)
	{
		if(ctx->state->scissor_test.enable)
		{
			GLint l, t, r, b;
			
			/* Set scissor rect */
			l = ctx->state->scissor_test.box.x;
			t = ctx->viewPort.size.h - (ctx->state->scissor_test.box.y + ctx->state->scissor_test.box.h);
			
			r = l + ctx->state->scissor_test.box.w;
			b = t + ctx->state->scissor_test.box.h;
			
			/* Clip to buffer */
			if(l < 0) l = 0;
			if(t < 0) t = 0;
			if(r > ctx->viewPort.size.w) r = ctx->viewPort.size.w;
			if(b > ctx->viewPort.size.h) b = ctx->viewPort.size.h;
			
			grClipWindow(l, t, r, b);
		}
		else
		{
			grClipWindow(0, 0, ctx->viewPort.size.w, ctx->viewPort.size.h);
		}
	}
	
	if(change_flags & GLD_STATE_SHADE_MODEL)
	{
		// This is done in drawing funcs
	}
	
#if SHOW_OVERDRAW
    if(ctx->glideExtensions.gls_pix_ext)
        grDisable(GR_STENCIL_MODE_EXT);
#else              	
	if((change_flags & GLD_STATE_STENCIL_TEST) && ctx->glideExtensions.gls_pix_ext)
	{
	  if(ctx->state->stencil_test.enable) {
        grEnable(GR_STENCIL_MODE_EXT);
        grStencilFuncExt(ctx->state->stencil_test.func - GL_NEVER, ctx->state->stencil_test.ref, ctx->state->stencil_test.mask);
        grStencilOpExt(glrStencilOp(ctx->state->stencil_test.op_fail),
                       glrStencilOp(ctx->state->stencil_test.op_zfail),
                       glrStencilOp(ctx->state->stencil_test.op_zpass));
	  } else {
	    grDisable(GR_STENCIL_MODE_EXT);
	  }
	}
#endif	  
	
	if(grCommandTransportInfoExt){
		PROFILE_ENTRY("grCommandTransportInfoExt");
		grCommandTransportInfoExt();
		PROFILE_EXIT();
	}
}






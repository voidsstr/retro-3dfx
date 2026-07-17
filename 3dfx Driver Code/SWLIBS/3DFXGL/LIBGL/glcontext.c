/* 
** Copyright (c) 1997, 3Dfx Interactive, Inc. 
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
** 
** 
*/ 
#include <windows.h>
#include <glide.h>
#include <math.h>
#include <GL/gl.h>
#include "glint.h"

#include <stdio.h>

void glIntInitContext(pglContext Context)
{
  int i;

  Context->hdc = NULL;
  Context->hwnd = NULL;

  Context->MatMode = GL_MODELVIEW;

  Context->ModelViewPos = 0;
  glIntIdentity(Context->CurModelView);
  glIntIdentity(Context->CurInvTransp);
  Context->InvTranspDirty = FALSE;

  Context->ProjMatPos = 0;
  glIntIdentity(Context->CurProjMat);

  glIntIdentity(Context->CurComposite);
  Context->CompositeDirty = FALSE;
  Context->NeedEye = FALSE;
  Context->NeedEyeDirty = FALSE;

  Context->TexMatPos = 0;
  glIntIdentity(Context->CurTexMat);
  Context->TexMatrixIdentity = TRUE;

  Context->BeginEnd = FALSE;
  Context->GLerr = GL_NO_ERROR;

  // Vertex Array
  Context->VertexEnable = FALSE;
  Context->VertexPtr = NULL;
  Context->VertexStride = 16;
  Context->VertexSize = 4;
  Context->VertexType = GL_FLOAT;

  Context->NormalEnable = FALSE;
  Context->NormalPtr = NULL;
  Context->NormalStride = 12;
  Context->NormalType = GL_FLOAT;

  Context->ColorEnable = FALSE;
  Context->ColorPtr = NULL;
  Context->ColorStride = 16;
  Context->ColorSize = 4;
  Context->ColorType = GL_FLOAT;

  Context->TexEnable = FALSE;
  Context->TexPtr = NULL;
  Context->TexStride = 16;
  Context->TexSize = 4;
  Context->TexType = GL_FLOAT;

  Context->EdgeFlagEnable = FALSE;
  Context->EdgeFlagPtr = NULL;
  Context->EdgeFlagStride = 4;

  // 640x480, 16 bit Z
  Context->ViewX = 0;
  Context->ViewY = 0;
  Context->ViewWidth = 640;
  Context->ViewHeight = 480;
  Context->xScale = 640.0f/2.0f;
  Context->xOffset = 0.0f + (640.0f/2.0f)+SNAP_BIAS;
  Context->yScale = 480.0f/2.0f;
  Context->yOffset = 0.0f + (480.0f/2.0f)+SNAP_BIAS;
  Context->zScale = (1.0f - 0.0f)/2.0f;
  Context->zOffset = (0.0f + 1.0f)/2.0f;
  Context->zNear = 0.0f;
  Context->zFar = 1.0f; 
  
  Context->RedMask = TRUE;
  Context->GreenMask = TRUE;
  Context->BlueMask = TRUE;
  // Can't do alpha
  Context->AlphaMask = FALSE;
  Context->DepthMask = TRUE;

  // Current State
  Context->CurColor = Context->LocCurColor;
  Context->CurColor[0] = 1.0f;
  Context->CurColor[1] = 1.0f;
  Context->CurColor[2] = 1.0f;
  Context->CurColor[3] = 1.0f;

  Context->CurIndex = 1.0f;

  Context->CurTex = Context->LocCurTex;
  Context->CurTex[0] = 0.0f;
  Context->CurTex[1] = 0.0f;
  Context->CurTex[2] = 0.0f;
  Context->CurTex[3] = 1.0f;

  Context->CurNormal = Context->LocCurNormal;
  Context->CurNormal[0] = 0.0f;
  Context->CurNormal[1] = 0.0f;
  Context->CurNormal[2] = 1.0f;

  Context->CurObj = Context->LocCurObj;

  // Initial Clear State
  Context->ColorClrRed = 0.0f;
  Context->ColorClrGreen = 0.0f;
  Context->ColorClrBlue = 0.0f;
  Context->ClrColor = 0x0000;
  Context->ColorClrAlpha = 1.0f;
  Context->ClrAlpha = 0xff;
  Context->ClrIndex = 0.0f;
  Context->GLClrDepth = 1.0f;
  Context->ClrDepth = 0xffff;

  // initial enable state 
  Context->Normalize = TRUE;
  Context->DepthBuffer = FALSE;
  Context->StencilTest = FALSE;
  Context->ScissorEnable = FALSE;
  Context->Lighting = FALSE;
  Context->Local_Viewer = FALSE;
  Context->Two_Sided = FALSE;
  Context->ColorMaterial = FALSE;
  Context->CullEnable = FALSE;
  Context->PointAA = FALSE;
  Context->LineAA = FALSE;
  Context->PolyAA = FALSE;
  Context->AlphaTest = FALSE;
  Context->Blend = FALSE;
  Context->Tex1D = FALSE;
  Context->Tex2D = FALSE;
  Context->PolyOffset = FALSE;
  Context->LineOffset = FALSE;
  Context->PointOffset = FALSE;
  Context->Fog = FALSE;
  Context->AutoNormal = FALSE;
  Context->LineStipple = FALSE;
  Context->PolyStipple = FALSE;

  Context->CullMode = GL_BACK;
  Context->FrontFace = GL_CCW;

  Context->ShadeModel = GL_SMOOTH;

  Context->LightDirty = TRUE;

  Context->PolygonFrontMode = GL_FILL;
  Context->PolygonBackMode = GL_FILL;

  Context->minx = 0;
  Context->miny = 0;
  Context->maxx = 640;
  Context->maxy = 480;

  Context->RasObj[0] = 0.0f;
  Context->RasObj[1] = 0.0f;
  Context->RasObj[2] = 0.0f;
  Context->RasObj[3] = 1.0f;

  Context->RasVtx.Glide.x = 0.0f;
  Context->RasVtx.Glide.y = 0.0f;
  Context->RasVtx.Glide.z = 0.0f;

  Context->RasVtx.Color[0] = 1.0f;
  Context->RasVtx.Color[1] = 1.0f;
  Context->RasVtx.Color[2] = 1.0f;
  Context->RasVtx.Color[3] = 1.0f;

  Context->RasValid = TRUE;

  Context->gAlphaFunc = GR_CMP_ALWAYS;
  Context->AlphaFunc = GL_ALWAYS;
  Context->AlphaRef = 0;

  Context->gSrcBlend = GR_BLEND_ONE;
  Context->SrcBlend = GL_ONE;
  Context->gDstBlend = GR_BLEND_ZERO;
  Context->DstBlend = GL_ZERO;

  Context->gDepthFunc = GR_CMP_LESS;
  Context->DepthFunc = GL_LESS;
  Context->DepthBiasLevel = 0x0;

  Context->StencilZPass = GL_KEEP;
  Context->StencilZFail = GL_KEEP;
  Context->StencilFail = GL_KEEP;
  Context->StencilMask = 0xff;
  Context->StencilRef = 0x00;
  Context->StencilFuncMask = 0xff;
  Context->StencilFunc = GL_ALWAYS;

  // Buffer State
  if(curPixelFormat&1) {
    Context->DrawBuffer = GL_BACK;
    Context->ReadBuffer = GL_BACK;
  } else {
    Context->DrawBuffer = GL_FRONT;
    Context->ReadBuffer = GL_FRONT;
  }
  Context->RenderMode = GL_RENDER;

  // Fog State
  Context->FogMode = GL_EXP;
  Context->FogDensity = 1.0f;
  Context->FogStart = 0.0f;
  Context->FogEnd = 1.0f;
  Context->FogColor = 0x0;

  glIntInitFog(Context->FogTable, Context->FogMode, 
               Context->FogDensity, 
               Context->FogStart, Context->FogEnd);

  // Line State
  Context->LineStippleFactor = 1;
  Context->LinePattern = 0xffff;
  Context->LineWidth = 1.0f;


  // Model Clip
  for(i=0;i<6;i++) {
    Context->ClipEna[i] = GL_FALSE;
    Context->ClipDef[i][0] = 0.0f;
    Context->ClipDef[i][1] = 0.0f;
    Context->ClipDef[i][2] = 0.0f;
    Context->ClipDef[i][3] = 0.0f;
    Context->ClipXform[i][0] = 0.0f;
    Context->ClipXform[i][1] = 0.0f;
    Context->ClipXform[i][2] = 0.0f;
    Context->ClipXform[i][3] = 0.0f;
  }

  // Pixel State
  for(i=0;i<10;i++) {
    Context->PixelMaps[i][0] = 0.0f;
    Context->PixelMapSize[i] = 1;
  }

  Context->PixelZoomX = 1.0f;
  Context->PixelZoomY = 1.0f;

  Context->PackSwapBytes = FALSE;
  Context->PackLsbFirst = FALSE;
  Context->PackRowLength = 0;
  Context->PackSkipPixels = 0;
  Context->PackSkipRows = 0;
  Context->PackAlignment = 4;

  Context->UnPackSwapBytes = FALSE;
  Context->UnPackLsbFirst = FALSE;
  Context->UnPackRowLength = 0;
  Context->UnPackSkipPixels = 0;
  Context->UnPackSkipRows = 0;
  Context->UnPackAlignment = 4;

  Context->PixelMapColor = FALSE;
  Context->PixelMapStencil = FALSE;

  Context->PixelIndexShift = 0;
  Context->PixelIndexOffset = 0;

  Context->PixelRedScale = 1.0f;
  Context->PixelRedBias = 0.0f;
  Context->PixelGreenScale = 1.0f;
  Context->PixelGreenBias = 0.0f;
  Context->PixelBlueScale = 1.0f;
  Context->PixelBlueBias = 0.0f;
  Context->PixelAlphaScale = 1.0f;
  Context->PixelAlphaBias = 0.0f;
  Context->PixelDepthScale = 1.0f;
  Context->PixelDepthBias = 0.0f;

  glIntInitAccum(Context);

  // Attribute Stack
  Context->AttribDepth = 0;

  // Display List
  Context->ListHead = NULL;
  Context->Listing = FALSE;
  Context->Execute = GL_COMPILE;
  Context->ExecLevel = 0;
  Context->ListBase = 0;

  // Select
  Context->SelectBuffer = NULL;
  Context->SelectDepth = -1;

  // Feedback
  Context->FeedbackBuffer = NULL;

  Context->ContextDirty = TRUE;

  for(i=0;i<8;i++)
    Context->LightEna[i] = FALSE;

  glIntInitLighting(Context);

  glIntInitTexture(Context);

}

void glIntValidateContext()
{
  if(pglCurContext->ContextDirty) {

    if(pglCurContext->CullEnable) {
      if(pglCurContext->FrontFace == GL_CCW) {
	if(pglCurContext->CullMode == GL_FRONT) {
	  grCullMode(GR_CULL_POSITIVE);
	} else {
	  grCullMode(GR_CULL_NEGATIVE);
	}
      } else {
	if(pglCurContext->CullMode == GL_FRONT) {
	  grCullMode(GR_CULL_NEGATIVE);
	} else {
	  grCullMode(GR_CULL_POSITIVE);
	}
      }
    } else {
      grCullMode(GR_CULL_DISABLE);
    }

    if(pglCurContext->AlphaTest) {
      grAlphaTestReferenceValue(pglCurContext->AlphaRef);
      grAlphaTestFunction(pglCurContext->gAlphaFunc);
    } else {
      grAlphaTestFunction(GR_CMP_ALWAYS);
    }

    grColorMask(pglCurContext->RedMask,pglCurContext->AlphaMask);

    if(pglCurContext->DepthBuffer) {
      grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
      grDepthMask(pglCurContext->DepthMask);
      grDepthBufferFunction(pglCurContext->gDepthFunc);
    } else {
      grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
      grDepthMask(FXFALSE);
    }

    switch(pglCurContext->PrimAssemState) {
    case PA_POINT1:
      if(pglCurContext->PointOffset) {
	grDepthBiasLevel(pglCurContext->DepthBiasLevel);
      } else {
	grDepthBiasLevel(0);
      }
      break;
    case PA_LINE1:
    case PA_LINE_STRIP1:
    case PA_LINE_LOOP1:
      if(pglCurContext->LineOffset) {
	grDepthBiasLevel(pglCurContext->DepthBiasLevel);
      } else {
	grDepthBiasLevel(0);
      }
      break;
    case PA_TRIANGLES1:
    case PA_TRIANGLE_STRIP1:
    case PA_TRIANGLE_FAN1:
    case PA_QUADS1:
    case PA_QUAD_STRIP1:
    case PA_POLYGON1:
      if(pglCurContext->PolyOffset) {
	grDepthBiasLevel(pglCurContext->DepthBiasLevel);
      } else {
	grDepthBiasLevel(0);
      }
      break;
    }

    if(pglCurContext->ScissorEnable) {
      grClipWindow(pglCurContext->minx,
		   pglCurContext->miny,
		   pglCurContext->maxx,
		   pglCurContext->maxy);
    } else {
      /* MARK DELTAS */
      grClipWindow(0,0,pglCurContext->WindowWidth,pglCurContext->WindowHeight);
      /* END MARK DELTAS */
    }

    if(pglCurContext->Fog) {
      grFogMode(GR_FOG_WITH_TABLE);
      grFogColorValue(pglCurContext->FogColor);
      grFogTable(pglCurContext->FogTable);
    } else {
      grFogMode(GR_FOG_DISABLE);
    }

    if(pglCurContext->Blend) {
      // enable alpha blending
      grAlphaBlendFunction(pglCurContext->gSrcBlend, pglCurContext->gDstBlend,
			   pglCurContext->gSrcBlend, pglCurContext->gDstBlend);
    } else {
      // disable alpha blending
      grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
			   GR_BLEND_ONE, GR_BLEND_ZERO);
    }

    switch(pglCurContext->ShadeModel) {
    case GL_SMOOTH:
      if(pglCurContext->Tex2D) {
	grTexCombine(GR_TMU0,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     FXFALSE,
		     FXFALSE );

	switch(pglCurContext->TexMode) {
	case GL_REPLACE:
	  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_ONE,
			 GR_COMBINE_LOCAL_NONE,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_ITERATED,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  break;
	case GL_MODULATE:
	  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_ITERATED,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_ITERATED,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  break;
	case GL_DECAL:
	  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_ONE,
			 GR_COMBINE_LOCAL_NONE,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE );
	  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_ITERATED,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  break;
	case GL_BLEND:
	  grColorCombine(GR_COMBINE_FUNCTION_BLEND,
			 GR_COMBINE_FACTOR_TEXTURE_ALPHA,
			 GR_COMBINE_LOCAL_CONSTANT,
			 GR_COMBINE_OTHER_ITERATED,
			 FXFALSE );
	  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_ITERATED,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  break;
	}
      } else {
	// smooth shading - no texture
	grTexCombine(GR_TMU0,
		     GR_COMBINE_FUNCTION_NONE,
		     GR_COMBINE_FACTOR_NONE,
		     GR_COMBINE_FUNCTION_NONE,
		     GR_COMBINE_FACTOR_NONE,
		     FXFALSE,
		     FXFALSE );

        grColorCombine(GR_COMBINE_FUNCTION_BLEND,
		       GR_COMBINE_FACTOR_LOCAL_ALPHA,
		       GR_COMBINE_LOCAL_ITERATED,
		       GR_COMBINE_OTHER_ITERATED,
		       FXFALSE);
	grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
		       GR_COMBINE_FACTOR_NONE,
		       GR_COMBINE_LOCAL_ITERATED,
		       GR_COMBINE_OTHER_CONSTANT,
		       FXFALSE);
      }
      break;
    case GL_FLAT:
      if(pglCurContext->Tex2D) {
	grTexCombine(GR_TMU0,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     FXFALSE,
		     FXFALSE );

	switch(pglCurContext->TexMode) {
	case GL_REPLACE:
	  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_ONE,
			 GR_COMBINE_LOCAL_NONE,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_ITERATED,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  break;
	case GL_MODULATE:
	  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_CONSTANT,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_ITERATED,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  break;
	case GL_DECAL:
	  grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_ONE,
			 GR_COMBINE_LOCAL_NONE,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE );
	  grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_ITERATED,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  break;
	case GL_BLEND:
	  grColorCombine(GR_COMBINE_FUNCTION_BLEND,
			 GR_COMBINE_FACTOR_TEXTURE_ALPHA,
			 GR_COMBINE_LOCAL_CONSTANT,
			 GR_COMBINE_OTHER_ITERATED,
			 FXFALSE );
	  grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
			 GR_COMBINE_FACTOR_LOCAL,
			 GR_COMBINE_LOCAL_ITERATED,
			 GR_COMBINE_OTHER_TEXTURE,
			 FXFALSE);
	  break;
	}
      } else {
	// flat shading - no texture
	grTexCombine(GR_TMU0,
		     GR_COMBINE_FUNCTION_NONE,
		     GR_COMBINE_FACTOR_NONE,
		     GR_COMBINE_FUNCTION_NONE,
		     GR_COMBINE_FACTOR_NONE,
		     FXFALSE,
		     FXFALSE );
        grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
			 GR_COMBINE_FACTOR_LOCAL_ALPHA,
			 GR_COMBINE_LOCAL_CONSTANT,
			 GR_COMBINE_OTHER_CONSTANT,
			 FXFALSE);
	grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		       GR_COMBINE_FACTOR_ONE,
		       GR_COMBINE_LOCAL_ITERATED,
		       GR_COMBINE_OTHER_CONSTANT,
		       FXFALSE);
      }
      break;
    }

#define STIPPLE
#ifdef STIPPLE
    // Polygon stipple
    if(pglCurContext->PolyStipple) {
      // Stippling only works with no texture and no alpha compare
      if(!pglCurContext->AlphaTest && !pglCurContext->Tex2D &&
	 !pglCurContext->Blend && pglCurContext->CurPoly) {
        grAlphaTestReferenceValue(0x80);
        grAlphaTestFunction(GR_CMP_GREATER);
	
        grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		       GR_COMBINE_FACTOR_LOCAL,
		       GR_COMBINE_LOCAL_ITERATED,
		       GR_COMBINE_OTHER_TEXTURE,
		       FXFALSE);

	grTexCombine(GR_TMU0,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     FXFALSE,
		     FXFALSE );

	grTexMipMapMode(GR_TMU0,
			GR_MIPMAP_DISABLE,
			FXFALSE);
	
	if(pglCurContext->PolyStippleDirty) {
	  grTexDownloadMipMap(GR_TMU0,
			      pglCurContext->MaxCacheAddr+(16*2),
			      GR_MIPMAPLEVELMASK_BOTH,
			      &(pglCurContext->PolyStippleTex));
	  pglCurContext->PolyStippleDirty = FALSE;
	}
	
	grTexSource(GR_TMU0,
		    pglCurContext->MaxCacheAddr+(16*2),
		    GR_MIPMAPLEVELMASK_BOTH,
		    &(pglCurContext->PolyStippleTex));
	
	grTexClampMode(GR_TMU0,
		       GR_TEXTURECLAMP_WRAP,
		       GR_TEXTURECLAMP_WRAP);
	
	grTexFilterMode(GR_TMU0,
			GR_TEXTUREFILTER_POINT_SAMPLED,
			GR_TEXTUREFILTER_POINT_SAMPLED);
      }
    }

    // Line stipple
    if(pglCurContext->LineStipple) {
      // Stippling only works with no texture and no alpha compare
      if(!pglCurContext->AlphaTest && !pglCurContext->Tex2D &&
	 !pglCurContext->Blend && pglCurContext->CurLine) {
        grAlphaTestReferenceValue(0x80);
        grAlphaTestFunction(GR_CMP_GREATER);
	
        grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		       GR_COMBINE_FACTOR_LOCAL,
		       GR_COMBINE_LOCAL_ITERATED,
		       GR_COMBINE_OTHER_TEXTURE,
		       FXFALSE);

	grTexCombine(GR_TMU0,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     GR_COMBINE_FUNCTION_LOCAL,
		     GR_COMBINE_FACTOR_NONE,
		     FXFALSE,
		     FXFALSE );

	grTexMipMapMode(GR_TMU0,
			GR_MIPMAP_DISABLE,
			FXFALSE);
	
	if(pglCurContext->LineStippleDirty) {
	  grTexDownloadMipMap(GR_TMU0,
			      pglCurContext->MaxCacheAddr,
			      GR_MIPMAPLEVELMASK_BOTH,
			      &(pglCurContext->LineStippleTex));
	  pglCurContext->LineStippleDirty = FALSE;
	}
	
	grTexSource(GR_TMU0,
		    pglCurContext->MaxCacheAddr,
		    GR_MIPMAPLEVELMASK_BOTH,
		    &(pglCurContext->LineStippleTex));
	
	grTexClampMode(GR_TMU0,
		       GR_TEXTURECLAMP_WRAP,
		       GR_TEXTURECLAMP_CLAMP);
	
	grTexFilterMode(GR_TMU0,
			GR_TEXTUREFILTER_POINT_SAMPLED,
			GR_TEXTUREFILTER_POINT_SAMPLED);
      }
    }
#endif

    switch(pglCurContext->DrawBuffer) {
    case GL_NONE:
      break;
    case GL_FRONT:
    case GL_FRONT_LEFT:
    case GL_FRONT_RIGHT:
      grRenderBuffer(GR_BUFFER_FRONTBUFFER);
      break;
    case GL_BACK:
    case GL_BACK_LEFT:
    case GL_BACK_RIGHT:
      if(curPixelFormat&1) {
	grRenderBuffer(GR_BUFFER_BACKBUFFER);
      }
      break;
    case GL_FRONT_AND_BACK:
    case GL_LEFT:
    case GL_RIGHT:
      break;
    default:
      break;
    }

    pglCurContext->ContextDirty = FALSE;
  }
}

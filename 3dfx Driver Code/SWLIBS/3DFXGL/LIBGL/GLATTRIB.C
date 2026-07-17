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

void APIENTRY glPopAttrib (void)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    
    glIntIntToList(OP_POPATTRIB);
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if(pglCurContext->AttribDepth > 0) {
    pglCurContext->AttribDepth--;
    glIntCopyAttribToContext(
      &pglCurContext->AttribStack[pglCurContext->AttribDepth]);
    
  } else {
    GLINT_ERROR(GL_STACK_UNDERFLOW);
  }
}

void APIENTRY glPushAttrib (GLbitfield mask)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    
    glIntIntToList(OP_PUSHATTRIB);
    glIntIntToList((int)mask);
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if(pglCurContext->AttribDepth < 16) {
    glIntCopyContextToAttrib(
      &pglCurContext->AttribStack[pglCurContext->AttribDepth],
      mask);
    pglCurContext->AttribDepth++;
  } else {
    GLINT_ERROR(GL_STACK_OVERFLOW);
  }
}

void APIENTRY glPopClientAttrib (void)
{
}

void APIENTRY glPushClientAttrib (GLbitfield mask)
{
}

void glIntCopyAttribToContext(pglAttrib Attr)
{
  int i,j;

  if(Attr->mask&GL_CURRENT_BIT) {
    for(i=0;i<4;i++) pglCurContext->LocCurColor[i] = Attr->CurColor[i];
    for(i=0;i<4;i++) pglCurContext->LocCurTex[i] = Attr->CurTex[i];
    for(i=0;i<3;i++) pglCurContext->LocCurNormal[i] = Attr->CurNormal[i];
    
    pglCurContext->CurColor = pglCurContext->LocCurColor;
    pglCurContext->CurTex = pglCurContext->LocCurTex;
    pglCurContext->CurNormal = pglCurContext->LocCurNormal;
    pglCurContext->CurObj = pglCurContext->LocCurObj;

    for(i=0;i<4;i++) pglCurContext->RasObj[i] = Attr->RasObj[i];

    // Raster Eye Distance
    pglCurContext->RasVtx.Color[0] = Attr->RasRed;
    pglCurContext->RasVtx.Color[1] = Attr->RasGreen;
    pglCurContext->RasVtx.Color[2] = Attr->RasBlue;
    pglCurContext->RasVtx.Color[3] = Attr->RasAlpha;
    // Raster Index
    for(i=0;i<4;i++) pglCurContext->RasVtx.Texture[i] = Attr->RasTex[i];
    pglCurContext->RasValid = Attr->RasValid;
  }  
  
  if(Attr->mask&GL_VIEWPORT_BIT) {
    pglCurContext->ViewX = Attr->ViewX;
    pglCurContext->ViewY = Attr->ViewY;
    pglCurContext->ViewWidth = Attr->ViewWidth;
    pglCurContext->ViewHeight = Attr->ViewHeight;
    pglCurContext->xScale = Attr->xScale;
    pglCurContext->xOffset = Attr->xOffset;
    pglCurContext->yScale = Attr->yScale;
    pglCurContext->yOffset = Attr->yOffset;
    pglCurContext->zScale = Attr->zScale;
    pglCurContext->zOffset = Attr->zOffset;
    pglCurContext->zNear = Attr->zNear;
    pglCurContext->zFar = Attr->zFar;
  }

  if(Attr->mask&GL_TRANSFORM_BIT) {
    pglCurContext->MatMode = Attr->MatMode;
    for(i=0;i<6;i++) for(j=0;j<4;j++) pglCurContext->ClipDef[i][j] = 
      Attr->ClipDef[i][j];
    for(i=0;i<6;i++) for(j=0;j<4;j++) pglCurContext->ClipXform[i][j] = 
      Attr->ClipXform[i][j];
    pglCurContext->Normalize = Attr->Normalize;
    for(i=0;i<6;i++) pglCurContext->ClipEna[i] = Attr->ClipEna[i];
  }

  if(Attr->mask&GL_FOG_BIT) {
    for(i=0;i<GR_FOG_TABLE_SIZE;i++) pglCurContext->FogTable[i] = 
      Attr->FogTable[i];
    pglCurContext->FogMode = Attr->FogMode;
    pglCurContext->FogDensity = Attr->FogDensity;
    pglCurContext->FogStart = Attr->FogStart;
    pglCurContext->FogEnd = Attr->FogEnd;
    pglCurContext->FogColor = Attr->FogColor;
    pglCurContext->Fog = Attr->Fog;
  }

  if(Attr->mask&GL_LIGHTING_BIT) {
    pglCurContext->ShadeModel = Attr->ShadeModel;
    pglCurContext->ColorMaterialMode = Attr->ColorMaterialMode;
    pglCurContext->ColorMaterialFace = Attr->ColorMaterialFace;
    for(i=0;i<3;i++) pglCurContext->FrontAmbient[i] = Attr->FrontAmbient[i];
    for(i=0;i<3;i++) pglCurContext->FrontDiffuse[i] = Attr->FrontDiffuse[i];
    for(i=0;i<3;i++) pglCurContext->FrontSpecular[i] = Attr->FrontSpecular[i];
    for(i=0;i<3;i++) pglCurContext->FrontEmission[i] = Attr->FrontEmission[i];
    pglCurContext->FrontShininess = Attr->FrontShininess;
    for(i=0;i<3;i++) pglCurContext->BackAmbient[i] = Attr->BackAmbient[i];
    for(i=0;i<3;i++) pglCurContext->BackDiffuse[i] = Attr->BackDiffuse[i];
    for(i=0;i<3;i++) pglCurContext->BackSpecular[i] = Attr->BackSpecular[i];
    for(i=0;i<3;i++) pglCurContext->BackEmission[i] = Attr->BackEmission[i];
    pglCurContext->BackShininess = Attr->BackShininess;
    for(i=0;i<3;i++) pglCurContext->LightModelAmbient[i] = 
      Attr->LightModelAmbient[i];
    pglCurContext->Local_Viewer = Attr->Local_Viewer;
    pglCurContext->Two_Sided = Attr->Two_Sided;
    for(i=0;i<8;i++) pglCurContext->Light[i] = Attr->Light[i];
    pglCurContext->Lighting = Attr->Lighting;
    pglCurContext->ColorMaterial = Attr->ColorMaterial;
    pglCurContext->LightEna[8] = Attr->LightEna[8];

    pglCurContext->LightDirty = TRUE;
  }

  if(Attr->mask&GL_POINT_BIT) {
    pglCurContext->PointWidth = Attr->PointWidth;
    pglCurContext->PointAA = Attr->PointAA;
  }
  
  if(Attr->mask&GL_LINE_BIT) {
    pglCurContext->LineStippleFactor = Attr->LineStippleFactor;
    pglCurContext->LinePattern = Attr->LinePattern;
    pglCurContext->LineWidth = Attr->LineWidth;
    pglCurContext->LineAA = Attr->LineAA;
    pglCurContext->LineStipple = Attr->LineStipple;
  }

  if(Attr->mask&GL_POLYGON_BIT) {
    pglCurContext->CullMode = Attr->CullMode;
    pglCurContext->FrontFace = Attr->FrontFace;
    pglCurContext->PolygonFrontMode = Attr->PolygonFrontMode;
    pglCurContext->PolygonBackMode = Attr->PolygonBackMode;
    // Polygon Offset
    // Polygon Bias
    // End Polygon
    // Start Polygon/Enable
    pglCurContext->CullEnable = Attr->CullEnable;
    pglCurContext->PolyAA = Attr->PolyAA;
    // Polygon offset point
    // Polygon offset line
    // Polygon offset fill
    pglCurContext->PolyStipple = Attr->PolyStipple;
  }

  if(Attr->mask&GL_POLYGON_STIPPLE_BIT) {
    // Polygon stipple value
  }

  if(Attr->mask&GL_TEXTURE_BIT) {
    // Start Texture
    pglCurContext->Tex1DName = Attr->Tex1DName;
    pglCurContext->Tex1DPtr = Attr->Tex1DPtr;
    pglCurContext->Tex2DName = Attr->Tex2DName;
    pglCurContext->Tex2DPtr = Attr->Tex2DPtr;
    for(i=0;i<4;i++) pglCurContext->TexEnvColor[4] = Attr->TexEnvColor[4];
    pglCurContext->TexMode = Attr->TexMode;
    // Texture Priority
    // Texture Eye Linear
    // Texture Object Linear
    // Texture Gen Mode
    pglCurContext->Tex1D = Attr->Tex1D;
    pglCurContext->Tex2D = Attr->Tex2D;
    // Texture Gen
  }

  if(Attr->mask&GL_SCISSOR_BIT) {
    // Start Scissor
    pglCurContext->minx = Attr->minx;
    pglCurContext->miny = Attr->miny;
    pglCurContext->maxx = Attr->maxx;
    pglCurContext->maxy = Attr->maxy;
    pglCurContext->ScissorEnable = Attr->ScissorEnable;
  }

  if(Attr->mask&GL_COLOR_BUFFER_BIT) {
    pglCurContext->AlphaFunc = Attr->AlphaFunc;
    pglCurContext->gAlphaFunc = Attr->gAlphaFunc;
    pglCurContext->AlphaRef = Attr->AlphaRef;
    pglCurContext->SrcBlend = Attr->SrcBlend;
    pglCurContext->gSrcBlend = Attr->gSrcBlend;
    pglCurContext->DstBlend = Attr->DstBlend;
    pglCurContext->gDstBlend = Attr->gDstBlend;
    // Logic OP Mode
    pglCurContext->DrawBuffer = Attr->DrawBuffer;
    pglCurContext->RedMask = Attr->RedMask; 
    pglCurContext->GreenMask = Attr->GreenMask; 
    pglCurContext->BlueMask = Attr->BlueMask; 
    pglCurContext->AlphaMask = Attr->AlphaMask; 
    // Index Mask
    pglCurContext->ColorClrRed = Attr->ColorClrRed;
    pglCurContext->ColorClrGreen = Attr->ColorClrGreen;
    pglCurContext->ColorClrBlue = Attr->ColorClrBlue;
    pglCurContext->ClrColor = Attr->ClrColor;
    pglCurContext->ColorClrAlpha = Attr->ColorClrAlpha;
    pglCurContext->ClrAlpha = Attr->ClrAlpha;
    pglCurContext->ClrIndex = Attr->ClrIndex;
    pglCurContext->AlphaTest = Attr->AlphaTest;
    pglCurContext->Blend = Attr->Blend;
    // Dither
    // Index Logic OP
    // Color Logic OP
  }

  if(Attr->mask&GL_STENCIL_BUFFER_BIT) {
    pglCurContext->StencilZPass = Attr->StencilZPass;
    pglCurContext->StencilZFail = Attr->StencilZFail;
    pglCurContext->StencilFail = Attr->StencilFail;
    pglCurContext->StencilFunc = Attr->StencilFunc;
    pglCurContext->StencilFuncMask = Attr->StencilFuncMask;
    pglCurContext->StencilRef = Attr->StencilRef;
    pglCurContext->StencilMask = Attr->StencilMask;
    pglCurContext->ClrStencil = Attr->ClrStencil;
    // Stencil Mask
    pglCurContext->StencilTest = Attr->StencilTest;
  }

  if(Attr->mask&GL_DEPTH_BUFFER_BIT) {
    pglCurContext->DepthFunc = Attr->DepthFunc;
    pglCurContext->gDepthFunc = Attr->gDepthFunc;
    pglCurContext->GLClrDepth = Attr->GLClrDepth;
    pglCurContext->ClrDepth = Attr->ClrDepth;
    pglCurContext->DepthMask = Attr->DepthMask; 
    pglCurContext->DepthBuffer = Attr->DepthBuffer;
  }

  if(Attr->mask&GL_ACCUM_BUFFER_BIT) {
    pglCurContext->AccumClrRed = Attr->AccumClrRed;
    pglCurContext->AccumClrGreen = Attr->AccumClrGreen;
    pglCurContext->AccumClrBlue = Attr->AccumClrBlue;
    pglCurContext->AccumClrAlpha = Attr->AccumClrAlpha;
  }

  if(Attr->mask&GL_LIST_BIT) {
    pglCurContext->ListBase = Attr->ListBase;
  }

  if(Attr->mask&GL_EVAL_BIT) {
  }

  if(Attr->mask&GL_HINT_BIT) {
  }

  if(Attr->mask&GL_PIXEL_MODE_BIT) {
    pglCurContext->PixelMapColor = Attr->PixelMapColor;
    pglCurContext->PixelMapStencil = Attr->PixelMapStencil;
    pglCurContext->PixelIndexShift = Attr->PixelIndexShift;
    pglCurContext->PixelIndexOffset = Attr->PixelIndexOffset;
    pglCurContext->PixelRedScale = Attr->PixelRedScale;
    pglCurContext->PixelRedBias = Attr->PixelRedBias;
    pglCurContext->PixelGreenScale = Attr->PixelGreenScale;
    pglCurContext->PixelGreenBias = Attr->PixelGreenBias;
    pglCurContext->PixelBlueScale = Attr->PixelBlueScale;
    pglCurContext->PixelBlueBias = Attr->PixelBlueBias;
    pglCurContext->PixelAlphaScale = Attr->PixelAlphaScale;
    pglCurContext->PixelAlphaBias = Attr->PixelAlphaBias;
    pglCurContext->PixelDepthScale = Attr->PixelDepthScale;
    pglCurContext->PixelDepthBias = Attr->PixelDepthBias;
    pglCurContext->PixelZoomX = Attr->PixelZoomX;
    pglCurContext->PixelZoomY = Attr->PixelZoomY;
    for(i=0;i<10;i++) for(j=0;j<256;j++) pglCurContext->PixelMaps[i][j] = 
      Attr->PixelMaps[i][j];
    for(i=0;i<10;i++) pglCurContext->PixelMapSize[i] = 
      Attr->PixelMapSize[i]; 
    pglCurContext->ReadBuffer = Attr->ReadBuffer;
  }
  
  if(Attr->mask&GL_ENABLE_BIT) {
    pglCurContext->Normalize = Attr->Normalize;
    for(i=0;i<6;i++) pglCurContext->ClipEna[i] = Attr->ClipEna[i];
    pglCurContext->Fog = Attr->Fog;
    pglCurContext->Lighting = Attr->Lighting;
    pglCurContext->ColorMaterial = Attr->ColorMaterial;
    for(i=0;i<8;i++) pglCurContext->LightEna[i] = Attr->LightEna[i];
    pglCurContext->PointAA = Attr->PointAA;
    pglCurContext->LineAA = Attr->LineAA;
    pglCurContext->LineStipple = Attr->LineStipple;
    pglCurContext->CullEnable = Attr->CullEnable;
    pglCurContext->PolyAA = Attr->PolyAA;
    // Polygon offset point
    // Polygon offset line
    // Polygon offset fill
    pglCurContext->PolyStipple = Attr->PolyStipple;
    pglCurContext->Tex1D = Attr->Tex1D;
    pglCurContext->Tex2D = Attr->Tex2D;
    // Texture Gen
    pglCurContext->ScissorEnable = Attr->ScissorEnable;
    pglCurContext->AlphaTest = Attr->AlphaTest;
    pglCurContext->Blend = Attr->Blend;
    // Dither
    // Index Logic OP
    // Color Logic OP
    pglCurContext->StencilTest = Attr->StencilTest;
    pglCurContext->DepthBuffer = Attr->DepthBuffer;
  }
}

void glIntCopyContextToAttrib(pglAttrib Attr, GLbitfield mask)
{
  int i,j;

  Attr->mask = mask;

  if(mask&GL_CURRENT_BIT) {
    for(i=0;i<4;i++) Attr->CurColor[i] = pglCurContext->CurColor[i];
    Attr->CurIndex = pglCurContext->CurIndex;
    for(i=0;i<4;i++) Attr->CurTex[i] = pglCurContext->CurTex[i];
    for(i=0;i<3;i++) Attr->CurNormal[i] = pglCurContext->CurNormal[i];
    
    for(i=0;i<4;i++) Attr->RasObj[i] = pglCurContext->RasObj[i];
    // Raster Eye Distance
    Attr->RasRed = pglCurContext->RasVtx.Color[0];
    Attr->RasGreen = pglCurContext->RasVtx.Color[1];
    Attr->RasBlue = pglCurContext->RasVtx.Color[2];
    Attr->RasAlpha = pglCurContext->RasVtx.Color[3];
    // Raster Index
    for(i=0;i<4;i++) Attr->RasTex[i] = pglCurContext->RasVtx.Texture[i];
    Attr->RasValid = pglCurContext->RasValid;
  }  
  
  if(mask&GL_VIEWPORT_BIT) {
    Attr->ViewX = pglCurContext->ViewX;
    Attr->ViewY = pglCurContext->ViewY;
    Attr->ViewWidth = pglCurContext->ViewWidth;
    Attr->ViewHeight = pglCurContext->ViewHeight;
    Attr->xScale = pglCurContext->xScale;
    Attr->xOffset = pglCurContext->xOffset;
    Attr->yScale = pglCurContext->yScale;
    Attr->yOffset = pglCurContext->yOffset;
    Attr->zScale = pglCurContext->zScale;
    Attr->zOffset = pglCurContext->zOffset;
    Attr->zNear = pglCurContext->zNear;
    Attr->zFar = pglCurContext->zFar;
  }

  if(mask&GL_TRANSFORM_BIT) {
    Attr->MatMode = pglCurContext->MatMode;
    for(i=0;i<6;i++) for(j=0;j<4;j++) Attr->ClipDef[i][j] = 
      pglCurContext->ClipDef[i][j];
    for(i=0;i<6;i++) for(j=0;j<4;j++) Attr->ClipXform[i][j] = 
      pglCurContext->ClipXform[i][j];
    Attr->Normalize = pglCurContext->Normalize;
    for(i=0;i<6;i++) Attr->ClipEna[i] = pglCurContext->ClipEna[i];
  }

  if(mask&GL_FOG_BIT) {
    for(i=0;i<GR_FOG_TABLE_SIZE;i++) Attr->FogTable[i] = 
      pglCurContext->FogTable[i];
    Attr->FogMode = pglCurContext->FogMode;
    Attr->FogDensity = pglCurContext->FogDensity;
    Attr->FogStart = pglCurContext->FogStart;
    Attr->FogEnd = pglCurContext->FogEnd;
    Attr->FogColor = pglCurContext->FogColor;
    Attr->Fog = pglCurContext->Fog;
  }

  if(mask&GL_LIGHTING_BIT) {
    Attr->ShadeModel = pglCurContext->ShadeModel;
    Attr->ColorMaterialMode = pglCurContext->ColorMaterialMode;
    Attr->ColorMaterialFace = pglCurContext->ColorMaterialFace;
    for(i=0;i<3;i++) Attr->FrontAmbient[i] = pglCurContext->FrontAmbient[i];
    for(i=0;i<3;i++) Attr->FrontDiffuse[i] = pglCurContext->FrontDiffuse[i];
    for(i=0;i<3;i++) Attr->FrontSpecular[i] = pglCurContext->FrontSpecular[i];
    for(i=0;i<3;i++) Attr->FrontEmission[i] = pglCurContext->FrontEmission[i];
    Attr->FrontShininess = pglCurContext->FrontShininess;
    for(i=0;i<3;i++) Attr->BackAmbient[i] = pglCurContext->BackAmbient[i];
    for(i=0;i<3;i++) Attr->BackDiffuse[i] = pglCurContext->BackDiffuse[i];
    for(i=0;i<3;i++) Attr->BackSpecular[i] = pglCurContext->BackSpecular[i];
    for(i=0;i<3;i++) Attr->BackEmission[i] = pglCurContext->BackEmission[i];
    Attr->BackShininess = pglCurContext->BackShininess;
    for(i=0;i<3;i++) Attr->LightModelAmbient[i] = 
      pglCurContext->LightModelAmbient[i];
    Attr->Local_Viewer = pglCurContext->Local_Viewer;
    Attr->Two_Sided = pglCurContext->Two_Sided;
    for(i=0;i<8;i++) Attr->Light[i] = pglCurContext->Light[i];
    Attr->Lighting = pglCurContext->Lighting;
    Attr->ColorMaterial = pglCurContext->ColorMaterial;
    Attr->LightEna[8] = pglCurContext->LightEna[8];
  }

  if(mask&GL_POINT_BIT) {
    Attr->PointWidth = pglCurContext->PointWidth;
    Attr->PointAA = pglCurContext->PointAA;
  }
  
  if(mask&GL_LINE_BIT) {
    Attr->LineStippleFactor = pglCurContext->LineStippleFactor;
    Attr->LinePattern = pglCurContext->LinePattern;
    Attr->LineWidth = pglCurContext->LineWidth;
    Attr->LineAA = pglCurContext->LineAA;
    Attr->LineStipple = pglCurContext->LineStipple;
  }

  if(mask&GL_POLYGON_BIT) {
    Attr->CullMode = pglCurContext->CullMode;
    Attr->FrontFace = pglCurContext->FrontFace;
    Attr->PolygonFrontMode = pglCurContext->PolygonFrontMode;
    Attr->PolygonBackMode = pglCurContext->PolygonBackMode;
    // Polygon Offset
    // Polygon Bias
    // End Polygon
    // Start Polygon/Enable
    Attr->CullEnable = pglCurContext->CullEnable;
    Attr->PolyAA = pglCurContext->PolyAA;
    // Polygon offset point
    // Polygon offset line
    // Polygon offset fill
    Attr->PolyStipple = pglCurContext->PolyStipple;
  }

  if(mask&GL_POLYGON_STIPPLE_BIT) {
    // Polygon stipple value
  }

  if(mask&GL_TEXTURE_BIT) {
    // Start Texture
    Attr->Tex1DName = pglCurContext->Tex1DName;
    Attr->Tex1DPtr = pglCurContext->Tex1DPtr;
    Attr->Tex2DName = pglCurContext->Tex2DName;
    Attr->Tex2DPtr = pglCurContext->Tex2DPtr;
    for(i=0;i<4;i++) Attr->TexEnvColor[4] = pglCurContext->TexEnvColor[4];
    Attr->TexMode = pglCurContext->TexMode;
    // Texture Priority
    // Texture Eye Linear
    // Texture Object Linear
    // Texture Gen Mode
    Attr->Tex1D = pglCurContext->Tex1D;
    Attr->Tex2D = pglCurContext->Tex2D;
    // Texture Gen
  }

  if(mask&GL_SCISSOR_BIT) {
    // Start Scissor
    Attr->minx = pglCurContext->minx;
    Attr->miny = pglCurContext->miny;
    Attr->maxx = pglCurContext->maxx;
    Attr->maxy = pglCurContext->maxy;
    Attr->ScissorEnable = pglCurContext->ScissorEnable;
  }

  if(mask&GL_COLOR_BUFFER_BIT) {
    Attr->AlphaFunc = pglCurContext->AlphaFunc;
    Attr->gAlphaFunc = pglCurContext->gAlphaFunc;
    Attr->AlphaRef = pglCurContext->AlphaRef;
    Attr->SrcBlend = pglCurContext->SrcBlend;
    Attr->gSrcBlend = pglCurContext->gSrcBlend;
    Attr->DstBlend = pglCurContext->DstBlend;
    Attr->gDstBlend = pglCurContext->gDstBlend;
    // Logic OP Mode
    Attr->DrawBuffer = pglCurContext->DrawBuffer;
    Attr->RedMask = pglCurContext->RedMask; 
    Attr->GreenMask = pglCurContext->GreenMask; 
    Attr->BlueMask = pglCurContext->BlueMask; 
    Attr->AlphaMask = pglCurContext->AlphaMask; 
    // Index Mask
    Attr->ColorClrRed = pglCurContext->ColorClrRed;
    Attr->ColorClrGreen = pglCurContext->ColorClrGreen;
    Attr->ColorClrBlue = pglCurContext->ColorClrBlue;
    Attr->ClrColor = pglCurContext->ClrColor;
    Attr->ColorClrAlpha = pglCurContext->ColorClrAlpha;
    Attr->ClrAlpha = pglCurContext->ClrAlpha;
    Attr->ClrIndex = pglCurContext->ClrIndex;
    Attr->AlphaTest = pglCurContext->AlphaTest;
    Attr->Blend = pglCurContext->Blend;
    // Dither
    // Index Logic OP
    // Color Logic OP
  }

  if(mask&GL_STENCIL_BUFFER_BIT) {
    Attr->StencilZPass = pglCurContext->StencilZPass;
    Attr->StencilZFail = pglCurContext->StencilZFail;
    Attr->StencilFail = pglCurContext->StencilFail;
    Attr->StencilFunc = pglCurContext->StencilFunc;
    Attr->StencilFuncMask = pglCurContext->StencilFuncMask;
    Attr->StencilRef = pglCurContext->StencilRef;
    Attr->StencilMask = pglCurContext->StencilMask;
    Attr->ClrStencil = pglCurContext->ClrStencil;
    // Stencil Mask
    Attr->StencilTest = pglCurContext->StencilTest;
  }

  if(mask&GL_DEPTH_BUFFER_BIT) {
    Attr->DepthFunc = pglCurContext->DepthFunc;
    Attr->gDepthFunc = pglCurContext->gDepthFunc;
    Attr->GLClrDepth = pglCurContext->GLClrDepth;
    Attr->ClrDepth = pglCurContext->ClrDepth;
    Attr->DepthMask = pglCurContext->DepthMask; 
    Attr->DepthBuffer = pglCurContext->DepthBuffer;
  }

  if(mask&GL_ACCUM_BUFFER_BIT) {
    Attr->AccumClrRed = pglCurContext->AccumClrRed;
    Attr->AccumClrGreen = pglCurContext->AccumClrGreen;
    Attr->AccumClrBlue = pglCurContext->AccumClrBlue;
    Attr->AccumClrAlpha = pglCurContext->AccumClrAlpha;
  }

  if(mask&GL_LIST_BIT) {
    Attr->ListBase = pglCurContext->ListBase;
  }

  if(mask&GL_EVAL_BIT) {
  }

  if(mask&GL_HINT_BIT) {
  }

  if(mask&GL_PIXEL_MODE_BIT) {
    Attr->PixelMapColor = pglCurContext->PixelMapColor;
    Attr->PixelMapStencil = pglCurContext->PixelMapStencil;
    Attr->PixelIndexShift = pglCurContext->PixelIndexShift;
    Attr->PixelIndexOffset = pglCurContext->PixelIndexOffset;
    Attr->PixelRedScale = pglCurContext->PixelRedScale;
    Attr->PixelRedBias = pglCurContext->PixelRedBias;
    Attr->PixelGreenScale = pglCurContext->PixelGreenScale;
    Attr->PixelGreenBias = pglCurContext->PixelGreenBias;
    Attr->PixelBlueScale = pglCurContext->PixelBlueScale;
    Attr->PixelBlueBias = pglCurContext->PixelBlueBias;
    Attr->PixelAlphaScale = pglCurContext->PixelAlphaScale;
    Attr->PixelAlphaBias = pglCurContext->PixelAlphaBias;
    Attr->PixelDepthScale = pglCurContext->PixelDepthScale;
    Attr->PixelDepthBias = pglCurContext->PixelDepthBias;
    Attr->PixelZoomX = pglCurContext->PixelZoomX;
    Attr->PixelZoomY = pglCurContext->PixelZoomY;
    for(i=0;i<10;i++) for(j=0;j<256;j++) Attr->PixelMaps[i][j] = 
      pglCurContext->PixelMaps[i][j];
    for(i=0;i<10;i++) Attr->PixelMapSize[i] = 
      pglCurContext->PixelMapSize[i]; 
    Attr->ReadBuffer = pglCurContext->ReadBuffer;
  }
  
  if(mask&GL_ENABLE_BIT) {
    Attr->Normalize = pglCurContext->Normalize;
    for(i=0;i<6;i++) Attr->ClipEna[i] = pglCurContext->ClipEna[i];
    Attr->Fog = pglCurContext->Fog;
    Attr->Lighting = pglCurContext->Lighting;
    Attr->ColorMaterial = pglCurContext->ColorMaterial;
    for(i=0;i<8;i++) Attr->LightEna[i] = pglCurContext->LightEna[i];
    Attr->PointAA = pglCurContext->PointAA;
    Attr->LineAA = pglCurContext->LineAA;
    Attr->LineStipple = pglCurContext->LineStipple;
    Attr->CullEnable = pglCurContext->CullEnable;
    Attr->PolyAA = pglCurContext->PolyAA;
    // Polygon offset point
    // Polygon offset line
    // Polygon offset fill
    Attr->PolyStipple = pglCurContext->PolyStipple;
    Attr->Tex1D = pglCurContext->Tex1D;
    Attr->Tex2D = pglCurContext->Tex2D;
    // Texture Gen
    Attr->ScissorEnable = pglCurContext->ScissorEnable;
    Attr->AlphaTest = pglCurContext->AlphaTest;
    Attr->Blend = pglCurContext->Blend;
    // Dither
    // Index Logic OP
    // Color Logic OP
    Attr->StencilTest = pglCurContext->StencilTest;
    Attr->DepthBuffer = pglCurContext->DepthBuffer;
  }
}


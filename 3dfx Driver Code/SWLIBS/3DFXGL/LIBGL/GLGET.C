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
#include <memory.h>
#include <GL/gl.h>
#include "glint.h"

#include <stdio.h>

void APIENTRY glGetBooleanv (GLenum pname, GLboolean *params)
{
  glIntGet(pname, (void *)params, READ_BOOL);
}

void APIENTRY glGetDoublev (GLenum pname, GLdouble *params)
{
  glIntGet(pname, (void *)params, READ_DOUBLE);
}

void APIENTRY glGetFloatv (GLenum pname, GLfloat *params)
{
  glIntGet(pname, (void *)params, READ_FLOAT);
}

void APIENTRY glGetIntegerv (GLenum pname, GLint *params)
{
  glIntGet(pname, (void *)params, READ_INT);
}

GLenum APIENTRY glGetError (void)
{
  GLenum err;

  err = pglCurContext->GLerr;
  pglCurContext->GLerr = GL_NO_ERROR;

  return(err);
}

void APIENTRY glGetPointerv (GLenum pname, GLvoid* *params)
{
}

const GLubyte * APIENTRY glGetString (GLenum name)
{
  const GLubyte *string;

  switch( name ) {
  case GL_VENDOR:
    string = "3Dfx Interactive Inc.";
    break;
  case GL_VERSION:
    string = "1.1";
    break;
  case GL_RENDERER:
    string = "3Dfx Interactive Voodoo(tm)";
    break;
  case GL_EXTENSIONS:
    string = "";
    break;
  default:
    string = "no string";
    break;
  }

  return(string);
}


void glIntGet(GLenum pname, void *params, int type)
{
  int i;

  GLINT_OUTSIDE_BEGIN();

  switch(pname) {
  case GL_ALPHA_TEST_FUNC:
    GLINT_PARAMC(&params,pglCurContext->AlphaFunc);
    break;
  case GL_ALPHA_TEST_REF:
    GLINT_PARAMC(&params,pglCurContext->AlphaRef<<24);
    break;

  case GL_BLEND:
    GLINT_PARAM(&params, pglCurContext->Blend, READ_BOOL);
    break;
  case GL_BLEND_DST:
    GLINT_PARAMC(&params, pglCurContext->DstBlend);
    break;
  case GL_BLEND_SRC:
    GLINT_PARAMC(&params, pglCurContext->SrcBlend);
    break;
    
  case GL_DEPTH_FUNC:
    GLINT_PARAMC(&params, pglCurContext->DepthFunc);
    break;
  case GL_DEPTH_RANGE:
    GLINT_PARAM(&params, pglCurContext->zNear, READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->zFar, READ_FLOAT);
    break;

  case GL_STEREO:
    GLINT_PARAMC(&params, 0);
    break;
  case GL_SUBPIXEL_BITS:
    GLINT_PARAMC(&params, 2);
    break;
  case GL_DOUBLEBUFFER:
    {
      boolean tmp;

      if(curPixelFormat&1) {
	tmp = GL_TRUE;
      } else {
	tmp = GL_FALSE;
      }
      GLINT_PARAM(&params, tmp, READ_BOOL);
    }
    break;
  case GL_DRAW_BUFFER:
    GLINT_PARAMC(&params, pglCurContext->DrawBuffer);
    break;
  case GL_READ_BUFFER:
    GLINT_PARAMC(&params, pglCurContext->ReadBuffer);
    break;
  case GL_SHADE_MODEL:
    GLINT_PARAMC(&params, pglCurContext->ShadeModel);
    break;
  case GL_SCISSOR_BOX:
    GLINT_PARAMC(&params, pglCurContext->minx);
    GLINT_PARAMC(&params, pglCurContext->miny);
    GLINT_PARAMC(&params, pglCurContext->maxx - pglCurContext->minx + 1);
    GLINT_PARAMC(&params, pglCurContext->maxy - pglCurContext->miny + 1);
    break;

  case GL_STENCIL_BITS:
    GLINT_PARAMC(&params, 0);
    break;
  case GL_STENCIL_FAIL:
    GLINT_PARAMC(&params, pglCurContext->StencilFail);
    break;
  case GL_STENCIL_FUNC:
    GLINT_PARAMC(&params, pglCurContext->StencilFunc);
    break;
  case GL_STENCIL_PASS_DEPTH_FAIL:
    GLINT_PARAMC(&params, pglCurContext->StencilZFail);
    break;
  case GL_STENCIL_PASS_DEPTH_PASS:
    GLINT_PARAMC(&params, pglCurContext->StencilZPass);
    break;
  case GL_STENCIL_REF:
    GLINT_PARAMC(&params, pglCurContext->StencilRef);
    break;
  case GL_STENCIL_VALUE_MASK:
    GLINT_PARAMC(&params, pglCurContext->StencilFuncMask);
    break;

  case GL_INDEX_MODE:
    GLINT_PARAMC(&params, 0);
    break;
  case GL_RGBA_MODE:
    GLINT_PARAMC(&params, 1);
    break;
    
  case GL_COLOR_CLEAR_VALUE:
    GLINT_PARAM(&params, pglCurContext->ColorClrRed, READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->ColorClrGreen, READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->ColorClrBlue, READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->ColorClrAlpha, READ_FLOATC);
    break;
  case GL_COLOR_WRITEMASK:
    GLINT_PARAM(&params, pglCurContext->RedMask, READ_BOOL);
    GLINT_PARAM(&params, pglCurContext->GreenMask, READ_BOOL);
    GLINT_PARAM(&params, pglCurContext->BlueMask, READ_BOOL);
    GLINT_PARAM(&params, pglCurContext->AlphaMask, READ_BOOL);
    break;
  case GL_ACCUM_CLEAR_VALUE:
    GLINT_PARAM(&params, pglCurContext->AccumClrRed, READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->AccumClrGreen, READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->AccumClrBlue, READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->AccumClrAlpha, READ_FLOATC);
    break;
  case GL_STENCIL_CLEAR_VALUE:
    GLINT_PARAMC(&params, pglCurContext->ClrStencil);
    break;
  case GL_STENCIL_WRITEMASK:
    GLINT_PARAMC(&params, pglCurContext->StencilMask);
    break;
  case GL_DEPTH_CLEAR_VALUE:
    GLINT_PARAM(&params, pglCurContext->GLClrDepth, READ_SHORT);
    break;
  case GL_DEPTH_WRITEMASK:
    GLINT_PARAM(&params, pglCurContext->DepthMask, READ_BOOL);
    break;

  case GL_COLOR_MATERIAL:
    GLINT_PARAM(&params, pglCurContext->ColorMaterial, READ_BOOL);
    break;
  case GL_COLOR_MATERIAL_FACE:
    GLINT_PARAMC(&params, pglCurContext->ColorMaterialFace);
    break;
  case GL_COLOR_MATERIAL_PARAMETER:
    GLINT_PARAMC(&params, pglCurContext->ColorMaterialMode);
    break;

  case GL_ALPHA_BITS:
    GLINT_PARAMC(&params, 0);
    break;
  case GL_BLUE_BITS:
    GLINT_PARAMC(&params, 5);
    break;
  case GL_GREEN_BITS:
    GLINT_PARAMC(&params, 6);
    break;
  case GL_RED_BITS:
    GLINT_PARAMC(&params, 5);
    break;
  case GL_INDEX_BITS:
    GLINT_PARAMC(&params, 0);
    break;

  case GL_DEPTH_BITS:
    GLINT_PARAMC(&params, 16);
    break;

  case GL_MAX_TEXTURE_SIZE:
    GLINT_PARAMC(&params, 256);
    break;
  case GL_TEXTURE_ENV_MODE:
    GLINT_PARAMC(&params, pglCurContext->TexMode);
    break;
  case GL_TEXTURE_ENV_COLOR:
    GLINT_PARAM(&params, pglCurContext->TexEnvColor[0], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->TexEnvColor[1], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->TexEnvColor[2], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->TexEnvColor[3], READ_FLOATC);
    break;

  case GL_FOG_COLOR:
    {
      float tmp;

      tmp = ((float)((pglCurContext->FogColor>>16)&0xff))/255.0f;
      GLINT_PARAM(&params,tmp,READ_FLOATC);
      tmp = ((float)((pglCurContext->FogColor>>8)&0xff))/255.0f;
      GLINT_PARAM(&params,tmp,READ_FLOATC);
      tmp = ((float)(pglCurContext->FogColor&0xff))/255.0f;
      GLINT_PARAM(&params,tmp,READ_FLOATC);
      tmp = ((float)((pglCurContext->FogColor>>24)&0xff))/255.0f;
      GLINT_PARAM(&params,tmp,READ_FLOATC);
    }
    break;
  case GL_FOG_DENSITY:
    GLINT_PARAM(&params, pglCurContext->FogDensity, READ_FLOAT);
    break;
  case GL_FOG_END:
    GLINT_PARAM(&params, pglCurContext->FogEnd, READ_FLOAT);
    break;
  case GL_FOG_MODE:
    GLINT_PARAMC(&params, pglCurContext->FogMode);
    break;
  case GL_FOG_START:
    GLINT_PARAM(&params, pglCurContext->FogStart, READ_FLOAT);
    break;


  case GL_CULL_FACE:
    GLINT_PARAM(&params, pglCurContext->CullEnable, READ_BOOL);
    break;
  case GL_CULL_FACE_MODE:
    GLINT_PARAMC(&params, pglCurContext->CullMode);
    break;
  case GL_POLYGON_MODE:
    GLINT_PARAMC(&params, pglCurContext->PolygonFrontMode);
    GLINT_PARAMC(&params, pglCurContext->PolygonBackMode);
    break;
  case GL_FRONT_FACE:
    GLINT_PARAMC(&params, pglCurContext->FrontFace);
    break;
  case GL_RENDER_MODE:
    GLINT_PARAMC(&params, pglCurContext->FrontFace);
    break;
    
  case GL_CURRENT_COLOR:
    GLINT_PARAM(&params, pglCurContext->CurColor[0], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->CurColor[1], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->CurColor[2], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->CurColor[3], READ_FLOATC);
    break;
  case GL_CURRENT_NORMAL:
    GLINT_PARAM(&params, pglCurContext->CurNormal[0], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->CurNormal[1], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->CurNormal[2], READ_FLOAT);
    break;
  case GL_CURRENT_TEXTURE_COORDS:
    GLINT_PARAM(&params, pglCurContext->CurTex[0], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->CurTex[1], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->CurTex[2], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->CurTex[3], READ_FLOAT);
    break;
  case GL_EDGE_FLAG:
    GLINT_PARAMC(&params, pglCurContext->CurEdge);
    break;

  case GL_CURRENT_RASTER_COLOR:
    GLINT_PARAM(&params, pglCurContext->RasVtx.Color[0], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->RasVtx.Color[1], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->RasVtx.Color[2], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->RasVtx.Color[4], READ_FLOATC);
    break;
  case GL_CURRENT_RASTER_POSITION:
    GLINT_PARAM(&params, pglCurContext->RasVtx.Glide.x, READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->RasVtx.Glide.y, READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->RasVtx.Glide.z, READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->RasVtx.Clip[3], READ_FLOAT);
    break;
  case GL_CURRENT_RASTER_TEXTURE_COORDS:
    GLINT_PARAM(&params, pglCurContext->RasVtx.Texture[0], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->RasVtx.Texture[1], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->RasVtx.Texture[2], READ_FLOAT);
    GLINT_PARAM(&params, pglCurContext->RasVtx.Texture[3], READ_FLOAT);
    break;
  case GL_CURRENT_RASTER_POSITION_VALID:
    GLINT_PARAM(&params, pglCurContext->RasValid, READ_BOOL);
    break;

  case GL_MATRIX_MODE:
    GLINT_PARAMC(&params, pglCurContext->MatMode);
    break;
  case GL_MODELVIEW_MATRIX:
    for(i=0;i<16;i++) {
      GLINT_PARAM(&params, pglCurContext->CurModelView[i], READ_FLOAT);
    }
    break;
  case GL_PROJECTION_MATRIX:
    for(i=0;i<16;i++) {
      GLINT_PARAM(&params, pglCurContext->CurProjMat[i], READ_FLOAT);
    }
    break;
  case GL_TEXTURE_MATRIX:
    for(i=0;i<16;i++) {
      GLINT_PARAM(&params, pglCurContext->CurTexMat[i], READ_FLOAT);
    }
    break;
  case GL_MAX_CLIP_PLANES:
    GLINT_PARAMC(&params, 6);
    break;
  case GL_MAX_MODELVIEW_STACK_DEPTH:
    GLINT_PARAMC(&params, 32);
    break;
  case GL_MODELVIEW_STACK_DEPTH:
    GLINT_PARAMC(&params, pglCurContext->ModelViewPos);
    break;
  case GL_MAX_PROJECTION_STACK_DEPTH:
    GLINT_PARAMC(&params, 2);
    break;
  case GL_PROJECTION_STACK_DEPTH:
    GLINT_PARAMC(&params, pglCurContext->ProjMatPos);
    break;
  case GL_MAX_TEXTURE_STACK_DEPTH:
    GLINT_PARAMC(&params, 2);
    break;
  case GL_TEXTURE_STACK_DEPTH:
    GLINT_PARAMC(&params, pglCurContext->TexMatPos);
    break;
  case GL_MAX_VIEWPORT_DIMS:
    GLINT_PARAMC(&params, 640);
    GLINT_PARAMC(&params, 480);
    break;
  case GL_VIEWPORT:
    GLINT_PARAMC(&params, pglCurContext->ViewX);
    GLINT_PARAMC(&params, pglCurContext->ViewY);
    GLINT_PARAMC(&params, pglCurContext->ViewWidth);
    GLINT_PARAMC(&params, pglCurContext->ViewHeight);
    break;

  case GL_MAX_LIGHTS:
    GLINT_PARAMC(&params, 8);
    break;
  case GL_LIGHT_MODEL_AMBIENT:
    GLINT_PARAM(&params, pglCurContext->LightModelAmbient[0], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->LightModelAmbient[1], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->LightModelAmbient[2], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->LightModelAmbient[3], READ_FLOATC);
    break;

  case GL_LINE_STIPPLE_PATTERN:
    GLINT_PARAM(&params, pglCurContext->LinePattern, READ_SHORT);
    break;
  case GL_LINE_STIPPLE_REPEAT:
    GLINT_PARAMC(&params, pglCurContext->LineStippleFactor);
    break;
  case GL_LINE_WIDTH:
    GLINT_PARAM(&params, pglCurContext->LineWidth, READ_FLOAT);
    break;
  case GL_LINE_WIDTH_GRANULARITY:
    GLINT_PARAMC(&params, 1);
    break;
  case GL_LINE_WIDTH_RANGE:
    GLINT_PARAMC(&params, 1);
    GLINT_PARAMC(&params, 1);
    break;

  case GL_POINT_SIZE:
    GLINT_PARAM(&params, pglCurContext->PointWidth, READ_FLOAT);
    break;
  case GL_POINT_SIZE_GRANULARITY:
    GLINT_PARAMC(&params, 1);
    break;
  case GL_POINT_SIZE_RANGE:
    GLINT_PARAMC(&params, 1);
    GLINT_PARAMC(&params, 1);
    break;

  case GL_MAX_NAME_STACK_DEPTH:
    GLINT_PARAMC(&params, 64);
    break;
  case GL_NAME_STACK_DEPTH:
    GLINT_PARAMC(&params, pglCurContext->SelectDepth+1);
    break;

  case GL_MAX_LIST_NESTING:
    GLINT_PARAMC(&params, 64);
    break;
  case GL_LIST_BASE:
    GLINT_PARAMC(&params, pglCurContext->ListBase);
    break;
  case GL_LIST_INDEX:
    if(pglCurContext->Listing)
      GLINT_PARAMC(&params, pglCurContext->BuildName);
    else
      GLINT_PARAMC(&params, 0);
    break;
  case GL_LIST_MODE:
    GLINT_PARAMC(&params, pglCurContext->Execute);
    break;

  case GL_LOGIC_OP:
    GLINT_PARAMC(&params, 0);  // UNIMP
    break;

  case GL_AUTO_NORMAL:
    GLINT_PARAM(&params, pglCurContext->AutoNormal, READ_BOOL);
    break;
  case GL_MAX_EVAL_ORDER:
    GLINT_PARAMC(&params, 0);  // UNIMP
    break;
  case GL_MAP1_GRID_DOMAIN:
    GLINT_PARAMC(&params, 0);  // UNIMP
    break;
  case GL_MAP1_GRID_SEGMENTS:
    GLINT_PARAMC(&params, 0);  // UNIMP
    break;
  case GL_MAP2_GRID_DOMAIN:
    GLINT_PARAMC(&params, 0);  // UNIMP
    break;
  case GL_MAP2_GRID_SEGMENTS:
    GLINT_PARAMC(&params, 0);  // UNIMP
    break;


  case GL_ACCUM_ALPHA_BITS:
    GLINT_PARAMC(&params,0);  // UNIMP
    break;
  case GL_ACCUM_RED_BITS:
    GLINT_PARAMC(&params,0);  // UNIMP
    break;
  case GL_ACCUM_GREEN_BITS:
    GLINT_PARAMC(&params,0);  // UNIMP
    break;
  case GL_ACCUM_BLUE_BITS:
    GLINT_PARAMC(&params,0);  // UNIMP
    break;

  case GL_AUX_BUFFERS:
    GLINT_PARAMC(&params, 0);
    break;

  case GL_ATTRIB_STACK_DEPTH:
    GLINT_PARAMC(&params, pglCurContext->AttribDepth);
    break;
  case GL_MAX_ATTRIB_STACK_DEPTH:
    GLINT_PARAMC(&params, 16);
    break;

  case GL_MAP_COLOR:
    GLINT_PARAMC(&params, pglCurContext->PixelMapColor);
    break;
  case GL_MAP_STENCIL:
    GLINT_PARAMC(&params, pglCurContext->PixelMapStencil);
    break;

  case GL_ALPHA_SCALE:
    GLINT_PARAM(&params, pglCurContext->PixelAlphaScale, READ_FLOATC);
    break;
  case GL_ALPHA_BIAS:
    GLINT_PARAM(&params, pglCurContext->PixelAlphaBias, READ_FLOATC);
    break;
  case GL_BLUE_SCALE:
    GLINT_PARAM(&params, pglCurContext->PixelBlueScale, READ_FLOATC);
    break;
  case GL_BLUE_BIAS:
    GLINT_PARAM(&params, pglCurContext->PixelBlueBias, READ_FLOATC);
    break;
  case GL_GREEN_SCALE:
    GLINT_PARAM(&params, pglCurContext->PixelGreenScale, READ_FLOATC);
    break;
  case GL_GREEN_BIAS:
    GLINT_PARAM(&params, pglCurContext->PixelGreenBias, READ_FLOATC);
    break;
  case GL_RED_SCALE:
    GLINT_PARAM(&params, pglCurContext->PixelRedScale, READ_FLOATC);
    break;
  case GL_RED_BIAS:
    GLINT_PARAM(&params, pglCurContext->PixelRedBias, READ_FLOATC);
    break;

  case GL_MAX_PIXEL_MAP_TABLE:
    GLINT_PARAMC(&params, 256);
    break;

  case GL_PACK_SWAP_BYTES:
    GLINT_PARAMC(&params, pglCurContext->PackSwapBytes);
    break;
  case GL_PACK_LSB_FIRST:
    GLINT_PARAMC(&params, pglCurContext->PackLsbFirst);
    break;
  case GL_PACK_ROW_LENGTH:
    GLINT_PARAMC(&params, pglCurContext->PackRowLength);
    break;
  case GL_PACK_SKIP_ROWS:
    GLINT_PARAMC(&params, pglCurContext->PackSkipRows);
    break;
  case GL_PACK_SKIP_PIXELS:
    GLINT_PARAMC(&params, pglCurContext->PackSkipPixels);
    break;
  case GL_PACK_ALIGNMENT:
    GLINT_PARAMC(&params, pglCurContext->PackAlignment);
    break;
  case GL_UNPACK_SWAP_BYTES:
    GLINT_PARAMC(&params, pglCurContext->UnPackSwapBytes);
    break;
  case GL_UNPACK_LSB_FIRST:
    GLINT_PARAMC(&params, pglCurContext->UnPackLsbFirst);
    break;
  case GL_UNPACK_ROW_LENGTH:
    GLINT_PARAMC(&params, pglCurContext->UnPackRowLength);
    break;
  case GL_UNPACK_SKIP_ROWS:
    GLINT_PARAMC(&params, pglCurContext->UnPackSkipRows);
    break;
  case GL_UNPACK_SKIP_PIXELS:
    GLINT_PARAMC(&params, pglCurContext->UnPackSkipPixels);
    break;
  case GL_UNPACK_ALIGNMENT:
    GLINT_PARAMC(&params, pglCurContext->UnPackAlignment);
    break;

  case GL_PIXEL_MAP_R_TO_R_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[6]);
    break;
  case GL_PIXEL_MAP_G_TO_G_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[7]);
    break;
  case GL_PIXEL_MAP_B_TO_B_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[8]);
    break;
  case GL_PIXEL_MAP_A_TO_A_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[9]);
    break;
  case GL_PIXEL_MAP_I_TO_R_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[2]);
    break;
  case GL_PIXEL_MAP_I_TO_G_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[3]);
    break;
  case GL_PIXEL_MAP_I_TO_B_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[4]);
    break;
  case GL_PIXEL_MAP_I_TO_A_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[5]);
    break;
  case GL_PIXEL_MAP_I_TO_I_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[0]);
    break;
  case GL_PIXEL_MAP_S_TO_S_SIZE:
    GLINT_PARAMC(&params, pglCurContext->PixelMapSize[1]);
    break;

  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
}

void glIntRead(void **params, void *data, int rslt_type, int src_type)
{
  switch(rslt_type) {
  case READ_DOUBLE:
    switch(src_type) {
    case READ_FLOAT:
      *((double *)(*params))++ = (double)*((float *)data);
      break;
    case READ_FLOATC:
      *((double *)(*params))++ = (double)*((float *)data);
      break;
    case READ_INT:
      *((double *)(*params))++ = (double)*((int *)data);
      break;
    case READ_SHORT:
      *((double *)(*params))++ = (double)*((unsigned short *)data);
      break;
    case READ_BYTE:
      *((double *)(*params))++ = (double)*((unsigned char *)data);
      break;
    case READ_BOOL:
      *((double *)(*params))++ = (double)*((boolean *)data);
      break;
    }
    break;
  case READ_FLOAT:
    switch(src_type) {
    case READ_FLOAT:
      *((float *)(*params))++ = *((float *)data);
      break;
    case READ_FLOATC:
      *((float *)(*params))++ = *((float *)data);
      break;
    case READ_INT:
      *((float *)(*params))++ = (float)*((int *)data);
      break;
    case READ_SHORT:
      *((float *)(*params))++ = (float)*((unsigned short *)data);
      break;
    case READ_BYTE:
      *((float *)(*params))++ = (float)*((unsigned char *)data);
      break;
    case READ_BOOL:
      *((float *)(*params))++ = (float)*((boolean *)data);
      break;
    }
    break;
  case READ_INT:
    switch(src_type) {
    case READ_FLOAT:
      *((int *)(*params))++ = (int)*((float *)data);
      break;
    case READ_FLOATC:
      *((int *)(*params))++ = (int)(*((float *)data)*
				    (66535.0f*32768.0f));
      break;
    case READ_INT:
      *((int *)(*params))++ = *((int *)data);
      break;
    case READ_SHORT:
      *((int *)(*params))++ = (int)*((unsigned short *)data);
      break;
    case READ_BYTE:
      *((int *)(*params))++ = (int)*((unsigned char *)data);
      break;
    case READ_BOOL:
      *((int *)(*params))++ = (int)*((boolean *)data);
      break;
    }
    break;
  case READ_BOOL:
    switch(src_type) {
    case READ_FLOAT:
      if(*((float *)data)==0.0f)
	*((boolean *)(*params))++ = GL_FALSE;
      else
	*((boolean *)(*params))++ = GL_TRUE;
      break;
    case READ_FLOATC:
      if(*((float *)data)==0.0f)
	*((boolean *)(*params))++ = GL_FALSE;
      else
	*((boolean *)(*params))++ = GL_TRUE;
      break;
    case READ_INT:
      if(*((int *)data) == 0)
	*((boolean *)(*params))++ = GL_FALSE;
      else
	*((boolean *)(*params))++ = GL_TRUE;
      break;
    case READ_SHORT:
      if(*((unsigned short *)data) == 0)
	*((boolean *)(*params))++ = GL_FALSE;
      else
	*((boolean *)(*params))++ = GL_TRUE;
      break;
    case READ_BYTE:
      if(*((unsigned char *)data) == 0)
	*((boolean *)(*params))++ = GL_FALSE;
      else
	*((boolean *)(*params))++ = GL_TRUE;
      break;
    case READ_BOOL:
      *((boolean *)(*params))++ = *((boolean *)data);
      break;
    }
    break;
  }
}


void glIntReadC(void **params, int data, int rslt_type)
{
  switch(rslt_type) {
  case READ_DOUBLE:
    *((double *)(*params))++ = (double)data;
    break;
  case READ_FLOAT:
    *((float *)(*params))++ = (float)data;
    break;
  case READ_FLOATC:
    *((int *)(*params))++ = (int)(*((float *)data)*
				  (66535.0f*32768.0f));
    break;
  case READ_INT:
      *((int *)(*params))++ = data;
    break;
  case READ_BOOL:
    if(data==0)
      *((boolean *)(*params))++ = GL_FALSE;
    else
      *((boolean *)(*params))++ = GL_TRUE;
    break;
  }
}


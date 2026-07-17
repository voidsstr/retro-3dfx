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

void APIENTRY glFogf (GLenum pname, GLfloat param)
{
  GLINT_OUTSIDE_BEGIN();

  switch(pname) {
  case GL_FOG_DENSITY:
    break;
  case GL_FOG_START:
    break;
  case GL_FOG_END:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_FOG_DENSITY:
      glIntIntToList(OP_FOGDENISTY);
      glIntFloatToList(param);
      break;
    case GL_FOG_START:
      glIntIntToList(OP_FOGSTART);
      glIntFloatToList(param);
      break;
    case GL_FOG_END:
      glIntIntToList(OP_FOGEND);
      glIntFloatToList(param);
      break;
    default:
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }
  
  switch(pname) {
  case GL_FOG_DENSITY:
    pglCurContext->FogDensity = param;
    break;
  case GL_FOG_START:
    pglCurContext->FogStart = param;
    break;
  case GL_FOG_END:
    pglCurContext->FogEnd = param;
    break;
  default:
    break;
  }

  glIntInitFog(pglCurContext->FogTable,
	       pglCurContext->FogMode,
	       pglCurContext->FogDensity,
	       pglCurContext->FogStart,
	       pglCurContext->FogEnd);

  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glFogfv (GLenum pname, const GLfloat *params)
{
  GLINT_OUTSIDE_BEGIN();

  switch(pname) {
  case GL_FOG_DENSITY:
    break;
  case GL_FOG_START:
    break;
  case GL_FOG_END:
    break;
  case GL_FOG_COLOR:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_FOG_DENSITY:
      glIntIntToList(OP_FOGDENISTY);
      glIntFloatToList(params[0]);
      break;
    case GL_FOG_START:
      glIntIntToList(OP_FOGSTART);
      glIntFloatToList(params[0]);
      break;
    case GL_FOG_END:
      glIntIntToList(OP_FOGEND);
      glIntFloatToList(params[0]);
      break;
    case GL_FOG_COLOR:
      {
	GrColor_t FogColor;
	FogColor = ((GrColor_t)(params[0]*255.0f))&0xff;
	FogColor |= ((((GrColor_t)(params[1]*255.0f))&0xff)<<8);
	FogColor |= ((((GrColor_t)(params[2]*255.0f))&0xff)<<16);
	FogColor |= ((((GrColor_t)(params[3]*255.0f))&0xff)<<24);
	glIntIntToList(OP_FOGCOLOR);
	glIntIntToList(FogColor);
      }
      break;
    default:
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_FOG_DENSITY:
    pglCurContext->FogDensity = params[0];
    break;
  case GL_FOG_START:
    pglCurContext->FogStart = params[0];
    break;
  case GL_FOG_END:
    pglCurContext->FogEnd = params[0];
    break;
  case GL_FOG_COLOR:
    pglCurContext->FogColor = ((GrColor_t)(params[0]*255.0f))&0xff;
    pglCurContext->FogColor |= ((((GrColor_t)(params[1]*255.0f))&0xff)<<8);
    pglCurContext->FogColor |= ((((GrColor_t)(params[2]*255.0f))&0xff)<<16);
    pglCurContext->FogColor |= ((((GrColor_t)(params[3]*255.0f))&0xff)<<24);
    break;
  default:
    break;
  }

  glIntInitFog(pglCurContext->FogTable,
	       pglCurContext->FogMode,
	       pglCurContext->FogDensity,
	       pglCurContext->FogStart,
	       pglCurContext->FogEnd);

  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glFogi (GLenum pname, GLint param)
{
  GLINT_OUTSIDE_BEGIN();

  switch(pname) {
  case GL_FOG_MODE:
    switch(param) {
    case GL_LINEAR:
      break;
    case GL_EXP:
      break;
    case GL_EXP2:
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    break;
  case GL_FOG_DENSITY:
    break;
  case GL_FOG_START:
    break;
  case GL_FOG_END:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_FOG_MODE:
      glIntIntToList(OP_FOGMODE);
      glIntIntToList(param);
      break;
    case GL_FOG_DENSITY:
      glIntIntToList(OP_FOGDENISTY);
      glIntFloatToList(((float)(param>>24))/255.0f);
      break;
    case GL_FOG_START:
      glIntIntToList(OP_FOGSTART);
      glIntFloatToList(((float)(param>>24))/255.0f);
      break;
    case GL_FOG_END:
      glIntIntToList(OP_FOGEND);
      glIntFloatToList(((float)(param>>24))/255.0f);
      break;
    default:
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_FOG_MODE:
    switch(param) {
    case GL_LINEAR:
      pglCurContext->FogMode = param;
     break;
    case GL_EXP:
      pglCurContext->FogMode = param;
      break;
    case GL_EXP2:
      pglCurContext->FogMode = param;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_FOG_DENSITY:
    pglCurContext->FogDensity = ((float)(param>>24))/255.0f;
    break;
  case GL_FOG_START:
    pglCurContext->FogStart = ((float)(param>>24))/255.0f;
    break;
  case GL_FOG_END:
    pglCurContext->FogEnd = ((float)(param>>24))/255.0f;
    break;
  default:
    break;
  }

  glIntInitFog(pglCurContext->FogTable,
	       pglCurContext->FogMode,
	       pglCurContext->FogDensity,
	       pglCurContext->FogStart,
	       pglCurContext->FogEnd);

  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glFogiv (GLenum pname, const GLint *params)
{
  GLINT_OUTSIDE_BEGIN();

  switch(pname) {
  case GL_FOG_MODE:
    switch(params[0]) {
    case GL_LINEAR:
      break;
    case GL_EXP:
      break;
    case GL_EXP2:
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    break;
  case GL_FOG_DENSITY:
    break;
  case GL_FOG_START:
    break;
  case GL_FOG_END:
    break;
  case GL_FOG_COLOR:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_FOG_MODE:
      glIntIntToList(OP_FOGMODE);
      glIntIntToList(params[0]);
      break;
    case GL_FOG_DENSITY:
      glIntIntToList(OP_FOGDENISTY);
      glIntFloatToList(((float)(params[0]>>24))/255.0f);
      break;
    case GL_FOG_START:
      glIntIntToList(OP_FOGSTART);
      glIntFloatToList(((float)(params[0]>>24))/255.0f);
      break;
    case GL_FOG_END:
      glIntIntToList(OP_FOGEND);
      glIntFloatToList(((float)(params[0]>>24))/255.0f);
      break;
    case GL_FOG_COLOR:
      {
	GrColor_t FogColor;
	FogColor = (params[0]>>24)&0x0000000ff;
	FogColor |= (params[1]>>16)&0x0000ff00;
	FogColor |= (params[2]>>8)&0x00ff0000;
	FogColor |= params[3]&0xff000000;
	glIntIntToList(OP_FOGCOLOR);
	glIntIntToList(FogColor);
      }
      break;
    default:
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_FOG_MODE:
    switch(params[0]) {
    case GL_LINEAR:
      pglCurContext->FogMode = params[0];
      break;
    case GL_EXP:
      pglCurContext->FogMode = params[0];
      break;
    case GL_EXP2:
      pglCurContext->FogMode = params[0];
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_FOG_DENSITY:
    pglCurContext->FogDensity = ((float)(params[0]>>24))/255.0f;
    break;
  case GL_FOG_START:
    pglCurContext->FogStart = ((float)(params[0]>>24))/255.0f;
    break;
  case GL_FOG_END:
    pglCurContext->FogEnd = ((float)(params[0]>>24))/255.0f;
    break;
  case GL_FOG_COLOR:
    pglCurContext->FogColor = (params[0]>>24)&0x0000000ff;
    pglCurContext->FogColor |= (params[1]>>16)&0x0000ff00;
    pglCurContext->FogColor |= (params[2]>>8)&0x00ff0000;
    pglCurContext->FogColor |= params[3]&0xff000000;
    break;
  default:
    break;
  }

  glIntInitFog(pglCurContext->FogTable,
	       pglCurContext->FogMode,
	       pglCurContext->FogDensity,
	       pglCurContext->FogStart,
	       pglCurContext->FogEnd);

  pglCurContext->ContextDirty = TRUE;
}

void glIntInitFog(GrFog_t *table, GLenum mode, 
		  GLfloat density, GLfloat start, GLfloat end)
{
  switch(mode) {
  case GL_LINEAR:
    guFogGenerateLinear(table, start, end);
    break;
  case GL_EXP:
    guFogGenerateExp(table,density );
    break;
  case GL_EXP2:
    guFogGenerateExp2(table,density );
    break;
  }
}










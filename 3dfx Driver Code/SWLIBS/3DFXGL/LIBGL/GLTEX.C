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

void APIENTRY glTexCoord1d (GLdouble s)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList(0.0f);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = 0.0f;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord1dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList(0.0f);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = 0.0f;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord1f (GLfloat s)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList(0.0f);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = s;
  pglCurContext->CurTex[1] = 0.0f;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord1fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList(0.0f);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = v[0];
  pglCurContext->CurTex[1] = 0.0f;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord1i (GLint s)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList(0.0f);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = 0.0f;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord1iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList(0.0f);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = 0.0f;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord1s (GLshort s)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList(0.0f);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = 0.0f;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord1sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList(0.0f);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = 0.0f;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord2d (GLdouble s, GLdouble t)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = (float)t;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord2dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = (float)v[1];
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord2f (GLfloat s, GLfloat t)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = s;
  pglCurContext->CurTex[1] = t;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord2fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = v[0];
  pglCurContext->CurTex[1] = v[1];
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord2i (GLint s, GLint t)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = (float)t;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord2iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = (float)v[1];
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord2s (GLshort s, GLshort t)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = (float)t;
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord2sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = (float)v[1];
  pglCurContext->CurTex[2] = 0.0f;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord3d (GLdouble s, GLdouble t, GLdouble r)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList((float)r);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = (float)t;
  pglCurContext->CurTex[2] = (float)r;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord3dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = (float)v[1];
  pglCurContext->CurTex[2] = (float)v[2];
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord3f (GLfloat s, GLfloat t, GLfloat r)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList((float)r);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = s;
  pglCurContext->CurTex[1] = t;
  pglCurContext->CurTex[2] = r;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord3fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = v[0];
  pglCurContext->CurTex[1] = v[1];
  pglCurContext->CurTex[2] = v[2];
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord3i (GLint s, GLint t, GLint r)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList((float)r);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = (float)t;
  pglCurContext->CurTex[2] = (float)r;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord3iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = (float)v[1];
  pglCurContext->CurTex[2] = (float)v[2];
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord3s (GLshort s, GLshort t, GLshort r)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList((float)r);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = (float)t;
  pglCurContext->CurTex[2] = (float)r;
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord3sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = (float)v[1];
  pglCurContext->CurTex[2] = (float)v[2];
  pglCurContext->CurTex[3] = 1.0f;
}

void APIENTRY glTexCoord4d (GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList((float)r);
    glIntFloatToList((float)q);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = (float)t;
  pglCurContext->CurTex[2] = (float)r;
  pglCurContext->CurTex[3] = (float)q;
}

void APIENTRY glTexCoord4dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = (float)v[1];
  pglCurContext->CurTex[2] = (float)v[2];
  pglCurContext->CurTex[3] = (float)v[3];
}

void APIENTRY glTexCoord4f (GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList((float)r);
    glIntFloatToList((float)q);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = s;
  pglCurContext->CurTex[1] = t;
  pglCurContext->CurTex[2] = r;
  pglCurContext->CurTex[3] = q;
}

void APIENTRY glTexCoord4fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = v[0];
  pglCurContext->CurTex[1] = v[1];
  pglCurContext->CurTex[2] = v[2];
  pglCurContext->CurTex[3] = v[3];
}

void APIENTRY glTexCoord4i (GLint s, GLint t, GLint r, GLint q)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList((float)r);
    glIntFloatToList((float)q);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = (float)t;
  pglCurContext->CurTex[2] = (float)r;
  pglCurContext->CurTex[3] = (float)q;
}

void APIENTRY glTexCoord4iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = (float)v[1];
  pglCurContext->CurTex[2] = (float)v[2];
  pglCurContext->CurTex[3] = (float)v[3];
}

void APIENTRY glTexCoord4s (GLshort s, GLshort t, GLshort r, GLshort q)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)s);
    glIntFloatToList((float)t);
    glIntFloatToList((float)r);
    glIntFloatToList((float)q);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)s;
  pglCurContext->CurTex[1] = (float)t;
  pglCurContext->CurTex[2] = (float)r;
  pglCurContext->CurTex[3] = (float)q;
}

void APIENTRY glTexCoord4sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_TEXTURE_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurTex[0] = (float)v[0];
  pglCurContext->CurTex[1] = (float)v[1];
  pglCurContext->CurTex[2] = (float)v[2];
  pglCurContext->CurTex[3] = (float)v[3];
}

GLboolean APIENTRY glAreTexturesResident (GLsizei n, const GLuint *textures, GLboolean *residences)
{
  GLINT_OUTSIDE_BEGIN();

  return((GLboolean)FALSE);
}

void APIENTRY glBindTexture (GLenum target, GLuint texture)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    
    switch(target) {
    case GL_TEXTURE_1D:
      glIntIntToList(OP_BIND_1D);
      glIntIntToList(texture);
      break;
    case GL_TEXTURE_2D:
      glIntIntToList(OP_BIND_2D);
      glIntIntToList(texture);
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(target) {
  case GL_TEXTURE_1D:
  case GL_TEXTURE_2D:
    glIntBindTexture(target,texture);
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
  pglCurContext->TexDirty = TRUE;
}

void APIENTRY glDeleteTextures (GLsizei n, const GLuint *textures)
{
  pglTexBlock CurTex, LastTex;
  
  GLINT_OUTSIDE_BEGIN();

  while(n--) {
    if(*textures != 0) {
      if(*textures < NUM_FAST_TEX) {
	// texture found - delete
	if(pglCurContext->TexFastList[*textures] != NULL) {
	  // If bound - bind to default
	  switch(pglCurContext->TexFastList[*textures]->Type) {
	  case GL_TEXTURE_1D:
	    if(pglCurContext->TexFastList[*textures] ==
	       pglCurContext->Tex1DPtr)
	      pglCurContext->Tex1DPtr = pglCurContext->Tex1DDefPtr;
	    pglCurContext->TexDirty = TRUE;
	    break;
	  case GL_TEXTURE_2D:
	    if(pglCurContext->TexFastList[*textures] ==
	       pglCurContext->Tex2DPtr)
	      pglCurContext->Tex2DPtr = pglCurContext->Tex2DDefPtr;
	    pglCurContext->TexDirty = TRUE;
	    break;
          default:
	    GLINT_ERROR(GL_INVALID_ENUM);
	    break;
	  }
	    
	  // Free texture data
	  if(pglCurContext->TexFastList[*textures]->GlideTex.data != NULL)
	    free(pglCurContext->TexFastList[*textures]->GlideTex.data);
	    
	  // Free Texture block
	  free(pglCurContext->TexFastList[*textures]);
	  pglCurContext->TexFastList[*textures] = NULL;
	}
      } else {
	CurTex = pglCurContext->TexHead;
	LastTex = NULL;
	while(CurTex != NULL) {
	  if(CurTex->TexName == *textures)
	    break;
	  LastTex = CurTex;
	  CurTex = CurTex->Next;
	}
	
	// texture found - delete
	if(CurTex != NULL) {
	  // If bound - bind to default
	  switch(CurTex->Texture->Type) {
	  case GL_TEXTURE_1D:
	    if(CurTex->Texture == pglCurContext->Tex1DPtr)
	      pglCurContext->Tex1DPtr = pglCurContext->Tex1DDefPtr;
	    pglCurContext->TexDirty = TRUE;
	    break;
	  case GL_TEXTURE_2D:
	    if(CurTex->Texture == pglCurContext->Tex2DPtr)
	      pglCurContext->Tex2DPtr = pglCurContext->Tex2DDefPtr;
	    pglCurContext->TexDirty = TRUE;
	    break;
          default:
	    GLINT_ERROR(GL_INVALID_ENUM);
	    break;
	  }
	    
	  // Remove from list
	  if(LastTex == NULL) {
	    // Remove from head
	    pglCurContext->TexHead = CurTex->Next;
	  } else {
	    // Extract from list
	    LastTex->Next = CurTex->Next;
	  }
	    
	  // Free texture data
	  if(CurTex->Texture->GlideTex.data != NULL)
	    free(CurTex->Texture->GlideTex.data);
	    
	  // Free Texture block
	  free(CurTex->Texture);

	  // Free Header block
	  free(CurTex);
	}
      }
    }
      
    textures++;
  }
}

void APIENTRY glGenTextures (GLsizei n, GLuint *textures)
{
  GLuint texture;

  GLINT_OUTSIDE_BEGIN();

  texture = 1;
  while(n > 0) {
    if(!glIsTexture(texture)) {
      *textures++ = texture;
      n--;
    }
    texture++;
  }
}

GLboolean APIENTRY glIsTexture (GLuint texture)
{
  pglTexBlock CurTex, LastTex;
  
  GLINT_OUTSIDE_BEGIN();

  if(texture < NUM_FAST_TEX) {
    // texture found - report
    if(pglCurContext->TexFastList[texture] != NULL) {
      return((GLboolean)TRUE);
    } else {
      return((GLboolean)FALSE);
    }
  } else {
    CurTex = pglCurContext->TexHead;
    LastTex = NULL;
    while(CurTex != NULL) {
      if(CurTex->TexName == texture)
	break;
      LastTex = CurTex;
      CurTex = CurTex->Next;
    }
    
    // texture found - report
    if(CurTex != NULL) {
      return((GLboolean)TRUE);
    } else {
      return((GLboolean)FALSE);
    }
  }
}

void APIENTRY glPrioritizeTextures (GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
  GLINT_OUTSIDE_BEGIN();
}

void APIENTRY glTexEnvf (GLenum target, GLenum pname, GLfloat param)
{
  GLINT_OUTSIDE_BEGIN();

  if(target != GL_TEXTURE_ENV) {
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
  }

  switch(pname) {
  case GL_TEXTURE_ENV_MODE:
    switch((int)param) {
    case GL_REPLACE:
    case GL_MODULATE:
    case GL_DECAL:
    case GL_BLEND:
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_TEXTUREENVMODE);
    glIntIntToList((int)param);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->TexMode = (int)param;
  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params)
{
  GLfloat TexEnvColor[4];
  int TexMode;

  GLINT_OUTSIDE_BEGIN();

  switch(pname) {
  case GL_TEXTURE_ENV_MODE:
    switch((int)params[0]) {
    case GL_REPLACE:
    case GL_MODULATE:
    case GL_DECAL:
    case GL_BLEND:
      TexMode = (int)params[0];
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_ENV_COLOR:
    TexEnvColor[0] = params[0];
    TexEnvColor[1] = params[1];
    TexEnvColor[2] = params[2];
    TexEnvColor[3] = params[3];
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }

  if(pglCurContext->Listing) {

    switch(pname) {
    case GL_TEXTURE_ENV_MODE:
      glIntIntToList(OP_TEXTUREENVMODE);
      glIntIntToList(TexMode);
      break;
    case GL_TEXTURE_ENV_COLOR:
      glIntIntToList(OP_TEXTUREENVCOLOR);
      glIntFloatToList(TexEnvColor[0]);
      glIntFloatToList(TexEnvColor[1]);
      glIntFloatToList(TexEnvColor[2]);
      glIntFloatToList(TexEnvColor[3]);
      break;
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }
  
  switch(pname) {
  case GL_TEXTURE_ENV_MODE:
    pglCurContext->TexMode = TexMode;
    break;
  case GL_TEXTURE_ENV_COLOR:
    pglCurContext->TexEnvColor[0] = TexEnvColor[0];
    pglCurContext->TexEnvColor[1] = TexEnvColor[1];
    pglCurContext->TexEnvColor[2] = TexEnvColor[2];
    pglCurContext->TexEnvColor[3] = TexEnvColor[3];
    break;
  }

  pglCurContext->TexDirty = TRUE;
}

void APIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param)
{
  GLINT_OUTSIDE_BEGIN();

  if(target != GL_TEXTURE_ENV) {
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
  }

  switch(pname) {
  case GL_TEXTURE_ENV_MODE:
    switch(param) {
    case GL_REPLACE:
    case GL_MODULATE:
    case GL_DECAL:
    case GL_BLEND:
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_TEXTUREENVMODE);
    glIntIntToList(param);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->TexMode = param;
  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glTexEnviv (GLenum target, GLenum pname, const GLint *params)
{
  GLINT_OUTSIDE_BEGIN();

  if(target != GL_TEXTURE_ENV) {
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
  }

  switch(pname) {
  case GL_TEXTURE_ENV_MODE:
    switch(params[0]) {
    case GL_REPLACE:
    case GL_MODULATE:
    case GL_DECAL:
    case GL_BLEND:
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_TEXTUREENVMODE);
    glIntIntToList(params[0]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->TexMode = params[0];
  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param)
{
  pglTexture tex;

  GLINT_OUTSIDE_BEGIN();

  switch(target) {
  case GL_TEXTURE_1D:
    tex = pglCurContext->Tex1DPtr;
    break;
  case GL_TEXTURE_2D:
    tex = pglCurContext->Tex2DPtr;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {
    switch(pname) {
    case GL_TEXTURE_WRAP_S:
      switch((int)param) {
      case GL_CLAMP:
	glIntIntToList(OP_TEXTURESWRAP);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTURECLAMP_CLAMP);
	break;
      case GL_REPEAT:
	glIntIntToList(OP_TEXTURESWRAP);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTURECLAMP_WRAP);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    case GL_TEXTURE_WRAP_T:
      switch((int)param) {
      case GL_CLAMP:
	glIntIntToList(OP_TEXTURETWRAP);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTURECLAMP_CLAMP);
	break;
      case GL_REPEAT:
	glIntIntToList(OP_TEXTURETWRAP);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTURECLAMP_WRAP);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    case GL_TEXTURE_MIN_FILTER:
      switch((int)param) {
      case GL_NEAREST:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	glIntIntToList(GR_MIPMAP_DISABLE);
	glIntIntToList(FXFALSE);
	break;
      case GL_LINEAR:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	glIntIntToList(GR_MIPMAP_DISABLE);
	glIntIntToList(FXFALSE);
	break;
      case GL_NEAREST_MIPMAP_NEAREST:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXFALSE);
	break;
      case GL_NEAREST_MIPMAP_LINEAR:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXTRUE);
	break;
      case GL_LINEAR_MIPMAP_NEAREST:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXFALSE);
	break;
      case GL_LINEAR_MIPMAP_LINEAR:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXTRUE);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    case GL_TEXTURE_MAG_FILTER:
      switch((int)param) {
      case GL_NEAREST:
	glIntIntToList(OP_TEXTUREMAX);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	break;
      case GL_LINEAR:
	glIntIntToList(OP_TEXTUREMAX);
	glIntIntToList(target);
	glIntIntToList((int)param);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_TEXTURE_WRAP_S:
    switch((int)param) {
    case GL_CLAMP:
      tex->SWrap = (int)param;
      tex->GlideSWrap = GR_TEXTURECLAMP_CLAMP;
      break;
    case GL_REPEAT:
      tex->SWrap = (int)param;
      tex->GlideSWrap = GR_TEXTURECLAMP_WRAP;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_WRAP_T:
    switch((int)param) {
    case GL_CLAMP:
      tex->TWrap = (int)param;
      tex->GlideTWrap = GR_TEXTURECLAMP_CLAMP;
      break;
    case GL_REPEAT:
      tex->TWrap = (int)param;
      tex->GlideTWrap = GR_TEXTURECLAMP_WRAP;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_MIN_FILTER:
    switch((int)param) {
    case GL_NEAREST:
      tex->Min = (int)param;
      tex->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
      tex->GlideMIPMode = GR_MIPMAP_DISABLE;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_LINEAR:
      tex->Min = (int)param;
      tex->GlideMin = GR_TEXTUREFILTER_BILINEAR;
      tex->GlideMIPMode = GR_MIPMAP_DISABLE;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_NEAREST_MIPMAP_NEAREST:
      tex->Min = (int)param;
      tex->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_NEAREST_MIPMAP_LINEAR:
      tex->Min = (int)param;
      tex->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXTRUE;
      break;
    case GL_LINEAR_MIPMAP_NEAREST:
      tex->Min = (int)param;
      tex->GlideMin = GR_TEXTUREFILTER_BILINEAR;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_LINEAR_MIPMAP_LINEAR:
      tex->Min = (int)param;
      tex->GlideMin = GR_TEXTUREFILTER_BILINEAR;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXTRUE;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_MAG_FILTER:
    switch((int)param) {
    case GL_NEAREST:
      tex->Mag = (int)param;
      tex->GlideMag = GR_TEXTUREFILTER_POINT_SAMPLED;
      break;
    case GL_LINEAR:
      tex->Mag = (int)param;
      tex->GlideMag = GR_TEXTUREFILTER_BILINEAR;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }

  pglCurContext->TexDirty = TRUE;
}

void APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params)
{
  pglTexture tex;
  GLfloat BorderColor[4];

  GLINT_OUTSIDE_BEGIN();

  switch(target) {
  case GL_TEXTURE_1D:
    tex = pglCurContext->Tex1DPtr;
    break;
  case GL_TEXTURE_2D:
    tex = pglCurContext->Tex2DPtr;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(pname) {
  case GL_TEXTURE_BORDER_COLOR:
    BorderColor[0] = params[0]; 
    BorderColor[1] = params[1]; 
    BorderColor[2] = params[2]; 
    BorderColor[3] = params[3]; 
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_TEXTUREBORDER);
    glIntIntToList(target);
    glIntFloatToList(BorderColor[0]);
    glIntFloatToList(BorderColor[1]);
    glIntFloatToList(BorderColor[2]);
    glIntFloatToList(BorderColor[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  tex->BorderColor[0] = BorderColor[0];
  tex->BorderColor[1] = BorderColor[1];
  tex->BorderColor[2] = BorderColor[2];
  tex->BorderColor[3] = BorderColor[3];

  pglCurContext->TexDirty = TRUE;
}

void APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param)
{
  pglTexture tex;

  GLINT_OUTSIDE_BEGIN();

  switch(target) {
  case GL_TEXTURE_1D:
    tex = pglCurContext->Tex1DPtr;
    break;
  case GL_TEXTURE_2D:
    tex = pglCurContext->Tex2DPtr;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {
    switch(pname) {
    case GL_TEXTURE_WRAP_S:
      switch(param) {
      case GL_CLAMP:
	glIntIntToList(OP_TEXTURESWRAP);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTURECLAMP_CLAMP);
	break;
      case GL_REPEAT:
	glIntIntToList(OP_TEXTURESWRAP);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTURECLAMP_WRAP);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    case GL_TEXTURE_WRAP_T:
      switch(param) {
      case GL_CLAMP:
	glIntIntToList(OP_TEXTURETWRAP);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTURECLAMP_CLAMP);
	break;
      case GL_REPEAT:
	glIntIntToList(OP_TEXTURETWRAP);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTURECLAMP_WRAP);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    case GL_TEXTURE_MIN_FILTER:
      switch(param) {
      case GL_NEAREST:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	glIntIntToList(GR_MIPMAP_DISABLE);
	glIntIntToList(FXFALSE);
	break;
      case GL_LINEAR:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	glIntIntToList(GR_MIPMAP_DISABLE);
	glIntIntToList(FXFALSE);
	break;
      case GL_NEAREST_MIPMAP_NEAREST:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXFALSE);
	break;
      case GL_NEAREST_MIPMAP_LINEAR:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXTRUE);
	break;
      case GL_LINEAR_MIPMAP_NEAREST:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXFALSE);
	break;
      case GL_LINEAR_MIPMAP_LINEAR:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXTRUE);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    case GL_TEXTURE_MAG_FILTER:
      switch(param) {
      case GL_NEAREST:
	glIntIntToList(OP_TEXTUREMAX);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	break;
      case GL_LINEAR:
	glIntIntToList(OP_TEXTUREMAX);
	glIntIntToList(target);
	glIntIntToList(param);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_TEXTURE_WRAP_S:
    switch(param) {
    case GL_CLAMP:
      tex->SWrap = param;
      tex->GlideSWrap = GR_TEXTURECLAMP_CLAMP;
      break;
    case GL_REPEAT:
      tex->SWrap = param;
      tex->GlideSWrap = GR_TEXTURECLAMP_WRAP;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_WRAP_T:
    switch(param) {
    case GL_CLAMP:
      tex->TWrap = param;
      tex->GlideTWrap = GR_TEXTURECLAMP_CLAMP;
      break;
    case GL_REPEAT:
      tex->TWrap = param;
      tex->GlideTWrap = GR_TEXTURECLAMP_WRAP;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_MIN_FILTER:
    switch(param) {
    case GL_NEAREST:
      tex->Min = param;
      tex->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
      tex->GlideMIPMode = GR_MIPMAP_DISABLE;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_LINEAR:
      tex->Min = param;
      tex->GlideMin = GR_TEXTUREFILTER_BILINEAR;
      tex->GlideMIPMode = GR_MIPMAP_DISABLE;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_NEAREST_MIPMAP_NEAREST:
      tex->Min = param;
      tex->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_NEAREST_MIPMAP_LINEAR:
      tex->Min = param;
      tex->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXTRUE;
      break;
    case GL_LINEAR_MIPMAP_NEAREST:
      tex->Min = param;
      tex->GlideMin = GR_TEXTUREFILTER_BILINEAR;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_LINEAR_MIPMAP_LINEAR:
      tex->Min = param;
      tex->GlideMin = GR_TEXTUREFILTER_BILINEAR;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXTRUE;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_MAG_FILTER:
    switch(param) {
    case GL_NEAREST:
      tex->Mag = param;
      tex->GlideMag = GR_TEXTUREFILTER_POINT_SAMPLED;
      break;
    case GL_LINEAR:
      tex->Mag = param;
      tex->GlideMag = GR_TEXTUREFILTER_BILINEAR;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }

  pglCurContext->TexDirty = TRUE;
}

void APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params)
{
  pglTexture tex;

  GLINT_OUTSIDE_BEGIN();

  switch(target) {
  case GL_TEXTURE_1D:
    tex = pglCurContext->Tex1DPtr;
    break;
  case GL_TEXTURE_2D:
    tex = pglCurContext->Tex2DPtr;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {
    switch(pname) {
    case GL_TEXTURE_WRAP_S:
      switch(params[0]) {
      case GL_CLAMP:
	glIntIntToList(OP_TEXTURESWRAP);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTURECLAMP_CLAMP);
	break;
      case GL_REPEAT:
	glIntIntToList(OP_TEXTURESWRAP);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTURECLAMP_WRAP);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    case GL_TEXTURE_WRAP_T:
      switch(params[0]) {
      case GL_CLAMP:
	glIntIntToList(OP_TEXTURETWRAP);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTURECLAMP_CLAMP);
	break;
      case GL_REPEAT:
	glIntIntToList(OP_TEXTURETWRAP);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTURECLAMP_WRAP);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    case GL_TEXTURE_MIN_FILTER:
      switch(params[0]) {
      case GL_NEAREST:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	glIntIntToList(GR_MIPMAP_DISABLE);
	glIntIntToList(FXFALSE);
	break;
      case GL_LINEAR:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	glIntIntToList(GR_MIPMAP_DISABLE);
	glIntIntToList(FXFALSE);
	break;
      case GL_NEAREST_MIPMAP_NEAREST:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXFALSE);
	break;
      case GL_NEAREST_MIPMAP_LINEAR:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXTRUE);
	break;
      case GL_LINEAR_MIPMAP_NEAREST:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXFALSE);
	break;
      case GL_LINEAR_MIPMAP_LINEAR:
	glIntIntToList(OP_TEXTUREMIN);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	glIntIntToList(GR_MIPMAP_NEAREST);
	glIntIntToList(FXTRUE);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    case GL_TEXTURE_MAG_FILTER:
      switch(params[0]) {
      case GL_NEAREST:
	glIntIntToList(OP_TEXTUREMAX);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTUREFILTER_POINT_SAMPLED);
	break;
      case GL_LINEAR:
	glIntIntToList(OP_TEXTUREMAX);
	glIntIntToList(target);
	glIntIntToList(params[0]);
	glIntIntToList(GR_TEXTUREFILTER_BILINEAR);
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	break;
      }
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pname) {
  case GL_TEXTURE_WRAP_S:
    switch(params[0]) {
    case GL_CLAMP:
      tex->SWrap = params[0];
      tex->GlideSWrap = GR_TEXTURECLAMP_CLAMP;
      break;
    case GL_REPEAT:
      tex->SWrap = params[0];
      tex->GlideSWrap = GR_TEXTURECLAMP_WRAP;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_WRAP_T:
    switch(params[0]) {
    case GL_CLAMP:
      tex->TWrap = params[0];
      tex->GlideTWrap = GR_TEXTURECLAMP_CLAMP;
      break;
    case GL_REPEAT:
      tex->TWrap = params[0];
      tex->GlideTWrap = GR_TEXTURECLAMP_WRAP;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_MIN_FILTER:
    switch(params[0]) {
    case GL_NEAREST:
      tex->Min = params[0];
      tex->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
      tex->GlideMIPMode = GR_MIPMAP_DISABLE;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_LINEAR:
      tex->Min = params[0];
      tex->GlideMin = GR_TEXTUREFILTER_BILINEAR;
      tex->GlideMIPMode = GR_MIPMAP_DISABLE;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_NEAREST_MIPMAP_NEAREST:
      tex->Min = params[0];
      tex->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_NEAREST_MIPMAP_LINEAR:
      tex->Min = params[0];
      tex->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXTRUE;
      break;
    case GL_LINEAR_MIPMAP_NEAREST:
      tex->Min = params[0];
      tex->GlideMin = GR_TEXTUREFILTER_BILINEAR;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXFALSE;
      break;
    case GL_LINEAR_MIPMAP_LINEAR:
      tex->Min = params[0];
      tex->GlideMin = GR_TEXTUREFILTER_BILINEAR;
      tex->GlideMIPMode = GR_MIPMAP_NEAREST;
      tex->GlideLodBlend = FXTRUE;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  case GL_TEXTURE_MAG_FILTER:
    switch(params[0]) {
    case GL_NEAREST:
      tex->Mag = params[0];
      tex->GlideMag = GR_TEXTUREFILTER_POINT_SAMPLED;
      break;
    case GL_LINEAR:
      tex->Mag = params[0];
      tex->GlideMag = GR_TEXTUREFILTER_BILINEAR;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
  pglCurContext->TexDirty = TRUE;
}

void APIENTRY glGetTexEnvfv (GLenum target, GLenum pname, GLfloat *params)
{
  glIntGetTexEnv(target, pname, (void *)params, READ_FLOAT);
}

void APIENTRY glGetTexEnviv (GLenum target, GLenum pname, GLint *params)
{
  glIntGetTexEnv(target, pname, (void *)params, READ_INT);
}

void APIENTRY glGetTexGendv (GLenum coord, GLenum pname, GLdouble *params)
{
}

void APIENTRY glGetTexGenfv (GLenum coord, GLenum pname, GLfloat *params)
{
}

void APIENTRY glGetTexGeniv (GLenum coord, GLenum pname, GLint *params)
{
}

void APIENTRY glGetTexImage (GLenum target, GLint level, GLenum format, GLenum type, GLvoid *pixels)
{
}

void APIENTRY glGetTexLevelParameterfv (GLenum target, GLint level, GLenum pname, GLfloat *params)
{
}

void APIENTRY glGetTexLevelParameteriv (GLenum target, GLint level, GLenum pname, GLint *params)
{
}

void APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params)
{
}

void APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params)
{
}

void APIENTRY glTexImage1D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
  GLINT_OUTSIDE_BEGIN();

  pglCurContext->TexDirty = TRUE;
}

void APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
  int x,y;
  int pixelsize;
  int size;
  int subsize;
  unsigned char *cur_addr;
  unsigned char *pix_addr;
  int Width;
  int Height;
  GLfloat SScale;
  GLfloat TScale;
  GrLOD_t Lod;
  GrAspectRatio_t AspectRatio;
  GrTextureFormat_t TexFormat;

  GLINT_OUTSIDE_BEGIN();

  if(level == 0) {
    if((border != 0)&&(border != 1)) {
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
    }

    Width = width - (2*border);
    Height = height - (2*border);

    switch(width - (2*border)) {
    case 256:
      switch(height-(2*border)) {
      case 256:
	Lod = GR_LOD_256;
	AspectRatio = GR_ASPECT_1x1;
	break;
      case 128:
	Lod = GR_LOD_256;
	AspectRatio = GR_ASPECT_2x1;
	break;
      case 64:
	Lod = GR_LOD_256;
	AspectRatio = GR_ASPECT_4x1;
	break;
      case 32:
	Lod = GR_LOD_256;
	AspectRatio = GR_ASPECT_8x1;
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
      break;
    case 128:
      switch(height-(2*border)) {
      case 256:
	Lod = GR_LOD_256;
	AspectRatio = GR_ASPECT_1x2;
	break;
      case 128:
	Lod = GR_LOD_128;
	AspectRatio = GR_ASPECT_1x1;
	break;
      case 64:
	Lod = GR_LOD_128;
	AspectRatio = GR_ASPECT_2x1;
	break;
      case 32:
	Lod = GR_LOD_128;
	AspectRatio = GR_ASPECT_4x1;
	break;
      case 16:
	Lod = GR_LOD_128;
	AspectRatio = GR_ASPECT_8x1;
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
      break;
    case 64:
      switch(height-(2*border)) {
      case 256:
	Lod = GR_LOD_256;
	AspectRatio = GR_ASPECT_1x4;
	break;
      case 128:
	Lod = GR_LOD_128;
	AspectRatio = GR_ASPECT_1x2;
	break;
      case 64:
	Lod = GR_LOD_64;
	AspectRatio = GR_ASPECT_1x1;
	break;
      case 32:
	Lod = GR_LOD_64;
	AspectRatio = GR_ASPECT_2x1;
	break;
      case 16:
	Lod = GR_LOD_64;
	AspectRatio = GR_ASPECT_4x1;
	break;
      case 8:
	Lod = GR_LOD_64;
	AspectRatio = GR_ASPECT_8x1;
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
      break;
    case 32:
      switch(height-(2*border)) {
      case 256:
	Lod = GR_LOD_256;
	AspectRatio = GR_ASPECT_1x8;
	break;
      case 128:
	Lod = GR_LOD_128;
	AspectRatio = GR_ASPECT_1x4;
	break;
      case 64:
	Lod = GR_LOD_64;
	AspectRatio = GR_ASPECT_1x2;
	break;
      case 32:
	Lod = GR_LOD_32;
	AspectRatio = GR_ASPECT_1x1;
	break;
      case 16:
	Lod = GR_LOD_32;
	AspectRatio = GR_ASPECT_2x1;
	break;
      case 8:
	Lod = GR_LOD_32;
	AspectRatio = GR_ASPECT_4x1;
	break;
      case 4:
	Lod = GR_LOD_32;
	AspectRatio = GR_ASPECT_8x1;
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
      break;
    case 16:
      switch(height-(2*border)) {
      case 128:
	Lod = GR_LOD_128;
	AspectRatio = GR_ASPECT_1x8;
	break;
      case 64:
	Lod = GR_LOD_64;
	AspectRatio = GR_ASPECT_1x4;
	break;
      case 32:
	Lod = GR_LOD_32;
	AspectRatio = GR_ASPECT_1x2;
	break;
      case 16:
	Lod = GR_LOD_16;
	AspectRatio = GR_ASPECT_1x1;
	break;
      case 8:
	Lod = GR_LOD_16;
	AspectRatio = GR_ASPECT_2x1;
	break;
      case 4:
	Lod = GR_LOD_16;
	AspectRatio = GR_ASPECT_4x1;
	break;
      case 2:
	Lod = GR_LOD_16;
	AspectRatio = GR_ASPECT_8x1;
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
      break;
    case 8:
      switch(height-(2*border)) {
      case 64:
	Lod = GR_LOD_64;
	AspectRatio = GR_ASPECT_1x8;
	break;
      case 32:
	Lod = GR_LOD_32;
	AspectRatio = GR_ASPECT_1x4;
	break;
      case 16:
	Lod = GR_LOD_16;
	AspectRatio = GR_ASPECT_1x2;
	break;
      case 8:
	Lod = GR_LOD_8;
	AspectRatio = GR_ASPECT_1x1;
	break;
      case 4:
	Lod = GR_LOD_8;
	AspectRatio = GR_ASPECT_2x1;
	break;
      case 2:
	Lod = GR_LOD_8;
	AspectRatio = GR_ASPECT_4x1;
	break;
      case 1:
	Lod = GR_LOD_8;
	AspectRatio = GR_ASPECT_8x1;
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
      break;
    case 4:
      switch(height-(2*border)) {
      case 32:
	Lod = GR_LOD_32;
	AspectRatio = GR_ASPECT_1x8;
	break;
      case 16:
	Lod = GR_LOD_16;
	AspectRatio = GR_ASPECT_1x4;
	break;
      case 8:
	Lod = GR_LOD_8;
	AspectRatio = GR_ASPECT_1x2;
	break;
      case 4:
	Lod = GR_LOD_4;
	AspectRatio = GR_ASPECT_1x1;
	break;
      case 2:
	Lod = GR_LOD_4;
	AspectRatio = GR_ASPECT_2x1;
	break;
      case 1:
	Lod = GR_LOD_4;
	AspectRatio = GR_ASPECT_4x1;
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
      break;
    case 2:
      switch(height-(2*border)) {
      case 16:
	Lod = GR_LOD_16;
	AspectRatio = GR_ASPECT_1x8;
	break;
      case 8:
	Lod = GR_LOD_8;
	AspectRatio = GR_ASPECT_1x4;
	break;
      case 4:
	Lod = GR_LOD_4;
	AspectRatio = GR_ASPECT_1x2;
	break;
      case 2:
	Lod = GR_LOD_2;
	AspectRatio = GR_ASPECT_1x1;
	break;
      case 1:
	Lod = GR_LOD_2;
	AspectRatio = GR_ASPECT_2x1;
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
      break;
    case 1:
      switch(height-(2*border)) {
      case 8:
	Lod = GR_LOD_8;
	AspectRatio = GR_ASPECT_1x8;
	break;
      case 4:
	Lod = GR_LOD_4;
	AspectRatio = GR_ASPECT_1x4;
	break;
      case 2:
	Lod = GR_LOD_2;
	AspectRatio = GR_ASPECT_1x2;
	break;
      case 1:
	Lod = GR_LOD_1;
	AspectRatio = GR_ASPECT_1x1;
	break;
      default:
	GLINT_ERROR(GL_INVALID_ENUM);
	return;
	break;
      }
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    
    switch(AspectRatio) {
    case GR_ASPECT_8x1:
      SScale = 256.0f;
      TScale = 32.0f;
      break;
    case GR_ASPECT_4x1:
      SScale = 256.0f;
      TScale = 64.0f;
      break;
    case GR_ASPECT_2x1:
      SScale = 256.0f;
      TScale = 128.0f;
      break;
    case GR_ASPECT_1x1:
      SScale = 256.0f;
      TScale = 256.0f;
      break;
    case GR_ASPECT_1x2:
      SScale = 128.0f;
      TScale = 256.0f;
      break;
    case GR_ASPECT_1x4:
      SScale = 64.0f;
      TScale = 256.0f;
      break;
    case GR_ASPECT_1x8:
      SScale = 32.0f;
      TScale = 256.0f;
      break;
    default:
      break;
    }

    switch(internalformat) {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
      TexFormat = GR_TEXFMT_ALPHA_8;
      pixelsize = 1;
      break;
    case GL_LUMINANCE4_ALPHA4:
      TexFormat = GR_TEXFMT_ALPHA_INTENSITY_44;
      pixelsize = 1;
      break;
    case 2:
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
      TexFormat = GR_TEXFMT_ALPHA_INTENSITY_88;
      pixelsize = 2;
      break;
    case 1:
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
    case GL_INTENSITY12:
    case GL_INTENSITY16:
      TexFormat = GR_TEXFMT_INTENSITY_8;
      pixelsize = 1;
      break;
    case GL_R3_G3_B2:
      TexFormat = GR_TEXFMT_RGB_332;
      pixelsize = 1;
      break;
    case GL_RGB4:
    case GL_RGB5:
    case 3:
    case GL_RGB:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
      TexFormat = GR_TEXFMT_RGB_565;
      pixelsize = 2;
      break;
    case GL_RGB5_A1:
      TexFormat = GR_TEXFMT_ARGB_1555;
      pixelsize = 2;
      break;
    case GL_RGBA2:
    case GL_RGBA4:
    case 4:
    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGB10_A2:
    case GL_RGBA12:
    case GL_RGBA16:
      TexFormat = GR_TEXFMT_ARGB_4444;
      pixelsize = 2;
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
      break;
    }
    
    size = (width-(2*border))*(height-(2*border))*pixelsize;

    if(pglCurContext->Listing) {      
      glIntIntToList(OP_TEXTURE);
      glIntIntToList(GL_TEXTURE_2D);
      glIntIntToList(border);
      glIntIntToList(Width);
      glIntIntToList(Height);
      glIntIntToList(Lod);
      glIntIntToList(Lod);
      glIntIntToList(AspectRatio);
      glIntFloatToList(SScale);
      glIntFloatToList(TScale);
      glIntIntToList(TexFormat);
      glIntIntToList(size);
      
      pglCurContext->TexBuildData = 
	malloc((2*size)+4);
      
      // Link to data block list and advance pointer
      *((unsigned int **)pglCurContext->TexBuildData) = 
	pglCurContext->BuildDataStart;
      pglCurContext->BuildDataStart = pglCurContext->TexBuildData;
      ((unsigned int **)pglCurContext->TexBuildData)++;
      
      pglCurContext->TexBuildLevel[0] = 
	(unsigned char *)pglCurContext->TexBuildData;
      
      subsize = size;
      for(x=0;x<8;x++) {
	pglCurContext->TexBuildLevel[x+1] =
	  pglCurContext->TexBuildLevel[x] + subsize;
	subsize /= 4;
      } 
      glIntIntToList((int)(pglCurContext->TexBuildData));

      pglCurContext->TexBuildFormat = TexFormat;
    }
    
    if((!pglCurContext->Listing) ||
       (pglCurContext->Execute == GL_COMPILE_AND_EXECUTE)) {
      
      pglCurContext->Tex2DPtr->BorderWidth = border;
      pglCurContext->Tex2DPtr->Width = Width;
      pglCurContext->Tex2DPtr->Height = Height;
      pglCurContext->Tex2DPtr->GlideTex.smallLod = Lod;
      pglCurContext->Tex2DPtr->GlideTex.largeLod = Lod;
      pglCurContext->Tex2DPtr->GlideTex.aspectRatio = AspectRatio;
      pglCurContext->Tex2DPtr->SScale = SScale;
      pglCurContext->Tex2DPtr->TScale = TScale;
      pglCurContext->Tex2DPtr->GlideTex.format = TexFormat;
      pglCurContext->Tex2DPtr->GlideSize = 0;
      
      if(pglCurContext->Tex2DPtr->GlideTex.data != NULL)
	if(!pglCurContext->Tex2DPtr->TexFromList)
	  free((void *)pglCurContext->Tex2DPtr->GlideTex.data);
      pglCurContext->Tex2DPtr->TexFromList = FALSE;

      pglCurContext->Tex2DPtr->GlideTex.data = 
	malloc(2*size);
      
      pglCurContext->Tex2DPtr->Level[0] = 
	(unsigned char *)pglCurContext->Tex2DPtr->GlideTex.data;
      
      subsize = size;
      for(x=0;x<8;x++) {
	pglCurContext->Tex2DPtr->Level[x+1] =
	  pglCurContext->Tex2DPtr->Level[x] + subsize;
	subsize /= 4;
      }
    }
  } else {
    if(pglCurContext->Listing) {      
      glIntIntToList(OP_TEXTUREMIP);
      glIntIntToList(GL_TEXTURE_2D);
    }

    if((!pglCurContext->Listing) ||
       (pglCurContext->Execute == GL_COMPILE_AND_EXECUTE)) {

      /* not LOD 0 - enable full mip map */
      pglCurContext->Tex2DPtr->GlideTex.smallLod = GR_LOD_1;
    }
  }
  
  if(pglCurContext->Listing) {      
    cur_addr = (unsigned char *)pglCurContext->TexBuildLevel[level];
    TexFormat = pglCurContext->TexBuildFormat;
  } else {
    cur_addr = (unsigned char *)pglCurContext->Tex2DPtr->Level[level];
    TexFormat = pglCurContext->Tex2DPtr->GlideTex.format;
  }    

  if((internalformat == 1) && (border == 0)) {
    memcpy(cur_addr,pixels,height*width);
  } else {
    pix_addr = (unsigned char *)pixels;
    for(y=0;y<height;y++) {
      for(x=0;x<width;x++) {
	if(border) {
	  if((y != 0) &&
	     (x != 0) &&
	     (y != (height-1)) &&
	     (x != (width-1))) {
	    glIntWritePixel(&pix_addr,&cur_addr,format,type,TexFormat);
	  }
	} else {
	  glIntWritePixel(&pix_addr,&cur_addr,format,type,TexFormat);
	}
	
      }
    }
  }
  
  if(pglCurContext->Listing &&
     (pglCurContext->Execute == GL_COMPILE_AND_EXECUTE)) {

    // Copy compiled texture data to current texture data
    subsize = size;
    for(x=0;x<level;x++)
      subsize /= 4;

    memcpy((unsigned char *)pglCurContext->Tex2DPtr->Level[level],
	   (unsigned char *)pglCurContext->TexBuildLevel[level],
	   subsize);
  }

  if(pglCurContext->Tex2DPtr->Cached)
    pglCurContext->Tex2DPtr->Invalid = TRUE;

  pglCurContext->TexDirty = TRUE;
}

void APIENTRY glTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *pixels)
{
}

void APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
}

void APIENTRY glCopyTexImage1D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border)
{
}

void APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
}

void APIENTRY glCopyTexSubImage1D (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
}

void APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
}

void APIENTRY glTexGend (GLenum coord, GLenum pname, GLdouble param)
{
}

void APIENTRY glTexGendv (GLenum coord, GLenum pname, const GLdouble *params)
{
}

void APIENTRY glTexGenf (GLenum coord, GLenum pname, GLfloat param)
{
}

void APIENTRY glTexGenfv (GLenum coord, GLenum pname, const GLfloat *params)
{
}

void APIENTRY glTexGeni (GLenum coord, GLenum pname, GLint param)
{
}

void APIENTRY glTexGeniv (GLenum coord, GLenum pname, const GLint *params)
{
}

void glIntInitTexture(pglContext Context)
{
  int i;

  Context->Tex1D = FALSE;
  Context->Tex2D = FALSE;

  Context->TexMode = GL_MODULATE;

  Context->TexEnvColor[0] = 0.0f;
  Context->TexEnvColor[1] = 0.0f;
  Context->TexEnvColor[2] = 0.0f;
  Context->TexEnvColor[3] = 0.0f;

  Context->TexDirty = TRUE;

  Context->TexHead = NULL;;

  Context->CacheHead = NULL;;

  // Make this work on 4M systems by only using 2M
#define STIPPLE
#ifdef STIPPLE
  if(grTexMaxAddress(GR_TMU0) > 2048*1024) {
    // Account for John's Splash Screen
    Context->RemCacheSize = (2048*1024)-((256*32)*2)
      -(32*32)
      -(16*2);
    Context->MinCacheAddr = grTexMinAddress(GR_TMU0);
    Context->MaxCacheAddr = (2048*1024)-((256*32)*2)
      -(32*32)
      -(16*2);
  } else {
    // Account for John's Splash Screen
    Context->RemCacheSize = grTexMaxAddress(GR_TMU0)
      -((256*32)*2)
      -(32*32);
    Context->MinCacheAddr = grTexMinAddress(GR_TMU0);
    Context->MaxCacheAddr = grTexMaxAddress(GR_TMU0)
      -((256*32)*2)
      -(32*32);
  }
#else
  if(grTexMaxAddress(GR_TMU0) > 2048*1024) {
    // Account for John's Splash Screen
    Context->RemCacheSize = (2048*1024)-((256*32)*2);
    Context->MinCacheAddr = grTexMinAddress(GR_TMU0);
    Context->MaxCacheAddr = (2048*1024)-((256*32)*2);
  } else {
    // Account for John's Splash Screen
    Context->RemCacheSize = grTexMaxAddress(GR_TMU0)-((256*32)*2);
    Context->MinCacheAddr = grTexMinAddress(GR_TMU0);
    Context->MaxCacheAddr = grTexMaxAddress(GR_TMU0)-((256*32)*2);
  }
#endif

  Context->Tex1DDefPtr = (glTexture *)malloc(sizeof(glTexture));
  Context->Tex2DDefPtr = (glTexture *)malloc(sizeof(glTexture));

  Context->Tex1DPtr = Context->Tex1DDefPtr;
  Context->Tex2DPtr = Context->Tex2DDefPtr;

  Context->Tex1DName = 0;
  Context->Tex2DName = 0;

  for(i=0;i<NUM_FAST_TEX;i++)
    Context->TexFastList[i] = NULL;

  glIntInitTextureBlock(Context->Tex1DDefPtr);
  glIntInitTextureBlock(Context->Tex2DDefPtr);

#define STIPPLE
#ifdef STIPPLE
  // Init Poly Stipple
  Context->PolyStippleTex.smallLod = GR_LOD_32;
  Context->PolyStippleTex.largeLod = GR_LOD_32;
  Context->PolyStippleTex.aspectRatio = GR_ASPECT_1x1;
  Context->PolyStippleTex.format = GR_TEXFMT_ALPHA_8;
  Context->PolyStippleTex.data = Context->PolyStippleData;
  Context->PolyStippleDirty = TRUE;
  
  for(i=0;i<32*32;i++)
    Context->PolyStippleData[i] = 0xff;

  // Init Line Stipple
  Context->LineStippleTex.smallLod = GR_LOD_16;
  Context->LineStippleTex.largeLod = GR_LOD_16;
  Context->LineStippleTex.aspectRatio = GR_ASPECT_8x1;
  Context->LineStippleTex.format = GR_TEXFMT_ALPHA_8;
  Context->LineStippleTex.data = Context->LineStippleData;
  Context->LineStippleDirty = TRUE;
  
  for(i=0;i<16*2;i++)
    Context->LineStippleData[i] = 0xff;
#endif
}

void glIntInitTextureBlock(pglTexture Texture)
{
  Texture->CacheFwd = NULL;
  Texture->CacheBack = NULL;
  Texture->Cached = FALSE;
  Texture->CachedSize = 0;
  Texture->Invalid = FALSE;
  Texture->Address = 0;
  Texture->InternalFormat = 1;
  Texture->Width = 0;
  Texture->Height = 0;
  Texture->BorderWidth = 0;
  Texture->GlideSize = 0;
  Texture->SWrap = GL_REPEAT;
  Texture->GlideSWrap = GR_TEXTURECLAMP_WRAP;
  Texture->TWrap = GL_REPEAT;
  Texture->GlideTWrap = GR_TEXTURECLAMP_WRAP;
  Texture->Mag = GL_LINEAR;
  Texture->GlideMag = GR_TEXTUREFILTER_BILINEAR;
  Texture->Min = GL_NEAREST_MIPMAP_LINEAR;
  Texture->GlideMin = GR_TEXTUREFILTER_POINT_SAMPLED;
  Texture->GlideMIPMode = GR_MIPMAP_NEAREST;
  Texture->GlideLodBlend = FXTRUE;
  Texture->BorderWidth = 0;
  Texture->BorderColor[0] = 0.0f;
  Texture->BorderColor[1] = 0.0f;
  Texture->BorderColor[2] = 0.0f;
  Texture->BorderColor[3] = 0.0f;
  Texture->Priority = 1.0f;
  Texture->GlideTex.smallLod = 0;
  Texture->GlideTex.largeLod = 0;
  Texture->GlideTex.aspectRatio = 0;
  Texture->GlideTex.format = 0;
  Texture->GlideTex.data = NULL;
}

void glIntBindTexture(GLenum type, GLuint texture)
{
  pglTexBlock CurTex;
  pglTexture NewTex;

  if(texture == 0) {
    if(type == GL_TEXTURE_2D) {
      pglCurContext->Tex2DPtr = pglCurContext->Tex2DDefPtr;
    } else {
      pglCurContext->Tex1DPtr = pglCurContext->Tex1DDefPtr;
    }
  } else if(texture < NUM_FAST_TEX) {
    if(pglCurContext->TexFastList[texture] == NULL) {
      // No texture found - create new texture
      pglCurContext->TexFastList[texture] =
	(pglTexture)malloc(sizeof(glTexture));
      glIntInitTextureBlock(pglCurContext->TexFastList[texture]);
      
      // Make Current
      pglCurContext->TexFastList[texture]->Type = type;
      if(type == GL_TEXTURE_2D) {
	pglCurContext->Tex2DPtr = pglCurContext->TexFastList[texture];
      } else {
	pglCurContext->Tex1DPtr = pglCurContext->TexFastList[texture];
      }
    } else {
      // Bind to existing texture
      if(pglCurContext->TexFastList[texture]->Type != type)
	  // Wrong type - exit
	  return;
      if(type == GL_TEXTURE_2D) {
	pglCurContext->Tex2DPtr = pglCurContext->TexFastList[texture];
      } else {
	pglCurContext->Tex1DPtr = pglCurContext->TexFastList[texture];
      }
    }
  } else {
    // Search for existing texture
    CurTex = pglCurContext->TexHead;
    // DEBUG
    while(CurTex != NULL) {
      if(CurTex->TexName == texture)
	break;
      CurTex = CurTex->Next;
    }
    
    if(CurTex == NULL) {
      // No texture found - create new texture
      CurTex = (pglTexBlock)malloc(sizeof(glTexBlock));
      NewTex = (pglTexture)malloc(sizeof(glTexture));
      glIntInitTextureBlock(NewTex);
      
      // Link in
      CurTex->Next = pglCurContext->TexHead;
      pglCurContext->TexHead = CurTex;
      CurTex->TexName = texture;
      CurTex->Texture = NewTex;
      
      // Make Current
      CurTex->Texture->Type = type;
      if(type == GL_TEXTURE_2D)
	pglCurContext->Tex2DPtr = NewTex;
      else
	pglCurContext->Tex1DPtr = NewTex;
    } else {
      // Bind to existing texture
      if(CurTex->Texture->Type != type)
	// Wrong type - exit
	return;
      if(type == GL_TEXTURE_2D) {
	pglCurContext->Tex2DPtr = CurTex->Texture;
      } else {
	pglCurContext->Tex1DPtr = CurTex->Texture;
      }
    }
  }
  if(type == GL_TEXTURE_2D) {
    pglCurContext->Tex2DName = texture;
  } else {
    pglCurContext->Tex1DName = texture;
  }
}

void glIntValidateTexture()
{
  pglTexture CurTexture;
    
  if(pglCurContext->TexDirty) {
    pglCurContext->TexDirty = FALSE;

    if(!pglCurContext->Tex2D)
      return;

    if(pglCurContext->Tex2DPtr->GlideTex.data == NULL)
      return;
    
    if(pglCurContext->Tex2DPtr->Invalid) {
      if((pglCurContext->Tex2DPtr == pglCurContext->CacheHead) &&
	 (pglCurContext->CacheHead->CacheFwd == pglCurContext->CacheHead)) {
	// We are the only texture
	// This happens in old style OpenGL applications that don't
        //    use Bind
        pglCurContext->CacheHead->Cached = FALSE;
        pglCurContext->CacheHead = NULL;
        pglCurContext->RemCacheSize = pglCurContext->MaxCacheAddr;
      } else {
        if(pglCurContext->Tex2DPtr->CacheBack->Address < 
          pglCurContext->Tex2DPtr->Address) {
          // Not wrapped
          // Add cached size to previous block
          pglCurContext->Tex2DPtr->CacheBack->CachedSize += 
            pglCurContext->Tex2DPtr->CachedSize;
        } else {
          // Wrapped 
          if(pglCurContext->CacheHead == pglCurContext->Tex2DPtr) {
            // Backed up over boundary
            pglCurContext->CacheHead->CacheBack->CachedSize = 
              pglCurContext->CacheHead->CacheBack->GlideSize;
            pglCurContext->RemCacheSize = 
              pglCurContext->MaxCacheAddr -
              pglCurContext->CacheHead->CacheBack->Address -
              pglCurContext->CacheHead->CacheBack->GlideSize;
          }
        }

        // Remove from list
        pglCurContext->Tex2DPtr->CacheBack->CacheFwd = 
          pglCurContext->Tex2DPtr->CacheFwd;
        pglCurContext->Tex2DPtr->CacheFwd->CacheBack = 
          pglCurContext->Tex2DPtr->CacheBack;

        if(pglCurContext->CacheHead == pglCurContext->Tex2DPtr)
	  pglCurContext->CacheHead = pglCurContext->Tex2DPtr->CacheBack;
      }

      pglCurContext->Tex2DPtr->Invalid = FALSE;
      pglCurContext->Tex2DPtr->Cached = FALSE;
    }

    if(pglCurContext->Tex2DPtr->GlideSize == 0) {
      pglCurContext->Tex2DPtr->GlideSize = 
	grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH,
				&(pglCurContext->Tex2DPtr->GlideTex));
    }

    if(!pglCurContext->Tex2DPtr->Cached) {
      // Not Cached - Cache it
      if(pglCurContext->CacheHead != NULL) {
	// Not first texture
	GLboolean wrap;

	wrap = FALSE;
	while(pglCurContext->RemCacheSize < 
	      pglCurContext->Tex2DPtr->GlideSize) {

          CurTexture = pglCurContext->CacheHead->CacheFwd;

          if(wrap) {
            // Remove Tail Texture
            pglCurContext->CacheHead->CacheFwd =
              CurTexture->CacheFwd;
            CurTexture->CacheFwd->CacheBack =
              pglCurContext->CacheHead;
            CurTexture->Cached = FALSE;
            CurTexture->Invalid = FALSE;
            pglCurContext->RemCacheSize = CurTexture->CacheFwd->Address;
          } else {
            // Check for wrap
            if(CurTexture->Address < 
              pglCurContext->CacheHead->Address) {
            
              // Check to end of Cache
              pglCurContext->CacheHead->GlideSize = 
                pglCurContext->CacheHead->CachedSize;
              pglCurContext->RemCacheSize = 
                pglCurContext->MaxCacheAddr -
                pglCurContext->CacheHead->Address -
                pglCurContext->CacheHead->CachedSize;
            
              if(pglCurContext->RemCacheSize < 
                pglCurContext->Tex2DPtr->GlideSize) {
                // Check start of Cache
                wrap = TRUE;
                pglCurContext->RemCacheSize = 
                  CurTexture->Address;
              }
            } else {
              // Remove Tail Texture
              pglCurContext->CacheHead->CacheFwd =
                CurTexture->CacheFwd;
              CurTexture->CacheFwd->CacheBack =
                pglCurContext->CacheHead;
	      CurTexture->Cached = FALSE;
              CurTexture->Invalid = FALSE;
              pglCurContext->RemCacheSize += CurTexture->CachedSize;
            }
          }
	}

	// Insert Texture in List & Download
	pglCurContext->RemCacheSize -= 
	  pglCurContext->Tex2DPtr->GlideSize;
	
	if(wrap) {
	  pglCurContext->Tex2DPtr->Address = 
	    pglCurContext->MinCacheAddr;
	} else {
	  pglCurContext->Tex2DPtr->Address = 
	    pglCurContext->CacheHead->Address+
	      pglCurContext->CacheHead->CachedSize;
	}

	grTexDownloadMipMap(GR_TMU0,
			    pglCurContext->Tex2DPtr->Address,
			    GR_MIPMAPLEVELMASK_BOTH,
			    &(pglCurContext->Tex2DPtr->GlideTex));
	
	pglCurContext->Tex2DPtr->CacheFwd = 
	  pglCurContext->CacheHead->CacheFwd;
	pglCurContext->Tex2DPtr->CacheBack = 
	  pglCurContext->CacheHead;
	
	pglCurContext->CacheHead->CacheFwd->CacheBack =
	  pglCurContext->Tex2DPtr;
	
	pglCurContext->CacheHead->CacheFwd =
	  pglCurContext->Tex2DPtr;
	
	pglCurContext->CacheHead = pglCurContext->Tex2DPtr;
	
	pglCurContext->Tex2DPtr->CachedSize = 
	  pglCurContext->Tex2DPtr->GlideSize;

	pglCurContext->Tex2DPtr->Cached = TRUE;
      } else {
	// First Texture
	pglCurContext->Tex2DPtr->Address = pglCurContext->MinCacheAddr;
	
	grTexDownloadMipMap(GR_TMU0,
			    pglCurContext->Tex2DPtr->Address,
			    GR_MIPMAPLEVELMASK_BOTH,
			    &(pglCurContext->Tex2DPtr->GlideTex));
	
	pglCurContext->RemCacheSize -= pglCurContext->Tex2DPtr->GlideSize;
	
	pglCurContext->CacheHead = pglCurContext->Tex2DPtr;
	
	pglCurContext->Tex2DPtr->CacheFwd = pglCurContext->Tex2DPtr;
	pglCurContext->Tex2DPtr->CacheBack = pglCurContext->Tex2DPtr;
	
	pglCurContext->Tex2DPtr->CachedSize = 
	  pglCurContext->Tex2DPtr->GlideSize;

	pglCurContext->Tex2DPtr->Cached = TRUE;
	
      }
    }
    
    grTexMipMapMode(GR_TMU0,
		    pglCurContext->Tex2DPtr->GlideMIPMode,
		    FXFALSE);
  
    grTexSource(GR_TMU0,
		pglCurContext->Tex2DPtr->Address,
	        GR_MIPMAPLEVELMASK_BOTH,
	        &(pglCurContext->Tex2DPtr->GlideTex));
    
    grTexClampMode(GR_TMU0,
	  	   pglCurContext->Tex2DPtr->GlideSWrap,
		   pglCurContext->Tex2DPtr->GlideTWrap);
    
    grTexFilterMode(GR_TMU0,
		    pglCurContext->Tex2DPtr->GlideMin,
		    pglCurContext->Tex2DPtr->GlideMag);
  } 
}

void glIntGetTexEnv(GLenum target, GLenum pname, void *params, int type)
{
  if(target != GL_TEXTURE_ENV) {
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
  }

  switch(pname) {
  case GL_TEXTURE_ENV_MODE:
    GLINT_PARAMC(&params, pglCurContext->TexMode);
    break;
  case GL_TEXTURE_ENV_COLOR:
    GLINT_PARAM(&params, pglCurContext->TexEnvColor[0], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->TexEnvColor[1], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->TexEnvColor[2], READ_FLOATC);
    GLINT_PARAM(&params, pglCurContext->TexEnvColor[3], READ_FLOATC);
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
  }
}

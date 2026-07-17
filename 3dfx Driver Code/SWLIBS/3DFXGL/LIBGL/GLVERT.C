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

void APIENTRY glVertex2d (GLdouble x, GLdouble y)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)x;
  pglCurContext->CurObj[1] = (float)y;
  pglCurContext->CurObj[2] = 0.0f;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex2dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    // glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)v[0];
  pglCurContext->CurObj[1] = (float)v[1];
  pglCurContext->CurObj[2] = 0.0f;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex2f (GLfloat x, GLfloat y)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = x;
  pglCurContext->CurObj[1] = y;
  pglCurContext->CurObj[2] = 0.0f;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex2fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = v[0];
  pglCurContext->CurObj[1] = v[1];
  pglCurContext->CurObj[2] = 0.0f;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex2i (GLint x, GLint y)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)x;
  pglCurContext->CurObj[1] = (float)y;
  pglCurContext->CurObj[2] = 0.0f;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex2iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)v[0];
  pglCurContext->CurObj[1] = (float)v[1];
  pglCurContext->CurObj[2] = 0.0f;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex2s (GLshort x, GLshort y)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)x;
  pglCurContext->CurObj[1] = (float)y;
  pglCurContext->CurObj[2] = 0.0f;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex2sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)v[0];
  pglCurContext->CurObj[1] = (float)v[1];
  pglCurContext->CurObj[2] = 0.0f;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex3d (GLdouble x, GLdouble y, GLdouble z)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)x;
  pglCurContext->CurObj[1] = (float)y;
  pglCurContext->CurObj[2] = (float)z;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex3dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)v[0];
  pglCurContext->CurObj[1] = (float)v[1];
  pglCurContext->CurObj[2] = (float)v[2];
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex3f (GLfloat x, GLfloat y, GLfloat z)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = x;
  pglCurContext->CurObj[1] = y;
  pglCurContext->CurObj[2] = z;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex3fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = v[0];
  pglCurContext->CurObj[1] = v[1];
  pglCurContext->CurObj[2] = v[2];
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex3i (GLint x, GLint y, GLint z)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)x;
  pglCurContext->CurObj[1] = (float)y;
  pglCurContext->CurObj[2] = (float)z;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex3iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)v[0];
  pglCurContext->CurObj[1] = (float)v[1];
  pglCurContext->CurObj[2] = (float)v[2];
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex3s (GLshort x, GLshort y, GLshort z)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)x;
  pglCurContext->CurObj[1] = (float)y;
  pglCurContext->CurObj[2] = (float)z;
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex3sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)v[0];
  pglCurContext->CurObj[1] = (float)v[1];
  pglCurContext->CurObj[2] = (float)v[2];
  pglCurContext->CurObj[3] = 1.0f;

  glIntVertex();
}

void APIENTRY glVertex4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList((float)w);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)x;
  pglCurContext->CurObj[1] = (float)y;
  pglCurContext->CurObj[2] = (float)z;
  pglCurContext->CurObj[3] = (float)w;

  glIntVertex();
}

void APIENTRY glVertex4dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)v[0];
  pglCurContext->CurObj[1] = (float)v[1];
  pglCurContext->CurObj[2] = (float)v[2];
  pglCurContext->CurObj[3] = (float)v[3];

  glIntVertex();
}

void APIENTRY glVertex4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList((float)w);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = x;
  pglCurContext->CurObj[1] = y;
  pglCurContext->CurObj[2] = z;
  pglCurContext->CurObj[3] = w;

  glIntVertex();
}

void APIENTRY glVertex4fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = v[0];
  pglCurContext->CurObj[1] = v[1];
  pglCurContext->CurObj[2] = v[2];
  pglCurContext->CurObj[3] = v[3];

  glIntVertex();
}

void APIENTRY glVertex4i (GLint x, GLint y, GLint z, GLint w)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList((float)w);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)x;
  pglCurContext->CurObj[1] = (float)y;
  pglCurContext->CurObj[2] = (float)z;
  pglCurContext->CurObj[3] = (float)w;

  glIntVertex();
}

void APIENTRY glVertex4iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)v[0];
  pglCurContext->CurObj[1] = (float)v[1];
  pglCurContext->CurObj[2] = (float)v[2];
  pglCurContext->CurObj[3] = (float)v[3];

  glIntVertex();
}

void APIENTRY glVertex4s (GLshort x, GLshort y, GLshort z, GLshort w)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList((float)w);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)x;
  pglCurContext->CurObj[1] = (float)y;
  pglCurContext->CurObj[2] = (float)z;
  pglCurContext->CurObj[3] = (float)w;

  glIntVertex();
}

void APIENTRY glVertex4sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_VERTEX_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurObj[0] = (float)v[0];
  pglCurContext->CurObj[1] = (float)v[1];
  pglCurContext->CurObj[2] = (float)v[2];
  pglCurContext->CurObj[3] = (float)v[3];

  glIntVertex();
}


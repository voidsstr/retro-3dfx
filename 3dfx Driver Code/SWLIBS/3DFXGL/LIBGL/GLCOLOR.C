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


void APIENTRY glColor3b (GLbyte red, GLbyte green, GLbyte blue)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorbtoFloat(red));
    glIntFloatToList(ColorbtoFloat(green));
    glIntFloatToList(ColorbtoFloat(blue));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorbtoFloat(red);
  pglCurContext->CurColor[1] = ColorbtoFloat(green);
  pglCurContext->CurColor[2] = ColorbtoFloat(blue);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3bv (const GLbyte *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorbtoFloat(v[0]));
    glIntFloatToList(ColorbtoFloat(v[1]));
    glIntFloatToList(ColorbtoFloat(v[2]));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorbtoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorbtoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorbtoFloat(v[2]);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3d (GLdouble red, GLdouble green, GLdouble blue)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColordtoFloat(red));
    glIntFloatToList(ColordtoFloat(green));
    glIntFloatToList(ColordtoFloat(blue));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColordtoFloat(red);
  pglCurContext->CurColor[1] = ColordtoFloat(green);
  pglCurContext->CurColor[2] = ColordtoFloat(blue);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColordtoFloat(v[0]));
    glIntFloatToList(ColordtoFloat(v[1]));
    glIntFloatToList(ColordtoFloat(v[2]));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColordtoFloat(v[0]);
  pglCurContext->CurColor[1] = ColordtoFloat(v[1]);
  pglCurContext->CurColor[2] = ColordtoFloat(v[2]);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3f (GLfloat red, GLfloat green, GLfloat blue)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorftoFloat(red));
    glIntFloatToList(ColorftoFloat(green));
    glIntFloatToList(ColorftoFloat(blue));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorftoFloat(red);
  pglCurContext->CurColor[1] = ColorftoFloat(green);
  pglCurContext->CurColor[2] = ColorftoFloat(blue);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorftoFloat(v[0]));
    glIntFloatToList(ColorftoFloat(v[1]));
    glIntFloatToList(ColorftoFloat(v[2]));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorftoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorftoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorftoFloat(v[2]);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3i (GLint red, GLint green, GLint blue)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColoritoFloat(red));
    glIntFloatToList(ColoritoFloat(green));
    glIntFloatToList(ColoritoFloat(blue));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColoritoFloat(red);
  pglCurContext->CurColor[1] = ColoritoFloat(green);
  pglCurContext->CurColor[2] = ColoritoFloat(blue);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColoritoFloat(v[0]));
    glIntFloatToList(ColoritoFloat(v[1]));
    glIntFloatToList(ColoritoFloat(v[2]));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColoritoFloat(v[0]);
  pglCurContext->CurColor[1] = ColoritoFloat(v[1]);
  pglCurContext->CurColor[2] = ColoritoFloat(v[2]);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3s (GLshort red, GLshort green, GLshort blue)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorstoFloat(red));
    glIntFloatToList(ColorstoFloat(green));
    glIntFloatToList(ColorstoFloat(blue));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorstoFloat(red);
  pglCurContext->CurColor[1] = ColorstoFloat(green);
  pglCurContext->CurColor[2] = ColorstoFloat(blue);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorstoFloat(v[0]));
    glIntFloatToList(ColorstoFloat(v[1]));
    glIntFloatToList(ColorstoFloat(v[2]));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorstoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorstoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorstoFloat(v[2]);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3ub (GLubyte red, GLubyte green, GLubyte blue)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorubtoFloat(red));
    glIntFloatToList(ColorubtoFloat(green));
    glIntFloatToList(ColorubtoFloat(blue));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorubtoFloat(red);
  pglCurContext->CurColor[1] = ColorubtoFloat(green);
  pglCurContext->CurColor[2] = ColorubtoFloat(blue);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3ubv (const GLubyte *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorubtoFloat(v[0]));
    glIntFloatToList(ColorubtoFloat(v[1]));
    glIntFloatToList(ColorubtoFloat(v[2]));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorubtoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorubtoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorubtoFloat(v[2]);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3ui (GLuint red, GLuint green, GLuint blue)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColoruitoFloat(red));
    glIntFloatToList(ColoruitoFloat(green));
    glIntFloatToList(ColoruitoFloat(blue));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColoruitoFloat(red);
  pglCurContext->CurColor[1] = ColoruitoFloat(green);
  pglCurContext->CurColor[2] = ColoruitoFloat(blue);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3uiv (const GLuint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColoruitoFloat(v[0]));
    glIntFloatToList(ColoruitoFloat(v[1]));
    glIntFloatToList(ColoruitoFloat(v[2]));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColoruitoFloat(v[0]);
  pglCurContext->CurColor[1] = ColoruitoFloat(v[1]);
  pglCurContext->CurColor[2] = ColoruitoFloat(v[2]);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3us (GLushort red, GLushort green, GLushort blue)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorustoFloat(red));
    glIntFloatToList(ColorustoFloat(green));
    glIntFloatToList(ColorustoFloat(blue));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorustoFloat(red);
  pglCurContext->CurColor[1] = ColorustoFloat(green);
  pglCurContext->CurColor[2] = ColorustoFloat(blue);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor3usv (const GLushort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorustoFloat(v[0]));
    glIntFloatToList(ColorustoFloat(v[1]));
    glIntFloatToList(ColorustoFloat(v[2]));
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorustoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorustoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorustoFloat(v[2]);
  pglCurContext->CurColor[3] = 1.0f;
}

void APIENTRY glColor4b (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorbtoFloat(red));
    glIntFloatToList(ColorbtoFloat(green));
    glIntFloatToList(ColorbtoFloat(blue));
    glIntFloatToList(ColorbtoFloat(alpha));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorbtoFloat(red);
  pglCurContext->CurColor[1] = ColorbtoFloat(green);
  pglCurContext->CurColor[2] = ColorbtoFloat(blue);
  pglCurContext->CurColor[3] = ColorbtoFloat(alpha);
}

void APIENTRY glColor4bv (const GLbyte *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorbtoFloat(v[0]));
    glIntFloatToList(ColorbtoFloat(v[1]));
    glIntFloatToList(ColorbtoFloat(v[2]));
    glIntFloatToList(ColorbtoFloat(v[3]));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorbtoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorbtoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorbtoFloat(v[2]);
  pglCurContext->CurColor[3] = ColorbtoFloat(v[3]);
}

void APIENTRY glColor4d (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColordtoFloat(red));
    glIntFloatToList(ColordtoFloat(green));
    glIntFloatToList(ColordtoFloat(blue));
    glIntFloatToList(ColordtoFloat(alpha));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColordtoFloat(red);
  pglCurContext->CurColor[1] = ColordtoFloat(green);
  pglCurContext->CurColor[2] = ColordtoFloat(blue);
  pglCurContext->CurColor[3] = ColordtoFloat(alpha);
}

void APIENTRY glColor4dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColordtoFloat(v[0]));
    glIntFloatToList(ColordtoFloat(v[1]));
    glIntFloatToList(ColordtoFloat(v[2]));
    glIntFloatToList(ColordtoFloat(v[3]));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColordtoFloat(v[0]);
  pglCurContext->CurColor[1] = ColordtoFloat(v[1]);
  pglCurContext->CurColor[2] = ColordtoFloat(v[2]);
  pglCurContext->CurColor[3] = ColordtoFloat(v[3]);
}

void APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorftoFloat(red));
    glIntFloatToList(ColorftoFloat(green));
    glIntFloatToList(ColorftoFloat(blue));
    glIntFloatToList(ColorftoFloat(alpha));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorftoFloat(red);
  pglCurContext->CurColor[1] = ColorftoFloat(green);
  pglCurContext->CurColor[2] = ColorftoFloat(blue);
  pglCurContext->CurColor[3] = ColorftoFloat(alpha);
}

void APIENTRY glColor4fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorftoFloat(v[0]));
    glIntFloatToList(ColorftoFloat(v[1]));
    glIntFloatToList(ColorftoFloat(v[2]));
    glIntFloatToList(ColorftoFloat(v[3]));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorftoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorftoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorftoFloat(v[2]);
  pglCurContext->CurColor[3] = ColorftoFloat(v[3]);
}

void APIENTRY glColor4i (GLint red, GLint green, GLint blue, GLint alpha)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColoritoFloat(red));
    glIntFloatToList(ColoritoFloat(green));
    glIntFloatToList(ColoritoFloat(blue));
    glIntFloatToList(ColoritoFloat(alpha));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColoritoFloat(red);
  pglCurContext->CurColor[1] = ColoritoFloat(green);
  pglCurContext->CurColor[2] = ColoritoFloat(blue);
  pglCurContext->CurColor[3] = ColoritoFloat(alpha);
}

void APIENTRY glColor4iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColoritoFloat(v[0]));
    glIntFloatToList(ColoritoFloat(v[1]));
    glIntFloatToList(ColoritoFloat(v[2]));
    glIntFloatToList(ColoritoFloat(v[3]));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColoritoFloat(v[0]);
  pglCurContext->CurColor[1] = ColoritoFloat(v[1]);
  pglCurContext->CurColor[2] = ColoritoFloat(v[2]);
  pglCurContext->CurColor[3] = ColoritoFloat(v[3]);
}

void APIENTRY glColor4s (GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorstoFloat(red));
    glIntFloatToList(ColorstoFloat(green));
    glIntFloatToList(ColorstoFloat(blue));
    glIntFloatToList(ColorstoFloat(alpha));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorstoFloat(red);
  pglCurContext->CurColor[1] = ColorstoFloat(green);
  pglCurContext->CurColor[2] = ColorstoFloat(blue);
  pglCurContext->CurColor[3] = ColorstoFloat(alpha);
}

void APIENTRY glColor4sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorstoFloat(v[0]));
    glIntFloatToList(ColorstoFloat(v[1]));
    glIntFloatToList(ColorstoFloat(v[2]));
    glIntFloatToList(ColorstoFloat(v[3]));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorstoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorstoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorstoFloat(v[2]);
  pglCurContext->CurColor[3] = ColorstoFloat(v[3]);
}

void APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorubtoFloat(red));
    glIntFloatToList(ColorubtoFloat(green));
    glIntFloatToList(ColorubtoFloat(blue));
    glIntFloatToList(ColorubtoFloat(alpha));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorubtoFloat(red);
  pglCurContext->CurColor[1] = ColorubtoFloat(green);
  pglCurContext->CurColor[2] = ColorubtoFloat(blue);
  pglCurContext->CurColor[3] = ColorubtoFloat(alpha);
}

void APIENTRY glColor4ubv (const GLubyte *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorubtoFloat(v[0]));
    glIntFloatToList(ColorubtoFloat(v[1]));
    glIntFloatToList(ColorubtoFloat(v[2]));
    glIntFloatToList(ColorubtoFloat(v[3]));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorubtoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorubtoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorubtoFloat(v[2]);
  pglCurContext->CurColor[3] = ColorubtoFloat(v[3]);
}

void APIENTRY glColor4ui (GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColoruitoFloat(red));
    glIntFloatToList(ColoruitoFloat(green));
    glIntFloatToList(ColoruitoFloat(blue));
    glIntFloatToList(ColoruitoFloat(alpha));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColoruitoFloat(red);
  pglCurContext->CurColor[1] = ColoruitoFloat(green);
  pglCurContext->CurColor[2] = ColoruitoFloat(blue);
  pglCurContext->CurColor[3] = ColoruitoFloat(alpha);
}

void APIENTRY glColor4uiv (const GLuint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColoruitoFloat(v[0]));
    glIntFloatToList(ColoruitoFloat(v[1]));
    glIntFloatToList(ColoruitoFloat(v[2]));
    glIntFloatToList(ColoruitoFloat(v[3]));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColoruitoFloat(v[0]);
  pglCurContext->CurColor[1] = ColoruitoFloat(v[1]);
  pglCurContext->CurColor[2] = ColoruitoFloat(v[2]);
  pglCurContext->CurColor[3] = ColoruitoFloat(v[3]);
}

void APIENTRY glColor4us (GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorustoFloat(red));
    glIntFloatToList(ColorustoFloat(green));
    glIntFloatToList(ColorustoFloat(blue));
    glIntFloatToList(ColorustoFloat(alpha));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorustoFloat(red);
  pglCurContext->CurColor[1] = ColorustoFloat(green);
  pglCurContext->CurColor[2] = ColorustoFloat(blue);
  pglCurContext->CurColor[3] = ColorustoFloat(alpha);
}

void APIENTRY glColor4usv (const GLushort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(5);
    glIntIntToList(OP_COLOR_COORD);
    glIntFloatToList(ColorustoFloat(v[0]));
    glIntFloatToList(ColorustoFloat(v[1]));
    glIntFloatToList(ColorustoFloat(v[2]));
    glIntFloatToList(ColorustoFloat(v[3]));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurColor[0] = ColorustoFloat(v[0]);
  pglCurContext->CurColor[1] = ColorustoFloat(v[1]);
  pglCurContext->CurColor[2] = ColorustoFloat(v[2]);
  pglCurContext->CurColor[3] = ColorustoFloat(v[3]);
}


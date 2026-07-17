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

void APIENTRY glNormal3b (GLbyte nx, GLbyte ny, GLbyte nz)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)nx);
    glIntFloatToList((float)ny);
    glIntFloatToList((float)nz);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = (float)nx;
  pglCurContext->CurNormal[1] = (float)ny;
  pglCurContext->CurNormal[2] = (float)nz;
}

void APIENTRY glNormal3bv (const GLbyte *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = (float)v[0];
  pglCurContext->CurNormal[1] = (float)v[1];
  pglCurContext->CurNormal[2] = (float)v[2];
}

void APIENTRY glNormal3d (GLdouble nx, GLdouble ny, GLdouble nz)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)nx);
    glIntFloatToList((float)ny);
    glIntFloatToList((float)nz);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = (float)nx;
  pglCurContext->CurNormal[1] = (float)ny;
  pglCurContext->CurNormal[2] = (float)nz;
}

void APIENTRY glNormal3dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = (float)v[0];
  pglCurContext->CurNormal[1] = (float)v[1];
  pglCurContext->CurNormal[2] = (float)v[2];
}

void APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)nx);
    glIntFloatToList((float)ny);
    glIntFloatToList((float)nz);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = nx;
  pglCurContext->CurNormal[1] = ny;
  pglCurContext->CurNormal[2] = nz;
}

void APIENTRY glNormal3fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = v[0];
  pglCurContext->CurNormal[1] = v[1];
  pglCurContext->CurNormal[2] = v[2];
}

void APIENTRY glNormal3i (GLint nx, GLint ny, GLint nz)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)nx);
    glIntFloatToList((float)ny);
    glIntFloatToList((float)nz);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = (float)nx;
  pglCurContext->CurNormal[1] = (float)ny;
  pglCurContext->CurNormal[2] = (float)nz;
}

void APIENTRY glNormal3iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = (float)v[0];
  pglCurContext->CurNormal[1] = (float)v[1];
  pglCurContext->CurNormal[2] = (float)v[2];
}

void APIENTRY glNormal3s (GLshort nx, GLshort ny, GLshort nz)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)nx);
    glIntFloatToList((float)ny);
    glIntFloatToList((float)nz);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = (float)nx;
  pglCurContext->CurNormal[1] = (float)ny;
  pglCurContext->CurNormal[2] = (float)nz;
}

void APIENTRY glNormal3sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntMakeRoom(4);
    glIntIntToList(OP_NORMAL_COORD);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->CurNormal[0] = (float)v[0];
  pglCurContext->CurNormal[1] = (float)v[1];
  pglCurContext->CurNormal[2] = (float)v[2];
}


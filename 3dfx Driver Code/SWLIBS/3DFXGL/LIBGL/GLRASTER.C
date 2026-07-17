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

void APIENTRY glRasterPos2d (GLdouble x, GLdouble y)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)x;
  pglCurContext->RasObj[1] = (float)y;
  pglCurContext->RasObj[2] = 0.0f;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos2dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)v[0];
  pglCurContext->RasObj[1] = (float)v[1];
  pglCurContext->RasObj[2] = 0.0f;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos2f (GLfloat x, GLfloat y)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList(x);
    glIntFloatToList(y);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = x;
  pglCurContext->RasObj[1] = y;
  pglCurContext->RasObj[2] = 0.0f;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos2fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList(v[0]);
    glIntFloatToList(v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = v[0];
  pglCurContext->RasObj[1] = v[1];
  pglCurContext->RasObj[2] = 0.0f;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos2i (GLint x, GLint y)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)x;
  pglCurContext->RasObj[1] = (float)y;
  pglCurContext->RasObj[2] = 0.0f;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos2iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)v[0];
  pglCurContext->RasObj[1] = (float)v[1];
  pglCurContext->RasObj[2] = 0.0f;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos2s (GLshort x, GLshort y)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)x;
  pglCurContext->RasObj[1] = (float)y;
  pglCurContext->RasObj[2] = 0.0f;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos2sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList(0.0f);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)v[0];
  pglCurContext->RasObj[1] = (float)v[1];
  pglCurContext->RasObj[2] = 0.0f;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos3d (GLdouble x, GLdouble y, GLdouble z)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)x;
  pglCurContext->RasObj[1] = (float)y;
  pglCurContext->RasObj[2] = (float)z;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos3dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)v[0];
  pglCurContext->RasObj[1] = (float)v[1];
  pglCurContext->RasObj[2] = (float)v[2];
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos3f (GLfloat x, GLfloat y, GLfloat z)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList(x);
    glIntFloatToList(y);
    glIntFloatToList(z);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = x;
  pglCurContext->RasObj[1] = y;
  pglCurContext->RasObj[2] = z;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos3fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList(v[0]);
    glIntFloatToList(v[1]);
    glIntFloatToList(v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = v[0];
  pglCurContext->RasObj[1] = v[1];
  pglCurContext->RasObj[2] = v[2];
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos3i (GLint x, GLint y, GLint z)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)x;
  pglCurContext->RasObj[1] = (float)y;
  pglCurContext->RasObj[2] = (float)z;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos3iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)v[0];
  pglCurContext->RasObj[1] = (float)v[1];
  pglCurContext->RasObj[2] = (float)v[2];
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos3s (GLshort x, GLshort y, GLshort z)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)x;
  pglCurContext->RasObj[1] = (float)y;
  pglCurContext->RasObj[2] = (float)z;
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos3sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList(1.0f);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)v[0];
  pglCurContext->RasObj[1] = (float)v[1];
  pglCurContext->RasObj[2] = (float)v[2];
  pglCurContext->RasObj[3] = 1.0f;

  glIntRasterPos();
}

void APIENTRY glRasterPos4d (GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList((float)w);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)x;
  pglCurContext->RasObj[1] = (float)y;
  pglCurContext->RasObj[2] = (float)z;
  pglCurContext->RasObj[3] = (float)w;

  glIntRasterPos();
}

void APIENTRY glRasterPos4dv (const GLdouble *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)v[0];
  pglCurContext->RasObj[1] = (float)v[1];
  pglCurContext->RasObj[2] = (float)v[2];
  pglCurContext->RasObj[3] = (float)v[3];

  glIntRasterPos();
}

void APIENTRY glRasterPos4f (GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList(x);
    glIntFloatToList(y);
    glIntFloatToList(z);
    glIntFloatToList(w);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = x;
  pglCurContext->RasObj[1] = y;
  pglCurContext->RasObj[2] = z;
  pglCurContext->RasObj[3] = w;

  glIntRasterPos();
}

void APIENTRY glRasterPos4fv (const GLfloat *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList(v[0]);
    glIntFloatToList(v[1]);
    glIntFloatToList(v[2]);
    glIntFloatToList(v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = v[0];
  pglCurContext->RasObj[1] = v[1];
  pglCurContext->RasObj[2] = v[2];
  pglCurContext->RasObj[3] = v[3];

  glIntRasterPos();
}

void APIENTRY glRasterPos4i (GLint x, GLint y, GLint z, GLint w)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList((float)w);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)x;
  pglCurContext->RasObj[1] = (float)y;
  pglCurContext->RasObj[2] = (float)z;
  pglCurContext->RasObj[3] = (float)w;

  glIntRasterPos();
}

void APIENTRY glRasterPos4iv (const GLint *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)v[0];
  pglCurContext->RasObj[1] = (float)v[1];
  pglCurContext->RasObj[2] = (float)v[2];
  pglCurContext->RasObj[3] = (float)v[3];

  glIntRasterPos();
}

void APIENTRY glRasterPos4s (GLshort x, GLshort y, GLshort z, GLshort w)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)x);
    glIntFloatToList((float)y);
    glIntFloatToList((float)z);
    glIntFloatToList((float)w);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)x;
  pglCurContext->RasObj[1] = (float)y;
  pglCurContext->RasObj[2] = (float)z;
  pglCurContext->RasObj[3] = (float)w;

  glIntRasterPos();
}

void APIENTRY glRasterPos4sv (const GLshort *v)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_RASTERPOS);
    glIntFloatToList((float)v[0]);
    glIntFloatToList((float)v[1]);
    glIntFloatToList((float)v[2]);
    glIntFloatToList((float)v[3]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RasObj[0] = (float)v[0];
  pglCurContext->RasObj[1] = (float)v[1];
  pglCurContext->RasObj[2] = (float)v[2];
  pglCurContext->RasObj[3] = (float)v[3];

  glIntRasterPos();
}

void glIntRasterPos(void)
{
  GLfloat eye[4];

  glIntValidateContext();
  glIntValidateTexture();
  if(pglCurContext->Lighting) {
    glIntValidateInvTransp();
  }
  glIntValidateComposite();
  glIntValidateNeedEye();

  // Model to Eye
  glIntXform(pglCurContext->RasObj,
	     pglCurContext->CurModelView,
	     eye);

  // Eye to Clip
  glIntXform(eye,
	     pglCurContext->CurProjMat,
	     pglCurContext->RasVtx.Clip);
  
  pglCurContext->RasVtx.Projected = FALSE;
  
  pglCurContext->RasVtx.Color[0] = pglCurContext->CurColor[0];
  pglCurContext->RasVtx.Color[1] = pglCurContext->CurColor[1];
  pglCurContext->RasVtx.Color[2] = pglCurContext->CurColor[2];
  pglCurContext->RasVtx.Color[3] = pglCurContext->CurColor[3];

  pglCurContext->RasVtx.Texture[0] = pglCurContext->CurTex[0];
  pglCurContext->RasVtx.Texture[1] = pglCurContext->CurTex[1];
  pglCurContext->RasVtx.Texture[2] = pglCurContext->CurTex[2];
  pglCurContext->RasVtx.Texture[3] = pglCurContext->CurTex[3];

  // Light
  glIntLight(&pglCurContext->RasVtx);

  // Project
  glIntProject(&pglCurContext->RasVtx);

  glIntOutCodes(&pglCurContext->RasVtx);

  // Trivial Reject
  if(pglCurContext->RasVtx.OutCodes != 0) {
    pglCurContext->RasValid = TRUE;
    return;
  }

  pglCurContext->RasValid = TRUE;

  switch(pglCurContext->RenderMode) {
  case GL_RENDER:
    break;
  case GL_SELECT:
    {
      GLfloat zmin, zmax;

      zmin = zmax = pglCurContext->RasVtx.Glide.z;

      glIntWriteSelect(zmin, zmax);
    }
    break;
  case GL_FEEDBACK:
    glIntWriteFeedback(&pglCurContext->RasVtx);
    break;
  }
}

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

void APIENTRY glMatrixMode (GLenum mode)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    switch(mode) {
    case GL_MODELVIEW:
      break;
    case GL_PROJECTION:
      break;
    case GL_TEXTURE:
      break;
    default:
      GLINT_ERROR(GL_INVALID_ENUM);
      break;
    }
    glIntIntToList(OP_MATRIXMODE);
    glIntIntToList(mode);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(mode) {
  case GL_MODELVIEW:
    pglCurContext->MatMode = mode;
    break;
  case GL_PROJECTION:
    pglCurContext->MatMode = mode;
    break;
  case GL_TEXTURE:
    pglCurContext->MatMode = mode;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
}

void APIENTRY glLoadIdentity (void)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_LOADIDENTITY);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    glIntIdentity(pglCurContext->CurModelView);
    glIntIdentity(pglCurContext->CurInvTransp);
    pglCurContext->InvTranspDirty = FALSE;
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_PROJECTION:
    glIntIdentity(pglCurContext->CurProjMat);
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_TEXTURE:
    glIntIdentity(pglCurContext->CurTexMat);
    pglCurContext->TexMatrixIdentity = TRUE;
    break;
  }
}

void APIENTRY glLoadMatrixd (const GLdouble *m)
{
  GLfloat tmp[16];
  int i;

  for(i=0;i<16;i++)
    tmp[i] = (float)m[i];

  glLoadMatrixf (tmp);
}

void APIENTRY glLoadMatrixf (const GLfloat *m)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    int i;

    glIntIntToList(OP_LOADMATRIX);
    for(i=0;i<16;i++)
      glIntFloatToList(m[i]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    glIntMatCopy((GLfloat *)m,pglCurContext->CurModelView);
    // Calculate inverse transpose for normals
    pglCurContext->InvTranspDirty = TRUE;
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_PROJECTION:
    glIntMatCopy((GLfloat *)m,pglCurContext->CurProjMat);
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_TEXTURE:
    glIntMatCopy((GLfloat *)m,pglCurContext->CurTexMat);
    // Eventually Characterize?
    pglCurContext->TexMatrixIdentity = FALSE;
    break;
  }
}

void APIENTRY glMultMatrixd (const GLdouble *m)
{
  GLfloat tmp[16];
  int i;

  for(i=0;i<16;i++)
    tmp[i] = (float)m[i];

  glMultMatrixf (tmp);
}

void APIENTRY glMultMatrixf (const GLfloat *m)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    int i;

    glIntIntToList(OP_MULTMATRIX);
    for(i=0;i<16;i++)
      glIntFloatToList(m[i]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    glIntMatMult((GLfloat *)m,pglCurContext->CurModelView);
    // Calculate inverse transpose for normals
    pglCurContext->InvTranspDirty = TRUE;
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_PROJECTION:
    glIntMatMult((GLfloat *)m,pglCurContext->CurProjMat);
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_TEXTURE:
    glIntMatMult((GLfloat *)m,pglCurContext->CurTexMat);
    // Eventually Characterize?
    pglCurContext->TexMatrixIdentity = FALSE;
    break;
  }
}

void APIENTRY glPushMatrix (void)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    
    glIntIntToList(OP_PUSHMATRIX);
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    if(pglCurContext->ModelViewPos < 32) {
      glIntMatCopy(pglCurContext->CurModelView,
		   pglCurContext->ModelView[pglCurContext->ModelViewPos]);
      pglCurContext->ModelViewPos++;
    } else {
      GLINT_ERROR(GL_STACK_OVERFLOW);
    }
    break;
  case GL_PROJECTION:
    if(pglCurContext->ProjMatPos < 2) {
      glIntMatCopy(pglCurContext->CurProjMat,
		   pglCurContext->ProjMat[pglCurContext->ProjMatPos]);
      pglCurContext->ProjMatPos++;
    } else {
      GLINT_ERROR(GL_STACK_OVERFLOW);
    }
    break;
  case GL_TEXTURE:
    if(pglCurContext->TexMatPos < 2) {
      glIntMatCopy(pglCurContext->CurTexMat,
		   pglCurContext->TexMat[pglCurContext->TexMatPos]);
      pglCurContext->TexMatPos++;
    } else {
      GLINT_ERROR(GL_STACK_OVERFLOW);
    }
    break;
  }
}

void APIENTRY glPopMatrix (void)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    
    glIntIntToList(OP_POPMATRIX);
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    if(pglCurContext->ModelViewPos > 0) {
      pglCurContext->ModelViewPos--;
      glIntMatCopy(pglCurContext->ModelView[pglCurContext->ModelViewPos],
		   pglCurContext->CurModelView);

      // Calculate inverse transpose for normals
      pglCurContext->InvTranspDirty = TRUE;
      pglCurContext->CompositeDirty = TRUE;
    } else {
      GLINT_ERROR(GL_STACK_UNDERFLOW);
    }
    break;
  case GL_PROJECTION:
    if(pglCurContext->ProjMatPos > 0) {
      pglCurContext->ProjMatPos--;
      glIntMatCopy(pglCurContext->ProjMat[pglCurContext->ProjMatPos],
		   pglCurContext->CurProjMat);
      pglCurContext->CompositeDirty = TRUE;
    } else {
      GLINT_ERROR(GL_STACK_UNDERFLOW);
    }
    break;
  case GL_TEXTURE:
    if(pglCurContext->TexMatPos > 0) {
      pglCurContext->TexMatPos--;
      glIntMatCopy(pglCurContext->TexMat[pglCurContext->TexMatPos],
		   pglCurContext->CurTexMat);
      // Eventually Characterize?
      pglCurContext->TexMatrixIdentity = FALSE;
    } else {
      GLINT_ERROR(GL_STACK_UNDERFLOW);
    }
    break;
  }
}

void APIENTRY glRotated (GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
  glRotatef((float)angle, (float)x, (float)y, (float)z);
}

void APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
  GLfloat RotMat[16];
  GLfloat axis[3];
  GLdouble rad;
  GLfloat sine, cosine, ab, bc, ca, t;

  GLINT_OUTSIDE_BEGIN();

  if(angle == 0.0f)
    return;

  glIntIdentity(RotMat);

  axis[0] = x;
  axis[1] = y;
  axis[2] = z;
 
  glIntNormalize(axis);

  rad = angle * glDegtoRad;
  sine = (float)sin(rad);
  cosine = (float)cos(rad);
  ab = axis[0] * axis[1] * (1.0f - cosine);
  bc = axis[1] * axis[2] * (1.0f - cosine);
  ca = axis[2] * axis[0] * (1.0f - cosine);

  t = axis[0] * axis[0];
  RotMat[0] = t + cosine * (1.0f - t);
  RotMat[9] = bc - axis[0] * sine;
  RotMat[6] = bc + axis[0] * sine;

  t = axis[1] * axis[1];
  RotMat[5] = t + cosine * (1.0f - t);
  RotMat[8] = ca + axis[1] * sine;
  RotMat[2] = ca - axis[1] * sine;

  t = axis[2] * axis[2];
  RotMat[10] = t + cosine * (1.0f - t);
  RotMat[4] = ab - axis[2] * sine;
  RotMat[1] = ab + axis[2] * sine;

  if(pglCurContext->Listing) {
    int i;

    glIntIntToList(OP_MULTMATRIX);
    
    for(i=0;i<16;i++)
      glIntFloatToList(RotMat[i]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    glIntMatMult(RotMat,pglCurContext->CurModelView);
    // Calculate inverse transpose for normals
    pglCurContext->InvTranspDirty = TRUE;
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_PROJECTION:
    glIntMatMult(RotMat,pglCurContext->CurProjMat);
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_TEXTURE:
    glIntMatMult(RotMat,pglCurContext->CurTexMat);
    // Eventually Characterize?
    pglCurContext->TexMatrixIdentity = FALSE;
    break;
  }
}

void APIENTRY glScaled (GLdouble x, GLdouble y, GLdouble z)
{
  glScalef((float)x, (float)y, (float)z);
}

void APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z)
{
  GLfloat ScaleMat[16];

  GLINT_OUTSIDE_BEGIN();

  glIntIdentity(ScaleMat);
  ScaleMat[0] = x;
  ScaleMat[5] = y;
  ScaleMat[10] = z;

  if(pglCurContext->Listing) {
    int i;

    glIntIntToList(OP_MULTMATRIX);
    
    for(i=0;i<16;i++)
      glIntFloatToList(ScaleMat[i]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    glIntMatMult(ScaleMat,pglCurContext->CurModelView);
    // Calculate inverse transpose for normals
    pglCurContext->InvTranspDirty = TRUE;
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_PROJECTION:
    glIntMatMult(ScaleMat,pglCurContext->CurProjMat);
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_TEXTURE:
    glIntMatMult(ScaleMat,pglCurContext->CurTexMat);
    // Eventually Characterize?
    pglCurContext->TexMatrixIdentity = FALSE;
    break;
  }
}

void APIENTRY glTranslated (GLdouble x, GLdouble y, GLdouble z)
{
  glTranslatef((float)x, (float)y, (float)z);
}

void APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z)
{
  GLfloat TransMat[16];

  GLINT_OUTSIDE_BEGIN();

  glIntIdentity(TransMat);
  TransMat[12] = x;
  TransMat[13] = y;
  TransMat[14] = z;

  if(pglCurContext->Listing) {
    int i;

    glIntIntToList(OP_MULTMATRIX);
    
    for(i=0;i<16;i++)
      glIntFloatToList(TransMat[i]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    glIntMatMult(TransMat,pglCurContext->CurModelView);
    // Calculate inverse transpose for normals
    pglCurContext->InvTranspDirty = TRUE;
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_PROJECTION:
    glIntMatMult(TransMat,pglCurContext->CurProjMat);
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_TEXTURE:
    glIntMatMult(TransMat,pglCurContext->CurTexMat);
    // Eventually Characterize?
    pglCurContext->TexMatrixIdentity = FALSE;
    break;
  }
}

void APIENTRY glOrtho (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
  GLfloat Proj[16];

  GLINT_OUTSIDE_BEGIN();

  glIntIdentity(Proj);
  Proj[0] = (float)(2.0/(right-left));
  Proj[5] = (float)(2.0/(top-bottom));
  Proj[10] = (float)(-2.0/(zFar-zNear));
  Proj[12] = (float)-((right+left)/(right-left));
  Proj[13] = (float)-((top+bottom)/(top-bottom));
  Proj[14] = (float)-((zFar+zNear)/(zFar-zNear));

  if(pglCurContext->Listing) {
    int i;

    glIntIntToList(OP_MULTMATRIX);
    
    for(i=0;i<16;i++)
      glIntFloatToList(Proj[i]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    glIntMatMult(Proj,pglCurContext->CurModelView);
    // Calculate inverse transpose for normals
    pglCurContext->InvTranspDirty = TRUE;
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_PROJECTION:
    glIntMatMult(Proj,pglCurContext->CurProjMat);
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_TEXTURE:
    glIntMatMult(Proj,pglCurContext->CurTexMat);
    // Eventually Characterize?
    pglCurContext->TexMatrixIdentity = FALSE;
    break;
  }
}

void APIENTRY glFrustum (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
  GLfloat Proj[16];

  GLINT_OUTSIDE_BEGIN();

  glIntIdentity(Proj);
  Proj[0] = (float)((2.0*zNear)/(right-left));
  Proj[5] = (float)((2.0*zNear)/(top-bottom));
  Proj[8] = (float)((right+left)/(right-left));
  Proj[9] = (float)((top+bottom)/(top-bottom));
  Proj[10] = (float)(-(zFar+zNear)/(zFar-zNear));
  Proj[11] = -1.0f;
  Proj[14] = (float)-((2.0*(zFar*zNear))/(zFar-zNear));
  Proj[15] = 0.0f;

  if(pglCurContext->Listing) {
    int i;

    glIntIntToList(OP_MULTMATRIX);
    
    for(i=0;i<16;i++)
      glIntFloatToList(Proj[i]);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  switch(pglCurContext->MatMode) {
  case GL_MODELVIEW:
    glIntMatMult(Proj,pglCurContext->CurModelView);
    // Calculate inverse transpose for normals
    pglCurContext->InvTranspDirty = TRUE;
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_PROJECTION:
    glIntMatMult(Proj,pglCurContext->CurProjMat);
    pglCurContext->CompositeDirty = TRUE;
    break;
  case GL_TEXTURE:
    glIntMatMult(Proj,pglCurContext->CurTexMat);
    // Eventually Characterize?
    pglCurContext->TexMatrixIdentity = FALSE;
    break;
  }
}

GLfloat glIntIdentityMat[] = {
  1.0f, 0.0f, 0.0f, 0.0f,
  0.0f, 1.0f, 0.0f, 0.0f,
  0.0f, 0.0f, 1.0f, 0.0f,
  0.0f, 0.0f, 0.0f, 1.0f
};

void glIntIdentity(GLfloat *mat)
{
#ifdef RMG
  mat[0] = 1.0f;
  mat[1] = 0.0f;
  mat[2] = 0.0f;
  mat[3] = 0.0f;

  mat[4] = 0.0f;
  mat[5] = 1.0f;
  mat[6] = 0.0f;
  mat[7] = 0.0f;

  mat[8] = 0.0f;
  mat[9] = 0.0f;
  mat[10] = 1.0f;
  mat[11] = 0.0f;

  mat[12] = 0.0f;
  mat[13] = 0.0f;
  mat[14] = 0.0f;
  mat[15] = 1.0f;
#endif
  memcpy(mat,glIntIdentityMat,sizeof(GLfloat)*16);
}

void glIntTranspose(GLfloat *mat)
{
  GLfloat Transp[16];

  Transp[0] = mat[0];
  Transp[4] = mat[1];
  Transp[8] = mat[2];
  Transp[12] = mat[3];

  Transp[1] = mat[4];
  Transp[5] = mat[5];
  Transp[9] = mat[6];
  Transp[13] = mat[7];

  Transp[2] = mat[8];
  Transp[6] = mat[9];
  Transp[10] = mat[10];
  Transp[14] = mat[11];

  Transp[3] = mat[12];
  Transp[7] = mat[13];
  Transp[11] = mat[14];
  Transp[15] = mat[15];

  glIntMatCopy(Transp,mat);
}

void glIntMatCopy(GLfloat *src, GLfloat *dest)
{
#ifdef RMG
  int i;

  for(i=0;i<16;i++)
    dest[i] = src[i];
#endif
  memcpy(dest,src,sizeof(GLfloat)*16);
}

void glIntMatMult(GLfloat *new, GLfloat *old)
{
  GLfloat tmp[16];
#ifdef RMG
  int i,j,index;

  for(i=0;i<4;i++) {
    for(j=0;j<4;j++) {
      tmp[(i*4)+j] = 0.0f;
      for(index=0;index<4;index++) {
	tmp[(i*4)+j] += old[(index*4)+j]*new[index+(i*4)];
      }
    }
  }
#endif
  tmp[0] = (old[0] * new[0]) +
    (old[4] * new[1]) +
      (old[8] * new[2]) +
	(old[12] * new[3]);
  tmp[1] = (old[1] * new[0]) +
    (old[5] * new[1]) +
      (old[9] * new[2]) +
	(old[13] * new[3]);
  tmp[2] = (old[2] * new[0]) +
    (old[6] * new[1]) +
      (old[10] * new[2]) +
	(old[14] * new[3]);
  tmp[3] = (old[3] * new[0]) +
    (old[7] * new[1]) +
      (old[11] * new[2]) +
	(old[15] * new[3]);

  tmp[4] = (old[0] * new[4]) +
    (old[4] * new[5]) +
      (old[8] * new[6]) +
	(old[12] * new[7]);
  tmp[5] = (old[1] * new[4]) +
    (old[5] * new[5]) +
      (old[9] * new[6]) +
	(old[13] * new[7]);
  tmp[6] = (old[2] * new[4]) +
    (old[6] * new[5]) +
      (old[10] * new[6]) +
	(old[14] * new[7]);
  tmp[7] = (old[3] * new[4]) +
    (old[7] * new[5]) +
      (old[11] * new[6]) +
	(old[15] * new[7]);

  tmp[8] = (old[0] * new[8]) +
    (old[4] * new[9]) +
      (old[8] * new[10]) +
	(old[12] * new[11]);
  tmp[9] = (old[1] * new[8]) +
    (old[5] * new[9]) +
      (old[9] * new[10]) +
	(old[13] * new[11]);
  tmp[10] = (old[2] * new[8]) +
    (old[6] * new[9]) +
      (old[10] * new[10]) +
	(old[14] * new[11]);
  tmp[11] = (old[3] * new[8]) +
    (old[7] * new[9]) +
      (old[11] * new[10]) +
	(old[15] * new[11]);

  tmp[12] = (old[0] * new[12]) +
    (old[4] * new[13]) +
      (old[8] * new[14]) +
	(old[12] * new[15]);
  tmp[13] = (old[1] * new[12]) +
    (old[5] * new[13]) +
      (old[9] * new[14]) +
	(old[13] * new[15]);
  tmp[14] = (old[2] * new[12]) +
    (old[6] * new[13]) +
      (old[10] * new[14]) +
	(old[14] * new[15]);
  tmp[15] = (old[3] * new[12]) +
    (old[7] * new[13]) +
      (old[11] * new[14]) +
	(old[15] * new[15]);

  glIntMatCopy(tmp,old);
}

void glIntInverse(GLfloat *mat)
{
    GLint i, j, k, swap;
    GLfloat t;
    GLfloat temp[16],inverse[16];

    glIntIdentity(inverse);
    glIntMatCopy(mat, temp);

    for(i=0;i<16;i++)
      if((temp[i] < 0.000001f) &&
	 (temp[i] > -0.000001f))
	temp[i] = 0.0f;

    for (i = 0; i < 4; i++) {
	/*
	** Look for largest element in column
	*/
	swap = i;
	for (j = i + 1; j < 4; j++) {
	    if (fabs(temp[(j*4)+i]) > fabs(temp[(i*4)+i])) {
		swap = j;
	    }
	}

	if (swap != i) {
	    /*
	    ** Swap rows.
	    */
	    for (k = 0; k < 4; k++) {
		t = temp[(i*4)+k];
		temp[(i*4)+k] = temp[(swap*4)+k];
		temp[(swap*4)+k] = t;

		t = inverse[(i*4)+k];
		inverse[(i*4)+k] = inverse[(swap*4)+k];
		inverse[(swap*4)+k] = t;
	    }
	}

	if ((temp[(i*4)+i] <= 0.000001f) &&
	    (temp[(i*4)+i] >= -0.000001f)) {
	    /*
	    ** No non-zero pivot.  The matrix is singular, which typically
	    ** shouldn't happen.
	    */
	    return;
	}

	t = temp[(i*4)+i];
	for (k = 0; k < 4; k++) {
	    temp[(i*4)+k] /= t;
	    inverse[(i*4)+k] /= t;
	}
	for (j = 0; j < 4; j++) {
	    if (j != i) {
		t = temp[(j*4)+i];
		for (k = 0; k < 4; k++) {
		    temp[(j*4)+k] -= temp[(i*4)+k]*t;
		    inverse[(j*4)+k] -= inverse[(i*4)+k]*t;
		}
	    }
	}
    }

    glIntMatCopy(inverse, mat);
}

void glIntXform(GLfloat *coord, GLfloat *mat, GLfloat *rslt)
{
#ifdef RMG
  int i, index;

  for(i=0;i<4;i++) {
    rslt[i] = 0.0f;
    for(index=0;index<4;index++) {
      rslt[i] += mat[(index*4)+i]*coord[index];
    }
  }
#endif
  rslt[0] = (mat[0]*coord[0])+
    (mat[4]*coord[1])+
      (mat[8]*coord[2])+
	(mat[12]*coord[3]);
  rslt[1] = (mat[1]*coord[0])+
    (mat[5]*coord[1])+
      (mat[9]*coord[2])+
	(mat[13]*coord[3]);
  rslt[2] = (mat[2]*coord[0])+
    (mat[6]*coord[1])+
      (mat[10]*coord[2])+
	(mat[14]*coord[3]);
  rslt[3] = (mat[3]*coord[0])+
    (mat[7]*coord[1])+
      (mat[11]*coord[2])+
	(mat[15]*coord[3]);
}

void glIntXform3(GLfloat *coord, GLfloat *mat, GLfloat *rslt)
{
#ifdef RMG
  int i, index;

  for(i=0;i<3;i++) {
    rslt[i] = 0.0f;
    for(index=0;index<3;index++) {
      rslt[i] += mat[(index*4)+i]*coord[index];
    }
  }
#endif
  rslt[0] = (mat[0]*coord[0])+
    (mat[4]*coord[1])+
      (mat[8]*coord[2]);
  rslt[1] = (mat[1]*coord[0])+
    (mat[5]*coord[1])+
      (mat[9]*coord[2]);
  rslt[2] = (mat[2]*coord[0])+
    (mat[6]*coord[1])+
      (mat[10]*coord[2]);
}

void glIntValidateInvTransp(void)
{
  if(pglCurContext->InvTranspDirty) {
    pglCurContext->InvTranspDirty = FALSE;
    glIntMatCopy(pglCurContext->CurModelView,pglCurContext->CurInvTransp);
    glIntInverse(pglCurContext->CurInvTransp);
    glIntTranspose(pglCurContext->CurInvTransp);
  }
}

void glIntValidateComposite(void)
{
  if(pglCurContext->CompositeDirty) {
    glIntMatCopy(pglCurContext->CurProjMat,pglCurContext->CurComposite);
    glIntMatMult(pglCurContext->CurModelView,pglCurContext->CurComposite);
    pglCurContext->CompositeDirty = FALSE;
  }
}

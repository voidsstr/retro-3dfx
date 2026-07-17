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

void APIENTRY glRectd (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
  glBegin(GL_POLYGON);
  glVertex2d(x1,y1);
  glVertex2d(x2,y1);
  glVertex2d(x2,y2);
  glVertex2d(x1,y2);
  glEnd();
}

void APIENTRY glRectdv (const GLdouble *v1, const GLdouble *v2)
{
  glBegin(GL_POLYGON);
  glVertex2d(v1[0],v1[1]);
  glVertex2d(v2[0],v1[1]);
  glVertex2d(v2[0],v2[1]);
  glVertex2d(v1[0],v2[1]);
  glEnd();
}

void APIENTRY glRectf (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
  glBegin(GL_POLYGON);
  glVertex2f(x1,y1);
  glVertex2f(x2,y1);
  glVertex2f(x2,y2);
  glVertex2f(x1,y2);
  glEnd();
}

void APIENTRY glRectfv (const GLfloat *v1, const GLfloat *v2)
{
  glBegin(GL_POLYGON);
  glVertex2f(v1[0],v1[1]);
  glVertex2f(v2[0],v1[1]);
  glVertex2f(v2[0],v2[1]);
  glVertex2f(v1[0],v2[1]);
  glEnd();
}

void APIENTRY glRecti (GLint x1, GLint y1, GLint x2, GLint y2)
{
  glBegin(GL_POLYGON);
  glVertex2i(x1,y1);
  glVertex2i(x2,y1);
  glVertex2i(x2,y2);
  glVertex2i(x1,y2);
  glEnd();
}

void APIENTRY glRectiv (const GLint *v1, const GLint *v2)
{
  glBegin(GL_POLYGON);
  glVertex2i(v1[0],v1[1]);
  glVertex2i(v2[0],v1[1]);
  glVertex2i(v2[0],v2[1]);
  glVertex2i(v1[0],v2[1]);
  glEnd();
}

void APIENTRY glRects (GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
  glBegin(GL_POLYGON);
  glVertex2s(x1,y1);
  glVertex2s(x2,y1);
  glVertex2s(x2,y2);
  glVertex2s(x1,y2);
  glEnd();
}

void APIENTRY glRectsv (const GLshort *v1, const GLshort *v2)
{
  glBegin(GL_POLYGON);
  glVertex2s(v1[0],v1[1]);
  glVertex2s(v2[0],v1[1]);
  glVertex2s(v2[0],v2[1]);
  glVertex2s(v1[0],v2[1]);
  glEnd();
}


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



void APIENTRY glIndexd (GLdouble c)
{
  pglCurContext->CurIndex = (float)c;
}

void APIENTRY glIndexdv (const GLdouble *c)
{
  pglCurContext->CurIndex = (float)c[0];
}

void APIENTRY glIndexf (GLfloat c)
{
  pglCurContext->CurIndex = c;
}

void APIENTRY glIndexfv (const GLfloat *c)
{
  pglCurContext->CurIndex = c[0];
}

void APIENTRY glIndexi (GLint c)
{
  pglCurContext->CurIndex = (float)c;
}

void APIENTRY glIndexiv (const GLint *c)
{
  pglCurContext->CurIndex = (float)c[0];
}

void APIENTRY glIndexs (GLshort c)
{
  pglCurContext->CurIndex = (float)c;
}

void APIENTRY glIndexsv (const GLshort *c)
{
  pglCurContext->CurIndex = (float)c[0];
}

void APIENTRY glIndexub (GLubyte c)
{
  pglCurContext->CurIndex = (float)c;
}

void APIENTRY glIndexubv (const GLubyte *c)
{
  pglCurContext->CurIndex = (float)c[0];
}

void APIENTRY glIndexMask (GLuint mask)
{
}

void APIENTRY glLogicOp (GLenum opcode)
{
}


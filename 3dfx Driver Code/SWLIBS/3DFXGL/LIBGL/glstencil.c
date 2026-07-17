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

void APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask)
{
  GLINT_OUTSIDE_BEGIN();

  switch(func) {
  case GL_NEVER:
  case GL_LESS:
  case GL_EQUAL:
  case GL_LEQUAL:
  case GL_GREATER:
  case GL_NOTEQUAL:
  case GL_GEQUAL:
  case GL_ALWAYS:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_STENCILFUNC);
    glIntIntToList(func);
    glIntIntToList(ref);
    glIntIntToList(mask);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->StencilFunc = func;
  pglCurContext->StencilRef = ref;
  pglCurContext->StencilFuncMask = mask;
}

void APIENTRY glStencilMask (GLuint mask)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_STENCILMASK);
    glIntIntToList(mask);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->StencilMask = mask;
}

void APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass)
{
  GLINT_OUTSIDE_BEGIN();

  switch(fail) {
  case GL_KEEP:
  case GL_ZERO:
  case GL_REPLACE:
  case GL_INCR:
  case GL_DECR:
  case GL_INVERT:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(zfail) {
  case GL_KEEP:
  case GL_ZERO:
  case GL_REPLACE:
  case GL_INCR:
  case GL_DECR:
  case GL_INVERT:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(zpass) {
  case GL_KEEP:
  case GL_ZERO:
  case GL_REPLACE:
  case GL_INCR:
  case GL_DECR:
  case GL_INVERT:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_STENCILOP);
    glIntIntToList(zfail);
    glIntIntToList(fail);
    glIntIntToList(zpass);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->StencilZFail = zfail;
  pglCurContext->StencilFail = fail;
  pglCurContext->StencilZPass = zpass;
}




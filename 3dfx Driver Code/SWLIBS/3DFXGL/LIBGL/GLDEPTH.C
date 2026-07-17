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

void APIENTRY glDepthFunc (GLenum func)
{
  GrCmpFnc_t gDepthFunc;

  GLINT_OUTSIDE_BEGIN();

  switch(func) {
  case GL_NEVER:
    gDepthFunc = GR_CMP_NEVER;
    break;
  case GL_LESS:
    gDepthFunc = GR_CMP_LESS;
    break;
  case GL_EQUAL:
    gDepthFunc = GR_CMP_EQUAL;
    break;
  case GL_LEQUAL:
    gDepthFunc = GR_CMP_LEQUAL;
    break;
  case GL_GREATER:
    gDepthFunc = GR_CMP_GREATER;
    break;
  case GL_NOTEQUAL:
    gDepthFunc = GR_CMP_NOTEQUAL;
    break;
  case GL_GEQUAL:
    gDepthFunc = GR_CMP_GEQUAL;
    break;
  case GL_ALWAYS:
    gDepthFunc = GR_CMP_ALWAYS;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_DEPTHFUNC);
    glIntIntToList((int)gDepthFunc);
    glIntIntToList(func);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->gDepthFunc = gDepthFunc;
  pglCurContext->DepthFunc = func;
  pglCurContext->ContextDirty = TRUE;
}


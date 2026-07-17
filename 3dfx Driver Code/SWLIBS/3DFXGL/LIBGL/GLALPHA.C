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

void APIENTRY glAlphaFunc (GLenum func, GLclampf ref)
{
  GrCmpFnc_t gAlphaFunc;  
  
  GLINT_OUTSIDE_BEGIN();

  switch(func) {
  case GL_NEVER:
    gAlphaFunc = GR_CMP_NEVER;
    break;
  case GL_LESS:
    gAlphaFunc = GR_CMP_LESS;
    break;
  case GL_EQUAL:
    gAlphaFunc = GR_CMP_EQUAL;
    break;
  case GL_LEQUAL:
    gAlphaFunc = GR_CMP_LEQUAL;
    break;
  case GL_GREATER:
    gAlphaFunc = GR_CMP_GREATER;
    break;
  case GL_NOTEQUAL:
    gAlphaFunc = GR_CMP_NOTEQUAL;
    break;
  case GL_GEQUAL:
    gAlphaFunc = GR_CMP_GEQUAL;
    break;
  case GL_ALWAYS:
    gAlphaFunc = GR_CMP_ALWAYS;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_ALPHAFUNC);
    glIntIntToList((int)gAlphaFunc);
    glIntIntToList(func);
    glIntIntToList((int)(ref*255.0f));

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->gAlphaFunc = gAlphaFunc;
  pglCurContext->AlphaFunc = func;
  pglCurContext->AlphaRef = (GrAlpha_t)(ref*255.0f);
  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor)
{
  GrAlphaBlendFnc_t gSrcBlend;
  GrAlphaBlendFnc_t gDstBlend;

  GLINT_OUTSIDE_BEGIN();

  switch(sfactor) {
  case GL_ZERO:
    gSrcBlend = GR_BLEND_ZERO;
    break;
  case GL_ONE:
    gSrcBlend = GR_BLEND_ONE;
    break;
  case GL_DST_COLOR:
    gSrcBlend = GR_BLEND_DST_COLOR;
    break;
  case GL_ONE_MINUS_DST_COLOR:
    gSrcBlend = GR_BLEND_ONE_MINUS_DST_COLOR;
    break;
  case GL_SRC_ALPHA_SATURATE:
    gSrcBlend = GR_BLEND_ALPHA_SATURATE;
    break;
  case GL_SRC_ALPHA:
    gSrcBlend = GR_BLEND_SRC_ALPHA;
    break;
  case GL_ONE_MINUS_SRC_ALPHA:
    gSrcBlend = GR_BLEND_ONE_MINUS_SRC_ALPHA;
    break;
  case GL_DST_ALPHA:
    gSrcBlend = GR_BLEND_DST_ALPHA;
    break;
  case GL_ONE_MINUS_DST_ALPHA:
    gSrcBlend = GR_BLEND_ONE_MINUS_DST_ALPHA;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(dfactor) {
  case GL_ZERO:
    gDstBlend = GR_BLEND_ZERO;
    break;
  case GL_ONE:
    gDstBlend = GR_BLEND_ONE;
    break;
  case GL_SRC_COLOR:
    gDstBlend = GR_BLEND_SRC_COLOR;
    break;
  case GL_ONE_MINUS_SRC_COLOR:
    gDstBlend = GR_BLEND_ONE_MINUS_SRC_COLOR;
    break;
  case GL_SRC_ALPHA:
    gDstBlend = GR_BLEND_SRC_ALPHA;
    break;
  case GL_ONE_MINUS_SRC_ALPHA:
    gDstBlend = GR_BLEND_ONE_MINUS_SRC_ALPHA;
    break;
  case GL_DST_ALPHA:
    gDstBlend = GR_BLEND_DST_ALPHA;
    break;
  case GL_ONE_MINUS_DST_ALPHA:
    gDstBlend = GR_BLEND_ONE_MINUS_DST_ALPHA;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_BLENDFUNC);
    glIntIntToList((int)gSrcBlend);
    glIntIntToList(sfactor);
    glIntIntToList((int)gDstBlend);
    glIntIntToList(dfactor);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->gSrcBlend = gSrcBlend;
  pglCurContext->SrcBlend = sfactor;

  pglCurContext->gDstBlend = gDstBlend;
  pglCurContext->DstBlend = dfactor;

  pglCurContext->ContextDirty = TRUE;
}

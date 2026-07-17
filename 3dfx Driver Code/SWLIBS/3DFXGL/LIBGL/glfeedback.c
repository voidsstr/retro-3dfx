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

void APIENTRY glFeedbackBuffer (GLsizei size, GLenum type, GLfloat *buffer)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->RenderMode == GL_FEEDBACK) {
    GLINT_ERROR(GL_INVALID_OPERATION);
    return;
  }

  switch(type) {
  case GL_2D:
  case GL_3D:
  case GL_3D_COLOR:
  case GL_3D_COLOR_TEXTURE:
  case GL_4D_COLOR_TEXTURE:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  pglCurContext->FeedbackSize = size;
  pglCurContext->FeedbackBuffer = buffer;
  pglCurContext->FeedbackType = type;
}

void APIENTRY glPassThrough (GLfloat token)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_PASSTHRU);
    glIntFloatToList(token);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  glIntWriteFeedbackFloat(token);
}

void glIntWriteFeedback(glLocalVertex *Vtx)
{
  switch(pglCurContext->FeedbackType) {
  case GL_2D:
    glIntWriteFeedbackFloat(Vtx->Glide.x);
    glIntWriteFeedbackFloat(Vtx->Glide.y);
    break;
  case GL_3D:
    glIntWriteFeedbackFloat(Vtx->Glide.x);
    glIntWriteFeedbackFloat(Vtx->Glide.y);
    glIntWriteFeedbackFloat(Vtx->Glide.z);
    break;
  case GL_3D_COLOR:
    glIntWriteFeedbackFloat(Vtx->Glide.x);
    glIntWriteFeedbackFloat(Vtx->Glide.y);
    glIntWriteFeedbackFloat(Vtx->Glide.z);

    glIntWriteFeedbackFloat(Vtx->Glide.r/255.0f);
    glIntWriteFeedbackFloat(Vtx->Glide.g/255.0f);
    glIntWriteFeedbackFloat(Vtx->Glide.b/255.0f);
    glIntWriteFeedbackFloat(Vtx->Glide.a/255.0f);
    break;
  case GL_3D_COLOR_TEXTURE:
    glIntWriteFeedbackFloat(Vtx->Glide.x);
    glIntWriteFeedbackFloat(Vtx->Glide.y);
    glIntWriteFeedbackFloat(Vtx->Glide.z);

    glIntWriteFeedbackFloat(Vtx->Glide.r/255.0f);
    glIntWriteFeedbackFloat(Vtx->Glide.g/255.0f);
    glIntWriteFeedbackFloat(Vtx->Glide.b/255.0f);
    glIntWriteFeedbackFloat(Vtx->Glide.a/255.0f);

    glIntWriteFeedbackFloat(Vtx->Texture[0]);
    glIntWriteFeedbackFloat(Vtx->Texture[1]);
    glIntWriteFeedbackFloat(Vtx->Texture[2]);
    glIntWriteFeedbackFloat(Vtx->Texture[3]);
    break;
  case GL_4D_COLOR_TEXTURE:
    glIntWriteFeedbackFloat(Vtx->Glide.x);
    glIntWriteFeedbackFloat(Vtx->Glide.y);
    glIntWriteFeedbackFloat(Vtx->Glide.z);
    glIntWriteFeedbackFloat(Vtx->Clip[4]);

    glIntWriteFeedbackFloat(Vtx->Glide.r/255.0f);
    glIntWriteFeedbackFloat(Vtx->Glide.g/255.0f);
    glIntWriteFeedbackFloat(Vtx->Glide.b/255.0f);
    glIntWriteFeedbackFloat(Vtx->Glide.a/255.0f);

    glIntWriteFeedbackFloat(Vtx->Texture[0]);
    glIntWriteFeedbackFloat(Vtx->Texture[1]);
    glIntWriteFeedbackFloat(Vtx->Texture[2]);
    glIntWriteFeedbackFloat(Vtx->Texture[3]);
    break;
  }

  if(pglCurContext->FeedbackNumHits != -1)
    pglCurContext->FeedbackNumHits++;
}

void glIntWriteFeedbackFloat(GLfloat val)
{
  if(pglCurContext->FeedbackNumHits < 0)
    // Already Overflow
    return;
  
  if(pglCurContext->CurFeedbackSize == 0) {
    // Start of overflow
    pglCurContext->FeedbackNumHits = -1;
    return;
  }

  *(pglCurContext->CurFeedbackBuffer)++ = val;
  pglCurContext->CurFeedbackSize--;
}

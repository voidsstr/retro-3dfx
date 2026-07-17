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

void APIENTRY glSelectBuffer (GLsizei size, GLuint *buffer)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->RenderMode == GL_SELECT) {
    GLINT_ERROR(GL_INVALID_OPERATION);
    return;
  }

  pglCurContext->SelectSize = size;
  pglCurContext->SelectBuffer = buffer;
}

void APIENTRY glInitNames (void)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_INITNAMES);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if(pglCurContext->RenderMode != GL_SELECT)
    return;

  pglCurContext->SelectDepth = -1;
}

void APIENTRY glLoadName (GLuint name)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_LOADNAME);
    glIntIntToList(name);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if(pglCurContext->RenderMode != GL_SELECT)
    return;

  if(pglCurContext->SelectDepth != -1)
    pglCurContext->NameStack[pglCurContext->SelectDepth] = name;
  else
    GLINT_ERROR(GL_INVALID_OPERATION);
}

void APIENTRY glPopName (void)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_POPNAME);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if(pglCurContext->RenderMode != GL_SELECT)
    return;

  if(pglCurContext->SelectDepth != -1)
    pglCurContext->SelectDepth--;
  else
    GLINT_ERROR(GL_STACK_UNDERFLOW);
}

void APIENTRY glPushName (GLuint name)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_PUSHNAME);
    glIntIntToList(name);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if(pglCurContext->RenderMode != GL_SELECT)
    return;

  if(pglCurContext->SelectDepth < 63)
    pglCurContext->NameStack[pglCurContext->SelectDepth++] = name;
  else
    GLINT_ERROR(GL_STACK_OVERFLOW);
}

void glIntWriteSelect(GLfloat zmin, GLfloat zmax)
{
  int i;

  glIntWriteSelectInt(pglCurContext->SelectDepth+1);
  glIntWriteSelectInt((unsigned int)(zmin*65535.0*65535.0));
  glIntWriteSelectInt((unsigned int)(zmax*65535.0*65535.0));

  for(i=0;i<=pglCurContext->SelectDepth;i++)
    glIntWriteSelectInt(pglCurContext->NameStack[i]);

  if(pglCurContext->SelectNumHits != -1)
    pglCurContext->SelectNumHits++;
}

void glIntWriteSelectInt(unsigned int val)
{
  if(pglCurContext->SelectNumHits < 0)
    // Already Overflow
    return;
  
  if(pglCurContext->CurSelectSize == 0) {
    // Start of overflow
    pglCurContext->SelectNumHits = -1;
    return;
  }

  *(pglCurContext->CurSelectBuffer)++ = val;
  pglCurContext->CurSelectSize--;
}

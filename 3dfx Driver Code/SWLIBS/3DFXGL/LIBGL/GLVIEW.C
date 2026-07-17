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

void APIENTRY glDepthRange (GLclampd zNear, GLclampd zFar)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_DEPTHRANGE);
    glIntFloatToList((float)zNear);
    glIntFloatToList((float)zFar);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->zNear = (float)zNear;
  pglCurContext->zFar = (float)zFar; 
  pglCurContext->zScale = (float)((zFar - zNear)/2.0);
  pglCurContext->zOffset = (float)((zNear + zFar)/2.0);
}

void APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_VIEWPORT);
    glIntIntToList(x);
    glIntIntToList(y);
    glIntIntToList(width+1);
    glIntIntToList(height+1);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->ViewX = x;
  pglCurContext->ViewY = y;
  pglCurContext->ViewWidth = width+1;
  pglCurContext->ViewHeight = height+1;

  pglCurContext->xScale = (float)(pglCurContext->ViewWidth/2);
  pglCurContext->xOffset = (float)(pglCurContext->ViewX +
				   (pglCurContext->ViewWidth/2))+SNAP_BIAS;
  pglCurContext->yScale = (float)(pglCurContext->ViewHeight/2);
  pglCurContext->yOffset = (float)(pglCurContext->ViewY +
				   (pglCurContext->ViewHeight/2))+SNAP_BIAS;
}


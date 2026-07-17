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

void APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {
    int maxx, maxy;

    glIntIntToList(OP_SCISSOR);
    if(x < 0)
      glIntIntToList(0);
    else
      glIntIntToList(x);

    if(y < 0)
      glIntIntToList(y);
    else
      glIntIntToList(y);

    maxx = x+width;
    if(maxx > pglCurContext->WindowWidth)
      maxx = pglCurContext->WindowWidth;
    glIntIntToList(maxx);

    maxy = y+height;
    if(maxy > pglCurContext->WindowHeight)
      maxy = pglCurContext->WindowHeight;
    glIntIntToList(maxy);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if(x < 0)
    pglCurContext->minx = 0;
  else
    pglCurContext->minx = x;

  if(y < 0)
    pglCurContext->miny = 0;
  else
    pglCurContext->miny = y;
  
  pglCurContext->maxx = x+width;
  if(pglCurContext->maxx > pglCurContext->WindowWidth)
     pglCurContext->maxx = pglCurContext->WindowWidth;

  pglCurContext->maxy = y+height;
  if(pglCurContext->maxy > pglCurContext->WindowHeight)
     pglCurContext->maxy = pglCurContext->WindowHeight;

  pglCurContext->ContextDirty = TRUE;
}








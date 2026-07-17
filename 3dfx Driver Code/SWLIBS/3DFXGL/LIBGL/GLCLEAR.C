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

void APIENTRY glClear(GLbitfield mask)
{
  GLINT_OUTSIDE_BEGIN();
  
  if(pglCurContext->ContextDirty == TRUE)
    glIntValidateContext();
  
  if((mask&(GL_COLOR_BUFFER_BIT|
	    GL_DEPTH_BUFFER_BIT|
	    GL_ACCUM_BUFFER_BIT|
	    GL_STENCIL_BUFFER_BIT)) != mask)
    GLINT_ERROR(GL_INVALID_ENUM);

  if(pglCurContext->Listing) {

    glIntIntToList(OP_CLEAR);
    glIntIntToList(mask);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if(mask&GL_COLOR_BUFFER_BIT) {
    // no alpha
    grColorMask(FXTRUE,FXFALSE);
  } else {
    grColorMask(FXFALSE,FXFALSE);
  }

  if(mask&GL_DEPTH_BUFFER_BIT) {
    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
    grDepthMask(FXTRUE);
  } else {
    grDepthMask(FXFALSE);
  }

  if(mask&GL_ACCUM_BUFFER_BIT) {
    int i;
    GLfloat *fptr;

    fptr = pglCurContext->AccumBuf;
    for(i=0;i<640*480;i++) {
      *fptr++ = pglCurContext->AccumClrRed;
      *fptr++ = pglCurContext->AccumClrGreen;
      *fptr++ = pglCurContext->AccumClrBlue;
      *fptr++ = pglCurContext->AccumClrAlpha;
    }
  }

  if(mask&GL_STENCIL_BUFFER_BIT) {
  } else {
  }

  grDepthBufferFunction(GR_CMP_ALWAYS);
  grCullMode(GR_CULL_DISABLE);
  grAlphaTestFunction(GR_CMP_ALWAYS);
  grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
		       GR_BLEND_ONE, GR_BLEND_ZERO);
  
  if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
    grRenderBuffer(GR_BUFFER_FRONTBUFFER);
    grBufferClear(pglCurContext->ClrColor,
		  pglCurContext->ClrAlpha,
		  pglCurContext->ClrDepth);
    grRenderBuffer(GR_BUFFER_BACKBUFFER);
    grBufferClear(pglCurContext->ClrColor,
		  pglCurContext->ClrAlpha,
		  pglCurContext->ClrDepth);
  } else {
    grBufferClear(pglCurContext->ClrColor,
		  pglCurContext->ClrAlpha,
		  pglCurContext->ClrDepth);
  }
  
  /* restore buffer masks*/
  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_COLORMASK);
    glIntIntToList((int)red);
    glIntIntToList((int)green);
    glIntIntToList((int)blue);
    glIntIntToList((int)alpha);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->RedMask = red;
  pglCurContext->GreenMask = green;
  pglCurContext->BlueMask = blue;
  pglCurContext->AlphaMask = alpha;

  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glDepthMask (GLboolean flag)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_DEPTHMASK);
    glIntIntToList((int)flag);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->DepthMask = flag;

  pglCurContext->ContextDirty = TRUE;
}

void APIENTRY glClearAccum (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_CLEARACCUM);
    glIntFloatToList(red);
    glIntFloatToList(green);
    glIntFloatToList(blue);
    glIntFloatToList(alpha);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->AccumClrRed = red;
  pglCurContext->AccumClrGreen = green;
  pglCurContext->AccumClrBlue = blue;
  pglCurContext->AccumClrAlpha = alpha;
}

void APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_CLEARCOLOR);
    glIntFloatToList(red);
    glIntFloatToList(green);
    glIntFloatToList(blue);
    glIntFloatToList(alpha);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->ColorClrRed = red;
  pglCurContext->ColorClrGreen = green;
  pglCurContext->ColorClrBlue = blue;
  pglCurContext->ColorClrAlpha = alpha;
  pglCurContext->ClrColor = (((FxU8)(blue*255.0))<<16)|
                            (((FxU8)(green*255.0))<<8)|
                             ((FxU8)(red*255.0));
  pglCurContext->ClrAlpha = (FxU8)(alpha*255.0);
}

void APIENTRY glClearDepth (GLclampd depth)
{
  GLINT_OUTSIDE_BEGIN();

  if(pglCurContext->Listing) {

    glIntIntToList(OP_CLEARDEPTH);
    glIntFloatToList((float)depth);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->GLClrDepth = depth;
  pglCurContext->ClrDepth = (FxU16)(depth*65535.0);
  pglCurContext->ClrDepth = (FxU16)0xffff;
}

void APIENTRY glClearIndex (GLfloat c)
{
  GLINT_OUTSIDE_BEGIN();

  pglCurContext->ClrIndex = c;
}

void APIENTRY glClearStencil (GLint s)
{
  GLINT_OUTSIDE_BEGIN();

  pglCurContext->ClrStencil = s;
}

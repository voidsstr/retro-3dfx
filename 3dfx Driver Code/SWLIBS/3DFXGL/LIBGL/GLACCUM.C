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

void APIENTRY glAccum (GLenum op, GLfloat value)
{
  GrLfbInfo_t info;
  GrBuffer_t buffer;
  int x,y;
  unsigned short *bufferptr;
  GLfloat *accumptr;
  GLfloat r, g, b;
  unsigned short color;

  GLINT_OUTSIDE_BEGIN();

  switch (op) {
  case GL_ACCUM:
  case GL_LOAD:
  case GL_MULT:
  case GL_ADD:
  case GL_RETURN:
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
  }
  
  if(pglCurContext->Listing) {
    
    switch (op) {
    case GL_ACCUM:
      glIntIntToList(OP_ACCUM);
      glIntFloatToList(value);
      break;
    case GL_LOAD:
      glIntIntToList(OP_ACCUMLOAD);
      glIntFloatToList(value);
      break;
    case GL_MULT:
      glIntIntToList(OP_ACCUMMULT);
      glIntFloatToList(value);
      break;
    case GL_ADD:
      glIntIntToList(OP_ACCUMADD);
      glIntFloatToList(value);
      break;
    case GL_RETURN:
      glIntIntToList(OP_ACCUMRETURN);
      glIntFloatToList(value);
      break;
    }
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  accumptr = pglCurContext->AccumBuf;

  if(op == GL_RETURN) {
    switch(pglCurContext->DrawBuffer) {
    case GL_BACK:
    case GL_BACK_LEFT:
    case GL_BACK_RIGHT:
      buffer = GR_BUFFER_BACKBUFFER;
      break;
    default:
      buffer = GR_BUFFER_FRONTBUFFER;
      break;
    }
    
    if ( !grLfbLock( GR_LFB_WRITE_ONLY, buffer,
		    GR_TEXFMT_RGB_565, GR_ORIGIN_UPPER_LEFT, FXTRUE,
		    &info ) ) return;
    bufferptr = (unsigned short *)info.lfbPtr;
    
    for(y=0;y<480;y++)
      for(x=0;x<640;x++)
	if((x >= pglCurContext->minx) &&
	   (x < pglCurContext->maxx) &&
	   (y >= pglCurContext->miny) &&
	   (y < pglCurContext->maxy)) {
	  r = *accumptr++*32.0f;
	  g = *accumptr++*64.0f;
	  b = *accumptr++*32.0f;
	  
	  color = (((int)r)<<11)&0xf800;
	  color |= (((int)g)<<5)&0x07e0;
	  color |= ((int)b)&0x001f;
	  
	  *bufferptr++ = color;
	} else {
	  accumptr += 3;
	  bufferptr++;
	}
    
    grLfbUnlock( GR_LFB_WRITE_ONLY, buffer );
  } else {
    switch(pglCurContext->ReadBuffer) {
    case GL_BACK:
    case GL_BACK_LEFT:
    case GL_BACK_RIGHT:
      buffer = GR_BUFFER_BACKBUFFER;
      break;
    default:
      buffer = GR_BUFFER_FRONTBUFFER;
      break;
    }
    
    if ( !grLfbLock( GR_LFB_READ_ONLY, GR_BUFFER_DEPTHBUFFER,
		    GR_TEXFMT_DEPTH, GR_ORIGIN_LOWER_LEFT, FXTRUE,
		    &info ) ) return;
    bufferptr = (unsigned short *)info.lfbPtr;

    for(y=0;y<480;y++)
      for(x=0;x<640;x++)
	if((x >= pglCurContext->minx) &&
	   (x < pglCurContext->maxx) &&
	   (y >= pglCurContext->miny) &&
	   (y < pglCurContext->maxy)) {
	  switch (op) {
	  case GL_ACCUM:
	    r = ((float)(((*bufferptr)>>8)&0xf8)/32.0f);
	    g = ((float)(((*bufferptr)>>3)&0xfc)/64.0f);
	    b = ((float)(((*bufferptr++)<<3)&0xf8)/32.0f);
	    
	    *accumptr = *accumptr + (r*value);
	    accumptr++;
	    *accumptr = *accumptr + (g*value);
	    accumptr++;
	    *accumptr = *accumptr + (b*value);
	    accumptr++;
	    *accumptr = *accumptr + value;
	    accumptr++;
	    break;
	  case GL_LOAD:
	    r = ((float)(((*bufferptr)>>8)&0xf8)/32.0f);
	    g = ((float)(((*bufferptr)>>3)&0xfc)/64.0f);
	    b = ((float)(((*bufferptr++)<<3)&0xf8)/32.0f);
	    
	    *accumptr = (r*value);
	    accumptr++;
	    *accumptr = (g*value);
	    accumptr++;
	    *accumptr = (b*value);
	    accumptr++;
	    *accumptr = value;
	    accumptr++;
	    break;
	  case GL_MULT:
	    *accumptr = *accumptr * value;
	    accumptr++;
	    *accumptr = *accumptr * value;
	    accumptr++;
	    *accumptr = *accumptr * value;
	    accumptr++;
	    *accumptr = *accumptr * value;
	    accumptr++;
	    break;
	  case GL_ADD:
	    *accumptr = *accumptr + value;
	    accumptr++;
	    *accumptr = *accumptr + value;
	    accumptr++;
	    *accumptr = *accumptr + value;
	    accumptr++;
	    *accumptr = *accumptr + value;
	    accumptr++;
	    break;
	  }
	} else {
	  accumptr += 3;
	  bufferptr++;
	}

    grLfbUnlock( GR_LFB_READ_ONLY, buffer );
  }

}


void glIntInitAccum(pglContext Context)
{
  Context->AccumBuf = (GLfloat *)malloc(640*480*4);
}


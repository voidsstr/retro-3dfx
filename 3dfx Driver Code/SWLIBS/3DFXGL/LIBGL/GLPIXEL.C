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

void APIENTRY glBitmap (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
  unsigned short *startptr, *sptr;
  unsigned char *bitptr;
  unsigned char mask;
  unsigned short color;
  int x, y;
  GrLfbInfo_t info;
  GrBuffer_t buffer;
  unsigned char *srcbitptr,*dstbitptr;
  unsigned char srcmask, dstmask;
  unsigned char bytepad;


  GLINT_OUTSIDE_BEGIN();

  if((width < 0) || (height < 0)) {
    GLINT_ERROR(GL_INVALID_VALUE);
    return;
  }

  switch(pglCurContext->UnPackAlignment) {
  case 2:
    // Word Align
      if((width%16) == 0) {
	bytepad = 0;
      } else {
	bytepad = 2-((width%16)/8);
      }
    break;
  case 4:
    // DWord Align
      if((width%32) == 0) {
	bytepad = 0;
      } else {
	bytepad = 4-((width%32)/8);
      }
    break;
  case 8:
    // QWord Align
      if((width%64) == 0) {
	bytepad = 0;
      } else {
	bytepad = 8-((width%64)/8);
      }
    break;
  case 1:
  default:
    // Byte Align
      if((width%8) == 0) {
	bytepad = 0;
      } else {
	bytepad = 1;
      }
    break;
  }
  
  if(pglCurContext->Listing) {
    int size;
    unsigned int **bmpptr;

    glIntIntToList(OP_BITMAP);
    glIntIntToList(width);
    glIntIntToList(height);
    glIntFloatToList(xorig);
    glIntFloatToList(yorig);
    glIntFloatToList(xmove);
    glIntFloatToList(ymove);

    size = (width*height)/8;

    bmpptr = (unsigned int **)malloc(size+4);

    // Link to data block list and advance pointer
    *bmpptr = pglCurContext->BuildDataStart;
    pglCurContext->BuildDataStart = (unsigned int *)bmpptr;
    bmpptr++;


    srcbitptr = (unsigned char *)bitmap;
    dstbitptr = (unsigned char *)bmpptr;
    srcmask = 0x80;
    dstmask = 0x80;
    *dstbitptr = 0x00;

    for(y=0;y<height;y++) {
      for(x=0;x<width;x++) {
	if((*srcbitptr)&srcmask)
	  *dstbitptr |= dstmask;
	
	if(srcmask == 0x01) {
	  srcmask = 0x80;
	  srcbitptr++;
	} else {
	  srcmask = srcmask>>1;
	}

	if(dstmask == 0x01) {
	  dstmask = 0x80;
	  dstbitptr++;
	  *dstbitptr = 0x00;
	} else {
	  dstmask = dstmask>>1;
	}
      }
      // Unpack Align
      srcmask = 0x80;
      srcbitptr += bytepad;
    }
    glIntIntToList((int)bmpptr);

    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  if(!pglCurContext->RasValid)
    return;

  switch(pglCurContext->DrawBuffer) {
  case GL_BACK:
  case GL_BACK_LEFT:
  case GL_BACK_RIGHT:
    buffer = GR_BUFFER_BACKBUFFER;
    break;
  case GL_FRONT_AND_BACK:
    buffer = GR_BUFFER_BACKBUFFER;
    break;
  default:
    buffer = GR_BUFFER_FRONTBUFFER;
    break;
  }

  // Get raster color
  color = ((unsigned short)
	   (pglCurContext->RasVtx.Color[0]*31.0f*2048.0f))&0xf800;
  color |= ((unsigned short)
	    (pglCurContext->RasVtx.Color[1]*63.0f*32.0f))&0x07e0;
  color |= ((unsigned short)
	    (pglCurContext->RasVtx.Color[2]*31.0f))&0x001f;
  
  if(pglCurContext->DrawBuffer == GL_FRONT_AND_BACK) {
    if ( !grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_FRONTBUFFER,
		    GR_LFBWRITEMODE_565, GR_ORIGIN_LOWER_LEFT, FXTRUE,
		    &info ) ) return;
    startptr = (unsigned short *)info.lfbPtr;
    startptr += ((int)(xorig+(pglCurContext->RasVtx.Glide.x-SNAP_BIAS))) +
      (((int)(yorig+(pglCurContext->RasVtx.Glide.y-SNAP_BIAS)))*
       (info.strideInBytes/2));
    
    bitptr = (unsigned char *)bitmap;
    mask = 0x80;
    for(y=0;y<height;y++) {
      sptr = startptr;
      for(x=0;x<width;x++) {
	if(*bitptr&mask)
	  *sptr = color;
	sptr++;
	if(mask == 0x01) {
	  mask = 0x80;
	  bitptr++;
	} else {
	  mask = mask>>1;
	}
      }
      // Unpack Align
      mask = 0x80;
      bitptr += bytepad;
      startptr += info.strideInBytes/2;
    }
    grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_FRONTBUFFER );
  }
  
  if ( !grLfbLock( GR_LFB_WRITE_ONLY, buffer,
		  GR_LFBWRITEMODE_565, GR_ORIGIN_LOWER_LEFT, FXTRUE,
		  &info ) ) return;
  startptr = (unsigned short *)info.lfbPtr;
  startptr += ((int)(-xorig+(pglCurContext->RasVtx.Glide.x-SNAP_BIAS))) +
    (((int)(-yorig+(pglCurContext->RasVtx.Glide.y-SNAP_BIAS)))*
     (info.strideInBytes/2));

  bitptr = (unsigned char *)bitmap;
  mask = 0x80;
  for(y=0;y<height;y++) {
    sptr = startptr;
    for(x=0;x<width;x++) {
      if(*bitptr&mask)
	*sptr = color;
      sptr++;
      if(mask == 0x01) {
	mask = 0x80;
	bitptr++;
      } else {
	mask = mask>>1;
      }
    }
    // Byte Align
    if(mask != 0x80) {
      mask = 0x80;
      bitptr++;
    }
    startptr += info.strideInBytes/2;
  }
  grLfbUnlock( GR_LFB_WRITE_ONLY, buffer );

  pglCurContext->RasVtx.Glide.x += xmove;
  pglCurContext->RasVtx.Glide.y += ymove;
}

void APIENTRY glCopyPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
}

void APIENTRY glDrawPixels (GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *pixels)
{
  unsigned char *bptr, *start_ptr, *data_ptr;
  unsigned char *cptr;
  int x,y;
  GrLfbInfo_t info;
  GrBuffer_t buffer;

  GLINT_OUTSIDE_BEGIN();

  if((width < 0) || (height < 0)) {
    GLINT_ERROR(GL_INVALID_VALUE);
    return;
  }

  switch(format) {
  case GL_COLOR_INDEX:
  case GL_DEPTH_COMPONENT:
  case GL_RGBA:
  case GL_RED:
  case GL_GREEN:
  case GL_BLUE:
  case GL_ALPHA:
  case GL_RGB:
  case GL_LUMINANCE:
  case GL_LUMINANCE_ALPHA:
    break;
  case GL_STENCIL_INDEX:
    GLINT_ERROR(GL_INVALID_OPERATION);
    return;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(type) {
  case GL_BYTE:
  case GL_UNSIGNED_BYTE:
  case GL_SHORT:
  case GL_UNSIGNED_SHORT:
  case GL_INT:
  case GL_UNSIGNED_INT:
  case GL_FLOAT:
    break;
  case GL_BITMAP:
    if(format != GL_COLOR_INDEX) {
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {
    glIntIntToList(OP_DRAWPIXEL);
    glIntIntToList(format);
    glIntIntToList(height);
    glIntIntToList(width);

    data_ptr = (unsigned char *)malloc((width*height*2)+4);

    // Link to data block list and advance pointer
    *((unsigned int **)data_ptr) = pglCurContext->BuildDataStart;
    pglCurContext->BuildDataStart = (unsigned int *)data_ptr;
    ((unsigned int *)data_ptr)++;

    glIntIntToList((int)data_ptr);

  }

  if(format == GL_DEPTH_COMPONENT) {
    // point to start
    cptr = (unsigned char *)pixels;

    if((!pglCurContext->Listing)||
       (pglCurContext->Execute == GL_COMPILE_AND_EXECUTE)) {
      if ( !grLfbLock( GR_LFB_WRITE_ONLY, GR_BUFFER_DEPTHBUFFER,
		      GR_TEXFMT_DEPTH, GR_ORIGIN_LOWER_LEFT, FXTRUE,
		      &info ) ) return;
      start_ptr = (unsigned char *)info.lfbPtr;
      start_ptr += (int)(pglCurContext->RasVtx.Glide.x*2) +
	(int)(pglCurContext->RasVtx.Glide.y*640*2); // assumes 640*480
    }
	
    for(y=0;y<height;y++) {
      bptr = start_ptr;
      for(x=0;x<width;x++) {
	if(pglCurContext->Listing) {
	  switch(pglCurContext->Execute) {
	  case GL_COMPILE_AND_EXECUTE:
	    glIntWritePixel(&cptr,&data_ptr,format,type,GR_TEXFMT_DEPTH);
	    *((unsigned short *)bptr)++ = *(((unsigned short *)data_ptr)-1);
	    break;
	  case GL_COMPILE:
	    glIntWritePixel(&cptr,&data_ptr,format,type,GR_TEXFMT_DEPTH);
	    break;
	  }
	} else {
	  glIntWritePixel(&cptr,&bptr,format,type,GR_TEXFMT_DEPTH);
	}
      }
      start_ptr += 640*2; // assumes 640*480
    }
    if((!pglCurContext->Listing)||
       (pglCurContext->Execute == GL_COMPILE_AND_EXECUTE)) {
      grLfbUnlock( GR_LFB_WRITE_ONLY, GR_BUFFER_DEPTHBUFFER );
    }
  } else {
    cptr = (unsigned char *)pixels;

    if((!pglCurContext->Listing)||
       (pglCurContext->Execute == GL_COMPILE_AND_EXECUTE)) {
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
		      GR_LFBWRITEMODE_565, GR_ORIGIN_LOWER_LEFT, FXTRUE,
		      &info ) ) return;
      start_ptr = (unsigned char *)info.lfbPtr;
      start_ptr += (int)(pglCurContext->RasVtx.Glide.x*2) +
	(int)(pglCurContext->RasVtx.Glide.y*640*2); // assumes 640*480
    }
    
    for(y=0;y<height;y++) {
      bptr = start_ptr;
      for(x=0;x<width;x++) {
	if(pglCurContext->Listing) {
	  switch(pglCurContext->Execute) {
	  case GL_COMPILE_AND_EXECUTE:
	    glIntWritePixel(&cptr,&data_ptr,format,type,GR_LFBWRITEMODE_565);
	    *((unsigned short *)bptr)++ = *(((unsigned short *)data_ptr)-1);
	    break;
	  case GL_COMPILE:
	    glIntWritePixel(&cptr,&data_ptr,format,type,GR_LFBWRITEMODE_565);
	    break;
	  }
	} else {
	  glIntWritePixel(&cptr,&bptr,format,type,GR_LFBWRITEMODE_565);
	}
      }
      start_ptr += 640*2; // assumes 640*480
    }
    if((!pglCurContext->Listing)||
       (pglCurContext->Execute == GL_COMPILE_AND_EXECUTE)) {
      grLfbUnlock( GR_LFB_WRITE_ONLY, buffer);
    }
  }
}

void APIENTRY glPixelMapfv (GLenum map, GLsizei mapsize, const GLfloat *values)
{
  int mapindex;
  int i;

  GLINT_OUTSIDE_BEGIN();
  
  if((mapsize < 1) || (mapsize > 256)) {
    GLINT_ERROR(GL_INVALID_VALUE);
    return;
  }

  switch(map) {
  case GL_PIXEL_MAP_I_TO_I:
  case GL_PIXEL_MAP_S_TO_S:
  case GL_PIXEL_MAP_I_TO_R:
  case GL_PIXEL_MAP_I_TO_G:
  case GL_PIXEL_MAP_I_TO_B:
  case GL_PIXEL_MAP_I_TO_A:
    switch(mapsize) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
    case 64:
    case 128:
    case 256:
      break;
    default:
      GLINT_ERROR(GL_INVALID_VALUE);
      return;
    }
    break;
  default:
    break;
  }

  switch(map) {
  case GL_PIXEL_MAP_I_TO_I:
    mapindex = 0;
    break;
  case GL_PIXEL_MAP_S_TO_S:
    mapindex = 1;
    break;
  case GL_PIXEL_MAP_I_TO_R:
    mapindex = 2;
    break;
  case GL_PIXEL_MAP_I_TO_G:
    mapindex = 3;
    break;
  case GL_PIXEL_MAP_I_TO_B:
    mapindex = 4;
    break;
  case GL_PIXEL_MAP_I_TO_A:
    mapindex = 5;
    break;
  case GL_PIXEL_MAP_R_TO_R:
    mapindex = 6;
    break;
  case GL_PIXEL_MAP_G_TO_G:
    mapindex = 7;
    break;
  case GL_PIXEL_MAP_B_TO_B:
    mapindex = 8;
    break;
  case GL_PIXEL_MAP_A_TO_A:
    mapindex = 9;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_PIXELMAP);
    glIntIntToList(mapindex);
    glIntIntToList(mapsize);
    
    for(i=0;i<mapsize;i++) {
      glIntFloatToList(values[i]);
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }
  
  for(i=0;i<mapsize;i++) {
    pglCurContext->PixelMaps[mapindex][i] = values[i];
  }
  pglCurContext->PixelMapSize[mapindex] = mapsize;
}

void APIENTRY glPixelMapuiv (GLenum map, GLsizei mapsize, const GLuint *values)
{
  int mapindex;
  int i;

  GLINT_OUTSIDE_BEGIN();
  
  if((mapsize < 1) || (mapsize > 256)) {
    GLINT_ERROR(GL_INVALID_VALUE);
    return;
  }

  switch(map) {
  case GL_PIXEL_MAP_I_TO_I:
  case GL_PIXEL_MAP_S_TO_S:
  case GL_PIXEL_MAP_I_TO_R:
  case GL_PIXEL_MAP_I_TO_G:
  case GL_PIXEL_MAP_I_TO_B:
  case GL_PIXEL_MAP_I_TO_A:
    switch(mapsize) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
    case 64:
    case 128:
    case 256:
      break;
    default:
      GLINT_ERROR(GL_INVALID_VALUE);
      return;
    }
    break;
  default:
    break;
  }

  switch(map) {
  case GL_PIXEL_MAP_I_TO_I:
    mapindex = 0;
    break;
  case GL_PIXEL_MAP_S_TO_S:
    mapindex = 1;
    break;
  case GL_PIXEL_MAP_I_TO_R:
    mapindex = 2;
    break;
  case GL_PIXEL_MAP_I_TO_G:
    mapindex = 3;
    break;
  case GL_PIXEL_MAP_I_TO_B:
    mapindex = 4;
    break;
  case GL_PIXEL_MAP_I_TO_A:
    mapindex = 5;
    break;
  case GL_PIXEL_MAP_R_TO_R:
    mapindex = 6;
    break;
  case GL_PIXEL_MAP_G_TO_G:
    mapindex = 7;
    break;
  case GL_PIXEL_MAP_B_TO_B:
    mapindex = 8;
    break;
  case GL_PIXEL_MAP_A_TO_A:
    mapindex = 9;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_PIXELMAP);
    glIntIntToList(mapindex);
    glIntIntToList(mapsize);
    
    for(i=0;i<mapsize;i++) {
      glIntFloatToList(((float)values[i])/(65535.0f*65535.0f));
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }
  
  for(i=0;i<mapsize;i++) {
    pglCurContext->PixelMaps[mapindex][i] = 
      ((float)values[i])/(65535.0f*65535.0f);
  }
  pglCurContext->PixelMapSize[mapindex] = mapsize;
}

void APIENTRY glPixelMapusv (GLenum map, GLsizei mapsize, const GLushort *values)
{
  int mapindex;
  int i;

  GLINT_OUTSIDE_BEGIN();
  
  if((mapsize < 1) || (mapsize > 256)) {
    GLINT_ERROR(GL_INVALID_VALUE);
    return;
  }

  switch(map) {
  case GL_PIXEL_MAP_I_TO_I:
  case GL_PIXEL_MAP_S_TO_S:
  case GL_PIXEL_MAP_I_TO_R:
  case GL_PIXEL_MAP_I_TO_G:
  case GL_PIXEL_MAP_I_TO_B:
  case GL_PIXEL_MAP_I_TO_A:
    switch(mapsize) {
    case 1:
    case 2:
    case 4:
    case 8:
    case 16:
    case 32:
    case 64:
    case 128:
    case 256:
      break;
    default:
      GLINT_ERROR(GL_INVALID_VALUE);
      return;
    }
    break;
  default:
    break;
  }

  switch(map) {
  case GL_PIXEL_MAP_I_TO_I:
    mapindex = 0;
    break;
  case GL_PIXEL_MAP_S_TO_S:
    mapindex = 1;
    break;
  case GL_PIXEL_MAP_I_TO_R:
    mapindex = 2;
    break;
  case GL_PIXEL_MAP_I_TO_G:
    mapindex = 3;
    break;
  case GL_PIXEL_MAP_I_TO_B:
    mapindex = 4;
    break;
  case GL_PIXEL_MAP_I_TO_A:
    mapindex = 5;
    break;
  case GL_PIXEL_MAP_R_TO_R:
    mapindex = 6;
    break;
  case GL_PIXEL_MAP_G_TO_G:
    mapindex = 7;
    break;
  case GL_PIXEL_MAP_B_TO_B:
    mapindex = 8;
    break;
  case GL_PIXEL_MAP_A_TO_A:
    mapindex = 9;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(pglCurContext->Listing) {

    glIntIntToList(OP_PIXELMAP);
    glIntIntToList(mapindex);
    glIntIntToList(mapsize);
    
    for(i=0;i<mapsize;i++) {
      glIntFloatToList(((float)values[i])/65535.0f);
    }
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }
  
  for(i=0;i<mapsize;i++) {
    pglCurContext->PixelMaps[mapindex][i] = ((float)values[i])/65535.0f;
  }
  pglCurContext->PixelMapSize[mapindex] = mapsize;
}

void APIENTRY glPixelStoref (GLenum pname, GLfloat param)
{
  switch(pname) {
  case GL_PACK_SWAP_BYTES:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKSWAPBYTES);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0.0f)
      pglCurContext->PackSwapBytes = FALSE;
    else
      pglCurContext->PackSwapBytes = TRUE;
    break;
  case GL_PACK_LSB_FIRST:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKLSBFIRST);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0.0f)
      pglCurContext->PackLsbFirst = FALSE;
    else
      pglCurContext->PackLsbFirst = TRUE;
    break;
  case GL_PACK_ROW_LENGTH:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKROWLENGTH);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PackRowLength = (int)param;
    break;
  case GL_PACK_SKIP_PIXELS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKSKIPPIXELS);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PackSkipPixels = (int)param;
    break;
  case GL_PACK_SKIP_ROWS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKSKIPROWS);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PackSkipRows = (int)param;
    break;
  case GL_PACK_ALIGNMENT:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKALIGNMENT);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PackAlignment = (int)param;
    break;
    
  case GL_UNPACK_SWAP_BYTES:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKSWAPBYTES);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0.0f)
      pglCurContext->UnPackSwapBytes = FALSE;
    else
      pglCurContext->UnPackSwapBytes = TRUE;
    break;
  case GL_UNPACK_LSB_FIRST:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKLSBFIRST);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0.0f)
      pglCurContext->UnPackLsbFirst = FALSE;
    else
      pglCurContext->UnPackLsbFirst = TRUE;
    break;
  case GL_UNPACK_ROW_LENGTH:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKROWLENGTH);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->UnPackRowLength = (int)param;
    break;
  case GL_UNPACK_SKIP_PIXELS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKSKIPPIXELS);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->UnPackSkipPixels = (int)param;
    break;
  case GL_UNPACK_SKIP_ROWS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKSKIPROWS);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->UnPackSkipRows = (int)param;
    break;
  case GL_UNPACK_ALIGNMENT:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKALIGNMENT);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->UnPackAlignment = (int)param;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
}

void APIENTRY glPixelStorei (GLenum pname, GLint param)
{
  switch(pname) {
  case GL_PACK_SWAP_BYTES:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKSWAPBYTES);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0)
      pglCurContext->PackSwapBytes = FALSE;
    else
      pglCurContext->PackSwapBytes = TRUE;
    break;
  case GL_PACK_LSB_FIRST:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKLSBFIRST);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0)
      pglCurContext->PackLsbFirst = FALSE;
    else
      pglCurContext->PackLsbFirst = TRUE;
    break;
  case GL_PACK_ROW_LENGTH:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKROWLENGTH);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PackRowLength = param;
    break;
  case GL_PACK_SKIP_PIXELS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKSKIPPIXELS);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PackSkipPixels = param;
    break;
  case GL_PACK_SKIP_ROWS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKSKIPROWS);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PackSkipRows = param;
    break;
  case GL_PACK_ALIGNMENT:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_PACKALIGNMENT);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PackAlignment = param;
    break;
    
  case GL_UNPACK_SWAP_BYTES:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKSWAPBYTES);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0)
      pglCurContext->UnPackSwapBytes = FALSE;
    else
      pglCurContext->UnPackSwapBytes = TRUE;
    break;
  case GL_UNPACK_LSB_FIRST:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKLSBFIRST);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0)
      pglCurContext->UnPackLsbFirst = FALSE;
    else
      pglCurContext->UnPackLsbFirst = TRUE;
    break;
  case GL_UNPACK_ROW_LENGTH:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKROWLENGTH);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->UnPackRowLength = param;
    break;
  case GL_UNPACK_SKIP_PIXELS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKSKIPPIXELS);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->UnPackSkipPixels = param;
    break;
  case GL_UNPACK_SKIP_ROWS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKSKIPROWS);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->UnPackSkipRows = param;
    break;
  case GL_UNPACK_ALIGNMENT:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_UNPACKALIGNMENT);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->UnPackAlignment = param;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
}

void APIENTRY glPixelTransferf (GLenum pname, GLfloat param)
{
  switch(pname) {
  case GL_MAP_COLOR:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_MAPCOLOR);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0.0f)
      pglCurContext->PixelMapColor = FALSE;
    else
      pglCurContext->PixelMapColor = TRUE;
    break;
  case GL_MAP_STENCIL:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_MAPSTENCIL);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0.0f)
      pglCurContext->PixelMapStencil = FALSE;
    else
      pglCurContext->PixelMapStencil = TRUE;
    break;
  case GL_INDEX_SHIFT:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_INDEXSHIFT);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelIndexShift = (int)param;
    break;
  case GL_INDEX_OFFSET:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_INDEXOFFSET);
      glIntIntToList((int)param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelIndexOffset = (int)param;
    break;
  case GL_RED_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_REDSCALE);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelRedScale = param;
    break;
  case GL_RED_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_REDBIAS);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelRedBias = param;
    break;
  case GL_GREEN_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_GREENSCALE);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelGreenScale = param;
    break;
  case GL_GREEN_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_GREENBIAS);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelGreenBias = param;
    break;
  case GL_BLUE_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_BLUESCALE);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelBlueScale = param;
    break;
  case GL_BLUE_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_BLUEBIAS);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelBlueBias = param;
    break;
  case GL_ALPHA_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_ALPHASCALE);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelAlphaScale = param;
    break;
  case GL_ALPHA_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_ALPHABIAS);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelAlphaBias = param;
    break;
  case GL_DEPTH_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_DEPTHSCALE);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelDepthScale = param;
    break;
  case GL_DEPTH_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_DEPTHBIAS);
      glIntFloatToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelDepthBias = param;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
}

void APIENTRY glPixelTransferi (GLenum pname, GLint param)
{
  switch(pname) {
  case GL_MAP_COLOR:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_MAPCOLOR);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0)
      pglCurContext->PixelMapColor = FALSE;
    else
      pglCurContext->PixelMapColor = TRUE;
    break;
  case GL_MAP_STENCIL:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_MAPSTENCIL);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    if(param == 0)
      pglCurContext->PixelMapStencil = FALSE;
    else
      pglCurContext->PixelMapStencil = TRUE;
    break;
  case GL_INDEX_SHIFT:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_INDEXSHIFT);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelIndexShift = param;
    break;
  case GL_INDEX_OFFSET:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_INDEXOFFSET);
      glIntIntToList(param);
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelIndexOffset = param;
    break;
  case GL_RED_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_REDSCALE);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelRedScale = ((float)param)/(66535.0f*65535.0f);
    break;
  case GL_RED_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_REDBIAS);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelRedBias = ((float)param)/(66535.0f*65535.0f);
    break;
  case GL_GREEN_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_GREENSCALE);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelGreenScale = ((float)param)/(66535.0f*65535.0f);
    break;
  case GL_GREEN_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_GREENBIAS);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelGreenBias = ((float)param)/(66535.0f*65535.0f);
    break;
  case GL_BLUE_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_BLUESCALE);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelBlueScale = ((float)param)/(66535.0f*65535.0f);
    break;
  case GL_BLUE_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_BLUEBIAS);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelBlueBias = ((float)param)/(66535.0f*65535.0f);
    break;
  case GL_ALPHA_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_ALPHASCALE);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelAlphaScale = ((float)param)/(66535.0f*65535.0f);
    break;
  case GL_ALPHA_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_ALPHABIAS);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelAlphaBias = ((float)param)/(66535.0f*65535.0f);
    break;
  case GL_DEPTH_SCALE:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_DEPTHSCALE);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelDepthScale = ((float)param)/(66535.0f*65535.0f);
    break;
  case GL_DEPTH_BIAS:
    if(pglCurContext->Listing) {
      
      glIntIntToList(OP_DEPTHBIAS);
      glIntFloatToList(((float)param)/(66535.0f*65535.0f));
      
      if(pglCurContext->Execute == GL_COMPILE)
	return;
    }
    
    pglCurContext->PixelDepthBias = ((float)param)/(66535.0f*65535.0f);
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }
}

void APIENTRY glPixelZoom (GLfloat xfactor, GLfloat yfactor)
{
  if(pglCurContext->Listing) {

    glIntIntToList(OP_PIXELZOOM);
    glIntFloatToList(xfactor);
    glIntFloatToList(yfactor);
    
    if(pglCurContext->Execute == GL_COMPILE)
      return;
  }

  pglCurContext->PixelZoomX = xfactor;
  pglCurContext->PixelZoomY = yfactor;
}


void APIENTRY glDrawBuffer (GLenum mode)
{
  GLINT_OUTSIDE_BEGIN();

  switch( mode ) {
  case GL_NONE:
    // DEBUG;
    pglCurContext->DrawBuffer = mode;
    break;
  case GL_FRONT:
  case GL_FRONT_LEFT:
  case GL_FRONT_RIGHT:
    pglCurContext->DrawBuffer = mode;
    break;
  case GL_BACK:
  case GL_BACK_LEFT:
  case GL_BACK_RIGHT:
    pglCurContext->DrawBuffer = mode;
    break;
  case GL_FRONT_AND_BACK:
  case GL_LEFT:
  case GL_RIGHT:
    pglCurContext->DrawBuffer = mode;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }

  pglCurContext->ContextDirty = FALSE;
}

void APIENTRY glReadBuffer (GLenum mode)
{
  GLINT_OUTSIDE_BEGIN();

  switch( mode ) {
  case GL_NONE:
    // DEBUG;
    pglCurContext->ReadBuffer = mode;
    break;
  case GL_FRONT:
  case GL_FRONT_LEFT:
  case GL_FRONT_RIGHT:
    pglCurContext->ReadBuffer = mode;
    break;
  case GL_BACK:
  case GL_BACK_LEFT:
  case GL_BACK_RIGHT:
    pglCurContext->ReadBuffer = mode;
    break;
  case GL_FRONT_AND_BACK:
  case GL_LEFT:
  case GL_RIGHT:
    pglCurContext->ReadBuffer = mode;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    break;
  }
}

void APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
  unsigned char *bptr, *start_ptr;
  unsigned char *cptr;
  int curx,cury;
  GrLfbInfo_t info;
  GrBuffer_t buffer;

  GLINT_OUTSIDE_BEGIN();

  if((width < 0) || (height < 0)) {
    GLINT_ERROR(GL_INVALID_VALUE);
    return;
  }

  switch(format) {
  case GL_COLOR_INDEX:
  case GL_DEPTH_COMPONENT:
  case GL_RGBA:
  case GL_RED:
  case GL_GREEN:
  case GL_BLUE:
  case GL_ALPHA:
  case GL_RGB:
  case GL_LUMINANCE:
  case GL_LUMINANCE_ALPHA:
    break;
  case GL_STENCIL_INDEX:
    GLINT_ERROR(GL_INVALID_OPERATION);
    return;
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  switch(type) {
  case GL_BYTE:
  case GL_UNSIGNED_BYTE:
  case GL_SHORT:
  case GL_UNSIGNED_SHORT:
  case GL_INT:
  case GL_UNSIGNED_INT:
  case GL_FLOAT:
    break;
  case GL_BITMAP:
    if(format != GL_COLOR_INDEX) {
      GLINT_ERROR(GL_INVALID_ENUM);
      return;
    }
    break;
  default:
    GLINT_ERROR(GL_INVALID_ENUM);
    return;
    break;
  }

  if(format == GL_DEPTH_COMPONENT) {
    // point to start
    cptr = (unsigned char *)pixels;
    if ( !grLfbLock( GR_LFB_READ_ONLY, GR_BUFFER_DEPTHBUFFER,
                     GR_TEXFMT_DEPTH, GR_ORIGIN_LOWER_LEFT, FXFALSE,
                     &info ) ) return;
    start_ptr = ((unsigned char*)info.lfbPtr) + x*2 + y*info.strideInBytes;
	
    for(cury=0;cury<height;cury++) {
      bptr = start_ptr;
      for(curx=0;curx<width;curx++) {
	glIntReadPixel(&cptr,&bptr,format,type,GR_TEXFMT_DEPTH);
      }
      start_ptr += 640*2; // assumes 640*480
    }
    grLfbUnlock( GR_LFB_READ_ONLY, GR_BUFFER_DEPTHBUFFER );
  } else {
    cptr = (unsigned char *)pixels;

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
    if ( !grLfbLock( GR_LFB_READ_ONLY, buffer,
                     GR_LFBWRITEMODE_565, GR_ORIGIN_LOWER_LEFT, FXFALSE,
                     &info ) ) return;
    start_ptr = ((unsigned char*)info.lfbPtr) + x*2 + y*info.strideInBytes;
	
    for(cury=0;cury<height;cury++) {
      bptr = start_ptr;
      for(curx=0;curx<width;curx++) {
	glIntReadPixel(&cptr,&bptr,format,type,GR_LFBWRITEMODE_565);
      }
      start_ptr += 640*2; // assumes 640*480
    }
    grLfbUnlock( GR_LFB_READ_ONLY, buffer );
  }
}

void APIENTRY glGetPixelMapfv (GLenum map, GLfloat *values)
{
}

void APIENTRY glGetPixelMapuiv (GLenum map, GLuint *values)
{
}

void APIENTRY glGetPixelMapusv (GLenum map, GLushort *values)
{
}

void glIntWritePixel(unsigned char **pix_addr,unsigned char **cur_addr,
		     GLenum format,GLenum type,GrTextureFormat_t out_format)
{
  unsigned char r,g,b,a;
  unsigned short depth;

  unsigned char *cptr;
  unsigned short *sptr;
  unsigned int *iptr;
  float *fptr;

  switch(format) {
  case GL_RED:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      r = *cptr++;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      r = (unsigned char)((*sptr++)>>8);
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      r = (unsigned char)((*iptr++)>>24);
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      r = (unsigned char)((*fptr++)*255.0f);
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_GREEN:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      g = *cptr++;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      g = (unsigned char)((*sptr++)>>8);
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      g = (unsigned char)((*iptr++)>>24);
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      g = (unsigned char)((*fptr++)*255.0f);
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_BLUE:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      b = *cptr++;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      b = (unsigned char)((*sptr++)>>8);
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      b = (unsigned char)((*iptr++)>>24);
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      b = (unsigned char)((*fptr++)*255.0f);
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_ALPHA:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      a = *cptr++;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      a = (unsigned char)((*sptr++)>>8);
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      a = (unsigned char)((*iptr++)>>24);
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      a = (unsigned char)((*fptr++)*255.0f);
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_RGB:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      r = *cptr++;
      g = *cptr++;
      b = *cptr++;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      r = (unsigned char)((*sptr++)>>8);
      g = (unsigned char)((*sptr++)>>8);
      b = (unsigned char)((*sptr++)>>8);
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      r = (unsigned char)((*iptr++)>>24);
      g = (unsigned char)((*iptr++)>>24);
      b = (unsigned char)((*iptr++)>>24);
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      r = (unsigned char)((*fptr++)*255.0f);
      g = (unsigned char)((*fptr++)*255.0f);
      b = (unsigned char)((*fptr++)*255.0f);
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_RGBA:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      r = *cptr++;
      g = *cptr++;
      b = *cptr++;
      a = *cptr++;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      r = (unsigned char)((*sptr++)>>8);
      g = (unsigned char)((*sptr++)>>8);
      b = (unsigned char)((*sptr++)>>8);
      a = (unsigned char)((*sptr++)>>8);
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      r = (unsigned char)((*iptr++)>>24);
      g = (unsigned char)((*iptr++)>>24);
      b = (unsigned char)((*iptr++)>>24);
      a = (unsigned char)((*iptr++)>>24);
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      r = (unsigned char)((*fptr++)*255.0f);
      g = (unsigned char)((*fptr++)*255.0f);
      b = (unsigned char)((*fptr++)*255.0f);
      a = (unsigned char)((*fptr++)*255.0f);
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_LUMINANCE:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      r = *cptr++;
      g = r;
      b = r;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      r = (unsigned char)((*sptr++)>>8);
      g = r;
      b = r;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      r = (unsigned char)((*iptr++)>>24);
      g = r;
      b = r;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      r = (unsigned char)((*fptr++)*255.0f);
      g = r;
      b = r;
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_LUMINANCE_ALPHA:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      r = *cptr++;
      g = r;
      b = r;
      a = *cptr++;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      r = (unsigned char)((*sptr++)>>8);
      g = r;
      b = r;
      a = (unsigned char)((*sptr++)>>8);
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      r = (unsigned char)((*iptr++)>>24);
      g = r;
      b = r;
      a = (unsigned char)((*iptr++)>>24);
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      r = (unsigned char)((*fptr++)*255.0f);
      g = r;
      b = r;
      a = (unsigned char)((*fptr++)*255.0f);
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_DEPTH_COMPONENT:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      depth = (unsigned short)((*cptr++)<<8);
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      depth = *sptr++;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      depth = (unsigned short)((*iptr++)>>16);
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      depth = (unsigned short)((*fptr++)*255.0f*255.0f);
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_COLOR_INDEX:
    break;
  case GL_STENCIL_INDEX:
    break;
  default:
    break;
  }


  switch(out_format) {
  case GR_TEXFMT_ALPHA_8:
    cptr = *cur_addr;
    *cptr++ = a;
    *cur_addr = cptr;
    break;
  case GR_TEXFMT_ALPHA_INTENSITY_44:
    cptr = *cur_addr;
    *cptr = a&0xf0;
    *cptr++ |= (r>>4)&0xf;
    *cur_addr = cptr;
    break;
  case GR_TEXFMT_ALPHA_INTENSITY_88:
    cptr = *cur_addr;
    *cptr++ = a;
    *cptr++ = r;
    *cur_addr = cptr;
    break;
  case GR_TEXFMT_INTENSITY_8:
    cptr = *cur_addr;
    *cptr++ = r;
    *cur_addr = cptr;
    break;
  case GR_TEXFMT_RGB_332:
    cptr = *cur_addr;
    *cptr = r&0xe0;
    *cptr |= (g>>3)&0x1c;
    *cptr++ |= (b>>6)&0x3;
    *cur_addr = cptr;
    break;
  case GR_TEXFMT_RGB_565:
    sptr = (unsigned short *)*cur_addr;
    *sptr = (((unsigned short)r)<<8)&0xf800;
    *sptr |= (((unsigned short)g)<<3)&0x07e0;
    *sptr++ |= (((unsigned short)b)>>3)&0x001f;
    *cur_addr = (unsigned char *)sptr;
    break;
  case GR_TEXFMT_ARGB_1555:
    sptr = (unsigned short *)*cur_addr;
    *sptr = (((unsigned short)a)<<8)&0x8000;
    *sptr |= (((unsigned short)r)<<7)&0x7c00;
    *sptr |= (((unsigned short)g)<<2)&0x03e0;
    *sptr++ |= (((unsigned short)b)>>3)&0x001f;
    *cur_addr = (unsigned char *)sptr;
    break;
  case GR_TEXFMT_ARGB_4444:
    sptr = (unsigned short *)*cur_addr;
    *sptr = (((unsigned short)a)<<8)&0xf000;
    *sptr |= (((unsigned short)r)<<4)&0x0f00;
    *sptr |= ((unsigned short)g)&0x00f0;
    *sptr++ |= (((unsigned short)b)>>4)&0x000f;
    *cur_addr = (unsigned char *)sptr;
    break;
  case GR_TEXFMT_DEPTH:
    sptr = (unsigned short *)*cur_addr;
    *sptr++ = depth;
    *cur_addr = (unsigned char *)sptr;
    break;
  default:
    break;
  }
}

void glIntReadPixel(unsigned char **pix_addr,unsigned char **cur_addr,
		    GLenum format,GLenum type,GrTextureFormat_t in_format)
{
  unsigned char r,g,b,a;
  unsigned short depth;

  unsigned char *cptr;
  unsigned short *sptr;
  unsigned int *iptr;
  float *fptr;

  switch(in_format) {
  case GR_TEXFMT_RGB_565:
    sptr = (unsigned short *)*cur_addr;
    r = ((*sptr)>>8)&0xf8;
    g = ((*sptr)>>3)&0xfc;
    b = ((*sptr++)<<3)&0xf8;
    a = 0xff;
    break;
  case GR_TEXFMT_DEPTH:
    sptr = (unsigned short *)*cur_addr;
    depth = *sptr++;
    *cur_addr = (unsigned char *)sptr;
    break;
  default:
    break;
  }

  switch(format) {
  case GL_RED:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      *cptr++ = r;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      *sptr++ = r<<8;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      *iptr++ = r<<24;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      *fptr++ = ((float)r)/255.0f;
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_GREEN:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      *cptr++ = g;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      *sptr++ = g<<8;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      *iptr++ = g<<24;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      *fptr++ = ((float)g)/255.0f;
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_BLUE:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      *cptr++ = b;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      *sptr++ = b<<8;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      *iptr++ = b<<24;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      *fptr++ = ((float)b)/255.0f;
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_ALPHA:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      *cptr++ = 0xff;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      *sptr++ = 0xffff;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      *iptr++ = 0xffffffff;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      *fptr++ = 1.0f;
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_RGB:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      *cptr++ = r;
      *cptr++ = g;
      *cptr++ = b;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      *sptr++ = r<<8;
      *sptr++ = g<<8;
      *sptr++ = b<<8;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      *iptr++ = r<<24;
      *iptr++ = g<<24;
      *iptr++ = b<<24;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      *fptr++ = ((float)r)/255.0f;
      *fptr++ = ((float)g)/255.0f;
      *fptr++ = ((float)b)/255.0f;
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_RGBA:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      *cptr++ = r;
      *cptr++ = g;
      *cptr++ = b;
      *cptr++ = 0xff;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      *sptr++ = r<<8;
      *sptr++ = g<<8;
      *sptr++ = b<<8;
      *sptr++ = 0xff;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      *iptr++ = r<<24;
      *iptr++ = g<<24;
      *iptr++ = b<<24;
      *iptr++ = 0xffffffff;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      *fptr++ = ((float)r)/255.0f;
      *fptr++ = ((float)g)/255.0f;
      *fptr++ = ((float)b)/255.0f;
      *fptr++ = 1.0f;
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_LUMINANCE:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      *cptr++ = r;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      *sptr++ = r<<8;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      *iptr++ = r<<24;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      *fptr++ = ((float)r)/255.0f;
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_LUMINANCE_ALPHA:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      *cptr++ = r;
      *cptr++ = 0xff;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      *sptr++ = r<<8;
      *sptr++ = 0xffff;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      *iptr++ = r<<24;
      *iptr++ = 0xffffffff;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      *fptr++ = ((float)r)/255.0f;
      *fptr++ = 1.0f;
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_DEPTH_COMPONENT:
    switch(type) {
    case GL_BITMAP:
      break;
    case GL_UNSIGNED_BYTE:
    case GL_BYTE:
      cptr = *pix_addr;
      *cptr++ = depth>>8;
      *pix_addr = cptr;
      break;
    case GL_UNSIGNED_SHORT:
    case GL_SHORT:
      sptr = (unsigned short *)*pix_addr;
      *sptr++ = depth;
      *pix_addr = (unsigned char *)sptr;
      break;
    case GL_UNSIGNED_INT:
    case GL_INT:
      iptr = (unsigned int *)*pix_addr;
      *iptr++ = depth<<16;
      *pix_addr = (unsigned char *)iptr;
      break;
    case GL_FLOAT:
      fptr = (float *)*pix_addr;
      *fptr++ = depth/(255.0f*255.0f);
      *pix_addr = (unsigned char *)fptr;
      break;
    default:
      break;
    }
    break;
  case GL_COLOR_INDEX:
    break;
  case GL_STENCIL_INDEX:
    break;
  default:
    break;
  }
}










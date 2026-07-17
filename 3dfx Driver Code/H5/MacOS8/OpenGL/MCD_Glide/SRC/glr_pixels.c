/*________________________________________________________________________________________
** 
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
**________________________________________________________________________________________
**
**  Description: 
**
** 
**
*/

#include "glr.h"
#include "glr_drawing.h"
#include "glr_glide.h"
#include <string.h>
#include "glr_debug_structs.h"

static FxU8 bytesPerTexelMap[16] = 
{
	1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2
};


glrTexture_t * glrInternalUseTexture(
	GLDContext 				inContext,
	GLint					level,
	GLsizei					width,
	GLsizei					height,
	GLenum					base_fmt,
	GLenum					req_fmt,
	const GLTtexComp		*pixels);

void glrTranslatePixels(
	const void * 		source_pixels, 
	void * 				dest_pixels, 
	GLenum 				source_format, 
	GrTextureFormat_t 	dest_format, 
	GLsizei 			pixel_count);

void glrReloadTexture(GLDContext inContext, glrTexture_t * theTexture);

static inline unsigned long NextPowerOf2(unsigned long in)
{
	unsigned long bit;
	if(in == 0) return 0;
	for(bit = 1; bit != 0; bit <<= 1){
		if(in <= bit) return bit;
	}
	
	return 0;
}



static GrLOD_t glrCheckLOD(GLsizei size)
{
	switch (size) {
	case 1:
		return GR_LOD_LOG2_1;
	case 2:
		return GR_LOD_LOG2_2;
	case 4:
		return GR_LOD_LOG2_4;
	case 8:
		return GR_LOD_LOG2_8;
	case 16:
		return GR_LOD_LOG2_16;
	case 32:
		return GR_LOD_LOG2_32;
	case 64:
		return GR_LOD_LOG2_64;
	case 128:
		return GR_LOD_LOG2_128;
	case 256:
		return GR_LOD_LOG2_256;
	case 512:
		return GR_LOD_LOG2_512;
	case 1024:
		return GR_LOD_LOG2_1024;
	case 2048:
		return GR_LOD_LOG2_2048;
	default:
		return -1;
	}
}

#include "glr_pixels_generic.h"

#define MODE_TYPE_COMBO(mode, type) (type * 3 + mode)

#define SPECIAL_CASE(mode, type) \
	case MODE_TYPE_COMBO(mode, type): \
		_h3ReadDepthPixelsGeneric(mode, type, inX, inY, inWidth, inHeight, buffHeight, stride, depthBuffer, outPixels, out_row_bytes, depth_bias, depth_scale); \
		break


static void _h3ReadDepthPixelsGeneric2(
	GLint endian_mode,			// must be a constant
	GLint output_type,			// must be a constant

	GLint inX, 
	GLint inY, 
	GLsizei inWidth, 
	GLsizei inHeight, 
	GLsizei buffHeight,
	FxU32 stride,
	FxU8 *depthBuffer, 
	GLvoid *outPixels, 
	GLint out_row_bytes,
	GLfloat depth_bias, 
	GLfloat depth_scale)
{
	switch(MODE_TYPE_COMBO(endian_mode, output_type)){
		SPECIAL_CASE(EM_SWAP_BYTES, GL_BYTE);
		SPECIAL_CASE(EM_SWAP_BYTES, GL_UNSIGNED_BYTE);
		SPECIAL_CASE(EM_SWAP_BYTES, GL_SHORT);
		SPECIAL_CASE(EM_SWAP_BYTES, GL_UNSIGNED_SHORT);
		SPECIAL_CASE(EM_SWAP_BYTES, GL_INT);
		SPECIAL_CASE(EM_SWAP_BYTES, GL_UNSIGNED_INT);
		SPECIAL_CASE(EM_SWAP_BYTES, GL_FLOAT);
		
		SPECIAL_CASE(EM_NO_SWAP, GL_BYTE);
		SPECIAL_CASE(EM_NO_SWAP, GL_UNSIGNED_BYTE);
		SPECIAL_CASE(EM_NO_SWAP, GL_SHORT);
		SPECIAL_CASE(EM_NO_SWAP, GL_UNSIGNED_SHORT);
		SPECIAL_CASE(EM_NO_SWAP, GL_INT);
		SPECIAL_CASE(EM_NO_SWAP, GL_UNSIGNED_INT);
		SPECIAL_CASE(EM_NO_SWAP, GL_FLOAT);
		
		SPECIAL_CASE(EM_SWAP_SHORTS, GL_BYTE);
		SPECIAL_CASE(EM_SWAP_SHORTS, GL_UNSIGNED_BYTE);
		SPECIAL_CASE(EM_SWAP_SHORTS, GL_SHORT);
		SPECIAL_CASE(EM_SWAP_SHORTS, GL_UNSIGNED_SHORT);
		SPECIAL_CASE(EM_SWAP_SHORTS, GL_INT);
		SPECIAL_CASE(EM_SWAP_SHORTS, GL_UNSIGNED_INT);
		SPECIAL_CASE(EM_SWAP_SHORTS, GL_FLOAT);
	}

}

#undef MODE_TYPE_COMBO
#undef SPECIAL_CASE



#pragma mark -                       
static void _h3ReadRGBPixelsUByteLfb(GLint inX, GLint inY, 
                                       GLsizei inWidth, GLsizei inHeight, GLsizei buffHeight,
                                       FxU32 stride, /* GLboolean inPacked, */
                                       FxU8 *colorBuffer, GLvoid *outPixels, GLint out_row_bytes)
{
  FxU32 x, y;
  FxU16 rgbPixel, *rowPixels;
  FxU8 r, g, b, *outRGB;
    
  for(y = 0; y < inHeight; y++) {
  	outRGB = (FxU8 *)outPixels + y * out_row_bytes;

    rowPixels = (FxU16 *)(colorBuffer + stride * (buffHeight - y - inY - 1) + inX * sizeof(FxU16));
    for(x = 0; x < inWidth; x++) {
      rgbPixel = __lhbrx(&rowPixels[x],0);
      r = (rgbPixel >> 11) & 0x1f;
      g = (rgbPixel >> 5) & 0x3f;
      b = (rgbPixel & 0x1f);
      r = (r << 3) | (r >> 2);
      g = (g << 2) | (g >> 4);
      b = (b << 3) | (b >> 2);
      outRGB[x * 3 + 0] = r;
      outRGB[x * 3 + 1] = g;
      outRGB[x * 3 + 2] = b;
    }
  }
}                                      

static void _h3ReadRGBPixelsUByte565_16(GLint inX, GLint inY, 
                                       GLsizei inWidth, GLsizei inHeight, GLsizei buffHeight,
                                       FxU32 stride, /* GLboolean inPacked, */
                                       FxU8 *colorBuffer, GLvoid *outPixels, GLint out_row_bytes)
{
  FxU32 x, y;
  FxU16 rgbPixel, *rowPixels;
  FxU8 r, g, b, *outRGB;
    
  for(y = 0; y < inHeight; y++) {
  	outRGB = (FxU8 *)outPixels + y * out_row_bytes;

    rowPixels = (FxU16 *)(colorBuffer + stride * (buffHeight - y - inY - 1) + inX * sizeof(FxU16));
    for(x = 0; x < inWidth; x++) {
      rgbPixel = rowPixels[x];
      r = (rgbPixel >> 11) & 0x1f;
      g = (rgbPixel >> 5) & 0x3f;
      b = (rgbPixel & 0x1f);
      r = (r << 3) | (r >> 2);
      g = (g << 2) | (g >> 4);
      b = (b << 3) | (b >> 2);
      outRGB[x * 3 + 0] = r;
      outRGB[x * 3 + 1] = g;
      outRGB[x * 3 + 2] = b;
    }
  }
}                                      

static void _h3ReadRGBPixelsUByte15(GLint inX, GLint inY, 
                                       GLsizei inWidth, GLsizei inHeight, GLsizei buffHeight,
                                       FxU32 stride, /* GLboolean inPacked, */
                                       FxU8 *colorBuffer, GLvoid *outPixels, GLint out_row_bytes)
{
  FxU32 x, y;
  FxU16 rgbPixel, *rowPixels;
  FxU8 r, g, b, *outRGB;
    
  for(y = 0; y < inHeight; y++) {
  	outRGB = (FxU8 *)outPixels + y * out_row_bytes;

    rowPixels = (FxU16 *)(colorBuffer + stride * (buffHeight - y - inY - 1) + inX * sizeof(FxU16));
    for(x = 0; x < inWidth; x++) {
      rgbPixel = rowPixels[x];
      r = (rgbPixel >> 10) & 0x1f;
      g = (rgbPixel >> 5) & 0x1f;
      b = (rgbPixel & 0x1f);
      r = (r << 3) | (r >> 2);
      g = (g << 3) | (g >> 2);
      b = (b << 3) | (b >> 2);
      outRGB[x * 3 + 0] = r;
      outRGB[x * 3 + 1] = g;
      outRGB[x * 3 + 2] = b;
    }
  }
}                                      

static void _h3ReadRGBPixelsUByte565_32(GLint inX, GLint inY, 
                                      GLsizei inWidth, GLsizei inHeight, GLsizei buffHeight,
                                      FxU32 stride, /* GLboolean inPacked, */
                                      FxU8 *colorBuffer, GLvoid *outPixels, GLint out_row_bytes)
{
  FxU32 x, y;
  FxU16 rgbPixel, *rowPixels;
  FxU8 r, g, b, *outRGB;
    
  for(y = 0; y < inHeight; y++) {
  	outRGB = (FxU8 *)outPixels + y * out_row_bytes;

    rowPixels = (FxU16 *)(colorBuffer + stride * (buffHeight - y - inY - 1) + inX * sizeof(FxU16));
    for(x = 0; x < inWidth; x++) {
      rgbPixel = rowPixels[x ^ 1];
      r = (rgbPixel >> 11) & 0x1f;
      g = (rgbPixel >> 5) & 0x3f;
      b = (rgbPixel & 0x1f);
      r = (r << 3) | (r >> 2);
      g = (g << 2) | (g >> 4);
      b = (b << 3) | (b >> 2);
      outRGB[x * 3 + 0] = r;
      outRGB[x * 3 + 1] = g;
      outRGB[x * 3 + 2] = b;
    }
  }
}               


#pragma mark -                       
static void _h3ReadRGBAPixelsUByteLfb(GLint inX, GLint inY, 
                                       GLsizei inWidth, GLsizei inHeight, GLsizei buffHeight,
                                       FxU32 stride, /* GLboolean inPacked, */
                                       FxU8 *colorBuffer, GLvoid *outPixels, GLint out_row_bytes)
{
  FxU32 x, y;
  FxU16 rgbPixel, *rowPixels;
  FxU8 r, g, b, *outRGB;
    
  for(y = 0; y < inHeight; y++) {
  	outRGB = (FxU8 *)outPixels + y * out_row_bytes;

    rowPixels = (FxU16 *)(colorBuffer + stride * (buffHeight - y - inY - 1) + inX * sizeof(FxU16));
    for(x = 0; x < inWidth; x++) {
      rgbPixel = __lhbrx(&rowPixels[x],0);
      r = (rgbPixel >> 11) & 0x1f;
      g = (rgbPixel >> 5) & 0x3f;
      b = (rgbPixel & 0x1f);
      r = (r << 3) | (r >> 2);
      g = (g << 2) | (g >> 4);
      b = (b << 3) | (b >> 2);
      outRGB[x * 4 + 0] = r;
      outRGB[x * 4 + 1] = g;
      outRGB[x * 4 + 2] = b;
      outRGB[x * 4 + 3] = 0xff;
    }
  }
}                                      

static void _h3ReadRGBAPixelsUByte565_16(GLint inX, GLint inY, 
                                       GLsizei inWidth, GLsizei inHeight, GLsizei buffHeight,
                                       FxU32 stride, /* GLboolean inPacked, */
                                       FxU8 *colorBuffer, GLvoid *outPixels, GLint out_row_bytes)
{
  FxU32 x, y;
  FxU16 rgbPixel, *rowPixels;
  FxU8 r, g, b, *outRGB;
  
  for(y = 0; y < inHeight; y++) {
  	outRGB = (FxU8 *)outPixels + y * out_row_bytes;

    rowPixels = (FxU16 *)(colorBuffer + stride * (buffHeight - y - inY - 1) + inX * sizeof(FxU16));
    for(x = 0; x < inWidth; x++) {
      rgbPixel = rowPixels[x];
      r = (rgbPixel >> 11) & 0x1f;
      g = (rgbPixel >> 5) & 0x3f;
      b = (rgbPixel & 0x1f);
      r = (r << 3) | (r >> 2);
      g = (g << 2) | (g >> 4);
      b = (b << 3) | (b >> 2);
      outRGB[x * 4 + 0] = r;
      outRGB[x * 4 + 1] = g;
      outRGB[x * 4 + 2] = b;
      outRGB[x * 4 + 3] = 0xff;
    }
  }
}                                      

static void _h3ReadRGBAPixelsUByte15(GLint inX, GLint inY, 
                                       GLsizei inWidth, GLsizei inHeight, GLsizei buffHeight,
                                       FxU32 stride, /* GLboolean inPacked, */
                                       FxU8 *colorBuffer, GLvoid *outPixels, GLint out_row_bytes)
{
  FxU32 x, y;
  FxU16 rgbPixel, *rowPixels;
  FxU8 r, g, b, *outRGB;
  
  for(y = 0; y < inHeight; y++) {
  	outRGB = (FxU8 *)outPixels + y * out_row_bytes;

    rowPixels = (FxU16 *)(colorBuffer + stride * (buffHeight - y - inY - 1) + inX * sizeof(FxU16));
    for(x = 0; x < inWidth; x++) {
      rgbPixel = rowPixels[x];
      r = (rgbPixel >> 10) & 0x1f;
      g = (rgbPixel >> 5) & 0x1f;
      b = (rgbPixel & 0x1f);
      r = (r << 3) | (r >> 2);
      g = (g << 3) | (g >> 2);
      b = (b << 3) | (b >> 2);
      outRGB[x * 4 + 0] = r;
      outRGB[x * 4 + 1] = g;
      outRGB[x * 4 + 2] = b;
      outRGB[x * 4 + 3] = 0xff;
    }
  }
}                                      

static void _h3ReadRGBAPixelsUByte565_32(GLint inX, GLint inY, 
                                      GLsizei inWidth, GLsizei inHeight, GLsizei buffHeight,
                                      FxU32 stride, /* GLboolean inPacked, */
                                      FxU8 *colorBuffer, GLvoid *outPixels, GLint out_row_bytes)
{
  FxU32 x, y;
  FxU16 rgbPixel, *rowPixels;
  FxU8 r, g, b, *outRGB;
  
  for(y = 0; y < inHeight; y++) {
  	outRGB = (FxU8 *)outPixels + y * out_row_bytes;

    rowPixels = (FxU16 *)(colorBuffer + stride * (buffHeight - y - inY - 1) + inX * sizeof(FxU16));
    for(x = 0; x < inWidth; x++) {
      rgbPixel = rowPixels[x ^ 1];
      r = (rgbPixel >> 11) & 0x1f;
      g = (rgbPixel >> 5) & 0x3f;
      b = (rgbPixel & 0x1f);
      r = (r << 3) | (r >> 2);
      g = (g << 2) | (g >> 4);
      b = (b << 3) | (b >> 2);
      outRGB[x * 4 + 0] = r;
      outRGB[x * 4 + 1] = g;
      outRGB[x * 4 + 2] = b;
      outRGB[x * 4 + 3] = 0xff;
    }
  }
}                                      
                                  
#pragma mark -                       
void glrReadPixels(
	GLDContext			inContext,
	GLint				inX,
	GLint				inY,
	GLsizei				inWidth,
	GLsizei				inHeight,
	GLenum				inFormat,
	GLenum				inType,
	GLvoid *			outPixels,
	GLboolean			inPacked)
{
	GLint dest_pixel_bytes;
	GLint dest_row_bytes;
	GLint alignment;
	GLint skip_rows;
	GLint skip_pixels;
	GLint row_length;

	if(inPacked){
		alignment = 1;
		skip_rows = 0;
		skip_pixels = 0;
		row_length = inWidth;
	} else {
		alignment = inContext->state->pixel_mode.store.pack.alignment;
		skip_rows = inContext->state->pixel_mode.store.pack.skip_rows;
		skip_pixels = inContext->state->pixel_mode.store.pack.skip_pixels;
		row_length = inContext->state->pixel_mode.store.pack.row_length;
		if(row_length <= 0){
			row_length = inWidth;
		}
	}

	switch(inFormat){
	case GL_DEPTH_COMPONENT:
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
	case GL_LUMINANCE:
		dest_pixel_bytes = 1;
		break;
	case GL_LUMINANCE_ALPHA:
		dest_pixel_bytes = 2;
		break;
	case GL_RGB:
		dest_pixel_bytes = 3;
		break;
	case GL_RGBA:
		dest_pixel_bytes = 4;
		break;
	}
	
	switch(inType){
	case GL_SHORT:
	case GL_UNSIGNED_SHORT:
		dest_pixel_bytes *= 2;
		break;
	case GL_INT:
	case GL_UNSIGNED_INT:
	case GL_FLOAT:
		dest_pixel_bytes *= 4;
		break;
	}
	
	dest_row_bytes = row_length * dest_pixel_bytes;
	dest_row_bytes += (alignment - dest_row_bytes) & (alignment - 1);
	
	outPixels = (unsigned char *)outPixels + dest_row_bytes * skip_rows + dest_pixel_bytes * skip_pixels;

	if(inFormat == GL_DEPTH_COMPONENT) {
		GLint endian_mode;
		if(inContext->auxSurface) {	// windowed mode
			grFinish();
			/* Figure out desktop bit depth so we can swizzle correctly on Voodoo3. */
			if((*(*inContext->device)->gdPMap)->pixelSize == 16){
				endian_mode = EM_NO_SWAP;
			} else {
				endian_mode = EM_SWAP_SHORTS;
			}
			_h3ReadDepthPixelsGeneric2(
				endian_mode, inType,
				inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
				inContext->auxSurfaceDesc.pitch, (FxU8 *)inContext->auxSurfaceDesc.surface, outPixels, dest_row_bytes,
				inContext->state->pixel_mode.transfer.depth_bias,
				inContext->state->pixel_mode.transfer.depth_scale);             
		} else {	// fullscreen mode
			GrLfbInfo_t lfbInfo;
			lfbInfo.size = sizeof(lfbInfo);
			if(grLfbLock(GR_LFB_READ_ONLY, GR_BUFFER_AUXBUFFER, GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE, &lfbInfo)) {
				if(gls_surface_ext && !inContext->glideExtensions.gls_pix_ext) {
					endian_mode = EM_SWAP_BYTES;
				} else {
					endian_mode = EM_NO_SWAP;
				}
				_h3ReadDepthPixelsGeneric2(
					endian_mode, inType,
					inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
					lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes, 
					inContext->state->pixel_mode.transfer.depth_bias,
					inContext->state->pixel_mode.transfer.depth_scale);
				grLfbUnlock(GR_LFB_READ_ONLY, GR_BUFFER_AUXBUFFER);
			}
		}
    } else if(inFormat == GL_RGB && inType == GL_UNSIGNED_BYTE) {
      if(inContext->renderSurface) {	// windowed mode
        grFinish();
        
        if(gls_surface_ext && inContext->glideExtensions.gls_pix_ext) { // voodoo 4/5 +
             _h3ReadRGBPixelsUByte15(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
                inContext->renderSurfaceDesc.pitch, (FxU8 *)inContext->renderSurfaceDesc.surface, outPixels, dest_row_bytes);
        } else { // voodoo 3
	        /* Figure out desktop bit depth so we can swizzle correctly on Voodoo3. */
	        switch((*(*inContext->device)->gdPMap)->pixelSize) {
	          case 16:
	           _h3ReadRGBPixelsUByte565_16(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
	             inContext->renderSurfaceDesc.pitch, (FxU8 *)inContext->renderSurfaceDesc.surface, outPixels, dest_row_bytes);
	           break;
	          case 32:
	           _h3ReadRGBPixelsUByte565_32(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
	             inContext->renderSurfaceDesc.pitch, (FxU8 *)inContext->renderSurfaceDesc.surface, outPixels, dest_row_bytes);
	           break;
	         }
	    } 
      } else {							// full screen mode
        GrLfbInfo_t lfbInfo;
        lfbInfo.size = sizeof(lfbInfo);
        if(grLfbLock(GR_LFB_READ_ONLY, inContext->readBuffer, GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
          &lfbInfo)) {
            switch(lfbInfo.writeMode) {
            case GR_LFBWRITEMODE_565:
            if(gls_surface_ext  && !inContext->glideExtensions.gls_pix_ext) {
	          _h3ReadRGBPixelsUByteLfb(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
	              lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes);
	        } else {
	          _h3ReadRGBPixelsUByte565_16(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
	              lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes);
	        }
	        break;
	        
            /* This should only happen on Napalm */
            case GR_LFBWRITEMODE_1555:
            if(gls_surface_ext && inContext->glideExtensions.gls_pix_ext) {
              _h3ReadRGBPixelsUByte15(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
                lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes);
            }
            break;
	        }
            grLfbUnlock(GR_LFB_READ_ONLY, inContext->readBuffer);
        }
      }
    } else if(inFormat == GL_RGBA && inType == GL_UNSIGNED_BYTE) {
      if(inContext->renderSurface) {	// windowed mode
        grFinish();
        if(gls_surface_ext && inContext->glideExtensions.gls_pix_ext) { // voodoo 4/5 +
             _h3ReadRGBAPixelsUByte15(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
                inContext->renderSurfaceDesc.pitch, (FxU8 *)inContext->renderSurfaceDesc.surface, outPixels, dest_row_bytes);
        } else { // voodoo 3
	        /* Figure out desktop bit depth so we can swizzle correctly on Voodoo3. */
	        switch((*(*inContext->device)->gdPMap)->pixelSize) {
	          case 16:
	           _h3ReadRGBAPixelsUByte565_16(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
	             inContext->renderSurfaceDesc.pitch, (FxU8 *)inContext->renderSurfaceDesc.surface, outPixels, dest_row_bytes);
	           break;
	          case 32:
	           _h3ReadRGBAPixelsUByte565_32(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
	             inContext->renderSurfaceDesc.pitch, (FxU8 *)inContext->renderSurfaceDesc.surface, outPixels, dest_row_bytes);
	           break;
	         }
	    }    
      } else {	// full screen mode
        GrLfbInfo_t lfbInfo;
        lfbInfo.size = sizeof(lfbInfo);
        if(grLfbLock(GR_LFB_READ_ONLY, inContext->readBuffer, GR_LFBWRITEMODE_ANY, GR_ORIGIN_UPPER_LEFT, FXFALSE,
          &lfbInfo)) {
            switch(lfbInfo.writeMode) {
            case GR_LFBWRITEMODE_565:
            if(gls_surface_ext && !inContext->glideExtensions.gls_pix_ext) {
              _h3ReadRGBAPixelsUByteLfb(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
                lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes);
            } else {
              _h3ReadRGBAPixelsUByte565_16(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
                lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes);
            }
            break;
            
            /* This should only happen on Napalm */
            case GR_LFBWRITEMODE_1555:
            if(gls_surface_ext && inContext->glideExtensions.gls_pix_ext) {
              _h3ReadRGBAPixelsUByte15(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
                lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes);
            }
            break;
#if 0            
            case GR_LFBWRITEMODE_8888:
            if(gls_surface_ext && inContext->glideExtensions.gls_pix_ext) {
              _h3ReadRGBAPixelsUByte15(inX,inY,inWidth,inHeight,inContext->viewPort.size.h,
                lfbInfo.strideInBytes, (FxU8 *)lfbInfo.lfbPtr, outPixels, dest_row_bytes);
            }
            break;
#endif            
            }
            grLfbUnlock(GR_LFB_READ_ONLY, inContext->readBuffer);
        }
      }
    }

}


/*
________________________________________________________________________________________

      glrDrawPixels
________________________________________________________________________________________

*/

void glrDrawPixels(
	GLDContext			inContext,
	const GLDVertex *	inPos,
	GLsizei				inWidth,
	GLsizei				inHeight,
	GLenum				inFormat,
	GLenum				inType,
	const GLvoid *		inPixels,
	GLboolean			inPacked)
{
	unsigned long theColor;
	FxI32 i;
	GLint bytes_per_pixel;
	GLint source_row_bytes;
	
	GLint grid_x, grid_y;
	GLint height, width;
	GLint tex_height, tex_width;

	
	GLint alignment;
	GLint skip_rows;
	GLint skip_pixels;
	GLint row_length;
	
	GLfloat zoom_x, zoom_y;


	FxI32 scale;
	GrLOD_t lod;
	GrVertex verts[4];
	
	char * source_row_base;
	glrTexture_t * theTexture;

	DEBUG_ENTRY( glrDrawPixels );
	
	DEBUG_PRINTF(
		"height:%d  width:%d  "
		"x:%f  y:%f  z:%f  \n",
		inHeight,
		inWidth,
		inPos->window.x,
		inPos->window.y,
		inPos->window.z
		);
	
	DEBUG_PRINTF(inPacked ? "packed true\n" : "packed false\n");
	
	DEBUG_PRINTF("format:%s\n", glr_debug_enum_to_string(inFormat));
	DEBUG_PRINTF("data type:%s\n", glr_debug_enum_to_string(inType));
	
	if(inWidth == 0 || inHeight == 0 || inPixels == NULL){
		return;
	}
	
	switch(inFormat){
	case GL_RGB:
		bytes_per_pixel = 3;
		break;
	case GL_RGBA:
		bytes_per_pixel = 4;
		break;
	case GL_RED:
	case GL_GREEN:
	case GL_BLUE:
	case GL_ALPHA:
	case GL_LUMINANCE:
		bytes_per_pixel = 1;
		break;
	case GL_LUMINANCE_ALPHA:
		bytes_per_pixel = 2;
		break;
	default:
		DEBUG_NOT_YET_IMPLEMENTED("glrDrawPixels unsuported format");
		return;
		break;
	}
	
	if(inPacked){
		alignment = 1;
		skip_rows = 0;
		skip_pixels = 0;
		row_length = inWidth;
	} else {
		alignment = inContext->state->pixel_mode.store.unpack.alignment;
		skip_rows = inContext->state->pixel_mode.store.unpack.skip_rows;
		skip_pixels = inContext->state->pixel_mode.store.unpack.skip_pixels;
		row_length = inContext->state->pixel_mode.store.unpack.row_length;
		if(row_length <= 0){
			row_length = inWidth;
		}
	}
	
	zoom_x = inContext->state->pixel_mode.zoom_x;
	zoom_y = inContext->state->pixel_mode.zoom_y;
	
	source_row_bytes = row_length * bytes_per_pixel;
	source_row_bytes += (alignment - source_row_bytes) & (alignment - 1);
		
	if(inType != GL_UNSIGNED_BYTE){
		DEBUG_NOT_YET_IMPLEMENTED("glrDrawPixels unsuported type");
		return;
	}

	theColor = packARGB(1.0, inPos->color.r, inPos->color.g, inPos->color.b);
	theColor = 0xffffffff;

	
	grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_POINT_SAMPLED, GR_TEXTUREFILTER_POINT_SAMPLED);
	grTexMipMapMode(GR_TMU0, GR_MIPMAP_DISABLE, GR_TEXTUREFILTER_POINT_SAMPLED);
	grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
	grTexCombine(GR_TMU0,
		GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		FXFALSE,FXFALSE);

	inContext->combineUnitState = 0;
	grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		GR_COMBINE_FACTOR_ONE,
		GR_COMBINE_LOCAL_NONE,
		GR_COMBINE_OTHER_TEXTURE,
		FXFALSE);				      

	grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		GR_COMBINE_FACTOR_ONE,
		GR_COMBINE_LOCAL_NONE,
		GR_COMBINE_OTHER_TEXTURE,
		FXFALSE);
		
	grAlphaTestReferenceValue(0);
	grAlphaTestFunction(GR_CMP_NOTEQUAL);

	grCullMode(GR_CULL_DISABLE);

	grVertexLayout(GR_PARAM_XY,  0, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Z,   8, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q,   12, GR_PARAM_DISABLE); // Q param is only used for fog
	grVertexLayout(GR_PARAM_PARGB,16,GR_PARAM_ENABLE);

	grVertexLayout(GR_PARAM_RGB, 20, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_A,   32, GR_PARAM_DISABLE);

	grVertexLayout(GR_PARAM_ST0, 40, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_Q0,  48, GR_PARAM_ENABLE);
	grVertexLayout(GR_PARAM_ST1, 52, GR_PARAM_DISABLE);
	grVertexLayout(GR_PARAM_Q1,  60, GR_PARAM_DISABLE);
		
	for(grid_y = 0; grid_y < inHeight; grid_y += 256){
		char * dest_row_base;
		GLint dest_row_bytes;
	
		height = inHeight - grid_y;
		if(height > 256){
			height = 256;
		}
		for(grid_x = 0; grid_x < inWidth; grid_x += 256){
			width = inWidth - grid_x;
			if(width > 256){
				width = 256;
			}
			
			tex_width = NextPowerOf2(width);
			tex_height = NextPowerOf2(height);
			
			if(tex_width > tex_height){
				if(tex_height < (tex_width / 8)){
					tex_height = (tex_width / 8);
				}
				scale = 256.0f / (float)tex_width;
				lod = glrCheckLOD(tex_width);
			} else {
				if(tex_width < (tex_height / 8)){
					tex_width = (tex_height / 8);
				}
				scale = 256.0f / (float)tex_height;	
				lod = glrCheckLOD(tex_height);
			}
			
			theTexture = glrInternalUseTexture(
				inContext,
				0,			// level
				tex_width,	// width
				tex_height,	// height
				inFormat,	// base_fmt
				inFormat, 	// req_fmt
				NULL);		// pixel buffer

			dest_row_bytes = tex_width * bytesPerTexelMap[theTexture->sys_texture.format];
			
			source_row_base = (char *)inPixels + (skip_rows + grid_y) * source_row_bytes + (grid_x + skip_pixels) * bytes_per_pixel;
			dest_row_base = theTexture->sys_data[lod];
			
			for(i = 0; i < height; i++){
				glrTranslatePixels(
					source_row_base, 
					dest_row_base, 
					inFormat, 
					theTexture->sys_texture.format, 
					width);
					
				source_row_base += source_row_bytes;
				dest_row_base += dest_row_bytes;
			}
			
			// future optimization
			// partial download could be faster
			glrReloadTexture(inContext, theTexture);

			// draw the quad

			verts[0].pargb = theColor;
			verts[0].oow = inPos->fog;
			
			verts[0].x = inPos->window.x + (float)grid_x * zoom_x;
			verts[0].y = inPos->window.y - (float)grid_y * zoom_y;
			verts[0].z = inPos->window.z;
			verts[0].tmuvtx[0].sow = 0;
			verts[0].tmuvtx[0].tow = 0;
			verts[0].tmuvtx[0].oow = 1.0;
		
			verts[3] = verts[2] = verts[1] = verts[0];
			
			verts[1].x += (float)width * zoom_x;
			verts[2].x += (float)width * zoom_x;
		
			verts[2].y -= (float)height * zoom_y;
			verts[3].y -= (float)height * zoom_y;
		
			verts[1].tmuvtx[0].sow = width * scale;
			verts[2].tmuvtx[0].sow = width * scale;
		
			verts[2].tmuvtx[0].tow = height * scale;
			verts[3].tmuvtx[0].tow = height * scale;
		
			grDrawTriangle(verts, verts + 1, verts + 2);
			grDrawTriangle(verts, verts + 2, verts + 3);
		}	
	}
	
	glrUpdateHardwareState(inContext, GLD_STATE_ALL);

}


/*
________________________________________________________________________________________

      glrCopyPixels
________________________________________________________________________________________

*/

void glrCopyPixels(
	GLDContext			inContext,
	const GLDVertex *	inPos,
	GLint				inX,
	GLint				inY,
	GLsizei				inWidth,
	GLsizei				inHeight,
	GLenum				inType)
{
	DEBUG_NOT_YET_IMPLEMENTED( glrCopyPixels );
}
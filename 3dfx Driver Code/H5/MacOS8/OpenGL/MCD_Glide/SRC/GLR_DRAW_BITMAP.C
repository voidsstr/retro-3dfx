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
#include <string.h>
#include <math.h>


void glrReloadTexture(GLDContext inContext, glrTexture_t * theTexture);
glrTexture_t * glrInternalUseTexture(
	GLDContext inContext,
	GLint                     level,
	GLsizei                   width,
	GLsizei                   height,
	GLenum                    base_fmt,
	GLenum                    req_fmt,
	const GLTtexComp         *pixels);


static inline unsigned long NextPowerOf2(unsigned long in)
{
	unsigned long bit;
	if(in == 0) return 0;
	for(bit = 1; bit != 0; bit <<= 1){
		if(in <= bit) return bit;
	}
	
	return 0;
}

/*
________________________________________________________________________________________

      glrRenderBitmap
________________________________________________________________________________________

*/

void glrRenderBitmap(
	GLDContext			inContext,
	 const GLDVertex *	inPos,
	 GLsizei			inWidth,
	 GLsizei			inHeight,
	 GLfloat			inXorig,
	 GLfloat			inYorig,
	 GLuint				inName,
	 const GLubyte *	inBitmap,
	 GLboolean			inPacked)
{
	static unsigned char pixelBuffer[256*256];
	unsigned long theColor;
	FxI32 i, j;
	const GLubyte *	currentByte;

	FxI32 texHeight;
	FxI32 texWidth;
	FxI32 scale;
	GrVertex verts[4];
	glrTexture_t * theTexture;
	
	if(inWidth == 0 || inHeight == 0 || inBitmap == NULL){
		return;
	}
	
	texWidth = NextPowerOf2(inWidth);
	if(texWidth < 8){
		texWidth = 8;
	}
	
	texHeight = NextPowerOf2(inHeight);
		
	if(texWidth > 256 || texHeight > 256) {
		// to do: handle bigger bitmaps
		return;
	}

	if(texWidth > texHeight){
		if(texHeight < (texWidth / 8)){
			texHeight = (texWidth / 8);
		}
		scale = 256 / texWidth;
	} else {
		if(texWidth < (texHeight / 8)){
			texWidth = (texHeight / 8);
		}
		scale = 256 / texHeight;	
	}

//	pixelBuffer = glmMalloc(texWidth * texHeight);
	
	if(pixelBuffer == NULL){
		return;
	}
	
	theColor = packARGB(1.0, inPos->color.r, inPos->color.g, inPos->color.b);

  /* Packing seems to be meaningless for bitmaps, which are always packed into unsigned bytes */
	if(1 || inPacked) {
		unsigned long * currentPixel;
		unsigned long byte;
		FxI32 source_row_bytes = (inWidth + 7) / 8;
		static unsigned long lut[16] = {
			0x00000000, 0x000000ff, 0x0000ff00, 0x0000ffff, 
			0x00ff0000, 0x00ff00ff, 0x00ffff00, 0x00ffffff, 
			0xff000000, 0xff0000ff, 0xff00ff00, 0xff00ffff, 
			0xffff0000, 0xffff00ff, 0xffffff00, 0xffffffff
			};
		
		currentByte = inBitmap;
		for(i = 0; i < inHeight; i++){
			currentPixel = (unsigned long *)(pixelBuffer + i * texWidth);
			for(j = 0; j < source_row_bytes; j++){
				byte = *currentByte++;
				*currentPixel++ = lut[byte >> 4];
				*currentPixel++ = lut[byte & 0xf];
			}
		}
	} else {
	
	}
	

	theTexture = glrInternalUseTexture(
		inContext,
		0,			// level
		texWidth,	// width
		texHeight,	// height
		GL_ALPHA,	// base_fmt
		GL_ALPHA, 	// req_fmt
		(const GLTtexComp *)pixelBuffer);
		
	glrReloadTexture(inContext, theTexture);
	
	grTexFilterMode(GR_TMU0, GR_TEXTUREFILTER_POINT_SAMPLED, GR_TEXTUREFILTER_POINT_SAMPLED);
	grTexMipMapMode(GR_TMU0, GR_MIPMAP_DISABLE, GR_TEXTUREFILTER_POINT_SAMPLED);
	grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
	grTexCombine(GR_TMU0,
		GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		FXFALSE,FXFALSE);

#if 0
	grTexFilterMode(GR_TMU1, GR_TEXTUREFILTER_POINT_SAMPLED, GR_TEXTUREFILTER_POINT_SAMPLED);
	grTexMipMapMode(GR_TMU1, GR_MIPMAP_DISABLE, GR_TEXTUREFILTER_POINT_SAMPLED);
	grTexClampMode(GR_TMU1, GR_TEXTURECLAMP_CLAMP, GR_TEXTURECLAMP_CLAMP);
	grTexCombine(GR_TMU1,
		GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		GR_COMBINE_FUNCTION_LOCAL,GR_COMBINE_FACTOR_NONE,
		FXFALSE,FXFALSE);
#endif

	inContext->combineUnitState = 0;
	grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER,
		GR_COMBINE_FACTOR_TEXTURE_ALPHA,
		GR_COMBINE_LOCAL_NONE,
		GR_COMBINE_OTHER_ITERATED,
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

	//grDisableAllEffects();
	//grDitherMode(GR_DITHER_DISABLE);
		
	verts[0].pargb = theColor;
	verts[0].oow = inPos->fog;
	
	/* Floor the x and y values so they always fall on pixel boundaries.  This
	   fixes some bitmap rendering artifacts. */
	verts[0].x = floor(inPos->window.x - inXorig + 0.5f);
	verts[0].y = floor(inPos->window.y + inYorig + 0.5f);
	verts[0].z = inPos->window.z;
	verts[0].tmuvtx[0].sow = 0.0;
	verts[0].tmuvtx[0].tow = 0.0;
	verts[0].tmuvtx[0].oow = 1.0;

	verts[3] = verts[2] = verts[1] = verts[0];
	
	verts[1].x += inWidth;
	verts[2].x += inWidth;

	verts[2].y -= inHeight;
	verts[3].y -= inHeight;

	verts[1].tmuvtx[0].sow = inWidth * scale;
	verts[2].tmuvtx[0].sow = inWidth * scale;

	verts[2].tmuvtx[0].tow = inHeight * scale;
	verts[3].tmuvtx[0].tow = inHeight * scale;
    
    grDrawTriangle(verts, verts + 1, verts + 2);
	grDrawTriangle(verts, verts + 2, verts + 3);
	
	//grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
	inContext->context_reinit = 1;
	//glrUpdateHardwareState(inContext, GLD_STATE_ALL);

//	glmFree(pixelBuffer);
}

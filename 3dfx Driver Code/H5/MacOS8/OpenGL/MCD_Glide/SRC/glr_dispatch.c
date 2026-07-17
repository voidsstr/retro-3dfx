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
**  Description: Context dispatch table management methods for the Apple OpenGL plugin
**
** 
**
*/


#include "glr.h"

/* local method declaration: */

static GLboolean	glrGetDispatchTable(
							GLDContext			inContext,
							GLDRenderDispatch *	outTable,
							GLbitfield			ioFlags);


static void			glrNotImplemented( void );

static GrContext_t currentGlideContext;

/*
________________________________________________________________________________________

      gldGetRenderDispatch
________________________________________________________________________________________

fix me: we don't want to return GLD_RENDER_UPDATE all the time!

*/

GLenum
gldGetRenderDispatch(
	GLDContext			inContext,
	GLDRenderDispatch *	outTable,
	GLDViewport *		outViewChange,
	GLbitfield			inStateChange)
{
	GLenum				theResult = GLI_NO_ERROR;

	DEBUG_ENTRY( gldGetRenderDispatch );

	if( !inContext ) {
		DEBUG_ERROR( gldGetRenderDispatch, "Invalid context pointer : 0x%08x\n", inContext);
		return GLI_BAD_POINTER;
	}
	
	if( !outTable ) {
		DEBUG_ERROR( gldGetRenderDispatch, "Invalid dispatch table pointer : 0x%08x\n", outTable);
		return GLI_BAD_POINTER;
	}
	
	/* Well, this isn't perfect but it lets us be at least as good as ATI. */
	if(currentGlideContext != inContext->glideContext) {
	  if(inContext->glideContext) {
	    grSelectContext(inContext->glideContext);
	  }
	  currentGlideContext = 0;
	}
	
	/* Save a copy of the dispatch table in case we have no nop it out later if we lose
	   our rendering surfaces. */
	inContext->dispatch_table = outTable;
	
	if( !inContext->glideContext ) {
		GLuint *		theFuncPtr;
		GLint			i;

		/* clear all the dispatch table (by filling it with glrNotImplemented() methods ) */
		theFuncPtr = (GLuint *) outTable;
		i = (sizeof(GLDRenderDispatch) / sizeof(void*) ) - 1;
		while ( i >= 0 ) {
			theFuncPtr[i--] = (GLuint) glrNotImplemented;
		}
		
		/* we are done here, let's go */

	} else {
	
		if( inStateChange & GLD_STATE_DRAWABLE ) {
			inStateChange |= GLD_STATE_ALL;
		}
		
		if ( glrGetDispatchTable( inContext, outTable, inStateChange ) ){
			DEBUG_VERBOSE( gldGetRenderDispatch, "the dispatch table has been updated\n");
		}

	}
	
	if(inContext->viewPortDirty){
		*outViewChange = inContext->viewPort;
		DEBUG_VERBOSE( gldGetRenderDispatch, "viewport: w = %d, h = %d, x = %d, y = %d, dead_w = %d, dead_h = %d\n", outViewChange->size.w, outViewChange->size.h, outViewChange->offset.x, outViewChange->offset.y, outViewChange->deadband.w, outViewChange->deadband.h );
		inContext->viewPortDirty = GL_FALSE;
		theResult = GLD_RENDER_UPDATE;
	}

	if ( theResult != 0 ) {
		DEBUG_VERBOSE( gldGetRenderDispatch, "changes have been recorded (0x%08x)...\n", theResult );
	}
	
	return theResult;
}


/*
________________________________________________________________________________________

      gldGetSystemDispatch
________________________________________________________________________________________

*/

GLenum
gldGetSystemDispatch(
	GLISystemDispatch *	outTable)
{
	DEBUG_ENTRY( gldGetSystemDispatch );

	// check if the table is large enough
	
	if( !outTable || outTable->sys_count < ((sizeof(GLISystemDispatch) >> 2) - 1) ) {
		return GLI_BAD_VALUE;
	}

	return GLI_NO_ERROR;
}



/*
________________________________________________________________________________________

      glrGetDispatchTable
________________________________________________________________________________________


- a lot of functions should only be set once.
- the others should only be changed when we have the corresponding state change.

*/

static GLboolean
glrGetDispatchTable(
	GLDContext			inContext,
	GLDRenderDispatch *	outTable,
	GLbitfield			inStateChange)
{
	GLboolean			theResult = GL_FALSE;
	
	DEBUG_ENTRY( glrGetDispatchTable );
	
	glrUpdateHardwareState( inContext, inStateChange );
	
	if( inStateChange & GLD_STATE_RENDER_START ) {

		inStateChange |= GLD_STATE_SHADE_MODEL;
		
		outTable->gldAccum = gldAccum;
		outTable->gldClear = gldClear;
		
		outTable->gldReadPixels = glrReadPixels;
		outTable->gldDrawPixels = glrDrawPixels;
		outTable->gldCopyPixels = glrCopyPixels;
		
		outTable->gldRenderBitmap = glrRenderBitmap;
		
		outTable->gldRenderPoints = glrRenderPoints;
		outTable->gldRenderPointsPtr = glrRenderPointsPtr;

		outTable->gldRenderVertexBuffer = glrRenderVertexBuffer;
		outTable->gldEndPrimitiveBuffer = glrEndPrimitiveBuffer;
	
		outTable->gldFinish = gldFinish;
		outTable->gldFlush = gldFlush;
		outTable->gldSwapBuffers = gldSwapBuffers;
		
		theResult = GL_TRUE;		
	}
	
	if(inStateChange & (GLD_STATE_SHADE_MODEL|GLD_STATE_TEXTURE_MODE|GLD_STATE_POLYGON_MODE) ) {
	    FxU32 numTextures = 0;
	    if(inContext->hw_texture[0]) numTextures = 1;
	    if(inContext->hw_texture[1]) numTextures = 2;
	    
		if( inContext->state->shade_model == GL_SMOOTH ) {				
			if( inContext->hw_texture[1] ) {
				outTable->gldRenderLines = glrRenderSmoothLines;
				outTable->gldRenderLineStrip = glrRenderSmoothLineStrip;
				outTable->gldRenderLineLoop = glrRenderSmoothLineLoop;			
				outTable->gldRenderLinesPtr = glrRenderSmoothLinesPtr;
				outTable->gldRenderTriangles = glrRenderSmoothMultiTextureTriangles;
				outTable->gldRenderTriangleFan = glrRenderSmoothMultiTextureTriangleFan;
				outTable->gldRenderTriangleStrip = glrRenderSmoothMultiTextureTriangleStrip;
				outTable->gldRenderQuads = glrRenderSmoothMultiTextureQuads;
				outTable->gldRenderQuadStrip = glrRenderSmoothMultiTextureQuadStrip;
				outTable->gldRenderPolygon = glrRenderSmoothMultiTexturePolygon;				
				outTable->gldRenderPolygonPtr = glrRenderSmoothMultiTexturePolygonPtr;			
			} else if( inContext->hw_texture[0] ) {
				outTable->gldRenderLines = glrRenderSmoothLines;
				outTable->gldRenderLineStrip = glrRenderSmoothLineStrip;
				outTable->gldRenderLineLoop = glrRenderSmoothLineLoop;			
				outTable->gldRenderLinesPtr = glrRenderSmoothLinesPtr;	
				outTable->gldRenderTriangles = glrRenderSmoothTextureTriangles;
				outTable->gldRenderTriangleFan = glrRenderSmoothTextureTriangleFan;
				outTable->gldRenderTriangleStrip = glrRenderSmoothTextureTriangleStrip;
				outTable->gldRenderQuads = glrRenderSmoothTextureQuads;
				outTable->gldRenderQuadStrip = glrRenderSmoothTextureQuadStrip;
				outTable->gldRenderPolygon = glrRenderSmoothTexturePolygon;				
				outTable->gldRenderPolygonPtr = glrRenderSmoothTexturePolygonPtr;	
			} else {
				outTable->gldRenderLines = glrRenderSmoothLines;
				outTable->gldRenderLineStrip = glrRenderSmoothLineStrip;
				outTable->gldRenderLineLoop = glrRenderSmoothLineLoop;			
				outTable->gldRenderLinesPtr = glrRenderSmoothLinesPtr;	
				outTable->gldRenderTriangles = glrRenderSmoothTriangles;
				outTable->gldRenderTriangleFan = glrRenderSmoothTriangleFan;
				outTable->gldRenderTriangleStrip = glrRenderSmoothTriangleStrip;
				outTable->gldRenderQuads = glrRenderSmoothQuads;
				outTable->gldRenderQuadStrip = glrRenderSmoothQuadStrip;
				outTable->gldRenderPolygon = glrRenderSmoothPolygon;				
				outTable->gldRenderPolygonPtr = glrRenderSmoothPolygonPtr;
			}
		} else {				
			if(inContext->hw_texture[1]) {
				outTable->gldRenderLines = glrRenderFlatLines;
				outTable->gldRenderLineStrip = glrRenderFlatLineStrip;
				outTable->gldRenderLineLoop = glrRenderFlatLineLoop;				
				outTable->gldRenderLinesPtr = glrRenderFlatLinesPtr;
				outTable->gldRenderTriangles = glrRenderFlatMultiTextureTriangles;
				outTable->gldRenderTriangleFan = glrRenderFlatMultiTextureTriangleFan;
				outTable->gldRenderTriangleStrip = glrRenderFlatMultiTextureTriangleStrip;
				outTable->gldRenderQuads = glrRenderFlatMultiTextureQuads;
				outTable->gldRenderQuadStrip = glrRenderFlatMultiTextureQuadStrip;
				outTable->gldRenderPolygon = glrRenderFlatMultiTexturePolygon;				
				outTable->gldRenderPolygonPtr = glrRenderFlatMultiTexturePolygonPtr;		
			} else if(inContext->hw_texture[0]) {
				outTable->gldRenderLines = glrRenderFlatLines;
				outTable->gldRenderLineStrip = glrRenderFlatLineStrip;
				outTable->gldRenderLineLoop = glrRenderFlatLineLoop;				
				outTable->gldRenderLinesPtr = glrRenderFlatLinesPtr;	
				outTable->gldRenderTriangles = glrRenderFlatTextureTriangles;
				outTable->gldRenderTriangleFan = glrRenderFlatTextureTriangleFan;
				outTable->gldRenderTriangleStrip = glrRenderFlatTextureTriangleStrip;
				outTable->gldRenderQuads = glrRenderFlatTextureQuads;
				outTable->gldRenderQuadStrip = glrRenderFlatTextureQuadStrip;
				outTable->gldRenderPolygon = glrRenderFlatTexturePolygon;				
				outTable->gldRenderPolygonPtr = glrRenderFlatTexturePolygonPtr;
			} else {	
				outTable->gldRenderLines = glrRenderFlatLines;
				outTable->gldRenderLineStrip = glrRenderFlatLineStrip;
				outTable->gldRenderLineLoop = glrRenderFlatLineLoop;				
				outTable->gldRenderLinesPtr = glrRenderFlatLinesPtr;
				outTable->gldRenderTriangles = glrRenderFlatTriangles;
				outTable->gldRenderTriangleFan = glrRenderFlatTriangleFan;
				outTable->gldRenderTriangleStrip = glrRenderFlatTriangleStrip;
				outTable->gldRenderQuads = glrRenderFlatQuads;
				outTable->gldRenderQuadStrip = glrRenderFlatQuadStrip;
				outTable->gldRenderPolygon = glrRenderFlatPolygon;				
				outTable->gldRenderPolygonPtr = glrRenderFlatPolygonPtr;
			}
		}		
		theResult = GL_TRUE;		
	}
	return theResult;
}




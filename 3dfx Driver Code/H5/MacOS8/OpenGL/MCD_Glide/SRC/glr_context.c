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
**  Description: Context creation and destruction methods for the Apple OpenGL plugin
**
** 
**
*/


#include <stdlib.h>
#include <string.h>

#include "glr.h"
#include "glr_glide.h"

#include "glt_fixed_coord.h"

#include "hrm_3d_envvars.h"



/* local method declaration: */

static void			glrSetConfigData(
							GLDConfig *				outConfig,
							const GLIPixelFormat *	inFormat);

static void			glrFetchEnvVars();

GLDContext __gCurrentContext = NULL;

/* For reference: */

#include <gestalt.h>

static unsigned long cpuType(void)
{
  OSErr err;
  long processorAttributes;
  Boolean	hasAltiVec = false;

  err	= Gestalt(gestaltPowerPCProcessorFeatures, &processorAttributes);

  if(err == noErr) { 
    hasAltiVec = (1L << gestaltPowerPCHasVectorInstructions) & processorAttributes;
  }  
  return hasAltiVec ? 1 : 0;
}

/* */

/*
________________________________________________________________________________________

      gldCreateContext
________________________________________________________________________________________

*/

GLenum
gldCreateContext(
	GLDContext *			outContext,
	const GLIPixelFormat *	inFormat,
	GLDConfig *				outConfig,
	const GLDState *		inState)
{
	GLint					i,j;
	GLDContext				theContext;
	
	DEBUG_ENTRY( gldCreateContext );

	if( !outContext ) {
		DEBUG_ERROR( gldCreateContext, "Invalid context pointer : 0x%08x\n", outContext);
		return GLI_BAD_POINTER;
	} else {
		*outContext = NULL;
	}
	
	glrFetchEnvVars();
	
#if PROFILE
	{
		OSErr err;
		
		err = ProfilerInit(collectDetailed, bestTimeBase, 1000, 100);
		if(err){
			Debugger();
		}
		ProfilerSetStatus(false);
	}
#endif
	
#if 0	
	if( __gCurrentContext ) {
		DEBUG_NOT_YET_IMPLEMENTED( gldCreateContext );
		DEBUG_ERROR( gldCreateContext, "Only one context can be open\n");
		return GLI_BAD_CONTEXT;
	}
#endif	

	if( !glrValidatePixelFormat(inFormat) ) {
		DEBUG_ERROR( gldCreateContext, "invalid context format\n" );
		return GLI_BAD_PIXELFMT;
	}
	
	if( !glrValidateEnvironment() ) {
		DEBUG_ERROR( gldCreateContext, "no hardware could be found\n" );
		return GLI_BAD_CONTEXT;
	}
	
	theContext = (GLDContext) glmMalloc(sizeof(struct GLDContextRec));
	if( !theContext ) {
		DEBUG_ERROR( gldCreateContext, "couldn't allocate memory for the new context\n" );
		return GLI_BAD_ALLOC;
	}
	
	/* initialize the new context */
	
	memset(theContext, 0, sizeof * theContext);
#if 0	
	__gCurrentContext  = theContext;
#endif	
	theContext->state  = inState;

	/* drawable data initialization */
	theContext->drawableType = 0;
	
	theContext->viewPort.size.w = 0;
	theContext->viewPort.size.h = 0;
	theContext->viewPort.offset.x = 0;
	theContext->viewPort.offset.y = 0;
	theContext->viewPort.deadband.w = 0;
	theContext->viewPort.deadband.h = 0;
	
	theContext->viewPortDirty = GL_TRUE;
	
	theContext->targetPort = 0;
	theContext->renderPort = 0;
	theContext->renderSurface = 0;
	theContext->auxSurface = 0;
	theContext->glideContext = 0;
	theContext->device = inFormat->devices[0];
	theContext->glideBoardSelectID = 0;
    theContext->cpuType = cpuType();

	theContext->buffer_rect.x                      = 0;
	theContext->buffer_rect.y                      = 0;
	theContext->buffer_rect.w                      = 0;
	theContext->buffer_rect.h                      = 0;
	theContext->buffer_rect_enabled                = GL_FALSE;
	
	theContext->swap_rect.x                        = 0;
	theContext->swap_rect.y                        = 0;
	theContext->swap_rect.w                        = 0;
	theContext->swap_rect.h                        = 0;
	theContext->swap_rect_enabled                  = GL_FALSE;

	theContext->drawableType                       = GLI_NONE;
	
	theContext->primitive                          = 0;

	// textures		
 	for(j = 0; j < GLR_TEXTURE_HASH_SIZE; j++)
	{
		theContext->textures[j] = NULL;
	}
	
	theContext->cur_texture_name[0][0] = 0;
	theContext->cur_texture_name[0][1] = 0;
	theContext->cur_texture_name[1][0] = 0;
	theContext->cur_texture_name[1][1] = 0;
	
	theContext->hw_texture[0]  = NULL;
	theContext->hw_texture[1]  = NULL;
	
	theContext->surface_size[0] = 0;
	theContext->surface_size[1] = 0;
	
	// theContext->current_texture_unit  = 0;
	
	// Default z offset 
	theContext->fill_z_offset   = 0.0;
	theContext->context_reinit  = GL_TRUE;
	theContext->error = GL_NO_ERROR;
	theContext->has_depth = inFormat->depth_mode != GLI_0_BIT;

	glrSetConfigData( outConfig, inFormat);
	
	theContext->gliPixelFormat = *inFormat;

	for ( i = 0 ; i < GLR_VA_TYPE_LAST ; i++ )
	{
	  theContext->va[i].current_va_index = 0;
	  for ( j = 0 ; j < GLR_NUM_VA_BUFFERS ; j++ )
	  {
	    theContext->va[i].dataSize[j] = 0;
	    theContext->va[i].data[j] = 0;
	  }
	}
	
	// Configure hardware dependant function pointers
	glrSetupHardwareFuncs(theContext);

	// we are done! everything went fine.
	
	*outContext = theContext;

	DEBUG_PRINT_GLDSTATE( gldCreateContext, (GLDState*) inState );
	DEBUG_PRINT_GLDCONFIG( gldCreateContext, outConfig );
	
	return GLI_NO_ERROR;
}




/*
________________________________________________________________________________________

      gldDestroyContext
________________________________________________________________________________________


Fix Me: closing of glide

*/

GLenum
gldDestroyContext(
	GLDContext			inContext)
{

	DEBUG_ENTRY( gldDestroyContext );

	if( !inContext ) {
		DEBUG_ERROR( gldDestroyContext, "Invalid context pointer : 0x%08x\n", inContext);
		return GLI_BAD_POINTER;
	}
	
	// shut down the hardware if running
/*	
	if( inContext->glide_initialized ) {
		gmSstWinClose();
		gmGlideShutdown();
	}
*/	
    if(inContext->drawableType) {
      gldAttachDrawable(inContext, GLI_NONE, 0);
    }
    
	__gCurrentContext = NULL;
	
	if(inContext->indexBuffer)
		glmFree(inContext->indexBuffer);
	
	// a little more cleanup
	glrFreeAllTextures( inContext);
	glmFree( (char *) inContext );
	
#if PROFILE
	{
		//ProfilerSetStatus(false);
		//ProfilerDump("\p3dfx OpenGL Profiler Data");
		ProfilerTerm();
	}
#endif

	
	return GLI_NO_ERROR;
}



/*
________________________________________________________________________________________

      glrSetConfigData
________________________________________________________________________________________

*/

static void
glrSetConfigData(
	GLDConfig *				outConfig,
	const GLIPixelFormat *	inFormat)
{
	GLRColorSizes theSizes;

	/* we can specify to invert the coordinates for the view port:
		GLD_VIEWPORT_INVERT_X
		GLD_VIEWPORT_INVERT_Y
	*/
	outConfig->view_flags = GLD_VIEWPORT_INVERT_Y;

	/* Depth scale*/
	outConfig->depth_scale    = 65535UL;

	/* Subpixel accuracy */
	outConfig->subpixel_bits  = GLT_FIXED_COORD_SUBPIXEL_BITS;

	/* Drawable size limits */
	outConfig->drawable_max.w = 2048;
	outConfig->drawable_max.h = 2048;

	/* Buffer configuration */
	outConfig->pixel_format.rgba_mode     = ( inFormat->color_mode  & GLI_COLOR_RGB_BITS )     != 0;
	outConfig->pixel_format.double_buffer = ( inFormat->buffer_mode & GLI_DOUBLEBUFFER_BIT )   != 0;
	outConfig->pixel_format.stereo_mode   = GL_FALSE;

	glrColorSizes( inFormat->color_mode, &theSizes );
	outConfig->pixel_format.red_size         = theSizes.red_size;
	outConfig->pixel_format.green_size       = theSizes.green_size;
	outConfig->pixel_format.blue_size        = theSizes.blue_size;
	outConfig->pixel_format.alpha_size       = theSizes.alpha_size;

	outConfig->pixel_format.buffer_size      = theSizes.buffer_size;

	glrColorSizes( inFormat->accum_mode, &theSizes );
	outConfig->pixel_format.accum_red_size   = theSizes.red_size;
	outConfig->pixel_format.accum_green_size = theSizes.green_size;
	outConfig->pixel_format.accum_blue_size  = theSizes.blue_size;
	outConfig->pixel_format.accum_alpha_size = theSizes.alpha_size;

	outConfig->pixel_format.depth_size    = glrDepthSize( inFormat->depth_mode );
	outConfig->pixel_format.stencil_size  = glrDepthSize( inFormat->stencil_mode );
	outConfig->pixel_format.aux_buffers   = inFormat->aux_buffers;

	outConfig->vertex_type.coord_color          = GLD_VA_TYPE_V4F_C4UB;
	outConfig->vertex_type.coord_color_tex      = GLD_VA_TYPE_V4F_C4UB_T2F;
	outConfig->vertex_type.coord_color_2tex     = GLD_VA_TYPE_V4F_C4UB_T2F_T2F_OWF;
	outConfig->vertex_type.coord_color_fog      = GLD_VA_TYPE_V4F_C4UB_SF4UB;
	outConfig->vertex_type.coord_color_fog_tex  = GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F;
	outConfig->vertex_type.coord_color_fog_2tex = GLD_VA_TYPE_V4F_C4UB_SF4UB_T2F_T2F_OWF;
	
	outConfig->vertex_max_count = GLR_VERTEX_MAX_COUNT;
    outConfig->vertex_options   = (GLD_VA_OPTION_MEM_CACHED |GLD_VA_OPTION_CACHE_COHERENT );

	/* Anti-Aliasing Point size and line width limits */
	outConfig->point_size.min         =  0.10;
	outConfig->point_size.max         = 50.00;
	outConfig->point_size.granularity =  0.10;
	outConfig->line_width.min         =  0.10;
	outConfig->line_width.max         = 10.00;
	outConfig->line_width.granularity =  0.10;

	/* Optional rendering features */
	outConfig->features.does_culling = GL_TRUE;
}

/*
________________________________________________________________________________________

      glrValidatePixelFormat
________________________________________________________________________________________

This function is called by gldChoosePixelFormat() and gldCreateContext()
*/

GLboolean
glrValidatePixelFormat(
	const GLIPixelFormat *	inFormat)
{
	GLbitfield				theMask;
	
	DEBUG_ENTRY( glrValidatePixelFormat );
	
	if( !inFormat ) {
		DEBUG_ERROR( glrValidatePixelFormat, "Invalid GLIPixelFormat pointer : 0x%08x\n", inFormat);
		return GL_FALSE;
	}
	
	if( (inFormat->renderer_id & GLI_DRIVER_ID_MASK) != GLR_DRIVER_ID ){
		DEBUG_ERROR( glrValidatePixelFormat, "this pixel format was not produced by this renderer\n");
		return GL_FALSE;
	}
	
	if( inFormat->buffer_level < GLR_MIN_LEVEL || inFormat->buffer_level > GLR_MAX_LEVEL ){
		DEBUG_ERROR( glrValidatePixelFormat, "buffer_level is out of range : %d\n", inFormat->buffer_level);
		return GL_FALSE;
	}
	
	if( inFormat->buffer_mode & GLI_STEREOSCOPIC_BIT ){
		DEBUG_ERROR( glrValidatePixelFormat, "Stereoscopic rendering is not supported\n" );
		return GL_FALSE;
	}
	
	if( inFormat->color_mode & GLI_COLOR_INDEX_BITS ){
		DEBUG_ERROR( glrValidatePixelFormat, "indexed pixel format is not supported\n" );
		return GL_FALSE;
	}
#if 0	
	if( inFormat->aux_buffers != 0 ){
		DEBUG_ERROR( glrValidatePixelFormat, "invalid number of auxiliary buffers : %d\n", inFormat->aux_buffers );
		return GL_FALSE;
	}
#endif	
	if( inFormat->stencil_mode & ~GLI_0_BIT ){
		DEBUG_ERROR( glrValidatePixelFormat, "stencil buffer is not supported\n" );
		return GL_FALSE;
	}

	theMask = GLI_RGB555_BIT | GLI_RGB565_BIT | GLI_RGB888_BIT | GLI_ARGB1555_BIT | GLI_ARGB8888_BIT;
	if( inFormat->color_mode & ~theMask ) {
		DEBUG_ERROR( glrValidatePixelFormat, "Invalid color mode : 0x%08x\n", inFormat->color_mode);
		return GL_FALSE;
	}
			
	theMask = GLI_0_BIT | GLI_16_BIT | GLI_24_BIT | GLI_32_BIT;		
	if( inFormat->depth_mode & ~theMask ) {
		DEBUG_ERROR( glrValidatePixelFormat, "Invalid depth mode : 0x%08x\n", inFormat->depth_mode);
		return GL_FALSE;
	}
	
	if( inFormat->accum_mode ){
		DEBUG_ERROR( glrValidatePixelFormat, "accumulation buffer is not supported\n" );
		return GL_FALSE;
	}
	
	return GL_TRUE;
}



/*
________________________________________________________________________________________

      glrFetchEnvVars
________________________________________________________________________________________

*/

void
glrFetchEnvVars()
{
	char * theEnvVarsStream;
	
	hrm_GetOpenGLEnvVars( 0, &theEnvVarsStream );
	
}
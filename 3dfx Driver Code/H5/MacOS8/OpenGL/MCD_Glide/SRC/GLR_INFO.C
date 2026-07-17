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
**
** 
**
*/


#include "glr.h"
#include "glr_glide.h"
#include <CodeFragments.h>
#include <string.h>

#define kMaxNumberOfTarget 8

static void			glrDefaultPixelFormat(
							GLIPixelFormat *	outFormat);





/*
________________________________________________________________________________________

      gldGetRendererInfo
________________________________________________________________________________________

*/

GLenum
gldGetRendererInfo(
	GLIRendererInfo *	outInfo,
	const GLIDevice *	inDevice,
	GLint				inNumberOfDevs)
{
	GLenum				err;
	
	DEBUG_ENTRY( gldGetRendererInfo );

	if(inNumberOfDevs > 1){
		// we only render to one device at a time
		return GLI_BAD_MATCH;
	}

    
	/* Get the hardware info */
	if( !glrValidateEnvironment() ) {
		return GLI_BAD_POINTER;
	};
	
	
        
	/* fill in the answers */
	outInfo->renderer_id            = GLR_DRIVER_ID;

	outInfo->os_support             = GLI_WINDOW_BIT
									| GLI_FULLSCREEN_BIT
//									| GLI_BACKING_STORE_BIT
//									| GLI_MP_SAFE_BIT
//									| GLI_AUTO_UPDATE_BIT
									| GLI_ROBUST_BIT
									| GLI_RECOVERABLE_BIT
									| GLI_ACCELERATED_BIT
//									| GLI_MULTISCREEN_BIT
									| GLI_COMPLIANT_BIT;

	outInfo->buffer_modes           = GLI_SINGLEBUFFER_BIT
									| GLI_DOUBLEBUFFER_BIT
//									| GLI_STEREOSCOPIC_BIT
									| GLI_MONOSCOPIC_BIT;
								   
	outInfo->min_buffer_level       = GLR_MIN_LEVEL;
	outInfo->max_buffer_level       = GLR_MAX_LEVEL;

	outInfo->color_modes            = GLI_RGB555_BIT
									| GLI_ARGB1555_BIT
									| GLI_RGB565_BIT
									| GLI_RGB888_BIT
									| GLI_ARGB8888_BIT;
									
	outInfo->depth_modes            = GLI_0_BIT
									| GLI_16_BIT 
									| GLI_24_BIT;
										
	outInfo->accum_modes            = GLI_NONE;
	
	outInfo->stencil_modes          = GLI_0_BIT
	                                | GLI_8_BIT;

	outInfo->max_aux_buffers        = 0; /* fix me */
	
	outInfo->video_memory           = 8 * 1024 * 1024; /* fix me */
	outInfo->texture_memory         = 8 * 1024 * 1024; /* fix me */

	outInfo->next_renderer_info     = 0;
	
	/* check out the GLIDevices */
	
	DEBUG_PRINTF("Number of devices %d\n", inNumberOfDevs);
	
	if(inNumberOfDevs == 1 && glrDeviceToBoardID(inDevice[0]) < 0){
		// app is trying to match the renderer to a particular GDevice
		err = GLI_BAD_MATCH;
	} else {
		// app is NOT trying to match the renderer to a particular GDevice
		err = GLI_NO_ERROR;
	}


	DEBUG_PRINT_RENDERER_INFO( gldGetRendererInfo, outInfo );
	
	return err;
}


/*
________________________________________________________________________________________

      gldChoosePixelFormat
________________________________________________________________________________________


*/

GLenum
gldChoosePixelFormat(
	GLIPixelFormat **	outPixelFormat,
	const GLIDevice *	inDeviceList,
	GLint				inNumberOfDevs,
	const GLint *		inAttributeList)
{
	GLIPixelFormat		theTempPixFormat;
	GLIPixelFormat *    theNewPixFormat;
	GLIDevice *         theDeviceList;
	const GLint *		theAttributeList;
	GLint				theColorSize = 0;
	GLint				theAccumSize = 0;
	GLboolean           theRGBA = GL_FALSE;
	GLboolean           theDoubleBuffer = GL_FALSE;
	GLint				i;
	
	DEBUG_ENTRY( gldChoosePixelFormat );


	/* Validation ... */

	if( !outPixelFormat )
	{
		DEBUG_ERROR( gldChoosePixelFormat, "Invalid GLIPixelFormat handle : 0x%08x\n", outPixelFormat);
		return GLI_BAD_POINTER;
	}
	
	*outPixelFormat = NULL;

	if( !glrValidateEnvironment() )
	{
		DEBUG_ERROR( gldChoosePixelFormat, "No target could be found...\n", outPixelFormat);
		return GLI_NO_ERROR;
	};
	

	/* Initialization ... */

	glrDefaultPixelFormat( &theTempPixFormat );


	/* Parse attribute list */
	theAttributeList = inAttributeList;
	DEBUG_VERBOSE( gldChoosePixelFormat, "Attribute list: ...\n");
	while( *theAttributeList != GLI_NONE )
	{
		switch(*theAttributeList++)
		{
			case GLI_BUFFER_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- buffer size = 0x%08x\n", *theAttributeList);
				if(*theAttributeList++ < 0) 
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is invalid\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				break;

			case GLI_LEVEL:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- level size = 0x%08x\n", *theAttributeList);
				if(*theAttributeList++)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "no parameter specified\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				break;

			case GLI_RGBA:
				theRGBA = GL_TRUE;
				DEBUG_VERBOSE( gldChoosePixelFormat, "- RGBA = true\n");
				break;

			case GLI_DOUBLEBUFFER:
				theDoubleBuffer = GL_TRUE;
				DEBUG_VERBOSE( gldChoosePixelFormat, "- double buffer = true\n");
				break;

			case GLI_STEREO:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- stereo = true\n");
				DEBUG_ERROR( gldChoosePixelFormat, "stereo buffer is not supported\n" );
				return GLI_NO_ERROR;
				break;

			case GLI_AUX_BUFFERS:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- auxiliary buffers = %d\n", *theAttributeList);
				if(*theAttributeList++)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "no parameter specified\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				break;

			case GLI_ALPHA_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- alpha size = %d\n", *theAttributeList);
				if(*theAttributeList > 0)
					theTempPixFormat.color_mode &= (GLI_ARGB1555_BIT | GLI_ARGB8888_BIT);
				i = *theAttributeList++;
				if(i < 0 || i > 8)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				glr_debug_printf("alpha: %d\n",i);
				if(i > theColorSize) theColorSize = i;
				break;

			case GLI_RED_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- red size = %d\n", *theAttributeList);
				i = *theAttributeList++;
				if(i < 0 || i > 8)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				glr_debug_printf("red: %d\n",i);
				if(i > theColorSize) theColorSize = i;
				break;

			case GLI_GREEN_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- green size = %d\n", *theAttributeList);
				i = *theAttributeList++;
				if(i < 0 || i > 8)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				glr_debug_printf("green: %d\n",i);
				if(i > theColorSize) theColorSize = i;
				break;

			case GLI_BLUE_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- blue size = %d\n", *theAttributeList);
				i = *theAttributeList++;
				if(i < 0 || i > 8)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				glr_debug_printf("blue: %d\n",i);
				if(i > theColorSize) theColorSize = i;
				break;

			case GLI_DEPTH_SIZE:
				i = *theAttributeList++;
				DEBUG_VERBOSE( gldChoosePixelFormat, "- depth size = %d\n", i);
				if( i > 0 && i <= 32 )
				{
				    // FIXME - Should do this dynamically based on the chipset we're on.
					if (1 || i <= 24 )
					{
						theTempPixFormat.depth_mode = GLI_16_BIT;
					}
					else
					{
						theTempPixFormat.depth_mode = GLI_32_BIT;
						DEBUG_ERROR( gldChoosePixelFormat, "depth size too big (32 bit)\n" );
						return GLI_BAD_ATTRIBUTE;
					}
				}
				else 
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				break;

			case GLI_STENCIL_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- stencil size = %d\n", *theAttributeList);
				i = *theAttributeList++;
				if(i < 0 || i > 8)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				break;

			case GLI_ACCUM_RED_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- accum red size = %d\n", *theAttributeList);
				i = *theAttributeList++;
				if(i < 0 || i > 8)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				if(i > theAccumSize) theAccumSize = i;
				break;

			case GLI_ACCUM_GREEN_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- accum green size = %d\n", *theAttributeList);
				if(i < 0 || i > 8)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				if(i > theAccumSize) theAccumSize = i;
				break;

			case GLI_ACCUM_BLUE_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- accum blue size = %d\n", *theAttributeList);
				if(i < 0 || i > 8)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				if(i > theAccumSize) theAccumSize = i;
				break;

			case GLI_ACCUM_ALPHA_SIZE:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- accum alpha size = %d\n", *theAttributeList);
				if(i < 0 || i > 8)
				{
					DEBUG_ERROR( gldChoosePixelFormat, "parameter is out of range\n" );
					return GLI_BAD_ATTRIBUTE;
				}
				break;

			case GLI_OFFSCREEN:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- offscreen\n");
				if(*theAttributeList++) return GLI_NO_ERROR;
				DEBUG_ERROR( gldChoosePixelFormat, "offscreen mode is not supported\n" );
				return GLI_NO_ERROR;
				break;

			case GLI_WINDOW:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- windowed\n");
				theTempPixFormat.os_support |= GLI_WINDOW_BIT;
				break;

			case GLI_FULLSCREEN:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- fullscreen\n");
				theTempPixFormat.os_support |= GLI_FULLSCREEN_BIT;
				break;

			case GLI_MINIMUM_POLICY:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- minimum policy = true\n");
				break;

			case GLI_MAXIMUM_POLICY:
				DEBUG_VERBOSE( gldChoosePixelFormat, "- maximum policy = true\n");
				break;

			case GLI_PIXEL_SIZE:
				i = *theAttributeList++;
				DEBUG_VERBOSE( gldChoosePixelFormat, "- pixel size = %d\n", i);
				if( i == 16 || i == 32 )
					theColorSize = i;
				else {
					DEBUG_ERROR( gldChoosePixelFormat, "invalid pixel depth\n" );
					return GLI_NO_ERROR;
				}
				break;
	
		}
			
		/* If hit max attrib count, must be an error, so move on */
		if((theAttributeList - inAttributeList) > GLR_MAX_ATTRIB_ARRAY_SIZE) break;
	}
	DEBUG_VERBOSE( gldChoosePixelFormat, "Attribute list: parsing complete!\n");

	theDeviceList = (GLIDevice *) glmMalloc( glrDeviceListCount * sizeof(GLIDevice*) );
	if( !theDeviceList ) {
		DEBUG_ERROR( gldChoosePixelFormat, "Could not allocate the device list\n" );
		return GLI_BAD_ALLOC;
	}
	
	if ( !theRGBA )
	{
		if( theColorSize <= 5 )
			theTempPixFormat.color_mode = GLI_RGB565_BIT;
		else
			theTempPixFormat.color_mode = GLI_ARGB8888_BIT;
	}
	else
	{
		if( theColorSize <= 5 )
			theTempPixFormat.color_mode = GLI_ARGB1555_BIT;
		else
			theTempPixFormat.color_mode = GLI_ARGB8888_BIT;
	}
	
	glr_debug_printf("color_mode: %08lx theColorSize: %d\n",theTempPixFormat.color_mode,
	  theColorSize);
	
	theTempPixFormat.num_devices = 0;
	theTempPixFormat.devices = theDeviceList;

	for (i = 0; i < inNumberOfDevs; i++) {
		if(glrDeviceToBoardID(inDeviceList[i]) >= 0){
				theDeviceList[ theTempPixFormat.num_devices ] = inDeviceList[i];
				theTempPixFormat.num_devices++;
			}
	}
	
	if(theDoubleBuffer){
		theTempPixFormat.buffer_mode |= GLI_DOUBLEBUFFER_BIT;
	} else {
		//theTempPixFormat.buffer_mode |= GLI_SINGLEBUFFER_BIT;
	}
		
	if ( !(theTempPixFormat.os_support & (GLI_WINDOW_BIT | GLI_FULLSCREEN_BIT | GLI_OFFSCREEN_BIT) ) )
	{
		theTempPixFormat.os_support |= GLI_WINDOW_BIT;
	}
	
	
	theNewPixFormat = (GLIPixelFormat *) glmMalloc( sizeof(GLIPixelFormat) );
	if( !theNewPixFormat ) {
		DEBUG_ERROR( gldChoosePixelFormat, "Could not allocate the new pixel format structure!\n" );
		return GLI_BAD_ALLOC;
	}
	
	*theNewPixFormat = theTempPixFormat;
	theNewPixFormat->next_pixel_format = NULL;
	
	
	*outPixelFormat = theNewPixFormat;
	
	DEBUG_PRINT_PIXEL_FORMAT( gldChoosePixelFormat, *outPixelFormat );	
	DEBUG_VERBOSE( gldChoosePixelFormat, "successfull ...\n" );
	return GLI_NO_ERROR;
}



/*
________________________________________________________________________________________

      gldDestroyPixelFormat
________________________________________________________________________________________

*/

GLenum
gldDestroyPixelFormat(
	GLIPixelFormat *	inPixelFormat)
{
	GLIPixelFormat *	thePixelFormat = inPixelFormat;
	GLIPixelFormat *	theNextPixelFormat;
	
	DEBUG_ENTRY( gldDestroyPixelFormat );

	if ( !inPixelFormat )
	{
		DEBUG_ERROR( gldDestroyPixelFormat, "Invalid GLIPixelFormat pointer : 0x%08x\n", inPixelFormat);
		return GLI_BAD_POINTER;
	}

	glmFree( (char *) thePixelFormat->devices );
	
	while ( !thePixelFormat )
	{
		theNextPixelFormat = thePixelFormat->next_pixel_format;
		glmFree( (char *) thePixelFormat );
		thePixelFormat = theNextPixelFormat;
	}

	return GLI_NO_ERROR;
}





/*
________________________________________________________________________________________

      glrDefaultPixelFormat
________________________________________________________________________________________

*/

static void
glrDefaultPixelFormat(
	GLIPixelFormat *	outFormat)
{
	if( !outFormat ) {
		DEBUG_ERROR( glrDefaultPixelFormat, "Invalid GLIPixelFormat pointer : 0x%08x\n", outFormat);
		return;
	}
	
	/* Pointer to next format */
	outFormat->next_pixel_format   = NULL;
	
	/* Set renderer id */
	outFormat->renderer_id         = GLR_DRIVER_ID;
	
	/* OS support feature bits */
	outFormat->os_support          = GLI_BACKING_STORE_BIT
//								   | GLI_MP_SAFE_BIT
								   | GLI_ROBUST_BIT
								   | GLI_RECOVERABLE_BIT
								   | GLI_ACCELERATED_BIT
								   | GLI_COMPLIANT_BIT;
	
	/* Buffer mode info */
	outFormat->buffer_mode         = 0;
//	outFormat->buffer_mode         = GLI_DOUBLEBUFFER_BIT;

	/* Buffer size info */
	outFormat->color_mode          = GLI_RGB555_BIT;

	outFormat->accum_mode          = GLI_NONE;
	outFormat->depth_mode          = GLI_16_BIT;
	outFormat->stencil_mode        = GLI_0_BIT;

	outFormat->buffer_level        = 0;
	outFormat->aux_buffers         = 0;

}

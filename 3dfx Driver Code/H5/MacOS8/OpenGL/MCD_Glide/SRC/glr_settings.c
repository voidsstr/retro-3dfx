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


/*
________________________________________________________________________________________

      gldSetInteger
________________________________________________________________________________________


return:
GLD_RECT_SAME or GLD_RECT_CHANGED

*/

GLenum
gldSetInteger(
	GLDContext			inContext,
	GLenum				inPname,
	const GLint *		inParams)
{
	DEBUG_ENTRY( gldSetInteger );
	DEBUG_ENTRY_INPUT( gldSetInteger, "GLDContext = 0x%08x\n", inContext );

	if( !inContext ) {
		DEBUG_ERROR( gldSetInteger, "Invalid context pointer : 0x%08x\n", inContext);
		return GLI_BAD_POINTER;
	}
	
	if( !inParams ) {
		DEBUG_ERROR( gldSetInteger, "Invalid params pointer : 0x%08x\n", inParams);
		return GLI_BAD_POINTER;
	}
	
	switch( inPname ) {

		case GLI_BUFFER_RECT_ENABLE:
			DEBUG_ENTRY_INPUT( gldSetInteger, "set GLI_BUFFER_RECT_ENABLE to : %d\n", inParams[0]);
			if(( inParams[0] != 0) == inContext->buffer_rect_enabled )
			  return GLD_RECT_SAME;
			
			inContext->buffer_rect_enabled = inParams[0] != 0;
			
			if(gldAttachDrawable(inContext,inContext->drawableType,inContext->drawable) == GLI_BAD_ALLOC)
			  return GLI_BAD_ALLOC;
			else
			  return GLD_RECT_CHANGED;
			  
			break;

		case GLI_BUFFER_RECT:
			DEBUG_ENTRY_INPUT( gldSetInteger, "set GLI_BUFFER_RECT to : x = %d / y = %d / w = %d / h = %d\n",
											 inParams[0], inParams[1], inParams[2], inParams[3]);
			if( inParams[2] < 0 || inParams[3] < 0 )
				return GLI_BAD_VALUE;
			inContext->buffer_rect.x = inParams[0];
			inContext->buffer_rect.y = inParams[1];
			inContext->buffer_rect.w = inParams[2];
			inContext->buffer_rect.h = inParams[3];
			if( !inContext->buffer_rect_enabled )
				return GLD_RECT_SAME;
				
			if(gldAttachDrawable(inContext,inContext->drawableType,inContext->drawable) == GLI_BAD_ALLOC)
			  return GLI_BAD_ALLOC;
			else
			  return GLD_RECT_CHANGED;

			break;

		case GLI_SWAP_RECT_ENABLE:
			DEBUG_ENTRY_INPUT( gldSetInteger, "set GLI_SWAP_RECT_ENABLE to : %d\n", inParams[0]);
			inContext->swap_rect_enabled = *inParams != 0;
			break;

		case GLI_SWAP_RECT:
			DEBUG_ENTRY_INPUT( gldSetInteger, "set GLI_SWAP_RECT to : x = %d / y = %d / w = %d / h = %d\n",
											 inParams[0], inParams[1], inParams[2], inParams[3]);
			if(inParams[2] < 0 || inParams[3] < 0)
				return GLI_BAD_VALUE;
			inContext->swap_rect.x = inParams[0];
			inContext->swap_rect.y = inParams[1];
			inContext->swap_rect.w = inParams[2];
			inContext->swap_rect.h = inParams[3];
			break;

		case GLI_SWAP_INTERVAL:
			DEBUG_ENTRY_INPUT( gldSetInteger, "set GLI_SWAP_INTERVAL to : %d\n", inParams[0]);
			inContext->swap_interval = inParams[0];
			break;

		default:
			DEBUG_ERROR( gldSetInteger, "Invalid params name : 0x%08x (%d)\n", inPname, inPname);
			return GLI_BAD_ENUM;
	}
		
	return GLI_NO_ERROR;
}



/*
________________________________________________________________________________________

      gldGetInteger
________________________________________________________________________________________

*/

GLenum
gldGetInteger(
	GLDContext			inContext,
	GLenum				inPname,
	GLint *				outParams)
{
	DEBUG_ENTRY( gldGetInteger );
	DEBUG_ENTRY_INPUT( gldGetInteger, "GLDContext = 0x%08x\n", inContext );

	if( !inContext ) {
		DEBUG_ERROR( gldGetInteger, "Invalid context pointer : 0x%08x\n", inContext);
		return GLI_BAD_POINTER;
	}
	
	if( !outParams ) {
		DEBUG_ERROR( gldGetInteger, "Invalid params pointer : 0x%08x\n", outParams);
		return GLI_BAD_POINTER;
	}
	
	switch( inPname ) {
	
		case GLI_BUFFER_RECT_ENABLE:
			outParams[0] = inContext->buffer_rect_enabled;
			break;

		case GLI_BUFFER_RECT:
			outParams[0] = inContext->buffer_rect.x;
			outParams[1] = inContext->buffer_rect.y;
			outParams[2] = inContext->buffer_rect.w;
			outParams[3] = inContext->buffer_rect.h;
			break;

		case GLI_SWAP_RECT_ENABLE:
			outParams[0] = inContext->swap_rect_enabled;
			break;

		case GLI_SWAP_RECT:
			outParams[0] = inContext->swap_rect.x;
			outParams[1] = inContext->swap_rect.y;
			outParams[2] = inContext->swap_rect.w;
			outParams[3] = inContext->swap_rect.h;
			break;

		case GLD_DRAWABLE_DIMS:
			outParams[0] = inContext->viewPort.size.w;
			outParams[1] = inContext->viewPort.size.h;
			break;
			
		case GLD_DRAWABLE_OFFSET:
			outParams[0] = inContext->viewPort.offset.x;
			outParams[1] = inContext->viewPort.offset.y;
			break;
			
		case GLD_DRAWABLE_DEADBAND:
			outParams[0] = inContext->viewPort.deadband.w;
			outParams[1] = inContext->viewPort.deadband.h;
			break;

		default:
			DEBUG_ERROR( gldGetInteger, "Invalid params name : 0x%08x (%d)\n", inPname, inPname);
			return GLI_BAD_ENUM;
	}
	
	return GLI_NO_ERROR;
}


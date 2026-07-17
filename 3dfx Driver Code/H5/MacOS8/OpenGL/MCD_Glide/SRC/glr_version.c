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
#include "glr_glide.h"
#include <CodeFragments.h>



/*
________________________________________________________________________________________

      gldGetVersion
________________________________________________________________________________________

*/

GLboolean
gldGetVersion(
	GLint *			outMajor,
	GLint *			ouMinor,
	GLushort *		ouDriverID )
{
	
	DEBUG_ENTRY( gldGetVersion );


	/* Get the hardware info */
	if( !glrValidateEnvironment() ) {
		return GL_FALSE;
	};

	*outMajor = GLD_MAJOR_VERSION;
	*ouMinor = GLD_MINOR_VERSION;
	*ouDriverID = GLR_DRIVER_ID;
	
	DEBUG_VERBOSE( gldGetVersion, "OpenGL plugin API version %d.%d / driverID = 0x%04x\n", *outMajor, *ouMinor, *ouDriverID);

	return GL_TRUE;
}


/*
________________________________________________________________________________________

      gldGetString
________________________________________________________________________________________

*/

const GLubyte *
gldGetString(
	GLDContext		inContext,
	GLenum			inName)
{
	static const GLubyte glr_vendor[]     = { "3dfx Interactive" };
	static const GLubyte glr_renderer2[]   = { "3dfx Voodoo 2" };
	static const GLubyte glr_renderer3[]   = { "3dfx Voodoo 3" };
	static const GLubyte glr_version[]    = { "1.1_3dfx_1.0b11" };
	static const GLubyte glr_extensions[] = { 
#if GLR_EXT_compiled_vertex_array
                                              " GL_EXT_compiled_vertex_array"
#endif
#if GLR_EXT_texture_env_add
                                              " GL_EXT_texture_env_add"
#endif
#if GLR_ARB_multitexture
                                              " GL_ARB_multitexture"
#endif
#if GLR_EXT_abgr
                                              " GL_EXT_abgr"
#endif
	                                          " " };

	const GLubyte *str = NULL;
	
	DEBUG_ENTRY( gldGetString );

	if( !inContext ) {
		DEBUG_ERROR( gldGetString, "Invalid context 0x%08x\n", inContext );
		return NULL;
	}
	
	switch( inName )
	{
		case GL_VENDOR:
			str = glr_vendor;
			DEBUG_ENTRY_INPUT( gldGetString, "vendor = %s\n", str );
			break;

		case GL_RENDERER:
			if(gls_surface_ext){
				str = glr_renderer3;
			} else {
				str = glr_renderer2;			
			}
			DEBUG_ENTRY_INPUT( gldGetString, "renderer = %s\n", str );
			break;

		case GL_VERSION:
			str = glr_version;
			DEBUG_ENTRY_INPUT( gldGetString, "version = %s\n", str );
			break;

		case GL_EXTENSIONS:
			str = glr_extensions;
			DEBUG_ENTRY_INPUT( gldGetString, "extensions = %s\n", str );
			break;

		default:
			DEBUG_ERROR( gldGetString, "Invalid string index : %d (0x%08x)\n", inName, inName);
			break;
	}
	
	return str;
}
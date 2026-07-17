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

#include <stdlib.h>
#include "glr.h"



/*
________________________________________________________________________________________

      glrSetError
________________________________________________________________________________________

*/

void
glrSetError(
  GLDContext	ctx,
  GLenum        err)
{
	DEBUG_PRINTF( "glrSetError() : err == %X", err);
	
	if(ctx->error == GL_NO_ERROR)
		ctx->error = err;
}


/*
________________________________________________________________________________________

      gldGetError
________________________________________________________________________________________

*/

GLenum gldGetError(
	GLDContext			inContext)
{
	GLenum theErr = inContext->error;

	DEBUG_ENTRY( gldGetError );

	inContext->error = GL_NO_ERROR;
	
	return theErr;
}

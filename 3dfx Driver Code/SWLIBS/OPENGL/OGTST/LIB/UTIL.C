/**************************************************************************
 *									  *
 * 		 Copyright (C) 1989, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/* readwrite.c - $Revision: 2$ */

#include "ogtst.h"

/********************************************************************
*
* This file contains utility functions
*
*********************************************************************/


/**************************************************************************
* ogLibParseColor() - convert from compressed format to something readable
*                     (used for debugging)
***************************************************************************/

void 
ogLibParseColor(GLuint color, GLubyte *r, GLubyte *g, GLubyte *b, GLubyte *a)
{
  *r = ((color & 0xff000000) >> 24);
  *g = ((color & 0x00ff0000) >> 16);
  *b = ((color & 0x0000ff00) >>  8);
  *a = (color & 0x000000ff);
}

/**************************************************************************
* ogCheckErrors() - Used to get all glGetError's 
***************************************************************************/

void 
ogCheckErrors(void)
{
   GLenum err;

   while ((err = glGetError()) != GL_NO_ERROR)
	ogEnvLog(OG_LALWAYS, "ERROR: glGetError == 0x%x (%s)\n", err, 
	   ogLibGLError(err));

}

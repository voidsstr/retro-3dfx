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

#include <alloca.h>
#include "ogtst.h"

/********************************************************************
*
* This file contains the read and write gl functions. By putting 
* them in here, and only calling the gtst versions in the tests, it
* becomes easier to fiddle around with them when desired. This can
* be very useful during bringup.
*
*********************************************************************/

/********************************************************************
* ogLibReadPixels() -
********************************************************************/
void
ogLibReadPixels(int x1, int y1, int x2, int y2, GLuint pixels[])
{
    if (ogEnvCurVisualInfo(GLX_RGBA)) {
	int i;
	int w = x2-x1+1;
	int h = y2-y1+1;
	GLubyte *d, *s, r, g, b, a;

	glReadPixels(x1,y1,x2-x1+1,y2-y1+1,GL_RGBA,GL_UNSIGNED_BYTE,pixels);
	s = (GLubyte *)pixels;
	d = (GLubyte *)pixels;
	for (i=0; i < w*h; i++) {
	    a = *s++;
	    b = *s++;
	    g = *s++;
	    r = *s++;
	    *d++ = r;
	    *d++ = g;
	    *d++ = b;
	    *d++ = a;
	}
    } else
	glReadPixels(x1,y1,x2-x1+1,y2-y1+1,GL_COLOR_INDEX,GL_UNSIGNED_INT,
		     pixels);
}


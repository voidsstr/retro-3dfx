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

/* getviewport.c - $Revision: 2$ */

/*
 * This program tests the viewport/scissor command by drawing rectangles
 * larger than random viewports and making sure they get clipped properly. 
 *
 * Debug levels: 1 - print the scissor and clear parameters 2 - print
 * display-list/immediate mode 5 - print checking trace 
 */

#include <stdlib.h>
#include "ogtst.h"		/* include test environment		 */
#include "xform.h"		/* include test environment		 */

TESTMOD(getviewport) {
    GLint x1, y1, x2, y2, width, height;
    int obj;
    int temp, xmax, ymax;
    GLint iv[4], ov[4];
    GLfloat fv[4];
    GLdouble dv[4];
    GLint maxv[2], error;

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxv);
    ogEnvLog(1, "glGetIntegerv(GL_MAX_VIEWPORT_DIMS, %d %d);\n", maxv[0], maxv[1]);

    while (pass--) {

	if (ogLibBitRand(2) == 0) {
	    /* force clamping */
	    /* only do this 25 % of the time */

	    x1 = 2 + ogLibIntRand(0,maxv[0]*3 - 4);
	    x2 = 2 + x1+maxv[0]+ogLibIntRand(0,maxv[0]*3 - 4);

	    y1 = 2 + ogLibIntRand(0,maxv[1]*3 - 4);
	    y2 = 2 + y1+maxv[1]+ogLibIntRand(0,maxv[1]*3 - 4);

	    if (x1 > x2) {
		temp = x1;
		x1 = x2;
		x2 = temp;
	    }
	    if (y1 > y2) {
		temp = y1;
		y1 = y2;
		y2 = temp;
	    }
	    width = maxv[0];
	    height = maxv[1];
	} else {
	    do {
		x1 = 2 + ogLibIntRand(0,xmax - 4);
		x2 = 2 + ogLibIntRand(0,xmax - 4);
	    } while (abs(x1 - x2) < 4);

	    do {
		y1 = 2 + ogLibIntRand(0,ymax - 4);
		y2 = 2 + ogLibIntRand(0,ymax - 4);
	    } while (abs(y1 - y2) < 4);

	    if (x1 > x2) {
		temp = x1;
		x1 = x2;
		x2 = temp;
	    }
	    if (y1 > y2) {
		temp = y1;
		y1 = y2;
		y2 = temp;
	    }
	    width = x2-x1+1;
	    height = y2-y1+1;
	}
	START_DL_OR_IM(1);

	ogEnvLog(1, "glViewport(%d,%d,%d,%d);\n", x1, y1, x2 - x1 + 1, y2 - y1 + 1);
	glViewport(x1, y1, (x2) - (x1) + 1, (y2) - (y1) + 1);

	FINIS_DL_OR_IM(1);

	switch(ogLibIntRand(0,2)) {
	case 0:
		glGetIntegerv(GL_VIEWPORT, iv);
		ogEnvLog(1, "glGetIntegerv(GL_VIEWPORT, %d %d %d %d)\n", iv[0], iv[1], iv[2], iv[3]);
		if (iv[0] != x1 || iv[1] != y1 || iv[2] != width || iv[3] != height)
		    ogEnvLog(OG_LFAIL, "retrived incorrect integer viewport\n");
		break;

	case 1:
		glGetFloatv(GL_VIEWPORT, fv);
		ogEnvLog(1, "glGetFloatv(GL_VIEWPORT, %f %f %f %f)\n", fv[0], fv[1], fv[2], fv[3]);
		if (fv[0] != x1 || fv[1] != y1 || fv[2] != width || fv[3] != height)
		    ogEnvLog(OG_LFAIL, "retrived incorrect float viewport\n");
		break;
	case 2:
		glGetDoublev(GL_VIEWPORT, dv);
		ogEnvLog(1, "glGetDoublev(GL_VIEWPORT, %f %f %f %f)\n", dv[0], dv[1], dv[2], dv[3]);
		if (dv[0] != x1 || dv[1] != y1 || dv[2] != width || dv[3] != height)
		    ogEnvLog(OG_LFAIL, "retrived incorrect double viewport\n");
		break;
	}
    }

    /* final check - test that negative width/height cause errors */
    if ((error = glGetError()) != GL_NO_ERROR)
	ogEnvLog(OG_LFAIL, "premature error glGetError(%s)\n", ogLibGLError(error));

    glGetIntegerv(GL_VIEWPORT, ov);
    ogEnvLog(1, "glGetIntegerv(GL_VIEWPORT, %d %d %d %d)\n", ov[0], ov[1], ov[2], ov[3]);

    ogEnvLog(1, "glViewport(%d,%d,%d,%d);\n", ov[0]+1, ov[1]+1, -ov[2]-1, ov[3]+1);
    glViewport(ov[0]+1, ov[1]+1, -ov[2]-1, ov[3]+1);
    if ((error = glGetError()) != GL_INVALID_VALUE) {
	if (error != GL_NO_ERROR)
	    ogEnvLog(OG_LFAIL, "unexpected error glGetError(%s)\n", ogLibGLError(error));
	else
	    ogEnvLog(OG_LFAIL, "failure to report error\n", ogLibGLError(error));
	    
    }

    /* make sure viewport didn't change */
    glGetIntegerv(GL_VIEWPORT, iv);
    ogEnvLog(1, "glGetIntegerv(GL_VIEWPORT, %d %d %d %d)\n", iv[0], iv[1], iv[2], iv[3]);
    if (iv[0] != ov[0] || iv[1] != ov[1] || iv[2] != ov[2] || iv[3] != ov[3])
	    ogEnvLog(OG_LFAIL, "illegal viewport updated viewport state\n", ogLibGLError(error));


    ogEnvLog(1, "glViewport(%d,%d,%d,%d);\n", ov[0]+1, ov[1]+1, ov[2]+1, -ov[3]-1);
    glViewport(ov[0]+1, ov[1]+1, -ov[2]-1, ov[3]+1);

    if ((error = glGetError()) != GL_INVALID_VALUE) {
	if (error != GL_NO_ERROR)
	    ogEnvLog(OG_LFAIL, "unexpected error glGetError(%s)\n", ogLibGLError(error));
	else
	    ogEnvLog(OG_LFAIL, "failure to report error\n", ogLibGLError(error));
	    
    }
    /* make sure viewport didn't change */
    glGetIntegerv(GL_VIEWPORT, iv);
    ogEnvLog(1, "glGetIntegerv(GL_VIEWPORT, %d %d %d %d)\n", iv[0], iv[1], iv[2], iv[3]);
    if (iv[0] != ov[0] || iv[1] != ov[1] || iv[2] != ov[2] || iv[3] != ov[3])
	    ogEnvLog(OG_LFAIL, "illegal viewport updated viewport state\n", ogLibGLError(error));
}

CLEANUP(getviewport)
{
    glViewport(0,0,ogEnvQuery(OG_XWSIZE),ogEnvQuery(OG_YWSIZE));
}


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

/* cleardepth.c - $Revision: 2$ */

/*
 * This program tests the viewport/clear commands. 
 *
 * Debug levels: 1 - print the viewport/clear parameters 2 - print
 * display-list/immediate mode 
 */

#include <stdio.h>
#include "ogtst.h"		/* include test environment		 */

#define DEPTH_TOLERANCE		0.0001
static float depth_tolerance = DEPTH_TOLERANCE;
#define DLIST_FREC		3

static GLint xmax, ymax, left, right, bottom, top;

static void setClearRect(void);
	/* this function defined in clearall.c file */
extern int rectCheckDepth(GLint left, GLint bottom, GLint right, GLint top, 
			  GLclampd insideDepth, GLclampd outsideDepth,
			  GLfloat tolerance);

#define VERT	0
#define HORIZ	1


TESTMOD(cleardepth)
{
    GLboolean doDlist;
    GLint depthBits;
    GLfloat depthValue = 1.0;

    depthBits = ogEnvCurVisualInfo(GLX_DEPTH_SIZE);
    if (depthBits)
	ogEnvLog(OG_LGENERAL, "Depth buffer bits %d\n", depthBits);
    else {
	ogEnvLog(OG_LGENERAL, "Depth buffer is not available\n");
	return;
    }

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    glEnable(GL_SCISSOR_TEST);

    while (pass--) {

	glViewport(0, 0, xmax + 1, ymax + 1);
	glScissor(0, 0, xmax + 1, ymax + 1);
	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);
        ogEnvLog(2, "glClearDepth(0.0), glClear(GL_DEPTH_BUFFER_BIT)\n");

	doDlist = (pass % DLIST_FREC) == 0;
	if (doDlist){
	    ogEnvLog(OG_LPARAMETERS, "display-list mode\n");
	    glNewList(1, GL_COMPILE);
	}else
	    ogEnvLog(OG_LPARAMETERS, "immediate mode\n");

	setClearRect();
	glClearDepth(depthValue);
        ogEnvLog(2, "depthValue: %f\n", depthValue);
	glClear(GL_DEPTH_BUFFER_BIT);
	if (doDlist){
	    glEndList();
	    glCallList(1);
	    glDeleteLists(1, 1);
	}

	rectCheckDepth(left, bottom, right, top, depthValue, 0.0, depth_tolerance);
	depthValue = ogLibFloatRand(0.0, 1.0);

    }

}

void setClearRect(void)
{
    GLint tmp;

    left = ogLibIntRand(1, xmax - 4);
    right = ogLibIntRand(1, xmax - 4);
    if (left > right){
	tmp = left;
	left = right;
	right = tmp;
    }

    bottom = ogLibIntRand(1,ymax - 4);
    top = ogLibIntRand(1,ymax - 4);
    if (bottom > top){
	tmp = bottom;
	bottom = top;
	top = tmp;
    }

    glViewport(left, bottom, right - left + 1, top - bottom + 1);
    glScissor(left, bottom, right - left + 1, top - bottom + 1);

    ogEnvLog(OG_LPARAMETERS,"clear rectangle: left %d right %d bottom %d top %d\n",
                left, right, bottom, top);

}

CLEANUP(cleardepth)
{
    glClearDepth(1.0);
    glDisable(GL_SCISSOR_TEST);
    glScissor(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    glViewport(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
}

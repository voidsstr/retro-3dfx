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

/* clearstencil.c - $Revision: 2$ */

/*
 * This program tests the viewport/clear commands. 
 *
 * Debug levels: 1 - print the viewport/clear parameters 2 - print
 * display-list/immediate mode 
 */

#include <stdio.h>
#include "ogtst.h"		/* include test environment		 */

#define STENCIL_TOLERANCE		0.0001
#define DLIST_FREC		3

static GLint xmax, ymax, left, right, bottom, top;

static void setClearRect(void);
	/* this function defined in clearall.c file */
int rectCheckStencil(GLint left, GLint bottom, GLint right, GLint top, 
				GLint insideStencil, GLint outsideStencil);

#define VERT	0
#define HORIZ	1


TESTMOD(clearstencil)
{
    GLboolean doDlist;
    GLint stencilBits;
    GLint stencilValue;

    stencilBits = ogEnvCurVisualInfo(GLX_STENCIL_SIZE);
    if (stencilBits)
	ogEnvLog(OG_LGENERAL, "Stencil bits %d\n", stencilBits);
    else{
	ogEnvLog(OG_LGENERAL, "Stencil buffer is not available\n");
	return;
    }

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    glEnable(GL_SCISSOR_TEST);

    while (pass--) {

	glViewport(0, 0, xmax + 1, ymax + 1);
	glScissor(0, 0, xmax + 1, ymax + 1);
	glClearStencil(0);
	ogEnvLog(3, "glClearStencil(0)\n");
	glClear(GL_STENCIL_BUFFER_BIT);
	ogEnvLog(3, "glClear(GL_STENCIL_BUFFER_BIT)\n");

	doDlist = (pass % DLIST_FREC) == 0;
	if (doDlist){
	    ogEnvLog(OG_LPARAMETERS, "display-list mode\n");
	    glNewList(1, GL_COMPILE);
	}else
	    ogEnvLog(OG_LPARAMETERS, "immediate mode\n");

	setClearRect();
	stencilValue = ogLibBitRand(stencilBits);
	glClearStencil(stencilValue);
	ogEnvLog(3, "glClearStencil(%d)\n", stencilValue);
	glClear(GL_STENCIL_BUFFER_BIT);
	ogEnvLog(3, "glClear(GL_STENCIL_BUFFER_BIT)\n");
	if (doDlist){
	    glEndList();
	    glCallList(1);
	    glDeleteLists(1, 1);
	}

	rectCheckStencil(left, bottom, right, top, stencilValue, 0);

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
    ogEnvLog(3, "glViewport(%d, %d, %d, %d)\n", left, bottom, (right - left + 1), (top - bottom + 1));
    ogEnvLog(3, "glScissor(%d, %d, %d, %d)\n", left, bottom, (right - left + 1), (top - bottom + 1));
}

CLEANUP(clearstencil)
{
    glDisable(GL_SCISSOR_TEST);
    glScissor(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    glViewport(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    glClearStencil(0);
}

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

/* clearaccum.c - $Revision: 2$ */

/*
 * This program tests the viewport/clear commands. 
 *
 * Debug levels: 1 - print the viewport/clear parameters 2 - print
 * display-list/immediate mode 
 */

#include <stdio.h>
#include "ogtst.h"		/* include test environment		 */

#define DLIST_FREC		3

static GLint xmax, ymax, left, right, bottom, top;
static void setClearRect(void);

TESTMOD(clearaccum)
{
    GLboolean doDlist;
    GLint redBits, greenBits, blueBits, alphaBits;
    unsigned int r, g, b, a, accumValue;

    if (!ogEnvCurVisualInfo(GLX_RGBA)) {
	ogEnvLog(OG_LGENERAL, "Visual is rgba\n");
	return;
    }

    redBits = ogEnvCurVisualInfo(GLX_ACCUM_RED_SIZE);
    greenBits = ogEnvCurVisualInfo(GLX_ACCUM_GREEN_SIZE);
    blueBits = ogEnvCurVisualInfo(GLX_ACCUM_BLUE_SIZE);
    alphaBits = ogEnvCurVisualInfo(GLX_ACCUM_ALPHA_SIZE);

    if ((redBits == 0) && (greenBits == 0) && 
	(blueBits == 0) && (alphaBits == 0)){
	ogEnvLog(OG_LGENERAL, "Accum buffer is not available.\n");
	return;
    }else
	ogEnvLog(OG_LGENERAL, "redBits %d greenBits %d blueBits %d alphaBits %d\n",
		redBits, greenBits, blueBits, alphaBits);

    glEnable(GL_SCISSOR_TEST);

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    glViewport(0, 0, xmax + 1, ymax + 1);
    glScissor(0, 0, xmax + 1, ymax + 1);
    while (pass--) {

	glClearAccum(0.0, 0.0, 0.0, 0.0);
	glClear(GL_ACCUM_BUFFER_BIT);

	doDlist = (pass % DLIST_FREC) == 0;
	if (doDlist){
	    ogEnvLog(OG_LPARAMETERS, "display-list mode\n");
	    glNewList(1, GL_COMPILE);
	}else
	    ogEnvLog(OG_LPARAMETERS, "immediate mode\n");

	setClearRect();
	r = ogLibBitRand(8);
	g = ogLibBitRand(8);
	b = ogLibBitRand(8);
	a = ogLibBitRand(8);
	accumValue = (r<<24) | (g << 16) | (b << 8) | (a);
	glClearAccum(r/255.0, g/255.0, b/255.0, a/255.0);
	glClear(GL_ACCUM_BUFFER_BIT);

	if (doDlist){
	    glEndList();
	    glCallList(1);
	    glDeleteLists(1, 1);
	}

	glViewport(0, 0, xmax + 1, ymax + 1);
	glScissor(0, 0, xmax + 1, ymax + 1);

	glAccum(GL_RETURN, 1.0);
	ogLibRectCheck(left, bottom, right, top, accumValue, 0);
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

CLEANUP(clearaccum)
{
    glDisable(GL_SCISSOR_TEST);
    glViewport(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    ogLibSetDefaultClears();
}

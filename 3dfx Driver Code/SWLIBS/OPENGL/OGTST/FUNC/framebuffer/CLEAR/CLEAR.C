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

/* clear.c - $Revision: 2$ */

/*
 * This program tests the viewport/clear commands. 
 *
 * Debug levels: 1 - print the viewport/clear parameters 2 - print
 * display-list/immediate mode 
 */

#include <stdlib.h>    /* for abs */
#include "ogtst.h"		/* include test environment		 */

TESTMOD(clear)
{
    GLint temp, x1, x2, y1, y2;
    int obj;
    int xmax, ymax;
    int maxcolor;
    int rgbamode, indexbits, masking;

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    glGetIntegerv(GL_RGBA_MODE,&rgbamode);
    glGetIntegerv(GL_INDEX_BITS,&indexbits);

    while (pass--) {
	ogLibClear(0);
	maxcolor = ogLibClearColor();

	x1 = 2 + ogLibIntRand(0,xmax - 4);
	x2 = 2 + ogLibIntRand(0,xmax - 4);

	y1 = 2 + ogLibIntRand(0,ymax - 4);
	y2 = 2 + ogLibIntRand(0,ymax - 4);

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
	if (obj = ogLibBitRand(1))
	    glNewList(1, GL_COMPILE);

	glViewport(x1, y1, (x2) - (x1), (y2) - (y1));
	glScissor(x1, y1, (x2) - (x1), (y2) - (y1));
	glEnable(GL_SCISSOR_TEST);

	if (masking=ogLibBitRand(1)) {
	    unsigned mask,r,g,b,a;
	    /*use a cmask*/
	    if (rgbamode) {
		glColorMask(r=ogLibBitRand(1),g=ogLibBitRand(1),b=ogLibBitRand(1),a=ogLibBitRand(1));
		ogEnvLog(1, "glColorMask(%d, %d, %d, %d);\n",r,g,b,a);
		mask =  (0xff*r << 24) |
			(0xff*g << 16) |
			(0xff*b << 8 ) |
			(0xff*a);
	    } else {
		glIndexMask(mask=ogLibBitRand(indexbits));
		ogEnvLog(1, "glIndexMask(0x%x);\n",mask);
	    }
	    maxcolor &= mask;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	ogEnvLog(1, "glViewport(%d, %d, %d, %d); glScissor(%d, %d, %d, %d);  glEnable(GL_SCISSOR_TEST); glClear(GL_COLOR_BUFFER_BIT);\n", x1, y1, x2 - x1, y2 - y1, x1, y1, x2 - x1, y2 - y1);

	if (obj) {
	    glEndList();
	    glCallList(1);
	    glDeleteLists(1, 1);
	    ogEnvLog(2, "display-list mode\n");
	} else
	    ogEnvLog(2, "immediate mode\n");

	glViewport(0, 0, (xmax) - (0), (ymax) - (0));
	glScissor(0, 0, (xmax) - (0), (ymax) - (0));
	glDisable(GL_SCISSOR_TEST);
	if ((x1 == x2) || (y1 == y2))
	    maxcolor = 0;
	ogLibRectCheck(x1, y1, x2-1, y2-1, maxcolor, 0);
	if (masking) {
	    if (rgbamode) glColorMask(1,1,1,1);
	    else glIndexMask((1<<indexbits)-1);
	}
    }
}

CLEANUP(clear)
{
    int rgbamode, indexbits;
    glGetIntegerv(GL_RGBA_MODE,&rgbamode);
    glGetIntegerv(GL_INDEX_BITS,&indexbits);

    glViewport(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    glScissor(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    glDisable(GL_SCISSOR_TEST);
    ogLibSetDefaultClears();
    if (rgbamode) glColorMask(1,1,1,1);
    else glIndexMask((1<<indexbits)-1);
}

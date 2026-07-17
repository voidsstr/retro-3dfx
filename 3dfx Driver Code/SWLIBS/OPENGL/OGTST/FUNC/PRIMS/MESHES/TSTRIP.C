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

/* tstrip.c - $Revision: 2$ */

/*
 * This program tests Begin(GL_TRIANGLE_STRIP), End() by choosing
 * Vertex[234]*() randomly then drawing a mesh and scaning 4 lines
 *
 * Debug levels : 1 - Begin,End,Vertex* parameters, 2 - display mode and color
 * mode 
 */

#include <stdlib.h>   /* for abs */
#include "ogtst.h"		/* include test environment		 */

TESTMOD(tstrip)
{
    register GLint temp, x1, y1, x2, y2;
    int j, obj, col;
    int xmax, ymax;
    int maxcolor;
    int value, step;
    GLint cimode = ogEnvIsCIMode();

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.,(double) xmax+1., 0., (double) ymax+1., -1.,1.);

    while (pass--) {
	ogLibClear(0);
	maxcolor = ogLibColor();

	do {
	    x1 = 2 + ogLibIntRand(0,xmax - 4);
	    x2 = 2 + ogLibIntRand(0,xmax - 4);
	}
	while (abs(x1 - x2) < 4);

	do {
	    y1 = 2 + ogLibIntRand(0,ymax - 4);
	    y2 = 2 + ogLibIntRand(0,ymax - 4);
	}
	while (abs(y1 - y2) < 4);

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

	ogEnvLog(1, "glBegin(GL_TRIANGLE_STRIP);\n");
	glBegin(GL_TRIANGLE_STRIP);
	ogLibSetVertex(x1, y1, 0, 1);
	ogLibSetVertex(x1, y2, 0, 1);
	ogLibSetVertex(x2, y1, 0, 1);
	ogLibSetVertex(x2, y2, 0, 1);
	glEnd();
	ogEnvLog(1, "glEnd(); \n");

	/* try to draw outside of bgn/end */
	ogLibSetVertex(0, 0, 0, 1);

	if (obj) {
	    glEndList();
	    glCallList(1);
	    glDeleteLists(1,1);
	    ogEnvLog(2, "display-list mode\n");
	} else
	    ogEnvLog(2, "immediate mode\n");

	ogLibRectCheck(x1, y1, x2 - 1, y2 - 1, maxcolor, 0);
    }

    ogLibClear(0);

    if (xmax > ymax)
	value = ymax - 20;
    else
	value = xmax - 20;
    
    x1 = y1 = 20;
    x2 = y2 = value;
    step = (value - 20)/10;
    
    ogEnvLog(1, "final immediate mode test\n");
    for (j = 0; j <= 8; x2 -= step, y2 -= step, j++){ 
	ogLibClear(0);
    
	if (cimode) {
	    col = ogLibColor();
	    ogEnvLog(1, "glIndexi(%d); \n", j);
	} else {
	    col = (j << 20) + (j << 12) + (j << 4);
	    glColor4ub((col>>24)&0xff, (col>>16)&0xff, (col>>8)&0xff, (col)&0xff);
	    ogEnvLog(1, "glColor4ub(0x%0x);\n", col);
	}

	ogEnvLog(1, "glBegin(GL_TRIANGLE_STRIP); \n");
	glBegin(GL_TRIANGLE_STRIP);
	ogLibSetVertex(x1, y1, 0, 1);
	ogLibSetVertex(x1, y2, 0, 1);
	ogLibSetVertex(x2, y1, 0, 1);
	ogLibSetVertex(x2, y2, 0, 1);
	glEnd();
	ogEnvLog(1, "glEnd(); \n");

	ogLibRectCheck(x1, y1, x2 - 1, y2 - 1, col, 0);
    }
}

CLEANUP(tstrip) {
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();
}

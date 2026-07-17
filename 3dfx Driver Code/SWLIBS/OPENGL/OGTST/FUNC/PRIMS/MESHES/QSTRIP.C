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

/* qstrip.c - $Revision: 2$ */

/*
 * This program tests Begin(GL_QUAD_STRIP), End() by choosing Vertex[234]*()
 * calls randomly then draws mesh and scans 4 lines, then erases the results. 
 *
 * Debug levels : 1 - Begin,End,Vertex* parameters 2 - display mode and
 * color mode 
 */

#include <stdlib.h>   /* for abs */
#include "ogtst.h"		/* include test environment		 */

TESTMOD(qstrip)
{
    register GLint temp, x1, y1, x2, y2;
    int obj;
    int xmax, ymax;
    int maxcolor;

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

	START_DL_OR_IM(1);

	ogEnvLog(1, "glBegin(GL_QUAD_STRIP);\n");
	glBegin(GL_QUAD_STRIP);
	ogLibSetVertex(x1, y1, 0, 1);
	ogLibSetVertex(x1, y2, 0, 1);
	ogLibSetVertex(x2, y1, 0, 1);
	ogLibSetVertex(x2, y2, 0, 1);
	glEnd();
	ogEnvLog(1, "glEnd(); \n");

	FINIS_DL_OR_IM(1);

	/* check filled pixels */
	ogLibRectCheck(x1, y1, x2 - 1, y2 - 1, maxcolor, 0);
    }
}

CLEANUP(qstrip) {
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();
}

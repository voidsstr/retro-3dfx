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

/* viewport.c - $Revision: 2$ */

/*
 * This program tests the viewport/scissor command by drawing rectangles
 * larger than random viewports and making sure they get clipped properly. 
 *
 * Debug levels: 1 - print the scissor and clear parameters 2 - print
 * display-list/immediate mode 5 - print checking trace 
 */

#include <stdlib.h>   /* for abs */
#include "ogtst.h"		/* include test environment		 */
#include "xform.h"		/* include test environment		 */

TESTMOD(viewport) {
    GLint x1, y1, width, height;
    int obj, maxcolor,col;
    int xmax, ymax;
    float rllx, rlly, rurx, rury;

    xmax = ogEnvQuery(OG_XWSIZE);
    ymax = ogEnvQuery(OG_YWSIZE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, xmax, 0, ymax, -1, 1);

    while (pass--) {
	glEnable(GL_SCISSOR_TEST);
	glViewport(0, 0, xmax, ymax);
	glScissor(0, 0, xmax, ymax);
	maxcolor = ogLibColor();
	ogLibClear(maxcolor);

        x1 = ogLibIntRand(2, xmax - 3);
        width = ogLibIntRand(1, xmax - x1 - 1);

	y1 = ogLibIntRand(2, ymax - 2);
        height = ogLibIntRand(1, ymax - y1 - 1);

        START_DL_OR_IM(1);

	ogEnvLog(1, "glViewport(%d,%d,%d,%d); glScissor(%d, %d, %d, %d);\n",
                 x1, y1, width, height, x1, y1, width, height);
	glViewport(x1, y1, width, height);
	glScissor(x1, y1, width, height);
	glLoadIdentity();
	glOrtho(x1, x1 + width, y1, y1 + height, -1., 1.);

        FINIS_DL_OR_IM(1);

	col = ogLibColor();

	rllx = -105.23 + ogLibExpRand(2);
	rlly = -123.54 + ogLibExpRand(2);
	rurx = 13678.93 + xmax + ogLibExpRand(4) - 1;
	rury = 1.12 + ymax + ogLibExpRand(0) - 1;
	glRectf(rllx, rlly, rurx, rury);
	ogEnvLog(1, "glRectf(%g,  %g,  %g,  %g)\n", rllx, rlly, rurx, rury);

	/* check the pixels */
	ogLibRectCheck(x1, y1, x1 + width - 1, y1 + height - 1, col, maxcolor);
    }
}

CLEANUP(viewport)
{
    int xmax = ogEnvQuery(OG_XWSIZE);
    int ymax = ogEnvQuery(OG_YWSIZE);

    glViewport(0, 0, xmax, ymax);
    glScissor(0, 0, xmax, ymax);
    glDisable(GL_SCISSOR_TEST);
    ogLibSetDefaultColors();
    ogLibSetDefaultClears();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

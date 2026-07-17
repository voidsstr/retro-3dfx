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

/* rect.c - $Revision: 2$ */

/*
 * This program tests rect by drawing random rectangles and then checking the
 * output. 
 *
 * Debug levels : 1 - rect parameters 2 - display mode, color mode 
 */

#include <stdlib.h>
#include "ogtst.h"		/* include test environment		 */

static int set_rect(int, int, int, int);

TESTMOD(rect) {
    register GLint x1, x2, y1, y2;
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

	ogEnvLog(1, "----- degenerate case: point -----\n");
	set_rect(x1, y1, x1, y1);
	ogLibRectCheck(x1, y1, x1, y1, 0, 0);

	ogEnvLog(1, "----- degenerate case: horizontal line -----\n");
	set_rect(x1, y2, x2, y2);
	ogLibRectCheck(x1, y2, x2, y2, 0, 0);
	ogLibClear(0);

	ogEnvLog(1, "----- degenerate case: vertical line -----\n");
	set_rect(x1, y1, x1, y2);
	ogLibRectCheck(x1, y1, x1, y2, 0, 0);
	ogLibClear(0);

	ogEnvLog(1, "----- normal rect case -----\n");
	maxcolor = ogLibColor();
	set_rect(x1, y1, x2, y2);
	/* half open */
	if (x1 > x2) x1--; else x2--;
	if (y1 > y2) y1--; else y2--;
	ogLibRectCheck(x1, y1, x2, y2, maxcolor, 0);
    }
}

CLEANUP(rect) {
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();
}

/*************************************************************
*  set_rect()  -  set rectangle position
*************************************************************/
static int
set_rect(int x1, int y1, int x2, int y2)
{
    /* register */ short ts[4];
    /* register */ int ti[4];
    /* register */ float tf[4];
    /* register */ double td[4];
    register int obj = 0;

    if (!glIsList(1))
        if (obj = ogLibBitRand(1)) {
            ogEnvLog(2, "building display-list....\n");
            glNewList(1, GL_COMPILE);
        }
    ts[0] = (short) x1;
    ts[1] = (short) y1;
    ts[2] = (short) x2;
    ts[3] = (short) y2;
    ti[0] = (int) x1;
    ti[1] = (int) y1;
    ti[2] = (int) x2;
    ti[3] = (int) y2;
    tf[0] = (float) x1;
    tf[1] = (float) y1;
    tf[2] = (float) x2;
    tf[3] = (float) y2;
    td[0] = (double) x1;
    td[1] = (double) y1;
    td[2] = (double) x2;
    td[3] = (double) y2;

    switch (ogLibIntRand(0,7)) {
    case 0:
        glRects(ts[0], ts[1], ts[2], ts[3]);
        ogEnvLog(1, "glRects(%d, %d, %d, %d);\n", ts[0], ts[1], ts[2], ts[3]);
        break;
    case 1:
        glRecti(ti[0], ti[1], ti[2], ti[3]);
        ogEnvLog(1, "glRecti(%d, %d, %d, %d);\n", ti[0], ti[1], ti[2], ti[3]);
        break;
    case 2:
        glRectf(tf[0], tf[1], tf[2], tf[3]);
        ogEnvLog(1, "glRectf(%f, %f, %f, %f);\n", tf[0], tf[1], tf[2], tf[3]);
        break;
    case 3:
        glRectd(td[0], td[1], td[2], td[3]);
        ogEnvLog(1, "glRectd(%f, %f, %f, %f);\n", td[0], td[1], td[2], td[3]);
        break;

    case 4:
        glRectsv(ts, ts + 2);
        ogEnvLog(1, "glRectsv(%d, %d, %d, %d);\n", ts[0], ts[1], ts[2], ts[3]);
        break;
    case 5:
        glRectiv(ti, ti + 2);
        ogEnvLog(1, "glRectiv(%d, %d, %d, %d);\n", ti[0], ti[1], ti[2], ti[3]);
        break;
    case 6:
        glRectfv(tf, tf + 2);
        ogEnvLog(1, "glRectfv(%f, %f, %f, %f);\n", tf[0], tf[1], tf[2], tf[3]);
        break;
    case 7:
        glRectdv(td, td + 2);
        ogEnvLog(1, "glRectdv(%f, %f, %f, %f);\n", td[0], td[1], td[2], td[3]);
        break;
    }

    if (obj) {
        glEndList();
        glCallList(1);
        glDeleteLists(1, 1);
        ogEnvLog(2, "ending display-list mode\n");
    }
    return 0;
}


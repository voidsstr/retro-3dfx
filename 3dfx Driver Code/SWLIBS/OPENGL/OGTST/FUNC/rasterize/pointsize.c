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

/* pointsize.c - $Revision: 2$ */

/*
 * This program tests Begin(GL_POINTS), End() by choosing vertex2* vertex3*
 * or vertex4* on random vertex points in x and y direction then reads back
 * pixels 
 *
 * Debug levels : 1 - begin, end, v2 or v3 parameters 2 - display list mode,
 * color mode 
 */

#include "ogtst.h"		/* include test environment		 */


TESTMOD(pointsize) {

    int vx, vy, ips;
    int xmax, ymax;
    int maxcolor,obj;
    float ps, xps;
    GLfloat maxsize, v[2];

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    glGetFloatv(GL_POINT_SIZE_RANGE,v);
    maxsize = v[1] < 100. ? v[1] : 100.;
    ogEnvLog(3, "glGetFloatv(GL_POINT_SIZE_RANGE,(%f,%f));\n", v[0], v[1]);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., ogEnvQuery(OG_XWSIZE), 0., ogEnvQuery(OG_YWSIZE), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    while (pass--) {
	ogLibClear(0);
	maxcolor = ogLibColor();

	vx = ogLibIntRand(5,xmax-5);
	vy = ogLibIntRand(5,ymax-5);

	do {
	    xps = ps = ogLibFloatRand(-maxsize, maxsize);
	    if (xps < 0)
		xps = 1.;
	    ips = xps +.5;
	    xps *=.5;
	} while (xps + vx > xmax - 1 || vx - xps < 2 || vy + xps > ymax - 1 || vy - xps < 2 || xps < 1.5);

	START_DL_OR_IM(1);

	glPointSize(ps);
	ogEnvLog(1, "glPointSize(%f);\n", ps);

	ogEnvLog(1, "glBegin(GL_POINTS);  \n");
	glBegin(GL_POINTS);
	ogLibSetVertex(vx, vy, 0, 1);
	glEnd();
	ogEnvLog(1, "glEnd();  \n");

	/* try to draw outside bgn/end */
	ogLibSetVertex(vx + 5, vy + 5, 0, 1);

	FINIS_DL_OR_IM(1);

	/* check pixels around the point */
	if (ips & 1) {
	    /* odd width */
	    ogLibRectCheck(vx - ips / 2, vy - ips / 2, vx + ips / 2, vy + ips / 2, maxcolor, 0);
	} else {
	    /* even width */
	    ogLibRectCheck(vx - ips / 2, vy - ips / 2, vx + ips / 2 - 1, vy + ips / 2 - 1, maxcolor, 0);
	}
    }
}

CLEANUP(pointsize)
{
    glPointSize(1.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();

    ogEnvMultiSamplingState(1);
}

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

/* point.c - $Revision: 2$ */

/*
 * This program tests Begin(GL_POINTS), End() by choosing vertex2* vertex3*
 * or vertex4* on random vertex points in x and y direction then reads back 
 * pixels
 * 
 *      Debug levels :
 *        1 - begin, end, v2 or v3 parameters
 *        2 - display list mode, color mode 
 */

#include "ogtst.h"	/* include test environment		*/

TESTMOD(point) {
    GLint vx, vy;
    int i, obj;
    int xmax, ymax, xOff;
    int   maxcolor;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.,(double) ogEnvQuery(OG_XWSIZE), 0.,
            (double) ogEnvQuery(OG_YWSIZE), -1.,1.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (ogEnvIsMultiSampled()) {
        /* Need more separation as multisampled points cover multiple pixels */
        xOff = 3;
        xmax = ogEnvQuery(OG_XWSIZE) - 17;
    } else {
        xOff = 2;
        xmax = ogEnvQuery(OG_XWSIZE) - 16;
    }
    ymax = ogEnvQuery(OG_YWSIZE) - 16;

    while (pass--) {
        ogLibClear(0) ;
        maxcolor = ogLibColor() ;

	vx = 2 + ogLibIntRand(0,xmax);
	vy = 2 + ogLibIntRand(0,ymax) ;

        if (obj = ogLibBitRand(1))
            glNewList(1, GL_COMPILE);

	ogEnvLog(1, "glBegin(GL_POINTS);  \n");
        glBegin(GL_POINTS);
	for (i=0; i<=9; i++) {
	    ogLibSetVertex(vx, vy + i,0,1);
	    ogLibSetVertex(vx + xOff + i, vy,0,1);
	}
	glEnd();
	ogEnvLog(1, "glEnd();  \n");

        /* try to draw outside bgn/end */ 
        ogLibSetVertex(vx+5,vy+5,0,1);

        if (obj) {
            glEndList();
            glCallList(1);
            glDeleteLists( 1, 1 );
            ogEnvLog(2, "display-list mode\n");
        }
        else ogEnvLog(2, "immediate mode\n");

	/* check pixels around the point */
        ogLibRectCheck(vx+2,vy,vx+11,vy,maxcolor,0) ;
        ogLibRectCheck(vx,vy,vx,vy+9,maxcolor,0) ;
        ogLibRectCheck(vx+5,vy+5,vx+5,vy+5,0,0) ;
    }
}

CLEANUP(point) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ogLibSetDefaultColors();
    ogEnvMultiSamplingState(1);
}

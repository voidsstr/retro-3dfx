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

/* line.c - $Revision: 2$ */

/*
 * This program tests lines, by choosing vertex2* vertex3* or vertex4*
 * as random vertex points and drawing points, vert or hor lines
 * and inverted 'U' and then checks pixels on drawn ones and around.
 * 
 *      Debug levels :
 *        1 - begin, end, vertex* parameters
 *        2 - display mode, color mode
 */

#include <stdlib.h>  /* for abs */
#include "ogtst.h"	/* include test environment		*/

TESTMOD(line)
{
    int temp,x1,y1,x2,y2;
    int obj;
    GLint xmax, ymax;
    int   maxcolor;
    float fx1,fx2,fy1,fy2;

    glMatrixMode(GL_PROJECTION);
    /*
     * Set the projection so that integer coordinate are at the center of
     * pixels
     */

   glOrtho(-0.5,(double) ogEnvQuery(OG_XWSIZE) - 0.5, -0.5,
	(double) ogEnvQuery(OG_YWSIZE) - 0.5, -1.,1.);

    glMatrixMode(GL_MODELVIEW);
    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    while (pass--) {
        ogLibClear(0) ;
        maxcolor = ogLibColor() ;

	x1 = ogLibIntRand(2,xmax - 2);
	y1 = ogLibIntRand(2,ymax - 2);

	ogEnvLog(1, "---point test---\n");
        START_DL_OR_IM(1);
	ogEnvLog(1, "glBegin(GL_LINES);  \n");
	glBegin(GL_LINES);	/* shouldn't draw anything */
	ogLibSetVertex(x1,y1,0,1);
	ogLibSetVertex(x1,y1,0,1);
	glEnd();
	ogEnvLog(1, "glEnd(); \n");
	FINIS_DL_OR_IM(1);

	/* check pixels around the point */
        ogLibRectCheck(x1,y1,x1,y1,0,0) ;

        ogLibClear(0) ;
        maxcolor = ogLibColor() ;

	x1 = ogLibIntRand(2,xmax - 2);
	y1 = ogLibIntRand(2,ymax - 2);

	x2 = ogLibIntRand(2,xmax - 2);
	y2 = ogLibIntRand(2,ymax - 2); 

	ogEnvLog(1, "--- horizontal line test---\n");
	START_DL_OR_IM(1);

	ogEnvLog(1, "glBegin(GL_LINES); \n");
	glBegin(GL_LINES);
	ogLibSetVertex(x1,y1,0,1);
	ogLibSetVertex(x2,y1,0,1);
	/* send an extra vertex which shouldn't be drawn */
	ogLibSetVertex(0,0,0,1);
	glEnd();
	ogEnvLog(1, "glEnd(); \n");
	FINIS_DL_OR_IM(1);

	/* check pixels */
	if (x1 < x2)
	    ogLibRectCheck(x1,y1,x2-1,y1,maxcolor,0);
	else if (x1 > x2) 
	    ogLibRectCheck(x2+1,y1,x1,y1,maxcolor,0);
	else
	    ogLibRectCheck(x2,y1,x1,y1,0,0);

        ogLibClear(0) ;
        maxcolor = ogLibColor() ;

	ogEnvLog(1, "--- vertical line test---\n");
	START_DL_OR_IM(1);
        ogEnvLog(1, "glBegin(GL_LINES); \n");
	glBegin(GL_LINES);
	ogLibSetVertex(x1,y1,0,1);
	ogLibSetVertex(x1,y2,0,1);
	/* send an extra vertex which shouldn't be drawn */
	ogLibSetVertex(0,0,0,1);
	glEnd();
	ogEnvLog(1, "glEnd(); \n");
	FINIS_DL_OR_IM(1);


	/* check pixels */
	if (y1 < y2)
	    ogLibRectCheck(x1,y1,x1,y2-1,maxcolor,0);
	else if (y1 > y2)
	    ogLibRectCheck(x1,y2+1,x1,y1,maxcolor,0);
	else
	    ogLibRectCheck(x1,y2,x1,y1,0,0);

        ogLibClear(0) ;
        maxcolor = ogLibColor() ;

        do {
            x1 = ogLibIntRand(2,xmax-2);
            x2 = ogLibIntRand(2,xmax-2);
        } while (abs(x1 - x2) < 4);

        do {
            y1 = ogLibIntRand(2,ymax-2);
            y2 = ogLibIntRand(2,ymax-2);
        } while (abs(y1 - y2) < 4);

        if (x1 < x2) {
	    fx1 = x1 + 0.0 - 0.5;
	    fx2 = x2 - 0.35 - 0.5;
  	} else { 
	    fx1 = x1 - 0.35 - 0.5;
	    fx2 = x2 + 0.0 - 0.5;
	}
	if (y1 < y2) {
	    fy1 = y1 + 0.0 - 0.5;
	    fy2 = y2 - 0.35 - 0.5;
  	} else { 
	    fy1 = y1 - 0.35 - 0.5;
	    fy2 = y2 + 0.0 - 0.5;
	}

	ogEnvLog(1, "---hollow box test---\n");
	START_DL_OR_IM(1);

	ogEnvLog(1, "glBegin(GL_LINES); \n");
	glBegin(GL_LINES);
	glVertex2f(fx1,fy1);
	glVertex2f(fx1,fy2);
	glVertex2f(fx1,fy2);
	glVertex2f(fx2,fy2);
	glVertex2f(fx2,fy2);
	glVertex2f(fx2,fy1);
	glVertex2f(fx2,fy1);
	glVertex2f(fx1,fy1);

	glEnd();
	ogEnvLog(1, "glEnd(); \n");

        /* try to draw outside of bgn/end */
	ogLibSetVertex(x1,y1,0,1);

	FINIS_DL_OR_IM(1);

	if (x1 > x2) { temp = x1; x1 = x2; x2 = temp; }
	if (y1 > y2) { temp = y1; y1 = y2; y2 = temp; }

	ogLibRectEdgeCheck(x1,y1,x2-1,y2-1,maxcolor,0); 
    }
}

CLEANUP(line)
{
    ogLibSetDefaultColors();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


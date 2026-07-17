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

/* tfans.c - $Revision: 2$ */

/*
 * This program tests basic triangle fan functionality.
 * 
 *      Debug levels :
 *        1 - begin, end, vertex* parameters
 *        2 - display mode, color mode
 */

#include "ogtst.h"	/* include test environment		*/

static void  tfan_box(int,int,int,int);
static void extras(void);
static GLboolean CImode;

TESTMOD(tfans)
{
    GLint temp,x1,y1,x2,y2;
    int i, obj;
    GLint xmax, ymax;
    int   maxcolor;

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.,(double) xmax+1., 0., (double) ymax+1., -1.,1.);

    CImode = !ogEnvCurVisualInfo(GLX_RGBA);

    /* 
     * try to catch double hits! 
     */
    if (CImode) 
	glLogicOp(GL_XOR);
    else
	glBlendFunc(GL_ONE,GL_ONE);
    
    

    while (pass--) {
        ogLibClear(0) ;
        maxcolor = ogLibColor() ;

	x1 = 2 + ogLibIntRand(0,xmax - 4);
	y1 = 2 + ogLibIntRand(0,ymax - 4);

	ogEnvLog(1, "---vertex alone test---\n");
        START_DL_OR_IM(1);

	ogLibSetVertex(x1,y1,0,1);
	ogEnvLog(1, "glBegin(GL_TRIANGLE_FAN);  \n");
	glBegin(GL_TRIANGLE_FAN);	/* draw nothing */
	glEnd();
	ogEnvLog(1, "glEnd(); \n");

        FINIS_DL_OR_IM(1);

	/* check pixels around the point */
        ogLibRectCheck(x1,y1,x1,y1,0,0) ;

        ogLibClear(0) ;
        maxcolor = ogLibColor() ;

	x1 = ogLibIntRand(2, xmax - 2);
	y1 = ogLibIntRand(2, ymax - 2);
	x2 = ogLibIntRand(2, xmax - 2);
	y2 = ogLibIntRand(2, ymax - 2);

	ogEnvLog(1, "---zero area test---\n");
        START_DL_OR_IM(1);

	ogEnvLog(1, "glBegin(GL_TRIANGLE_FAN);  \n");
	glBegin(GL_TRIANGLE_FAN);	/* draw nothing */
	ogLibSetVertex(x1,y1,0,1);
	ogLibSetVertex(x2,y2,0,1);
	ogLibSetVertex(x1,y1,0,1);
	glEnd();
	ogEnvLog(1, "glEnd(); \n");

        FINIS_DL_OR_IM(1);

	/* check pixels around the point */
        ogLibRectCheck(x1,y1,x1,y1,0,0) ;

	for (i=0; i<10; i++) {
	    ogEnvLog(1, "---many triangle tfan test---\n");
            ogLibClear(0) ;
	    maxcolor = ogLibColor() ;
	    do {
		x1 = 2 + ogLibIntRand(0,xmax - 4);
		x2 = 2 + ogLibIntRand(0,xmax - 4);
	    } while ((x1 > x2 ? (x1-x2 < 3) : (x2-x1 < 3)));
	    do {
		y1 = 2 + ogLibIntRand(0,ymax - 4);
		y2 = 2 + ogLibIntRand(0,ymax - 4);
	    } while ((y1 > y2 ? (y1-y2 < 3) : (y2-y1 < 3)));

            START_DL_OR_IM(1);
	    tfan_box(x1,y1,x2,y2);
            FINIS_DL_OR_IM(1);

	    if (x1 > x2) { temp = x1; x1 = x2; x2 = temp; }
	    if (y1 > y2) { temp = y1; y1 = y2; y2 = temp; }
	    ogEnvLog(1,"box coords: (%d,%d), (%d,%d)\n", x1,y1,x2,y2);
	    /* check the edges */
	    ogLibRectCheck(x1,y1,x2-1,y2-1,maxcolor,0);
	}
    }
}

CLEANUP(tfans)
{
    ogLibSetDefaultColors();
    if (CImode) 
	glLogicOp(GL_COPY);
    else
	glBlendFunc(GL_ONE,GL_ZERO);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
}

static void
extras(void)
{
    int n,i;

    n = ogLibIntRand(0,2); 
    for (i=0; i<n; i++) 
	glVertex2i(ogLibBitRand(31),ogLibBitRand(31)); 
}

static void
tfan_box(int x0,int y0,int x1,int y1)
{
    int n,i;
    float x,y,xf,yf;

    ogEnvLog(1, "glBegin(GL_TRIANGLE_FAN); \n");
    glBegin(GL_TRIANGLE_FAN);
    /* specify the starting vertex */
    ogLibSetVertex(x0,y0,0,1);
    /*
     * draw the left edge of the box, even though it is zero area 
     */
    
    n = ogLibIntRand(1,5);
    x = x0;
    y = y0;
    yf = y1;
    for (i=0; i<n; i++) {
	if (i == n-1) {
	    y = yf;
        } else {
	    y = y <= yf ? ogLibFloatRand(y,yf) : ogLibFloatRand(yf,y);
	}
	glVertex2f(x,y);
    }

    /*
     * draw the top edge of the box
     */
    
    n = ogLibIntRand(1,500);
    x = x0;
    y = y1;
    xf = x1;
    for (i=0; i<n; i++) {
	if (i == n-1) {
	    x = xf;
        } else {
	    x = x <= xf ? ogLibFloatRand(x,xf) : ogLibFloatRand(xf,x);
	}
	glVertex2f(x,y);
    }

    /*
     * draw the right edge of the box
     */
    
    n = ogLibIntRand(1,500);
    x = x1;
    y = y1;
    yf = y0;
    for (i=0; i<n; i++) {
	if (i == n-1) {
	    y = yf;
        } else {
	    y = y <= yf ? ogLibFloatRand(y,yf) : ogLibFloatRand(yf,y);
	}
	glVertex2f(x,y);
    }

    /*
     * draw the bottom edge of the box, even though it is zero area
     */
    
    n = ogLibIntRand(1,5);
    x = x1;
    y = y0;
    xf = x0;
    for (i=0; i<n; i++) {
	if (i == n-1) {
	    x = xf;
        } else {
	    x = x <= xf ? ogLibFloatRand(x,xf) : ogLibFloatRand(xf,x);
	}
	glVertex2f(x,y);
    }
    glEnd();
    ogEnvLog(1, "glEnd(); \n");
    extras();
}


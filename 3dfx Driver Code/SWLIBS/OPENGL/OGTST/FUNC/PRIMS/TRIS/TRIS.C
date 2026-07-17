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

/* tris.c - $Revision: 2$ */

/*
 * This program tests basic triangle functionality.
 * 
 *      Debug levels :
 *        1 - begin, end, vertex* parameters
 *        2 - display mode, color mode
 */

#include "ogtst.h"	/* include test environment		*/

static void tri_box(int,int,int,int);
static void extras(void);
static void random_end(void);
static GLboolean CImode;

TESTMOD(tris)
{
    GLint temp,x1,y1,x2,y2;
    int i, obj;
    GLint xmax, ymax;
    int   maxcolor;

    glMatrixMode(GL_PROJECTION);
    glOrtho(0.,(double) ogEnvQuery(OG_XWSIZE), 0., (double) ogEnvQuery(OG_YWSIZE), -1.,1.);
    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

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
        if (obj = ogLibBitRand(1))
            glNewList(1, GL_COMPILE);

	ogLibSetVertex(x1,y1,0,1);
	ogEnvLog(1, "glBegin(GL_TRIANGLES);  \n");
	glBegin(GL_TRIANGLES);	/* draw nothing */
	glEnd();
	ogEnvLog(1, "glEnd(); \n");

        if (obj) {
            glEndList();
            glCallList(1);
            glDeleteLists( 1, 1 );
            ogEnvLog(2, "display-list mode\n");
        }
        else ogEnvLog(2, "immediate mode\n");

	/* check pixels around the point */
        ogLibRectCheck(x1,y1,x1,y1,0,0) ;

        ogLibClear(0) ;
        maxcolor = ogLibColor() ;

	x1 = 2 + ogLibIntRand(0,xmax - 4);
	y1 = 2 + ogLibIntRand(0,ymax - 4);
	x2 = 2 + ogLibIntRand(0,xmax - 4);
	y2 = 2 + ogLibIntRand(0,ymax - 4);

	ogEnvLog(1, "---zero area test---\n");
        if (obj = ogLibBitRand(1))
            glNewList(1, GL_COMPILE);

	ogEnvLog(1, "glBegin(GL_TRIANGLES);  \n");
	glBegin(GL_TRIANGLES);	/* draw nothing */
	ogLibSetVertex(x1,y1,0,1);
	ogLibSetVertex(x2,y2,0,1);
	ogLibSetVertex(x1,y1,0,1);
	glEnd();
	ogEnvLog(1, "glEnd(); \n");

        if (obj) {
            glEndList();
            glCallList(1);
            glDeleteLists( 1, 1 );
            ogEnvLog(2, "display-list mode\n");
        }
        else ogEnvLog(2, "immediate mode\n");

	/* check pixels around the point */
        ogLibRectCheck(x1,y1,x1,y1,0,0) ;

	for (i=0; i<10; i++) {
	    ogEnvLog(1, "---many triangle test---\n");
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
            if (obj = ogLibBitRand(1))
                glNewList(1, GL_COMPILE);
	    tri_box(x1,y1,x2,y2);
            if (obj) {
		glEndList();
		glCallList(1);
		glDeleteLists( 1, 1 );
		ogEnvLog(2, "display-list mode\n");
	    } else ogEnvLog(2, "immediate mode\n");

	    if (x1 > x2) { temp = x1; x1 = x2; x2 = temp; }
	    if (y1 > y2) { temp = y1; y1 = y2; y2 = temp; }
	    ogEnvLog(1,"box coords: (%d,%d), (%d,%d)\n", x1,y1,x2,y2);
	    /* check the edges */
	    ogLibRectCheck(x1,y1,x2-1,y2-1,maxcolor,0);
	}
    }
}

CLEANUP(tris)
{
    if (CImode) 
	glLogicOp(GL_COPY);
    else
	glBlendFunc(GL_ONE,GL_ZERO);
    ogLibSetDefaultColors();
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
tri_box(int x0,int y0,int x1,int y1)
{
    float tx0,tx1,ty;
    float mx0,mx1,my;
    float bx0,bx1,by;
    int n,i;

    n = ogLibIntRand(1,1000);
    tx0 = mx0 = bx0 = x0; 
    ty = y0;
    by = y1;
    my = ty <= by ? ogLibFloatRand(ty,by) : ogLibFloatRand(by,ty);

    ogEnvLog(1, "glBegin(GL_TRIANGLES); \n");
    glBegin(GL_TRIANGLES);
    for (i=0; i<n; i++) {
	if (i == n-1) {
	    tx1 = x1;
	    mx1 = x1;
	    bx1 = x1;
        } else {
	    tx1 = tx0 <= x1 ? ogLibFloatRand(tx0,x1) : ogLibFloatRand(x1,tx0);
	    mx1 = mx0 <= x1 ? ogLibFloatRand(mx0,x1) : ogLibFloatRand(x1,mx0);
	    bx1 = bx0 <= x1 ? ogLibFloatRand(bx0,x1) : ogLibFloatRand(x1,bx0);
	}

	glVertex2f(tx0,ty);
	glVertex2f(mx0,my);
	glVertex2f(tx1,ty);
	random_end();
	glVertex2f(tx1,ty);
	glVertex2f(mx1,my);
	glVertex2f(mx0,my);
	random_end();
		
	glVertex2f(bx0,by);
	glVertex2f(mx0,my);
	glVertex2f(bx1,by);
	random_end();
	glVertex2f(bx1,by);
	glVertex2f(mx1,my);
	glVertex2f(mx0,my);
	if (i == n-1) {
	    ogEnvLog(1, "glEnd(); \n");
	    glEnd();
	} else
	    random_end();

	tx0 = tx1; mx0 = mx1; bx0 = bx1;
    }
}

static void
random_end(void)
{
    if (ogLibIntRand(0,127) == 0) {
	extras();
	ogEnvLog(1, "glEnd(); \n");
	glEnd();
	extras();
	ogEnvLog(1, "glBegin(GL_TRIANGLES); \n");
	glBegin(GL_TRIANGLES);
    }
}

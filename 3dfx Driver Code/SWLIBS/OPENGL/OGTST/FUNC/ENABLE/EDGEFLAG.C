/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1994, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************/


#include "ogtst.h"

static void checkEdgeFlag(const GLboolean val, const char *name) {
    GLboolean btmp;
    GLdouble dtmp;
    GLfloat ftmp;
    GLint itmp;
    GLboolean fail;

    glGetBooleanv(GL_EDGE_FLAG, &btmp);
    glGetDoublev(GL_EDGE_FLAG, &dtmp);
    glGetFloatv(GL_EDGE_FLAG, &ftmp);
    glGetIntegerv(GL_EDGE_FLAG, &itmp);
    fail = 0;
    if (val) {
	fail = fail || !btmp;
	fail = fail || !dtmp;
	fail = fail || !ftmp;
	fail = fail || !itmp;
    } else {
	fail = fail || btmp;
	fail = fail || dtmp;
	fail = fail || ftmp;
	fail = fail || itmp;
    }
    if (fail) {
	ogEnvLog(OG_LFAIL, "%s or glGet(GL_EDGE_FLAGV) not working\n",
		 name);
    }
}    

TESTMOD(edgeflag)
{
    GLboolean flags[50];
    int xmax, ymax;
    GLuint col;
    GLint x0, y0, x1, y1, x2, y2;
    int i, imax;

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    glMatrixMode(GL_PROJECTION);
    glOrtho(.5, xmax, .5, ymax, -1, 1);

    imax = ymax / 5;
    if (imax > 50) imax = 50;

    glLineWidth(3);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (pass--) {
	ogLibClear(0);

	for (i = 0; i < imax; i++) flags[i] = ogLibBitRand(1); 
	
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < imax - 3; i++) {
	    glEdgeFlag(ogLibBitRand(1));
	    glEdgeFlag(flags[i]);
	    checkEdgeFlag(flags[i], "glEdgeFlag");
	    glEdgeFlag(ogLibBitRand(1));
	    glEdgeFlagv(&flags[i]);
	    checkEdgeFlag(flags[i], "glEdgeFlagv");
	    col = ogLibColor();

	    /* every fourth time, draw a square and check the results */
	    if (i%4 == 0) {
		x0 = xmax/2;
		y0 = ymax/2;
		x1 = xmax/2 - ((i+1)*4);
		y1 = ymax/2 - ((i+1)*4);
		x2 = xmax/2 + ((i+1)*4);
		y2 = ymax/2 + ((i+1)*4);
		glBegin(GL_POLYGON);
		glEdgeFlag(flags[i]);
		glVertex2f(x1, y1);
		glEdgeFlag(flags[i+1]);
		glVertex2f(x2, y1);
		glEdgeFlag(flags[i+2]);
		glVertex2f(x2, y2);
		glEdgeFlag(flags[i+3]);
		glVertex2f(x1, y2);
		glEnd();
		
		/* bottome edge */
		ogLibPixelCheck(x0, y1, flags[i] ? col : 0);
		/* right edge */
		ogLibPixelCheck(x2, y0, flags[i+1] ? col : 0);
		/* top edge */
		ogLibPixelCheck(x0, y2, flags[i+2] ? col : 0);
		/* left edge */
		ogLibPixelCheck(x1, y0, flags[i+3] ? col : 0);
	    }
	}
    }
}

CLEANUP(edgeflag)
{
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLineWidth(1);
    glEdgeFlag(1);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    ogLibSetDefaultColors();
}
    

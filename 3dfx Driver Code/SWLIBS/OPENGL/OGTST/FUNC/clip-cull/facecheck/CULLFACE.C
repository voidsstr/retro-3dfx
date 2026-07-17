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

/* cullface.c - $Revision: 2$ */

/*
 * This program tests backface removal by drawing random rectangles and
 * polygons and then checking a pixel within the area. 
 *
 * Debug levels: 1 - print the trace 2 - print display-list/immediate mode 
 */

#include "ogtst.h"		/* include test environment		 */

static GLboolean rgbMode;

TESTMOD(cullface)
{
    GLint llx, lly, urx, ury;
    register GLint c;
    int clockwise, obj;
    register int bf, cw;
    int xmaxscrn, ymaxscrn;

    xmaxscrn = ogEnvQuery(OG_XWSIZE) - 1;
    ymaxscrn = ogEnvQuery(OG_YWSIZE) - 1;

    rgbMode = ogEnvCurVisualInfo(GLX_RGBA); /* check if it is an rgb visual */
    
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.,(double) xmaxscrn+1., 0., (double) ymaxscrn+1., -1.,1.);

    if(rgbMode)
	glClearColor(0.0, 0.0, 0.0, 0.0);
    else
	glClearIndex(0.);
    glClear(GL_COLOR_BUFFER_BIT);
#if 0
    c = maxci();
    /*
     * OGLXXX mapcolor not supported -- See Window Manager mapcolor
     * (c,144,144,144) 
     */
     /* DELETED */ ;
#endif
    c = ogLibColor();

    glCullFace(GL_BACK);

    while (pass--) {
	START_DL_OR_IM(1);
    	(cw = ogLibBitRand(1)) ? glFrontFace(GL_CCW) : glFrontFace(GL_CW);
	if (cw)
	    ogEnvLog(1, "glFrontFace(GL_CCW)\n");
	else
	    ogEnvLog(1, "glFrontFace(GL_CW)\n");
	
	(bf = ogLibBitRand(1)) ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
	if (bf)
	    ogEnvLog(1, "glEnable(GL_CULL_FACE)\n");
	else
	    ogEnvLog(1, "glDisable(GL_CULL_FACE)\n");

	if(rgbMode)
	    glColor4ub(((c>>24)&0xff), (c>>16)&0xff, (c>>8)&0xff, (c>>0)&0xff);
	else
	    glIndexi(c);

	llx = ogLibIntRand(0,xmaxscrn);	/* random rectangle	 */
	do
	    urx = ogLibIntRand(0,xmaxscrn);
	while (llx == urx);
	lly = ogLibIntRand(0,ymaxscrn);
	do
	    ury = ogLibIntRand(0,ymaxscrn);
	while (lly == ury);
	clockwise = (urx - llx) * (ury - lly) < 0;
	if (!cw) clockwise ^= 1;
	glRecti(llx, lly, urx, ury);
	ogEnvLog(1, "glRecti(%d,  %d,  %d,  %d)\n", llx, lly, urx, ury);
        FINIS_DL_OR_IM(1);

	ogLibPixelCheck((llx + urx) / 2, (lly + ury) / 2, (bf && clockwise) ? 0 : c);

	if(rgbMode)
	    glColor4f(0.0, 0.0, 0.0, 0.0);
	else
	    glIndexi(0);
#ifdef foo
	glRecti(llx, lly, llx, lly);	/* degenerate case	 */
	ogEnvLog(1, "glRecti(%d,  %d,  %d,  %d)\n", llx, lly, llx, lly);
	ogLibPixelCheck(llx, lly, 0);
#endif

	glRecti(llx, lly, urx, ury);	/* erase larger rect	 */
	ogLibPixelCheck(llx, lly, 0); /* XXXblythe */

#if 0
	if(rgbMode)
	    glColor4ub(((c>>24)&0xff), (c>>16)&0xff, (c>>8)&0xff, (c>>0)&0xff);
	else
	    glIndexi(c);		/* now test a circle	 */
	/* OGLXXX See gluDisk man page. */
	{
	    GLUquadricObj *qobj = gluNewQuadric();
	    glPushMatrix();
	    glTranslatef((llx + urx) / 2, (lly + ury) / 2, 0.);
	    gluDisk(qobj, 0., 10, 32, 1);
	    glPopMatrix();
	    gluDeleteQuadric(qobj);
	};
	/* OGLXXX See gluDisk man page. */
	ogEnvLog(1, "{ GLUquadricObj *qobj = gluNewQuadric(); glPushMatrix(); glTranslatef(%d,  %d, 0.); gluDisk( qobj, 0.,  %d, 32, 1); glPopMatrix(); gluDeleteQuadric(qobj); }\n", (llx + urx) / 2, (lly + ury) / 2, 10);
	ogLibPixelCheck((llx + urx) / 2, (lly + ury) / 2, c);
	if(rgbMode)
	    glColor4f(0.0, 0.0, 0.0, 0.0);
	else
	    glIndexi(0);		/* and erase		 */
	/* OGLXXX See gluDisk man page. */
	{
	    GLUquadricObj *qobj = gluNewQuadric();
	    glPushMatrix();
	    glTranslatef((llx + urx) / 2, (lly + ury) / 2, 0.);
	    gluDisk(qobj, 0., 10, 32, 1);
	    glPopMatrix();
	    gluDeleteQuadric(qobj);
	};
	ogLibPixelCheck((llx + urx) / 2, (lly + ury) / 2, 0);

	if(rgbMode)
	    glColor4ub(((c>>24)&0xff), (c>>16)&0xff, (c>>8)&0xff, (c>>0)&0xff);
	else
	    glIndexi(c);		/* now test triangles	 */
	glBegin(GL_TRIANGLES);
	glVertex2i(llx, lly);
	glVertex2i(urx, lly);
	glVertex2i(urx, ury);
	glEnd();
	ogEnvLog(1, "glBegin(GL_TRIANGLES);glVertex2i(%d, %d); glVertex2i(%d, %d); glVertex2i(%d, %d); glEnd()\n",
	     llx, lly, urx, lly, urx, ury);
	ogLibPixelCheck(urx, lly, (bf && clockwise) ? 0 : c);
	if(rgbMode)
	    glColor4f(0.0, 0.0, 0.0, 0.0);
	else
	    glIndexi(0);		/* and erase		 */
	glBegin(GL_TRIANGLES);
	glVertex2i(llx, lly);
	glVertex2i(urx, lly);
	glVertex2i(urx, ury);
	glEnd();
	ogLibPixelCheck(urx, lly, 0);
#endif
    }
}

CLEANUP(cullface)
{
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ogLibSetDefaultColors();
}

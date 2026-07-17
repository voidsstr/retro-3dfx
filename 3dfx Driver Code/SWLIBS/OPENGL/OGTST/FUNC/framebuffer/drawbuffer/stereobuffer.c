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

/* stereobuffer.c - $Revision: 2$ */

/*
 * This program tests DrawBuffer in stereo & doublebuffer mode by
 * drawing points and then reading the pixels back.
 * 
 *     Debug levels:
 * 	1 - print the DrawBuffer trace
 * 	2 - print point parameters
 * 	3 - print the checking trace
 */

#include "ogtst.h"	/* include test environment		*/

static char *
buffer_name(GLenum b) {
    static char *bname[] = {
    "GL_NONE",			/* 0	  */
    "GL_FRONT_LEFT",		/* 0x0400 */
    "GL_FRONT_RIGHT",		/* 0x0401 */
    "GL_BACK_LEFT",		/* 0x0402 */
    "GL_BACK_RIGHT",		/* 0x0403 */
    "GL_FRONT",			/* 0x0404 */
    "GL_BACK",			/* 0x0405 */
    "GL_LEFT",			/* 0x0406 */
    "GL_RIGHT",			/* 0x0407 */
    "GL_FRONT_AND_BACK",	/* 0x0408 */
    "GL_AUX0",			/* 0x0409 */
    "GL_AUX1",			/* 0x040A */
    "GL_AUX2",			/* 0x040B */
    "GL_AUX3",			/* 0x040C */
    };
    if (b == 0) return bname[0];
    if (b >= GL_FRONT_LEFT || b <= GL_AUX3) return bname[b-GL_FRONT_LEFT+1];
    return "???";
}

#define CHECK_POINT(COLOR) \
    if (multiSampled) \
        ogLibRectCheck(x, y, x, y, (COLOR), 0); \
    else \
        ogLibPixelCheck(x,y, (COLOR));

TESTMOD(stereobuffer)
{
    GLint x,y;
    GLboolean front,back,right,left,stereo,dble;
    int c;
    int xyMin, xMax, yMax;
    GLboolean multiSampled = ogEnvIsMultiSampled();
    GLint naux, aux;
    DL_PROLOG();

    dble = ogEnvIsDoubleBuffered();
    stereo = ogEnvIsStereo();
    glGetIntegerv(GL_AUX_BUFFERS, &naux);

    xMax = ogEnvQuery(OG_XWSIZE);
    yMax = ogEnvQuery(OG_YWSIZE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., xMax, 0., yMax, -1.,1.);

    xyMin = multiSampled ? 1 : 0;
    xMax -= 1 + xyMin;
    yMax -= 1 + xyMin;

    for(aux = 0; aux < naux; aux++) {
	glDrawBuffer(GL_AUX0+aux);
	ogLibClear(0);
    }
    glDrawBuffer(dble ? GL_BACK : GL_FRONT);
    ogLibClear(0);
    if (dble) {
	ogEnvSwapBuffers();
	ogLibClear(0);
    }
    if (stereo)
	glDrawBuffer(dble ? GL_BACK_LEFT : GL_LEFT);

    while (pass--) {
	GLenum buffer, readBuffer;

	if (naux && ogLibBitRand(2) == 0) { /* 25% of the time */
	    front = back = left = right = 0;
	    buffer = GL_AUX0+ogLibIntRand(0, naux-1);
	} else {
	    front = ogLibBitRand(1);
	    back = ogLibBitRand(1)&dble;
	    left = ogLibBitRand(1);
	    right = ogLibBitRand(1)&stereo;
	    if (left && right) {
		if (front && back)
		    buffer = GL_FRONT_AND_BACK;
		else if (front)
		    buffer =  GL_FRONT;
		else if (back)
		    buffer = GL_BACK;
		else
		    buffer = GL_NONE;
	    } else if (right) {
		if (back && front)
		    buffer = GL_RIGHT;
		else if ((front | back) && !dble && ogLibBitRand(1))
		    buffer = GL_RIGHT;
		else if (front)
		    buffer = GL_FRONT_RIGHT;
		else if (back)
		    buffer = GL_BACK_RIGHT;
		else
		    buffer = GL_NONE;
	    } else if (left) { /* left */
		if (back && front)
		    buffer = GL_LEFT;
		else if ((front | back) && !dble && ogLibBitRand(1))
		    buffer = GL_LEFT;
		else if (front)
		    buffer = GL_FRONT_LEFT;
		else if (back)
		    buffer = GL_BACK_LEFT;
		else
		    buffer = GL_NONE;
	    } else
		buffer = GL_NONE;
	}

	START_DL_OR_IM(1);
	ogEnvLog(OG_LPARAMETERS, "glDrawBuffer(%s);\n", buffer_name(buffer));
	glDrawBuffer(buffer);
	x = ogLibIntRand(xyMin, xMax);		/* draw a random point	*/
	y = ogLibIntRand(xyMin, yMax);

        if (multiSampled) {
            int j, k;
            /*
             * Clear the 5x5 rectangle around the point.  3x3 is not enough
             * becase of the slight shift up and to the right of the vertex
             * w.r.t pixel centers.
             */
            glColor4ub(0, 0, 0, 0);
            glBegin(GL_POINTS); {
                for (k = -2; k <= 2; k++)
                    for (j = -2; j <= 2; j++)
                        glVertex2i(x + k, y + j);
            } glEnd();
        }
	c = ogLibColor();
	ogEnvLog(OG_LPARAMETERS,"color (0x%x); point vertex (%d, %d)\n",c,x,y);
	ogLibDrawFragments(x,y);
	FINIS_DL_OR_IM(1);

	if (!dble)
            back = front;
        ogEnvLog(1,"checking the %s%s buffer ...\n",
                 dble ? "back" : "front", stereo ? " left" : "");
        CHECK_POINT((back&left) ? c : 0);
	if (stereo) {
	    ogEnvLog(1, "checking the %s right buffer ...\n",
                     dble ? "back" : "front");
            readBuffer = dble ? GL_BACK_RIGHT : GL_FRONT_RIGHT;
            ogEnvLog(OG_LPARAMETERS, "glReadBuffer(%s);\n",
                     buffer_name(readBuffer));
	    glReadBuffer(readBuffer);
            CHECK_POINT((back&right) ? c : 0);
            readBuffer = dble ? GL_BACK_LEFT : GL_FRONT_LEFT;
            ogEnvLog(OG_LPARAMETERS, "glReadBuffer(%s);\n",
                     buffer_name(readBuffer));
	    glReadBuffer(readBuffer);
	}
	if (dble) {
	    ogEnvLog(1,"swapbuffers and checking the front%s buffer ...\n",
                     stereo ? " left" : "");
	    ogEnvSwapBuffers();
            CHECK_POINT((front&left) ? c : 0);
	    if (stereo) {
		ogEnvLog(1,"checking the front right buffer ...\n");
                ogEnvLog(OG_LPARAMETERS, "glReadBuffer(GL_BACK_RIGHT);\n");
		glReadBuffer(GL_BACK_RIGHT);
                CHECK_POINT((front&right) ? c : 0);
                ogEnvLog(OG_LPARAMETERS, "glReadBuffer(GL_BACK_LEFT);\n");
		glReadBuffer(GL_BACK_LEFT);
	    }
	    ogEnvSwapBuffers();	/* swap back */
	}
	for(aux = 0; aux < naux; aux++) {
	    ogEnvLog(1,"checking %s ...\n", buffer_name(GL_AUX0+aux));
	    glReadBuffer(GL_AUX0+aux);
            CHECK_POINT((buffer==GL_AUX0+aux) ? c : 0);
	}
	if (aux) glReadBuffer(dble ? GL_BACK_LEFT : GL_FRONT_LEFT);
	/* clear point */
	ogLibSetColor(0, 0, 0, 0);
	glBegin(GL_POINTS);
	ogLibSetVertex(x, y,0,1);
	glEnd();
    }
}

CLEANUP(stereobuffer) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultBuffers();
    ogLibSetDefaultColors();
}

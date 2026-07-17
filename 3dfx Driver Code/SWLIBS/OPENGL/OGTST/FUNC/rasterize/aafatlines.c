/**************************************************************************
 *                                                                        *
 *               Copyright (C) 1989, Silicon Graphics, Inc.               *
 *                                                                        *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *                                                                        *
 **************************************************************************/

/* aafatlines.c$ */

#include "ogtst.h"		/* include test environment             */
#include "math.h"


typedef struct {
    float x, y, z;
} Vector3f;

#define LINES_NUM 30
#define LENGTH 500.5

/*ARGSUSED*/
TESTMOD(aafatlines) {
    Vector3f v0, v1;
    int i;
    float width;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., ogEnvQuery(OG_XWSIZE), 0., ogEnvQuery(OG_YWSIZE), -1.0, 1.0);

    ogLibClear(0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glEnable(GL_BLEND);

    glColor4f(1., 1., 1., 1.);

    width = 0.5;
    v0.z = v1.z = 0.0;
    v0.x = 0.5;
    v1.x = LENGTH;
    v0.y = 0.5 + 0.25;
    v1.y = v0.y;

    glHint(GL_LINE_SMOOTH_HINT,GL_FASTEST);
    glEnable(GL_LINE_SMOOTH);

    for (i = 0; i < LINES_NUM / 2;
         i++, width++, v0.y += width + 0.5, v1.y = v0.y + i) {
	glLineWidth(width);
	glBegin(GL_LINE_STRIP);
	glVertex3fv((float *) &v0);
	glVertex3fv((float *) &v1);
	glEnd();
    }

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    for (; i < LINES_NUM;
         i++, width++, v0.y += width + 0.5, v1.y = v0.y + i) {
	glLineWidth(width);
	glBegin(GL_LINE_STRIP);
	glVertex3fv((float *) &v0);
	glVertex3fv((float *) &v1);
	glEnd();
    }

    glDisable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
}

CLEANUP(aafatlines)
{
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);

    glLineWidth(1.0);
    glDisable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_DONT_CARE);
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);
}

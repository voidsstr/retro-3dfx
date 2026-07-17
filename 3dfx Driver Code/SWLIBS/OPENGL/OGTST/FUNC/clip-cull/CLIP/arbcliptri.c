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

/* arbcliptri.c - $Revision: 2$ */

/*
 * tests clipped triangles with an arbitrary
 * plane - bruceh
 */

#include "ogtst.h"       /* include test environment             */
#include "math.h"


static GLdouble plane[4];

static float v1[3] = {0.2, -0.2, 0.1};
static float v2[3] = {0.4, 0.2, 0.1};
static float v3[3] = {0.1, 0.1, 0.7};
static float v4[3] = {0.0, 0.0, 0.1};
static float v5[3] = {0.25, 0.0, 0.1};
static float v6[3] = {0.0, 0.5, 0.7};
static float v7[3] = {0.2, 0.5, 0.1};
static float v8[3] = {0.4, 0.3, 0.1};
static float v9[3] = {0.1, 0.2, 0.7};

static void subr(void);

/*ARGSUSED*/
TESTMOD(arbcliptri)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0,  1.0,  -1.0,  1.0);

    glMatrixMode(GL_MODELVIEW);

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    plane[0] = -1.0;
    plane[1] = 1.0;
    plane[2] = 0.0;
    plane[3] = 0.1;
    glClipPlane( GL_CLIP_PLANE0, plane);
    glEnable(GL_CLIP_PLANE0);

    subr();

    glTranslatef(0.85,  0.,  0.);
    subr();

    glTranslatef(-2.02,  0.,  0.);
    subr();

    glTranslatef(1.17,  -1.17,  0.);
    subr();

    glTranslatef(0.,  2.02,  0.);
    subr();

    glTranslatef(0.5,  -.35,  0.5);
    subr();

    glTranslatef(-1.,  -1.,  -1.8);
    subr();
}

static void subr(void)
{
	glBegin(GL_POLYGON);
	glColor3ub(255, 0, 0);
	glVertex3fv(v1);
	glColor3ub(0, 255, 0);
	glVertex3fv(v2);
	glColor3ub(0, 0, 255);
	glVertex3fv(v3);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3ub(0, 0, 255);
	glVertex3fv(v4);
	glColor3ub(0, 255, 0);
	glVertex3fv(v5);
	glColor3ub(255, 0, 0);
	glVertex3fv(v6);
	glEnd();

	glBegin(GL_POLYGON);
	glColor3ub(0, 255, 0);
	glVertex3fv(v7);
	glColor3ub(0, 0, 255);
	glVertex3fv(v8);
	glColor3ub(255, 0, 0);
	glVertex3fv(v9);
	glEnd();
}

CLEANUP(arbcliptri)
{
    plane[0] = 0;
    plane[1] = 0;
    plane[2] = 0;
    plane[3] = 0;
    glClipPlane( GL_CLIP_PLANE0, plane);
    glDisable(GL_CLIP_PLANE0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ogLibSetDefaultColors();
}

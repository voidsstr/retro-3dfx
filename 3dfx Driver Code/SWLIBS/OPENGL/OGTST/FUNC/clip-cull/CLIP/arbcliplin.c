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

/* arbcliplin.c - $Revision: 2$ */

/*
 * tests clipped lines with an arbitrary plane
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
TESTMOD(arbcliplin)
{
    glMatrixMode(GL_PROJECTION);
    ogEnvLog(2, "glMatrixMode(GL_PROJECTION)\n");
    glLoadIdentity();
    ogEnvLog(2, "glLoadIdentity()\n");
    gluOrtho2D(-1.0,  1.0,  -1.0,  1.0);
    ogEnvLog(2, "gluOrtho2D(-1.0,  1.0,  -1.0,  1.0)\n");
    glMatrixMode(GL_MODELVIEW);
    ogEnvLog(2, "glMatrixMode(GL_MODELVIEW)\n");
    
    glClearColor(0, 0, 0, 0);
    ogEnvLog(2, "glClearColor(0, 0, 0, 0)\n");
    glClear(GL_COLOR_BUFFER_BIT);
    ogEnvLog(2, "glClear(GL_COLOR_BUFFER_BIT)\n");
    
    plane[0] = -1.0;	
    plane[1] = 1.0;	  
    plane[2] = 0.0;
    plane[3] = 0.1;
    ogEnvLog(2, "plane = {-1.0, 1.0, 0.0, 0.1}\n");
    glClipPlane(GL_CLIP_PLANE0, plane);
    ogEnvLog(2, "glClipPlane(GL_CLIP_PLANE0, plane)\n");
    glEnable(GL_CLIP_PLANE0);
    ogEnvLog(2, "glEnable(GL_CLIP_PLANE0)\n");

    subr();

    glTranslatef(0.85,  0.,  0.);
    ogEnvLog(2, "glTranslatef(0.85,  0.,  0.)\n");
    subr();

    glTranslatef(-2.02,  0.,  0.);
    ogEnvLog(2, "glTranslatef(-2.02,  0.,  0.)\n");
    subr();

    glTranslatef(1.17,  -1.17,  0.);
    ogEnvLog(2, "glTranslatef(1.17,  -1.17,  0.)\n");
    subr();

    glTranslatef(0.,  2.02,  0.);
    ogEnvLog(2, "glTranslatef(0.,  2.02,  0.)\n");
    subr();

    glTranslatef(0.5,  -.35,  0.5);
    ogEnvLog(2, "glTranslatef(0.5,  -.35,  0.5)\n");
    subr();

    glTranslatef(-1.,  -1.,  -1.8);
    ogEnvLog(2, "glTranslatef(-1.,  -1.,  -1.8)\n");
    subr();
}

static void subr(void)
{
	glBegin(GL_LINE_STRIP);
	ogEnvLog(2, "glBegin(GL_LINE_STRIP)\n");
	glColor3ub(255, 0, 0);
	ogEnvLog(2, "glColor3ub(255, 0, 0)\n");
	glVertex3fv(v1);
        ogEnvLog(2, "glVertex3fv(%f, %f, %f)\n", v1[0], v1[1], v1[2]);
	glColor3ub(0, 255, 0);
	ogEnvLog(2, "glColor3ub(0, 255, 0)\n");
	glVertex3fv(v2);
	ogEnvLog(2, "glVertex3fv(%f, %f, %f)\n", v2[0], v2[1], v2[2]);
	glColor3ub(0, 0, 255);
	ogEnvLog(2, "glColor3ub(0, 0, 255)\n");
	glVertex3fv(v3);
	ogEnvLog(2, "glVertex3fv(%f, %f, %f)\n", v3[0], v3[1], v3[2]);
	glEnd();
	ogEnvLog(2, "glEnd()\n");
	

	glBegin(GL_LINE_STRIP);
	ogEnvLog(2, "glBegin(GL_LINE_STRIP)\n");
	glColor3ub(0, 0, 255);
	ogEnvLog(2, "glColor3ub(0, 0, 255)\n");
	glVertex3fv(v4);
	ogEnvLog(2, "glVertex3fv(%f, %f, %f)\n", v4[0], v4[1], v4[2]);
	glColor3ub(0, 255, 0);
	ogEnvLog(2, "glColor3ub(0, 255, 0)\n");
	glVertex3fv(v5);
	ogEnvLog(2, "glVertex3fv(%f, %f, %f)\n", v5[0], v5[1], v5[2]);
	glColor3ub(255, 0, 0);
	ogEnvLog(2, "glColor3ub(255, 0, 0)\n");
	glVertex3fv(v6);
	ogEnvLog(2, "glVertex3fv(%f, %f, %f)\n", v6[0], v6[1], v6[2]);
	glColor3ub(0, 255, 0);
	ogEnvLog(2, "glColor3ub(0, 255, 0)\n");
	glVertex3fv(v7);
	ogEnvLog(2, "glVertex3fv(%f, %f, %f)\n", v7[0], v7[1], v7[2]);
	glColor3ub(0, 0, 255);
	ogEnvLog(2, "glColor3ub(0, 0, 255)\n");
	glVertex3fv(v8);
	ogEnvLog(2, "glVertex3fv(%f, %f, %f)\n", v8[0], v8[1], v8[2]);
	glColor3ub(255, 0, 0);
	ogEnvLog(2, "glColor3ub(255, 0, 0)\n");
	glVertex3fv(v9);
	ogEnvLog(2, "glVertex3fv(%f, %f, %f)\n", v9[0], v9[1], v9[2]);
	glEnd();
	ogEnvLog(2, "glEnd()\n");
}



CLEANUP(arbcliplin)
{
    plane[0] = 0;
    plane[1] = 0;
    plane[2] = 0; 
    plane[3] = 0;
    ogEnvLog(2, "plane = { 0, 0, 0, 0}\n");
    glClipPlane( GL_CLIP_PLANE0, plane);
    ogEnvLog(2, "glClipPlane( GL_CLIP_PLANE0, plane)\n");
    glDisable(GL_CLIP_PLANE0);
    ogEnvLog(2, "glDisable(GL_CLIP_PLANE0)\n");

    glMatrixMode(GL_PROJECTION);
    ogEnvLog(2, "glMatrixMode(GL_PROJECTION)\n");
    glLoadIdentity();
    ogEnvLog(2, "glLoadIdentity()\n");
    glMatrixMode(GL_MODELVIEW);
    ogEnvLog(2, "glMatrixMode(GL_MODELVIEW)\n");
    glLoadIdentity();
    ogEnvLog(2, "glLoadIdentity()\n");
    ogLibSetDefaultColors();
    ogEnvLog(2, "ogLibSetDefaultColors()\n");
}

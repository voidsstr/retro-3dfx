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

/* $Revision: 2$ */

/*
** Test mixing evalcoord, vertex, and material calls between begin/end.
*/

#include <alloca.h>
#include <stdio.h>
#include <math.h>
#include <bstring.h>
#include "ogtst.h"

static void init_lighting(void);
static int x,y;
static int vpw, vph;

static void init_position(void)
{
    x = 0;
    y = 0;
    vpw = ogEnvQuery(OG_XWSIZE) / 5;
    vph = ogEnvQuery(OG_YWSIZE) / 4;
    glViewport(0, 0, vpw, vph);
}

static void nextcol(void)
{
    x++;
    glViewport(x*vpw, y*vph, vpw, vph);
}

static float vmap2[] = {
    0,0,0,1, 8,0,0,1,
    0,0,0,1, 8,0,0,1,
};
static float vmap1[] = {
    0,0,0,1, 8,0,0,1,
};

static float bluemat[] = { 0.0, 0.0, 0.8, 1.0 };
static float yellowmat[] = { 0.8, 0.8, 0.0, 1.0 };

static float v0[] = { 0, 8, 0 };
static float v1[] = { 8, 8, 0 };

static void
do1d(void)
{
    int i;
    int udivs = 8;
    float u;
    
    glEnable(GL_MAP1_VERTEX_4);
    glMap1f(GL_MAP1_VERTEX_4, 0, 1, 4, 2, vmap1);
    glMapGrid1f(udivs, 0, 1);

    /* evalcoord and vertex */
    glPolygonMode(GL_FRONT, GL_LINE);
    glBegin(GL_TRIANGLE_STRIP);
    for (i=0; i < udivs; i++) {
	u = i/(udivs-1.0);
	glVertex2f(v0[0]*(1-u) + v1[0]*u,
		   v0[1]*(1-u) + v1[1]*u);
	glEvalCoord1f(u);
    }
    glEnd();
    
    /* evalcoord, vertex, and material */
    nextcol();
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_FILL);
    glBegin(GL_TRIANGLE_STRIP);
    for (i=0; i < udivs; i++) {
	u = i/(udivs-1.0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, bluemat);
	glVertex2f(v0[0]*(1-u) + v1[0]*u,
		   v0[1]*(1-u) + v1[1]*u);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, yellowmat);
	glEvalCoord1f(u);
    }
    glEnd();
}

static void
do2d(void)
{
    int i;
    int udivs = 8;
    float u;
    
    glMapGrid2f(udivs, 0, 1, udivs, 0, 1);
    glEnable(GL_MAP2_VERTEX_4);
    glMap2f(GL_MAP2_VERTEX_4, 0, 1, 4, 2, 0, 1, 8, 2, vmap2);

    /* evalcoord and vertex */ 
    nextcol();
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_LINE);
    glBegin(GL_TRIANGLE_STRIP);
    for (i=0; i < udivs; i++) {
	u = i/(udivs-1.0);
	glVertex2f(v0[0]*(1-u) + v1[0]*u,
		   v0[1]*(1-u) + v1[1]*u);
	glEvalCoord2f(u,0);
    }
    glEnd();

    /* evalcoord, vertex, and material */
    nextcol();
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT, GL_FILL);
    glBegin(GL_TRIANGLE_STRIP);
    for (i=0; i < udivs; i++) {
	u = i/(udivs-1.0);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, bluemat);
	glVertex2f(v0[0]*(1-u) + v1[0]*u,
		   v0[1]*(1-u) + v1[1]*u);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, yellowmat);
	glEvalCoord2f(u,0);
    }
    glEnd();
}

/*ARGSUSED*/
TESTMOD(evalvertexmix)
{
    x = y = 0;    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-.8,8.8,-.8,8.8,-10,10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT);
    init_lighting();

    init_position();
    do1d();
    do2d();
}

CLEANUP(evalvertexmix)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ogLibSetDefaultColors();
    ogLibSetDefaultClears();
    ogLibSetDefaultLight();
    ogLibSetDefaultEvaluators();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glViewport(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    glDisable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
}

static void
init_lighting(void)
{
    static float diffuse[] = { 2.0, 2.0, 2.0, 2.0 };
    static float position[] = { 50.0, 150.0, 120.0, 0.0 };
    static float front_mat_diffuse[] = { 0.3, 0.3, 0.7, 1.0 };

    glMaterialfv(GL_FRONT, GL_DIFFUSE, front_mat_diffuse);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glEnable(GL_LIGHT0);
    glShadeModel(GL_FLAT);
}

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

#include <alloca.h>
#include <stdio.h>
#include <math.h>
#include <bstring.h>
#include "ogtst.h"

static void init_lighting(void);
static void init_texture(void);

static int x,y;
static int vpw, vph;
static int hx;

static void
init_position(void)
{
    x = 0;
    y = 0;
    hx = 0;
    vpw = ogEnvQuery(OG_XWSIZE) / 7;
    vph = ogEnvQuery(OG_YWSIZE) / 5;
    glViewport(0, 0, vpw, vph);
}

static void
nexthalf(void)
{
    y = 0;
    x = hx = 4;
    glViewport(x*vpw, y*vph, vpw, vph);
}

static void
nextcol(void)
{
    x++;
    glViewport(x*vpw, y*vph, vpw, vph);
}

static void
nextrow(void)
{
    x = hx;
    y++;
    glViewport(x*vpw, y*vph, vpw, vph);
}

static float vmap2[] = {
    0,0,0,1, 1,0,0,1, 2,0,0,1, 3,0,0,1,
    0,1,0,1, 1,1,4,1, 2,1,4,1, 3,1,0,1,
    0,2,0,1, 1,2,4,1, 2,2,4,1, 3,2,0,1,
    0,3,0,1, 1,3,0,1, 2,3,0,1, 3,3,0,1,
};
static float tmap2[] = {
    0,0, 1,0,
    0,1, 1,1,
};
static float cmap2[] = {
    1,1,0,0, 0,0,1,0,
    1,0,0,0, 0,0,0,0,
};
static float nmap2[] = {
    -1,-1,.1,  0,-1,.1,  1,-1,.1,
    -1, 0,.1,  0, 0,1.,  1, 0,.1,
    -1, 1,.1,  0, 1,.1,  1, 1,.1,
};

static float vmap1[] = {
    0,1.5,0,1, 1,1.5,4,1, 2,1.5,4,1, 3,1.5,0,1,
};
static float tmap1[] = {
    0,0.2, 1,0.2
};
static float nmap1[] = {
    -1,0,.1, 0,0,.5, 1,0,.1
};
static float cmap1[] = {
    1,1,0,0, 1,0,0,0,
};

static int udivs, vdivs;
static GLenum mode;

void do2d(void)
{
    glEnable(GL_MAP2_VERTEX_4);
    glMap2f(GL_MAP2_VERTEX_4, 0, 1, 4, 4, 0, 1, 16, 4, vmap2);
    glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, 0, 1, 4, 2, tmap2);
    glMap2f(GL_MAP2_COLOR_4, 0, 1, 4, 2, 0, 1, 8, 2, cmap2);
    glMap2f(GL_MAP2_NORMAL, 0, 1, 3, 3, 0, 1, 9, 3, nmap2);

    /* test map grid */
    mode = GL_LINE;
    udivs = 8, vdivs = 8;
    glMapGrid2f(udivs, 0, 1, vdivs, 0, 1);
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    udivs = 4, vdivs = 4;
    glPushAttrib(GL_EVAL_BIT);
    glMapGrid2f(udivs, 0.3, 0.7, vdivs, 0.3, 0.7);
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glPopAttrib();
    udivs = 8, vdivs = 8;
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);

    /* test auto normal */
    mode = GL_FILL;
    init_lighting();
    glEnable(GL_AUTO_NORMAL);
    nextrow();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glPushAttrib(GL_EVAL_BIT);
    glDisable(GL_AUTO_NORMAL);
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glPopAttrib();
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glDisable(GL_AUTO_NORMAL);

    /* test normal map */
    glEnable(GL_NORMALIZE);
    glEnable(GL_MAP2_NORMAL);
    nextrow();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glPushAttrib(GL_EVAL_BIT);
    glDisable(GL_MAP2_NORMAL);
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glPopAttrib();
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glDisable(GL_MAP2_NORMAL);
    glDisable(GL_NORMALIZE);
    glDisable(GL_LIGHTING);

    /* test color map */
    glEnable(GL_MAP2_COLOR_4);
    nextrow();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glPushAttrib(GL_EVAL_BIT);
    glDisable(GL_MAP2_COLOR_4);
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glPopAttrib();
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glDisable(GL_MAP2_COLOR_4);

    /* test texture map */
    init_texture();
    glTexCoord2f(0.1,0.1);
    glEnable(GL_MAP2_TEXTURE_COORD_2);
    nextrow();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glPushAttrib(GL_EVAL_BIT);
    glDisable(GL_MAP2_TEXTURE_COORD_2);
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glPopAttrib();
    nextcol();
    glEvalMesh2(mode, 0, udivs, 0, vdivs);
    glDisable(GL_TEXTURE_2D);
}

void do1d(void)
{
    glEnable(GL_MAP1_VERTEX_4);
    glMap1f(GL_MAP1_VERTEX_4, 0, 1, 4, 4, vmap1);
    glMap1f(GL_MAP1_TEXTURE_COORD_2, 0, 1, 2, 2, tmap1);
    glMap1f(GL_MAP1_COLOR_4, 0, 1, 4, 2, cmap1);
    glMap1f(GL_MAP1_NORMAL, 0, 1, 3, 3, nmap1);
    glLineWidth(20);
    glPointSize(3);

    /* test map grid */
    mode = GL_POINT;
    udivs = 8;
    glMapGrid1f(udivs, 0, 1);
    glEvalMesh1(mode, 0, udivs);
    glPushAttrib(GL_EVAL_BIT);
    udivs = 4;
    glMapGrid1f(udivs, 0.3, 0.7);
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    udivs = 8;
    glPopAttrib();
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    
    /* test auto normal */
    /* it has no effect for 1D, but want to make sure it doesn't trip 1D */
    mode = GL_LINE;
    init_lighting();
    glEnable(GL_AUTO_NORMAL);
    nextrow();
    glEvalMesh1(mode, 0, udivs);
    glPushAttrib(GL_EVAL_BIT);
    glDisable(GL_AUTO_NORMAL);
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    glPopAttrib();
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    glDisable(GL_AUTO_NORMAL);

    /* test normal map */
    glEnable(GL_NORMALIZE);
    glEnable(GL_MAP1_NORMAL);
    nextrow();
    glEvalMesh1(mode, 0, udivs);
    glPushAttrib(GL_EVAL_BIT);
    glDisable(GL_MAP1_NORMAL);
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    glPopAttrib();
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    glDisable(GL_MAP1_NORMAL);
    glDisable(GL_NORMALIZE);
    glDisable(GL_LIGHTING);

    /* test color map */
    glEnable(GL_MAP1_COLOR_4);
    nextrow();
    glEvalMesh1(mode, 0, udivs);
    glPushAttrib(GL_EVAL_BIT);
    glDisable(GL_MAP1_COLOR_4);
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    glPopAttrib();
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    glDisable(GL_MAP1_COLOR_4);

    /* test texture map */
    init_texture();
    glTexCoord1f(0.1);
    glEnable(GL_MAP1_TEXTURE_COORD_2);
    nextrow();
    glEvalMesh1(mode, 0, udivs);
    glPushAttrib(GL_EVAL_BIT);
    glDisable(GL_MAP1_TEXTURE_COORD_2);
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    glPopAttrib();
    nextcol();
    glEvalMesh1(mode, 0, udivs);
    glDisable(GL_TEXTURE_2D);

    glLineWidth(1.0);
    glPointSize(1.0);
}

/*ARGSUSED*/
TESTMOD(evalattrib)
{
    x = y = hx = 0;    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.8,1.8,-1.8,1.8,-10,10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-1.5, -1.5, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    init_position();
    do2d();
    nexthalf();
    do1d();
}

CLEANUP(evalattrib)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ogLibSetDefaultColors();
    ogLibSetDefaultClears();
    ogLibSetDefaultTextures();
    ogLibSetDefaultLight();
    ogLibSetDefaultEvaluators();

    glFrontFace(GL_CCW);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glViewport(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_AUTO_NORMAL);
}

static void
init_lighting(void)
{
    static float diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    static float position[] = { 50.0, 50.0, 120.0, 0.0 };
    static float front_mat_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };

    glMaterialfv(GL_FRONT, GL_DIFFUSE, front_mat_diffuse);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glFrontFace(GL_CW);
}

#define B 0xff,0xff,0xff,0xff
#define F 0x00,0x00,0xff,0xff
static unsigned char tex[] = {
    B,B,B,B,F,F,F,F,B,B,B,B,F,F,F,F,
    B,B,B,B,F,F,F,F,B,B,B,B,F,F,F,F,
    B,B,B,B,F,F,F,F,B,B,B,B,F,F,F,F,
    B,B,B,B,F,F,F,F,B,B,B,B,F,F,F,F,
    F,F,F,F,B,B,B,B,F,F,F,F,B,B,B,B,
    F,F,F,F,B,B,B,B,F,F,F,F,B,B,B,B,
    F,F,F,F,B,B,B,B,F,F,F,F,B,B,B,B,
    F,F,F,F,B,B,B,B,F,F,F,F,B,B,B,B,
    B,B,B,B,F,F,F,F,B,B,B,B,F,F,F,F,
    B,B,B,B,F,F,F,F,B,B,B,B,F,F,F,F,
    B,B,B,B,F,F,F,F,B,B,B,B,F,F,F,F,
    B,B,B,B,F,F,F,F,B,B,B,B,F,F,F,F,
    F,F,F,F,B,B,B,B,F,F,F,F,B,B,B,B,
    F,F,F,F,B,B,B,B,F,F,F,F,B,B,B,B,
    F,F,F,F,B,B,B,B,F,F,F,F,B,B,B,B,
    F,F,F,F,B,B,B,B,F,F,F,F,B,B,B,B,
};

static void
init_texture(void)
{
    glEnable(GL_TEXTURE_2D);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 tex);
}

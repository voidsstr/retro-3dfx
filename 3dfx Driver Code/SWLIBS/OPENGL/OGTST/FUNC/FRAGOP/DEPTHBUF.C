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

/* depthbuf.c - $Revision: 2$ */

#include "ogtst.h"

#define	ZFAR	1000
#define	Z0	ZFAR/4
#define	Z1	ZFAR*3/4
#define	C0	0x1
#define	C1	0x2
#define	C2	0x4
#define	C3	0x8

static GLuint indexColors[] = { 1, 2, 3, 4};
static GLuint rgbColors[] = {0xfedcba00, 0x01234500, 0x142a2500, 0xd314a200};

static char *zfs[] = {
      "GL_NEVER",
      "GL_LESS",
      "GL_EQUAL",
      "GL_LEQUAL",
      "GL_GREATER",
      "GL_NOTEQUAL",
      "GL_GEQUAL",
      "GL_ALWAYS"
};

static GLenum zff[] = {
    GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL,
    GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS,
};
#define NFUNCS (sizeof(zff)/sizeof(GLenum))

static void draw(void);
static void drawMS(void);
static void firstDrawCI(void);
static void firstDrawRGB(void);
static void firstDrawMs(void);
static void colorCI(int);
static void colorRGB(int);
static GLboolean chkCI(int);
static GLboolean chkRGB(int);
static GLboolean chkMs(int);
static void (*color)(int);

TESTMOD(depthbuf)
{
    int i;
    int order[8];
    void (*firstDraw)(void);
    void (*drawer)(void);
    GLboolean (*chkFunc)(int);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., ogEnvQuery(OG_XWSIZE), 0., ogEnvQuery(OG_YWSIZE), 0., ZFAR);
    glMatrixMode(GL_MODELVIEW);

    glEnable(GL_DEPTH_TEST);

    drawer = draw;
    if (ogEnvCurVisualInfo(GLX_RGBA)) {
        if (ogEnvIsMultiSampled()) {
            chkFunc = chkMs;
            firstDraw = firstDrawMs;
	    drawer = drawMS;
        } else {
            chkFunc = chkRGB;
            firstDraw = firstDrawRGB;
        }
        color = colorRGB;
    } else {
        chkFunc = chkCI;
        firstDraw = firstDrawCI;
        color = colorCI;
    }
        
    while (pass--) {
        ogLibOrderRand(order, NFUNCS);
        /* test all possible functions */
        for (i = 0; i < NFUNCS; i++) {
	    /* clear the work area */
	    glDepthFunc(GL_ALWAYS);
            firstDraw();
	    /* Assign a depth func from the order list*/
	    glDepthFunc(zff[order[i]]);
	    ogEnvLog(OG_LPARAMETERS, "glDepthFunc(%s)\n", zfs[order[i]]);
	    
	    (*drawer)();

	    /* check results */
            if(chkFunc(order[i]))
                ogEnvLog(OG_LFAIL, "glDepthFunc(%s) failed\n", zfs[order[i]]);
        }
    }
}

void
draw(void) {
    /* draw 4 dots from near to far */
    glBegin(GL_POINTS);
    color(0);
    glVertex3i(1, 1, -Z0);
    glEnd();
    glBegin(GL_POINTS);
    color(1);
    glVertex3i(1, 1, -Z0);
    glEnd();
    glBegin(GL_POINTS);
    color(2);
    glVertex3i(1, 1, -Z1);
    glEnd();
    glBegin(GL_POINTS);
    color(3);
    glVertex3i(1, 1, -Z1);
    glEnd();
    /* draw 4 dots from far to near */
    glBegin(GL_POINTS);
    glVertex3i(4, 1, -Z1);
    glEnd();
    color(2);
    glBegin(GL_POINTS);
    glVertex3i(4, 1, -Z1);
    glEnd();
    color(1);
    glBegin(GL_POINTS);
    glVertex3i(4, 1, -Z0);
    glEnd();
    color(0);
    glBegin(GL_POINTS);
    glVertex3i(4, 1, -Z0);
    glEnd();
}

void
drawMS(void) {
    /* draw 4 dots from near to far */
#define MSpoint(x,y,z)		\
	glVertex3i(x, y, z); glVertex3i(x+1, y, z); \
	glVertex3i(x+1, y+1, z); glVertex3i(x, y+1, z); 

    glBegin(GL_QUADS);
    color(0);
    MSpoint(1, 1, -Z0);
    glEnd();
    glBegin(GL_QUADS);
    color(1);
    MSpoint(1, 1, -Z0);
    glEnd();
    glBegin(GL_QUADS);
    color(2);
    MSpoint(1, 1, -Z1);
    glEnd();
    glBegin(GL_QUADS);
    color(3);
    MSpoint(1, 1, -Z1);
    glEnd();
    /* draw 4 dots from far to near */
    glBegin(GL_QUADS);
    MSpoint(4, 1, -Z1);
    glEnd();
    color(2);
    glBegin(GL_QUADS);
    MSpoint(4, 1, -Z1);
    glEnd();
    color(1);
    glBegin(GL_QUADS);
    MSpoint(4, 1, -Z0);
    glEnd();
    color(0);
    glBegin(GL_QUADS);
    MSpoint(4, 1, -Z0);
    glEnd();
}

CLEANUP(depthbuf)
{
    glDepthFunc(GL_LESS);
    glDisable(GL_DEPTH_TEST);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();
}

static void
firstDrawCI(void)
{
    glIndexi(0);
    glBegin(GL_POINTS);
    glVertex3i(1, 1, -ZFAR / 2);
    glVertex3i(4, 1, -ZFAR / 2);
    glEnd();
}

static void
firstDrawRGB(void)
{
    glColor4f(0.0, 0.0, 0.0, 0.0);
    glBegin(GL_POINTS);
    glVertex3i(1, 1, -ZFAR / 2);
    glVertex3i(4, 1, -ZFAR / 2);
    glEnd();
}

static void
firstDrawMs(void)
{
    glColor4f(0.0, 0.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3i(0, 0, -ZFAR / 2);
    glVertex3i(6, 0, -ZFAR / 2);
    glVertex3i(6, 3, -ZFAR / 2);
    glVertex3i(0, 3, -ZFAR / 2);
    glEnd();
}

static void
colorCI(int i)
{
    glIndexi(indexColors[i]);
}

static void
colorRGB(int i)
{
    GLuint c = rgbColors[i];
    glColor4ub((c >> 24) & 0xff, (c >> 16) & 0xff, (c >> 8) & 0xff, c & 0xff);
}

static GLboolean
chkCI(int zf)
{
    GLuint color[4];
    
    ogLibReadPixels(1, 1, 4, 1, color);
    switch (zff[zf]) {
      case GL_NEVER:
	return (color[0] != 0 || color[3] != 0);
      case GL_LESS:
	return (color[0] != indexColors[0] || color[3] != indexColors[1]);
      case GL_EQUAL:
	return (color[0] != 0 || color[3] != 0);
      case GL_LEQUAL:
	return (color[0] != indexColors[1] || color[3] != indexColors[0]);
      case GL_GREATER:
	return (color[0] != indexColors[2] || color[3] != indexColors[3]);
      case GL_NOTEQUAL:
	return (color[0] != indexColors[2] || color[3] != indexColors[1]);
      case GL_GEQUAL:
	return (color[0] != indexColors[3] || color[3] != indexColors[2]);
      case GL_ALWAYS:
	return (color[0] != indexColors[3] || color[3] != indexColors[0]);
    }
    return 0;                   /* Stupid lint */
}

static GLboolean
chkRGB(int zf)
{
    switch (zff[zf]) {
      case GL_NEVER:
        return (ogLibPixelCheck(1, 1, 0) || ogLibPixelCheck(4, 1, 0));
      case GL_LESS:
	return (ogLibPixelCheck(1, 1, rgbColors[0]) ||
                ogLibPixelCheck(4, 1, rgbColors[1]));
      case GL_EQUAL:
	return (ogLibPixelCheck(1, 1, 0) || ogLibPixelCheck(4, 1, 0));
      case GL_LEQUAL:
	return (ogLibPixelCheck(1, 1, rgbColors[1]) ||
                ogLibPixelCheck(4, 1, rgbColors[0]));
      case GL_GREATER:
	return (ogLibPixelCheck(1, 1, rgbColors[2]) ||
                ogLibPixelCheck(4, 1, rgbColors[3]));
      case GL_NOTEQUAL:
	return (ogLibPixelCheck(1, 1, rgbColors[2]) ||
                ogLibPixelCheck(4, 1, rgbColors[1]));
      case GL_GEQUAL:
	return (ogLibPixelCheck(1, 1, rgbColors[3]) ||
                ogLibPixelCheck(4, 1, rgbColors[2]));
      case GL_ALWAYS:
	return (ogLibPixelCheck(1, 1, rgbColors[3]) ||
                ogLibPixelCheck(4, 1, rgbColors[0]));
    }
    return 0;                   /* Stupid Lint */
}

static GLboolean
chkMs(int zf)
{
    switch (zff[zf]) {
      case GL_NEVER:
        return (ogLibRectCheck(1, 1, 1, 1, 0, 0) ||
                ogLibRectCheck(4, 1, 4, 1, 0, 0));
      case GL_LESS:
	return (ogLibRectCheck(1, 1, 1, 1, rgbColors[0], 0) ||
                ogLibRectCheck(4, 1, 4, 1, rgbColors[1], 0));
      case GL_EQUAL:
	return (ogLibRectCheck(1, 1, 1, 1, 0, 0) ||
                ogLibRectCheck(4, 1, 4, 1, 0, 0));
      case GL_LEQUAL:
	return (ogLibRectCheck(1, 1, 1, 1, rgbColors[1], 0) ||
                ogLibRectCheck(4, 1, 4, 1, rgbColors[0], 0));
      case GL_GREATER:
	return (ogLibRectCheck(1, 1, 1, 1, rgbColors[2], 0) ||
                ogLibRectCheck(4, 1, 4, 1, rgbColors[3], 0));
      case GL_NOTEQUAL:
	return (ogLibRectCheck(1, 1, 1, 1, rgbColors[2], 0) ||
                ogLibRectCheck(4, 1, 4, 1, rgbColors[1], 0));
      case GL_GEQUAL:
	return (ogLibRectCheck(1, 1, 1, 1, rgbColors[3], 0) ||
                ogLibRectCheck(4, 1, 4, 1, rgbColors[2], 0));
      case GL_ALWAYS:
	return (ogLibRectCheck(1, 1, 1, 1, rgbColors[3], 0) ||
                ogLibRectCheck(4, 1, 4, 1, rgbColors[0], 0));
    }
    return 0;                   /* Stupid Lint */
}

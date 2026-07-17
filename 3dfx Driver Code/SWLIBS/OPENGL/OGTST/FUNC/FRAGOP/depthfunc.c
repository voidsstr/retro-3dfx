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

/* depthfunc.c - $Revision: 2$ */

/*
 * This program tests depthfunc 
 *
 * Debug levels: 1 -  2 -  
 */

#include "ogtst.h"		/* include test environment		 */

static GLenum zfunc[] = {
    GL_NEVER,
    GL_LESS,
    GL_EQUAL,
    GL_LEQUAL,
    GL_GREATER,
    GL_NOTEQUAL,
    GL_GEQUAL,
    GL_ALWAYS,};
#define NFUNCS (sizeof(zfunc)/sizeof(GLenum))

static char *zname[] = {
    "GL_NEVER",
    "GL_LESS",
    "GL_EQUAL",
    "GL_LEQUAL",
    "GL_GREATER",
    "GL_NOTEQUAL",
    "GL_GEQUAL",
    "GL_ALWAYS",
};

static unsigned int
expectedColor(GLenum func, GLuint clr1, GLuint clr2);
static int callRectCheck(int, int, unsigned int);
static void firstDrawMs(register int, register int, register int,
                        unsigned int);
static void drawOneMsRGB(register int, register int, register int,
                       unsigned int);
static void drawOneRGB(register int, register int, register int,
                       unsigned int);
static void drawOneCI(register int, register int, register int,
                      unsigned int);

TESTMOD(depthfunc)
{
    GLuint white;
    int k, obj, order[8];
    register int x, y, z1, z2, zf, temp;
    int xmax, ymax, zi;
    void (*firstDraw)(register int, register int, register int,
                      unsigned int);
    void (*secondDraw)(register int, register int, register int,
                       unsigned int);
    int (*libCheckFunc) (int, int, unsigned int);
    GLboolean rgbMode;

    ogEnvLog(OG_LPARAMETERS, "glEnable(GL_DEPTH_TEST)\n");
    glEnable(GL_DEPTH_TEST);

    xmax = ogEnvQuery(OG_XWSIZE) - 3; /* -3 because of possible Multisampling */
    ymax = ogEnvQuery(OG_YWSIZE) - 3;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., ogEnvQuery(OG_XWSIZE), 0., ogEnvQuery(OG_YWSIZE), -xmax - 1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    rgbMode = ogEnvCurVisualInfo(GLX_RGBA);
    secondDraw = rgbMode ? drawOneRGB : drawOneCI;
    if (ogEnvIsMultiSampled()) {
        libCheckFunc = callRectCheck;
        firstDraw = firstDrawMs;
	secondDraw = drawOneMsRGB;
    } else {
        libCheckFunc = ogLibPixelCheck;
        firstDraw = secondDraw;
    }
    white = rgbMode ? 0xffffffff : (1 << ogEnvCurVisualInfo(GLX_BUFFER_SIZE)) - 1;
    while (pass--) {
        /* randomly order the functions */
        ogLibOrderRand(order, NFUNCS); 
	
	/* test all possible functiosn */
	for (k = 0; k < NFUNCS; k++) { 
	    x = ogLibIntRand(2,xmax); /* Start at 2 to allow a 5x5 ms clear */
	    y = ogLibIntRand(2,ymax);
	    z1 = ogLibIntRand(0,xmax);
	    do
		z2 = ogLibIntRand(0,xmax);
	    while (z2 == z1);
	    if (z2 <= z1) {
		temp = z1;
		z1 = z2;
		z2 = temp;
	    }
	    
            ogEnvLog(OG_LPARAMETERS, "glDepthFunc(GL_ALWAYS);\n");
	    glDepthFunc(GL_ALWAYS);
            firstDraw(x, y, z1, white);
            (void) libCheckFunc(x, y, white);
	    
	    if (obj = ogLibBitRand(1)) {
		glNewList(1, GL_COMPILE);
		ogEnvLog(OG_LDLIMM, "display-list mode\n");
	    } else
		ogEnvLog(OG_LDLIMM, "immediate mode\n");

            zf = zfunc[zi = order[k]];
            
            ogEnvLog(OG_LPARAMETERS, "glDepthFunc(%s);\n", zname[zi]);
	    glDepthFunc(zf);
	    
	    if (obj) {
		glEndList();
		glDepthFunc(GL_ALWAYS);
		glCallList(1);
		glDeleteLists(1, 1);
	    }
            
	    secondDraw(x, y, z2, 0);
            (void) libCheckFunc(x, y, expectedColor(zf, white, 0));
        }
    }
}

CLEANUP(depthfunc)
{
    glDisable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();
}

static int
callRectCheck(int x, int y, unsigned int color)
{
    return ogLibRectCheck(x, y, x, y, color, 0);
}

static void
drawOneCI(register int x, register int y, register int z, unsigned int color)
{
    ogEnvLog(OG_LPARAMETERS, "glBegin(GL_POINTS); glIndexi(%d); \
glVertex3i(%d, %d, %d); glEnd();\n", color, x, y, z);
    glBegin(GL_POINTS);
    glIndexi(color);
    glVertex3i(x, y, z);
    glEnd();
}

static void
firstDrawMs(register int x, register int y, register int z, unsigned int color)
{
    ogEnvLog(OG_LPARAMETERS, "glColorub(0x%x); glBegin(GL_QUADS); glVertex3i(%d, %d, %d);glEnd();\n", color, x, y, z);

    glColor4ub(0, 0, 0, 0);
    glRecti(x-2, y-2, x+2, y+2);

    glColor4ub(color & 0xff, (color >> 8) & 0xff, (color >> 16) & 0xff,
               (color >> 24) & 0xff);
    glBegin(GL_QUADS);
	glVertex3i(x, y, z);
	glVertex3i(x, y+1, z);
	glVertex3i(x+1, y+1, z);
	glVertex3i(x+1, y, z);
    glEnd();
}

static void
drawOneMsRGB(register int x, register int y, register int z, unsigned int color)
{
    ogEnvLog(OG_LPARAMETERS, "glColorub(0x%x); glBegin(GL_QUADS); \
glVertex3i(%d, %d, %d); glEnd();\n", color, x, y, z);
    glColor4ub(color & 0xff, (color >> 8) & 0xff, (color >> 16) & 0xff,
               (color >> 24) & 0xff);
    glBegin(GL_QUADS);
	glVertex3i(x, y, z);
	glVertex3i(x, y+1, z);
	glVertex3i(x+1, y+1, z);
	glVertex3i(x+1, y, z);
    glEnd();
}

static void
drawOneRGB(register int x, register int y, register int z, unsigned int color)
{
    ogEnvLog(OG_LPARAMETERS, "glBegin(GL_POINTS); glColorub(0x%x); \
glVertex3i(%d, %d, %d); glEnd();\n", color, x, y, z);
    glBegin(GL_POINTS);
    glColor4ub(color & 0xff, (color >> 8) & 0xff, (color >> 16) & 0xff,
               (color >> 24) & 0xff);
    glVertex3i(x, y, z);
    glEnd();
}

static unsigned int
expectedColor(GLenum func, GLuint clr1, GLuint clr2)
{
    switch (func) {
      case GL_NEVER:
      case GL_EQUAL:
      case GL_GREATER:
      case GL_GEQUAL:
	return clr1;
      case GL_LESS:
      case GL_LEQUAL:
      case GL_NOTEQUAL:
      case GL_ALWAYS:
	return clr2;
    }
    return 0;                   /* Stupid Lint */
}

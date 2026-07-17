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

/* clearall.c - $Revision: 2$ */

/*
 * This program tests the viewport/clear commands. 
 *
 * Debug levels: 1 - print the viewport/clear parameters 2 - print
 * display-list/immediate mode 
 */

#include <stdio.h>
#include "ogtst.h"		/* include test environment		 */

#define DEPTH_TOLERANCE		0.0001
static float depth_tolerance = DEPTH_TOLERANCE;
#define STENCIL_TOLERANCE	0

#define DLIST_FREC		3
#define CLEARS_PER_PASS	3

#define NUM_COLOR_BUFFERS	20

static GLint xmax, ymax, left, right, bottom, top;
static int numColorBuffers, numAuxBuffers, numBufferBits;
static GLenum allColorBuffers[NUM_COLOR_BUFFERS], colorBuffers;
static GLbitfield allBufferBits[4], clearBufferBits, availBufferBits;
static GLboolean rgbMode, doubleBuffer, stereo;
static GLint depthBits, stencilBits;
static GLuint colorValue, accumValue;
static GLint stencilValue;
static float depthValue;

static void selectBuffers(void);
static void setClearRect(void);
static void setClearValues(GLboolean);
static void checkClearRect(void);
static void initClearTest(void);
int rectCheckDepth(GLint, GLint, GLint, GLint, GLclampd, GLclampd, GLfloat);
int rectCheckStencil(GLint, GLint, GLint, GLint, GLint, GLint);

#define VERT	0
#define HORIZ	1


TESTMOD(clearall)
{
    int i;
    GLboolean doDlist;

    initClearTest();
    while (pass--) {

	selectBuffers();

	for (i = 0; i < CLEARS_PER_PASS; i ++ ){
	    glViewport(0, 0, xmax + 1, ymax + 1);
	    glScissor(0, 0, xmax + 1, ymax + 1);
	    setClearValues(0);
            ogEnvLog(OG_LPARAMETERS,
                     "glDrawBuffer(FRONT_AND_BACK); glClear(0x%x);\n",
                     availBufferBits);
	    glDrawBuffer(GL_FRONT_AND_BACK);
	    glClear(availBufferBits);
            if (numAuxBuffers != 0) {
                int j;
                for (j = GL_AUX0 + numAuxBuffers - 1; j >= GL_AUX0; j--) {
                    glDrawBuffer(j);
                    glClear(GL_COLOR_BUFFER_BIT);
                }
            }
            if (clearBufferBits & GL_COLOR_BUFFER_BIT) {
                ogEnvLog(OG_LPARAMETERS, "glDrawBuffer(0x%x)\n", colorBuffers);
                glDrawBuffer(colorBuffers);
            }

	    doDlist = (i % DLIST_FREC) == 0;
	    if (doDlist){
		ogEnvLog(OG_LPARAMETERS, "display-list mode\n");
		glNewList(1, GL_COMPILE);
	    }else
		ogEnvLog(OG_LPARAMETERS, "immediate mode\n");

	    setClearRect();
	    setClearValues(1);

	    glClear(clearBufferBits);
	    if (doDlist){
		glEndList();
		glCallList(1);
		glDeleteLists(1, 1);
	    }
	    checkClearRect();
	}

    }

    /* XXX test non existing buffers */
}

void selectBuffers(void)
{
    int bits, i;
    char msg[256];

    while ((bits = ogLibBitRand(numBufferBits)) == 0);

    clearBufferBits = 0;
    for (i = 0; i < numBufferBits; i++){
	if (bits & (1 << i))
	    clearBufferBits |= allBufferBits[i];
    }
    if (ogEnvWillOutput(OG_LGENERAL))
        i = sprintf(msg, "Buffer(s) cleared: %s%s%s",
                    clearBufferBits & GL_DEPTH_BUFFER_BIT ? " DEPTH" : "", 
                    clearBufferBits & GL_STENCIL_BUFFER_BIT ? " STENCIL" : "", 
                    clearBufferBits & GL_ACCUM_BUFFER_BIT ? " ACCUM" : "");
    if (clearBufferBits & GL_COLOR_BUFFER_BIT) {
        colorBuffers = allColorBuffers[ogLibIntRand(0, numColorBuffers-1)];
        if (ogEnvWillOutput(OG_LGENERAL)) {
            i += sprintf(&msg[i], " COLOR (");
            if (colorBuffers >= GL_AUX0)
                i += sprintf(&msg[i], "GL_AUX%d)", colorBuffers - GL_AUX0);
            else
                i += sprintf(&msg[i], "%s)",
                             colorBuffers == GL_FRONT_LEFT ? "GL_FRONT_LEFT" :
                             colorBuffers == GL_FRONT_RIGHT ? "GL_FRONT_RIGHT" :
                             colorBuffers == GL_BACK_LEFT ? "GL_BACK_LEFT" :
                             colorBuffers == GL_BACK_RIGHT ? "GL_BACK_RIGHT" :
                             colorBuffers == GL_FRONT ? "GL_FRONT" :
                             colorBuffers == GL_BACK ? "GL_BACK" :
                             colorBuffers == GL_LEFT ? "GL_LEFT" :
                             colorBuffers == GL_RIGHT ? "GL_RIGHT" :
                             colorBuffers == GL_FRONT_AND_BACK ?
                             "GL_FRONT_AND_BACK" : "GL_NONE");
        }
    }
    if (ogEnvWillOutput(OG_LGENERAL)) {
        sprintf(&msg[i], ".\n");
        ogEnvLog(OG_LGENERAL, msg);
    }
}

void setClearRect(void)
{
    GLint tmp;

    left = ogLibIntRand(1, xmax - 4);
    right = ogLibIntRand(1, xmax - 4);
    if (left > right){
	tmp = left;
	left = right;
	right = tmp;
    }

    bottom = ogLibIntRand(1,ymax - 4);
    top = ogLibIntRand(1,ymax - 4);
    if (bottom > top){
	tmp = bottom;
	bottom = top;
	top = tmp;
    }

    glViewport(left, bottom, right - left + 1, top - bottom + 1);
    glScissor(left, bottom, right - left + 1, top - bottom + 1);

    ogEnvLog(OG_LPARAMETERS,"clear rectangle: left %d right %d bottom %d top %d\n",
             left, right, bottom, top);

}

void setClearValues(GLboolean notZeroValue)
{
    /* Color is always available */
    if (notZeroValue)
        colorValue = ogLibClearColor();
    else {
        if (rgbMode)
            glClearColor(0, 0, 0, 0);
        else	/* CI mode */
            glClearIndex(0);
    }
    if (availBufferBits & GL_DEPTH_BUFFER_BIT) {
	if (notZeroValue){
	    depthValue = ogLibFloatRand(0.0, 1.0);
	    glClearDepth(depthValue);
	}else
	    glClearDepth(0.0);
    }

    if (availBufferBits & GL_ACCUM_BUFFER_BIT) {
	if (notZeroValue){
	    int r, g, b, a;
	    r = ogLibBitRand(8);
	    g = ogLibBitRand(8);
	    b = ogLibBitRand(8);
	    a = ogLibBitRand(8);
	    accumValue = (r<<24) | (g << 16) | (b << 8) | (a);
	    glClearAccum(r/255.0, g/255.0, b/255.0, a/255.0);
	}else
	    glClearAccum(0.0, 0.0, 0.0, 0.0);
    }
    if (availBufferBits & GL_STENCIL_BUFFER_BIT) {
	if (notZeroValue){
	    stencilValue = ogLibBitRand(stencilBits);
	    glClearStencil(stencilValue);
	}else
	    glClearStencil(0);
    }
}

void
checkClearRect(void)
{
    GLuint colorCleared = clearBufferBits & GL_COLOR_BUFFER_BIT;

    ogEnvLog(OG_LPARAMETERS, "Check front_left buffer\n");
    glReadBuffer(GL_FRONT_LEFT);
    ogLibRectCheck(left, bottom, right, top,
                   colorCleared && (colorBuffers == GL_LEFT ||
                                    colorBuffers == GL_FRONT_LEFT ||
                                    colorBuffers == GL_FRONT ||
                                    colorBuffers == GL_FRONT_AND_BACK) ?
                   colorValue : 0, 0);

    if (doubleBuffer) {
        ogEnvLog(OG_LPARAMETERS, "Check back_left buffer\n");
        glReadBuffer(GL_BACK_LEFT);
        ogLibRectCheck(left, bottom, right, top,
                       colorCleared && (colorBuffers == GL_LEFT ||
                                        colorBuffers == GL_BACK_LEFT ||
                                        colorBuffers == GL_BACK ||
                                        colorBuffers == GL_FRONT_AND_BACK) ?
                       colorValue : 0, 0);
    }
    if (stereo) {
        GLboolean frontRightCleared = GL_FALSE, backRightCleared = GL_FALSE;
        if (colorCleared)
            switch (colorBuffers) {
              case GL_BACK:
              case GL_BACK_RIGHT:
                backRightCleared = GL_TRUE;
                break;
              case GL_FRONT_AND_BACK:
              case GL_RIGHT:
                backRightCleared = GL_TRUE;
              case GL_FRONT_RIGHT:
              case GL_FRONT:
                frontRightCleared = GL_TRUE;
            }
        ogEnvLog(OG_LPARAMETERS, "Check front_right buffer\n");
        glReadBuffer(GL_FRONT_RIGHT);
        ogLibRectCheck(left, bottom, right, top,
                       frontRightCleared ? colorValue : 0, 0);
        if (doubleBuffer) {
            ogEnvLog(OG_LPARAMETERS, "Check back_right buffer\n");
            glReadBuffer(GL_BACK_RIGHT);
            ogLibRectCheck(left, bottom, right, top,
                           backRightCleared ? colorValue : 0, 0);
        }
    }
    if (numAuxBuffers != 0) {
        int j;
        for (j = GL_AUX0 + numAuxBuffers - 1; j >= GL_AUX0; j--) {
            ogEnvLog(OG_LPARAMETERS, "Check AUX_BUFFER %d\n", j - GL_AUX0);
            glReadBuffer(j);
            ogLibRectCheck(left, bottom, right, top,
                           colorCleared && colorBuffers == j ? colorValue : 0,
                           0);
        }
    }
    if (availBufferBits & GL_DEPTH_BUFFER_BIT) {
	ogEnvLog(OG_LPARAMETERS, "Check depth buffer\n");
        rectCheckDepth(left, bottom, right, top,
                       clearBufferBits & GL_DEPTH_BUFFER_BIT ? depthValue : 0,
                       0, depth_tolerance);
    }

    if (availBufferBits & GL_STENCIL_BUFFER_BIT) {
	ogEnvLog(OG_LPARAMETERS, "Check stencil buffer\n");
        rectCheckStencil(left, bottom, right, top,
                         clearBufferBits & GL_STENCIL_BUFFER_BIT ?
                         stencilValue : 0, 0);
    }
    if (availBufferBits & GL_ACCUM_BUFFER_BIT){
	ogEnvLog(OG_LPARAMETERS, "Check accum buffer\n");
	glViewport(0, 0, xmax + 1, ymax + 1);
	glScissor(0, 0, xmax + 1, ymax + 1);

	glDrawBuffer(GL_FRONT_LEFT);
	glAccum(GL_RETURN, 1.0);
	glReadBuffer(GL_FRONT_LEFT);
	if (clearBufferBits & GL_ACCUM_BUFFER_BIT)
	    ogLibRectCheck(left, bottom, right, top, accumValue, 0);
	else
	    ogLibRectCheck(left, bottom, right, top, 0, 0);
        if (clearBufferBits & GL_COLOR_BUFFER_BIT)
            glDrawBuffer(colorBuffers);
    }
}

void
initClearTest(void)
{
    int i;
    int accumBits[4];
    
    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    rgbMode = ogEnvCurVisualInfo(GLX_RGBA);;

	/* define set of color buffers */

    numColorBuffers = 0;
    allColorBuffers[numColorBuffers] = GL_FRONT;
    numColorBuffers ++;
    allColorBuffers[numColorBuffers] = GL_LEFT;
    numColorBuffers ++;
    allColorBuffers[numColorBuffers] = GL_FRONT_LEFT;
    numColorBuffers ++;

    doubleBuffer = ogEnvCurVisualInfo(GLX_DOUBLEBUFFER);
    if (doubleBuffer){
	allColorBuffers[numColorBuffers] = GL_BACK;
	numColorBuffers ++;
	allColorBuffers[numColorBuffers] = GL_BACK_LEFT;
	numColorBuffers ++;
	allColorBuffers[numColorBuffers] = GL_FRONT_AND_BACK;
	numColorBuffers ++;
    }

    stereo = ogEnvCurVisualInfo(GLX_STEREO);
    if (stereo){
	allColorBuffers[numColorBuffers] = GL_RIGHT;
	numColorBuffers ++;
	allColorBuffers[numColorBuffers] = GL_FRONT_RIGHT;
	numColorBuffers ++;
        if (doubleBuffer) {
            allColorBuffers[numColorBuffers] = GL_BACK_RIGHT;
            numColorBuffers ++;
        }
    }

    numAuxBuffers = ogEnvCurVisualInfo(GLX_AUX_BUFFERS);
    for (i = 0; i < numAuxBuffers; i++) {
	allColorBuffers[numColorBuffers] = GL_AUX0 + i;
	numColorBuffers ++;
    }

    /* define set of buffers bits */

    numBufferBits = 0;
    allBufferBits[numBufferBits] = GL_COLOR_BUFFER_BIT;
    numBufferBits ++;

    depthBits = ogEnvCurVisualInfo(GLX_DEPTH_SIZE);
    if (depthBits) {
        allBufferBits[numBufferBits] = GL_DEPTH_BUFFER_BIT;
        numBufferBits ++;
    }

    {
        stencilBits = ogEnvCurVisualInfo(GLX_STENCIL_SIZE);
        if (stencilBits) {
            allBufferBits[numBufferBits] = GL_STENCIL_BUFFER_BIT;
            numBufferBits ++;
        }
    }

    if (rgbMode){
	accumBits[0] = ogEnvCurVisualInfo(GLX_ACCUM_RED_SIZE);
	accumBits[1] = ogEnvCurVisualInfo(GLX_ACCUM_GREEN_SIZE);
	accumBits[2] = ogEnvCurVisualInfo(GLX_ACCUM_BLUE_SIZE);
	accumBits[3] = ogEnvCurVisualInfo(GLX_ACCUM_ALPHA_SIZE);
	if (accumBits[0] != 0 || accumBits[1] != 0 ||
            accumBits[2] != 0 || accumBits[3] != 0) {
            allBufferBits[numBufferBits] = GL_ACCUM_BUFFER_BIT;
            numBufferBits ++;
        }
    }

    for (i = 0, availBufferBits = 0; i < numBufferBits; i++) {
	availBufferBits |= allBufferBits[i];
    }
    ogEnvLog(OG_LPARAMETERS, "glEnable(GL_SCISSOR_TEST);\n");
    glEnable(GL_SCISSOR_TEST);
}

int
lineCheckDepth(GLint x, GLint y, GLint length, GLint horiz, GLclampd depth,
               GLfloat tolerance)
{
    int i, err;
    GLfloat pixels[700];

    err = 0;

    if (horiz)
	glReadPixels(x, y, length, 1, GL_DEPTH_COMPONENT, GL_FLOAT, 
							(GLvoid *) pixels);
    else	/* vertical */
	glReadPixels(x, y, 1, length, GL_DEPTH_COMPONENT, GL_FLOAT, 
							(GLvoid *) pixels);

    for (i = 0; i < length; i ++){
	if (ABS(pixels[i] - depth) > tolerance){
	    err ++;
	    ogEnvLog(OG_LFAIL, "pixel (%d,%d) depth is %f instead of %f\n", 
		(horiz ? x + i : x), (horiz ? y : y + i), pixels[i], depth);
	}
    }

    return err;
}

int
rectCheckDepth(GLint left, GLint bottom, GLint right, GLint top, 
               GLclampd insideDepth, GLclampd outsideDepth,
               GLfloat tolerance)
{
    int err, dx, dy, x, y;

    err = 0;
    dx = right - left + 1;
    dy = top - bottom + 1;

	/* check outside area */
    err += lineCheckDepth(left - 1, bottom - 1, dy + 2, VERT,
                          outsideDepth, tolerance);
    err += lineCheckDepth(right + 1, bottom - 1, dy + 2, VERT,
                          outsideDepth, tolerance);
    err += lineCheckDepth(left - 1, bottom - 1, dx + 2, HORIZ,
                          outsideDepth, tolerance);
    err += lineCheckDepth(left - 1, top + 1, dx + 2, HORIZ,
                          outsideDepth, tolerance);

	/* check inside area */
    err += lineCheckDepth(left, bottom, dy, VERT, insideDepth, tolerance);
    err += lineCheckDepth(right, bottom, dy, VERT, insideDepth, tolerance);
    err += lineCheckDepth(left, bottom, dx, HORIZ, insideDepth, tolerance);
    err += lineCheckDepth(left, top, dx, HORIZ, insideDepth, tolerance);

    if (dx > 4){
	x = ogLibIntRand(left + 1, right - 1);
	err = lineCheckDepth(x, bottom, dy, VERT, insideDepth, tolerance);
	x = ogLibIntRand(left + 1, right - 1);
	err = lineCheckDepth(x, bottom, dy, VERT, insideDepth, tolerance);
    }
    if (dy > 4){
	y = ogLibIntRand(bottom + 1, top - 1);
	err = lineCheckDepth(left, y, dx, HORIZ, insideDepth, tolerance);
	y = ogLibIntRand(bottom + 1, top - 1);
	err = lineCheckDepth(left, y, dx, HORIZ, insideDepth, tolerance);
    }

    return err;
}

static int
lineCheckStencil(GLint x, GLint y, GLint length, GLint horiz, GLint stencil)
{
    int i, err;
    GLint pixels[700];

    err = 0;

    if (horiz)
	glReadPixels(x, y, length, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, 
                     (GLvoid *) pixels);
    else	/* vertical */
	glReadPixels(x, y, 1, length, GL_STENCIL_INDEX, GL_UNSIGNED_INT, 
                     (GLvoid *) pixels);

    for (i = 0; i < length; i ++){
	if (ABS(pixels[i] - stencil) > STENCIL_TOLERANCE){
	    err ++;
	    ogEnvLog(OG_LFAIL, 
                     "pixel (%d,%d) stencil is 0x%x instead of 0x%x\n", 
                     (horiz ? x + i : x), (horiz ? y : y + i), pixels[i],
                     stencil);
	}
    }
    return err;
}

int
rectCheckStencil(GLint left, GLint bottom, GLint right, GLint top, 
                 GLint insideStencil, GLint outsideStencil)
{
    int err, dx, dy, x, y;

    err = 0;
    dx = right - left + 1;
    dy = top - bottom + 1;

	/* check outside area */
    err += lineCheckStencil(left - 1, bottom - 1, dy + 2, VERT, outsideStencil);
    err += lineCheckStencil(right + 1, bottom - 1, dy + 2, VERT,outsideStencil);
    err += lineCheckStencil(left - 1, bottom - 1, dx + 2, HORIZ,outsideStencil);
    err += lineCheckStencil(left - 1, top + 1, dx + 2, HORIZ, outsideStencil);

	/* check inside area */
    err += lineCheckStencil(left, bottom, dy, VERT, insideStencil);
    err += lineCheckStencil(right, bottom, dy, VERT, insideStencil);
    err += lineCheckStencil(left, bottom, dx, HORIZ, insideStencil);
    err += lineCheckStencil(left, top, dx, HORIZ, insideStencil);

    if (dx > 4){
	x = ogLibIntRand(left + 1, right - 1);
	err = lineCheckStencil(x, bottom, dy, VERT, insideStencil);
	x = ogLibIntRand(left + 1, right - 1);
	err = lineCheckStencil(x, bottom, dy, VERT, insideStencil);
    }
    if (dy > 4){
	y = ogLibIntRand(bottom + 1, top - 1);
	err = lineCheckStencil(left, y, dx, HORIZ, insideStencil);
	y = ogLibIntRand(bottom + 1, top - 1);
	err = lineCheckStencil(left, y, dx, HORIZ, insideStencil);
    }

    return err;
}

CLEANUP(clearall)
{
    glDisable(GL_SCISSOR_TEST);
    glScissor(0, 0,  ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    glViewport(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    ogLibSetDefaultBuffers();
    ogLibSetDefaultClears();
}

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

/* gltst.c - $Revision: 2$ */

#include <math.h>
#include "ogtst.h"

/*************************************************************
*  ogLibClear()  -  clear screen
*************************************************************/
void 
ogLibClear(GLuint clear_col)
{

    if (ogEnvIsCIMode()) {
	glClearIndex(clear_col);
	ogEnvLog(OG_LPARAMETERS, "glClearIndex(0x%lx);\n",clear_col);
    } else {
        float cf[4];

	cf[3] = ((clear_col >>  0) & 0xff) / 255.0;
	cf[2] = ((clear_col >>  8) & 0xff) / 255.0;
	cf[1] = ((clear_col >> 16) & 0xff) / 255.0;
	cf[0] = ((clear_col >> 24) & 0xff) / 255.0;
	glClearColor(cf[0], cf[1], cf[2], cf[3]);
	ogEnvLog(OG_LPARAMETERS, "glClearColor(%f %f %f %f);\n",cf[0], cf[1], cf[2], cf[3]);
    }

    glClear(GL_COLOR_BUFFER_BIT);
    ogEnvLog(OG_LPARAMETERS, "glClear(GL_COLOR_BUFFER_BIT);\n");
}

/*************************************************************
*  ogLibColor()  -  generate and set a random color
*************************************************************/
static int
BitsToColor(int bits)
{

    if (bits == 0) {
	return 0;
    } else {
	if (bits > 8) {
	    bits = 8;
	}
	return (int)((255.0/(float)((1<<bits)-1))*(float)ogLibIntRand(0,(1<<bits)-1)+0.5);
    }
}

GLuint 
ogLibColor(void)
{
    GLuint col;
    int r, g, b, a, ci, rBits, gBits, bBits, aBits, ciBits;

    ogEnvColorBits(&rBits, &gBits, &bBits, &aBits, &ciBits);
    if (ogEnvIsCIMode()) {
	while (!(ci = ogLibBitRand(ciBits)));	/* don't use zero */
	glIndexi(ci);
	ogEnvLog(OG_LPARAMETERS,"glIndexi(0x%lx);\n",ci);
	col = ci;
    } else {
	r = BitsToColor(rBits);
	g = BitsToColor(gBits);
	b = BitsToColor(bBits);
	a = BitsToColor(aBits);
	glColor4ub((GLubyte)r, (GLubyte)g, (GLubyte)b, (GLubyte)a);
	ogEnvLog(OG_LPARAMETERS,"glColor4ub(%02x,%02x,%02x,%02x)\n",r,g,b,a);
	col = ((r << 24) & 0xFF000000) | ((g << 16) & 0x00FF0000) |
	      ((b << 8) & 0x0000FF00) | (a & 0x000000FF);
    }
    return (col);
}

/*************************************************************
*  ogLibClearColor()  -  generate and set a random color
*************************************************************/
GLuint
ogLibClearColor(void)
{
    GLuint col;
    int r,g,b,a,bits;

    if (ogEnvIsCIMode()) {
	bits = ogEnvCurVisualInfo(GLX_BUFFER_SIZE);
	while (!(col = ogLibBitRand(bits)));	/* don't use zero */
        glClearIndex((float) col);
        ogEnvLog(OG_LPARAMETERS, "glClearIndex(0x%lx)\n",col);
    } else {
	col = ogLibBitRand(32);
	r = (col>>24) & 0xff;
	g = (col>>16) & 0xff;
	b = (col>> 8) & 0xff;
	a = (col>> 0) & 0xff;
        glClearColor(r/255.0, g/255.0, b/255.0, a/255.0);
        ogEnvLog(OG_LPARAMETERS, "glClearColor(%02x,%02x,%02x,%02x);\n",r,g,b,a);
    }
    return (col);
}

void
ogLibDrawFragments(GLint x, GLint y)
{
    if (ogEnvIsMultiSampled()) {
	glRecti(x,y,x+1,y+1);
	ogEnvLog(OG_LPARAMETERS, "glRecti(%d, %d);\n", x, y);
    } else {
	glBegin(GL_POINTS);
	ogLibSetVertex(x,y,0,1);
	glEnd();
	ogEnvLog(OG_LPARAMETERS, "glBegin(GL_POINTS);glVertex2i(%d, %d);glEnd();\n", x, y);
    }
}

static const char *
errlst[] = {
    "INVALID_ENUM",      /* 0x0500 */
    "INVALID_VALUE",     /* 0x0501 */
    "INVALID_OPERATION", /* 0x0502 */
    "STACK_OVERFLOW",    /* 0x0503 */
    "STACK_UNDERFLOW",   /* 0x0504 */
    "OUT_OF_MEMORY",     /* 0x0505 */
};

/*************************************************************
*
*************************************************************/
const char *
ogLibGLError(GLenum error)
{
    if (error == GL_NO_ERROR)
        return "NO_ERROR";
    else if (error >= GL_INVALID_ENUM && error <= GL_OUT_OF_MEMORY)
        return errlst[error-0x0500];
#ifdef GL_TABLE_TOO_LARGE_EXT
    else if (error == GL_TABLE_TOO_LARGE_EXT)
        return "TABLE_TOO_LARGE_EXT"; /* 0x8031 */
#endif
    else
        return "UNKNOWN_ERROR";
}


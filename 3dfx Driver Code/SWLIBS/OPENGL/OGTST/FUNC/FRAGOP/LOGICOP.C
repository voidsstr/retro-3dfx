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

/* logicop.c - $Revision: 2$ */

/*
 * This program tests the logicop command using points to check. 
 */

#include "ogtst.h"		/* include test environment		 */

static int lp[] = {
    GL_CLEAR,
    GL_AND,
    GL_AND_REVERSE,
    GL_COPY,
    GL_AND_INVERTED,
    GL_NOOP,
    GL_XOR,
    GL_OR,
    GL_NOR,
    GL_EQUIV,
    GL_INVERT,
    GL_OR_REVERSE,
    GL_COPY_INVERTED,
    GL_OR_INVERTED,
    GL_NAND,
    GL_SET,};

static char *lname[] = {
    "GL_CLEAR",
    "GL_AND",
    "GL_AND_REVERSE",
    "GL_COPY",
    "GL_AND_INVERTED",
    "GL_NOOP",
    "GL_XOR",
    "GL_OR",
    "GL_NOR",
    "GL_EQUIV",
    "GL_INVERT",
    "GL_OR_REVERSE",
    "GL_COPY_INVERTED",
    "GL_OR_INVERTED",
    "GL_NAND",
    "GL_SET",};

TESTMOD(logicop) {
    int rBits, gBits, bBits, aBits, ciBits, mask;
    int x, y, lop, obj, li;
    GLint col1, col2, col3;
    int xmax, ymax;
    GLboolean rgba = ogEnvCurVisualInfo(GLX_RGBA);
    GLenum enable = rgba ? GL_COLOR_LOGIC_OP : GL_INDEX_LOGIC_OP;

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    ogLibClear(0);
    glEnable(enable);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., ogEnvQuery(OG_XWSIZE), 0., ogEnvQuery(OG_YWSIZE), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    ogEnvColorBits(&rBits, &gBits, &bBits, &aBits, &ciBits);
    mask = rgba ? ~0L : (1 << ciBits) - 1;

    while (pass--) {
	x = 2 + ogLibIntRand(0,xmax - 4);
	y = 2 + ogLibIntRand(0,ymax - 4);

	glLogicOp(GL_COPY);
	ogEnvLog(1, "glLogicOp(GL_COPY);\n");

	col2 = ogLibColor();
	ogEnvLog(2, "dst color is 0x%08x\n", col2);

	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
	ogEnvLog(2, "glBegin(GL_POINTS);glVertex2i(%d, %d);glEnd();\n", x, y);

	ogLibPixelCheck(x, y, col2);

	START_DL_OR_IM(1);

	lop = lp[li = ogLibBitRand(4)];	/* only 4 low-order bits should be
					 * valid */
	glLogicOp(lop);
	ogEnvLog(1, "glLogicOp(%s);\n", lname[li]);

	if (ogLibBitRand(1)) {
	    glEnable(enable);
	    ogEnvLog(1, "glEnable(GL_LOGIC_OP);\n");
	} else {
	    glDisable(enable);
	    ogEnvLog(1, "glDisable(GL_LOGIC_OP);\n");
	    lop = GL_COPY;
	}

	if (obj) {
	    glEndList();
	    glLogicOp(GL_XOR);
	    glCallList(1);
	    glDeleteLists(1,1);
	}
	col1 = ogLibColor();
	ogEnvLog(2, "src color is 0x%08x\n", col1);

	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
	ogEnvLog(2, "glBegin(GL_POINTS);glVertex2i(%d, %d);glEnd();\n", x, y);

	switch (lop) {
	case GL_CLEAR:
	    col3 = 0x00000000;
	    break;
	case GL_AND:
	    col3 = col1 & col2;
	    break;
	case GL_AND_REVERSE:
	    col3 = col1 & (~col2);
	    break;
	case GL_COPY:
	    col3 = col1;
	    break;
	case GL_AND_INVERTED:
	    col3 = (~col1) & col2;
	    break;
	case GL_NOOP:
	    col3 = col2;
	    break;
	case GL_XOR:
	    col3 = col1 ^ col2;
	    break;
	case GL_OR:
	    col3 = col1 | col2;
	    break;
	case GL_NOR:
	    col3 = ~(col1 | col2);
	    break;
	case GL_EQUIV:
	    col3 = ~(col1 ^ col2);
	    break;
	case GL_INVERT:
	    col3 = ~col2;
	    break;
	case GL_OR_REVERSE:
	    col3 = col1 | (~col2);
	    break;
	case GL_COPY_INVERTED:
	    col3 = ~col1;
	    break;
	case GL_OR_INVERTED:
	    col3 = (~col1) | col2;
	    break;
	case GL_NAND:
	    col3 = ~(col1 & col2);
	    break;
	case GL_SET:
	    col3 = 0xFFFFFFFF;
	    break;
	}
	col3 &= mask;

	ogEnvLog(1, "color should be: 0x%08x\n", col3);
	ogLibPixelCheck(x, y, col3);
    }
}

CLEANUP(logicop) {
    glLogicOp(GL_COPY);
    glDisable(GL_INDEX_LOGIC_OP);
    glDisable(GL_COLOR_LOGIC_OP);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();
}

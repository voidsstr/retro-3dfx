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

/* matrixflow.c - $Revision: 2$ */

/*
 * This program tests for correct detection of matrix over/under flow
 *
 * Debug levels: 1 - print the push/pop levels
 * 2 - print the loaded matrix 3 - print display-list/immediate mode 
 */

#include "ogtst.h"		/* include test environment		 */
#include "xform.h"		/* include test environment		 */

static void test_stack(GLenum which, int npasses);

TESTMOD(matrixflow)
{
    ogEnvLog(1, "test GL_MODELVIEW\n");
    test_stack(GL_MODELVIEW, pass);
    ogEnvLog(1, "test GL_PROJECTION\n");
    test_stack(GL_PROJECTION, pass);
    ogEnvLog(1, "test GL_TEXTURE\n");
    test_stack(GL_TEXTURE, pass);
}

CLEANUP(matrixflow)
{
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

static void
test_stack(GLenum which, int npasses) {
    register int i;
    int obj;
    GLint depth, v;
    GLenum error, getname;

    switch(which) {
    case GL_MODELVIEW:
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &depth);
	getname = GL_MODELVIEW_STACK_DEPTH;
	break;
    case GL_PROJECTION:
	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &depth);
	getname = GL_PROJECTION_STACK_DEPTH;
	break;
    case GL_TEXTURE:
	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &depth);
	getname = GL_TEXTURE_STACK_DEPTH;
	break;
    }
    ogEnvLog(1, "max stack depth = %d\n", depth);

    glMatrixMode(which);

    while (npasses--) {
	for(i = 0; i < depth-1; i++) {
	    ogEnvLog(1, "depth = %d\n", i+1);
	    ogEnvLog(1, "glPushMatrix();\n");
	    START_DL_OR_IM(1);
	    glPushMatrix();
	    FINIS_DL_OR_IM(1);
	    if ((error = glGetError()) != GL_NO_ERROR) {
		ogEnvLog(OG_LFAIL, "premature glError(%s)\n", ogLibGLError(error));
	    }

	    glGetIntegerv(getname, &v);
	    ogEnvLog(1, "glGetIntegerv(..._DEPTH, %d);\n", v);
	    if (v != i+2) {
		ogEnvLog(OG_LFAIL, "glGetIntegerv(..._DEPTH) expected %d observed %d\n", i+2, v);
	    }

	}

	/* this push should cause the error */
	ogEnvLog(1, "depth = %d\n", i+1);
	ogEnvLog(1, "glPushMatrix();\n");
	START_DL_OR_IM(1);
	glPushMatrix();
	FINIS_DL_OR_IM(1);
	if ((error = glGetError()) != GL_STACK_OVERFLOW) {
	    if (error != GL_NO_ERROR)
		ogEnvLog(OG_LFAIL, "wrong glError(%s) wanted glError(GL_STACK_OVERFLOW)\n", ogLibGLError(error));
	    else
		ogEnvLog(OG_LFAIL, "missing glError(GL_STACK_OVERFLOW)\n");
	}

	/* unwind */

	for(i = 0; i < depth-1; i++) {
	    ogEnvLog(1, "depth = %d\n", depth-i);
	    ogEnvLog(1, "glPopMatrix();\n");
	    START_DL_OR_IM(1);
	    glPopMatrix();
	    FINIS_DL_OR_IM(1);
	    if ((error = glGetError()) != GL_NO_ERROR) {
		ogEnvLog(OG_LFAIL, "premature glError(%s)\n", ogLibGLError(error));
	    }
	    glGetIntegerv(getname, &v);
	    ogEnvLog(1, "glGetIntegerv(..._DEPTH, %d);\n", v);
	    if (v != depth-i-1) {
		ogEnvLog(OG_LFAIL, "glGetIntegerv(..._DEPTH) expected %d observed %d\n", depth-i-1, v);
	    }
	}

	/* this pop should cause the error */
	ogEnvLog(1, "depth = %d\n", depth-i);
	ogEnvLog(1, "glPopMatrix();\n");
	START_DL_OR_IM(1);
	glPopMatrix();
	FINIS_DL_OR_IM(1);
	if ((error = glGetError()) != GL_STACK_UNDERFLOW) {
	    if (error != GL_NO_ERROR)
		ogEnvLog(OG_LFAIL, "wrong glError(%s) wanted glError(GL_STACK_UNDERFLOW)\n", ogLibGLError(error));
	    else
		ogEnvLog(OG_LFAIL, "missing glError(GL_STACK_UNDERFLOW)\n");
	}
    }
}

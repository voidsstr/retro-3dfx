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

/* popmatrix.c - $Revision: 2$ */

/*
 * This program tests popmatrix/pushmatrix by loading a random matrix and
 * then pushing and popping up to 31 times. 
 *
 * Default error tolerence is 1.0e-6 Debug levels: 1 - print the push/pop levels
 * 2 - print the loaded matrix 3 - print display-list/immediate mode 
 */

#include "ogtst.h"		/* include test environment		 */
#include "xform.h"		/* include test environment		 */

static void test_stack(GLenum which, int npasses);

TESTMOD(popmatrix)
{
    ogEnvLog(1, "test GL_MODELVIEW\n");
    test_stack(GL_MODELVIEW, pass);
    ogEnvLog(1, "test GL_PROJECTION\n");
    test_stack(GL_PROJECTION, pass);
    ogEnvLog(1, "test GL_TEXTURE\n");
    test_stack(GL_TEXTURE, pass);
}

CLEANUP(popmatrix)
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
    float stma[4][4], mat_in[4][4], identity[4][4];
    int i, j, n;
    int obj, pp;
    GLint maxdepth, depth;
    GLenum getname, getdepthname;

    ogLibUntMatrix(identity[0]);

    switch(which) {
    case GL_MODELVIEW:
	glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &maxdepth);
	getname = GL_MODELVIEW_MATRIX;
	getdepthname = GL_MODELVIEW_STACK_DEPTH;
	break;
    case GL_PROJECTION:
	glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &maxdepth);
	getname = GL_PROJECTION_MATRIX;
	getdepthname = GL_PROJECTION_STACK_DEPTH;
	break;
    case GL_TEXTURE:
	glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &maxdepth);
	getname = GL_TEXTURE_MATRIX;
	getdepthname = GL_TEXTURE_STACK_DEPTH;
	break;
    }
    ogEnvLog(1, "stack max depth = %d\n", maxdepth);

    glMatrixMode(which);

    while (npasses--) {
	for (i = 0; i < 4; i++)	/* init the matrix		 */
	    for (j = 0; j < 4; j++)
		mat_in[i][j] = ogLibExpRand(ogLibIntRand(0,30));

	ogEnvLog(1, "\n");		/* add a blank line		 */
	ogLibPntMatrix(mat_in[0], 2, "Loadmatrix");
	glLoadMatrixf((GLfloat *) mat_in);

	do {
	    pp = ogLibIntRand(0,maxdepth - 1);	/* do this many pushes */
	} while (pp >= maxdepth/2);
	glGetIntegerv(getdepthname, &depth);
	if (depth != 1) {
	    ogEnvLog(OG_LFAIL, "stack depth %d (should be one)\n", depth);
	}

	ogEnvLog(1, "	push %d, compare\n", pp);
	for (n = 0; n < pp; n++) {
	    START_DL_OR_IM(1);
	    glPushMatrix();
	    FINIS_DL_OR_IM(1);
	    glGetFloatv(getname, (GLfloat *) stma);
	    ogLibRcmMatrix(mat_in[0], stma[0]);	/* compare it		 */
	}
	glGetIntegerv(getdepthname, &depth);
	if (pp && depth != pp + 1) {
	    ogEnvLog(OG_LFAIL, "stack depth %d (should be %d)\n", depth, pp+1);
	}

	ogEnvLog(1, "	pop %d, compare\n", pp - 1);
	for (n = 1; n < pp; n++) {	/* pop off all but last one	 */
	    glLoadIdentity(); 
	    START_DL_OR_IM(1);
	    glPopMatrix();
	    FINIS_DL_OR_IM(1);
	    glGetFloatv(getname, (GLfloat *) stma);
	    ogLibRcmMatrix(mat_in[0], stma[0]);	/* compare it		 */
	}
	if (pp > 0) {
	    glGetIntegerv(getdepthname, &depth);
	    if (depth != 2) {
		ogEnvLog(OG_LFAIL, "stack depth %d (should be two)\n", depth);
	    }

	    START_DL_OR_IM(1);
	    glLoadMatrixf((GLfloat *) identity);
	    glPushMatrix();
	    glPushMatrix();
	    glPushMatrix();
	    FINIS_DL_OR_IM(1);
	    ogEnvLog(1, "	load identity, push 3, compare\n");

	    glGetFloatv(getname, (GLfloat *) stma);
	    ogLibRcmMatrix(identity[0], stma[0]);	/* compare it		 */

	    START_DL_OR_IM(1);
	    glPopMatrix();	/* do one more pop to get back	 */
	    glPopMatrix();	/* to original			 */
	    glPopMatrix();
	    glLoadIdentity(); 
	    glPopMatrix();
	    FINIS_DL_OR_IM(1);
	}
	glGetIntegerv(getdepthname, &depth);
	if (depth != 1) {
	    ogEnvLog(OG_LFAIL, "stack depth %d (should be one)\n", depth);
	}

	ogEnvLog(1, "	pop 4, compare to original\n");
	glGetFloatv(getname, (GLfloat *) stma);
	ogLibRcmMatrix(mat_in[0], stma[0]);/* compare it		 */
    }
}

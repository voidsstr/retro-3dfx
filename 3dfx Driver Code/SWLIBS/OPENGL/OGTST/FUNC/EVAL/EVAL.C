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

/* XXX remember to put in flag for dlist or not */

#include <alloca.h>
#include <stdio.h>
#include <bstring.h>
#include "ogtst.h"

#define POINT_TOLERANCE 0.01
#define LINE_TOLERANCE 0.05
#define _DEBUG_EVAL_ 0

#define DO_EVALMESH 1
/* doesn't really exercise anything different, so leave out for now */
#define DO_EVALCOORD 0

static void
use_evalcoord(int dim, GLenum mode, int uints, int vints)
{
    int i, j;
    float v0, v1;
    
    if (dim == 1) {
	switch (mode) {
	  case GL_POINT:
	    glBegin(GL_POINTS);
	    break;
	  case GL_LINE:
	    glBegin(GL_LINE_STRIP);
	    break;
	  default:
	    ogEnvLog(OG_LALWAYS,"bad mode for 1D map\n");
	}
	for (i = 0; i <= uints; i++)
	  glEvalCoord1f((GLfloat)i/uints);
	glEnd();
	    
    } else {
	switch (mode) {
	  case GL_POINT:
	    for (j = 0; j <= vints; j++) {
		glBegin(GL_POINTS);
		for (i = 0; i <= uints; i++)
		  glEvalCoord2f((GLfloat)i/uints, (GLfloat)j/vints);
		glEnd();
	    }
	    break;
	  case GL_LINE:
	    for (j = 0; j <= vints; j++) {
		glBegin(GL_LINE_STRIP);
		for (i = 0; i <= uints; i++)
		  glEvalCoord2f((GLfloat)i/uints, (GLfloat)j/vints);
		glEnd();
	    }
	    for (j = 0; j <= uints; j++) {
		glBegin(GL_LINE_STRIP);
		for (i = 0; i <= vints; i++)
		  glEvalCoord2f((GLfloat)j/uints, (GLfloat)i/vints);
		glEnd();
	    }
	    break;
	  case GL_FILL:
	    for (j = 0; j < vints; j++) {
		v0 = (GLfloat)j/vints;
		v1 = (GLfloat)(j+1)/vints;
		glBegin(GL_QUAD_STRIP);
		for (i = 0; i <= uints; i++) {
		    glEvalCoord2f((GLfloat)i/uints, v0);
		    glEvalCoord2f((GLfloat)i/uints, v1);
		}
		glEnd();
	    }
	    break;
	}
    }
}


static void
do_eval_order(int dim, int uorder, int vorder, int uints, int vints,
	      GLenum prim, float tolerance, int passes, int evalcoord)
{
    GLint xmax, ymax;
    int i, j, n, m;
    float bez[3], u, v;
    GLuint *img1, *img2;
    int w, h, status;
    int ncomp=3;
    float *cpts;
    DL_PROLOG();

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    w = ogEnvQuery(OG_XWSIZE) / 2;
    h = ogEnvQuery(OG_YWSIZE);
    xmax = w - 1;
    ymax = h - 1;
    /*
     * Set the projection so that integer coordinate are at the center of
     * pixels
     */
    glOrtho(0, w, 0, h, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    img1 = ogLibMalloc(w * h * 4 * sizeof(GL_UNSIGNED_BYTE));
    img2 = ogLibMalloc(w * h * 4 * sizeof(GL_UNSIGNED_BYTE));
    cpts = alloca(uorder * vorder * 3 * sizeof(GLfloat));

    for (n=0; n < passes; n++) {
	for (i=0; i < vorder; i++) {
	    for (j=0; j < uorder; j++) {
		m = (i * uorder + j) * ncomp;
		cpts[m+0] = ogLibFloatRand(0, 1) * xmax;
		cpts[m+1] = ogLibFloatRand(0, 1) * ymax;
		cpts[m+2] = ogLibFloatRand(-.8, .8);
	    }
	}
	ogLibClear(0x000000ff);
	glColor3f(1, 0, 0);

	glViewport(0, 0, w, h);

	if (dim==1) {
	    glEnable(GL_MAP1_VERTEX_3);
	    START_DL_OR_IM(1);
	    glMap1f(GL_MAP1_VERTEX_3, 0, 1, 3, uorder, cpts);
	    glMapGrid1f(uints, 0, 1);
	    FINIS_DL_OR_IM(1);

	    START_DL_OR_IM(2);
	    if (evalcoord) {
		use_evalcoord(1, prim, uints, vints);
	    } else {
		glEvalMesh1(prim, 0, uints);
	    }
	    FINIS_DL_OR_IM(2);
	} else {
	    glEnable(GL_MAP2_VERTEX_3);
	    START_DL_OR_IM(1);
	    glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, uorder, 0, 1, uorder*3, vorder,
		    cpts);
	    glMapGrid2f(uints, 0, 1, vints, 0, 1);
	    FINIS_DL_OR_IM(1);

	    START_DL_OR_IM(2);
	    if (evalcoord) {
		use_evalcoord(2, prim, uints, vints);
	    } else {
		glEvalMesh2(prim, 0, uints, 0, vints);
	    }
	    FINIS_DL_OR_IM(2);
	}
	ogLibReadPixels(0, 0, w-1, h-1, img1);

	glViewport(w, 0, w, h);
	switch (prim) {
	    
	  case GL_POINT:
	    if (dim==1) {
		glBegin(GL_POINTS);
		for (i=0; i <= uints; i++) {
		    u = i/(float)uints;
		    ogLibDeCasteljau1( u, uorder, 3, cpts, bez);
		    glVertex3fv(bez);
		}
		glEnd();
	    } else {
		glBegin(GL_POINTS);
		for (i=0; i <= uints; i++) {
		    u = i/(float)uints;
		    for (j=0; j <= vints; j++) {
			v = j/(float)vints;
			ogLibDeCasteljau2(u, uorder, v, vorder, 3, cpts, bez);
			glVertex3fv(bez);
		    }
		}
		glEnd();
	    }
	    break;

	  case GL_LINE:
	    if (dim==1) {
		glBegin(GL_LINE_STRIP);
		for (i=0; i <= uints; i++) {
		    u = i/(float)uints;
		    ogLibDeCasteljau1(u, uorder, 3, cpts, bez);
		    glVertex3fv(bez);
		}
		glEnd();
	    } else {
		for (i=0; i <= uints; i++) {
		    u = i/(float)uints;
		    glBegin(GL_LINE_STRIP);
		    for (j=0; j <= vints; j++) {
			v = j/(float)vints;
			ogLibDeCasteljau2(u, uorder, v, vorder, 3, cpts, bez);
			glVertex3fv(bez);
		    }
		    glEnd();
		}
		for (i=0; i <= vints; i++) {
		    v = i/(float)vints;
		    glBegin(GL_LINE_STRIP);
		    for (j=0; j <= uints; j++) {
			u = j/(float)uints;
			ogLibDeCasteljau2(u, uorder, v, vorder, 3, cpts, bez);
			glVertex3fv(bez);
		    }
		    glEnd();
		}
	    }
	    break;
	}
	ogLibReadPixels(w, 0, w+w-1, h-1, img2);

	status = bcmp(img1, img2, w*h*4);
	if (status) {
	    /* see what failed */
	    int diffs = 0;
	    for (i=0; i < w*h; i++) {
		status = (img1[i] != img2[i]);
		if (status) {
		    glColor3f(1, 1, 0);
		    glBegin(GL_POINTS);
		    glVertex2f(i%w, i/w);
		    glEnd();
		    diffs++;
		}
	    }
#if _DEBUG_EVAL_
	    if (diffs > tolerance*(uints+1)*(vints+1)) {
		char buf[10];
		ogEnvLog(OG_LFAIL,
			 "evaluator order test: images differ in %d pixels vorder:%d uorder%d\n",
			 diffs, vorder, uorder );
		gets(buf);
	    } else {
		ogEnvLog(OG_LALWAYS,"images don't match; diffs %d, PASSED anyway\n",
		       diffs);
	    }
#else	    
	    if (diffs > tolerance*(uints+1)*(vints+1)) {
		ogEnvLog(OG_LFAIL,
			 "evaluator order test: images differ in %d pixels\n",
			 diffs);
	    }
#endif
	}
    }
    ogLibFree(img1);
    ogLibFree(img2);
}

/*ARGSUSED*/
TESTMOD(eval_order)
{
    int i, j;
    int maxorder;

    maxorder = 8;

#if DO_EVALMESH    
    for (i=2; i <= maxorder; i++) {
	for (j=2; j <= maxorder; j++) {
	    ogEnvLog(1, "---evalmesh 2D map test, order %d x %d---\n", i, j);
	    do_eval_order(2, i, j, 16, 16, GL_POINT, POINT_TOLERANCE, 1, GL_FALSE);
	}
    }
    for (i=2; i <= maxorder; i++) {
	for (j=2; j <= maxorder; j++) {
	    ogEnvLog(1, "---evalmesh 2D map test, order %d x %d---\n", i, j);
	    do_eval_order(2, i, j, 16, 16, GL_LINE, LINE_TOLERANCE, 1, GL_FALSE);
	}
    }
    for (i=2; i <= maxorder; i++) {
	ogEnvLog(1, "---evalmesh 1D map test, order %d---\n", i);
	do_eval_order(1, i, 1, 32, 1, GL_POINT, POINT_TOLERANCE, 1, GL_FALSE);
    }
    for (i=2; i <= maxorder; i++) {
	ogEnvLog(1, "---evalmesh 1D map test, order %d---\n", i);
	do_eval_order(1, i, 1, 32, 1, GL_LINE, LINE_TOLERANCE, 1, GL_FALSE);
    }
#endif    
#if DO_EVALCOORD
    for (i=2; i <= maxorder; i++) {
	for (j=2; j <= maxorder; j++) {
	    ogEnvLog(1, "---evalcoord 2D map test, order %d x %d---\n", i, j);
	    do_eval_order(2, i, j, 16, 16, GL_POINT, POINT_TOLERANCE, 1, GL_TRUE);
	}
    }
    for (i=2; i <= maxorder; i++) {
	for (j=2; j <= maxorder; j++) {
	    ogEnvLog(1, "---evalcoord 2D map test, order %d x %d---\n", i, j);
	    do_eval_order(2, i, j, 16, 16, GL_LINE, LINE_TOLERANCE, 1, GL_TRUE);
	}
    }
    for (i=2; i <= maxorder; i++) {
	ogEnvLog(1, "---evalcoord 1D map test, order %d---\n", i);
	do_eval_order(1, i, 1, 32, 1, GL_POINT, POINT_TOLERANCE, 1, GL_TRUE);
    }
    for (i=2; i <= maxorder; i++) {
	ogEnvLog(1, "---evalcoord 1D map test, order %d---\n", i);
	do_eval_order(1, i, 1, 32, 1, GL_LINE, LINE_TOLERANCE, 1, GL_TRUE);
    }
#endif
}

static void cleanup(void)
{
    ogLibSetDefaultColors();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glViewport(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    glClearColor(0,0,0,0);
    ogLibSetDefaultEvaluators();
}

CLEANUP(eval_order)
{
    cleanup();
}


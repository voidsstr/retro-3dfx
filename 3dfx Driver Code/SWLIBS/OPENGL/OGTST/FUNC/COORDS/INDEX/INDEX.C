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

/* index.c - $Revision: 2$ */

/*
 * This program tests all glIndex commands
 * 
 */

#include <stdio.h>
#include "ogtst.h"	/* include test environment		*/

static void dod(float *);
static void dof(float *);
static void doi(float *);
static void dos(float *);
static void dodv(float *);
static void dofv(float *);
static void doiv(float *);
static void dosv(float *);

static struct {
    char *name;
    int called;
    void (*f)(float *);
} funcs[] = {
    "d",0,dod, "f",0,dof, "i",0,doi,"s",0,dos, 
    "dv",0,dodv, "fv",0,dofv, "iv",0,doiv,"sv",0,dosv, 
};
#define NFUNCS	sizeof(funcs)/sizeof(funcs[0])

TESTMOD(index)
{
    int bits;
    float maxi,index,frac;
    int i,j,obj;
    int xmax,ymax;
    int maxerr,vx,vy;
    GLfloat gi;
    int dindex,imask;
    int funcOrder[NFUNCS];

    ogLibClear(0);
    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.,(double)xmax+1.,0.,(double)ymax+1.,-1.,1.);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    bits = ogEnvCurVisualInfo(GLX_BUFFER_SIZE);
    maxi = (1<<bits) -  1.;
    imask = (1<<bits) - 1;
    maxerr = 0;

    while (pass--) {
	/* randomly order the functions */
	ogLibOrderRand(funcOrder, NFUNCS); 
	for (i=0; i < NFUNCS; i++) {
	    j = funcOrder[i];
	    START_DL_OR_IM(1);
	    index = ogLibFloatRand(0.0,maxi) + ogLibFloatRand(0.1,0.9);
	    /* stay away from rounding near 0.5 */
	    frac = index - ((int) index);
	    if (frac > 0.49 && frac < 0.51) {
	        index += 0.02;
	    }
	    funcs[j].f(&index);
	    frac = index - ((int) index);
	    vx = ogLibIntRand(0,xmax);
	    vy = ogLibIntRand(0,ymax);
	    glBegin(GL_POINTS);
	        ogLibSetVertex(vx,vy,0,1);
            glEnd();		
	    FINIS_DL_OR_IM(1);
	    dindex = (int) index;
            dindex = ((unsigned int) dindex) & imask; 
	    dindex = dindex + frac + 0.5;
	    if (dindex > imask)
	        dindex = imask;
	    
	    if (ogLibPixelCheck(vx,vy,dindex)) {
		  ogEnvLog(3, "---> %s: index: %g dindex: %d\n",
                           funcs[j].name,index,dindex);
	    }
	    glGetFloatv(GL_CURRENT_INDEX,&gi);
	    if (ABS(index-gi) > maxerr) {
		ogEnvLog(OG_LFAIL, "glIndex%s() set to %g instead of %g\n",
                         funcs[j].name,gi,index);
	    }
	}
    }
}

CLEANUP(index)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ogLibSetDefaultColors();
}

static void dod(float *i)
{
    GLdouble mi;

    mi = *i;

    /* determine the expected returned float */
    *i = mi;

    ogEnvLog(OG_LPARAMETERS, "glIndexd(%f);\n", mi);
    glIndexd(mi);
}

static void dof(float *i)
{
    ogEnvLog(OG_LPARAMETERS, "glIndexf(%f);\n", *i);
    glIndexf(*i);
}

static void doi(float *i)
{
    GLint mi;

    mi = *i;

    /* determine the expected returned float */
    *i = mi;

    ogEnvLog(OG_LPARAMETERS, "glIndexi(%d);\n", mi);
    glIndexi(mi);
}

static void dos(float *i)
{
    GLshort mi;

    mi = *i;

    /* determine the expected returned float */
    *i = mi;

    ogEnvLog(OG_LPARAMETERS, "glIndexs(%i);\n", mi);
    glIndexs(mi);
}

static void dodv(float *i)
{
    GLdouble mi;

    mi = *i;

    /* determine the expected returned float */
    *i = mi;

    ogEnvLog(OG_LPARAMETERS, "glIndexdv(%f);\n", mi);
    glIndexdv(&mi);
}

static void dofv(float *i)
{
    ogEnvLog(OG_LPARAMETERS, "glIndexfv(%f);\n", *i);
    glIndexfv(i);
}

static void doiv(float *i)
{
    GLint mi;

    mi = *i;

    /* determine the expected returned float */
    *i = mi;

    ogEnvLog(OG_LPARAMETERS, "glIndexiv(%i);\n", mi);
    glIndexiv(&mi);
}

static void dosv(float *i)
{
    GLshort mi;

    mi = *i;

    /* determine the expected returned float */
    *i = mi;

    ogEnvLog(OG_LPARAMETERS, "glIndexsv(%i);\n", mi);
    glIndexsv(&mi);
}

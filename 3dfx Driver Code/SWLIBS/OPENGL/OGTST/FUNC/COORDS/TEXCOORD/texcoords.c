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

/* texcoords.c - $Revision: 2$ */

/*
 * This program tests all glTexcoord commands
 * 
 */

#include "ogtst.h"	/* include test environment		*/

static void do1d(float *,float *, float *, float *);
static void do1f(float *,float *, float *, float *);
static void do1i(float *,float *, float *, float *);
static void do1s(float *,float *, float *, float *);

static void do2d(float *,float *, float *, float *);
static void do2f(float *,float *, float *, float *);
static void do2i(float *,float *, float *, float *);
static void do2s(float *,float *, float *, float *);

static void do3d(float *,float *, float *, float *);
static void do3f(float *,float *, float *, float *);
static void do3i(float *,float *, float *, float *);
static void do3s(float *,float *, float *, float *);

static void do4d(float *,float *, float *, float *);
static void do4f(float *,float *, float *, float *);
static void do4i(float *,float *, float *, float *);
static void do4s(float *,float *, float *, float *);

static void do1dv(float *,float *, float *, float *);
static void do1fv(float *,float *, float *, float *);
static void do1iv(float *,float *, float *, float *);
static void do1sv(float *,float *, float *, float *);

static void do2dv(float *,float *, float *, float *);
static void do2fv(float *,float *, float *, float *);
static void do2iv(float *,float *, float *, float *);
static void do2sv(float *,float *, float *, float *);

static void do3dv(float *,float *, float *, float *);
static void do3fv(float *,float *, float *, float *);
static void do3iv(float *,float *, float *, float *);
static void do3sv(float *,float *, float *, float *);

static void do4dv(float *,float *, float *, float *);
static void do4fv(float *,float *, float *, float *);
static void do4iv(float *,float *, float *, float *);
static void do4sv(float *,float *, float *, float *);

#define EO	1./32767.

static struct {
    char *name;
    int called;
    float maxerr;
    void (*f)(float *, float *, float *, float *);
} funcs[] = {
    "1d",0,EO,do1d, "1f",0,EO,do1f, "1i",0,EO,do1i, "1s",0,EO,do1s,
    "2d",0,EO,do2d, "2f",0,EO,do2f, "2i",0,EO,do2i, "2s",0,EO,do2s,
    "3d",0,EO,do3d, "3f",0,EO,do3f, "3i",0,EO,do3i, "3s",0,EO,do3s,
    "4d",0,EO,do4d, "4f",0,EO,do4f, "4i",0,EO,do4i, "4s",0,EO,do4s,
    "1dv",0,EO,do1dv, "1fv",0,EO,do1fv, "1iv",0,EO,do1iv, "1sv",0,EO,do1sv,
    "2dv",0,EO,do2dv, "2fv",0,EO,do2fv, "2iv",0,EO,do2iv, "2sv",0,EO,do2sv,
    "3dv",0,EO,do3dv, "3fv",0,EO,do3fv, "3iv",0,EO,do3iv, "3sv",0,EO,do3sv,
    "4dv",0,EO,do4dv, "4fv",0,EO,do4fv, "4iv",0,EO,do4iv, "4sv",0,EO,do4sv,
};
#define NFUNCS	sizeof(funcs)/sizeof(funcs[0])

TESTMOD(texcoords)
{
    float s,t,q,r;
    float gt[4];
    int i,j,obj;
    int xmax,ymax,vx,vy, xymin;
    float maxerr;
    GLboolean multiSampled = ogEnvIsMultiSampled();
    int funcOrder[NFUNCS];

    ogLibClear(0);
    if (multiSampled) {
        xmax = ogEnvQuery(OG_XWSIZE) - 2;
        ymax = ogEnvQuery(OG_YWSIZE) - 2;
        xymin = 1;
    } else {
        xmax = ogEnvQuery(OG_XWSIZE) - 1;
        ymax = ogEnvQuery(OG_YWSIZE) - 1;
        xymin = 0;
    }
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.,(double)xmax+1.,0.,(double)ymax+1.,-1.,1.);

    while (pass--) {
	/* randomly order the functions */
	ogLibOrderRand(funcOrder, NFUNCS); 
	for (i=0; i < NFUNCS; i++) {
	    j = funcOrder[i];
	    vx = ogLibIntRand(xymin,xmax);
	    vy = ogLibIntRand(xymin,ymax);
#if 0                           /* Enable if drawing is checked */
            if (multiSampled) {
                int k, l;
                /*
                 * Clear the 5x5 rectangle around the point.  3x3 is not enough
                 * becase of the slight shift up and to the right of the vertex
                 * w.r.t pixel centers.
                 */
                glColor4ub(0, 0, 0, 0);
                glBegin(GL_POINTS); {
                    for (k = -2; k <= 2; k++)
                        for (l = -2; l <= 2; j++)
                            glVertex2i(vx + k, vy + l);
                } glEnd();
            }
#endif
 	    START_DL_OR_IM(1);
	    s = ogLibFloatRand(-10.,10.);
	    t = ogLibFloatRand(-10.,10.);
	    r = ogLibFloatRand(-10.,10.);
	    q = ogLibFloatRand(-10.,10.);
	    funcs[j].f(&s,&t,&r,&q);
	    ogEnvLog(2, "glTexCoord%s(%g,%g,%g,%g)\n",funcs[j].name,s,t,r,q);

	    glBegin(GL_POINTS);
	        ogLibSetVertex(vx,vy,0,1);
            glEnd();		
	    FINIS_DL_OR_IM(1);
#if 0
            if (multiSampled)
                ogLibRectCheck(vx, vy, vx, vy, wordcolor, 0);
            else
                ogLibPixelCheck(vx,vy,wordcolor);
#endif
	    glGetFloatv(GL_CURRENT_TEXTURE_COORDS,gt);
            maxerr = funcs[j].maxerr;
	    if (ABS(s-gt[0]) > maxerr || ABS(t-gt[1]) > maxerr || 
                ABS(r-gt[2]) > maxerr || ABS(q-gt[3]) > maxerr) {
		ogEnvLog(OG_LFAIL, "glTexCoord%s() set (s,t,r,q) to \
(%g,%g,%g,%g) instead of (%g,%g,%g,%g)\n",
                         funcs[j].name,gt[0],gt[1],gt[2],gt[3],s,t,r,q);
	    }
	}
    }
}

CLEANUP(texcoords)
{
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();
    glTexCoord4i(0, 0, 0, 1);
}

static void do1d(float *s, float *t, float *r, float *q)
{
    GLdouble ms;

    ms = *s;

    /* determine the expected returned floats */
    *s = ms;
    *t = 0.;
    *r = 0.;
    *q = 1.;

    glTexCoord1d(ms);
}

static void do1f(float *s, float *t, float *r, float *q)
{
    *t = 0.; *r = 0.; *q = 1.;
    glTexCoord1f(*s);
}

static void do1i(float *s, float *t, float *r, float *q)
{
    GLint ms;

    ms = *s;

    /* determine the expected returned floats */
    *s = ms;
    *t = 0.;
    *r = 0.;
    *q = 1.;

    glTexCoord1i(ms);
}

static void do1s(float *s, float *t, float *r, float *q)
{
    GLshort ms;

    ms = *s;

    /* determine the expected returned floats */
    *s = ms;
    *t = 0.;
    *r = 0.;
    *q = 1.;

    glTexCoord1s(ms);
}

static void do2d(float *s, float *t, float *r, float *q)
{
    GLdouble ms,mt;

    ms = *s;
    mt = *t;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = 0.;
    *q = 1.;

    glTexCoord2d(ms,mt);
}

static void do2f(float *s, float *t, float *r, float *q)
{
    *r = 0.; *q = 1.;
    glTexCoord2f(*s,*t);
}

static void do2i(float *s, float *t, float *r, float *q)
{
    GLint ms,mt;

    ms = *s;
    mt = *t;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = 0.;
    *q = 1.;

    glTexCoord2i(ms,mt);
}

static void do2s(float *s, float *t, float *r, float *q)
{
    GLshort ms,mt;

    ms = *s;
    mt = *t;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = 0.;
    *q = 1.;

    glTexCoord2s(ms,mt);
}

static void do3d(float *s, float *t, float *r, float *q)
{
    GLdouble ms,mt,mr;

    ms = *s;
    mt = *t;
    mr = *r;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = 1.;

    glTexCoord3d(ms,mt,mr);
}

static void do3f(float *s, float *t, float *r, float *q)
{
    *q = 1.;
    glTexCoord3f(*s,*t,*r);
}

static void do3i(float *s, float *t, float *r, float *q)
{
    GLint ms,mt,mr;

    ms = *s;
    mt = *t;
    mr = *r;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = 1.;

    glTexCoord3i(ms,mt,mr);
}

static void do3s(float *s, float *t, float *r, float *q)
{
    GLshort ms,mt,mr;

    ms = *s;
    mt = *t;
    mr = *r;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = 1.;

    glTexCoord3s(ms,mt,mr);
}

static void do4d(float *s, float *t, float *r, float *q)
{
    GLdouble ms,mt,mr,mq;

    ms = *s;
    mt = *t;
    mr = *r;
    mq = *q;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = mq;

    glTexCoord4d(ms,mt,mr,mq);
}

static void do4f(float *s, float *t, float *r, float *q)
{
    glTexCoord4f(*s,*t,*r,*q);
}

static void do4i(float *s, float *t, float *r, float *q)
{
    GLint ms,mt,mr,mq;

    ms = *s;
    mt = *t;
    mr = *r;
    mq = *q;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = mq;

    glTexCoord4i(ms,mt,mr,mq);
}

static void do4s(float *s, float *t, float *r, float *q)
{
    GLshort ms,mt,mr,mq;

    ms = *s;
    mt = *t;
    mr = *r;
    mq = *q;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = mq;

    glTexCoord4s(ms,mt,mr,mq);
}

static void do1dv(float *s, float *t, float *r, float *q)
{
    GLdouble ms,x[1];

    ms = *s;

    /* determine the expected returned floats */
    *s = ms;
    *t = 0.;
    *r = 0.;
    *q = 1.;

    x[0] = ms;
    glTexCoord1dv(x);
}

static void do1fv(float *s, float *t, float *r, float *q)
{
    GLfloat x[1];

    *t = 0.; *r = 0.; *q = 1.;
    x[0] = *s;
    glTexCoord1fv(x);
}

static void do1iv(float *s, float *t, float *r, float *q)
{
    GLint ms,x[1];

    ms = *s;

    /* determine the expected returned floats */
    *s = ms;
    *t = 0.;
    *r = 0.;
    *q = 1.;

    x[0] = ms;
    glTexCoord1iv(x);
}

static void do1sv(float *s, float *t, float *r, float *q)
{
    GLshort ms,x[1];

    ms = *s;

    /* determine the expected returned floats */
    *s = ms;
    *t = 0.;
    *r = 0.;
    *q = 1.;

    x[0] = ms;
    glTexCoord1sv(x);
}

static void do2dv(float *s, float *t, float *r, float *q)
{
    GLdouble ms,mt,x[2];

    ms = *s;
    mt = *t;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = 0.;
    *q = 1.;

    x[0] = ms; x[1] = mt;
    glTexCoord2dv(x);
}

static void do2fv(float *s, float *t, float *r, float *q)
{
    GLfloat x[2];

    *r = 0.; *q = 1.;
    x[0] = *s; x[1] = *t;
    glTexCoord2fv(x);
}

static void do2iv(float *s, float *t, float *r, float *q)
{
    GLint ms,mt,x[2];

    ms = *s;
    mt = *t;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = 0.;
    *q = 1.;

    x[0] = ms; x[1] = mt;
    glTexCoord2iv(x);
}

static void do2sv(float *s, float *t, float *r, float *q)
{
    GLshort ms,mt,x[2];

    ms = *s;
    mt = *t;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = 0.;
    *q = 1.;

    x[0] = ms; x[1] = mt;
    glTexCoord2sv(x);
}

static void do3dv(float *s, float *t, float *r, float *q)
{
    GLdouble ms,mt,mr,x[3];

    ms = *s;
    mt = *t;
    mr = *r;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = 1.;

    x[0] = ms; x[1] = mt; x[2] = mr;
    glTexCoord3dv(x);
}

static void do3fv(float *s, float *t, float *r, float *q)
{
    GLfloat x[3];

    *q = 1.;
    x[0] = *s; x[1] = *t; x[2] = *r;
    glTexCoord3fv(x);
}

static void do3iv(float *s, float *t, float *r, float *q)
{
    GLint ms,mt,mr,x[3];

    ms = *s;
    mt = *t;
    mr = *r;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = 1.;

    x[0] = ms; x[1] = mt; x[2] = mr;
    glTexCoord3iv(x);
}

static void do3sv(float *s, float *t, float *r, float *q)
{
    GLshort ms,mt,mr,x[3];

    ms = *s;
    mt = *t;
    mr = *r;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = 1.;

    x[0] = ms; x[1] = mt; x[2] = mr;
    glTexCoord3sv(x);
}

static void do4dv(float *s, float *t, float *r, float *q)
{
    GLdouble ms,mt,mr,mq,x[4];

    ms = *s;
    mt = *t;
    mr = *r;
    mq = *q;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = mq;

    x[0] = ms; x[1] = mt; x[2] = mr; x[3] = mq;
    glTexCoord4dv(x);
}

static void do4fv(float *s, float *t, float *r, float *q)
{
    GLfloat x[4];

    x[0] = *s; x[1] = *t; x[2] = *r; x[3] = *q;
    glTexCoord4fv(x);
}

static void do4iv(float *s, float *t, float *r, float *q)
{
    GLint ms,mt,mr,mq,x[4];

    ms = *s;
    mt = *t;
    mr = *r;
    mq = *q;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = mq;

    x[0] = ms; x[1] = mt; x[2] = mr; x[3] = mq;
    glTexCoord4iv(x);
}

static void do4sv(float *s, float *t, float *r, float *q)
{
    GLshort ms,mt,mr,mq,x[4];

    ms = *s;
    mt = *t;
    mr = *r;
    mq = *q;

    /* determine the expected returned floats */
    *s = ms;
    *t = mt;
    *r = mr;
    *q = mq;

    x[0] = ms; x[1] = mt; x[2] = mr; x[3] = mq;
    glTexCoord4sv(x);
}

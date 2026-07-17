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

/* colors.c - $Revision: 2$ */

/*
 * This program tests all glColor commands
 * 
 */

#include <stdio.h>
#include "ogtst.h"	/* include test environment		*/

static void do3b(float *,float *, float *, float *);
static void do3d(float *,float *, float *, float *);
static void do3f(float *,float *, float *, float *);
static void do3i(float *,float *, float *, float *);
static void do3s(float *,float *, float *, float *);
static void do3ub(float *,float *, float *, float *);
static void do3ui(float *,float *, float *, float *);
static void do3us(float *,float *, float *, float *);
static void do4b(float *,float *, float *, float *);
static void do4d(float *,float *, float *, float *);
static void do4f(float *,float *, float *, float *);
static void do4i(float *,float *, float *, float *);
static void do4s(float *,float *, float *, float *);
static void do4ub(float *,float *, float *, float *);
static void do4ui(float *,float *, float *, float *);
static void do4us(float *,float *, float *, float *);
static void do3bv(float *,float *, float *, float *);
static void do3dv(float *,float *, float *, float *);
static void do3fv(float *,float *, float *, float *);
static void do3iv(float *,float *, float *, float *);
static void do3sv(float *,float *, float *, float *);
static void do3ubv(float *,float *, float *, float *);
static void do3uiv(float *,float *, float *, float *);
static void do3usv(float *,float *, float *, float *);
static void do4bv(float *,float *, float *, float *);
static void do4dv(float *,float *, float *, float *);
static void do4fv(float *,float *, float *, float *);
static void do4iv(float *,float *, float *, float *);
static void do4sv(float *,float *, float *, float *);
static void do4ubv(float *,float *, float *, float *);
static void do4uiv(float *,float *, float *, float *);
static void do4usv(float *,float *, float *, float *);

#define EB	1./127.
#define EO	1./32767.
#define EEX	1./255.

static struct {
    char *name;
    int called;
    float maxerr;
    void (*f)(float *, float *, float *, float *);
} funcs[] = {
    "3b",0,EB,do3b, "3d",0,EO,do3d, "3f",0,EO,do3f, "3i",0,EO,do3i, 
    "3s",0,EO,do3s, "3ub",0,EO,do3ub, "3ui",0,EO,do3ui, "3us",0,EO,do3us,
    "4b",0,EB,do4b, "4d",0,EO,do4d, "4f",0,EO,do4f, "4i",0,EO,do4i, 
    "4s",0,EO,do4s, "4ub",0,EO,do4ub, "4ui",0,EO,do4ui, "4us",0,EO,do4us,
    "3bv",0,EB,do3bv, "3dv",0,EO,do3dv, "3fv",0,EO,do3fv, "3iv",0,EO,do3iv, 
    "3sv",0,EO,do3sv, "3ubv",0,EO,do3ubv, "3uiv",0,EO,do3uiv,"3usv",0,EO,do3usv,
    "4bv",0,EB,do4bv, "4dv",0,EO,do4dv, "4fv",0,EO,do4fv, "4iv",0,EO,do4iv, 
    "4sv",0,EO,do4sv, "4ubv",0,EO,do4ubv, "4uiv",0,EO,do4uiv,"4usv",0,EO,do4usv,
};
#define NFUNCS	sizeof(funcs)/sizeof(funcs[0])

TESTMOD(colors)
{
    float r,g,b,a;
    float gc[4];
    int i, j, obj;
    GLubyte ri,gi,bi,ai;
    unsigned int wordcolor;
    int xmax,ymax,vx,vy;
    float maxerr, scale, minColor, roundVal;
    int shift,bits;
    GLboolean multiSampled = ogEnvIsMultiSampled();
    int funcOrder[NFUNCS];

    ogLibClear(0);
    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0,(double) xmax + 1, 0, (double)ymax + 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);

    /* Special platform dependent other stuff */
    bits = ogEnvCurVisualInfo(GLX_RED_SIZE);
    /* set default values of scale & shift instead of replicating code... */
    if (bits > 8) {
	scale = (1<<bits) - 1.;
	shift = bits - 8;
    } else {
	scale = 255.0;
	shift = 0;
    }
    roundVal = 0.5;
    minColor = -1;

    while (pass--) {
	/* randomly order the functions */
	ogLibOrderRand(funcOrder, NFUNCS); 
	for (i=0; i < NFUNCS; i++) {
	    j = funcOrder[i];
	    vx = ogLibIntRand(2,xmax-2);
	    vy = ogLibIntRand(2,ymax-2);
            if (multiSampled) {
                /*
                 * Clear the 5x5 rectangle around the point.  3x3 is not enough
                 * becase of the slight shift up and to the right of the vertex
                 * w.r.t pixel centers.
                 */
                glColor4ub(0, 0, 0, 0);
		glRecti(vx-2, vy-2, vx+2, vy+2);
            }
 	    START_DL_OR_IM(1);
            r = ogLibFloatRand(minColor, 1);
            g = ogLibFloatRand(minColor, 1);
            b = ogLibFloatRand(minColor, 1);
            a = ogLibFloatRand(minColor, 1);
	    funcs[j].f(&r,&g,&b,&a);
	    ogEnvLog(2, "glColor%s(%g,%g,%g,%g)\n",funcs[j].name,r,g,b,a);

            ri = r < 0 ? 0 : (((int)(scale * r + roundVal)) >> shift);
            gi = g < 0 ? 0 : (((int)(scale * g + roundVal)) >> shift);
            bi = b < 0 ? 0 : (((int)(scale * b + roundVal)) >> shift);
            ai = a < 0 ? 0 : (((int)(scale * a + roundVal)) >> shift);

	    wordcolor = (ri<<24)|(gi<<16)|(bi<<8)|ai;

	    ogLibDrawFragments(vx,vy);

	    FINIS_DL_OR_IM(1);

            if (multiSampled)
                ogLibPointCheck(vx, vy, wordcolor, 0, 0);
            else
                ogLibPixelCheck(vx,vy,wordcolor);

	    glGetFloatv(GL_CURRENT_COLOR,gc);
            maxerr = funcs[j].maxerr;
	    if (ABS(r-gc[0]) > maxerr || ABS(g-gc[1]) > maxerr || 
                ABS(b-gc[2]) > maxerr || ABS(a-gc[3]) > maxerr) {
		ogEnvLog(OG_LFAIL, "glColor%s() set RGBA to (%g,%g,%g,%g) instead of (%g,%g,%g,%g)\n",
		    funcs[j].name,gc[0],gc[1],gc[2],gc[3],r,g,b,a);
	    }
	}
    }
}

CLEANUP(colors)
{
    ogLibSetDefaultColors();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


static void do3b(float *r, float *g, float *b, float *a)
{
    GLbyte mr,mg,mb;

    /* convert to this functions range */
    mr = OGTST_F_TO_B(*r);
    mg = OGTST_F_TO_B(*g);
    mb = OGTST_F_TO_B(*b);

    /* determine the expected returned floats */
    *r = OGTST_B_TO_F(mr);
    *g = OGTST_B_TO_F(mg);
    *b = OGTST_B_TO_F(mb);
    *a = 1.;

    glColor3b(mr,mg,mb);
}

static void do3d(float *r, float *g, float *b, float *a)
{
    GLdouble mr,mg,mb;

    mr = *r;
    mg = *g;
    mb = *b;

    /* determine the expected returned floats */
    *r = mr;
    *g = mg;
    *b = mb;
    *a = 1.;

    glColor3d(mr,mg,mb);
}

static void do3f(float *r, float *g, float *b, float *a)
{
    *a = 1.;
    glColor3f(*r,*g,*b);
}

static void do3i(float *r, float *g, float *b, float *a)
{
    GLint mr,mg,mb;

    /* convert to this functions range */
    mr = OGTST_F_TO_I(*r);
    mg = OGTST_F_TO_I(*g);
    mb = OGTST_F_TO_I(*b);

    /* determine the expected returned floats */
    *r = OGTST_I_TO_F(mr);
    *g = OGTST_I_TO_F(mg);
    *b = OGTST_I_TO_F(mb);
    *a = 1.;

    glColor3i(mr,mg,mb);
}

static void do3s(float *r, float *g, float *b, float *a)
{
    GLshort mr,mg,mb;

    /* convert to this functions range */
    mr = OGTST_F_TO_S(*r);
    mg = OGTST_F_TO_S(*g);
    mb = OGTST_F_TO_S(*b);

    /* determine the expected returned floats */
    *r = OGTST_S_TO_F(mr);
    *g = OGTST_S_TO_F(mg);
    *b = OGTST_S_TO_F(mb);
    *a = 1.;

    glColor3s(mr,mg,mb);
}

static void do3ub(float *r, float *g, float *b, float *a)
{
    GLubyte mr,mg,mb;

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_UB(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_UB(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_UB(*b);

    /* determine the expected returned floats */
    *r = OGTST_UB_TO_F(mr);
    *g = OGTST_UB_TO_F(mg);
    *b = OGTST_UB_TO_F(mb);
    *a = 1.;
    glColor3ub(mr,mg,mb);
}

static void do3ui(float *r, float *g, float *b, float *a)
{
    GLuint mr,mg,mb;

    mr = *r < 0 ? 0 : OGTST_F_TO_UI(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_UI(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_UI(*b);

    *r = OGTST_UI_TO_F(mr);
    *g = OGTST_UI_TO_F(mg);
    *b = OGTST_UI_TO_F(mb);

    *a = 1.;
    glColor3ui(mr,mg,mb);
}

static void do3us(float *r, float *g, float *b, float *a)
{
    GLushort mr,mg,mb;

    mr = *r < 0 ? 0 : OGTST_F_TO_US(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_US(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_US(*b);
    *r = OGTST_US_TO_F(mr);
    *g = OGTST_US_TO_F(mg);
    *b = OGTST_US_TO_F(mb);
    *a = 1.;
    glColor3us(mr,mg,mb);
}

static void do4b(float *r, float *g, float *b, float *a)
{
    GLbyte mr,mg,mb,ma;

    /* convert to this functions range */
    mr = OGTST_F_TO_B(*r);
    mg = OGTST_F_TO_B(*g);
    mb = OGTST_F_TO_B(*b);
    ma = OGTST_F_TO_B(*a);

    /* determine the expected returned floats */
    *r = OGTST_B_TO_F(mr);
    *g = OGTST_B_TO_F(mg);
    *b = OGTST_B_TO_F(mb);
    *a = OGTST_B_TO_F(ma);

    glColor4b(mr,mg,mb,ma);
}

static void do4d(float *r, float *g, float *b, float *a)
{
    GLdouble mr,mg,mb,ma;

    mr = *r;
    mg = *g;
    mb = *b;
    ma = *a;

    /* determine the expected returned floats */
    *r = mr;
    *g = mg;
    *b = mb;
    *a = ma;

    glColor4d(mr,mg,mb,ma);
}

static void do4f(float *r, float *g, float *b, float *a)
{
    glColor4f(*r,*g,*b,*a);
}

static void do4i(float *r, float *g, float *b, float *a)
{
    GLint mr,mg,mb,ma;

    /* convert to this functions range */
    mr = OGTST_F_TO_I(*r);
    mg = OGTST_F_TO_I(*g);
    mb = OGTST_F_TO_I(*b);
    ma = OGTST_F_TO_I(*a);

    /* determine the expected returned floats */
    *r = OGTST_I_TO_F(mr);
    *g = OGTST_I_TO_F(mg);
    *b = OGTST_I_TO_F(mb);
    *a = OGTST_I_TO_F(ma);

    glColor4i(mr,mg,mb,ma);
}

static void do4s(float *r, float *g, float *b, float *a)
{
    GLshort mr,mg,mb,ma;

    /* convert to this functions range */
    mr = OGTST_F_TO_S(*r);
    mg = OGTST_F_TO_S(*g);
    mb = OGTST_F_TO_S(*b);
    ma = OGTST_F_TO_S(*a);

    /* determine the expected returned floats */
    *r = OGTST_S_TO_F(mr);
    *g = OGTST_S_TO_F(mg);
    *b = OGTST_S_TO_F(mb);
    *a = OGTST_S_TO_F(ma);

    glColor4s(mr,mg,mb,ma);
}

static void do4ub(float *r, float *g, float *b, float *a)
{
    GLubyte mr,mg,mb,ma;

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_UB(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_UB(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_UB(*b);
    ma = *a < 0 ? 0 : OGTST_F_TO_UB(*a);

    /* determine the expected returned floats */
    *r = OGTST_UB_TO_F(mr);
    *g = OGTST_UB_TO_F(mg);
    *b = OGTST_UB_TO_F(mb);
    *a = OGTST_UB_TO_F(ma);

    glColor4ub(mr,mg,mb,ma);
}

static void do4ui(float *r, float *g, float *b, float *a)
{
    GLuint mr,mg,mb,ma;

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_UI(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_UI(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_UI(*b);
    ma = *a < 0 ? 0 : OGTST_F_TO_UI(*a);

    /* determine the expected returned floats */
    *r = OGTST_UI_TO_F(mr);
    *g = OGTST_UI_TO_F(mg);
    *b = OGTST_UI_TO_F(mb);
    *a = OGTST_UI_TO_F(ma);

    glColor4ui(mr,mg,mb,ma);
}

static void do4us(float *r, float *g, float *b, float *a)
{
    GLushort mr,mg,mb,ma;

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_US(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_US(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_US(*b);
    ma = *a < 0 ? 0 : OGTST_F_TO_US(*a);

    /* determine the expected returned floats */
    *r = OGTST_US_TO_F(mr);
    *g = OGTST_US_TO_F(mg);
    *b = OGTST_US_TO_F(mb);
    *a = OGTST_US_TO_F(ma);

    glColor4us(mr,mg,mb,ma);
}

static void do3bv(float *r, float *g, float *b, float *a)
{
    GLbyte mr,mg,mb,c[3];

    /* convert to this functions range */
    mr = OGTST_F_TO_B(*r);
    mg = OGTST_F_TO_B(*g);
    mb = OGTST_F_TO_B(*b);

    /* determine the expected returned floats */
    *r = OGTST_B_TO_F(mr);
    *g = OGTST_B_TO_F(mg);
    *b = OGTST_B_TO_F(mb);
    *a = 1.;

    c[0] = mr; c[1] = mg; c[2] = mb;
    glColor3bv(c);
}

static void do3dv(float *r, float *g, float *b, float *a)
{
    GLdouble mr,mg,mb,c[3];

    mr = *r;
    mg = *g;
    mb = *b;

    /* determine the expected returned floats */
    *r = mr;
    *g = mg;
    *b = mb;
    *a = 1.;

    c[0] = mr; c[1] = mg; c[2] = mb;
    glColor3dv(c);
}

static void do3fv(float *r, float *g, float *b, float *a)
{
    GLfloat c[3];

    *a = 1.;
    c[0] = *r; c[1] = *g; c[2] = *b;
    glColor3fv(c);
}

static void do3iv(float *r, float *g, float *b, float *a)
{
    GLint mr,mg,mb,c[3];

    /* convert to this functions range */
    mr = OGTST_F_TO_I(*r);
    mg = OGTST_F_TO_I(*g);
    mb = OGTST_F_TO_I(*b);

    /* determine the expected returned floats */
    *r = OGTST_I_TO_F(mr);
    *g = OGTST_I_TO_F(mg);
    *b = OGTST_I_TO_F(mb);
    *a = 1;

    c[0] = mr; c[1] = mg; c[2] = mb;
    glColor3iv(c);
}

static void do3sv(float *r, float *g, float *b, float *a)
{
    GLshort mr,mg,mb,c[3];

    /* convert to this functions range */
    mr = OGTST_F_TO_S(*r);
    mg = OGTST_F_TO_S(*g);
    mb = OGTST_F_TO_S(*b);

    /* determine the expected returned floats */
    *r = OGTST_S_TO_F(mr);
    *g = OGTST_S_TO_F(mg);
    *b = OGTST_S_TO_F(mb);
    *a = 1.;

    c[0] = mr; c[1] = mg; c[2] = mb;
    glColor3sv(c);
}

static void do3ubv(float *r, float *g, float *b, float *a)
{
    GLubyte mr,mg,mb,c[3];

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_UB(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_UB(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_UB(*b);

    /* determine the expected returned floats */
    *r = OGTST_UB_TO_F(mr);
    *g = OGTST_UB_TO_F(mg);
    *b = OGTST_UB_TO_F(mb);
    *a = 1;

    c[0] = mr; c[1] = mg; c[2] = mb;
    glColor3ubv(c);
}

static void do3uiv(float *r, float *g, float *b, float *a)
{
    GLuint mr,mg,mb,c[3];

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_UI(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_UI(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_UI(*b);

    /* determine the expected returned floats */
    *r = OGTST_UI_TO_F(mr);
    *g = OGTST_UI_TO_F(mg);
    *b = OGTST_UI_TO_F(mb);
    *a = 1.;

    c[0] = mr; c[1] = mg; c[2] = mb;
    glColor3uiv(c);
}

static void do3usv(float *r, float *g, float *b, float *a)
{
    GLushort mr,mg,mb,c[3];

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_US(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_US(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_US(*b);

    /* determine the expected returned floats */
    *r = OGTST_US_TO_F(mr);
    *g = OGTST_US_TO_F(mg);
    *b = OGTST_US_TO_F(mb);
    *a = 1.;

    c[0] = mr; c[1] = mg; c[2] = mb;
    glColor3usv(c);
}

static void do4bv(float *r, float *g, float *b, float *a)
{
    GLbyte mr,mg,mb,ma,c[4];

    /* convert to this functions range */
    mr = OGTST_F_TO_B(*r);
    mg = OGTST_F_TO_B(*g);
    mb = OGTST_F_TO_B(*b);
    ma = OGTST_F_TO_B(*a);

    /* determine the expected returned floats */
    *r = OGTST_B_TO_F(mr);
    *g = OGTST_B_TO_F(mg);
    *b = OGTST_B_TO_F(mb);
    *a = OGTST_B_TO_F(ma);

    c[0] = mr; c[1] = mg; c[2] = mb; c[3] = ma;
    glColor4bv(c);
}

static void do4dv(float *r, float *g, float *b, float *a)
{
    GLdouble mr,mg,mb,ma,c[4];

    mr = *r;
    mg = *g;
    mb = *b;
    ma = *a;

    /* determine the expected returned floats */
    *r = mr;
    *g = mg;
    *b = mb;
    *a = ma;

    c[0] = mr; c[1] = mg; c[2] = mb; c[3] = ma;
    glColor4dv(c);
}

static void do4fv(float *r, float *g, float *b, float *a)
{
    GLfloat c[4];

    c[0] = *r; c[1] = *g; c[2] = *b; c[3] = *a;
    glColor4fv(c);
}

static void do4iv(float *r, float *g, float *b, float *a)
{
    GLint mr,mg,mb,ma,c[4];

    /* convert to this functions range */
    mr = OGTST_F_TO_I(*r);
    mg = OGTST_F_TO_I(*g);
    mb = OGTST_F_TO_I(*b);
    ma = OGTST_F_TO_I(*a);

    /* determine the expected returned floats */
    *r = OGTST_I_TO_F(mr);
    *g = OGTST_I_TO_F(mg);
    *b = OGTST_I_TO_F(mb);
    *a = OGTST_I_TO_F(ma);

    c[0] = mr; c[1] = mg; c[2] = mb; c[3] = ma;
    glColor4iv(c);
}

static void do4sv(float *r, float *g, float *b, float *a)
{
    GLshort mr,mg,mb,ma,c[4];

    /* convert to this functions range */
    mr = OGTST_F_TO_S(*r);
    mg = OGTST_F_TO_S(*g);
    mb = OGTST_F_TO_S(*b);
    ma = OGTST_F_TO_S(*a);
    /* determine the expected returned floats */
    *r = OGTST_S_TO_F(mr);
    *g = OGTST_S_TO_F(mg);
    *b = OGTST_S_TO_F(mb);
    *a = OGTST_S_TO_F(ma);

    c[0] = mr; c[1] = mg; c[2] = mb; c[3] = ma;
    glColor4sv(c);
}

static void do4ubv(float *r, float *g, float *b, float *a)
{
    GLubyte mr,mg,mb,ma,c[4];

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_UB(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_UB(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_UB(*b);
    ma = *a < 0 ? 0 : OGTST_F_TO_UB(*a);

    /* determine the expected returned floats */
    *r = OGTST_UB_TO_F(mr);
    *g = OGTST_UB_TO_F(mg);
    *b = OGTST_UB_TO_F(mb);
    *a = OGTST_UB_TO_F(ma);

    c[0] = mr; c[1] = mg; c[2] = mb; c[3] = ma;
    glColor4ubv(c);
}

static void do4uiv(float *r, float *g, float *b, float *a)
{
    GLuint mr,mg,mb,ma,c[4];

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_UI(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_UI(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_UI(*b);
    ma = *a < 0 ? 0 : OGTST_F_TO_UI(*a);

    /* determine the expected returned floats */
    *r = OGTST_UI_TO_F(mr);
    *g = OGTST_UI_TO_F(mg);
    *b = OGTST_UI_TO_F(mb);
    *a = OGTST_UI_TO_F(ma);

    c[0] = mr; c[1] = mg; c[2] = mb; c[3] = ma;
    glColor4uiv(c);
}

static void do4usv(float *r, float *g, float *b, float *a)
{
    GLushort mr,mg,mb,ma,c[4];

    /* convert to this functions range */
    mr = *r < 0 ? 0 : OGTST_F_TO_US(*r);
    mg = *g < 0 ? 0 : OGTST_F_TO_US(*g);
    mb = *b < 0 ? 0 : OGTST_F_TO_US(*b);
    ma = *a < 0 ? 0 : OGTST_F_TO_US(*a);

    /* determine the expected returned floats */
    *r = OGTST_US_TO_F(mr);
    *g = OGTST_US_TO_F(mg);
    *b = OGTST_US_TO_F(mb);
    *a = OGTST_US_TO_F(ma);

    c[0] = mr; c[1] = mg; c[2] = mb; c[3] = ma;
    glColor4usv(c);
}

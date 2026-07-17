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

/* normals.c - $Revision: 2$ */

/*
 * This program tests all glNormal commands
 * 
 */

#include "ogtst.h"	/* include test environment		*/

static void do3b(float *,float *, float *);
static void do3d(float *,float *, float *);
static void do3f(float *,float *, float *);
static void do3i(float *,float *, float *);
static void do3s(float *,float *, float *);
static void do3bv(float *,float *, float *);
static void do3dv(float *,float *, float *);
static void do3fv(float *,float *, float *);
static void do3iv(float *,float *, float *);
static void do3sv(float *,float *, float *);

#define EB	1./127.
#define EO	1./32767.

static struct {
    char *name;
    int called;
    float maxerr;
    void (*f)(float *, float *, float *);
} funcs[] = {
    "3b",0,EB,do3b, "3d",0,EO,do3d, "3f",0,EO,do3f, "3i",0,EO,do3i, 
    "3s",0,EO,do3s, 
    "3bv",0,EB,do3bv, "3dv",0,EO,do3dv, "3fv",0,EO,do3fv, "3iv",0,EO,do3iv, 
    "3sv",0,EO,do3sv, 
};
#define NFUNCS	sizeof(funcs)/sizeof(funcs[0])

TESTMOD(normals)
{
    float nx,ny,nz;
    float gn[3];
    int i,j,obj;
    int xmax,ymax;
    float maxerr;
    int funcOrder[NFUNCS];

    ogLibClear(0);
    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;
    glMatrixMode(GL_PROJECTION);
    glOrtho(0.,(double)xmax+1.,0.,(double)ymax+1.,-1.,1.);

    while (pass--) {
	/* randomly order the functions */
	ogLibOrderRand(funcOrder, NFUNCS); 
	for (i=0; i < NFUNCS; i++) {
	    j = funcOrder[i];
            START_DL_OR_IM(1);
	    nx = ogLibFloatRand(-1.,1.);
	    ny = ogLibFloatRand(-1.,1.);
	    nz = ogLibFloatRand(-1.,1.);
	    ogEnvLog(1,"glNormal%s(%f,%f,%f)\n",funcs[j].name,nx,ny,nz);
	    funcs[j].f(&nx,&ny,&nz);
	    
	    FINIS_DL_OR_IM(1);
	    
	    glGetFloatv(GL_CURRENT_NORMAL,gn);
            maxerr = funcs[j].maxerr;
	    if (ABS(nx-gn[0]) > maxerr || ABS(ny-gn[1]) > maxerr || 
		ABS(nz-gn[2]) > maxerr) {
		ogEnvLog(OG_LFAIL, "glNormal%s() set to (%g,%g,%g) instead of (%g,%g,%g)\n",
			 funcs[j].name,gn[0],gn[1],gn[2],nx,ny,nz);
	    }
	}
    }
}

CLEANUP(normals)
{
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glNormal3f(0, 0, 1);
}


static void do3b(float *x, float *y, float *z)
{
    float fx,fy,fz,max;
    GLbyte mx,my,mz;

    max = 0xff;
    /* convert to this functions range */
    fx = ((*x * max)-1)/2.;
    fy = ((*y * max)-1)/2.;
    fz = ((*z * max)-1)/2.;
    mx = fx < 0 ? fx - 0.5 : fx + 0.5;
    my = fy < 0 ? fy - 0.5 : fy + 0.5;
    mz = fz < 0 ? fz - 0.5 : fz + 0.5;

    /* determine the expected returned floats */
    *x = (2. * mx + 1)/ max;
    *y = (2. * my + 1)/ max;
    *z = (2. * mz + 1)/ max;

    glNormal3b(mx,my,mz);
}

static void do3d(float *x, float *y, float *z)
{
    GLdouble mx,my,mz;

    mx = *x;
    my = *y;
    mz = *z;

    /* determine the expected returned floats */
    *x = mx;
    *y = my;
    *z = mz;

    glNormal3d(mx,my,mz);
}

static void do3f(float *x, float *y, float *z)
{
    glNormal3f(*x,*y,*z);
}

static void do3i(float *x, float *y, float *z)
{
    float fx,fy,fz,max;
    GLint mx,my,mz;

    max = 0xffffffff;
    /* convert to this functions range */
    fx = ((*x * max)-1)/2.;
    fy = ((*y * max)-1)/2.;
    fz = ((*z * max)-1)/2.;
    mx = fx < 0 ? fx - 0.5 : fx + 0.5;
    my = fy < 0 ? fy - 0.5 : fy + 0.5;
    mz = fz < 0 ? fz - 0.5 : fz + 0.5;

    /* determine the expected returned floats */
    *x = (2. * mx + 1)/ max;
    *y = (2. * my + 1)/ max;
    *z = (2. * mz + 1)/ max;

    glNormal3i(mx,my,mz);
}

static void do3s(float *x, float *y, float *z)
{
    float fx,fy,fz,max;
    GLshort mx,my,mz;

    max = 0xffff;
    /* convert to this functions range */
    fx = ((*x * max)-1)/2.;
    fy = ((*y * max)-1)/2.;
    fz = ((*z * max)-1)/2.;
    mx = fx < 0 ? fx - 0.5 : fx + 0.5;
    my = fy < 0 ? fy - 0.5 : fy + 0.5;
    mz = fz < 0 ? fz - 0.5 : fz + 0.5;

    /* determine the expected returned floats */
    *x = (2. * mx + 1)/ max;
    *y = (2. * my + 1)/ max;
    *z = (2. * mz + 1)/ max;

    glNormal3s(mx,my,mz);
}

static void do3bv(float *x, float *y, float *z)
{
    float fx,fy,fz,max;
    GLbyte mx,my,mz,n[3];

    max = 0xff;
    /* convert to this functions range */
    fx = ((*x * max)-1)/2.;
    fy = ((*y * max)-1)/2.;
    fz = ((*z * max)-1)/2.;
    mx = fx < 0 ? fx - 0.5 : fx + 0.5;
    my = fy < 0 ? fy - 0.5 : fy + 0.5;
    mz = fz < 0 ? fz - 0.5 : fz + 0.5;

    /* determine the expected returned floats */
    *x = (2. * mx + 1)/ max;
    *y = (2. * my + 1)/ max;
    *z = (2. * mz + 1)/ max;

    n[0] = mx; n[1] = my; n[2] = mz;
    glNormal3bv(n);
}

static void do3dv(float *x, float *y, float *z)
{
    GLdouble mx,my,mz,n[3];

    mx = *x;
    my = *y;
    mz = *z;

    /* determine the expected returned floats */
    *x = mx;
    *y = my;
    *z = mz;

    n[0] = mx; n[1] = my; n[2] = mz;
    glNormal3dv(n);
}

static void do3fv(float *x, float *y, float *z)
{
    GLfloat n[3];

    n[0] = *x; n[1] = *y; n[2] = *z;
    glNormal3fv(n);
}

static void do3iv(float *x, float *y, float *z)
{
    float fx,fy,fz,max;
    GLint mx,my,mz,n[3];

    max = 0xffffffff;
    /* convert to this functions range */
    fx = ((*x * max)-1)/2.;
    fy = ((*y * max)-1)/2.;
    fz = ((*z * max)-1)/2.;
    mx = fx < 0 ? fx - 0.5 : fx + 0.5;
    my = fy < 0 ? fy - 0.5 : fy + 0.5;
    mz = fz < 0 ? fz - 0.5 : fz + 0.5;

    /* determine the expected returned floats */
    *x = (2. * mx + 1)/ max;
    *y = (2. * my + 1)/ max;
    *z = (2. * mz + 1)/ max;

    n[0] = mx; n[1] = my; n[2] = mz;
    glNormal3iv(n);
}

static void do3sv(float *x, float *y, float *z)
{
    float fx,fy,fz,max;
    GLshort mx,my,mz,n[3];

    max = 0xffff;
    /* convert to this functions range */
    fx = ((*x * max)-1)/2.;
    fy = ((*y * max)-1)/2.;
    fz = ((*z * max)-1)/2.;
    mx = fx < 0 ? fx - 0.5 : fx + 0.5;
    my = fy < 0 ? fy - 0.5 : fy + 0.5;
    mz = fz < 0 ? fz - 0.5 : fz + 0.5;

    /* determine the expected returned floats */
    *x = (2. * mx + 1)/ max;
    *y = (2. * my + 1)/ max;
    *z = (2. * mz + 1)/ max;

    n[0] = mx; n[1] = my; n[2] = mz;
    glNormal3sv(n);
}

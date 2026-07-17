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

/* vertexes.c - $Revision: 2$ */

/*
 * This program tests all glColor commands
 * 
 */

#include <math.h>
#include "ogtst.h"	/* include test environment		*/

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

#define NFUNCS	24
#define NTIMES  5
#define MAXERR  1.0 	/* This should be reset to 1.0 ??  */
#define EO	1./32767.
#define ES 	1./255.

#define SLOP1   0.9959
#define SLOP2   0.0041

static struct {
    char *name;
    int called;
    float maxerr;
    void (*f)(float *, float *, float *, float *);
} func[] = {
    "2d",0,EO,do2d,   "2f",0,EO,do2f,   "2i",0,EO,do2i,   "2s",0,EO,do2s,
    "3d",0,EO,do3d,   "3f",0,EO,do3f,   "3i",0,EO,do3i,   "3s",0,EO,do3s, 
    "4d",0,EO,do4d,   "4f",0,EO,do4f,   "4i",0,EO,do4i,   "4s",0,EO,do4s, 
    "2dv",0,EO,do2dv, "2fv",0,EO,do2fv, "2iv",0,EO,do2iv, "2sv",0,EO,do2sv,
    "3dv",0,EO,do3dv, "3fv",0,EO,do3fv, "3iv",0,EO,do3iv, "3sv",0,EO,do3sv, 
    "4dv",0,EO,do4dv, "4fv",0,EO,do4fv, "4iv",0,EO,do4iv, "4sv",0,EO,do4sv
};

float orthxmin = -1.0;
float orthymin = -1.0;
float orthzmin = 1.0;
float orthxmax = 1.0;
float orthymax = 1.0;
float orthzmax =  -10.0;

TESTMOD(vertexes)
{
    int i,j,obj;
    unsigned int color;
    int xmax, ymax;
    float vx, vy, vz, vw;
    int rx, ry;
    float xtemp, ytemp;

    xmax = ogEnvQuery(OG_XWSIZE);
    ymax = ogEnvQuery(OG_YWSIZE);

    while (pass--) {
	for (i=0; i<NFUNCS; i++)
	    func[i].called = 0;
	/* call each function NTIMES */
	for (i=0; i<(NFUNCS*NTIMES); i++) {
	    /* find a function that hasn't been called NTIMES */
	    for (j=ogLibIntRand(0,NFUNCS-1); 
                 func[j].called == NTIMES; j=ogLibIntRand(0,NFUNCS-1));

	    vz = ogLibFloatRand(-1.0, 1.0);
	    vw = 1.0;

	    do {
		vx = ogLibFloatRand(-1.0, 1.0);
		vy = ogLibFloatRand(-1.0, 1.0);
		xtemp = (vx+1.0)*xmax*0.5;
		ytemp = (vy+1.0)*ymax*0.5;
		rx = (int)xtemp;
		ry = (int)ytemp;
	    } while((ABS(xtemp-rx) >= SLOP1 || ABS(xtemp-rx) <= SLOP2) || (ABS(ytemp-ry) >= SLOP1 || ABS(ytemp-ry) <= SLOP2));
	   /* This should avoid the problems of differences in conversions */  
	       
	    ogEnvLog(3, "xmax:%i, ymax:%i, xtemp:%f, ytemp:%f, rx:%i, ry:%i\n", xmax, ymax, xtemp, ytemp, rx, ry);
	    
	    glPointSize(MAXERR); /* This should be reset to 1.0 */
	
		glScissor(rx-2, ry-2, 5,5); /* Clear a small area */
		glClearColor(0.0,0.0,0.0,0.0);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor4f(1.0,0.0,0.0,1.0);

 	    START_DL_OR_IM(1);
	    ogEnvLog(1,"glVertex%s(%f,%f,%f,%f)\n",func[j].name,vx,vy,vz,vw);

	    func[j].f(&vx, &vy, &vz, &vw);
	    FINIS_DL_OR_IM(1);


		color = 0xff0000ff;
		ogLibPixelCheck(rx,ry,color);
	}
    }
}



CLEANUP(vertexes)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultColors();
    glScissor(0, 0, ogEnvQuery(OG_XWSIZE), ogEnvQuery(OG_YWSIZE));
    ogEnvMultiSamplingState(1);
}





/*ARGSUSED*/
static void do2d(float *x, float *y, float *z, float *w)
{
    GLdouble mx,my;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);


    mx = *x;
    my = *y;

    glBegin(GL_POINTS);
    glVertex2d(mx, my);
    glEnd();
}

/*ARGSUSED*/
static void do2f(float *x, float *y, float *z, float *w)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);


    glBegin(GL_POINTS);
    glVertex2f(*x, *y);
    glEnd();

}

/*ARGSUSED*/
static void do2i(float *x, float *y, float *z, float *w)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_I(orthxmin), (float)OGTST_F_TO_I(orthxmax),
            (float)OGTST_F_TO_I(orthymin), (float)OGTST_F_TO_I(orthymax),
            orthzmin, orthzmax);		

    glBegin(GL_POINTS);
    glVertex2i(OGTST_F_TO_I(*x), OGTST_F_TO_I(*y));
    glEnd();
}

/*ARGSUSED*/
static void do2s(float *x, float *y, float *z, float *w)
{ 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_S(orthxmin), (float)OGTST_F_TO_S(orthxmax),
            (float)OGTST_F_TO_S(orthymin), (float)OGTST_F_TO_S(orthymax),         
            orthzmin, orthzmax);            

    glBegin(GL_POINTS);
    glVertex2s(OGTST_F_TO_S(*x), OGTST_F_TO_S(*y));
    glEnd();

}

  
  

/*ARGSUSED*/
static void do3d(float *x, float *y, float *z, float *w)
{
    GLdouble mx,my,mz;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);

    mx = *x;
    my = *y;
    mz = *z;

    glBegin(GL_POINTS);
    glVertex3d(mx, my, mz);
    glEnd();
}

/*ARGSUSED*/
static void do3f(float *x, float *y, float *z, float *w)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);

    glBegin(GL_POINTS);
    glVertex3f(*x, *y, *z);
    glEnd();

}

/*ARGSUSED*/
static void do3i(float *x, float *y, float *z, float *w)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_I(orthxmin), (float)OGTST_F_TO_I(orthxmax), 
            (float)OGTST_F_TO_I(orthymin), (float)OGTST_F_TO_I(orthymax), 
            orthzmin, orthzmax);		

    glBegin(GL_POINTS);
    glVertex3i(OGTST_F_TO_I(*x), OGTST_F_TO_I(*y), (int)(*z));
    glEnd();
}

/*ARGSUSED*/
static void do3s(float *x, float *y, float *z, float *w)
{ 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_S(orthxmin), (float)OGTST_F_TO_S(orthxmax),
            (float)OGTST_F_TO_S(orthymin), (float)OGTST_F_TO_S(orthymax),         
            (float)(orthzmin), (float)(orthzmax));            

    glBegin(GL_POINTS);
    glVertex3s(OGTST_F_TO_S(*x), OGTST_F_TO_S(*y), (short)(*z));
    glEnd();
}




static void do4d(float *x, float *y, float *z, float *w)
{
    GLdouble mx,my,mz, mw;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);

    mx = *x;
    my = *y;
    mz = *z;
    mw = *w;

    glBegin(GL_POINTS);
    glVertex4d(mx, my, mz, mw);
    glEnd();
}



static void do4f(float *x, float *y, float *z, float *w)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);

    glBegin(GL_POINTS);
    glVertex4f(*x, *y, *z, *w);
    glEnd();

}

static void do4i(float *x, float *y, float *z, float *w)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_I(orthxmin), (float)OGTST_F_TO_I(orthxmax),
            (float)OGTST_F_TO_I(orthymin), (float)OGTST_F_TO_I(orthymax),
            orthzmin, orthzmax);		

    glBegin(GL_POINTS);
    glVertex4i(OGTST_F_TO_I(*x), OGTST_F_TO_I(*y), (int)(*z), (int)(*w));
    glEnd();
}

static void do4s(float *x, float *y, float *z, float *w)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_S(orthxmin), (float)OGTST_F_TO_S(orthxmax),
            (float)OGTST_F_TO_S(orthymin), (float)OGTST_F_TO_S(orthymax),
            orthzmin, orthzmax);            

    glBegin(GL_POINTS);
    glVertex4s(OGTST_F_TO_S(*x), OGTST_F_TO_S(*y), (short)(*z), (short)(*w));
    glEnd();
}



/*------ Vector Calls --------*/



/*ARGSUSED*/
static void do2dv(float *x, float *y, float *z, float *w)
{
    double mv[4];
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);
        
    mv[0] = *x;
    mv[1] = *y;

    glBegin(GL_POINTS);
    glVertex2dv(mv);
    glEnd();
}

/*ARGSUSED*/
static void do2fv(float *x, float *y, float *z, float *w)
{
    float mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);

    mv[0] = *x;
    mv[1] = *y;

    glBegin(GL_POINTS);
    glVertex2fv(mv);
    glEnd();

}

/*ARGSUSED*/
static void do2iv(float *x, float *y, float *z, float *w)
{
    int mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_I(orthxmin), (float)OGTST_F_TO_I(orthxmax),
            (float)OGTST_F_TO_I(orthymin), (float)OGTST_F_TO_I(orthymax),
            orthzmin, orthzmax);		

    mv[0] = OGTST_F_TO_I(*x);
    mv[1] = OGTST_F_TO_I(*y);
    glBegin(GL_POINTS);
    glVertex2iv(mv);
    glEnd();
}

/*ARGSUSED*/
static void do2sv(float *x, float *y, float *z, float *w)
{
    short mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_S(orthxmin), (float)OGTST_F_TO_S(orthxmax), 
            (float)OGTST_F_TO_S(orthymin), (float)OGTST_F_TO_S(orthymax), 
            orthzmin, orthzmax);            

    mv[0] = OGTST_F_TO_S(*x);
    mv[1] = OGTST_F_TO_S(*y);

    glBegin(GL_POINTS);
    glVertex2sv(mv);
    glEnd();

}



/*ARGSUSED*/
static void do3dv(float *x, float *y, float *z, float *w)
{
    double mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);

        
    mv[0] = *x;
    mv[1] = *y;
    mv[2] = *z;

    glBegin(GL_POINTS);
    glVertex3dv(mv);
    glEnd();
}

/*ARGSUSED*/
static void do3fv(float *x, float *y, float *z, float *w)
{
    float mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);

    mv[0] = *x;
    mv[1] = *y;
    mv[2] = *z;

    glBegin(GL_POINTS);
    glVertex3fv(mv);
    glEnd();

}


/*ARGSUSED*/
static void do3iv(float *x, float *y, float *z, float *w)
{
    int mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_I(orthxmin), (float)OGTST_F_TO_I(orthxmax), 
            (float)OGTST_F_TO_I(orthymin), (float)OGTST_F_TO_I(orthymax), 
            orthzmin, orthzmax);		

    mv[0] = OGTST_F_TO_I(*x);
    mv[1] = OGTST_F_TO_I(*y);
    mv[2] = (int)(*z);

    glBegin(GL_POINTS);
    glVertex3iv(mv);
    glEnd();
}


/*ARGSUSED*/
static void do3sv(float *x, float *y, float *z, float *w)
{
    short mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_S(orthxmin), (float)OGTST_F_TO_S(orthxmax), 
            (float)OGTST_F_TO_S(orthymin), (float)OGTST_F_TO_S(orthymax), 
            orthzmin, orthzmax);            

    mv[0] = OGTST_F_TO_S(*x);
    mv[1] = OGTST_F_TO_S(*y);
    mv[2] = (short)(*z);

    glBegin(GL_POINTS);
    glVertex3sv(mv);
    glEnd();
}


static void do4dv(float *x, float *y, float *z, float *w)
{
    double mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);
        
    mv[0] = *x;
    mv[1] = *y;
    mv[2] = *z;
    mv[3] = *w;

    glBegin(GL_POINTS);
    glVertex4dv(mv);
    glEnd();
}

static void do4fv(float *x, float *y, float *z, float *w)
{
    float mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(orthxmin, orthxmax, orthymin, orthymax,orthzmin, orthzmax);
    glMatrixMode(GL_MODELVIEW);

    mv[0] = *x;
    mv[1] = *y;
    mv[2] = *z;
    mv[3] = *w;

    glBegin(GL_POINTS);
    glVertex4fv(mv);
    glEnd();
}


static void do4iv(float *x, float *y, float *z, float *w)
{
    int mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_I(orthxmin), (float)OGTST_F_TO_I(orthxmax), 
            (float)OGTST_F_TO_I(orthymin), (float)OGTST_F_TO_I(orthymax), 
            orthzmin, orthzmax);		

    mv[0] = OGTST_F_TO_I(*x);
    mv[1] = OGTST_F_TO_I(*y);
    mv[2] = (int)(*z);
    mv[3] = (int)(*w);

    glBegin(GL_POINTS);
    glVertex4iv(mv);
    glEnd();
}


static void do4sv(float *x, float *y, float *z, float *w)
{
    short mv[4];
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho((float)OGTST_F_TO_S(orthxmin), (float)OGTST_F_TO_S(orthxmax), 
            (float)OGTST_F_TO_S(orthymin), (float)OGTST_F_TO_S(orthymax), 
            orthzmin, orthzmax);            

    mv[0] = OGTST_F_TO_S(*x);
    mv[1] = OGTST_F_TO_S(*y);
    mv[2] = (short)(*z);
    mv[3] = (short)(*w);

    glBegin(GL_POINTS);
    glVertex4sv(mv);
    glEnd();
}


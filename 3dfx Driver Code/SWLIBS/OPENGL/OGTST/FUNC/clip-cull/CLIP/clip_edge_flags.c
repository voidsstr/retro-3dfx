
/* clip_edge_flags.c - $Revision: 2$ */

/*
 * tests edge flag behavior under clipping.
 *
 */

#include "ogtst.h"       /* include test environment             */

static int Divs = 2;
static int DoClip = 1;
static int PolyMode = 1;

void doit(double plane[4])
{
    int i,j;
    float v0,v1,u;
    /* float u0,u1; */
    int c;
    static GLfloat colors[7][3] = 
      {{  1.0, 0.0, 0.0 }, /* red */
       {  0.0, 1.0, 0.0 }, /* green */
       {  0.0, 0.0, 1.0 }, /* blue */
       {  1.0, 1.0, 0.0 }, /* yellow */
       {  1.0, 0.0, 1.0 }, /* magenta */
       {  0.0, 1.0, 1.0 }, /* cyan */
       {  0.5, 0.5, 0.5 }, /* cyan */
      };


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-0.6,0.6,-0.6,0.6,-10,10);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glLineWidth(1.);
    if (DoClip) glEnable(GL_CLIP_PLANE0);
    glClipPlane(GL_CLIP_PLANE0, plane);
    glTranslatef(-.5, -.5, -2.0);

    if (PolyMode){
       glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    }

#if 1
    c = 0;
    for (j = 0; j < Divs; j++) {
	v0 = (GLfloat)j/Divs;
	v1 = (GLfloat)(j+1)/Divs;
#if 0
	glBegin(GL_TRIANGLE_STRIP);
#else
	glBegin(GL_QUAD_STRIP);
#endif
	for (i = 0; i <= Divs; i++) {
	    u = (float)i/Divs;
	       glColor3fv(&colors[c][0]); c = (c+1)%7;
	    glVertex3f(u,v0,0);
	       glColor3fv(&colors[c][0]); c = (c+1)%7;
	    glVertex3f(u,v1,0);
	}
	glEnd();
    }
#else
    glColor3f(0.5,0.5,0.5);
    for (j = 0; j < Divs; j++) {
	v0 = (GLfloat)j/Divs;
	v1 = (GLfloat)(j+1)/Divs;
	glBegin(GL_QUADS);
	for (i = 0; i < Divs; i++) {
	    u0 = (float)i/Divs;
	    u1 = (float)(i+1)/Divs;
	    glVertex3f(u0+(0.125*(u1-u0)),v0+(0.125*(v1-v0)),0);
	    glVertex3f(u0+(0.125*(u1-u0)),v1,                0);
	    glVertex3f(u1,                v1,                0);
	    glVertex3f(u1,                v0+(0.125*(v1-v0)),0);
	}
	glEnd();
    }
#endif
}

/*ARGSUSED*/
TESTMOD(clip_edge_flags)
{
   int i,r,c;
   float a;
   double plane[4];

   glClear(GL_COLOR_BUFFER_BIT);
   i = 0;

#define ROWS 4
#define COLS 4

   for  (r = ROWS-1; r >= 0; r--){
      for (c = 0; c < COLS; c++){

         glViewport(c*(640/8),r*(512/8),(640/8),(512/8));

         a = i/((float)(ROWS*COLS)) * 2.0 * M_PI;
         plane[0] = sin(a);
         plane[1] = cos(a);
         plane[2] = 0.0;
         plane[3] = 0.0;
         doit(plane);
         i++;
      }
   }
}


CLEANUP(clip_edge_flags)
{
    GLdouble plane[4] = {0.0, 0.0, 0.0, 0.0};

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

    glViewport(0,0,ogEnvQuery(OG_XWSIZE),ogEnvQuery(OG_YWSIZE));

    glClipPlane(GL_CLIP_PLANE0, plane);
    glDisable(GL_CLIP_PLANE0);
    ogLibSetDefaultClears();
    ogLibSetDefaultColors();
}

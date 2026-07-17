/*
 *  cyl.c - $Revision: 2$
 *
 */

/*
 * tests the gluCylinder primitive
 */

#include <math.h>
#include "ogtst.h"

static float lm_amb[4] = {.10, .11, .12};
static float lm_localviewer = 0;
static float mat_emis[4] = {0, 0, 0};
static float mat_amb[4] = {.10, .11, .12};
static float mat_diff[4] = {0.369, 0.0, 0.165, 1};
static float mat_spec[4] = {.50, .51, .52};
static float mat_shin = 4;
static float lt_pos[4] = {0, 0, 1, 0};
static float lt_amb[4] = {0, 0, 0, 0};
static float lt_diff[4] = {1, 1, 1, 0};
static float lt_spec[4] = {1, 1, 1, 0};

static void drawobj(GLUquadricObj *qobj);


/*ARGSUSED*/
TESTMOD(cyls)
{
    GLUquadricObj *qobj;

    ogLibClear(0x808080ff) ;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 6.0, 0.0, 4.5, -4.0, 4.0);

    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,lm_amb);
    glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,lm_localviewer);

    glMaterialfv(GL_FRONT,GL_EMISSION,mat_emis);
    glMaterialfv(GL_FRONT,GL_AMBIENT,mat_amb);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,mat_diff);
    glMaterialfv(GL_FRONT,GL_SPECULAR,mat_spec);
    glMaterialf(GL_FRONT,GL_SHININESS,mat_shin);

    glLightfv(GL_LIGHT0,GL_POSITION,lt_pos);
    glLightfv(GL_LIGHT0,GL_AMBIENT,lt_amb);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,lt_diff);
    glLightfv(GL_LIGHT0,GL_SPECULAR,lt_spec);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);


    glMatrixMode(GL_MODELVIEW);
    qobj = gluNewQuadric();
    glLoadIdentity();
    glTranslatef(1.,1.,0.);
    drawobj(qobj);

    glLoadIdentity();
    glTranslatef(3.,1.,0.);
    gluQuadricDrawStyle(qobj,GLU_LINE);
    drawobj(qobj);

    glLoadIdentity();
    glTranslatef(5.,1.,0.);
    gluQuadricDrawStyle(qobj,GLU_SILHOUETTE);
    drawobj(qobj);

    glLoadIdentity();
    glTranslatef(1.,3.,0.);
    gluQuadricDrawStyle(qobj,GLU_POINT);
    drawobj(qobj);

    glLoadIdentity();
    glTranslatef(3.,3.,0.);
    gluQuadricDrawStyle(qobj,GLU_FILL);
    gluQuadricNormals(qobj,GLU_FLAT);
    drawobj(qobj);

    glLoadIdentity();
    glTranslatef(5.,3.,0.);
    gluQuadricNormals(qobj,GLU_SMOOTH);
    gluQuadricOrientation(qobj,GLU_INSIDE);
    drawobj(qobj);
}

static void
drawobj(GLUquadricObj *qobj)
{
	glRotatef(-60.,1.,0.,0.);
	glTranslatef(0.,0.,-.5);
	gluCylinder(qobj,1.0,0.5, 1.,16,2);
}

CLEANUP(cyls)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    ogLibSetDefaultLight();

    glNormal3f(0, 0, 1);
    ogLibSetDefaultClears();
}

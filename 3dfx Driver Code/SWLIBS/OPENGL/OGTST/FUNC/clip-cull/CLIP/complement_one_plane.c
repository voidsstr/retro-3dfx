
/* complement_one_plane.c - $Revision: 2$ */

/*
 * tests complementary property for clipped colored triangles with an 
 *  arbitrary plane
 *
 * Note that the cmplt* series of tests use multiple planes in a way
 * that has 4 planes active for one pass and then each of the 4 planes
 * active individually for the next 4 passes. These operations can 
 * generate "T-vertices" and so tearing will be seen in the image.
 * 
 * It is however, a large and comprehensive test, so I (airey) would
 * prefer to see it stick around and this test can be used to establish
 * that one plane, when negated, doesn't produce tears, as the OpenGL
 * spec requires.
 */

#include "ogtst.h"       /* include test environment             */

static void disableClipPlane(void);
static void setClipPlane(int status);
static void drawOneDiamond(void);
static void drawDiamonds(void);
static void initClipTest(void);
static void drawBackground(void);

/*ARGSUSED*/
TESTMOD(complement_one_plane)
{
    int planeStatus;

    initClipTest();

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glScaled(0.7, 0.7, 1.0);
    glTranslatef(-30.0, 80.0, -10.0);
    planeStatus = 1;
    setClipPlane(planeStatus);
    drawBackground();

    glLoadIdentity();
    glTranslatef(50.0, 50.0, -10.0);
    glScaled(0.7, 0.7, 1.0);
    drawDiamonds(); 
    disableClipPlane();

    glLoadIdentity();
    glScaled(0.7, 0.7, 1.0);
    glTranslatef(-30.0, 80.0, -10.0);
    planeStatus = -1;
    setClipPlane(planeStatus);

    glLoadIdentity();
    glTranslatef(50.0, 50.0, -10.0);
    glScaled(0.7, 0.7, 1.0);
    drawDiamonds(); 
    glDisable(GL_CLIP_PLANE0);
    planeStatus = 0;

    /*
    glEndList();
    glCallList(1);
    glCallList(2);
    glDeleteLists(1, 2);
    */

}

static void setClipPlane(int status)
{
    GLdouble plane[4];

    if (status != 0){
	plane[0] = -1.0 * status;
	plane[1] = 2.0 * status;
	plane[2] = 0.0 * status;
	plane[3] = 300.0 * status;
	glClipPlane(GL_CLIP_PLANE0, plane);
	glEnable(GL_CLIP_PLANE0);
    }


}

static void disableClipPlane(void)
{
    glDisable(GL_CLIP_PLANE0);
}

static void drawBackground(void)
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
        glVertex2f(1.0, 1.0); 
        glVertex2f(600.0, 1.0); 
        glVertex2f(600.0, 300.0);
        glVertex2f(1.0, 300.0); 
    glEnd();
}

static void drawOneDiamond(void)
{
    glBegin(GL_POLYGON);
	glColor3f(1.0, 0.0, 0.0);
        glVertex2f(40.0, 160.0); 

	glColor3f(0.0, 1.0, 0.0);
        glVertex2f(80.0, 80.0); 

	glColor3f(0.0, 0.0, 1.0);
        glVertex2f(40.0, 0.0);

	glColor3f(1.0, 0.0, 1.0);
        glVertex2f(0.0, 80.0); 
    glEnd();

}

static void drawDiamonds(void)
{

#if 0
    glTranslatef(-25.0, 65.0, -10.0);
    glTranslatef(85.0, 0.0, -10.0);
    glTranslatef(60.0, 85.0, -10.0);
    glTranslatef(100.0, 15.0, -10.0);
    glTranslatef(100.0, 5.0, -10.0);

    glTranslatef(110.0, -70.0, -10.0);
#endif
    glTranslatef(430.,100.,-60.);
    drawOneDiamond();

    glTranslatef(-70.0, -60.0, -10.0);
    drawOneDiamond();

    glTranslatef(-80.0, 25.0, -10.0);
    drawOneDiamond();

    glTranslatef(-60.0, -140.0, -10.0);
    drawOneDiamond();

    glTranslatef(-90.0, 60.0, -10.0);
    drawOneDiamond();

}

static void initClipTest(void)
{
    GLint width, height;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    width = ogEnvQuery(OG_XWSIZE) - 1;
    height = ogEnvQuery(OG_YWSIZE) - 1;
    glOrtho(-0.5, width + 0.5, -0.5, height + 0.5, 0.01, 1000.0);

    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
}

CLEANUP(complement_one_plane)
{
    GLdouble plane[4] = {0.0, 0.0, 0.0, 0.0};

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClipPlane(GL_CLIP_PLANE0, plane);
    ogLibSetDefaultClears();
    ogLibSetDefaultColors();
}

/*ARGSUSED*/

/* clipcolorpnt.c - $Revision: 2$ */

/*
 * tests clipped colored points with an arbitrary planes
 */

#include "ogtst.h"       /* include test environment             */

static void disableClipPlanes(void);
static void setClipPlanes(void);
static void drawOneDiamond(void);
static void drawDiamonds(void);
static void drawOneRectangle(void);
static void drawRectangles(void);
static void initClipTest(void);


/*ARGSUSED*/
TESTMOD(clipcolorpnt)
{

    initClipTest();

    glMatrixMode(GL_MODELVIEW);

    glLoadIdentity();
    glTranslatef(-80.0, -5.0, -10.0);
    setClipPlanes();

    glLoadIdentity();
    drawDiamonds(); 
    disableClipPlanes();


    glNewList(1, GL_COMPILE);
    glLoadIdentity();
    glTranslatef(0.0, 220.0, -10.0);
    setClipPlanes();

    glLoadIdentity();
    glTranslatef(80.0, 220.0, -10.0);
    drawRectangles();
    disableClipPlanes();
    glEndList();
    glCallList(1);
    glDeleteLists(1, 1);

}

static void setClipPlanes(void)
{
    GLdouble plane[4];

/*	clipping planes

		0
	      /   \
	     /     \
	    1       2
	     \     /
	      \   /
		3
*/
	/* 3 - 2 */
    plane[0] = -1.0;
    plane[1] = 2.0;
    plane[2] = 0.0;
    plane[3] = 300.0;
    glClipPlane(GL_CLIP_PLANE0, plane);
    glEnable(GL_CLIP_PLANE0);

	/* 1 - 3 */
    plane[0] = 1.0;
    plane[1] = 2.0;
    plane[2] = 0.0;
    plane[3] = -400.0;
    glClipPlane(GL_CLIP_PLANE1, plane);
    glEnable(GL_CLIP_PLANE1); 

	/* 2 - 0 */
    plane[0] = -1.0;
    plane[1] = -2.0;
    plane[2] = 0.0;
    plane[3] = 900.0;
    glClipPlane(GL_CLIP_PLANE2, plane);
    glEnable(GL_CLIP_PLANE2); 

	/* 1 - 0 */
    plane[0] = 1.0;
    plane[1] = -2.0;
    plane[2] = 0.0;
    plane[3] = 200.0;
    glClipPlane(GL_CLIP_PLANE3, plane);
    glEnable(GL_CLIP_PLANE3);

	/* draw background */
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
        glVertex2f(1.0, 1.0); 
        glVertex2f(600.0, 1.0); 
        glVertex2f(600.0, 300.0);
        glVertex2f(1.0, 300.0); 
    glEnd();
}

static void disableClipPlanes(void)
{
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
}

static void drawOneDiamond(void)
{
    glBegin(GL_POINTS);
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

    glPointSize(1.0);
    glTranslatef(-25.0, 65.0, -10.0);
    drawOneDiamond();

    glPointSize(2.0);
    glTranslatef(85.0, 0.0, -10.0);
    drawOneDiamond();

    glPointSize(4.0);
    glTranslatef(60.0, 85.0, -10.0);
    drawOneDiamond();

    glPointSize(8.0);
    glTranslatef(100.0, 15.0, -10.0);
    drawOneDiamond();

    glPointSize(2.0);
    glTranslatef(100.0, 5.0, -10.0);
    drawOneDiamond();

    glPointSize(12.0);
    glTranslatef(110.0, -70.0, -10.0);
    drawOneDiamond();

    glPointSize(5.0);
    glTranslatef(-70.0, -60.0, -10.0);
    drawOneDiamond();

    glPointSize(7.0);
    glTranslatef(-80.0, 30.0, -10.0);
    drawOneDiamond();

    glPointSize(14.0);
    glTranslatef(-60.0, -140.0, -10.0);
    drawOneDiamond();

    glPointSize(1.0);
    glTranslatef(-90.0, 60.0, -10.0);
    drawOneDiamond();

}

static void drawOneRectangle(void)
{
    glBegin(GL_POINTS);
	glColor3f(1.0, 0.0, 0.0);
        glVertex2f(0.0, 0.0); 

	glColor3f(0.0, 1.0, 0.0);
        glVertex2f(50.0, 0.0); 

	glColor3f(0.0, 0.0, 1.0);
        glVertex2f(50.0, 95.0);

	glColor3f(1.0, 0.0, 1.0);
        glVertex2f(0.0, 95.0); 
    glEnd();

}

static void drawRectangles(void)
{

    glPointSize(1.0);
    glTranslatef(-5.0, 100.0, -10.0);
    drawOneRectangle();

    glPointSize(3.0);
    glTranslatef(85.0, 0.0, -10.0);
    drawOneRectangle();

    glPointSize(5.0);
    glTranslatef(60.0, 85.0, -10.0);
    drawOneRectangle();

    glPointSize(7.0);
    glTranslatef(100.0, 15.0, -10.0);
    drawOneRectangle();

    glPointSize(8.0);
    glTranslatef(100.0, 5.0, -10.0);
    drawOneRectangle();

    glPointSize(5.0);
    glTranslatef(110.0, -70.0, -10.0);
    drawOneRectangle();

    glPointSize(2.0);
    glTranslatef(-70.0, -60.0, -10.0);
    drawOneRectangle();

    glPointSize(4.0);
    glTranslatef(-80.0, 30.0, -10.0);
    drawOneRectangle();

    glPointSize(11.0);
    glTranslatef(-60.0, -140.0, -10.0);
    drawOneRectangle();

    glPointSize(13.0);
    glTranslatef(-90.0, 60.0, -10.0);
    drawOneRectangle();

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

CLEANUP(clipcolorpnt)
{
    GLdouble plane[4] = {0.0, 0.0, 0.0, 0.0};

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glPointSize(1.0);

    glClipPlane(GL_CLIP_PLANE0, plane);
    glClipPlane(GL_CLIP_PLANE1, plane);
    glClipPlane(GL_CLIP_PLANE2, plane);
    glClipPlane(GL_CLIP_PLANE3, plane);
    ogLibSetDefaultClears();
    ogLibSetDefaultColors();
}





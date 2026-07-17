
/* cmplttextlin.c - $Revision: 2$ */

/*
 * tests clipped textured lines with an arbitrary planes
 */

#include "ogtst.h"       /* include test environment             */


#define BYTE_ONE 0xff

#define COMPON	4
#define TEXSIZE	4
#define TEXSIZE_4	(TEXSIZE >> 2)

static void disableClipPlanes(void);
static void setClipPlanes(int *status);
static void drawOneDiamond(void);
static void drawDiamonds(void);
static void drawOneRectangle(void);
static void drawRectangles(void);
static void initClipTest(void);
static void drawBackground(void);

/*ARGSUSED*/
TESTMOD(cmplttextlin)
{
    int i, planeStatus[4];

    initClipTest();

    glMatrixMode(GL_MODELVIEW);

	/* test # 1 */
	    /* draw interior */
    glLoadIdentity();
    glScaled(0.7, 0.7, 1.0);
    glTranslatef(-30.0, 80.0, -10.0);
    planeStatus[0] = 1;
    planeStatus[1] = 1;
    planeStatus[2] = 1;
    planeStatus[3] = 1;
    setClipPlanes(planeStatus);
    drawBackground();

    glLoadIdentity();
    glTranslatef(50.0, 50.0, -10.0);
    glScaled(0.7, 0.7, 1.0);
    drawDiamonds(); 
    disableClipPlanes();

	    /* draw exterior */
    planeStatus[0] = 0;
    planeStatus[1] = 0;
    planeStatus[2] = 0;
    planeStatus[3] = 0;
    for (i = 0; i < 4; i ++ ){
	glLoadIdentity();
	glScaled(0.7, 0.7, 1.0);
	glTranslatef(-30.0, 80.0, -10.0);
	planeStatus[i] = -1;
	setClipPlanes(planeStatus);

	glLoadIdentity();
	glTranslatef(50.0, 50.0, -10.0);
	glScaled(0.7, 0.7, 1.0);
	drawDiamonds(); 
	glDisable(GL_CLIP_PLANE0 + i);
	planeStatus[i] = 0;
    }

	/* test # 2 */
	    /* draw interior */
    glNewList(1, GL_COMPILE); 
    glLoadIdentity();
    glScaled(0.8, 0.8, 1.0); 
    glTranslatef(200.0, 320.0, -10.0);
    planeStatus[0] = 1;
    planeStatus[1] = 1;
    planeStatus[2] = 1;
    planeStatus[3] = 1;
    setClipPlanes(planeStatus);
    drawBackground();

    glLoadIdentity();
    glScaled(0.8, 0.8, 1.0); 
    glTranslatef(280.0, 320.0, -10.0);
    drawRectangles();
    disableClipPlanes();
    glEndList();

	    /* draw exterior */
    glNewList(2, GL_COMPILE); 
    planeStatus[0] = 0;
    planeStatus[1] = 0;
    planeStatus[2] = 0;
    planeStatus[3] = 0;
    for (i = 0; i < 4; i ++ ){
	glLoadIdentity();
	glScaled(0.8, 0.8, 1.0); 
	glTranslatef(200.0, 320.0, -10.0);
	planeStatus[i] = -1;
	setClipPlanes(planeStatus);

	glLoadIdentity();
	glScaled(0.8, 0.8, 1.0); 
	glTranslatef(280.0, 320.0, -10.0);
	drawRectangles(); 
	glDisable(GL_CLIP_PLANE0 + i);
	planeStatus[i] = 0;
    }
    glEndList();
    glCallList(1);
    glCallList(2);
    glDeleteLists(1, 2);


	/* test # 3 */
	    /* draw interior */
    glLoadIdentity();
    glTranslatef(10.0, 340.0, -10.0);
    glScaled(0.3, 0.3, 1.0); 
    planeStatus[0] = 1;
    planeStatus[1] = 1;
    planeStatus[2] = 1;
    planeStatus[3] = 1;
    setClipPlanes(planeStatus);
    drawBackground();

    glLoadIdentity();
    glTranslatef(75.0, 300.0, -10.0); 
    glScaled(1.0, 1.0, 1.0);
    drawOneDiamond();
    disableClipPlanes();

	    /* draw exterior */
    planeStatus[0] = 0;
    planeStatus[1] = 0;
    planeStatus[2] = 0;
    planeStatus[3] = 0;
    for (i = 0; i < 4; i ++ ){
	glLoadIdentity();
	glTranslatef(10.0, 340.0, -10.0);
	glScaled(0.3, 0.3, 1.0); 
	planeStatus[i] = -1;
	setClipPlanes(planeStatus);

	glLoadIdentity();
	glTranslatef(75.0, 300.0, -10.0); 
	glScaled(1.0, 1.0, 1.0);
	drawOneDiamond(); 
	glDisable(GL_CLIP_PLANE0 + i);
	planeStatus[i] = 0;
    }

	/* test # 4 */
	    /* draw interior */
    glNewList(1, GL_COMPILE); 
    glLoadIdentity();
    glTranslatef(450.0, 100.0, -10.0);
    glScaled(0.2, 0.6, 1.0); 
    planeStatus[0] = 1;
    planeStatus[1] = 1;
    planeStatus[2] = 1;
    planeStatus[3] = 1;
    setClipPlanes(planeStatus);
    drawBackground();

    glLoadIdentity();
    glTranslatef(495.0, 130.0, -10.0); 
    glScaled(1.0, 1.3, 1.0);
    drawOneRectangle();
    disableClipPlanes();
    glEndList();

	    /* draw exterior */
    glNewList(2, GL_COMPILE); 
    planeStatus[0] = 0;
    planeStatus[1] = 0;
    planeStatus[2] = 0;
    planeStatus[3] = 0;
    for (i = 0; i < 4; i ++ ){
	glLoadIdentity();
	glTranslatef(450.0, 100.0, -10.0); 
	glScaled(0.2, 0.6, 1.0); 
	planeStatus[i] = -1;
	setClipPlanes(planeStatus);

	glLoadIdentity();
	glTranslatef(495.0, 130.0, -10.0); 
	glScaled(1.0, 1.3, 1.0);
	drawOneRectangle(); 
	glDisable(GL_CLIP_PLANE0 + i);
	planeStatus[i] = 0;
    }
    glEndList();
    glCallList(1);
    glCallList(2);
    glDeleteLists(1, 2);


}

static void setClipPlanes(int *status)
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
    if (status[0] != 0){
	plane[0] = -1.0 * status[0];
	plane[1] = 2.0 * status[0];
	plane[2] = 0.0 * status[0];
	plane[3] = 300.0 * status[0];
	glClipPlane(GL_CLIP_PLANE0, plane);
	glEnable(GL_CLIP_PLANE0);
    }

	/* 1 - 3 */
    if (status[1] != 0){
	plane[0] = 1.0 * status[1];
	plane[1] = 2.0 * status[1];
	plane[2] = 0.0 * status[1];
	plane[3] = -400.0 * status[1];
	glClipPlane(GL_CLIP_PLANE1, plane);
	glEnable(GL_CLIP_PLANE1); 
    }

	/* 2 - 0 */
    if (status[2] != 0){
	plane[0] = -1.0 * status[2];
	plane[1] = -2.0 * status[2];
	plane[2] = 0.0 * status[2];
	plane[3] = 900.0 * status[2];
	glClipPlane(GL_CLIP_PLANE2, plane);
	glEnable(GL_CLIP_PLANE2); 
    }

	/* 1 - 0 */
    if (status[3] != 0){
	plane[0] = 1.0 * status[3];
	plane[1] = -2.0 * status[3];
	plane[2] = 0.0 * status[3];
	plane[3] = 200.0 * status[3];
	glClipPlane(GL_CLIP_PLANE3, plane);
	glEnable(GL_CLIP_PLANE3);
    }

}

static void disableClipPlanes(void)
{
    glDisable(GL_CLIP_PLANE0);
    glDisable(GL_CLIP_PLANE1);
    glDisable(GL_CLIP_PLANE2);
    glDisable(GL_CLIP_PLANE3);
}

static void drawBackground(void)
{
    glDisable(GL_TEXTURE_2D);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
        glVertex2f(1.0, 1.0); 
        glVertex2f(600.0, 1.0); 
        glVertex2f(600.0, 300.0);
        glVertex2f(1.0, 300.0); 
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

static void drawOneDiamond(void)
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINE_LOOP);
	glTexCoord2f(0.0, 0.0);
        glVertex2f(40.0, 160.0); 

	glTexCoord2f(2.0, 0.0);
        glVertex2f(80.0, 80.0); 

	glTexCoord2f(2.0, 2.0);
        glVertex2f(40.0, 0.0);

	glTexCoord2f(0.0, 2.0);
        glVertex2f(0.0, 80.0); 
    glEnd();

}

static void drawDiamonds(void)
{

    glLineWidth(2.0);
    glTranslatef(-25.0, 65.0, -10.0);
    drawOneDiamond();

    glLineWidth(3.0);
    glTranslatef(85.0, 0.0, -10.0);
    drawOneDiamond();

    glLineWidth(4.0);
    glTranslatef(60.0, 85.0, -10.0);
    drawOneDiamond();

    glLineWidth(7.0);
    glTranslatef(100.0, 15.0, -10.0);
    drawOneDiamond();

    glLineWidth(1.0);
    glTranslatef(100.0, 5.0, -10.0);
    drawOneDiamond();

    glLineWidth(8.0);
    glTranslatef(110.0, -70.0, -10.0);
    drawOneDiamond();

    glLineWidth(9.0);
    glTranslatef(-70.0, -60.0, -10.0);
    drawOneDiamond();

    glLineWidth(1.0);
    glTranslatef(-80.0, 30.0, -10.0);
    drawOneDiamond();

    glLineWidth(2.0);
    glTranslatef(-60.0, -140.0, -10.0);
    drawOneDiamond();

    glLineWidth(5.0);
    glTranslatef(-90.0, 60.0, -10.0);
    drawOneDiamond();

}

static void drawOneRectangle(void)
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINE_LOOP);
	glTexCoord2f(0.0, 0.0);
        glVertex2f(0.0, 0.0); 

	glTexCoord2f(2.0, 0.0);
        glVertex2f(50.0, 0.0); 

	glTexCoord2f(2.0, 2.0);
        glVertex2f(50.0, 95.0);

	glTexCoord2f(0.0, 2.0);
        glVertex2f(0.0, 95.0); 
    glEnd();

}

static void drawRectangles(void)
{

    glLineWidth(3.0);
    glTranslatef(-5.0, 100.0, -10.0);
    drawOneRectangle();

    glLineWidth(1.0);
    glTranslatef(85.0, 0.0, -10.0);
    drawOneRectangle();

    glLineWidth(5.0);
    glTranslatef(60.0, 85.0, -10.0);
    drawOneRectangle();

    glLineWidth(5.0);
    glTranslatef(100.0, 15.0, -10.0);
    drawOneRectangle();

    glLineWidth(2.0);
    glTranslatef(100.0, 5.0, -10.0);
    drawOneRectangle();

    glLineWidth(4.0);
    glTranslatef(110.0, -70.0, -10.0);
    drawOneRectangle();

    glLineWidth(5.0);
    glTranslatef(-70.0, -60.0, -10.0);
    drawOneRectangle();

    glLineWidth(2.0);
    glTranslatef(-80.0, 30.0, -10.0);
    drawOneRectangle();

    glLineWidth(0.5);
    glTranslatef(-60.0, -140.0, -10.0);
    drawOneRectangle();

    glLineWidth(3.0);
    glTranslatef(-90.0, 60.0, -10.0);
    drawOneRectangle();

}


static void initClipTest(void)
{
    int i, j;
    GLint width, height;
    GLubyte texture[TEXSIZE * TEXSIZE * COMPON];

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    width = ogEnvQuery(OG_XWSIZE) - 1;
    height = ogEnvQuery(OG_YWSIZE) - 1;
    glOrtho(-0.5, width + 0.5, -0.5, height + 0.5, 0.01, 1000.0);

    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

	/* unsigned byte */
    for (i = 0; i <TEXSIZE; i ++){
      for(j=0; j<TEXSIZE; j++){
        if (((j + i*TEXSIZE_4) % TEXSIZE) < TEXSIZE_4){
           texture[i*TEXSIZE*COMPON + j*COMPON] = BYTE_ONE;
           texture[i*TEXSIZE*COMPON + j*COMPON + 1] = BYTE_ONE;
           texture[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           texture[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ONE;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*2)){
           texture[i*TEXSIZE*COMPON + j*COMPON] = BYTE_ONE;
           texture[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           texture[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           texture[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ONE;
        }else if (((j + i*TEXSIZE_4) % TEXSIZE) < (TEXSIZE_4*3)){
           texture[i*TEXSIZE*COMPON + j*COMPON] = 0;
           texture[i*TEXSIZE*COMPON + j*COMPON + 1] = BYTE_ONE;
           texture[i*TEXSIZE*COMPON + j*COMPON + 2] = 0;
           texture[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ONE;
        }else{
           texture[i*TEXSIZE*COMPON + j*COMPON] = 0;
           texture[i*TEXSIZE*COMPON + j*COMPON + 1] = 0;
           texture[i*TEXSIZE*COMPON + j*COMPON + 2] = BYTE_ONE;
           texture[i*TEXSIZE*COMPON + j*COMPON + 3] = BYTE_ONE;
        }
      }
    }


    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXSIZE, TEXSIZE, 0, 
					GL_RGBA, GL_UNSIGNED_BYTE, texture);
}

CLEANUP(cmplttextlin)
{
    GLdouble plane[4] = {0.0, 0.0, 0.0, 0.0};

    glLineWidth(1.0);
    glDisable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClipPlane(GL_CLIP_PLANE0, plane);
    glClipPlane(GL_CLIP_PLANE1, plane);
    glClipPlane(GL_CLIP_PLANE2, plane);
    glClipPlane(GL_CLIP_PLANE3, plane);

    ogLibSetDefaultTextures();
    ogLibSetDefaultClears();
}

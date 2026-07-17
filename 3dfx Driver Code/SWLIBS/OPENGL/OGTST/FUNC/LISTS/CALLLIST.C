/*
 * This program tests the ability to call display lists from within other
 * display lists testing out weird corner cases.
 *
 */
      
#include "ogtst.h"	/* include test environment */

static void
quadv(GLint x0, GLint y0, GLint x1, GLint y1) {
    ogEnvLog(OG_LPARAMETERS, "quadv(%d,%d,%d,%d)\n", x0,y0,x1,y1);
    glVertex2i(x0,y0);
    glVertex2i(x0,y1);
    glVertex2i(x1,y1);
    glVertex2i(x1,y0);
}

TESTMOD(calllist) {
    unsigned int xmax, ymax, color;
    GLint x0, y0, x1, y1;
    GLint cx0, cy0, cx1, cy1;
    GLuint dl1, dl2, dl3, dl4;

    glMatrixMode(GL_PROJECTION);
    glOrtho(0.0, (double)ogEnvQuery(OG_XWSIZE),
	  0.0, (double)ogEnvQuery(OG_YWSIZE),
	  -1.,1.);

    glMatrixMode(GL_MODELVIEW);

    xmax = ogEnvQuery(OG_XWSIZE) - 1; /* window x values 0 -> xmax */
    ymax = ogEnvQuery(OG_YWSIZE) - 1; /* window y values 0 -> ymax */

    while(pass--) {
	cx0 = x0 = ogLibIntRand(1,xmax-1);
	cx1 = x1 = ogLibIntRand(1,xmax-1);
	cy0 = y0 = ogLibIntRand(1,ymax-1);
	cy1 = y1 = ogLibIntRand(1,ymax-1);

	if (cy0 == cy1) cy0--, y0--;
	if (cx0 == cx1) cx0--, x0--;

	/* half open */
	if (cx0 > cx1) cx0--; else cx1--;
	if (cy0 > cy1) cy0--; else cy1--;

	ogLibClear(0);
	color = ogLibColor(); /* new color for each dlist */

	/* call non-existent list */
	ogEnvLog(OG_LGENERAL, "Call non-existent list\n");
	glCallList(2);

	/* delete non-existent list */
	ogEnvLog(OG_LGENERAL, "Delete non-existent list\n");
	glDeleteLists(2, 1);

	/* call empty list */
	ogEnvLog(OG_LGENERAL, "Create and call empty list\n");
	glNewList(1, GL_COMPILE);
	glEndList();
	glCallList(1);

	/* test list containing vertices */
	ogEnvLog(OG_LGENERAL, "Test list containing vertices\n");
	glNewList(1, GL_COMPILE);
	    quadv(x0,y0,x1,y1);
	glEndList();

	glBegin(GL_QUADS);
	    glCallList(1);
	glEnd();
        ogLibRectCheck(cx0, cy0, cx1, cy1, color, 0);

	/* test list containing naked begin */
	ogEnvLog(OG_LGENERAL, "Test naked begin\n");
	glNewList(1, GL_COMPILE);
	    glBegin(GL_QUADS);
	glEndList();

	ogLibClear(0);
	glCallList(1);
	    quadv(x0,y0,x1,y1);
	glEnd();
        ogLibRectCheck(cx0, cy0, cx1, cy1, color, 0);

	/* test list containing naked end */
	ogEnvLog(OG_LGENERAL, "Test naked end\n");
	glNewList(1, GL_COMPILE);
	    glEnd();
	glEndList();

	ogLibClear(0);
	glBegin(GL_QUADS);
	    quadv(x0,y0,x1,y1);
	glCallList(1);
        ogLibRectCheck(cx0, cy0, cx1, cy1, color, 0);

	/* test list containing calllist to end */
	ogEnvLog(OG_LGENERAL, "Test naked end within list\n");
	glNewList(2, GL_COMPILE);
	    glBegin(GL_QUADS);
		quadv(x0,y0,x1,y1);
	    glCallList(1);
	glEndList();

	ogLibClear(0);
	glCallList(2);
        ogLibRectCheck(cx0, cy0, cx1, cy1, color, 0);

	/* delete created display lists */
	glDeleteLists(1, 2);
    
	/* test embedded genlist calls */
	/* We are just looking for a segmentation fault. I don't
	   really care if it draws or not. */

	ogLibClear(0);
	
	ogEnvLog(OG_LGENERAL, "Test embedded glGenLists call\n");
	dl1 = glGenLists(1);
	
	color = ogLibColor();
	/* first list */
	glNewList(dl1, GL_COMPILE);
	    dl2 = glGenLists(1);
	    quadv(x0, y0, x1, y1);
	glEndList();
	
	/* second list */
	dl3 = glGenLists(1);
	glNewList(dl3, GL_COMPILE);
	    quadv(x0, y0, x1, y1);
	glEndList();

	/* third list */
	dl4 = glGenLists(1);
	glNewList(dl4, GL_COMPILE);
	    quadv(x0, y0, x1, y1);
	glEndList();

	/* call first list */
	glCallList(dl1);

	/* redefine first list */
	glNewList(dl1, GL_COMPILE);
	    glCallList(dl2);
	    glCallList(dl3);
	    glCallList(dl4);
	glEndList();

	glCallList(dl1);

	/* delete created display lists */
	glDeleteLists(dl1, 1);
	glDeleteLists(dl2, 1);
	glDeleteLists(dl3, 1);
	glDeleteLists(dl4, 1);

    }
}

CLEANUP(calllist) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    /* delete created display lists */
    glDeleteLists(1, 2);
    ogLibSetDefaultColors();
}

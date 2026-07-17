#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <windows.h>
#include <glide.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "winutil.h"

#pragma warning(disable : 4244)

#define CHKERROR() \
{ \
    int e = glGetError(); \
    if (e) printf("got error %d at line %d\n", e, __LINE__); \
}

#define DOUBLE_BUFFER 1

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(-2,2,-2,2,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,0,0,1);
    glEnable(GL_DEPTH_TEST);
}

static void
draw_scene(void) {
    static float ang = 0;

    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);

    glLoadIdentity();
    glRotatef(ang,0,0,1);
    ang += 1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1,1,0);
    glBegin(GL_TRIANGLE_STRIP);
    	glVertex3f(-1,-1,0); 
    	glVertex3f(-1, 1,0); 
    	glVertex3f( 1, 0,0); 
    glEnd();
    glColor3f(0,0,1);
    glBegin(GL_TRIANGLE_STRIP);
    	glVertex3f( 1,-1,-1); 
    	glVertex3f(-1, 0, 1); 
    	glVertex3f( 1, 1,-1); 
    glEnd();
    CHKERROR();
}

void main( int argc, char **argv) {
    int frames;
    int swapinterval = 5;
      
    initApplication(GetModuleHandle(NULL), 0, DOUBLE_BUFFER);

    init();
    frames = 5000/swapinterval;
    while (frames--) {
	draw_scene();
#if DOUBLE_BUFFER
	SwapBuffers(hDC);
#endif
    }
    while (!getkey());
    return;
}

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

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(-2,2,-2,2,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,0,0,1);
}

static void
draw_scene(void) {
    static float ang = 0;

    glLoadIdentity();
    glRotatef(ang,0,0,1);
    ang += 1;

    glClear(GL_COLOR_BUFFER_BIT + GL_DEPTH_BUFFER_BIT);
    glColor3f(1,1,0);
    glBegin(GL_TRIANGLE_STRIP);
    	glVertex2f(-1,-1); 
    	glVertex2f(-1, 1); 
    	glVertex2f( 1,-1); 
    	glVertex2f( 1, 1); 
    glEnd();
    CHKERROR();
}

void main( int argc, char **argv) {
    int frames;

    initApplication(GetModuleHandle(NULL), 0, FALSE);

    init();
    frames = 500;
    while (frames--) {
	draw_scene();
	SwapBuffers(hDC);
    }
}


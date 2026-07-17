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

HWND hWndMain;
HDC hDC;

#define CHKERROR() \
{ \
    int e = glGetError(); \
    if (e) printf("got error %d at line %d\n", e, __LINE__); \
}

float winw = 640.0f;
float winh = 480.0f;
float rectw, recth;
float spacew, spaceh;

float red[] = { 1, 0, 0, 1 };
float yellow[] = { 1, 1, 0, 1 };
float blue[] = { 0, 0, 1, 1 };

float lightpos[4] = { 0, 0, 1000, 1 };
float black[4] = { 0, 0, 0, 1 };
float white[4] = { 1, 1, 1, 1 };
float gray[4] = { .5, .5, .5, 1 };

static void 
init_tex(void) {
    int i, j, w, h, bsize;
    unsigned int *p, *tex;

    w = 64;
    h = 64;
    bsize = h * w * sizeof(int);
    bsize = (bsize + 7) & ~0x7;
    p = tex = (unsigned int *) malloc(bsize);
    for (i=0; i < h; i++) {
	for (j=0; j < w; j++) {
#define YELLOW 0x8000FFFF
#define BLUE   0x00FF0000
	    if ((i/8 + j/8) % 2) {
		*p++ = BLUE;
	    } else {
		*p++ = YELLOW;
	    }
	}
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 tex);
}

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(0,winw,0,winh,-2,2);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0,0,0,1);

    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, black);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
    glMaterialfv(GL_BACK, GL_DIFFUSE, red);
    glMaterialfv(GL_FRONT, GL_AMBIENT, black);
    glMaterialfv(GL_BACK, GL_AMBIENT, black);

    init_tex();

    spacew = winw / 4.0f * 0.335f; 
    spaceh = winh / 3.0f * 0.335f;
    rectw = 0.9f * spacew;
    recth = 0.9f * spaceh;
}

static void
alias(void) {
    glDisable(GL_BLEND);
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
}

static void
smooth(void) {
    glShadeModel(GL_SMOOTH);
}

static void
flat(void) {
    glShadeModel(GL_FLAT);
}

static void
draw(void) {
    float xlen = rectw;
    float ylen = recth;
    
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,0); glVertex2f(0,0);
    glTexCoord2f(0,1); glVertex2f(0,ylen);
    glTexCoord2f(1,0); glVertex2f(xlen,0);
    glTexCoord2f(1,1); glVertex2f(xlen,ylen);
    glEnd();
}

static void
draw_tri(void) {
    float xlen = rectw;
    float ylen = recth;

    glBegin(GL_TRIANGLES);
    	glVertex3f(0, 0, 0); 
    	glVertex3f(0, ylen, 0); 
    	glVertex3f(xlen, ylen/2.0, 0);
    glEnd();
}

static void
draw_bowtie(void) {
    float xlen = rectw;
    float ylen = recth;

    glBegin(GL_TRIANGLES);
	glColor4f(1,1,0,.5);
    	glVertex3f(0, 0, 0); 
    	glVertex3f(0, ylen, 0); 
    	glVertex3f(xlen, ylen/2.0, 0); 
	glColor4f(0,0,1,.5);
    	glVertex3f(xlen, 0, -1); 
    	glVertex3f(0, ylen/2.0, 1); 
    	glVertex3f(xlen, ylen, -1); 
    glEnd();
}

static void
draw_scene(void) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    draw();
    glTranslatef(spacew, 0, 0);
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_TEXTURE_2D);
    draw();
    glTranslatef(spacew, 0, 0);
    glPopAttrib();
    draw();
    glTranslatef(-2*spacew, spaceh, 0);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    draw_bowtie();
    glTranslatef(spacew, 0, 0);
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    draw_bowtie();
    glTranslatef(spacew, 0, 0);
    glPopAttrib();
    draw_bowtie();
    glTranslatef(-2*spacew, spaceh, 0);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    draw_bowtie();
    glTranslatef(spacew, 0, 0);
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_BLEND);
    draw_bowtie();
    glTranslatef(spacew, 0, 0);
    glPopAttrib();
    draw_bowtie();
    glTranslatef(-2*spacew, spaceh, 0);

    glDisable(GL_BLEND);
    glColor4f(1,1,1,1);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_LESS, 0.5);
    draw();
    glTranslatef(spacew, 0, 0);
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_ALPHA_TEST);
    draw();
    glTranslatef(spacew, 0, 0);
    glPopAttrib();
    draw();
    glTranslatef(-2*spacew, spaceh, 0);

    CHKERROR();
}

void main( int argc, char **argv) {
    initApplication(GetModuleHandle(NULL), 0, FALSE);

    init();
    draw_scene();
    SwapBuffers(hDC);

    while (getkey() != 'q');
    return;
}


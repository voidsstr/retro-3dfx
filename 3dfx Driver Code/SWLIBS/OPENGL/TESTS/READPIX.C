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

float posx, posy;
float rectw = 10, recth = 10;
int redraw;
int maxsize = 1024;
int rgba = GL_FALSE;
GLenum drawBuffer = GL_FRONT;
GLenum readBuffer = GL_FRONT;

unsigned int *dbuf, *rcbuf;
unsigned short *rzbuf;
float *rcfbuf;

static void 
init_pixels(void) {
    int i, j, w, h, bsize;
    unsigned int *p;

#define YELLOW 0x0000FFFF
#define BLUE   0x00FF0000
    w = 256;
    h = 256;
    bsize = h * w * sizeof(int);
    bsize = (bsize + 7) & ~0x7;
    p = dbuf = (unsigned int *) malloc(bsize);
    for (i=0; i < h; i++) {
	for (j=0; j < w; j++) {
	    if ((i/8 + j/8) % 2) {
		*p++ = BLUE;
	    } else {
		*p++ = YELLOW;
	    }
	}
    }
    rcbuf = (unsigned int *) malloc (maxsize * maxsize * sizeof(int));
    rzbuf = (unsigned short *) malloc (maxsize * maxsize * sizeof(short));
    rcfbuf = (unsigned float *) malloc (maxsize * maxsize * sizeof(float));
}

static void
init(void) {
    float w = 640;
    float h = 480;
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glScalef(1.0,1.0,1.0);
    glMatrixMode(GL_PROJECTION);
    glOrtho(0,w,0,h,-1,1);
    glMatrixMode(GL_MODELVIEW);

    glDepthFunc(GL_ALWAYS);
    glDepthMask(GL_TRUE);

    glShadeModel(GL_SMOOTH);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    init_pixels();
    glTexImage2D(GL_TEXTURE_2D, 0, 4, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 dbuf);
}

static void
draw_scene(void) {
    float xlen, ylen;

    glColor3f(1,1,1);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    xlen = 240; 
    ylen = 320;
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,0); glVertex3f(0,0,-1); 
    glTexCoord2f(0,1); glVertex3f(0,ylen,-1); 
    glTexCoord2f(1,0); glVertex3f(xlen,0,1); 
    glTexCoord2f(1,1); glVertex3f(xlen,ylen,1);
    glEnd();
}

static void
read_pix(void) {
    int w = 64, h = 64;
    unsigned char *buf;

    buf = (unsigned char *) malloc(4 * w * h);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, buf);
}

static void
draw_pix(void) {
    glRasterPos2f(posx, posy);
    glDrawPixels(256, 256, GL_RGBA, GL_UNSIGNED_BYTE, dbuf);
    printf("6.2f %6.2f\n", posx, posy);
}

void
redraw_scene(void) {
    GLenum fmt = rgba ? GL_RGBA : GL_RGB;

    switch (drawBuffer) {
    case GL_FRONT:
	glDrawBuffer(GL_BACK);
	break;
    case GL_BACK:
	glDrawBuffer(GL_FRONT);
	break;
    }
    glClearColor(1,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    glDrawBuffer(drawBuffer);
    glReadBuffer(readBuffer);

    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    draw_scene();

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    /* read and draw fast case */
    glReadPixels(posx, posy, rectw, recth, fmt, GL_UNSIGNED_BYTE, rcbuf);
    glRasterPos2f(250.0, 10.0);
    glDrawPixels(rectw, recth, fmt, GL_UNSIGNED_BYTE, rcbuf);

    /* read and draw general case */
    glReadPixels(posx, posy, rectw, recth, fmt, GL_FLOAT, rcfbuf);
    glRasterPos2f(250.0, 30.0);
    glDrawPixels(rectw, recth, fmt, GL_FLOAT, rcfbuf);

    /* read and draw depth */
    glReadPixels(posx, posy, rectw, recth, GL_DEPTH_COMPONENT,
		 GL_UNSIGNED_SHORT, rzbuf);
    glRasterPos2f(250.0, 200.0);
    glDrawPixels(rectw, recth, GL_RED, GL_UNSIGNED_SHORT, rzbuf);

    glColor3f(1,0,0);
    glBegin(GL_LINE_LOOP);
    glVertex2f(posx, posy);
    glVertex2f(posx, posy + recth);
    glVertex2f(posx + rectw, posy + recth);
    glVertex2f(posx + rectw, posy);
    glEnd();

    glFlush();
    CHKERROR();
}

int
dispatch_key(int key) {
    switch (key) {
    case 'f':
	rgba = !rgba;
	break;
    case 'j':
	posx = posx > 0 ? --posx : 0;
	break;
    case 'k':
	posx = posx < 640 ? ++posx : 640;
	break;
    case 'm':
	posy = posy > 0 ? --posy : 0;
	break;
    case 'i':
	posy = posy < 480 ? ++posy : 480;
	break;
    case 'w':
	rectw = rectw > 0 ? --rectw : 0;
	break;
    case 'W':
	rectw = rectw < 480 ? ++rectw : 480;
	break;
    case 'h':
	recth = recth > 0 ? --recth : 0;
	break;
    case 'H':
	recth = recth < 480 ? ++recth : 480;
	break;
    case 'd':
	drawBuffer = GL_FRONT;
	break;
    case 'D':
	drawBuffer = GL_BACK;
	break;
    case 'r':
	readBuffer = GL_FRONT;
	break;
    case 'R':
	readBuffer = GL_BACK;
	break;
    case 's':
	SwapBuffers(hDC);
	return 0;
    default:
	return 0;
	break;
    }
    return 1;
}

void main( int argc, char **argv) {
    int key;
    
    initApplication(GetModuleHandle(NULL), 0, TRUE);
    init();
    draw_scene();
    read_pix();

    while ((key = getkey()) != 'q') {
	if (dispatch_key(key)) {
	    redraw_scene();
	}
    }
    return;
}

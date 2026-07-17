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

int lod;
float winw = 640.0;
float winh = 480.0;

static void 
init_texture(void) {
    int i, j, w, h, wi, hi, bsize, n;
    unsigned int *tmap, *p;

    w = 256;
    h = 256;
    wi = w;
    hi = h;
    for (lod=0; wi || hi; lod++) {
      wi >>= 1;
      hi >>= 1;
    }
#define RED    0x000000FF
#define YELLOW 0x0000FFFF
#define BLUE   0x00FF0000
#define GREEN  0x0000FF00
    wi = w;
    hi = h;
    for (n=0; n < lod ; n++) {
      bsize = hi * wi * sizeof(int);
      bsize = (bsize + 7) & ~0x7;
      tmap = (unsigned int *) malloc(bsize * 2 /* XXX */);
      p = tmap;
      for (i=0; i < hi; i++) {
	for (j=0; j < wi; j++) {
	  if ((i/8 + j/8) % 2) {
	    if (n % 2) *p++ = RED;
	    else *p++ = BLUE;
	  } else {
	    if (n % 2) *p++ = GREEN;
	    else *p++ = YELLOW;
	  }
	}
      }
      glTexImage2D(GL_TEXTURE_2D, n, 3, wi, hi, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmap); 
      if (wi > 1) wi >>= 1;
      if (hi > 1) hi >>= 1;
    }
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(0,winw,0,winh,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,1,1,1);
#if 0
    glEnable(GL_DEPTH_TEST);
#endif
    init_texture();
}

static void
draw_scene(void) {
    static float ang = 0;
    float xlen, ylen;
    float rectw, recth;
    float spacew, spaceh;
    float smalls, smallt;
    int n;

#if 0
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glClearDepth(1.0);
#endif

    glLoadIdentity();
    glRotatef(ang,0,0,1);
    ang += 1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1,1,1);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    spacew = 0.25 * winw; 
    spaceh = 0.33 * winh;
    rectw = 0.9 * spacew;
    recth = 0.9 * spaceh;

    glLoadIdentity();

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    xlen = rectw;
    ylen = recth;
    for (n=0; n < lod; n++) {
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0,0); glVertex3f(0,0,0); 
      glTexCoord2f(0,1); glVertex3f(0,ylen,0); 
      glTexCoord2f(1,0); glVertex3f(xlen,0,0); 
      glTexCoord2f(1,1); glVertex3f(xlen,ylen,0);
      glEnd();
      xlen /= 2.0; 
      ylen /= 2.0;
    }

    glTranslatef(spacew, 0, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    xlen = rectw;
    ylen = recth;
    for (n=0; n < lod; n++) {
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0,0); glVertex3f(0,0,0); 
      glTexCoord2f(0,1); glVertex3f(0,ylen,0); 
      glTexCoord2f(1,0); glVertex3f(xlen,0,0); 
      glTexCoord2f(1,1); glVertex3f(xlen,ylen,0);
      glEnd();
      xlen /= 2.0; 
      ylen /= 2.0;
    }

    glTranslatef(spacew, 0, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    xlen = rectw;
    ylen = recth;
    for (n=0; n < lod; n++) {
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0,0); glVertex3f(0,0,0); 
      glTexCoord2f(0,1); glVertex3f(0,ylen,0); 
      glTexCoord2f(1,0); glVertex3f(xlen,0,0); 
      glTexCoord2f(1,1); glVertex3f(xlen,ylen,0);
      glEnd();
      xlen /= 2.0; 
      ylen /= 2.0;
    }

    glTranslatef(spacew, 0, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    xlen = rectw;
    ylen = recth;
    for (n=0; n < lod; n++) {
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0,0); glVertex3f(0,0,0); 
      glTexCoord2f(0,1); glVertex3f(0,ylen,0); 
      glTexCoord2f(1,0); glVertex3f(xlen,0,0); 
      glTexCoord2f(1,1); glVertex3f(xlen,ylen,0);
      glEnd();
      xlen /= 2.0; 
      ylen /= 2.0;
    }

    glTranslatef(-3.0*spacew, spaceh, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(-1,-1); glVertex3f(0,0,0); 
    glTexCoord2f(-1, 2); glVertex3f(0,recth,0); 
    glTexCoord2f( 2,-1); glVertex3f(rectw,0,0); 
    glTexCoord2f( 2, 2); glVertex3f(rectw,recth,0);
    glEnd();

    glTranslatef(spacew, 0, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(-1,-1); glVertex3f(0,0,0); 
    glTexCoord2f(-1, 2); glVertex3f(0,recth,0); 
    glTexCoord2f( 2,-1); glVertex3f(rectw,0,0); 
    glTexCoord2f( 2, 2); glVertex3f(rectw,recth,0);
    glEnd();

    glTranslatef(spacew, 0, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(-1,-1); glVertex3f(0,0,0); 
    glTexCoord2f(-1, 2); glVertex3f(0,recth,0); 
    glTexCoord2f( 2,-1); glVertex3f(rectw,0,0); 
    glTexCoord2f( 2, 2); glVertex3f(rectw,recth,0);
    glEnd();

    glTranslatef(spacew, 0, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(-1,-1); glVertex3f(0,0,0); 
    glTexCoord2f(-1, 2); glVertex3f(0,recth,0); 
    glTexCoord2f( 2,-1); glVertex3f(rectw,0,0); 
    glTexCoord2f( 2, 2); glVertex3f(rectw,recth,0);
    glEnd();

    glTranslatef(-3.0*spacew, spaceh, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBegin(GL_TRIANGLE_STRIP);
smalls = smallt = 1.0/8.0;
    glTexCoord2f(0, 0); glVertex3f(0,0,0); 
    glTexCoord2f(0, smallt); glVertex3f(0,recth,0); 
    glTexCoord2f(smalls, 0); glVertex3f(rectw,0,0); 
    glTexCoord2f(smalls, smallt); glVertex3f(rectw,recth,0);
    glEnd();

    glTranslatef(spacew, 0, 0);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0, 0); glVertex3f(0,0,0); 
    glTexCoord2f(0, smallt); glVertex3f(0,recth,0); 
    glTexCoord2f(smalls, 0); glVertex3f(rectw,0,0); 
    glTexCoord2f(smalls, smallt); glVertex3f(rectw,recth,0);
    glEnd();

    CHKERROR();
}

void main( int argc, char **argv) {
    initApplication(GetModuleHandle(NULL), 0, FALSE);

    init();
    draw_scene();
    SwapBuffers(hDC);

    while (!getkey());
    return;
}

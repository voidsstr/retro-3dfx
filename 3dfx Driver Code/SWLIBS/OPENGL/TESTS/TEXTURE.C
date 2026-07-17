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

int lod;
int mipmap = GL_FALSE;

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
      if (!mipmap && n > 0) break;
      bsize = hi * wi * sizeof(int);
      bsize = (bsize + 7) & ~0x7;
      tmap = (unsigned int *) malloc(bsize);
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
      glTexImage2D(GL_TEXTURE_2D, n, 3, wi, hi, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		   tmap); 
      if (wi > 1) wi >>= 1;
      if (hi > 1) hi >>= 1;
    }
}

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(0,1,0,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,1,1,1);

    glEnable(GL_TEXTURE_2D);
    init_texture();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

static void
draw_scene(void) {
    static float ang = 0;
    float xlen, ylen;
    int n;

    glLoadIdentity();
    glRotatef(ang,0,0,1);
    ang += 1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1,1,1);

    xlen = 0.5; 
    ylen = 0.5;
    for (n=0; n < lod; n++) {
      if (!mipmap && n > 0) break;
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0,0); glVertex3f(0,0,0); 
      glTexCoord2f(0,1); glVertex3f(0,ylen,0); 
      glTexCoord2f(1,0); glVertex3f(xlen,0,0); 
      glTexCoord2f(1,1); glVertex3f(xlen,ylen,0);
      glEnd();
      xlen /= 2.0; 
      ylen /= 2.0;
    }
    CHKERROR();
}

void main( int argc, char **argv) {
    initApplication(GetModuleHandle(NULL), 0, FALSE);

    init();
    draw_scene();

    while (getkey() != 'q');
    return;
}

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

#if 1
static void
init_texture(void) {
    unsigned int *tex = malloc(256 * 256 * sizeof(unsigned int));
    unsigned int *p;
    unsigned int blue = 0x00ff0000;
    unsigned int yellow = 0x0000ffff;
    int i, j;

    p = tex;
    for (j=0; j < 256; j++) {
	for (i=0; i < 256; i++) {
	    if (((i/8) + (j/8)) % 2) {
		*p++ = blue;
	    } else {
		*p++ = yellow;
	    }
	}
    }
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE,
		      tex);
}
#else
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
      tmap = (unsigned int *) malloc(bsize);
      p = tmap;
      for (i=0; i < hi; i++) {
	for (j=0; j < wi; j++) {
	  if ((i/32 + j/32) % 2) {
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
}
#endif

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    gluPerspective(30.0, 1.33, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,1,1,1);

    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    init_texture();
}

static void
draw_scene(void) {
    static float ang = 0;
    float xlen, ylen;

    xlen = 7.0; 
    ylen = 7.0;

    glLoadIdentity();
    glTranslatef(0.0,0,-10.0);
    glRotatef(-70,1,0,0);
    glTranslatef(-xlen/2.0, -ylen/2.0, 0.0);
    ang += 1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor3f(1,1,1);

    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,0); glVertex3f(0,0,0); 
    glTexCoord2f(0,1); glVertex3f(0,ylen,0); 
    glTexCoord2f(1,0); glVertex3f(xlen,0,0); 
    glTexCoord2f(1,1); glVertex3f(xlen,ylen,0);
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

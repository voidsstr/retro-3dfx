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

unsigned int red = 0x00ff0000;
unsigned int yellow = 0x00ff00ff;
unsigned int blue = 0x000000ff;
unsigned int green = 0x0000ff00;

unsigned int black = 0x00000000;
unsigned int white = 0xffffffff;

#define BLUE 0x00ff0000
#define GREEN 0x0000ff00
#define YELLOW 0x0000ffff
#define RED 0x000000ff
unsigned int colors[][2] = {
    BLUE, YELLOW,
    BLUE, RED,
    GREEN, YELLOW,
    GREEN, RED,
};

static void 
init_texture(int w, int h, unsigned int evenColor, unsigned int oddColor) {
    int i, j, wi, hi, bsize, n;
    unsigned int *tmap, *p;

    wi = w;
    hi = h;
    for (lod=0; wi || hi; lod++) {
      wi >>= 1;
      hi >>= 1;
    }
    wi = w;
    hi = h;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    if (mipmap) {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    } else {
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    for (n=0; n < lod ; n++) {
      if (!mipmap && n > 0) break;
      bsize = hi * wi * sizeof(int);
      bsize = (bsize + 7) & ~0x7;
      tmap = (unsigned int *) malloc(bsize);
      p = tmap;
      for (i=0; i < hi; i++) {
	for (j=0; j < wi; j++) {
	  if ((i/32 + j/32) % 2) {
	    if (n % 2) *p++ = black;
	    else *p++ = oddColor;
	  } else {
	    if (n % 2) *p++ = white;
	    else *p++ = evenColor;
	  }
	}
      }
      glTexImage2D(GL_TEXTURE_2D, n, 3, wi, hi, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmap); 
      if (wi > 1) wi >>= 1;
      if (hi > 1) hi >>= 1;
    }
}

static int row, col;
static int numrows = 12, numcols = 16;
static float xlen, ylen;

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(0,1,0,1,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,1,1,1);
    glColor3f(1,1,1);

    glEnable(GL_TEXTURE_2D);
    xlen = 1.0 / numcols; 
    ylen = 1.0 / numrows;
}

static void
next_position(void) {
    col++;
    if (col >= numcols) {
	col = 0;
	row++;
	if (row > numrows) {
	    row = 0;
	}
    }
    glLoadIdentity();
    glTranslatef(col * xlen, row * ylen, 0.0);
}

static void
draw_scene(void) {
    float w, h;
    int n;

    w = xlen * 0.9;
    h = ylen * 0.9;
    for (n=0; n < lod; n++) {
      if (!mipmap && n > 0) break;
      glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2f(0,0); glVertex3f(0,0,0); 
      glTexCoord2f(0,1); glVertex3f(0,h,0); 
      glTexCoord2f(1,0); glVertex3f(w,0,0); 
      glTexCoord2f(1,1); glVertex3f(w,h,0);
      glEnd();
      w /= 2.0; 
      h /= 2.0;
    }
    CHKERROR();
}

void main( int argc, char **argv)
{
    int i;
    int numtex = 64;
    int w = 256, h = 256;
    GLuint *names;

    initApplication(GetModuleHandle(NULL), 0, FALSE);

    names = (GLuint *) malloc(sizeof(GLuint) * numtex);
    glGenTextures(numtex, names);
    init();
    glClear(GL_COLOR_BUFFER_BIT);

    for (i=0; i < numtex; i++) {
	glBindTexture(GL_TEXTURE_2D, names[i]);
	init_texture(w, h, colors[i%4][0], colors[i%4][1]);
	draw_scene();
	next_position();
    }
    for (i=0; i < 16; i++) {
	next_position();
    }
    for (i=0; i < numtex; i++) {
	glBindTexture(GL_TEXTURE_2D, names[i]);
	draw_scene();
	next_position();
    }
    while (getkey() != 'q');
    return;
}

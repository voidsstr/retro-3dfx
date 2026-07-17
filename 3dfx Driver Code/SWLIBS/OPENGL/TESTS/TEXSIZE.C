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
float winw = 640.0f;
float winh = 480.0f;
float rectw, recth;
float spacew, spaceh;

unsigned int red = 0x000000FF;
unsigned int yellow = 0x0000FFFF;
unsigned int blue = 0x00FF0000;
unsigned int green = 0x0000FF00;

unsigned int maxAlpha = 0xFF000000;
unsigned int highAlpha = 0x80000000;
unsigned int mediumAlpha = 0x80000000;

static void
set_tex(unsigned int evenColor, unsigned int oddColor, unsigned int alpha, GLenum format) {
    int i, j, w, h, wi, hi, n, bsize;
    unsigned int *tmap, *l, c;
    unsigned short *s;
    unsigned char *b;

    wi = w = 64;
    hi = h = 64;
    for (lod=0; wi || hi; lod++) {
	wi >>= 1;
	hi >>= 1;
    }
    wi = w;
    hi = h;
    for (n=0; n < lod ; n++) {
	bsize = hi * wi * sizeof(int);
	bsize = (bsize + 7) & ~0x7;
	tmap = (unsigned int *) malloc(bsize * 2 /* XXX */);
	l = tmap;
	s = (unsigned short *) tmap;
	b = (unsigned char *) tmap;
	for (i=0; i < hi; i++) {
	    for (j=0; j < wi; j++) {
		if ((i/8 + j/8) % 2) {
#if SHOW_MIPMAPS
		    if (n % 2) {
			c = red | alpha;
		    } else {
			c = oddColor | alpha;
		    }
#else
		    c = oddColor | alpha;
#endif		    
		    switch (format) {
		    case GL_RGB:
		    case GL_RGBA:
			*l++ = c;
			break;
		    case GL_LUMINANCE:
			*l++ = c;
			break;
		    case GL_LUMINANCE_ALPHA:
			*l++ = c;
			break;
		    case GL_ALPHA:
			*l++ = 0x00;
			break;
		    }
		} else {
#if SHOW_MIPMAPS		    
		    if (n % 2) {
			c = green | alpha;
		    } else {
			c = evenColor | alpha;
		    }
#else
		    c = evenColor | alpha;
#endif		    
		    switch (format) {
		    case GL_RGB:
		    case GL_RGBA:
			*l++ = c;
			break;
		    case GL_LUMINANCE:
			*l++ = c;
			break;
		    case GL_LUMINANCE_ALPHA:
			*l++ = c;
			break;
		    case GL_ALPHA:
			*l++ = c;
			break;
		    }
		}
	    }
	}
	glTexImage2D(GL_TEXTURE_2D, n, format, wi, hi, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, tmap);
	if (wi > 1) wi >>= 1;
	if (hi > 1) hi >>= 1;
    }
}

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(0,winw,0,winh,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,1,1,1);
    glEnable(GL_TEXTURE_2D);

    spacew = winw / 4.0f * 0.335f; 
    spaceh = winh / 3.0f * 0.335f;
    rectw = 0.9f * spacew;
    recth = 0.9f * spaceh;
}

static void
draw_square(void) {
    float xlen, ylen;
    int n;
    
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
}

static void
make_tex(int w, int h) {
    int m, n;
    unsigned int *tex = malloc(w * h * sizeof(unsigned int));
    unsigned int *p = tex;

    for (n=0; n < h; n++) {
	for (m=0; m < w; m++) {
	    if ((m+n) % 2) {
		*p++ = 0x0000ffff;
	    } else {
		*p++ = 0x00ff0000;
	    }
	}
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 tex);
}

static void
draw_tex(int w, int h) {
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,0); glVertex3f(0,0,0); 
    glTexCoord2f(0,1); glVertex3f(0,h,0); 
    glTexCoord2f(1,0); glVertex3f(w,0,0); 
    glTexCoord2f(1,1); glVertex3f(w,h,0);
    glEnd();
}

static void
draw_scene(void) {
    int maxlod = 9;
    int m, n, w, h;
    float xo, yo;
    
    glClear(GL_COLOR_BUFFER_BIT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    /* GL_LINEAR_MIPMAP_NEAREST); */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    yo = 1;
    for (n=0; n < maxlod; n++) {
	h = 1 << n;
	xo = 1;
	for (m=0; m < maxlod; m++) {
	    w = 1 << m;
	    make_tex(w, h);
	    if (glGetError()) {
		glDisable(GL_TEXTURE_2D);
		glColor4f(1,0,0,1);
	    } else {
		glEnable(GL_TEXTURE_2D);
		glColor4f(1,1,1,1);
	    }
	    glLoadIdentity();
	    glTranslatef(xo, yo, 0);
	    draw_tex(w, h);
	    xo += w + 1;
	}
	yo += h + 1;
    }
    CHKERROR();
}

void main( int argc, char **argv) {
    initApplication(GetModuleHandle(NULL), 0, FALSE);

    init();
    draw_scene();

    while (!getkey());
    return;
}

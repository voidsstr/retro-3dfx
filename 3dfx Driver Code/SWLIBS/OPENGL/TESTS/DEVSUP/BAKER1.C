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

static void
init_texture(void) {
    int maxx = 128;
    int maxy = 8;
    int i, m, n, w, h, nlod = 0;
    int ncomps = 3;
    GLuint name;
    unsigned int **texels;
    GLenum x_wrap, y_wrap;

    w = maxx;
    h = maxy;
    while (w >= 1 || h >= 1) {
	nlod++;
	w >>= 1;
	h >>= 1;
    }
    
    texels = (unsigned int **) malloc(nlod * sizeof(unsigned int *));
    for (i=0; i < nlod; i++) {
	texels[i] = (unsigned int *) malloc(maxx * maxy * 4);
	for (n=0; n < maxy; n++) {
	    for (m=0; m < maxx; m++) {
		if (((n/8)+(m/8)) % 2) {
		    texels[i][n*maxx+m] = 0xff00ff00;
		} else {
		    texels[i][n*maxx+m] = 0xffffffff;
		}
	    }
	}
    }

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&name);
    glBindTexture(GL_TEXTURE_2D, name);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);

    for (i=0; i < nlod; i++) {
	w = maxx>>i ;
	h = maxy>>i ;
	if ( w <= 0 ) w = 1 ;
	if ( h <= 0 ) h = 1 ;
#if 0
	glTexImage2D  ( GL_TEXTURE_2D, i, ncomps, w, h,
			FALSE /* Border */, (ncomps==1)?GL_LUMINANCE:
			(ncomps==2)?GL_LUMINANCE_ALPHA:
			(ncomps==3)?GL_RGB:
			GL_RGBA,
			GL_UNSIGNED_BYTE, (GLvoid *)texels[i] ) ;
#else	
	glTexImage2D  ( GL_TEXTURE_2D, i, ncomps, w, h,
			FALSE /* Border */, GL_RGBA,
			GL_UNSIGNED_BYTE, (GLvoid *)texels[i] ) ;
#endif
    }
    x_wrap = y_wrap = GL_REPEAT;
    glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S    , x_wrap   ) ;
    glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T    , y_wrap   ) ;
    glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER, GL_LINEAR) ;
    glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER,
		      GL_LINEAR_MIPMAP_LINEAR ) ;

}

static void
init(void) {
    glMatrixMode(GL_PROJECTION);
    glOrtho(-2,2,-2,2,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,0,0,1);
    glEnable(GL_DEPTH_TEST);

    init_texture();
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
    	glTexCoord2f(0,0); glVertex3f(-1,-1,0); 
    	glTexCoord2f(0,1); glVertex3f(-1, 1,0); 
    	glTexCoord2f(1,0); glVertex3f( 1,-1,0); 
    	glTexCoord2f(1,1); glVertex3f( 1, 1,0);
    glEnd();
    CHKERROR();
}

void main( int argc, char **argv) {
    int getkey(void);
    int frames;
    int swapinterval = 5;

    init();
    frames = 100/swapinterval;
    while (frames--) {
	draw_scene();
	SwapBuffers(hDC);
    }
    while (!getkey());
    return;
}


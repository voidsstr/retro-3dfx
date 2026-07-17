#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#ifdef WIN32
#include <conio.h>
#include <windows.h>
#include <glide.h>
#include <gl/gl.h>
#include <gl/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>
#endif
#include "winutil.h"

#pragma warning(disable : 4244)

int getkey(void);

#define CHKERROR() \
{ \
    int e = glGetError(); \
    if (e) printf("got error %d at line %d\n", e, __LINE__); \
}

int texxsize = 256, texysize = 256;
int winxsize = 256, winysize = 256;
unsigned int *tex[4], *bas, *cmp;
int filter = GL_LINEAR;
int blend = 1;
#ifdef WIN32
GLenum ifmt = GL_RGB;
#else
GLenum ifmt = GL_LUMINANCE8;
#endif

#define BASTEX 1
#define CMPTEX 2
#define NOITEX 3
#define LAYTEX 10
#define RETTEX 50 /* hope we don't run into LAYTEX */
#define ABSTEX 100
#define CLOUDTEX 150
#define JITTERTEX 151

static int coffs[][2] = {
    0, 0,
    1, 0,
    0, 1,
    1, 1,
};
static void 
init_texture(void) {
    int i, j, n;
    int w, h;
    int cxo, cyo;
    int minn = 0xff, maxn = 0x00;
    
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glEnable(GL_TEXTURE_2D);

    bas = malloc(4 * texxsize * texysize);
    w = texxsize / 2;
    h = texysize / 2;
    for (j=0; j < h; j++) {
	for (i=0; i < w; i++) {
	    int r;
	    float u = i / (w - 1.0);
	    float v = j / (h - 1.0);
	    float f = 3 * u * u - 2 * u * u * u;
	    float g = 3 * v * v - 2 * v * v * v;
#if 0
	    float d = sqrt((1-u)*(1-u) + (1-v)*(1-v));
	    if (d > 1.0) d = 1.0;
	    d = 1.0 - d;
	    r = (3 * d * d  - 2 * d * d * d) * 0xff;
#else	    
	    r = f * g * 0xff;
#endif	    
	    r = (r << 24) | (r << 16) | (r << 8) | r; 
	    bas[j*texxsize + i] = r;
	    bas[j*texxsize + texxsize-i-1] = r;
	    bas[(texysize-j-1)*texxsize + i] = r;
	    bas[(texysize-j-1)*texxsize + texxsize-i-1] = r;
	}
    }
    glBindTexture(GL_TEXTURE_2D, BASTEX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, ifmt, texxsize, texysize, 0,
		 GL_RGBA, GL_UNSIGNED_BYTE, bas);
{
int rsize, bsize, gsize;
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE, &rsize);
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE, &bsize);
glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE, &gsize);
printf("texture size: %d %d %d \n", rsize, gsize, bsize);
}

    cmp = malloc(4 * texxsize * 2 * texysize * 2);
    for (n=0; n < 4; n++) {
	cxo = coffs[n][0];
	cyo = coffs[n][1];
	tex[n] = malloc(4 * texxsize * texysize);
	for (j=0; j < texysize; j++) {
	    for (i=0; i < texxsize; i++) {
		int r = rand() & 0xff;
		if (r < minn) minn = r;
		if (r > maxn) maxn = r;
#ifndef WIN32  /* rgba */
		r = (r << 24) | (r << 16) | (r << 8) | 0xff;
#else /* abgr */
		r = (0xff << 24) | (r << 16) | (r << 8) | r;
#endif		
		tex[n][j*texxsize + i] = r;
		cmp[(j*2+cyo)*(texxsize*2) + (i*2+cxo)] =  r;
	    }
	}
	glBindTexture(GL_TEXTURE_2D, n+NOITEX);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, ifmt, texxsize, texysize, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, tex[n]);
    }
}

float half =  0.5;
float extra = 2.0;

static void
draw_basis(int tsize, int ssize, int xadj, int yadj) {
    float bssize = 1.0 / ssize;
    float btsize = 1.0 / tsize;
    float xoff = (xadj - 0.5) * half * bssize;
    float yoff = (yadj - 0.5) * half * btsize;
    float xo, yo;
    int i, j;

    glBindTexture(GL_TEXTURE_2D, BASTEX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();

#ifdef WIN32 /* XXX */
    if (ssize <= 64) {
      grDitherMode(GR_DITHER_4x4);
    } else if (ssize <= 128) {
      grDitherMode(GR_DITHER_2x2);
    }
#endif
    for (j=0; j < tsize; j++) {
	for (i=0; i < ssize; i++) {
	    xo = xoff + i * bssize;
	    yo = yoff + j * btsize;
	    glBegin(GL_TRIANGLE_STRIP);
	    glTexCoord2f(0,0); glVertex2f(xo,yo); 
	    glTexCoord2f(0,1); glVertex2f(xo,yo+btsize); 
	    glTexCoord2f(1,0); glVertex2f(xo+bssize,yo); 
	    glTexCoord2f(1,1); glVertex2f(xo+bssize,yo+btsize); 
	    glEnd();
	}
    }
    glFinish();
#ifdef WIN32 /* XXX */
    grDitherMode(GR_DITHER_DISABLE);
#endif
}

float sbias = 0;
float tbias = 0;

static void
draw_tex(int tsize, int ssize, int xadj, int yadj) {
    float bssize = 1.0 / ssize;
    float btsize = 1.0 / tsize;
    float xoff = (xadj - 0.5) * half * bssize;
    float yoff = (yadj - 0.5) * half * btsize;
    float scale = 1.0 / (texxsize / ssize);

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glTranslatef(sbias, tbias, 0.0);
    glScalef(scale,scale,scale);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,0); glVertex2f(xoff,yoff); 
    glTexCoord2f(0,1); glVertex2f(xoff,yoff+1.0); 
    glTexCoord2f(1,0); glVertex2f(xoff+1.0,yoff); 
    glTexCoord2f(1,1); glVertex2f(xoff+1.0,yoff+1.0); 
    glEnd();
    glFinish();
}

#define NOISE_SLIDES 0
#define BASIS_SLIDES 0

static void
draw_layer(int gridw, int gridh, float accumScale) {
#if NOISE_SLIDES
    accumScale = 1.0;
    glClear(GL_ACCUM_BUFFER_BIT);
#endif    

    glClear(GL_COLOR_BUFFER_BIT);
    glColor4f(1,1,1,1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);
    draw_basis(gridw, gridh, 0, 0);
    if (blend) glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, NOITEX);
    draw_tex(gridw, gridh, 0, 0);
    glAccum(GL_ACCUM, accumScale);

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);
    draw_basis(gridw, gridh, 1, 0);
    if (blend) glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, NOITEX+1);
    draw_tex(gridw, gridh, 1, 0);
    glAccum(GL_ACCUM, accumScale);

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);
    draw_basis(gridw, gridh, 0, 1);
    if (blend) glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
    glBindTexture(GL_TEXTURE_2D, NOITEX+2);
    draw_tex(gridw, gridh, 0, 1);
    glAccum(GL_ACCUM, accumScale);

    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);
    draw_basis(gridw, gridh, 1, 1);
    if (blend) glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ZERO);
#if BASIS_SLIDES
    while (!getkey());
#endif
    glBindTexture(GL_TEXTURE_2D, NOITEX+3);
    draw_tex(gridw, gridh, 1, 1);
#if BASIS_SLIDES    
    while (!getkey());
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_BLEND);    
    draw_tex(gridw, gridh, 1, 1);
    while (!getkey());
#endif
    glAccum(GL_ACCUM, accumScale);

#if NOISE_SLIDES
    glAccum(GL_RETURN, 1.0);
    while (!getkey());
#endif    
    glFinish();
}

static void
draw_return(void) {
    float tw = 0.5, th = 0.5;

    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,0); glVertex2f(0,0); 
    glTexCoord2f(0,1); glVertex2f(0,tw); 
    glTexCoord2f(1,0); glVertex2f(th,0); 
    glTexCoord2f(1,1); glVertex2f(th,tw); 
    glEnd();
}

static void
init(void) {
    int i;
    GLfloat *map;
    float w = 639.0/512.0;
#ifdef WIN32
    float h = w*0.75;
#else
    float h = w;
#endif    
    float xo = -0.0 * w;
    float yo = -0.0 * h;

    glMatrixMode(GL_PROJECTION);
    glOrtho(xo,w,yo,h,-1,1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1,1,1,1);
    glRasterPos2f(0,0);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    init_texture();
#if 0
    map = (GLfloat *) malloc(sizeof(float) * 256);
    for (i=0; i < 256; i++) map[i] = i;
    glPixelMapfv(GL_PIXEL_MAP_R_TO_R, 256, map);
    glPixelMapfv(GL_PIXEL_MAP_G_TO_G, 256, map);
    glPixelMapfv(GL_PIXEL_MAP_B_TO_B, 256, map);
    glPixelMapfv(GL_PIXEL_MAP_A_TO_A, 256, map);
    glPixelTransferi(GL_MAP_COLOR, GL_TRUE);
#endif    
}


int pixxsize = 640;
int pixysize = 480;
unsigned int *buf[4];
unsigned int *oct[10];

static void
do_reflect(unsigned int *buf[]) {
    glAccum(GL_ADD,-0.5);
    glAccum(GL_RETURN,2.0);
    glReadPixels(0, 0, pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE,
		 (GLvoid *) buf[0]);
    glAccum(GL_RETURN,-2.0);
    glReadPixels(0, 0, pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE,
		 (GLvoid *) buf[1]);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glDrawPixels(pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE,
		 (GLvoid *) buf[0]);
    glDrawPixels(pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE, 
		 (GLvoid *) buf[1]);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
}

void
draw_scene(void) {
    int w, h, i, octaves = 7, joctaves = 5;
    float xlen, ylen;
    float wscale = 0.6;
    float weight, sumweight;
    float jsumweight;
    GLenum retfmt = 3;

    weight = 1.0;
    sumweight = 0;
    for (i=0; i < octaves; i++) {
	sumweight += weight;
	weight *= wscale;
    }

#if 1
    buf[1] = (unsigned int *) malloc(4 * pixxsize * pixysize);
    buf[0] = (unsigned int *) malloc(4 * pixxsize * pixysize);

#if 0 /* one abs */
    glClear(GL_ACCUM_BUFFER_BIT);
    w = 2;
    h = 2;
    weight = 1.0;
    for (i=0; i < octaves; i++) {
	draw_layer(w, h, weight / sumweight);
	weight *= wscale;
	w <<= 1;
	h <<= 1;
    }
    do_reflect(buf);
#elif 1 /* no abs */

    weight = 1.0;
    jsumweight = 0;
    for (i=0; i < joctaves; i++) {
	jsumweight += weight;
	weight *= wscale;
    }

    sbias = 0.5;
    tbias = 0.0;
    w = 1;
    h = 1;
    weight = 1.0;
    glClear(GL_ACCUM_BUFFER_BIT);
    for (i=0; i < octaves; i++) {
	draw_layer(w, h, weight / sumweight);
	weight *= wscale;
	w <<= 1;
	h <<= 1;
    }
glDisable(GL_BLEND);
    glAccum(GL_RETURN, 1.0);
    glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buf[0]);
    glBindTexture(GL_TEXTURE_2D, CLOUDTEX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, ifmt, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf[0]);

    sbias = 0.0;
    tbias = 0.0;
    w = 8;
    h = 8;
    weight = 1.0;
    glClear(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);
    for (i=0; i < joctaves; i++) {
	draw_layer(w, h, weight / jsumweight);
	weight *= wscale;
	w <<= 1;
	h <<= 1;
    }
glDisable(GL_BLEND);
    glAccum(GL_RETURN, 1.0);

    glReadPixels(0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, buf[1]);
    glBindTexture(GL_TEXTURE_2D, JITTERTEX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, ifmt, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf[1]);

    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_TEXTURE);
    glLoadIdentity();
    glDisable(GL_BLEND);
    glBegin(GL_TRIANGLE_STRIP);
    glTexCoord2f(0,0); glVertex2f(0,0);
    glTexCoord2f(0,1); glVertex2f(0,1);
    glTexCoord2f(1,0); glVertex2f(1,0);
    glTexCoord2f(1,1); glVertex2f(1,1);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, CLOUDTEX);
    {
	float x, y0, y1;
	float s, t0, t1;
	float ds, dt;
	unsigned int pix0, pix1;
	int vrows = 256;
	int vcols = 256;
	int i, j;
	
	for (j=0; j < (vrows - 1); j++) {
	    t0 = y0 = j / (vrows - 1.0);
	    t1 = y1 = (j + 1) / (vrows - 1.0);
	    glBegin(GL_TRIANGLE_STRIP);
	    for (i=0; i < vcols; i++) {
		s = x = i / (vcols - 1.0);

		pix0 = buf[1][j*256 + i];
		pix1 = buf[1][(j+1)*256 + i];
		ds = ((pix0 & 0x00ff0000) >> 16);
		dt = ((pix0 & 0x0000ff00) >>  8);
		ds = (ds - 127.5) / 127.5;
		dt = (dt - 127.5) / 127.5;
		ds *= 0.02;
		dt *= 0.02;
		glTexCoord2f(s+ds,t0+dt); glVertex2f(x,y0);
		ds = ((pix1 & 0x00ff0000) >> 16);
		dt = ((pix1 & 0x0000ff00) >>  8);
		ds = (ds - 127.5) / 127.5;
		dt = (dt - 127.5) / 127.5;
		ds *= 0.1 ;
		dt *= 0.1 ;
		glTexCoord2f(s+ds,t1+dt); glVertex2f(x,y1);
	    }
	    glEnd();
	}
	glAccum(GL_LOAD,1.0);
	do_reflect(buf);
    }
    
#else /* multiple abs */
    w = 2;
    h = 2;
    for (i=0; i < octaves; i++) {
	glClear(GL_ACCUM_BUFFER_BIT);
	draw_layer(w, h, 1.0);
	w <<= 1;
	h <<= 1;
	do_reflect(buf);
	oct[i] = (unsigned int *) malloc(4 * pixxsize * pixysize);
	glReadPixels(0, 0, pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE, oct[i]);
    }
    glClear(GL_COLOR_BUFFER_BIT|GL_ACCUM_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
    weight = 1.0;
    for (i=0; i < octaves; i++) {
	glDrawPixels(pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE, oct[i]);
	glAccum(GL_ACCUM, weight/sumweight);
	glAccum(GL_ADD,weight/sumweight * -0.5);
	weight *= wscale;
    }
    glAccum(GL_ADD, 0.5);
    glAccum(GL_RETURN, 1.0);
#endif    
    
#elif 0
    draw_layer(2,2,1.0);
    glAccum(GL_LOAD,1.0);
    glAccum(GL_ADD,-0.5);

    glAccum(GL_RETURN,2.0);
#define HOLD_IN_TEXTURE 0    
#if HOLD_IN_TEXTURE
    buf[0] = (unsigned int *) malloc(4 * winxsize * winysize);
    glBindTexture(GL_TEXTURE_2D, RETTEX);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, retfmt, winxsize, winysize, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, buf[0]);
#else
    buf[0] = (unsigned int *) malloc(4 * pixxsize * pixysize);
    glReadPixels(0, 0, pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE, buf[0]);
#endif    
    
    glAccum(GL_RETURN,-2.0);
#if HOLD_IN_TEXTURE
    buf[1] = (unsigned int *) malloc(4 * winxsize * winysize);
    glBindTexture(GL_TEXTURE_2D, RETTEX+1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, retfmt, winxsize, winysize, 0, GL_RGBA,
		 GL_UNSIGNED_BYTE, buf[1]);
#else    
    buf[1] = (unsigned int *) malloc(4 * pixxsize * pixysize);
    glReadPixels(0, 0, pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE, buf[1]);
#endif

#if HOLD_IN_TEXTURE
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glBindTexture(GL_TEXTURE_2D, RETTEX);
    draw_return();
    glBindTexture(GL_TEXTURE_2D, RETTEX+1);
    draw_return();
#else
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE,GL_ONE);
    glDrawPixels(pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE, buf[0]);
    glDrawPixels(pixxsize, pixysize, GL_RGBA, GL_UNSIGNED_BYTE, buf[1]);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
#endif    

#else
    draw_layer(128,128,1.0);
    glAccum(GL_RETURN,1.0);
#endif
    CHKERROR();
}

#ifndef WIN32
static int attributeList[] = { GLX_RGBA, GLX_ACCUM_RED_SIZE, 1, None };
#endif

void main( int argc, char **argv) {

#ifdef WIN32
    init();
    draw_scene();
    while (!getkey());
#else    
    init_window(argv, attributeList);
    init();
    draw_scene();
    while (!getkey());
#endif
    return;
}


#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <conio.h>
#include <assert.h>
#include <windows.h>
#include <glide.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "winutil.h"
#include "clock.h"

enum { TRI, TSTRIP, TFAN, POLY, QUAD };

float period = 1.0;
float trisize = 5.0;
int numtris = 4;
int perspective = 1;

int primitive = TSTRIP;
GLenum polymode = GL_FILL;
char *primitivestr = "tstrip";
char *polymodestr = "fill";
char *commentstr = "";

float transx, transy;

/* per-vertex attributes */
int pvtexture = 1;	/* texture is on by default, unless -a is given */
int pvcolor = 0;
int pvnormal = 0;

/* only for seeing results with minidriver */
int doswap = 0;

unsigned int blue = 0x00ff0000;
unsigned int yellow = 0x0000ffff;

#define TEXSIZE 256
#define NUMCHECKS 4
#define CHECKSIZE (TEXSIZE/NUMCHECKS)

#if 0 /* minidriver doesn't like gluBuild2DMipmaps */
static void
init_tex(void) {
    unsigned int *tex = malloc(TEXSIZE * TEXSIZE * sizeof(unsigned int));
    unsigned int *p = tex;
    int texsize = TEXSIZE;
    int i, j;

    for (j=0; j < texsize; j++) {
	for (i=0; i < texsize; i++) {
	    if (((i/CHECKSIZE) + (j/CHECKSIZE)) % 2) {
		*p++ = blue;
	    } else {
		*p++ = yellow;
	    }
	}
    }
    gluBuild2DMipmaps(GL_TEXTURE_2D, 4, texsize, texsize,
		      GL_RGBA, GL_UNSIGNED_BYTE, tex);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
#else
static void 
init_tex(void) {
    int i, j, w, h, wi, hi, bsize, n;
    unsigned int *iptr, *optr, *tmp, *p;

    w = TEXSIZE;
    h = TEXSIZE;
    bsize = h * w * sizeof(int);
    bsize = (bsize + 7) & ~0x7;
    iptr = (unsigned int *) malloc(bsize);
    optr = (unsigned int *) malloc(bsize);

#define RED    0x000000FF
#define YELLOW 0x0000FFFF
#define BLUE   0x00FF0000
#define GREEN  0x0000FF00
    
    p = iptr;
    for (i=0; i < h; i++) {
	for (j=0; j < w; j++) {
	    if ((i/CHECKSIZE + j/CHECKSIZE) % 2) {
		*p++ = BLUE;
	    } else {
		*p++ = YELLOW;
	    }
	}
    }
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    wi = w;
    hi = h;
    for (n=0; wi || hi; n++) {
	if (!wi) wi = 1;
	if (!hi) hi = 1;
	glTexImage2D(GL_TEXTURE_2D, n, 3, wi, hi, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		     iptr); 
	gluScaleImage(GL_RGBA, wi, hi, GL_UNSIGNED_BYTE, iptr,
		      wi/2, hi/2, GL_UNSIGNED_BYTE, optr);
	tmp = iptr;
	iptr = optr;
	optr = tmp;
	wi >>= 1;
	hi >>= 1;
    }
    free(iptr);
    free(optr);
    glEnable(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		    GL_LINEAR_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
#endif


static void
init(void) {
    if (perspective) {
	glMatrixMode(GL_PROJECTION);
	glFrustum(0, 320, 0, 240, 100, 300);
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(1.0, 1.0, -200.0);
    } else {
	glMatrixMode(GL_PROJECTION);
	glOrtho(0, 640, 0, 480, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glTranslatef(1.0, 1.0, 0.0);
    }
    glTranslatef(transx, transy, 0);
    glClearColor(1, 0, 0, 1);
    glShadeModel(GL_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK, polymode);
    glColor3f(1,1,1); /* minidriver seems to need this */
    if (pvtexture) init_tex();
}

float *pv;
float *pt;
float *pc;
float *pn;

float n0[3] = { 0, 0, -1 };
float n1[3] = { 0, 0,  1 };
float cwhite[3] = { 1, 1, 1 };
float cblack[3] = { 0, 0, 0 };

static void
init_vertex(float *pv, float *pt, float *pc, float *pn,
	    float x, float y, float z,
	    float s, float t,
	    float c[3], float n[3]) {
    pv[0] = x;
    pv[1] = y;
    pv[2] = z;

    pt[0] = s;
    pt[1] = t;

    pc[0] = c[0];
    pc[1] = c[1];
    pc[2] = c[2];
	
    pn[0] = n[0];
    pn[1] = n[1];
    pn[2] = n[2];
}

static void
init_tstrip(void) {
    int i;
    float *v, *t, *c, *n;
    float s, ds, t0, t1;
    float x, dx, y0, y1;
    
    v = pv = (float *) malloc(3 * sizeof(float) * (numtris + 2));
    t = pt = (float *) malloc(2 * sizeof(float) * (numtris + 2));
    c = pc = (float *) malloc(3 * sizeof(float) * (numtris + 2));
    n = pn = (float *) malloc(3 * sizeof(float) * (numtris + 2));

    dx = trisize;
    x = 0;
    y0 = 0;
    y1 = dx;

    /* so that all tris put together will span the texture along s */
    ds = 1.0 / (numtris / 2);
    s = 0;
    t0 = 0;
    t1 = ds;

    for (i=0; i < (numtris / 2) + 1; i++) {
	init_vertex(v, t, c, n, x, y0, 0, s, t0, cwhite, n0);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	init_vertex(v, t, c, n, x, y1, 0, s, t1, cblack, n1);
	v += 3;
	t += 2;
	c += 3;
	n += 3;

	x += dx;
	s += ds;
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_tstrip_t(int iter) {
    int i, j, m;
    float *v, *t=0, *c=0, *n=0;

    for (j=0; j < iter; j++) {

	v = pv;
	if (pvtexture) t = pt;
	if (pvcolor) c = pc;

	glBegin(GL_TRIANGLE_STRIP);
	m = (numtris / 2) + 1;
	for (i=0; i < m; i++) {
	    glTexCoord2f(t[0],t[1]); glVertex3f(v[0],v[1],v[2]);
	    glTexCoord2f(t[2],t[3]); glVertex3f(v[3],v[4],v[5]);
	    v += 6;
	    t += 4;
	    c += 6;
	}
	glEnd();
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_tstrip_c(int iter) {
    int i, j, m;
    float *v, *t=0, *c=0, *n=0;

    for (j=0; j < iter; j++) {

	v = pv;
	if (pvtexture) t = pt;
	if (pvcolor) c = pc;

	glBegin(GL_TRIANGLE_STRIP);
	m = (numtris / 2) + 1;
	for (i=0; i < m; i++) {
	    glColor3f(c[0],c[1],c[2]); glVertex3f(v[0],v[1],v[2]);
	    glColor3f(c[3],c[4],c[5]); glVertex3f(v[3],v[4],v[5]);
	    v += 6;
	    t += 4;
	    c += 6;
	}
	glEnd();
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_tstrip_tc(int iter) {
    int i, j, m;
    float *v, *t=0, *c=0, *n=0;

    for (j=0; j < iter; j++) {

	v = pv;
	if (pvtexture) t = pt;
	if (pvcolor) c = pc;

	glBegin(GL_TRIANGLE_STRIP);
	m = (numtris / 2) + 1;
	for (i=0; i < m; i++) {
	    glTexCoord2f(t[0],t[1]); glColor3f(c[0],c[1],c[2]);
	    glVertex3f(v[0],v[1],v[2]);
	    glTexCoord2f(t[2],t[3]); glColor3f(c[3],c[4],c[5]);
	    glVertex3f(v[3],v[4],v[5]);
	    v += 6;
	    t += 4;
	    c += 6;
	}
	glEnd();
    }
}

static void
init_tfan(int is_polygon) {
    int i, extra;
    float *v, *t, *c, *n;
    float ts, tt;
    float x, dx, x0, y, dy, y0;
    float da, ang, radius, pi = 3.14159;
    float area;
    
    v = pv = (float *) malloc(3 * sizeof(float) * (numtris + 2));
    t = pt = (float *) malloc(2 * sizeof(float) * (numtris + 2));
    c = pc = (float *) malloc(3 * sizeof(float) * (numtris + 2));
    n = pn = (float *) malloc(3 * sizeof(float) * (numtris + 2));

    ang = 0.0;
    if (is_polygon) {
      da = pi * 2.0 / (numtris + 2);
      area = numtris * trisize;
    } else {
      if (numtris > 3) {
	da = pi * 2.0 / numtris;
	area = numtris * trisize;
      } else {
	da = pi * 2.0 / 4.0;
	area = 4 * trisize;
      }
    }
    radius = sqrt(area / pi);
    x0 = radius + 2;
    y0 = radius + 2;

    if (is_polygon) {
      extra = 2;
    } else {
      init_vertex(v, t, c, n, x0, y0, 0, 0.5, 0.5, cwhite, n0);
      v += 3;
      t += 2;
      c += 3;
      n += 3;
      extra = 1;
    } 
    for (i=0; i < numtris + extra; i++) {
	dx = cos(ang) * radius;
	dy = sin(ang) * radius;
	x = x0 + dx;
	y = y0 + dy;
	ts = (x - 2) / (2.0 * radius);
	tt = (y - 2) / (2.0 * radius);

	if (is_polygon && i==0) {
	  init_vertex(v, t, c, n, x, y, 0, ts, tt, cwhite, n0);
	} else {
	  init_vertex(v, t, c, n, x, y, 0, ts, tt, cblack, n0);
	}
	v += 3;
	t += 2;
	c += 3;
	n += 3;

	ang += da;
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_tfan_t(int iter) {
    int i, j;
    float *v, *t=0;
    GLenum prim;

    if (primitive == TFAN) {
      prim = GL_TRIANGLE_FAN;
    } else {
      prim = GL_POLYGON;
    }

    for (j=0; j < iter; j++) {

	v = pv;
	if (pvtexture) t = pt;

	glBegin(prim);
	for (i=0; i < numtris + 2; i++) {
	    glTexCoord2f(t[0],t[1]); glVertex3f(v[0],v[1],v[2]);
	    v += 3;
	    t += 2;
	}
	glEnd();
    }
}

static void
draw_tfan_c(int iter) {
    int i, j;
    float *v, *c=0;
    GLenum prim;

    if (primitive == TFAN) {
      prim = GL_TRIANGLE_FAN;
    } else {
      prim = GL_POLYGON;
    }

    for (j=0; j < iter; j++) {

	v = pv;
	if (pvcolor) c = pc;

	glBegin(prim);
	for (i=0; i < numtris + 2; i++) {
	    glColor3f(c[0],c[1],c[2]); glVertex3f(v[0],v[1],v[2]);
	    v += 3;
	    c += 3;
	}
	glEnd();
    }
}

static void
draw_tfan_tc(int iter) {
    int i, j;
    float *v, *t=0, *c=0;
    GLenum prim;

    if (primitive == TFAN) {
      prim = GL_TRIANGLE_FAN;
    } else {
      prim = GL_POLYGON;
    }

    for (j=0; j < iter; j++) {

	v = pv;
	if (pvtexture) t = pt;
	if (pvcolor) c = pc;

	glBegin(prim);
	for (i=0; i < numtris + 2; i++) {
	    glTexCoord2f(t[0],t[1]); glColor3f(c[0],c[1],c[2]); 
	    glVertex3f(v[0],v[1],v[2]);
	    v += 3;
	    t += 2;
	    c += 3;
	}
	glEnd();
    }
}

static void
init_tri(void) {
    int i;
    float *v, *t, *c, *n;
    float s, ds, t0, t1;
    float x, dx, y0, y1;
    
    v = pv = (float *) malloc(3 * 3 * sizeof(float) * numtris);
    t = pt = (float *) malloc(2 * 3 * sizeof(float) * numtris);
    c = pc = (float *) malloc(3 * 3 * sizeof(float) * numtris);
    n = pn = (float *) malloc(3 * 3 * sizeof(float) * numtris);

    dx = trisize;
    x = 0;
    y0 = 0;
    y1 = dx;

    /* so that all tris put together will span the texture along s */
    ds = 1.0 / (numtris / 2);
    s = 0;
    t0 = 0;
    t1 = ds;

    for (i=0; i < (numtris / 2); i++) {
	/*
	** A---B
	** |  /|
	** | / |
	** |/  |
	** C---D
	**
	** Draw C-B-A, then B-C-D.
	*/
	init_vertex(v, t, c, n, x, y0, 0, s, t0, cwhite, n0);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	init_vertex(v, t, c, n, x+dx, y1, 0, s+ds, t1, cblack, n1);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	init_vertex(v, t, c, n, x, y1, 0, s, t1, cblack, n0);
	v += 3;
	t += 2;
	c += 3;
	n += 3;

	init_vertex(v, t, c, n, x+dx, y1, 0, s+ds, t1, cblack, n1);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	init_vertex(v, t, c, n, x, y0, 0, s, t0, cwhite, n0);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	init_vertex(v, t, c, n, x+dx, y0, 0, s+ds, t0, cwhite, n0);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	
	x += dx;
	s += ds;
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_tri_t(int iter) {
    int i, j;
    float *v, *t=0, *c=0, *n=0;

    for (j=0; j < iter; j++) {

	v = pv;
	if (pvtexture) t = pt;
	if (pvcolor) c = pc;

	glBegin(GL_TRIANGLES);
	for (i=0; i < numtris; i++) {
	    glTexCoord2f(t[0],t[1]); glVertex3f(v[0],v[1],v[2]);
	    glTexCoord2f(t[2],t[3]); glVertex3f(v[3],v[4],v[5]);
	    glTexCoord2f(t[4],t[5]); glVertex3f(v[6],v[7],v[8]);
	    v += 9;
	    t += 6;
	    c += 9;
	}
	glEnd();
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_tri_c(int iter) {
    int i, j;
    float *v, *t=0, *c=0, *n=0;

    for (j=0; j < iter; j++) {

	v = pv;
	if (pvtexture) t = pt;
	if (pvcolor) c = pc;

	glBegin(GL_TRIANGLES);
	for (i=0; i < numtris; i++) {
	    glColor3f(c[0],c[1],c[2]); glVertex3f(v[0],v[1],v[2]);
	    glColor3f(c[3],c[4],c[5]); glVertex3f(v[3],v[4],v[5]);
	    glColor3f(c[6],c[7],c[8]); glVertex3f(v[6],v[7],v[8]);
	    v += 9;
	    t += 6;
	    c += 9;
	}
	glEnd();
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_tri_tc(int iter) {
    int i, j;
    float *v, *t=0, *c=0, *n=0;

    for (j=0; j < iter; j++) {

	v = pv;
	if (pvtexture) t = pt;
	if (pvcolor) c = pc;

	glBegin(GL_TRIANGLES);
	for (i=0; i < numtris; i++) {
	    glTexCoord2f(t[0],t[1]); glColor3f(c[0],c[1],c[2]);
	    glVertex3f(v[0],v[1],v[2]);
	    glTexCoord2f(t[2],t[3]); glColor3f(c[3],c[4],c[5]);
	    glVertex3f(v[3],v[4],v[5]);
	    glTexCoord2f(t[4],t[5]); glColor3f(c[6],c[7],c[8]);
	    glVertex3f(v[6],v[7],v[8]);
	    v += 9;
	    t += 6;
	    c += 9;
	}
	glEnd();
    }
}

static void
init_quad(void) {
    int i;
    float *v, *t, *c, *n;
    float s, ds, t0, t1;
    float x, dx, y0, y1;

    if (numtris % 2) {
      printf("invalid numtris: %d\n", numtris);
    }
    v = pv = (float *) malloc(3 * 3 * sizeof(float) * numtris * 2);
    t = pt = (float *) malloc(2 * 3 * sizeof(float) * numtris * 2);
    c = pc = (float *) malloc(3 * 3 * sizeof(float) * numtris * 2);
    n = pn = (float *) malloc(3 * 3 * sizeof(float) * numtris * 2);

    dx = trisize;
    x = 0;
    y0 = 0;
    y1 = dx;

    /* so that all tris put together will span the texture along s */
    ds = 1.0 / (numtris / 2);
    s = 0;
    t0 = 0;
    t1 = ds;

    for (i=0; i < (numtris / 2); i++) {
	/*
	** B---C
	** |   |
	** |   |
	** |   |
	** A---D
	**
	** Draw A-B-C-D.
	*/
	init_vertex(v, t, c, n, x, y0, 0, s, t0, cwhite, n0);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	init_vertex(v, t, c, n, x, y1, 0, s, t1, cblack, n1);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	init_vertex(v, t, c, n, x+dx, y1, 0, s+ds, t1, cblack, n0);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	init_vertex(v, t, c, n, x+dx, y0, 0, s+ds, t0, cwhite, n0);
	v += 3;
	t += 2;
	c += 3;
	n += 3;
	
	x += dx;
	s += ds;
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_quad_t(int iter) {
    int i, j;
    float *v, *t;

    for (j=0; j < iter; j++) {
	v = pv;
	t = pt;
	glBegin(GL_QUADS);
	for (i=0; i < numtris/2; i++) {
	    glTexCoord2f(t[0],t[1]); glVertex3f(v[0],v[1],v[2]);
	    glTexCoord2f(t[2],t[3]); glVertex3f(v[3],v[4],v[5]);
	    glTexCoord2f(t[4],t[5]); glVertex3f(v[6],v[7],v[8]);
	    glTexCoord2f(t[6],t[7]); glVertex3f(v[9],v[10],v[11]);
	    v += 12;
	    t += 8;
	}
	glEnd();
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_quad_c(int iter) {
    int i, j;
    float *v, *c;

    for (j=0; j < iter; j++) {
	v = pv;
	c = pc;
	glBegin(GL_QUADS);
	for (i=0; i < numtris/2; i++) {
	    glColor3f(c[0],c[1],c[2]); glVertex3f(v[0],v[1],v[2]);
	    glColor3f(c[3],c[4],c[5]); glVertex3f(v[3],v[4],v[5]);
	    glColor3f(c[6],c[7],c[8]); glVertex3f(v[6],v[7],v[8]);
	    glColor3f(c[9],c[10],c[11]); glVertex3f(v[9],v[10],v[11]);
	    v += 12;
	    c += 12;
	}
	glEnd();
    }
}

/*
** Careful!  If you change anything in a draw routine, be sure
** to apply it to all 3 variants.
*/ 
static void
draw_quad_tc(int iter) {
    int i, j;
    float *v, *t, *c;

    for (j=0; j < iter; j++) {
	v = pv;
	t = pt;
	c = pc;
	glBegin(GL_QUADS);
	for (i=0; i < numtris/2; i++) {
	    glTexCoord2f(t[0],t[1]); glColor3f(c[0],c[1],c[2]); glVertex3f(v[0],v[1],v[2]);
	    glTexCoord2f(t[2],t[3]); glColor3f(c[3],c[4],c[5]); glVertex3f(v[3],v[4],v[5]);
	    glTexCoord2f(t[4],t[5]); glColor3f(c[6],c[7],c[8]); glVertex3f(v[6],v[7],v[8]);
	    glTexCoord2f(t[6],t[7]); glColor3f(c[9],c[10],c[11]); glVertex3f(v[9],v[10],v[11]);
	    v += 12;
	    t += 8;
	    c += 12;
	}
	glEnd();
    }
}

static void
run_bench(void) {
    int iter = 1000;
    int ntris;
    void (*draw_func)(int iter);
    float elapsed = 0;
    float trispersec, secspertri;

    switch (primitive) {
    case TSTRIP:
	init_tstrip();
	if (pvtexture) {
	    if (pvcolor) {
		draw_func = draw_tstrip_tc;
	    } else {
		draw_func = draw_tstrip_t;
	    }
	} else {
	    draw_func = draw_tstrip_c;
	}
	break;
    case TRI:
	init_tri();
	if (pvtexture) {
	    if (pvcolor) {
		draw_func = draw_tri_tc;
	    } else {
		draw_func = draw_tri_t;
	    }
	} else {
	    draw_func = draw_tri_c;
	}
	break;
    case TFAN:
	init_tfan(GL_FALSE);
	if (pvtexture) {
	    if (pvcolor) {
		draw_func = draw_tfan_tc;
	    } else {
		draw_func = draw_tfan_t;
	    }
	} else {
	    draw_func = draw_tfan_c;
	}
	break;
    case POLY:
      /* polygons used the same routines as tfans */
	init_tfan(GL_TRUE);
	if (pvtexture) {
	    if (pvcolor) {
		draw_func = draw_tfan_tc;
	    } else {
		draw_func = draw_tfan_t;
	    }
	} else {
	    draw_func = draw_tfan_c;
	}
	break;
    case QUAD:
	init_quad();
	if (pvtexture) {
	    if (pvcolor) {
		draw_func = draw_quad_tc;
	    } else {
		draw_func = draw_quad_t;
	    }
	} else {
	    draw_func = draw_quad_c;
	}
	break;
    }
    /* find a number of reps to match the desired period */
    while (elapsed < period) {
	iter *= 2;
	startclock();
	(*draw_func)(iter);
	glFinish();
	if (doswap) swap(); 
	elapsed = stopclock();
    }

    startclock();
    (*draw_func)(iter);
    glFinish();
    elapsed = stopclock();

    ntris = iter * numtris;
    trispersec = ntris / elapsed;
    secspertri = elapsed / ntris * 1000000.0;
    printf("%-4s %5.2f %-6s %2d size:%3.0f %4s pv:%c%c %4.2fs t/s: %7.0f \n",
	   commentstr, secspertri, 
	   primitivestr, numtris, trisize, polymodestr, 
	   pvtexture ? 't' : ' ', pvcolor ? 'c' : ' ', elapsed, 
	   /* iter, ntris, */
	   trispersec, commentstr);
}

void usage(void) {
    printf("usage: trispeed [-t numtris][-p period][-a [c][n][t]][-q]\n");
    exit(1);
}

void main( int argc, char **argv) {
    int i, n;

    for (i=1; i < argc; i++) {
	if (argv[i][0] == '-') {
	    switch (argv[i][1]) {
	    case 'a':
		if (++i >= argc) usage();
		pvtexture = 0;
		for (n=0; argv[i][n] != (char)NULL; n++) {
		    switch (argv[i][n]) {
		    case 'c': 
			pvcolor = 1;
			break;
		    case 'n':
			pvnormal = 1;
			break;
		    case 't':
			pvtexture = 1;
			break;
		    default:
			usage();
		    }
		}
		break;
	    case 't':
		if (++i >= argc) usage();
		numtris = atoi(argv[i]);
		break;
	    case 'p':
		if (++i >= argc) usage();
		period = atof(argv[i]);
		break;
	    case 's':
		if (++i >= argc) usage();
		trisize = atof(argv[i]);
		break;
	    case 'P':
		if (++i >= argc) usage();
		primitivestr = argv[i];
		if (!strcmp(argv[i], "tstrip")) {
		    primitive = TSTRIP;
		} else if (!strcmp(argv[i], "tri")) {
		    primitive = TRI;
		} else if (!strcmp(argv[i], "tfan")) {
		    primitive = TFAN;
		} else if (!strcmp(argv[i], "poly")) {
		    primitive = POLY;
		} else if (!strcmp(argv[i], "quad")) {
		    primitive = QUAD;
		} else {
		    usage();
		}
		break;
	    case 'm':
		if (++i >= argc) usage();
		polymodestr = argv[i];
		if (!strcmp(argv[i], "line")) {
		    polymode = GL_LINE;
		} else if (!strcmp(argv[i], "fill")) {
		    polymode = GL_FILL;
		} else if (!strcmp(argv[i], "point")) {
		    polymode = GL_POINT;
		} else {
		    usage();
		}
		break;
	    case 'c':
		if (++i >= argc) usage();
		commentstr = argv[i];
		break;
	    case 'x':
		if (++i >= argc-1) usage();
		transx = atof(argv[i++]); 
		transy = atof(argv[i++]);
		break;
	    case 'r':
	        perspective = 1;
		break;
	    case 'q':
	        doswap = 1;
		break;
	    default:
		usage();
	    }
	}
    }
    
    initApplication(GetModuleHandle(NULL), 0, doswap);

    init();
    glClear(GL_COLOR_BUFFER_BIT);
    run_bench();
    CHKERROR();
}


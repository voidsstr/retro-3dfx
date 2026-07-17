/**************************************************************************
 *									  *
 * 		 Copyright (C) 1989, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

/*
 * This program tests smooth shading and flat shading
 * 
 */

#include "ogtst.h"	/* include test environment		*/

static void tri_strip(  GLint, GLint, GLint, GLint, int);
static void tri_fan(    GLint, GLint, GLint, GLint, int);
static void tri_ind(    GLint, GLint, GLint, GLint, int);
static void quad_strip( GLint, GLint, GLint, GLint, int);
static void quad_ind(   GLint, GLint, GLint, GLint, int);
static void poly_single(GLint, GLint, GLint, GLint, int);
static void SetColor(GLint);
static void SmoothColorCheck(int x,int y,int ymax,
                             unsigned int col1,unsigned int col2);
static void slopes(unsigned int col1, unsigned int col2,
		unsigned int x1, unsigned int x2,float first[4], float dc[4]);
static void lowhi(float first[4], float dc[4], int o,
		unsigned int *locol, unsigned int *hicol);
static int ciBits, maxCiVal;
static GLboolean multiSampled;
static int r, g, b, a; /* number of bits in each color component */

static int
BitsToColor(int bits)
{

    if (bits == 0) {
        return 0;
    } else {
        if (bits > 8) {
            bits = 8;
        }
        return (int)((255.0/(float)((1<<bits)-1))*(float)ogLibIntRand(0,(1<<bits)-1)+0.5);
    }
}

static GLuint ogLibColor2(void) {
    GLuint col;
    int r, g, b, a, ci, rBits, gBits, bBits, aBits, ciBits;

    ogEnvColorBits(&rBits, &gBits, &bBits, &aBits, &ciBits);
    if (ogEnvIsCIMode()) {
        while (!(ci = ogLibBitRand(ciBits)));   /* don't use zero */
        glIndexi(ci);
        ogEnvLog(OG_LPARAMETERS,"glIndexi(0x%lx);\n",ci);
        col = ci;
    } else {
        r = BitsToColor(rBits) & 0x7F;
        g = BitsToColor(gBits) & 0x7F;
        b = BitsToColor(bBits) & 0x7F;
        a = BitsToColor(aBits) & 0x7F;
        glColor4ub((GLubyte)r, (GLubyte)g, (GLubyte)b, (GLubyte)a);
        ogEnvLog(OG_LPARAMETERS,"glColor4ub(%02x,%02x,%02x,%02x)\n",r,g,b,a);
        col = ((r << 24) & 0xFF000000) | ((g << 16) & 0x00FF0000) |
              ((b << 8) & 0x0000FF00) | (a & 0x000000FF);
    }
    return (col);
}

GLuint (*getColor)(void);

static int obj;
TESTMOD(shade)
{
    GLint x1,y1;
    int mode;
    GLint xmax, ymax;
    int   maxcolor;

    ogEnvColorBits(&r, &g, &b, &a, &ciBits);
    getColor = ogLibColor;
    if (a > 8)
        a = 8;

    maxCiVal = (1 << ciBits) - 1;
    multiSampled = ogEnvIsMultiSampled();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.,(double) ogEnvQuery(OG_XWSIZE), 0.,
            (double) ogEnvQuery(OG_YWSIZE), -1.,1.);
    glMatrixMode(GL_MODELVIEW);
    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    while (pass--) {
	glShadeModel(GL_FLAT);
        ogLibClear(0) ;

	x1 = ogLibIntRand(2,xmax - 2);
	y1 = ogLibIntRand(2,ymax - 2);

	ogEnvLog(1, "---vertex alone test---\n");
	START_DL_OR_IM(1);

	maxcolor = getColor();
	ogLibDrawFragments(x1, y1);

	FINIS_DL_OR_IM(1);

        ogLibRectCheck(x1,y1,x1,y1,maxcolor,0) ;

        ogLibClear(0) ;
#if 0
        maxcolor = getColor() ;

	x1 = ogLibIntRand(2,xmax - 2);
	y1 = ogLibIntRand(2,ymax - 2);
	x2 = ogLibIntRand(2,xmax - 2);
	y2 = ogLibIntRand(2,ymax - 2);

	ogEnvLog(1, "---line test---\n");
	START_DL_OR_IM(1);

	ogEnvLog(1, "glBegin(GL_LINES);  \n");
	glBegin(GL_LINES);	 
	ogLibSetVertex(x1,y1,0,1);
	maxcolor = getColor();
	ogLibSetVertex(x2,y1,0,1);
	glEnd();
	ogEnvLog(1, "glEnd(); \n");
	FINIS_DL_OR_IM(1);

	if (x2 > x1)
          ogLibRectCheck(x1,y1,x2-1,y1,maxcolor,0) ;
	else if (x1 > x2)
          ogLibRectCheck(x2+1,y1,x1,y1,maxcolor,0) ;
	else 
          ogLibRectCheck(x1,y1,x2,y1,0,0) ;

#endif 
	if (ogLibBitRand(1)) {
	    ogEnvLog(1, "glShadeModel(GL_FLAT);\n");
	    glShadeModel(GL_FLAT);
	    mode = GL_FLAT;
	} else {
	    ogEnvLog(1, "glShadeModel(GL_SMOOTH);\n");
	    glShadeModel(GL_SMOOTH);
	    mode = GL_SMOOTH;
	}
        ogLibClear(0) ;

	x1 = ogLibIntRand(2,xmax - 10);
	y1 = ogLibIntRand(2,ymax - 10);

	START_DL_OR_IM(1);
	switch(ogLibIntRand(0, 5)) {
	    case 0:
	    	poly_single(x1,y1,xmax-2,ymax-2,mode);
		break;
	    case 1:
		tri_strip(x1,y1,xmax-2,ymax-2,mode);
		break;
	    case 2:
		tri_fan(x1,y1,xmax-2,ymax-2,mode);
		break;
	    case 3:
		tri_ind(x1,y1,xmax-2,ymax-2,mode);
		break;
	    case 4:
		quad_strip(x1,y1,xmax-2,ymax-2,mode);
		break;
	    case 5:
		quad_ind(x1,y1,xmax-2,ymax-2,mode);
		break;
	    default:
		;
	}
    }
}

CLEANUP(shade)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glShadeModel(GL_SMOOTH);
    ogLibSetDefaultColors();
}

static void
SmoothColorCheck(int x, int y, int ymax, unsigned int col1,unsigned int col2)
{
    GLuint buf[640];
    int i, high;
    float first[4],dc[4];
    unsigned int locol,hicol;

    ogLibReadPixels(x, y, x, ymax, buf);
    slopes(col1, col2, y, ymax-1, first, dc);
    if (multiSampled) {
        i = 1;
        high = ymax - y - 1;
    } else {
        i = 0;
        high = ymax - y;
    }
    for (; i< high; i++) {
	lowhi(first,dc,i,&locol,&hicol);
	ogLibRangeCheck(buf[i],locol,hicol);
    }
}

static void
SetColor(GLint col)
{
    int r,g,b,a;

    if (ciBits) {
	glIndexi(col);
	ogEnvLog(OG_LPARAMETERS, "glIndexi(0x%lx);\n",col);
    } else {
	r = (col>>24) & 0xff;
	g = (col>>16) & 0xff;
	b = (col>> 8) & 0xff;
	a = (col>> 0) & 0xff;
	glColor4ub(r, g, b, a);
	ogEnvLog(OG_LPARAMETERS, "glColor4ub(%02x,%02x,%02x,%02x)\n",
                 r, g, b, a);
    }
}

static void
slopes(unsigned int col1, unsigned int col2, 
		unsigned int x1, unsigned int x2,float first[4], float dc[4])
{
    float r1,g1,b1,a1;
    float r2,g2,b2,a2;

    if (ciBits) {
	dc[0] = ((int) col2 - (int) col1)/(float) (x2-x1+1);
	first[0] = (int) col1 + 0.5 * dc[0];
    } else {
	r1 = (float)((col1 >> 24) & 0xff)/255.0;
	g1 = (float)((col1 >> 16) & 0xff)/255.0;
	b1 = (float)((col1 >>  8) & 0xff)/255.0;
        a1 = (float)((col1 >>  0) & 0xff)/255.0;
	r2 = (float)((col2 >> 24) & 0xff)/255.0;
	g2 = (float)((col2 >> 16) & 0xff)/255.0;
	b2 = (float)((col2 >>  8) & 0xff)/255.0;
        a2 = ((col2 >>  0) & 0xff)/255.0;
	dc[0] = (r2 - r1)/(float) (x2-x1+1);
	first[0] = r1 + 0.5 * dc[0];
	dc[1] = (g2 - g1)/(float) (x2-x1+1);
	first[1] = g1 + 0.5 * dc[1];
	dc[2] = (b2 - b1)/(float) (x2-x1+1);
	first[2] = b1 + 0.5 * dc[2];
	dc[3] = (a2 - a1)/(float) (x2-x1+1);
	first[3] = a1 + 0.5 * dc[3];
    }
}

static void
lowhi(float first[4], float dc[4], int o, 
      unsigned int *locol, unsigned int *hicol)
{
    float lc[4], hc[4], tmp;
    int l,h; 
    int i;

    if (ciBits) {
	l = first[0] + (o-1) * dc[0];
	h = first[0] + (o+1) * dc[0];
	if (l > h) {
	    i = l;
	    l = h;
	    h = i;
	}
	*locol = l < 0 ? 0 : (l > maxCiVal ? maxCiVal : l);
	*hicol = h < 0 ? 0 : (h > maxCiVal ? maxCiVal : h);
    } else {
        for (i = 0; i < 4; i++) {
            lc[i] = first[i] + (o-1) * dc[i];
            hc[i] = lc[i] + 2*dc[i];
            if (lc[i] < 0)
                lc[i] = 0;
            else if (lc[i] > 1)
                lc[i] = 1;
            if (hc[i] < 0)
                hc[i] = 0;
            else if (hc[i] > 1)
                hc[i] = 1;
	    if (lc[i] > hc[i]) {
		tmp = lc[i];
		lc[i] = hc[i];
		hc[i] = tmp;
	    }
	}

	*locol = (((int)(lc[0]*255))<<24) | (((int)(lc[1]*255))<<16) |
            (((int)(lc[2]*255))<<8) | (int)(lc[3]*((1<<a)-1));

	*hicol = (((int)(hc[0]*255 + 0.5))<<24) | 
            (((int)(hc[1]*255 + 0.5))<<16) | (((int)(hc[2]*255 + 0.5))<<8) |
            (int)(hc[3]*((1<<a)-1)+0.5);
    }
}

static void
poly_single(GLint x, GLint y, GLint xmax, GLint ymax, int mode)
{
    int color[2];

    ogEnvLog(1,"single polygon: (%d,%d)\n", x,y);
    glBegin(GL_POLYGON);
	color[0] = getColor();
	ogLibSetVertex(x, y, 0, 1);
	ogLibSetVertex(xmax, y, 0, 1);
	color[1] = getColor();
	ogLibSetVertex(xmax, ymax, 0, 1);
	ogLibSetVertex(x, ymax, 0, 1);
    glEnd();
    FINIS_DL_OR_IM(1);
    if (mode == GL_FLAT)
	ogLibRectCheck(x,y,xmax-1,ymax-1,color[0],0);
    else 
	SmoothColorCheck((x+xmax)/2,y,ymax,color[0],color[1]);
}

static void
tri_strip(GLint x, GLint y, GLint xmax, GLint ymax, int mode)
{
    GLint xsize;
    GLint color[6];
    int i;

    xsize = (xmax-x)/3;

    ogEnvLog(1,"triangle strip: (%d,%d)\n", x, y);
    glBegin(GL_TRIANGLE_STRIP);
        color[0]=getColor();
        ogLibSetVertex(x,y,0,1); /* v0 */
	color[1]=getColor();
        ogLibSetVertex(x,ymax,0,1); /* v1 */
	SetColor(color[0]);
        ogLibSetVertex(x+xsize,y,0,1); /* v2 */ 
	SetColor(color[1]);
        ogLibSetVertex(x+xsize,ymax,0,1); /* v3 */
	SetColor(color[0]);
    	ogLibSetVertex(x+2*xsize,y,0,1); /* v4 */
	SetColor(color[1]);
        ogLibSetVertex(x+2*xsize,ymax,0,1); /* v5 */
	SetColor(color[0]);
    	ogLibSetVertex(xmax,y,0,1); /* v6 */
	SetColor(color[1]);
    	ogLibSetVertex(xmax,ymax,0,1); /* v7 */
    glEnd();
    FINIS_DL_OR_IM(1);
    if (mode == GL_FLAT) {
    	for(i=0; i<3; i++) {
       	    ogLibPixelCheck(x+i*xsize+1, y+1, color[0]);
       	    ogLibPixelCheck(x+(i+1)*xsize-1, ymax-1, color[1]);
    	}
    } else {
    	SmoothColorCheck((x+xmax)/2,y,ymax,color[0],color[1]);
    }
}

static void
tri_fan(GLint x, GLint y, GLint xmax, GLint ymax, int mode)
{
    int color[2];
    int xsize;

    ogEnvLog(1,"triangle fan: (%d,%d)\n", x, y);
    xsize = (xmax-x)/3;

    glBegin(GL_TRIANGLE_FAN);
        color[0]=getColor();
        ogLibSetVertex(x,y,0,1);
      	ogLibSetVertex(xmax,y,0,1);
    	color[1]=getColor();
    	ogLibSetVertex(xmax,ymax,0,1);
    	ogLibSetVertex(x+2*xsize,ymax,0,1);
    	ogLibSetVertex(x+xsize,ymax,0,1);
	ogLibSetVertex(x,ymax,0,1);
    glEnd();
    FINIS_DL_OR_IM(1);
    if (mode == GL_FLAT) {
	ogLibRectCheck(x,y,xmax-1,ymax-1,color[1],0);
    } else {
	SmoothColorCheck((x+xmax)/2,y,ymax,color[0],color[1]);
    }
}

static void
tri_ind(GLint x, GLint y, GLint xmax, GLint ymax, int mode)
{

    int color[2];

    ogEnvLog(1,"independent triangle: (%d,%d)\n", x, y);
    glBegin(GL_TRIANGLES);
     	color[0]=getColor();
     	ogLibSetVertex(x,y,0,1);
     	ogLibSetVertex(xmax,y,0,1);
     	color[1]=getColor();
     	ogLibSetVertex(xmax,ymax,0,1);
    glEnd();
    glBegin(GL_TRIANGLES);
     	SetColor(color[0]);
     	ogLibSetVertex(x,y,0,1);
     	SetColor(color[1]);
     	ogLibSetVertex(xmax,ymax,0,1);
     	ogLibSetVertex(x,ymax,0,1);
    glEnd();
    FINIS_DL_OR_IM(1);
    if (mode == GL_FLAT) {
	ogLibRectCheck(x,y,xmax-1,ymax-1,color[1],0);
    } else {
	SmoothColorCheck((x+xmax)/2,y,ymax,color[0],color[1]);
    }
}

static void
quad_strip(GLint x, GLint y, GLint xmax, GLint ymax, int mode)
{
    int color[2];
    int xsize;

    ogEnvLog(1,"quad strip: (%d,%d)\n", x, y);
    xsize = (xmax-x)/3;

    glBegin(GL_QUAD_STRIP);
    	color[0]=getColor();
    	ogLibSetVertex(x,y,0,1); /* v0 */
    	color[1]=getColor();
    	ogLibSetVertex(x,ymax,0,1); /* v1 */
    	SetColor(color[0]);
    	ogLibSetVertex(x+xsize,y,0,1); /* v2 */
    	SetColor(color[1]);
    	ogLibSetVertex(x+xsize,ymax,0,1); /* v3 */
    	SetColor(color[0]);
    	ogLibSetVertex(x+2*xsize,y,0,1); /* v4 */
    	SetColor(color[1]);
    	ogLibSetVertex(x+2*xsize,ymax,0,1); /* v5 */
    	SetColor(color[0]);
    	ogLibSetVertex(xmax,y,0,1); /* v6 */
    	SetColor(color[1]);
    	ogLibSetVertex(xmax,ymax,0,1); /* v7 */
    glEnd();
    FINIS_DL_OR_IM(1);
    if (mode == GL_FLAT) {
	ogLibRectCheck(x,y,xmax-1,ymax-1,color[1],0);
    } else {
	SmoothColorCheck((x+xmax)/2,y,ymax,color[0],color[1]);
    }

}

static void
quad_ind(GLint x, GLint y, GLint xmax, GLint ymax, int mode)
{
    int color[2];

    ogEnvLog(1,"independent quad: (%d,%d)\n", x, y);
    glBegin(GL_QUADS);
	color[0] = getColor();
	ogLibSetVertex(x, y, 0, 1);
	ogLibSetVertex(xmax, y, 0, 1);
	color[1] = getColor();
	ogLibSetVertex(xmax, ymax, 0, 1);
	ogLibSetVertex(x, ymax, 0, 1);
    glEnd();
    FINIS_DL_OR_IM(1);
    if (mode == GL_FLAT) {
	ogLibRectCheck(x,y,xmax-1,ymax-1,color[1],0);
    } else {
	SmoothColorCheck((x+xmax)/2,y,ymax,color[0],color[1]);
    }
}

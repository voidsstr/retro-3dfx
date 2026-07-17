/**************************************************************************
 *									  *
 * 		 Copyright (C) 1994, Silicon Graphics, Inc.		  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/


/*
 *  abgr.c - $Revision: 2$
 *
 *  test the abgr support for texturing.
 *  pixel support for abgr is (should be) tested in the pixel test.
 *
 *  TODO:
 *	- the 1,2 component cases for MODULATE,BLEND were left out
 *
 */

#include "math.h"
#include "ogtst.h"

static GLuint pack4ub(GLubyte r, GLubyte g, GLubyte b, GLubyte a)
{
    return (r << 24) | (g << 16) | (b << 8) | a;
}

static void unpack4ub(GLuint rgba, GLubyte* r, GLubyte* g, GLubyte* b, GLubyte* a)
{
    *r = (rgba >> 24) & 0xff;
    *g = (rgba >> 16) & 0xff;
    *b = (rgba >>  8) & 0xff;
    *a = (rgba >>  0) & 0xff;
}


TESTMOD(abgr)
{
    GLint xmax, ymax;
    GLint x,y;
    GLint readx,ready;
    GLuint rgbaColor, abgrColor, newrgba;
    GLubyte r,g,b,a;
    GLubyte nr,ng,nb,na;
    GLubyte aa;
    GLuint image[4*4*4]; 	/* big enough for 4 x 4 x 4 x GL_UNSIGNED_INT */
    GLenum intlfmt, hosttype, envmode;
    GLubyte *ip;
    int tolerance = 1;
    int i,j;
    int tnum, hwtype;

    SUPPORTED_EXTENSION("GL_EXT_abgr");

    hwtype = ogEnvQuery(OG_HW);
    xmax = ogEnvQuery(OG_XWSIZE);
    ymax = ogEnvQuery(OG_YWSIZE);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, xmax, 0, ymax, -1, 1);

    ogLibClear(0);
    glEnable(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    while (pass--) {

	/* 6 tests x 7 hosttypes, hence 42 */
	for (i = 0; i < 42; i++) {

	    /* generate color and force a non-zero alpha (ogLibColor foolishly zeroes it) */
	    rgbaColor = ogLibColor();
	    aa = ogLibBitRand(8) & 0xff;
	    rgbaColor = (rgbaColor & ~0xff) | aa;

	    glColor4f(1,1,1,1); /* restore the color that ogLibColor quietly sets */

	    /* if texture alpha is 0xff change it to differentiate it from A(fragment) */
	    if ((rgbaColor & 0xff) == 0xff)
		rgbaColor = (rgbaColor & ~0xff) | 0x7f;

	    unpack4ub(rgbaColor, &r, &g, &b, &a);

	    abgrColor = pack4ub(a, b, g, r);

	    ip = (GLubyte *)image;
	    for (j = 0; j < 4*4; j++) {
		switch (i / 6) {
		  case 0:
		    hosttype = GL_UNSIGNED_BYTE;
		    *((GLuint *)ip) = abgrColor; ip += sizeof(GLuint);
		    break;
		  case 1:
		    hosttype = GL_BYTE;
		    *((GLbyte *)ip) = OGTST_F_TO_B(OGTST_UB_TO_F(a)); ip += sizeof(GLbyte);
		    *((GLbyte *)ip) = OGTST_F_TO_B(OGTST_UB_TO_F(b)); ip += sizeof(GLbyte);
		    *((GLbyte *)ip) = OGTST_F_TO_B(OGTST_UB_TO_F(g)); ip += sizeof(GLbyte);
		    *((GLbyte *)ip) = OGTST_F_TO_B(OGTST_UB_TO_F(r)); ip += sizeof(GLbyte);
		    break;
		  case 2:
		    hosttype = GL_UNSIGNED_SHORT;
		    *((GLushort *)ip) = OGTST_F_TO_US(OGTST_UB_TO_F(a)); ip += sizeof(GLushort);
		    *((GLushort *)ip) = OGTST_F_TO_US(OGTST_UB_TO_F(b)); ip += sizeof(GLushort);
		    *((GLushort *)ip) = OGTST_F_TO_US(OGTST_UB_TO_F(g)); ip += sizeof(GLushort);
		    *((GLushort *)ip) = OGTST_F_TO_US(OGTST_UB_TO_F(r)); ip += sizeof(GLushort);
		    break;
		  case 3:
		    hosttype = GL_SHORT;
		    *((GLshort *)ip) = OGTST_F_TO_S(OGTST_UB_TO_F(a)); ip += sizeof(GLshort);
		    *((GLshort *)ip) = OGTST_F_TO_S(OGTST_UB_TO_F(b)); ip += sizeof(GLshort);
		    *((GLshort *)ip) = OGTST_F_TO_S(OGTST_UB_TO_F(g)); ip += sizeof(GLshort);
		    *((GLshort *)ip) = OGTST_F_TO_S(OGTST_UB_TO_F(r)); ip += sizeof(GLshort);
		    break;
		  case 4:
		    hosttype = GL_UNSIGNED_INT;
		    *((GLuint *)ip) = OGTST_F_TO_UI(OGTST_UB_TO_F(a)); ip += sizeof(GLuint);
		    *((GLuint *)ip) = OGTST_F_TO_UI(OGTST_UB_TO_F(b)); ip += sizeof(GLuint);
		    *((GLuint *)ip) = OGTST_F_TO_UI(OGTST_UB_TO_F(g)); ip += sizeof(GLuint);
		    *((GLuint *)ip) = OGTST_F_TO_UI(OGTST_UB_TO_F(r)); ip += sizeof(GLuint);
		    break;
		  case 5:
		    hosttype = GL_INT;
		    *((GLint *)ip) = OGTST_F_TO_I(OGTST_UB_TO_F(a)); ip += sizeof(GLint);
		    *((GLint *)ip) = OGTST_F_TO_I(OGTST_UB_TO_F(b)); ip += sizeof(GLint);
		    *((GLint *)ip) = OGTST_F_TO_I(OGTST_UB_TO_F(g)); ip += sizeof(GLint);
		    *((GLint *)ip) = OGTST_F_TO_I(OGTST_UB_TO_F(r)); ip += sizeof(GLint);
		    break;
		  case 6:
		    hosttype = GL_FLOAT;
		    *((GLfloat *)ip) = OGTST_UB_TO_F(a); ip += sizeof(GLfloat);
		    *((GLfloat *)ip) = OGTST_UB_TO_F(b); ip += sizeof(GLfloat);
		    *((GLfloat *)ip) = OGTST_UB_TO_F(g); ip += sizeof(GLfloat);
		    *((GLfloat *)ip) = OGTST_UB_TO_F(r); ip += sizeof(GLfloat);
		    break;		    
		}
	    }
	    /*
	     *	these cases choose the texture internal format (components) and
	     *	the texture environment mode.
	     *	the appropriate color of the final textured pixel is predicted here.
	     *
	     *	XXX a few cases were left out, namely the 1,2 component cases.
	     *	to add them, we need to account for the current tex env color.
	     */
	    tnum = i % 6;
	    switch (tnum) {
		case 0:
		    intlfmt = GL_RGB;  envmode = GL_MODULATE;
		    nr = r;  ng = g;  nb = b;
		    na = 0xff;
		    break;
		case 1:
		    intlfmt = GL_RGB;  envmode = GL_DECAL;
		    nr = r;  ng = g;  nb = b;
		    na = 0xff;
		    break;
		case 2:
		    intlfmt = GL_RGBA;  envmode = GL_MODULATE;
		    nr = r;  ng = g;  nb = b;
		    na = a;
		    break;
		case 3:
		    intlfmt = GL_RGBA;  envmode = GL_DECAL;
		    nr = (0xff - a) + (a*r/0xff);
		    ng = (0xff - a) + (a*g/0xff);
		    nb = (0xff - a) + (a*b/0xff);
		    na = 0xff;
		    break;
		case 4:
		    intlfmt = GL_LUMINANCE; envmode = GL_MODULATE;
		    nr = r; ng = r; nb = r;
		    na = 0xff;
		    break;
		case 5:
		    intlfmt = GL_LUMINANCE_ALPHA; envmode = GL_MODULATE;
		    nr = r; ng = r; nb = r;
		    na = a;
		    break;
	    }

	    newrgba = pack4ub(nr,ng,nb,na);

	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, envmode);

#ifdef GL_EXT_abgr
	    glTexImage2D(GL_TEXTURE_2D, 0, intlfmt, 4, 4, 0, GL_ABGR_EXT, hosttype, image);
#endif 

	    ogEnvLog(1, "intlfmt 0x%x, texenvmode 0x%x\n", intlfmt, envmode);

	    x = 2 + ogLibIntRand(0,xmax - 24);
	    y = 2 + ogLibIntRand(0,ymax - 24);
	    ogEnvLog(1, "draw quad with texture color %s at (%d %d)\n", ogEnvColorString(rgbaColor), x, y);

	    glColor4f(1,1,1,1);
	    glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2i(x, y);
		glTexCoord2f(1.0, 0.0); glVertex2i(x + 16, y);
		glTexCoord2f(1.0, 1.0); glVertex2i(x + 16, y + 16);
		glTexCoord2f(0.0, 1.0); glVertex2i(x, y + 16);
	    glEnd();

	    readx = x+8;
	    ready = y+8;

#if 0
/* nice for debugging */
{
  GLuint pixel;
  ogLibReadPixels(readx, ready, readx, ready, &pixel);
  ogEnvLog(1, "expected color %s, real color %s\n", ogEnvColorString(newrgba), ogEnvColorString(pixel));
}
#endif
	    ogLibPointCheck(readx, ready, newrgba, newrgba,
			    tolerance + (hosttype == GL_BYTE ? 1 : 0));
	}
    }
}


CLEANUP(abgr)
{
    glColor4f(1,1,1,1);
    glDisable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
		GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    ogLibSetDefaultTextures();
}

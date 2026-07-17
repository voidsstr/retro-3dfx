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

/* blendfunc.c - $Revision: 2$ */

/*
 * This program tests the blendfunc command using points to check. 
 */

#include "ogtst.h"		/* include test environment		 */

static GLenum dst[] = {
    GL_ZERO,
    GL_ONE,
    GL_SRC_COLOR,
    GL_ONE_MINUS_SRC_COLOR,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
};

#define OG_NDST sizeof(dst)/sizeof(GLenum)

static char *dname[] = {
    "GL_ZERO",
    "GL_ONE",
    "GL_SRC_COLOR",
    "GL_ONE_MINUS_SRC_COLOR",
    "GL_SRC_ALPHA",
    "GL_ONE_MINUS_SRC_ALPHA",
    "GL_DST_ALPHA",
    "GL_ONE_MINUS_DST_ALPHA",
};

static GLenum src[] = {
    GL_ZERO,
    GL_ONE,
    GL_DST_COLOR,
    GL_ONE_MINUS_DST_COLOR,
    GL_SRC_ALPHA_SATURATE,
    GL_SRC_ALPHA,
    GL_ONE_MINUS_SRC_ALPHA,
    GL_DST_ALPHA,
    GL_ONE_MINUS_DST_ALPHA,
};

#define OG_NSRC sizeof(src)/sizeof(GLenum)

static char *sname[] = {
    "GL_ZERO",
    "GL_ONE",
    "GL_DST_COLOR",
    "GL_ONE_MINUS_DST_COLOR",
    "GL_SRC_ALPHA_SATURATE",
    "GL_SRC_ALPHA",
    "GL_ONE_MINUS_SRC_ALPHA",
    "GL_DST_ALPHA",
    "GL_ONE_MINUS_DST_ALPHA",
};

#define OG_NEQ 0

TESTMOD(blendfunc)
{
    int x, y, sfc, dfc, obj, si, di;
    int srcInd, dstInd, eqInd;
    GLuint expected;
    int xmax, ymax;
    int Rs, Gs, Bs, As;
    int Rd, Gd, Bd, Ad;
    int Ro, Go, Bo, Ao;
    GLint RGBbits[4], sorder[OG_NSRC], dorder[OG_NDST];
    float a[4], b[4], ffOverRGBmax[4];
    GLint nsrc, ndst, neq;
    
    nsrc = OG_NSRC; ndst = OG_NDST;

    neq = 1;

    RGBbits[0] = ogEnvCurVisualInfo(GLX_RED_SIZE);
    RGBbits[1] = ogEnvCurVisualInfo(GLX_GREEN_SIZE);
    RGBbits[2] = ogEnvCurVisualInfo(GLX_BLUE_SIZE);
    RGBbits[3] = ogEnvCurVisualInfo(GLX_ALPHA_SIZE);

    ffOverRGBmax[0] = 255.0 / ((1 << RGBbits[0]) - 1);
    ffOverRGBmax[1] = 255.0 / ((1 << RGBbits[1]) - 1);
    ffOverRGBmax[2] = 255.0 / ((1 << RGBbits[2]) - 1);
    ffOverRGBmax[3] = RGBbits[3] ? 255.0/((1 << RGBbits[3]) - 1) : 1;

    xmax = ogEnvQuery(OG_XWSIZE) - 1;
    ymax = ogEnvQuery(OG_YWSIZE) - 1;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0., ogEnvQuery(OG_XWSIZE), 0., ogEnvQuery(OG_YWSIZE), -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    ogLibClear(0);

    glEnable(GL_BLEND);
    ogEnvLog(OG_LPARAMETERS, "glEnable(GL_BLEND);\n");
    while (pass--) {
	for (eqInd = 0; eqInd < neq; eqInd++) {
	    ogLibOrderRand(sorder, nsrc);
	    for (srcInd = 0; srcInd < nsrc; srcInd++ ) {
		si = sorder[srcInd];
		sfc = src[si];
		ogLibOrderRand(dorder, ndst);	
		for (dstInd = 0; dstInd < ndst; dstInd++) { 
		    di = dorder[dstInd];
		    dfc = dst[di];

		    x = 2 + ogLibIntRand(0,xmax - 4);
		    y = 2 + ogLibIntRand(0,ymax - 4);

		    glBlendFunc(GL_ONE, GL_ZERO);
		    ogEnvLog(OG_LPARAMETERS, "glBlendFunc(GL_ONE, GL_ZERO);\n");

		    /*
		     * Make sure the backgroud of the point is cleared by
		     * writing a 5x5 rectangle of black points. 3x3 is not
		     * enough becase of the slight shift up and to the right
		     * of the vertex w.r.t pixel centers messes up multisampling.
		     */
		    glColor4ub(0, 0, 0, 0);
		    glRecti(x-2, y-2, x+2, y+2);
		    Rd = ogLibBitRand(RGBbits[0]) * ffOverRGBmax[0] + .5;
		    Gd = ogLibBitRand(RGBbits[1]) * ffOverRGBmax[1] + .5;
		    Bd = ogLibBitRand(RGBbits[2]) * ffOverRGBmax[2] + .5;
		    Ad = RGBbits[3] ? 
			ogLibBitRand(RGBbits[3]) * ffOverRGBmax[3] + .5 : 255;
		    glColor4ub(Rd, Gd, Bd, Ad);
		    ogEnvLog(OG_LPARAMETERS, 
			     "glColor4ub(0x%02x,0x%02x,0x%02x,0x%02x);\n",
			     Rd, Gd, Bd, Ad);

		    ogLibDrawFragments(x, y);

		    START_DL_OR_IM(1);

		    glBlendFunc(sfc, dfc);
		    ogEnvLog(OG_LGENERAL, "glBlendFunc(%s, %s);\n", 
			     sname[si], dname[di]);
		    if (obj) {
			glEndList();
			glBlendFunc(GL_ONE, GL_ZERO); /* can't use FINIS_DL_OR_IM() */
			glCallList(1);
			glDeleteLists(1, 1);
		    }

		    Rs = ogLibBitRand(8);
		    Gs = ogLibBitRand(8);
		    Bs = ogLibBitRand(8);
		    As = ogLibBitRand(8);
		    glColor4ub(Rs, Gs, Bs, As);
		    ogEnvLog(OG_LPARAMETERS, 
			     "glColor4ub(0x%02x,0x%02x,0x%02x,0x%02x);\n",
			     Rs, Gs, Bs, As);

		    ogLibDrawFragments(x, y);

		    switch (sfc) {
		    case GL_ZERO:
			a[0] = a[1] = a[2] = a[3] = 0;
			break;
		    case GL_ONE:
			a[0] = a[1] = a[2] = a[3] = 1;
			break;
		    case GL_DST_COLOR:
			a[0] = 1/255.0 * Rd;
			a[1] = 1/255.0 * Gd;
			a[2] = 1/255.0 * Bd;
			a[3] = 1/255.0 * Ad;
			break;
		    case GL_ONE_MINUS_DST_COLOR:
			a[0] = 1.0 - 1/255.0 * Rd;
			a[1] = 1.0 - 1/255.0 * Gd;
			a[2] = 1.0 - 1/255.0 * Bd;
			a[3] = 1.0 - 1/255.0 * Ad;
			break;
		    case GL_SRC_ALPHA:
			a[0] = a[1] = a[2] = a[3] = 1/255.0 * As;
			break;
		    case GL_ONE_MINUS_SRC_ALPHA:
			a[0] = a[1] = a[2] = a[3] = 1.0 - 1/255.0 * As;
			break;
		    case GL_DST_ALPHA:
			a[0] = a[1] = a[2] = a[3] = 1/255.0 * Ad;
			break;
		    case GL_ONE_MINUS_DST_ALPHA:
			a[0] = a[1] = a[2] = a[3] = 1.0 - 1/255.0 * Ad;
			break;
		    case GL_SRC_ALPHA_SATURATE:
			a[0] = a[1] = a[2] = 1/255.0 *
			    ((As < 255 - Ad) ? As : 255 - Ad);
			a[3] = 1;
			break;
		    default:
			ogEnvLog(OG_LFAIL, "sfactor botch %x\n", sfc);
			break;
		    }

		    switch (dfc) {
		    case GL_ZERO:
			b[0] = b[1] = b[2] = b[3] = 0;
			break;
		    case GL_ONE:
			b[0] = b[1] = b[2] = b[3] = 1;
			break;
		    case GL_SRC_COLOR:
			b[0] = 1/255.0 * Rs;
			b[1] = 1/255.0 * Gs;
			b[2] = 1/255.0 * Bs;
			b[3] = 1/255.0 * As;
			break;
		    case GL_ONE_MINUS_SRC_COLOR:
			b[0] = 1.0 - 1/255.0 * Rs;
			b[1] = 1.0 - 1/255.0 * Gs;
			b[2] = 1.0 - 1/255.0 * Bs;
			b[3] = 1.0 - 1/255.0 * As;
			break;
		    case GL_SRC_ALPHA:
			b[0] = b[1] = b[2] = b[3] = 1/255.0 * As;
			break;
		    case GL_ONE_MINUS_SRC_ALPHA:
			b[0] = b[1] = b[2] = b[3] = 1.0 - 1/255.0 * As;
			break;
		    case GL_DST_ALPHA:
			b[0] = b[1] = b[2] = b[3] = 1/255.0 * Ad;
			break;
		    case GL_ONE_MINUS_DST_ALPHA:
			b[0] = b[1] = b[2] = b[3] = 1.0 - 1/255.0 * Ad;
			break;
		    default:
			ogEnvLog(OG_LFAIL, "dfactor botch %x\n", dfc);
			break;
		    }

		    Ro = a[0] * Rs + b[0] * Rd;
		    Go = a[1] * Gs + b[1] * Gd;
		    Bo = a[2] * Bs + b[2] * Bd;
		    Ao = RGBbits[3] == 0 ? 255 : a[3] * As + b[3] * Ad;
		    
		    if (Ro > 255)
			Ro = 255;
                    else if (Ro < 0)
			Ro = 0;
		    if (Go > 255)
			Go = 255;
                    else if (Go < 0)
                        Go = 0;
		    if (Bo > 255)
			Bo = 255;
                    else if (Bo < 0)
                        Bo = 0;
                    if (Ao > 255)
			Ao = 255;
                    else if (Ao < 0)
                        Ao = 0;
		    expected = (Ro << 24) | (Go << 16) | (Bo << 8) | (Ao);
#if DEBUG_BLEND_FUNC
                {
                    GLuint buf;

                    ogLibReadPixels(x, y, x, y, &buf);
                    ogEnvLog(OG_LPARAMETERS,
                             "should be: (%x, %x, %x, %x) read %s\n",
                             Ro, Go, Bo, Ao, ogEnvColorString(buf));
                }
#endif
		    ogLibPointCheck(x, y, expected, 0, 3);
		}
	    }
	}
    }
}

CLEANUP(blendfunc)
{
    glBlendFunc(GL_ONE, GL_ZERO);
    glDisable(GL_BLEND);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    ogLibSetDefaultColors();
}

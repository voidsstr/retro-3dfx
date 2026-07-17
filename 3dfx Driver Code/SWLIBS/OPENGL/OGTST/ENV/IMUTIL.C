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

/* imutil.c - $Revision: 2$ */
/* Image file manipulation utilities built upon libimage.a */

#include <stdlib.h>
#include <alloca.h>
#ifndef WIN32
#include <GL/glx.h>
#endif
#include "env.h"
#include "ogtst.h"
#ifdef WIN32
#include "sgi.h"
#endif

/*
 * A 32 bit CRC algorithm derived from the 16 bit version in
 * Numerical Recipes 2nd Ed., pp 900-901.  Uses the polynomial
 * x^32 + x^26 + x^23 + x^22 + x^16 + x^12 + x^11 + x^10 + x^8 +
 * x^7 + x^5 + x^4 + x^2 + x^1 + 1.
 */

/*
 * mask corresponds to the polynomial (excludes 32nd bit)
 */
#define CRCMASK 0x04c11db7

static unsigned int
crcinit(unsigned int crc)
{
    int i;
    unsigned int ans = crc;

    for (i=0; i < 8; i++) {
	if (ans & 0x80000000) {
	    ans = (ans << 1) ^ CRCMASK;
	} else {
	    ans <<= 1;
	}
    }
    return ans;
}

static unsigned int crctab[256];

static unsigned int
crcgen(unsigned char *bufp, int len)
{
    unsigned int i, cword = ~0;
    static int crcinited = 0;
    
    if (!crcinited) {
	/* 
	 * Initialize a lookup table for the 8 most significant bits of the
	 * cumulative remainder.  This way we can do the division 8 bits at 
	 * a time, instead of 1 at a time.
	 */
	for (i=0; i < 256; i++) {
	    crctab[i] = crcinit(i << 24);
	}
	crcinited = 1;
    } 
    for (i=0; i < len; i++) {
	cword = crctab[ bufp[i] ^ (cword >> 24) ] ^ (cword << 8);
    }
    return cword;
}

/****************************************************************************
*  ogEnvImageSnap()  -
****************************************************************************/
GLuint
ogEnvImageSnap(Test *tst, GLint x0, GLint y0, GLint x1, GLint y1)
{
    GLuint limgsum;
    GLint count;
    gimg img;

    img.zsiz = ogEnvCurVisualInfo(GLX_RGBA);
    img.xsiz = x1 - x0 + 1;
    img.ysiz = y1 - y0 + 1;

    count = img.xsiz * img.ysiz;

    if ((img.buf = malloc(count * sizeof(GLuint))) == NULL)
      ogEnvLog(OG_LINTERNALERROR, "ogEnvImageSnap(): out of memory\n");

    ogLibReadPixels(x0,y0,x1-x0,y1-y0,(GLuint *) img.buf);

    /* compute CRC */
    limgsum = crcgen((unsigned char *)img.buf, count*sizeof(int));
    ogEnvCheckSum(tst, limgsum, &img);

    free(img.buf);
    return(limgsum);
}

/****************************************************************************
*  ogEnvImageSave()  -
****************************************************************************/
#ifdef WIN32
void 
ogEnvImageSave(char *f, gimg * img)
{
    sgi_t *image;
    unsigned short *ocp, *cp;
    GLint *tlp, *lp;
    int x, y;

    ocp = (unsigned short *) malloc(img->xsiz * sizeof(short));
    if (img->zsiz & 0x1) {
	image = sgiOpen(f, SGI_WRITE, SGI_COMP_RLE, 1, img->xsiz, img->ysiz, 4);

	for (y = 0, lp = (GLint *) img->buf; y < img->ysiz; y++, lp += img->xsiz) {
	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = (*tlp++ & 0xff000000) >> 24;
	    sgiPutRow(image, ocp, y, 0);

	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = (*tlp++ & 0x00ff0000) >> 16;
	    sgiPutRow(image, ocp, y, 1);

	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = (*tlp++ & 0x0000ff00) >> 8;
	    sgiPutRow(image, ocp, y, 2);

	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = (*tlp++ & 0x000000ff) >> 0;
	    sgiPutRow(image, ocp, y, 3);
	}
    } else {
	image = sgiOpen(f, SGI_WRITE, SGI_COMP_RLE, 2, img->xsiz, img->ysiz, 1);
	for (y = 0, lp = (GLint *) img->buf; y < img->ysiz; y++, lp += img->xsiz) {
	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = *tlp++;
	    sgiPutRow(image, ocp, y, 0);
	}
    }
    free(ocp);
    sgiClose(image);
}
#else
void 
ogEnvImageSave(char *f, gimg * img)
{
    IMAGE *image;
    unsigned short *ocp, *cp;
    GLint *tlp, *lp;
    int x, y;

    ocp = (unsigned short *) malloc(img->xsiz * sizeof(short));
    if (img->zsiz & 0x1) {
	image = iopen(f, "w", RLE(1), 3, img->xsiz, img->ysiz, 4);

	for (y = 0, lp = (GLint *) img->buf; y < img->ysiz; y++, lp += img->xsiz) {
	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = (*tlp++ & 0xff000000) >> 24;
	    putrow(image, ocp, y, 0);

	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = (*tlp++ & 0x00ff0000) >> 16;
	    putrow(image, ocp, y, 1);

	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = (*tlp++ & 0x0000ff00) >> 8;
	    putrow(image, ocp, y, 2);

	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = (*tlp++ & 0x000000ff) >> 0;
	    putrow(image, ocp, y, 3);
	}
    } else {
	image = iopen(f, "w", RLE(2), 2, img->xsiz, img->ysiz, 1);
	for (y = 0, lp = (GLint *) img->buf; y < img->ysiz; y++, lp += img->xsiz) {
	    for (x = 0, tlp = lp, cp = ocp; x < img->xsiz; x++)
		*cp++ = *tlp++;
	    putrow(image, ocp, y, 0);
	}
    }
    free(ocp);
    iclose(image);
}
#endif

/*
** Copyright 1996,1997 Silicon Graphics, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of Silicon Graphics, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of Silicon Graphics, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 7:57:09 PM$
*/
#include "fmacros.h"

/* Flat shaded */
GLboolean DRAWLINE1(__GLcontext *gc)
{
    GLint dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint len;
    FL_FBTYPE *fp;
    FL_FBTYPE result;
#if FL_IS_RGB
    GLuint r, g, b;
#else
    GLuint index;
#endif

    len = gc->polygon.shader.length;
    cfb = gc->polygon.shader.cfb;
    dBig = gc->line.options.xBig +
	gc->line.options.yBig * cfb->buf.outerWidth;
    dLittle = gc->line.options.xLittle +
	gc->line.options.yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (FL_FBTYPE *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

#if FL_IS_RGB
    SFloat2Int(gc->polygon.shader.frag.color.r, r);
    SFloat2Int(gc->polygon.shader.frag.color.g, g);
    SFloat2Int(gc->polygon.shader.frag.color.b, b);
    result =
	(r << cfb->redShift) |
	(g << cfb->greenShift) |
	(b << cfb->blueShift);
#else
    SFloat2Int(gc->polygon.shader.frag.color.r, index);
    result = (FL_FBTYPE) index;
#endif

    while (--len >= 0) {
	
	*fp = result;

	fraction += dfraction;
	if (fraction < 0) {
	    fraction &= ~0x80000000;
	    fp += dBig;
	} else {
	    fp += dLittle;
	}
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}


/* Flat shaded, dithered */
GLboolean DRAWLINE2(__GLcontext *gc)
{
    GLint x, y, xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint len;
    FL_FBTYPE *fp;
    float _tmp;
#if FL_IS_RGB
    FixedT fr, fg, fb;
#else
    FixedT findex;
#endif

#if FL_IS_RGB
    fr = PosFloatToFixed(gc->polygon.shader.frag.color.r);
    fg = PosFloatToFixed(gc->polygon.shader.frag.color.g);
    fb = PosFloatToFixed(gc->polygon.shader.frag.color.b);
#else
    findex = PosFloatToFixed(gc->polygon.shader.frag.color.r);
#endif

    len = gc->polygon.shader.length;
    cfb = gc->polygon.shader.cfb;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    dBig = xBig + yBig * cfb->buf.outerWidth;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    dLittle = xLittle + yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (FL_FBTYPE *), x, y);

    /* These are used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;

    while (--len >= 0) {
	
#if FL_IS_RGB
	GLuint r, g, b;
#else
	GLuint index;
#endif
	GLuint ix = ((x & 3) | (y & 12));
	FixedT inc;

	inc = __glFixedDitherTable[ix];

#if FL_IS_RGB
	r = FixedToUns(fr + inc) << cfb->redShift;
	g = FixedToUns(fg + inc) << cfb->greenShift;
	b = FixedToUns(fb + inc) << cfb->blueShift;
	*fp = r | g | b;
#else
	index = FixedToUns(findex + inc);
	*fp = index;
#endif

	fraction += dfraction;
	if (fraction < 0) {
	    fraction &= ~0x80000000;
	    x += xBig;
	    y += yBig;
	    fp += dBig;
	} else {
	    x += xLittle;
	    y += yLittle;
	    fp += dLittle;
	}
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}


/* Smooth shaded */
GLboolean DRAWLINE3(__GLcontext *gc)
{
    GLint dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint len;
    FL_FBTYPE *fp;
    float _tmp;
#if FL_IS_RGB
    FixedT fr, fg, fb;
    FixedT drdx, dgdx, dbdx;
#else
    FixedT findex;
    FixedT didx;
#endif

#if FL_IS_RGB
    fr = PosFloatToFixed(gc->polygon.shader.frag.color.r + 0.5f);
    fg = PosFloatToFixed(gc->polygon.shader.frag.color.g + 0.5f);
    fb = PosFloatToFixed(gc->polygon.shader.frag.color.b + 0.5f);
    drdx = SignedFloatToFixed(gc->polygon.shader.drdx);
    dgdx = SignedFloatToFixed(gc->polygon.shader.dgdx);
    dbdx = SignedFloatToFixed(gc->polygon.shader.dbdx);
#else
    findex = PosFloatToFixed(gc->polygon.shader.frag.color.r + 0.5f);
    didx = SignedFloatToFixed(gc->polygon.shader.drdx);
#endif

    len = gc->polygon.shader.length;
    cfb = gc->polygon.shader.cfb;
    dBig = gc->line.options.xBig +
	gc->line.options.yBig * cfb->buf.outerWidth;
    dLittle = gc->line.options.xLittle +
	gc->line.options.yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (FL_FBTYPE *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

    while (--len >= 0) {
	
#if FL_IS_RGB
	GLuint r, g, b;

	r = FixedToUns(fr) << cfb->redShift;
	g = FixedToUns(fg) << cfb->greenShift;
	b = FixedToUns(fb) << cfb->blueShift;

	*fp = r | g | b;
#else
	FL_FBTYPE index;

	index = FixedToUns(findex);

	*fp = index;
#endif

	fraction += dfraction;
	if (fraction < 0) {
	    fraction &= ~0x80000000;
	    fp += dBig;
	} else {
	    fp += dLittle;
	}

#if FL_IS_RGB
	fr += drdx;
	fg += dgdx;
	fb += dbdx;
#else
	findex += didx;
#endif
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}


/* Smooth shaded, dithered */
GLboolean DRAWLINE4(__GLcontext *gc)
{
    GLint x, y, xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolorBuffer *cfb;
    GLint len;
    FL_FBTYPE *fp;
    float _tmp;
#if FL_IS_RGB
    FixedT fr, fg, fb;
    FixedT drdx, dgdx, dbdx;
#else
    FixedT findex;
    FixedT didx;
#endif

#if FL_IS_RGB
    fr = PosFloatToFixed(gc->polygon.shader.frag.color.r);
    fg = PosFloatToFixed(gc->polygon.shader.frag.color.g);
    fb = PosFloatToFixed(gc->polygon.shader.frag.color.b);
    drdx = SignedFloatToFixed(gc->polygon.shader.drdx);
    dgdx = SignedFloatToFixed(gc->polygon.shader.dgdx);
    dbdx = SignedFloatToFixed(gc->polygon.shader.dbdx);
#else
    findex = PosFloatToFixed(gc->polygon.shader.frag.color.r);
    didx = SignedFloatToFixed(gc->polygon.shader.drdx);
#endif

    len = gc->polygon.shader.length;
    cfb = gc->polygon.shader.cfb;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    dBig = xBig + yBig * cfb->buf.outerWidth;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    dLittle = xLittle + yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (FL_FBTYPE *), x, y);

    /* These are used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;

    while (--len >= 0) {
	
#if FL_IS_RGB
	GLuint r, g, b;
#else
	GLuint index;
#endif
	GLuint ix = ((x & 3) | (y & 12));
	FixedT inc;

	inc = __glFixedDitherTable[ix];

#if FL_IS_RGB
	r = FixedToUns(fr + inc) << cfb->redShift;
	g = FixedToUns(fg + inc) << cfb->greenShift;
	b = FixedToUns(fb + inc) << cfb->blueShift;

	*fp = r | g | b;
#else
	index = FixedToUns(findex + inc);

	*fp = index;
#endif

	fraction += dfraction;
	if (fraction < 0) {
	    fraction &= ~0x80000000;
	    x += xBig;
	    y += yBig;
	    fp += dBig;
	} else {
	    x += xLittle;
	    y += yLittle;
	    fp += dLittle;
	}

#if FL_IS_RGB
	fr += drdx;
	fg += dgdx;
	fb += dbdx;
#else
	findex += didx;
#endif
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

/* Stippled versions */

/* Stippled, Flat shaded */
GLboolean DRAWLINE5(__GLcontext *gc)
{
    GLint dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint len, count;
    FL_FBTYPE *fp;
    FL_FBTYPE result;
#if FL_IS_RGB
    GLuint r, g, b;
#else
    GLuint index;
#endif

    len = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    cfb = gc->polygon.shader.cfb;
    dBig = gc->line.options.xBig +
	gc->line.options.yBig * cfb->buf.outerWidth;
    dLittle = gc->line.options.xLittle +
	gc->line.options.yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (FL_FBTYPE *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

#if FL_IS_RGB
    SFloat2Int(gc->polygon.shader.frag.color.r, r);
    SFloat2Int(gc->polygon.shader.frag.color.g, g);
    SFloat2Int(gc->polygon.shader.frag.color.b, b);
    result =
	(r << cfb->redShift) |
	(g << cfb->greenShift) |
	(b << cfb->blueShift);
#else
    SFloat2Int(gc->polygon.shader.frag.color.r, index);
    result = (FL_FBTYPE) index;
#endif

    while (len) {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		*fp = result;
	    }

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		fp += dBig;
	    } else {
		fp += dLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

/* Stippled, Flat shaded, dithered */
GLboolean DRAWLINE6(__GLcontext *gc)
{
    GLint x, y, xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint len, count;
    FL_FBTYPE *fp;
    float _tmp;
#if FL_IS_RGB
    FixedT fr, fg, fb;
#else
    FixedT findex;
#endif

#if FL_IS_RGB
    fr = PosFloatToFixed(gc->polygon.shader.frag.color.r);
    fg = PosFloatToFixed(gc->polygon.shader.frag.color.g);
    fb = PosFloatToFixed(gc->polygon.shader.frag.color.b);
#else
    findex = PosFloatToFixed(gc->polygon.shader.frag.color.r);
#endif

    len = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    cfb = gc->polygon.shader.cfb;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    dBig = xBig + yBig * cfb->buf.outerWidth;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    dLittle = xLittle + yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (FL_FBTYPE *), x, y);

    /* These are used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;


    while (len) {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
#if FL_IS_RGB
		GLuint r, g, b;
#else
		GLuint index;
#endif
		GLuint ix = ((x & 3) | (y & 12));
		FixedT inc;

		inc = __glFixedDitherTable[ix];

#if FL_IS_RGB
		r = FixedToUns(fr + inc) << cfb->redShift;
		g = FixedToUns(fg + inc) << cfb->greenShift;
		b = FixedToUns(fb + inc) << cfb->blueShift;
		*fp = r | g | b;
#else
		index = FixedToUns(findex + inc);
		*fp = index;
#endif
	    }

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		x += xBig;
		y += yBig;
		fp += dBig;
	    } else {
		x += xLittle;
		y += yLittle;
		fp += dLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	}
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

/* Stippled, Smooth shaded */
GLboolean DRAWLINE7(__GLcontext *gc)
{
    GLint dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint len, count;
    FL_FBTYPE *fp;
    float _tmp;
#if FL_IS_RGB
    FixedT fr, fg, fb;
    FixedT drdx, dgdx, dbdx;
#else
    FixedT findex;
    FixedT didx;
#endif

    len = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    cfb = gc->polygon.shader.cfb;
    dBig = gc->line.options.xBig +
	gc->line.options.yBig * cfb->buf.outerWidth;
    dLittle = gc->line.options.xLittle +
	gc->line.options.yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (FL_FBTYPE *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

#if FL_IS_RGB
    fr = PosFloatToFixed(gc->polygon.shader.frag.color.r + 0.5f);
    fg = PosFloatToFixed(gc->polygon.shader.frag.color.g + 0.5f);
    fb = PosFloatToFixed(gc->polygon.shader.frag.color.b + 0.5f);
    drdx = SignedFloatToFixed(gc->polygon.shader.drdx);
    dgdx = SignedFloatToFixed(gc->polygon.shader.dgdx);
    dbdx = SignedFloatToFixed(gc->polygon.shader.dbdx);
#else
    findex = PosFloatToFixed(gc->polygon.shader.frag.color.r + 0.5f);
    didx = SignedFloatToFixed(gc->polygon.shader.drdx);
#endif

    while (len) {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
#if FL_IS_RGB
		GLuint r, g, b;

		r = FixedToUns(fr) << cfb->redShift;
		g = FixedToUns(fg) << cfb->greenShift;
		b = FixedToUns(fb) << cfb->blueShift;
		*fp = r | g | b;
#else
		FL_FBTYPE index;

		index = FixedToUns(findex);
		*fp = index;
#endif
	    }

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		fp += dBig;
	    } else {
		fp += dLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
#if FL_IS_RGB
	    fr += drdx;
	    fg += dgdx;
	    fb += dbdx;
#else
	    findex += didx;
#endif
	}
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}


/* Stippled, Smooth shaded, dithered */
GLboolean DRAWLINE8(__GLcontext *gc)
{
    GLint x, y, xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint len, count;
    FL_FBTYPE *fp;
    float _tmp;
#if FL_IS_RGB
    FixedT fr, fg, fb;
    FixedT drdx, dgdx, dbdx;
#else
    FixedT findex;
    FixedT didx;
#endif

    len = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    cfb = gc->polygon.shader.cfb;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    dBig = xBig + yBig * cfb->buf.outerWidth;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    dLittle = xLittle + yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (FL_FBTYPE *), x, y);

    /* These are used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;

#if FL_IS_RGB
    fr = PosFloatToFixed(gc->polygon.shader.frag.color.r);
    fg = PosFloatToFixed(gc->polygon.shader.frag.color.g);
    fb = PosFloatToFixed(gc->polygon.shader.frag.color.b);
    drdx = SignedFloatToFixed(gc->polygon.shader.drdx);
    dgdx = SignedFloatToFixed(gc->polygon.shader.dgdx);
    dbdx = SignedFloatToFixed(gc->polygon.shader.dbdx);
#else
    findex = PosFloatToFixed(gc->polygon.shader.frag.color.r);
    didx = SignedFloatToFixed(gc->polygon.shader.drdx);
#endif

    while (len) {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
#if FL_IS_RGB
		GLuint r, g, b;
#else
		FL_FBTYPE index;
#endif
		GLuint ix = ((x & 3) | (y & 12));
		FixedT inc;

		inc = __glFixedDitherTable[ix];

#if FL_IS_RGB
		r = FixedToUns(fr) << cfb->redShift;
		g = FixedToUns(fg) << cfb->greenShift;
		b = FixedToUns(fb) << cfb->blueShift;
		*fp = r | g | b;
#else
		index = FixedToUns(findex);
		*fp = index;
#endif
	    }

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		x += xBig;
		y += yBig;
		fp += dBig;
	    } else {
		x += xLittle;
		y += yLittle;
		fp += dLittle;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
#if FL_IS_RGB
	    fr += drdx;
	    fg += dgdx;
	    fb += dbdx;
#else
	    findex += didx;
#endif
	}
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

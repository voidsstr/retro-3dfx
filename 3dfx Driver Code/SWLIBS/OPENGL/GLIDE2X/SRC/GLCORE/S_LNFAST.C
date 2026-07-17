/*
** Copyright 1991-1997, Silicon Graphics, Inc.
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
*/
#include "render.h"
#include "context.h"
#include "global.h"
#include "imports.h"
#include "fmacros.h"

/* Pre-computed dithers */
__GLfloat __glFastDitherTable[16] = {
    -0.46875,  0.03125, -0.34375,  0.15625,
     0.28125, -0.21875,  0.40625, -0.09375,
    -0.28125,  0.21875, -0.40625,  0.09375,
     0.46875, -0.03125,  0.34375, -0.15625,
};

#ifndef SFloat2Int
#define SFloat2Int(_flt_, _int_) \
    (*((float *)(&(_int_)))=(_flt_)+((float)(3<<(FLOAT_MANTISSA_BITS-1))), \
     (_int_)=((_int_)&FLOAT_MANTISSA_MASK)-(1<<(FLOAT_MANTISSA_BITS-1)))
#endif

#if 0
/* General path for 8-bit color index */
GLboolean __glStoreLine_CI_8_General(__GLcontext *gc)
{
    GLint xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint len;
    GLint x, y;

    GLubyte *fp;
    __GLfloat inc;
    GLubyte fbcolor, result;
    GLuint enables = gc->state.enables.general;
    GLuint dither = enables & __GL_DITHER_ENABLE;
    GLuint logicop = enables & __GL_INDEX_LOGIC_OP_ENABLE;

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
    cp = gc->polygon.shader.colors;
    x = gc->line.options.xStart;
    y = gc->line.options.yStart;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLubyte *), x, y);
    inc = __glHalf;

    while (--len >= 0) {

	if (dither) {
	    GLuint ix = __GL_DITHER_INDEX(x, y);
	    inc = __glFastDitherTable[ix]; 
	}

	result = (cp++)->r + inc;

	fbcolor = *fp;
	if (logicop) {
	    switch(gc->state.raster.logicOp) {
	    case GL_CLEAR:         result = 0; break;
	    case GL_AND:           result = result & fbcolor; break;
	    case GL_AND_REVERSE:   result = result & (~fbcolor); break;
	    case GL_COPY:          result = result; break;
	    case GL_AND_INVERTED:  result = (~result) & fbcolor; break;
	    case GL_NOOP:          result = fbcolor; break;
	    case GL_XOR:           result = result ^ fbcolor; break;
	    case GL_OR:            result = result | fbcolor; break;
	    case GL_NOR:           result = ~(result | fbcolor); break;
	    case GL_EQUIV:         result = ~(result ^ fbcolor); break;
	    case GL_INVERT:        result = ~fbcolor; break;
	    case GL_OR_REVERSE:    result = result | (~fbcolor); break;
	    case GL_COPY_INVERTED: result = ~result; break;
	    case GL_OR_INVERTED:   result = (~result) | fbcolor; break;
	    case GL_NAND:          result = ~(result & fbcolor); break;
	    case GL_SET:           result = ~0; break;
	    }
	}

	*fp = (fbcolor & cfb->destMask) | (result & cfb->sourceMask);
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
#endif

/* Flat shaded only! */
GLboolean __glStoreLine_CI_8_Flat(__GLcontext *gc)
{
    GLint xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint len, ii;
    GLubyte *fp;
    GLuint result;

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
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLubyte *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

    SFloat2Int(cp->r, result);

    for (ii = 0; ii < len; ii++) {

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

/* No dither, color per pixel */
GLboolean __glStoreLine_CI_8_Smooth(__GLcontext *gc)
{
    GLint dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint len, ii;
    GLubyte *fp;
    GLuint result;

    len = gc->polygon.shader.length;
    cfb = gc->polygon.shader.cfb;
    dBig = gc->line.options.xBig +
	gc->line.options.yBig * cfb->buf.outerWidth;
    dLittle = gc->line.options.xLittle +
	gc->line.options.yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLubyte *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

    for (ii = 0; ii < len; ii++, cp++) {

	SFloat2Int(cp->r, result);
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
GLboolean __glStoreLine_CI_8_Flat_Dither(__GLcontext *gc)
{
    GLint dLittle, dBig, fraction, dfraction, len;
    GLubyte *fp;
    GLint x, y, xLittle, yLittle, xBig, yBig;
    __GLfloat fresult;

    __GL_LOCK_BUFFERS(gc);

    /* Initialization */
    {
	__GLcolorBuffer *cfb;
	__GLlineOptions *options;
	GLint outerWidth;

        {
	    __GLshade *shader;
	    shader = &gc->polygon.shader;
	    cfb = shader->cfb;
	    len = shader->length;
	    fresult = shader->colors->r;
	}

	outerWidth = cfb->buf.outerWidth;
	options = &gc->line.options;
	xBig = options->xBig;
	yBig = options->yBig;
	dBig = xBig + yBig * outerWidth;
	yBig <<= 2;
	xLittle = options->xLittle;
	yLittle = options->yLittle;
	dLittle = xLittle + yLittle * outerWidth;
	yLittle <<= 2;
	x = options->xStart;
	y = options->yStart << 2;
	fraction = options->fraction;
	dfraction = options->dfraction;
	fp = __GL_FB_ADDRESS(cfb, (GLubyte *), options->xStart,
			     options->yStart);
    }

    while (--len >= 0) {

	GLuint ix = ((x & 3) | (y & 12));
	__GLfloat ftmp;
	GLuint result;

	ftmp = fresult + __glFastDitherTable[ix];
	SFloat2Int(ftmp, result);
	*fp = result;

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

/* dithered, color per pixel */
GLboolean __glStoreLine_CI_8_Smooth_Dither(__GLcontext *gc)
{
    GLint dLittle, dBig, fraction, dfraction, len;
    GLubyte *fp;
    __GLcolor *cp;
    GLint x, y, xLittle, yLittle, xBig, yBig;

    __GL_LOCK_BUFFERS(gc);

    /* Initialization */
    {
	__GLcolorBuffer *cfb;
	__GLlineOptions *options;
	GLint outerWidth;

        {
	    __GLshade *shader;
	    shader = &gc->polygon.shader;
	    cfb = shader->cfb;
	    len = shader->length;
	    cp = shader->colors;
	}

	outerWidth = cfb->buf.outerWidth;
	options = &gc->line.options;
	xBig = options->xBig;
	yBig = options->yBig;
	dBig = xBig + yBig * outerWidth;
	yBig <<= 2;
	xLittle = options->xLittle;
	yLittle = options->yLittle;
	dLittle = xLittle + yLittle * outerWidth;
	yLittle <<= 2;
	x = options->xStart;
	y = options->yStart << 2;
	fraction = options->fraction;
	dfraction = options->dfraction;
	fp = __GL_FB_ADDRESS(cfb, (GLubyte *), options->xStart,
			     options->yStart);
    }

    while (--len >= 0) {

	GLuint ix = ((x & 3) | (y & 12));
	__GLfloat fresult;
	GLuint result;

	fresult = cp->r + __glFastDitherTable[ix];
	SFloat2Int(fresult, result);
	*fp = result;
	cp++;

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

GLboolean __glStoreLine_RGB_16_Flat(__GLcontext *gc)
{
    GLint xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint len, ii;
    GLushort *fp;
    GLushort result;
    GLuint r, g, b;

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
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLushort *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

    SFloat2Int(cp->r, r);
    SFloat2Int(cp->g, g);
    SFloat2Int(cp->b, b);
    result =
	(r << cfb->redShift) |
	(g << cfb->greenShift) |
	(b << cfb->blueShift);

    for (ii = 0; ii < len; ii++) {
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

GLboolean __glStoreLine_RGB_16_Dither(__GLcontext *gc)
{
    GLint x, y, xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint len, ii;
    GLushort *fp;
    GLuint r, g, b;

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
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);

    /* These are used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;

    for (ii = 0; ii < len; ii++, cp++) {
	
	GLuint ix = ((x & 3) | (y & 12));
	__GLfloat ftmp, inc;

	inc = __glFastDitherTable[ix];
	ftmp = cp->r + inc;
	SFloat2Int(ftmp, r);
	ftmp = cp->g + inc;
	SFloat2Int(ftmp, g);
	ftmp = cp->b + inc;
	SFloat2Int(ftmp, b);

	*fp =
	    (r << cfb->redShift) |
	    (g << cfb->greenShift) |
	    (b << cfb->blueShift);

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

GLboolean __glStoreLine_RGB_16_Smooth(__GLcontext *gc)
{
    GLint xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint len, ii;
    GLushort *fp;
    GLint rs, gs, bs;

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
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLushort *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

    rs = cfb->redShift;
    gs = cfb->greenShift;
    bs = cfb->blueShift;

    for (ii = 0; ii < len; ii++, cp++) {
	unsigned int r, g, b;

	SFloat2Int(cp->r, r);
	SFloat2Int(cp->g, g);
	SFloat2Int(cp->b, b);

	*fp = (r << rs) | (g << gs) | (b << bs);

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

/* Stippled versions */

/* Flat shaded only! */
GLboolean __glStoreStippledLine_CI_8_Flat(__GLcontext *gc)
{
    GLint xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint len, count;
    GLubyte *fp;
    GLuint result;

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
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLubyte *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

    SFloat2Int(cp->r, result);

    do {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);

	do {
	    if (inMask & bit) {
		*fp = result;
	    }

#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		fp += dBig;
	    } else {
		fp += dLittle;
	    }

	} while (--count);
    } while (len);

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

/* No dither, color per pixel */
GLboolean __glStoreStippledLine_CI_8_Smooth(__GLcontext *gc)
{
    GLint dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint len, count;
    GLubyte *fp;
    GLuint result;

    len = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;
    cfb = gc->polygon.shader.cfb;
    dBig = gc->line.options.xBig +
	gc->line.options.yBig * cfb->buf.outerWidth;
    dLittle = gc->line.options.xLittle +
	gc->line.options.yLittle * cfb->buf.outerWidth;
    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLubyte *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

    do {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);

	do {
	    if (inMask & bit) {
		SFloat2Int(cp->r, result);
		*fp = result;
	    }

#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	    cp++;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		fp += dBig;
	    } else {
		fp += dLittle;
	    }
	} while (--count);
    } while (len);

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

/* Flat shaded, dithered */
GLboolean __glStoreStippledLine_CI_8_Flat_Dither(__GLcontext *gc)
{
    GLint dLittle, dBig, fraction, dfraction, len;
    GLubyte *fp;
    GLint x, y, xLittle, yLittle, xBig, yBig, count;
    __GLstippleWord inMask, bit, *sp;
    __GLfloat fresult;

    __GL_LOCK_BUFFERS(gc);

    /* Initialization */
    {
	__GLcolorBuffer *cfb;
	__GLlineOptions *options;
	GLint outerWidth;

        {
	    __GLshade *shader;
	    shader = &gc->polygon.shader;
	    sp = shader->stipplePat;
	    cfb = shader->cfb;
	    len = shader->length;
	    fresult = shader->colors->r;
	}

	outerWidth = cfb->buf.outerWidth;
	options = &gc->line.options;
	xBig = options->xBig;
	yBig = options->yBig;
	dBig = xBig + yBig * outerWidth;
	yBig <<= 2;
	xLittle = options->xLittle;
	yLittle = options->yLittle;
	dLittle = xLittle + yLittle * outerWidth;
	yLittle <<= 2;
	x = options->xStart;
	y = options->yStart << 2;
	fraction = options->fraction;
	dfraction = options->dfraction;
	fp = __GL_FB_ADDRESS(cfb, (GLubyte *), options->xStart,
			     options->yStart);
    }

    do {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	do {
	    if (inMask & bit) {
		GLuint ix = ((x & 3) | (y & 12));
		__GLfloat ftmp;
		GLuint result;

		ftmp = fresult + __glFastDitherTable[ix];
		SFloat2Int(ftmp, result);
		*fp = result;
	    }

#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
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
	} while (--count);
    } while (len);

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

/* dithered, color per pixel */
GLboolean __glStoreStippledLine_CI_8_Smooth_Dither(__GLcontext *gc)
{
    GLint dLittle, dBig, fraction, dfraction, len;
    GLubyte *fp;
    __GLcolor *cp;
    GLint x, y, xLittle, yLittle, xBig, yBig, count;
    __GLstippleWord inMask, bit, *sp;

    __GL_LOCK_BUFFERS(gc);

    /* Initialization */
    {
	__GLcolorBuffer *cfb;
	__GLlineOptions *options;
	GLint outerWidth;

        {
	    __GLshade *shader;
	    shader = &gc->polygon.shader;
	    sp = shader->stipplePat;
	    cfb = shader->cfb;
	    len = shader->length;
	    cp = shader->colors;
	}

	outerWidth = cfb->buf.outerWidth;
	options = &gc->line.options;
	xBig = options->xBig;
	yBig = options->yBig;
	dBig = xBig + yBig * outerWidth;
	yBig <<= 2;
	xLittle = options->xLittle;
	yLittle = options->yLittle;
	dLittle = xLittle + yLittle * outerWidth;
	yLittle <<= 2;
	x = options->xStart;
	y = options->yStart << 2;
	fraction = options->fraction;
	dfraction = options->dfraction;
	fp = __GL_FB_ADDRESS(cfb, (GLubyte *), options->xStart,
			     options->yStart);
    }

    do {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	do {
	    if (inMask & bit) {
		GLuint ix = ((x & 3) | (y & 12));
		__GLfloat fresult;
		GLuint result;

		fresult = cp->r + __glFastDitherTable[ix];
		SFloat2Int(fresult, result);
		*fp = result;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	    cp++;

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
	} while (--count);
    } while (len);

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

GLboolean __glStoreStippledLine_RGB_16_Flat(__GLcontext *gc)
{
    GLint xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint len, count;
    GLushort *fp;
    GLushort result;
    GLuint r, g, b;

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
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLushort *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

    SFloat2Int(cp->r, r);
    SFloat2Int(cp->g, g);
    SFloat2Int(cp->b, b);
    result =
	(r << cfb->redShift) |
	(g << cfb->greenShift) |
	(b << cfb->blueShift);

    do {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	do {
	    if (inMask & bit) {
		*fp = result;
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		fp += dBig;
	    } else {
		fp += dLittle;
	    }
	} while (--count);
    } while (len);

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

GLboolean __glStoreStippledLine_RGB_16_Dither(__GLcontext *gc)
{
    GLint x, y, xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint len, count;
    GLushort *fp;
    GLuint r, g, b;

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
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);

    /* These are used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;

    do {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	do {
	    if (inMask & bit) {
		GLuint ix = ((x & 3) | (y & 12));
		__GLfloat ftmp, inc;

		inc = __glFastDitherTable[ix];
		ftmp = cp->r + inc;
		SFloat2Int(ftmp, r);
		ftmp = cp->g + inc;
		SFloat2Int(ftmp, g);
		ftmp = cp->b + inc;
		SFloat2Int(ftmp, b);

		*fp =
		     (r << cfb->redShift) |
		     (g << cfb->greenShift) |
		     (b << cfb->blueShift);
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	    cp++;

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
	} while (--count);
    } while (len);

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

GLboolean __glStoreStippledLine_RGB_16_Smooth(__GLcontext *gc)
{
    GLint xLittle, xBig, yLittle, yBig, dLittle, dBig;
    GLint fraction, dfraction;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint len, count;
    GLushort *fp;
    GLint rs, gs, bs;

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
    cp = gc->polygon.shader.colors;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLushort *),
			 gc->line.options.xStart,
			 gc->line.options.yStart);

    rs = cfb->redShift;
    gs = cfb->greenShift;
    bs = cfb->blueShift;

    do {
	count = len;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	len -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	do {
	    if (inMask & bit) {
		unsigned int r, g, b;

		SFloat2Int(cp->r, r);
		SFloat2Int(cp->g, g);
		SFloat2Int(cp->b, b);

		*fp = (r << rs) | (g << gs) | (b << bs);
	    }
#ifdef __GL_STIPPLE_MSB
	    bit >>= 1;
#else
	    bit <<= 1;
#endif
	    cp++;

	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		fp += dBig;
	    } else {
		fp += dLittle;
	    }
	} while (--count);
    } while (len);

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

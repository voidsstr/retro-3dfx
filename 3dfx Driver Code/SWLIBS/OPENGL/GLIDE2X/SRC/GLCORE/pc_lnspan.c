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
#include "fastline.h"
#include "pc_lnspan.h"

/* Pre-computed dithers */
FixedT __glFixedDitherTable[16] = {
    FloatToFixed(-0.46875 + 0.5), FloatToFixed( 0.03125 + 0.5),
    FloatToFixed(-0.34375 + 0.5), FloatToFixed( 0.15625 + 0.5),
    FloatToFixed( 0.28125 + 0.5), FloatToFixed(-0.21875 + 0.5),
    FloatToFixed( 0.40625 + 0.5), FloatToFixed(-0.09375 + 0.5),
    FloatToFixed(-0.28125 + 0.5), FloatToFixed( 0.21875 + 0.5),
    FloatToFixed(-0.40625 + 0.5), FloatToFixed( 0.09375 + 0.5),
    FloatToFixed( 0.46875 + 0.5), FloatToFixed(-0.03125 + 0.5),
    FloatToFixed( 0.34375 + 0.5), FloatToFixed(-0.15625 + 0.5),
};

/*
** Take incoming line and draw it to both FRONT and BACK buffers.
**
** For use only with optimized line procs.  Since shade and store is
** done in one pass, it is impractical to save the shaded and stippled
** result prior to store.  In this version we actually do all work to
** shade the line twice.
**
** Return value is ignored.
*/
GLboolean __glSlowDrawBothLine(__GLcontext *gc)
{
    /* Step 3:  Draw to FRONT_AND_BACK */
    gc->polygon.shader.cfb = &gc->frontBuffer;
    (*gc->procs.line.drawLine)(gc);
    gc->polygon.shader.cfb = &gc->backBuffer;
    (*gc->procs.line.drawLine)(gc);

    return GL_FALSE;
}

/*
** Process the incoming line by calling the 2 appropriate line procs.  It does
** not chain to gc->procs.line.wideLineRep, but returns instead.  This is a 
** specific fast path.
**
** Return value is ignored.
**
** It sets gc->polygon.shader.cfb to gc->drawBuffer.
*/
GLboolean __glProcessLine2NW(__GLcontext *gc)
{
    gc->polygon.shader.cfb = gc->drawBuffer;

    gc->polygon.shader.done = GL_FALSE;

    /* Call non-stippled procs... */
    if ((*gc->procs.line.lineFuncs[0])(gc)) {
	if (gc->polygon.shader.done) return GL_TRUE;
	goto stippled;
    }
    return (*gc->procs.line.lineFuncs[1])(gc);

stippled:
    return (*gc->procs.line.stippledLineFuncs[1])(gc);
}


/* 8-bit CI */
#define FL_FBMODE	CI8
#define FL_FBTYPE	GLubyte
#define FL_IS_RGB	GL_FALSE

#define DRAWLINE1 PROCNAME1(CI8)
#define DRAWLINE2 PROCNAME2(CI8)
#define DRAWLINE3 PROCNAME3(CI8)
#define DRAWLINE4 PROCNAME4(CI8)
#define DRAWLINE5 PROCNAME5(CI8)
#define DRAWLINE6 PROCNAME6(CI8)
#define DRAWLINE7 PROCNAME7(CI8)
#define DRAWLINE8 PROCNAME8(CI8)

#include "fastline.c"

#undef DRAWLINE8
#undef DRAWLINE7
#undef DRAWLINE6
#undef DRAWLINE5
#undef DRAWLINE4
#undef DRAWLINE3
#undef DRAWLINE2
#undef DRAWLINE1

#undef FL_FBMODE
#undef FL_FBTYPE
#undef FL_IS_RGB

/* 16-bit RGB */
#define FL_FBMODE	RGB16
#define FL_FBTYPE	GLushort
#define FL_IS_RGB	GL_TRUE

#define DRAWLINE1 PROCNAME1(RGB16)
#define DRAWLINE2 PROCNAME2(RGB16)
#define DRAWLINE3 PROCNAME3(RGB16)
#define DRAWLINE4 PROCNAME4(RGB16)
#define DRAWLINE5 PROCNAME5(RGB16)
#define DRAWLINE6 PROCNAME6(RGB16)
#define DRAWLINE7 PROCNAME7(RGB16)
#define DRAWLINE8 PROCNAME8(RGB16)

#include "fastline.c"

#undef DRAWLINE8
#undef DRAWLINE7
#undef DRAWLINE6
#undef DRAWLINE5
#undef DRAWLINE4
#undef DRAWLINE3
#undef DRAWLINE2
#undef DRAWLINE1

#undef FL_FBMODE
#undef FL_FBTYPE
#undef FL_IS_RGB


/* Fast path: 16-bit RGB, flat shaded, depth tested, anti-aliased */
GLboolean __glDrawAALine_RGB_16_Flat_LESS16_SA_MSA(__GLcontext *gc)
{
    __GLfloat length;	/* Dist along length */
    __GLfloat width;	/* Dist along width */
    GLint fraction, dfraction;
    __GLfloat dlLittle, dlBig;
    __GLfloat ddLittle, ddBig;
    __GLcolor *cp;
    __GLfloat coverage;
    __GLfloat lineWidth;
    __GLfloat lineLength;
    GLint count;
    GLint w;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    /* Depth declarations */
    __GLzValue16 zFrac, z, *zfb;
    GLint dzdx, dzdxBig;
    GLuint cnt;
    GLint numFracBits;
    GLint xLittle, xBig, yLittle, yBig;
    GLint dzpLittle, dzpBig;

    /* Store declarations */
    __GLcolorBuffer *cfb;
    GLint fbLittle, fbBig;
    GLushort *fp;
    __GLfloat fr, fg, fb, fa, alpha;
    GLint x, y, rs, gs, bs;
    GLuint enables;

    w = gc->polygon.shader.length;

    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    cp = gc->polygon.shader.colors;

    dlLittle = gc->line.options.dlLittle;
    dlBig = gc->line.options.dlBig;
    ddLittle = gc->line.options.ddLittle;
    ddBig = gc->line.options.ddBig;

    length = gc->line.options.plength;
    width = gc->line.options.pwidth;
    lineLength = gc->line.options.realLength - __glHalf;
    lineWidth = __glHalf * gc->state.line.smoothWidth - __glHalf;


    /* Depth initialization */
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLzValue16*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;

    numFracBits = gc->depthBuffer.numFracBits;
    cnt = gc->depthBuffer.cnt;
    zFrac = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx >> numFracBits;
    dzdxBig = gc->polygon.shader.dzdxBig;

    /* Store initializations */
    enables = gc->state.enables.general;
    cfb = gc->polygon.shader.cfb;
    fbBig = xBig + yBig * cfb->buf.outerWidth;
    fbLittle = xLittle + yLittle * cfb->buf.outerWidth;
    rs = cfb->redShift;
    gs = cfb->greenShift;
    bs = cfb->blueShift;
    fr = gc->polygon.shader.frag.color.r;
    fg = gc->polygon.shader.frag.color.g;
    fb = gc->polygon.shader.frag.color.b;
    fa = gc->polygon.shader.frag.color.a * gc->frontBuffer.oneOverAlphaScale;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);

    /* These are hereafter used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;

    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	z = (zFrac >> numFracBits) + cnt;

	while (--count >= 0) {
	    /* This normally goes after the alpha calculation below,
	     * but can safely be moved here as an optimization.
	     */
	    if ( (GLuint) z < (GLuint) *zfb ) {

		/* Coverage for sides */
		if (width > lineWidth) {
		    coverage = lineWidth - width + __glOne;
		    if (coverage <= __glZero) {
			goto next_pixel;
		    }
		} else if (width < -lineWidth) {
		    coverage = width + lineWidth + __glOne;
		    if (coverage <= __glZero) {
			goto next_pixel;
		    }
		} else {
		    coverage = __glOne;
		}

		/* Coverage for start, end */
		if (length < __glHalf) {
		    coverage *= length + __glHalf;
		    if (coverage <= __glZero) {
			goto next_pixel;
		    }
		} else if (length > lineLength) {
		    coverage *= lineLength - length + __glOne;
		    if (coverage <= __glZero) {
			goto next_pixel;
		    }
		} 

		alpha = (fa == __glOne) ? coverage : fa * coverage;
		*zfb = z;

		/* Store the pixel */
	        {
		    unsigned int r, g, b;
		    __GLfloat dr, dg, db;
		    __GLfloat msa, rr, rg, rb;
		    GLushort value;


		    if (__glOne == alpha) {
			rr = fr;
			rg = fg;
			rb = fb;
		    } else {
			value = *fp;
			dr = (value & gc->modes.redMask) >> rs;
			dg = (value & gc->modes.greenMask) >> gs;
			db = (value & gc->modes.blueMask) >> bs;

			msa = __glOne - alpha;

			rr = fr * alpha + dr * msa;
			rg = fg * alpha + dg * msa;
			rb = fb * alpha + db * msa;
		    }

		    if (enables & __GL_DITHER_ENABLE) {
			GLuint ix = ((x & 3) | (y & 12));
			__GLfloat inc = __glFastDitherTable[ix];
			rr += inc;
			rg += inc;
			rb += inc;
		    }

		    SFloat2Int(rr, r);
		    SFloat2Int(rg, g);
		    SFloat2Int(rb, b);

		    *fp = (r << rs) | (g << gs) | (b << bs);
		}
	    }
next_pixel:
	    z += dzdx;
	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		length += dlBig;
		width += ddBig;
		zfb += dzpBig;
		x += xBig;
		y += yBig;
		fp += fbBig;
	    } else {
		length += dlLittle;
		width += ddLittle;
		zfb += dzpLittle;
		x += xLittle;
		y += yLittle;
		fp += fbLittle;
	    }

	}
	zFrac += dzdxBig;
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_TRUE;
}

/* Fast path: 16-bit RGB, flat shaded, depth tested, anti-aliased */
GLboolean __glDrawAAStippledLine_RGB_16_Flat_LESS16_SA_MSA(__GLcontext *gc)
{
    /* This will happen only if an anti-aliased line is scissored, so we
     * won't worry about optimizing this path.
     */
    if (__glAntiAliasStippledLine(gc))
	return GL_TRUE;
    if (__glDepthTestStippledLine(gc))
	return GL_TRUE;
    return __glStoreStippledLine(gc);
}

/* Fast path: 16-bit RGB, flat shaded, depth tested, anti-aliased */
GLboolean __glDrawAALine_RGB_16_Flat_LESS32_SA_MSA(__GLcontext *gc)
{
    __GLfloat length;	/* Dist along length */
    __GLfloat width;	/* Dist along width */
    GLint fraction, dfraction;
    __GLfloat dlLittle, dlBig;
    __GLfloat ddLittle, ddBig;
    __GLcolor *cp;
    __GLfloat coverage;
    __GLfloat lineWidth;
    __GLfloat lineLength;
    GLint count;
    GLint w;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    /* Depth declarations */
    __GLzValue zFrac, z, *zfb;
    GLint dzdx, dzdxBig;
    GLuint cnt;
    GLint numFracBits;
    GLint xLittle, xBig, yLittle, yBig;
    GLint dzpLittle, dzpBig;

    /* Store declarations */
    __GLcolorBuffer *cfb;
    GLint fbLittle, fbBig;
    GLushort *fp;
    __GLfloat fr, fg, fb, fa, alpha;
    GLint x, y, rs, gs, bs;
    GLuint enables;

    w = gc->polygon.shader.length;

    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    cp = gc->polygon.shader.colors;

    dlLittle = gc->line.options.dlLittle;
    dlBig = gc->line.options.dlBig;
    ddLittle = gc->line.options.ddLittle;
    ddBig = gc->line.options.ddBig;

    length = gc->line.options.plength;
    width = gc->line.options.pwidth;
    lineLength = gc->line.options.realLength - __glHalf;
    lineWidth = __glHalf * gc->state.line.smoothWidth - __glHalf;


    /* Depth initialization */
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    zfb = __GL_DEPTH_ADDR(&gc->depthBuffer, (__GLzValue*),
	    gc->line.options.xStart, gc->line.options.yStart);
    dzpLittle = xLittle + yLittle * gc->depthBuffer.buf.outerWidth;
    dzpBig = xBig + yBig * gc->depthBuffer.buf.outerWidth;

    numFracBits = gc->depthBuffer.numFracBits;
    cnt = gc->depthBuffer.cnt;
    zFrac = gc->polygon.shader.frag.z;
    dzdx = gc->polygon.shader.dzdx >> numFracBits;
    dzdxBig = gc->polygon.shader.dzdxBig;

    /* Store initializations */
    enables = gc->state.enables.general;
    cfb = gc->polygon.shader.cfb;
    fbBig = xBig + yBig * cfb->buf.outerWidth;
    fbLittle = xLittle + yLittle * cfb->buf.outerWidth;
    rs = cfb->redShift;
    gs = cfb->greenShift;
    bs = cfb->blueShift;
    fr = gc->polygon.shader.frag.color.r;
    fg = gc->polygon.shader.frag.color.g;
    fb = gc->polygon.shader.frag.color.b;
    fa = gc->polygon.shader.frag.color.a * gc->frontBuffer.oneOverAlphaScale;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);

    /* These are hereafter used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;

    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	z = (zFrac >> numFracBits) + cnt;

	while (--count >= 0) {
	    /* This normally goes after the alpha calculation below,
	     * but can safely be moved here as an optimization.
	     */
	    if ( (GLuint) z < (GLuint) *zfb ) {

		/* Coverage for sides */
		if (width > lineWidth) {
		    coverage = lineWidth - width + __glOne;
		    if (coverage <= __glZero) {
			goto next_pixel;
		    }
		} else if (width < -lineWidth) {
		    coverage = width + lineWidth + __glOne;
		    if (coverage <= __glZero) {
			goto next_pixel;
		    }
		} else {
		    coverage = __glOne;
		}

		/* Coverage for start, end */
		if (length < __glHalf) {
		    coverage *= length + __glHalf;
		    if (coverage <= __glZero) {
			goto next_pixel;
		    }
		} else if (length > lineLength) {
		    coverage *= lineLength - length + __glOne;
		    if (coverage <= __glZero) {
			goto next_pixel;
		    }
		} 

		alpha = (fa == __glOne) ? coverage : fa * coverage;
		*zfb = z;

		/* Store the pixel */
	        {
		    unsigned int r, g, b;
		    __GLfloat dr, dg, db;
		    __GLfloat msa, rr, rg, rb;
		    GLushort value;

		    if (__glOne == alpha) {
			rr = fr;
			rg = fg;
			rb = fb;
		    } else {
			value = *fp;
			dr = (value & gc->modes.redMask) >> rs;
			dg = (value & gc->modes.greenMask) >> gs;
			db = (value & gc->modes.blueMask) >> bs;

			msa = __glOne - alpha;

			rr = fr * alpha + dr * msa;
			rg = fg * alpha + dg * msa;
			rb = fb * alpha + db * msa;
		    }

		    if (enables & __GL_DITHER_ENABLE) {
			GLuint ix = ((x & 3) | (y & 12));
			__GLfloat inc = __glFastDitherTable[ix];
			rr += inc;
			rg += inc;
			rb += inc;
		    }

		    SFloat2Int(rr, r);
		    SFloat2Int(rg, g);
		    SFloat2Int(rb, b);

		    *fp = (r << rs) | (g << gs) | (b << bs);
		}
	    }
next_pixel:
	    z += dzdx;
	    fraction += dfraction;
	    if (fraction < 0) {
		fraction &= ~0x80000000;
		length += dlBig;
		width += ddBig;
		zfb += dzpBig;
		x += xBig;
		y += yBig;
		fp += fbBig;
	    } else {
		length += dlLittle;
		width += ddLittle;
		zfb += dzpLittle;
		x += xLittle;
		y += yLittle;
		fp += fbLittle;
	    }

	}
	zFrac += dzdxBig;
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_TRUE;
}

/* Fast path: 16-bit RGB, flat shaded, depth tested, anti-aliased */
GLboolean __glDrawAAStippledLine_RGB_16_Flat_LESS32_SA_MSA(__GLcontext *gc)
{
    /* This will happen only if an anti-aliased line is scissored, so we
     * won't worry about optimizing this path.
     */
    if (__glAntiAliasStippledLine(gc))
	return GL_TRUE;
    if (__glDepthTestStippledLine(gc))
	return GL_TRUE;
    return __glStoreStippledLine(gc);
}

/* Fast path: 16-bit RGB, flat shaded, anti-aliased */
GLboolean __glDrawAALine_RGB_16_Flat_SA_ONE(__GLcontext *gc)
{
    __GLfloat length;	/* Dist along length */
    __GLfloat width;	/* Dist along width */
    GLint fraction, dfraction;
    __GLfloat dlLittle, dlBig;
    __GLfloat ddLittle, ddBig;
    __GLfloat coverage;
    __GLfloat lineWidth;
    __GLfloat lineLength;
    GLint w;

    /* Store declarations */
    GLint fbLittle, fbBig, xLittle, yLittle, xBig, yBig;
    __GLcolorBuffer *cfb;
    GLushort *fp;
    __GLfloat fr, fg, fb, fa, alpha;
    GLint x, y, rs, gs, bs;
    GLuint enables;

    w = gc->polygon.shader.length;

    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    dlLittle = gc->line.options.dlLittle;
    dlBig = gc->line.options.dlBig;
    ddLittle = gc->line.options.ddLittle;
    ddBig = gc->line.options.ddBig;

    length = gc->line.options.plength;
    width = gc->line.options.pwidth;
    lineLength = gc->line.options.realLength - __glHalf;
    lineWidth = __glHalf * gc->state.line.smoothWidth - __glHalf;


    /* Store initializations */
    enables = gc->state.enables.general;
    cfb = gc->polygon.shader.cfb;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    fbBig = xBig + yBig * cfb->buf.outerWidth;
    fbLittle = xLittle + yLittle * cfb->buf.outerWidth;
    rs = cfb->redShift;
    gs = cfb->greenShift;
    bs = cfb->blueShift;
    fr = gc->polygon.shader.frag.color.r;
    fg = gc->polygon.shader.frag.color.g;
    fb = gc->polygon.shader.frag.color.b;
    fa = gc->polygon.shader.frag.color.a * gc->frontBuffer.oneOverAlphaScale;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);

    /* These are hereafter used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;

    while (--w >= 0) {
	/* Coverage for sides */
	if (width > lineWidth) {
	    coverage = lineWidth - width + __glOne;
	    if (coverage <= __glZero) {
		goto next_pixel;
	    }
	} else if (width < -lineWidth) {
	    coverage = width + lineWidth + __glOne;
	    if (coverage <= __glZero) {
		goto next_pixel;
	    }
	} else {
	    coverage = __glOne;
	}

	/* Coverage for start, end */
	if (length < __glHalf) {
	    coverage *= length + __glHalf;
	    if (coverage <= __glZero) {
		goto next_pixel;
	    }
	} else if (length > lineLength) {
	    coverage *= lineLength - length + __glOne;
	    if (coverage <= __glZero) {
		goto next_pixel;
	    }
	} 

	alpha = (fa == __glOne) ? coverage : fa * coverage;

	/* Store the pixel */
        {
	    unsigned int r, g, b;
	    __GLfloat dr, dg, db;
	    __GLfloat rr, rg, rb;
	    GLushort value;

	    value = *fp;
	    dr = (value & gc->modes.redMask) >> rs;
	    dg = (value & gc->modes.greenMask) >> gs;
	    db = (value & gc->modes.blueMask) >> bs;

	    if (__glOne == alpha) {
		rr = fr + dr;
		rg = fg + dg;
		rb = fb + db;
	    } else {
		rr = fr * alpha + dr;
		rg = fg * alpha + dg;
		rb = fb * alpha + db;
	    }
	    if (rr > gc->frontBuffer.redScale)
		rr = gc->frontBuffer.redScale;
	    if (rg > gc->frontBuffer.greenScale)
		rg = gc->frontBuffer.greenScale;
	    if (rb > gc->frontBuffer.blueScale)
		rb = gc->frontBuffer.blueScale;

	    if (enables & __GL_DITHER_ENABLE) {
		GLuint ix = ((x & 3) | (y & 12));
		__GLfloat inc = __glFastDitherTable[ix];
		rr += inc;
		rg += inc;
		rb += inc;
	    }

	    SFloat2Int(rr, r);
	    SFloat2Int(rg, g);
	    SFloat2Int(rb, b);

	    *fp = (r << rs) | (g << gs) | (b << bs);
	}
next_pixel:
	fraction += dfraction;
	if (fraction < 0) {
	    fraction &= ~0x80000000;
	    length += dlBig;
	    width += ddBig;
	    x += xBig;
	    y += yBig;
	    fp += fbBig;
	} else {
	    length += dlLittle;
	    width += ddLittle;
	    x += xLittle;
	    y += yLittle;
	    fp += fbLittle;
	}
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_TRUE;
}

/* Fast path: 16-bit RGB, flat shaded, anti-aliased */
GLboolean __glDrawAAStippledLine_RGB_16_Flat_SA_ONE(__GLcontext *gc)
{
    /* This will happen only if an anti-aliased line is scissored, so we
     * won't worry about optimizing this path.
     */
    if (__glAntiAliasStippledLine(gc))
	return GL_TRUE;
    return __glStoreStippledLine(gc);
}

/* Fast path: 16-bit RGB, flat shaded, anti-aliased */
GLboolean __glDrawAALine_RGB_16_Smooth_SA_ONE(__GLcontext *gc)
{
    __GLfloat length;	/* Dist along length */
    __GLfloat width;	/* Dist along width */
    GLint fraction, dfraction;
    __GLfloat dlLittle, dlBig;
    __GLfloat ddLittle, ddBig;
    __GLfloat coverage;
    __GLfloat lineWidth;
    __GLfloat lineLength;
    GLint w;

    /* Store declarations */
    GLint fbLittle, fbBig, xLittle, yLittle, xBig, yBig;
    __GLcolorBuffer *cfb;
    GLushort *fp;
    __GLfloat fr, fg, fb, fa, alpha;
    __GLfloat drdx, dgdx, dbdx, dadx;
    GLint x, y, rs, gs, bs;
    GLuint enables;

    w = gc->polygon.shader.length;

    fraction = gc->line.options.fraction;
    dfraction = gc->line.options.dfraction;

    dlLittle = gc->line.options.dlLittle;
    dlBig = gc->line.options.dlBig;
    ddLittle = gc->line.options.ddLittle;
    ddBig = gc->line.options.ddBig;

    length = gc->line.options.plength;
    width = gc->line.options.pwidth;
    lineLength = gc->line.options.realLength - __glHalf;
    lineWidth = __glHalf * gc->state.line.smoothWidth - __glHalf;


    /* Store initializations */
    enables = gc->state.enables.general;
    cfb = gc->polygon.shader.cfb;
    xBig = gc->line.options.xBig;
    yBig = gc->line.options.yBig;
    xLittle = gc->line.options.xLittle;
    yLittle = gc->line.options.yLittle;
    fbBig = xBig + yBig * cfb->buf.outerWidth;
    fbLittle = xLittle + yLittle * cfb->buf.outerWidth;
    rs = cfb->redShift;
    gs = cfb->greenShift;
    bs = cfb->blueShift;
    fr = gc->polygon.shader.frag.color.r;
    fg = gc->polygon.shader.frag.color.g;
    fb = gc->polygon.shader.frag.color.b;
    fa = gc->polygon.shader.frag.color.a * gc->frontBuffer.oneOverAlphaScale;
    drdx = gc->polygon.shader.drdx;
    dgdx = gc->polygon.shader.dgdx;
    dbdx = gc->polygon.shader.dbdx;
    dadx = gc->polygon.shader.dadx * gc->frontBuffer.oneOverAlphaScale;

    __GL_LOCK_BUFFERS(gc);

    x = gc->line.options.xStart;
    y = gc->line.options.yStart;
    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);

    /* These are hereafter used only for dithering */
    y <<= 2;
    yBig <<= 2;
    yLittle <<= 2;

    while (--w >= 0) {
	/* Coverage for sides */
	if (width > lineWidth) {
	    coverage = lineWidth - width + __glOne;
	    if (coverage <= __glZero) {
		goto next_pixel;
	    }
	} else if (width < -lineWidth) {
	    coverage = width + lineWidth + __glOne;
	    if (coverage <= __glZero) {
		goto next_pixel;
	    }
	} else {
	    coverage = __glOne;
	}

	/* Coverage for start, end */
	if (length < __glHalf) {
	    coverage *= length + __glHalf;
	    if (coverage <= __glZero) {
		goto next_pixel;
	    }
	} else if (length > lineLength) {
	    coverage *= lineLength - length + __glOne;
	    if (coverage <= __glZero) {
		goto next_pixel;
	    }
	} 

	alpha = (fa == __glOne) ? coverage : fa * coverage;

	/* Store the pixel */
        {
	    unsigned int r, g, b;
	    __GLfloat dr, dg, db;
	    __GLfloat rr, rg, rb;
	    GLushort value;

	    value = *fp;
	    dr = (value & gc->modes.redMask) >> rs;
	    dg = (value & gc->modes.greenMask) >> gs;
	    db = (value & gc->modes.blueMask) >> bs;

	    if (__glOne == alpha) {
		rr = fr + dr;
		rg = fg + dg;
		rb = fb + db;
	    } else {
		rr = fr * alpha + dr;
		rg = fg * alpha + dg;
		rb = fb * alpha + db;
	    }
	    if (rr > gc->frontBuffer.redScale)
		rr = gc->frontBuffer.redScale;
	    if (rg > gc->frontBuffer.greenScale)
		rg = gc->frontBuffer.greenScale;
	    if (rb > gc->frontBuffer.blueScale)
		rb = gc->frontBuffer.blueScale;

	    if (enables & __GL_DITHER_ENABLE) {
		GLuint ix = ((x & 3) | (y & 12));
		__GLfloat inc = __glFastDitherTable[ix];
		rr += inc;
		rg += inc;
		rb += inc;
	    }

	    SFloat2Int(rr, r);
	    SFloat2Int(rg, g);
	    SFloat2Int(rb, b);

	    *fp = (r << rs) | (g << gs) | (b << bs);
	}
next_pixel:
	fr += drdx;
	fg += drdx;
	fb += drdx;
	fa += drdx;
	fraction += dfraction;
	if (fraction < 0) {
	    fraction &= ~0x80000000;
	    length += dlBig;
	    width += ddBig;
	    x += xBig;
	    y += yBig;
	    fp += fbBig;
	} else {
	    length += dlLittle;
	    width += ddLittle;
	    x += xLittle;
	    y += yLittle;
	    fp += fbLittle;
	}
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_TRUE;
}

/* Fast path: 16-bit RGB, flat shaded, anti-aliased */
GLboolean __glDrawAAStippledLine_RGB_16_Smooth_SA_ONE(__GLcontext *gc)
{
    /* This will happen only if an anti-aliased line is scissored, so we
     * won't worry about optimizing this path.
     */
    if (__glAntiAliasStippledLine(gc))
	return GL_TRUE;
    return __glStoreStippledLine(gc);
}

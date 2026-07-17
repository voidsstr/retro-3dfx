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
#include "context.h"
#include "global.h"
#include "fmemclr.h"
#include "fmacros.h"
#ifdef __GL_PC_RAST
#include "fr_fbtype.h"
#endif

extern __GLfloat __glFastDitherTable[16];

#if 0
/* No Dither, No blend, No Write, No Nothing */
static void Store_NOT(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
}
#endif

/* 8 bit generic Store */
static void Store_8(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLubyte *fp;
    __GLcolor blendColor;
    const __GLcolor *color;
    __GLfloat inc;
    GLubyte fbcolor, result;
    GLint ix;

    __GL_LOCK_BUFFERS(gc);

    if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		!__glTestOwnership(gc, frag->x, frag->y)) {
	__GL_UNLOCK_BUFFERS(gc);
	return;
    }

    fp = __GL_FB_ADDRESS(cfb, (GLubyte *), frag->x, frag->y);
    if (modeFlags & __GL_SHADE_DITHER) {
	ix = __GL_DITHER_INDEX(frag->x, frag->y);
	inc = ((__glDitherTable[ix] << 1) + 1) / 
	    (__GLfloat) (2 * __GL_DITHER_PRECISION);
    } else {
	inc = __glHalf;
    }
    if (modeFlags & __GL_SHADE_BLEND) {
	color = &blendColor;
	(*gc->procs.blend)(gc, cfb, frag, &blendColor);
    } else {
	color = &(frag->color);
    }
    result = (((GLuint) (color->r + inc)) << cfb->redShift) |
	    (((GLuint) (color->g + inc)) << cfb->greenShift) |
	    (((GLuint) (color->b + inc)) << cfb->blueShift);

    fbcolor = *fp;
    if (modeFlags & __GL_SHADE_LOGICOP) {
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

    __GL_UNLOCK_BUFFERS(gc);
}

/* 16 bit generic Store */
static void Store_16(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLushort *fp;
    __GLcolor blendColor;
    const __GLcolor *color;
    __GLfloat inc;
    GLushort fbcolor, result;
    GLint ix;

    __GL_LOCK_BUFFERS(gc);

    if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		!__glTestOwnership(gc, frag->x, frag->y)) {
	__GL_UNLOCK_BUFFERS(gc);
	return;
    }

    fp = __GL_FB_ADDRESS(cfb, (GLushort *), frag->x, frag->y);
    if (modeFlags & __GL_SHADE_DITHER) {
	ix = __GL_DITHER_INDEX(frag->x, frag->y);
	inc = ((__glDitherTable[ix] << 1) + 1) / 
	    (__GLfloat) (2 * __GL_DITHER_PRECISION);
    } else {
	inc = __glHalf;
    }
    if (modeFlags & __GL_SHADE_BLEND) {
	color = &blendColor;
	(*gc->procs.blend)(gc, cfb, frag, &blendColor);
    } else {
	color = &(frag->color);
    }
    result = (((GLuint) (color->r + inc)) << cfb->redShift) |
	    (((GLuint) (color->g + inc)) << cfb->greenShift) |
	    (((GLuint) (color->b + inc)) << cfb->blueShift);

    fbcolor = *fp;
    if (modeFlags & __GL_SHADE_LOGICOP) {
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

    __GL_UNLOCK_BUFFERS(gc);
}

/* 24 bit generic Store */
static void Store_24(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLubyte *fp;
    __GLcolor blendColor;
    const __GLcolor *color;
    __GLfloat inc;
    GLuint fbcolor, result;
    GLint ix;
    GLuint stride;

    __GL_LOCK_BUFFERS(gc);

    if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		!__glTestOwnership(gc, frag->x, frag->y)) {
	__GL_UNLOCK_BUFFERS(gc);
	return;
    }

    stride = ((cfb->buf.outerWidth * 3) + 3) & ~3;
    fp = ((GLubyte *) cfb->buf.base) +
	(frag->y - gc->constants.viewportYAdjust) * stride +
	(frag->x - gc->constants.viewportXAdjust) * 3;

    if (modeFlags & __GL_SHADE_DITHER) {
	ix = __GL_DITHER_INDEX(frag->x, frag->y);
	inc = ((__glDitherTable[ix] << 1) + 1) / 
	    (__GLfloat) (2 * __GL_DITHER_PRECISION);
    } else {
	inc = __glHalf;
    }
    if (modeFlags & __GL_SHADE_BLEND) {
	color = &blendColor;
	(*gc->procs.blend)(gc, cfb, frag, &blendColor);
    } else {
	color = &(frag->color);
    }
    result = (((GLuint) (color->r + inc)) << cfb->redShift) |
	    (((GLuint) (color->g + inc)) << cfb->greenShift) |
	    (((GLuint) (color->b + inc)) << cfb->blueShift);

    fbcolor = (((GLuint) fp[2]) << cfb->redShift) |
	    (((GLuint) fp[1]) << cfb->greenShift) |
	    (((GLuint) fp[0]) << cfb->blueShift);
    if (modeFlags & __GL_SHADE_LOGICOP) {
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

    result = (fbcolor & cfb->destMask) | (result & cfb->sourceMask);
    fp[2] = (GLubyte) (result >> cfb->redShift);
    fp[1] = (GLubyte) (result >> cfb->greenShift);
    fp[0] = (GLubyte) (result >> cfb->blueShift);

    __GL_UNLOCK_BUFFERS(gc);
}

/* 32 bit generic Store with alpha */
static void Store_32A(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint *fp;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLuint fbcolor, result;

    /*
    ** No dithering for 32 bit pixmaps.  It would probably be a waste
    ** of time.
    */

    __GL_LOCK_BUFFERS(gc);

    if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		!__glTestOwnership(gc, frag->x, frag->y)) {
	__GL_UNLOCK_BUFFERS(gc);
	return;
    }

    fp = __GL_FB_ADDRESS(cfb, (GLuint *), frag->x, frag->y);
    if (modeFlags & __GL_SHADE_BLEND) {
	color = &blendColor;
	(*gc->procs.blend)(gc, cfb, frag, &blendColor);
    } else {
	color = &(frag->color);
    }
    result = (((GLuint) (color->r + __glHalf)) << cfb->redShift) |
	    (((GLuint) (color->g + __glHalf)) << cfb->greenShift) |
	    (((GLuint) (color->b + __glHalf)) << cfb->blueShift) |
	    (((GLuint) (color->a + __glHalf)) << cfb->alphaShift);

    fbcolor = *fp;
    if (modeFlags & __GL_SHADE_LOGICOP) {
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

    __GL_UNLOCK_BUFFERS(gc);
}

/* 32 bit generic Store without alpha */
static void Store_32(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint *fp;
    __GLcolor blendColor;
    const __GLcolor *color;
    GLuint fbcolor, result;

    /*
    ** No dithering for 32 bit pixmaps.  It would probably be a waste
    ** of time.
    */

    __GL_LOCK_BUFFERS(gc);

    if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		!__glTestOwnership(gc, frag->x, frag->y)) {
	__GL_UNLOCK_BUFFERS(gc);
	return;
    }

    fp = __GL_FB_ADDRESS(cfb, (GLuint *), frag->x, frag->y);
    if (modeFlags & __GL_SHADE_BLEND) {
	color = &blendColor;
	(*gc->procs.blend)(gc, cfb, frag, &blendColor);
    } else {
	color = &(frag->color);
    }
    result = (((GLuint) (color->r + __glHalf)) << cfb->redShift) |
	    (((GLuint) (color->g + __glHalf)) << cfb->greenShift) |
	    (((GLuint) (color->b + __glHalf)) << cfb->blueShift);

    fbcolor = *fp;
    if (modeFlags & __GL_SHADE_LOGICOP) {
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

    __GL_UNLOCK_BUFFERS(gc);
}

static void Fetch_8(__GLcolorBuffer *cfb, GLint x, GLint y,
		    __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLubyte *fp;
    GLubyte value;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLubyte *), x, y);
    value = *fp;
    result->r = (value & gc->modes.redMask) >> cfb->redShift;
    result->g = (value & gc->modes.greenMask) >> cfb->greenShift;
    result->b = (value & gc->modes.blueMask) >> cfb->blueShift;
    result->a = cfb->alphaScale;

    __GL_UNLOCK_BUFFERS(gc);
}

static void Fetch_16(__GLcolorBuffer *cfb, GLint x, GLint y,
		     __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLushort *fp;
    GLushort value;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);
    value = *fp;
    result->r = (value & gc->modes.redMask) >> cfb->redShift;
    result->g = (value & gc->modes.greenMask) >> cfb->greenShift;
    result->b = (value & gc->modes.blueMask) >> cfb->blueShift;
    result->a = cfb->alphaScale;

    __GL_UNLOCK_BUFFERS(gc);
}

static void Fetch_24(__GLcolorBuffer *cfb, GLint x, GLint y,
		     __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLubyte *fp;
    GLuint stride;

    __GL_LOCK_BUFFERS(gc);

    stride = ((cfb->buf.outerWidth * 3) + 3) & ~3;
    fp = ((GLubyte *) cfb->buf.base) +
	(y - gc->constants.viewportYAdjust) * stride +
	(x - gc->constants.viewportXAdjust) * 3;

    result->r = fp[2];
    result->g = fp[1];
    result->b = fp[0];
    result->a = cfb->alphaScale;

    __GL_UNLOCK_BUFFERS(gc);
}

/* 32 bit generic fetch with alpha */
static void Fetch_32A(__GLcolorBuffer *cfb, GLint x, GLint y,
		      __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint *fp;
    GLuint value;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLuint *), x, y);
    value = *fp;
    result->r = (value & gc->modes.redMask) >> cfb->redShift;
    result->g = (value & gc->modes.greenMask) >> cfb->greenShift;
    result->b = (value & gc->modes.blueMask) >> cfb->blueShift;
    result->a = (value & gc->modes.alphaMask) >> cfb->alphaShift;

    __GL_UNLOCK_BUFFERS(gc);
}

/* 32 bit generic fetch without alpha */
static void Fetch_32(__GLcolorBuffer *cfb, GLint x, GLint y,
		     __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint *fp;
    GLuint value;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLuint *), x, y);
    value = *fp;
    result->r = (value & gc->modes.redMask) >> cfb->redShift;
    result->g = (value & gc->modes.greenMask) >> cfb->greenShift;
    result->b = (value & gc->modes.blueMask) >> cfb->blueShift;
    result->a = cfb->alphaScale;

    __GL_UNLOCK_BUFFERS(gc);
}

static void ClearRect(__GLcolorBuffer *cfb,
				GLint x, GLint y, GLint x1, GLint y1)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLint i, j;
    __GLfragment frag;

    /* Turn off blending and logicop so store proc doesn't do it */
    gc->polygon.shader.modeFlags =
	modeFlags & ~(__GL_SHADE_BLEND|__GL_SHADE_LOGICOP);

    frag.color.r = gc->state.raster.clear.r * gc->frontBuffer.redScale;
    frag.color.g = gc->state.raster.clear.g * gc->frontBuffer.greenScale;
    frag.color.b = gc->state.raster.clear.b * gc->frontBuffer.blueScale;
    frag.color.a = gc->state.raster.clear.a * gc->frontBuffer.alphaScale;

    __GL_LOCK_BUFFERS(gc);

    for (j = y; j < y1; j++) {
	frag.y = j;

	for (i = x; i < x1; i++) {
	    frag.x = i;
		
	    (*cfb->store)(cfb, &frag);
	}
    }

    __GL_UNLOCK_BUFFERS(gc);

    gc->polygon.shader.modeFlags = modeFlags;
}

#ifdef _WIN32

static void ClearRect_24(__GLcolorBuffer *cfb,
			 GLint x0, GLint y, GLint x1, GLint y1)
{
    __GLcontext *gc = cfb->buf.gc;
    GLubyte *fp, *rfp;
    __GLfloat fr, fg, fb;
    unsigned int r, g, b;
    unsigned int color;
    GLuint pack0, pack1, pack2;
    int      stride;
    int      width, height;
    int      dword_width, pre_step_pixels, fixup_pixels;
    
    fr = gc->state.raster.clear.r * gc->frontBuffer.redScale;
    fg = gc->state.raster.clear.g * gc->frontBuffer.greenScale;
    fb = gc->state.raster.clear.b * gc->frontBuffer.blueScale;
                
    Float2Int(fr, r);
    Float2Int(fg, g);
    Float2Int(fb, b);

    color = (r << cfb->redShift) |
	    (g << cfb->greenShift) |
	    (b << cfb->blueShift);

    width  = x1 - x0;
    height = y1 - y;

    /*
    ** Setup packed DWORDS
    */
#ifdef WIN32
    pack0 = (color << 24) | color;
    pack1 = (color << 16) | (color >> 8);
    pack2 = (color << 8) | (color >> 16);
#else
    pack0 = (color << 8) | (color >> 16);
    pack1 = (color << 16) | (color >> 8);
    pack2 = (color << 24) | color;
#endif

    __GL_LOCK_BUFFERS(gc);
    
    stride = ((cfb->buf.outerWidth * 3) + 3) & ~3;
    fp = ((GLubyte *) cfb->buf.base) +
	(y - gc->constants.viewportYAdjust) * stride +
	(x0 - gc->constants.viewportXAdjust) * 3;

    while (height) {
	rfp = fp;

	pre_step_pixels = (((unsigned) rfp) & 3);
	if (pre_step_pixels > width) {
	    pre_step_pixels = width;
	    dword_width = 0;
	    fixup_pixels = 0;
	} else {
	    dword_width     = ( width - pre_step_pixels ) >> 2;
	    fixup_pixels    = width - (dword_width*4 + pre_step_pixels);
	}

	while (pre_step_pixels) {
	    rfp[0] = b;
	    rfp[1] = g;
	    rfp[2] = r;
	    rfp += 3;
	    pre_step_pixels--;
	}

	while (dword_width) {
	    ((GLuint *)rfp)[0] = pack0;
	    ((GLuint *)rfp)[1] = pack1;
	    ((GLuint *)rfp)[2] = pack2;

	    rfp += 12;
	    dword_width--;
	}

	while (fixup_pixels) {
	    rfp[0] = b;
	    rfp[1] = g;
	    rfp[2] = r;

	    rfp += 3;
	    fixup_pixels--;
	}

	fp += stride;
	height--;
    }

    __GL_UNLOCK_BUFFERS(gc);
}

static void ClearRect_16_Dither(__GLcolorBuffer *cfb,
					GLint x0, GLint y, GLint x1, GLint y1)
{
    __GLcontext *gc = cfb->buf.gc;
    GLint x;
    GLushort *fp, *rfp;
    __GLfloat fr, fg, fb;
    __GLfloat ftmp, inc;
    int       row, col;
    unsigned int r, g, b;
    GLushort result;
    GLushort dither_colors[4][7];
    int      stride;
    int      width, height;

    int      qword_width, pre_step_pixels, fixup_pixels;
    
    fr = gc->state.raster.clear.r * gc->frontBuffer.redScale;
    fg = gc->state.raster.clear.g * gc->frontBuffer.greenScale;
    fb = gc->state.raster.clear.b * gc->frontBuffer.blueScale;

    width  = x1 - x0;
    height = y1 - y;

    /*
    ** precompute dither table
    */
    for ( row = 0; row < 4; row++ )
    {
        __GLfloat *dither = __glFastDitherTable + ((row<<2) & 12);

        for ( col = 0; col < 7; col++ )
        {
            inc = dither[col&3];
            ftmp = fr + inc;
            SFloat2Int( ftmp, r );
            ftmp = fg + inc;
            SFloat2Int( ftmp, g );
            ftmp = fb + inc;
            SFloat2Int( ftmp, b );

            result =
                (r << cfb->redShift) |
                (g << cfb->greenShift) |
                (b << cfb->blueShift);

            dither_colors[row][col] = result;
        }
    }
    
    __GL_LOCK_BUFFERS(gc);
    
    rfp = __GL_FB_ADDRESS(cfb, (GLushort *), x0, y);

    /*
    ** characterize our clear destination alignment
    ** properties.  We want to have a QWORD aligned 
    ** width and a DWORD aligned starting address
    */
    pre_step_pixels = ( ( 4 - ( ( ( unsigned long ) rfp ) & 3 ) ) & 3 ) >> 1;
    if (pre_step_pixels > width) {
	pre_step_pixels = width;
	qword_width = 0;
	fixup_pixels = 0;
    } else {
	qword_width     = ( width - pre_step_pixels ) >> 2;
	fixup_pixels    = width - (qword_width*4 + pre_step_pixels);
    }

    stride = cfb->buf.outerWidth;

    /*
    ** CASE 1:  No adjustments necessary
    **          This should be the most common case, e.g. when
    **          rendering to a backbuffer.
    */
    if ( !pre_step_pixels && !fixup_pixels && qword_width )
    {
        while ( height )
        {
            long first_pair  = ( dither_colors[y&3][(x0+0)&3] << 16 ) |
                               ( dither_colors[y&3][(x0+1)&3] );
            long second_pair = ( dither_colors[y&3][(x0+2)&3] << 16 ) | 
                               ( dither_colors[y&3][(x0+3)&3] );

            {
            __asm mov edi, rfp
            __asm mov eax, first_pair
            __asm mov ebx, second_pair
            __asm mov ecx, qword_width
top_x_loop:
            __asm mov [edi],   eax
            __asm mov [edi+4], ebx
            __asm add edi, 8
            __asm dec ecx
            __asm jnz top_x_loop
            }

            rfp += stride;
            --height;
            ++y;
        }
    }
    /*
    ** CASE 2: Prestepping to DWORD boundary required, but no
    **         post-scanline fixup is required
    */
    else if ( pre_step_pixels && !fixup_pixels && qword_width )
    {
        while ( height )
        {

            /*
            ** write first pixel
            */
            if ( pre_step_pixels )
                *rfp = dither_colors[y&3][x0&3];

            /*
            ** write rest of pixels
            */

            {
                long first_pair  = ( dither_colors[y&3][(x0+1)&3] << 16 ) |
                    ( dither_colors[y&3][(x0+2)&3] );
                long second_pair = ( dither_colors[y&3][(x0+3)&3] << 16 ) | 
                    ( dither_colors[y&3][(x0+4)&3] );
                
                __asm mov edi, rfp
                __asm mov eax, first_pair
                __asm add edi, 2
                __asm mov ebx, second_pair
                __asm mov ecx, qword_width
top_x_loop1:
                __asm mov [edi],   eax
                __asm mov [edi+4], ebx
                __asm add edi, 8
                __asm dec ecx
                __asm jnz top_x_loop1
               
            }

            rfp += stride;
            --height;
            ++y;
        }
    }
    /*
    ** CASE 3: Buffer is DWORD aligned, but there are some left
    **         over pixels to take care of
    */
    else if ( !pre_step_pixels && fixup_pixels && qword_width )
    {
        while ( height )
        {
            int fup = fixup_pixels, qww = qword_width;
            unsigned long  *buf = ( unsigned long * ) rfp;
            unsigned short *sbuf;

            long first_pair  = ( dither_colors[y&3][(x0+0)&3] << 16 ) |
                ( dither_colors[y&3][(x0+1)&3] );
            long second_pair = ( dither_colors[y&3][(x0+2)&3] << 16 ) | 
                ( dither_colors[y&3][(x0+3)&3] );

            while ( qww )
            {
                buf[0] = first_pair;
                buf[1] = second_pair;

                buf += 2;

                qww--;
            }

            sbuf = ( unsigned short * ) buf;
            while ( fup )
            {
                *sbuf++ = dither_colors[y&3][(x0+qword_width*4+3-fup)&3];

                --fup;
            }

            rfp += stride;
            height--;
            ++y;
        }
    }
    /*
    ** Case 4:  Buffer is not DWORD aligned and the scan line is
    **          not QWORD aligned.
    */
    else if ( pre_step_pixels && fixup_pixels && qword_width )
    {
        while ( height )
        {
            int fup = fixup_pixels, qww = qword_width;
            unsigned long  *buf = ( unsigned long * ) ( ( unsigned short * ) rfp + 1 );
            unsigned short *sbuf;

            long first_pair  = ( dither_colors[y&3][(x0+1)&3] << 16 ) |
                ( dither_colors[y&3][(x0+2)&3] );
            long second_pair = ( dither_colors[y&3][(x0+3)&3] << 16 ) | 
                ( dither_colors[y&3][(x0+4)&3] );

            if ( pre_step_pixels )
                *( unsigned short * ) rfp = dither_colors[y&3][x0&3];

            while ( qww )
            {
                buf[0] = first_pair;
                buf[1] = second_pair;

                buf += 2;

                qww--;
            }

            sbuf = ( unsigned short * ) buf;
            while ( fup )
            {

                *sbuf++ = dither_colors[y&3][(x0+qword_width*4+3-fup)&3];

                --fup;
            }

            rfp += stride;
            --height;
            ++y;
        }
    }
    /*
    ** buffer is less than 8-bytes wide
    */
    else if ( !qword_width )
    {
        for (; y < y1; y++, rfp += cfb->buf.outerWidth) {
            __GLfloat *dither = __glFastDitherTable + ((y<<2) & 12);
            
            for (x = x0, fp = rfp; x < x1; x++, fp++) {
                __GLfloat ftmp, inc;
                unsigned int r, g, b;
                GLushort result;
                
                inc = dither[x&3];
                ftmp = fr + inc;
                SFloat2Int(ftmp, r);
                ftmp = fg + inc;
                SFloat2Int(ftmp, g);
                ftmp = fb + inc;
                SFloat2Int(ftmp, b);
                
                result =
                    (r << cfb->redShift) |
                    (g << cfb->greenShift) |
                    (b << cfb->blueShift);
                
                *fp = result;
            }
        }
    }

    __GL_UNLOCK_BUFFERS(gc);
}

static void ClearRect_8_Dither(__GLcolorBuffer *cfb,
					GLint x0, GLint y, GLint x1, GLint y1)
{
    __GLcontext *gc = cfb->buf.gc;
    GLint x;
    GLubyte  *fp, *rfp;
    __GLfloat fr, fg, fb;
    __GLfloat ftmp, inc;
    int       row, col;
    unsigned int r, g, b;
    GLubyte  result;
    GLubyte  dither_colors[4][7];
    int      stride;
    int      width, height;

    int      qword_width, pre_step_pixels, fixup_pixels;
    
    fr = gc->state.raster.clear.r * gc->frontBuffer.redScale;
    fg = gc->state.raster.clear.g * gc->frontBuffer.greenScale;
    fb = gc->state.raster.clear.b * gc->frontBuffer.blueScale;

    width  = x1 - x0;
    height = y1 - y;

    /*
    ** precompute dither table
    */
    for ( row = 0; row < 4; row++ )
    {
        __GLfloat *dither = __glFastDitherTable + ((row<<2) & 12);

        for ( col = 0; col < 7; col++ )
        {
            inc = dither[col&3];
            ftmp = fr + inc;
            SFloat2Int( ftmp, r );
            ftmp = fg + inc;
            SFloat2Int( ftmp, g );
            ftmp = fb + inc;
            SFloat2Int( ftmp, b );

            result =
                (r << cfb->redShift) |
                (g << cfb->greenShift) |
                (b << cfb->blueShift);

            dither_colors[row][col] = result;
        }
    }
    
    __GL_LOCK_BUFFERS(gc);
    
    rfp = __GL_FB_ADDRESS(cfb, (GLubyte * ), x0, y);

    /*
    ** characterize our clear destination alignment
    ** properties.  We want to have a QWORD aligned 
    ** width and a DWORD aligned starting address
    */
    pre_step_pixels = ( 8 - ( ( ( unsigned long ) rfp ) & 7 ) ) & 7;
    if (pre_step_pixels > width) {
	pre_step_pixels = width;
	fixup_pixels = 0;
	qword_width = 0;
    } else {
	fixup_pixels    = ( ( unsigned long ) ( rfp + width ) & 7 );
	qword_width     = ( width - pre_step_pixels - fixup_pixels ) >> 3;
    }

    stride = cfb->buf.outerWidth;

    /*
    ** CASE 1:  No adjustments necessary
    **          This should be the most common case, e.g. when
    **          rendering to a backbuffer.
    */
    if ( !pre_step_pixels && !fixup_pixels && qword_width )
    {
        while ( height )
        {
            long quad = ( dither_colors[y&3][(x0+0)&3] << 24 ) |
                        ( dither_colors[y&3][(x0+1)&3] << 16 ) |
                        ( dither_colors[y&3][(x0+2)&3] << 8 ) |
                        ( dither_colors[y&3][(x0+3)&3] );

            {
            __asm mov edi, rfp
            __asm mov eax, quad
            __asm mov ebx, quad
            __asm mov ecx, qword_width
top_x_loop:
            __asm mov [edi],   eax
            __asm mov [edi+4], ebx
            __asm add edi, 8
            __asm dec ecx
            __asm jnz top_x_loop
            }

            rfp += stride;
            --height;
            ++y;
        }
    }
    /*
    ** CASE 2: Prestepping to DWORD boundary required, but no
    **         post-scanline fixup is required
    */
    else if ( pre_step_pixels && !fixup_pixels && qword_width )
    {
        while ( height )
        {
            int i;
            GLubyte *bbuf = ( GLubyte * ) rfp;
            long quad = ( dither_colors[y&3][(x0+pre_step_pixels+3)&3] << 24 ) |
                        ( dither_colors[y&3][(x0+pre_step_pixels+2)&3] << 16 ) |
                        ( dither_colors[y&3][(x0+pre_step_pixels+1)&3] << 8 ) |
                        ( dither_colors[y&3][(x0+pre_step_pixels+0)&3] );

            /*
            ** write up to first three pixels
            */
            for ( i = 0; i < pre_step_pixels; i++ )
            {
               *bbuf++ = dither_colors[y&3][(x0+i)&3];
            }

            /*
            ** write rest of pixels
            */

            {
                __asm mov edi, rfp
                __asm mov eax, quad 
                __asm add edi, pre_step_pixels
                __asm mov ebx, quad
                __asm mov ecx, qword_width
top_x_loop1:
                __asm mov [edi],   eax
                __asm mov [edi+4], ebx
                __asm add edi, 8
                __asm dec ecx
                __asm jnz top_x_loop1
            }

            rfp += stride;
            --height;
            ++y;
        }
    }
    /*
    ** CASE 3: Buffer is DWORD aligned, but there are some left
    **         over pixels to take care of
    */
    else if ( !pre_step_pixels && fixup_pixels && qword_width )
    {
        while ( height )
        {
            int i, qww = qword_width;
            unsigned long  *lbuf = ( unsigned long * ) rfp;
            GLubyte        *bbuf;

            long quad = ( dither_colors[y&3][(x0+3)&3] << 24 ) |
                        ( dither_colors[y&3][(x0+2)&3] << 16 ) |
                        ( dither_colors[y&3][(x0+1)&3] << 8 ) |
                        ( dither_colors[y&3][(x0+0)&3] );

            while ( qww )
            {
                lbuf[0] = quad;
                lbuf[1] = quad;

                lbuf += 2;

                qww--;
            }

            bbuf = ( GLubyte * ) lbuf;

            for ( i = 0; i < fixup_pixels; i++ )
            {
                *bbuf++ = dither_colors[y&3][(x0+(qword_width<<2)+i)&3];
            }

            rfp += stride;
            height--;
            ++y;
        }
    }
    /*
    ** Case 4:  Buffer is not DWORD aligned and the scan line is
    **          not QWORD aligned.
    */
    else if ( pre_step_pixels && fixup_pixels && qword_width )
    {
        unsigned long *lbuf;

        while ( height )
        {
            int qww = qword_width, i;
            GLubyte        *bbuf = ( GLubyte * ) rfp;

            long quad = ( dither_colors[y&3][(x0+pre_step_pixels+3)&3] << 24 ) |
                        ( dither_colors[y&3][(x0+pre_step_pixels+2)&3] << 16 ) |
                        ( dither_colors[y&3][(x0+pre_step_pixels+1)&3] << 8 ) |
                        ( dither_colors[y&3][(x0+pre_step_pixels+0)&3] );

            for ( i = 0; i < pre_step_pixels; i++ )
            {
               *bbuf++ = dither_colors[y&3][(x0+i)&3];
            }

            lbuf = ( unsigned long * ) bbuf;

            while ( qww )
            {
                lbuf[0] = quad;
                lbuf[1] = quad;

                lbuf += 2;

                qww--;
            }

            bbuf = ( unsigned char * ) lbuf;

            for ( i = 0; i < fixup_pixels; i++ )
            {
                *bbuf++ = dither_colors[y&3][(x0+pre_step_pixels+(qword_width<<2)+i)&3];
            }

            rfp += stride;
            --height;
            ++y;
        }
    }
    /*
    ** buffer is less than 8-bytes wide
    */
    else if ( !qword_width )
    {
        for (; y < y1; y++, rfp += cfb->buf.outerWidth) {
            __GLfloat *dither = __glFastDitherTable + ((y<<2) & 12);
            
            for (x = x0, fp = rfp; x < x1; x++, fp++) {
                __GLfloat ftmp, inc;
                unsigned int r, g, b;
                GLubyte result;
                
                inc = dither[x&3];
                ftmp = fr + inc;
                SFloat2Int(ftmp, r);
                ftmp = fg + inc;
                SFloat2Int(ftmp, g);
                ftmp = fb + inc;
                SFloat2Int(ftmp, b);
                
                result =
                    (r << cfb->redShift) |
                    (g << cfb->greenShift) |
                    (b << cfb->blueShift);
                
                *fp = result;
            }
        }
    }

    __GL_UNLOCK_BUFFERS(gc);
}

#else

static void ClearRect_16_Dither(__GLcolorBuffer *cfb,
					GLint x0, GLint y, GLint x1, GLint y1)
{
    __GLcontext *gc = cfb->buf.gc;
    GLint x;
    GLushort *fp, *rfp;
    __GLfloat fr, fg, fb;
    
    fr = gc->state.raster.clear.r * gc->frontBuffer.redScale;
    fg = gc->state.raster.clear.g * gc->frontBuffer.greenScale;
    fb = gc->state.raster.clear.b * gc->frontBuffer.blueScale;
    
    __GL_LOCK_BUFFERS(gc);
    
    rfp = __GL_FB_ADDRESS(cfb, (GLushort *), x0, y);
    
    for (; y < y1; y++, rfp += cfb->buf.outerWidth) {
        __GLfloat *dither = __glFastDitherTable + ((y<<2) & 12);
        
        for (x = x0, fp = rfp; x < x1; x++, fp++) {
            __GLfloat ftmp, inc;
            unsigned int r, g, b;
            GLushort result;
            
            inc = dither[x&3];
            ftmp = fr + inc;
            SFloat2Int(ftmp, r);
            ftmp = fg + inc;
            SFloat2Int(ftmp, g);
            ftmp = fb + inc;
            SFloat2Int(ftmp, b);
            
            result =
                (r << cfb->redShift) |
                (g << cfb->greenShift) |
                (b << cfb->blueShift);
            
            *fp = result;
        }
    }
    
    __GL_UNLOCK_BUFFERS(gc);
}
#endif

static void ClearRectFast(__GLcolorBuffer *cfb,
					GLint x0, GLint y0, GLint x1, GLint y1)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLdrawableBuffer *drawableBuf = cfb->buf.drawableBuf;
    __GLdrawablePrivate *dp = gc->drawablePrivate;
    __GLcolor color;
    GLuint clearVal;
    GLint x, y, w, h;

    color.r = gc->state.raster.clear.r * gc->frontBuffer.redScale;
    color.g = gc->state.raster.clear.g * gc->frontBuffer.greenScale;
    color.b = gc->state.raster.clear.b * gc->frontBuffer.blueScale;

    if (gc->modes.alphaBits>0) {
        color.a = gc->state.raster.clear.a * gc->frontBuffer.alphaScale;
        clearVal = (((GLuint) (color.r + __glHalf)) << cfb->redShift) |
	           (((GLuint) (color.g + __glHalf)) << cfb->greenShift) |
	           (((GLuint) (color.b + __glHalf)) << cfb->blueShift) |
	           (((GLuint) (color.a + __glHalf)) << cfb->alphaShift);
    }
    else {
        clearVal = (((GLuint) (color.r + __glHalf)) << cfb->redShift) |
	           (((GLuint) (color.g + __glHalf)) << cfb->greenShift) |
	           (((GLuint) (color.b + __glHalf)) << cfb->blueShift);
    }

    if (!drawableBuf->fill) {
	x = x0;
	y = y0;
	w = x1 - x0;
	h = y1 - y0;

	if((w <= 0) || (h <= 0)) return;

	__GL_LOCK_BUFFERS(gc);
	switch(cfb->buf.elementSize) {
	  case 1:
	    { 
		unsigned char *fp = __GL_FB_ADDRESS(cfb, (unsigned char *), x, y);
		FastMemClear1(gc, fp, w, h, cfb->buf.outerWidth - w, clearVal);
	    }
	    break;

	  case 2:
	    { 
		unsigned short *fp = __GL_FB_ADDRESS(cfb, (unsigned short *), x, y);
		FastMemClear2(gc,
			      fp, w, h,  (cfb->buf.outerWidth - w)*sizeof(unsigned short),
			      clearVal);
	    }
	    break;

	  case 4:
	    { 
		unsigned long *fp = __GL_FB_ADDRESS(cfb, (unsigned long *), x, y);
		FastMemClear4(gc,
			      fp, w, h, (cfb->buf.outerWidth - w)*sizeof(unsigned long), 
			      clearVal);
	    }
	    break;
	}
	    
	__GL_UNLOCK_BUFFERS(gc);
    } else {
	x = x0 - gc->constants.viewportXAdjust;
	y = y0 - gc->constants.viewportYAdjust;
	w = x1 - x0;
	h = y1 - y0;

	if((w <= 0) || (h <= 0)) return;

	(*drawableBuf->fill)(drawableBuf, dp, clearVal, x, y, w, h);
    }
}

static void ClearRegionRects(__GLcolorBuffer *cfb,
	void (*clearRect)(__GLcolorBuffer *, GLint, GLint, GLint, GLint))
{
    __GLcontext *gc = cfb->buf.gc;
    __GLdrawablePrivate *dp = gc->drawablePrivate;
    __GLregionRect *rect;
    int i, n;

    __GL_LOCK_BUFFERS(gc);

    n = dp->ownershipRegion.numRects;
    for (i=0, rect = &dp->ownershipRegion.rects[0]; i<n; ++i, ++rect) {
	(*clearRect)(cfb,
		     rect->x0 + gc->constants.viewportXAdjust,
		     rect->y0 + gc->constants.viewportYAdjust,
		     rect->x1 + gc->constants.viewportXAdjust,
		     rect->y1 + gc->constants.viewportYAdjust);
    }

    __GL_UNLOCK_BUFFERS(gc);
}

static void Clear(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if (modeFlags & __GL_SHADE_OWNERSHIP_TEST) {
	ClearRegionRects(cfb, ClearRect);
    } else {
	ClearRect(cfb,
		  gc->transform.clipX0,
		  gc->transform.clipY0,
		  gc->transform.clipX1,
		  gc->transform.clipY1);
    }
}

static void Clear_24(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if (modeFlags & __GL_SHADE_OWNERSHIP_TEST) {
	ClearRegionRects(cfb, ClearRect_24);
    } else {
	ClearRect_24(cfb,
		  gc->transform.clipX0,
		  gc->transform.clipY0,
		  gc->transform.clipX1,
		  gc->transform.clipY1);
    }
}

static void Clear_16_Dither(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if (modeFlags & __GL_SHADE_OWNERSHIP_TEST) {
	ClearRegionRects(cfb, ClearRect_16_Dither);
    } else {
	ClearRect_16_Dither(cfb,
		  gc->transform.clipX0,
		  gc->transform.clipY0,
		  gc->transform.clipX1,
		  gc->transform.clipY1);
    }
}

static void Clear_8_Dither( __GLcolorBuffer *cfb )
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if (modeFlags & __GL_SHADE_OWNERSHIP_TEST) {
	ClearRegionRects(cfb, ClearRect_8_Dither);
    } else {
	ClearRect_8_Dither(cfb,
		  gc->transform.clipX0,
		  gc->transform.clipY0,
		  gc->transform.clipX1,
		  gc->transform.clipY1);
    }
}

static void ClearFast(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if (modeFlags & __GL_SHADE_OWNERSHIP_TEST) {
	ClearRegionRects(cfb, ClearRectFast);
    } else {
	ClearRectFast(cfb,
		  gc->transform.clipX0,
		  gc->transform.clipY0,
		  gc->transform.clipX1,
		  gc->transform.clipY1);
    }
}

static GLboolean StoreSpan(__GLcontext *gc)
{
    int x, x1;
    int i;
    __GLfragment frag;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint w;

    w = gc->polygon.shader.length;

    frag.y = gc->polygon.shader.frag.y;
    x = gc->polygon.shader.frag.x;
    x1 = gc->polygon.shader.frag.x + w;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

    __GL_LOCK_BUFFERS(gc);

    for (i = x; i < x1; i++) {
	frag.x = i;
	frag.color = *cp++;

	(*cfb->store)(cfb, &frag);
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

static GLboolean StoreStippledSpan(__GLcontext *gc)
{
    int x;
    __GLfragment frag;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    __GLstippleWord inMask, bit, *sp;
    GLint count;
    GLint w;

    w = gc->polygon.shader.length;
    sp = gc->polygon.shader.stipplePat;

    frag.y = gc->polygon.shader.frag.y;
    x = gc->polygon.shader.frag.x;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

    __GL_LOCK_BUFFERS(gc);

    while (w) {
	count = w;
	if (count > __GL_STIPPLE_BITS) {
	    count = __GL_STIPPLE_BITS;
	}
	w -= count;

	inMask = *sp++;
	bit = __GL_STIPPLE_SHIFT(0);
	while (--count >= 0) {
	    if (inMask & bit) {
		frag.x = x;
		frag.color = *cp;

		(*cfb->store)(cfb, &frag);
	    }
	    x++;
	    cp++;
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

static GLboolean StoreSpan_16_General(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    int x, x1;
    int i;
    __GLfragment frag;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLint w;
    GLushort *fp;
    __GLcolor blendColor;
    const __GLcolor *color;
    __GLfloat inc;
    GLushort fbcolor, result;

    w = gc->polygon.shader.length;

    frag.y = gc->polygon.shader.frag.y;
    x = gc->polygon.shader.frag.x;
    x1 = gc->polygon.shader.frag.x + w;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, frag.y);

    for (i = x; i < x1; i++, fp++, cp++) {

	frag.x = i;
	frag.color = *cp;

	if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		    !__glTestOwnership(gc, frag.x, frag.y)) {
	    continue;
	}

	if (modeFlags & __GL_SHADE_DITHER) {
	    GLint ix = __GL_DITHER_INDEX(frag.x, frag.y);
	    inc = ((__glDitherTable[ix] << 1) + 1) / 
		(__GLfloat) (2 * __GL_DITHER_PRECISION);
	} else {
	    inc = __glHalf;
	}
	if (modeFlags & __GL_SHADE_BLEND) {
	    color = &blendColor;
	    (*gc->procs.blend)(gc, cfb, &frag, &blendColor);
	} else {
	    color = &(frag.color);
	}
	result = (((GLuint) (color->r + inc)) << cfb->redShift) |
	    (((GLuint) (color->g + inc)) << cfb->greenShift) |
	    (((GLuint) (color->b + inc)) << cfb->blueShift);

	fbcolor = *fp;
	if (modeFlags & __GL_SHADE_LOGICOP) {
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
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

static GLboolean StoreSpan_16(__GLcontext *gc)
{
    int x, x1, y;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLushort *fp;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    y = gc->polygon.shader.frag.y;
    x = gc->polygon.shader.frag.x;
    x1 = x + gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);

    for (;x < x1; x++, cp++, fp++) {

	unsigned int r, g, b;
	GLushort result;

	if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		    !__glTestOwnership(gc, x, y)) {
	    continue;
	}

	Float2Int(cp->r, r);
	Float2Int(cp->g, g);
	Float2Int(cp->b, b);

	result =
	    (r << cfb->redShift) |
	    (g << cfb->greenShift) |
	    (b << cfb->blueShift);

	*fp = result;
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

static GLboolean StoreSpan_16_Dither(__GLcontext *gc)
{
    int x, x1, y;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLushort *fp;
    __GLfloat *dither;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    y = gc->polygon.shader.frag.y;
    x = gc->polygon.shader.frag.x;
    x1 = x + gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

    __GL_LOCK_BUFFERS(gc);

    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);

    dither = __glFastDitherTable + ((y<<2) & 12);

    for (;x < x1; x++, cp++, fp++) {
	unsigned int r, g, b;
	__GLfloat ftmp, inc;
	GLushort result;

	if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		    !__glTestOwnership(gc, x, y)) {
	    continue;
	}

	inc = dither[x & 3];

	ftmp = cp->r + inc;
	SFloat2Int(ftmp, r);
	ftmp = cp->g + inc;
	SFloat2Int(ftmp, g);
	ftmp = cp->b + inc;
	SFloat2Int(ftmp, b);

	result =
	    (r << cfb->redShift) |
	    (g << cfb->greenShift) |
	    (b << cfb->blueShift);

	*fp = result;
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

static GLboolean StoreSpan_24(__GLcontext *gc)
{
    int x, x1, y;
    __GLcolor *cp;
    __GLcolorBuffer *cfb;
    GLubyte *fp;
    GLuint stride;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    y = gc->polygon.shader.frag.y;
    x = gc->polygon.shader.frag.x;
    x1 = x + gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    cfb = gc->polygon.shader.cfb;

    __GL_LOCK_BUFFERS(gc);

    stride = ((cfb->buf.outerWidth * 3) + 3) & ~3;
    fp = ((GLubyte *) cfb->buf.base) +
	(y - gc->constants.viewportYAdjust) * stride +
	(x - gc->constants.viewportXAdjust) * 3;

    for (;x < x1; x++, cp++, fp+=3) {

	unsigned int r, g, b;

	if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		    !__glTestOwnership(gc, x, y)) {
	    continue;
	}

	Float2Int(cp->r, r);
	Float2Int(cp->g, g);
	Float2Int(cp->b, b);

	fp[2] = r;
	fp[1] = g;
	fp[0] = b;
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

static void Pick(__GLcontext *gc, __GLcolorBuffer *cfb)
{
    GLuint totalMask, sourceMask;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    totalMask = gc->modes.redMask | gc->modes.greenMask | gc->modes.blueMask |
	    gc->modes.alphaMask;
    sourceMask = 0;
    if (gc->state.raster.rMask) {
	sourceMask |= gc->modes.redMask;
    }
    if (gc->state.raster.gMask) {
	sourceMask |= gc->modes.greenMask;
    }
    if (gc->state.raster.bMask) {
	sourceMask |= gc->modes.blueMask;
    }
    if (gc->state.raster.aMask) {
	sourceMask |= gc->modes.alphaMask;
    }
    cfb->sourceMask = sourceMask;
    cfb->destMask = totalMask & ~sourceMask;

    if (gc->state.raster.drawBuffer == GL_NONE) {
	cfb->sourceMask = 0;
	cfb->destMask = totalMask;
    } 

#ifdef __GL_PC_RAST
    /* Set up framebuffer characterization */
    cfb->fbtype = 0;
#endif
    switch (cfb->buf.elementSize) {
      case 1:
	cfb->store = Store_8;
#ifdef __GL_PC_RAST
	if (5 == cfb->redShift &
	    2 == cfb->greenShift &
	    0 == cfb->blueShift) {
	    cfb->fbtype = RGB332;
	    assert(3 == gc->modes.redBits);
	    assert(3 == gc->modes.greenBits);
	    assert(2 == gc->modes.blueBits);
	}
#endif
	break;
      case 2:
	cfb->store = Store_16;
#ifdef __GL_PC_RAST
	if (0 == cfb->blueShift &
	    5 == cfb->greenShift) {
	    if (10 == cfb->redShift) {
		cfb->fbtype = RGB5;
		assert(5 == gc->modes.redBits);
		assert(5 == gc->modes.greenBits);
		assert(5 == gc->modes.blueBits);
	    } else if (11 == cfb->redShift) {
		cfb->fbtype = RGB565;
		assert(5 == gc->modes.redBits);
		assert(6 == gc->modes.greenBits);
		assert(5 == gc->modes.blueBits);
	    }
	}
#endif
	break;

      case 3:
	cfb->store = Store_24;

#ifdef __GL_PC_RAST
	if (16 == cfb->redShift &&
	    8 == cfb->greenShift &&
	    0 == cfb->blueShift) {
	    cfb->fbtype = RGB8;
	    assert(8 == gc->modes.redBits);
	    assert(8 == gc->modes.greenBits);
	    assert(8 == gc->modes.blueBits);
	}
#endif
	if (gc->polygon.shader.modeFlags & (__GL_SHADE_LOGICOP |
					    __GL_SHADE_BLEND))
	    cfb->storeSpan = StoreSpan;
	else
	    cfb->storeSpan = StoreSpan_24;
	break;

      case 4:
	if (gc->modes.alphaMask) {
	    cfb->store = Store_32A;
	} else {
	    cfb->store = Store_32;
#ifdef __GL_PC_RAST
	    if (16 == cfb->redShift &&
		8 == cfb->greenShift &&
		0 == cfb->blueShift) {
		cfb->fbtype = XRGB8;
		assert(8 == gc->modes.redBits);
		assert(8 == gc->modes.greenBits);
		assert(8 == gc->modes.blueBits);
	    }
#endif
	}
	break;
    }

    if (cfb->sourceMask != totalMask) {
        cfb->clear = Clear;
    } else if (cfb->buf.elementSize == 3) {
	cfb->clear = Clear_24;
    } else if (modeFlags & __GL_SHADE_DITHER) {
        switch (cfb->buf.elementSize) {
        case 1:
            cfb->clear = Clear_8_Dither;
            break;
        case 2:
            cfb->clear = Clear_16_Dither;
            break;
        case 3:
            cfb->clear = ClearFast;
            break;
        case 4:
            cfb->clear = ClearFast;
            break;
        }
    } else {
	cfb->clear = ClearFast;
    }
}

static void Pick16(__GLcontext *gc, __GLcolorBuffer *cfb)
{
    GLuint totalMask, sourceMask;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    totalMask = gc->modes.redMask | gc->modes.greenMask | gc->modes.blueMask |
	    gc->modes.alphaMask;
    sourceMask = 0;
    if (gc->state.raster.rMask) {
	sourceMask |= gc->modes.redMask;
    }
    if (gc->state.raster.gMask) {
	sourceMask |= gc->modes.greenMask;
    }
    if (gc->state.raster.bMask) {
	sourceMask |= gc->modes.blueMask;
    }
    if (gc->state.raster.aMask) {
	sourceMask |= gc->modes.alphaMask;
    }
    cfb->sourceMask = sourceMask;
    cfb->destMask = totalMask & ~sourceMask;

    if (gc->state.raster.drawBuffer == GL_NONE) {
	cfb->sourceMask = 0;
	cfb->destMask = totalMask;
    } 

    cfb->store = Store_16;
    cfb->storeSpan = StoreSpan_16_General;

#ifdef __GL_PC_RAST
    cfb->fbtype = 0;
    if (0 == cfb->blueShift &
	5 == cfb->greenShift) {
	if (10 == cfb->redShift) {
	    cfb->fbtype = RGB5;
	    assert(5 == gc->modes.redBits);
	    assert(5 == gc->modes.greenBits);
	    assert(5 == gc->modes.blueBits);
	} else if (11 == cfb->redShift) {
	    cfb->fbtype = RGB565;
	    assert(5 == gc->modes.redBits);
	    assert(6 == gc->modes.greenBits);
	    assert(5 == gc->modes.blueBits);
	}
    }
#endif

    if (cfb->sourceMask == totalMask) {

	if (0 == (modeFlags & (__GL_SHADE_LOGICOP | __GL_SHADE_BLEND))) {
	    if (modeFlags & __GL_SHADE_DITHER) {
		cfb->storeSpan = StoreSpan_16_Dither;
	    } else {
		cfb->storeSpan = StoreSpan_16;
	    }
	}

	if (modeFlags & __GL_SHADE_DITHER) {
	    cfb->clear = Clear_16_Dither;
	} else {
	    cfb->clear = ClearFast;
	}

    } else {
	cfb->clear = Clear;
    }
}

static void Resize( __GLbuffer *fb, GLint w, GLint h)
{
    GLint paddedWidth;

    paddedWidth = (((w * fb->elementSize + 15) >> 4) << 4) / fb->elementSize;

    __glResizeBuffer(fb, paddedWidth, h);
}


void __glInitRGB(__GLcolorBuffer *cfb, __GLcontext *gc )
{
    GLint totalBits;
    GLint i;
    GLuint mask;
    GLuint sourceMask;

    __glInitBuffer( &cfb->buf, gc );

    cfb->needColorFragmentOps = GL_FALSE;

    cfb->buf.resize = Resize;

    cfb->readSpan = __glReadSpan;
    cfb->returnSpan = __glReturnSpan;

    cfb->clear = Clear;
    cfb->pick = Pick;
    cfb->fetchSpan = __glFetchSpan;
    cfb->fetchStippledSpan = __glFetchSpan;
    cfb->storeSpan = StoreSpan;
    cfb->storeStippledSpan = StoreStippledSpan;

    mask = gc->modes.redMask;
    sourceMask = mask;
    for (i=0; mask; i++, mask>>=1) if (mask & 1) break;
    cfb->redShift = i;
    cfb->redScale = cfb->iRedScale = cfb->redMax = mask;

    mask = gc->modes.greenMask;
    sourceMask |= mask;
    for (i=0; mask; i++, mask>>=1) if (mask & 1) break;
    cfb->greenShift = i;
    cfb->greenScale = cfb->iGreenScale = cfb->greenMax = mask;

    mask = gc->modes.blueMask;
    sourceMask |= mask;
    for (i=0; mask; i++, mask>>=1) if (mask & 1) break;
    cfb->blueShift = i;
    cfb->blueScale = cfb->iBlueScale = cfb->blueMax = mask;

    mask = gc->modes.alphaMask;
    sourceMask |= mask;
    for (i=0; mask; i++, mask>>=1) if (mask & 1) break;
    cfb->alphaShift = i;
    if (mask) {
	cfb->alphaScale = cfb->iAlphaScale = mask;
    } else {
	/* Set to number large enough for table lookups */
	cfb->alphaScale = cfb->iAlphaScale = 255;
    }

    cfb->sourceMask = sourceMask;

    totalBits = 0;
    for (i=0; i<32; i++) {
	if (sourceMask & (1<<i)) totalBits++;
    }

    cfb->buf.depth = totalBits;

    switch (gc->modes.indexBits) {
    case 8:
	cfb->fetch = cfb->readColor = Fetch_8;
	break;
    case 16:
	cfb->fetch = cfb->readColor = Fetch_16;

	/* Enable FB-specific picker */
	cfb->pick = Pick16;
	break;
    case 24:
	cfb->fetch = cfb->readColor = Fetch_24;
	break;
    case 32:
	if (gc->modes.alphaMask) {
	    cfb->fetch = cfb->readColor = Fetch_32A;
	} else {
	    cfb->fetch = cfb->readColor = Fetch_32;
	}
	break;
    }
}

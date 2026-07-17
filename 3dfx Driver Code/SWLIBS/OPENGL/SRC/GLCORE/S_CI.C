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
#include "glmath.h"
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
    __GLfloat inc;
    GLubyte fbcolor, result;
    GLint ix;

    if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		!__glTestOwnership(gc, frag->x, frag->y)) {
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
    result = (frag->color.r + inc); 
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

/* 16 bit generic Store */
static void Store_16(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLushort *fp;
    __GLfloat inc;
    GLushort fbcolor, result;
    GLint ix;

    if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		!__glTestOwnership(gc, frag->x, frag->y)) {
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
    result = (frag->color.r + inc); 
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

/* 32 bit generic Store */
static void Store_32(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint *fp;
    __GLfloat inc;
    GLuint fbcolor, result;
    GLint ix;

    if ((modeFlags & __GL_SHADE_OWNERSHIP_TEST) &&
		!__glTestOwnership(gc, frag->x, frag->y)) {
	return;
    }

    fp = __GL_FB_ADDRESS(cfb, (GLuint *), frag->x, frag->y);
    if (modeFlags & __GL_SHADE_DITHER) {
        ix = __GL_DITHER_INDEX(frag->x, frag->y);
        inc = ((__glDitherTable[ix] << 1) + 1) / 
            (__GLfloat) (2 * __GL_DITHER_PRECISION);
    } else {
        inc = __glHalf;
    }
    result = (frag->color.r + inc); 
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

static void (*StoreProcs[])(__GLcolorBuffer*, const __GLfragment*) = {
    Store_8,    /* 8 bit */
    Store_16,   /* 16 bit */
    Store_32,   /* 32 bit */
};


/*
** Note: The Fetch routines below are called for both draw and read buffers.
** It's the responsibility of the caller to lock the appropriate buffers
*/
static void Fetch_8(__GLcolorBuffer *cfb, GLint x, GLint y,
                    __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    GLubyte *fp;

    fp = __GL_FB_ADDRESS(cfb, (GLubyte *), x, y);
    result->r = *fp;
}

static void Fetch_16(__GLcolorBuffer *cfb, GLint x, GLint y,
                     __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    GLushort *fp;

    fp = __GL_FB_ADDRESS(cfb, (GLushort *), x, y);
    result->r = *fp;
}

static void Fetch_32(__GLcolorBuffer *cfb, GLint x, GLint y,
                     __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint *fp;

    fp = __GL_FB_ADDRESS(cfb, (GLuint *), x, y);
    result->r = *fp;
}

#ifdef _WIN32
static void ClearRect_8(__GLcolorBuffer *cfb,
				GLint x, GLint y, GLint x1, GLint y1)
{
    __GLcontext *gc = cfb->buf.gc;
    GLint i;
    int    stride, dword_width, pre_step_pixels, fixup_pixels;
    int    width_pixels, height;
    unsigned long pattern[4];
    GLubyte *rfp;
    
    height = y1 - y;

    /*
    ** build patterns
    */
    for ( i = 0; i < 4; i++ )
    {
        float ftmp;
        unsigned int a, b, c, d;
        __GLfloat *row = ( __GLfloat * ) __glFastDitherTable + ((i<<2)&12 );

        ftmp = row[0] + gc->state.raster.clearIndex;
	SFloat2Int(ftmp, a);
	a &= 0xff;

        ftmp = row[1] + gc->state.raster.clearIndex;
	SFloat2Int(ftmp, b);
	b &= 0xff;

        ftmp = row[2] + gc->state.raster.clearIndex;
	SFloat2Int(ftmp, c);
	c &= 0xff;

        ftmp = row[3] + gc->state.raster.clearIndex;
	SFloat2Int(ftmp, d);
	d &= 0xff;

        pattern[i] = ( d << 24 )  | ( c << 16 ) | ( b << 8 ) | a;
    }

    __GL_LOCK_BUFFER(gc, &cfb->buf, cfb->buf.drawableBuf);

    rfp = ( GLubyte * ) __GL_FB_ADDRESS(cfb, (GLubyte *), x, y);

    stride = cfb->buf.outerWidth;

    pre_step_pixels = ( ( unsigned long ) rfp ) & 3;
    width_pixels    = x1 - x;
    if (pre_step_pixels > width_pixels) {
	pre_step_pixels = width_pixels;
	dword_width = 0;
	fixup_pixels = 0;
    } else {
	dword_width     = ( width_pixels - pre_step_pixels ) >> 2;
	fixup_pixels    = ( width_pixels - pre_step_pixels ) - dword_width*4;
    }

    while ( height )
    {
        GLubyte   *buf = rfp;
        int        psp = pre_step_pixels;
        int        fup = fixup_pixels;
        unsigned long pat = pattern[y&3];

        /*
        ** prestep pixels
        */
        while ( psp )
        {
            switch ( psp )
            {
            case 3:
                *buf++ = ( GLubyte ) ( pat & 0xFF );
                break;
            case 2:
                *buf++ = ( GLubyte ) ( ( pat & 0xFF00 ) >> 8 );
                break;
            case 1:
                *buf++ = ( GLubyte ) ( ( pat & 0xFF0000 ) >> 16 );
                break;
            }
            --psp;
        }

        /*
        ** do dwords
        */
        if ( dword_width )
        {
            __asm cld
            __asm mov ecx, dword_width
            __asm mov edi, buf
            __asm mov eax, pat
            __asm rep stosd
        }

        buf += dword_width*4;

        /*
        ** do fixup
        */
        while ( fup )
        {
            switch ( fup )
            {
            case 3:
                *buf++ = ( GLubyte ) ( pat & 0xFF );
                break;
            case 2:
                *buf++ = ( GLubyte ) ( ( pat & 0xFF00 ) >> 8 );
                break;
            case 1:
                *buf++ = ( GLubyte ) ( ( pat & 0xFF0000 ) >> 16 );
                break;
            }
            --fup;
        }

        rfp += stride;
        y++;
        --height;
    }

    __GL_UNLOCK_BUFFER(gc, &cfb->buf, cfb->buf.drawableBuf);
}
#endif

static void ClearRect(__GLcolorBuffer *cfb,
			GLint x, GLint y, GLint x1, GLint y1)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLint i, j;
    __GLfragment frag;

    /* Turn off logic op so store proc doesn't do it */
    gc->polygon.shader.modeFlags = modeFlags & ~__GL_SHADE_LOGICOP;

    frag.color.r = gc->state.raster.clearIndex;

    __GL_LOCK_BUFFER(gc, &cfb->buf, cfb->buf.drawableBuf);

    for (j = y; j < y1; j++) {
        frag.y = j;
        for (i = x; i < x1; i++) {
            frag.x = i;
                
            (*cfb->store)(cfb, &frag);
        }
    }

    __GL_UNLOCK_BUFFER(gc, &cfb->buf, cfb->buf.drawableBuf);

    gc->polygon.shader.modeFlags = modeFlags;
}

static void ClearRectFast(__GLcolorBuffer *cfb,
				GLint x0, GLint y0, GLint x1, GLint y1)
{
    __GLcontext *gc = cfb->buf.gc;
    __GLdrawableBuffer *drawableBuf = cfb->buf.drawableBuf;
    __GLdrawablePrivate *dp = gc->drawablePrivate;
    GLuint clearVal = (GLuint) gc->state.raster.clearIndex;
    GLubyte *fp;
    GLint x, y, w, h;

    if (!drawableBuf->fill) {
	x = x0;
	y = y0;
	w = x1 - x0;
	h = y1 - y0;

	if((w <= 0) || (h <= 0)) return;

	__GL_LOCK_BUFFER(gc, &cfb->buf, cfb->buf.drawableBuf);
	fp = __GL_FB_ADDRESS(cfb, (GLubyte *), x, y);
	FastMemClear1(gc,
		      fp, w, h, cfb->buf.outerWidth - w, clearVal);
	__GL_UNLOCK_BUFFER(gc, &cfb->buf, cfb->buf.drawableBuf);
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

    __GL_LOCK_BUFFER(gc, &cfb->buf, cfb->buf.drawableBuf);

    n = dp->ownershipRegion.numRects;
    for (i=0, rect = &dp->ownershipRegion.rects[0]; i<n; ++i, ++rect) {
	(*clearRect)(cfb,
		     rect->x0 + gc->constants.viewportXAdjust,
		     rect->y0 + gc->constants.viewportYAdjust,
		     rect->x1 + gc->constants.viewportXAdjust,
		     rect->y1 + gc->constants.viewportYAdjust);
    }

    __GL_UNLOCK_BUFFER(gc, &cfb->buf, cfb->buf.drawableBuf);
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

static void Clear_8(__GLcolorBuffer *cfb)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if (modeFlags & __GL_SHADE_OWNERSHIP_TEST) {
	ClearRegionRects(cfb, ClearRect_8);
    } else {
	ClearRect_8(cfb,
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

    for (i = x; i < x1; i++) {
        frag.x = i;
        frag.color.r = cp->r;
        cp++;

        (*cfb->store)(cfb, &frag);
    }

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
                frag.color.r = cp->r;

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

    return GL_FALSE;
}

static void Pick(__GLcontext *gc, __GLcolorBuffer *cfb)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLint ix = 0;

    if (gc->state.raster.drawBuffer == GL_NONE) {
        cfb->sourceMask = 0;
        cfb->destMask = cfb->redMax;
    } else {
        cfb->sourceMask = gc->state.raster.writeMask & cfb->redMax;
        cfb->destMask = cfb->redMax & ~gc->state.raster.writeMask;
    }

#ifdef __GL_PC_RAST
    /* Set up framebuffer characterization */
    cfb->fbtype = 0;
#endif

    switch (cfb->buf.elementSize) {
      case 1:
#ifdef __GL_PC_RAST
	  cfb->fbtype = CINDEX;
#endif
        break;
      case 2:
        ix += 1;
        break;
      case 4:
        ix += 2;
        break;
    }
    cfb->store = StoreProcs[ix];

    if ((cfb->sourceMask != (GLuint) cfb->redMax) ||
	(cfb->buf.elementSize != 1)) {
        cfb->clear = Clear;
    } else if (modeFlags & __GL_SHADE_DITHER) {
	cfb->clear = Clear_8;
    } else {
        cfb->clear = ClearFast;
    }
}

static GLboolean Resize(__GLbuffer *fb, GLint w, GLint h, GLuint bufferMask)
{
    GLint paddedWidth;

    paddedWidth = (((w * fb->elementSize + 3) >> 2) << 2) / fb->elementSize;

    return __glResizeBuffer(fb, paddedWidth, h, bufferMask);
}


void __glInitCI(__GLcolorBuffer *cfb, __GLcontext *gc )
{
    GLint indexBits;

    __glInitBuffer( &cfb->buf, gc );

    cfb->buf.resize = Resize;

    cfb->readSpan = __glReadSpan;
    cfb->returnSpan = __glReturnSpan;

    cfb->clear = Clear;
    cfb->pick = Pick;
    cfb->fetchSpan = __glFetchSpan;
    cfb->fetchStippledSpan = __glFetchSpan;
    cfb->storeSpan = StoreSpan;
    cfb->storeStippledSpan = StoreStippledSpan;

    cfb->redScale = 1.0;
    cfb->greenScale = 1.0;
    cfb->blueScale = 1.0;
    cfb->alphaScale = 1.0;

    indexBits = gc->modes.indexBits;
    if (indexBits < 32) {
        cfb->sourceMask = (1 << indexBits) - 1;
    } else {
        cfb->sourceMask = 0xffffffff;
    }
    cfb->redMax = cfb->sourceMask;

    if (indexBits > 16) {       
        cfb->fetch = cfb->readColor = Fetch_32;
    } else if (indexBits > 8) {
        cfb->fetch = cfb->readColor = Fetch_16;
    } else {
        cfb->fetch = cfb->readColor = Fetch_8;
    }
}

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
#include <windows.h>
#include "context.h"
#include "global.h"
#include "fmemclr.h"
#include "fmacros.h"
#ifdef __GL_PC_RAST
#include "fr_fbtype.h"
#endif

#include <glide.h>
#include "sst_globals.h"

/* 16 bit generic Store */
static void Store_16(__GLcolorBuffer *cfb, const __GLfragment *frag)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLcolor blendColor;
    const __GLcolor *color;
    __GLfloat inc;
    GLushort *fp, fbcolor, result;
    GLint ix;

    fp = __GL_FB_ADDRESS(cfb, (GLushort *), frag->x, frag->y);

    if (modeFlags & __GL_SHADE_DITHER) {
        ix = __GL_DITHER_INDEX(frag->x, frag->y);
        inc = ((__glDitherTable[ix] << 1) + 1) / 
            (__GLfloat) (2 * __GL_DITHER_PRECISION);
    } else {
//        inc = __glHalf;
        inc = 0;
    }

    if (modeFlags & __GL_SHADE_BLEND) {
        color = &blendColor;
        (*gc->procs.blend)(gc, cfb, frag, &blendColor);
    } else {
        color = &(frag->color);
    }
    result = (((((GLuint) (color->r + inc)) & 0xf8) << 8) |
              ((((GLuint) (color->g + inc)) & 0xfc) << 3) |
              ((((GLuint) (color->b + inc)) & 0xf8) >> 3));

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

static void Fetch_16(__GLcolorBuffer *cfb, GLint x, GLint y,
                     __GLcolor *result)
{
    __GLcontext *gc = cfb->buf.gc;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLushort value;
    unsigned char *p;

    p = cfb->buf.base;
    p += (y - gc->constants.viewportYAdjust) * cfb->buf.byteWidth;
    p += (x - gc->constants.viewportXAdjust) * 2;
    value = *(unsigned short *)p;

#if 0
    result->r = ((value & 0xf800) >> 8) | (value >> 13);
    result->g = ((value & 0x07e0) >> 3) | ((value & 0x0600) >> 9);
    result->b = ((value & 0x001f) << 3) | ((value & 0x001c) >> 2);
    result->a = 255;
#else
    result->r = ((value & 0xf800) >> 11)*255.0/31.0;
    result->g = ((value & 0x07e0) >> 5)*255.0/63.0;
    result->b = ((value & 0x001f))*255.0/31.0;
    result->a = 255;
#endif

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
        result = ((((GLuint) (color->r + inc))&0xf8) << 8) |
            ((((GLuint) (color->g + inc))&0xfc) << 3) |
            ((((GLuint) (color->b + inc))&0xf8) >> 3);

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
            ((r & 0xf8) << 8) |
            ((g & 0xfc) << 3) |
            ((b & 0xf8) >> 3);

        *fp = result;
    }

    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
}

static void Pick16(__GLcontext *gc, __GLcolorBuffer *cfb)
{
    GLuint totalMask, sourceMask;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

#ifdef __GL_PC_RAST
    cfb->fbtype = RGB565;
#endif

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

    if (cfb->sourceMask != totalMask) {
        /* shoule be in sw path */
    }

    cfb->storeSpan = StoreSpan_16;
}

static void Resize( __GLbuffer *fb, GLint w, GLint h)
{
    GLint paddedWidth;

    paddedWidth = (((w * fb->elementSize + 15) >> 4) << 4) / fb->elementSize;

    __glResizeBuffer(fb, paddedWidth, h);
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

  //  __GL_LOCK_BUFFERS(gc);

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

//    __GL_UNLOCK_BUFFERS(gc);

    return GL_FALSE;
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

void __glSSTInitRGB(__GLcolorBuffer *cfb, __GLcontext *gc )
{
    __glInitBuffer( &cfb->buf, gc );

    cfb->needColorFragmentOps = GL_TRUE;

    cfb->buf.resize = Resize;

    cfb->readSpan = __glReadSpan;
    cfb->returnSpan = __glReturnSpan;

    cfb->clear = Clear;    
    cfb->pick = Pick16;
    cfb->fetchSpan = __glFetchSpan;
    cfb->storeSpan = StoreSpan;
    cfb->fetchStippledSpan = __glFetchSpan;
    cfb->storeStippledSpan = StoreStippledSpan;

    /* Override the mask setting from gc->modes.  We set the color scales to */
    /* what Glides expects, not what "modes" thinks is the format of the fb. */

    /* mask = gc->modes.redMask; */
    cfb->redShift = 11;
    cfb->redScale = cfb->iRedScale = cfb->redMax = __GL_SST_MAX_RED;

    /* mask = gc->modes.greenMask; */
    cfb->greenShift = 5;
    cfb->greenScale = cfb->iGreenScale = cfb->greenMax = __GL_SST_MAX_GREEN;

    /* mask = gc->modes.blueMask; */
    cfb->blueShift = 0;
    cfb->blueScale = cfb->iBlueScale = cfb->blueMax = __GL_SST_MAX_BLUE;

    /* mask = gc->modes.alphaMask; */
    cfb->alphaShift = 0;
    cfb->alphaScale = cfb->iAlphaScale = __GL_SST_MAX_ALPHA;

    cfb->sourceMask = 0xff;
    cfb->buf.depth = 16;
    cfb->fetch = cfb->readColor = Fetch_16;

    /* Enable FB-specific picker */
    cfb->pick = Pick16;
}

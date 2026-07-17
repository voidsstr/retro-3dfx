/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
*/
#ifdef __GL_PC_RAST
#include "context.h"
#include "global.h"
#include "xform.h"
#include "string.h"
#include "fr_modes.h"
#include "fr_tri.h"
#include "fr_fbtype.h"
#include "fr_store.h"

__GLspanlet __glStoreTab[] = {
    0,
    __glStoreSpanCINDEX,
    __glStoreSpanRGB332,
    0,
    __glStoreSpanRGB5,
    __glStoreSpanRGB565,
    0,
    __glStoreSpanRGB8,
    0,
    0,
    __glStoreSpanXRGB8,
    0
};

void __fastcall __glStoreSpanCINDEX(unsigned int mask, __GLtri *tr) 
{
    GLubyte *fp = (GLubyte *) tr->cp;
    GLuint *cp = (GLuint *) tr->colorBuf;
    GLuint result;
    GLuint index;
    GLbitfield modeFlags = tr->gc->polygon.shader.modeFlags;
    int dither = modeFlags & __GL_SHADE_DITHER ? 1 : 0;
    int logicop = modeFlags & __GL_SHADE_LOGICOP ? 1 : 0;

    while (1) {
	while (((int)mask) < 0) {

	    index = cp[0];

	    if (dither) {
		index += *tr->dither;
	    }

	    /* assemble value */
	    result = TruncFixed(index, COLOR_FRAC_BITS);

	    if (logicop) {
		GLuint fbcolor = *fp;

		switch(tr->gc->state.raster.logicOp) {
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

	    /* apply mask */
	    if (modeFlags & __GL_SHADE_MASK) {
		__GLcolorBuffer *cfb = tr->gc->drawBuffer;
		GLuint fbcolor = *fp;

		result = (fbcolor & cfb->destMask) |
		    (result & cfb->sourceMask);
	    }

	    /* Store result */
	    *fp = result;

	    /* Step interpolants */
	    fp += tr->dx;
	    cp ++;
	    tr->dither++;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */
	fp += tr->dx;
	cp ++;
	tr->dither++;
	mask <<= 1;
    }
}

void __fastcall __glStoreSpanRGB332(unsigned int mask, __GLtri *tr) 
{
    GLubyte *fp = (GLubyte *) tr->cp;
    GLubyte *cp = (GLubyte *) tr->colorBuf;
    GLuint result;
    GLuint r, g, b;
    GLbitfield modeFlags = tr->gc->polygon.shader.modeFlags;
    int dither = modeFlags & __GL_SHADE_DITHER ? 1 : 0;
    int blend = modeFlags & __GL_SHADE_BLEND ? 1 : 0;
    int logicop = modeFlags & __GL_SHADE_LOGICOP ? 1 : 0;

    while (1) {
	while (((int)mask) < 0) {

	    r = cp[0];
	    g = cp[1];
	    b = cp[2];

	    if (blend) {
		GLuint sr, sg, sb, sa, dr, dg, db;
		GLubyte fbcolor = *fp;

		sa = cp[3];

		dr = ((fbcolor & (0x7<<5)) >> 5) << 5;
		dg = ((fbcolor & (0x7<<2)) >> 2) << 5;
		db = ((fbcolor & (0x3<<0)) >> 0) << 6;

		switch (tr->gc->state.raster.blendSrc) {
		case GL_ZERO:
		    sr = sg = sb = 0;
		    break;
		case GL_ONE:
		    sr = r; sg = g; sb = b;
		    break;
		case GL_DST_COLOR:
		    sr = BLEND(r, dr);
		    sg = BLEND(g, dg);
		    sb = BLEND(b, db);
		    break;
		case GL_ONE_MINUS_DST_COLOR:
		    sr = BLEND(r, 255-dr);
		    sg = BLEND(g, 255-dg);
		    sb = BLEND(b, 255-db);
		    break;
		case GL_SRC_ALPHA:
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_DST_ALPHA:
		    sr = r;
		    sg = g;
		    sb = b;
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
		    /* These are useless w/o dest alpha */
		    sr = sg = sb = 0;
		    break;
		}

		switch (tr->gc->state.raster.blendDst) {
		case GL_ZERO:
		    dr = dg = db = 0;
		    break;
		case GL_ONE:
		    /* nop */
		    break;
		case GL_SRC_COLOR:
		    dr = BLEND(dr, r);
		    dg = BLEND(dg, g);
		    db = BLEND(db, b);
		    break;
		case GL_ONE_MINUS_SRC_COLOR:
		    dr = BLEND(dr, 255-r);
		    dg = BLEND(dg, 255-g);
		    db = BLEND(db, 255-b);
		    break;
		case GL_SRC_ALPHA:
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_DST_ALPHA:
		    /* nop */
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		    /* These are useless w/o dest alpha */
		    dr = dg = db = 0;
		    break;
		}

		r = sr + dr; if (r > 0xe0) r = 0xe0;
		g = sg + dg; if (g > 0xe0) g = 0xe0;
		b = sb + db; if (b > 0xc0) b = 0xc0;
	    }

	    if (dither) {
		int di = *tr->dither;
		r += (di >> 11) - (r >> 3);
		g += (di >> 11) - (g >> 3);
		b += (di >> 10) - (b >> 2);
	    }

	    /* assemble value */
	    r >>= 5;
	    g >>= 5;
	    b >>= 6;

	    result = (r<<5) | (g<<2) | (b<<0);

	    if (logicop) {
		GLuint fbcolor = *fp;

		switch(tr->gc->state.raster.logicOp) {
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

	    /* apply mask */
	    if (modeFlags & __GL_SHADE_MASK) {
		__GLcolorBuffer *cfb = tr->gc->drawBuffer;
		GLuint fbcolor = *fp;

		result = (fbcolor & cfb->destMask) |
		    (result & cfb->sourceMask);
	    }

	    /* Store result */
	    *fp = result;

	    /* Step interpolants */
	    fp += tr->dx;
	    cp += 4;
	    tr->dither++;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */
	fp += tr->dx;
	cp += 4;
	tr->dither++;
	mask <<= 1;
    }
}

void __fastcall __glStoreSpanRGB5(unsigned int mask, __GLtri *tr)
{
    GLushort *fp = (GLushort *) tr->cp;
    GLubyte *cp = (GLubyte *) tr->colorBuf;
    GLuint result;
    GLuint r, g, b;
    GLbitfield modeFlags = tr->gc->polygon.shader.modeFlags;
    int dither = modeFlags & __GL_SHADE_DITHER ? 1 : 0;
    int blend = modeFlags & __GL_SHADE_BLEND ? 1 : 0;
    int logicop = modeFlags & __GL_SHADE_LOGICOP ? 1 : 0;

    while (1) {
	while (((int)mask) < 0) {

	    r = cp[0];
	    g = cp[1];
	    b = cp[2];

	    if (blend) {
		GLuint sr, sg, sb, sa, dr, dg, db;
		GLushort fbcolor = *fp;

		sa = cp[3];

		dr = ((fbcolor & (0x1f<<10)) >> 10) << 3;
		dg = ((fbcolor & (0x1f<<5) ) >>  5) << 3;
		db = ((fbcolor & (0x1f)    ) >>  0) << 3;

		switch (tr->gc->state.raster.blendSrc) {
		case GL_ZERO:
		    sr = sg = sb = 0;
		    break;
		case GL_ONE:
		    sr = r; sg = g; sb = b;
		    break;
		case GL_DST_COLOR:
		    sr = BLEND(r, dr);
		    sg = BLEND(g, dg);
		    sb = BLEND(b, db);
		    break;
		case GL_ONE_MINUS_DST_COLOR:
		    sr = BLEND(r, 255-dr);
		    sg = BLEND(g, 255-dg);
		    sb = BLEND(b, 255-db);
		    break;
		case GL_SRC_ALPHA:
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_DST_ALPHA:
		    sr = r;
		    sg = g;
		    sb = b;
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
		    /* These are useless w/o dest alpha */
		    sr = sg = sb = 0;
		    break;
		}

		switch (tr->gc->state.raster.blendDst) {
		case GL_ZERO:
		    dr = dg = db = 0;
		    break;
		case GL_ONE:
		    /* nop */
		    break;
		case GL_SRC_COLOR:
		    dr = BLEND(dr, r);
		    dg = BLEND(dg, g);
		    db = BLEND(db, b);
		    break;
		case GL_ONE_MINUS_SRC_COLOR:
		    dr = BLEND(dr, 255-r);
		    dg = BLEND(dg, 255-g);
		    db = BLEND(db, 255-b);
		    break;
		case GL_SRC_ALPHA:
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_DST_ALPHA:
		    /* nop */
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		    /* These are useless w/o dest alpha */
		    dr = dg = db = 0;
		    break;
		}

		r = sr + dr; if (r > 0xf8) r = 0xf8;
		g = sg + dg; if (g > 0xf8) g = 0xf8;
		b = sb + db; if (b > 0xf8) b = 0xf8;
	    }

	    if (dither) {
		int di = *tr->dither >> 13;
		r += di - (r >> 5);
		g += di - (g >> 5);
		b += di - (b >> 5);
	    }

	    /* assemble value */
	    r >>= 3;
	    g >>= 3;
	    b >>= 3;

	    result = (r<<10) | (g<<5) | b;

	    if (logicop) {
		GLuint fbcolor = *fp;

		switch(tr->gc->state.raster.logicOp) {
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

	    /* apply mask */
	    if (modeFlags & __GL_SHADE_MASK) {
		__GLcolorBuffer *cfb = tr->gc->drawBuffer;
		GLuint fbcolor = *fp;

		result = (fbcolor & cfb->destMask) |
		    (result & cfb->sourceMask);
	    }


	    /* Store result */
	    *fp = result;

	    /* Step interpolants */
	    fp += tr->dx;
	    cp += 4;
	    tr->dither++;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */
	fp += tr->dx;
	cp += 4;
	tr->dither++;
	mask <<= 1;
    }
}

void __fastcall __glStoreSpanRGB565(unsigned int mask, __GLtri *tr)
{
    GLushort *fp = (GLushort *) tr->cp;
    GLubyte *cp = (GLubyte *) tr->colorBuf;
    GLuint result;
    GLuint r, g, b;
    GLbitfield modeFlags = tr->gc->polygon.shader.modeFlags;
    int dither = modeFlags & __GL_SHADE_DITHER ? 1 : 0;
    int blend = modeFlags & __GL_SHADE_BLEND ? 1 : 0;
    int logicop = modeFlags & __GL_SHADE_LOGICOP ? 1 : 0;

    while (1) {
	while (((int)mask) < 0) {

	    r = cp[0];
	    g = cp[1];
	    b = cp[2];

	    if (blend) {
		GLuint sr, sg, sb, sa, dr, dg, db;
		GLushort fbcolor = *fp;

		sa = cp[3];

		dr = ((fbcolor & (0x1f<<11)) >> 11) << 3;
		dg = ((fbcolor & (0x3f<<5) ) >>  5) << 2;
		db = ((fbcolor & (0x1f)    ) >>  0) << 3;

		switch (tr->gc->state.raster.blendSrc) {
		case GL_ZERO:
		    sr = sg = sb = 0;
		    break;
		case GL_ONE:
		    sr = r; sg = g; sb = b;
		    break;
		case GL_DST_COLOR:
		    sr = BLEND(r, dr);
		    sg = BLEND(g, dg);
		    sb = BLEND(b, db);
		    break;
		case GL_ONE_MINUS_DST_COLOR:
		    sr = BLEND(r, 255-dr);
		    sg = BLEND(g, 255-dg);
		    sb = BLEND(b, 255-db);
		    break;
		case GL_SRC_ALPHA:
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_DST_ALPHA:
		    sr = r;
		    sg = g;
		    sb = b;
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
		    /* These are useless w/o dest alpha */
		    sr = sg = sb = 0;
		    break;
		}

		switch (tr->gc->state.raster.blendDst) {
		case GL_ZERO:
		    dr = dg = db = 0;
		    break;
		case GL_ONE:
		    /* nop */
		    break;
		case GL_SRC_COLOR:
		    dr = BLEND(dr, r);
		    dg = BLEND(dg, g);
		    db = BLEND(db, b);
		    break;
		case GL_ONE_MINUS_SRC_COLOR:
		    dr = BLEND(dr, 255-r);
		    dg = BLEND(dg, 255-g);
		    db = BLEND(db, 255-b);
		    break;
		case GL_SRC_ALPHA:
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_DST_ALPHA:
		    /* nop */
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		    /* These are useless w/o dest alpha */
		    dr = dg = db = 0;
		    break;
		}

		r = sr + dr; if (r > 0xf8) r = 0xf8;
		g = sg + dg; if (g > 0xfc) g = 0xfc;
		b = sb + db; if (b > 0xf8) b = 0xf8;
	    }

	    if (dither) {
		int di = *tr->dither;
		r += (di >> 13) - (r >> 5);
		g += (di >> 14) - (g >> 6);
		b += (di >> 13) - (b >> 5);
	    }

	    /* assemble value */
	    r >>= 3;
	    g >>= 2;
	    b >>= 3;

	    result = (r<<11) | (g<<5) | b;

	    if (logicop) {
		GLuint fbcolor = *fp;

		switch(tr->gc->state.raster.logicOp) {
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

	    /* apply mask */
	    if (modeFlags & __GL_SHADE_MASK) {
		__GLcolorBuffer *cfb = tr->gc->drawBuffer;
		GLuint fbcolor = *fp;

		result = (fbcolor & cfb->destMask) |
		    (result & cfb->sourceMask);
	    }


	    /* Store result */
	    *fp = result;

	    /* Step interpolants */
	    fp += tr->dx;
	    cp += 4;
	    tr->dither++;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */
	fp += tr->dx;
	cp += 4;
	tr->dither++;
	mask <<= 1;
    }
}

void __fastcall __glStoreSpanRGB8(unsigned int mask, __GLtri *tr)
{
    GLubyte *fp = (GLubyte *) tr->cp;
    GLubyte *cp = (GLubyte *) tr->colorBuf;
    GLuint result;
    GLuint r, g, b;
    GLbitfield modeFlags = tr->gc->polygon.shader.modeFlags;
    int blend = modeFlags & __GL_SHADE_BLEND ? 1 : 0;
    int logicop = modeFlags & __GL_SHADE_LOGICOP ? 1 : 0;

    while (1) {
	while (((int)mask) < 0) {

	    r = cp[0];
	    g = cp[1];
	    b = cp[2];

	    if (blend) {
		GLuint sr, sg, sb, sa, dr, dg, db;

		sa = cp[3];

		dr = fp[2];
		dg = fp[1];
		db = fp[0];

		switch (tr->gc->state.raster.blendSrc) {
		case GL_ZERO:
		    sr = sg = sb = 0;
		    break;
		case GL_ONE:
		    sr = r; sg = g; sb = b;
		    break;
		case GL_DST_COLOR:
		    sr = BLEND(r, dr);
		    sg = BLEND(g, dg);
		    sb = BLEND(b, db);
		    break;
		case GL_ONE_MINUS_DST_COLOR:
		    sr = BLEND(r, 255-dr);
		    sg = BLEND(g, 255-dg);
		    sb = BLEND(b, 255-db);
		    break;
		case GL_SRC_ALPHA:
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_DST_ALPHA:
		    sr = r;
		    sg = g;
		    sb = b;
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
		    /* These are useless w/o dest alpha */
		    sr = sg = sb = 0;
		    break;
		}

		switch (tr->gc->state.raster.blendDst) {
		case GL_ZERO:
		    dr = dg = db = 0;
		    break;
		case GL_ONE:
		    /* nop */
		    break;
		case GL_SRC_COLOR:
		    dr = BLEND(dr, r);
		    dg = BLEND(dg, g);
		    db = BLEND(db, b);
		    break;
		case GL_ONE_MINUS_SRC_COLOR:
		    dr = BLEND(dr, 255-r);
		    dg = BLEND(dg, 255-g);
		    db = BLEND(db, 255-b);
		    break;
		case GL_SRC_ALPHA:
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_DST_ALPHA:
		    /* nop */
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		    /* These are useless w/o dest alpha */
		    dr = dg = db = 0;
		    break;
		}

		r = sr + dr; if (r > 255) r = 255;
		g = sg + dg; if (g > 255) g = 255;
		b = sb + db; if (b > 255) b = 255;
	    }

	    result = (r<<16) | (g<<8) | b;

	    if (logicop) {
		GLuint fbcolor = (fp[2] << 16) | (fp[1] << 8) | fp[0];

		switch(tr->gc->state.raster.logicOp) {
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

	    /* apply mask */
	    if (modeFlags & __GL_SHADE_MASK) {
		__GLcolorBuffer *cfb = tr->gc->drawBuffer;
		GLuint fbcolor = *fp;

		result = (fbcolor & cfb->destMask) |
		    (result & cfb->sourceMask);
	    }


	    /* Store result */
	    fp[2] = (GLubyte) (result >> 16);
	    fp[1] = (GLubyte) (result >>  8);
	    fp[0] = (GLubyte) (result >>  0);

	    /* Step interpolants */
	    fp += 3*tr->dx;
	    cp += 4;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */
	fp += 3*tr->dx;
	cp += 4;
	mask <<= 1;
    }
}

void __fastcall __glStoreSpanXRGB8(unsigned int mask, __GLtri *tr)
{
    GLuint *fp = (GLuint *) tr->cp;
    GLubyte *cp = (GLubyte *) tr->colorBuf;
    GLuint result;
    GLuint r, g, b;
    GLbitfield modeFlags = tr->gc->polygon.shader.modeFlags;
    int blend = modeFlags & __GL_SHADE_BLEND ? 1 : 0;
    int logicop = modeFlags & __GL_SHADE_LOGICOP ? 1 : 0;

    while (1) {
	while (((int)mask) < 0) {

	    r = cp[0];
	    g = cp[1];
	    b = cp[2];

	    if (blend) {
		GLuint sr, sg, sb, sa, dr, dg, db;
		GLuint fbcolor = *fp;

		sa = cp[3];

		dr = (fbcolor & (0xff0000)) >> 16;
		dg = (fbcolor & (0x00ff00)) >>  8;
		db = (fbcolor & (0x0000ff)) >>  0;

		switch (tr->gc->state.raster.blendSrc) {
		case GL_ZERO:
		    sr = sg = sb = 0;
		    break;
		case GL_ONE:
		    sr = r; sg = g; sb = b;
		    break;
		case GL_DST_COLOR:
		    sr = BLEND(r, dr);
		    sg = BLEND(g, dg);
		    sb = BLEND(b, db);
		    break;
		case GL_ONE_MINUS_DST_COLOR:
		    sr = BLEND(r, 255-dr);
		    sg = BLEND(g, 255-dg);
		    sb = BLEND(b, 255-db);
		    break;
		case GL_SRC_ALPHA:
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    sr = BLEND(r, sa);
		    sg = BLEND(g, sa);
		    sb = BLEND(b, sa);
		    break;
		case GL_DST_ALPHA:
		    sr = r;
		    sg = g;
		    sb = b;
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		case GL_SRC_ALPHA_SATURATE:
		    /* These are useless w/o dest alpha */
		    sr = sg = sb = 0;
		    break;
		}

		switch (tr->gc->state.raster.blendDst) {
		case GL_ZERO:
		    dr = dg = db = 0;
		    break;
		case GL_ONE:
		    /* nop */
		    break;
		case GL_SRC_COLOR:
		    dr = BLEND(dr, r);
		    dg = BLEND(dg, g);
		    db = BLEND(db, b);
		    break;
		case GL_ONE_MINUS_SRC_COLOR:
		    dr = BLEND(dr, 255-r);
		    dg = BLEND(dg, 255-g);
		    db = BLEND(db, 255-b);
		    break;
		case GL_SRC_ALPHA:
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_ONE_MINUS_SRC_ALPHA:
		    sa = 255-sa;
		    dr = BLEND(dr, sa);
		    dg = BLEND(dg, sa);
		    db = BLEND(db, sa);
		    break;
		case GL_DST_ALPHA:
		    /* nop */
		    break;
		case GL_ONE_MINUS_DST_ALPHA:
		    /* These are useless w/o dest alpha */
		    dr = dg = db = 0;
		    break;
		}

		r = sr + dr; if (r > 255) r = 255;
		g = sg + dg; if (g > 255) g = 255;
		b = sb + db; if (b > 255) b = 255;
	    }

	    /* assemble value */
	    result = (r<<16) | (g<<8) | b;

	    if (logicop) {
		GLuint fbcolor = *fp;

		switch(tr->gc->state.raster.logicOp) {
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

	    /* apply mask */
	    if (modeFlags & __GL_SHADE_MASK) {
		__GLcolorBuffer *cfb = tr->gc->drawBuffer;
		GLuint fbcolor = *fp;

		result = (fbcolor & cfb->destMask) |
		    (result & cfb->sourceMask);
	    }


	    /* Store result */
	    *fp = result;

	    /* Step interpolants */
	    fp += tr->dx;
	    cp += 4;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */
	fp += tr->dx;
	cp += 4;
	mask <<= 1;
    }
}

#endif /* __GL_PC_RAST */

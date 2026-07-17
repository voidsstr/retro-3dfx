#ifdef __GL_PC_RAST
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
#include "context.h"
#include "global.h"
#include "xform.h"
#include "string.h"
#include "fr_modes.h"
#include "fr_tri.h"
#include "fr_fbtype.h"


#define PIXEL_TYPE 4
#include "fr_fbconf.h"

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_0(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);

	    /* Step interpolants */

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_1(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
	    cp[3] = ACoordToUByte(a);

	    /* Step interpolants */

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_2(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 0
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_3(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
	    cp[3] = ACoordToUByte(a);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_4(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);

	    /* Step interpolants */

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_5(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);

	    cp[3] = ACoordToUByte(a);

	    /* Step interpolants */

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_6(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);

	    /* Step interpolants */

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_7(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);

	    cp[3] = ACoordToUByte(a);

	    /* Step interpolants */

	    a += tr->dadx;

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_C(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);

	    /* Step interpolants */

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_D(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);

	    cp[3] = ACoordToUByte(a);

	    /* Step interpolants */

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_E(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_F(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);

	    cp[3] = ACoordToUByte(a);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_10(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);

	    /* Step interpolants */

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_11(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);

	    cp[3] = (CoordToInt(a) << ASHIFT);

	    /* Step interpolants */

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_12(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_13(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);

	    cp[3] = (CoordToInt(a) << ASHIFT);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_14(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);

	    /* Step interpolants */

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_15(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);
	    cp[3] = TBLEND_A_UB(tp[3], a, ba);

	    /* Step interpolants */

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_16(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsRGB4_17(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);
	    cp[3] = TBLEND_A_UB(tp[3], a, ba);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;

	    tp += 3;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_24(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_25(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);

	    cp[3] = ACLAMP_UB(tp[3]);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_26(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 1
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_27(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(tp[0]);
	    cp[1] = GCLAMP_UB(tp[1]);
	    cp[2] = BCLAMP_UB(tp[2]);

	    cp[3] = ACLAMP_UB(tp[3]);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 1
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_28(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(DECAL_R_UB(tp[0], tp[3], r));
	    cp[1] = GCLAMP_UB(DECAL_G_UB(tp[1], tp[3], g));
	    cp[2] = BCLAMP_UB(DECAL_B_UB(tp[2], tp[3], b));

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 1
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_29(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(DECAL_R_UB(tp[0], tp[3], r));
	    cp[1] = GCLAMP_UB(DECAL_G_UB(tp[1], tp[3], g));
	    cp[2] = BCLAMP_UB(DECAL_B_UB(tp[2], tp[3], b));
	    cp[3] = ACoordToUByte(a);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 1
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_2A(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(DECAL_R_UB(tp[0], tp[3], r));
	    cp[1] = GCLAMP_UB(DECAL_G_UB(tp[1], tp[3], g));
	    cp[2] = BCLAMP_UB(DECAL_B_UB(tp[2], tp[3], b));

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 1
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_2B(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCLAMP_UB(DECAL_R_UB(tp[0], tp[3], r));
	    cp[1] = GCLAMP_UB(DECAL_G_UB(tp[1], tp[3], g));
	    cp[2] = BCLAMP_UB(DECAL_B_UB(tp[2], tp[3], b));
	    cp[3] = ACoordToUByte(a);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_2C(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_2D(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);

	    cp[3] = MODULATE_A_UB(tp[3], a);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_2E(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 1
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_2F(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = MODULATE_R_UB(tp[0], r);
	    cp[1] = MODULATE_G_UB(tp[1], g);
	    cp[2] = MODULATE_B_UB(tp[2], b);

	    cp[3] = MODULATE_A_UB(tp[3], a);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_30(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_31(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);

	    cp[3] = MODULATE_A_UB(tp[3], a);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_32(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 1
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_33(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);

	    cp[3] = MODULATE_A_UB(tp[3], a);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_34(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_35(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);
	    cp[3] = TBLEND_A_UB(tp[3], a, ba);

	    /* Step interpolants */

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 1
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_36(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 1
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_37(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    GLuint br = gc->texture.envColor[0];
    GLuint bg = gc->texture.envColor[1];
    GLuint bb = gc->texture.envColor[2];

    GLuint ba = gc->texture.envColor[3];

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = TBLEND_R_UB(tp[0], r, br);
	    cp[1] = TBLEND_G_UB(tp[1], g, bg);
	    cp[2] = TBLEND_B_UB(tp[2], b, bb);
	    cp[3] = TBLEND_A_UB(tp[3], a, ba);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;

	    tp += 4;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 1
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_39(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
	    cp[3] = ACLAMP_UB(tp[3]);

	    /* Step interpolants */

	    tp += 1;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 1
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_3B(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
	    cp[3] = ACLAMP_UB(tp[3]);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;

	    tp += 1;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 1
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_3D(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
	    cp[3] = MODULATE_A_UB(tp[0], a);

	    /* Step interpolants */

	    tp += 1;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 0
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 1
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 1
 * #define FR_ADD 0
 * #define FR_TEXALPHA 1
 */

void __fastcall __glGenColorsRGB4_3F(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLubyte *cp = (GLubyte *) tr->colorBuf;

    unsigned int r = tr->r;
    unsigned int g = tr->g;
    unsigned int b = tr->b;
    unsigned int a = tr->a;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = RCoordToUByte(r);
	    cp[1] = GCoordToUByte(g);
	    cp[2] = BCoordToUByte(b);
	    cp[3] = MODULATE_A_UB(tp[0], a);

	    /* Step interpolants */

	    r += tr->drdx;
	    g += tr->dgdx;
	    b += tr->dbdx;
	    a += tr->dadx;

	    tp += 1;

	    cp += 4;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	r += tr->drdx;
	g += tr->dgdx;
	b += tr->dbdx;
	a += tr->dadx;

	cp += 4;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}
/*
 *
 */
void (__fastcall *__fr_rgb_genclr_4[64])(unsigned int mask, __GLtri *tr) =
{
    __glGenColorsRGB4_0,
    __glGenColorsRGB4_1,
    __glGenColorsRGB4_2,
    __glGenColorsRGB4_3,
    __glGenColorsRGB4_4,
    __glGenColorsRGB4_5,
    __glGenColorsRGB4_6,
    __glGenColorsRGB4_7,
    0,
    0,
    0,
    0,
    __glGenColorsRGB4_C,
    __glGenColorsRGB4_D,
    __glGenColorsRGB4_E,
    __glGenColorsRGB4_F,
    __glGenColorsRGB4_10,
    __glGenColorsRGB4_11,
    __glGenColorsRGB4_12,
    __glGenColorsRGB4_13,
    __glGenColorsRGB4_14,
    __glGenColorsRGB4_15,
    __glGenColorsRGB4_16,
    __glGenColorsRGB4_17,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    __glGenColorsRGB4_24,
    __glGenColorsRGB4_25,
    __glGenColorsRGB4_26,
    __glGenColorsRGB4_27,
    __glGenColorsRGB4_28,
    __glGenColorsRGB4_29,
    __glGenColorsRGB4_2A,
    __glGenColorsRGB4_2B,
    __glGenColorsRGB4_2C,
    __glGenColorsRGB4_2D,
    __glGenColorsRGB4_2E,
    __glGenColorsRGB4_2F,
    __glGenColorsRGB4_30,
    __glGenColorsRGB4_31,
    __glGenColorsRGB4_32,
    __glGenColorsRGB4_33,
    __glGenColorsRGB4_34,
    __glGenColorsRGB4_35,
    __glGenColorsRGB4_36,
    __glGenColorsRGB4_37,
    0,
    __glGenColorsRGB4_39,
    0,
    __glGenColorsRGB4_3B,
    0,
    __glGenColorsRGB4_3D,
    0,
    __glGenColorsRGB4_3F,
};

#endif /* __GL_PC_RAST */

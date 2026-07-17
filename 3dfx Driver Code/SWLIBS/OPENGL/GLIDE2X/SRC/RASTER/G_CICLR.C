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


#define PIXEL_TYPE CINDEX
#include "fr_fbconf.h"

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
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

void __fastcall __glGenColorsCI0_0(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    unsigned int c = tr->r;

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = c;

	    /* Step interpolants */

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
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

void __fastcall __glGenColorsCI0_1(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    unsigned int c = tr->r;

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = c;

	    /* Step interpolants */

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
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

void __fastcall __glGenColorsCI0_2(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    unsigned int c = tr->r;

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = c;

	    /* Step interpolants */

	    c += tr->drdx;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	c += tr->drdx;

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
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

void __fastcall __glGenColorsCI0_3(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    unsigned int c = tr->r;

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = c;

	    /* Step interpolants */

	    c += tr->drdx;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	c += tr->drdx;

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
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

void __fastcall __glGenColorsCI0_4(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = *tp << COLOR_FRAC_BITS;

	    /* Step interpolants */

	    tp += 1;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
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

void __fastcall __glGenColorsCI0_5(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = *tp << COLOR_FRAC_BITS;

	    /* Step interpolants */

	    tp += 1;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
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

void __fastcall __glGenColorsCI0_6(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = *tp << COLOR_FRAC_BITS;

	    /* Step interpolants */

	    tp += 1;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
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

void __fastcall __glGenColorsCI0_7(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = *tp << COLOR_FRAC_BITS;

	    /* Step interpolants */

	    tp += 1;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 1
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsCI0_8(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    unsigned int c = tr->r;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = (*tp << COLOR_FRAC_BITS) + c;

	    /* Step interpolants */

	    tp += 1;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 1
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsCI0_9(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    unsigned int c = tr->r;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = (*tp << COLOR_FRAC_BITS) + c;

	    /* Step interpolants */

	    tp += 1;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 1
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsCI0_A(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    unsigned int c = tr->r;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = (*tp << COLOR_FRAC_BITS) + c;

	    /* Step interpolants */

	    c += tr->drdx;

	    tp += 1;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	c += tr->drdx;

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_RGBA_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_TEXTURING 1
 * #define FR_MODULATE 0
 * #define FR_DECAL 0
 * #define FR_BLEND 0
 * #define FR_BLEND_INTENSITY 0
 * #define FR_REPLACE 0
 * #define FR_REPLACE_ALPHA 0
 * #define FR_MODULATE_ALPHA 0
 * #define FR_ADD 1
 * #define FR_TEXALPHA 0
 */

void __fastcall __glGenColorsCI0_B(unsigned int mask, __GLtri *tr) 
{
    __GLcontext *gc = tr->gc;
    GLbitfield saveMask = mask;

    GLuint *cp = (GLuint *) tr->colorBuf;

    unsigned int c = tr->r;

    GLubyte *tp = (GLubyte *) tr->texelBuf;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    cp[0] = (*tp << COLOR_FRAC_BITS) + c;

	    /* Step interpolants */

	    c += tr->drdx;

	    tp += 1;

	    cp += 1;

	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	c += tr->drdx;

	cp += 1;

	mask <<= 1;
    }

    (*gc->procs.afterGenColors)(saveMask, tr);
}
/*
 *
 */
void (__fastcall *__fr_ci_genclr_table[16])(unsigned int mask, __GLtri *tr) =
{
    __glGenColorsCI0_0,
    __glGenColorsCI0_1,
    __glGenColorsCI0_2,
    __glGenColorsCI0_3,
    __glGenColorsCI0_4,
    __glGenColorsCI0_5,
    __glGenColorsCI0_6,
    __glGenColorsCI0_7,
    __glGenColorsCI0_8,
    __glGenColorsCI0_9,
    __glGenColorsCI0_A,
    __glGenColorsCI0_B,
    0,
    0,
    0,
    0,
};

#endif /* __GL_PC_RAST */

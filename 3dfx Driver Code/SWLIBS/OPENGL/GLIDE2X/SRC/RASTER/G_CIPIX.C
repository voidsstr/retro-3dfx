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
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
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

void __fastcall __glGenPixelsCI1_0(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int c = tr->r;

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result = tr->r0;

	    /* Step interpolants */

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
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

void __fastcall __glGenPixelsCI1_1(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int c = tr->r;

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result = CoordToInt(c + dither);

	    /* Step interpolants */

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
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

void __fastcall __glGenPixelsCI1_2(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int c = tr->r;

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result = CoordToInt(c);

	    /* Step interpolants */

	    c += tr->drdx;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	c += tr->drdx;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
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

void __fastcall __glGenPixelsCI1_3(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int c = tr->r;

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result = CoordToInt(c + dither);

	    /* Step interpolants */

	    c += tr->drdx;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	c += tr->drdx;

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
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

void __fastcall __glGenPixelsCI1_4(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result = *tp;

	    /* Step interpolants */

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
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

void __fastcall __glGenPixelsCI1_5(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    result = *tp;

	    /* Step interpolants */

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
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

void __fastcall __glGenPixelsCI1_6(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result = *tp;

	    /* Step interpolants */

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
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

void __fastcall __glGenPixelsCI1_7(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    result = *tp;

	    /* Step interpolants */

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 0
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

void __fastcall __glGenPixelsCI1_8(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int c = tr->r;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result = *tp + CoordToInt(c);

	    /* Step interpolants */

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 0
 * #define FR_FLAT_SHADING 1
 * #define FR_DITHER 1
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

void __fastcall __glGenPixelsCI1_9(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int c = tr->r;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result = *tp + CoordToInt(c + dither);

	    /* Step interpolants */

	    tp += 1;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 0
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

void __fastcall __glGenPixelsCI1_A(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int c = tr->r;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	/* No dithering */

	    result = *tp + CoordToInt(c);

	    /* Step interpolants */

	    c += tr->drdx;

	    tp += 1;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	c += tr->drdx;

	fp += tr->dx;
	mask <<= 1;
    }

}

/*
 * #define FR_CI_COLOR 1
 * #define FR_RGB_COLOR 0
 * #define FR_SMOOTH_SHADING 1
 * #define FR_FLAT_SHADING 0
 * #define FR_DITHER 1
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

void __fastcall __glGenPixelsCI1_B(unsigned int mask, __GLtri *tr) 
{

    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;
    unsigned int result;

    unsigned int c = tr->r;

    GLubyte *tp = (GLubyte *) tr->texelBuf;
    __GLcontext *gc = tr->gc;

    (*gc->procs.extractTexels)(mask, tr);

    while (1) {
	while (((int)mask) < 0) {

	    /* Dithered */
	    int dither = *tr->dither;

	    result = *tp + CoordToInt(c + dither);

	    /* Step interpolants */

	    c += tr->drdx;

	    tp += 1;

	    tr->dither++;

	    *fp = result;
	    fp += tr->dx;
	    mask <<= 1;
	}

	if (mask == 0) break;

	/* Step interpolants */

	c += tr->drdx;

	tr->dither++;

	fp += tr->dx;
	mask <<= 1;
    }

}
/*
 *
 */
void (__fastcall *__fr_ci_pix_table[16])(unsigned int mask, __GLtri *tr) =
{
    __glGenPixelsCI1_0,
    __glGenPixelsCI1_1,
    __glGenPixelsCI1_2,
    __glGenPixelsCI1_3,
    __glGenPixelsCI1_4,
    __glGenPixelsCI1_5,
    __glGenPixelsCI1_6,
    __glGenPixelsCI1_7,
    __glGenPixelsCI1_8,
    __glGenPixelsCI1_9,
    __glGenPixelsCI1_A,
    __glGenPixelsCI1_B,
    0,
    0,
    0,
    0,
};

#endif /* __GL_PC_RAST */

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
#include "fr_tri.h"
#include "fr_fbtype.h"

#define PIXEL_TYPE 10
#include "fr_fbconf.h"

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 0
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_0(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = 0;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 1
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_1(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result & fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 2
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_2(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result & (~fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 3
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_3(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 4
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_4(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = (~result) & fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 5
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_5(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 6
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_6(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result ^ fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 7
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_7(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result | fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 8
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_8(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result | fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 9
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_9(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result ^ fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 10
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_A(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = ~fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 11
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_B(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result | (~fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 12
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_C(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~result;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 13
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_D(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = (~result) | fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 14
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_E(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result & fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 15
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_F(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = ~0;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 0
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_10(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = 0;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 1
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_11(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result & fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 2
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_12(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result & (~fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 3
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_13(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 4
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_14(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = (~result) & fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 5
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_15(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 6
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_16(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result ^ fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 7
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_17(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result | fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 8
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_18(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result | fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 9
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_19(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result ^ fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 10
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_1A(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = ~fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 11
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_1B(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result | (~fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 12
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_1C(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~result;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 13
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_1D(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = (~result) | fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 14
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_1E(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result & fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 15
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 0
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_1F(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = ~0;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 0
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_40(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = 0;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 1
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_41(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result & fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 2
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_42(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result & (~fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 3
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_43(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 4
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_44(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = (~result) & fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 5
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_45(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 6
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_46(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result ^ fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 7
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_47(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result | fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 8
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_48(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result | fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 9
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_49(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result ^ fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 10
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_4A(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = ~fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 11
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_4B(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result | (~fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 12
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_4C(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~result;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 13
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_4D(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = (~result) | fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 14
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_4E(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    sr = cp[0];
	    sg = cp[1];
	    sb = cp[2];
#if ASIZE
	    sa = cp[3];
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result & fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 15
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_4F(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = ~0;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 0
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_50(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = 0;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 1
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_51(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result & fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 2
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_52(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result & (~fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 3
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_53(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 4
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_54(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = (~result) & fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 5
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_55(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 6
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_56(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result ^ fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 7
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_57(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result | fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 8
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_58(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result | fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 9
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_59(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result ^ fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 10
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_5A(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = ~fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 11
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_5B(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = result | (~fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 12
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_5C(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~result;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 13
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_5D(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = (~result) | fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 14
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_5E(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    sr = cp[0] - (cp[0] >> RSIZE) + (dither >> RUB2C);
	    sg = cp[1] - (cp[1] >> GSIZE) + (dither >> GUB2C);
	    sb = cp[2] - (cp[2] >> BSIZE) + (dither >> BUB2C);
#if ASIZE
	    sa = cp[3] - (cp[3] >> ASIZE) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    result = ~(result & fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 0
 * #define FR_SRC_BLEND_FUNC -1
 * #define FR_DST_BLEND_FUNC -1
 * #define FR_LOGICOP 15
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 0
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_5F(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = ~0;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 0
 * #define FR_DST_BLEND_FUNC 0
 * #define FR_LOGICOP -1
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_60(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    sf = cp[3];

	    sr = BLEND(sf, cp[0]);
	    sg = BLEND(sf, cp[1]);
	    sb = BLEND(sf, cp[2]);
#if ASIZE
	    sa = BLEND(sf, cp[3]);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 0
 * #define FR_DST_BLEND_FUNC 1
 * #define FR_LOGICOP -1
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 1
 * #define FR_ALPHA 1
 * #define FR_CLAMP 1
 */

void __fastcall __glCombinePixelsRGBa_61(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    GLuint fbcolor;

    GLuint dr, dg, db;
#if ASIZE
    GLuint da;
#endif /* ASIZE */

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    dr = ((fbcolor & RMASK) >> RSHIFT) << RFRAC;
	    dg = ((fbcolor & GMASK) >> GSHIFT) << GFRAC;
	    db = ((fbcolor & BMASK) >> BSHIFT) << BFRAC;
#if ASIZE
	    da = ((fbcolor & AMASK) >> ASHIFT) << AFRAC;
#endif /* ASIZE */

	    sf = cp[3];

	    sr = BLEND(sf, cp[0]);
	    sg = BLEND(sf, cp[1]);
	    sb = BLEND(sf, cp[2]);
#if ASIZE
	    sa = BLEND(sf, cp[3]);
#endif /* ASIZE */

	    dr = (dr + sr) >> RFRAC;
	    dg = (dg + sg) >> GFRAC;
	    db = (db + sb) >> BFRAC;
#if ASIZE
	    da = (da + sa) >> AFRAC;
#endif /* ASIZE */

	    if (dr > MAX_R) dr = MAX_R;
	    if (dg > MAX_G) dg = MAX_G;
	    if (db > MAX_B) db = MAX_B;
#if ASIZE
	    if (da > MAX_A) da = MAX_A;
#endif /* ASIZE */

#if ASIZE
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT) |
		(da << ASHIFT);
#else /* ASIZE */
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 0
 * #define FR_DST_BLEND_FUNC 3
 * #define FR_LOGICOP -1
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 1
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_63(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    GLuint fbcolor;

    GLuint dr, dg, db;
#if ASIZE
    GLuint da;
#endif /* ASIZE */

    GLuint df;

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    dr = ((fbcolor & RMASK) >> RSHIFT) << RFRAC;
	    dg = ((fbcolor & GMASK) >> GSHIFT) << GFRAC;
	    db = ((fbcolor & BMASK) >> BSHIFT) << BFRAC;
#if ASIZE
	    da = ((fbcolor & AMASK) >> ASHIFT) << AFRAC;
#endif /* ASIZE */

	    sf = cp[3];

	    sr = BLEND(sf, cp[0]);
	    sg = BLEND(sf, cp[1]);
	    sb = BLEND(sf, cp[2]);
#if ASIZE
	    sa = BLEND(sf, cp[3]);
#endif /* ASIZE */

	    df = 255-cp[3];
	    dr = BLEND(df, dr);
	    dg = BLEND(df, dg);
	    db = BLEND(df, db);
#if ASIZE
	    da = BLEND(df, da);
#endif /* ASIZE */

	    dr = (dr + sr) >> RFRAC;
	    dg = (dg + sg) >> GFRAC;
	    db = (db + sb) >> BFRAC;
#if ASIZE
	    da = (da + sa) >> AFRAC;
#endif /* ASIZE */

#if ASIZE
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT) |
		(da << ASHIFT);
#else /* ASIZE */
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 4
 * #define FR_DST_BLEND_FUNC 2
 * #define FR_LOGICOP -1
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 1
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_66(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    GLuint fbcolor;

    GLuint dr, dg, db;
#if ASIZE
    GLuint da;
#endif /* ASIZE */

    GLuint df;

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    dr = ((fbcolor & RMASK) >> RSHIFT) << RFRAC;
	    dg = ((fbcolor & GMASK) >> GSHIFT) << GFRAC;
	    db = ((fbcolor & BMASK) >> BSHIFT) << BFRAC;
#if ASIZE
	    da = ((fbcolor & AMASK) >> ASHIFT) << AFRAC;
#endif /* ASIZE */

	    sf = 255-cp[3];

	    sr = BLEND(sf, cp[0]);
	    sg = BLEND(sf, cp[1]);
	    sb = BLEND(sf, cp[2]);
#if ASIZE
	    sa = BLEND(sf, cp[3]);
#endif /* ASIZE */

	    df = cp[3];
	    dr = BLEND(df, dr);
	    dg = BLEND(df, dg);
	    db = BLEND(df, db);
#if ASIZE
	    da = BLEND(df, da);
#endif /* ASIZE */

	    dr = (dr + sr) >> RFRAC;
	    dg = (dg + sg) >> GFRAC;
	    db = (db + sb) >> BFRAC;
#if ASIZE
	    da = (da + sa) >> AFRAC;
#endif /* ASIZE */

#if ASIZE
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT) |
		(da << ASHIFT);
#else /* ASIZE */
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 8
 * #define FR_DST_BLEND_FUNC 1
 * #define FR_LOGICOP -1
 * #define FR_DITHER 0
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 1
 * #define FR_ALPHA 1
 * #define FR_CLAMP 1
 */

void __fastcall __glCombinePixelsRGBa_69(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    GLuint fbcolor;

    GLuint dr, dg, db;
#if ASIZE
    GLuint da;
#endif /* ASIZE */

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    dr = ((fbcolor & RMASK) >> RSHIFT) << RFRAC;
	    dg = ((fbcolor & GMASK) >> GSHIFT) << GFRAC;
	    db = ((fbcolor & BMASK) >> BSHIFT) << BFRAC;
#if ASIZE
	    da = ((fbcolor & AMASK) >> ASHIFT) << AFRAC;
#endif /* ASIZE */

#if ASIZE
	    sf = cp[3] < (255-da) ? cp[3] : (255-da);
#else /* ASIZE */
	    sf = 0;
#endif /* ASIZE */

	    sr = BLEND(sf, cp[0]);
	    sg = BLEND(sf, cp[1]);
	    sb = BLEND(sf, cp[2]);
#if ASIZE
	    sa = BLEND(sf, cp[3]);
#endif /* ASIZE */

	    dr = (dr + sr) >> RFRAC;
	    dg = (dg + sg) >> GFRAC;
	    db = (db + sb) >> BFRAC;
#if ASIZE
	    da = (da + sa) >> AFRAC;
#endif /* ASIZE */

	    if (dr > MAX_R) dr = MAX_R;
	    if (dg > MAX_G) dg = MAX_G;
	    if (db > MAX_B) db = MAX_B;
#if ASIZE
	    if (da > MAX_A) da = MAX_A;
#endif /* ASIZE */

#if ASIZE
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT) |
		(da << ASHIFT);
#else /* ASIZE */
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 0
 * #define FR_DST_BLEND_FUNC 0
 * #define FR_LOGICOP -1
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 0
 * #define FR_DST_COMPS 0
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_70(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    sf = cp[3];

	    sr = BLEND(sf, cp[0]) + (dither >> RUB2C);
	    sg = BLEND(sf, cp[1]) + (dither >> GUB2C);
	    sb = BLEND(sf, cp[2]) + (dither >> BUB2C);
#if ASIZE
	    sa = BLEND(sf, cp[3]) + (dither >> AUB2C);
#endif /* ASIZE */

#if ASIZE
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT) |
		((sa >> AFRAC) << ASHIFT);
#else /* ASIZE */
	    result =
		((sr >> RFRAC) << RSHIFT) |
		((sg >> GFRAC) << GSHIFT) |
		((sb >> BFRAC) << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 0
 * #define FR_DST_BLEND_FUNC 1
 * #define FR_LOGICOP -1
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 1
 * #define FR_ALPHA 1
 * #define FR_CLAMP 1
 */

void __fastcall __glCombinePixelsRGBa_71(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    GLuint fbcolor;

    GLuint dr, dg, db;
#if ASIZE
    GLuint da;
#endif /* ASIZE */

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    dr = ((fbcolor & RMASK) >> RSHIFT) << RFRAC;
	    dg = ((fbcolor & GMASK) >> GSHIFT) << GFRAC;
	    db = ((fbcolor & BMASK) >> BSHIFT) << BFRAC;
#if ASIZE
	    da = ((fbcolor & AMASK) >> ASHIFT) << AFRAC;
#endif /* ASIZE */

	    sf = cp[3];

	    sr = BLEND(sf, cp[0]) + (dither >> RUB2C);
	    sg = BLEND(sf, cp[1]) + (dither >> GUB2C);
	    sb = BLEND(sf, cp[2]) + (dither >> BUB2C);
#if ASIZE
	    sa = BLEND(sf, cp[3]) + (dither >> AUB2C);
#endif /* ASIZE */

	    dr = (dr + sr) >> RFRAC;
	    dg = (dg + sg) >> GFRAC;
	    db = (db + sb) >> BFRAC;
#if ASIZE
	    da = (da + sa) >> AFRAC;
#endif /* ASIZE */

	    if (dr > MAX_R) dr = MAX_R;
	    if (dg > MAX_G) dg = MAX_G;
	    if (db > MAX_B) db = MAX_B;
#if ASIZE
	    if (da > MAX_A) da = MAX_A;
#endif /* ASIZE */

#if ASIZE
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT) |
		(da << ASHIFT);
#else /* ASIZE */
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 0
 * #define FR_DST_BLEND_FUNC 3
 * #define FR_LOGICOP -1
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 1
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_73(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    GLuint fbcolor;

    GLuint dr, dg, db;
#if ASIZE
    GLuint da;
#endif /* ASIZE */

    GLuint df;

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    dr = ((fbcolor & RMASK) >> RSHIFT) << RFRAC;
	    dg = ((fbcolor & GMASK) >> GSHIFT) << GFRAC;
	    db = ((fbcolor & BMASK) >> BSHIFT) << BFRAC;
#if ASIZE
	    da = ((fbcolor & AMASK) >> ASHIFT) << AFRAC;
#endif /* ASIZE */

	    sf = cp[3];

	    sr = BLEND(sf, cp[0]) + (dither >> RUB2C);
	    sg = BLEND(sf, cp[1]) + (dither >> GUB2C);
	    sb = BLEND(sf, cp[2]) + (dither >> BUB2C);
#if ASIZE
	    sa = BLEND(sf, cp[3]) + (dither >> AUB2C);
#endif /* ASIZE */

	    df = 255-cp[3];
	    dr = BLEND(df, dr);
	    dg = BLEND(df, dg);
	    db = BLEND(df, db);
#if ASIZE
	    da = BLEND(df, da);
#endif /* ASIZE */

	    dr = (dr + sr) >> RFRAC;
	    dg = (dg + sg) >> GFRAC;
	    db = (db + sb) >> BFRAC;
#if ASIZE
	    da = (da + sa) >> AFRAC;
#endif /* ASIZE */

#if ASIZE
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT) |
		(da << ASHIFT);
#else /* ASIZE */
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 4
 * #define FR_DST_BLEND_FUNC 2
 * #define FR_LOGICOP -1
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 1
 * #define FR_ALPHA 1
 * #define FR_CLAMP 0
 */

void __fastcall __glCombinePixelsRGBa_76(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    GLuint fbcolor;

    GLuint dr, dg, db;
#if ASIZE
    GLuint da;
#endif /* ASIZE */

    GLuint df;

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    dr = ((fbcolor & RMASK) >> RSHIFT) << RFRAC;
	    dg = ((fbcolor & GMASK) >> GSHIFT) << GFRAC;
	    db = ((fbcolor & BMASK) >> BSHIFT) << BFRAC;
#if ASIZE
	    da = ((fbcolor & AMASK) >> ASHIFT) << AFRAC;
#endif /* ASIZE */

	    sf = 255-cp[3];

	    sr = BLEND(sf, cp[0]) + (dither >> RUB2C);
	    sg = BLEND(sf, cp[1]) + (dither >> GUB2C);
	    sb = BLEND(sf, cp[2]) + (dither >> BUB2C);
#if ASIZE
	    sa = BLEND(sf, cp[3]) + (dither >> AUB2C);
#endif /* ASIZE */

	    df = cp[3];
	    dr = BLEND(df, dr);
	    dg = BLEND(df, dg);
	    db = BLEND(df, db);
#if ASIZE
	    da = BLEND(df, da);
#endif /* ASIZE */

	    dr = (dr + sr) >> RFRAC;
	    dg = (dg + sg) >> GFRAC;
	    db = (db + sb) >> BFRAC;
#if ASIZE
	    da = (da + sa) >> AFRAC;
#endif /* ASIZE */

#if ASIZE
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT) |
		(da << ASHIFT);
#else /* ASIZE */
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}

/*
 * #define FR_CI_MODE 0
 * #define FR_RGB_MODE 1
 * #define FR_BLEND 1
 * #define FR_SRC_BLEND_FUNC 8
 * #define FR_DST_BLEND_FUNC 1
 * #define FR_LOGICOP -1
 * #define FR_DITHER 1
 * #define FR_NEED_SRC 1
 * #define FR_NEED_DST 1
 * #define FR_DST_COMPS 1
 * #define FR_ALPHA 1
 * #define FR_CLAMP 1
 */

void __fastcall __glCombinePixelsRGBa_79(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLubyte *cp = tr->colorBuf;

    GLuint sr, sg, sb;
#if ASIZE
    GLuint sa;
#endif /* ASIZE */

    GLuint sf;

    GLuint result;

    GLuint fbcolor;

    GLuint dr, dg, db;
#if ASIZE
    GLuint da;
#endif /* ASIZE */

    __GLcontext *gc = tr->gc;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    dr = ((fbcolor & RMASK) >> RSHIFT) << RFRAC;
	    dg = ((fbcolor & GMASK) >> GSHIFT) << GFRAC;
	    db = ((fbcolor & BMASK) >> BSHIFT) << BFRAC;
#if ASIZE
	    da = ((fbcolor & AMASK) >> ASHIFT) << AFRAC;
#endif /* ASIZE */

#if ASIZE
	    sf = cp[3] < (255-da) ? cp[3] : (255-da);
#else /* ASIZE */
	    sf = 0;
#endif /* ASIZE */

	    sr = BLEND(sf, cp[0]) + (dither >> RUB2C);
	    sg = BLEND(sf, cp[1]) + (dither >> GUB2C);
	    sb = BLEND(sf, cp[2]) + (dither >> BUB2C);
#if ASIZE
	    sa = BLEND(sf, cp[3]) + (dither >> AUB2C);
#endif /* ASIZE */

	    dr = (dr + sr) >> RFRAC;
	    dg = (dg + sg) >> GFRAC;
	    db = (db + sb) >> BFRAC;
#if ASIZE
	    da = (da + sa) >> AFRAC;
#endif /* ASIZE */

	    if (dr > MAX_R) dr = MAX_R;
	    if (dg > MAX_G) dg = MAX_G;
	    if (db > MAX_B) db = MAX_B;
#if ASIZE
	    if (da > MAX_A) da = MAX_A;
#endif /* ASIZE */

#if ASIZE
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT) |
		(da << ASHIFT);
#else /* ASIZE */
	    result =
		(dr << RSHIFT) |
		(dg << GSHIFT) |
		(db << BSHIFT);
#endif /* ASIZE */

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 4;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 4;

    }
}
/*
 *
 */
void (__fastcall *__fr_rgb_combine_a[128])(unsigned int mask, __GLtri *tr) =
{
    __glCombinePixelsRGBa_0,
    __glCombinePixelsRGBa_1,
    __glCombinePixelsRGBa_2,
    __glCombinePixelsRGBa_3,
    __glCombinePixelsRGBa_4,
    __glCombinePixelsRGBa_5,
    __glCombinePixelsRGBa_6,
    __glCombinePixelsRGBa_7,
    __glCombinePixelsRGBa_8,
    __glCombinePixelsRGBa_9,
    __glCombinePixelsRGBa_A,
    __glCombinePixelsRGBa_B,
    __glCombinePixelsRGBa_C,
    __glCombinePixelsRGBa_D,
    __glCombinePixelsRGBa_E,
    __glCombinePixelsRGBa_F,
    __glCombinePixelsRGBa_10,
    __glCombinePixelsRGBa_11,
    __glCombinePixelsRGBa_12,
    __glCombinePixelsRGBa_13,
    __glCombinePixelsRGBa_14,
    __glCombinePixelsRGBa_15,
    __glCombinePixelsRGBa_16,
    __glCombinePixelsRGBa_17,
    __glCombinePixelsRGBa_18,
    __glCombinePixelsRGBa_19,
    __glCombinePixelsRGBa_1A,
    __glCombinePixelsRGBa_1B,
    __glCombinePixelsRGBa_1C,
    __glCombinePixelsRGBa_1D,
    __glCombinePixelsRGBa_1E,
    __glCombinePixelsRGBa_1F,
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
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    __glCombinePixelsRGBa_40,
    __glCombinePixelsRGBa_41,
    __glCombinePixelsRGBa_42,
    __glCombinePixelsRGBa_43,
    __glCombinePixelsRGBa_44,
    __glCombinePixelsRGBa_45,
    __glCombinePixelsRGBa_46,
    __glCombinePixelsRGBa_47,
    __glCombinePixelsRGBa_48,
    __glCombinePixelsRGBa_49,
    __glCombinePixelsRGBa_4A,
    __glCombinePixelsRGBa_4B,
    __glCombinePixelsRGBa_4C,
    __glCombinePixelsRGBa_4D,
    __glCombinePixelsRGBa_4E,
    __glCombinePixelsRGBa_4F,
    __glCombinePixelsRGBa_50,
    __glCombinePixelsRGBa_51,
    __glCombinePixelsRGBa_52,
    __glCombinePixelsRGBa_53,
    __glCombinePixelsRGBa_54,
    __glCombinePixelsRGBa_55,
    __glCombinePixelsRGBa_56,
    __glCombinePixelsRGBa_57,
    __glCombinePixelsRGBa_58,
    __glCombinePixelsRGBa_59,
    __glCombinePixelsRGBa_5A,
    __glCombinePixelsRGBa_5B,
    __glCombinePixelsRGBa_5C,
    __glCombinePixelsRGBa_5D,
    __glCombinePixelsRGBa_5E,
    __glCombinePixelsRGBa_5F,
    __glCombinePixelsRGBa_60,
    __glCombinePixelsRGBa_61,
    0,
    __glCombinePixelsRGBa_63,
    0,
    0,
    __glCombinePixelsRGBa_66,
    0,
    0,
    __glCombinePixelsRGBa_69,
    0,
    0,
    0,
    0,
    0,
    0,
    __glCombinePixelsRGBa_70,
    __glCombinePixelsRGBa_71,
    0,
    __glCombinePixelsRGBa_73,
    0,
    0,
    __glCombinePixelsRGBa_76,
    0,
    0,
    __glCombinePixelsRGBa_79,
    0,
    0,
    0,
    0,
    0,
    0,
};

#endif /* __GL_PC_RAST */

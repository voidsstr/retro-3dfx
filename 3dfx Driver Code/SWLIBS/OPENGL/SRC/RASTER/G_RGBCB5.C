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

#define PIXEL_TYPE 5
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

void __fastcall __glCombinePixelsRGB5_0(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_1(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_2(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_3(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_4(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_5(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_6(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_7(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_8(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_9(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_A(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_B(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_C(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_D(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_E(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_F(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_10(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_11(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_12(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_13(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_14(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_15(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_16(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_17(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_18(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_19(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_1A(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_1B(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_1C(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_1D(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_1E(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_1F(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_40(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_41(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_42(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_43(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_44(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_45(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_46(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_47(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_48(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_49(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_4A(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_4B(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_4C(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_4D(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_4E(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_4F(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_50(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_51(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_52(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_53(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_54(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_55(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_56(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_57(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_58(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_59(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_5A(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_5B(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_5C(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_5D(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_5E(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_5F(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_60(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_61(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_63(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_66(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_69(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_70(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_71(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_73(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_76(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB5_79(unsigned int mask, __GLtri *tr) 
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
void (__fastcall *__fr_rgb_combine_5[128])(unsigned int mask, __GLtri *tr) =
{
    __glCombinePixelsRGB5_0,
    __glCombinePixelsRGB5_1,
    __glCombinePixelsRGB5_2,
    __glCombinePixelsRGB5_3,
    __glCombinePixelsRGB5_4,
    __glCombinePixelsRGB5_5,
    __glCombinePixelsRGB5_6,
    __glCombinePixelsRGB5_7,
    __glCombinePixelsRGB5_8,
    __glCombinePixelsRGB5_9,
    __glCombinePixelsRGB5_A,
    __glCombinePixelsRGB5_B,
    __glCombinePixelsRGB5_C,
    __glCombinePixelsRGB5_D,
    __glCombinePixelsRGB5_E,
    __glCombinePixelsRGB5_F,
    __glCombinePixelsRGB5_10,
    __glCombinePixelsRGB5_11,
    __glCombinePixelsRGB5_12,
    __glCombinePixelsRGB5_13,
    __glCombinePixelsRGB5_14,
    __glCombinePixelsRGB5_15,
    __glCombinePixelsRGB5_16,
    __glCombinePixelsRGB5_17,
    __glCombinePixelsRGB5_18,
    __glCombinePixelsRGB5_19,
    __glCombinePixelsRGB5_1A,
    __glCombinePixelsRGB5_1B,
    __glCombinePixelsRGB5_1C,
    __glCombinePixelsRGB5_1D,
    __glCombinePixelsRGB5_1E,
    __glCombinePixelsRGB5_1F,
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
    __glCombinePixelsRGB5_40,
    __glCombinePixelsRGB5_41,
    __glCombinePixelsRGB5_42,
    __glCombinePixelsRGB5_43,
    __glCombinePixelsRGB5_44,
    __glCombinePixelsRGB5_45,
    __glCombinePixelsRGB5_46,
    __glCombinePixelsRGB5_47,
    __glCombinePixelsRGB5_48,
    __glCombinePixelsRGB5_49,
    __glCombinePixelsRGB5_4A,
    __glCombinePixelsRGB5_4B,
    __glCombinePixelsRGB5_4C,
    __glCombinePixelsRGB5_4D,
    __glCombinePixelsRGB5_4E,
    __glCombinePixelsRGB5_4F,
    __glCombinePixelsRGB5_50,
    __glCombinePixelsRGB5_51,
    __glCombinePixelsRGB5_52,
    __glCombinePixelsRGB5_53,
    __glCombinePixelsRGB5_54,
    __glCombinePixelsRGB5_55,
    __glCombinePixelsRGB5_56,
    __glCombinePixelsRGB5_57,
    __glCombinePixelsRGB5_58,
    __glCombinePixelsRGB5_59,
    __glCombinePixelsRGB5_5A,
    __glCombinePixelsRGB5_5B,
    __glCombinePixelsRGB5_5C,
    __glCombinePixelsRGB5_5D,
    __glCombinePixelsRGB5_5E,
    __glCombinePixelsRGB5_5F,
    __glCombinePixelsRGB5_60,
    __glCombinePixelsRGB5_61,
    0,
    __glCombinePixelsRGB5_63,
    0,
    0,
    __glCombinePixelsRGB5_66,
    0,
    0,
    __glCombinePixelsRGB5_69,
    0,
    0,
    0,
    0,
    0,
    0,
    __glCombinePixelsRGB5_70,
    __glCombinePixelsRGB5_71,
    0,
    __glCombinePixelsRGB5_73,
    0,
    0,
    __glCombinePixelsRGB5_76,
    0,
    0,
    __glCombinePixelsRGB5_79,
    0,
    0,
    0,
    0,
    0,
    0,
};

#endif /* __GL_PC_RAST */

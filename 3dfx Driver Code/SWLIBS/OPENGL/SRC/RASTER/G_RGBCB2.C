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

#define PIXEL_TYPE 2
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

void __fastcall __glCombinePixelsRGB2_0(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_1(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_2(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_3(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_4(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_5(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_6(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_7(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_8(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_9(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_A(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_B(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_C(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_D(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_E(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_F(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_10(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_11(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_12(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_13(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_14(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_15(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_16(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_17(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_18(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_19(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_1A(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_1B(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_1C(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_1D(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_1E(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_1F(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_40(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_41(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_42(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_43(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_44(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_45(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_46(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_47(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_48(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_49(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_4A(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_4B(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_4C(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_4D(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_4E(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_4F(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_50(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_51(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_52(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_53(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_54(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_55(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_56(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_57(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_58(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_59(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_5A(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_5B(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_5C(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_5D(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_5E(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_5F(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_60(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_61(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_63(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_66(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_69(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_70(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_71(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_73(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_76(unsigned int mask, __GLtri *tr) 
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

void __fastcall __glCombinePixelsRGB2_79(unsigned int mask, __GLtri *tr) 
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
void (__fastcall *__fr_rgb_combine_2[128])(unsigned int mask, __GLtri *tr) =
{
    __glCombinePixelsRGB2_0,
    __glCombinePixelsRGB2_1,
    __glCombinePixelsRGB2_2,
    __glCombinePixelsRGB2_3,
    __glCombinePixelsRGB2_4,
    __glCombinePixelsRGB2_5,
    __glCombinePixelsRGB2_6,
    __glCombinePixelsRGB2_7,
    __glCombinePixelsRGB2_8,
    __glCombinePixelsRGB2_9,
    __glCombinePixelsRGB2_A,
    __glCombinePixelsRGB2_B,
    __glCombinePixelsRGB2_C,
    __glCombinePixelsRGB2_D,
    __glCombinePixelsRGB2_E,
    __glCombinePixelsRGB2_F,
    __glCombinePixelsRGB2_10,
    __glCombinePixelsRGB2_11,
    __glCombinePixelsRGB2_12,
    __glCombinePixelsRGB2_13,
    __glCombinePixelsRGB2_14,
    __glCombinePixelsRGB2_15,
    __glCombinePixelsRGB2_16,
    __glCombinePixelsRGB2_17,
    __glCombinePixelsRGB2_18,
    __glCombinePixelsRGB2_19,
    __glCombinePixelsRGB2_1A,
    __glCombinePixelsRGB2_1B,
    __glCombinePixelsRGB2_1C,
    __glCombinePixelsRGB2_1D,
    __glCombinePixelsRGB2_1E,
    __glCombinePixelsRGB2_1F,
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
    __glCombinePixelsRGB2_40,
    __glCombinePixelsRGB2_41,
    __glCombinePixelsRGB2_42,
    __glCombinePixelsRGB2_43,
    __glCombinePixelsRGB2_44,
    __glCombinePixelsRGB2_45,
    __glCombinePixelsRGB2_46,
    __glCombinePixelsRGB2_47,
    __glCombinePixelsRGB2_48,
    __glCombinePixelsRGB2_49,
    __glCombinePixelsRGB2_4A,
    __glCombinePixelsRGB2_4B,
    __glCombinePixelsRGB2_4C,
    __glCombinePixelsRGB2_4D,
    __glCombinePixelsRGB2_4E,
    __glCombinePixelsRGB2_4F,
    __glCombinePixelsRGB2_50,
    __glCombinePixelsRGB2_51,
    __glCombinePixelsRGB2_52,
    __glCombinePixelsRGB2_53,
    __glCombinePixelsRGB2_54,
    __glCombinePixelsRGB2_55,
    __glCombinePixelsRGB2_56,
    __glCombinePixelsRGB2_57,
    __glCombinePixelsRGB2_58,
    __glCombinePixelsRGB2_59,
    __glCombinePixelsRGB2_5A,
    __glCombinePixelsRGB2_5B,
    __glCombinePixelsRGB2_5C,
    __glCombinePixelsRGB2_5D,
    __glCombinePixelsRGB2_5E,
    __glCombinePixelsRGB2_5F,
    __glCombinePixelsRGB2_60,
    __glCombinePixelsRGB2_61,
    0,
    __glCombinePixelsRGB2_63,
    0,
    0,
    __glCombinePixelsRGB2_66,
    0,
    0,
    __glCombinePixelsRGB2_69,
    0,
    0,
    0,
    0,
    0,
    0,
    __glCombinePixelsRGB2_70,
    __glCombinePixelsRGB2_71,
    0,
    __glCombinePixelsRGB2_73,
    0,
    0,
    __glCombinePixelsRGB2_76,
    0,
    0,
    __glCombinePixelsRGB2_79,
    0,
    0,
    0,
    0,
    0,
    0,
};

#endif /* __GL_PC_RAST */

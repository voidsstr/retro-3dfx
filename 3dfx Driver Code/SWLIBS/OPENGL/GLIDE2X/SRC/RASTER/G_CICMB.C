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

#define PIXEL_TYPE CINDEX
#include "fr_fbconf.h"

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_0(unsigned int mask, __GLtri *tr) 
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
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_1(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = result & fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_2(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = result & (~fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_3(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = CoordToInt(*cp);

	    result = result;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_4(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = (~result) & fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_5(unsigned int mask, __GLtri *tr) 
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
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_6(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = result ^ fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_7(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = result | fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_8(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = ~(result | fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_9(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = ~(result ^ fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_A(unsigned int mask, __GLtri *tr) 
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
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_B(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = result | (~fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_C(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    result = CoordToInt(*cp);

	    result = ~result;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_D(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = (~result) | fbcolor;

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_E(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    fbcolor = *fp;

	    result = CoordToInt(*cp);

	    result = ~(result & fbcolor);

	    *fp = result;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_F(unsigned int mask, __GLtri *tr) 
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
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_10(unsigned int mask, __GLtri *tr) 
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
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_11(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = result & fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_12(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = result & (~fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_13(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    result = CoordToInt(*cp + dither);

	    result = result;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_14(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = (~result) & fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_15(unsigned int mask, __GLtri *tr) 
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
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_16(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = result ^ fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_17(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = result | fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_18(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = ~(result | fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_19(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = ~(result ^ fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_1A(unsigned int mask, __GLtri *tr) 
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
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_1B(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = result | (~fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_1C(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    result = CoordToInt(*cp + dither);

	    result = ~result;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_1D(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = (~result) | fbcolor;

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_1E(unsigned int mask, __GLtri *tr) 
{
    FR_PIXEL *fp = (FR_PIXEL *) tr->cp;

    GLuint *cp = (GLuint *) tr->colorBuf;

    GLuint result;

    GLuint fbcolor;

    while (1) {
	while (((int)mask) < 0) {

	    int dither = *tr->dither;

	    fbcolor = *fp;

	    result = CoordToInt(*cp + dither);

	    result = ~(result & fbcolor);

	    *fp = result;

	    tr->dither++;

	    mask <<= 1;
	    fp += tr->dx;

	    cp += 1;

	}

	if (mask == 0) break;

	tr->dither++;

	mask <<= 1;
	fp += tr->dx;

	cp += 1;

    }
}

/*
 * #define FR_CI_MODE 1
 * #define FR_RGB_MODE 0
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

void __fastcall __glCombinePixelsCI0_1F(unsigned int mask, __GLtri *tr) 
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
 *
 */
void (__fastcall *__fr_ci_combine_table[32])(unsigned int mask, __GLtri *tr) =
{
    __glCombinePixelsCI0_0,
    __glCombinePixelsCI0_1,
    __glCombinePixelsCI0_2,
    __glCombinePixelsCI0_3,
    __glCombinePixelsCI0_4,
    __glCombinePixelsCI0_5,
    __glCombinePixelsCI0_6,
    __glCombinePixelsCI0_7,
    __glCombinePixelsCI0_8,
    __glCombinePixelsCI0_9,
    __glCombinePixelsCI0_A,
    __glCombinePixelsCI0_B,
    __glCombinePixelsCI0_C,
    __glCombinePixelsCI0_D,
    __glCombinePixelsCI0_E,
    __glCombinePixelsCI0_F,
    __glCombinePixelsCI0_10,
    __glCombinePixelsCI0_11,
    __glCombinePixelsCI0_12,
    __glCombinePixelsCI0_13,
    __glCombinePixelsCI0_14,
    __glCombinePixelsCI0_15,
    __glCombinePixelsCI0_16,
    __glCombinePixelsCI0_17,
    __glCombinePixelsCI0_18,
    __glCombinePixelsCI0_19,
    __glCombinePixelsCI0_1A,
    __glCombinePixelsCI0_1B,
    __glCombinePixelsCI0_1C,
    __glCombinePixelsCI0_1D,
    __glCombinePixelsCI0_1E,
    __glCombinePixelsCI0_1F,
};

#endif /* __GL_PC_RAST */

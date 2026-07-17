#ifndef __gl_machdep_h_
#define __gl_machdep_h_

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
**
** $Header: machdep.h, 2, 10/11/00 8:05:11 PM, Brent$
*/
#include "context.h"

/*
** Machine dependent implementation limits
*/
#define	__GL_DEFAULT_NUMBER_OF_CLIP_PLANES	6
#define	__GL_DEFAULT_NUMBER_OF_LIGHTS		8
#define __GL_DEFAULT_NUMBER_OF_TEXTURES		1
#define __GL_DEFAULT_NUMBER_OF_TEXTURE_ENVS	1

#define	__GL_DEFAULT_MAX_MODELVIEW_STACK_DEPTH	32
#define	__GL_DEFAULT_MAX_PROJECTION_STACK_DEPTH	2
#define	__GL_DEFAULT_MAX_TEXTURE_STACK_DEPTH	10
#define	__GL_DEFAULT_MAX_ATTRIB_STACK_DEPTH	16
#define	__GL_DEFAULT_MAX_CLIENT_ATTRIB_STACK_DEPTH	16
#define	__GL_DEFAULT_MAX_NAME_STACK_DEPTH	128
#define __GL_DEFAULT_MAX_EVAL_ORDER		30
#define __GL_DEFAULT_MAX_MIPMAP_LEVEL		11
#define __GL_DEFAULT_MAX_PIXEL_MAP_TABLE	65536
#define __GL_DEFAULT_MAX_LIST_NESTING		64
#define __GL_DEFAULT_MAX_CONV_1D_WIDTH		11
#define __GL_DEFAULT_MAX_CONV_2D_WIDTH		11
#define __GL_DEFAULT_MAX_CONV_2D_HEIGHT		11
#define __GL_DEFAULT_MAX_SEP_2D_WIDTH		11
#define __GL_DEFAULT_MAX_SEP_2D_HEIGHT		11

#define __GL_DEFAULT_POINT_SIZE_MINIMUM		0.5
#define __GL_DEFAULT_POINT_SIZE_MAXIMUM		10.0
#define __GL_DEFAULT_POINT_SIZE_GRANULARITY	0.125

#define __GL_DEFAULT_LINE_WIDTH_MINIMUM		0.5 
#define __GL_DEFAULT_LINE_WIDTH_MAXIMUM		10.0
#define __GL_DEFAULT_LINE_WIDTH_GRANULARITY	0.125

/************************************************************************/

#define	__GL_DEFAULT_MAX_WINDOW_HEIGHT		2048
#define	__GL_DEFAULT_MAX_WINDOW_WIDTH		2048

/*
** Bias numbers for moving window coordinates into a positive space.
** These values are used during viewport computations.
*/
#define __GL_DEFAULT_VERTEX_X_BIAS		2048
#define __GL_DEFAULT_VERTEX_Y_BIAS		2048

/* 
** Fixing numbers.  These are used to move the biased window coordinates
** into a range where the number of fraction bits are constant from the
** minimal value in the range to the largest value in the range.
*/
#define __GL_DEFAULT_VERTEX_X_FIX		4096
#define __GL_DEFAULT_VERTEX_Y_FIX		4096

#define __GL_DEFAULT_COORD_SUBPIXEL_BITS	3

/************************************************************************/

/*
** Primary dispatch tables
*/
extern __GLdispatchState __glImmedState;
extern __GLdispatchState __glListCompState;
extern __GLdispatchState __glNopDispatchState;

/************************************************************************/

/*
** Device dependent context state
*/
typedef struct __GLDDcontextRec {
    /* This must be first in this structure */
    struct __GLcontextRec gc;

    /* Which buffer is visible 0 or 1 */
    GLint displayBank;
} __GLDDcontext;

#endif /* __gl_machdep_h_ */

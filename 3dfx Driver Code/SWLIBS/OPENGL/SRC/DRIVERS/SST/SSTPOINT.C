/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 2$ 
** $Date: 10/11/00 8:03:08 PM$ 
**
*/

#include "sstcontext.h"
#include "global.h"
#include "imports.h"

void __glSSTRenderPoint(__GLcontext *gc, __GLvertex *v) {
    grDrawPoint(v);
}

void __glSSTRenderWidePoint(__GLcontext *gc, __GLvertex *v) {
    GLint pointSize, pointSizeHalf;
    __GLvertex temp0, temp1;
   
    pointSize = gc->state.point.aliasedSize;
    pointSizeHalf = pointSize >> 1;
    
    /* Shift window coords */
    v->window.x -= pointSizeHalf;
    v->window.y -= pointSizeHalf;

    /* XXXwheeler - this should be optimized */
    __GL_GLIDE_SET_DIRTY(cullMode);
    grCullMode(GR_CULL_DISABLE);
    temp0 = temp1 = *v;

    temp0.window.y += pointSize;
    temp1.window.x += pointSize;
    grDrawTriangle(v, &temp0, &temp1);

    v->window.x += pointSize;
    v->window.y += pointSize;
    grDrawTriangle(v, &temp0, &temp1);

    /* Restore window coords */
    v->window.x -= (pointSize - pointSizeHalf);
    v->window.y -= (pointSize - pointSizeHalf);
}

void __glSSTRenderWideAAPoint(__GLcontext *gc, __GLvertex *v) {
    __GLvertex temp0, temp1;
    __GLfloat radius, ang;
    __GLfloat x, y, prevx, prevy;
    GLint i, n;

    /* XXXwheeler - this routine is ganked from the beta and it sucks balls */

    radius = gc->state.point.smoothSize / 2.0f;

    /* XXXwheeler - this should be optimized */
    __GL_GLIDE_SET_DIRTY(cullMode);
    grCullMode(GR_CULL_DISABLE);
    temp0 = temp1 = *v;

    prevx = prevy = 0.0f;

    /* 
    ** XXXshui This is almost certainly wrong. We draw a number of 
    ** triangles with an antialiased outer edge.  Choose radius * 4
    ** because hopefully that covers every pixel along the perimeter
    ** of the circle.  Also need to honor gc.constants.pointSizeGranularity.
    */
    n = (int)(radius * 4 + __glHalf);
    if (n < 4) n = 4;

    for (i=0; i <= n; i++) {
        ang = i / (float) n * 2.0 * __glPi;
        x = v->window.x + radius * __GL_COSF(ang);
        y = v->window.y + radius * __GL_SINF(ang);
        temp0.window.x = prevx;
        temp0.window.y = prevy;
        temp1.window.x = x;
        temp1.window.y = y;
        prevx = x;
        prevy = y;
        if (i == 0) {
	    continue;
        }
	grAADrawTriangle(v, &temp0, &temp1, FXFALSE, FXTRUE, FXFALSE);

    }
}

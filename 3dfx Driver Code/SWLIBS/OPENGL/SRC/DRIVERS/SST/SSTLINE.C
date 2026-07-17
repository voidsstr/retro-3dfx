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
** $Date: 10/11/00 8:03:07 PM$ 
**
*/

#include "sstcontext.h"

void __glSSTRenderLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1) {
    grDrawLine(v0, v1);
}

/* XXXwheeler - this is not anti-aliased yet */
void __glSSTRenderWideAALine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    __GLvertex temp0, temp1;
    __GLfloat dx, dy, w, x_shift, y_shift;
    GLint ix, iy;
    GLboolean flat;

    flat = !(gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT);
    w = gc->state.line.aliasedWidth / 2.0f;
    dx = v1->window.x - v0->window.x;
    dy = v1->window.y - v0->window.y;
    ix = (*(GLint *)&dx) & 0x7fffffff;
    iy = (*(GLint *)&dy) & 0x7fffffff;

    if (ix > iy) {
        x_shift = 0;
        y_shift = w;
    } else {
        x_shift = w;
        y_shift = 0;
    }

    /* XXXwheeler - this should be optimized */
    __GL_GLIDE_SET_DIRTY(cullMode);
    grCullMode(GR_CULL_DISABLE);
    temp0 = *v0;
    temp1 = *v1;

    /* shift window coords */
    v0->window.x += x_shift;
    v0->window.y -= y_shift;
    temp0.window.x -= x_shift;
    temp0.window.y += y_shift;

    v1->window.x += x_shift;
    v1->window.y -= y_shift;
    temp1.window.x -= x_shift;
    temp1.window.y += y_shift;

    grDrawTriangle(&temp0, v0, v1);
    grDrawTriangle(&temp0, v1, &temp1);

    /* restore window coords */
    v0->window.x -= x_shift;
    v0->window.y += y_shift;
    v1->window.x -= x_shift;
    v1->window.y += y_shift;
}

void __glSSTRenderWideLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    __GLvertex temp0, temp1;
    __GLfloat dx, dy, w, x_shift, y_shift;
    GLint ix, iy;
    GLboolean flat;

    flat = !(gc->polygon.shader.modeFlags & __GL_SHADE_SMOOTH_LIGHT);
    w = gc->state.line.aliasedWidth / 2.0f;
    dx = v1->window.x - v0->window.x;
    dy = v1->window.y - v0->window.y;
    ix = (*(GLint *)&dx) & 0x7fffffff;
    iy = (*(GLint *)&dy) & 0x7fffffff;

    if (ix > iy) {
        x_shift = 0;
        y_shift = w;
    } else {
        x_shift = w;
        y_shift = 0;
    }

    /* XXXwheeler - this should be optimized */
    __GL_GLIDE_SET_DIRTY(cullMode);
    grCullMode(GR_CULL_DISABLE);
    temp0 = *v0;
    temp1 = *v1;

    /* shift window coords */
    v0->window.x += x_shift;
    v0->window.y -= y_shift;
    temp0.window.x -= x_shift;
    temp0.window.y += y_shift;

    v1->window.x += x_shift;
    v1->window.y -= y_shift;
    temp1.window.x -= x_shift;
    temp1.window.y += y_shift;

    grDrawTriangle(&temp0, v0, v1);
    grDrawTriangle(&temp0, v1, &temp1);

    /* restore window coords */
    v0->window.x -= x_shift;
    v0->window.y += y_shift;
    v1->window.x -= x_shift;
    v1->window.y += y_shift;
}



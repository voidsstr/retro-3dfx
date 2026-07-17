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
** $Revision: 2$
** $Date: 10/11/00 8:04:26 PM$
*/
#include "context.h"
#include "global.h"

void __glRasterPos2(__GLcontext *gc, const __GLfloat v[2])
{
    __GLvertex *rp = &gc->state.current.rasterPos;
    __GLtransform *tr;
    GLuint needs;
    void (*cc)(__GLcontext*, GLint, __GLvertex*);
    void (*ct)(__GLcontext*, __GLvertex*);

    rp->obj.x = v[0];
    rp->obj.y = v[1];
    rp->obj.z = __glZero;
    rp->obj.w = __glOne;

    tr = gc->transform.modelView;
    (*tr->mvp.xf2)(&rp->clip, &rp->obj.x, &tr->mvp);

    /* clip check the raster pos */
    rp->hasAndClipCode &= __GL_ALL_CLIP_MASK;
    rp->hasAndClipCode |= __GL_HAS_VERTEX_2D;
    if ((*gc->procs.clipCheck2)(gc, rp)) {
	gc->state.current.validRasterPos = GL_FALSE;
	return;
    }
    gc->state.current.validRasterPos = GL_TRUE;

    /* Copy info into vertex structure */
    rp->normal = gc->state.current.normal;
    if (gc->modes.rgbMode) {
	rp->colors[__GL_FRONTFACE] = gc->state.current.color;
    } else {
	rp->colors[__GL_FRONTFACE].r = gc->state.current.userColorIndex;
    }
    rp->texture = gc->state.current.texture;

    /* This may be generous, but it will work */
    needs = gc->vertex.needs | __GL_HAS_EYE | __GL_HAS_NORMAL
	| __GL_HAS_TEXTURE | __GL_HAS_FRONT_COLOR;

    /*
    ** Save away calcColor and calcTexture procs, then validate vertex.
    */
    cc = gc->procs.calcColor;
    gc->procs.calcColor = gc->procs.calcRasterColor;
    ct = gc->procs.calcTexture;
    gc->procs.calcTexture = gc->procs.calcRasterTexture;
    DO_VALIDATE(gc, rp, needs);
    gc->procs.calcColor = cc;
    gc->procs.calcTexture = ct;

    if (gc->renderMode == GL_SELECT) {
	__glSelectPoint(gc, rp);
    }
}

void __glRasterPos3(__GLcontext *gc, const __GLfloat v[3])
{
    __GLvertex *rp = &gc->state.current.rasterPos;
    __GLtransform *tr;
    GLuint needs;
    void (*cc)(__GLcontext*, GLint, __GLvertex*);
    void (*ct)(__GLcontext*, __GLvertex*);

    rp->obj.x = v[0];
    rp->obj.y = v[1];
    rp->obj.z = v[2];
    rp->obj.w = __glOne;

    tr = gc->transform.modelView;
    (*tr->mvp.xf3)(&rp->clip, &rp->obj.x, &tr->mvp);

    /* clip check the raster pos */
    rp->hasAndClipCode &= __GL_ALL_CLIP_MASK;
    rp->hasAndClipCode |= __GL_HAS_VERTEX_3D;
    if ((*gc->procs.clipCheck3)(gc, rp)) {
	gc->state.current.validRasterPos = GL_FALSE;
	return;
    }
    gc->state.current.validRasterPos = GL_TRUE;

    /* Copy info into vertex structure */
    rp->normal = gc->state.current.normal;
    if (gc->modes.rgbMode) {
	rp->colors[__GL_FRONTFACE] = gc->state.current.color;
    } else {
	rp->colors[__GL_FRONTFACE].r = gc->state.current.userColorIndex;
    }
    rp->texture = gc->state.current.texture;

    /* This may be generous, but it will work */
    needs = gc->vertex.needs | __GL_HAS_EYE | __GL_HAS_NORMAL
	| __GL_HAS_TEXTURE | __GL_HAS_FRONT_COLOR;

    /*
    ** Save away calcColor and calcTexture procs, then validate vertex.
    */
    cc = gc->procs.calcColor;
    gc->procs.calcColor = gc->procs.calcRasterColor;
    ct = gc->procs.calcTexture;
    gc->procs.calcTexture = gc->procs.calcRasterTexture;
    DO_VALIDATE(gc, rp, needs);
    gc->procs.calcColor = cc;
    gc->procs.calcTexture = ct;

    if (gc->renderMode == GL_SELECT) {
	__glSelectPoint(gc, rp);
    }
}

void __glRasterPos4(__GLcontext *gc, const __GLfloat v[4])
{
    __GLvertex *rp = &gc->state.current.rasterPos;
    __GLtransform *tr;
    GLuint needs;
    void (*cc)(__GLcontext*, GLint, __GLvertex*);
    void (*ct)(__GLcontext*, __GLvertex*);

    rp->obj.x = v[0];
    rp->obj.y = v[1];
    rp->obj.z = v[2];
    rp->obj.w = v[3];

    tr = gc->transform.modelView;
    (*tr->mvp.xf4)(&rp->clip, &rp->obj.x, &tr->mvp);

    /* clip check the raster pos */
    rp->hasAndClipCode &= __GL_ALL_CLIP_MASK;
    rp->hasAndClipCode |= __GL_HAS_VERTEX_4D;
    if ((*gc->procs.clipCheck4)(gc, rp)) {
	gc->state.current.validRasterPos = GL_FALSE;
	return;
    }
    gc->state.current.validRasterPos = GL_TRUE;

    /* Copy info into vertex structure */
    rp->normal = gc->state.current.normal;
    if (gc->modes.rgbMode) {
	rp->colors[__GL_FRONTFACE] = gc->state.current.color;
    } else {
	rp->colors[__GL_FRONTFACE].r = gc->state.current.userColorIndex;
    }
    rp->texture = gc->state.current.texture;

    /* This may be generous, but it will work */
    needs = gc->vertex.needs | __GL_HAS_EYE | __GL_HAS_NORMAL
	| __GL_HAS_TEXTURE | __GL_HAS_FRONT_COLOR;

    /*
    ** Save away calcColor and calcTexture procs, then validate vertex.
    */
    cc = gc->procs.calcColor;
    gc->procs.calcColor = gc->procs.calcRasterColor;
    ct = gc->procs.calcTexture;
    gc->procs.calcTexture = gc->procs.calcRasterTexture;
    DO_VALIDATE(gc, rp, needs);
    gc->procs.calcColor = cc;
    gc->procs.calcTexture = ct;

    if (gc->renderMode == GL_SELECT) {
	__glSelectPoint(gc, rp);
    }
}

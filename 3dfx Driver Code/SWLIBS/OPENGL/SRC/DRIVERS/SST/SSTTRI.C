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
** $Date: 10/11/00 8:03:09 PM$ 
**
*/

#include "sstcontext.h"

/*
** Generic triangle handling code.  This code is used when render mode
** is GL_RENDER and the polygon modes are not both fill.
*/
void __glSSTRenderTriangle(__GLcontext *gc, __GLvertex *a, __GLvertex *b,
			   __GLvertex *c)
{
    GLuint needs, modeFlags, faceNeeds;
    GLint ccw, colorFace, face;
    __GLfloat dxAC, dxBC, dyAC, dyBC, area;
    __GLvertex *pv;

    /* Compute signed area of the triangle */
    dxAC = a->window.x - c->window.x;
    dxBC = b->window.x - c->window.x;
    dyAC = a->window.y - c->window.y;
    dyBC = b->window.y - c->window.y;
    area = dxAC * dyBC - dxBC * dyAC;
    ccw = !(*(int *)&area >> 31);

    /* Figure out if face is culled or not */
    face = gc->polygon.face[ccw];
    if (face == gc->polygon.cullFace) {
	/* Culled */
	return;
    }

    /*
    ** Pick face to use for coloring
    */
    modeFlags = gc->polygon.shader.modeFlags;
    if (modeFlags & __GL_SHADE_TWOSIDED) {
	colorFace = face;
	faceNeeds = gc->vertex.faceNeeds[face];
    } else {
	colorFace = __GL_FRONTFACE;
	faceNeeds = gc->vertex.faceNeeds[__GL_FRONTFACE];
    }

    /*
    ** Choose colors for the vertices.
    */
    needs = gc->vertex.needs;
    pv = gc->vertex.provoking;
    if (modeFlags & __GL_SHADE_SMOOTH_LIGHT) {
	/* Smooth shading */
	a->color = &a->colors[colorFace];
	b->color = &b->colors[colorFace];
	c->color = &c->colors[colorFace];
	needs |= faceNeeds;
    } else {
	GLuint pvneeds;

	/*
	** Validate the lighting (and color) information in the provoking
	** vertex only.  Fill routines always use gc->vertex.provoking->color
	** to find the color.
	*/
	pv->color = &pv->colors[colorFace];
	a->color = pv->color;
	b->color = pv->color;
	c->color = pv->color;
	pvneeds = faceNeeds & (__GL_HAS_LIGHTING | 
		__GL_HAS_FRONT_COLOR | __GL_HAS_BACK_COLOR);
	if (~pv->hasAndClipCode & pvneeds) DO_VALIDATE(gc, pv, pvneeds);
    }

    /* Validate vertices */
    if (~a->hasAndClipCode & needs) DO_VALIDATE(gc, a, needs);
    if (~b->hasAndClipCode & needs) DO_VALIDATE(gc, b, needs);
    if (~c->hasAndClipCode & needs) DO_VALIDATE(gc, c, needs);

    /* Render triangle using the faces polygon mode */
    switch (gc->polygon.mode[face]) {
      case __GL_POLYGON_MODE_FILL:
	if (*(int *)&area << 1) {
            grDrawTriangle(a, b, c);
	}
	break;
      case __GL_POLYGON_MODE_POINT:
	if (a->boundaryEdge) (*gc->procs.renderPoint)(gc, a);
	if (b->boundaryEdge) (*gc->procs.renderPoint)(gc, b);
	if (c->boundaryEdge) (*gc->procs.renderPoint)(gc, c);
	break;
      case __GL_POLYGON_MODE_LINE:
	if (a->boundaryEdge) (*gc->procs.renderLine)(gc, a, b);
	if (b->boundaryEdge) (*gc->procs.renderLine)(gc, b, c);
	if (c->boundaryEdge) (*gc->procs.renderLine)(gc, c, a);
	break;
    }

    /* Restore color pointers */
    a->color = &a->colors[__GL_FRONTFACE];
    b->color = &b->colors[__GL_FRONTFACE];
    c->color = &c->colors[__GL_FRONTFACE];
    pv->color = &pv->colors[__GL_FRONTFACE];
}

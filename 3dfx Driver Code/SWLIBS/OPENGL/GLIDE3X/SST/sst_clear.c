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
** $Date: 10/11/00 7:57:48 PM$
*/
#include <windows.h>
#include <glide.h>
#include "context.h"
#include "global.h"
#include "sst_imfncs.h"
#include "sst_globals.h"

void APIENTRY __glsstim_Clear(GLbitfield mask)
{
    __GL_SETUP();
    GLuint beginMode;
    int r, g, b, a;
    GrColor_t color;
    GrAlpha_t alpha;
    FxU16 depth;
    int doColor, doDepth;
    int doFront = GL_FALSE, doBack = GL_FALSE;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    beginMode = __gl_beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    __gl_beginMode = __GL_NOT_IN_BEGIN;
	    glClear(mask);
	    return;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

    __GL_API_NOTBE_RENDER();

    if (mask & ~(GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT
		 | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }

    if (gc->renderMode == GL_RENDER) {
        if (mask & GL_COLOR_BUFFER_BIT) {
            if (modeFlags & __GL_SHADE_MASK) {
                doColor = GL_FALSE;
                switch (gc->state.raster.drawBuffer) {
                case GL_NONE:
                    break;
                case GL_FRONT:
                    (*gc->front->clear)(gc->front);
                    break;
                case GL_BACK:
                    if (gc->modes.doubleBufferMode) {
                        (*gc->back->clear)(gc->back);
                    }
                    break;
                }
            } else {
                doColor = gc->state.raster.rMask;
            }
        } else {
            doColor = GL_FALSE;
        }
	
	switch (gc->state.raster.drawBuffer) {
	case GL_NONE:
	    break;
	case GL_FRONT:
	    doFront = GL_TRUE;
	    break;
	case GL_BACK:
	    doBack = GL_TRUE;
	    break;
	case GL_FRONT_AND_BACK:
	    doFront = GL_TRUE;
	    doBack = GL_TRUE;
	    break;
	}

	grColorMask(doColor,FXFALSE); /* XXX alpha mask */

	doDepth = (mask & GL_DEPTH_BUFFER_BIT) && gc->modes.haveDepthBuffer;
	if (doDepth && gc->state.depth.writeEnable) {
	    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
	    grDepthMask(FXTRUE);
	} else {
	    grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
	    grDepthMask(FXFALSE);
	}

	r = gc->state.raster.clear.r * __GL_SST_MAX_RED;
	g = gc->state.raster.clear.g * __GL_SST_MAX_GREEN;
	b = gc->state.raster.clear.b * __GL_SST_MAX_BLUE;
	a = gc->state.raster.clear.a * __GL_SST_MAX_ALPHA;
	/* XXX need to reflect the component ordering */
	color = r << 16 | g << 8 | b;
	alpha = a;
	/* XXX name the max Z value of Glide API */
	depth = gc->state.depth.clear * 0xffff;

	if (doFront) {
	    grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	    grBufferClear(color, alpha, depth);
	}
	if (doBack) {
	    grRenderBuffer(GR_BUFFER_BACKBUFFER);
	    grBufferClear(color, alpha, depth);
	}

	/* restore glide state */
	grColorMask(gc->state.raster.rMask,FXFALSE);
	if (gc->state.enables.general & __GL_DEPTH_TEST_ENABLE) {
	    grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
	    grDepthMask(gc->state.depth.writeEnable);
	} else {
	    grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
	    grDepthMask(FXFALSE);
	}
	switch (gc->state.raster.drawBuffer) {
	case GL_FRONT:
	    grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	    break;
	case GL_BACK:
	    grRenderBuffer(GR_BUFFER_BACKBUFFER);
	    break;
	case GL_NONE:
	case GL_FRONT_AND_BACK:
	    /* XXXshui not supported in hw */
	    break;
	}

	if ((mask & GL_ACCUM_BUFFER_BIT) && gc->modes.haveAccumBuffer) {
	    (*gc->accumBuffer.clear)(&gc->accumBuffer);
	}
	if ((mask & GL_STENCIL_BUFFER_BIT) && gc->modes.haveStencilBuffer) {
	    (*gc->stencilBuffer.clear)(&gc->stencilBuffer);
	}
    }
}

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
** $Date: 10/11/00 8:03:00 PM$
*/
#include <glide.h>
#include "context.h"
#include "global.h"
#include "apimacro.h"
#include "sstimfuncs.h"
#include "sstcontext.h"
#include "sstconstants.h"

void APIENTRY __glsstim_Clear(GLbitfield mask)
{
    __GL_SETUP();
    GLuint beginMode;
    int r, g, b, a;
    GrColor_t color;
    GrAlpha_t alpha;
    FxU32 depth;
    GLboolean clearcolorbuf = GL_FALSE;
    GLboolean cleardepthbuf = GL_FALSE;
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
	    switch (gc->state.raster.drawBuffer) {
	      case GL_NONE:
		break;
	      case GL_FRONT:
		if (modeFlags & __GL_SHADE_MASK) {
		    (*gc->front->clear)(gc->back);
		} else {
		    clearcolorbuf = GL_TRUE;
		}
		break;
	      case GL_BACK:
		if (modeFlags & __GL_SHADE_MASK) {
		    (*gc->back->clear)(gc->back);
		} else {
		    clearcolorbuf = GL_TRUE;
		}
		break;
	      case GL_FRONT_AND_BACK:
		(*gc->front->clear)(gc->front);
		(*gc->back->clear)(gc->back);
		break;
	    }
	}

	if ((mask & GL_DEPTH_BUFFER_BIT) && gc->modes.haveDepthBuffer) {
            cleardepthbuf = GL_TRUE;
	}

        if (clearcolorbuf || cleardepthbuf) {
	    if (!clearcolorbuf) {
/* mask off colorbuffer using Rob's macro   The logic goes like this, the
   buffer is already enabled and we should clear it because the user didn't set
   the bit in the clear mask and we need to mask the write through glide and 
   then restore the state.  */
		grColorMask(FXFALSE, FXFALSE);
		__GL_GLIDE_SET_DIRTY(colorMask);
	    } else if (!cleardepthbuf) {
/* mask off colorbuffer using Rob's macro */
		grDepthMask(FXFALSE);
		__GL_GLIDE_SET_DIRTY(depthMask);
	    }

	    r = gc->state.raster.clear.r * __GL_SST_MAX_RED;
	    g = gc->state.raster.clear.g * __GL_SST_MAX_GREEN;
	    b = gc->state.raster.clear.b * __GL_SST_MAX_BLUE;
	    a = gc->state.raster.clear.a * __GL_SST_MAX_ALPHA;
	    /*  need to reflect the component ordering */
	    color = r << 16 | g << 8 | b;
	    alpha = a;
	    depth = gc->state.depth.clear * __GL_SST_MAX_DEPTH;

	    grBufferClear(color, alpha, depth);
        }

	if ((mask & GL_ACCUM_BUFFER_BIT) && gc->modes.haveAccumBuffer) {
	    (*gc->accumBuffer.clear)(&gc->accumBuffer);
	}
	if ((mask & GL_STENCIL_BUFFER_BIT) && gc->modes.haveStencilBuffer) {
	    (*gc->stencilBuffer.clear)(&gc->stencilBuffer);
	}
    }
}

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
** $Date: 10/11/00 8:06:56 PM$
*/

/*
** initialization and allocation of drawablePrivate side of the
** buffer structures
*/

#include <ddraw.h>

#include "wgllib.h"
#include "gldevice.h"
#include "wglddraw.h"
#include "wgldci.h"
#include "wgldib.h"
#include "wglmem.h"

void
__wglDDrawInitDrawable(__WGLdrawablePrivate *wglPriv, __GLcontextModes *modes)
{
    __GLdrawablePrivate *glPriv;
    GLint colorBits;
    GLint accumColorBits;

    glPriv = &wglPriv->glPriv;
    colorBits = modes->rgbBits;
    accumColorBits = modes->accumRedBits + modes->accumGreenBits +
	modes->accumBlueBits + modes->accumAlphaBits;

    /* initialize swap buffers routines first */
    wglPriv->swapBuffersMain = __wglDDrawDDrawSwapBuffers;
    wglPriv->swapBuffersFallback = __wglDDrawMemSwapBuffers;
    wglPriv->swapBuffers = wglPriv->swapBuffersMain;

    /* initialize front/back color buffers */
    if (modes->doubleBufferMode) {
	if (modes->colorIndexMode) {
	    __wglInitDDrawPrimary(&glPriv->frontBuffer, glPriv);
	    __wglInitDDraw(&glPriv->backBuffer, glPriv, modes->indexBits, 
			   __wglInitMem);
	} else {
	    __wglInitDDrawPrimary(&glPriv->frontBuffer, glPriv);
	    __wglInitDDraw(&glPriv->backBuffer, glPriv, colorBits, 
			   __wglInitMem);
	}
    } else {
	__wglInitDDrawPrimary(&glPriv->frontBuffer, glPriv);
    }

    glPriv->yInverted = GL_TRUE;

#if __GL_MAX_AUXBUFFERS > 0
    /* initilize the aux color buffers */
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i = 0; i < modes->maxAuxBuffers, i++) {
	    if (modes->colorIndexMode) {
		__wglInitDDraw(&glPriv->auxBuffer[i], glPriv, modes->indexBits, 
			       __wglInitMem);
	    } else {
		__wglInitDDraw(&glPriv->auxBuffer[i], glPriv, colorBits, 
			       __wglInitMem);
	    }
	}
    }
#endif /* __GL_MAX_AUXBUFFERS */

    /* initialize the other ancillary buffers */
    if (modes->haveAccumBuffer) {
	__wglInitMem(&glPriv->accumBuffer, glPriv, accumColorBits);
    }
    if (modes->haveDepthBuffer) {
	__wglInitDDrawDepth(&glPriv->depthBuffer, glPriv, modes->depthBits, 
			    __wglInitMem);
    }
    if (modes->haveStencilBuffer) {
	__wglInitDDraw(&glPriv->stencilBuffer, glPriv, modes->stencilBits, 
		       __wglInitMem);
    }

    /* owndership buffer is 1 bpp */
    __wglInitDIB(&glPriv->ownershipBuffer, glPriv, 1);
}

#if 0
void
__wglDCIInitDrawable(__WGLdrawablePrivate *wglPriv, __GLcontextModes *modes)
{
    __GLdrawablePrivate *glPriv;
    GLint colorBits;
    GLint accumColorBits;

    glPriv = &wglPriv->glPriv;
    colorBits = modes->rgbBits;
    accumColorBits = modes->accumRedBits + modes->accumGreenBits +
	modes->accumBlueBits + modes->accumAlphaBits;

    /* initialize swap buffers routines first */
    wglPriv->swapBuffersMain = __wglDCIDIBSwapBuffers;
    wglPriv->swapBuffersFallback = NULL;
    wglPriv->swapBuffers = wglPriv->swapBuffersMain;

    /* initialize front/back color buffers */
    if (modes->doubleBufferMode) {
	if (modes->colorIndexMode) {
	    __wglInitDCI(&glPriv->frontBuffer, glPriv, modes->indexBits);
	    __wglInitDIB(&glPriv->backBuffer, glPriv, modes->indexBits);
	} else {
	    __wglInitDCI(&glPriv->frontBuffer, glPriv, colorBits);
	    __wglInitDIB(&glPriv->backBuffer, glPriv, colorBits);
	}
    } else {
	if (modes->colorIndexMode) {
	    __wglInitDCI(&glPriv->frontBuffer, glPriv, modes->indexBits);
	} else {
	    __wglInitDCI(&glPriv->frontBuffer, glPriv, colorBits);
	}
    }

    glPriv->yInverted = GL_TRUE;

#if __GL_MAX_AUXBUFFERS > 0
    /* initilize the aux color buffers */
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i = 0; i < modes->maxAuxBuffers, i++) {
	    if (modes->colorIndexMode) {
		__wglInitDIB(&glPriv->auxBuffer[i], glPriv, modes->indexBits);
	    } else {
		__wglInitDIB(&glPriv->auxBuffer[i], glPriv, colorBits);
	    }
	}
    }
#endif /* __GL_MAX_AUXBUFFERS */

    /* initialize the other ancillary buffers */
    if (modes->haveAccumBuffer) {
	__wglInitMem(&glPriv->accumBuffer, glPriv, accumColorBits);
    }
    if (modes->haveDepthBuffer) {
	__wglInitMem(&glPriv->depthBuffer, glPriv, modes->depthBits);
    }
    if (modes->haveStencilBuffer) {
	__wglInitDIB(&glPriv->stencilBuffer, glPriv, modes->stencilBits);
    }

    /* owndership buffer is 1 bpp */
    __wglInitDIB(&glPriv->ownershipBuffer, glPriv, 1);
}
#endif

void
__wglDIBInitDrawable(__WGLdrawablePrivate *wglPriv, __GLcontextModes *modes)
{
    __GLdrawablePrivate *glPriv;
    GLint colorBits;
    GLint accumColorBits;

    glPriv = &wglPriv->glPriv;
    colorBits = modes->redBits + modes->greenBits +
	modes->blueBits + modes->alphaBits;
    accumColorBits = modes->accumRedBits + modes->accumGreenBits +
	modes->accumBlueBits + modes->accumAlphaBits;

    /* initialize front/back color buffers */
    if (modes->colorIndexMode) {
	__wglInitUnmanagedDIB(&glPriv->frontBuffer, glPriv, modes->indexBits);
    } else {
	__wglInitUnmanagedDIB(&glPriv->frontBuffer, glPriv, colorBits);
    }

#if __GL_MAX_AUXBUFFERS > 0
    /* initilize the aux color buffers */
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i = 0; i < modes->maxAuxBuffers, i++) {
	    if (modes->colorIndexMode) {
		__wglInitDIB(&glPriv->auxBuffer[i], glPriv, modes->indexBits);
	    } else {
		__wglInitDIB(&glPriv->auxBuffer[i], glPriv, colorBits);
	    }
	}
    }
#endif /* __GL_MAX_AUXBUFFERS */

    /* initialize the other ancillary buffers */
    if (modes->haveAccumBuffer) {
	__wglInitMem(&glPriv->accumBuffer, glPriv, accumColorBits);
    }
    if (modes->haveDepthBuffer) {
	__wglInitDIB(&glPriv->depthBuffer, glPriv, modes->depthBits);
    }
    if (modes->haveStencilBuffer) {
	__wglInitDIB(&glPriv->stencilBuffer, glPriv, modes->stencilBits);
    }

    /* owndership buffer is 1 bpp */
    __wglInitDIB(&glPriv->ownershipBuffer, glPriv, 1);
}

void
__wglFreeBuffers(__WGLdrawablePrivate *wglPriv)
{
    __GLdrawablePrivate *glPriv = &wglPriv->glPriv;
    __GLcontextModes *modes = glPriv->modes;

    if (glPriv->frontBuffer.free) {
	(*glPriv->frontBuffer.free)(&glPriv->frontBuffer, glPriv);
    }
    if (glPriv->backBuffer.free) {
	(*glPriv->backBuffer.free)(&glPriv->backBuffer, glPriv);
    }

#if __GL_MAX_AUXBUFFERS > 0
    if (modes->maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < modes->maxAuxBuffers; i++) {
	    if (glPriv->auxBuffer[i].free) {
		(*glPriv->auxBuffer[i].free)(&glPriv->auxBuffer[i], glPriv);
	    }
	}
    }
#endif /* __GL_MAX_AUXBUFFERS */

    if (glPriv->accumBuffer.free) {
	(*glPriv->accumBuffer.free)(&glPriv->accumBuffer, glPriv);
    }

    if (glPriv->depthBuffer.free) {
	(*glPriv->depthBuffer.free)(&glPriv->depthBuffer, glPriv);
    }

    if (glPriv->stencilBuffer.free) {
	(*glPriv->stencilBuffer.free)(&glPriv->stencilBuffer, glPriv);
    }

    if (glPriv->ownershipBuffer.free) {
	(*glPriv->ownershipBuffer.free)(&glPriv->ownershipBuffer, glPriv);
    }
}




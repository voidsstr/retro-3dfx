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
*/
#include "wgllib.h"

static int (*getDisplayMasks)(int *, int *, int *);
static void (*updatePalette)(__WGLdrawablePrivate *);
static BOOL (*swapBuffers)(__WGLdrawablePrivate *);

static void (*freeFrontBuffer)(__GLdrawableBuffer *, __WGLdrawablePrivate *);
static void (*updateFrontBuffer)(__GLdrawableBuffer *, __GLdrawablePrivate *);
static void (*lockFrontBuffer)(__GLdrawableBuffer *, __GLdrawablePrivate *);
static void (*unlockFrontBuffer)(__GLdrawableBuffer *, __GLdrawablePrivate *);

static void (*freeBackBuffer)(__GLdrawableBuffer *, __WGLdrawablePrivate *);
static void (*updateBackBuffer)(__GLdrawableBuffer *, __GLdrawablePrivate *);
static void (*lockBackBuffer)(__GLdrawableBuffer *, __GLdrawablePrivate *);
static void (*unlockBackBuffer)(__GLdrawableBuffer *, __GLdrawablePrivate *);

static void (*lockDepthBuffer)(__GLdrawableBuffer *, __GLdrawablePrivate *);
static void (*unlockDepthBuffer)(__GLdrawableBuffer *, __GLdrawablePrivate *);

#if defined(__WGL_USE_DIRECTDRAW)
static BOOL __wglUseDDrawFront;
static BOOL __wglUseDDrawBack;

BOOL
__wglFBInit(void)
{
    __wglUseDDrawFront = FALSE;
    if (__wglDCIInit() == FALSE) {
	__wglUseDDrawFront = TRUE;
	if (__wglDDrawInit() == FALSE) {
	    return FALSE;
	}
    }

    if (__wglUseDDrawFront) {
	getDisplayMasks = __wglDDrawGetDisplayMasks;

	/* Front buffer methods */
	freeFrontBuffer = __wglDDrawFreeBuffer;
	updateFrontBuffer = __wglDDrawUpdateBuffer;
	lockFrontBuffer = __wglDDrawLockBuffer;
	unlockFrontBuffer = __wglDDrawUnlockBuffer;
    } else {
	getDisplayMasks = __wglDCIGetDisplayMasks;

	/* Front buffer methods */
	freeFrontBuffer = __wglDCIFreeBuffer;
	updateFrontBuffer = __wglDCIUpdateBuffer;
	lockFrontBuffer = __wglDCILockBuffer;
	unlockFrontBuffer = __wglDCIUnlockBuffer;
    }

    if (__wglUseDDrawFront && __wglUseDDrawBack) {
	updatePalette = __wglDDrawUpdateDrawablePalette;
	swapBuffers = __wglDDrawSwapBuffers;

	/* Back buffer methods */
	freeBackBuffer = __wglDDrawFreeBuffer;
	updateBackBuffer = __wglDDrawUpdateBuffer;
	lockBackBuffer = __wglDDrawLockBuffer;
	unlockBackBuffer = __wglDDrawUnlockBuffer;
    } else {
	updatePalette = __wglMemUpdateDrawablePalette;
	swapBuffers = __wglMemSwapBuffers;

	/* Back buffer methods */
	freeBackBuffer = __wglMemFreeBuffer;
	updateBackBuffer = __wglMemUpdateBuffer;
	lockBackBuffer = NULL;
	unlockBackBuffer = NULL;
    }

    return TRUE;
}

BOOL
__wglFBFinish(void)
{
    if (__wglUseDDrawFront) {
	return __wglDDrawFinish();
    } else {
	return __wglDCIFinish();
    }
}
#elif defined( __WGL_USE_DCI )
BOOL
__wglFBInit(void)
{
    if (__wglDCIInit() == FALSE) {
	return FALSE;
    }

    getDisplayMasks = __wglDCIGetDisplayMasks;

    updatePalette = __wglMemUpdateDrawablePalette;
    swapBuffers = __wglMemSwapBuffers;

    freeFrontBuffer = __wglDCIFreeBuffer;
    updateFrontBuffer = __wglDCIUpdateBuffer;
    lockFrontBuffer = __wglDCILockBuffer;
    unlockFrontBuffer = __wglDCIUnlockBuffer;

    freeBackBuffer = __wglMemFreeBuffer;
    updateBackBuffer = __wglMemUpdateBuffer;
    lockBackBuffer = NULL;
    unlockBackBuffer = NULL;

    return TRUE;
}

BOOL
__wglFBFinish(void)
{
    return __wglDCIFinish();
}
#elif defined( __WGL_USE_GLIDE )

/*-------------------------------------------------------------------
  Function: __wglFBInit
  Date: 9/16
  Implementor(s): jdt
  Library: OpenGL 
  Description:
  Initialize frame buffer library and set frame buffer
  operating pointers.
  Arguments:
  none
  Return:
  TRUE  - succeed
  FALSE - fail
  -------------------------------------------------------------------*/
BOOL __wglFBInit(void)
{
    if (__wglGlideInit() == FALSE) {
		return FALSE;
    }

    getDisplayMasks   = __wglGlideGetDisplayMasks;

    updatePalette     = __wglMemUpdateDrawablePalette;
    swapBuffers       = __wglGlideSwapBuffers;

    freeFrontBuffer   = __wglGlideFreeBuffer;
    updateFrontBuffer = __wglGlideUpdateBuffer;
    lockFrontBuffer   = __wglGlideLockBuffer;
    unlockFrontBuffer = __wglGlideUnlockBuffer;

    freeBackBuffer    = __wglGlideFreeBuffer;
    updateBackBuffer  = __wglGlideUpdateBuffer;
    lockBackBuffer    = __wglGlideLockBuffer;
    unlockBackBuffer  = __wglGlideUnlockBuffer;

    lockDepthBuffer    = __wglGlideLockBuffer;
    unlockDepthBuffer  = __wglGlideUnlockBuffer;

    return TRUE;
}

BOOL
__wglFBFinish(void)
{
	return TRUE;
}
#else
#error "Frame buffer method not defined __WGL_USE_*"
#endif /* defined(__WGL_USE_DIRECTDRAW) */

void
__wglFBFreeBuffers(__WGLdrawablePrivate *wglPriv)
{
    (*freeFrontBuffer)(&wglPriv->glPriv.frontBuffer, wglPriv);
    (*freeBackBuffer)(&wglPriv->glPriv.backBuffer, wglPriv);
    __wglMemFreeBuffer(&wglPriv->glPriv.ownershipBuffer, wglPriv);
}

int
__wglFBGetDisplayMasks(int *rMask, int *gMask, int *bMask)
{
    return (*getDisplayMasks)(rMask, gMask, bMask);
}

void
__wglFBInitDrawable(__WGLdrawablePrivate *wglPriv)
{
    __GLdrawablePrivate *glPriv = &wglPriv->glPriv;
    int bitsPerPixel = glPriv->modes->indexBits;
    int elementSize;

    wglPriv->freeBuffers = __wglFBFreeBuffers;
    wglPriv->updatePalette = updatePalette;
    wglPriv->swapBuffers = swapBuffers;

    switch (bitsPerPixel) {
    case 8:
        elementSize = 1;
	break;
    case 16:
        elementSize = 2;
	break;
    case 24:
        elementSize = 3;
	break;
    case 32:
        elementSize = 4;
	break;
    }

    glPriv->frontBuffer.depth = bitsPerPixel;
    glPriv->frontBuffer.elementSize = elementSize;
    glPriv->frontBuffer.update = updateFrontBuffer;
    glPriv->frontBuffer.lock = lockFrontBuffer;
    glPriv->frontBuffer.unlock = unlockFrontBuffer;
    glPriv->frontBuffer.fill = NULL;
    glPriv->frontBuffer.type = GR_BUFFER_FRONTBUFFER;

    glPriv->backBuffer.depth = bitsPerPixel;
    glPriv->backBuffer.elementSize = elementSize;
    glPriv->backBuffer.update = updateBackBuffer;
    glPriv->backBuffer.lock = lockBackBuffer;
    glPriv->backBuffer.unlock = unlockBackBuffer;
    glPriv->backBuffer.fill = NULL;
    glPriv->backBuffer.type = GR_BUFFER_BACKBUFFER;

    glPriv->depthBuffer.lock = lockDepthBuffer;
    glPriv->depthBuffer.unlock = unlockDepthBuffer;

    glPriv->ownershipBuffer.depth = 1;
    glPriv->ownershipBuffer.elementSize = 1;
    glPriv->ownershipBuffer.update = __wglMemUpdateBuffer;
    glPriv->ownershipBuffer.lock = NULL;
    glPriv->ownershipBuffer.unlock = NULL;
    glPriv->ownershipBuffer.fill = NULL;

    glPriv->yInverted = TRUE;
}

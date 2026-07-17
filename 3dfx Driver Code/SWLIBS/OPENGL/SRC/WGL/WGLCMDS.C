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
#include "g_xproto.h"
#include "gldevice.h"

int WINAPI wglGetPixelFormat(HDC hDC);
int WINAPI wglDescribePixelFormat(HDC hDC, int iPixelFormat,
				  UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);


static int freeHandle = 1;

extern __GLcontext *__wglInvalidGC;
/************************************************************/

HGLRC WINAPI
wglCreateContext(HDC hDC)
{
    int pixelFormat = wglGetPixelFormat(hDC);
    __WGLcontext *glrc;

    if (pixelFormat == 0) {
        __wglSetSystemError("wglCreateContex", WGL_BAD_PIXEL_FORMAT);
	return (HGLRC) 0;
    }

    glrc = (__WGLcontext *) __wglMalloc(sizeof(__WGLcontext));
    if (glrc == NULL) {
        __wglSetSystemError("wglCreateContext", WGL_MALLOC_FAILED);
	return (HGLRC) 0;
    }
    memset(glrc, 0, sizeof(*glrc));

    if (!wglDescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &glrc->pFD)) {
        __wglFree(glrc);
        __wglSetSystemError("wglCreateContext", WGL_UNKNOWN_PIXEL_FORMAT);
	return (HGLRC) 0;
    }

    glrc->isInUse = FALSE;
    glrc->owner = (DWORD) __WGL_NO_OWNER;

    glrc->pixelFormat = pixelFormat;
    glrc->hasBeenCurrent = FALSE;
    glrc->pendingDestroy = FALSE;
    glrc->pendingWindowChange = TRUE;
    glrc->isDirect = TRUE;

    __wglLockMutex();
    glrc->hGLRC = (HGLRC) freeHandle++;

    if (!__wglCreateHWContext(glrc)) {
        __wglFree(glrc);
        __wglSetSystemError("wglCreateContext", WGL_ERROR);
	__wglUnlockMutex();
	return (HGLRC) 0;
    }

    __wglAddContext(glrc);
    __wglUnlockMutex();

    return glrc->hGLRC;
}

BOOL WINAPI
wglDeleteContext(HGLRC hGLRC)
{
    __GLcontext *gc = __wglGetCurrentGC();
    __WGLcontext *glrc = __wglFindWGLContext(hGLRC);

    if (gc == NULL) {
	__wglAttachThread(GetCurrentThreadId());
	gc = __wglGetCurrentGC();
    }

    if (glrc == NULL) {
        __wglSetSystemError("wglDeleteContext", WGL_INVALID_HRC);
	return FALSE;
    }

    if (glrc->isInUse) {
	if (glrc->owner == GetCurrentThreadId()) {
	    __WGLdrawablePrivate *wglPriv = glrc->wglPriv;

	    if (!(*gc->exports.loseCurrent)(gc)) {
		return FALSE;
	    }

	    wglPriv->refCount--;
	    if (wglPriv->hWnd == NULL) {
		__wglDestroyDrawablePrivate(wglPriv);
	    }

	    glrc->wglPriv = NULL;
	    glrc->isInUse = FALSE;
	    glrc->owner = (DWORD) __WGL_NO_OWNER;
	    glrc->hWnd = NULL;
	    glrc->hDC = NULL;
	    __wglSetCurrentGC(__wglInvalidGC);
	    __glCoreNopDispatch();
	} else {
	    return FALSE;
	}
    }

    if (!(*glrc->gc->exports.destroyContext)(glrc->gc)) {
        __wglSetSystemError("wglDeleteContext", WGL_ERROR);
	return FALSE;
    }

    __wglLockMutex();
    __wglRemoveContext(glrc);
    __wglUnlockMutex();

    __wglFree(glrc);
    return TRUE;
}

HGLRC WINAPI
wglGetCurrentContext(void)
{
    __GLcontext *gc = __wglGetCurrentGC();
    __WGLcontext *glrc;

    if (gc == NULL) {
	__wglAttachThread(GetCurrentThreadId());
	gc = __wglGetCurrentGC();
    }

    glrc = (__WGLcontext *) gc->imports.other;
    return glrc->hGLRC;
}

HDC WINAPI
wglGetCurrentDC(void)
{
    __GLcontext *gc = __wglGetCurrentGC();
    __WGLcontext *glrc;

    if (gc == NULL) {
	__wglAttachThread(GetCurrentThreadId());
	gc = __wglGetCurrentGC();
    }

    glrc = (__WGLcontext *) gc->imports.other;
    return glrc->hDC;
}

BOOL WINAPI
wglMakeCurrent(HDC hDC, HGLRC hGLRC)
{
    __GLcontext *gcOld = __wglGetCurrentGC();
    __WGLcontext *glrc = __wglFindWGLContext(hGLRC);
    DWORD objType = GetObjectType(hDC);
    HWND hWnd = NULL;
    HBITMAP hBitmap = NULL;
    __WGLdrawablePrivate *wglPriv;

    if (gcOld == NULL) {
	__wglAttachThread(GetCurrentThreadId());
	gcOld = __wglGetCurrentGC();
    }

    if (hGLRC != (HGLRC) NULL) {

	if (glrc == NULL) {
	    __wglSetSystemError("wglMakeCurrent", WGL_INVALID_HRC);
	    return FALSE;
	}

	switch (objType) {
	case OBJ_DC:
	    hWnd = WindowFromDC(hDC);
	    break;
	case OBJ_MEMDC:
	    hBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
	    break;
	default:
	    return FALSE;
	}

	if ((hWnd == NULL) && (hBitmap == NULL)) {
	    __wglSetSystemError("wglMakeCurrent", WGL_INVALID_HDC);
	    return FALSE;
	}

	if (glrc->isInUse && (glrc->owner != GetCurrentThreadId())) {
	    __wglSetSystemError("wglMakeCurrent", WGL_CONTEXT_IN_USE);
	    return FALSE;
	}

	wglPriv = __wglGetDrawablePrivate(hDC, hWnd, glrc->modes);
	if (wglPriv == NULL) {
	    __wglSetSystemError("wglMakeCurrent", WGL_INVALID_HDC);
	    return FALSE;
	}

	if (glrc->hDC == hDC && glrc->gc == gcOld && wglPriv->hWnd == hWnd) {
	    return TRUE;
	}

	if (wglPriv->pixelFormat != glrc->pixelFormat) {
	    __wglSetSystemError("wglMakeCurrent", WGL_DIFFERENT_P_FMAT);
	    return FALSE;
	}
    }

    __wglLockMutex();

    /* LoseCurrent from old context */
    if (gcOld != __wglInvalidGC) {
	__WGLcontext *glrcOld = (__WGLcontext *) gcOld->imports.other;
	__WGLdrawablePrivate *wglPriv = glrcOld->wglPriv;

	if (!(*gcOld->exports.loseCurrent)(gcOld)) {
	    __wglUnlockMutex();
	    return FALSE;
	}

	wglPriv->refCount--;

#if 0
	/*
	** If hWnd == NULL, it means that even when the context thinks it is
	** attached to that drawable, it really isn't, because the window
	** has gone away.  If this was the last context that this drawable
	** was bound to (refCount == 0), get rid of it.
	*/
	if ((wglPriv->hWnd == NULL) && (wglPriv->refCount == 0)) {
	    __wglDestroyDrawablePrivate(wglPriv);
	}
#endif

	glrcOld->wglPriv = NULL;
	glrcOld->isInUse = FALSE;
	glrcOld->owner = (DWORD) __WGL_NO_OWNER;
	glrc->hWnd = NULL;
	glrc->hDC = NULL;
	__wglSetCurrentGC(__wglInvalidGC);
    }

    /* MakeCurrent to new context */
    if (hGLRC != NULL) {
	__GLcontext *gc = glrc->gc;

	wglPriv->refCount++;
	glrc->wglPriv = wglPriv;
/* XXXXX  we need to know the hdc in order to get resolution info in MakeCurrent
	  This is yet again, a vicious hack!  */
	glrc->hDC = hDC;

	if ((*gc->exports.makeCurrent)(gc) == 0) {
	    glrc->hDC = NULL;
	    __wglUnlockMutex();
	    return FALSE;
	}
	__wglSetCurrentGC(gc);

	/* Set the flag to say that the context is in use. */
	glrc->isInUse = TRUE;
	glrc->owner	= GetCurrentThreadId();
	glrc->hWnd = hWnd;
	glrc->hasBeenCurrent = TRUE;
    } else {
	__glCoreNopDispatch();
    }

    __wglUnlockMutex();

    /* Has the Window's size/position changed ? */
    if (hGLRC != NULL && glrc->pendingWindowChange) {
	__wglUpdateDrawableBuffers(glrc);
    }

    return TRUE;
}

BOOL WINAPI
wglShareLists(HGLRC hGLRC1, HGLRC hGLRC2)
{
    __GLcontext *gcSrc, *gcDst;
    __WGLcontext *glrcSrc, *glrcDst;

    glrcSrc = __wglFindWGLContext(hGLRC1);
    if (glrcSrc == NULL) {
	__wglSetSystemError("wglShareLists", WGL_SHR_INVALID_SRC_CONTEXT);
	return FALSE;
    }

    glrcDst = __wglFindWGLContext(hGLRC2);
    if (glrcDst == NULL) {
	__wglSetSystemError("wglShareLists", WGL_SHR_INVALID_DST_CONTEXT);
	return FALSE;
    }

    __wglLockMutex();

    gcSrc = glrcSrc->gc;
    gcDst = glrcDst->gc;
    if (!(*gcDst->exports.shareContext)(gcDst, gcSrc)) {
	__wglSetSystemError("wglShareLists", WGL_SHR_DST_LIST_NOT_EMPTY);
	return FALSE;
    }

    __wglUnlockMutex();

    return TRUE;
}

BOOL WINAPI
wglSwapBuffers(HDC hDC)
{
    HWND hWnd = WindowFromDC(hDC);
    __WGLcontext *glrc;
    
    /* Make sure that the device context has a valid window */
    if (hWnd == NULL) {
        return FALSE;
    }

    /* Make sure there is a current context, and that it is double buffered */
    glrc = __wglFindWGLWindow(hWnd, NULL);
    if ((glrc == NULL) || !glrc->modes->doubleBufferMode) {
        return FALSE;
    }

    (*glrc->gc->exports.swapBuffers)(glrc->gc);

    return (*glrc->wglPriv->swapBuffers)(glrc->wglPriv);
}

BOOL WINAPI
wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask)
{
    __wglSetSystemError("wglCopyContext not supported", WGL_ERROR);
    return FALSE;
}

/* ---------------------------------------------------------------------- */

int WINAPI
wglDescribePixelFormat(HDC hDC, int iPixelFormat,
		       UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
{
    int numPixelFormats;
    PIXELFORMATDESCRIPTOR pfd;

    if (ppfd) {
	numPixelFormats = 
	    (*__glDevice->devGetPixelFormat)(&pfd, iPixelFormat-1);
    } else {
	numPixelFormats = 
	    (*__glDevice->devGetPixelFormat)(NULL, -1);
    }

    if (numPixelFormats) {
	if (nBytes > sizeof(PIXELFORMATDESCRIPTOR)) {
	    nBytes = sizeof(PIXELFORMATDESCRIPTOR);
	}
	if (nBytes) {
	    memcpy(ppfd, &pfd, nBytes);
	}
    }
    return numPixelFormats;
}

int WINAPI
wglChoosePixelFormat(HDC hDC, const PIXELFORMATDESCRIPTOR *ppfd)
{
    PIXELFORMATDESCRIPTOR ppfdBest;
    int i, bestIndex = -1;
    int numPixelFormats;

    if (ppfd->dwFlags != (ppfd->dwFlags & 
				    (
				    PFD_DRAW_TO_WINDOW |
				    PFD_DRAW_TO_BITMAP |
				    PFD_SUPPORT_GDI |
				    PFD_SUPPORT_OPENGL |
				    PFD_GENERIC_FORMAT |
				    PFD_NEED_PALETTE |
				    PFD_NEED_SYSTEM_PALETTE |
				    PFD_DOUBLEBUFFER |
				    PFD_STEREO |
				    /*PFD_SWAP_LAYER_BUFFERS |*/
				    PFD_DOUBLEBUFFER_DONTCARE |
				    PFD_STEREO_DONTCARE |
				    PFD_SWAP_COPY |
				    PFD_SWAP_EXCHANGE |
				    0)))
    {
	/* error: bad dwFlags */
	return 0;
    }

    switch (ppfd->iPixelType) {
    case PFD_TYPE_RGBA:
    case PFD_TYPE_COLORINDEX:
	break;
    default:
	/* error: bad iPixelType */
	return 0;
    }

    switch (ppfd->iLayerType) {
    case PFD_MAIN_PLANE:
    case PFD_OVERLAY_PLANE:
    case PFD_UNDERLAY_PLANE:
	break;
    default:
	/* error: bad iLayerType */
	return 0;
    }

    numPixelFormats = (*__glDevice->devGetPixelFormat)(NULL, -1);

    /* loop through candidate pixel format descriptors */
    for (i=0; i<numPixelFormats; ++i) {
	PIXELFORMATDESCRIPTOR ppfdCandidate;

	(void) (*__glDevice->devGetPixelFormat)(&ppfdCandidate, i);

	/*
	** Check attributes which must match
	*/
	if (ppfd->iPixelType != ppfdCandidate.iPixelType) {
	    continue;
	}

	if (ppfd->iLayerType != ppfdCandidate.iLayerType) {
	    continue;
	}

	if (((ppfd->dwFlags ^ ppfdCandidate.dwFlags) & ppfd->dwFlags) &
	    (PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP |
		PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL))
	{
	    continue;
	}

	if (!(ppfd->dwFlags & PFD_DOUBLEBUFFER_DONTCARE)) {
	    if ((ppfd->dwFlags & PFD_DOUBLEBUFFER) !=
		(ppfdCandidate.dwFlags & PFD_DOUBLEBUFFER))
	    {
		continue;
	    }
	}

	if (!(ppfd->dwFlags & PFD_STEREO_DONTCARE)) {
	    if ((ppfd->dwFlags & PFD_STEREO) !=
		(ppfdCandidate.dwFlags & PFD_STEREO))
	    {
		continue;
	    }
	}

        if (ppfd->iPixelType==PFD_TYPE_RGBA
            && ppfd->cAlphaBits && !ppfdCandidate.cAlphaBits) {
            continue;
	}

        if (ppfd->iPixelType==PFD_TYPE_RGBA
            && ppfd->cAccumBits && !ppfdCandidate.cAccumBits) {
	    continue;
        }

        if (ppfd->cDepthBits && !ppfdCandidate.cDepthBits) {
           continue;
        }

        if (ppfd->cStencilBits && !ppfdCandidate.cStencilBits) {
            continue;
        }

	if (ppfd->cAuxBuffers && !ppfdCandidate.cAuxBuffers) {
	    continue;
	}

	/*
	** See if candidate is better than the previous best choice
	*/
	if (bestIndex == -1) {
	    ppfdBest = ppfdCandidate;
	    bestIndex = i;
	    continue;
	}

	if ((ppfd->cColorBits > ppfdBest.cColorBits &&
		ppfdCandidate.cColorBits > ppfdBest.cColorBits) ||
	    (ppfd->cColorBits <= ppfdCandidate.cColorBits &&
		ppfdCandidate.cColorBits < ppfdBest.cColorBits))
	{
	    ppfdBest = ppfdCandidate;
	    bestIndex = i;
	    continue;
	}

	if (ppfd->iPixelType==PFD_TYPE_RGBA
            && ppfd->cAlphaBits
            && ppfdCandidate.cAlphaBits > ppfdBest.cAlphaBits)
	{
	    ppfdBest = ppfdCandidate;
	    bestIndex = i;
	    continue;
	}

	if (ppfd->iPixelType==PFD_TYPE_RGBA
            && ppfd->cAccumBits
            && ppfdCandidate.cAccumBits > ppfdBest.cAccumBits)
	{
	    ppfdBest = ppfdCandidate;
	    bestIndex = i;
	    continue;
	}

	if ((ppfd->cDepthBits > ppfdBest.cDepthBits &&
		ppfdCandidate.cDepthBits > ppfdBest.cDepthBits) ||
	    (ppfd->cDepthBits <= ppfdCandidate.cDepthBits &&
		ppfdCandidate.cDepthBits < ppfdBest.cDepthBits))
	{
	    ppfdBest = ppfdCandidate;
	    bestIndex = i;
	    continue;
	}

	if (ppfd->cStencilBits &&
		ppfdCandidate.cStencilBits > ppfdBest.cStencilBits)
	{
	    ppfdBest = ppfdCandidate;
	    bestIndex = i;
	    continue;
	}

	if (ppfd->cAuxBuffers &&
		ppfdCandidate.cAuxBuffers > ppfdBest.cAuxBuffers)
	{
	    ppfdBest = ppfdCandidate;
	    bestIndex = i;
	    continue;
	}
    }
    return bestIndex + 1;
}

int WINAPI
wglGetPixelFormat(HDC hDC)
{
    HWND hWnd = WindowFromDC(hDC);
    __WGLdrawablePrivate *wglPriv = __wglFindDrawablePrivate(hDC, hWnd);

    if (wglPriv == NULL) {
	return 0;	/* no pixelformat defined */
    }

    return wglPriv->pixelFormat;
}

BOOL WINAPI
wglSetPixelFormat(HDC hDC, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd)
{
    HWND hWnd = WindowFromDC(hDC);
    __WGLdrawablePrivate *wglPriv = __wglFindDrawablePrivate(hDC, hWnd);
    int numPixelFormats;
    PIXELFORMATDESCRIPTOR pfd;

    if (wglPriv == NULL) {
	__GLcontextModes modes;
	PIXELFORMATDESCRIPTOR ourPFD;

	/* 
	** Should we use the ppfd passed?  It may be wrong.  What was Microsoft
	** thinking???  Do a DescribePixelFormat and use that structure in
	** order to be consistent
	*/
	if (!wglDescribePixelFormat(hDC, iPixelFormat, 
				    sizeof(PIXELFORMATDESCRIPTOR),
				    &ourPFD)) {
	    __wglSetSystemError("wglSetPixelFormat", WGL_UNKNOWN_PIXEL_FORMAT);
	    return FALSE;
	}

	__wglFormatGLModes(&modes, &ourPFD);
	wglPriv = __wglCreateDrawablePrivate(hDC, hWnd, &modes);
    }

    /* It's an error to set the pixel format more than once */
    if (wglPriv->pixelFormat != 0) {
	return FALSE;
    }

    numPixelFormats = wglDescribePixelFormat(hDC, iPixelFormat,
					     sizeof(PIXELFORMATDESCRIPTOR),
					     &pfd);
    if (numPixelFormats == 0) {
	return FALSE;
    }

    /* Make sure the object supports this pixel format */
    switch (GetObjectType(hDC)) {
    case OBJ_DC:
	if (!(pfd.dwFlags & PFD_DRAW_TO_WINDOW)) {
	    return FALSE;
	}
	break;
    case OBJ_MEMDC:
	if (!(pfd.dwFlags & PFD_DRAW_TO_BITMAP)) {
	    return FALSE;
	}
	break;
    default:
	return FALSE;
    }

    wglPriv->pixelFormat = iPixelFormat;
    return TRUE;
}

/****************************************************************************/

void
__wglSetProcTable(__GLcontext *gc, const struct __GLdispatchStateRec *state)
{
}

/****************************************************************************/


PROC WINAPI
wglGetProcAddress(LPCSTR lpszProc)
{
    return __wglGetProcAddress(lpszProc);
}

PROC WINAPI
wglGetDefaultProcAddress(LPCSTR lpszProc)
{
    __wglSetSystemError("wglGetDefaultProcAddress not supported", WGL_ERROR);
    return NULL;
}

HGLRC WINAPI
wglCreateLayerContext(HDC hDC, int iLayerPlane)
{
    __wglSetSystemError("wglCreateLayerContext not supported", WGL_ERROR);
    return 0;
}

BOOL WINAPI
wglDescribeLayerPlane(HDC hDC, int iPixelFormat, int iLayerPlane,
		      UINT nBytes, LPLAYERPLANEDESCRIPTOR lpd)
{
    __wglSetSystemError("wglDescribeLayerPlane not supported", WGL_ERROR);
    return FALSE;
}

int WINAPI
wglGetLayerPaletteEntries(HDC hDC, int iLayerPlane,
			  int iStart, int cEntries, COLORREF *par)
{
    __wglSetSystemError("wglGetLayerPaletteEntries not supported", WGL_ERROR);
    return 0;
}

BOOL WINAPI
wglRealizeLayerPalette(HDC hDC, int iLayerPlane, BOOL bRealize)
{
    __wglSetSystemError("wglRealizeLayerPalette not supported", WGL_ERROR);
    return FALSE;
}

int WINAPI
wglSetLayerPaletteEntries(HDC hDC, int iLayerPlane,
			  int iStart, int cEntries, const COLORREF *par)
{
    __wglSetSystemError("wglSetlayerPaletteEntries not supported", WGL_ERROR);
    return 0;
}

BOOL WINAPI
wglSwapLayerBuffers(HDC hDC, UINT fuPlanes)
{
    __wglSetSystemError("wglSwapLayerBuffers not supported", WGL_ERROR);
    return FALSE;
}


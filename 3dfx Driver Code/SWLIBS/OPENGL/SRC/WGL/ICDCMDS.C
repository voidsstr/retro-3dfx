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

extern int freeHandle = 1;

extern __GLcontext *__wglInvalidGC;

extern struct __GLdispatchStateRec *__glGetCurrentDispatch();

/* win95 specific */
#ifdef _WIN95_
// Keep these per-process so that the thunk code in Control will not
// need to keep grabbing them.  Pass in with each Escape.  Copy back
// out upon return.

DWORD hmodOGLSRV = 0;
DWORD lpfnEscapeOGLSRV = 0;
#endif


/************************************************************/

BOOL APIENTRY
DrvDeleteContext(DHGLRC hGLRC)
{
    __WGLcontext *glrc = __wglFindWGLContext(hGLRC);

    if (glrc == NULL) {
        __wglSetSystemError("wglDeleteContext", WGL_INVALID_HRC);
	return FALSE;
    }

    if (glrc->isInUse && (glrc->owner != GetCurrentThreadId())) {
        return FALSE;
    }

    __wglLockMutex();

    if (!(*glrc->gc->exports.destroyContext)(glrc->gc)) {
        __wglSetSystemError("wglDeleteContext", WGL_ERROR);
	__wglUnlockMutex();
	return FALSE;
    }



    /*
    ** If there is no current GC, this means that the TLA is not initialized
    ** to a value that the ICD expects, so we have to swap our thread value
    ** in, just to initialize the dispatch table.
    **
    ** Oh, btw.  Do that only if we have a TLA.  If we don't, that means that
    ** this is process termination time, the ICD is already uninitialized and
    ** opengl32 just loops thru to delete contexts
    */
#if 0
    if (TlsGetValue(__wglTLSCXIndex)) {
	if (__wglGetCurrentGC() == __wglInvalidGC) {
	    __glSwapThreadLocalArea((void *)TlsGetValue(__wglTLSCXIndex));
	    
	    if (glrc->owner == GetCurrentThreadId()) {
		__wglSetCurrentGC(OGLTlsIndex, __wglInvalidGC);
		__glCoreNopDispatch();
	    }

	    __glSwapThreadLocalArea((void *)TlsGetValue(OGLTlaIndex));
	} else {
	    if (glrc->owner == GetCurrentThreadId()) {
		__wglSetCurrentGC(OGLTlsIndex, __wglInvalidGC);
		__glCoreNopDispatch();
	    }
	}
    }
#endif

    __wglRemoveContext(glrc);
    __wglUnlockMutex();

    __wglFree(glrc);
    return TRUE;
}

void APIENTRY
DrvLockBuffers(DHGLRC hGLRC)
{
    __WGLcontext *glrc = __wglFindWGLContext(hGLRC);

    if (glrc == NULL) {
        __wglSetSystemError("wglLockBuffers", WGL_INVALID_HRC);
	return;
    }

    (*glrc->gc->exports.lockBuffers)(glrc->gc);
}

void APIENTRY
DrvUnlockBuffers(DHGLRC hGLRC)
{
    __WGLcontext *glrc = __wglFindWGLContext(hGLRC);

    if (glrc == NULL) {
        __wglSetSystemError("wglUnlockBuffers", WGL_INVALID_HRC);
	return;
    }

    (*glrc->gc->exports.unlockBuffers)(glrc->gc);

}

DHGLRC APIENTRY
DrvGetCurrentContext(void)
{
    __GLcontext *gc = __wglGetCurrentGC();
    __WGLcontext *glrc;

    if (gc == NULL) {
	return (DHGLRC) 0;
    }
    glrc = (__WGLcontext *) gc->imports.other;
    return glrc->hGLRC;
}

HDC APIENTRY
DrvGetCurrentDC(void)
{
    __GLcontext *gc = __wglGetCurrentGC();
    __WGLcontext *glrc;

    if (gc == NULL) {
	return (HDC) 0;
    }
    glrc = (__WGLcontext *) gc->imports.other;
    return glrc->hDC;
}

/*
 * Setting the context, NT style
 */
PGLCLTPROCTABLE APIENTRY
DrvSetContext(HDC hDC, DHGLRC hGLRC, PFN_SETPROCTABLE pfnSetProcTable)
{
    __GLcontext *gcOld = __wglGetCurrentGC();
    __WGLcontext *glrc = __wglFindWGLContext(hGLRC);
    DWORD objType = GetObjectType(hDC);
    HWND hWnd = NULL;
    HBITMAP hBitmap = NULL;
    __WGLdrawablePrivate *wglPriv;

    if (!gcOld) {
	__wglAttachThread(GetCurrentThreadId());
    }

    if (glrc != NULL && glrc->hDC == hDC && glrc->gc == gcOld) {
	return __glGetCurrentDispatch();
    }

    switch (objType) {
    case OBJ_DC:
	hWnd = WindowFromDC(hDC);
	break;
    case OBJ_MEMDC:
	hBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
	break;
    default:
	return NULL;
    }

    if (hGLRC != 0) {
	if (glrc == NULL) {
	    __wglSetSystemError("wglDrvCliSetContext", WGL_INVALID_HRC);
	    return NULL;
	}

	if ((hWnd == NULL) && (hBitmap == NULL)) {
	    __wglSetSystemError("wglDrvCliSetContext", WGL_INVALID_HDC);
	    return NULL;
	}

	if (glrc->isInUse && (glrc->owner != GetCurrentThreadId())) {
	    __wglSetSystemError("wglDrvCliSetContext", WGL_CONTEXT_IN_USE);
	    return NULL;
	}

	wglPriv = __wglGetDrawablePrivate(hDC, hWnd, glrc->modes);
	if (wglPriv == NULL) {
	    __wglSetSystemError("wglDrvCliSetContext", WGL_INVALID_HDC);
	    return NULL;
	}

	if (wglPriv->pixelFormat != glrc->pixelFormat) {
	    __wglSetSystemError("wglDrvCliSetContext", WGL_DIFFERENT_P_FMAT);
	    return NULL;
	}
    }

    __wglLockMutex();

    /* store the function to change the dispatch tables */
    glrc->pfnSetProcTable = pfnSetProcTable;

    /* MakeCurrent to new context */
    if (hGLRC != 0) {
	__GLcontext *gc = glrc->gc;

#if 0
	/* update our thread local info */
	__glSwapThreadLocalArea((void *)TlsGetValue(OGLTlaIndex));
#endif

	wglPriv->refCount++;
	glrc->wglPriv = wglPriv;

	if (!(*gc->exports.makeCurrent)(gc)) {
#if 0
	    /* swap tla back */
	    __glSwapThreadLocalArea((void *)TlsGetValue(OGLTlaIndex));
#endif

	    __wglUnlockMutex();
	    return NULL;
	}
	__wglSetCurrentGC(gc);

	/* Set the flag to say that the context is in use. */
	glrc->isInUse = TRUE;
	glrc->owner = GetCurrentThreadId();
	glrc->hWnd = hWnd;
	glrc->hDC = hDC;
	glrc->hasBeenCurrent = TRUE;
    } else {
	__glCoreNopDispatch();
    }

    __wglUnlockMutex();

    /* Has the Window's size/position changed ? */
    if (hGLRC != 0 && glrc->pendingWindowChange) {
	__wglUpdateDrawableBuffers(glrc);
    }

    return __glGetCurrentDispatch();
}

/*
 * Release the context.
 */
BOOL APIENTRY
DrvReleaseContext(DHGLRC hGLRC)
{
    __GLcontext *gcOld = __wglGetCurrentGC();
    __WGLcontext *glrc = __wglFindWGLContext(hGLRC);
    HWND hWnd = NULL;
    HBITMAP hBitmap = NULL;

    if (!gcOld) {
	__wglAttachThread(GetCurrentThreadId());
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

	glrcOld->wglPriv = NULL;
	glrcOld->isInUse = FALSE;
	glrcOld->owner = (DWORD) __WGL_NO_OWNER;
	glrc->hWnd = NULL;
	glrc->hDC = NULL;
	__wglSetCurrentGC(__wglInvalidGC);
    }

    __wglUnlockMutex();

    /* Has the Window's size/position changed ? */
    if (hGLRC != 0 && glrc->pendingWindowChange) {
	__wglUpdateDrawableBuffers(glrc);
    }

#if 0
    /* update our thread local info */
    __glSwapThreadLocalArea((void *)TlsGetValue(OGLTlaIndex));
#endif

    return TRUE;
}

BOOL APIENTRY
DrvShareLists(DHGLRC hGLRC1, DHGLRC hGLRC2)
{
    __GLcontext *gcSrc, *gcDst;
    __WGLcontext *glrcSrc, *glrcDst;

    glrcSrc = __wglFindWGLContext(hGLRC1);
    if (glrcSrc == NULL) {
	__wglSetSystemError("DrvShareLists", WGL_SHR_INVALID_SRC_CONTEXT);
	return FALSE;
    }

    glrcDst = __wglFindWGLContext(hGLRC2);
    if (glrcDst == NULL) {
	__wglSetSystemError("DrvShareLists", WGL_SHR_INVALID_DST_CONTEXT);
	return FALSE;
    }

    __wglLockMutex();

    gcSrc = glrcSrc->gc;
    gcDst = glrcDst->gc;
    if (!(*gcSrc->exports.shareContext)(gcDst, gcSrc)) {
	__wglSetSystemError("DrvShareLists", WGL_SHR_DST_LIST_NOT_EMPTY);
	__wglUnlockMutex();
	return FALSE;
    }

    __wglUnlockMutex();

    return TRUE;
}

BOOL APIENTRY
DrvSwapBuffers(HDC hDC)
{
    HWND hWnd = WindowFromDC(hDC);
    __WGLcontext *glrc = __wglFindWGLWindow(hWnd, NULL);
    int ret;

    __wglLockMutex();

    /* Make sure there is a current context, and that it is double buffered */
    if ((glrc == NULL) || !glrc->modes->doubleBufferMode) {
	return FALSE;
    }

    (*glrc->gc->exports.swapBuffers)(glrc->gc);
    ret = (*glrc->wglPriv->swapBuffers)(glrc->wglPriv);

    __wglUnlockMutex();

    return ret;
}

/*
** This routine does the actual swapbuffers, but it's called from glFinish().
*/
int
__wglSwapBuffers(__GLcontext *gc)
{
  __WGLcontext *glrc = (__WGLcontext *)gc->imports.other;
  int ret;

  __wglLockMutex();

  (*glrc->gc->exports.swapBuffers)(gc);

  ret = (*glrc->wglPriv->swapBuffers)(glrc->wglPriv);

  __wglUnlockMutex();

  return ret;
}


BOOL APIENTRY
DrvCopyContext(DHGLRC hglrcSrc, DHGLRC hglrcDst, UINT mask)
{
    __wglSetSystemError("wglCopyContext not supported", WGL_ERROR);
    return FALSE;
}

BOOL APIENTRY
DrvDescribeLayerPlane(HDC hDC, int iPixelFormat, int iLayerPlane,
		      UINT nBytes, LPLAYERPLANEDESCRIPTOR plpd)
{
    if (!__wglGetCurrentGC()) {
	__wglAttachThread(GetCurrentThreadId());
    }

    if ((iPixelFormat == 0) || (iLayerPlane == 0)) {
	__wglSetSystemError("wglDescribeLayerPlane", WGL_BAD_PIXEL_FORMAT);
	return FALSE;
    }

    /* XXX: Implement */

    /* done */
    return TRUE;
}

DHGLRC APIENTRY
DrvCreateLayerContext(HDC hDC, int iLayerPlane)
{
    int pixelFormat = GetPixelFormat(hDC);
    __WGLcontext *glrc;

    if (!__wglGetCurrentGC()) {
        __wglAttachThread(GetCurrentThreadId());
    }

    if (pixelFormat == 0) {
        __wglSetSystemError("wglCreateLayerContext", WGL_BAD_PIXEL_FORMAT);
	return (DHGLRC) 0;
    }

    glrc = (__WGLcontext *) __wglMalloc(sizeof(__WGLcontext));
    if (glrc == NULL) {
        __wglSetSystemError("wglCreateLayerContext", WGL_MALLOC_FAILED);
	return (DHGLRC) 0;
    }
    memset(glrc, 0, sizeof(*glrc));

    if (!DescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), 
				&glrc->pFD)) {
        __wglFree(glrc);
        __wglSetSystemError("wglCreateLayerContext", WGL_UNKNOWN_PIXEL_FORMAT);
	return (DHGLRC) 0;
    }
    glrc->pFD.iLayerType = 0; /* normally not used. Now, plane #. 0 == main plane */
    if (iLayerPlane) {
	LAYERPLANEDESCRIPTOR lpd;

	if (!DrvDescribeLayerPlane(hDC, pixelFormat, iLayerPlane,
				      sizeof(LAYERPLANEDESCRIPTOR), &lpd)) {
	    __wglFree(glrc);
	    __wglSetSystemError("wglCreateLayerContext", WGL_UNKNOWN_PIXEL_FORMAT);
	    return (DHGLRC) 0;
	}

	/* copy */
	glrc->pFD.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	glrc->pFD.nVersion = 1;
	glrc->pFD.dwFlags = PFD_DRAW_TO_WINDOW |
	    PFD_SUPPORT_OPENGL |
	    PFD_NEED_PALETTE;
	glrc->pFD.iPixelType = lpd.iPixelType;
	glrc->pFD.cColorBits = lpd.cColorBits;
	glrc->pFD.cRedBits = lpd.cRedBits;
	glrc->pFD.cRedShift = lpd.cRedShift;
	glrc->pFD.cGreenBits = lpd.cGreenBits;
	glrc->pFD.cGreenShift = lpd.cGreenShift;
	glrc->pFD.cBlueBits = lpd.cBlueBits;
	glrc->pFD.cBlueShift = lpd.cBlueShift;
	glrc->pFD.cAlphaBits = lpd.cAlphaBits;
	glrc->pFD.cAlphaShift = lpd.cAlphaShift;
	glrc->pFD.cAccumBits = lpd.cAccumBits;
	glrc->pFD.cAccumRedBits = lpd.cAccumRedBits;
	glrc->pFD.cAccumGreenBits = lpd.cAccumGreenBits;
	glrc->pFD.cAccumBlueBits = lpd.cAccumBlueBits;
	glrc->pFD.cAccumAlphaBits = lpd.cAccumAlphaBits;
	glrc->pFD.cDepthBits = lpd.cDepthBits;
	glrc->pFD.cStencilBits = lpd.cStencilBits;
	glrc->pFD.cAuxBuffers = lpd.cAuxBuffers;
	glrc->pFD.iLayerType = iLayerPlane; /* normally not used. Now, plane # */
	glrc->pFD.dwVisibleMask = lpd.crTransparent;
    }

    __wglLockMutex();

    glrc->isInUse = FALSE;
    glrc->owner = (DWORD) __WGL_NO_OWNER;

    glrc->pixelFormat = pixelFormat;
    glrc->hasBeenCurrent = FALSE;
    glrc->pendingDestroy = FALSE;
    glrc->pendingWindowChange = TRUE;
    glrc->isDirect = TRUE;

    glrc->hGLRC = (DHGLRC) freeHandle++;

    if (!__wglCreateHWContext(glrc)) {
        __wglFree(glrc);
        __wglSetSystemError("wglCreateContext", WGL_ERROR);
	__wglUnlockMutex();
	return (DHGLRC) 0;
    }

    __wglAddContext(glrc);
    __wglUnlockMutex();

    return glrc->hGLRC;
}

DHGLRC APIENTRY
DrvCreateContext(HDC hDC)
{
    return DrvCreateLayerContext(hDC, 0);
}

int APIENTRY
DrvGetLayerPaletteEntries(HDC hDC, int iLayerPlane,
			  int iStart, int cEntries, COLORREF *par)
{
    DWORD objType = GetObjectType(hDC);
    HWND hWnd;

    if (!__wglGetCurrentGC()) {
	__wglAttachThread(GetCurrentThreadId());
    }

    if (iLayerPlane == 0) {
	__wglSetSystemError("wglDescribeLayerPlane", WGL_BAD_PIXEL_FORMAT);
	return FALSE;
    }

    /*
    ** we assume that we're not going to get an OBJ_MEMDC, since we have overlays
    */
    switch (objType) {
    case OBJ_DC:
	hWnd = WindowFromDC(hDC);
	break;
    default:
	return FALSE;
    }

    /* XXX: Implement */

    /* done */
    return TRUE;
}

BOOL APIENTRY
DrvRealizeLayerPalette(HDC hDC, int iLayerPlane, BOOL bRealize)
{
    DWORD objType = GetObjectType(hDC);
    HWND hWnd;

    if (!__wglGetCurrentGC()) {
	__wglAttachThread(GetCurrentThreadId());
    }

    if (iLayerPlane == 0) {
	__wglSetSystemError("wglRealizeLayerPalette", WGL_BAD_PIXEL_FORMAT);
	return FALSE;
    }

    /*
    ** we assume that we're not going to get an OBJ_MEMDC, since we have overlays
    */
    switch (objType) {
    case OBJ_DC:
	hWnd = WindowFromDC(hDC);
	break;
    default:
	return 0;
    }

    /* XXX: implement */

    return TRUE;
}

int APIENTRY
DrvSetLayerPaletteEntries(HDC hDC, int iLayerPlane,
			  int iStart, int cEntries, const COLORREF *par)
{
    DWORD objType = GetObjectType(hDC);
    HWND hWnd;

    if (!__wglGetCurrentGC()) {
	__wglAttachThread(GetCurrentThreadId());
    }

    if (iLayerPlane == 0) {
	__wglSetSystemError("wglDescribeLayerPlane", WGL_BAD_PIXEL_FORMAT);
	return 0;
    }

    /*
    ** we assume that we're not going to get an OBJ_MEMDC, since we have overlays
    */
    switch (objType) {
    case OBJ_DC:
	hWnd = WindowFromDC(hDC);
	break;
    default:
	return 0;
    }

    {
	int nEntries;
	/* XXX: implement */

	nEntries = 0;

	return nEntries;
    }
}

BOOL APIENTRY
DrvSwapLayerBuffers(HDC hDC, UINT fuPlanes)
{
    /*
    ** XXX:
    **
    ** In our case, overlays are not doublebuffered, so we are safe.
    ** But this should change to something more portable, and let the
    ** kernel decide what to swap.
    */
    if (fuPlanes | WGL_SWAP_MAIN_PLANE) {
	DrvSwapBuffers(hDC);
    }

    return TRUE;
}

PROC WINAPI
DrvGetProcAddress(LPCSTR lpszProc)
{
    return __wglGetProcAddress(lpszProc);
}

BOOL APIENTRY
DrvValidateVersion(ULONG ulDrvVer)
{
    /* 
    ** driver version is passed in here.  We check if we support it or not
    */

    if (ulDrvVer > 1) {
	return FALSE;
    }

    return TRUE;
}

BOOL
__wglSetupMSDispatch(HINSTANCE hInstanceMS)
{
    /* XXX: for now, wimp out */

    return FALSE;
}

void
__wglSetProcTable(__GLcontext *gc, const struct __GLdispatchStateRec *state)
{
    __WGLcontext *glrc = (__WGLcontext *) gc->imports.other;

    if (glrc->pfnSetProcTable) {
	(*glrc->pfnSetProcTable)((PGLCLTPROCTABLE)state);
    }
}

/************************************************************/



int APIENTRY
DrvDescribePixelFormat(HDC hDC, int iPixelFormat,
		       UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
{
    PIXELFORMATDESCRIPTOR pfd;
    int	numFormats;

    if (ppfd) {
	numFormats = (*__glDevice->devGetPixelFormat)(&pfd, iPixelFormat-1);
    } else {
	numFormats = (*__glDevice->devGetPixelFormat)(NULL, -1);
    }

    if (numFormats) {
	if (nBytes > sizeof(PIXELFORMATDESCRIPTOR)) {
	    nBytes = sizeof(PIXELFORMATDESCRIPTOR);
	}
	if (nBytes) {
	    memcpy(ppfd, &pfd, nBytes);
	}
    }

    return numFormats;
}

BOOL APIENTRY
DrvSetPixelFormat(HDC hDC, int iPixelFormat)
{
    HWND hWnd = WindowFromDC(hDC);
    __WGLdrawablePrivate *wglPriv = __wglFindDrawablePrivate(hDC, hWnd);
    PIXELFORMATDESCRIPTOR pfd;
    int	numFormats;

    if (wglPriv == NULL) {
	__GLcontextModes modes;
	PIXELFORMATDESCRIPTOR ourPFD;

	if (!DescribePixelFormat(hDC, iPixelFormat,
				    sizeof(PIXELFORMATDESCRIPTOR),
				    &ourPFD)) {
	    return FALSE;
	}

	__wglFormatGLModes(&modes, &ourPFD);
	wglPriv = __wglCreateDrawablePrivate(hDC, hWnd, &modes);
    }

    /* it's an error to set the pixel format more than once */
    if (wglPriv->pixelFormat != 0) {
	return FALSE;
    }

    numFormats = DescribePixelFormat(hDC, iPixelFormat,
				     sizeof(PIXELFORMATDESCRIPTOR),
				     &pfd);
    if (numFormats == 0) {
	return FALSE;
    }

    /* Make sure the object supports this pixel format */
    switch(GetObjectType(hDC)) {
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

    /* now finally set the pixel format */
    wglPriv->pixelFormat = iPixelFormat;

    return TRUE;
}

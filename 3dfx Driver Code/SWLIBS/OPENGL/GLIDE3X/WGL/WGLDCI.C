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

#if defined( __WGL_USE_DCI ) || defined( __WGL_USE_DIRECTDRAW)
#include "wgllib.h"
#include "dciman.h"

struct DCIFunctions {
    HDC (WINAPI *OpenProvider)(void);
    void (WINAPI *CloseProvider)(HDC);

    int (WINAPI *CreatePrimary)(HDC, LPDCISURFACEINFO *);
    void (WINAPI *Destroy)(LPDCISURFACEINFO);

    HWINWATCH (WINAPI *WinWatchOpen)(HWND);
    VOID (WINAPI *WinWatchClose)(HWINWATCH);
    UINT (WINAPI *GetClipList)(HWINWATCH, LPRECT, UINT, LPRGNDATA);
    BOOL (WINAPI *DidStatusChange)(HWINWATCH);

    DCIRVAL (WINAPI *BeginAccess)(LPDCISURFACEINFO, int, int, int, int);
    void (WINAPI *EndAccess)(LPDCISURFACEINFO);
};

#if defined(__WGL_DYNAMIC_DCI)
static struct DCIFunctions DCI;
static HINSTANCE hDCIManLibrary;
#define NUM_DCI_FUNCS (sizeof(DCIFuncNames) / sizeof(DCIFuncNames[0]))
static char *DCIFuncNames[] = {
    "DCIOpenProvider",
    "DCICloseProvider",
    "DCICreatePrimary",
    "DCIDestroy",
    "WinWatchOpen",
    "WinWatchClose",
    "WinWatchGetClipList",
    "WinWatchDidStatusChange",
    "DCIBeginAccess",
    "DCIEndAccess",
};
#else
static struct DCIFunctions DCI = {
    DCIOpenProvider,
    DCICloseProvider,
    DCICreatePrimary,
    DCIDestroy,
    WinWatchOpen,
    WinWatchClose,
    WinWatchGetClipList,
    WinWatchDidStatusChange,
    DCIBeginAccess,
    DCIEndAccess,
};
#endif

/*
** Support for direct framebuffer access using the Display Control
** Interface (DCI).
*/

static HDC hDCIMan;
static LPDCISURFACEINFO lpPrimarySurface;

typedef struct __WGLsurfaceInfoRec {
    /*
    ** DCI Surface and WinWatch for front buffer surfaces.
    */
    LPDCISURFACEINFO lpSurface;
    HWINWATCH hWinWatch;
} __WGLsurfaceInfo;

BOOL
__wglDCIInit(void)
{
#if defined(__WGL_DYNAMIC_DCI)
    if (!hDCIManLibrary) {
	int i;

	if ((hDCIManLibrary = LoadLibrary("DCIMAN32.DLL")) == NULL) {
	    return FALSE;
	}
	for (i=0; i<NUM_DCI_FUNCS; ++i) {
	    FARPROC *p = &((FARPROC *)&DCI)[i];
	    *p = GetProcAddress(hDCIManLibrary, DCIFuncNames[i]);
	}
    }
#endif
    if (!hDCIMan) {
	hDCIMan = DCI.OpenProvider();
	if (hDCIMan == NULL) {
	    __wglError("__wglDCIInit: DCIOpenProvider failed");
	    __wglDCIFinish();
	    return FALSE;
	}
    }

    if (!lpPrimarySurface) {
	DCIRVAL status;

	/* Allocate a primary surface for the front buffer */
	status = DCI.CreatePrimary(hDCIMan, &lpPrimarySurface);
	if (status != DCI_OK) {
	    __wglError("__wglDCIInit: DCICreatePrimary failed");
	    __wglDCIFinish();
	    return FALSE;
	}
    }

    return TRUE;
}

BOOL
__wglDCIFinish(void)
{
    if (lpPrimarySurface) {
	DCI.Destroy(lpPrimarySurface);
	lpPrimarySurface = NULL;
    }

    if (hDCIMan) {
	DCI.CloseProvider(hDCIMan);
	hDCIMan = NULL;
    }

#if defined(__WGL_DYNAMIC_DCI)
    if (hDCIManLibrary) {
	FreeLibrary(hDCIManLibrary);
	hDCIManLibrary = NULL;
    }
#endif
    return TRUE;
}


int
__wglDCIGetDisplayMasks(int *rMask, int *gMask, int *bMask)
{
    if (lpPrimarySurface) {
	DCIRVAL status;

	DCI.Destroy(lpPrimarySurface);
	status = DCI.CreatePrimary(hDCIMan, &lpPrimarySurface);
	if (status != DCI_OK) {
	    __wglError("__wglDCIGetDisplayMasks: DCICreatePrimary failed");
	    return 0;
	}
    }

    switch (lpPrimarySurface->dwBitCount) {
    case 16:
	if ((lpPrimarySurface->dwMask[0] != 0) &&
	    (lpPrimarySurface->dwMask[1] != 0) &&
	    (lpPrimarySurface->dwMask[2] != 0))
	{
	    *rMask = lpPrimarySurface->dwMask[0];
	    *gMask = lpPrimarySurface->dwMask[1];
	    *bMask = lpPrimarySurface->dwMask[2];
	} else {
	    /* Use default 16-bit format: X555 */
	    *rMask = 0x1f << 10;
	    *gMask = 0x1f <<  5;
	    *bMask = 0x1f <<  0;
	}
	break;
    case 32:
	if ((lpPrimarySurface->dwMask[0] != 0) &&
	    (lpPrimarySurface->dwMask[1] != 0) &&
	    (lpPrimarySurface->dwMask[2] != 0))
	{
	    *rMask = lpPrimarySurface->dwMask[0];
	    *gMask = lpPrimarySurface->dwMask[1];
	    *bMask = lpPrimarySurface->dwMask[2];
	} else {
	    /* Use default 32-bit format: X888 */
	    *rMask = 0xff << 16;
	    *gMask = 0xff <<  8;
	    *bMask = 0xff <<  0;
	}
	break;
    case 8:
    case 24:
    default:
	*rMask = *gMask = *bMask = 0;
	break;
    }

    return lpPrimarySurface->dwBitCount;
}

void
__wglDCIUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;

    __wglUpdateDrawableSize(wglPriv);

    if (!wglSurfaceInfo) {
	wglSurfaceInfo = (__WGLsurfaceInfo *)
				__wglCalloc(1, sizeof(*wglSurfaceInfo));

	buf->other = wglSurfaceInfo;
    }

    /*
    ** The front buffers of all windows share a single primary surface.
    */
    if (!wglSurfaceInfo->lpSurface) {
	wglSurfaceInfo->lpSurface = lpPrimarySurface;
	wglSurfaceInfo->hWinWatch = DCI.WinWatchOpen(wglPriv->hWnd);
    }

    buf->width = wglPriv->width;
    buf->height = wglPriv->height;
}

void
__wglDCIFreeBuffer(__GLdrawableBuffer *buf, __WGLdrawablePrivate *wglPriv)
{
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;

    if (wglSurfaceInfo) {
	DCI.WinWatchClose(wglSurfaceInfo->hWinWatch);
	__wglFree(wglSurfaceInfo);
    }
    buf->other = NULL;
}

void
__wglDCIUpdateClipList(__WGLdrawablePrivate *wglPriv, HWINWATCH hWinWatch)
{
    UINT rgnDataSize;
    LPRGNDATA rgnData;

    /* Allocate a buffer to receive window region clip list */
    rgnDataSize = DCI.GetClipList(hWinWatch,
				&wglPriv->coreClipRect, 0, NULL);
    if (rgnDataSize > 0) {
	rgnData = (LPRGNDATA) __wglMalloc(rgnDataSize);

	/* Get window region clip list */
	rgnDataSize = DCI.GetClipList(hWinWatch,
			&wglPriv->coreClipRect, rgnDataSize, rgnData);
	
	__wglMemUpdateOwnershipBuffer(wglPriv, rgnData);
	__wglFree(rgnData);
    } else {
	__wglMemUpdateOwnershipBuffer(wglPriv, NULL);
    }
}

void
__wglDCILockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    LPDCISURFACEINFO lpSurface = wglSurfaceInfo->lpSurface;
    HWINWATCH hWinWatch = wglSurfaceInfo->hWinWatch;
    RECT *rect = &wglPriv->rect;
    DCIRVAL status;

    status = DCI.BeginAccess(lpSurface, rect->left, rect->top,
		    rect->right - rect->left, rect->bottom - rect->top);
    if (status != DCI_OK && (status < 0)) {
	__wglError("__wglDCILockBuffer: DCIBeginAccess failed");
    }
    if (lpSurface->wSelSurface != 0) {
	LDT_ENTRY desc;
	BOOL result;
	DWORD base;

	result = GetThreadSelectorEntry(GetCurrentThread(),
					lpSurface->wSelSurface, &desc);
	if (result != TRUE) {
	    __wglError("__wglDCILockBuffer: GetThreadSelectorEntry failed");
	}

	base = (desc.HighWord.Bytes.BaseHi << 24) |
	       (desc.HighWord.Bytes.BaseMid << 16) | desc.BaseLow;

	buf->base = (unsigned char *) (base + lpSurface->dwOffSurface)
	  + rect->top * lpSurface->lStride + rect->left * buf->elementSize;
    } else {
	buf->base = (unsigned char *) lpSurface->dwOffSurface
	  + rect->top * lpSurface->lStride + rect->left * buf->elementSize;
    }
    buf->byteWidth = lpSurface->lStride;

    if (wglPriv->coreClipRectChanged || DCI.DidStatusChange(hWinWatch)) {
	__wglDCIUpdateClipList(wglPriv, hWinWatch);
	wglPriv->coreClipRectChanged = FALSE;
    }
}

void
__wglDCIUnlockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    LPDCISURFACEINFO lpSurface = wglSurfaceInfo->lpSurface;

    DCI.EndAccess(lpSurface);
}

void
__wglDCIFillBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
			GLuint val, GLint x, GLint y, GLint w, GLint h)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    HBRUSH hBrush = CreateSolidBrush((COLORREF) 0x0);
    RECT rect;

    rect.left = x;
    rect.right = x + w;
    rect.top = y;
    rect.bottom = y + h;

    FillRect(wglPriv->hDC, &rect, hBrush);
    DeleteObject(hBrush);
}
#endif /* __WGL_USE_DCI */

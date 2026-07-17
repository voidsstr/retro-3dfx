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

/*
** support for direct framebuffer acess using DCI
*/

#include "wgllib.h"
#include "wgldci.h"
#include "wgldib.h"
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

static HDC hDCIMan;
static LPDCISURFACEINFO lpPrimarySurface;
static LPVOID lpPrimaryBase = NULL;
static cDCICount = 0;

typedef struct __DCIsurfaceInfoRec {
    /*
    ** DCI Surface and WinWatch for front buffer surfaces.
    */
    LPDCISURFACEINFO lpSurface;
    HWINWATCH hWinWatch;
} __DCIsurfaceInfo;

/*
** Initialization/termination of DCI
*/
GLboolean
__wglDCIOpen(GLvoid)
{
    cDCICount++;

    if (cDCICount > 1) {
	return GL_TRUE;
    }

    if (!hDCIManLibrary) {
	int i;

	if ((hDCIManLibrary = LoadLibrary("DCIMAN32.DLL")) == NULL) {
	    return GL_FALSE;
	}

	for (i=0; i < NUM_DCI_FUNCS; i++) {
	    FARPROC *p = &((FARPROC *) &DCI)[i];
	    *p = GetProcAddress(hDCIManLibrary, DCIFuncNames[i]);
	}
    }

    if (!hDCIMan) {
	hDCIMan = DCI.OpenProvider();
	if (hDCIMan == NULL) {
	    __wglError("__wglInitializeDCI: DCIOpenProvider failed");
	    __wglDCIClose();
	    return GL_FALSE;
	}
    }

    if (!lpPrimarySurface) {
	DCIRVAL status;

	/* Allocate a primary surface for the front buffer */
	status = DCI.CreatePrimary(hDCIMan, &lpPrimarySurface);
	if (status != DCI_OK) {
	    __wglError("__wglInitializeDCI: DCICreatePrimary failed");
	    __wglDCIClose();
	    return GL_FALSE;
	}
    }

#if 0
    if (!lpPrimaryBase) {
	LDT_ENTRY desc;
	BOOL result;
	DWORD base;

	result = GetThreadSelectorEntry(GetCurrentThread(),
					lpPrimarySurface->wSelSurface, &desc);
	if (result != TRUE) {
	    __wglError("__wglInitializeDCI: GetThreadSelectorEntry failed");
	    __wglDCIClose();
	    return GL_FALSE;
	}

	base = (desc.HighWord.Bytes.BaseHi << 24) |
	    (desc.HighWord.Bytes.BaseMid << 16) | desc.BaseLow;

	lpPrimaryBase = (void *) (base + lpPrimarySurface->dwOffSurface);
    }
#endif

    return GL_TRUE;
}

GLboolean
__wglDCIClose(GLvoid)
{
    cDCICount--;

    assert(cDCICount >= 0);
    if (cDCICount) {
	return GL_TRUE;
    }

    if (lpPrimarySurface) {
	DCI.Destroy(lpPrimarySurface);
	lpPrimarySurface = NULL;
    }

    if (hDCIMan) {
	DCI.CloseProvider(hDCIMan);
	hDCIMan = NULL;
    }

    if (hDCIManLibrary) {
	FreeLibrary(hDCIManLibrary);
	hDCIManLibrary = NULL;
    }

    return GL_TRUE;
}


/* ----------------------------------------------------------------- */

GLint
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

GLvoid
__wglDCIUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv)
{
}

/*
** Do a buffer swap with 
** a DCI surface as a front buffer and
** a DIB surface as a back buffer
*/
GLboolean
__wglDCIDIBSwapBuffers(__WGLdrawablePrivate *wglPriv)
{
    __GLdrawableBuffer *front = &wglPriv->glPriv.frontBuffer;
    __GLdrawableBuffer *back = &wglPriv->glPriv.backBuffer;
    LPRGNDATA rgnData = wglPriv->swapRgn;
    BOOL status;

    /* 
    ** In the current implementation, we use only the bounding rect
    ** of all the swap hint rects provided
    */
    if (0 != rgnData->rdh.nCount) {
	LPRECT lpRect = &rgnData->rdh.rcBound;
	int x, y, w, h;

	x = lpRect->left;
	y = lpRect->top;
	w = lpRect->right - lpRect->left;
	h = lpRect->bottom - lpRect->top;

	status = BitBlt(wglPriv->hDC, x, y, w, h,
			__wglDIBGetHDC(back), x, y,
			SRCCOPY);
	if (status == FALSE) {
	    __wglError("__wglDCIDIBSwapBuffers: BitBlt failed");
	}
	rgnData->rdh.nCount = 0;
    } else {
	status = BitBlt(wglPriv->hDC, 0, 0, wglPriv->width, wglPriv->height,
			__wglDIBGetHDC(back), 0, 0,
			SRCCOPY);
	if (status == FALSE) {
	    __wglError("__wglDCIDIBSwapBuffers: BitBlt (2) failed");
	}
    }

    return status;
}

/* ----------------------------------------------------------------- */

static GLvoid
UpdateClipList(__WGLdrawablePrivate *wglPriv, HWINWATCH hWinWatch)
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
	
	__wglDIBUpdateOwnershipBuffer(wglPriv, rgnData);
	__wglFree(rgnData);
    } else {
	__wglDIBUpdateOwnershipBuffer(wglPriv, NULL);
    }
}



static GLboolean
Update(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    __DCIsurfaceInfo *dciSurfaceInfo = (__DCIsurfaceInfo *) buf->handle;

    __wglUpdateDrawableSize(wglPriv);

    if (!dciSurfaceInfo) {
	dciSurfaceInfo = (__DCIsurfaceInfo *)
				__wglCalloc(1, sizeof(*dciSurfaceInfo));

	buf->handle = dciSurfaceInfo;
    }

    /*
    ** The front buffers of all windows share a single primary surface.
    */
    if (!dciSurfaceInfo->lpSurface) {
	dciSurfaceInfo->lpSurface = lpPrimarySurface;
	dciSurfaceInfo->hWinWatch = DCI.WinWatchOpen(wglPriv->hWnd);
    }

    buf->width = wglPriv->width;
    buf->height = wglPriv->height;

    return GL_TRUE;
}

static GLvoid
Lock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    __DCIsurfaceInfo *dciSurfaceInfo = (__DCIsurfaceInfo *) buf->handle;
    LPDCISURFACEINFO lpSurface = dciSurfaceInfo->lpSurface;
    HWINWATCH hWinWatch = dciSurfaceInfo->hWinWatch;
    RECT *rect = &wglPriv->rect;
    DCIRVAL status;

    status = DCI.BeginAccess(lpSurface, rect->left, rect->top,
		    rect->right - rect->left, rect->bottom - rect->top);
    if (status != DCI_OK && (status < 0)) {
	__wglError("DCI: Lock: DCIBeginAccess failed");
    }
    if (lpSurface->wSelSurface != 0) {
	LDT_ENTRY desc;
	BOOL result;
	DWORD base;

	result = GetThreadSelectorEntry(GetCurrentThread(),
					lpSurface->wSelSurface, &desc);
	if (result != TRUE) {
	    __wglError("DCI: Lock: GetThreadSelectorEntry failed");
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
	UpdateClipList(wglPriv, hWinWatch);
	wglPriv->coreClipRectChanged = FALSE;
    }
}

static GLvoid
Unlock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __DCIsurfaceInfo *dciSurfaceInfo = (__DCIsurfaceInfo *) buf->handle;
    LPDCISURFACEINFO lpSurface = dciSurfaceInfo->lpSurface;

    DCI.EndAccess(lpSurface);
}

static GLvoid
Fill(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
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

static GLvoid
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __DCIsurfaceInfo *dciSurfaceInfo = (__DCIsurfaceInfo *) buf->handle;

    if (dciSurfaceInfo) {
	DCI.WinWatchClose(dciSurfaceInfo->hWinWatch);
	__wglFree(dciSurfaceInfo);
    }
    buf->handle = NULL;
}


GLvoid
__wglInitDCI(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLint bits)
{
    buf->depth = bits;
    buf->width = buf->height = 0;	/* not assigned yet */
    buf->handle = buf->base = NULL;	/* not assigned yet */
    buf->size = 0;
    buf->byteWidth = 0;

    buf->elementSize = ((bits-1) / 8) + 1;

    buf->lockCnt = 0;

    buf->update = Update;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = Fill;
    buf->free = Free;

    /* initialize DCI */
    __wglDCIOpen();
}

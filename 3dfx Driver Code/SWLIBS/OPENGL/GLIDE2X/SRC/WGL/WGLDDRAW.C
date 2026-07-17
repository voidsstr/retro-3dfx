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
#if defined(__WGL_USE_DIRECTDRAW)
#include "wgllib.h"
#include "ddraw.h"

#if defined(__WGL_DYNAMIC_DIRECTDRAW)
static HINSTANCE hDirectDraw;
#endif

/*
** Support for direct framebuffer access and double buffering
** using DirectDraw.
*/

static LPDIRECTDRAW lpDirectDraw;
static LPDIRECTDRAWSURFACE lpPrimarySurface;

typedef struct __WGLsurfaceInfoRec {
    /*
    ** DirectDraw Surface and Clipper for front buffer surfaces.
    */
    LPDIRECTDRAWSURFACE lpSurface;
    LPDIRECTDRAWCLIPPER lpClipper;
} __WGLsurfaceInfo;

static void
restoreSurface(__WGLsurfaceInfo *wglSurfaceInfo)
{
    LPDIRECTDRAWSURFACE lpSurface = wglSurfaceInfo->lpSurface;
    HRESULT status;

    /* Make sure there is a surface */
    if (!lpSurface) {
	return;
    }

    /* Check that it is really lost */
    status = IDirectDrawSurface_IsLost(lpSurface);
    if (status == DD_OK) {
	return;
    } else if (status != DDERR_SURFACELOST) {
	__wglDDrawError("restoreSurface: IsLost", status);
	return;
    }

    /* Attempt to restore the surface */
    status = IDirectDrawSurface_Restore(lpSurface);
    if (status == DDERR_WRONGMODE) {
	DDSURFACEDESC ddsd;

	if (wglSurfaceInfo->lpClipper) {
	    /* Recreate primary surface */
	    status = IDirectDrawSurface_Release(lpSurface);
	    if (status != DD_OK) {
		__wglDDrawError("restoreSurface: Release (primary)", status);
		return;
	    }
	    wglSurfaceInfo->lpSurface = NULL;

	    ddsd.dwSize = sizeof(ddsd);
	    ddsd.dwFlags = DDSD_CAPS;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	    status = IDirectDraw_CreateSurface(lpDirectDraw, &ddsd,
					    &lpPrimarySurface, NULL);
	    if (status != DD_OK) {
		__wglDDrawError("restoreSurface: CreateSurface (primary)", status);
		return;
	    }
	    wglSurfaceInfo->lpSurface = lpPrimarySurface;

	} else {
	    /* Recreate other surface */
	    ddsd.dwSize = sizeof(ddsd);
	    status = IDirectDrawSurface_GetSurfaceDesc(lpSurface, &ddsd);
	    if (status != DD_OK) {
		__wglDDrawError("restoreSurface: GetSurfaceDesc", status);
	    }

	    status = IDirectDrawSurface_Release(lpSurface);
	    if (status != DD_OK) {
		__wglDDrawError("restoreSurface: Release", status);
		return;
	    }
	    wglSurfaceInfo->lpSurface = NULL;

	    ddsd.dwSize = sizeof(ddsd);
	    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	    status = IDirectDraw_CreateSurface(lpDirectDraw, &ddsd,
							&lpSurface, NULL);
	    if (status != DD_OK) {
		__wglDDrawError("restoreSurface: CreateSurface", status);
	    }
	    wglSurfaceInfo->lpSurface = lpSurface;
	}

    } else if (status != DD_OK) {
	__wglDDrawError("restoreSurface: Restore: failed", status);
    }
}

BOOL
__wglDDrawInit(void)
{
    HRESULT status;

    if (!lpDirectDraw) {
#if defined(__WGL_DYNAMIC_DIRECTDRAW)
        typedef HRESULT (WINAPI *DDCreateProto)(GUID *, LPDIRECTDRAW *, IUnknown *);
	DDCreateProto DDCreate;

	if ((hDirectDraw = LoadLibrary("DDRAW.DLL")) == NULL) {
	    return FALSE;
	}
	DDCreate = (DDCreateProto) GetProcAddress(hDirectDraw, "DirectDrawCreate");

	status = (*DDCreate)(NULL, &lpDirectDraw, NULL);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawInit: DirectDrawCreate", status);
	    return FALSE;
	}
#else
	status = DirectDrawCreate(NULL, &lpDirectDraw, NULL);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawInit: DirectDrawCreate", status);
	    return FALSE;
	}
#endif

	status = IDirectDraw_SetCooperativeLevel(lpDirectDraw, NULL,
							DDSCL_NORMAL);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawInit: SetCooperativeLevel", status);
	    return FALSE;
	}
    }

    if (!lpPrimarySurface) {
	DDSURFACEDESC ddsd;

	/* Allocate a primary surface for the front buffer */
	ddsd.dwSize = sizeof(ddsd);
	ddsd.dwFlags = DDSD_CAPS;
	ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	status = IDirectDraw_CreateSurface(lpDirectDraw, &ddsd,
					&lpPrimarySurface, NULL);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawInit: CreateSurface (primary)", status);
	    return FALSE;
	}
    }

    return TRUE;
}

BOOL
__wglDDrawFinish(void)
{
    HRESULT status;

    if (lpPrimarySurface) {
	status = IDirectDrawSurface_Release(lpPrimarySurface);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawCleanup: Release (primary)", status);
	}
	lpPrimarySurface = NULL;
    }
    if (lpDirectDraw) {
	IDirectDraw_Release(lpDirectDraw);
	lpDirectDraw = NULL;
    }
#if defined(__WGL_DYNAMIC_DIRECTDRAW)
    if (hDirectDraw) {
	FreeLibrary(hDirectDraw);
    }
#endif
    return TRUE;
}

int
__wglDDrawGetDisplayMasks(int *rMask, int *gMask, int *bMask)
{
    DDPIXELFORMAT ddpf;
    HRESULT status;

    ddpf.dwSize = sizeof(ddpf);
    status = IDirectDrawSurface_GetPixelFormat(lpPrimarySurface, &ddpf);
    if (status != DD_OK) {
	__wglDDrawError("__wglDDrawGetDisplayMasks: GetPixelFormat", status);
    }

    *rMask = ddpf.dwRBitMask;
    *gMask = ddpf.dwGBitMask;
    *bMask = ddpf.dwBBitMask;

    return ddpf.dwRGBBitCount;
}

void
__wglDDrawUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    BOOL frontBuffer = (buf == &glPriv->frontBuffer);
    DDSURFACEDESC ddsd;
    HRESULT status;

    __wglUpdateDrawableSize(wglPriv);

    if (!wglSurfaceInfo) {
	wglSurfaceInfo = (__WGLsurfaceInfo *)
				__wglCalloc(1, sizeof(*wglSurfaceInfo));
	buf->other = wglSurfaceInfo;
    }

    if (frontBuffer) {
	/*
	** The front buffers of all windows share a single primary surface.
	*/
	if (!wglSurfaceInfo->lpSurface) {
	    wglSurfaceInfo->lpSurface = lpPrimarySurface;
	}

	if (!wglSurfaceInfo->lpClipper) {
	    LPDIRECTDRAWCLIPPER lpClipper;

	    status = IDirectDraw_CreateClipper(lpDirectDraw, 0, &lpClipper, NULL);
	    if (status != DD_OK) {
		__wglDDrawError("__wglDDrawUpdateBuffer: CreateClipper", status);
	    }

	    status = IDirectDrawClipper_SetHWnd(lpClipper, 0, wglPriv->hWnd);
	    if (status != DD_OK) {
		__wglDDrawError("__wglDDrawUpdateBuffer: SetHWND", status);
	    }

	    wglSurfaceInfo->lpClipper = lpClipper;
	}
    } else {
	/*
	** Other buffers are separate memory surfaces.
	*/

	/* Free the old surface if it is too small */
	if (wglSurfaceInfo->lpSurface) {
	    LPDIRECTDRAWSURFACE lpSurface = wglSurfaceInfo->lpSurface;

	    ddsd.dwSize = sizeof(ddsd);
	    status = IDirectDrawSurface_GetSurfaceDesc(lpSurface, &ddsd);
	    if (status != DD_OK) {
		__wglDDrawError("__wglDDrawUpdateBuffer: GetSurfaceDesc", status);
	    }

	    if ((wglPriv->width > ddsd.dwWidth) ||
		(wglPriv->height > ddsd.dwHeight))
	    {
		status = IDirectDrawSurface_Release(lpSurface);
		if (status != DD_OK) {
		    __wglDDrawError("__wglDDrawUpdateBuffer: Release", status);
		}
		wglSurfaceInfo->lpSurface = NULL;
	    }
	}

	/* Allocate a new surface if necessary */
	if (!wglSurfaceInfo->lpSurface) {
	    LPDIRECTDRAWSURFACE lpSurface;

	    /* Create off screen surfaces for other buffers */
	    ddsd.dwSize = sizeof(ddsd);
	    ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_SYSTEMMEMORY;
	    ddsd.dwWidth = wglPriv->width;
	    ddsd.dwHeight = wglPriv->height;
	    status = IDirectDraw_CreateSurface(lpDirectDraw, &ddsd,
							&lpSurface, NULL);
	    if (status != DD_OK) {
		__wglDDrawError("__wglDDrawUpdateBuffer: CreateSurface (other)",
									status);
	    }
	    wglSurfaceInfo->lpSurface = lpSurface;
	}
    }

    buf->width = wglPriv->width;
    buf->height = wglPriv->height;
}

void
__wglDDrawFreeBuffer(__GLdrawableBuffer *buf, __WGLdrawablePrivate *wglPriv)
{
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    BOOL frontBuffer = (buf == &wglPriv->glPriv.frontBuffer);
    HRESULT status;

    if (wglSurfaceInfo) {
	if (!frontBuffer) {
	    LPDIRECTDRAWSURFACE lpSurface = wglSurfaceInfo->lpSurface;

	    if (lpSurface) {
		status = IDirectDrawSurface_Release(lpSurface);
		if (status != DD_OK) {
		    __wglDDrawError("__wglDDrawFreeBuffer: Surface: Release", status);
		}
	    }
	}

	if (wglSurfaceInfo->lpClipper) {
	    LPDIRECTDRAWCLIPPER lpClipper = wglSurfaceInfo->lpClipper;

	    status = IDirectDrawClipper_Release(lpClipper);
	    if (status != DD_OK) {
		__wglDDrawError("__wglDDrawFreeBuffer: Clipper: Release", status);
	    }
	}

	__wglFree(wglSurfaceInfo);
    }
    buf->other = NULL;
}

void
__wglDDrawUpdateClipList(__WGLdrawablePrivate *wglPriv, LPDIRECTDRAWCLIPPER lpClipper)
{
    RECT screenClipRect;
    LPRGNDATA rgnData;
    DWORD rgnDataSize;
    HRESULT status;

    /* Convert core clip rect to be screen relative */
    screenClipRect.left = wglPriv->coreClipRect.left + wglPriv->origin.x;
    screenClipRect.top = wglPriv->coreClipRect.top + wglPriv->origin.y;
    screenClipRect.right = wglPriv->coreClipRect.right + wglPriv->origin.x;
    screenClipRect.bottom = wglPriv->coreClipRect.bottom + wglPriv->origin.y;

    /* Allocate a buffer to receive window region clip list */
    status = IDirectDrawClipper_GetClipList(lpClipper,
				&screenClipRect, NULL, &rgnDataSize);
    if (status != DD_OK) {
	__wglDDrawError("__wglDDrawUpdateClipList: GetClipList (1)", status);
    }

    if (rgnDataSize > 0) {
	rgnData = (LPRGNDATA) __wglMalloc(rgnDataSize);

	/* Get window region clip list */
	status = IDirectDrawClipper_GetClipList(lpClipper,
				    &screenClipRect, rgnData, &rgnDataSize);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawUpdateClipList: GetClipList (2)", status);
	}

	__wglMemUpdateOwnershipBuffer(wglPriv, rgnData);
	__wglFree(rgnData);
    } else {
	__wglMemUpdateOwnershipBuffer(wglPriv, NULL);
    }
}

void
__wglDDrawLockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    BOOL frontBuffer = buf == &glPriv->frontBuffer;
    RECT *rect = (frontBuffer) ? &wglPriv->rect : NULL;
    LPDIRECTDRAWSURFACE lpSurface = wglSurfaceInfo->lpSurface;
    DDSURFACEDESC ddsd;
    HRESULT status;

    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = sizeof(ddsd);
    while (1) {
	status = IDirectDrawSurface_Lock(lpSurface, rect, &ddsd,
						DDLOCK_WAIT, NULL);
	if (status == DD_OK) {
	    break;
	} else if (status == DDERR_SURFACELOST) {
	    restoreSurface(wglSurfaceInfo);
	} else if (status == DDERR_SURFACEBUSY) {
	    /* nothing to do, but try the lock again */
	} else {
	    __wglDDrawError("__wglDDrawLockBuffer: Lock", status);
	}
    }

    buf->base = ddsd.lpSurface;
    buf->byteWidth = ddsd.lPitch;

    if (frontBuffer && wglSurfaceInfo->lpClipper) {
	LPDIRECTDRAWCLIPPER lpClipper = wglSurfaceInfo->lpClipper;
	BOOL clipListChanged;

	status = IDirectDrawClipper_IsClipListChanged(lpClipper, &clipListChanged);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawSwapBuffers: IsClipListChanged", status);
	}
	if (wglPriv->coreClipRectChanged || clipListChanged) {
	    __wglDDrawUpdateClipList(wglPriv, lpClipper);
	    wglPriv->coreClipRectChanged = FALSE;
	}
    }
}

void
__wglDDrawUnlockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    LPDIRECTDRAWSURFACE lpSurface = wglSurfaceInfo->lpSurface;
    HRESULT status;

    status = IDirectDrawSurface_Unlock(lpSurface, buf->base);
    if (status != DD_OK) {
	__wglDDrawError("__wglDDrawUnlockBuffer: Unlock", status);
    }
}

void
__wglDDrawFillBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
			GLuint val, GLint x, GLint y, GLint w, GLint h)
{
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    LPDIRECTDRAWSURFACE lpSurface = wglSurfaceInfo->lpSurface;
    BOOL frontBuffer = (buf == &glPriv->frontBuffer);
    RECT dstRect;
    DDBLTFX ddbltfx;
    HRESULT status;

    if (frontBuffer && wglSurfaceInfo->lpClipper) {
	LPDIRECTDRAWCLIPPER lpClipper = wglSurfaceInfo->lpClipper;

	/* Install clipper on the destination surface */
	__wglLockMutex();
	status = IDirectDrawSurface_SetClipper(lpSurface, lpClipper);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawFillBuffer: SetClipper", status);
	}
	__wglUnlockMutex();
    }

    dstRect.left = x;
    dstRect.right = x + w;
    dstRect.top = y;
    dstRect.bottom = y + h;

    memset(&ddbltfx, 0, sizeof(ddbltfx));
    ddbltfx.dwSize = sizeof(ddbltfx);
    ddbltfx.dwFillColor = (WORD) val;
    status = IDirectDrawSurface_Blt(lpSurface, &dstRect,
	    NULL, NULL, DDBLT_COLORFILL | DDBLT_WAIT, &ddbltfx);
    if (status != DD_OK) {
	if (status == DDERR_SURFACELOST) {
	    __WGLdrawablePrivate *wglPriv =
				(__WGLdrawablePrivate *) glPriv->other;

	    restoreSurface(wglSurfaceInfo);
	} else {
	    __wglDDrawError("__wglDDrawFillBuffer: Blt", status);
	}
    }
}

void 
__wglDDrawUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv)
{
}

BOOL
__wglDDrawSwapBuffers(__WGLdrawablePrivate *wglPriv)
{
    __WGLsurfaceInfo *wglSurfaceDst, *wglSurfaceSrc;
    LPDIRECTDRAWSURFACE lpDst, lpSrc;
    LPDIRECTDRAWCLIPPER lpClipper;
    BOOL clipListChanged;
    LPRGNDATA rgnData;
    HRESULT status;

    /* Get source and destination surfaces */
    wglSurfaceDst = (__WGLsurfaceInfo *) wglPriv->glPriv.frontBuffer.other;
    if (wglSurfaceDst == NULL || wglSurfaceDst->lpSurface == NULL) {
        return FALSE;
    }
    lpDst = wglSurfaceDst->lpSurface;
    wglSurfaceSrc = (__WGLsurfaceInfo *) wglPriv->glPriv.backBuffer.other;
    if (wglSurfaceSrc == NULL || wglSurfaceSrc->lpSurface == NULL) {
        return FALSE;
    }
    lpSrc = wglSurfaceSrc->lpSurface;

    /* Install clipper on the destination surface */
    if (wglSurfaceDst->lpClipper) {
	lpClipper = wglSurfaceDst->lpClipper;
	__wglLockMutex();
	status = IDirectDrawSurface_SetClipper(lpDst, lpClipper);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawSwapBuffers: SetClipper", status);
	}
	__wglUnlockMutex();

	status = IDirectDrawClipper_IsClipListChanged(lpClipper, &clipListChanged);
	if (status != DD_OK) {
	    __wglDDrawError("__wglDDrawSwapBuffers: IsClipListChanged", status);
	    return FALSE;
	}
	if (clipListChanged) {
	    __wglDDrawUpdateClipList(wglPriv, lpClipper);
	}
    }

    rgnData = wglPriv->swapRgn;

    /*
    ** In the current implementation, we use only the bounding rect
    ** of all swap hint rects provided.
    */
    if (0 != rgnData->rdh.nCount) {
	LPRECT lpRect = &rgnData->rdh.rcBound;
	RECT dstRect;

	dstRect.left = wglPriv->rect.left + lpRect->left;
	dstRect.right = wglPriv->rect.left + lpRect->right;
	dstRect.top = wglPriv->rect.top + lpRect->top;
	dstRect.bottom = wglPriv->rect.top + lpRect->bottom;

	status = IDirectDrawSurface_Blt(lpDst, &dstRect, lpSrc, lpRect,
					DDBLT_WAIT, 0);
	rgnData->rdh.nCount = 0;
    } else {
	RECT srcRect;

	srcRect.left = 0;
	srcRect.right = wglPriv->width;
	srcRect.top = 0;
	srcRect.bottom = wglPriv->height;

	status = IDirectDrawSurface_Blt(lpDst, &wglPriv->rect, lpSrc, &srcRect,
					DDBLT_WAIT, 0);
    }

    if (status != DD_OK) {
	if (status == DDERR_SURFACELOST) {
	    restoreSurface(wglSurfaceSrc);
	    restoreSurface(wglSurfaceDst);
	} else {
	__wglDDrawError("__wglDDrawSwapBuffers: Blt", status);
	}
	return FALSE;
    }
    return TRUE;
}
#endif /* defined(__WGL_USE_DIRECTDRAW) */

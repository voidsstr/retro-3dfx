/*
** Copyright 1991-1996, Silicon Graphics, Inc.
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

#include <ddraw.h>

#include "wgllib.h"
#include "wglddraw.h"
#include "wgldib.h"
#include "gldevice.h"
/*
** Support for direct framebuffer access and double buffering
** using DirectDraw.
*/

/* XXX: Define MLOCK for iffy lock/unlock trick */
#undef MLOCK

static HINSTANCE hDirectDraw = NULL;
static LPDIRECTDRAW lpDirectDraw = NULL;
static LPDIRECTDRAWSURFACE lpPrimarySurface = NULL;
static LPVOID lpPrimaryBase = NULL;
static GLint lpDirectDrawCount = 0;

typedef struct DDrawSurfaceInfoRec {
    GLboolean validated;
} DDrawSurfaceInfo;

typedef struct DDrawPrimarySurfaceInfoRec {
    DDrawSurfaceInfo	si;	/* this must be first element */
    LPDIRECTDRAWCLIPPER	lpClipper;
} DDrawPrimarySurfaceInfo;

/*
** Allocation / deallocation routines for the DDraw object.  They are
** public, so you can call them outside the scope of this file.
*/
LPDIRECTDRAW
__wglAllocateDDrawObject(GLvoid)
{
    typedef HRESULT (WINAPI *DDCreateProto)
	(GUID *, LPDIRECTDRAW *, IUnknown *);
    DDCreateProto DDCreate;
    LPDIRECTDRAW lpDD;
    DDSURFACEDESC ddsd;
    HRESULT status;

    lpDirectDrawCount++;

    if (lpDirectDrawCount > 1) {
	return lpDirectDraw;
    }

    if ((hDirectDraw = LoadLibrary("DDRAW.DLL")) == NULL) {
	return NULL;
    }
    DDCreate = (DDCreateProto) 
	GetProcAddress(hDirectDraw, "DirectDrawCreate");

    status = (*DDCreate)(NULL, &lpDD, NULL);
    if (status != DD_OK) {
	__wglDDrawError("DDraw: Allocate: Create", status);
	return NULL;
    }

    status = IDirectDraw_SetCooperativeLevel(lpDD, NULL,
					     DDSCL_NORMAL);
    if (status != DD_OK) {
	__wglDDrawError("DDraw: Allocate: SetCooperativeLevel", status);
	return NULL;
    }

    /* allocate primary surface */
    ddsd.dwSize = sizeof(DDSURFACEDESC);
    ddsd.dwFlags = DDSD_CAPS;
    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

    status = IDirectDraw_CreateSurface(lpDD, &ddsd,
				       &lpPrimarySurface, NULL);
    if (status != DD_OK) {
	__wglDDrawError("DDraw: Allocate: CreateSurface", status);
	return NULL;
    }

    lpDirectDraw = lpDD;

    return lpDD;
}

GLvoid
__wglFreeDDrawObject(LPDIRECTDRAW lpDD)
{
    lpDirectDrawCount--;

    assert(lpDirectDrawCount >= 0);
    if (lpDirectDrawCount) {
	return;
    }

    assert(lpDirectDraw != NULL);
    IDirectDraw_Release(lpDD);

    assert(hDirectDraw != NULL);
    FreeLibrary(hDirectDraw);
}

LPDIRECTDRAW
__wglGetDDrawObject(void)
{
    return lpDirectDraw;
}

/* ----------------------------------------------------------------- */

GLint
__wglDDrawGetDisplayMasks(GLint *rMask, GLint *gMask, GLint *bMask)
{
    DDPIXELFORMAT ddpf;
    HRESULT status;

    assert(lpPrimarySurface);

    ddpf.dwSize = sizeof(DDPIXELFORMAT);
    status = IDirectDrawSurface_GetPixelFormat(lpPrimarySurface, &ddpf);
    if (status != DD_OK) {
	__wglDDrawError("DDraw: GetDisplayMasks: GetPixelFormat", status);
    }

    *rMask = ddpf.dwRBitMask;
    *gMask = ddpf.dwGBitMask;
    *bMask = ddpf.dwBBitMask;

    return ddpf.dwRGBBitCount;
}

GLvoid 
__wglDDrawUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv)
{
}

/*
** Do a buffer swap with 
** a DDraw surface as a front buffer and
** a DDraw surface as a back buffer
*/
GLboolean
__wglDDrawDDrawSwapBuffers(__WGLdrawablePrivate *wglPriv)
{
    LPDIRECTDRAWSURFACE lpDst, lpSrc;
    LPDIRECTDRAWCLIPPER lpClipper;
    DDrawPrimarySurfaceInfo *surfaceInfo;
    BOOL clipListChanged;
    LPRGNDATA rgnData;
    HRESULT status;

    /* Get source and destination surfaces */
    if (wglPriv->glPriv.frontBuffer.handle == NULL) {
	return GL_FALSE;
    }
    lpDst = wglPriv->glPriv.frontBuffer.handle;
    if (wglPriv->glPriv.backBuffer.handle == NULL) {
	return GL_FALSE;
    }
    lpSrc = wglPriv->glPriv.backBuffer.handle;

    surfaceInfo = wglPriv->glPriv.frontBuffer.other;
    lpClipper = surfaceInfo->lpClipper;

    /* Install clipper on the destination surface */
    if (lpClipper) {
	__wglLockMutex();
	status = IDirectDrawSurface_SetClipper(lpDst, lpClipper);
	if (status != DD_OK) {
	    __wglDDrawError("DDraw: SwapBuffers: SetClipper", status);
	}
	__wglUnlockMutex();

	status = IDirectDrawClipper_IsClipListChanged(lpClipper, 
						      &clipListChanged);
	if (status != DD_OK) {
	    __wglDDrawError("DDraw: SwapBuffers: IsClipListChanged", status);
	    return GL_FALSE;
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
	    __wglDDrawRestoreSurface(&wglPriv->glPriv.frontBuffer, &wglPriv->glPriv);
	    __wglDDrawRestoreSurface(&wglPriv->glPriv.backBuffer, &wglPriv->glPriv);
	} else {
	    __wglDDrawError("DDraw: SwapBuffers: Blt", status);
	}
	return GL_FALSE;
    }
    return GL_TRUE;
}

/*
** Do a buffer swap with 
** a DDraw surface as a front buffer and
** a DDraw surface as a back buffer
*/
GLboolean
__wglDDrawMemSwapBuffers(__WGLdrawablePrivate *wglPriv)
{
    __GLdrawablePrivate *dp = &wglPriv->glPriv;
    __GLdrawableBuffer *front = &dp->frontBuffer;
    __GLdrawableBuffer *back = &dp->backBuffer;
    __GLregionRect *rect;
    GLubyte *f, *b;
    int i, n, y;
    int x0, y0, x1, y1;
    int padf, padb;
    int width, height;

    width = back->width;
    height = back->height;

    (*front->lock)(front, dp);
    (*back->lock)(back, dp);

    n = dp->ownershipRegion.numRects;
    for (i=0, rect = &dp->ownershipRegion.rects[0]; i<n; i++, rect++) {
	int cnt;

	x0 = rect->x0;
	x1 = rect->x1;
	y0 = rect->y0;
	y1 = rect->y1;

	f = (GLubyte *) front->base 
	    + y0 * front->byteWidth + x0 * front->elementSize;
	b = (GLubyte *) back->base 
	    + y0 * back->byteWidth + x0 * back->elementSize;

	padf = front->byteWidth;
	padb = back->byteWidth;

	cnt = (x1-x0) * front->elementSize;

	/*
	** starting addresses should be suffiently padded
	*/
	if ((((GLuint)f)&3) || (((GLuint)b)&3) || (padf&3) || (padb&3)) {
	    /* unaligned path */
	    for (y=y0; y < y1; y++) {
		memcpy(f, b, cnt);
		f += padf;
		b += padb;
	    }
	} else {
	    /* dword aligned path. */
	    for (y=y0; y < y1; y++) {
		int c16 = cnt & ~0xf;
		int c1 = cnt - c16;

		__asm {
		    mov	ecx, c16;
		    mov	esi, b;
		    mov	edi, f;
		    push	ebp;

		    cmp	ecx, 0;
		    je	l_16end;
		l_16loop:
		    mov	eax, dword ptr [esi];
		    mov	ebx, dword ptr [esi+4];
		    mov	edx, dword ptr [esi+8];
		    mov	ebp, dword ptr [esi+12];
		    mov	dword ptr [edi], eax;
		    mov	dword ptr [edi+4], ebx;
		    mov	dword ptr [edi+8], edx;
		    mov	dword ptr [edi+12], ebp;
		    add	esi, 16;
		    add	edi, 16;
		    sub	ecx, 16;
		    jne	l_16loop;

		l_16end:
		    pop	ebp;
		    mov	ecx, c1;
		    push	ebp;

		    cmp	ecx, 0;
		    je	l_1end;
		l_1loop:
		    mov	al, byte ptr [esi];
		    mov	byte ptr [edi], al;
		    inc	esi;
		    inc	edi;
		    dec	ecx;
		    jne l_1loop;
		l_1end:
		    pop	ebp;
		}
		f += padf;
		b += padb;
	    }
	}
    }

    (*front->unlock)(front, dp);
    (*back->unlock)(back, dp);

    return GL_TRUE;
}

GLvoid *
__wglDDrawGetPrimaryBase(GLvoid)
{
    return lpPrimaryBase;
}

GLboolean
__wglDDrawOpen(void)
{
    if (__wglAllocateDDrawObject()) {
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

GLboolean
__wglDDrawClose(GLvoid)
{
    __wglFreeDDrawObject(lpDirectDraw);

    return GL_TRUE;
}

/* ----------------------------------------------------------------- */

GLvoid
__wglDDrawRestoreSurface(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    LPDIRECTDRAWSURFACE lpSurface = buf->handle;
#ifdef MLOCK
    DDrawSurfaceInfo *surfaceInfo = (DDrawSurfaceInfo *) glPriv->other;
#endif
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
	__wglDDrawError("DDraw: RestoreSurface: IsLost", status);
	return;
    }

#ifdef MLOCK
    assert(surfaceInfo);
    surfaceInfo->validated = GL_FALSE;
#endif

    /* Attempt to restore the surface */
    status = IDirectDrawSurface_Restore(lpSurface);
    if (status == DDERR_WRONGMODE) {
	DDSURFACEDESC ddsd;

	if (buf == &glPriv->frontBuffer) {
	    /* Recreate primary surface */
	    status = IDirectDrawSurface_Release(lpSurface);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: RestoreSurface: Release (primary)", status);
		return;
	    }
	    buf->handle = NULL;

	    ddsd.dwSize = sizeof(ddsd);
	    ddsd.dwFlags = DDSD_CAPS;
	    ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
	    status = IDirectDraw_CreateSurface(lpDirectDraw, &ddsd,
					    &lpSurface, NULL);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: RestoreSurface: CreateSurface (primary)", status);
		return;
	    }
	    buf->handle = lpSurface;

	} else {
	    /* Recreate other surface */
	    ddsd.dwSize = sizeof(ddsd);
	    status = IDirectDrawSurface_GetSurfaceDesc(lpSurface, &ddsd);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: RestoreSurface: GetSurfaceDesc", status);
	    }

	    status = IDirectDrawSurface_Release(lpSurface);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: RestoreSurface: Release", status);
		return;
	    }
	    buf->handle = NULL;

	    ddsd.dwSize = sizeof(ddsd);

	    if (buf == &glPriv->depthBuffer) {
		/* video memory depth buffer */
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT |
			       DDSD_ZBUFFERBITDEPTH;
		ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
		if (__glDevice->bufferFlags & __GL_DEPTH_BUFFER_MASK) {
		    ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		} else {
		    ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}
	    } else {
		/* system memory color buffer */
		ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
		if (__glDevice->bufferFlags & __GL_BACK_BUFFER_MASK) {
		    ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
		} else {
		    ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		}
	    }
	    status = IDirectDraw_CreateSurface(lpDirectDraw, &ddsd,
							&lpSurface, NULL);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: RestoreSurface: CreateSurface", status);
	    }
	    buf->handle = lpSurface;
	}

    } else if (status != DD_OK) {
	__wglDDrawError("DDraw: RestoreSurface: Restore: failed", status);
    }
}


GLvoid
__wglDDrawUpdateClipList(__WGLdrawablePrivate *wglPriv, 
			 LPDIRECTDRAWCLIPPER lpClipper)
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
	__wglDDrawError("DDraw: UpdateClipList: GetClipList (1)", status);
    }

    if (rgnDataSize > 0) {
	rgnData = (LPRGNDATA) __wglMalloc(rgnDataSize);

	/* Get window region clip list */
	status = IDirectDrawClipper_GetClipList(lpClipper,
				    &screenClipRect, rgnData, &rgnDataSize);
	if (status != DD_OK) {
	    __wglDDrawError("DDraw: UpdateClipList: GetClipList (2)", status);
	}

	__wglDIBUpdateOwnershipBuffer(wglPriv, rgnData);
	__wglFree(rgnData);
    } else {
	__wglDIBUpdateOwnershipBuffer(wglPriv, NULL);
    }
}

static GLboolean
UpdateFallback(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
	       GLuint bufferMask)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;

    /* re-allocate a preferred buffer */
    (*buf->free)(buf, glPriv);
    (*buf->mainInit)(buf, glPriv, buf->depth, buf->fallbackInit);

    /* update to the main swapbuffers routine */
    wglPriv->swapBuffers = wglPriv->swapBuffersMain;

    /* and update */
    return (*buf->update)(buf, glPriv, bufferMask);
}

static GLboolean
Update(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    __WGLdrawablePrivate *wglPriv;
    DDSURFACEDESC ddsd;
#ifdef MLOCK
    DDrawSurfaceInfo *surfaceInfo;
#endif
    HRESULT status;

    wglPriv = (__WGLdrawablePrivate *) glPriv->other;

    __wglUpdateDrawableSize(wglPriv);

#ifdef MLOCK
    surfaceInfo = (DDrawSurfaceInfo *) buf->other;
    surfaceInfo->validated = GL_FALSE;
#endif

    /* free the old surface if it is small */
    if (buf->handle) {
	LPDIRECTDRAWSURFACE lpSurface = (LPDIRECTDRAWSURFACE) buf->handle;

	ddsd.dwSize = sizeof(DDSURFACEDESC);
	status = IDirectDrawSurface_GetSurfaceDesc(lpSurface, &ddsd);
	if (status != DD_OK) {
	    __wglDDrawError("DDraw: Update: GetSurfaceDesc", status);
	    return GL_FALSE;
	}

	if ((wglPriv->width > ddsd.dwWidth) ||
	    (wglPriv->height > ddsd.dwHeight)) {
	    status = IDirectDrawSurface_Release(lpSurface);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: Update: Release", status);
		return GL_FALSE;
	    }
	    buf->handle = NULL;
	}
    }

    /* allocate a new surface, if we need to */
    if (buf->handle == NULL) {
	LPDIRECTDRAWSURFACE lpSurface;

	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwWidth = wglPriv->width;
	ddsd.dwHeight = wglPriv->height;

	/* sanity checks. */
	if (ddsd.dwWidth == 0) ddsd.dwWidth = 1;
	if (ddsd.dwHeight == 0) ddsd.dwHeight = 1;

	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT;
	ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;
	if (bufferMask & __glDevice->bufferFlags) {
	    ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	} else {
	    ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	}

	status = IDirectDraw_CreateSurface(lpDirectDraw, &ddsd,
					   &lpSurface, NULL);
	if (status != DD_OK) {
	    /* 
	    ** allocating a ddraw surface failed.  Have to retry 
	    ** using fallback
	    */
	    /* first free ourselves */
	    (*buf->free)(buf, glPriv);
	    /* allocate fallback */
	    (*buf->fallbackInit)(buf, glPriv, buf->depth);
	    /* set the alternate swapbuffers routine */
	    wglPriv->swapBuffers = wglPriv->swapBuffersFallback;
	    /* ..and finally update */
	    if( (*buf->update)(buf, glPriv, bufferMask) == GL_FALSE) {
		/* oops..  Could not resize that one either! */
		__wglDDrawError("DDraw: Update: UpdateFallback", status);
		return GL_FALSE;
	    }

	    /* a new update function, so that we can use preferred method again */
	    buf->update = UpdateFallback;

	    buf->width = wglPriv->width;
	    buf->height = wglPriv->height;
	    return __GL_BUFFER_FALLBACK;
	}
	buf->handle = (void *) lpSurface;
    }

    buf->width = wglPriv->width;
    buf->height = wglPriv->height;

    return GL_TRUE;
}

static GLboolean
UpdateDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    __WGLdrawablePrivate *wglPriv;
    DDSURFACEDESC ddsd;
#ifdef MLOCK
    DDrawSurfaceInfo *surfaceInfo;
#endif
    HRESULT status;

    wglPriv = (__WGLdrawablePrivate *) glPriv->other;

    __wglUpdateDrawableSize(wglPriv);

#ifdef MLOCK
    surfaceInfo = (DDrawSurfaceInfo *) buf->other;
    surfaceInfo->validated = GL_FALSE;
#endif

    /* free the old surface if it is small */
    if (buf->handle) {
	LPDIRECTDRAWSURFACE lpSurface = (LPDIRECTDRAWSURFACE) buf->handle;

	ddsd.dwSize = sizeof(DDSURFACEDESC);
	status = IDirectDrawSurface_GetSurfaceDesc(lpSurface, &ddsd);
	if (status != DD_OK) {
	    __wglDDrawError("DDraw: Update: GetSurfaceDesc", status);
	    return GL_FALSE;
	}

	if ((wglPriv->width > ddsd.dwWidth) ||
	    (wglPriv->height > ddsd.dwHeight)) {
	    status = IDirectDrawSurface_Release(lpSurface);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: Update: Release", status);
		return GL_FALSE;
	    }
	    buf->handle = NULL;
	}
    }

    /* allocate a new surface, if we need to */
    if (buf->handle == NULL) {
	LPDIRECTDRAWSURFACE lpSurface;

	ddsd.dwSize = sizeof(DDSURFACEDESC);
	ddsd.dwWidth = wglPriv->width;
	ddsd.dwHeight = wglPriv->height;
	ddsd.dwZBufferBitDepth = buf->depth;

	/* sanity checks. */
	if (ddsd.dwWidth == 0) ddsd.dwWidth = 1;
	if (ddsd.dwHeight == 0) ddsd.dwHeight = 1;

	ddsd.dwFlags = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT |
	    DDSD_ZBUFFERBITDEPTH;
	ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
	if (bufferMask & __glDevice->bufferFlags) {
	    ddsd.ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY;
	} else {
	    ddsd.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
	}

	status = IDirectDraw_CreateSurface(lpDirectDraw, &ddsd,
					   &lpSurface, NULL);
	if (status != DD_OK) {
	    /* 
	    ** allocating a ddraw surface failed.  Have to retry 
	    ** using fallback
	    */
	    /* first free ourselves */
	    (*buf->free)(buf, glPriv);
	    /* allocate fallback */
	    (*buf->fallbackInit)(buf, glPriv, buf->depth);
	    /* set the alternate swapbuffers routine */
	    wglPriv->swapBuffers = wglPriv->swapBuffersFallback;
	    /* ..and finally update */
	    if( (*buf->update)(buf, glPriv, bufferMask) == GL_FALSE) {
		/* oops..  Could not resize that one either! */
		__wglDDrawError("DDraw: UpdateDepth: UpdateFallback", status);
		return GL_FALSE;
	    }

	    /* a new update function, so that we can use preferred method again */
	    buf->update = UpdateFallback;

	    buf->width = wglPriv->width;
	    buf->height = wglPriv->height;
	    return __GL_BUFFER_FALLBACK;
	}
	buf->handle = (void *) lpSurface;
    }

    buf->width = wglPriv->width;
    buf->height = wglPriv->height;

    return GL_TRUE;
}

static GLboolean
UpdatePrimary(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    __WGLdrawablePrivate *wglPriv;
    DDrawPrimarySurfaceInfo *surfaceInfo;
    HRESULT status;

    wglPriv = (__WGLdrawablePrivate *) glPriv->other;

    __wglUpdateDrawableSize(wglPriv);

    surfaceInfo = (DDrawPrimarySurfaceInfo *) buf->other;
#ifdef MLOCK
    surfaceInfo->si.validated = GL_FALSE;
#endif

    if (surfaceInfo->lpClipper == NULL) {
	LPDIRECTDRAWCLIPPER lpClipper;

	status = IDirectDraw_CreateClipper(lpDirectDraw, 0, &lpClipper, NULL);
	if (status != DD_OK) {
	    __wglDDrawError("Update: CreateClipper", status);
	    return GL_FALSE;
	}

	status = IDirectDrawClipper_SetHWnd(lpClipper, 0, wglPriv->hWnd);
	if (status != DD_OK) {
	    __wglDDrawError("Update: SetHWND", status);
	    return GL_FALSE;
	}

	surfaceInfo->lpClipper = lpClipper;
    }

    return GL_TRUE;
}

static GLvoid
Lock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    DDSURFACEDESC ddsd;
    LPDIRECTDRAWSURFACE lpSurface = buf->handle;
#ifdef MLOCK
    DDrawSurfaceInfo *surfaceInfo = (DDrawSurfaceInfo *) buf->other;
#endif
    HRESULT status;

    if ( ++buf->lockCnt != 1) return;

#ifdef MLOCK
    /* check if surface is lost */
    if (surfaceInfo->validated &&
	(IDirectDrawSurface_IsLost(lpSurface) == DD_OK)) {
	return;
    }
#endif

    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);

    /* lock the surface */
    while(1) {
	status = IDirectDrawSurface_Lock(lpSurface, NULL, &ddsd,
					 DDLOCK_WAIT, NULL);
	if (status == DD_OK) {
	    break;
	} else if (status == DDERR_SURFACELOST) {
	    __wglDDrawRestoreSurface(buf, glPriv);
	} else if (status == DDERR_SURFACEBUSY) {
	    /* try again */
	    continue;
	} else {
	    __wglDDrawError("DDraw: Lock", status);
	}
    }

#ifdef MLOCK
    surfaceInfo->validated = GL_TRUE;
#endif

    /* get the pointer */
    buf->base = ddsd.lpSurface;
    buf->byteWidth = ddsd.lPitch;

#ifdef MLOCK
    /* unlock the surface */
    status = IDirectDrawSurface_Unlock(lpSurface, buf->base);
    if (status != DD_OK) {
	__wglDDrawError("DDraw: Lock(2)", status);
    }
#endif
}

static GLvoid
LockPrimary(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    DDSURFACEDESC ddsd;
    LPDIRECTDRAWSURFACE lpSurface = buf->handle;
    DDrawPrimarySurfaceInfo *surfaceInfo = (DDrawPrimarySurfaceInfo *) buf->other;
    RECT *rect;
    HRESULT status;

    if ( ++buf->lockCnt != 1) return;

#ifdef MLOCK
    /* check if surface is lost */
    if (surfaceInfo->si.validated &&
	(wglPriv->coreClipRectChanged == GL_FALSE) &&
	(IDirectDrawSurface_IsLost(lpSurface) == DD_OK)) {
	return;
    }
#endif

    memset(&ddsd, 0, sizeof(DDSURFACEDESC));
    ddsd.dwSize = sizeof(DDSURFACEDESC);

    rect = &wglPriv->rect;

    /* lock the surface */
    while(1) {
	status = IDirectDrawSurface_Lock(lpSurface, rect, &ddsd,
					 DDLOCK_WAIT, NULL);
	if(status == DD_OK) {
	    break;
	} else if (status == DDERR_SURFACELOST) {
	    __wglDDrawRestoreSurface(buf, glPriv);
	} else if (status == DDERR_SURFACEBUSY) {
	    /* try again */
	    continue;
	} else {
	    __wglDDrawError("DDraw: LockPrimary: Lock", status);
	}
    }

#ifdef MLOCK
    surfaceInfo->si.validated = GL_TRUE;
#endif

    /* get the pointer */
    buf->base = ddsd.lpSurface;
    buf->byteWidth = ddsd.lPitch;

    /* update clip list */
    if (surfaceInfo->lpClipper) {
	BOOL clipListChanged;

	status = IDirectDrawClipper_IsClipListChanged(surfaceInfo->lpClipper,
						      &clipListChanged);
	if (status != DD_OK) {
	    __wglDDrawError("DDraw: LockPrimary: Clip", status);
	}
	if (wglPriv->coreClipRectChanged || clipListChanged) {
	    __wglDDrawUpdateClipList(wglPriv, surfaceInfo->lpClipper);
	    wglPriv->coreClipRectChanged = FALSE;
	}
    }

#ifdef MLOCK
    /* finally unlock the surface */
    status = IDirectDrawSurface_Unlock(lpSurface, buf->base);
    if (status != DD_OK) {
	__wglDDrawError("DDraw: LockPrimary: Unlock", status);
    }
#endif
}

static GLvoid
Unlock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
#ifndef MLOCK
    HRESULT status;
    LPDIRECTDRAWSURFACE lpSurface;
#endif

    if (--buf->lockCnt != 0) return;

#ifndef MLOCK
    lpSurface = buf->handle;

    status = IDirectDrawSurface_Unlock(lpSurface, buf->base);
    if (status != DD_OK) {
	__wglDDrawError("DDraw: Unlock", status);
    }

    /* we should not access the surface after the unlock */
    buf->base = NULL;

#endif
}

static GLvoid
Fill(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint val,
     GLint x, GLint y, GLint w, GLint h)
{
    LPDIRECTDRAWSURFACE lpSurface;
    RECT dstRect;
    DDBLTFX ddbltfx;
    DWORD flags;
    HRESULT status;

    lpSurface = buf->handle;

    dstRect.left = x;
    dstRect.right = x + w;
    dstRect.top = y;
    dstRect.bottom = y + h;

    memset(&ddbltfx, 0, sizeof(DDBLTFX));
    ddbltfx.dwSize = sizeof(DDBLTFX);
    ddbltfx.dwFillColor = (DWORD) val;
    flags = DDBLT_WAIT | DDBLT_COLORFILL;

    status = IDirectDrawSurface_Blt(lpSurface, &dstRect,
				    NULL, NULL, flags, &ddbltfx);
    if (status != DD_OK) {
	if (status == DDERR_SURFACELOST) {
	    __WGLdrawablePrivate *wglPriv = 
		(__WGLdrawablePrivate *) glPriv->other;
	    __wglDDrawRestoreSurface(buf, glPriv);
	    status = IDirectDrawSurface_Blt(lpSurface, &dstRect,
					    NULL, NULL, flags, &ddbltfx);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: Fill: Blt", status);
	    }
	} else {
	    __wglDDrawError("DDraw: Fill: Blt(2)", status);
	}
    }
}

static GLvoid
FillDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint val,
	  GLint x, GLint y, GLint w, GLint h)
{
    LPDIRECTDRAWSURFACE lpSurface;
    RECT dstRect;
    DDBLTFX ddbltfx;
    DWORD flags;
    HRESULT status;

    lpSurface = buf->handle;

    dstRect.left = x;
    dstRect.right = x + w;
    dstRect.top = y;
    dstRect.bottom = y + h;

    memset(&ddbltfx, 0, sizeof(DDBLTFX));
    ddbltfx.dwSize = sizeof(DDBLTFX);
    ddbltfx.dwFillDepth = (DWORD) val;
    flags = DDBLT_WAIT | DDBLT_DEPTHFILL;

    status = IDirectDrawSurface_Blt(lpSurface, &dstRect,
				    NULL, NULL, flags, &ddbltfx);
    if (status != DD_OK) {
	if (status == DDERR_SURFACELOST) {
	    __WGLdrawablePrivate *wglPriv = 
		(__WGLdrawablePrivate *) glPriv->other;
	    __wglDDrawRestoreSurface(buf, glPriv);
	    status = IDirectDrawSurface_Blt(lpSurface, &dstRect,
					    NULL, NULL, flags, &ddbltfx);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: Fill: Blt", status);
	    }
	}
    }
}

static GLvoid
FillPrimary(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint val,
	    GLint x, GLint y, GLint w, GLint h)
{
    LPDIRECTDRAWSURFACE lpSurface;
    LPDIRECTDRAWCLIPPER lpClipper;
    DDrawPrimarySurfaceInfo *surfaceInfo;
    __WGLdrawablePrivate *wglPriv;
    HRESULT status;

    surfaceInfo = (DDrawPrimarySurfaceInfo *) buf->other;

    wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    lpSurface = buf->handle;
    lpClipper = surfaceInfo->lpClipper;

    /* install clipper on the destination surface */
    __wglLockMutex();
    status = IDirectDrawSurface_SetClipper(lpSurface, lpClipper);
    if (status != DD_OK) {
	__wglDDrawError("DDraw: Fill: SetClipper", status);
    }
    __wglUnlockMutex();

    /* make the coords screen relative */
    x += wglPriv->origin.x;
    y += wglPriv->origin.y;

    Fill( buf, glPriv, val, x, y, w, h);
}

static GLvoid
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    HRESULT status;
    LPDIRECTDRAWSURFACE lpSurface;
    DDrawPrimarySurfaceInfo *surfaceInfo;

    surfaceInfo = (DDrawPrimarySurfaceInfo *) buf->other;

    /* handle points to the surface object, base points to a usable address */
    if (buf) {
	if (buf->handle) {
	    lpSurface = buf->handle;
	    status = IDirectDrawSurface_Release(lpSurface);
	    if (status != DD_OK) {
		__wglDDrawError("DDraw: Free", status);
	    }
	    buf->handle = NULL;
	    buf->base = NULL;
	}
    }

    __wglFreeDDrawObject(lpDirectDraw);

    __wglFree(surfaceInfo);
    buf->other = NULL;
}

static GLvoid
FreePrimary(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    HRESULT status;
    LPDIRECTDRAWCLIPPER lpClipper;
    DDrawPrimarySurfaceInfo *surfaceInfo;

    surfaceInfo = (DDrawPrimarySurfaceInfo *) buf->other;
    lpClipper = surfaceInfo->lpClipper;

    if (lpClipper) {
	status = IDirectDrawClipper_Release(lpClipper);
	if (status != DD_OK) {
	    __wglDDrawError("FreePrimary: Clipper: Release", status);
	}

	surfaceInfo->lpClipper = NULL;
    }

    __wglFreeDDrawObject(lpDirectDraw);

    __wglFree(surfaceInfo);
    buf->other = NULL;
}


GLvoid
__wglInitDDraw(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
	       GLint bits, __GLbufFallbackInitFn back)
{
    buf->depth = bits;
    buf->width = buf->height = 0;	/* not assigned yet */
    buf->handle = buf->base = NULL;	/* not assigned yet */
    buf->size = 0;			/* not assigned yet */
    buf->byteWidth = 0;

    buf->elementSize = ((bits-1) / 8) + 1;

    buf->lockCnt = 0;

    buf->update = Update;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = Fill;
    buf->free = Free;
    buf->mainInit = __wglInitDDraw;
    buf->fallbackInit = back;

    /* allocate ddraw object */
    __wglAllocateDDrawObject();

    /* finally, allocate a surfaceInfo structure */
    {
	DDrawSurfaceInfo *surfaceInfo;

	surfaceInfo = (DDrawSurfaceInfo *) __wglCalloc(1, sizeof(DDrawSurfaceInfo));
	buf->other = surfaceInfo;
    }
}

GLvoid
__wglInitDDrawDepth(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
		    GLint bits, __GLbufFallbackInitFn back)
{
    buf->depth = bits;
    buf->width = buf->height = 0;	/* not assigned yet */
    buf->handle = buf->base = NULL;	/* not assigned yet */
    buf->size = 0;			/* not assigned yet */
    buf->byteWidth = 0;

    buf->elementSize = ((bits-1) / 8) + 1;

    buf->lockCnt = 0;

    buf->update = UpdateDepth;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = FillDepth;
    buf->free = Free;
    buf->mainInit = __wglInitDDrawDepth;
    buf->fallbackInit = back;

    /* allocate ddraw object */
    __wglAllocateDDrawObject();

    /* finally, allocate a surfaceInfo structure */
    {
	DDrawSurfaceInfo *surfaceInfo;

	surfaceInfo = (DDrawSurfaceInfo *) __wglCalloc(1, sizeof(DDrawSurfaceInfo));
	buf->other = surfaceInfo;
    }
}


GLvoid
__wglInitDDrawPrimary(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    buf->width = buf->height = 0;	/* not assigned yet */
    buf->handle = buf->base = NULL;	/* not assigned yet */
    buf->size = 0;			/* not assigned yet */
    buf->byteWidth = 0;

    buf->lockCnt = 0;

    buf->update = UpdatePrimary;
    buf->lock = LockPrimary;
    buf->unlock = Unlock;
    buf->fill = FillPrimary;
    buf->free = FreePrimary;
    buf->mainInit = NULL;
    buf->fallbackInit = NULL;

    /* allocate actual primary surface */
    {
	DDSURFACEDESC ddsd;
	DDSURFACEDESC DDSurfaceDesc;
	DDrawPrimarySurfaceInfo *surfaceInfo;
	HRESULT status;

	__wglAllocateDDrawObject();

	/* we are the primary surface. */
	buf->handle = (void *) lpPrimarySurface;

	/* Before doing anything, check if the surface is lost */
	__wglDDrawRestoreSurface(buf, glPriv);

	/* Find the base of the primary surface */
	memset(&ddsd, 0, sizeof(DDSURFACEDESC));
	ddsd.dwSize = sizeof(DDSURFACEDESC);
	status = IDirectDrawSurface_Lock(lpPrimarySurface, NULL, &ddsd,
					 DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
					 NULL);
	if (status != DD_OK) {
	    __wglDDrawError("__wglInitDDrawPrimary: Lock", status);
	    return;
	}
	lpPrimaryBase = (void *) ddsd.lpSurface;
	status = IDirectDrawSurface_Unlock(lpPrimarySurface, ddsd.lpSurface);
	if (status != DD_OK) {
	    __wglDDrawError("__wglInitDDrawPrimary: Unlock", status);
	    return;
	}

	DDSurfaceDesc.dwSize = sizeof(DDSURFACEDESC);
	status = IDirectDraw_GetDisplayMode(lpDirectDraw,
					    &DDSurfaceDesc);
	if (status != DD_OK) {
	    __wglDDrawError("__wglInitDDrawPrimary: GetPixelFormat", status);
	    return;
	}

	buf->base = NULL;
	buf->depth = DDSurfaceDesc.ddpfPixelFormat.dwRGBBitCount;
	buf->elementSize = ((buf->depth-1) / 8) + 1;
	buf->byteWidth = DDSurfaceDesc.lPitch;

	/* finally, allocate a surfaceInfo structure */
	surfaceInfo = (DDrawPrimarySurfaceInfo *) 
	    __wglCalloc(1, sizeof(DDrawPrimarySurfaceInfo));
	buf->other = surfaceInfo;
    }
}

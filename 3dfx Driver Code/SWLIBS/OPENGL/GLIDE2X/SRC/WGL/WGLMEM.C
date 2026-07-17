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

/*
** Support for memory buffers using DIB sections.
**
** The DIB code here is used only to support rendering to double buffered
** windows, rendering to DIBs created by the application is supported
** by the code in wgldib.c
*/

typedef struct __WGLsurfaceInfoRec {
    /*
    ** DIBSection information.
    */
    HBITMAP	hBitmap;
    HBITMAP	hOldBitmap;
    BITMAPINFO  *bmInfo;
    void	*base;
    DWORD	byteWidth;
    HDC		hDC;
} __WGLsurfaceInfo;

void
__wglMemUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;

    __wglUpdateDrawableSize(wglPriv);

    if (!wglSurfaceInfo) {
	wglSurfaceInfo = (__WGLsurfaceInfo *)
				__wglCalloc(1, sizeof(*wglSurfaceInfo));

	buf->other = wglSurfaceInfo;
    }

    /* Free the old DIBSection if it is too small */
    if (wglSurfaceInfo->hBitmap) {
	if (((LONG) wglPriv->width > wglSurfaceInfo->bmInfo->bmiHeader.biWidth)
	    || ((LONG) wglPriv->height > -wglSurfaceInfo->bmInfo->bmiHeader.biHeight))
	{
	    if (DeleteObject(wglSurfaceInfo->hBitmap) == FALSE) {
		__wglError("__wglMemUpdateBuffer: DeleteObject failed");
	    }

	    if (DeleteDC(wglSurfaceInfo->hDC) == FALSE) {
		__wglError("__wglMemUpdateBuffer: DeleteDC failed");
	    }

	    __wglFree(wglSurfaceInfo->bmInfo);
	    wglSurfaceInfo->hBitmap = NULL;
	}
    }

    /* Allocate a new DIBSection if necessary */
    if (!wglSurfaceInfo->hBitmap) {
	HDC hDC = wglPriv->hDC;
	int width = wglPriv->width;
	int height = wglPriv->height + 1;
	int bitsPerPixel = buf->depth;
	int byteWidth;
	HBITMAP hBitmap;
	BITMAPINFO *bmInfo;
	BITMAPINFOHEADER *bmHead;
	VOID *base;
	UINT usage;
	int bmInfoSize;

	bmInfoSize = sizeof(*bmInfo);
	switch (bitsPerPixel) {
	case 1:
	    /* 2 WORD palette indexes */
	    bmInfoSize += 2 * sizeof(WORD);
	    break;
	case 8:
	    /* 256 WORD palette indexes */
	    bmInfoSize += 256 * sizeof(WORD);
	    break;
	case 16:
	    /* 3 DWORD component masks */
	    bmInfoSize += 3 * sizeof(DWORD);
	    break;
	case 24: case 32:
	    break;
	}

	byteWidth = 16 * (((bitsPerPixel * width) + 127) / 128);

	bmInfo = (BITMAPINFO *) __wglCalloc(1, bmInfoSize);

	bmHead = &bmInfo->bmiHeader;
	bmHead->biSize = sizeof(*bmHead);
	bmHead->biWidth = width ? ((byteWidth * 8) / bitsPerPixel) : 1;
	bmHead->biHeight = height ? -height : 1;
	bmHead->biPlanes = 1;
	bmHead->biBitCount = bitsPerPixel;
	bmHead->biXPelsPerMeter = 0;
	bmHead->biYPelsPerMeter = 0;
	bmHead->biClrUsed = 0;	/* all are used */
	bmHead->biClrImportant = 0;	/* all are important */

	switch (buf->depth) {
	case 1:
	    bmHead->biCompression = BI_RGB;
	    bmHead->biSizeImage = 0;
	    usage = DIB_PAL_COLORS;
	    {
		WORD *palIndex = (WORD *) (&bmInfo->bmiColors[0]);

		palIndex[0] = 0;
		palIndex[1] = 1;
	    }
	    break;
	case 8:
	    bmHead->biCompression = BI_RGB;
	    bmHead->biSizeImage = 0;
	    usage = DIB_PAL_COLORS;
	    {
		WORD *palIndex = (WORD *) (&bmInfo->bmiColors[0]);
		int i;

		for (i=0; i<256; i++) {
		    palIndex[i] = i;
		}
	    }
	    break;
	case 16:
	    bmHead->biCompression = BI_BITFIELDS;
	    bmHead->biSizeImage = byteWidth*height;
	    usage = DIB_RGB_COLORS;
	    {
		DWORD *componentMask = (DWORD *) (&bmInfo->bmiColors[0]);
		componentMask[0] = glPriv->modes->redMask;
		componentMask[1] = glPriv->modes->greenMask;
		componentMask[2] = glPriv->modes->blueMask;
	    }
	    break;
	case 24: case 32:
	    bmHead->biCompression = BI_RGB;
	    bmHead->biSizeImage = 0;
	    usage = DIB_RGB_COLORS;
	    break;
	}

	hBitmap = CreateDIBSection(hDC, bmInfo, usage, &base, NULL, 0);
	if (hBitmap == NULL) {
	    __wglError("__wglMemUpdateBuffer: CreateDIBSection failed");
	    exit(1);
	}

	wglSurfaceInfo->hDC = CreateCompatibleDC(hDC);
	wglSurfaceInfo->hOldBitmap = SelectObject(wglSurfaceInfo->hDC, hBitmap);

	wglSurfaceInfo->hBitmap = hBitmap;
	wglSurfaceInfo->bmInfo = bmInfo;
	wglSurfaceInfo->base = base;
	wglSurfaceInfo->byteWidth = byteWidth;

	buf->base = wglSurfaceInfo->base;
	buf->byteWidth = wglSurfaceInfo->byteWidth;
    }

    buf->width = wglPriv->width;
    buf->height = wglPriv->height;
}

void
__wglMemFreeBuffer(__GLdrawableBuffer *buf, __WGLdrawablePrivate *wglPriv)
{
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;

    if (wglSurfaceInfo) {
	SelectObject(wglSurfaceInfo->hDC, wglSurfaceInfo->hOldBitmap);
	if (DeleteObject(wglSurfaceInfo->hBitmap) == FALSE) {
	    __wglError("__wglMemFreeBuffer: DeleteObject failed");
	}

	if (DeleteDC(wglSurfaceInfo->hDC) == FALSE) {
	    __wglError("__wglMemFreeBuffer: DeleteDC failed");
	}

	__wglFree(wglSurfaceInfo->bmInfo);
	__wglFree(wglSurfaceInfo);
    }
    buf->other = NULL;
}

void
__wglMemLockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

void
__wglMemUnlockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

void
__wglMemFillBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
			GLuint val, GLint x, GLint y, GLint w, GLint h)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    HBRUSH hBrush = CreateSolidBrush((COLORREF) 0x0);
    RECT rect;

    rect.left = x;
    rect.right = x + w;
    rect.top = y;
    rect.bottom = y + h;

    FillRect(wglSurfaceInfo->hDC, &rect, hBrush);
    DeleteObject(hBrush);
}

void
__wglMemUpdateOwnershipBuffer(__WGLdrawablePrivate *wglPriv, LPRGNDATA rgnData)
{
    __GLdrawablePrivate *glPriv = &wglPriv->glPriv;
    __GLdrawableBuffer *buf = &glPriv->ownershipBuffer;
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    __GLdrawableRegion *glRegion = &glPriv->ownershipRegion;
    DWORD rgnDataSize;
    RECT clientRect;
    HRGN hRGN;
    int i;
    int freeRgnData = FALSE;

    if (glRegion->rects) {
	__wglFree(glRegion->rects);
    }

    if (rgnData == NULL) {
	glRegion->numRects = 1;
	glRegion->rects = __wglMalloc(sizeof(__GLregionRect));

	/* copy region to drawable private */
	glRegion->rects[0].x0 = wglPriv->coreClipRect.left;
	glRegion->rects[0].y0 = wglPriv->coreClipRect.top;
	glRegion->rects[0].x1 = wglPriv->coreClipRect.right;
	glRegion->rects[0].y1 = wglPriv->coreClipRect.bottom;

	/* Initialize ownership buffer to all zeros */
	clientRect.left = wglPriv->rect.left - wglPriv->origin.x;
	clientRect.top = wglPriv->rect.top - wglPriv->origin.y;
	clientRect.right = wglPriv->rect.right - wglPriv->origin.x;
	clientRect.bottom = wglPriv->rect.bottom - wglPriv->origin.y;
	FillRect(wglSurfaceInfo->hDC, &clientRect, GetStockObject(BLACK_BRUSH));

	/* Set bits in visible regions to one */
	FillRect(wglSurfaceInfo->hDC, &wglPriv->coreClipRect, GetStockObject(WHITE_BRUSH));

	/* Wait for GDI to finish */
	GdiFlush();

	return;
    }

    rgnDataSize = sizeof(rgnData->rdh) + rgnData->rdh.nCount*sizeof(RECT);

    /* translate clip list to client area */
    for (i=0; i<(int)rgnData->rdh.nCount; ++i) {
	RECT *winRect = &((RECT *)rgnData->Buffer)[i];

	winRect->left -= wglPriv->origin.x;
	winRect->top -= wglPriv->origin.y;
	winRect->right -= wglPriv->origin.x;
	winRect->bottom -= wglPriv->origin.y;
    }

    /* Some drivers don't properly clip to the scissor rect we supply;
     * clip here to ensure that scissored clears are actully scissored.
     */
    if (rgnData->rdh.nCount == 1) {
	RECT *dstRect = (RECT *)rgnData->Buffer;
	dstRect->top    = max(wglPriv->coreClipRect.top,    dstRect->top),
	dstRect->bottom = min(wglPriv->coreClipRect.bottom, dstRect->bottom),
	dstRect->left   = max(wglPriv->coreClipRect.left,   dstRect->left),
	dstRect->right  = min(wglPriv->coreClipRect.right,  dstRect->right);

	/* Convert clip list to a region that can be used by GDI */
	hRGN = ExtCreateRegion(NULL, rgnDataSize, rgnData);

    } else {
	HRGN coreClipRgn, fbClipRgn;

	/* Let GDI determine the intersection of the clip list and
	 * the scissor
	 */
	coreClipRgn = CreateRectRgnIndirect(&wglPriv->coreClipRect);
	fbClipRgn = ExtCreateRegion(NULL, rgnDataSize, rgnData);
	hRGN = CreateRectRgn(0, 0, 0, 0);
	CombineRgn(hRGN, coreClipRgn, fbClipRgn, RGN_AND);
	DeleteObject(coreClipRgn);
	DeleteObject(fbClipRgn);

	/* Extract the new rgn data */
	rgnDataSize = GetRegionData(hRGN, 0, NULL);
	rgnData = __wglMalloc(rgnDataSize);
	GetRegionData(hRGN, rgnDataSize, rgnData);
	freeRgnData = TRUE;
    }

    glRegion->numRects = rgnData->rdh.nCount;
    glRegion->rects = __wglMalloc(glRegion->numRects * sizeof(__GLregionRect));

    for (i=0; i<glRegion->numRects; ++i) {
	__GLregionRect *rect = &glRegion->rects[i];
	RECT *winRect = &((RECT *)rgnData->Buffer)[i];

	/* copy region to drawable private */
	rect->x0 = winRect->left;
	rect->y0 = winRect->top;
	rect->x1 = winRect->right;
	rect->y1 = winRect->bottom;
    }

    /* Initialize ownership buffer to all zeros */
    clientRect.left = wglPriv->rect.left - wglPriv->origin.x;
    clientRect.top = wglPriv->rect.top - wglPriv->origin.y;
    clientRect.right = wglPriv->rect.right - wglPriv->origin.x;
    clientRect.bottom = wglPriv->rect.bottom - wglPriv->origin.y;
    FillRect(wglSurfaceInfo->hDC, &clientRect, GetStockObject(BLACK_BRUSH));

    /* Set bits in visible regions to one */
    FillRgn(wglSurfaceInfo->hDC, hRGN, GetStockObject(WHITE_BRUSH));

    /* Wait for GDI to finish */
    GdiFlush();

    DeleteObject(hRGN);
    if (freeRgnData)
	__wglFree(rgnData);
}

void 
__wglMemUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv)
{
    if (wglPriv->glPriv.frontBuffer.elementSize == 1) {
	HDC hDC = wglPriv->hDC;
	HPALETTE hPal = GetCurrentObject(hDC, OBJ_PAL);
	PALETTEENTRY *entries;
	RGBQUAD *colors;
	int i;

	entries = (PALETTEENTRY *) malloc(256 * sizeof(PALETTEENTRY));
	colors = (RGBQUAD *) malloc(256 * sizeof(RGBQUAD));

	GetPaletteEntries(hPal, 0, 256, entries);
	for (i=0; i<256; i++) {
	    colors[i].rgbRed = entries[i].peRed;
	    colors[i].rgbGreen = entries[i].peGreen;
	    colors[i].rgbBlue = entries[i].peBlue;
	}

	if (wglPriv->glPriv.modes->doubleBufferMode) {
	    __GLdrawableBuffer *buf = &wglPriv->glPriv.backBuffer;
	    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;

	    SetDIBColorTable(wglSurfaceInfo->hDC, 0, 256, colors);
	}

	free(entries);
	free(colors);
    }
}

BOOL
__wglMemSwapBuffers(__WGLdrawablePrivate *wglPriv)
{
    __GLdrawableBuffer *buf = &wglPriv->glPriv.backBuffer;
    __WGLsurfaceInfo *wglSurfaceInfo = (__WGLsurfaceInfo *) buf->other;
    LPRGNDATA rgnData = wglPriv->swapRgn;
    BOOL status;

    /*
    ** In the current implementation, we use only the bounding rect
    ** of all swap hint rects provided.
    */
    if (0 != rgnData->rdh.nCount) {
	LPRECT lpRect = &rgnData->rdh.rcBound;
	int x, y, w, h;

	x = lpRect->left;
	y = lpRect->top;
	w = lpRect->right - lpRect->left;
	h = lpRect->bottom - lpRect->top;

	status = BitBlt(wglPriv->hDC, x, y, w, h,
			    wglSurfaceInfo->hDC, x, y, SRCCOPY);
	if (status == FALSE) {
	    __wglError("__wglMemSwapBuffers: BitBlt (1) failed");
	}
	rgnData->rdh.nCount = 0;
    } else {
	status = BitBlt(wglPriv->hDC, 0, 0, wglPriv->width, wglPriv->height,
			    wglSurfaceInfo->hDC, 0, 0, SRCCOPY);
	if (status == FALSE) {
	    __wglError("__wglMemSwapBuffers: BitBlt (2) failed");
	}
    }
    return status;
}

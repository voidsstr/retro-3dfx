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

typedef struct __DIBsurfaceInfoRec {
    /*
    ** DIBSection information.
    */
    HBITMAP     hBitmap;
    HBITMAP     hOldBitmap;
    BITMAPINFO  *bmInfo;
    void        *base;
    DWORD       byteWidth;
    HDC         hDC;
} __DIBsurfaceInfo;


/* -------------------------------------------------------------------- */

void
__wglDIBUpdateOwnershipBuffer(__WGLdrawablePrivate *wglPriv, LPRGNDATA rgnData)
{
    __GLdrawablePrivate *glPriv = &wglPriv->glPriv;
    __GLdrawableBuffer *buf = &glPriv->ownershipBuffer;
    __DIBsurfaceInfo *dibSurfaceInfo = (__DIBsurfaceInfo *) buf->other;
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
	FillRect(dibSurfaceInfo->hDC, &clientRect, GetStockObject(BLACK_BRUSH));

	/* Set bits in visible regions to one */
	FillRect(dibSurfaceInfo->hDC, &wglPriv->coreClipRect, GetStockObject(WHITE_BRUSH));

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
    FillRect(dibSurfaceInfo->hDC, &clientRect, GetStockObject(BLACK_BRUSH));

    /* Set bits in visible regions to one */
    FillRgn(dibSurfaceInfo->hDC, hRGN, GetStockObject(WHITE_BRUSH));

    /* Wait for GDI to finish */
    GdiFlush();

    DeleteObject(hRGN);
    if (freeRgnData)
	__wglFree(rgnData);
}

void 
__wglDIBUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv)
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
	    __DIBsurfaceInfo *dibSurfaceInfo = (__DIBsurfaceInfo *) buf->other;

	    SetDIBColorTable(dibSurfaceInfo->hDC, 0, 256, colors);
	}

	free(entries);
	free(colors);
    }
}

HDC
__wglDIBGetHDC(__GLdrawableBuffer *buf)
{
    __DIBsurfaceInfo *dibSurfaceInfo = (__DIBsurfaceInfo *) buf->other;

    return dibSurfaceInfo->hDC;
}

/* -------------------------------------------------------------------- */

static GLboolean
Update(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    __DIBsurfaceInfo *dibSurfaceInfo = (__DIBsurfaceInfo *) buf->other;

    __wglUpdateDrawableSize(wglPriv);

    if (dibSurfaceInfo == NULL) {
	dibSurfaceInfo = (__DIBsurfaceInfo *)
	    __wglCalloc(1, sizeof(__DIBsurfaceInfo));

	buf->other = dibSurfaceInfo;
    }

    /* free the old dib section, if we have to */
    if (dibSurfaceInfo->hBitmap) {
	if (((LONG) wglPriv->width > 
	     dibSurfaceInfo->bmInfo->bmiHeader.biWidth) ||
	    ((LONG) wglPriv->height >
	     dibSurfaceInfo->bmInfo->bmiHeader.biHeight)) {
	    if (DeleteObject(dibSurfaceInfo->hBitmap) == FALSE) {
		__wglError("DIB: Update: DeleteObject failed");
	    }
	    if (DeleteDC(dibSurfaceInfo->hDC) == FALSE) {
		__wglError("DIB: Update: DeleteDC failed");
	    }
	    __wglFree(dibSurfaceInfo->bmInfo);
	    dibSurfaceInfo->hBitmap = NULL;
	}
    }

    /* allocate new dib section if necessery */
    if (dibSurfaceInfo->hBitmap == NULL) {
	HDC hDC = wglPriv->hDC;
	BITMAPINFO *bmInfo;
	int bmInfoSize;
	BITMAPINFOHEADER *bmHead;
	HBITMAP hBitmap;
	VOID *base;
	int width = wglPriv->width;
	int height = wglPriv->height + 1;
	int bitsPerPixel = buf->depth;
	int byteWidth;
	UINT usage;

	bmInfoSize = sizeof(BITMAPINFO);
	switch(bitsPerPixel) {
	  case 1:
	      /* 2 WORD palette indices */
	      bmInfoSize += 2 * sizeof(WORD);
	      break;
	  case 8:
	      /* 256 WORD palette indices */
	      bmInfoSize += 256 * sizeof(WORD);
	      break;
	  case 16:
	      /* 3 DWORD component masks */
	      bmInfoSize += 3 * sizeof(DWORD);
	      break;
	  case 24:
	  case 32:
	      break;
	}

	byteWidth = 16 * (((bitsPerPixel * width) + 127) / 128);

	bmInfo = (BITMAPINFO *) __wglCalloc(1, bmInfoSize);

	bmHead = &bmInfo->bmiHeader;
	bmHead->biSize = sizeof(BITMAPINFOHEADER);
	bmHead->biWidth = width ? ((byteWidth * 8) / bitsPerPixel) : 1;
	bmHead->biHeight = height? -height : 1;
	bmHead->biPlanes = 1;
	bmHead->biBitCount = bitsPerPixel;
	bmHead->biXPelsPerMeter = 0;
	bmHead->biYPelsPerMeter = 0;
	bmHead->biClrUsed = 0;		/* all are used */
	bmHead->biClrImportant = 0;	/* all are important */

	switch(bitsPerPixel) {
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

		  for (i = 0; i < 256; i++) {
		      palIndex[i] = i;
		  }
	      }
	      break;
	  case 16:
	      bmHead->biCompression = BI_BITFIELDS;
	      bmHead->biSizeImage = byteWidth * height;
	      usage = DIB_RGB_COLORS;
	      {
		  DWORD *componentMask = (DWORD *) (&bmInfo->bmiColors[0]);
		  componentMask[0] = glPriv->modes->redMask;
		  componentMask[1] = glPriv->modes->greenMask;
		  componentMask[2] = glPriv->modes->blueMask;
	      }
	      break;
	  case 24:
	  case 32:
	      bmHead->biCompression = BI_RGB;
	      bmHead->biSizeImage = 0;
	      usage = DIB_RGB_COLORS;
	      break;
	}

	hBitmap = CreateDIBSection(hDC, bmInfo, usage, &base, NULL, 0);
	if (hBitmap == NULL) {
	    __wglError("DIB: Update: CreateDIBSection failed");
	    return GL_FALSE;
	}

	dibSurfaceInfo->hDC = CreateCompatibleDC(hDC);
	dibSurfaceInfo->hOldBitmap = SelectObject(dibSurfaceInfo->hDC, hBitmap);
	dibSurfaceInfo->hBitmap = hBitmap;
	dibSurfaceInfo->bmInfo = bmInfo;
	dibSurfaceInfo->base = base;
	dibSurfaceInfo->byteWidth = byteWidth;

	buf->base = base;
	buf->byteWidth = dibSurfaceInfo->byteWidth;
    }

    buf->width = wglPriv->width;
    buf->height = wglPriv->height;

    return GL_TRUE;
}

static GLboolean
UpdateUnmanaged(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, 
		GLuint bufferMask)
{
    return GL_TRUE;
}

static void
Lock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
LockUnmanaged(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
Unlock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
UnlockUnmanaged(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
Fill(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint val,
     GLint x, GLint y, GLint w, GLint h)
{
    __DIBsurfaceInfo *dibSurfaceInfo = (__DIBsurfaceInfo *) buf->other;
    HBRUSH hBrush;
    RECT rect;

    rect.left = x;
    rect.right = x + w;
    rect.top = y;
    rect.bottom = y + h;

    hBrush = CreateSolidBrush((COLORREF) 0x0);

    FillRect(dibSurfaceInfo->hDC, &rect, hBrush);
    DeleteObject(hBrush);
}

static void
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    __DIBsurfaceInfo *dibSurfaceInfo = (__DIBsurfaceInfo *) buf->other;

    if (dibSurfaceInfo) {
	SelectObject(dibSurfaceInfo->hDC, dibSurfaceInfo->hOldBitmap);
	if (DeleteObject(dibSurfaceInfo->hBitmap) == FALSE) {
	    __wglError("DIB: Free: DeleteObject failed");
	}

	if (DeleteDC(dibSurfaceInfo->hDC) == FALSE) {
	    __wglError("DIB: Free: DeleteDC failed");
	}

	__wglFree(dibSurfaceInfo->bmInfo);
	__wglFree(dibSurfaceInfo);
    }
    buf->other = NULL;
}

/* -------------------------------------------------------------------- */

void
__wglInitDIB(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLint bits)
{
    buf->depth = bits;
    buf->width = buf->height = 0;	/* not assigned yet */
    buf->handle = buf->base = NULL;	/* not assigned yet */
    buf->size = 0;
    buf->byteWidth = 0;

    buf->elementSize = ((bits-1) / 8) + 1;

    buf->update = Update;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = Fill;
    buf->free = Free;
}

void
__wglInitUnmanagedDIB(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
		      GLint bits)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    HBITMAP hBitmap;
    DIBSECTION DIBSection;

    hBitmap = GetCurrentObject(wglPriv->hDC, OBJ_BITMAP);
    if (hBitmap == NULL) {
	__wglError("__wglInitUnmanagedDIB: GetCurrentObject failed");
    }
    if (GetObject(hBitmap, sizeof(DIBSECTION), &DIBSection) == 0) {
	__wglError("__wglInitUnmanagedDIB: GetObject failed");
    }

    buf->depth = bits;
    buf->elementSize = ((bits-1) / 8) + 1;

    buf->update = UpdateUnmanaged;
    buf->lock = LockUnmanaged;
    buf->unlock = UnlockUnmanaged;
    buf->fill = NULL;
    buf->free = NULL;

    buf->width = DIBSection.dsBm.bmWidth;
    buf->height = DIBSection.dsBm.bmHeight;
    if (DIBSection.dsBmih.biHeight < 0) {
	wglPriv->glPriv.yInverted = GL_TRUE;
    } else {
	wglPriv->glPriv.yInverted = GL_FALSE;
    }
    buf->base = (GLvoid *) DIBSection.dsBm.bmBits;

    /*
    ** BOGUS: Windows NT4 lies about the WidthBytes of DIBs.  The
    ** reported value is 2-byte aligned, whereas it should really be
    ** 4-byte aligned.
    */
    buf->byteWidth = (DIBSection.dsBm.bmWidthBytes+3) & ~3;

    wglPriv->width = buf->width;
    wglPriv->height = buf->height;
}

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

static void
__wglDIBUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
__wglDIBFreeBuffer(__GLdrawableBuffer *buf, __WGLdrawablePrivate *wglPriv)
{
}

static void
__wglDIBFreeBuffers(__WGLdrawablePrivate *wglPriv)
{
}

static void
__wglDIBUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv)
{
}

static void
__wglDIBLockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
__wglDIBUnlockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
__wglDIBFillBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
			GLuint val, GLint x, GLint y, GLint w, GLint h)
{
}

static BOOL
__wglDIBSwapBuffers(__WGLdrawablePrivate *wglPriv)
{
    return TRUE;
}

void
__wglDIBInitDrawable(__WGLdrawablePrivate *wglPriv)
{
    __GLdrawablePrivate *glPriv = &wglPriv->glPriv;
    __GLdrawableBuffer *buf = &glPriv->frontBuffer;
    int bitsPerPixel = glPriv->modes->indexBits;
    int elementSize;
    HBITMAP hBitmap;
    DIBSECTION DIBSection;

    wglPriv->freeBuffers = __wglDIBFreeBuffers;
    wglPriv->updatePalette = __wglDIBUpdateDrawablePalette;
    wglPriv->swapBuffers = __wglDIBSwapBuffers;

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
    glPriv->frontBuffer.update = __wglDIBUpdateBuffer;
    glPriv->frontBuffer.lock = NULL;
    glPriv->frontBuffer.unlock = NULL;
    glPriv->frontBuffer.fill = NULL;

    glPriv->backBuffer.depth = bitsPerPixel;
    glPriv->backBuffer.elementSize = elementSize;
    glPriv->backBuffer.update = __wglDIBUpdateBuffer;
    glPriv->backBuffer.lock = NULL;
    glPriv->backBuffer.unlock = NULL;
    glPriv->backBuffer.fill = NULL;

    glPriv->ownershipBuffer.depth = 1;
    glPriv->ownershipBuffer.elementSize = 1;
    glPriv->ownershipBuffer.update = NULL;
    glPriv->ownershipBuffer.lock = NULL;
    glPriv->ownershipBuffer.unlock = NULL;
    glPriv->ownershipBuffer.fill = NULL;

    if ((hBitmap = GetCurrentObject(wglPriv->hDC, OBJ_BITMAP)) == NULL) {
	__wglError("__wglDIBInitDrawable: GetCurrentObject(OBJ_BITMAP) failed.");
    }
    if (GetObject(hBitmap, sizeof(DIBSECTION), &DIBSection) == 0) {
	__wglError("__wglDIBInitDrawable: GetObject(DIBSECTION) failed.");
    }

    wglPriv->width = DIBSection.dsBm.bmWidth;
    wglPriv->height = DIBSection.dsBm.bmHeight;
    if (DIBSection.dsBmih.biHeight < 0) {
	glPriv->yInverted = TRUE;
    } else {
	glPriv->yInverted = FALSE;
    }

    buf->width = wglPriv->width;
    buf->height = wglPriv->height;
    buf->base = (void *) DIBSection.dsBm.bmBits;

    /* BOGUS!  Windows NT 4 lies about the WidthBytes of DIBs.  The reported
     * value is 2-byte aligned whereas it should really be 4-byte aligned.
     */
    buf->byteWidth = (DIBSection.dsBm.bmWidthBytes+3) & ~3;
}

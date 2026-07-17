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
/****************************************************************************
*
*                       MegaGraph Graphics Library
*
*                   Copyright (C) 1996 SciTech Software.
*                           All rights reserved.
*
* Filename:     $Workfile: cosmomgl.c$
* Version:      $Revision: 4$
*
* Language:     ANSI C
* Environment:  IBM PC (MS DOS)
*
* Description:  Module to interface between the SciTech MGL and Cosmo OpenGL
*               so we can get Cosmo OpenGL to render directly to MGL 
*               fullscreen surfaces for maximum performance.
*
* $Date: 10/11/00 7:52:38 PM$ $Author: Brent$
*
****************************************************************************/

#if defined(__GL_SUPPORT_MGL)
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "GL/gl.h"
typedef struct __GLinterfaceRec __GLcontext;
#include "GL/glcore.h"
#include "cosmomgl.h"

/****************************************************************/
/*
** Objects
*/

/* rendering surface (color buffers and ancillary buffers) */
struct __MGLCosmoSurface {
    /* surface info and visual for which this surface was created */
    __MGLCosmoSurfaceInfo info;
    MGLVisual             visual;
    __GLdrawablePrivate   glPriv;
    };

/* rendering context */
struct __MGLCosmoContext {
    /* surface info and visual for which this context was created */
    __GLcontextModes      *modes;
    __GLcontext           *gc;
    MGLCosmoSurface       surface;
    };


/****************************************************************/
/*
** Global State
*/

/* current context */
static MGLCosmoContext __mglCurrentContext;

#define SET_CURRENT_CONTEXT(ctx)	__mglCurrentContext = ctx
#define GET_CURRENT_CONTEXT()		__mglCurrentContext


/****************************************************************/
/*
** Surface Imports/Callbacks
** (callbacks used by glcore to get surface information)
*/
static void __mglCosmoUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    MGLCosmoSurface surface = (MGLCosmoSurface)glPriv->other;
    buf->width = surface->info.width;
    buf->height = surface->info.height;
}

static void __mglCosmoFillRect(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv,
    int color,int x,int y,int w,int h)
{
    MGLCosmoSurface surface = (MGLCosmoSurface)glPriv->other;
    surface->info.Fill(x,y,w,h,color);
}

static void __mglCosmoInitSurface(MGLCosmoSurface surface)
{
    __GLdrawablePrivate *glPriv = &surface->glPriv;
    int bitsPerPixel = glPriv->modes->indexBits;
    int elementSize;

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
    glPriv->frontBuffer.update = __mglCosmoUpdateBuffer;
    glPriv->frontBuffer.lock = (void*)(surface->info.BeginDirectAccess);
    glPriv->frontBuffer.unlock = (void*)(surface->info.EndDirectAccess);
	if (surface->info.Fill)
		glPriv->frontBuffer.fill = __mglCosmoFillRect;
	else 
		glPriv->frontBuffer.fill = NULL;

    glPriv->backBuffer.depth = bitsPerPixel;
    glPriv->backBuffer.elementSize = elementSize;
    glPriv->backBuffer.update = __mglCosmoUpdateBuffer;
    glPriv->backBuffer.lock = (void*)(surface->info.BeginDirectAccess);
    glPriv->backBuffer.unlock = (void*)(surface->info.EndDirectAccess);
	if (surface->info.Fill)
		glPriv->backBuffer.fill = __mglCosmoFillRect;
	else
		glPriv->backBuffer.fill = NULL;

    glPriv->ownershipBuffer.depth = 1;
    glPriv->ownershipBuffer.elementSize = 1;
    glPriv->ownershipBuffer.update = __mglCosmoUpdateBuffer;
    glPriv->ownershipBuffer.lock = NULL;
    glPriv->ownershipBuffer.unlock = NULL;
    glPriv->ownershipBuffer.fill = NULL;

    glPriv->yInverted = TRUE;

    glPriv->width = 0;
    glPriv->height = 0;
}


/****************************************************************/
/*
** Context Imports/Callbacks
** (callbacks used by glcore for memory allocation and surface information)
*/
static void * __mglImpMalloc(__GLcontext *gc, size_t size)
{
    void *ptr;

    if (size == 0) 
	return NULL;
    ptr = (void *) GlobalAlloc(GPTR, size);
    if (ptr == NULL) 
	return NULL;	/* XXX out of memory error */
    return ptr;
}

static void * __mglImpCalloc(__GLcontext *gc, size_t numElements, size_t elementSize)
{
    void *ptr;

    if (numElements == 0 || elementSize == 0) 
	return NULL;
    ptr = (void *) GlobalAlloc(GPTR, numElements * elementSize);
    if (ptr == NULL) 
	return NULL;	/* XXX out of memory error */
    return ptr;
}

static void * __mglImpRealloc(__GLcontext *gc, void *oldPtr, size_t newSize)
{
    void *newPtr = NULL;

    if (newSize != 0) {
	newPtr = (void *) GlobalAlloc(GPTR, newSize);
	if (oldPtr && newPtr) {
	    DWORD oldSize = GlobalSize(oldPtr);

	    memcpy(newPtr, oldPtr, (oldSize <= newSize ? oldSize : newSize));
	    GlobalFree(oldPtr);
	    }
        } 
    else if (oldPtr) 
	GlobalFree(oldPtr);
    if (newPtr == NULL) 
	return NULL;	/* XXX out of memory error */
    return newPtr;
}

static void __mglImpFree(__GLcontext *gc, void *ptr)
{
    if (ptr) 
	GlobalFree(ptr);
}

static void __mglImpWarning(__GLcontext *gc, const char* msg, ...)
{
}

static void __mglImpFatal(__GLcontext *gc, const char* msg, ...)
{
    abort();
}

static __GLdrawablePrivate * __mglImpGetDrawablePrivate(__GLcontext *gc)
{
    MGLCosmoContext ctx = (MGLCosmoContext) gc->imports.other;
    return &ctx->surface->glPriv;
}

static void __mglImpGetDrawableSize(__GLcontext *gc, int *w, int *h)
{
    MGLCosmoContext ctx = (MGLCosmoContext)gc->imports.other;
    MGLCosmoSurface surface = ctx->surface;
    *w = surface->info.width;
    *h = surface->info.height;
}

static __GLimports imports = {
    __mglImpMalloc,
    __mglImpCalloc,
    __mglImpRealloc,
    __mglImpFree,
    __mglImpWarning,
    __mglImpFatal,
    __mglImpGetDrawablePrivate,
    __mglImpGetDrawableSize,
    NULL,
    };


void mglCosmoSetCoreClipRect(__GLdrawablePrivate *glPriv,GLint x, GLint y, GLsizei w, GLsizei h)
{
    MGLCosmoSurface surface = (MGLCosmoSurface)glPriv->other;
// TODO: Add a callback to inform the MGL when the OpenGL clip rect changes.
//	surface->info.setClipRect(x,y,w,h);
}

/****************************************************************/
/*
** Visual/PixelFormat Selection
*/

static void CreateRGBPalette(__MGLCosmoSurfaceInfo *info)
{
    pixel_format_t *pf = &info->pf;
    palette_t *p = &info->pal[0];
    int i;

    for (i = 0; i < 256; i++) {
        p->red =   (((i >> 0) & 0x7) * 255) / 0x7;
        p->green = (((i >> 3) & 0x7) * 255) / 0x7;
        p->blue =  (((i >> 6) & 0x3) * 255) / 0x3;
        p->alpha = 0;
        }
}

/*
** Modifies surface info and visual to be compatible with the glcore.
*/
void WINAPI mglCosmoChooseVisual(__MGLCosmoSurfaceInfo *info, MGLVisual *visual)
{
    /* glcore supports 16 and 32 bit depth buffers */
    if (visual->depth_size) {
	if (visual->depth_size <= 16) 
	    visual->depth_size = 16;
	else 
	    visual->depth_size = 32;
        }

    /* glcore supports 8 bit stencil buffers */
    if (visual->stencil_size) 
	visual->stencil_size = 8;

    /* glcore supports 64 bit accumulation buffers */
    if (visual->accum_size) 
	visual->accum_size = 64;

    /* Force RGB mode for non-palette display modes */
    if (info->bitsPerPixel > 8) 
	visual->rgb_flag = GL_TRUE;

    /* Return the palette used for 8bpp dithering */
    if (info->bitsPerPixel == 8 && visual->rgb_flag) 
	CreateRGBPalette(info);
}

/*
** Checks that the visual info compatible with the glcore.
*/
BOOL WINAPI mglCosmoCheckVisual(__MGLCosmoSurfaceInfo *info, MGLVisual *visual)
{
    /* glcore supports 16 and 32 bit depth buffers */
    if (visual->depth_size && (visual->depth_size != 16 && visual->depth_size != 32))
	return GL_FALSE;

    /* glcore supports 8 bit stencil buffers */
    if (visual->stencil_size && (visual->stencil_size != 8))
	return GL_FALSE;

    /* glcore supports 64 bit accumulation buffers */
    if (visual->accum_size && (visual->accum_size != 64))
	return GL_FALSE;

    /* Force RGB mode for non-palette display modes */
    if (info->bitsPerPixel > 8 && !visual->rgb_flag)
	return GL_FALSE;

    /* Return the palette used for 8bpp dithering */
    if (info->bitsPerPixel == 8 && visual->rgb_flag) 
	CreateRGBPalette(info);
    return GL_TRUE;
}

/****************************************************************/
/*
** Context Management API
*/

/*
** Counts the number of set bits in a mask and determines the shift amount.
*/
static int decodeColorMask(DWORD mask)
{
    int i = 0, firstset = 0;
    int size, shift;

    if (mask) {
	/* skip clear bits */
	while (~mask & (1 << i)) { ++i; };
	firstset = i;
	/* count set bits */
	while ( mask & (1 << i)) { ++i; };
        }

    size = i - firstset;
    shift = firstset;
    return size;
}

/*
** Creates a context compatible with the specified surface info and visual.
*/
MGLCosmoContext WINAPI mglCosmoCreateContext(__MGLCosmoSurfaceInfo *info, MGLVisual *visual)
{
    MGLCosmoContext ctx;
    __GLcontextModes *modes;
    int redBits, greenBits, blueBits, alphaBits;

    if ((ctx = (MGLCosmoContext)malloc(sizeof(*ctx))) == NULL)
	return NULL;
    memset(ctx, 0, sizeof(*ctx));

    /* set up modes */
    if ((modes = (__GLcontextModes *)malloc(sizeof(*modes))) == NULL)
	return NULL;
    ctx->modes = modes;

    memset(modes, 0, sizeof(*modes));
    modes->rgbMode = visual->rgb_flag;
    modes->colorIndexMode = !visual->rgb_flag;

    /* Note that as far as Cosmo is concerned, we are always double buffering.
     * In the case of a memory bitmap, the front and back buffers point to
     * the same location.
     */
    modes->doubleBufferMode = TRUE;
    modes->stereoMode = FALSE;
    modes->haveAccumBuffer = visual->accum_size != 0;
    modes->haveDepthBuffer = visual->depth_size != 0;
    modes->haveStencilBuffer = visual->stencil_size != 0;
    if (info->bitsPerPixel > 8) {
        modes->redBits = redBits = decodeColorMask(info->pf.redMask);
        modes->greenBits = greenBits = decodeColorMask(info->pf.greenMask);
        modes->blueBits = blueBits = decodeColorMask(info->pf.blueMask);
        modes->alphaBits = alphaBits = decodeColorMask(info->pf.rsvdMask);
        modes->redMask = info->pf.redMask << info->pf.redPos;
        modes->greenMask = info->pf.greenMask << info->pf.greenPos;
        modes->blueMask = info->pf.blueMask << info->pf.bluePos;
        modes->alphaMask = info->pf.rsvdMask << info->pf.rsvdPos;
        }
    else {
        modes->redBits = redBits = 3;
        modes->greenBits = greenBits = 3;
        modes->blueBits = blueBits = 2;
        modes->alphaBits = alphaBits = 0;
        modes->redMask = 0x7;
        modes->greenMask = 0x7;
        modes->blueMask = 0x3;
        modes->alphaMask = 0x0;
        }

    /* SGI OpenGL does not support destination alpha, so we set these
     * bits to zero to solve the problem.
     */
    modes->alphaBits = 0;
    modes->alphaMask = 0;

    modes->indexBits = redBits + greenBits + blueBits + alphaBits;
    modes->accumRedBits = 16;
    modes->accumGreenBits = 16;
    modes->accumBlueBits = 16;
    modes->accumAlphaBits = 16;
    modes->depthBits = visual->depth_size;
    modes->stencilBits = visual->stencil_size;
    modes->numAuxBuffers = 0;
    modes->level = 0;

    /* pointer back to context */
    imports.other = (void *)ctx;

    /* create a core rendering context */
    ctx->gc = __glCoreCreateContext(&imports, modes);
    return ctx;
}

/*
** Deletes a context.
*/
BOOL WINAPI mglCosmoDeleteContext(MGLCosmoContext ctx)
{
    /* have glcore destroy context */
    if (!ctx->gc->exports.destroyContext(ctx->gc)) 
	return FALSE;
    __glCoreNopDispatch();
    free(ctx);
    return TRUE;
}

/*
** Makes the specified context the current context and binds the
** specified surface to the current context.
*/
BOOL WINAPI mglCosmoMakeCurrent(MGLCosmoContext ctx, MGLCosmoSurface surface)
{
    MGLCosmoContext ctxOld = GET_CURRENT_CONTEXT();

    /* LoseCurrent from old context/surface */
    if (ctxOld) {
	/* have glcore lose current */
	if (!ctxOld->gc->exports.loseCurrent(ctxOld->gc))
	    return FALSE;

	/* unbind surface from context */
	ctxOld->surface = NULL;

	/* context is no longer current */
	SET_CURRENT_CONTEXT(NULL);
        }

    /* MakeCurrent to new context/surface */
    if (ctx) {
	__GLcontext *gc = ctx->gc;
	__GLdrawablePrivate *glPriv = &surface->glPriv;

	/* initialize surface (if necessary) */
	if (glPriv->modes == NULL) {
	    /* data for context modes */
	    glPriv->modes = (__GLcontextModes *) malloc(sizeof(*glPriv->modes));

	    /* surface modes match context modes */
	    *glPriv->modes = *ctx->modes;

	    /* fill in surface imports functions */
	    glPriv->malloc = malloc;
	    glPriv->calloc = calloc;
	    glPriv->realloc = realloc;
	    glPriv->free = free;
	    glPriv->addSwapRect = NULL;
	    glPriv->setClipRect = mglCosmoSetCoreClipRect;

	    /* pointer back to surface */
	    glPriv->other = surface;

	    __mglCosmoInitSurface(surface);
	    }

	/* bind surface to context */
	ctx->surface = surface;

	/* have glcore make current */
	if (!gc->exports.makeCurrent(gc))
	    return FALSE;

	SET_CURRENT_CONTEXT(ctx);
        } 
    else 
	__glCoreNopDispatch();

    return TRUE;
}


/****************************************************************/
/*
** Surface Management API
*/

/*
** Creates a surface (color buffers and ancillary buffers).
*/
MGLCosmoSurface WINAPI mglCosmoCreateSurface(__MGLCosmoSurfaceInfo *info, MGLVisual *visual)
{
    MGLCosmoSurface surface;

    /* copy surface info and visual */
    if ((surface = (MGLCosmoSurface)malloc(sizeof(*surface))) == NULL)
	return NULL;
    memset(surface, 0, sizeof(*surface));
    surface->info = *info;
    surface->visual = *visual;
    return surface;
}

/*
** Deletes a surface.
*/
BOOL WINAPI mglCosmoDeleteSurface(MGLCosmoSurface surface)
{
    /* Have glcore free any memory it may have attached to the surface */
    if (surface->glPriv.freePrivate) 
	surface->glPriv.freePrivate(&surface->glPriv);
    free(surface->glPriv.modes);
    free(surface);
    return TRUE;
}

/*
** Sets the front and back pointers of the surface bound to the current context.
*/
void WINAPI mglCosmoSetSurfacePtr(MGLCosmoSurface surface,void *frontSurface, void *backSurface)
{
    MGLCosmoContext ctx = GET_CURRENT_CONTEXT();
	__GLcontext *gc = ctx->gc;

    surface->glPriv.frontBuffer.base = frontSurface;
    surface->glPriv.frontBuffer.byteWidth = surface->info.bytesPerLine;
    surface->glPriv.backBuffer.base = backSurface;
    surface->glPriv.backBuffer.byteWidth = surface->info.bytesPerLine;

	/* Let glcore know that the buffer surface pointers have changed */
	gc->exports.changeBuffers(gc);
}
#endif /* defined(__GL_SUPPORT_MGL) */

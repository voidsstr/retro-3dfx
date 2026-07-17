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
#include "gldevice.h"


/*
** Generic pixel formats
**
** Used as a first stab at what pixel formats are supported.  It is filtered
** according to the current frame buffer depth, into another array.
*/

#define NUM_WGL_RAW_PIXELFORMATS \
		(sizeof(wglRawPixelFormats) / sizeof(PIXELFORMATDESCRIPTOR))
static PIXELFORMATDESCRIPTOR wglRawPixelFormats[] = {
    /*
    ** 32 bit
    */
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_RGBA,					// pixel type
	32, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_RGBA,					// pixel type
	32, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,					// pixel type
	32, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,					// pixel type
	32, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_COLORINDEX,				// pixel type
	32, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_COLORINDEX,				// pixel type
	32, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_COLORINDEX,				// pixel type
	32, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_COLORINDEX,				// pixel type
	32, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    /*
    ** 24 bit
    */
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_RGBA,					// pixel type
	24, 8, 16, 8, 8, 8, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_RGBA,					// pixel type
	24, 8, 16, 8, 8, 8, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,					// pixel type
	24, 8, 16, 8, 8, 8, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,					// pixel type
	24, 8, 16, 8, 8, 8, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_COLORINDEX,				// pixel type
	24, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_COLORINDEX,				// pixel type
	24, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_COLORINDEX,				// pixel type
	24, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_COLORINDEX,				// pixel type
	24, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    /*
    ** 16 bit
    */
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_RGBA,					// pixel type
	16, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_RGBA,					// pixel type
	16, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,					// pixel type
	16, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,					// pixel type
	16, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_COLORINDEX,				// pixel type
	16, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_COLORINDEX,				// pixel type
	16, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_COLORINDEX,				// pixel type
	16, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_COLORINDEX,				// pixel type
	16, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    /*
    ** 8 bit
    */
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_RGBA,					// pixel type
	8, 3, 5, 3, 2, 2, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_RGBA,					// pixel type
	8, 3, 5, 3, 2, 2, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,					// pixel type
	8, 3, 5, 3, 2, 2, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_RGBA,					// pixel type
	8, 3, 5, 3, 2, 2, 0,				// color buffer
	0, 0,						// alpha buffer
	64, 16, 16, 16, 16,				// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_COLORINDEX,				// pixel type
	8, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
	PFD_TYPE_COLORINDEX,				// pixel type
	8, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_COLORINDEX,				// pixel type
	8, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	32,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
    {
	sizeof(PIXELFORMATDESCRIPTOR),			// size
	1,						// version
	PFD_SUPPORT_OPENGL |				// flags
	PFD_DRAW_TO_WINDOW |
	PFD_DOUBLEBUFFER,
	PFD_TYPE_COLORINDEX,				// pixel type
	8, 0, 0, 0, 0, 0, 0,				// color buffer
	0, 0,						// alpha buffer
	0, 0, 0, 0, 0,					// accumulation buffer
	16,						// depth buffer
	8,						// stencil buffer
	0,						// aux buffers
	PFD_MAIN_PLANE,					// layer type
	0,						// (reserved)
	0, 0, 0,					// layer masks
    },
};

/*
** List of actual supported pixel formats.  This is filled in as
** appropriate whenever the display mode changes.
*/
static int wglNumPixelFormats;
static PIXELFORMATDESCRIPTOR **wglPixelFormats;
static int wglPixelFormatsDirty = TRUE;



/*
** utility routines to be also used by other driver pixel format
** implementations
*/

void
__wglDecodeColorMask(DWORD mask, int *size, int *shift)
{
    int i = 0, firstset = 0;

    if (mask) {
	/* skip clear bits */
	while (~mask & (1 << i)) { ++i; };
	firstset = i;
	/* count set bits */
	while ( mask & (1 << i)) { ++i; };
    }

    *size = i - firstset;
    *shift = firstset;
}

BOOL
__wglPixelFormatSupported(PIXELFORMATDESCRIPTOR *ppfd,
	int displayDepth, int rMask, int gMask, int bMask)
{
    /*
    ** Modify DRAW_TO_XXX and SUPPORT_XXX flags as needed
    */

    /*
    ** Can only DRAW_TO_WINDOW if pixel format matches display depth
    */
    if (ppfd->cColorBits == displayDepth) {
	ppfd->dwFlags |= PFD_DRAW_TO_WINDOW;
    } else {
	ppfd->dwFlags &= ~PFD_DRAW_TO_WINDOW;
    }

    /*
    ** Can only SUPPORT_GDI or DRAW_TO_BITMAPS if not double buffered
    */
    if (!(ppfd->dwFlags & PFD_DOUBLEBUFFER)) {
	ppfd->dwFlags |= (PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP);
    } else {
	ppfd->dwFlags &= ~(PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP);
	/*
	** Can only double buffer if draw to window
	*/
	if (!(ppfd->dwFlags & PFD_DRAW_TO_WINDOW)) {
	    return FALSE;
	}
    }

    /*
    ** Might need to set palette flags for 8 bit display modes
    */
    if (ppfd->cColorBits == 8 && ppfd->iPixelType == PFD_TYPE_RGBA) {
	ppfd->dwFlags |= PFD_NEED_PALETTE;
#if defined(__WGL_USE_DIRECTDRAW)
	/*
	** DirectDraw framebuffer support code doesn't support palette mapping.
	*/
	ppfd->dwFlags |= PFD_NEED_SYSTEM_PALETTE;
#endif
    } else {
	ppfd->dwFlags &= ~(PFD_NEED_PALETTE | PFD_NEED_SYSTEM_PALETTE);
    }

    /*
    ** 16 bit display modes can be either 555 or 565
    ** 32 bit display modes can be either 888 or 8888
    */
    if (ppfd->cColorBits == 16 || ppfd->cColorBits == 32) {
	int rSize, rShift, gSize, gShift, bSize, bShift;

	__wglDecodeColorMask(rMask, &rSize, &rShift);
	__wglDecodeColorMask(gMask, &gSize, &gShift);
	__wglDecodeColorMask(bMask, &bSize, &bShift);

	ppfd->cRedBits = rSize;
	ppfd->cRedShift = rShift;
	ppfd->cGreenBits = gSize;
	ppfd->cGreenShift = gShift;
	ppfd->cBlueBits = bSize;
	ppfd->cBlueShift = bShift;
	ppfd->cAlphaBits = 0;
	ppfd->cAlphaShift = 0;
    }

    /*
    ** Can only DRAW_TO_WINDOW for color index mode in 8-bit
    */
    if (displayDepth > 8 && ppfd->iPixelType == PFD_TYPE_COLORINDEX) {
	ppfd->dwFlags &= ~PFD_DRAW_TO_WINDOW;
	if ((ppfd->dwFlags & (PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP)) == 0) {
	    return FALSE;
	}
    }

    return TRUE;
}

void
__wglUpdatePixelFormats(PIXELFORMATDESCRIPTOR rawPixelFormats[], 
			int numRawPixelFormats,
			PIXELFORMATDESCRIPTOR **pixelFormats, 
			int *numPxlFormats
			)
{
    int displayDepth, rMask, gMask, bMask;
    int i;
    int numPixelFormats;

    numPixelFormats = 0;

    displayDepth = (*__glDevice->devGetDisplayMasks)(&rMask, &gMask, &bMask);

    /* first collect pixel formats which match the current display mode */
    for (i=0; i<numRawPixelFormats; ++i) {
	PIXELFORMATDESCRIPTOR *pRaw = &rawPixelFormats[i];

        if (pRaw->cColorBits == displayDepth) {
	    if (__wglPixelFormatSupported(pRaw, displayDepth, rMask, gMask, bMask)) {
		pixelFormats[numPixelFormats++] = pRaw;
	    }
	}
    }

    /* then collect remaining pixel formats */
    for (i=0; i<numRawPixelFormats; ++i) {
	PIXELFORMATDESCRIPTOR *pRaw = &rawPixelFormats[i];

        if (pRaw->cColorBits != displayDepth) {

	    if (pRaw->iPixelType == PFD_TYPE_RGBA) {
		/* Set masks for DIB rendering */
		switch (pRaw->cColorBits) {
		case 8: /* Default to RGB332 */
		    rMask = (7<<5);
		    gMask = (7<<2);
		    bMask = (3<<0);
		    break;
		case 16: /* Default to RGB565 */
		    rMask = (31<<11);
		    gMask = (63<<5);
		    bMask = (31<<0);
		    break;
		case 24: /* Default to RGB8 */
		    rMask = (255<<16);
		    gMask = (255<<8);
		    bMask = (255<<0);
		    break;
		case 32: /* Default to XRGB8 */
		    rMask = (255<<16);
		    gMask = (255<<8);
		    bMask = (255<<0);
		    break;
		default:
		    rMask = gMask = bMask = 0;
		    break;
		}
	    } else {
		rMask = gMask = bMask = 0;
	    }

	    if (__wglPixelFormatSupported(pRaw, displayDepth, rMask, gMask, bMask)) {
		pixelFormats[numPixelFormats++] = pRaw;
	    }
	}
    }

    *numPxlFormats = numPixelFormats;
}


/*
** wgl specific pixel format routines.  To be overriden by the specific
** implementation
*/

void
__wglInvalidatePixelFormatList(void)
{
    __wglLockMutex();

    if (wglPixelFormats) {
	__wglFree(wglPixelFormats);
	wglPixelFormats = NULL;
    }

    wglPixelFormatsDirty = TRUE;

    __wglUnlockMutex();
}

int
__wglGetPixelFormat(PIXELFORMATDESCRIPTOR *ppfd, int iPixelFormat)
{
    __wglLockMutex();

    if (wglPixelFormatsDirty) {
	/* allocate mem for pixel formats */
	wglPixelFormats = (PIXELFORMATDESCRIPTOR **)
	    __wglMalloc(sizeof(PIXELFORMATDESCRIPTOR *) * NUM_WGL_RAW_PIXELFORMATS);
	if (wglPixelFormats == NULL) return 0;

	__wglUpdatePixelFormats(wglRawPixelFormats, NUM_WGL_RAW_PIXELFORMATS,
				wglPixelFormats, &wglNumPixelFormats);
	wglPixelFormatsDirty = FALSE;
    }

    __wglUnlockMutex();

    if ((0 > iPixelFormat) || (iPixelFormat >= wglNumPixelFormats)) {
	if (ppfd == NULL) {
	    return wglNumPixelFormats;
	} else {
	    return 0;
	}
    }

    *ppfd = *wglPixelFormats[iPixelFormat];

    return wglNumPixelFormats;
}


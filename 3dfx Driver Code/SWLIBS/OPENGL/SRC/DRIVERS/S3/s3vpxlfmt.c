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
** S3 specific pixel formats.
*/

#define NUM_S3_RAW_PIXELFORMATS \
		(sizeof(rawS3PixelFormats) / sizeof(PIXELFORMATDESCRIPTOR))
static PIXELFORMATDESCRIPTOR rawS3PixelFormats[] = {
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
static int S3NumPixelFormats;
static PIXELFORMATDESCRIPTOR **S3PixelFormats;
static int S3PixelFormatsDirty = TRUE;



/*
** S3 specific pixel format routines. 
*/

void
__wglS3InvalidatePixelFormatList(void)
{
    __wglLockMutex();

    if (S3PixelFormats) {
	__wglFree(S3PixelFormats);
	S3PixelFormats = NULL;
    }

    S3PixelFormatsDirty = TRUE;

    __wglUnlockMutex();
}

int
__wglS3GetPixelFormat(PIXELFORMATDESCRIPTOR *ppfd, int iPixelFormat)
{
    __wglLockMutex();

    if (S3PixelFormatsDirty) {
	/* allocate mem for pixel formats */
	S3PixelFormats = (PIXELFORMATDESCRIPTOR **)
	    __wglMalloc(sizeof(PIXELFORMATDESCRIPTOR *) * NUM_S3_RAW_PIXELFORMATS);
	if (S3PixelFormats == NULL) return 0;

	__wglUpdatePixelFormats(rawS3PixelFormats, NUM_S3_RAW_PIXELFORMATS,
				S3PixelFormats, &S3NumPixelFormats);
	S3PixelFormatsDirty = FALSE;
    }

    __wglUnlockMutex();

    if ((0 > iPixelFormat) || (iPixelFormat >= S3NumPixelFormats)) {
	if (ppfd == NULL) {
	    return S3NumPixelFormats;
	} else {
	    return 0;
	}
    }

    *ppfd = *S3PixelFormats[iPixelFormat];

    return S3NumPixelFormats;
}


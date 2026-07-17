/*
** Copyright 1992-1997 Silicon Graphics, Inc.
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
/************************************************************************
 *                                                                      *
 *                         GLINT                                        *
 *                                                                      *
 *                                                                      *
 *          Copyright (C) 1994	3Dlabs Inc. Ltd                         *
 *                                                                      *
 *                                                                      *
 * This software and its associated documentation contains proprietary, *
 * confidential and trade secret information of 3Dlabs Ltd and except   *
 * as provided by written agreement with 3Dlabs Ltd.                    *
 *                                                                      *
 * a) no part may be disclosed, distributed, reproduced, transmitted,   *
 *    transcribed, stored in a retieval system, adapted or translated   *
 *    in any form or by any means electronic, mechanical, magnetic,     *
 *    optical, chemical, manual or otherwise,                           *
 *                                                                      *
 *    and                                                               *
 *                                                                      *
 * b) the recipient is not entitled to discover through reverse         *
 *    engineering or reverse compiling or other such techniques or      *
 *    processes the trade secrets contained therein or in the           *
 *    documentation.                                                    *
 *                                                                      *
 ************************************************************************/

#include "wgllib.h"

BOOL WINAPI wglUseFontBitmapsA(HDC hDC, DWORD first, DWORD count,
			       DWORD listBase);

/*****************************************************************************
**
** InvertGlypBitmap.
**
** Invert the bitmap so that it suits OpenGL's representation.
** Each row starts on a double word boundary.
**
*****************************************************************************/

static void
InvertGlyphBitmap(int w, int h, DWORD *fptr, DWORD *tptr)
{
    int dWordsInRow = (w+31)/32;
    int i, j;
    DWORD *tmp = tptr;

    if (w <= 0 || h <= 0) {
	return;
    }

    tptr += ((h-1)*dWordsInRow);
    for (i = 0; i < h; i++) {
	for (j = 0; j < dWordsInRow; j++) {
	    *(tptr + j) = *(fptr + j);
	}
	tptr -= dWordsInRow;
	fptr += dWordsInRow;
    }
}

/****************************************************************************
**
** wglUseFontBitmapsA
**
** This routine extracts glyph information from the currently selected font
** and transcribes it into gl bitmap data.   The bitmaps are stored in display
** lists and can be rendered using glCallLists.
**
** It seems that the bitmap data supplied by the GetGlyphOutline routine needs
** to be inverted for OpenGL, as does the glyph origin.  Note that this works 
** on True-Type fonts.
**
*****************************************************************************/

#define EXTRA_SPACE		128

WINGDIAPI BOOL APIENTRY
wglUseFontBitmapsW(
    HDC hDC,			/* Current device context. */
    DWORD first,		/* The first list/character index. */
    DWORD count,		/* Number of Glyphs to do the conversion for. */
    DWORD listBase)		/* Base number of the display lists to use. */
{
    return wglUseFontBitmapsA (hDC, first, count, listBase);
}

WINGDIAPI BOOL APIENTRY
wglUseFontBitmapsA(
    HDC hDC,			/* Current device context. */
    DWORD first,		/* The first list/character index. */
    DWORD count,		/* Number of Glyphs to do the conversion for. */
    DWORD listBase)		/* Base number of the display lists to use. */
{
    int i, ox, oy, ix, iy; 
    DWORD *bitmapBuffer = NULL;
    DWORD *invertedBitmapBuffer = NULL;
    GLYPHMETRICS lpgm;
    int w, h;
    RASTERIZER_STATUS rasStat;
    MAT2 idMat;
    DWORD bSize;
    DWORD currentBsize = 0;
    BOOL successOrFail = TRUE;

    idMat.eM11.value = 1; 	/* Set up a unity matrix. */
    idMat.eM11.fract = 0;
    idMat.eM12.value = 0;
    idMat.eM12.fract = 0;
    idMat.eM21.value = 0;
    idMat.eM21.fract = 0;
    idMat.eM22.value = 1;
    idMat.eM22.fract = 0;
	     
    /* Test to see if TRUE-TYPE capabilities are installed */

    if (!GetRasterizerCaps (&rasStat, sizeof (RASTERIZER_STATUS))) {
	__wglSetSystemError("wglUseFontBitmaps", WGL_NO_TT_CAPS);
	return (FALSE);
    }

    for (i = first; (DWORD) i < (first + count); i++) {
	/* Create an OpenGL display list. */
	glNewList((listBase + i), GL_COMPILE);

	/* Find out how much space is needed for the bitmap so we can */
	/* Set the buffer size correctly. */

	bSize = GetGlyphOutline(hDC, i, GGO_BITMAP, &lpgm,
					0, bitmapBuffer, &idMat);

	/* If we need to allocate Larger Buffers, then do so - but allocate */
	/* An extra 50 % so that we don't do too many mallocs ! */

	if (bSize > currentBsize) {
	    if (bitmapBuffer) {
		__wglFree(bitmapBuffer);
	    }
	    if (invertedBitmapBuffer) {
		__wglFree(invertedBitmapBuffer);
	    }

	    currentBsize = bSize + EXTRA_SPACE;
	    bitmapBuffer = (DWORD *) __wglMalloc(currentBsize);
	    invertedBitmapBuffer = (DWORD *) __wglMalloc(currentBsize);

	    if (bitmapBuffer == NULL || invertedBitmapBuffer == NULL) {
		glEndList();
		successOrFail = FALSE;
		break;
	    }
	}	 

	/* If we fail to get the Glyph data, delete the display lists */
	/* Created so far and return FALSE. */

	if (GetGlyphOutline(hDC, i, GGO_BITMAP, &lpgm,
				bSize, bitmapBuffer, &idMat) == -1)
	{
	    glEndList();
	    successOrFail = FALSE;
	    break;
	}

	w  = lpgm.gmBlackBoxX;
	h  = lpgm.gmBlackBoxY;
	ox = lpgm.gmptGlyphOrigin.x;
	oy = lpgm.gmptGlyphOrigin.y;
	ix = lpgm.gmCellIncX;
	iy = lpgm.gmCellIncY;

	/* Some fonts have no data for the space character, yet advertise
	 * a non-zero size.
	 */
	if (0 == bSize) {
	    glBitmap(0, 0, 0.0f, 0.0f, (GLfloat) ix, (GLfloat) iy, NULL);
	} else {
	    /* Invert the Glyph data. */
	    InvertGlyphBitmap(w, h, bitmapBuffer, invertedBitmapBuffer);

	    /* Render an OpenGL bitmap and invert the origin. */
	    glBitmap(w, h,
		     (GLfloat) ox, (GLfloat) (h-oy),
		     (GLfloat) ix, (GLfloat) iy,
		     (GLubyte *) invertedBitmapBuffer);
	}
	 
	/* Close this display list. */
	glEndList();
    }
    if (successOrFail == FALSE) {
	__wglSetSystemError("wglUseFontBitmaps", WGL_GET_GLYPH_FAILED); 
	glDeleteLists((i+listBase), (i-first));
    }
    if (bitmapBuffer) {
	__wglFree(bitmapBuffer);
    }
    if (invertedBitmapBuffer) {
	__wglFree(invertedBitmapBuffer);
    }
    return(successOrFail);
}


/****************************************************************************/

#ifndef __WGLCMDS_H
#define __WGLCMDS_H

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


HGLRC WINAPI oglCreateContext(HDC hDC);

BOOL WINAPI oglDeleteContext(HGLRC hGLRC);

HGLRC WINAPI oglGetCurrentContext(void);

HDC WINAPI oglGetCurrentDC(void);

BOOL WINAPI oglMakeCurrent(HDC hDC, HGLRC hGLRC);

BOOL WINAPI oglShareLists(HGLRC hGLRC1, HGLRC hGLRC2);

BOOL WINAPI oglSwapBuffers(HDC hDC);

BOOL WINAPI oglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask);

int WINAPI oglDescribePixelFormat(HDC hDC, int iPixelFormat,
				  UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);

int WINAPI oglChoosePixelFormat(HDC hDC, const PIXELFORMATDESCRIPTOR *ppfd);

int WINAPI oglGetPixelFormat(HDC hDC);

BOOL WINAPI oglSetPixelFormat(HDC hDC, int iPixelFormat,
			      const PIXELFORMATDESCRIPTOR *ppfd);

PROC WINAPI oglGetProcAddress(LPCSTR lpszProc);

PROC WINAPI oglGetDefaultProcAddress(LPCSTR lpszProc);

HGLRC WINAPI oglCreateLayerContext(HDC hDC, int iLayerPlane);

BOOL WINAPI oglDescribeLayerPlane(HDC hDC, int iPixelFormat, int iLayerPlane,
				  UINT nBytes, void *plpd);

int WINAPI oglGetLayerPaletteEntries(HDC hDC, int iLayerPlane, int iStart,
				     int cEntries, const COLORREF *par);

BOOL WINAPI oglRealizeLayerPalette(HDC hDC, int iLayerPlane, BOOL bRealize);

int WINAPI oglSetLayerPaletteEntries(HDC hDC, int iLayerPlane, int iStart,
				     int cEntries, const COLORREF *par);

BOOL WINAPI oglSwapLayerBuffers(HDC hDC, UINT fuPlanes);

BOOL WINAPI oglUseFontBitmapsW(HDC hDC, DWORD first, DWORD count,
			       DWORD listBase);

BOOL WINAPI oglUseFontBitmapsA(HDC hDC, DWORD first, DWORD count,
			       DWORD listBase);

BOOL WINAPI oglUseFontOutlinesW(HDC hDC, DWORD first, DWORD count,
				DWORD listBase, FLOAT chordalDeviation,
				FLOAT extrusion, INT format,
				LPGLYPHMETRICSFLOAT lpgmf);

BOOL WINAPI oglUseFontOutlinesA(HDC hDC, DWORD first, DWORD count,
				DWORD listBase, FLOAT chordalDeviation, FLOAT
				extrusion, INT format, LPGLYPHMETRICSFLOAT
				glyphMetricsFloatArray);

#endif /* __WGLCMDS_H */

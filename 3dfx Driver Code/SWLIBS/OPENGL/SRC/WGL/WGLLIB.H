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

#ifndef	__wgllib_h_
#define	__wgllib_h_

#include <stdio.h>
#include <assert.h>
#include "windows.h"
#include "GL/gl.h"
#include "wgl.h"
#include "wglhrc.h"
#include "wglerror.h"
#ifdef __GL_COSMO_BUILD
#include "oglwgl.h"
#endif

#ifdef DEBUG
#define WGL_DEBUG_MESSAGE(str)			__wglMessage(str)
#define WGL_DEBUG_MESSAGE_ARG(str, arg)		__wglMessageArg (str, arg)
#define WGL_DEBUG_MESSAGE_FLOAT(str, arg)	__wglMessageFloat (str, arg)
#else
#define WGL_DEBUG_MESSAGE(str)
#define WGL_DEBUG_MESSAGE_ARG(str, arg)
#define WGL_DEBUG_MESSAGE_FLOAT(str, arg)
#endif	

/*****************************************************************************
**
** Routine declarations.
**
*****************************************************************************/

/* wglcomm.c */
void *__wglMalloc(size_t size);
void *__wglCalloc(size_t numElements, size_t elementSize);
void *__wglRealloc(void *oldPtr, size_t newSize);
void __wglFree(void *ptr);

void __wglMessage(const char *str);
void __wglMessageArg(const char *str, long arg);
void __wglMessageArg2(const char *str, long arg1, long arg2);
void __wglMessageFloat(const char *str, float arg);

void __wglError(const char *str);
void __wglSetSystemError(const char *str, long errorNumber);

BOOL __wglLockMutex(void);
BOOL __wglUnlockMutex(void);

void __wglEnterCriticalSection(void);
void __wglLeaveCriticalSection(void);

void __wglAddContext(__WGLcontext *glrc);
void __wglRemoveContext(__WGLcontext *glrc);
BOOL __wglIsStartList(__WGLcontext *glrc);
void __wglInitialiseContextList(void);
__WGLcontext *__wglFindWGLContext(HGLRC hRC);
__WGLcontext *__wglFindWGLWindow(HWND win, __WGLcontext *hrcPtr);

void __wglUpdateDrawableSize(__WGLdrawablePrivate *wglPriv);
GLboolean __wglUpdateDrawableBuffers(__WGLcontext *glrc);
__WGLdrawablePrivate *__wglGetDrawablePrivate(HDC hDC, HWND hWnd, 
					      __GLcontextModes *modes);

BOOL __wglAttachThread(DWORD hThread);
BOOL __wglDetachThread(HINSTANCE hInst, DWORD hThread);

GLvoid __wglFormatGLModes(__GLcontextModes *modes, LPPIXELFORMATDESCRIPTOR pPFD);
GLboolean __wglCreateHWContext(__WGLcontext *glrc);

void __wglAddDrawable(__WGLdrawablePrivate *wglPriv);
void __wglRemoveDrawable(__WGLdrawablePrivate *wglPriv);
__WGLdrawablePrivate *__wglCreateDrawablePrivate(HDC hDC, HWND hWnd,
						 __GLcontextModes *modes);
void __wglDestroyDrawablePrivate(__WGLdrawablePrivate *wglPriv);
__WGLdrawablePrivate *__wglFindDrawablePrivate(HDC hDC, HWND hWnd);
void __wglDestroyDrawable(HWND hWnd);

PROC __wglGetProcAddress(LPCSTR lpszProc);

/* wgldebug.c */
void __wglDDrawError(const char *str, HRESULT ddrval);

/* wglbuf.c */
void __wglDDrawInitDrawable(__WGLdrawablePrivate *wglPriv, __GLcontextModes *modes);
void __wglDCIInitDrawable(__WGLdrawablePrivate *wglPriv, __GLcontextModes *modes);
void __wglDIBInitDrawable(__WGLdrawablePrivate *wglPriv, __GLcontextModes *modes);
void __wglFreeBuffers(__WGLdrawablePrivate *wglPriv);

/* wglpxlfmt.c */

/* utils */
void __wglDecodeColorMask(DWORD mask, int *size, int *shift);
BOOL __wglPixelFormatSupported(PIXELFORMATDESCRIPTOR *ppfd, int displayDepth, 
			       int rMask, int gMask, int bMask);
void __wglUpdatePixelFormats(PIXELFORMATDESCRIPTOR rawPixelFormats[],
			     int numRawPixelFormats,
			     PIXELFORMATDESCRIPTOR **pixelFormats,
			     int *numPixelFormats);
/* hooks */
void __wglInvalidatePixelFormatList(void);
int __wglGetPixelFormat(struct tagPIXELFORMATDESCRIPTOR *pfd, int iPixelFormat);


#endif /* __wgllib_h_ */

/*****************************************************************************/

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
/************************************************************************
 *                                                                      *
 *                         GLINT                                        *
 *                                                                      *
 *                                                                      *
 *          Copyright (C) 1994	3Dlabs Inc. Ltd                         *
 *                                                                      *
 *                                                                      *
 * This software and its associated documentation contains proprietary, *
 * confidential and trade secret information of 3Dlabs Lts and except   *
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

void __wglAddContext(WGLCONTEXT *glrc);
void __wglRemoveContext(WGLCONTEXT *glrc);
BOOL __wglIsStartList(WGLCONTEXT *glrc);
void __wglInitialiseContextList(void);
WGLCONTEXT *__wglFindWGLContext(HGLRC hRC);
WGLCONTEXT *__wglFindWGLWindow(HWND win, WGLCONTEXT *hrcPtr);

void __wglUpdateDrawableSize(__WGLdrawablePrivate *wglPriv);
void __wglUpdateDrawableBuffers(WGLCONTEXT *glrc);

BOOL __wglAttachThread(DWORD hThread);
BOOL __wglDetachThread(HINSTANCE hInst, DWORD hThread);

__WGLdrawablePrivate *_wglGetDrawablePrivate(HWND, __GLcontextModes *modes);

/* wglfb.c */
BOOL __wglFBInit(void);
BOOL __wglFBFinish(void);
int __wglFBGetDisplayMasks(int *rmask, int *gmask, int *bmask);
void __wglFBInitDrawable(__WGLdrawablePrivate *wglPriv);

/* wgldib.c */
void __wglDIBInitDrawable(__WGLdrawablePrivate *wglPriv);

/* wgldebug.c */
void __wglDDrawError(const char *str, HRESULT ddrval);

/* wglddraw.c */
BOOL __wglDDrawInit(void);
BOOL __wglDDrawFinish(void);
int __wglDDrawGetDisplayMasks(int *rMask, int *gMask, int *bMask);
void __wglDDrawUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglDDrawFreeBuffer(__GLdrawableBuffer *buf, __WGLdrawablePrivate *wglPriv);
void __wglDDrawLockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglDDrawUnlockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglDDrawUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv);
BOOL __wglDDrawSwapBuffers(__WGLdrawablePrivate *wglPriv);

/* wgldci.c */
BOOL __wglDCIInit(void);
BOOL __wglDCIFinish(void);
int __wglDCIGetDisplayMasks(int *rMask, int *gMask, int *bMask);
void __wglDCIUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglDCIFreeBuffer(__GLdrawableBuffer *buf, __WGLdrawablePrivate *wglPriv);
void __wglDCILockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglDCIUnlockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);

/* wglglide.c */
BOOL __wglGlideInit(void);
BOOL __wglGlideFinish(void);
BOOL __wglGlideSwapBuffers(__WGLdrawablePrivate *wglPriv);
int  __wglGlideGetDisplayMasks(int *rMask, int *gMask, int *bMask);
void __wglGlideUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglGlideFreeBuffer(__GLdrawableBuffer *buf, __WGLdrawablePrivate *wglPriv);
void __wglGlideLockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglGlideUnlockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);

/* wglmem.c */
void __wglMemUpdateBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglMemFreeBuffer(__GLdrawableBuffer *buf, __WGLdrawablePrivate *wglPriv);
void __wglMemLockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglMemUnlockBuffer(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv);
void __wglMemUpdateOwnershipBuffer(__WGLdrawablePrivate *wglPriv, LPRGNDATA rgnData);
void __wglMemUpdateDrawablePalette(__WGLdrawablePrivate *wglPriv);
BOOL __wglMemSwapBuffers(__WGLdrawablePrivate *wglPriv);

#endif /* __wgllib_h_ */

/*****************************************************************************/

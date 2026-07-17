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


/*
** Common routines between the standalone and the icd version
*/


#include "wgllib.h"
#include "g_xproto.h"
#include "gldevice.h"

#include "wglext.h"

HINSTANCE hInstanceOpenGL;
BOOL	__wglOSWin95;

/* TLS index handles */
DWORD __wglTLSCXIndex = TLS_MINIMUM_AVAILABLE;
DWORD __wglTLSCXOffset = 0;
DWORD __wglTLSIndex = TLS_MINIMUM_AVAILABLE;
DWORD __wglTLSOffset = 0;

__GLcontext *__wglInvalidGC;

static BOOL shareProcessContext;
static __GLcontext *processContext;

/*
** per-thread info
*/
typedef struct __GLthreadAreaRec {
    HHOOK messageHook;
} __GLthreadArea;


extern struct __glDeviceStruct *__glDevice;

/************************************************************/
/* Memory Allocation for WGL */

void *
__wglMalloc(size_t size)
{
    void *ptr;

    if (size == 0) {
	return NULL;
    }
    ptr = (void *) GlobalAlloc(GPTR, size);
#ifdef DEBUG
    __wglMessageArg2("__wglMalloc", (long) ptr, size);
#endif
    if (ptr == NULL) {
	return NULL;	/* XXX out of memory error */
    }
    return ptr;
}

void *
__wglCalloc(size_t numElements, size_t elementSize)
{
    void *ptr;

    if (numElements == 0 || elementSize == 0) {
	return NULL;
    }
    ptr = (void *) GlobalAlloc(GPTR, numElements * elementSize);
#ifdef DEBUG
    __wglMessageArg2("__wglCalloc", (long) ptr, numElements * elementSize);
#endif
    if (ptr == NULL) {
	return NULL;	/* XXX out of memory error */
    }
    return ptr;
}

void *
__wglRealloc(void *oldPtr, size_t newSize)
{
    void *newPtr = NULL;

    if (newSize != 0) {
	newPtr = (void *) GlobalAlloc(GPTR, newSize);
	if (oldPtr && newPtr) {
	    DWORD oldSize = GlobalSize(oldPtr);

	    memcpy(newPtr, oldPtr, (oldSize <= newSize ? oldSize : newSize));
	    GlobalFree(oldPtr);
	}
    } else if (oldPtr) {
	GlobalFree(oldPtr);
    }
    if (newPtr == NULL) {
	return NULL;	/* XXX out of memory error */
    }
    return newPtr;
}

void
__wglFree(void *ptr)
{
    if (ptr) {
	int size;

	size = GlobalSize(ptr);
#ifdef DEBUG
	__wglMessageArg2("__wglFree", (long) ptr, size);
#endif
	GlobalFree(ptr);
    }
}

/************************************************************/
/* Imports Stuff (functions imported by the core rendering code) */

void *
__wglImpMalloc(__GLcontext *gc, size_t size)
{
    void *ptr;

    if (size == 0) {
	return NULL;
    }
    ptr = (void *) GlobalAlloc(GPTR, size);
    if (ptr == NULL) {
	return NULL;	/* XXX out of memory error */
    }
    return ptr;
}

void *
__wglImpCalloc(__GLcontext *gc, size_t numElements, size_t elementSize)
{
    void *ptr;

    if (numElements == 0 || elementSize == 0) {
	return NULL;
    }
    ptr = (void *) GlobalAlloc(GPTR, numElements * elementSize);
    if (ptr == NULL) {
	return NULL;	/* XXX out of memory error */
    }
    return ptr;
}

void *
__wglImpRealloc(__GLcontext *gc, void *oldPtr, size_t newSize)
{
    void *newPtr = NULL;

    if (newSize != 0) {
	newPtr = (void *) GlobalAlloc(GPTR, newSize);
	if (oldPtr && newPtr) {
	    DWORD oldSize = GlobalSize(oldPtr);

	    memcpy(newPtr, oldPtr, (oldSize <= newSize ? oldSize : newSize));
	    GlobalFree(oldPtr);
	}
    } else if (oldPtr) {
	GlobalFree(oldPtr);
    }
    if (newPtr == NULL) {
	return NULL;	/* XXX out of memory error */
    }
    return newPtr;
}

void
__wglImpFree(__GLcontext *gc, void *ptr)
{
    if (ptr) {
	GlobalFree(ptr);
    }
}

void
__wglImpWarning(__GLcontext *gc, const char* msg, ...)
{
    __wglError((char *) msg);
}

void
__wglImpFatal(__GLcontext *gc, const char* msg, ...)
{
    __wglError((char *) msg);
    abort();
}

__GLdrawablePrivate *
__wglImpGetDrawablePrivate(__GLcontext *gc)
{
    __WGLcontext *glrc = (__WGLcontext *) gc->imports.other;

    return &glrc->wglPriv->glPriv;
}

void
__wglImpGetDrawableSize(__GLcontext *gc, int *w, int *h)
{
    __WGLcontext *glrc = (__WGLcontext *) gc->imports.other;
    __WGLdrawablePrivate *wglPriv = glrc->wglPriv;

    __wglUpdateDrawableSize(wglPriv);

    *w = (int) wglPriv->width;
    *h = (int) wglPriv->height;
}

PROC
__wglGetProcAddress(LPCSTR lpszProc)
{
    CONST CHAR *pch1, *pch2;
    int   i;

    /* Return error if there is no current RC. */
    if (__wglGetCurrentGC() == __wglInvalidGC)  {
	__wglSetSystemError("wglGetProcAddress", WGL_INVALID_HRC);
        return (PROC) NULL;
    }

    /* Return extension function address if it is found. */
    for (i=0; i<__WGL_EXT_TABLE_SIZE; i++) {
        /* Compare names. */
        for (pch1 = lpszProc, pch2 = __wglExtProcs[i].szProc;
             *pch1 == *pch2 && *pch1;
             pch1++, pch2++)
            ;

        /* If found, return the address. */
        if (*pch1 == *pch2 && !*pch1) {
            return __wglExtProcs[i].Proc;
	}
    }
    return (PROC) NULL;
}

/************************************************************/

static __GLimports imports = {
    __wglImpMalloc,
    __wglImpCalloc,
    __wglImpRealloc,
    __wglImpFree,
    __wglImpWarning,
    __wglImpFatal,
    __wglImpGetDrawablePrivate,
    __wglImpGetDrawableSize,
    NULL,
};

/************************************************************/
/* Error Logging */

#include <io.h>
#include <fcntl.h>
#include <sys/types.h>


#define OPENGL_ERROR_LOG_NAME \
			"OGLError.txt"

#define OPENGL_ERROR_LOG \
			"ErrorLog"


static BOOL errorLog;
static HANDLE msgFile = INVALID_HANDLE_VALUE;

static void
__wglInitErrorLog(void)
{
    char *env = getenv("__GL_ERROR_LOG");

    /* Environment has precedence over registry */
    if (env) {
	errorLog = atoi(env);
    } else {
	DWORD queryRegistry(LPSTR valueName);

	errorLog = queryRegistry(OPENGL_ERROR_LOG);
    }
}

static void
__wglOpenErrorFile(void)
{
    if (!errorLog) {
	return;
    }

    if (msgFile == INVALID_HANDLE_VALUE) {
	SECURITY_ATTRIBUTES eSecurity;

	eSecurity.nLength = sizeof(SECURITY_ATTRIBUTES);
	eSecurity.lpSecurityDescriptor = NULL;
	eSecurity.bInheritHandle = FALSE;
	msgFile = CreateFile(OPENGL_ERROR_LOG_NAME,
			GENERIC_WRITE, 0, &eSecurity,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);  
    }
}

void
__wglMessage(const char *str)
{
    char sBuffer[256];
    ULONG aWritten;

    sprintf(sBuffer, "WGL Message:%s\n", str);
    OutputDebugString(sBuffer);

    __wglOpenErrorFile();
    if (msgFile != INVALID_HANDLE_VALUE) {
	WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    }
}

void
__wglMessageArg(const char *str, long arg)
{
    char sBuffer[256];
    ULONG aWritten;

    sprintf(sBuffer, "WGL Message:%s: %d (%08x)\n", str, arg, arg);
    OutputDebugString(sBuffer);

    __wglOpenErrorFile();
    if (msgFile != INVALID_HANDLE_VALUE) {
	WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    } 
}

void
__wglMessageArg2(const char *str, long arg1, long arg2)
{
    char sBuffer[256];
    ULONG aWritten;

    sprintf(sBuffer, "WGL Message:%s: %5d (%08x)\n", str, arg1, arg2);
    OutputDebugString(sBuffer);

    __wglOpenErrorFile();
    if (msgFile != INVALID_HANDLE_VALUE) {
	WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    } 
}

void
__wglMessageFloat(const char *str, float arg)
{
    char sBuffer[256];
    ULONG aWritten;

    sprintf(sBuffer, "WGL Message:%s: %f (%08x)\n", str, arg, *((DWORD *) &arg));
    OutputDebugString(sBuffer);

    __wglOpenErrorFile();
    if (msgFile != INVALID_HANDLE_VALUE) {
	WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    } 
}

void
__wglError(const char *str)
{
    char sBuffer[256];
    ULONG aWritten;

    sprintf(sBuffer, "OGL ERR:%s\n", str);
    OutputDebugString(sBuffer);

    __wglOpenErrorFile();
    if (msgFile != INVALID_HANDLE_VALUE) {
	WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    } 
} 

void
__wglSetSystemError(const char *str, long errorNumber)
{
    char sysError[256];

    sprintf(sysError, "System Error: (%d):%s", errorNumber, str);
    __wglMessage(sysError);
    SetLastError(errorNumber | WGL_OGL_ID | WGL_ERROR | WGL_APP_ERROR);
}

/*****************************************************************************/
/* synchronization stuff */

static HANDLE __wglMutex;
	
HANDLE
__wglCreateMutex(void)
{
    __wglMutex = CreateMutex(NULL, FALSE, NULL);
    return (__wglMutex);
}

void
__wglDestroyMutex(void)
{
    CloseHandle(__wglMutex);
    __wglMutex = (HANDLE) NULL;
}

BOOL
__wglLockMutex(void)
{
    return (WaitForSingleObject(__wglMutex, INFINITE) != WAIT_FAILED);
}

BOOL
__wglUnlockMutex(void)
{
    return ReleaseMutex(__wglMutex);
}  

static CRITICAL_SECTION __wglCriticalSection;
	
void
__wglCreateCriticalSection(void)
{
    InitializeCriticalSection(&__wglCriticalSection);
}

void
__wglDestroyCriticalSection(void)
{
    DeleteCriticalSection(&__wglCriticalSection);
}

void
__wglEnterCriticalSection(void)
{
    EnterCriticalSection(&__wglCriticalSection);
}

void
__wglLeaveCriticalSection(void)
{
    LeaveCriticalSection(&__wglCriticalSection);
}  

/************************************************************/
/* Context List Stuff */

static __WGLcontext glrcListEnd;

void
__wglInitializeContextList(void)
{
    __GLcontextModes modes;	/* dummy modes */

    __wglLockMutex();

    __wglInvalidGC = __glCoreCreateContext(&imports, &modes);
    __wglInvalidGC->imports.other = (void *) &glrcListEnd;
    glrcListEnd.gc = __wglInvalidGC;

    __wglUnlockMutex();
}

void
__wglAddContext(__WGLcontext *glrc)
{
    __WGLcontext *glrcList = &glrcListEnd;

    /* Add the structure to the list. */
    glrc->next = glrcList->next;
    glrc->last = glrcList;
    if (glrcList->next) {
	glrcList->next->last = glrc;
    }
    glrcList->next = glrc;
}

void
__wglRemoveContext(__WGLcontext *glrc)
{
    /* Remove the structure from the list. */	
    glrc->last->next = glrc->next;
    if (glrc->next) {
	glrc->next->last = glrc->last;
    }
}

/*
** Find a context on the list which matches the specified context handle
**
** Returns:
**   context  if hGLRC is found on the list
**   dummy    if hGLRC is NULL
**   NULL     otherwise
*/
__WGLcontext *
__wglFindWGLContext(DHGLRC hGLRC)
{
    __WGLcontext *glrc = &glrcListEnd;

    __wglLockMutex();
    while (glrc != NULL) {
	if (glrc->hGLRC == hGLRC) {
	    __wglUnlockMutex();
	    return glrc;
	}
	glrc = glrc->next;
    }
    __wglUnlockMutex();
    return NULL;
}

/*
** Find a context on the list which is bound to the specified window handle.
**
** If the argument glrc is NULL then the search starts from the head of
** the list, otherwise the search continues from the specified context.
** 
** Returns:
**   context  if found
**   NULL     otherwise
*/
__WGLcontext *
__wglFindWGLWindow(HWND hWnd, __WGLcontext *glrc)
{
    if (glrc == NULL) {
	glrc = &glrcListEnd;
    } else {
	glrc = glrc->next;
    }

    __wglLockMutex();
    while (glrc != NULL) {
	if (glrc->hWnd == hWnd) {
	    __wglUnlockMutex();
	    return glrc;
	}
	glrc = glrc->next;
    }
    __wglUnlockMutex();
    return NULL;
}

/************************************************************/
/* Drawable List Stuff */

static __WGLdrawablePrivate __wglDrawPrivListEnd;

void
__wglAddDrawable(__WGLdrawablePrivate *wglPriv)
{
    __WGLdrawablePrivate *wglPrivList = &__wglDrawPrivListEnd;

    /* Add the structure to the list. */
    __wglLockMutex();
    wglPriv->next = wglPrivList->next;
    wglPriv->last = wglPrivList;
    if (wglPrivList->next) {
	wglPrivList->next->last = wglPriv;
    }
    wglPrivList->next = wglPriv;
    __wglUnlockMutex();
}

void
__wglRemoveDrawable(__WGLdrawablePrivate *wglPriv)
{
    /* Remove the structure from the list. */	
    __wglLockMutex();
    wglPriv->last->next = wglPriv->next;
    if (wglPriv->next) {
	wglPriv->next->last = wglPriv->last;
    }
    __wglUnlockMutex();
}

/*******************************/
/*
 * wgl window related functions */

void
__wglAddSwapHintRect(__GLdrawablePrivate *glPriv, GLint x, GLint y,
		     GLsizei width, GLsizei height)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    LPRGNDATA rgnData;
    LPRECT lpRect;

    if (wglPriv->swapRgn->rdh.nCount == wglPriv->swapSize) {
	wglPriv->swapSize <<= 1;
	wglPriv->swapRgn = (LPRGNDATA)
	    __wglRealloc(wglPriv->swapRgn,
			 sizeof(RGNDATA) + wglPriv->swapSize * sizeof(RECT));
    }

    rgnData = wglPriv->swapRgn;

    lpRect = ((LPRECT) rgnData->Buffer) + rgnData->rdh.nCount;

    lpRect->left = x;
    lpRect->right = lpRect->left + width;
    lpRect->bottom = wglPriv->height - y;
    lpRect->top = lpRect->bottom - height;

    rgnData->rdh.nCount++;

    if (1 == rgnData->rdh.nCount) {
	rgnData->rdh.nRgnSize = sizeof(RECT);
	rgnData->rdh.rcBound = *lpRect;
    } else {
	rgnData->rdh.nRgnSize += sizeof(RECT);
	if (lpRect->left < rgnData->rdh.rcBound.left)
	    rgnData->rdh.rcBound.left = lpRect->left;
	if (lpRect->right > rgnData->rdh.rcBound.right)
	    rgnData->rdh.rcBound.right = lpRect->right;
	if (lpRect->top < rgnData->rdh.rcBound.top)
	    rgnData->rdh.rcBound.top = lpRect->top;
	if (lpRect->bottom > rgnData->rdh.rcBound.bottom)
	    rgnData->rdh.rcBound.bottom = lpRect->bottom;
    }
}

void
__wglSetCoreClipRect(__GLdrawablePrivate *glPriv,
			GLint x, GLint y, GLsizei w, GLsizei h)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;

    wglPriv->coreClipRect.left = x;
    wglPriv->coreClipRect.top = y;
    wglPriv->coreClipRect.right = x+w;
    wglPriv->coreClipRect.bottom = y+h;
    wglPriv->coreClipRectChanged = TRUE;
}

/************************************************************/
/* Drawable Stuff */

__WGLdrawablePrivate *
__wglCreateDrawablePrivate(HDC hDC, HWND hWnd, __GLcontextModes *modes)
{
    __WGLdrawablePrivate *wglPriv;
    __GLdrawablePrivate *glPriv;
    LPRGNDATA rgnData;

    wglPriv = (__WGLdrawablePrivate *) __wglMalloc(sizeof(*wglPriv));
    memset(wglPriv, 0, sizeof(*wglPriv));

    wglPriv->hDC = hDC;
    wglPriv->hWnd = hWnd;

    /* fill up glPriv */
    glPriv = &wglPriv->glPriv;
    glPriv->modes = (__GLcontextModes *) __wglMalloc(sizeof(__GLcontextModes));

    *glPriv->modes = *modes;
    glPriv->malloc = __wglMalloc;
    glPriv->calloc = __wglCalloc;
    glPriv->realloc = __wglRealloc;
    glPriv->free = __wglFree;
    glPriv->addSwapRect = __wglAddSwapHintRect;
    glPriv->setClipRect = __wglSetCoreClipRect;
    glPriv->other = wglPriv;

    /* data for swap hint rects */
    wglPriv->swapSize = 4;
    wglPriv->swapRgn = rgnData = (LPRGNDATA)
	__wglMalloc(sizeof(RGNDATA) + wglPriv->swapSize * sizeof(RECT));
    rgnData->rdh.dwSize = sizeof(rgnData->rdh);
    rgnData->rdh.iType = RDH_RECTANGLES;
    rgnData->rdh.nCount = 0;
    rgnData->rdh.nRgnSize = 0;
    rgnData->rdh.rcBound.left = rgnData->rdh.rcBound.right =
	rgnData->rdh.rcBound.top = rgnData->rdh.rcBound.bottom = 0;

    /* function pointers */
    wglPriv->freeBuffers = __glDevice->freeBuffers;
    wglPriv->updatePalette = __glDevice->updatePalette;

    /* allocate the buffers */
    if (hWnd) {
	wglPriv->glPriv.modes->pixmapMode = GL_FALSE;
	(*__glDevice->devFBInitDrawable)(wglPriv, modes);
    } else {
	wglPriv->glPriv.modes->pixmapMode = GL_TRUE;
	(*__glDevice->devDIBInitDrawable)(wglPriv, modes);
    }


    __wglAddDrawable(wglPriv);

    return wglPriv;
}

void
__wglDestroyDrawablePrivate(__WGLdrawablePrivate *wglPriv)
{
    __GLdrawablePrivate *glPriv = &wglPriv->glPriv;

    /* Take the drawable off the drawable list */
    __wglRemoveDrawable(wglPriv);

    /* Have the core free any memory it may have attached to the drawable */
    if (wglPriv->glPriv.freePrivate) {
	(*wglPriv->glPriv.freePrivate)(&wglPriv->glPriv);
    }

    /* Free any framebuffer memory attached to the drawable */
    if (wglPriv->freeBuffers) {
	(*wglPriv->freeBuffers)(wglPriv);
    }

    /* Free the drawable private */
    __wglFree(wglPriv->swapRgn);
    __wglFree(wglPriv->glPriv.modes);
    __wglFree(wglPriv);
}

__WGLdrawablePrivate *
__wglFindDrawablePrivate(HDC hDC, HWND hWnd)
{
    __WGLdrawablePrivate *wglPriv = &__wglDrawPrivListEnd;

    __wglLockMutex();
    while (wglPriv != NULL) {
	if (wglPriv->hWnd == NULL) {
	    /* if hWnd == NULL, we have a memDC */
	    if (wglPriv->hDC == hDC) {
		__wglUnlockMutex();
		return wglPriv;
	    }
	} else {
	    /* hWnd != NULL..  Same handle ==> Same window. John Chen said so */
	    if (wglPriv->hWnd == hWnd) {
		__wglUnlockMutex();
		return wglPriv;
	    }
	}
	wglPriv = wglPriv->next;
    }
    __wglUnlockMutex();
    return NULL;
}

void
__wglDestroyDrawable(HWND hWnd)
{
    __WGLdrawablePrivate *wglPriv = &__wglDrawPrivListEnd;

    __wglLockMutex();
    while (wglPriv != NULL) {
	__WGLdrawablePrivate *next = wglPriv->next;

	if (wglPriv->hWnd == hWnd) {
	    /* Clobber the window handle to mark the drawable as destroyed */
	    wglPriv->hWnd = NULL;
	    if (wglPriv->refCount == 0) {
		__wglDestroyDrawablePrivate(wglPriv);
	    }
	}

	wglPriv = next;
    }
    __wglUnlockMutex();
}

void
__wglUpdateDrawableSize(__WGLdrawablePrivate *wglPriv)
{
    HWND hWnd = wglPriv->hWnd;

    if (hWnd) {
	/* Find screen relative origin of client area */
	wglPriv->origin.x = wglPriv->origin.y = 0;
	ClientToScreen(hWnd, &wglPriv->origin);

	/* Find window relative extent of client area */
	GetClientRect(hWnd, &wglPriv->rect);

	/* Convert client rect to be screen relative */
	wglPriv->rect.left += wglPriv->origin.x;
	wglPriv->rect.top += wglPriv->origin.y;
	wglPriv->rect.right += wglPriv->origin.x;
	wglPriv->rect.bottom += wglPriv->origin.y;

	wglPriv->width = wglPriv->rect.right - wglPriv->rect.left;
	wglPriv->height = wglPriv->rect.bottom - wglPriv->rect.top;
    }
}

/*****************************************************************************
**
** __wglUpdateDrawableBuffers.
** 
** This routine should get called at initialization and whenever the window is
** changed (resized or moved). Care has to be taken to ensure that this is 
** only called when the context is valid. 
** 
*****************************************************************************/

GLboolean
__wglUpdateDrawableBuffers(__WGLcontext* glrc)
{
    __GLcontext *gc = glrc->gc;
    __WGLdrawablePrivate *wglPriv = glrc->wglPriv;

    __wglUpdateDrawableSize(wglPriv);

    if (wglPriv->updatePalette) {
	(*wglPriv->updatePalette)(wglPriv);
    }

    if ((*gc->exports.changeDrawableSize)(gc, wglPriv->width, wglPriv->height)) {
	glrc->pendingWindowChange = GL_FALSE;
	return GL_TRUE;
    }

    return GL_FALSE;
}

__WGLdrawablePrivate *
__wglGetDrawablePrivate(HDC hDC, HWND hWnd, __GLcontextModes *modes)
{
    __WGLdrawablePrivate *wglPriv;

    wglPriv = __wglFindDrawablePrivate(hDC, hWnd);

    if (wglPriv == NULL) {
	wglPriv = __wglCreateDrawablePrivate(hDC, hWnd, modes);
    }

    return wglPriv;
}

GLvoid
__wglFormatGLModes(__GLcontextModes *modes, LPPIXELFORMATDESCRIPTOR pPFD)
{
    memset(modes, 0, sizeof(*modes));
    modes->rgbMode = pPFD->iPixelType == PFD_TYPE_RGBA;
    modes->colorIndexMode = pPFD->iPixelType == PFD_TYPE_COLORINDEX;
    modes->doubleBufferMode = (BYTE) pPFD->dwFlags & PFD_DOUBLEBUFFER; 
    modes->stereoMode = (BYTE) pPFD->dwFlags & PFD_STEREO;
    modes->haveAccumBuffer = pPFD->cAccumBits != 0;
    modes->haveDepthBuffer = pPFD->cDepthBits != 0;
    modes->haveStencilBuffer = pPFD->cStencilBits != 0;
    modes->redBits = pPFD->cRedBits;
    modes->greenBits = pPFD->cGreenBits;
    modes->blueBits = pPFD->cBlueBits;
    modes->alphaBits = pPFD->cAlphaBits;
    modes->redMask = ((1UL << pPFD->cRedBits) - 1) << pPFD->cRedShift;
    modes->greenMask = ((1UL << pPFD->cGreenBits) - 1) << pPFD->cGreenShift;
    modes->blueMask = ((1UL << pPFD->cBlueBits) - 1) << pPFD->cBlueShift;
    modes->alphaMask = ((1UL << pPFD->cAlphaBits) - 1) << pPFD->cAlphaShift;
    modes->rgbBits = pPFD->cColorBits;
    modes->indexBits = pPFD->cColorBits;
    modes->accumRedBits = pPFD->cAccumRedBits;
    modes->accumGreenBits = pPFD->cAccumGreenBits;
    modes->accumBlueBits = pPFD->cAccumBlueBits;
    modes->accumAlphaBits = pPFD->cAccumAlphaBits;
    modes->depthBits = pPFD->cDepthBits;
    modes->stencilBits = pPFD->cStencilBits;
    modes->numAuxBuffers = 0;
    modes->level = 0;
}

GLboolean
__wglCreateHWContext(__WGLcontext *glrc)
{
    LPPIXELFORMATDESCRIPTOR pPFD = &glrc->pFD;
    __GLcontextModes *modes;

    /* set up modes (get info from PixelFormatDescriptor) */
    modes = (__GLcontextModes *) __wglMalloc(sizeof(__GLcontextModes));
    glrc->modes = modes;
    __wglFormatGLModes(modes, pPFD);

    /* set up imports (pointer back to wgl glrc) */
    imports.other = (void *) glrc;

    /* create the core rendering context */
    glrc->gc = (__glDevice->devCreateContext)(&imports, modes);

    return (glrc->gc != NULL);
}


/*****************************************************************************
**
**  __wglMonitorWindowChanges.
**
**  Callback function that parses the message stream looking for messages 
**  of interest to the wgl drawable management code.
**
****************************************************************************/

LRESULT	CALLBACK
__wglMonitorWindowChanges(int code, WPARAM wParam, LPARAM lParam)
{
    HHOOK messageHook = ((__GLthreadArea *)TlsGetValue(__wglTLSIndex))->messageHook;
    CWPSTRUCT *callBackMessage = (CWPSTRUCT *) lParam;

    /*
    ** Examine the message and see if it is a window change type.
    ** Then look to see if is a window associated with an openGL
    ** context.  If it is adjust its buffers, viewport, and scissor
    ** area.  Note that there may be more than one context attached
    ** to a single window.
    */
    switch (callBackMessage->message) {
    case WM_WINDOWPOSCHANGED:
    case WM_PALETTECHANGED:
	{
	    __WGLcontext *glrc = NULL;

	    while ((glrc = __wglFindWGLWindow(callBackMessage->hwnd, glrc)) != NULL) {
		/*
		** If the context does not belong to this thread, then return - each
		** thread will look after its own window changes.
		*/
		if (glrc->owner != GetCurrentThreadId()) {
		    continue;
		}

		/*
		** Update the drawable now if the context is current, otherwise
		** just mark it as needing a drawable change.  Which will happen
		** the next time the context is made current.
		*/
		if (glrc->isInUse) {	
		    if (__wglUpdateDrawableBuffers(glrc) == GL_FALSE) {
			/* resize failed */
			/* XXX: What can we possibly do here? */
			;
		    }
		} else {
		    glrc->pendingWindowChange = GL_TRUE;
		}
	    }
	}
	break;
    case WM_DESTROY:
	__wglDestroyDrawable(callBackMessage->hwnd);
	break;
    case WM_DISPLAYCHANGE:
	{
	    static int width, height, depth;
	    int nwidth, nheight, ndepth;
	    ndepth = callBackMessage->wParam;
	    nwidth = LOWORD(callBackMessage->lParam);
	    nheight = LOWORD(callBackMessage->lParam);
	    if (ndepth != depth || nwidth != width || nheight != height) {
		width = nwidth; height = nheight; depth = ndepth;
		(*__glDevice->devInvalidatePixelFormatList)();
	    }
	}
	break;
    default:
	break;
    }	
    return CallNextHookEx(messageHook, code, wParam, lParam);
}

void WINAPI
wglLockBuffers(HGLRC hGLRC)
{
    __GLcontext *gc = __wglGetCurrentGC();
    __WGLcontext *glrc = __wglFindWGLContext(hGLRC);

    if (gc == NULL) {
	__wglAttachThread(GetCurrentThreadId());
	gc = __wglGetCurrentGC();
    }

    if (glrc == NULL || glrc->gc == __wglInvalidGC || glrc->gc != gc) {
        __wglSetSystemError("wglLockBuffers", WGL_INVALID_HRC);
	return;
    }

    (*glrc->gc->exports.lockBuffers)(glrc->gc);
}

void WINAPI
wglUnlockBuffers(HGLRC hGLRC)
{
    __GLcontext *gc = __wglGetCurrentGC();
    __WGLcontext *glrc = __wglFindWGLContext(hGLRC);

    if (gc == NULL) {
	__wglAttachThread(GetCurrentThreadId());
	gc = __wglGetCurrentGC();
    }

    if (glrc == NULL || glrc->gc == __wglInvalidGC || glrc->gc != gc) {
        __wglSetSystemError("wglUnlockBuffers", WGL_INVALID_HRC);
	return;
    }

    (*glrc->gc->exports.unlockBuffers)(glrc->gc);
}



/************************************************************/
/* DLL and Initialization Stuff */

static BOOL
__wglGetOS(void)
{
    OSVERSIONINFO OSVersion;

    OSVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&OSVersion)) {
	/* If we can't find what operating system we are on then exit */
	return FALSE;
    }

    if (OSVersion.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
	__wglOSWin95 = TRUE;
    } else {
	__wglOSWin95 = FALSE;
    }
    return TRUE;
}

__GLcontext *
__wglGetCurrentGC(void)
{
    __GLcontext *context = __glGetTLSCXValue();

    if (context == NULL) {
	__wglAttachThread(GetCurrentThreadId());
        context = __glGetTLSCXValue();
    }

    assert(context != NULL);
    return context;
}

void
__wglSetCurrentGC(__GLcontext *context)
{
    __glSetTLSCXValue(context);
}

#ifdef SST
extern LRESULT	CALLBACK __wglSSTMonitorWindowChanges(int code, 
						WPARAM wParam, LPARAM lParam)
#endif

BOOL
__wglAttachThread(DWORD hThread)
{
    __GLcontext *context = processContext;
    __GLthreadArea *tla;
    HHOOK messageHook;

    /* Setup context area */
    if (shareProcessContext && context != NULL) {
	TlsSetValue(__wglTLSCXIndex, (LPVOID) context);
    } else {
	TlsSetValue(__wglTLSCXIndex, (LPVOID) context);
	__wglSetCurrentGC(__wglInvalidGC);
	__glCoreNopDispatch();

	processContext = context;
    }

    /* Allocate thread local area */
    tla = (__GLthreadArea *) __wglMalloc(sizeof(__GLthreadArea));
    if (tla == NULL) {
	return FALSE;
    }
    TlsSetValue(__wglTLSIndex, (LPVOID) tla);

    /* Setup message hook */
    if (!shareProcessContext) {
	messageHook = SetWindowsHookEx(WH_CALLWNDPROC,

/*  XXXX  We need to have our own MonitorWindowChanges func because we
	  have to deal with all kinds of "pass thru" issues with VooDoo gfx */
#ifdef SST
				(HOOKPROC)__wglSSTMonitorWindowChanges,
#else
				(HOOKPROC) __wglMonitorWindowChanges,
#endif
				hInstanceOpenGL, hThread);
	if (messageHook == NULL) {
	    __wglSetSystemError("__wglAttachThread", WGL_HOOK_FAILED);
	    return FALSE;
	}
	tla->messageHook = messageHook;
    }

    return TRUE;
}

BOOL
__wglDetachThread(HINSTANCE hInst, DWORD hThread)
{
    __GLcontext *context = TlsGetValue(__wglTLSCXIndex);
    __GLthreadArea *tla = (__GLthreadArea *)TlsGetValue(__wglTLSIndex);

    if (context != NULL) {
	HHOOK messageHook = tla->messageHook;

	/* Remove message hook */
	if (messageHook) {
	    if (UnhookWindowsHookEx(messageHook) == FALSE) {
		return FALSE;
	    } 
	}

	TlsSetValue(__wglTLSCXIndex, (LPVOID) 0);
    }

    TlsSetValue(__wglTLSIndex, (LPVOID) 0);

    return TRUE;
}

BOOL
__wglAttachProcess(HINSTANCE hInst, DWORD hThread)
{
    __wglCreateMutex();
    __wglCreateCriticalSection();

    __wglInitErrorLog();

    __wglTLSCXIndex = TlsAlloc();
    __wglTLSIndex = TlsAlloc();
    if (__wglOSWin95) {
	__wglTLSCXOffset = W95_TLS_INDEX_TO_OFFSET(__wglTLSCXIndex);
	__wglTLSOffset = W95_TLS_INDEX_TO_OFFSET(__wglTLSIndex);
    } else {
	__wglTLSCXOffset = WNT_TLS_INDEX_TO_OFFSET(__wglTLSCXIndex);
	__wglTLSOffset = WNT_TLS_INDEX_TO_OFFSET(__wglTLSIndex);
    }

    __wglInitializeContextList();

    return __wglAttachThread(hThread);
}

BOOL
__wglDetachProcess(HINSTANCE hInst, DWORD hThread)
{
    processContext = NULL;
    if (__wglDetachThread(hInst, hThread) == FALSE) {
	return FALSE;
    }
    TlsFree(__wglTLSCXIndex);
    TlsFree(__wglTLSIndex);

    __wglDestroyCriticalSection();
    __wglDestroyMutex();

    return TRUE;
}

/****************************************************************************
**
** Functions for accessing values from the registry.
**
*****************************************************************************/

static HKEY hRegKey;

#define OPENGL_KEY \
			"Software\\Silicon Graphics\\OpenGL"

static DWORD
queryRegistry(LPSTR valueName)
{
    DWORD value;
    DWORD valueType;
    DWORD valueSize = sizeof(value);
    LONG status;

    if (hRegKey == 0) {
	status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, OPENGL_KEY,
					    0, KEY_ALL_ACCESS, &hRegKey);

	if (status != ERROR_SUCCESS) {
	    hRegKey = 0;
	    return 0;
	}
    }

    status = RegQueryValueEx(hRegKey, valueName, NULL,
				&valueType, (LPBYTE) &value, &valueSize);

    if (status != ERROR_SUCCESS) {
	return 0;
    }

    return value;
}


#ifdef __GL_COSMO_BUILD

/****************************************************************************
**
** __wglInitializeDispatch
**
** Determine if SGI's OpenGL or Microsoft's implementation should be
** used.  If the latter is chosen, initialize the dispatch table to
** revector to opengl32.dll.  Return value is TRUE if Microsoft's
** library is chosen, FALSE otherwise.
**
*****************************************************************************/

#define OPENGL_OVERRIDE_DISPATCH \
			"OverrideDispatch"

static HINSTANCE hInstanceMS = NULL;

/* PIXELFORMATDESCRIPTOR flags for OpenGL 1.1 */
#ifndef PFD_GENERIC_ACCELERATED
#define PFD_GENERIC_ACCELERATED         0x00001000
#endif

/* Determine if the given DLL supports hardware rendering.
 * Return the boolean result.
 */
static BOOL __wglHasHardwareAcceleration(HINSTANCE hInstance)
{
    HDC hDC;
    int ii, count;
    int (WINAPI *Describe)(HDC, int, UINT, LPPIXELFORMATDESCRIPTOR);

    Describe = (void *) GetProcAddress(hInstance, "wglDescribePixelFormat");
    if (NULL == Describe)
	return FALSE;

    hDC = GetDC(0);
    count = (*Describe)(hDC, 0, 0, NULL);

    for (ii = 1; ii <= count; ii++) {
	PIXELFORMATDESCRIPTOR pfd;

	if ((*Describe)(hDC, ii, sizeof(pfd), &pfd)) {
	    BOOL genericFormat = (pfd.dwFlags & PFD_GENERIC_FORMAT) != 0;
	    BOOL genericAccel = (pfd.dwFlags & PFD_GENERIC_ACCELERATED) != 0;

	    /* Determine hardware driver status by the combination of these
	     * two flags.  Both FALSE means ICD, both TRUE means MCD.
	     */
	    if (genericFormat == genericAccel) {
		ReleaseDC(0, hDC);
		return TRUE;
	    }
	}
    }

    ReleaseDC(0, hDC);
    return FALSE;
}

/* Return values:
 * 0: do not dispatch to opengl32.dll
 * 1: dispatch to opengl32.dll
 * 2: fatal error, loading library failed
 */
static int __wglInitializeDispatch(HINSTANCE hInst)
{
    extern BOOL __wglSetupMSDispatch(HINSTANCE hInstance);
    DWORD dispatch = queryRegistry(OPENGL_OVERRIDE_DISPATCH);
    char *env = getenv("__GL_OVERRIDE_DISPATCH");
    BOOL isOpenGL32 = FALSE;

    /*
    ** Environment variable has precedence over the registry value:
    **   0: default (use opengl.dll unless hardware accelerated)
    **   1: force opengl.dll
    **   2: force opengl32.dll
    */

#ifdef __GL_ICD
    /* in the ICD case, do not dispatch.  We support hw */
    dispatch = 1;
#else
    if (env) {
	dispatch = atoi(env);
    }
#endif

    if (0 > dispatch || dispatch > 2) {
	dispatch = 0;
    }

    if (1 == dispatch) {
	return 0;
    } else {
	char name[256];
	char *ptr;
	int len;

	len = GetModuleFileName(hInst, name, 255);
	if (0 != len) {
	    ptr = name+len-1;
	    while (ptr > name && *ptr != '\\')
		ptr--;
	    if (*ptr == '\\')
		ptr++;
	} else {
	    return 2;
	}

	/* Check if we have been renamed to opengl32.dll */
	if (!stricmp(ptr, "opengl32.dll")) {
	    return 0;
	}
    }

    switch (dispatch) {

    case 0:
	hInstanceMS = LoadLibrary("OPENGL32.DLL");

	/* Use opengl32 only if there is hardware acceleration */
	if ((hInstanceMS && __wglHasHardwareAcceleration(hInstanceMS))) {
	    if (__wglSetupMSDispatch(hInstanceMS)) {
		return 1;
	    }
	}

	if (hInstanceMS) {
	    FreeLibrary(hInstanceMS);
	    hInstanceMS = NULL;
	}

	return 0;

    case 2:
	/* Use opengl32 or die */
	hInstanceMS = LoadLibrary("OPENGL32.DLL");

	if (hInstanceMS && __wglSetupMSDispatch(hInstanceMS)) {
	    return 1;
	}

	/* We couldn't load the library.  This is a fatal error. */
	if (hInstanceMS) {
	    FreeLibrary(hInstanceMS);
	    hInstanceMS = NULL;
	}

	MessageBox(NULL,
		   "A required .DLL file, OPENGL32.DLL, was not found.",
		   "Error Starting Program",
		   MB_ICONEXCLAMATION|MB_SYSTEMMODAL|MB_OK);

	return 2;

    default:
	assert(0);
	break;
    }

    return 0;
}

static void __wglTerminateDispatch()
{
    if (hInstanceMS) {
	FreeLibrary(hInstanceMS);
	hInstanceMS = NULL;
    }
}

#endif /* __GL_COSMO_BUILD */

BOOL WINAPI
DllMain(
    HINSTANCE hInst,
    DWORD Reason,
    LPVOID Reserverd)
{
    hInstanceOpenGL = hInst;

    switch (Reason) {
    case DLL_PROCESS_ATTACH:	/* Process started up. */
	if (__wglGetOS() == FALSE) {
	    return FALSE;
	}
	if (__wglAttachProcess(hInst, GetCurrentThreadId()) == FALSE) {
	    return FALSE;
	}
        if (__glDevice->devProcessAttach) {
	    if ((*__glDevice->devProcessAttach)(GetCurrentThreadId()) == FALSE) {
		/* before we return, undo what we've done */
		__wglDetachProcess(hInst, GetCurrentThreadId());
		return FALSE;
	    }
	}

#ifdef __GL_COSMO_BUILD
	switch (__wglInitializeDispatch(hInst)) {
	case 0:
	    return TRUE;
	case 1:
	    shareProcessContext = TRUE;
	    return TRUE;
	case 2:
	    return FALSE;
	}
#endif
	break;

    case DLL_PROCESS_DETACH:	/* Process closed down. */
#ifdef __GL_COSMO_BUILD
	__wglTerminateDispatch();
#endif
        if (__glDevice->devProcessDetach) {
            if ((*__glDevice->devProcessDetach)(GetCurrentThreadId()) == FALSE) {
		return FALSE;
	    }
	}
	return __wglDetachProcess(hInst, GetCurrentThreadId());
	break;

    case DLL_THREAD_ATTACH:	/* Thread has started up. */
	if (__wglAttachThread(GetCurrentThreadId()) == FALSE) {
	    return FALSE;
	}
        if (__glDevice->devThreadAttach) {
            if ((*__glDevice->devThreadAttach)(GetCurrentThreadId()) == FALSE) {
		/* first undo what we've done */
		__wglDetachThread(hInst, GetCurrentThreadId());
		return FALSE;
	    }
	}

	/* free up pixelformat memory */
	__glDevice->devInvalidatePixelFormatList();
	break;

    case DLL_THREAD_DETACH:	/* Thread has closed down. */
        if (__glDevice->devThreadDetach) {
            if ((*__glDevice->devThreadDetach)(GetCurrentThreadId()) == FALSE) {
		return FALSE;
	    }
	}
	return __wglDetachThread(hInst, GetCurrentThreadId());
	break;

    default:
	break;
    }
    return TRUE;
}

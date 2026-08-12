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
#include "g_xproto.h"

#ifdef __GL_SST
#include <glide.h>
#include "sst_context.h"
/* can be overriden with environment variable */
static int sstUseHardware = GL_TRUE;
static int __glIgnoreActivate = 0;
#endif

/* ------------------------------------------------------------------ */
/* OGLLOG - crash-robust debug logging to C:\3dfxogl.log.              */
/* Every line is opened/appended/flushed/closed so it survives an     */
/* instant process death.  Formatting via wvsprintfA (user32, no CRT   */
/* float support needed).  Failures are ignored silently.              */
/* ------------------------------------------------------------------ */
#include <stdarg.h>

void OGLLOG( const char *fmt, ... )
{
    static volatile LONG oglLogBusy = 0;
    char    msg[512];
    char    line[560];
    int     len;
    HANDLE  hf;
    DWORD   written;
    va_list ap;

    /* minimal reentrancy guard: drop the line if we are already logging */
    if ( InterlockedExchange( (LONG *)&oglLogBusy, 1 ) )
        return;

    va_start( ap, fmt );
    wvsprintfA( msg, fmt, ap );
    va_end( ap );

    len = wsprintfA( line, "[%lu] %s\r\n", GetTickCount(), msg );

    hf = CreateFileA( "C:\\3dfxogl.log", GENERIC_WRITE,
                      FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if ( hf != INVALID_HANDLE_VALUE ) {
        SetFilePointer( hf, 0, NULL, FILE_END );
        WriteFile( hf, line, (DWORD)len, &written, NULL );
        FlushFileBuffers( hf );
        CloseHandle( hf );
    }

    InterlockedExchange( (LONG *)&oglLogBusy, 0 );
}

/* OGLLOGV - VERBOSE tier of OGLLOG.  High-frequency traces (per-draw /
** per-bind, e.g. FILT@) go through here and only reach the log when the
** file marker C:\icd_verbose.on exists.  Checked once per process; the
** always-on OGLLOG stays for lifecycle + error lines so any crash is
** diagnosable from C:\3dfxogl.log without a special build. */
void OGLLOGV( const char *fmt, ... )
{
    static int verbose = -1;
    char    msg[512];
    va_list ap;

    if ( verbose < 0 ) {
        HANDLE g = CreateFileA( "C:\\icd_verbose.on", GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                OPEN_EXISTING, 0, NULL );
        verbose = ( g != INVALID_HANDLE_VALUE ) ? 1 : 0;
        if ( verbose ) CloseHandle( g );
    }
    if ( !verbose ) return;

    va_start( ap, fmt );
    wvsprintfA( msg, fmt, ap );
    va_end( ap );
    OGLLOG( "%s", msg );
}
/* ------------------------------------------------------------------ */

int WINAPI wglGetPixelFormat(HDC hDC);
int WINAPI wglDescribePixelFormat(HDC hDC, int iPixelFormat,
                                  UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd);

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
    if (ptr == NULL) {
        return NULL;    /* XXX out of memory error */
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
    if (ptr == NULL) {
        return NULL;    /* XXX out of memory error */
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
        return NULL;    /* XXX out of memory error */
    }
    return newPtr;
}

void
__wglFree(void *ptr)
{
    if (ptr != NULL) {
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
        return NULL;    /* XXX out of memory error */
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
        return NULL;    /* XXX out of memory error */
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
        return NULL;    /* XXX out of memory error */
    }
    return newPtr;
}

void
__wglImpFree(__GLcontext *gc, void *ptr)
{
    if (ptr != NULL) {
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
    WGLCONTEXT *glrc = (WGLCONTEXT *) gc->imports.other;

    return &glrc->wglPriv->glPriv;
}

void
__wglImpGetDrawableSize(__GLcontext *gc, int *w, int *h)
{
    WGLCONTEXT *glrc = (WGLCONTEXT *) gc->imports.other;
    __WGLdrawablePrivate *wglPriv = glrc->wglPriv;

    __wglUpdateDrawableSize(wglPriv);

    *w = (int) wglPriv->width;
    *h = (int) wglPriv->height;
}

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

    __wglOpenErrorFile();
        sprintf(sBuffer, "WGL Message:%s\n", str);
    if (msgFile != INVALID_HANDLE_VALUE) {
                WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    } else {
                OutputDebugString( sBuffer );
        }
}

void
__wglMessageArg(const char *str, long arg)
{
    char sBuffer[256];
    ULONG aWritten;

    __wglOpenErrorFile();
        sprintf(sBuffer, "WGL Message:%s: %d (%08x)\n", str, arg, arg);
    if (msgFile != INVALID_HANDLE_VALUE) {
                WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    } else {
                OutputDebugString( sBuffer );
        }
}

void
__wglMessageArg2(const char *str, long arg1, long arg2)
{
    char sBuffer[256];
    ULONG aWritten;

    __wglOpenErrorFile();
        sprintf(sBuffer, "WGL Message:%s: %5d (%08x)\n", str, arg1, arg2);
    if (msgFile != INVALID_HANDLE_VALUE) {
                WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    } else {
                OutputDebugString( sBuffer );
        }
}

void
__wglMessageFloat(const char *str, float arg)
{
    char sBuffer[256];
    ULONG aWritten;

    __wglOpenErrorFile();
        sprintf(sBuffer, "WGL Message:%s: %f (%08x)\n", str, arg, *((DWORD *) &arg));
    if (msgFile != INVALID_HANDLE_VALUE) {
                WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    } else {
                OutputDebugString( sBuffer );
        }
}

void
__wglError(const char *str)
{
    char sBuffer[256];
    ULONG aWritten;

    __wglOpenErrorFile();
        sprintf(sBuffer, "OGL ERR:%s\n", str);
    if (msgFile != INVALID_HANDLE_VALUE) {
                WriteFile(msgFile, sBuffer, strlen(sBuffer), &aWritten, NULL);
    } else {
                OutputDebugString( sBuffer );
        }
} 

void
__wglSetSystemError(const char *str, long errorNumber)
{
    char sysError[256];

    sprintf(sysError, "System Error: (%d):%s", errorNumber, str);
    OGLLOG( "3dfxogl ERROR path: %s (err=%ld)", str, errorNumber );
    __wglMessage(sysError);
    SetLastError(errorNumber | WGL_OGL_ID | WGL_ERROR | WGL_APP_ERROR);
}

/************************************************************/
/* Synchronization Stuff */

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

static int freeHandle = 1;
static __GLcontext dummyGC;
static __GLcontext *__wglInvalidGC;

static WGLCONTEXT glrcListEnd;

void
__wglInitializeContextList(void)
{
    __wglLockMutex();

    dummyGC.imports.other = (void *) &glrcListEnd;
    __wglInvalidGC = &dummyGC;
    glrcListEnd.gc = __wglInvalidGC;

    __wglUnlockMutex();
}

void
__wglAddContext(WGLCONTEXT *glrc)
{
    WGLCONTEXT *glrcList = &glrcListEnd;

    /* Add the structure to the list. */
    glrc->next = glrcList->next;
    glrc->last = glrcList;
    if (glrcList->next) {
        glrcList->next->last = glrc;
    }
    glrcList->next = glrc;
}

void
__wglRemoveContext(WGLCONTEXT *glrc)
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
WGLCONTEXT *
__wglFindWGLContext(HGLRC hGLRC)
{
    WGLCONTEXT *glrc = &glrcListEnd;

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
WGLCONTEXT *
__wglFindWGLWindow(HWND hWnd, WGLCONTEXT *glrc)
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

__WGLdrawablePrivate *
__wglCreateDrawablePrivate(HDC hDC, HWND hWnd)
{
    __WGLdrawablePrivate *wglPriv;
    LPRGNDATA rgnData;

    wglPriv = (__WGLdrawablePrivate *) __wglMalloc(sizeof(*wglPriv));
    memset(wglPriv, 0, sizeof(*wglPriv));
    wglPriv->hDC = hDC;
    wglPriv->hWnd = hWnd;

    /* data for context modes */
    wglPriv->glPriv.modes = (__GLcontextModes *)
        __wglMalloc(sizeof(__GLcontextModes));

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
        if (wglPriv->hDC == hDC && wglPriv->hWnd == hWnd) {
            __wglUnlockMutex();
            return wglPriv;
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

/************************************************************/
/* Drawable Stuff */

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

#if 0
#ifdef __GL_SST 
        if (sstUseHardware) {
          if (wglPriv->width > 640) {
            wglPriv->width = 640;
          }
          if (wglPriv->height > 480) {
            wglPriv->height = 480;
          }
        }
#endif  
#endif
    }
}

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

void
__wglUpdateDrawableBuffers(WGLCONTEXT* glrc)
{
    __GLcontext *gc = glrc->gc;
    __WGLdrawablePrivate *wglPriv = glrc->wglPriv;

    __wglUpdateDrawableSize(wglPriv);

    if (wglPriv->updatePalette) {
        (*wglPriv->updatePalette)(wglPriv);
    }

    (*gc->exports.changeDrawableSize)(gc, wglPriv->width, wglPriv->height);

    glrc->pendingWindowChange = GL_FALSE;
}

__WGLdrawablePrivate *
__wglGetDrawablePrivate(HDC hDC, HWND hWnd, __GLcontextModes *modes)
{
    __WGLdrawablePrivate *wglPriv = __wglFindDrawablePrivate(hDC, hWnd);
    __GLdrawablePrivate *glPriv;

    if (wglPriv == NULL) {
        wglPriv = __wglCreateDrawablePrivate(hDC, hWnd);
    }

    glPriv = &wglPriv->glPriv;
    *glPriv->modes = *modes;
    glPriv->malloc = __wglMalloc;
    glPriv->calloc = __wglCalloc;
    glPriv->realloc = __wglRealloc;
    glPriv->free = __wglFree;
    glPriv->addSwapRect = __wglAddSwapHintRect;
    glPriv->setClipRect = __wglSetCoreClipRect;
    glPriv->other = wglPriv;

    if (hWnd) {
        __wglFBInitDrawable(wglPriv);
    } else {
        __wglDIBInitDrawable(wglPriv);
    }

    return wglPriv;
}

/************************************************************/
/* WGL API */

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

BOOL
__wglCreateHWContext(WGLCONTEXT *glrc)
{
    LPPIXELFORMATDESCRIPTOR pPFD = &glrc->pFD;
    __GLcontextModes *modes;

    /* set up modes (translate PixelFormatDescriptor) */
    modes = (__GLcontextModes *) __wglMalloc(sizeof(*modes));
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
    modes->indexBits = pPFD->cColorBits;
    modes->accumRedBits = pPFD->cAccumRedBits;
    modes->accumGreenBits = pPFD->cAccumGreenBits;
    modes->accumBlueBits = pPFD->cAccumBlueBits;
    modes->accumAlphaBits = pPFD->cAccumAlphaBits;
    modes->depthBits = pPFD->cDepthBits;
    modes->stencilBits = pPFD->cStencilBits;
    modes->numAuxBuffers = 0;
    modes->level = 0;

    glrc->modes = modes;

    /* set up imports (pointer back to wgl glrc) */
    imports.other = (void *) glrc;

    /* create a core rendering context */
#ifdef __GL_SST
    {
      if (sstUseHardware) {
        glrc->gc = __glSSTCreateContext(&imports, modes);
      } else {
        glrc->gc = __glCoreCreateContext(&imports, modes);
      }
    }
#else
    glrc->gc = __glCoreCreateContext(&imports, modes);
#endif

    return (glrc->gc != NULL);
}

HGLRC WINAPI
wglCreateContext(HDC hDC)
{
    int pixelFormat = wglGetPixelFormat(hDC);
    WGLCONTEXT *glrc;

    OGLLOG( "wglCreateContext hdc=0x%x pixelFormat=%d",
            (unsigned)hDC, pixelFormat );

    if (pixelFormat == 0) {
        __wglSetSystemError("wglCreateContex", WGL_BAD_PIXEL_FORMAT);
        return (HGLRC) 0;
    }

    glrc = (WGLCONTEXT *) __wglMalloc(sizeof(WGLCONTEXT));
    if (glrc == NULL) {
        __wglSetSystemError("wglCreateContext", WGL_MALLOC_FAILED);
        return (HGLRC) 0;
    }
    memset(glrc, 0, sizeof(*glrc));

    if (!wglDescribePixelFormat(hDC, pixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &glrc->pFD)) {
        __wglFree(glrc);
        __wglSetSystemError("wglCreateContext", WGL_UNKNOWN_PIXEL_FORMAT);
        return (HGLRC) 0;
    }

    glrc->isInUse = FALSE;
    glrc->owner = (DWORD) NO_OWNER;

    glrc->pixelFormat = pixelFormat;
    glrc->hasBeenCurrent = FALSE;
    glrc->pendingDestroy = FALSE;
    glrc->pendingWindowChange = TRUE;
    glrc->isDirect = TRUE;

    __wglLockMutex();
    glrc->hGLRC = (HGLRC) freeHandle++;

    if (!__wglCreateHWContext(glrc)) {
        __wglFree(glrc);
        __wglSetSystemError("wglCreateContext", WGL_ERROR);
        __wglUnlockMutex();
        return (HGLRC) 0;
    }

    __wglAddContext(glrc);
    __wglUnlockMutex();

    OGLLOG( "wglCreateContext OK hglrc=0x%x", (unsigned)glrc->hGLRC );
    return glrc->hGLRC;
}

BOOL WINAPI
wglDeleteContext(HGLRC hGLRC)
{
    __GLcontext *gc = GET_CURRENT_GC;
    WGLCONTEXT *glrc = __wglFindWGLContext(hGLRC);

    if (gc == NULL) {
        __wglAttachThread(GetCurrentThreadId());
        gc = GET_CURRENT_GC;
    }

    if (glrc == NULL) {
        __wglSetSystemError("wglDeleteContext", WGL_INVALID_HRC);
        return FALSE;
    }

    if (glrc->isInUse) {
        if (glrc->owner == GetCurrentThreadId()) {
            __WGLdrawablePrivate *wglPriv = glrc->wglPriv;

            if (!(*gc->exports.loseCurrent)(gc)) {
                return FALSE;
            }

            wglPriv->refCount--;
            if (wglPriv->hWnd == NULL) {
                __wglDestroyDrawablePrivate(wglPriv);
            }

            glrc->wglPriv = NULL;
            glrc->isInUse = FALSE;
            glrc->owner = (DWORD) NO_OWNER;
            glrc->hWnd = NULL;
            glrc->hDC = NULL;
            SET_CURRENT_GC(__wglInvalidGC);
            __glCoreNopDispatch();
        } else {
            return FALSE;
        }
    }

    if (!(*glrc->gc->exports.destroyContext)(glrc->gc)) {
        __wglSetSystemError("wglDeleteContext", WGL_ERROR);
        return FALSE;
    }

    __wglLockMutex();
    __wglRemoveContext(glrc);
    __wglUnlockMutex();

    __wglFree(glrc);
    return TRUE;
}

void WINAPI
wglLockBuffers(HGLRC hGLRC)
{
    __GLcontext *gc = GET_CURRENT_GC;
    WGLCONTEXT *glrc = __wglFindWGLContext(hGLRC);

    if (gc == NULL) {
        __wglAttachThread(GetCurrentThreadId());
        gc = GET_CURRENT_GC;
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
    __GLcontext *gc = GET_CURRENT_GC;
    WGLCONTEXT *glrc = __wglFindWGLContext(hGLRC);

    if (gc == NULL) {
        __wglAttachThread(GetCurrentThreadId());
        gc = GET_CURRENT_GC;
    }

    if (glrc == NULL || glrc->gc == __wglInvalidGC || glrc->gc != gc) {
        __wglSetSystemError("wglUnlockBuffers", WGL_INVALID_HRC);
        return;
    }

    (*glrc->gc->exports.unlockBuffers)(glrc->gc);
}

HGLRC WINAPI
wglGetCurrentContext(void)
{
    __GLcontext *gc = GET_CURRENT_GC;
    WGLCONTEXT *glrc;

    if (gc == NULL) {
        __wglAttachThread(GetCurrentThreadId());
        gc = GET_CURRENT_GC;
    }

    glrc = (WGLCONTEXT *) gc->imports.other;
    return glrc->hGLRC;
}

HDC WINAPI
wglGetCurrentDC(void)
{
    __GLcontext *gc = GET_CURRENT_GC;
    WGLCONTEXT *glrc;

    if (gc == NULL) {
        __wglAttachThread(GetCurrentThreadId());
        gc = GET_CURRENT_GC;
    }

    glrc = (WGLCONTEXT *) gc->imports.other;
    return glrc->hDC;
}

HWND tacoHackHWND;
RECT tacoHackRect;
long tacoHackStyle;

BOOL WINAPI
wglMakeCurrent(HDC hDC, HGLRC hGLRC)
{
    __GLcontext *gcOld = GET_CURRENT_GC;
    WGLCONTEXT *glrc = __wglFindWGLContext(hGLRC);
    DWORD objType = GetObjectType(hDC);
    HWND hWnd = NULL;
    HBITMAP hBitmap = NULL;
    __WGLdrawablePrivate *wglPriv;

    if (gcOld == NULL) {
        __wglAttachThread(GetCurrentThreadId());
        gcOld = GET_CURRENT_GC;
    }

    if (hGLRC != (HGLRC) NULL) {

        if (glrc == NULL) {
            __wglSetSystemError("wglMakeCurrent", WGL_INVALID_HRC);
            return FALSE;
        }

        switch (objType) {
        case OBJ_DC:
            tacoHackHWND = hWnd = WindowFromDC(hDC);
            tacoHackStyle = GetWindowLong( tacoHackHWND, GWL_STYLE ) & WS_POPUP;
            GetClientRect(tacoHackHWND, &tacoHackRect);
            break;
        case OBJ_MEMDC:
            hBitmap = GetCurrentObject(hDC, OBJ_BITMAP);
            break;
        default:
            return FALSE;
        }

        if ((hWnd == NULL) && (hBitmap == NULL)) {
            __wglSetSystemError("wglMakeCurrent", WGL_INVALID_HDC);
            return FALSE;
        }

        if (glrc->isInUse && (glrc->owner != GetCurrentThreadId())) {
            __wglSetSystemError("wglMakeCurrent", WGL_CONTEXT_IN_USE);
            return FALSE;
        }

        wglPriv = __wglGetDrawablePrivate(hDC, hWnd, glrc->modes);
        if (wglPriv == NULL) {
            __wglSetSystemError("wglMakeCurrent", WGL_INVALID_HDC);
            return FALSE;
        }

        if (glrc->hDC == hDC && glrc->gc == gcOld && wglPriv->hWnd == hWnd) {
            return TRUE;
        }

        if (wglPriv->pixelFormat != glrc->pixelFormat) {
            __wglSetSystemError("wglMakeCurrent", WGL_DIFFERENT_P_FMAT);
            return FALSE;
        }
    }

    __wglLockMutex();

    /* LoseCurrent from old context */
    if (gcOld != __wglInvalidGC) {
        WGLCONTEXT *glrcOld = (WGLCONTEXT *) gcOld->imports.other;
        __WGLdrawablePrivate *wglPriv = glrcOld->wglPriv;

        if (!(*gcOld->exports.loseCurrent)(gcOld)) {
            __wglUnlockMutex();
            return FALSE;
        }

        wglPriv->refCount--;
#if 0
        /* Why? */
        if (wglPriv->hWnd == NULL) {
            __wglDestroyDrawablePrivate(wglPriv);
        }
#endif

        glrcOld->wglPriv = NULL;
        glrcOld->isInUse = FALSE;
        glrcOld->owner = (DWORD) NO_OWNER;
        glrc->hWnd = NULL;
        glrc->hDC = NULL;
        SET_CURRENT_GC(__wglInvalidGC);
    }

    /* MakeCurrent to new context */
    if (hGLRC != NULL) {
        __GLcontext *gc = glrc->gc;

        wglPriv->refCount++;
        glrc->wglPriv = wglPriv;

        if (!(*gc->exports.makeCurrent)(gc)) {
            __wglUnlockMutex();
            return FALSE;
        }
        SET_CURRENT_GC(gc);

        /* Set the flag to say that the context is in use. */
        glrc->isInUse = TRUE;
        glrc->owner     = GetCurrentThreadId();
        glrc->hWnd = hWnd;
        glrc->hDC = hDC;
        glrc->hasBeenCurrent = TRUE;
    } else {
        __glCoreNopDispatch();
    }

    __wglUnlockMutex();

    /* Has the Window's size/position changed ? */
    if (hGLRC != NULL && glrc->pendingWindowChange) {
        __wglUpdateDrawableBuffers(glrc);
    }

    return TRUE;
}

BOOL WINAPI
wglShareLists(HGLRC hGLRC1, HGLRC hGLRC2)
{
    __GLcontext *gcSrc, *gcDst;
    WGLCONTEXT *glrcSrc, *glrcDst;

    glrcSrc = __wglFindWGLContext(hGLRC1);
    if (glrcSrc == NULL) {
        __wglSetSystemError("wglShareLists", WGL_SHR_INVALID_SRC_CONTEXT);
        return FALSE;
    }

    glrcDst = __wglFindWGLContext(hGLRC2);
    if (glrcDst == NULL) {
        __wglSetSystemError("wglShareLists", WGL_SHR_INVALID_DST_CONTEXT);
        return FALSE;
    }

    __wglLockMutex();

    gcSrc = glrcSrc->gc;
    gcDst = glrcDst->gc;
    if (!(*gcDst->exports.shareContext)(gcDst, gcSrc)) {
        __wglSetSystemError("wglShareLists", WGL_SHR_DST_LIST_NOT_EMPTY);
        return FALSE;
    }

    __wglUnlockMutex();

    return TRUE;
}

BOOL WINAPI
wglSwapBuffers(HDC hDC)
{
    HWND hWnd = WindowFromDC(hDC);
    WGLCONTEXT *glrc;
    /* RETRO3DFX one-shot diagnostics: CS/GoldSrc renders but frames never
    ** reached the ICD swap -- log which exit path fires (first hit each). */
    static int __swFail1 = 1, __swFail2 = 1, __swFail3 = 1, __swOk = 1;

    /* Make sure that the device context has a valid window */
    if (hWnd == NULL) {
        if (__swFail1) { __swFail1 = 0;
            OGLLOG("wglSwapBuffers: EXIT WindowFromDC(0x%x)=NULL", (unsigned)hDC); }
        return FALSE;
    }

    /* Make sure there is a current context, and that it is double buffered */
    glrc = __wglFindWGLWindow(hWnd, NULL);
    if (glrc == NULL) {
        if (__swFail2) { __swFail2 = 0;
            OGLLOG("wglSwapBuffers: EXIT FindWGLWindow(hwnd=0x%x)=NULL", (unsigned)hWnd); }
        return FALSE;
    }
    if (!glrc->modes->doubleBufferMode) {
        if (__swFail3) { __swFail3 = 0;
            OGLLOG("wglSwapBuffers: EXIT singlebuffer modes (hwnd=0x%x)", (unsigned)hWnd); }
        return FALSE;
    }
    if (__swOk) { __swOk = 0;
        OGLLOG("wglSwapBuffers: OK first swap (hwnd=0x%x, further not logged)", (unsigned)hWnd); }

    (*glrc->gc->exports.swapBuffers)(glrc->gc);

    return (*glrc->wglPriv->swapBuffers)(glrc->wglPriv);
}

BOOL WINAPI
wglCopyContext(HGLRC hglrcSrc, HGLRC hglrcDst, UINT mask)
{
    __wglSetSystemError("wglCopyContext not supported", WGL_ERROR);
    return FALSE;
}

/*
** List of potentially supported pixel formats.  When adding entries
** to this list keep it sorted by (in order): color bits, depth bits,
** double buffer support.  Whenever the display mode changes, this
** list is filtered to produce a list of actual supported pixel formats.
**
** Since 32 bit RGBA can be either 888 or 8888 and 16 bit RGBA can be
** either 555 or 565, the bit sizes and bit shifts for 32 bit and 16 bit
** pixel formats are filled in when this list is filtered.
*/
#define NUM_RAW_PIXELFORMATS \
                (sizeof(rawPixelFormats) / sizeof(rawPixelFormats[0]))
static int pixelFormatsDirty = TRUE;
static PIXELFORMATDESCRIPTOR rawPixelFormats[] = {
    /*
    ** 32 bit
    */
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_RGBA,                                  // pixel type
        32, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_RGBA,                                  // pixel type
        32, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                                  // pixel type
        32, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                                  // pixel type
        32, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        32, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        32, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_COLORINDEX,                            // pixel type
        32, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_COLORINDEX,                            // pixel type
        32, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    /*
    ** 24 bit
    */
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_RGBA,                                  // pixel type
        24, 8, 16, 8, 8, 8, 0,                          // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_RGBA,                                  // pixel type
        24, 8, 16, 8, 8, 8, 0,                          // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                                  // pixel type
        24, 8, 16, 8, 8, 8, 0,                          // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                                  // pixel type
        24, 8, 16, 8, 8, 8, 0,                          // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        24, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        24, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_COLORINDEX,                            // pixel type
        24, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_COLORINDEX,                            // pixel type
        24, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    /*
    ** 16 bit
    */
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_RGBA,                                  // pixel type
        16, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_GENERIC_FORMAT,
        PFD_TYPE_RGBA,                                  // pixel type
        16, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                                  // pixel type
        16, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                                  // pixel type
        16, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        16, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        16, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_COLORINDEX,                            // pixel type
        16, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_COLORINDEX,                            // pixel type
        16, 0, 0, 0, 0, 0, 0,                           // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    /*
    ** 8 bit
    */
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_RGBA,                                  // pixel type
        8, 3, 5, 3, 2, 2, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_RGBA,                                  // pixel type
        8, 3, 5, 3, 2, 2, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                                  // pixel type
        8, 3, 5, 3, 2, 2, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                                  // pixel type
        8, 3, 5, 3, 2, 2, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        8, 0, 0, 0, 0, 0, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        8, 0, 0, 0, 0, 0, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_COLORINDEX,                            // pixel type
        8, 0, 0, 0, 0, 0, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_COLORINDEX,                            // pixel type
        8, 0, 0, 0, 0, 0, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    /*
    ** 4 bit
    */
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_RGBA,                                  // pixel type
        4, 1, 0, 1, 1, 1, 2,                            // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_RGBA,                                  // pixel type
        4, 1, 0, 1, 1, 1, 2,                            // color buffer
        0, 0,                                           // alpha buffer
        64, 16, 16, 16, 16,                             // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        4, 0, 0, 0, 0, 0, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        32,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // size
        1,                                              // version
        PFD_SUPPORT_OPENGL |                            // flags
        PFD_DRAW_TO_WINDOW,
        PFD_TYPE_COLORINDEX,                            // pixel type
        4, 0, 0, 0, 0, 0, 0,                            // color buffer
        0, 0,                                           // alpha buffer
        0, 0, 0, 0, 0,                                  // accumulation buffer
        16,                                             // depth buffer
        8,                                              // stencil buffer
        0,                                              // aux buffers
        PFD_MAIN_PLANE,                                 // layer type
        0,                                              // (reserved)
        0, 0, 0,                                        // layer masks
    },
};

/*
** List of actual supported pixel formats.  This is filled in as
** appropriate whenever the display mode changes.
*/
static int numPixelFormats;
static PIXELFORMATDESCRIPTOR *pixelFormats[NUM_RAW_PIXELFORMATS];

static void
decodeColorMask(DWORD mask, int *size, int *shift)
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

static BOOL
pixelFormatSupported(PIXELFORMATDESCRIPTOR *ppfd,
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
#if 0
        ppfd->dwFlags |= (PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP);
#endif
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

        decodeColorMask(rMask, &rSize, &rShift);
        decodeColorMask(gMask, &gSize, &gShift);
        decodeColorMask(bMask, &bSize, &bShift);

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

static void
updatePixelFormats(void)
{
    int displayDepth, rMask, gMask, bMask;
    int i;

    numPixelFormats = 0;

    displayDepth = __wglFBGetDisplayMasks(&rMask, &gMask, &bMask);

    /* first collect pixel formats which match the current display mode */
    for (i=0; i<NUM_RAW_PIXELFORMATS; ++i) {
        PIXELFORMATDESCRIPTOR *pRaw = &rawPixelFormats[i];

        if (pRaw->cColorBits == displayDepth) {
            if (pixelFormatSupported(pRaw, displayDepth, rMask, gMask, bMask)) {
                pixelFormats[numPixelFormats++] = pRaw;
            }
        }
    }

    /* then collect remaining pixel formats */
    for (i=0; i<NUM_RAW_PIXELFORMATS; ++i) {
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

            if (pixelFormatSupported(pRaw, displayDepth, rMask, gMask, bMask)) {
                pixelFormats[numPixelFormats++] = pRaw;
            }
        }
    }
}

int WINAPI
wglDescribePixelFormat(HDC hDC, int iPixelFormat,
                       UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
{
    if (pixelFormatsDirty) {
        updatePixelFormats();
        pixelFormatsDirty = FALSE;
    }

    if (ppfd) {
        if (iPixelFormat < 1 || iPixelFormat > numPixelFormats) {
            /* Should call SetLastError() here ! */
            return 0;
        }
        if (nBytes > sizeof(PIXELFORMATDESCRIPTOR))
            nBytes = sizeof(PIXELFORMATDESCRIPTOR);
        if (nBytes)
            memcpy(ppfd, pixelFormats[iPixelFormat - 1], nBytes);
    }
    return numPixelFormats;
}

int WINAPI
wglChoosePixelFormat(HDC hDC, const PIXELFORMATDESCRIPTOR *ppfd)
{
    PIXELFORMATDESCRIPTOR *ppfdBest = NULL;
    int i, bestIndex = -1;

    if (pixelFormatsDirty) {
        updatePixelFormats();
        pixelFormatsDirty = FALSE;
    }

    if (ppfd->dwFlags != (ppfd->dwFlags & 
                                    (
                                    PFD_DRAW_TO_WINDOW |
                                    PFD_DRAW_TO_BITMAP |
                                    PFD_SUPPORT_GDI |
                                    PFD_SUPPORT_OPENGL |
                                    PFD_GENERIC_FORMAT |
                                    PFD_NEED_PALETTE |
                                    PFD_NEED_SYSTEM_PALETTE |
                                    PFD_DOUBLEBUFFER |
                                    PFD_STEREO |
                                    /*PFD_SWAP_LAYER_BUFFERS |*/
                                    PFD_DOUBLEBUFFER_DONTCARE |
                                    PFD_STEREO_DONTCARE |
                                    PFD_SWAP_COPY |
                                    PFD_SWAP_EXCHANGE |
                                    0)))
    {
        /* error: bad dwFlags */
        return 0;
    }

    switch (ppfd->iPixelType) {
    case PFD_TYPE_RGBA:
    case PFD_TYPE_COLORINDEX:
        break;
    default:
        /* error: bad iPixelType */
        return 0;
    }

    switch (ppfd->iLayerType) {
    case PFD_MAIN_PLANE:
    case PFD_OVERLAY_PLANE:
    case PFD_UNDERLAY_PLANE:
        break;
    default:
        /* error: bad iLayerType */
        return 0;
    }

    /* loop through candidate pixel format descriptors */
    for (i=0; i<numPixelFormats; ++i) {
        PIXELFORMATDESCRIPTOR *ppfdCandidate = pixelFormats[i];

        /*
        ** Check attributes which must match
        */
        if (ppfd->iPixelType != ppfdCandidate->iPixelType) {
            continue;
        }

        if (ppfd->iLayerType != ppfdCandidate->iLayerType) {
            continue;
        }

        if (((ppfd->dwFlags ^ ppfdCandidate->dwFlags) & ppfd->dwFlags) &
            (PFD_DRAW_TO_WINDOW | PFD_DRAW_TO_BITMAP |
                PFD_SUPPORT_GDI | PFD_SUPPORT_OPENGL))
        {
            continue;
        }

        if (!(ppfd->dwFlags & PFD_DOUBLEBUFFER_DONTCARE)) {
            if ((ppfd->dwFlags & PFD_DOUBLEBUFFER) !=
                (ppfdCandidate->dwFlags & PFD_DOUBLEBUFFER))
            {
                continue;
            }
        }

        if (!(ppfd->dwFlags & PFD_STEREO_DONTCARE)) {
            if ((ppfd->dwFlags & PFD_STEREO) !=
                (ppfdCandidate->dwFlags & PFD_STEREO))
            {
                continue;
            }
        }

        if (ppfd->iPixelType==PFD_TYPE_RGBA
            && ppfd->cAlphaBits && !ppfdCandidate->cAlphaBits) {
            continue;
        }

        if (ppfd->iPixelType==PFD_TYPE_RGBA
            && ppfd->cAccumBits && !ppfdCandidate->cAccumBits) {
            continue;
        }

        if (ppfd->cDepthBits && !ppfdCandidate->cDepthBits) {
           continue;
        }

        if (ppfd->cStencilBits && !ppfdCandidate->cStencilBits) {
            continue;
        }

        if (ppfd->cAuxBuffers && !ppfdCandidate->cAuxBuffers) {
            continue;
        }

        /*
        ** See if candidate is better than the previous best choice
        */
        if (ppfdBest == NULL) {
            ppfdBest = ppfdCandidate;
            bestIndex = i;
            continue;
        }

        if ((ppfd->cColorBits > ppfdBest->cColorBits &&
                ppfdCandidate->cColorBits > ppfdBest->cColorBits) ||
            (ppfd->cColorBits <= ppfdCandidate->cColorBits &&
                ppfdCandidate->cColorBits < ppfdBest->cColorBits))
        {
            ppfdBest = ppfdCandidate;
            bestIndex = i;
            continue;
        }

        if (ppfd->iPixelType==PFD_TYPE_RGBA
            && ppfd->cAlphaBits
            && ppfdCandidate->cAlphaBits > ppfdBest->cAlphaBits)
        {
            ppfdBest = ppfdCandidate;
            bestIndex = i;
            continue;
        }

        if (ppfd->iPixelType==PFD_TYPE_RGBA
            && ppfd->cAccumBits
            && ppfdCandidate->cAccumBits > ppfdBest->cAccumBits)
        {
            ppfdBest = ppfdCandidate;
            bestIndex = i;
            continue;
        }

        if ((ppfd->cDepthBits > ppfdBest->cDepthBits &&
                ppfdCandidate->cDepthBits > ppfdBest->cDepthBits) ||
            (ppfd->cDepthBits <= ppfdCandidate->cDepthBits &&
                ppfdCandidate->cDepthBits < ppfdBest->cDepthBits))
        {
            ppfdBest = ppfdCandidate;
            bestIndex = i;
            continue;
        }

        if (ppfd->cStencilBits &&
                ppfdCandidate->cStencilBits > ppfdBest->cStencilBits)
        {
            ppfdBest = ppfdCandidate;
            bestIndex = i;
            continue;
        }

        if (ppfd->cAuxBuffers &&
                ppfdCandidate->cAuxBuffers > ppfdBest->cAuxBuffers)
        {
            ppfdBest = ppfdCandidate;
            bestIndex = i;
            continue;
        }
    }
    return bestIndex + 1;
}

int WINAPI
wglGetPixelFormat(HDC hDC)
{
    HWND hWnd = WindowFromDC(hDC);
    __WGLdrawablePrivate *wglPriv = __wglFindDrawablePrivate(hDC, hWnd);

    if (wglPriv == NULL) {
        wglPriv = __wglCreateDrawablePrivate(hDC, hWnd);
    }

    return wglPriv->pixelFormat;
}

BOOL WINAPI
wglSetPixelFormat(HDC hDC, int iPixelFormat, const PIXELFORMATDESCRIPTOR *ppfd)
{
    HWND hWnd = WindowFromDC(hDC);
    __WGLdrawablePrivate *wglPriv = __wglFindDrawablePrivate(hDC, hWnd);

    if (pixelFormatsDirty) {
        updatePixelFormats();
        pixelFormatsDirty = FALSE;
    }

    if (wglPriv == NULL) {
        wglPriv = __wglCreateDrawablePrivate(hDC, hWnd);
    }

    /* It's an error to set the pixel format more than once */
    if (wglPriv->pixelFormat != 0) {
        return FALSE;
    }

    /* Make sure the pixelFormat is valid */
    if (iPixelFormat < 1 || iPixelFormat > numPixelFormats) {
        return FALSE;
    }

    /* Make sure the object supports this pixel format */
    switch (GetObjectType(hDC)) {
    case OBJ_DC:
        if (!(pixelFormats[iPixelFormat - 1]->dwFlags & PFD_DRAW_TO_WINDOW)) {
            return FALSE;
        }
        break;
    case OBJ_MEMDC:
        if (!(pixelFormats[iPixelFormat - 1]->dwFlags & PFD_DRAW_TO_BITMAP)) {
            return FALSE;
        }
        break;
    default:
        return FALSE;
    }

    wglPriv->pixelFormat = iPixelFormat;
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* OPT 0.1.4 GL_ARB_multitexture: ARB entry points.  Thin forwards     */
/* onto the existing SGIS multitexture dispatch stubs (identical       */
/* semantics for 2 units; ARB unit enums are GL_TEXTURE0_ARB-based,    */
/* SGIS are TEXTURE0_SGIS-based).  glClientActiveTextureARB is a real  */
/* implementation and lives in glcore (s_varray.c).                    */
/* ------------------------------------------------------------------ */
#define __WGL_ARB_TEX0        0x84C0  /* GL_TEXTURE0_ARB */
#define __WGL_ARB_TEX1        0x84C1  /* GL_TEXTURE1_ARB */
#define __WGL_SGIS_TEX0       0x835E  /* TEXTURE0_SGIS   */
#define __WGL_ARB_TO_SGIS(u)  ((u) - __WGL_ARB_TEX0 + __WGL_SGIS_TEX0)
#define __WGL_ARB_UNIT_OK(u)  ((u) >= __WGL_ARB_TEX0 && (u) <= __WGL_ARB_TEX1)

void APIENTRY glActiveTextureARB( GLenum texture )
{
    if ( !__WGL_ARB_UNIT_OK( texture ) ) return;
    glSelectTextureSGIS( __WGL_ARB_TO_SGIS( texture ) );
}

void APIENTRY glMultiTexCoord1fARB( GLenum target, GLfloat s )
{
    if ( !__WGL_ARB_UNIT_OK( target ) ) return;
    glMTexCoord1fSGIS( __WGL_ARB_TO_SGIS( target ), s );
}

void APIENTRY glMultiTexCoord1fvARB( GLenum target, const GLfloat *v )
{
    if ( !__WGL_ARB_UNIT_OK( target ) ) return;
    glMTexCoord1fvSGIS( __WGL_ARB_TO_SGIS( target ), v );
}

void APIENTRY glMultiTexCoord2fARB( GLenum target, GLfloat s, GLfloat t )
{
    if ( !__WGL_ARB_UNIT_OK( target ) ) return;
    glMTexCoord2fSGIS( __WGL_ARB_TO_SGIS( target ), s, t );
}

void APIENTRY glMultiTexCoord2fvARB( GLenum target, const GLfloat *v )
{
    if ( !__WGL_ARB_UNIT_OK( target ) ) return;
    glMTexCoord2fvSGIS( __WGL_ARB_TO_SGIS( target ), v );
}

void APIENTRY glMultiTexCoord3fARB( GLenum target, GLfloat s, GLfloat t, GLfloat r )
{
    if ( !__WGL_ARB_UNIT_OK( target ) ) return;
    glMTexCoord3fSGIS( __WGL_ARB_TO_SGIS( target ), s, t, r );
}

void APIENTRY glMultiTexCoord3fvARB( GLenum target, const GLfloat *v )
{
    if ( !__WGL_ARB_UNIT_OK( target ) ) return;
    glMTexCoord3fvSGIS( __WGL_ARB_TO_SGIS( target ), v );
}

void APIENTRY glMultiTexCoord4fARB( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q )
{
    if ( !__WGL_ARB_UNIT_OK( target ) ) return;
    glMTexCoord4fSGIS( __WGL_ARB_TO_SGIS( target ), s, t, r, q );
}

void APIENTRY glMultiTexCoord4fvARB( GLenum target, const GLfloat *v )
{
    if ( !__WGL_ARB_UNIT_OK( target ) ) return;
    glMTexCoord4fvSGIS( __WGL_ARB_TO_SGIS( target ), v );
}

typedef struct __WGLEXTPROC {
    LPCSTR szProc;              /* extension function name */
    PROC   Proc;                /* extension function address */
} WGLEXTPROC, *PWGLEXTPROC;

#define __WGL_EXT_TABLE_SIZE (sizeof(__wglExtProcs) / sizeof(__wglExtProcs[0]))
WGLEXTPROC __wglExtProcs[] =
{
    { "glArrayElementEXT"     , (PROC) glArrayElementEXT      },
    { "glDrawArraysEXT"       , (PROC) glDrawArraysEXT        },
    { "glVertexPointerEXT"    , (PROC) glVertexPointerEXT     },
    { "glNormalPointerEXT"    , (PROC) glNormalPointerEXT     },
    { "glColorPointerEXT"     , (PROC) glColorPointerEXT      },
    { "glIndexPointerEXT"     , (PROC) glIndexPointerEXT      },
    { "glTexCoordPointerEXT"  , (PROC) glTexCoordPointerEXT   },
    { "glEdgeFlagPointerEXT"  , (PROC) glEdgeFlagPointerEXT   },
    { "glGetPointervEXT"      , (PROC) glGetPointervEXT       },
    { "glColorSubTableEXT"    , (PROC) glColorSubTableEXT     },
    { "glColorTableEXT"       , (PROC) glColorTableEXT        },
    { "glCopyColorTableEXT"   , (PROC) glCopyColorTableEXT    },
    { "glGetColorTableEXT"    , (PROC) glGetColorTableEXT     },
    { "glGetColorTableParameterfvEXT" , (PROC) glGetColorTableParameterfvEXT },
    { "glGetColorTableParameterivEXT" , (PROC) glGetColorTableParameterivEXT },
    { "glLockArraysSGI" , (PROC) glLockArraysSGI },
    { "glUnlockArraysSGI" , (PROC) glUnlockArraysSGI },
    { "glCullParameterdvSGI" , (PROC) glCullParameterdvSGI },
    { "glCullParameterfvSGI" , (PROC) glCullParameterfvSGI },
    { "glIndexFuncSGI" , (PROC) glIndexFuncSGI },
    { "glIndexMaterialSGI" , (PROC) glIndexMaterialSGI },
    { "glAddSwapHintRectWIN" , (PROC) glAddSwapHintRectWIN },
    { "glPointParameterfEXT", (PROC) glPointParameterfEXT },
    { "glPointParameterfvEXT", (PROC) glPointParameterfvEXT },
    { "glMTexCoord1dSGIS" , (PROC) glMTexCoord1dSGIS },
    { "glMTexCoord1dvSGIS" , (PROC) glMTexCoord1dvSGIS },
    { "glMTexCoord1fSGIS" , (PROC) glMTexCoord1fSGIS },
    { "glMTexCoord1fvSGIS" , (PROC) glMTexCoord1fvSGIS },
    { "glMTexCoord1iSGIS" , (PROC) glMTexCoord1iSGIS },
    { "glMTexCoord1ivSGIS" , (PROC) glMTexCoord1ivSGIS },
    { "glMTexCoord1sSGIS" , (PROC) glMTexCoord1sSGIS },
    { "glMTexCoord1svSGIS" , (PROC) glMTexCoord1svSGIS },
    { "glMTexCoord2dSGIS" , (PROC) glMTexCoord2dSGIS },
    { "glMTexCoord2dvSGIS" , (PROC) glMTexCoord2dvSGIS },
    { "glMTexCoord2fSGIS" , (PROC) glMTexCoord2fSGIS },
    { "glMTexCoord2fvSGIS" , (PROC) glMTexCoord2fvSGIS },
    { "glMTexCoord2iSGIS" , (PROC) glMTexCoord2iSGIS },
    { "glMTexCoord2ivSGIS" , (PROC) glMTexCoord2ivSGIS },
    { "glMTexCoord2sSGIS" , (PROC) glMTexCoord2sSGIS },
    { "glMTexCoord2svSGIS" , (PROC) glMTexCoord2svSGIS },
    { "glMTexCoord3dSGIS" , (PROC) glMTexCoord3dSGIS },
    { "glMTexCoord3dvSGIS" , (PROC) glMTexCoord3dvSGIS },
    { "glMTexCoord3fSGIS" , (PROC) glMTexCoord3fSGIS },
    { "glMTexCoord3fvSGIS" , (PROC) glMTexCoord3fvSGIS },
    { "glMTexCoord3iSGIS" , (PROC) glMTexCoord3iSGIS },
    { "glMTexCoord3ivSGIS" , (PROC) glMTexCoord3ivSGIS },
    { "glMTexCoord3sSGIS" , (PROC) glMTexCoord3sSGIS },
    { "glMTexCoord3svSGIS" , (PROC) glMTexCoord3svSGIS },
    { "glMTexCoord4dSGIS" , (PROC) glMTexCoord4dSGIS },
    { "glMTexCoord4dvSGIS" , (PROC) glMTexCoord4dvSGIS },
    { "glMTexCoord4fSGIS" , (PROC) glMTexCoord4fSGIS },
    { "glMTexCoord4fvSGIS" , (PROC) glMTexCoord4fvSGIS },
    { "glMTexCoord4iSGIS" , (PROC) glMTexCoord4iSGIS },
    { "glMTexCoord4ivSGIS" , (PROC) glMTexCoord4ivSGIS },
    { "glMTexCoord4sSGIS" , (PROC) glMTexCoord4sSGIS },
    { "glMTexCoord4svSGIS" , (PROC) glMTexCoord4svSGIS },
    { "glMTexCoordPointerSGIS" , (PROC) glMTexCoordPointerSGIS },
    { "glSelectTextureSGIS" , (PROC) glSelectTextureSGIS },
    { "glSelectTextureCoordSetSGIS" , (PROC) glSelectTextureCoordSetSGIS },
    /* OPT 0.1.4 GL_ARB_multitexture */
    { "glActiveTextureARB" , (PROC) glActiveTextureARB },
    { "glClientActiveTextureARB" , (PROC) glClientActiveTextureARB },
    { "glMultiTexCoord1fARB" , (PROC) glMultiTexCoord1fARB },
    { "glMultiTexCoord1fvARB" , (PROC) glMultiTexCoord1fvARB },
    { "glMultiTexCoord2fARB" , (PROC) glMultiTexCoord2fARB },
    { "glMultiTexCoord2fvARB" , (PROC) glMultiTexCoord2fvARB },
    { "glMultiTexCoord3fARB" , (PROC) glMultiTexCoord3fARB },
    { "glMultiTexCoord3fvARB" , (PROC) glMultiTexCoord3fvARB },
    { "glMultiTexCoord4fARB" , (PROC) glMultiTexCoord4fARB },
    { "glMultiTexCoord4fvARB" , (PROC) glMultiTexCoord4fvARB },
    { "wglLockBuffers" , (PROC) wglLockBuffers },
    { "wglUnlockBuffers" , (PROC) wglUnlockBuffers },
};

/* OPT 0.1.4: shared extension-proc lookup, used by wglGetProcAddress and
** by the ICD entry DrvGetProcAddress (the path MS opengl32.dll uses). */
PROC __wglFindExtProc(LPCSTR lpszProc)
{
    CONST CHAR *pch1, *pch2;
    int   i;

    if (lpszProc == NULL) {
        return (PROC) NULL;
    }

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

PROC WINAPI
wglGetProcAddress(LPCSTR lpszProc)
{
    /* Return error if there is no current RC. */
    if (GET_CURRENT_GC == __wglInvalidGC)  {
        __wglSetSystemError("wglGetProcAddress", WGL_INVALID_HRC);
        return (PROC) NULL;
    }

    /* Return extension function address if it is found. */
    return __wglFindExtProc(lpszProc);
}

PROC WINAPI
wglGetDefaultProcAddress(LPCSTR lpszProc)
{
    __wglSetSystemError("wglGetDefaultProcAddress not supported", WGL_ERROR);
    return NULL;
}

HGLRC WINAPI
wglCreateLayerContext(HDC hDC, int iLayerPlane)
{
    __wglSetSystemError("wglCreateLayerContext not supported", WGL_ERROR);
    return 0;
}

BOOL WINAPI
wglDescribeLayerPlane(HDC hDC, int iPixelFormat, int iLayerPlane,
                      UINT nBytes, LPLAYERPLANEDESCRIPTOR *plpd)
{
    __wglSetSystemError("wglDescribeLayerPlane not supported", WGL_ERROR);
    return FALSE;
}

int WINAPI
wglGetLayerPaletteEntries(HDC hDC, int iLayerPlane,
                          int iStart, int cEntries, COLORREF *par)
{
    __wglSetSystemError("wglGetLayerPaletteEntries not supported", WGL_ERROR);
    return 0;
}

BOOL WINAPI
wglRealizeLayerPalette(HDC hDC, int iLayerPlane, BOOL bRealize)
{
    __wglSetSystemError("wglRealizeLayerPalette not supported", WGL_ERROR);
    return FALSE;
}

int WINAPI
wglSetLayerPaletteEntries(HDC hDC, int iLayerPlane,
                          int iStart, int cEntries, const COLORREF *par)
{
    __wglSetSystemError("wglSetlayerPaletteEntries not supported", WGL_ERROR);
    return 0;
}

BOOL WINAPI
wglSwapLayerBuffers(HDC hDC, UINT fuPlanes)
{
    __wglSetSystemError("wglSwapLayerBuffers not supported", WGL_ERROR);
    return FALSE;
}

/*****************************************************************************
**
**  __wglMonitorWindowChanges.
**
**  Callback function that parses the message stream looking for messages 
**  of interest to the wgl drawable management code.
**
****************************************************************************/

unsigned long tacoHackGlideInit = 0;
/* RETRO3DFX: when set (env RETRO3DFX_NODITHER), keep VSA-100 ordered dither
** off so flat-color 2D UI text renders solid (see sst_export.c MakeCurrent). */
int __r3d_nodither = 0;

LRESULT CALLBACK
__wglMonitorWindowChanges(int code, WPARAM wParam, LPARAM lParam)
{
    __GLcontextArea *contextArea = TlsGetValue(__wglTLSIndex);
    HHOOK messageHook = (HHOOK) contextArea->other;
    CWPSTRUCT *callBackMessage = (CWPSTRUCT *) lParam;

    /*
    ** Examine the message and see if it is a window change type.
    ** Then look to see if is a window associated with an openGL
    ** context.  If it is adjust its buffers, viewport, and scissor
    ** area.  Note that there may be more than one context attached
    ** to a single window.
    */
    switch (callBackMessage->message) {
    case WM_CLOSE:
      wglMakeCurrent( 0, 0 );
      break;
    case WM_ACTIVATE:
        if (!__glIgnoreActivate && tacoHackGlideInit ) {
            if ( LOWORD( callBackMessage->wParam ) == WA_INACTIVE ) {
                grDisable( GR_PASSTHRU );
            } else {
                grEnable( GR_PASSTHRU );
            }
        }
        break;
    case WM_WINDOWPOSCHANGING:
        {
            static int b;
            WINDOWPOS *wp = (WINDOWPOS*)callBackMessage->lParam;
            if ( wp->flags & SWP_HIDEWINDOW && (__glIgnoreActivate == 2 )) {
                /* loathsome hacke for voodoo rush */
                wglMakeCurrent( 0, 0 );
            }
        }
        break;
    case WM_SIZE:
        /* XXX taco need some implementation */
        break;
    case WM_WINDOWPOSCHANGED:
    case WM_PALETTECHANGED:
        {
            WGLCONTEXT *glrc = NULL;
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
                    __wglUpdateDrawableBuffers(glrc);
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
                pixelFormatsDirty = TRUE;
            }
        }
        break;
    default:
        break;
    }   
    return CallNextHookEx(messageHook, code, wParam, lParam);
}

/************************************************************/
/* DLL and Initialization Stuff */

HINSTANCE hInstanceOpenGL;

BOOL __wglOSWin95;

DWORD __wglTLSIndex = TLS_MINIMUM_AVAILABLE;
DWORD __wglTLSOffset = 0;

static BOOL shareProcessContextArea;
static __GLcontextArea *processContextArea;

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
#if defined(__GL_OLD_THREAD)
    return __gl_context;
#else
    __GLcontextArea *contextArea = __glGetThreadValue();

    if (contextArea == NULL) {
        __wglAttachThread(GetCurrentThreadId());
        contextArea = __glGetThreadValue();
    }

    assert(contextArea != NULL);
    return contextArea->context;
#endif
}

BOOL
__wglAttachThread(DWORD hThread)
{
    __GLcontextArea *contextArea = processContextArea;
    HHOOK messageHook;

    /* Setup context area */
    if (shareProcessContextArea && contextArea != NULL) {
        TlsSetValue(__wglTLSIndex, (LPVOID) contextArea);
    } else {
        contextArea = (__GLcontextArea *) __wglCalloc(1, sizeof(*contextArea));
        if (contextArea == NULL) {
            __wglSetSystemError("__wglAttachThread", WGL_MALLOC_FAILED);
            return FALSE;
        }
        TlsSetValue(__wglTLSIndex, (LPVOID) contextArea);
        SET_CURRENT_GC(__wglInvalidGC);
        __glCoreNopDispatch();

        processContextArea = contextArea;
    }

    /* Setup message hook */
    if (!shareProcessContextArea) {
        messageHook = SetWindowsHookEx(WH_CALLWNDPROC,
                                (HOOKPROC) __wglMonitorWindowChanges,
                                hInstanceOpenGL, hThread);
        if (messageHook == NULL) {
            __wglSetSystemError("__wglAttachThread", WGL_HOOK_FAILED);
            return FALSE;
        }
        contextArea->other = (void *) messageHook;
    }

    return TRUE;
}

BOOL
__wglDetachThread(HINSTANCE hInst, DWORD hThread)
{
    __GLcontextArea *contextArea = TlsGetValue(__wglTLSIndex);

    if (contextArea != NULL) {
        /* Remove message hook */
        if (contextArea->other != NULL) {
            HHOOK messageHook = (HHOOK) contextArea->other;

            if (messageHook) {
                if (UnhookWindowsHookEx(messageHook) == FALSE) {
                    return FALSE;
                } 
                contextArea->other = NULL;
            }
        }

        /* Remove context area */
        if (contextArea != processContextArea) {
            if (contextArea->context != __wglInvalidGC) {
                /* XXX lose current context */
            }
            __wglFree(contextArea);
        }
        TlsSetValue(__wglTLSIndex, (LPVOID) 0);
    }

    return TRUE;
}

BOOL
__wglAttachProcess(HINSTANCE hInst, DWORD hThread)
{
  // Init OS services
  __wglCreateMutex();
  __wglCreateCriticalSection();
  
  // Set up error logging level
  __wglInitErrorLog();
  
  // Thread Local Storage Allocation
  // Including hack for fast TLS Access
  __wglTLSIndex  = TlsAlloc();
  if ( __wglOSWin95 ) {
    __wglTLSOffset = W95_TLS_INDEX_TO_OFFSET(__wglTLSIndex);
  } else {
    __wglTLSOffset = WNT_TLS_INDEX_TO_OFFSET(__wglTLSIndex);
  }
  
  // Set up frame buffer routine dispatch
  if (__wglFBInit() == FALSE) {
    return FALSE;
  }

  // Init Context linked list for this process
  __wglInitializeContextList();

  return __wglAttachThread(hThread);
}

BOOL
__wglDetachProcess(HINSTANCE hInst, DWORD hThread)
{
    processContextArea = NULL;
    if (__wglDetachThread(hInst, hThread) == FALSE) {
        return FALSE;
    }
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

    // !!TACO 
    // Always has to be 1 - if this is ever vectored through opengl32.dll then we
    // are in a gnarly loop as we are being called by opengl32.dll after all
    dispatch = 1;

    if (env) {
        dispatch = atoi(env);
    }

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

#include "db_trace.h"

#if defined( __GL_SST )
BOOL WINAPI DllMain( HINSTANCE hInst,
                     DWORD     Reason,
                     LPVOID    Reserved ) {
  BOOL       rv = FALSE;            // Return Value
  static int refcount = 0;          // We will only allow one process to load this 
                                    // dll at a time until we handle multi-process
  /* JDT:  this is based on my *FALSE* assumption that module statics are DLL state
     when they are actually process state, this code basically does nothing */

  hInstanceOpenGL = hInst;

  switch (Reason) {
  case DLL_PROCESS_ATTACH:    /* Process started up. */
    OGLLOG( "3dfxogl DLL_PROCESS_ATTACH pid=%lu tid=%lu",
            GetCurrentProcessId(), GetCurrentThreadId() );
    if ( !refcount ) {
      refcount++;

      DisableThreadLibraryCalls( hInst );  // No Thread Attach/Detach Messages

      if ( __wglGetOS() == FALSE ) {
        OGLLOG( "3dfxogl ATTACH FAIL: __wglGetOS returned FALSE" );
        rv = FALSE;
        break;
      }

      if ( !__wglAttachProcess( hInst, GetCurrentThreadId() ) ) {
        OGLLOG( "3dfxogl ATTACH FAIL: __wglAttachProcess returned FALSE" );
        rv = FALSE;
        break;
      }
      OGLLOG( "3dfxogl attach: OS ok, process attached, sstUseHardware=%d",
              sstUseHardware );

      if ( getenv("SST_USESW") ) {
        sstUseHardware = GL_FALSE;
      }

      if( sstUseHardware ) {
          char *v;
          OSVERSIONINFO osinfo;

          if (v = getenv("SST_DUALHEAD")) {
              if (atoi(v)) {
                  __glIgnoreActivate = 1;
              }
          }
          if (v = getenv("SSTV2_DUALHEAD")) {
              if (atoi(v)) {
                  __glIgnoreActivate = 1;
              }
          }

        osinfo.dwOSVersionInfoSize = sizeof(osinfo);
        GetVersionEx(&osinfo);
        OGLLOG( "3dfxogl attach: platformId=%lu (1=Win9x 2=NT)",
                osinfo.dwPlatformId );
        if (osinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
            FreeLibrary(GetModuleHandle("GLIDE3X"));
            putenv("SST_DUALHEAD=1");
            putenv("SSTV2_DUALHEAD=1");
            LoadLibrary("GLIDE3X");
        }
        OGLLOG( "3dfxogl attach: calling grGlideInit()" );
        grGlideInit();
        OGLLOG( "3dfxogl attach: grGlideInit() returned" );

        {
          const char *hwstr;
          OGLLOG( "3dfxogl attach: calling grGetString(GR_HARDWARE)" );
          hwstr = grGetString( GR_HARDWARE );
          OGLLOG( "3dfxogl attach: GR_HARDWARE = '%s'",
                  hwstr ? hwstr : "(null)" );
          if ( hwstr && !strcmp( hwstr, "VoodooRush" ) ) {
              __glIgnoreActivate = 2;
          }
        }

        OGLLOG( "3dfxogl attach: calling grSstSelect(0)" );
        grSstSelect(0);
        OGLLOG( "3dfxogl attach: grSstSelect(0) returned" );
      }

      switch (__wglInitializeDispatch(hInst)) {
      case 0:
        rv = TRUE;
        break;
      case 1:
        shareProcessContextArea = TRUE;
        rv = TRUE;
        break;
      case 2:
        OGLLOG( "3dfxogl ATTACH FAIL: __wglInitializeDispatch returned 2" );
        rv = FALSE;
        break;
      }
      OGLLOG( "3dfxogl DLL_PROCESS_ATTACH done rv=%d", rv );
    } else {
      /* Disallow multiple processes from loading the GL */
      OGLLOG( "3dfxogl ATTACH FAIL: refcount!=0, second load refused" );
      rv = FALSE;
    }
    break;
  case DLL_PROCESS_DETACH:    /* Process closed down. */
    OGLLOG( "3dfxogl DLL_PROCESS_DETACH pid=%lu", GetCurrentProcessId() );
    refcount--;
    wglMakeCurrent( 0, 0 );
#ifdef __GL_BUILD_TRACE
    if (__gl_debug_trace) {
      __gldb_CollectOverallStats();
    }
#endif
    __wglTerminateDispatch();
    rv = __wglDetachProcess(hInst, GetCurrentThreadId());
    break;
  }
  return rv;
}
#else
BOOL WINAPI
DllMain(
    HINSTANCE hInst,
    DWORD Reason,
    LPVOID Reserverd)
{
    hInstanceOpenGL = hInst;

    switch (Reason) {
    case DLL_PROCESS_ATTACH:    /* Process started up. */
        if (__wglGetOS() == FALSE) {
            return FALSE;
        }
        if (__wglAttachProcess(hInst, GetCurrentThreadId()) == FALSE) {
            return FALSE;
        }
#ifdef __GL_COSMO_BUILD
        switch (__wglInitializeDispatch(hInst)) {
        case 0:
            return TRUE;
        case 1:
            shareProcessContextArea = TRUE;
            return TRUE;
        case 2:
            return FALSE;
        }
#endif
        break;

    case DLL_PROCESS_DETACH:    /* Process closed down. */
#ifdef __GL_COSMO_BUILD
        __wglTerminateDispatch();
#endif
        return __wglDetachProcess(hInst, GetCurrentThreadId());
        break;

    case DLL_THREAD_ATTACH:     /* Thread has started up. */
        return __wglAttachThread(GetCurrentThreadId());
        break;

    case DLL_THREAD_DETACH:     /* Thread has closed down. */
        return __wglDetachThread(hInst, GetCurrentThreadId());
        break;

    default:
        break;
    }
    return TRUE;
}
#endif


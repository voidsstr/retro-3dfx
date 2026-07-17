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

#ifndef __wglhrc_h_
#define __wglhrc_h_

#ifndef __gl_core_h_
typedef struct __GLinterfaceRec __GLcontext;
#endif /* __gl_core_h */

#include "glcore.h"
#include "os.h"

typedef void *DHGLRC;
typedef void *PGLCLTPROCTABLE;
typedef void (APIENTRY *PFN_SETPROCTABLE)(PGLCLTPROCTABLE);

typedef struct __WGLdrawablePrivateRec {
    /*
    ** linked list of drawable private structs
    */
    struct __WGLdrawablePrivateRec *last;
    struct __WGLdrawablePrivateRec *next;

    /*
    ** Device context and window for this drawable.  hWnd can
    ** be NULL if the device context is a memory device context.
    */
    HDC hDC;
    HWND hWnd;

    /*
    ** Position and dimensions of the drawable.
    */
    POINT origin;
    RECT rect;
    DWORD width, height;

    int pixelFormat;

    LPRGNDATA swapRgn;
    unsigned int swapSize;

    RECT coreClipRect;
    BOOL coreClipRectChanged;

    void (*freeBuffers)(struct __WGLdrawablePrivateRec *);
    void (*updatePalette)(struct __WGLdrawablePrivateRec *);
    GLboolean (*swapBuffers)(struct __WGLdrawablePrivateRec *);

    /* Swapbuffers */
    GLboolean (*swapBuffersMain)(struct __WGLdrawablePrivateRec *);
    GLboolean (*swapBuffersFallback)(struct __WGLdrawablePrivateRec *);

    __GLdrawablePrivate glPriv;

    int refCount;
} __WGLdrawablePrivate;

typedef struct  __WGLcontextRec {
    struct __WGLcontextRec	*last;			/* Linked list */
    struct __WGLcontextRec	*next;
    __GLcontext			*gc;			/* The context used by OpenGL. */
    __GLcontextModes		*modes;			/* type of context */
    GLboolean			isInUse;		/* used to exclude other threads. */
    DWORD			owner;			/* Thread ID of the current owner. */
    GLboolean			hasBeenCurrent;		/* Used for one-off initialisation. */
    GLboolean			pendingDestroy;		/* Cleanup needed. */
    GLboolean			pendingWindowChange;	/* Indicates a change in window size/pos. */
    GLboolean			isDirect;		/* Direct connection flag. */
    HGLRC			hGLRC;			/* The HGLRC handle associated with context. */
    HWND			hWnd;			/* The window handle associated with this context. */
    HDC				hDC;			/* The HDC associated with the context. */
    PIXELFORMATDESCRIPTOR	pFD;			/* Pixel configuration structure */
    int				pixelFormat;		/* Pixel Format Index. */
    PFN_SETPROCTABLE		pfnSetProcTable;	/* Hook to setup the dispatch table */
    __WGLdrawablePrivate	*wglPriv;		/* drawable private */
} __WGLcontext;

#define	__WGL_NO_OWNER	(-1)

extern __GLcontext * __wglGetCurrentGC(void);
extern void __wglSetCurrentGC(__GLcontext *);

#endif /* __wglhrc_h_ */


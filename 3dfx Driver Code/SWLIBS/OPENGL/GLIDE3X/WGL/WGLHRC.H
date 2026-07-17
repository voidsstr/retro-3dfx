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
 *                         GLiNT                                        *
 *                                                                      *
 *                                                                      *
 *          Copyright (C) 1994  3Dlabs Inc. Ltd                         *
 *                                                                      *
 *                                                                      *
 * This software and its associated documentation contains proprietary, *
 * confidential and trade secret information of DuPont Pixel Systems    *
 * Limited and except as provided by written agreement with DuPont      *
 * Pixel Systems Limited                                                *
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

/************************************************************************
*
* Name: wglhrc.h
*
* Description:
*
* This file contains structures and definitions needed to interface to
* the WGL API and OpenGL.
*
* Author: P. Huxley
*
* See SCCS for History.
*
************************************************************************/

#ifndef __wglhrc_h_
#define __wglhrc_h_

typedef struct __GLinterfaceRec __GLcontext;
#include "glcore.h"
#include "os.h"

/** WGL context structure **********************************************************************
**
** This structure collects together all of the information which is relevant to a given context.
** It's allocation, deletion and places where it needs to be set will be done through the use
** of a lock.  The isInUse flag will indicate that a thread is using the context and will stop
** any other thread from using it.
**
************************************************************************************************/ 

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
    BOOL (*swapBuffers)(struct __WGLdrawablePrivateRec *);

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
    __WGLdrawablePrivate	*wglPriv;		/* drawable private */
} __WGLcontext;

typedef __WGLcontext WGLCONTEXT;

#define	NO_OWNER	(-1)

extern __GLcontext * __wglGetCurrentGC(void);
#define	GET_CURRENT_GC				(__wglGetCurrentGC())
#define SET_CURRENT_GC(gc)			(__gl_context = (gc))

#endif /* __wglhrc_h_ */


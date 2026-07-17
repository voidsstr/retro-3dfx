#ifndef __gl_os_h_
#define __gl_os_h_

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

#include <stdlib.h>
#include <assert.h>

#if defined(WIN32)
#if !defined(APIENTRY)
#define APIENTRY __stdcall
#endif
#if !defined(FASTCALL)
#define FASTCALL __fastcall
#endif
#pragma warning(disable: 4244 4245)
#else /* defined(WIN32) */
#define APIENTRY
#define FASTCALL
#endif /* defined(WIN32) */
#define WINGDIAPI

#include <GL/gl.h>
#include "glthread.h"
#include "g_disp.h"

typedef enum __GLbeginModeEnum {
    __GL_NOT_IN_BEGIN = 0,
    __GL_IN_BEGIN = 1,
    __GL_NEED_VALIDATE = 2
} __GLbeginMode;

typedef struct __GLcontextArea {
    __GLcontext *context;
    __GLbeginMode beginMode;
    __GLdispatchState dispatch;
    __GLdispatchState *dispatchOverride;
    void *other;				/* os system thread data */
} __GLcontextArea;

#if defined(__GL_OLD_THREAD)
extern __GLcontextArea __gl_contextArea;
#define __gl_context (__gl_contextArea.context)
#define __gl_beginMode (__gl_contextArea.beginMode)
#define __gl_dispatch (__gl_contextArea.dispatch)
#define __gl_dispatchOverride (__gl_contextArea.dispatchOverride)
#else
#define __gl_context (((__GLcontextArea *) __glGetThreadValue())->context)
#define __gl_beginMode (((__GLcontextArea *) __glGetThreadValue())->beginMode)
#define __gl_dispatch (((__GLcontextArea *) __glGetThreadValue())->dispatch)
#define __gl_dispatchOverride (((__GLcontextArea *) __glGetThreadValue())->dispatchOverride)
#endif

#endif /* __gl_os_h_ */

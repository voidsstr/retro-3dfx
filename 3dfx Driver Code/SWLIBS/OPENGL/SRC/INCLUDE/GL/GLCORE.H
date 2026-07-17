#ifndef __gl_core_h_
#define __gl_core_h_

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
#include <sys/types.h>

/*
** This file defines the interface between the GL core and the surrounding
** "operating system" that supports it (currently the GLX or WGL extensions).
**
** Members (data and function pointers) are documented as imported or
** exported according to how they are used by the core rendering functions.
** Imported members are initialized by the "operating system" and used by
** the core functions.  Exported members are initialized by the core functions
** and used by the "operating system".
*/

/*
** Mode and limit information for a context.  This information is
** kept around in the context so that values can be used during
** command execution, and for returning information about the
** context to the application.
*/
typedef struct __GLcontextModesRec {
    GLboolean rgbMode;
    GLboolean colorIndexMode;
    GLboolean doubleBufferMode;
    GLboolean stereoMode;

    GLboolean haveAccumBuffer;
    GLboolean haveDepthBuffer;
    GLboolean haveStencilBuffer;

    GLint redBits, greenBits, blueBits, alphaBits;	/* bits per comp */
    GLuint redMask, greenMask, blueMask, alphaMask;
    GLint rgbBits;		/* total bits for rgb */
    GLint indexBits;		/* total bits for colorindex */

    GLint accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits;
    GLint depthBits;
    GLint stencilBits;

    GLint numAuxBuffers;

    GLint level;

    GLint pixmapMode;
} __GLcontextModes;

/************************************************************************/

/*
** Structure used for allocating and freeing drawable private memory.
** (like software buffers, for example).
**
** The memory allocation routines are provided by the surrounding
** "operating system" code, and they are to be used for allocating
** software buffers and things which are associated with the drawable,
** and used by any context which draws to that drawable.  There are
** separate memory allocation functions for drawables and contexts
** since drawables and contexts can be created and destroyed independently
** of one another, and the "operating system" may want to use separate
** allocation arenas for each.
**
** The freePrivate function is filled in by the core routines when they
** allocates software buffers, and stick them in "private".  The freePrivate
** function will destroy anything allocated to this drawable (to be called
** when the drawable is destroyed).
*/
typedef struct __GLdrawableRegionRec __GLdrawableRegion;
typedef struct __GLdrawableBufferRec __GLdrawableBuffer;
typedef struct __GLdrawablePrivateRec __GLdrawablePrivate;

typedef struct __GLregionRectRec {
    /* lower left (inside the rectangle) */
    GLint x0, y0;
    /* upper right (outside the rectangle) */
    GLint x1, y1;
} __GLregionRect;

struct __GLdrawableRegionRec {
    GLint numRects;
    __GLregionRect *rects;
    __GLregionRect boundingRect;
};

/************************************************************************/

/* masks for the buffers */
#define __GL_FRONT_BUFFER_MASK		0x00000001
#define	__GL_FRONT_LEFT_BUFFER_MASK	0x00000001
#define	__GL_FRONT_RIGHT_BUFFER_MASK	0x00000002
#define	__GL_BACK_BUFFER_MASK		0x00000004
#define __GL_BACK_LEFT_BUFFER_MASK	0x00000004
#define __GL_BACK_RIGHT_BUFFER_MASK	0x00000008
#define	__GL_ACCUM_BUFFER_MASK		0x00000010
#define	__GL_DEPTH_BUFFER_MASK		0x00000020
#define	__GL_STENCIL_BUFFER_MASK	0x00000040
#define	__GL_OWNERSHIP_BUFFER_MASK	0x00000080
#define	__GL_AUX_BUFFER_MASK(i)		(0x0000100 << (i))

#define __GL_ALL_BUFFER_MASK		0xffffffff

/* what Update routines return if update resorted to fallback case */
#define __GL_BUFFER_FALLBACK	0x10

typedef void (*__GLbufFallbackInitFn)(__GLdrawableBuffer *buf, 
				      __GLdrawablePrivate *glPriv, GLint bits);
typedef void (*__GLbufMainInitFn)(__GLdrawableBuffer *buf, 
				  __GLdrawablePrivate *glPriv, GLint bits,
				  __GLbufFallbackInitFn back);

struct __GLdrawableBufferRec {
    GLint width, height, depth;
    void *base;		/* base of framebuffer */

    GLuint size, elementSize;
    GLint byteWidth;

    void *handle;

    GLint lockCnt;
 
/* XXXXX   I hate to hell that I have to put this here, but SGI didn't have a
	clue that we would have to have read/write locks.  I can't wait until
	our next generation!!!	*/
    GLint readlock, writelock;

    /* imported */
    GLboolean (*update)(__GLdrawableBuffer *, __GLdrawablePrivate *, GLuint);
    void (*lock)(__GLdrawableBuffer *, __GLdrawablePrivate *);
    void (*unlock)(__GLdrawableBuffer *, __GLdrawablePrivate *);
    void (*fill)(__GLdrawableBuffer *, __GLdrawablePrivate *,
    		GLuint val, GLint x, GLint y, GLint w, GLint h);
    void (*free)(__GLdrawableBuffer *, __GLdrawablePrivate *);
    void *other;

    /* exported */
    void (*freePrivate)(__GLdrawableBuffer *, __GLdrawablePrivate *);
    void *private;

    /* private */
    __GLbufMainInitFn mainInit;
    __GLbufFallbackInitFn fallbackInit;
};

struct __GLdrawablePrivateRec {
    __GLcontextModes *modes;
    GLint width, height;
    GLint yInverted;
    GLint displayBuffer;

    __GLdrawableBuffer frontBuffer;
    __GLdrawableBuffer backBuffer;
    __GLdrawableBuffer accumBuffer;
    __GLdrawableBuffer depthBuffer;
    __GLdrawableBuffer stencilBuffer;
    __GLdrawableBuffer *auxBuffer;

    __GLdrawableBuffer ownershipBuffer;
    __GLdrawableRegion ownershipRegion;

    /* imported */
    void *(*malloc)(size_t size);
    void *(*calloc)(size_t numElem, size_t elemSize);
    void *(*realloc)(void *oldAddr, size_t newSize);
    void (*free)(void *addr);
    void (*addSwapRect)(__GLdrawablePrivate *, GLint, GLint, GLsizei, GLsizei);
    void (*setClipRect)(__GLdrawablePrivate *, GLint, GLint, GLsizei, GLsizei);
    void *other;

    /* exported */
    void (*freePrivate)(__GLdrawablePrivate *);
    void *private;
};

/*
** Procedures which are imported by the GL from the surrounding
** "operating system".  Math functions are not considered part of the
** "operating system".
*/
typedef struct __GLimportsRec {
    /* Memory management */
    void *(*malloc)(__GLcontext *gc, size_t size);
    void *(*calloc)(__GLcontext *gc, size_t numElem, size_t elemSize);
    void *(*realloc)(__GLcontext *gc, void *oldAddr, size_t newSize);
    void (*free)(__GLcontext *gc, void *addr);

    /* Error handling */
    void (*warning)(__GLcontext *gc, const char *fmt, ...);
    void (*fatal)(__GLcontext *gc, const char *fmt, ...);

    /* Drawing surface management */
    __GLdrawablePrivate *(*getDrawablePrivate)(__GLcontext *gc);
    void (*getDrawableSize)(__GLcontext *gc, GLint *w, GLint *h);

    /* Device Mapping/Permissions/ Enabling */
    
    /* Operating system dependent data goes here */
    void *other;
} __GLimports;

/************************************************************************/

/*
** Procedures which are exported by the GL to the surrounding "operating
** system" so that it can manage multiple GL context's.
*/
typedef struct __GLexportsRec {
    /* Context management (return GL_FALSE on failure) */
    GLboolean (*destroyContext)(__GLcontext *gc);
    GLboolean (*loseCurrent)(__GLcontext *gc);
    GLboolean (*makeCurrent)(__GLcontext *gc);
    GLboolean (*shareContext)(__GLcontext *gc, __GLcontext *gcShare);
    GLboolean (*copyContext)(__GLcontext *dst, const __GLcontext *src, GLuint mask);

    /* Swap the front and back buffers */
    void (*swapBuffers)(__GLcontext *gc);

    /* Drawing surface management */
    GLboolean (*changeDrawableSize)(__GLcontext *gc, GLint w, GLint h);
    GLboolean (*changeDrawableLocation)(__GLcontext *gc,
					GLint x, GLint y, GLint w, GLint h);
#if defined(__GL_SUPPORT_MGL)
    void (*changeBuffers)(__GLcontext *gc);
#endif
    void (*lockBuffers)(__GLcontext *gc);
    void (*unlockBuffers)(__GLcontext *gc);

    /* Dispatch table override control for external agents like libGLS */
    struct __GLdispatchStateRec* (*dispatchExec)(__GLcontext *gc);
    void (*beginDispatchOverride)(__GLcontext *gc);
    void (*endDispatchOverride)(__GLcontext *gc);
} __GLexports;

/************************************************************************/

/*
** This must be the first member of a __GLcontext structure.  This is the
** only part of a context that is exposed to the outside world; everything
** else is opaque.
*/
typedef struct __GLinterfaceRec {
    __GLimports imports;
    __GLexports exports;
} __GLinterface;

extern __GLcontext *__glCoreCreateContext(__GLimports *, __GLcontextModes *);
extern void __glCoreNopDispatch(void);

#endif /* __gl_core_h_ */

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

#include "fmemclr.h"

/* --------------------------------------------------------------- */

#define	BUF_ALIGN	32	/* x86 cache alignment */
#define BUF_ALIGN_MINUS_1  (BUF_ALIGN - 1)

static GLboolean
Update(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint bufferMask)
{
    __WGLdrawablePrivate *wglPriv = (__WGLdrawablePrivate *) glPriv->other;
    GLuint newSize;
    void *ubase;

    /*
    ** Note: buf->handle points to unaligned base.
    ** buf->base points to aligned base.
    */

    __wglUpdateDrawableSize(wglPriv);

    newSize = wglPriv->width * wglPriv->height * buf->elementSize;
    if (newSize > buf->size) {
	if (buf->handle) {
	    ubase = (*glPriv->realloc)(buf->handle, newSize + BUF_ALIGN_MINUS_1);
	    if (ubase == NULL) {
		__wglError("Mem: Update: Realloc failed");
		return GL_FALSE;
	    }
	} else {
	    ubase = (*glPriv->malloc)(newSize + BUF_ALIGN_MINUS_1);
	    if (ubase == NULL) {
		__wglError("Mem: Update: Malloc failed");
		return GL_FALSE;
	    }
	}
	buf->handle = ubase;
	buf->base = (void *)(((size_t)ubase + BUF_ALIGN_MINUS_1) &
			     ~BUF_ALIGN_MINUS_1);
	assert((size_t)buf->base % BUF_ALIGN == 0);

	buf->size = newSize;
    }

    buf->width = wglPriv->width;
    buf->byteWidth = buf->width * buf->elementSize;
    buf->height = wglPriv->height;

    return GL_TRUE;
}

static void
Lock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
Unlock(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
}

static void
Fill(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLuint val,
     GLint x, GLint y, GLint w, GLint h)
{
    __GLcontext *gc = __wglGetCurrentGC();
    unsigned char *ptr;

    ptr = ((unsigned char *) buf->base) + 
	(y * buf->width + x) * buf->elementSize;

    assert( buf->byteWidth - buf->width * buf->elementSize >= 0);

    switch(buf->elementSize) {
      case 1:	 /* 8-bit frame buffers */
	  FastMemClear1(gc,
			buf->base,
			w, h,
			buf->byteWidth - buf->width,
			val);
	  break;
      case 2:	/* 16-bit frame buffers */
	  FastMemClear2(gc,
			buf->base,
			w, h,
			(buf->byteWidth >> 1) - buf->width,
			val);
	  break;
      case 3:	/* 24-bit frame buffers */
	  /* oops..  We don't have this yet */
	  assert(0);
	  break;
      case 4:	/* 32-bit frame buffers */
	  FastMemClear4(gc,
			buf->base,
			w, h,
			(buf->byteWidth >> 2) - buf->width,
			val);
	  break;
      default:
	  /* not supported */
	  assert(0);
	  break;
    }
}

static void
Free(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv)
{
    if (buf->handle) {
	(*glPriv->free)(buf->handle);
	buf->handle = NULL;
    }
}

void
__wglInitMem(__GLdrawableBuffer *buf, __GLdrawablePrivate *glPriv, GLint bits)
{
    buf->depth = bits;
    buf->width = buf->height = 0;	/* not assigned yet */
    buf->handle = buf->base = NULL;	/* not assigned yet */
    buf->size = 0;
    buf->byteWidth = 0;

    buf->elementSize = ((bits-1) / 8) + 1;

    buf->update = Update;
    buf->lock = Lock;
    buf->unlock = Unlock;
    buf->fill = Fill;
    buf->free = Free;
}


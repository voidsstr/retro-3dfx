/*
** Copyright 1991, Silicon Graphics, Inc.
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
**
** $Revision: 2$
** $Date: 10/11/00 8:03:46 PM$
*/
#include "context.h"
#include "global.h"
#include "glmath.h"


/*
** initialize the buffer stuff
*/
/* ARGSUSED */
void __glInitBuffer(__GLbuffer *buf, __GLcontext *gc)
{
    buf->elementSize = 1;

    buf->makeCurrent = __glMakeCurrentBuffer;
    buf->loseCurrent = __glLoseCurrentBuffer;

    buf->resize = __glResizeBuffer;
    buf->move = __glMoveBuffer;
}

/*
** make a context current
*/
void __glMakeCurrentBuffer(__GLbuffer *buf, __GLdrawableBuffer *drawableBuf,
			   __GLcontext *gc)
{
    if (drawableBuf->update || drawableBuf->base) {
	buf->width = drawableBuf->width;
	buf->height = drawableBuf->height;
	buf->depth = drawableBuf->depth;
	buf->base = drawableBuf->base;
	buf->size = drawableBuf->size;
	buf->elementSize = drawableBuf->elementSize;
	buf->byteWidth = drawableBuf->byteWidth;

	buf->elementSizeLog2 = __glFloorLog2(buf->elementSize);
	buf->outerWidth = buf->byteWidth / buf->elementSize;
    }

    /* now make ours current */
    buf->gc = gc;
    buf->drawableBuf = drawableBuf;
}

/*
** make a context not current
*/
void __glLoseCurrentBuffer(__GLbuffer *buf, __GLcontext *gc)
{
    if (buf->gc == NULL) {
	/* it's already lost...  go away */
	return;
    }
    buf->gc = NULL;
}

/*
** move buffer
*/
void __glMoveBuffer(__GLbuffer *buf, GLint x, GLint y)
{
}

/*
** allocate or resize buffer
*/
GLboolean __glResizeBuffer(__GLbuffer *buf, GLint w, GLint h, GLuint bufferMask)
{
    __GLdrawableBuffer *drawableBuf = buf->drawableBuf;
    __GLdrawablePrivate *dp = buf->gc->drawablePrivate;
    GLboolean status;

    /* let the "operating system" reallocate the buffer */
    status = (*drawableBuf->update)(drawableBuf, dp, bufferMask);
    if (status == GL_FALSE) {
	/* could not resize */
	return GL_FALSE;
    }

    /* copy updated info from the drawablePrivate */
    buf->width = drawableBuf->width;
    buf->height = drawableBuf->height;

    buf->base = drawableBuf->base;
    buf->size = drawableBuf->size;

    buf->elementSize = drawableBuf->elementSize;
    buf->byteWidth = drawableBuf->byteWidth;

    buf->elementSizeLog2 = __glFloorLog2(buf->elementSize);
    buf->outerWidth = buf->byteWidth / buf->elementSize;

    return status;
}

#if defined(__GL_SUPPORT_MGL)
void __glChangeBuffers(__GLcontext *gc)
{
    __GLdrawablePrivate *dp = gc->drawablePrivate;
    __GLdrawableBuffer *drawableBuf;
    __GLbuffer *buf;

    drawableBuf = &dp->frontBuffer;
    buf = &gc->frontBuffer.buf;
    if (drawableBuf->update) {
	buf->base = drawableBuf->base;
	buf->byteWidth = drawableBuf->byteWidth;
	buf->outerWidth = buf->byteWidth / buf->elementSize;
    }

    drawableBuf = &dp->backBuffer;
    buf = &gc->backBuffer.buf;
    if (drawableBuf->update) {
	buf->base = drawableBuf->base;
	buf->byteWidth = drawableBuf->byteWidth;
	buf->outerWidth = buf->byteWidth / buf->elementSize;
    }
}
#endif

/* -------------------------------------------------------------------- */

/*
** routines for keeping track of the system lock
*/

/*
** routines to lock/unlock buffer according to a mask
*/
void __glLockBuffers(__GLcontext *gc, GLuint mask)
{
    __GLdrawablePrivate *dp = gc->drawablePrivate;

    if ((*gc->buffers.lock.obtainSLock)(gc)) return;

    if (mask & __GL_FRONT_BUFFER_MASK) {
	__GLdrawableBuffer *db = gc->front->buf.drawableBuf;
	if (db->lock) {
	    (*db->lock)(db, dp);
	    __GL_BUF_MAKE_CURRENT(&gc->front->buf, db);
	}
    }
    if (mask & __GL_BACK_BUFFER_MASK) {
	__GLdrawableBuffer *db = gc->back->buf.drawableBuf;
	if (db->lock) {
	    (*db->lock)(db, dp);
	    __GL_BUF_MAKE_CURRENT(&gc->back->buf, db);
	}
    }
#if __GL_MAX_AUXBUFFERS > 0
    if (gc->modes.maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < gc->modes.maxAuxBuffers; i++) {
	    if (mask & __GL_AUX_BUFFER_MASK(i)) {
		if (dp->auxBuffer[i].lock) {
		    (*dp->auxBuffer[i].lock)(&dp->auxBuffer[i], dp);
		    __GL_BUF_MAKE_CURRENT(&gc->auxBuffer[i].buf, &dp->auxBuffer[i]);
		}
	    }
	}
    }
#endif /* __GL_MAX_AUXBUFFERS */

    if (mask & __GL_DEPTH_BUFFER_MASK) {
	if (dp->depthBuffer.lock) {
	    (*dp->depthBuffer.lock)(&dp->depthBuffer, dp);
	    __GL_BUF_MAKE_CURRENT(&gc->depthBuffer.buf, &dp->depthBuffer);
	}
    }
    if (mask & __GL_STENCIL_BUFFER_MASK) {
	if (dp->stencilBuffer.lock) {
	    (*dp->stencilBuffer.lock)(&dp->stencilBuffer, dp);
	    __GL_BUF_MAKE_CURRENT(&gc->stencilBuffer.buf, &dp->stencilBuffer);
	}
    }
    if (mask & __GL_ACCUM_BUFFER_MASK) {
	if (dp->accumBuffer.lock) {
	    (*dp->accumBuffer.lock)(&dp->accumBuffer, dp);
	    __GL_BUF_MAKE_CURRENT(&gc->accumBuffer.buf, &dp->accumBuffer);
	}
    }

    if (gc->buffers.lock.lockDevice) (*gc->buffers.lock.lockDevice)(gc);
}

void __glUnlockBuffers(__GLcontext *gc, GLuint mask)
{
    __GLdrawablePrivate *dp = gc->drawablePrivate;

    if (gc->buffers.lock.sLockCnt == 1) {
	if (gc->buffers.lock.unlockDevice) 
	    (*gc->buffers.lock.unlockDevice)(gc);
    }
    if ((*gc->buffers.lock.releaseSLock)(gc)) return;

    if (mask & __GL_FRONT_BUFFER_MASK) {
	__GLdrawableBuffer *db = gc->front->buf.drawableBuf;
	if (db->unlock) {
	    (*db->unlock)(db, dp);
	}
    }
    if (mask & __GL_BACK_BUFFER_MASK) {
	__GLdrawableBuffer *db = gc->back->buf.drawableBuf;
	if (db->unlock) {
	    (*db->unlock)(db, dp);
	}
    }
#if __GL_MAX_AUXBUFFERS > 0
    if (gc->modes.maxAuxBuffers > 0) {
	GLint i;

	for (i=0; i < gc->modes.maxAuxBuffers; i++) {
	    if (mask & __GL_AUX_BUFFER_MASK(i)) {
		if (dp->auxBuffer[i].unlock) {
		    (*dp->auxBuffer[i].unlock)(&dp->auxBuffer[i], dp);
		}
	    }
	}
    }
#endif /* __GL_MAX_AUXBUFFERS */

    if (mask & __GL_DEPTH_BUFFER_MASK) {
	if (dp->depthBuffer.unlock) {
	    (*dp->depthBuffer.unlock)(&dp->depthBuffer, dp);
	}
    }
    if (mask & __GL_STENCIL_BUFFER_MASK) {
	if (dp->stencilBuffer.unlock) {
	    (*dp->stencilBuffer.unlock)(&dp->stencilBuffer, dp);
	}
    }
    if (mask & __GL_ACCUM_BUFFER_MASK) {
	if (dp->accumBuffer.unlock) {
	    (*dp->accumBuffer.unlock)(&dp->accumBuffer, dp);
	}
    }
}

/*
** routines to lock/unlock the renderable buffers.  Slow path.
*/
void __glLockRenderBuffers(__GLcontext *gc)
{
    __glLockBuffers(gc, gc->buffers.lock.renderBufferMask);
}

void __glUnlockRenderBuffers(__GLcontext *gc)
{
    __glUnlockBuffers(gc, gc->buffers.lock.renderBufferMask);
}

/*
** Certain fast paths for locking/unlocking renderable buffers.
*/
void __glLockColorBuffer(__GLcontext *gc)
{
    __GLbuffer *buf = gc->buffers.lock.bufferToLock;
    __GLdrawableBuffer *drawableBuf = buf->drawableBuf;

    if ((*gc->buffers.lock.obtainSLock)(gc)) return;

    assert(drawableBuf->lock);
    (*drawableBuf->lock)(drawableBuf, gc->drawablePrivate);
    __GL_BUF_MAKE_CURRENT(buf, drawableBuf);

    if (gc->buffers.lock.lockDevice) (*gc->buffers.lock.lockDevice)(gc);
}

void __glUnlockColorBuffer(__GLcontext *gc)
{
    __GLbuffer *buf = gc->buffers.lock.bufferToLock;
    __GLdrawableBuffer *drawableBuf = buf->drawableBuf;

    if (gc->buffers.lock.sLockCnt == 1) {
	if (gc->buffers.lock.unlockDevice) 
	    (*gc->buffers.lock.unlockDevice)(gc);
    }
    if ((*gc->buffers.lock.releaseSLock)(gc)) return;

    assert(drawableBuf->unlock);
    (*drawableBuf->unlock)(drawableBuf, gc->drawablePrivate);
}

void __glLockColorBufferDepthBuffer(__GLcontext *gc)
{
    __GLbuffer *buf;
    __GLdrawableBuffer *drawableBuf;

    if ((*gc->buffers.lock.obtainSLock)(gc)) return;

    /* lock depth */
    buf = &gc->depthBuffer.buf;
    drawableBuf = buf->drawableBuf;
    assert(drawableBuf->lock);
    (*drawableBuf->lock)(drawableBuf, gc->drawablePrivate);
    __GL_BUF_MAKE_CURRENT(buf, drawableBuf);

    /* lock color */
    buf = gc->buffers.lock.bufferToLock;
    drawableBuf = buf->drawableBuf;
    assert (drawableBuf->lock);
    (*drawableBuf->lock)(drawableBuf, gc->drawablePrivate);
    __GL_BUF_MAKE_CURRENT(buf, drawableBuf);

    if (gc->buffers.lock.lockDevice) (*gc->buffers.lock.lockDevice)(gc);
}

void __glUnlockColorBufferDepthBuffer(__GLcontext *gc)
{
    __GLbuffer *buf;
    __GLdrawableBuffer *drawableBuf;

    if (gc->buffers.lock.sLockCnt == 1) {
	if (gc->buffers.lock.unlockDevice) 
	    (*gc->buffers.lock.unlockDevice)(gc);
    }
    if ((*gc->buffers.lock.releaseSLock)(gc)) return;

    /* unlock color */
    buf = gc->buffers.lock.bufferToLock;
    drawableBuf = buf->drawableBuf;
    assert(drawableBuf->unlock);
    (*drawableBuf->unlock)(drawableBuf, gc->drawablePrivate);

    /* unlock depth */
    buf = &gc->depthBuffer.buf;
    drawableBuf = buf->drawableBuf;
    assert(drawableBuf->unlock);
    (*drawableBuf->unlock)(drawableBuf, gc->drawablePrivate);
}
    

/* -------------------------------------------------------------------- */

GLboolean __glTestOwnership(__GLcontext *gc, GLint x, GLint y)
{
    GLubyte *p, clipMask, clipBit;

    p = __GL_OWNERSHIP_ADDRESS(&gc->ownershipBuffer, (GLubyte *), x, y);

    clipMask = *p;
    clipBit = 0x80 >> (x & 7);

    return ((clipMask & clipBit) != 0);
}

/*
** free the buffer info stored in the drawable.
** This is called whenever the window will be destroyed
*/
void __glFreePrivate(__GLdrawablePrivate *dp)
{
    if (dp->frontBuffer.freePrivate) {
	(*dp->frontBuffer.freePrivate)(&dp->frontBuffer, dp);
    }
    if (dp->backBuffer.freePrivate) {
	(*dp->backBuffer.freePrivate)(&dp->backBuffer, dp);
    }
    if (dp->accumBuffer.freePrivate) {
	(*dp->accumBuffer.freePrivate)(&dp->accumBuffer, dp);
    }
    if (dp->depthBuffer.freePrivate) {
	(*dp->depthBuffer.freePrivate)(&dp->depthBuffer, dp);
    }
    if (dp->stencilBuffer.freePrivate) {
	(*dp->stencilBuffer.freePrivate)(&dp->stencilBuffer, dp);
    }
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
    if (dp->auxBuffer) {
	int i;
	for (i = 0; i < dp->modes->numAuxBuffers; i++ ) {
	    if (dp->auxBuffer[i].freePrivate) {
		(*dp->auxBuffer[i].freePrivate)(&dp->auxBuffer[i], dp);
	    }
	}
	(*dp->free)(buffers->auxBuffer);
    }
#endif
    if (dp->ownershipBuffer.freePrivate) {
	(*dp->ownershipBuffer.freePrivate)(&dp->ownershipBuffer, dp);
    }
    dp->private = NULL;
}

#define __GL_SET_HW_BUFFER_FLAG(bm) \
	if (status == GL_FALSE) return GL_FALSE; \
	gc->buffers.lock.hwBufferMask |= bm; \
	if (status == __GL_BUFFER_FALLBACK) \
		gc->buffers.lock.hwBufferMask &= ~(bm);
    

/*
** resize all buffers in a context
*/
GLboolean __glChangeWindowSize(__GLcontext *gc, GLint w, GLint h)
{
    __GLdrawablePrivate *dp = gc->drawablePrivate;
    GLboolean status;

    /* resize buffers to the new window size */
    if ((w != dp->width) ||
    	(h != dp->height) ||
	(!w && !h))
    {
	dp->width = w;
	dp->height = h;

	status = (*gc->frontBuffer.buf.resize)(&gc->frontBuffer.buf, w, h, 
					       __GL_FRONT_BUFFER_MASK);
	__GL_SET_HW_BUFFER_FLAG(__GL_FRONT_BUFFER_MASK);

	if (gc->modes.doubleBufferMode) {
	    status = (*gc->backBuffer.buf.resize)(&gc->backBuffer.buf, w, h,
						  __GL_BACK_BUFFER_MASK);
	    __GL_SET_HW_BUFFER_FLAG(__GL_BACK_BUFFER_MASK);
	}

	if (gc->modes.haveAccumBuffer) {
	    status = (*gc->accumBuffer.buf.resize)(&gc->accumBuffer.buf, w, h,
						   __GL_ACCUM_BUFFER_MASK);
	}
	__GL_SET_HW_BUFFER_FLAG(__GL_ACCUM_BUFFER_MASK);

	if (gc->modes.haveDepthBuffer) {
	    status = (*gc->depthBuffer.buf.resize)(&gc->depthBuffer.buf, w, h,
						   __GL_DEPTH_BUFFER_MASK);
	    __GL_SET_HW_BUFFER_FLAG(__GL_DEPTH_BUFFER_MASK);
	}

	if (gc->modes.haveStencilBuffer) {
	    status = (*gc->stencilBuffer.buf.resize)(&gc->stencilBuffer.buf, w, h,
						     __GL_STENCIL_BUFFER_MASK);
	    __GL_SET_HW_BUFFER_FLAG(__GL_STENCIL_BUFFER_MASK);
	}
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
	if (gc->modes.numAuxBuffers > 0) {
	    int i;
	    for (i = 0; i < gc->modes.numAuxBuffers; i++ ) {
		status = (*gc->auxBuffer[i].buf.resize)(&gc->auxBuffer[i].buf, w, h,
							__GL_AUX_BUFFER_MASK(i));
		__GL_SET_HW_BUFFER_FLAG(__GL_AUX_BUFFER_MASK(i));
	    }
	}
#endif
	status = (*gc->ownershipBuffer.buf.resize)(&gc->ownershipBuffer.buf, w, h,
						   __GL_OWNERSHIP_BUFFER_MASK);
	__GL_SET_HW_BUFFER_FLAG(__GL_OWNERSHIP_BUFFER_MASK);
    }

    /* recompute derived state which depends on the window size */
    if ((w != gc->constants.width) ||
        (h != gc->constants.height))
    {
	if (gc->constants.yInverted && h != gc->constants.height) {
	    gc->state.current.rasterPos.window.y += h - gc->constants.height;
	}

	gc->constants.width = w;
	gc->constants.height = h;

	(*gc->procs.computeClipBox)(gc);
	(*gc->procs.applyScissor)(gc);
    }

    return GL_TRUE;
}

/*
** Hack hook to update internal buffer memory whenever the user changes
** the viewport.
*/
GLboolean __glFindWindowSize(__GLcontext *gc)
{
    GLint width, height;

    /* get current window size */
    (*gc->imports.getDrawableSize)(gc, &width, &height);

    return __glChangeWindowSize(gc, width, height);
}

void __glMakeCurrentBuffers(__GLcontext *gc, GLint *displayBank)
{
    __GLdrawablePrivate *dp = (*gc->imports.getDrawablePrivate)(gc);

    /* get the drawable private */
    gc->drawablePrivate = dp;

    /* update the call back so that we will free anything we allocate */
    dp->freePrivate = __glFreePrivate;

    /* load updated info from the drawable */
    if (gc->drawablePrivate->yInverted != gc->constants.yInverted) {
	__GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_ALL);
    }

    if (gc->drawablePrivate->yInverted) {
	gc->constants.yInverted = GL_TRUE;
	gc->constants.ySign = -1;
    } else {
	gc->constants.yInverted = GL_FALSE;
	gc->constants.ySign = 1;
    }

    gc->constants.width = dp->width;
    gc->constants.height = dp->height;
    *displayBank = dp->displayBuffer;

    (*gc->frontBuffer.buf.makeCurrent)(&gc->frontBuffer.buf,
				       &dp->frontBuffer, gc);
    if (gc->modes.doubleBufferMode) {
	(*gc->backBuffer.buf.makeCurrent)(&gc->backBuffer.buf,
					  &dp->backBuffer, gc);
    }
    if (gc->modes.haveAccumBuffer) {
	(*gc->accumBuffer.buf.makeCurrent)(&gc->accumBuffer.buf,
					   &dp->accumBuffer, gc);
    }
    if (gc->modes.haveDepthBuffer) {
	(*gc->depthBuffer.buf.makeCurrent)(&gc->depthBuffer.buf,
					   &dp->depthBuffer, gc);
    }
    if (gc->modes.haveStencilBuffer) {
	(*gc->stencilBuffer.buf.makeCurrent)(&gc->stencilBuffer.buf,
					     &dp->stencilBuffer, gc);
    }
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
    if (dp->modes->numAuxBuffers > 0) {
	int i;
	for (i = 0; i < dp->modes->numAuxBuffers; i++) {
	    (*gc->auxBuffer[i].buf.makeCurrent)(&gc->auxBuffer[i].buf,
						&dp->auxBuffer[i], gc);
	}
    }
#endif
    (*gc->ownershipBuffer.buf.makeCurrent)(&gc->ownershipBuffer.buf,
					   &dp->ownershipBuffer, gc);
}

void __glLoseCurrentBuffers(__GLcontext *gc, GLint displayBank)
{
    __GLdrawablePrivate *dp = gc->drawablePrivate;

    dp->width = gc->constants.width;
    dp->height = gc->constants.height;
    dp->displayBuffer = displayBank;

    (*gc->frontBuffer.buf.loseCurrent)(&gc->frontBuffer.buf, gc);
    if (gc->modes.doubleBufferMode) {
	(*gc->backBuffer.buf.loseCurrent)(&gc->backBuffer.buf, gc);
    }
    if (gc->modes.haveAccumBuffer) {
	(*gc->accumBuffer.buf.loseCurrent)(&gc->accumBuffer.buf, gc);
    }
    if (gc->modes.haveDepthBuffer) {
	(*gc->depthBuffer.buf.loseCurrent)(&gc->depthBuffer.buf, gc);
    }
    if (gc->modes.haveStencilBuffer) {
	(*gc->stencilBuffer.buf.loseCurrent)(&gc->stencilBuffer.buf, gc);
    }
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
    if (gc->modes.numAuxBuffers > 0) {
	int i;
	for (i = 0; i < gc->modes.numAuxBuffers; i++) {
	    (*gc->auxBuffer[i].buf.loseCurrent)(&gc->auxBuffer[i].buf, gc);
	}
    }
#endif
    (*gc->ownershipBuffer.buf.loseCurrent)(&gc->ownershipBuffer.buf, gc);
}

/* 
** this frees tables that are really on a per-context basis,
** but they may (structurally) belong to the buffers.
*/
void __glFreeBufData(__GLcontext *gc)
{
    if (gc->frontBuffer.buf.freebuf) {
	(*gc->frontBuffer.buf.freebuf)(&gc->frontBuffer.buf, gc);
    }
    if (gc->backBuffer.buf.freebuf) {
	(*gc->frontBuffer.buf.freebuf)(&gc->backBuffer.buf, gc);
    }
    if (gc->stencilBuffer.buf.freebuf) {
	(*gc->stencilBuffer.buf.freebuf)(&gc->stencilBuffer.buf, gc);
    }
    if (gc->depthBuffer.buf.freebuf) {
	(*gc->depthBuffer.buf.freebuf)(&gc->depthBuffer.buf, gc);
    }
    if (gc->accumBuffer.buf.freebuf) {
	(*gc->accumBuffer.buf.freebuf)(&gc->accumBuffer.buf, gc);
    }
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
    if (gc->modes.numAuxBuffers > 0) {
	int i;
	for (i = 0; i < gc->modes.numAuxBuffers; i++) {
	    if (gc->auxBuffer[i].buf.freebuf) {
		(*gc->auxBuffer[i].buf.freebuf)(&gc->auxBuffer[i].buf, gc);
	    }
	}
    }
#endif
    if (gc->ownershipBuffer.buf.freebuf) {
	(*gc->ownershipBuffer.buf.freebuf)(&gc->ownershipBuffer.buf, gc);
    }
}

void APIENTRY __glim_AddSwapHintRectWIN(GLint x, GLint y, GLsizei width,
					GLsizei height)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (gc->modes.doubleBufferMode) {
	(*gc->drawablePrivate->addSwapRect)
		(gc->drawablePrivate, x, y, width, height);
    }
}

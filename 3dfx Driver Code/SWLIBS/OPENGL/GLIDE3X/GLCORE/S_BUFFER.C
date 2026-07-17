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
**
** $Revision: 2$
** $Date: 10/11/00 7:55:59 PM$
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
	buf->ubase = drawableBuf->ubase;
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

#define BUF_ALIGN	  32	/* x86 cache align our buffers */
#define BUF_ALIGN_MINUS_1 (BUF_ALIGN - 1)

/*
** allocate or resize buffer
*/
void __glResizeBuffer(__GLbuffer *buf, GLint w, GLint h)
{
    __GLdrawableBuffer *drawableBuf = buf->drawableBuf;
    __GLdrawablePrivate *dp = buf->gc->drawablePrivate;

    h++;	/* Guard line on the end  */

    if (drawableBuf->update) {
	/* let the "operating system" reallocate the buffer */
	(*drawableBuf->update)(drawableBuf, dp);

	/* copy updated info from the drawable's structure */
	buf->width = drawableBuf->width;
	buf->height = drawableBuf->height;

	buf->ubase = drawableBuf->ubase;
	buf->base = drawableBuf->base;
	buf->size = drawableBuf->size;
	buf->elementSize = drawableBuf->elementSize;
	buf->byteWidth = drawableBuf->byteWidth;

	buf->elementSizeLog2 = __glFloorLog2(buf->elementSize);
	buf->outerWidth = buf->byteWidth / buf->elementSize;
    } else {
	size_t newSize;

	/* reallocate the buffer */
	newSize = (size_t) (w * h * buf->elementSize);
	if (newSize > buf->size) {
	    if (buf->base) {
		buf->ubase = (*dp->realloc)(buf->ubase, newSize + BUF_ALIGN_MINUS_1);
	    } else {
		buf->ubase = (*dp->calloc)(1, newSize + BUF_ALIGN_MINUS_1);
	    }
	    buf->base = (void*)(((size_t)buf->ubase + BUF_ALIGN_MINUS_1) & 
				~BUF_ALIGN_MINUS_1);
	    assert((size_t)buf->base % BUF_ALIGN == 0);
	    buf->size = (GLuint)newSize;
	}

	/* update buffer info */
	buf->width = w;
	buf->height = h;
	buf->byteWidth = w * buf->elementSize;

	buf->elementSizeLog2 = __glFloorLog2(buf->elementSize);
	buf->outerWidth = buf->byteWidth / buf->elementSize;

	/* copy info back to the drawable's structure */
	drawableBuf->width = buf->width;
	drawableBuf->height = buf->height;

	drawableBuf->ubase = buf->ubase;
	drawableBuf->base = buf->base;
	drawableBuf->size = buf->size;
	drawableBuf->elementSize = buf->elementSize;
	drawableBuf->byteWidth = buf->byteWidth;
	drawableBuf->depth = buf->depth;

	/* update the call back so that we will free the buffer */
	drawableBuf->freePrivate = __glFreeBuffer;
    }
}

/*
** deallocate a buffer
*/
void __glFreeBuffer(__GLdrawableBuffer *drawableBuf, __GLdrawablePrivate *dp)
{
    if (drawableBuf->base) {
	(*dp->free)(drawableBuf->ubase);
    }
    drawableBuf->ubase = NULL;
    drawableBuf->base = NULL;
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

void __glLockBuffer(__GLcontext *gc)
{
    __GLbuffer *readbuf = gc->buffers.readbuffer;
    __GLbuffer *writebuf = gc->buffers.writebuffer;
    __GLbuffer *depthbuf = gc->buffers.depthbuffer;
    __GLbuffer *auxbuf = gc->buffers.auxbuffer;
    __GLdrawablePrivate *dp = gc->drawablePrivate;

    if (depthbuf) {
        __GLdrawableBuffer *depthdrawableBuf = depthbuf->drawableBuf;

        if (depthdrawableBuf->lock) {

            (*depthdrawableBuf->lock)(depthdrawableBuf, dp);

            /* copy updated info from the drawable's structure */
            depthbuf->ubase = depthdrawableBuf->ubase;
            depthbuf->base = depthdrawableBuf->base;
            depthbuf->byteWidth = depthdrawableBuf->byteWidth;

            depthbuf->elementSizeLog2 = __glFloorLog2(depthbuf->elementSize);
            depthbuf->outerWidth = depthbuf->byteWidth / depthbuf->elementSize;
        }
    }

    dp->readmode = GL_TRUE;

    if (readbuf) {
        __GLdrawableBuffer *readdrawableBuf = readbuf->drawableBuf;

        if (readdrawableBuf->lock) {

            (*readdrawableBuf->lock)(readdrawableBuf, dp);

            /* copy updated info from the drawable's structure */
            readbuf->ubase = readdrawableBuf->ubase;
            readbuf->base = readdrawableBuf->base;
            readbuf->byteWidth = readdrawableBuf->byteWidth;

            readbuf->elementSizeLog2 = __glFloorLog2(readbuf->elementSize);
            readbuf->outerWidth = readbuf->byteWidth / readbuf->elementSize;
        }
    }

    dp->readmode = GL_FALSE;

    if (writebuf) {
        __GLdrawableBuffer *writedrawableBuf = writebuf->drawableBuf;

        if (writedrawableBuf->lock) {

            (*writedrawableBuf->lock)(writedrawableBuf, dp);

            /* copy updated info from the drawable's structure */
            writebuf->ubase = writedrawableBuf->ubase;
            writebuf->base = writedrawableBuf->base;
            writebuf->byteWidth = writedrawableBuf->byteWidth;

            writebuf->elementSizeLog2 = __glFloorLog2(writebuf->elementSize);
            writebuf->outerWidth = writebuf->byteWidth / writebuf->elementSize;
        }
    }

    if (auxbuf) {
        __GLdrawableBuffer *auxdrawableBuf = auxbuf->drawableBuf;

        if (auxdrawableBuf->lock) {
            (*auxdrawableBuf->lock)(auxdrawableBuf, dp);

            /* copy updated info from the drawable's structure */
            auxbuf->ubase = auxdrawableBuf->ubase;
            auxbuf->base = auxdrawableBuf->base;
            auxbuf->byteWidth = auxdrawableBuf->byteWidth;

            auxbuf->elementSizeLog2 = __glFloorLog2(auxbuf->elementSize);
            auxbuf->outerWidth = auxbuf->byteWidth / auxbuf->elementSize;
        }
    }
}

void __glUnlockBuffer(__GLcontext *gc)
{
    __GLbuffer *readbuf = gc->buffers.readbuffer;
    __GLbuffer *writebuf = gc->buffers.writebuffer;
    __GLbuffer *depthbuf = gc->buffers.depthbuffer;
    __GLbuffer *auxbuf = gc->buffers.auxbuffer;
    __GLdrawablePrivate *dp = gc->drawablePrivate;

    dp->readmode = GL_TRUE;

    if (readbuf) {
        __GLdrawableBuffer *readdrawableBuf = readbuf->drawableBuf;
        if (readdrawableBuf->unlock ) {
            (*readdrawableBuf->unlock)(readdrawableBuf, dp);
            readbuf->ubase = NULL;
            readbuf->base = NULL;
        }
    }

    dp->readmode = GL_FALSE;

    if (writebuf) {
        __GLdrawableBuffer *writedrawableBuf = writebuf->drawableBuf;
        if (writedrawableBuf->unlock ) {
            (*writedrawableBuf->unlock)(writedrawableBuf, dp);
            writebuf->ubase = NULL;
            writebuf->base = NULL;
        }
    }

    if (depthbuf) {
        __GLdrawableBuffer *depthdrawableBuf = depthbuf->drawableBuf;
        if (depthdrawableBuf->unlock ) {
            (*depthdrawableBuf->unlock)(depthdrawableBuf, dp);
            depthbuf->ubase = NULL;
            depthbuf->base = NULL;
        }
    }

    if (auxbuf) {
        __GLdrawableBuffer *auxdrawableBuf = auxbuf->drawableBuf;
        if (auxdrawableBuf->unlock ) {
            (*auxdrawableBuf->unlock)(auxdrawableBuf, dp);
            auxbuf->ubase = NULL;
            auxbuf->base = NULL;
        }
    }

}


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

/*
** resize all buffers in a context
*/
void __glChangeWindowSize(__GLcontext *gc, GLint w, GLint h)
{
    __GLdrawablePrivate *dp = gc->drawablePrivate;

    if ((w != gc->constants.width) ||
        (h != gc->constants.height))
    {
	gc->constants.width = w;
	gc->constants.height = h;

	(*gc->procs.computeClipBox)(gc);
    }

    if ((w != dp->width) ||
    	(h != dp->height) ||
	(!w && !h))
    {
	dp->width = w;
	dp->height = h;

	(*gc->frontBuffer.buf.resize)(&gc->frontBuffer.buf, w, h);
	if (gc->modes.doubleBufferMode) {
	    (*gc->backBuffer.buf.resize)(&gc->backBuffer.buf, w, h);
	}
	if (gc->modes.haveAccumBuffer) {
	    (*gc->accumBuffer.buf.resize)(&gc->accumBuffer.buf, w, h);
	}
	if (gc->modes.haveDepthBuffer) {
	    (*gc->depthBuffer.buf.resize)(&gc->depthBuffer.buf, w, h);
	}
	if (gc->modes.haveStencilBuffer) {
	    (*gc->stencilBuffer.buf.resize)(&gc->stencilBuffer.buf, w, h);
	}
#if __GL_NUMBER_OF_AUX_BUFFERS > 0
	if (gc->modes.numAuxBuffers > 0) {
	    int i;
	    for (i = 0; i < gc->modes.numAuxBuffers; i++ ) {
		(*gc->auxBuffer[i].buf.resize)(&gc->auxBuffer[i].buf, w, h);
	    }
	}
#endif
	(*gc->ownershipBuffer.buf.resize)(&gc->ownershipBuffer.buf, w, h);
    }
}

/*
** Hack hook to update internal buffer memory whenever the user changes
** the viewport.
*/
void __glFindWindowSize(__GLcontext *gc)
{
    GLint width, height;

    /* get current window size */
    (*gc->imports.getDrawableSize)(gc, &width, &height);

    __glChangeWindowSize(gc, width, height);
}

void __glMakeCurrentBuffers(__GLcontext *gc, GLint *displayBank)
{
    __GLdrawablePrivate *dp = (*gc->imports.getDrawablePrivate)(gc);

    /* get the drawable private */
    gc->drawablePrivate = dp;

    /* update the call back so that we will free anything we allocate */
    dp->freePrivate = __glFreePrivate;

    /* load updated info from the drawable */
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

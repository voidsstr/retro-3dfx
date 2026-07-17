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
#include <windows.h>
#include "context.h"
#include "global.h"
#include "fmemclr.h"
#include <glide.h>

void __glSSTValidateDepthTest(__GLcontext *gc)
{
    if (gc->state.enables.general & __GL_DEPTH_TEST_ENABLE) {
        grDepthBufferMode(GR_DEPTHBUFFER_ZBUFFER);
        if (gc->state.depth.writeEnable) {
            grDepthMask(FXTRUE);
        }
    } else {
        grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
        grDepthMask(FXFALSE);
    }
}

void __glSSTValidateDepthFunc(__GLcontext *gc)
{
    grDepthBufferFunction(gc->state.depth.testFunc - GL_NEVER);
}


/* ARGSUSED */
static GLboolean StoreNEVER(__GLdepthBuffer *fb,
			    GLint x, GLint y, __GLzValue z)
{
    return GL_FALSE;
}

static GLboolean StoreLESS(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    if ( (GLuint)Z < (GLuint)fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreLESS_s(__GLdepthBuffer *fb,
			     GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue *), x, y);
    if( (GLint)Z < (GLint)fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreEQUAL(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    if (Z == fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreLEQUAL(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    if ( (GLuint)Z <= (GLuint)fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreLEQUAL_s(__GLdepthBuffer *fb,
			       GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue *), x, y);
    if( (GLint)Z <= (GLint)fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGREATER(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    if ( (GLuint)Z > (GLuint)fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGREATER_s(__GLdepthBuffer *fb,
				GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue *), x, y);
    if( (GLint)Z > (GLint)fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreNOTEQUAL(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    if ( Z != fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGEQUAL(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    if ( (GLuint)Z >=  (GLuint)fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGEQUAL_s(__GLdepthBuffer *fb,
			       GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue *), x, y);
    if( (GLint)Z >= (GLint)fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreALWAYS(__GLdepthBuffer *fb,
			     GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    fp[0] = (z >> fb->buf.gc->depthBuffer.numFracBits) + 
	fb->buf.gc->depthBuffer.cnt;
    return GL_TRUE;
}

/************************************************************************/

static GLboolean StoreLESS_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( (GLuint)Z < (GLuint)fp[0] );
}

static GLboolean StoreLESS_W_s(__GLdepthBuffer *fb,
			       GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( (GLint)Z < (GLint)fp[0] );
}

static GLboolean StoreEQUAL_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( (GLuint)Z == (GLuint)fp[0] );
}

static GLboolean StoreLEQUAL_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( (GLuint)Z <= (GLuint)fp[0] );
}

static GLboolean StoreLEQUAL_W_s(__GLdepthBuffer *fb,
				 GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( (GLint)Z <= (GLint)fp[0] );
}

static GLboolean StoreGREATER_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( (GLuint)Z > (GLuint)fp[0] );
}

static GLboolean StoreGREATER_W_s(__GLdepthBuffer *fb,
				  GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( (GLint)Z > (GLint)fp[0] );
}


static GLboolean StoreNOTEQUAL_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( Z != fp[0] );
}

static GLboolean StoreGEQUAL_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( (GLuint)Z >= (GLuint)fp[0] );
}

static GLboolean StoreGEQUAL_W_s(__GLdepthBuffer *fb,
				 GLint x, GLint y, __GLzValue z)
{
    __GLzValue *fp;
    __GLzValue Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return ( (GLint)Z >= (GLint)fp[0] );
}

/* ARGSUSED */
static GLboolean StoreALWAYS_W(__GLdepthBuffer *fb,
			     GLint x, GLint y, __GLzValue z)
{
    return GL_TRUE;
}

/************************************************************************/

static __GLzValue Fetch16(__GLdepthBuffer *fb, GLint x, GLint y)
{
    __GLzValue16 *fp;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return fp[0];
}

/* ARGSUSED */
static GLboolean StoreNEVER16(__GLdepthBuffer *fb,
			    GLint x, GLint y, __GLzValue z)
{
    return GL_FALSE;
}

static GLboolean StoreLESS16(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if ( (GLuint)Z < (GLuint)fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreLESS16_s(__GLdepthBuffer *fb,
			     GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if( (GLint)Z < (GLint)fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreEQUAL16(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if (Z == fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreLEQUAL16(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if ( (GLuint)Z <= (GLuint)fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreLEQUAL16_s(__GLdepthBuffer *fb,
			       GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if( (GLint)Z <= (GLint)fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGREATER16(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if ( (GLuint)Z > (GLuint)fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGREATER16_s(__GLdepthBuffer *fb,
				GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if( (GLint)Z > (GLint)fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreNOTEQUAL16(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if ( Z != fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGEQUAL16(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if ( (GLuint)Z >=  (GLuint)fp[0]) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreGEQUAL16_s(__GLdepthBuffer *fb,
			       GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    if( (GLint)Z >= (GLint)fp[0] ) {
	fp[0] = Z;
	return GL_TRUE;
    }
    return GL_FALSE;
}

static GLboolean StoreALWAYS16(__GLdepthBuffer *fb,
			     GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    fp[0] = (z >> fb->buf.gc->depthBuffer.numFracBits) + 
	fb->buf.gc->depthBuffer.cnt;
    return GL_TRUE;
}

/************************************************************************/

static GLboolean StoreLESS16_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( (GLuint)Z < (GLuint)fp[0] );
}

static GLboolean StoreLESS16_W_s(__GLdepthBuffer *fb,
			       GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( (GLint)Z < (GLint)fp[0] );
}

static GLboolean StoreEQUAL16_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( (GLuint)Z == (GLuint)fp[0] );
}

static GLboolean StoreLEQUAL16_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( (GLuint)Z <= (GLuint)fp[0] );
}

static GLboolean StoreLEQUAL16_W_s(__GLdepthBuffer *fb,
				 GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( (GLint)Z <= (GLint)fp[0] );
}

static GLboolean StoreGREATER16_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( (GLuint)Z > (GLuint)fp[0] );
}

static GLboolean StoreGREATER16_W_s(__GLdepthBuffer *fb,
				  GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( (GLint)Z > (GLint)fp[0] );
}


static GLboolean StoreNOTEQUAL16_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( Z != fp[0] );
}

static GLboolean StoreGEQUAL16_W(__GLdepthBuffer *fb,
			   GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( (GLuint)Z >= (GLuint)fp[0] );
}

static GLboolean StoreGEQUAL16_W_s(__GLdepthBuffer *fb,
				 GLint x, GLint y, __GLzValue z)
{
    __GLzValue16 *fp;
    __GLzValue16 Z = (z >> fb->buf.gc->depthBuffer.numFracBits) +
	fb->buf.gc->depthBuffer.cnt;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue16*), x, y);
    return ( (GLint)Z >= (GLint)fp[0] );
}

/* ARGSUSED */
static GLboolean StoreALWAYS16_W(__GLdepthBuffer *fb,
			     GLint x, GLint y, __GLzValue z)
{
    return GL_TRUE;
}


/************************************************************************/

static GLboolean (*StoreProcs[64])(__GLdepthBuffer*, GLint, GLint, __GLzValue)
 = {
     /* unsigned */
     StoreNEVER16,
     StoreLESS16,
     StoreEQUAL16,
     StoreLEQUAL16,
     StoreGREATER16,
     StoreNOTEQUAL16,
     StoreGEQUAL16,
     StoreALWAYS16,
     /* unsigned, masked */
     StoreNEVER16,
     StoreLESS16_W,
     StoreEQUAL16_W,
     StoreLEQUAL16_W,
     StoreGREATER16_W,
     StoreNOTEQUAL16_W,
     StoreGEQUAL16_W,
     StoreALWAYS16_W,
     /* unsigned */
     StoreNEVER,
     StoreLESS,
     StoreEQUAL,
     StoreLEQUAL,
     StoreGREATER,
     StoreNOTEQUAL,
     StoreGEQUAL,
     StoreALWAYS,
     /* unsigned, masked */
     StoreNEVER,
     StoreLESS_W,
     StoreEQUAL_W,
     StoreLEQUAL_W,
     StoreGREATER_W,
     StoreNOTEQUAL_W,
     StoreGEQUAL_W,
     StoreALWAYS_W,
};

/* ARGSUSED */
static void Pick(__GLcontext *gc, __GLdepthBuffer *fb, GLint depthIndex)
{
    fb->store = StoreProcs[depthIndex];
}

/************************************************************************/

static void Resize(__GLbuffer *fb, GLint w, GLint h)
{
    __GLdepthBuffer *dfb = (__GLdepthBuffer *)fb;
    GLuint paddedWidth;

    paddedWidth = (((w * fb->elementSize + 15) >> 4) << 4) / fb->elementSize;

    __glResizeBuffer(fb, paddedWidth, h);

    dfb->fullZClear = GL_TRUE;
}

/************************************************************************/

static void MakeCurrent(__GLbuffer *buf, __GLdrawableBuffer *drawableBuf,
                        __GLcontext *gc )
{
    __GLdepthBuffer *dfb = (__GLdepthBuffer *)buf;
    __GLdrawablePrivate *dp = gc->drawablePrivate;
    __GLdepthBufferP *persistent = (__GLdepthBufferP *) drawableBuf->private;

    __glMakeCurrentBuffer( buf, drawableBuf, gc );

	drawableBuf->type = GR_BUFFER_AUXBUFFER;

    if (persistent) {
        dfb->writeMask = persistent->writeMask;
        dfb->scale = persistent->scale;
        dfb->cnt = persistent->cnt;
        dfb->numFracBits = persistent->numFracBits;
        dfb->zDepthMask = persistent->zDepthMask;
        dfb->zClearInc = persistent->zClearInc;
        dfb->fullZClear = persistent->fullZClear;
        dfb->invertZRange = persistent->invertZRange;
    } else {
        persistent = (__GLdepthBufferP *)
            (*dp->calloc)(1, sizeof(__GLdepthBufferP));
        drawableBuf->private = persistent;
    }

    /* reset lastDepth, scissorRegion, etc */
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH | __GL_DIRTY_SCISSOR);
}

static void LoseCurrent( __GLbuffer *buf, __GLcontext *gc )
{
    __GLdepthBuffer *dfb = (__GLdepthBuffer *)buf;
    __GLdrawableBuffer *drawableBuf = (__GLdrawableBuffer *) buf->drawableBuf;
    __GLdepthBufferP *persistent = (__GLdepthBufferP *) drawableBuf->private;

    /* if not current to anything, return */
    if( buf->gc == NULL ) {
        return;
    }

    __glLoseCurrentBuffer( buf, gc );

    persistent->writeMask = dfb->writeMask;
    persistent->scale = dfb->scale;
    persistent->cnt = dfb->cnt;
    persistent->numFracBits = dfb->numFracBits;
    persistent->zDepthMask = dfb->zDepthMask;
    persistent->zClearInc = dfb->zClearInc;
    persistent->fullZClear = dfb->fullZClear;
    persistent->invertZRange = dfb->invertZRange;
}

/************************************************************************/

void __glSSTInitDepth16(__GLdepthBuffer *fb, __GLcontext *gc )
{
    __glInitBuffer( &fb->buf, gc );

    fb->buf.elementSize = sizeof(__GLzValue16);
    fb->buf.depth = 16;

    fb->buf.resize = Resize;
    fb->buf.makeCurrent = MakeCurrent;
    fb->buf.loseCurrent = LoseCurrent;

    fb->pick = Pick;
#if 0 /* not used - we handle clear in __glsstim_Clear */
    fb->clear = Clear;
#endif
    fb->store2 = StoreALWAYS16;
    fb->fetch = Fetch16;

    fb->scale = (__GLzValue) 0xffff;
    fb->numFracBits = 0;
    fb->cnt = 0x00000000;        /* unused */
    fb->zDepthMask = 0x0000ffff; /* unused */
    fb->zClearInc = 0x00000000;  /* unused */
    fb->fullZClear = GL_TRUE;    /* unused */
    fb->testFunc = GL_LESS;
    fb->fixZBuffer = ( void (*)(__GLcontext *, __GLdepthBuffer *, GLuint) ) __glNop;
    fb->writeMask = (__GLzValue) ~0;
    fb->minResolution = 0x1000000;
}

void __glSSTInitDepth(__GLdepthBuffer *fb, __GLcontext *gc )
{
    __glSSTInitDepth16( fb, gc );
}

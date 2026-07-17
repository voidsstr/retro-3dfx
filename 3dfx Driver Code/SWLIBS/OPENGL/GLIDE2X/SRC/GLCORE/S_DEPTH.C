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
#include "context.h"
#include "global.h"
#include "fmemclr.h"


static __GLzValue FetchFastClear(__GLdepthBuffer *fb, GLint x, GLint y)
{
    __GLzValue *fp;
    __GLcontext *gc = fb->buf.gc;
    GLuint cnt = fb->cnt;
    GLuint zClearInc = fb->zClearInc;
    GLuint zDepthMask = fb->zDepthMask;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);

    if( (fp[0] & ~zDepthMask) != cnt ) {
	/* 
	** big mess..  If the buffer is not from the current frame,
	** then we have to check the z clear value and return that to
	** the user.  Luckily, the clear value is only going to be 0.0 or
	** 1.0, otherwise we woudn't be doing accelerated z-buffering
	*/
	if( zClearInc == 0xff000000 ) {
	    if( gc->state.depth.clear == 1.0 ) {
		if (fb->invertZRange) {
		    return 0x00000000;
		} else {
		    return fb->scale;
		}
	    }
	}
	if( zClearInc == 0x01000000 ) {
	    if( gc->state.depth.clear == 0.0 ) {
		if (fb->invertZRange) {
		    return fb->scale;
		} else {
		    return 0x00000000;
		}
	    }
	}

	/* othwise, return the value there */
    }

    /* ignore frame count, and shift appropriately */
    return ( fp[0] & zDepthMask ) << fb->numFracBits; 
}

static __GLzValue Fetch(__GLdepthBuffer *fb, GLint x, GLint y)
{
    __GLzValue *fp;

    fp = __GL_DEPTH_ADDR(fb, (__GLzValue*), x, y);
    return fp[0];
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

/*
** Operations which affect the fast depth clear state machine:
**   depth test enable
**   depth function
**   depth range
**   depth write mask
**   depth clear value
**   scissor enable changed
**   scissor box changed
**   make current
**   lose current
**   read depth buffer
**   draw depth buffer
**   clear depth buffer
*/

/*
**  count   val     inc     func    lt      gt      range
**  -------------------------------------------------------
**  127     1.0     -1      <       -       invert  254-127
**  0       1.0     -1      <       -       invert  127-0
**  128     0.0     +1      >       invert  -       128-1
**  255     0.0     +1      >       invert  -       255-128
*/

void __glValidateZCount(__GLdepthBuffer *dfb)
{
    __GLcontext *gc = dfb->buf.gc;

    if (dfb->zClearInc == 0xff000000) {
	if (dfb->cnt == 0xff000000) {
	    dfb->cnt = 0x80000000;
	    dfb->zClearInc = 0x01000000;
	    switch (gc->state.depth.testFunc) {
	    case GL_LESS:
		dfb->testFunc = GL_GREATER;
		dfb->invertZRange = GL_TRUE;
		break;
	    case GL_LEQUAL:
		dfb->testFunc = GL_GEQUAL;
		dfb->invertZRange = GL_TRUE;
		break;
	    case GL_GREATER:
		dfb->testFunc = GL_GREATER;
		dfb->invertZRange = GL_FALSE;
		break;
	    case GL_GEQUAL:
		dfb->testFunc = GL_GEQUAL;
		dfb->invertZRange = GL_FALSE;
		break;
	    default:
		dfb->testFunc = gc->state.depth.testFunc;
		dfb->invertZRange = GL_FALSE;
		break;
	    }
	    __glUpdateDepthRange(gc);
	    /* changed testFunc, have to re-pick rendering procs */
	    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);
	}
    } else if (dfb->zClearInc == 0x01000000) {
	if (dfb->cnt == 0x00000000) {
	    dfb->cnt = 0x7f000000;
	    dfb->zClearInc = 0xff000000;
	    switch (gc->state.depth.testFunc) {
	    case GL_LESS:
		dfb->testFunc = GL_LESS;
		dfb->invertZRange = GL_FALSE;
		break;
	    case GL_LEQUAL:
		dfb->testFunc = GL_LEQUAL;
		dfb->invertZRange = GL_FALSE;
		break;
	    case GL_GREATER:
		dfb->testFunc = GL_LESS;
		dfb->invertZRange = GL_TRUE;
		break;
	    case GL_GEQUAL:
		dfb->testFunc = GL_LEQUAL;
		dfb->invertZRange = GL_TRUE;
		break;
	    default:
		dfb->testFunc = gc->state.depth.testFunc;
		dfb->invertZRange = GL_FALSE;
		break;
	    }
	    __glUpdateDepthRange(gc);
	    /* changed testFunc, have to re-pick rendering procs */
	    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);
	}
    } else {
	dfb->testFunc = gc->state.depth.testFunc;
	dfb->invertZRange = GL_FALSE;
    }

}

static __GLzValue DepthClearVal(__GLdepthBuffer *dfb)
{
    __GLcontext *gc = dfb->buf.gc;
    __GLzValue z;

    __glValidateZCount(dfb);

    /* create the z value to be put in the frame buffer */
    if (dfb->invertZRange) {
	z = (__GLzValue)((1.0 - gc->state.depth.clear) * dfb->scale);
    } else {
	z = (__GLzValue)(gc->state.depth.clear * dfb->scale);
    }
    z = (z >> dfb->numFracBits) + dfb->cnt;
    return z;
}

static void Clear(__GLdepthBuffer *dfb)
{
    __GLcontext *gc = dfb->buf.gc;
    __GLzValue *fb;
    __GLzValue z;
    GLint x, y, x1, y1, w, h, skip;
#ifndef WIN32
    GLint w32, w4, w1;
#endif

    if (!gc->state.depth.writeEnable) {
	return;
    }

    x = gc->transform.clipX0;
    y = gc->transform.clipY0;
    x1 = gc->transform.clipX1;
    y1 = gc->transform.clipY1;
    if (((w = x1 - x) == 0) || ((h = y1 - y) == 0)) {
	return;
    }

    if( dfb->fullZClear || !dfb->allowPartialZClear ) {
	/* reset the count to a first frame */
	if (dfb->zClearInc != 0x00000000) {
	    dfb->cnt = ~(dfb->zDepthMask);
	}

	z = DepthClearVal(dfb);

	/* cancel the full clear for next frame */
	dfb->fullZClear = GL_FALSE;

	/* now do the rest */
	fb = __GL_DEPTH_ADDR(dfb, (__GLzValue*), x, y);

	skip = dfb->buf.outerWidth - w;
#ifdef WIN32
	if(dfb->buf.elementSize == 2) {
	    FastMemClear2(gc,
			  (unsigned short*)fb, w, h, 
			  skip*sizeof(unsigned short), z);
	} else {
	    FastMemClear4(gc,
			  (unsigned long *)fb, w, h, 
			  skip*sizeof(unsigned long), z);
	}
#else /* WIN32 */
	w32 = w >> 5;
	w4 = (w & 31) >> 2;
	w1 = w & 3;

	for (; y < y1; y++) {
	    w = w32;
	    while (--w >= 0) {
		fb[0] = z; fb[1] = z; fb[2] = z; fb[3] = z;
		fb[4] = z; fb[5] = z; fb[6] = z; fb[7] = z;
		fb[8] = z; fb[9] = z; fb[10] = z; fb[11] = z;
		fb[12] = z; fb[13] = z; fb[14] = z; fb[15] = z;
		fb[16] = z; fb[17] = z; fb[18] = z; fb[19] = z;
		fb[20] = z; fb[21] = z; fb[22] = z; fb[23] = z;
		fb[24] = z; fb[25] = z; fb[26] = z; fb[27] = z;
		fb[28] = z; fb[29] = z; fb[30] = z; fb[31] = z;
		fb += 32;
	    }
	    w = w4;
	    while (--w >= 0) {
		fb[0] = z; fb[1] = z; fb[2] = z; fb[3] = z;
		fb += 4;
	    }
	    w = w1;
	    while (--w >= 0) {
		*fb++ = z;
	    }
	    fb += skip;
	}
#endif /* WIN32 */
    } else {
	/* partial buffer clear */
	GLint dx, dy;
	GLint np, ndp;
	GLint indx;
	GLint start_pix, end_pix;
	GLint start_line, end_line;

	/* increment the counter */
	dfb->cnt += dfb->zClearInc;

	z = DepthClearVal(dfb);

	/* now do the rest */

	/* figure out scanline for starting point */
	dx = x1 - x;
	dy = y1 - y;
	np = dx * dy;	/* number of pixels in window */
	ndp = np >> 7;	/* number of drawable pixels in wind in this itertn */
	indx = (dfb->cnt >> 24) & 0x7f;/* which iteration is this */

	start_pix = ndp * indx;		/* starting pixel */
	end_pix = start_pix + ndp;	/* ending pixel */

	start_line = y + start_pix / dx; /* truncate to form starting line */
	end_line = y + end_pix / dx;	/* truncate to form ending line */

	/* make sure lines don't access beyond the end of the buffer */
	if( start_line >= y1 ) start_line = y1-1;
	if( end_line >= y1 ) end_line = y1-1;

	/* if we should, clear a couple of scanlines */
	if( start_line != end_line ) {
	    fb = __GL_DEPTH_ADDR(dfb, (__GLzValue *), x, start_line );

	    skip = dfb->buf.outerWidth - w;
#ifdef WIN32
	    /* If we decide to support 16 bit incremental clears,
	       generalize to call FastMemClear2(). */
	    h = (end_line - start_line) + 1;
	    FastMemClear4(gc,
			  (unsigned long *) fb, w, h, 
			  skip*sizeof(unsigned long), z);
#else /* WIN32 */
	    w32 = w >> 5;
	    w4 = (w & 31) >> 2;
	    w1 = w & 3;

	    for( ; start_line < end_line; start_line++ ) {
		w = w32;
		while (--w >= 0) {
		    fb[0] = z; fb[1] = z; fb[2] = z; fb[3] = z;
		    fb[4] = z; fb[5] = z; fb[6] = z; fb[7] = z;
		    fb[8] = z; fb[9] = z; fb[10] = z; fb[11] = z;
		    fb[12] = z; fb[13] = z; fb[14] = z; fb[15] = z;
		    fb[16] = z; fb[17] = z; fb[18] = z; fb[19] = z;
		    fb[20] = z; fb[21] = z; fb[22] = z; fb[23] = z;
		    fb[24] = z; fb[25] = z; fb[26] = z; fb[27] = z;
		    fb[28] = z; fb[29] = z; fb[30] = z; fb[31] = z;
		    fb += 32;
		}
		w = w4;
		while (--w >= 0) {
		    fb[0] = z; fb[1] = z; fb[2] = z; fb[3] = z;
		    fb += 4;
		}
		w = w1;
		while (--w >= 0) {
		    *fb++ = z;
		}

		fb += skip;
	    }
#endif /* WIN32 */
	}

    }
}

/************************************************************************/

/*
** adjust the range of the z-buffer so that things work whenever
** we switch z-buffer clearing direction or scissor rect.
*/
static void Fix_zbuffer(__GLcontext *gc, __GLdepthBuffer *dfb, GLuint destRange)
{
    GLuint cnt = gc->depthBuffer.cnt;
    GLuint zDepthMask = gc->depthBuffer.zDepthMask;
    GLuint zClearInc = gc->depthBuffer.zClearInc;
    GLuint clearVal;
    __GLzValue *p;
    GLint x, y, x1, y1;
    GLint w, skip, i;

    x = gc->transform.clipX0;
    y = gc->transform.clipY0;
    x1 = gc->transform.clipX1;
    y1 = gc->transform.clipY1;
    if (((w = x1-x) == 0) || (y1-y == 0)) {
	return;
    }

#ifdef __GL_DEBUG_DEPTH_CLEAR
    if (getenv("DEBUG_ZBUFFER")) {
	printf("fixing from cnt: 0x%x to cnt: 0x%x\n", cnt, destRange);
    }
#endif

    /*
    ** there are several cases:
    ** 1. If we use accelerated z-buffering, then the depth clear value
    **    is at the end of the range (0.0 or 1.0), depending on the value
    **    of glDepthFunc.  In that case, use that value as a "background"
    **    value, if the current value of the buffer is outside the range
    **    for the current frame.
    ** 2. If we use accelerated z-buffering, and we change the scissor
    **    rectangle, then, in order to simplify things, we move the whole
    **    z-buffer in the 0xffxxxxxx block, and we start from there.  Then,
    **    the next time we do another scissor, we don't have to deal with
    **    identifying in which frame each pixel belongs, and if it is really
    **    above or below us..  We know that the scirror occured at the
    **    0xff block, so the next scirror will move the old scissor rectangle
    **    to the 0xff block and start from there again..  
    ** 3. We don't use accelerated z-buffering.  We have nothing to worry
    **    about.  This function does nothing.
    */

    /* decide what the clear value is */
    if ((zClearInc == 0xff000000) && (gc->state.depth.clear == 1.0)) {
	clearVal = destRange | zDepthMask;
    } else if ((zClearInc == 0x01000000) && (gc->state.depth.clear == 0.0)) {
	clearVal = destRange;
    } else {
	return;
    }

    /* figure out the beginning of the buffer */
    p = __GL_DEPTH_ADDR (dfb, (__GLzValue *), x, y);
    /* how much to skip from scanline to scanline */
    skip = dfb->buf.outerWidth - w;

    for (; y < y1; y++) {
	for (i=w; i; i--, p++) {
	    if ((p[0] & ~zDepthMask) == cnt) {
		p[0] = (p[0] & zDepthMask) | destRange;
	    } else {
		p[0] = clearVal;
	    }
	}
	p += skip;
    }
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
    GLenum testFunc = gc->state.depth.testFunc;

    /* check to see if we can use partial clear */
    fb->allowPartialZClear = GL_FALSE;
    if (fb->zClearInc != 0x00000000) {
	if( testFunc == GL_LESS || testFunc == GL_LEQUAL ) {
	    /* if we don't clear to the furthest point of the z-buffer */
	    if( gc->state.depth.clear == 1.0 ) {
		/* we can't accelerate z buffering */
		fb->allowPartialZClear = GL_TRUE;
	    }
	} else if( testFunc == GL_GREATER || testFunc == GL_GEQUAL ) {
	    /* if we don't clear to the closest point of the z-buffer */
	    if( gc->state.depth.clear == 0.0 ) {
		/* we can't accelate z buffering */
		fb->allowPartialZClear = GL_TRUE;
	    }
	}
    }

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


void __glInitDepth16(__GLdepthBuffer *fb, __GLcontext *gc )
{
    __glInitBuffer( &fb->buf, gc );

    fb->buf.elementSize = sizeof(__GLzValue16);
    fb->buf.depth = 16;

    fb->buf.resize = Resize;
    fb->buf.makeCurrent = MakeCurrent;
    fb->buf.loseCurrent = LoseCurrent;

    fb->pick = Pick;
    fb->clear = Clear;
    fb->store2 = StoreALWAYS16;
    fb->fetch = Fetch16;

    fb->scale = (__GLzValue) 0xffff0000;
    fb->numFracBits = 16;
    fb->cnt = 0x00000000;
    fb->zDepthMask = 0x0000ffff;
    fb->zClearInc = 0x00000000;
    fb->fullZClear = GL_TRUE;
    fb->testFunc = GL_LESS;
    fb->fixZBuffer = ( void (*)(__GLcontext *, __GLdepthBuffer *, GLuint) ) __glNop;
    fb->writeMask = (__GLzValue) ~0;
    fb->minResolution = 0x1000000;
}


void __glInitDepth24(__GLdepthBuffer *fb, __GLcontext *gc )
{
    __glInitBuffer( &fb->buf, gc );

    fb->buf.elementSize = sizeof(__GLzValue);
    fb->buf.depth = 24;

    fb->buf.resize = Resize;
    fb->buf.makeCurrent = MakeCurrent;
    fb->buf.loseCurrent = LoseCurrent;

    fb->pick = Pick;
    fb->clear = Clear;
    fb->store2 = StoreALWAYS;
    fb->fetch = FetchFastClear;

    fb->scale = (__GLzValue) 0x7fc00000;  /* XXXX  was 7fffff80 */
    fb->numFracBits = 7;
    fb->cnt = 0xff000000;
    fb->zDepthMask = 0x00ffffff;
    fb->zClearInc = 0xff000000;	/* -1 */
    fb->fullZClear = GL_FALSE;
    fb->testFunc = GL_LESS;
    fb->fixZBuffer = Fix_zbuffer;
    fb->writeMask = (__GLzValue) ~0;
    fb->minResolution = 0x1000000;
}


void __glInitDepth32(__GLdepthBuffer *fb, __GLcontext *gc )
{
    __glInitBuffer( &fb->buf, gc );

    fb->buf.elementSize = sizeof(__GLzValue);
    fb->buf.depth = 32;

    fb->buf.resize = Resize;
    fb->buf.makeCurrent = MakeCurrent;
    fb->buf.loseCurrent = LoseCurrent;

    fb->fixZBuffer = ( void (*)(__GLcontext *, __GLdepthBuffer *, GLuint) ) __glNop;
    fb->pick = Pick;
    fb->clear = Clear;
    fb->store2 = StoreALWAYS;
    fb->fetch = Fetch;

    fb->scale = (__GLzValue) 0x7fc00000; /* XXXX  was 7fffff80 */
    fb->numFracBits = 0;
    fb->cnt = 0x00000000;
    fb->zDepthMask = 0xffffffff;
    fb->zClearInc = 0x00000000;
    fb->fullZClear = GL_TRUE;
    fb->testFunc = GL_LESS;
    fb->fixZBuffer = ( void (*)(__GLcontext *, __GLdepthBuffer *, GLuint) ) __glNop;
    fb->writeMask = (__GLzValue) ~0;
    fb->minResolution = 0x1000000;
}

void __glInitDepth(__GLdepthBuffer *fb, __GLcontext *gc )
{
    int	zbits;

    if( getenv("GLFORCEZBITS") ) {
	zbits = atoi( getenv("GLFORCEZBITS") );
    } else {
	zbits = gc->modes.depthBits;
    }

    if( zbits <= 16 ) {
	__glInitDepth16( fb, gc );
    } else if( zbits <= 24 ) {
	__glInitDepth24( fb, gc );
    } else {
	__glInitDepth32( fb, gc );
    }
}

#ifndef __gl_buffers_h_
#define	__gl_buffers_h_

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
#include "render.h"
#include "procs.h"
#include <GL/glcore.h>
#ifdef __GL_SST
#include <glide.h>
#endif

typedef struct __GLbufferMachineRec {
    /*
    ** GL_TRUE if store procs need to call gc->front->store and 
    ** gc->back->store in order to store one fragment (only TRUE if 
    ** drawBuffer is GL_FRONT_AND_BACK).  This is needed because many
    ** frame buffers can draw to both front and back under some conditions
    ** (like when not blending), but not under other conditions.
    */
    GLboolean doubleStore;

    GLint lock;

#ifdef __GL_SST
    __GLbuffer *writebuffer;
    __GLbuffer *readbuffer;
    __GLbuffer *depthbuffer;
    __GLbuffer *auxbuffer;
#endif

} __GLbufferMachine;

#define __GL_LOCK_BUFFERS(gc) \
    if ((gc)->procs.lockBuffers != NULL && ++(gc)->buffers.lock == 1) { \
        (*(gc)->procs.lockBuffers)(gc); \
    }

#define __GL_UNLOCK_BUFFERS(gc) \
    if ((gc)->procs.unlockBuffers != NULL && --(gc)->buffers.lock == 0) { \
        (*(gc)->procs.unlockBuffers)(gc); \
    }

/************************************************************************/

/*
** Generic buffer description.  This description is used for software
** and hardware buffers of all kinds.
*/
struct __GLbufferRec {
    /*
    ** Which context is using this buffer.
    */
    __GLcontext *gc;

    /*
    ** Dimensions of the buffer.
    */
    GLint width, height, depth;

    /*
    ** Unaligned base of framebuffer.
    */
    void* ubase;

    /*
    ** Aligned base of framebuffer.
    */
    void* base;

    /*
    ** Number of bytes consumed by the framebuffer.
    */
    GLuint size;

    /*
    ** Size of each element in the framebuffer.
    */
    GLuint elementSize;
    GLuint elementSizeLog2;

    /*
    ** If this buffer is part of a larger (say full screen) buffer
    ** then this is the size of that larger buffer.  Otherwise it is
    ** just a copy of width.
    */
    GLint outerWidth;

    /*
    ** This is the number of bytes between rows in the buffer.  This
    ** is different than outerWidth which is the number of elements
    ** between rows.
    */
    GLint byteWidth;

    /*
    ** If this buffer is part of a larger (say full screen) buffer
    ** then these are the location of this buffer in the larger
    ** buffer.
    */
    GLint xOrigin, yOrigin;

    /*
    ** Private pointer used by particular buffer implementations to
    ** contain special data not supported by this generic structure.
    ** This is for "hardware" data.
    */
    void *other;

    /*
    ** These are kinda funny...  A buffer is conceptually a structure which
    ** is associated with the drawable.  Ie. The X Drawable containts the info
    ** as X would want to see it (one buffer), while OpenGL has miscalleneous
    ** other stuff associated, like stencil, depth, accum, etc. buffers.
    **
    ** The buffer structure explained here is not really a structure that is
    ** standalone, but it is the context's idea of a buffer.  So, in order words,
    ** the context's buffer is the sensient "object" that responds to methods,
    ** and the info stored in the drawable is a "persistent" copy of the data
    ** required to make a buffer "alive".
    **
    ** When a context becomes current, its buffers copy "in" the info that is 
    ** stored in the persistent copy.  When another context becomes current,
    ** the first one becomes "not current", which has the effect of making
    ** the old buffers write back "out" their information, so that the new
    ** buffers can copy it back "in".  It's like a cache coherency problem.
    **
    ** The reason there exist "caches" of the buffers in the contexts, instead
    ** of pointers to the actual buffers, is in order to avoid an additional
    ** level of indirection when accesing buffer info.
    **
    ** The reason why we use "write-back" instead of "write-thru" caches is
    ** mainly to avoid duplication of the "write-thru" mechanism whenever we
    ** want to alter a characteristic of the buffer.  With "write-back" caches
    ** the machanism is located is a central location, on a per-buffer-type
    ** basis, if needed.  Additionally, glXMakeCurrent() (and therefore the
    ** flushing of the queue) is assumed to be less frequent that all the 
    ** other operations that affect the buffers, so we will be trying to
    ** optimize those.
    **
    ** For multithreading, locks, signals, and "write-back" operations will
    ** have to be performed in an as-needed basis, but they idea is to
    ** try to keep those operations together with the rest of the buffer
    ** functionality.  If more performance is needed, then some "write-thru"
    ** operations may have to be performed, but I don't think this would
    ** be needed since the OS overhead for letting know the other thread that
    ** a buffer has changed will probably be time consuming on its own.
    **
    ** "methods" that every buffer should be able to respond to:
    **
    ** (init): "constructor". Called as __glInit*.  Not a part of the structure.
    ** makeCurrent: Make given context current for this buffer. Has the effect
    **		of copying data out of the drawable's info buffer to the 
    **		context's buffer structure.
    ** LoseCurrent:  Make given context not current for this buffer.
    **		Has the effect of "writing back" info from the buffer
    **		to the drawable's info buffer.
    ** free:   Deallocate memory for the buffer. A drawablePrivate is required
    **          since there is not active gc to grab it from.  This is a
    **		part of the perstitent structure.
    ** resize: Resize this buffer.
    ** move:   When the buffer moves inside the buffer than contains it.
    ** freebuf: Free the context specific data stored in the buffer structure.
    **		This is context data that, out of convenience, is stored in
    **		the context's side of the buffer.  Note that this data is
    **		not to be persistent. Such data does not exist in the generic
    **		buffer, but subclasses may need it.
    */
    void (*makeCurrent)(__GLbuffer *buf, __GLdrawableBuffer *drawableBuf,
			__GLcontext *gc );
    void (*loseCurrent)( __GLbuffer *buf, __GLcontext *gc );

    void (*resize)( __GLbuffer *buf, GLint w, GLint h );
    void (*move)( __GLbuffer *dfb, GLint x, GLint y );

    void (*freebuf)( __GLbuffer *buf, __GLcontext *gc);

    __GLdrawableBuffer *drawableBuf;
};

/*
** Generic address macro for a buffer.  Coded to assume that
** the buffer is not part of a larger buffer.
** The input coordinates x,y are biased by the x & y viewport
** adjusts in gc->transform, and thus they need to be de-adjusted
** here.
*/
#define	__GL_FB_ADDRESS(fb,cast,x,y) \
    (cast (((GLubyte *) (fb)->buf.base) \
	+ (((y) - (fb)->buf.gc->constants.viewportYAdjust) \
            * (fb)->buf.outerWidth \
	   + (x) - (fb)->buf.gc->constants.viewportXAdjust) \
	* (fb)->buf.elementSize))

/************************************************************************/

struct __GLcolorBufferRec {
    __GLbuffer buf;

    GLint redMax;
    GLint greenMax;
    GLint blueMax;

    GLboolean needColorFragmentOps;

    GLubyte *alphaTestFuncTable;
    GLubyte *indexTestFuncTable;

    /*
    ** Color component scale factors.  Given a component value between
    ** zero and one, this scales the component into a zero-N value
    ** which is suitable for usage in the color buffer.  Note that these
    ** values are not necessarily the same as the max values above,
    ** which define precise bit ranges for the buffer.  These values
    ** are never zero, for instance.
    **/
    __GLfloat redScale;
    __GLfloat greenScale;
    __GLfloat blueScale;

    /* Integer versions of above */
    GLint iRedScale;
    GLint iGreenScale;
    GLint iBlueScale;

    /* Used primarily by pixmap code */
    GLint redShift;
    GLint greenShift;
    GLint blueShift;
    GLint alphaShift;

    /*
    ** Alpha is treated a little bit differently.  alphaScale and
    ** iAlphaScale are used to define a range of alpha values that are
    ** generated during various rendering steps.  These values will then
    ** be used as indices into a lookup table to see if the alpha test
    ** passes or not.  Because of this, the number should be fairly large
    ** (e.g., one is not good enough).
    */
    __GLfloat alphaScale;
    GLint iAlphaScale;

    __GLfloat oneOverRedScale;
    __GLfloat oneOverGreenScale;
    __GLfloat oneOverBlueScale;
    __GLfloat oneOverAlphaScale;

    /*
    ** Color mask state for the buffer.  When writemasking is enabled
    ** the source and dest mask will contain depth depedent masking.
    */
    GLuint sourceMask, destMask;

    /*
    ** This function updates the internal procedure pointers based
    ** on a state change in the context.
    */
    void (*pick)(__GLcontext *gc, __GLcolorBuffer *cfb);

    /*
    ** Store a fragment into the buffer.  For color buffers, the
    ** procedure will optionally dither, writemask, blend and logic op
    ** the fragment before final storage.
    */
    void (*store)(__GLcolorBuffer *cfb, const __GLfragment *frag);

    /*
    ** Fetch a color from the buffer.  This returns the r, g, b and a
    ** values for an RGB buffer.  For an index buffer the "r" value
    ** returned is the index.
    */
    void (*fetch)(__GLcolorBuffer *cfb, GLint x, GLint y,
		  __GLcolor *result);

    /*
    ** Similar to fetch, except that the data is always read from
    ** the current read buffer, not from the current draw buffer.
    */
    void (*readColor)(__GLcolorBuffer *cfb, GLint x, GLint y,
		      __GLcolor *result);
    GLboolean (*readSpan)(__GLcolorBuffer *cfb, GLint x, GLint y,
		          __GLcolor *results, GLint w);

    /*
    ** Return a span of data from the accumulation buffer into the
    ** color buffer(s), multiplying by "scale" before storage.
    */
    void (*returnSpan)(__GLcolorBuffer *cfb, GLint x, GLint y,
		       const __GLaccumCell *acbuf, __GLfloat scale, GLint w);

    /*
    ** Store a span (line) of colors into the color buffer.  A minimal
    ** implementation need only copy the values directly into
    ** the framebuffer, assuming that the PickSpanProcs is providing
    ** software implementations of all of the modes.
    */
    __GLspanFunc storeSpan;
    __GLstippledSpanFunc storeStippledSpan;
    __GLspanFunc storeLine; 
    __GLstippledSpanFunc storeStippledLine;

    /*
    ** Read a span (line) of colors from the color buffer.  The returned
    ** format is in the same format used for storage.
    */
    __GLspanFunc fetchSpan;
    __GLstippledSpanFunc fetchStippledSpan;
    __GLspanFunc fetchLine;
    __GLstippledSpanFunc fetchStippledLine;

#ifdef __GL_PC_RAST
    /*
    ** Framebuffer characterization for rasterization code.
    ** See fr_fbtype.h for interpretation.  Used for RGB only.
    */
    GLint fbtype;
#endif
    /*
    ** Clear the scissor area of the color buffer, clipped to
    ** the window size.  Apply dithering if enabled.
    */
    void (*clear)(__GLcolorBuffer *cfb);

    /*
    ** Private pointer used by particular buffer implementations to
    ** contain special data not supported by this generic structure.
    */
    void *other;
};

/* generic span read routine */
extern GLboolean __glReadSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
			      __GLcolor *results, GLint w);

/* generic accum return span routine */
extern void __glReturnSpan(__GLcolorBuffer *cfb, GLint x, GLint y,
			   const __GLaccumCell *ac, __GLfloat scale,
			   GLint w);

/* generic span fetch routine */
extern GLboolean __glFetchSpan(__GLcontext *gc);

/************************************************************************/

struct __GLdepthBufferRec {
    __GLbuffer buf;

    GLuint writeMask;

    /*
    ** Scale factor used to convert users ZValues (0.0 to 1.0, inclusive)
    ** into this depth buffers range.
    */
    GLuint scale;

    /*
    ** For fast depth buffer clears the high order bits of each depth value
    ** are the frame count bits (in order to partition the zbuffer clear to
    ** improve performance) This is actually the shifted count (for 8 count
    ** bits it is 0x??000000).
    */
    GLuint cnt;

    /*
    ** the frame count for the rest of the frame buffer if scissoring
    ** is enabled
    */
    GLuint scissorCnt;

    /*
    ** number of fractional bits in the iteration. In the above case
    ** of 8 count bits, the number of fractional bits can be anywhere
    ** between 0 and 8.  Explanation: z iteration is performed at a
    ** higher resolution than what is stored.  Let's say that iteration
    ** is performed at 32 bits, and using 8 count bits, we store 24 bits.
    ** Then we have 8 frac bits.  If iteration was done at, say, 28 bits,
    ** then we have 4 frac bits.
    */
    GLint numFracBits;

    /*
    ** a mask specifying which bits are alterable by this z buffer
    ** implementation.  Note that this is different from 
    ** gc->depthBuffer.scale, since the scale can be less than 2^n - 1.
    ** This one always has the form 2^n-1 ( regular expression form: 0*1* )
    */
    GLuint zDepthMask;

    /* 
    ** this is an an indication of the current direction of clearing
    ** the zbuffer.  It is the actual constant that is being added
    ** to 'cnt', so it is either 0x01000000 or 0xff000000.  It is
    ** 0x00000000 if no accelerated zbuffering is being performed.
    ** The pick routines use this in order to pick the correct zbuffer
    ** routines.
    */
    GLuint zClearInc;

    /*
    ** this is an indication that something nasty has happened, so
    ** during the next clear we will clear the whole zbuffer to some
    ** sane value (instead of performing an incremental clear).
    */
    GLboolean fullZClear;
    GLboolean allowPartialZClear;

    /*
    ** an easy-to-access indicator telling the user if we are using
    ** a regular or inverted z buffer range.  If invertZRange is false,
    ** then testFunc is equal to state.depth.testFunc, otherwise testFunc
    ** is inverted from state.depth.testFunc.
    */
    GLboolean invertZRange;
    GLenum testFunc;

    /*
    ** the minimum resolvable difference for PolygonOffset
    */
    __GLfloat minResolution;
    /*
    ** a function that goes through the zbuffer and adjusts all the
    ** entries to a consistent range.  It is used in nasty
    ** situations, like changing the direction of the z compare
    ** (LESS to GREATER, for example), or altering the scissoring rect.
    */
    void (*fixZBuffer)(__GLcontext *gc, __GLdepthBuffer *dfb,
			GLuint destRange );

    /*
    ** This function updates the internal procedure pointers based
    ** on a state change in the context.
    */
    void (*pick)(__GLcontext *gc, __GLdepthBuffer *dfb, GLint depthIndex);

    /*
    ** Attempt to update the depth buffer using z.  If the depth function
    ** passes then the depth buffer is updated and True is returned,
    ** otherwise False is returned.  The caller is responsible for
    ** updating the stencil buffer.
    */
    GLboolean (*store)(__GLdepthBuffer *dfb, GLint x, GLint y, __GLzValue z);

    /*
    ** Clear the scissor area of the buffer clipped to the window
    ** area.  No other modes apply.
    */
    void (*clear)(__GLdepthBuffer *dfb);

    /*
    ** Direct access routines used by ReadPixels(), WritePixels(), 
    ** CopyPixels().
    */
    GLboolean (*store2)(__GLdepthBuffer *dfb, GLint x, GLint y, __GLzValue z);
    __GLzValue (*fetch)(__GLdepthBuffer *dfb, GLint x, GLint y);
};

/*
** the persistent version
*/
typedef struct __GLdepthBufferPRec __GLdepthBufferP;
struct __GLdepthBufferPRec {
    /* extra things that this buffer needs */
    GLuint writeMask;
    GLuint scale;
    GLuint cnt;
    GLint numFracBits;
    GLuint zDepthMask;
    GLuint zClearInc;
    GLboolean fullZClear;
    GLboolean invertZRange;
};

#define	__GL_DEPTH_ADDR(a,b,c,d) __GL_FB_ADDRESS(a,b,c,d)

/************************************************************************/

struct __GLstencilBufferRec {
    __GLbuffer buf;

    /*
    ** Stencil test lookup table.  The stencil buffer value is masked
    ** against the stencil mask and then used as an index into this
    ** table which contains either GL_TRUE or GL_FALSE for the
    ** index.
    */
    GLboolean *testFuncTable;

    /*
    ** Stencil op tables.  These tables contain the new stencil buffer
    ** value given the old stencil buffer value as an index.
    */
    __GLstencilCell *failOpTable;
    __GLstencilCell *depthFailOpTable;
    __GLstencilCell *depthPassOpTable;

    /*
    ** This function updates the internal procedure pointers based
    ** on a state change in the context.
    */
    void (*pick)(__GLcontext *gc, __GLstencilBuffer *sfb);

    /*
    ** Store a fragment into the buffer.
    */
    void (*store)(__GLstencilBuffer *sfb, GLint x, GLint y,
		  GLint value);

    /* 
    ** Fetch a value.
    */
    GLint (*fetch)(__GLstencilBuffer *sfb, GLint x, GLint y);

    /*
    ** Return GL_TRUE if the stencil test passes.
    */
    GLboolean (*testFunc)(__GLstencilBuffer *sfb, GLint x, GLint y);

    /*
    ** Apply the stencil ops to this position.
    */
    void (*failOp)(__GLstencilBuffer *sfb, GLint x, GLint y);
    void (*passDepthFailOp)(__GLstencilBuffer *sfb, GLint x, GLint y);
    void (*depthPassOp)(__GLstencilBuffer *sfb, GLint x, GLint y);

    /*
    ** Clear the scissor area of the buffer clipped to the window
    ** area.  No other modes apply.
    */
    void (*clear)(__GLstencilBuffer *sfb);
};

#define	__GL_STENCIL_ADDR(a,b,c,d) __GL_FB_ADDRESS(a,b,c,d)

/************************************************************************/

struct __GLaccumBufferRec {
    __GLbuffer buf;

    /*
    ** Scaling factors to convert from color buffer values to accum
    ** buffer values.
    */
    __GLfloat redScale;
    __GLfloat greenScale;
    __GLfloat blueScale;
    __GLfloat alphaScale;

    __GLfloat oneOverRedScale;
    __GLfloat oneOverGreenScale;
    __GLfloat oneOverBlueScale;
    __GLfloat oneOverAlphaScale;

    /*
    ** This function updates the internal procedure pointers based
    ** on a state change in the context.
    */
    void (*pick)(__GLcontext *gc, __GLaccumBuffer *afb);

    /*
    ** Clear a rectangular region in the buffer.  The scissor area is
    ** cleared.
    */
    void (*clear)(__GLaccumBuffer *afb);

    /*
    ** Accumulate data into the accum buffer.
    */
    void (*accumulate)(__GLaccumBuffer *afb, __GLfloat value);

    /*
    ** Load data into the accum buffer.
    */
    void (*load)(__GLaccumBuffer *afb, __GLfloat value);

    /*
    ** Return data from the accum buffer to the current framebuffer.
    */
    void (*ret)(__GLaccumBuffer *afb, __GLfloat value);

    /*
    ** Multiply the accum buffer by the value.
    */
    void (*mult)(__GLaccumBuffer *afb, __GLfloat value);


    /*
    ** Add the value to the accum buffer.
    */
    void (*add)(__GLaccumBuffer *afb, __GLfloat value);
};

#define	__GL_ACCUM_ADDRESS(a,b,c,d) __GL_FB_ADDRESS(a,b,c,d)

/************************************************************************/

struct __GLownershipBufferRec {
    __GLbuffer buf;
};

#define	__GL_OWNERSHIP_ADDRESS(fb,cast,x,y) \
    (cast (((GLubyte *) (fb)->buf.base) \
	+ (((y) - (fb)->buf.gc->constants.viewportYAdjust) \
            * (fb)->buf.outerWidth \
	   + (((x) - (fb)->buf.gc->constants.viewportXAdjust) >> 3)) \
	* (fb)->buf.elementSize))

/************************************************************************/

/* buffer specific routines */

extern void __glInitCI(__GLcolorBuffer *cfb, __GLcontext *gc );
extern void __glInitRGB(__GLcolorBuffer *cfb, __GLcontext *gc );

extern void __glInitAccum64(__GLaccumBuffer *afb, __GLcontext *gc );

extern void __glInitStencil8(__GLstencilBuffer *sfb, __GLcontext *gc );

extern void __glInitDepth(__GLdepthBuffer *dfb, __GLcontext *gc );
extern void __glInitDepth24(__GLdepthBuffer *dfb, __GLcontext *gc );
extern void __glInitDepth32(__GLdepthBuffer *dfb, __GLcontext *gc );

/************************************************************************/

/* generic buffer routines */

extern void __glInitBuffer(__GLbuffer *buf, __GLcontext *gc);
extern void __glMakeCurrentBuffer(__GLbuffer *buf,
				  __GLdrawableBuffer *drawableBuf,
				  __GLcontext *gc);
extern void __glLoseCurrentBuffer(__GLbuffer *buf, __GLcontext *gc);
extern void __glResizeBuffer(__GLbuffer *buf, GLint w, GLint h);
extern void __glMoveBuffer(__GLbuffer *buf, GLint x, GLint y);
extern void __glFreeBuffer(__GLdrawableBuffer *drawableBuf,
			    __GLdrawablePrivate *dp);

#if defined(__GL_SUPPORT_MGL)
extern void __glChangeBuffers(__GLcontext *gc);
#endif
extern void __glLockBuffer(__GLcontext *gc);
extern void __glUnlockBuffer(__GLcontext *gc);
extern void __glLockBuffersDoubleStore(__GLcontext *gc);
extern void __glUnlockBuffersDoubleStore(__GLcontext *gc);

extern GLboolean __glTestOwnership(__GLcontext *gc, GLint x, GLint y);

#endif /* __gl_buffers_h_ */

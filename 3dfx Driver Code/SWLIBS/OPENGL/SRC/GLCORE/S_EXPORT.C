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
#include <stdio.h>
#include "context.h"
#include "imports.h"
#include "machdep.h"
#include "global.h"
#include "g_imfncs.h"
#include "dlistopt.h"
#include "g_disp.h"

#if defined(__GL_USE_MIPSASMCODE)
#include "mips.h"
#endif

/************************************************************************/

static void CheckViewport(__GLcontext *gc)
{
    GLint llx, lly, urx, ury;
    GLint x0, x1, y0, y1;
    GLboolean oldReasonableViewport;

    /*
    ** If this viewport is fully contained in the window, we note this fact,
    ** and this can save us on scissoring tests.
    */
    x0 = gc->transform.clipX0;
    x1 = gc->transform.clipX1;
    y0 = gc->transform.clipY0;
    y1 = gc->transform.clipY1;

    llx = gc->state.viewport.x + gc->constants.viewportXAdjust;
    lly = gc->state.viewport.y + gc->constants.viewportYAdjust;
    urx = llx + gc->state.viewport.width;
    ury = lly + gc->state.viewport.height;

    oldReasonableViewport = gc->transform.reasonableViewport;
    if (llx >= x0 && lly >= y0 && urx <= x1 && ury <= y1) {
	gc->transform.reasonableViewport = GL_TRUE;
    } else {
	gc->transform.reasonableViewport = GL_FALSE;
    }
    if (oldReasonableViewport != gc->transform.reasonableViewport) {
	__GL_DELAY_VALIDATE(gc);
    }
}

static GLboolean ApplyViewport(__GLcontext *gc)
{
    if (__glFindWindowSize(gc) == GL_TRUE) {
	__glUpdateViewport(gc);
	CheckViewport(gc);
	return GL_TRUE;
    }

    return GL_FALSE;
}

static GLboolean ApplyScissor(__GLcontext *gc)
{
    if (__glFindWindowSize(gc) == GL_TRUE) {
	CheckViewport(gc);
	return GL_TRUE;
    }

    return GL_FALSE;
}

static GLboolean ChangeDrawableSize(__GLcontext *gc, GLint w, GLint h)
{
    if (__glChangeWindowSize(gc, w, h) == GL_TRUE) {
	__glUpdateViewport(gc);
	CheckViewport(gc);
	return GL_TRUE;
    }

    return GL_FALSE;
}

static GLboolean ChangeDrawableLocation(__GLcontext *gc, GLint x, GLint y, GLint w, GLint h)
{
    if (__glChangeWindowSize(gc, w, h) == GL_TRUE) {
	__glUpdateViewport(gc);
	CheckViewport(gc);
	return GL_TRUE;
    }

    return GL_FALSE;
}

static void LockBuffers(__GLcontext *gc)
{
    __GL_LOCK_BUFFER_MASK(gc, __GL_ALL_BUFFER_MASK);

    /* XXX: wasteful, but a portable way of incrementing lockCnt  */
    __GL_LOCK_RENDER_BUFFERS(gc);
}

static void UnlockBuffers(__GLcontext *gc)
{
    __GL_UNLOCK_BUFFER_MASK(gc, __GL_ALL_BUFFER_MASK);

    /* XXX: wasteful, but a portable way of incrementing lockCnt  */
    __GL_UNLOCK_RENDER_BUFFERS(gc);
}

/*
** GL_TRUE means we don't have to lock buffers
** GL_FALSE means we have to lock buffers
*/
static GLboolean ObtainSLock(__GLcontext *gc)
{
    return (gc->buffers.lock.sLockCnt++ != 0);
}

/*
** GL_TRUE means we don't have to lock buffers
** GL_FALSE means we have to lock buffers
*/
static GLboolean ReleaseSLock(__GLcontext *gc)
{
    return (--gc->buffers.lock.sLockCnt != 0);
}

static void LockDevice(__GLcontext *gc)
{
}

static void UnlockDevice(__GLcontext *gc)
{
}

/************************************************************************/

static void Finish(__GLcontext *gc)
{
    __GL_API_FLUSH();
}

static void Flush(__GLcontext *gc)
{
    __GL_API_FLUSH();
}

static void __fastcall WaitIdle(__GLcontext *gc)

{
    /* This is a noop in a pure software implementation */
}

/************************************************************************/

/*
** initialize context pointers
*/
static void InitFnPtrs(__GLcontext *gc)
{
    gc->procs.ec1 = __glDoEvalCoord1;
    gc->procs.ec2 = __glDoEvalCoord2;
    gc->procs.bitmap = __glDrawBitmap;
    gc->procs.rect = __glRect;
    gc->procs.clipPolygon = __glClipPolygon;
    gc->procs.validate = __glGenericValidate;
    gc->procs.convertPolygonStipple = __glConvertStipple;

    gc->procs.pushMatrix = __glPushModelViewMatrix;
    gc->procs.popMatrix = __glPopModelViewMatrix;
    gc->procs.loadIdentity = __glLoadIdentityModelViewMatrix;

    gc->procs.matrix.copy = __glCopyMatrix;
    gc->procs.matrix.invertTranspose = __glInvertTransposeMatrix;
    gc->procs.matrix.makeIdentity = __glMakeIdentity;
#if defined(__GL_USE_MIPSASMCODE)
    gc->procs.matrix.mult = __glMipsMultMatrix;
#else
    gc->procs.matrix.mult = __glMultMatrix;
#endif
    gc->procs.computeInverseTranspose = __glComputeInverseTranspose;
#if defined(__GL_USE_MIPSASMCODE)
    gc->procs.normalize = __glMipsNormalize;
#else
    gc->procs.normalize = __glNormalize;
#endif

    gc->procs.beginPrim[GL_LINE_LOOP] = __glBeginLLoop;
    gc->procs.beginPrim[GL_LINE_STRIP] = __glBeginLStrip;
    gc->procs.beginPrim[GL_LINES] = __glBeginLines;
    gc->procs.beginPrim[GL_POINTS] = __glBeginPoints;
    gc->procs.beginPrim[GL_POLYGON] = __glBeginPolygon;
    gc->procs.beginPrim[GL_TRIANGLE_STRIP] = __glBeginTStrip;
    gc->procs.beginPrim[GL_TRIANGLE_FAN] = __glBeginTFan;
    gc->procs.beginPrim[GL_TRIANGLES] = __glBeginTriangles;
    gc->procs.beginPrim[GL_QUAD_STRIP] = __glBeginQStrip;
    gc->procs.beginPrim[GL_QUADS] = __glBeginQuads;
    gc->procs.endPrim = __glEndPrim;

    gc->procs.vertex = (void (*)(__GLcontext*, __GLvertex*)) __glNop;
    gc->procs.rasterPos2 = __glRasterPos2;
    gc->procs.rasterPos3 = __glRasterPos3;
    gc->procs.rasterPos4 = __glRasterPos4;

    /*
    ** Load in the device specific pick procs.  Do this before
    ** resetting the default state as that may need pick procs.
    */
    gc->procs.pickAllProcs = __glGenericPickAllProcs;
    gc->procs.pickBlendProcs = __glGenericPickBlendProcs;
    gc->procs.pickBufferProcs = __glGenericPickBufferProcs;
    gc->procs.pickClipProcs = __glGenericPickClipProcs;
    gc->procs.pickColorMaterialProcs = __glGenericPickColorMaterialProcs;
    gc->procs.pickFogProcs = __glGenericPickFogProcs;
    gc->procs.pickTransformProcs = __glGenericPickTransformProcs;
    gc->procs.pickCullVertexProcs = __glGenericPickCullVertexProcs;
    gc->procs.pickLineProcs = __glGenericPickLineProcs;
    gc->procs.pickMatrixProcs = __glGenericPickMatrixProcs;
    gc->procs.pickInvTransposeProcs = __glGenericPickInvTransposeProcs;
    gc->procs.pickMvpMatrixProcs = __glGenericPickMvpMatrixProcs;
    gc->procs.pickParameterClipProcs = __glGenericPickParameterClipProcs;
    gc->procs.pickPixelProcs = __glGenericPickPixelProcs;
    gc->procs.pickPointProcs = __glGenericPickPointProcs;
    gc->procs.pickRenderBitmapProcs = __glOptPickRenderBitmapProcs;

    gc->procs.pickSpanProcs = __glGenericPickSpanProcs;
    gc->procs.pickStoreProcs = __glGenericPickStoreProcs;
    gc->procs.pickTextureProcs = __glGenericPickTextureProcs;
    gc->procs.pickCalcTextureProcs = __glGenericPickCalcTextureProcs;
    gc->procs.pickTriangleProcs = __glGenericPickTriangleProcs;
    gc->procs.pickVertexProcs = __glGenericPickVertexProcs;
    gc->procs.pickVertexArrayProcs = __glGenericPickVertexArrayProcs;
    gc->procs.pickDepthProcs = __glGenericPickDepthProcs;

    gc->procs.copyImage = __glGenericPickCopyImage;
    gc->procs.readImage = __glGenericPickReadImage;
    gc->procs.fetchTex     = __glGenericPickGetTexture;
    gc->procs.storeTex     = __glGenericPickStoreTexture;

    gc->procs.pixel.spanReadCI = __glSpanReadCI;
    gc->procs.pixel.spanReadCI2 = __glSpanReadCI2;
    gc->procs.pixel.spanReadRGBA = __glSpanReadRGBA;
    gc->procs.pixel.spanReadRGBA2 = __glSpanReadRGBA2;
    gc->procs.pixel.spanReadDepth = __glSpanReadDepth;
    gc->procs.pixel.spanReadDepth2 = __glSpanReadDepth2;
    gc->procs.pixel.spanReadStencil = __glSpanReadStencil;
    gc->procs.pixel.spanReadStencil2 = __glSpanReadStencil2;
    gc->procs.pixel.spanRenderCI = __glSpanRenderCI;
    gc->procs.pixel.spanRenderCI2 = __glSpanRenderCI2;
    gc->procs.pixel.spanRenderRGBA = __glSpanRenderRGBA;
    gc->procs.pixel.spanRenderRGBA2 = __glSpanRenderRGBA2;
    gc->procs.pixel.spanRenderDepth = __glSpanRenderDepth;
    gc->procs.pixel.spanRenderDepth2 = __glSpanRenderDepth2;
    gc->procs.pixel.spanRenderStencil = __glSpanRenderStencil;
    gc->procs.pixel.spanRenderStencil2 = __glSpanRenderStencil2;

    gc->procs.applyScissor = ApplyScissor;
    gc->procs.applyViewport = ApplyViewport;
    gc->procs.computeClipBox = __glComputeClipBox;
    gc->procs.finish = Finish;
    gc->procs.flush = Flush;

    gc->procs.waitIdle = WaitIdle;

    gc->procs.varray_funcs = __gl_varray_funcs;

    gc->procs.colortable = __glColorTableEXT;
    gc->procs.colorsubtable = __glColorSubTableEXT;
}

/*
** initialize context's side buffers
*/
static void InitBuffers(__GLcontext *gc)
{
    /*
    ** Dirty all bits so that all state will be recomputed when the next
    ** primitive is attempted. (Much of the state depends upon the
    ** color buffer scale factors, and may need to be recomputed when
    ** going from pixmap to hardware).
    */
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_ALL);

    /* Initialize front/back color buffer(s) */
    gc->front = &gc->frontBuffer;
    if (gc->modes.doubleBufferMode) {
	gc->back = &gc->backBuffer;
	if (gc->modes.colorIndexMode) {
	    __glInitCI(gc->front, gc);
	    __glInitCI(gc->back, gc);
	} else {
	    __glInitRGB(gc->front, gc);
	    __glInitRGB(gc->back, gc);
	}
    } else {
	if (gc->modes.colorIndexMode) {
	    __glInitCI(gc->front, gc);
	} else {
	    __glInitRGB(gc->front, gc);
	}
    }

#if __GL_MAX_AUXBUFFERS > 0
    /* Initialize any aux color buffers */
    if (gc->modes.maxAuxBuffers > 0) {
	GLint i;

	for (i = 0; i < gc->modes.maxAuxBuffers; ++i) {
	    if (gc->modes.colorIndexMode) {
		__glInitCI(&gc->auxBuffer[i], gc);
	    } else {
		__glInitRGB(&gc->auxBuffer[i], gc);
	    }
	}
    }
#endif

    /* Initialize any other ancillary buffers */
    if (gc->modes.haveAccumBuffer) {
	__glInitAccum64(&gc->accumBuffer, gc);
    }
    if (gc->modes.haveDepthBuffer) {
	__glInitDepth(&gc->depthBuffer, gc);
    } else {
	/*
	** Set the scale factor to allow window z values to be computed.
	** Set it not to use the high bit (to avoid floating point
	** exceptions) or low bits (to match floating point precision).
	*/
	gc->depthBuffer.scale = 0x7fffff80;
    }
    if (gc->modes.haveStencilBuffer) {
	__glInitStencil8(&gc->stencilBuffer, gc);
    }

    __glInitBuffer(&gc->ownershipBuffer.buf, gc);

    __glUpdateDepthRange(gc);

    /* initialize buffer lock pointers, etc. */
    gc->buffers.lock.obtainSLock = ObtainSLock;
    gc->buffers.lock.releaseSLock = ReleaseSLock;
    gc->buffers.lock.lockDevice = LockDevice;
    gc->buffers.lock.unlockDevice = UnlockDevice;
}

/************************************************************************/

static void SwapBuffers(__GLcontext *gc)
{
    gc->procs.flush(gc);

#if 0
    __GLDDcontext *hwcx = (__GLDDcontext *) gc;
    __GLcolorBuffer *temp;

    if (!gc->modes.doubleBufferMode) {
	return;
    }

    /* exchange front and back buffer pointers */
    temp = gc->front;
    gc->front = gc->back;
    gc->back = temp;
    gc->drawBuffer = temp;
    hwcx->displayBank = 1 - hwcx->displayBank;
#endif
}

/************************************************************************/

static GLboolean DestroyContext(__GLcontext *gc)
{
    /* 
    ** Free ancillary buffer related data.  Note that these calls do
    ** *not* free software ancillary buffers, just any related data
    ** stored in them.
    */
    __glFreeBufData(gc);

    /* Destroy rest of software context */
    __glDestroyContext(gc);

    /* Free memory for the host cpu hw context */
    (*gc->imports.free)(gc, gc);

    return GL_TRUE;
}

static GLboolean LoseCurrent(__GLcontext *gc)
{
    /* 
    ** Illegal to makeCurrent when the current context is in selection or 
    ** feedback mode.
    */
    if (gc->renderMode != GL_RENDER || __gl_beginMode == __GL_IN_BEGIN)
	return GL_FALSE;

    __glLoseCurrentBuffers( gc, ((__GLDDcontext *)gc)->displayBank );

    gc->beginMode = __gl_beginMode;
#ifdef _WINDOWS
    __glSetTLSCXValue(NULL);
#else /* _WINDOWS */
    __gl_context = NULL;
#endif /* _WINDOWS */

    return GL_TRUE;
}

/*
** Make this context the current context for this process.
*/
static GLboolean MakeCurrent(__GLcontext *gc)
{
    __GLDDcontext *hwcx = (__GLDDcontext *)gc;

    /* Load up global variables for the new context */
#ifdef _WINDOWS
    __glSetTLSCXValue(gc);
#else /* _WINDOWS */
    __gl_context = gc;
#endif /* _WINDOWS */
    __gl_beginMode = gc->beginMode;

    /* Initialize dispatch tables */
    gc->dispatchState = &gc->currentDispatchState;
    gc->currentDispatchState = __glImmedState;
    gc->listCompState = __glListCompState;

    /*
    ** Dirty the depth mask, since we may need to update depth pointers.
    */
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);

    /* Entering HW mode rendering */

    /*
    ** This initializes context's side buffer structures and various
    ** other pointers.
    */
    if( (gc->gcState & __GL_HW_MODE) == 0 ) {
	InitBuffers(gc);
    }

    /*
    ** page in buffer info
    */
    __glMakeCurrentBuffers( gc, &hwcx->displayBank );

    /* init pointers only when entering hw mode */
    if ((gc->gcState & __GL_HW_MODE) == 0 ) {
	InitFnPtrs(gc);
	gc->exports.swapBuffers = SwapBuffers;
	gc->exports.destroyContext = DestroyContext;
    }

    /* Reset the context if this is the first MakeCurrent */
    if ((gc->gcState & (__GL_HW_MODE | __GL_PIXMAP_MODE)) == 0) {
	GLint width, height;

	/* Now reset the context to its default state */
	__glSoftResetContext(gc);

	/*
	** The first time a context is made current the spec requires that
	** the viewport and scissor be initialized.
	*/
	(*gc->imports.getDrawableSize)(gc, &width, &height);
#ifdef __GL_ICD
	(*gc->currentDispatchState.dispatch.Viewport)(0, 0, width, height);
	(*gc->currentDispatchState.dispatch.Scissor)(0, 0, width, height);
#else /* __GL_ICD */
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
#endif /* __GL_ICD */
    } else {
	/*
	** We may have never bound to this window before, find out what
	** its size is.
	*/
	(*gc->procs.applyViewport)(gc);
	(*gc->procs.applyScissor)(gc);
    }

    /* Reset the HW context if this is the first device dependent MakeCurrent */
    if ((gc->gcState & __GL_INIT_HW) == 0) {
	/*
	** Now that we have a hardware context, we can initialize
	** all the proc pointers.
	*/
	(*gc->procs.validate)(gc);
    }

    /*
    ** Scale all state that depends upon the color scales.
    */
    __glContextSetColorScales(gc);

    /*
    ** NOTE: now that context is initialized reset to use the global
    ** table
    */
#ifdef _WINDOWS
    if (gc->dlist.currentList) {
	gc->dispatchState = &gc->savedDispatchState;
	__glCopyDispatch(&gc->listCompState, &__gl_dispatch);
    } else {
	gc->dispatchState = &__gl_dispatch;
    }
#else /* _WINDOWS */
    if (__gl_dispatchOverride) {
        if (gc->dlist.currentList) {
            gc->dispatchState = &gc->savedDispatchState;
            gc->currentDispatchState = gc->listCompState;
        } else {
            gc->dispatchState = &gc->currentDispatchState;
        }
    } else {
        if (gc->dlist.currentList) {
            gc->dispatchState = &gc->savedDispatchState;
            __gl_dispatch = gc->listCompState;
        } else {
            gc->dispatchState = &__gl_dispatch;
            __gl_dispatch = gc->currentDispatchState;
        }
    }
#endif /* _WINDOWS */

    gc->gcState |= __GL_INIT_HW | __GL_HW_MODE;
    gc->gcState &= ~__GL_PIXMAP_MODE;

    return GL_TRUE;
}

static GLboolean ShareContext(__GLcontext *dst, __GLcontext *src)
{
    /*
    ** Set up the sharable state: currently display lists and texture objects.
    */
    __glShareDlist(dst, src);
    __glShareTextureObjects(dst, src);

    return GL_TRUE;
}

/*
** Create a new context.  The new context is not made current.
** This can fail, but the caller is responsible for dealing with it.
*/
__GLcontext *__glCoreCreateContext(__GLimports *imports, __GLcontextModes *modes)
{
    __GLDDcontext *hwcx;
    __GLcontext *gc;

    /* Allocate memory for host cpu hardware context */
    hwcx = (__GLDDcontext *) (*imports->calloc)(0, 1, sizeof(__GLDDcontext));
    if (!hwcx) {
	return 0;
    }
    gc = &hwcx->gc;
    gc->imports = *imports;
    gc->modes = *modes;

    /*
    ** Load some device specific constants into the context
    */
    gc->constants.maxViewportWidth = __GL_DEFAULT_MAX_WINDOW_WIDTH;
    gc->constants.maxViewportHeight = __GL_DEFAULT_MAX_WINDOW_HEIGHT;
    gc->constants.viewportXAdjust =
	__GL_DEFAULT_VERTEX_X_BIAS + __GL_DEFAULT_VERTEX_X_FIX;
    gc->constants.viewportYAdjust =
	__GL_DEFAULT_VERTEX_Y_BIAS + __GL_DEFAULT_VERTEX_Y_FIX;
    gc->constants.subpixelBits = __GL_DEFAULT_COORD_SUBPIXEL_BITS;

#if defined(__GL_DEVICE_COLOR_SCALE)
    gc->constants.redScale = 255.0F;
    gc->constants.greenScale = 255.0F;
    gc->constants.blueScale = 255.0F;
    gc->constants.alphaScale = 255.0F;
#endif

#if defined(__GL_DEVICE_DEPTH_SCALE)
    gc->constants.depthScale = 1.0F;
#endif

    gc->constants.numberOfLights = __GL_DEFAULT_NUMBER_OF_LIGHTS;
    gc->constants.numberOfClipPlanes = __GL_DEFAULT_NUMBER_OF_CLIP_PLANES;
    gc->constants.numberOfTextures = __GL_DEFAULT_NUMBER_OF_TEXTURES;
    gc->constants.numberOfTextureEnvs = __GL_DEFAULT_NUMBER_OF_TEXTURE_ENVS;
    gc->constants.maxTextureSize = __GL_DEFAULT_MAX_MIPMAP_LEVEL;/*XXX*/
    gc->constants.maxMipMapLevel = __GL_DEFAULT_MAX_MIPMAP_LEVEL;
    gc->constants.maxListNesting = __GL_DEFAULT_MAX_LIST_NESTING;
    gc->constants.maxEvalOrder = __GL_DEFAULT_MAX_EVAL_ORDER;
    gc->constants.maxPixelMapTable = __GL_DEFAULT_MAX_PIXEL_MAP_TABLE;
    gc->constants.maxAttribStackDepth = __GL_DEFAULT_MAX_ATTRIB_STACK_DEPTH;
    gc->constants.maxClientAttribStackDepth =
	__GL_DEFAULT_MAX_CLIENT_ATTRIB_STACK_DEPTH;
    gc->constants.maxNameStackDepth = __GL_DEFAULT_MAX_NAME_STACK_DEPTH;
    gc->constants.maxModelViewStackDepth =
	__GL_DEFAULT_MAX_MODELVIEW_STACK_DEPTH;
    gc->constants.maxProjectionStackDepth =
	__GL_DEFAULT_MAX_PROJECTION_STACK_DEPTH;
    gc->constants.maxTextureStackDepth = __GL_DEFAULT_MAX_TEXTURE_STACK_DEPTH;

    gc->constants.pointSizeMinimum = __GL_DEFAULT_POINT_SIZE_MINIMUM;
    gc->constants.pointSizeMaximum = __GL_DEFAULT_POINT_SIZE_MAXIMUM;
    gc->constants.pointSizeGranularity = __GL_DEFAULT_POINT_SIZE_GRANULARITY;
    gc->constants.lineWidthMinimum = __GL_DEFAULT_LINE_WIDTH_MINIMUM;
    gc->constants.lineWidthMaximum = __GL_DEFAULT_LINE_WIDTH_MAXIMUM;
    gc->constants.lineWidthGranularity = __GL_DEFAULT_LINE_WIDTH_GRANULARITY;

    gc->dlist.optimizer = __glGenericDlistOptimizer;
    gc->dlist.compiler = __glGenericDlistCompiler;
    gc->dlist.listExec = __gl_GenericDlOps;
    gc->dlist.baseListExec = __glListExecTable;
    gc->dlist.machineListExec = NULL;
    gc->dlist.checkOp = (void (*)(__GLcontext *gc, __GLdlistOp *)) __glNop;
    gc->dlist.initState = (void (*)(__GLcontext *gc)) __glNop;

    gc->exports.loseCurrent = LoseCurrent;
    gc->exports.makeCurrent = MakeCurrent;
    gc->exports.shareContext = ShareContext;
    gc->exports.changeDrawableSize = ChangeDrawableSize;
    gc->exports.changeDrawableLocation = ChangeDrawableLocation;
#if defined(__GL_SUPPORT_MGL)
    gc->exports.changeBuffers = __glChangeBuffers;
#endif
    gc->exports.lockBuffers = LockBuffers;
    gc->exports.unlockBuffers = UnlockBuffers;

    __glEarlyInitContext(gc);

#if 0
    fprintf(stderr, "Size of core context = %d bytes\n", sizeof(*gc));
    fprintf(stderr, "Size of hw context = %d bytes\n", sizeof(*hwcx));
    fprintf(stderr, "Size of core attrib record = %d bytes\n", sizeof(gc->state));
#endif

    return gc;
}

void __glCoreNopDispatch(void)
{
    __gl_dispatch = __glNopDispatchState;
}

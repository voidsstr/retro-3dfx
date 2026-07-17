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
#include <ddraw.h>
#include "context.h"
#include "imports.h"
#include "machdep.h"
#include "global.h"
#include "g_imfncs.h"
#include "dlistopt.h"
#include "wgllib.h"

#include "sstcontext.h"

#define __DEBUG_PRINT

static void __fastcall WaitIdle(__GLcontext *gc)

{
    /* XXX: Fill in with your way or making the processor wait for graphics */
}

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
    __GLfloat height = gc->constants.height;
    GLboolean val = GL_FALSE;

    if (__glFindWindowSize(gc) == GL_TRUE) {
	__glUpdateViewport(gc);
	CheckViewport(gc);
	val = GL_TRUE;

    }

    if (gc->constants.yInverted && height != gc->constants.height) {
        gc->state.current.rasterPos.window.y += gc->constants.height - height;
    }

    return val;
}

static GLboolean ApplyScissor(__GLcontext *gc)
{
    __GLscissor *scissor = &gc->state.scissor;
    GLboolean val = GL_FALSE;

    if (__glFindWindowSize(gc) == GL_TRUE) {
	CheckViewport(gc);
	val = GL_TRUE;
    }
    if (gc->state.enables.general & __GL_SCISSOR_TEST_ENABLE) {
        if (gc->constants.yInverted) {
            int height = gc->constants.height;
            grClipWindow(scissor->scissorX,
                         height - (scissor->scissorY + scissor->scissorHeight),
                         scissor->scissorX + scissor->scissorWidth,
                         height - scissor->scissorY);
        } else {
            grClipWindow(scissor->scissorX,
                         scissor->scissorY,
                         scissor->scissorX + scissor->scissorWidth,
                         scissor->scissorY + scissor->scissorHeight);
        }
    } else {
        grClipWindow(0,0, gc->constants.maxViewportWidth, 
                          gc->constants.maxViewportHeight );
    }

    return val;
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

static GLboolean ChangeDrawableLocation(__GLcontext *gc, GLint x, GLint y, 
					GLint w, GLint h)
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
#if 0
    __GLSSTcontext *hwcx = (__GLSSTcontext *)gc;
	
    if (++hwcx->lockCnt != 1) return;

    /* Now we're locked, we can talk to the hardware */
    hwcx->hwLocked = GL_TRUE;
#endif
}



static void UnlockDevice(__GLcontext *gc)
{
#if 0
    __GLSSTcontext *hwcx = (__GLSSTcontext *)gc;

    if (--hwcx->lockCnt != 0) return;

    hwcx->hwLocked = GL_FALSE;
#endif
}

/************************************************************************/

static void Finish(__GLcontext *gc)
{
    __WGLcontext *glrc = (__WGLcontext *)gc->imports.other;
    __GL_API_FLUSH();

    if ( glrc->isInUse ) {
        grFinish();
    }
}

static void Flush(__GLcontext *gc)
{
    __WGLcontext *glrc = (__WGLcontext *)gc->imports.other;
    __GL_API_FLUSH();

    /* lightweight state change that will flush out the pipe */
    if ( glrc->isInUse ) {
        FxU32 dummy;
        grDepthBufferFunction(gc->state.depth.testFunc - GL_NEVER);
        grGet(GR_IS_BUSY, sizeof(dummy), &dummy);
    }
}

/************************************************************************/

/*
** initialize context pointers
*/
static void InitFnPtrs(__GLcontext *gc)
{
    gc->procs.ec1         = __glDoEvalCoord1;
    gc->procs.ec2         = __glDoEvalCoord2;
    gc->procs.bitmap      = __glDrawBitmap;
    gc->procs.rect        = __glRect;
    gc->procs.clipPolygon = __glClipPolygon;
    gc->procs.validate              = __glSSTValidate;
    gc->procs.convertPolygonStipple = __glConvertStipple;

    gc->procs.pushMatrix            = __glPushModelViewMatrix;
    gc->procs.popMatrix             = __glPopModelViewMatrix;
    gc->procs.loadIdentity          = __glLoadIdentityModelViewMatrix;

    gc->procs.matrix.copy            = __glCopyMatrix;
    gc->procs.matrix.invertTranspose = __glInvertTransposeMatrix;
    gc->procs.matrix.makeIdentity    = __glMakeIdentity;
    gc->procs.matrix.mult = __glMultMatrix;
    gc->procs.computeInverseTranspose = __glComputeInverseTranspose;
    gc->procs.normalize = __glNormalize;

    gc->procs.beginPrim[GL_LINE_LOOP]      = __glBeginLLoop;
    gc->procs.beginPrim[GL_LINE_STRIP]     = __glBeginLStrip;
    gc->procs.beginPrim[GL_LINES]          = __glBeginLines;
    gc->procs.beginPrim[GL_POINTS]         = __glBeginPoints;
    gc->procs.beginPrim[GL_POLYGON]        = __glBeginPolygon;
    gc->procs.beginPrim[GL_TRIANGLE_STRIP] = __glBeginTStrip;
    gc->procs.beginPrim[GL_TRIANGLE_FAN]   = __glBeginTFan;
    gc->procs.beginPrim[GL_TRIANGLES]      = __glBeginTriangles;
    gc->procs.beginPrim[GL_QUAD_STRIP]     = __glBeginQStrip;
    gc->procs.beginPrim[GL_QUADS]          = __glBeginQuads;
    gc->procs.endPrim                      = __glEndPrim;

    gc->procs.vertex = (void (*)(__GLcontext*, __GLvertex*)) __glNop;
    gc->procs.rasterPos2 = __glRasterPos2;
    gc->procs.rasterPos3 = __glRasterPos3;
    gc->procs.rasterPos4 = __glRasterPos4;

    /*
    ** Load in the device specific pick procs.  Do this before
    ** resetting the default state as that may need pick procs.
    */
    gc->procs.pickAllProcs           = __glGenericPickAllProcs;
    gc->procs.pickBlendProcs         = __glGenericPickBlendProcs;
    gc->procs.pickBufferProcs        = __glSSTPickBufferProcs;
    gc->procs.pickClipProcs          = __glGenericPickClipProcs;
    gc->procs.pickColorMaterialProcs = __glGenericPickColorMaterialProcs;
    gc->procs.pickFogProcs           = __glGenericPickFogProcs;
    gc->procs.pickTransformProcs     = __glGenericPickTransformProcs;
    gc->procs.pickCullVertexProcs    = __glGenericPickCullVertexProcs;
    gc->procs.pickLineProcs          = __glSSTPickLineProcs;
    gc->procs.pickMatrixProcs        = __glGenericPickMatrixProcs;
    gc->procs.pickInvTransposeProcs  = __glGenericPickInvTransposeProcs;
    gc->procs.pickMvpMatrixProcs     = __glGenericPickMvpMatrixProcs;
    gc->procs.pickParameterClipProcs = __glGenericPickParameterClipProcs;
    gc->procs.pickPixelProcs         = __glGenericPickPixelProcs;
    gc->procs.pickPointProcs         = __glSSTPickPointProcs;
    gc->procs.pickRenderBitmapProcs  = __glOptPickRenderBitmapProcs;

    gc->procs.pickSpanProcs          = __glGenericPickSpanProcs;
    gc->procs.pickStoreProcs         = __glGenericPickStoreProcs;
    gc->procs.pickTextureProcs       = __glGenericPickTextureProcs;
    gc->procs.pickCalcTextureProcs   = __glGenericPickCalcTextureProcs;
    gc->procs.pickTriangleProcs      = __glSSTPickTriangleProcs;
    gc->procs.pickVertexProcs        = __glGenericPickVertexProcs;
    gc->procs.pickVertexArrayProcs   = __glGenericPickVertexArrayProcs;
    gc->procs.pickDepthProcs         = __glGenericPickDepthProcs;

    gc->procs.copyImage = __glGenericPickCopyImage;
    gc->procs.fetchTex     = __glGenericPickGetTexture;
    gc->procs.storeTex     = __glGenericPickStoreTexture;
    gc->procs.readImage = __glGenericPickReadImage;

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

/* XXX  Here is how we over-ride the HW specific procs and return to SW */
    if (gc->drawablePrivate->modes->pixmapMode == GL_TRUE) {
	/* rendering to a DIB.  Fall back to sw */
	gc->procs.pickBufferProcs = __glGenericPickBufferProcs;
#if 0
	gc->procs.pickAllProcs	     = __glGenericPickAllProcs;
	gc->procs.pickPointProcs     = __glGenericPickPointProcs;
	gc->procs.pickTriangleProcs  = __glGenericPickTriangleProcs;
	gc->procs.waitIdle = (void (*))(__GLcontext *) __glNop;
	gc->buffers.lock.lockDevice = (void (*))(__GLcontext *) __glNop;
	gc->buffers.lock.unlockDevice = (void (*))(__GLcontext *) __glNop;
#endif
    }
}

/*
** initialize context's side buffers
*/
static void InitBuffers(__GLcontext *gc)
{
    __GLdrawablePrivate *dp = (*gc->imports.getDrawablePrivate)(gc);

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
	    __glInitCI(gc->front, gc);		/* sw */
	    __glInitCI(gc->back, gc);		/* sw */
	} else {
	    __glInitRGB(gc->front, gc);
	    __glInitRGB(gc->back, gc);
	}
    } else {
	if (gc->modes.colorIndexMode) {
	    __glInitCI(gc->front, gc);		/* sw */
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
		__glInitCI(&gc->auxBuffer[i], gc); /* sw */
	    } else {
		__glInitRGB(&gc->auxBuffer[i], gc); /* sw */
	    }
	}
    }
#endif

    /* Initialize any other ancillary buffers */
    if (gc->modes.haveAccumBuffer) {
	__glInitAccum64(&gc->accumBuffer, gc); /* sw */
    }
    if (gc->modes.haveDepthBuffer) {
	__glInitDepth(&gc->depthBuffer, gc);
	/* Scale for voodoo's 16 bit z buffer */
        gc->depthBuffer.scale = 0xffff;
        gc->depthBuffer.numFracBits = 0;
    } else {
	/*
	** Set the scale factor to allow window z values to be computed.
	** Set it not to use the high bit (to avoid floating point
	** exceptions) or low bits (to match floating point precision).
	*/
	gc->depthBuffer.scale = 0x7fffff80;
    }
    if (gc->modes.haveStencilBuffer) {
	__glInitStencil8(&gc->stencilBuffer, gc); /* sw */
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

static void SSTSwapBuffers(__GLcontext *gc)
{
    __WGLcontext *glrc = (__WGLcontext *)gc->imports.other;
    gc->procs.flush(gc);

    if (!gc->modes.doubleBufferMode) {
        return;
    }

    if ( glrc->isInUse ) {
        grBufferSwap(1);
    }
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

    /* Free Glide data structures */
    __glSSTGlideDestroyContext(gc);

    /* Free memory for the host cpu hw context */
    (*gc->imports.free)(gc, gc);

    return GL_TRUE;
}
static GLboolean LoseCurrent(__GLcontext *gc)
{
    __GLSSTcontext *hwcx = (__GLSSTcontext *)gc;
    /* 
    ** Illegal to makeCurrent when the current context is in selection or 
    ** feedback mode.
    */
    if (gc->renderMode != GL_RENDER || __gl_beginMode == __GL_IN_BEGIN)
	return GL_FALSE;

    grSstWinClose(hwcx->glide.state.context);

    __glLoseCurrentBuffers( gc, ((__GLSSTcontext *)gc)->displayBank );

    gc->beginMode = __gl_beginMode;
    __glSetTLSCXValue(NULL);

    return GL_TRUE;
}


static const int __sstResCapTable[SST_PLATFORMS][SST_SLICONFIGS][SST_MEMCONFIGS][SST_RESOLUTIONS] = {
    { /* voodoo */
        { /* No SLI */
            { /* 2M */
                { SST_DBZ | SST_TB }, /* 5x3 */
                { SST_DBZ | SST_TB }, /* 6x4 */
                { SST_DB }, /* 8x6 */
                { SST_NC }, /* 10x7 */
            }, 
            { /* 4M */
                { SST_DBZ | SST_TB }, /* 5x3 */
                { SST_DBZ | SST_TB }, /* 6x4 */
                { SST_DBZ | SST_TB }, /* 8x6 */
                { SST_NC }, /* 10x7 */
            }, 
        }, 
        { /* SLI */
            { /* 2M */
                { SST_DBZ | SST_TB }, /* 5x3 */
                { SST_DBZ | SST_TB }, /* 6x4 */
                { SST_DBZ | SST_TB }, /* 8x6 */
                { SST_NC }, /* 10x7 */
            }, 
            { /* 4M */
                { SST_DBZ | SST_TB }, /* 5x3 */
                { SST_DBZ | SST_TB }, /* 6x4 */
                { SST_DBZ | SST_TB }, /* 8x6 */
                { SST_NC }, /* 10x7 */
            }, 
        }  
    }, 
    { /* rush */
        { /* No SLI */
            { /* 2M */
                { SST_DBZ | SST_TBZ }, /* 5x3 */
                { SST_DBZ | SST_TB }, /* 6x4 */
                { SST_DB }, /* 8x6 */
                { SST_NC }, /* 10x7 */
            }, 
            { /* 4M */
                { SST_DBZ | SST_TBZ }, /* 5x3 */
                { SST_DBZ | SST_TBZ }, /* 6x4 */
                { SST_DBZ | SST_TBZ }, /* 8x6 */
                { SST_NC }, /* 10x7 */
            }, 
        }, 


        { /* SLI */
            { /* 2M */
                { SST_NC }, /* 5x3  */
                { SST_NC }, /* 6x4  */
                { SST_NC }, /* 8x6  */
                { SST_NC }, /* 10x7 */
            }, 
            { /* 4M */
                { SST_NC }, /* 5x3  */
                { SST_NC }, /* 6x4  */
                { SST_NC }, /* 8x6  */
                { SST_NC }, /* 10x7 */
            }, 
        }  
    }, 
    { /* voodoo ii */
        { /* No SLI */
            { /* 2M */
                { SST_DBZ | SST_TBZ }, /* 5x3 */
                { SST_DBZ | SST_TB }, /* 6x4 */
                { SST_DB }, /* 8x6 */
                { SST_NC }, /* 10x7 */
            }, 
            { /* 4M */
                { SST_DBZ | SST_TBZ }, /* 5x3 */
                { SST_DBZ | SST_TBZ }, /* 6x4 */
                { SST_DBZ | SST_TBZ }, /* 8x6 */
                { SST_DB }, /* 10x7 */
            }, 
        }, 
        { /* SLI */
            { /* 2M */
                { SST_DBZ | SST_TBZ }, /* 5x3 */
                { SST_DBZ | SST_TBZ }, /* 6x4 */
                { SST_DBZ | SST_TB }, /* 8x6 */
                { SST_DB }, /* 10x7 */
            }, 
            { /* 4M */
                { SST_DBZ | SST_TBZ }, /* 5x3 */
                { SST_DBZ | SST_TBZ }, /* 6x4 */
                { SST_DBZ | SST_TBZ }, /* 8x6 */
                { SST_DBZ | SST_TBZ }, /* 10x7 */
            }, 
        }  
    }  
};

static const int __sstResTable[SST_RESOLUTIONS][3] = {
    {  512,  384, GR_RESOLUTION_512x384  },
    {  640,  480, GR_RESOLUTION_640x480  },
    {  800,  600, GR_RESOLUTION_800x600  },
    { 1024,  768, GR_RESOLUTION_1024x768 }
};

/*
** Make this context the current context for this process.
*/
static GLboolean MakeCurrent(__GLcontext *gc)
{
    int width, height;
    int resolution, flags;
    int platform, res, sli, mem, step, windowable;
    __GLSSTcontext *hwcx = (__GLSSTcontext *)gc;
    __WGLcontext *glrc = (__WGLcontext *)gc->imports.other;
    long Style = 0;
    HWND hwnd;
    RECT rect;

    /* Load up global variables for the new context */
    __glSetTLSCXValue(gc);
    __gl_beginMode = gc->beginMode;

    /* Initialize dispatch tables */
    gc->dispatchState = &gc->currentDispatchState;
    gc->currentDispatchState = __glSSTImmedState;
    gc->listCompState = __glListCompState;
   
    /* 
    ** Dirty the depth mask, since we may need to update depth pointers.
    */
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);

    if ((gc->gcState & __GL_HW_MODE) == 0 ) {
        /* Entering HW mode rendering */

        hwnd = WindowFromDC( glrc->hDC );
        GetClientRect(hwnd, &rect);

/* XXX The stupid doc sez that .left and .top are always 0 so we don't really
	have to do the subtraction, but that seems pretty brain dead */

        width  = rect.right  - rect.left;
        height = rect.bottom - rect.top;

        platform = SST_VOODOO;
        sli      = SST_NOSLI;
        mem      = SST_2M;
        res      = SST_640x480;
        windowable = 0;

        if (!strcmp(grGetString(GR_HARDWARE), "Voodoo2")) {
            platform = SST_VOODOOII;
        } else if (!strcmp(grGetString(GR_HARDWARE), "Voodoo")) {
            platform = SST_VOODOO;
        } else if (!strcmp(grGetString(GR_HARDWARE), "VoodooRush")) {
            platform = SST_RUSH;
            if (getenv("OGL_ENABLE_RUSH_WINDOWING")) {
                windowable = 1;
            }
        }

        /* XXX Taco - Need a query mechanism for SLI from Glide 3 */
      
        switch(grGetInteger(GR_MEMORY_FB)) {
        case 2:
            mem = SST_2M;
            break;
        case 4:
            mem = SST_4M;
            break;
        }

        /* walk up the list until width/height match */
        /* walk down the list until supported */
        step = 1;
        for( res = 0; ( ( res < SST_RESOLUTIONS ) && ( res >= 0 ) ); res += step ) {
            flags = 0;
            if ( width <= __sstResTable[res][0] ) 
                flags++;
            if ( height <= __sstResTable[res][1] ) 
                flags++;
            if ( step == 1 ) {
                if ( flags == 2 ) { /* we have a match */
                    if ( ( __sstResCapTable[platform][sli][mem][res] & SST_DBZ ) == SST_DBZ ) {
                        break;
                    } else {
                        step = -1;
                    }
                } else if ( res == SST_1024x768 ) { /* out of resolutions */
                    if ( ( __sstResCapTable[platform][sli][mem][res] & SST_DBZ ) == SST_DBZ ) {
                        break;
                    } else {
                        step = -1;
                    }
                }
            } else { /* step == -1 */
                if ( ( __sstResCapTable[platform][sli][mem][res] & SST_DBZ ) == SST_DBZ ) {
                    break;
                }
            }
        }
        resolution = __sstResTable[res][2];

        gc->constants.maxViewportWidth  = __sstResTable[res][0];
        gc->constants.maxViewportHeight = __sstResTable[res][1];

        Style = GetWindowLong (hwnd, GWL_STYLE) & WS_POPUP;

        if ( windowable ) {
            if ( ! ( Style ) ) {
                resolution = GR_RESOLUTION_NONE;
            }
        }

        if ( (hwcx->glide.state.context = grSstWinOpen((FxU32) hwnd, 
						       resolution,
						       GR_REFRESH_60Hz,
						       GR_COLORFORMAT_ARGB,
						       GR_ORIGIN_UPPER_LEFT,
						       2,1 )) == 0) {
            return GL_FALSE;
        }

        hwcx->glide.config.platform = platform;
        hwcx->glide.config.mem = mem;
        hwcx->glide.config.sli = sli;
        hwcx->glide.config.resolution = resolution;
   
        if ( !gc->modes.doubleBufferMode || getenv( "SST_SINGLEBUFFER" ) ) {
            __GL_GLIDE_SET( renderBuffer, buf, GR_BUFFER_FRONTBUFFER );
        }
    }
    if ((gc->gcState & __GL_INIT_HW) == 0) {
#if 0
	if (__glSSTInitSLock(hwcx) == GL_FALSE) {
	    /* oops..  Could not get the system lock */
	    return GL_FALSE;
	}

        set up hardware context here, ie registers, video memory base,
	or call Glide equivalent.  maybe WinOpen above does this? 
	Additionally, we can set up any\all Glide\OpenGL context
	defaults here.  See sst_export.c

	get regbase 
#endif 
    }

    /*
    ** This initializes context's side buffer structures and various
    ** other pointers.
    */
    if( (gc->gcState & __GL_HW_MODE) == 0 ) {
	InitBuffers(gc);
/* XXX 
	this is not in create context because it needs to call glide.
	__glSSTInitTextureManager(gc);
*/
    }

    /*
    ** page in buffer info
    */
    __glMakeCurrentBuffers( gc, &hwcx->displayBank );

    /* init pointers only when entering hw mode */
    if ((gc->gcState & __GL_HW_MODE) == 0 ) {
	InitFnPtrs(gc);
	gc->exports.swapBuffers = SSTSwapBuffers;
	gc->exports.destroyContext = DestroyContext;
    }

    /* Reset the context if this is the first MakeCurrent */
    if ((gc->gcState & (__GL_HW_MODE | __GL_PIXMAP_MODE)) == 0) {
	GLint width, height;

	/* Now reset the context to its default state */
	__glSoftResetContext(gc);

	/* Override constant strings */
	gc->constants.vendor = "3Dfx Interactive Inc.";
	gc->constants.renderer = "3Dfx";
	gc->constants.version = "1.1.0 3Dfx";

        /*
        ** Put new extension names in alphabetical order; make sure to include
        ** a space after the name.
        */
	
	/* XXXX This is quite weird.  We can have different extension strings
         *	based on whether we have 2 TMUs or not.  We only export the
         *	multitexture extension on 2 TMU boards
	 * NOTE:
         *	You must add any extension to both these lists if you want 
         *	it exported on both configurations.
	 */
#if 1
    /* XXXwheeler: Don't expose any extensions for now */
    gc->constants.extensions = "";
#else
/* XXXX change the expression below when we add SST part of context */
	if ( hwconfig.SSTs[0].sstBoard.VoodooConfig.nTexelfx == 2 ) {
            gc->constants.extensions = 
#if 0       /* XXXwheeler don't enable until optimized */
            /* XXXtaco these extensions haven't been tested */
            /*         but they may work.... */
                "GL_EXT_point_parameters "
                "GL_EXT_bgra "
                "GL_EXT_abgr "
                "GL_EXT_vertex_array "
                "GL_SGI_compiled_vertex_array "
                "GL_SGI_cull_vertex "
#endif
                "GL_EXT_paletted_texture "
                "GL_EXT_shared_texture_palette "
		"GL_SGIS_multitexture "
                 ;
#if 0 /* XXXshui not supported by our driver yet */
                "GL_EXT_packed_pixels "
                "GL_SGI_index_array_formats "
                "GL_SGI_index_func "
                "GL_SGI_index_material "
                "GL_SGI_index_texture "
                "GL_WIN_swap_hint "
#endif
            ;
        } else {
            gc->constants.extensions = 
#if 0       /* XXXwheeler don't enable until optimized */
            /* XXXtaco these extensions haven't been tested */
            /*         but they may work.... */
                "GL_EXT_point_parameters "
                "GL_EXT_bgra "
                "GL_EXT_abgr "
                "GL_EXT_vertex_array "
                "GL_SGI_compiled_vertex_array "
                "GL_SGI_cull_vertex "
#endif
                "GL_EXT_paletted_texture "
                "GL_EXT_shared_texture_palette "
                 ;
#if 0 /* XXXshui not supported by our driver yet */
                "GL_EXT_packed_pixels "
                "GL_SGI_index_array_formats "
                "GL_SGI_index_func "
                "GL_SGI_index_material "
                "GL_SGI_index_texture "
                "GL_WIN_swap_hint "
            ;
#endif      
	}
#endif
	/*
	** The first time a context is made current the spec requires that
	** the viewport and scissor be initialized.
	*/
	(*gc->imports.getDrawableSize)(gc, &width, &height);
#ifdef __GL_NO_ICD_LICENSE
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height);
#else
	(*gc->currentDispatchState.dispatch.Viewport)(0, 0, width, height);
	(*gc->currentDispatchState.dispatch.Scissor)(0, 0, width, height);
#endif
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
    if (gc->dlist.currentList) {
	gc->dispatchState = &gc->savedDispatchState;
	__glCopyDispatch(&gc->listCompState, &__gl_dispatch);
    } else {
	gc->dispatchState = &__gl_dispatch;
    }

    gc->gcState |= __GL_INIT_HW | __GL_HW_MODE;
    gc->gcState &= ~__GL_PIXMAP_MODE;

    return GL_TRUE;
}

static GLboolean ShareContext(__GLcontext *gc, __GLcontext *gcShare)
{
    /*
    ** Set up the sharable state: currently display lists and texture objects.
    */
    __glShareDlist(gc, gcShare);
    __glShareTextureObjects(gc, gcShare);

    return GL_TRUE;
}

/*
** Create a new context.  The new context is not made current.
** This can fail, but the caller is responsible for dealing with it.
*/
__GLcontext *__glSSTCreateContext(__GLimports *imports, __GLcontextModes *modes)
{
    __GLSSTcontext *hwcx;
    __GLcontext *gc;

    /* Allocate memory for host cpu hardware context */
    hwcx = (__GLSSTcontext *) (*imports->calloc)(0, 1, sizeof(__GLSSTcontext));
    if (!hwcx) {
	return 0;
    }

#if 0
    /* Set up device-specific fields */
    hwcx->hwLocked = GL_FALSE;
    hwcx->regBase = 0UL;
#endif

    gc = &hwcx->gc;
    gc->imports = *imports;
    gc->modes = *modes;

    /*
    ** Load some device specific constants into the context
    */
    gc->constants.maxViewportWidth = __GL_DEFAULT_MAX_WINDOW_WIDTH;
    gc->constants.maxViewportHeight = __GL_DEFAULT_MAX_WINDOW_HEIGHT;
    gc->constants.viewportXAdjust = 0;
    gc->constants.viewportYAdjust = 0;
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
#if defined(__GL_SUPPORT_MGL)
    gc->exports.changeBuffers = __glChangeBuffers;
#endif
    gc->exports.lockBuffers = LockBuffers;
    gc->exports.unlockBuffers = UnlockBuffers;

    __glEarlyInitContext(gc);

#if DEBUG
    fprintf(stderr, "Size of core context = %d bytes\n", sizeof(*gc));
    fprintf(stderr, "Size of hw context = %d bytes\n", sizeof(*hwcx));
    fprintf(stderr, "Size of core attrib record = %d bytes\n", sizeof(gc->state));
#endif

#if 0
    hwcx->rScale = 1024.0;
    hwcx->gScale = 1024.0;
    hwcx->bScale = 1024.0;
    hwcx->aScale = 1024.0;
    hwcx->zScale = 0.5;
    hwcx->xScale = (0x1FFFF);
#endif

    hwcx->prevDestOffset = 0;
    hwcx->prevDestStride = 0;
    hwcx->prevTexOffset  = 0;
    hwcx->prevTexStride  = 0;
    hwcx->prevZOffset    = 0;
    hwcx->prevZStride    = 0;

    if (!__glSSTGlideCreateContext(gc)) {
        return 0;
    }

    return gc;
}

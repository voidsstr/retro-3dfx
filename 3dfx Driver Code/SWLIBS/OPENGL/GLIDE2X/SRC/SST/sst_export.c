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

#include <glide.h>
#include "sst_context.h"
#include "sst_globals.h"
#include "sst_imfncs.h"

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

static void ApplyViewport(__GLcontext *gc)
{
    __GLfloat height = gc->constants.height;

    __glFindWindowSize(gc);
    __glUpdateViewport(gc);
    CheckViewport(gc);

    if (gc->constants.yInverted && height != gc->constants.height) {
        gc->state.current.rasterPos.window.y += gc->constants.height - height;
    }
}

static void ApplyScissor(__GLcontext *gc)
{
    __GLscissor *scissor = &gc->state.scissor;
    __glFindWindowSize(gc);
    CheckViewport(gc);

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
        grClipWindow(0,0, gc->constants.maxViewportWidth, gc->constants.maxViewportHeight );
    }
}

static void ChangeDrawableSize(__GLcontext *gc, GLint w, GLint h)
{
    __glChangeWindowSize(gc, w, h);
    __glUpdateViewport(gc);
    CheckViewport(gc);
}

static void LockBuffers(__GLcontext *gc)
{
    __GL_LOCK_BUFFERS(gc);
}

static void UnlockBuffers(__GLcontext *gc)
{
    __GL_UNLOCK_BUFFERS(gc);
}

/************************************************************************/

static void Finish(__GLcontext *gc)
{
    extern unsigned long tacoHackGlideInit;
    __GL_API_FLUSH();

    if ( tacoHackGlideInit ) {
        grSstIdle();
    }
}

static void Flush(__GLcontext *gc)
{
    extern unsigned long tacoHackGlideInit;
    __GL_API_FLUSH();

    /* lightweight state change that will flush out the pipe */
    if ( tacoHackGlideInit ) {
        grDepthBufferFunction(gc->state.depth.testFunc - GL_NEVER);
        grSstStatus();
    }
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
    gc->procs.validate = __glSSTValidate;
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
    gc->procs.pickAllProcs = __glSSTPickAllProcs;
    gc->procs.pickBlendProcs = __glGenericPickBlendProcs;
    gc->procs.pickBufferProcs = __glGenericPickBufferProcs;
    gc->procs.pickClipProcs = __glGenericPickClipProcs;
    gc->procs.pickColorMaterialProcs = __glGenericPickColorMaterialProcs;
    gc->procs.pickFogProcs = __glGenericPickFogProcs;
    gc->procs.pickTransformProcs = __glGenericPickTransformProcs;
    gc->procs.pickCullVertexProcs = __glGenericPickCullVertexProcs;
    gc->procs.pickLineProcs = __glSSTPickLineProcs;
    gc->procs.pickMatrixProcs = __glGenericPickMatrixProcs;
    gc->procs.pickInvTransposeProcs = __glGenericPickInvTransposeProcs;
    gc->procs.pickMvpMatrixProcs = __glGenericPickMvpMatrixProcs;
    gc->procs.pickParameterClipProcs = __glGenericPickParameterClipProcs;
    gc->procs.pickPixelProcs = __glSSTPickPixelProcs;
    gc->procs.pickPointProcs = __glSSTPickPointProcs;
    gc->procs.pickRenderBitmapProcs = __glGenericPickRenderBitmapProcs;

    gc->procs.pickSpanProcs = __glGenericPickSpanProcs;
    gc->procs.pickStoreProcs = __glGenericPickStoreProcs;
    gc->procs.pickTextureProcs = __glSSTPickTextureProcs;
    gc->procs.pickCalcTextureProcs = __glGenericPickCalcTextureProcs;
    gc->procs.pickTriangleProcs = __glSSTPickTriangleProcs;
    gc->procs.pickVertexProcs = __glGenericPickVertexProcs;
    gc->procs.pickVertexArrayProcs = __glGenericPickVertexArrayProcs;
    gc->procs.pickDepthProcs = __glGenericPickDepthProcs;

    gc->procs.copyImage = __glGenericPickCopyImage;
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

    gc->procs.varray_funcs = __gl_varray_funcs;

    gc->procs.colortable = __glSSTColorTableEXT;
    gc->procs.colorsubtable = __glColorSubTableEXT;

    gc->procs.matValidate = (void (*)(__GLcontext *gc)) __glNop;

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
            __glSSTInitRGB(gc->front, gc);
            __glSSTInitRGB(gc->back, gc);
        }
    } else {
        if (gc->modes.colorIndexMode) {
            __glInitCI(gc->front, gc);
        } else {
            __glSSTInitRGB(gc->front, gc);
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
                __glSSTInitRGB(&gc->auxBuffer[i], gc);
            }
        }
    }
#endif

    /* Initialize any other ancillary buffers */
    if (gc->modes.haveAccumBuffer) {
        __glInitAccum64(&gc->accumBuffer, gc);
    }
    if (gc->modes.haveDepthBuffer) {
        __glSSTInitDepth(&gc->depthBuffer, gc);
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
}

/************************************************************************/

static void SwapBuffers(__GLcontext *gc)
{
    extern unsigned long tacoHackGlideInit;
    gc->procs.flush(gc);

    if (!gc->modes.doubleBufferMode) {
        return;
    }
#ifdef __GL_BUILD_TRACE
    if (__gl_debug_trace) {
        __gldb_CollectFrameStats();
    }
#endif

    if ( tacoHackGlideInit ) {
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
    __glSSTDestroyContext(gc);

    /* Free memory for the host cpu hw context */
    (*gc->imports.free)(gc, gc);

    return GL_TRUE;
}

static GLboolean LoseCurrent(__GLcontext *gc)
{
    extern unsigned long tacoHackGlideInit;
    /* 
    ** Illegal to makeCurrent when the current context is in selection or 
    ** feedback mode.
    */
    if (gc->renderMode != GL_RENDER || __gl_beginMode == __GL_IN_BEGIN)
        return GL_FALSE;

    grSstWinClose();
    tacoHackGlideInit = 0;

    __glLoseCurrentBuffers( gc, ((__GLDDcontext *)gc)->displayBank );

    gc->beginMode = __gl_beginMode;
    __gl_context = NULL;

    return GL_TRUE;
}


/* ******************************************************
   Table of Hardware Resolutions

   ****************************************************** */
typedef enum SST_PLATFORMS {
    SST_VOODOO,
    SST_RUSH,
    SST_VOODOOII,
    SST_PLATFORMS
};

typedef enum SST_MEMCONFIG {
    SST_2M,
    SST_4M,
    SST_MEMCONFIGS
};

typedef enum SST_SLICONFIG {
    SST_NOSLI,
    SST_DOESSLI,
    SST_SLICONFIGS
};

typedef enum SST_RESOLUTION {
    SST_512x324,
    SST_640x480,
    SST_800x600,
    SST_1024x768,
    SST_RESOLUTIONS
};

typedef enum SST_PLATFORM_CAPS {
    SST_NC  = 0x00000000, /* not capabale */
    SST_DB  = 0x00000001,
    SST_DBZ = 0x00000011,
    SST_TB  = 0x00000100,
    SST_TBZ = 0x00001100
};

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
typedef struct  tagRECT
    {
    long left;
    long top;
    long right;
    long bottom;
    }   RECT;

static GLboolean MakeCurrent(__GLcontext *gc)
{
    static GrHwConfiguration hwconfig;
    int width, height;
    int resolution, flags;
    int platform, res, sli, mem, step, windowable;
    __GLDDcontext *hwcx = (__GLDDcontext *)gc;

    extern unsigned long tacoHackGlideInit;
    extern unsigned long tacoHackHWND;
    extern RECT tacoHackRect;

    /* Load up global variables for the new context */
    __gl_context = gc;
    __gl_beginMode = gc->beginMode;

    /* Initialize dispatch tables */
    gc->dispatchState = &gc->currentDispatchState;
    gc->currentDispatchState = __glSSTImmedState;
    gc->listCompState = __glSSTListCompState;

    grSstQueryHardware( &hwconfig );
    gc->grNTexelFx = hwconfig.SSTs[0].sstBoard.VoodooConfig.nTexelfx;

#ifdef __GL_BUILD_TRACE
    if (getenv("SST_TRACEGL")) {
      __gl_debug_trace = GL_TRUE;
      __gl_debug_log = fopen("gllog", "w");
      __gl_real_immed = &__glSSTImmedState;
      gc->currentDispatchState = __glDebugState;
    }
#endif
    /*
    ** Dirty the depth mask, since we may need to update depth pointers.
    */
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_DEPTH);

    /* Entering HW mode rendering */

    /*
    global.gc->hwnd = WindowFromDC( hdc );
    GetClientRect(global.gc->hwnd, &rect);

     */
    width  = tacoHackRect.right  - tacoHackRect.left;
    height = tacoHackRect.bottom - tacoHackRect.top;

    platform = SST_VOODOO;
    sli      = SST_NOSLI;
    mem      = SST_2M;
    res      = SST_640x480;
    windowable = 0;
    switch( hwconfig.SSTs[0].type ) {
    case GR_SSTTYPE_VOODOO:
        if ( hwconfig.SSTs[0].sstBoard.VoodooConfig.fbiRev >= 0x100 ) {
            platform = SST_VOODOOII;
        } else {
            platform = SST_VOODOO;
        }
        if ( hwconfig.SSTs[0].sstBoard.VoodooConfig.sliDetect ) {
            sli = SST_DOESSLI;
        } else {
            sli = SST_NOSLI;
        }
        switch( hwconfig.SSTs[0].sstBoard.VoodooConfig.fbRam ) {
        case 2:
            mem = SST_2M;
            break;
        case 4:
            mem = SST_4M;
            break;
        }
        break;
    case GR_SSTTYPE_SST96:
        platform = SST_RUSH;
        sli = SST_NOSLI;
        switch( hwconfig.SSTs[0].sstBoard.SST96Config.fbRam ) {
        case 2:
            mem = SST_2M;
            break;
        case 4:
            mem = SST_4M;
            break;
        }
        if ( getenv( "OGL_ENABLE_RUSH_WINDOWING" ) ) {
            windowable = 1;
        }
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

    if ( windowable ) {
        extern long tacoHackStyle;
        if ( ! ( tacoHackStyle ) ) {
            resolution = GR_RESOLUTION_NONE;
        }
    }

    if ( !grSstWinOpen( tacoHackHWND, 
                        resolution,
                        GR_REFRESH_60Hz,
                        GR_COLORFORMAT_ARGB,
                        GR_ORIGIN_UPPER_LEFT,
                        2,1 ) ) {
        return GL_FALSE;
    }
    tacoHackGlideInit = 1;

    if ( !gc->modes.doubleBufferMode || getenv( "SST_SINGLEBUFFER" ) ) {
        grRenderBuffer( GR_BUFFER_FRONTBUFFER );
    }

    /* XXXshui glide initialization; may need to be moved to the place */
    /* where we create and init a glide context */
    grColorMask(FXTRUE,FXFALSE); /* XXX */
    grDepthBufferFunction(GR_CMP_LESS);
    grDepthBufferMode(GR_DEPTHBUFFER_DISABLE);
    grDepthMask(FXFALSE);
    gc->grBlendSrc = GR_BLEND_ONE;
    gc->grBlendDst = GR_BLEND_ZERO;
    grAlphaBlendFunction(GR_BLEND_ONE, GR_BLEND_ZERO,
                         GR_BLEND_ONE, GR_BLEND_ZERO);
    grAlphaTestFunction(GR_CMP_ALWAYS);
    grAlphaTestReferenceValue(0x00);
    grConstantColorValue( 0x00000000 );

    /* these are the settings for no texturing, the opengl default */
    grAlphaCombine(GR_COMBINE_FUNCTION_LOCAL, 
               GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_LOCAL_ITERATED, 
               GR_COMBINE_OTHER_NONE, FXFALSE);
    grColorCombine(GR_COMBINE_FUNCTION_LOCAL, 
               GR_COMBINE_FACTOR_NONE,
               GR_COMBINE_LOCAL_ITERATED, 
               GR_COMBINE_OTHER_NONE, FXFALSE);
    grTexCombine(GR_TMU0,
                 GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 FXFALSE, FXFALSE);

    gc->texture.hwMinMag[0] = ~0;
    gc->texture.hwMinMag[1] = ~0;
    grTexFilterMode(GR_TMU0, 
                    GR_TEXTUREFILTER_POINT_SAMPLED,
                    GR_TEXTUREFILTER_BILINEAR);
    gc->texture.hwMMMode[0] = ~0;
    gc->texture.hwMMMode[1] = ~0;
    grTexMipMapMode(GR_TMU0, GR_MIPMAP_NEAREST, FXFALSE);
    gc->texture.hwSTWrap[0] = ~0;
    gc->texture.hwSTWrap[1] = ~0;
    grTexClampMode(GR_TMU0, GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP);

    /*
    ** This initializes context's side buffer structures and various
    ** other pointers.
    */
    if( (gc->gcState & __GL_HW_MODE) == 0 ) {
        InitBuffers(gc);

        /* this is not in create context because it needs to call glide. */
        __glSSTInitTextureManager(gc);
    }

    /*
    ** page in buffer info
    */
    __glMakeCurrentBuffers( gc, &hwcx->displayBank );

    gc->drawablePrivate->yInverted = 1;

    if ((gc->gcState & __GL_HW_MODE) == 0 ||
        (gc->drawablePrivate->yInverted != gc->constants.yInverted))
    {
        if (gc->drawablePrivate->yInverted) {
            gc->constants.yInverted = GL_TRUE;
            gc->constants.ySign = -1;
        } else {
            gc->constants.yInverted = GL_FALSE;
            gc->constants.ySign = 1;
        }

        __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_ALL);
    }

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


        /* XXXTaco This init is a hack */
        if ( gc->grNTexelFx == 2 ) {
            static char mtexString[] = "GL_SGIS_multitexture ";
            if ( !strstr( gc->constants.extensions, mtexString ) ) {
                char *extString;
                extString = (void*)(gc->imports.calloc)( 0, 1, strlen( gc->constants.extensions ) + strlen( mtexString ) + 1 );
                strcpy( extString, gc->constants.extensions );
                strcat( extString, mtexString );
                gc->constants.extensions = extString;
            }
        }
        if ( gc->grNTexelFx == 1 ) {
            gc->texture.sst.texUnits[0] = GR_TMU0;
            gc->texture.sst.texUnits[1] = GR_TMU0;
        } else {
            gc->texture.sst.texUnits[0] = GR_TMU1;
            gc->texture.sst.texUnits[1] = GR_TMU0;
        }
        
        gc->texture.sst.currentTMU = gc->texture.sst.texUnits[gc->texture.currentTexUnit];

        /*
        ** The first time a context is made current the spec requires that
        ** the viewport and scissor be initialized.
        */
        (*gc->imports.getDrawableSize)(gc, &width, &height);
        (*gc->currentDispatchState.dispatch.Viewport)(0, 0, width, height);
        (*gc->currentDispatchState.dispatch.Scissor)(0, 0, width, height);
    } else {
        /*
        ** If we have never bound to this window before, find out what
        ** its size is.
        */
        __glFindWindowSize(gc);
        __glUpdateViewport(gc);
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
    gc->constants.maxViewportWidth = __GL_SST_MAX_WINDOW_WIDTH;
    gc->constants.maxViewportHeight = __GL_SST_MAX_WINDOW_HEIGHT;
    gc->constants.viewportXAdjust = __GL_SST_SNAP_BIAS;
    gc->constants.viewportYAdjust = __GL_SST_SNAP_BIAS;
    gc->constants.subpixelBits = __GL_DEFAULT_COORD_SUBPIXEL_BITS;

    gc->constants.numberOfLights = __GL_DEFAULT_NUMBER_OF_LIGHTS;
    gc->constants.numberOfClipPlanes = __GL_DEFAULT_NUMBER_OF_CLIP_PLANES;
    gc->constants.numberOfTextures = __GL_DEFAULT_NUMBER_OF_TEXTURES;
    gc->constants.numberOfTextureEnvs = __GL_DEFAULT_NUMBER_OF_TEXTURE_ENVS;
    /* XXXshui is really inited to 1 << maxMipMapLevel later */
    gc->constants.maxTextureSize = 1 << __GL_SST_MAX_LOD;
    gc->constants.maxMipMapLevel = __GL_SST_MAX_LOD;
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

    gc->constants.pointSizeMinimum = __GL_SST_POINT_SIZE_MINIMUM;
    gc->constants.pointSizeMaximum = __GL_SST_POINT_SIZE_MAXIMUM;
    gc->constants.pointSizeGranularity = __GL_SST_POINT_SIZE_GRANULARITY;
    gc->constants.lineWidthMinimum = __GL_SST_LINE_WIDTH_MINIMUM;
    gc->constants.lineWidthMaximum = __GL_SST_LINE_WIDTH_MAXIMUM;
    gc->constants.lineWidthGranularity = __GL_SST_LINE_WIDTH_GRANULARITY;

    gc->constants.yInverted = GL_TRUE;
    gc->constants.ySign = 1;

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

    __glSSTEarlyInitContext(gc);

#if 0
    fprintf(stderr, "Size of core context = %d bytes\n", sizeof(*gc));
    fprintf(stderr, "Size of hw context = %d bytes\n", sizeof(*hwcx));
    fprintf(stderr, "Size of core attrib record = %d bytes\n", sizeof(gc->state));
#endif

    return gc;
}


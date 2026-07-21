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
#include <stddef.h>
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

/* grSstWinOpenExt is a glide3x EXTENSION entry (not a static export) -- like
 * grColorCombineExt it must be fetched via grGetProcAddress (see SST_TEX.C).
 * (Do NOT #include <g3ext.h> -- it forces a link-time ref to the symbol.) */
#ifndef GR_PIXFMT_ARGB_8888
#define GR_PIXFMT_ARGB_8888 0x0005          /* from G3EXT.H */
#endif
typedef GrContext_t (FX_CALL *__pfnWinOpenExt)(FxU32, GrScreenResolution_t,
        GrScreenRefresh_t, GrColorFormat_t, GrOriginLocation_t, int /*GrPixelFormat_t*/, int, int);

/* crash-robust debug logging to C:\3dfxogl.log (defined in wgl/wglcmds.c) */
extern void OGLLOG( const char *fmt, ... );

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
        grFinish();
    }
}

static void Flush(__GLcontext *gc)
{
    extern unsigned long tacoHackGlideInit;
    __GL_API_FLUSH();

    /* lightweight state change that will flush out the pipe */
    if ( tacoHackGlideInit ) {
        long blah;
        grDepthBufferFunction(gc->state.depth.testFunc - GL_NEVER);
        grGet( GR_IS_BUSY, sizeof( blah ), &blah );
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

    /* XXXwheeler: this keeps some programs from crashing, but probably
       doesn't do the right thing still. */
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

/* --- RDTSC frame profiling (logging only -> C:\3dfxprof.log). Answers the
 * core question the resolution sweep can't: of each frame's wall-clock, how much
 * is the deferred Glide FIFO flush (procs.flush = T&L submission) vs grBufferSwap
 * (present/vsync wait) vs "other" (engine + inline GL). rdtsc via _emit for
 * MSVC6-safety. No render change. Enabled at runtime by env RETRO3DFX_PROF=1. */
static unsigned __int64 __prof_rdtsc(void)
{
    unsigned int __lo, __hi;
    __asm { _emit 0x0F __asm _emit 0x31 __asm mov __lo, eax __asm mov __hi, edx }
    return ((unsigned __int64)__hi << 32) | __lo;
}
static unsigned __int64 __prof_lastSwap = 0, __prof_frameAcc = 0,
                        __prof_flushAcc = 0, __prof_swapAcc = 0;
static int             __prof_frames = 0;

/* RETRO3DFX_PERFLOG: per-100-frame counter dump -> C:\icd_perf.log (raw
** Win32 I/O; GoldSrc low-fps hunt).  Counters incremented at the texture
** download / palette / allocator hot spots.  windows.h cannot be included
** here (its SwapBuffers(HDC) clashes with the local static SwapBuffers),
** so the few Win32 imports are declared by hand. */
void * __stdcall CreateFileA(const char*, unsigned long, unsigned long,
                             void*, unsigned long, unsigned long, void*);
int    __stdcall WriteFile(void*, const void*, unsigned long, unsigned long*, void*);
int    __stdcall FlushFileBuffers(void*);
unsigned long __stdcall GetTickCount(void);
int    __cdecl   wsprintfA(char*, const char*, ...);
#define __R3D_INVALID_HANDLE ((void*)(long)-1)

long __r3d_cTexDl = 0, __r3d_cTexDlPart = 0, __r3d_cTableDl = 0, __r3d_cTexAlloc = 0;
static int __r3d_dbMode = -1;   /* doubleBufferMode as seen at swap time */

/* RETRO3DFX FBDUMP: self-service "what is actually on screen" capture.
** Gate: file marker C:\icd_fbdump.on.  Every 100th swap (up to 10 dumps)
** the REAL hardware front buffer is read back via grLfbReadRegion in
** 16-line strips and written raw-565 to C:\fbdump_NN.raw (WxH recorded in
** 3dfxogl.log).  This is the exact scanout content - no GDI capture, no
** game cooperation, no human eyeballing a monitor needed. */
static void __r3dFbDump(__GLcontext *gc)
{
    static int en = -1, swaps = 0, ndump = 0;
    static unsigned short strip[640 * 16];
    char nm[64], msg[96];
    void *h; unsigned long wr;
    int w, hgt, y, lines, n;
    extern void OGLLOG(const char*, ...);

    if (en < 0) {
        void *g = CreateFileA("C:\\icd_fbdump.on", 0x80000000L, 3, 0, 3, 0, 0);
        en = (g != __R3D_INVALID_HANDLE) ? 1 : 0;
        if (en) { extern int __stdcall CloseHandle(void*); CloseHandle(g); }
    }
    if (!en) return;
    swaps++;
    if (swaps != 5 && (swaps % 100) != 0) return;   /* frame 5, 100, 200, ... */
    ndump = ndump % 10;   /* ROLLING: slots 0-9 overwrite, last 10 dumps kept */

    w = gc->constants.maxViewportWidth;
    hgt = gc->constants.maxViewportHeight;
    if (w > 640) w = 640;
    wsprintfA(nm, "C:\\fbdump_%02d.raw", ndump);
    h = CreateFileA(nm, 0x40000000L, 1, 0, 2 /*CREATE_ALWAYS*/, 0, 0);
    if (h == __R3D_INVALID_HANDLE) return;
    for (y = 0; y < hgt; y += 16) {
        lines = (hgt - y < 16) ? (hgt - y) : 16;
        if (!grLfbReadRegion(GR_BUFFER_FRONTBUFFER, 0, y, w, lines,
                             w * 2, strip))
            break;
        WriteFile(h, strip, (unsigned long)(w * 2 * lines), &wr, 0);
    }
    { extern int __stdcall CloseHandle(void*); CloseHandle(h); }
    n = wsprintfA(msg, "FBDUMP %d -> %dx%d 565 (swap %d)", ndump, w, hgt, swaps);
    (void)n;
    OGLLOG("%s", msg);
    ndump++;
}
static void __r3dPerfDump(void)
{
    static int en = -1;
    static void *h = 0;
    static unsigned long lastTick = 0;
    static int frames = 0;
    unsigned long now, wr, dt;
    char buf[200]; int n, fps10;
    if (en < 0) {
        /* gate on C:\icd_perf.on (file marker, not env: getenv proved
        ** unreliable under GoldSrc's process even though it works under
        ** gfix.exe) */
        void *g = CreateFileA("C:\\icd_perf.on", 0x80000000L /*GENERIC_READ*/,
                              3 /*share rw*/, 0, 3 /*OPEN_EXISTING*/, 0, 0);
        en = (g != __R3D_INVALID_HANDLE) ? 1 : 0;
        if (en) { extern int __stdcall CloseHandle(void*); CloseHandle(g); }
        if (!en && getenv("RETRO3DFX_PERFLOG")) en = 1;
    }
    if (!en) return;
    frames++;
    if (frames < 100) return;
    now = GetTickCount();
    if (!h) {
        h = CreateFileA("C:\\icd_perf.log", 0x40000000L /*GENERIC_WRITE*/,
                        1 /*FILE_SHARE_READ*/, 0, 2 /*CREATE_ALWAYS*/, 0, 0);
    } else if (h != __R3D_INVALID_HANDLE && lastTick) {
        dt = now - lastTick;
        fps10 = dt ? (int)(1000L * frames * 10 / dt) : 0;
        n = wsprintfA(buf, "f=%d dt=%lums fps10=%d db=%d texDl=%ld texDlPart=%ld tableDl=%ld alloc=%ld\r\n",
                      frames, dt, fps10, __r3d_dbMode,
                      __r3d_cTexDl, __r3d_cTexDlPart, __r3d_cTableDl, __r3d_cTexAlloc);
        WriteFile(h, buf, (unsigned long)n, &wr, 0); FlushFileBuffers(h);
    }
    lastTick = now; frames = 0;
    __r3d_cTexDl = __r3d_cTexDlPart = __r3d_cTableDl = __r3d_cTexAlloc = 0;
}

static void SwapBuffers(__GLcontext *gc)
{
    extern unsigned long tacoHackGlideInit;
    unsigned __int64 t0, t1, ts0, ts1;

    /* RETRO3DFX 2PPC-STALE FIX (CS green world, ICD 0.3.4).  2PPC ("2 pixels
    ** per clock") is Glide's SINGLE-texture optimization; it must be OFF for a
    ** genuine dual-texture (world+lightmap) draw.  GoldSrc alternates single
    ** (HUD/sprites, 2PPC ON) and dual (world, 2PPC OFF) every frame, but our
    ** combine-word cache skips the Glide re-issue when words are unchanged, so
    ** stale 2PPC bit-29 leaks into the world draw -> (0,G,0) green world.
    ** 0.3.5: fire ONLY on frames that really used dual-texture (__r3d_sawTMU0
    ** = the lightmap unit GR_TMU0 was sourced this frame).  Q3/idTech is
    ** single-texture (never sources TMU0) -> hook inert there.  0.3.4d fired
    ** in Q3 too (Q3's one unit maps to GR_TMU1, so __r3d_blitValid latched)
    ** and the per-frame combine override killed vertex-color modulate (white
    ** menu text) and went stale across mode changes (black 1024 world).
    ** Re-issue the FULL TMU1 state: grTexSource (dirties textureMode/
    ** texBaseAddr/tLOD) + grTexCombine + grColorCombine (dirty combineMode +
    ** tmuConfig) + grAlphaBlendFunction.  Together they force a full TMU
    ** re-validation on the next world draw, so Glide re-runs _grTex2ppc and
    ** clears the stale bit-29.  Bisect-proven set (0/4 green): grTexSource
    ** ALONE does NOT fix, combines ALONE do NOT fix (4/6), the four together
    ** DO.  Then invalidate ONLY the combine-word dedup cache so the app's
    ** next combine call genuinely re-issues (restores its own state) -- NOT
    ** __glSSTResetCombineCache(), which wipes ext/overbright and broke
    ** attempts 2/3. */
    {
        extern unsigned long __r3d_blitAddr; extern long __r3d_blitInfo[5];
        extern int __r3d_blitValid;
        extern int __r3d_sawTMU0;
        extern void __glSSTInvalidateCombineWords(void);
        if ( tacoHackGlideInit && __r3d_blitValid && __r3d_sawTMU0 ) {
            GrTexInfo ti;
            ti.smallLodLog2     = (GrLOD_t)__r3d_blitInfo[0];
            ti.largeLodLog2     = (GrLOD_t)__r3d_blitInfo[1];
            ti.aspectRatioLog2  = (GrAspectRatio_t)__r3d_blitInfo[2];
            ti.format           = (GrTextureFormat_t)__r3d_blitInfo[3];
            ti.data             = (void *)__r3d_blitInfo[4];
            grTexSource( GR_TMU1, __r3d_blitAddr, GR_MIPMAPLEVELMASK_BOTH, &ti );
            grTexCombine( GR_TMU1, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                          GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE );
            grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
                            GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
            grAlphaBlendFunction( GR_BLEND_ONE, GR_BLEND_ZERO,
                                  GR_BLEND_ONE, GR_BLEND_ZERO );
            /* 0.3.7: do NOT invalidate the combine-word cache here.  Doing so
            ** (0.3.5/0.3.6) made the game's combine re-issue every frame and
            ** the green/rainbow chaos returned (27/49) -- same failure class
            ** as ResetCombineCache in attempts 2/3.  The PROVEN-good state is
            ** exactly this override with the cache left alone (0.3.4d: 0/4
            ** green, correct-looking de_dust).  The dedup cache then skips
            ** the game's identical combine words and the world draws through
            ** the freshly-validated TMU state. */
        }
        __r3d_sawTMU0 = 0;   /* per-frame marker */
    }

    t0 = __prof_rdtsc();
    gc->procs.flush(gc);
    t1 = __prof_rdtsc();

    /* perf dump BEFORE the single-buffer early-return: a single-buffered
    ** context (front-buffer rendering) still counts as a "frame" for the
    ** GoldSrc fps hunt, and db= in the log tells us which mode we're in. */
    __r3d_dbMode = gc->modes.doubleBufferMode ? 1 : 0;
    __r3dPerfDump();

    if (!gc->modes.doubleBufferMode) {
        return;
    }
#ifdef __GL_BUILD_TRACE
    if (__gl_debug_trace) {
        __gldb_CollectFrameStats();
    }
#endif

    /* RETRO3DFX read-back blit (C:\icd_texblit.on): before the swap, draw the
    ** saved large-565 world texture to the top-left corner point-sampled, white
    ** modulate, so fbdump captures its ACTUAL sampled texels.  Green corner =
    ** TMU memory is corrupt; tan corner = memory is fine (bug is elsewhere). */
    { static int tb = -1;
      extern unsigned long __r3d_blitAddr; extern long __r3d_blitInfo[5];
      extern int __r3d_blitValid;
      if ( tb < 0 ) {
          void *g = CreateFileA("C:\\icd_texblit.on", 0x80000000L, 3, 0, 3, 0, 0);
          tb = (g != __R3D_INVALID_HANDLE) ? 1 : 0;
          if (tb) { extern int __stdcall CloseHandle(void*); CloseHandle(g); }
      }
      /* NOTE: this swap-time combine re-issue was the DIAGNOSTIC that proved
      ** the green is stale 2PPC combine state.  The PROPER fix now lives in
      ** __glSSTLoadCombineFunction (SST_TEX.C): invalidate the combine cache on
      ** an active-TMU topology change so the combine is re-issued and Glide
      ** re-evaluates 2PPC.  This blit is left inert (marker off by default);
      ** it stays only as the read-back tool (draw disabled). */
      if ( tb && tacoHackGlideInit && __r3d_blitValid ) {
          grTexCombine( GR_TMU1, GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE,
                        GR_COMBINE_FUNCTION_LOCAL, GR_COMBINE_FACTOR_NONE, FXFALSE, FXFALSE );
          grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER, GR_COMBINE_FACTOR_ONE,
                          GR_COMBINE_LOCAL_NONE, GR_COMBINE_OTHER_TEXTURE, FXFALSE );
      }
    }

    ts0 = __prof_rdtsc();
    if ( tacoHackGlideInit ) {
        grBufferSwap(1);   /* swapInterval 1 required — grBufferSwap(0) breaks
                              rendering on the Voodoo5 SLI (hard sync needed) */
        __r3dFbDump(gc);
    }
    ts1 = __prof_rdtsc();

    __prof_flushAcc += (t1 - t0);
    __prof_swapAcc  += (ts1 - ts0);
    if (__prof_lastSwap) { __prof_frameAcc += (ts1 - __prof_lastSwap); __prof_frames++; }
    __prof_lastSwap = ts1;
    if (__prof_frames >= 100) {
        /* RETRO3DFX: gated on C:\icd_prof.on (was unconditional despite the
        ** "env-gated" comment - every game appended 3dfxprof.log forever). */
        static int profEn = -1;
        FILE *f;
        if (profEn < 0) {
            void *g = CreateFileA("C:\\icd_prof.on", 0x80000000L, 3, 0, 3, 0, 0);
            profEn = (g != __R3D_INVALID_HANDLE) ? 1 : 0;
            if (profEn) { extern int __stdcall CloseHandle(void*); CloseHandle(g); }
        }
        if (!profEn) {
            __prof_frames = 0; __prof_frameAcc = 0; __prof_flushAcc = 0; __prof_swapAcc = 0;
            return;
        }
        f = fopen("C:\\3dfxprof.log", "a");
        if (f) {
            double fr = (double)__prof_frameAcc;
            fprintf(f, "frames=%d avgFrameKc=%u flushKc=%u swapKc=%u flush%%=%.1f swap%%=%.1f other%%=%.1f\n",
                __prof_frames,
                (unsigned)((__prof_frameAcc / __prof_frames) / 1000),
                (unsigned)((__prof_flushAcc / __prof_frames) / 1000),
                (unsigned)((__prof_swapAcc  / __prof_frames) / 1000),
                fr > 0 ? 100.0 * (double)__prof_flushAcc / fr : 0.0,
                fr > 0 ? 100.0 * (double)__prof_swapAcc  / fr : 0.0,
                fr > 0 ? 100.0 * (double)(__prof_frameAcc - __prof_flushAcc - __prof_swapAcc) / fr : 0.0);
            fclose(f);
        }
        __prof_frames = 0; __prof_frameAcc = 0; __prof_flushAcc = 0; __prof_swapAcc = 0;
    }
}

/************************************************************************/

static GrContext_t tacoHackContext;
/* RETRO3DFX Glide-context reuse bookkeeping (see MakeCurrent): the window and
** resolution the current Glide context was opened on. */
long __r3d_openHwnd = 0;
long __r3d_openRes  = -1;

static GLboolean DestroyContext(__GLcontext *gc)
{
    extern unsigned long tacoHackGlideInit;

    /* RETRO3DFX: the deferred Glide close (LoseCurrent no longer closes).
    ** If a multi-context app destroys a secondary context mid-run, the next
    ** MakeCurrent simply reopens -- one hiccup, still correct. */
    if (tacoHackGlideInit && tacoHackContext) {
        OGLLOG( "DestroyContext: grSstWinClose(ctx=0x%x)",
                (unsigned)tacoHackContext );
        grSstWinClose( tacoHackContext );
        tacoHackGlideInit = 0;
        __r3d_openHwnd = 0;
        __r3d_openRes = -1;
        /* 0.3.5: the latched world-texture address dies with the context */
        { extern int __r3d_blitValid; __r3d_blitValid = 0; }
    }

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

    /* RETRO3DFX: do NOT grSstWinClose here.  GoldSrc switches GL contexts on
    ** the same window constantly; closing on every LoseCurrent forced a full
    ** fullscreen re-open in the next MakeCurrent (70-600ms each = the CS
    ** "very low fps").  The Glide context now stays open; it is closed by
    ** MakeCurrent on a window/res change and by DestroyContext. */
    OGLLOG( "LoseCurrent: keeping Glide ctx=0x%x open (deferred close)",
            (unsigned)tacoHackContext );

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

static int grGetInteger( int pname ) {
    int value;
    grGet( pname, sizeof( value ), &value );
    return value;
}

static GLboolean MakeCurrent(__GLcontext *gc)
{
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

    OGLLOG( "MakeCurrent: enter gc=0x%x, calling grGet(GR_NUM_TMU)",
            (unsigned)gc );
    gc->grNTexelFx = grGetInteger( GR_NUM_TMU );
    OGLLOG( "MakeCurrent: GR_NUM_TMU=%d", gc->grNTexelFx );

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

    OGLLOG( "MakeCurrent: window %dx%d, hw='%s'", width, height,
            grGetString( GR_HARDWARE ) ? grGetString( GR_HARDWARE )
                                       : "(null)" );

    if ( !strcmp( grGetString( GR_HARDWARE ), "Voodoo2" ) ) {
        platform = SST_VOODOOII;
    } else if ( !strcmp( grGetString( GR_HARDWARE ), "Voodoo" ) ) {
        platform = SST_VOODOO;
    } else if ( !strcmp( grGetString( GR_HARDWARE ), "VoodooRush" ) ) {
        platform = SST_RUSH;
        if ( getenv( "OGL_ENABLE_RUSH_WINDOWING" ) ) {
            windowable = 1;
        }
    } else {
        /* RETRO3DFX 0.3.6 (Q3 black world at 800/1024, user-caught): any
        ** OTHER hardware string ("Voodoo3 (tm)", "Voodoo5 (tm)", Banshee...)
        ** fell through with platform=SST_VOODOO and, because GR_MEMORY_FB on
        ** Glide3/Napalm returns a byte-scale value that matches no case
        ** below, mem=SST_2M -- i.e. the res walk consulted the VOODOO1-2MB
        ** capability row, whose best double-buffered+Z mode is 640x480.  An
        ** 800x600/1024x768 window therefore opened a 640x480 Glide context
        ** under a game rendering an 800/1024 viewport -> black world (only
        ** ever visible on the monitor; timedemo fps still measured fine, so
        ** every "800/1024" benchmark before this fix really ran at 640).
        ** Modern boards (V3 and up) do 1024x768x16 db+Z trivially: skip the
        ** legacy cap-table gate entirely. */
        platform = -1;   /* sentinel: modern board, cap table bypassed */
    }

    /* XXX Taco - Need a query mechanism for SLI from Glide 3 */
    sli = SST_NOSLI;

    switch( grGetInteger( GR_MEMORY_FB ) ) {
    case 2:
        mem = SST_2M;
        break;
    case 4:
        mem = SST_4M;
        break;
    }
    
    /* walk up the list until width/height match */
    /* walk down the list until supported */
    /* RETRO3DFX 0.3.6: platform<0 = modern board (V3+), every table res is
    ** supported db+Z -- bypass the Voodoo1/2-era capability gate (see the
    ** platform detection above for why the old default clamped to 640x480). */
#define __SST_RES_OK(p,s,m,r) \
    ( (p) < 0 || ( ( __sstResCapTable[(p)][(s)][(m)][(r)] & SST_DBZ ) == SST_DBZ ) )
    step = 1;
    for( res = 0; ( ( res < SST_RESOLUTIONS ) && ( res >= 0 ) ); res += step ) {
        flags = 0;
        if ( width <= __sstResTable[res][0] )
            flags++;
        if ( height <= __sstResTable[res][1] )
            flags++;
        if ( step == 1 ) {
            if ( flags == 2 ) { /* we have a match */
                if ( __SST_RES_OK( platform, sli, mem, res ) ) {
                    break;
                } else {
                    step = -1;
                }
            } else if ( res == SST_1024x768 ) { /* out of resolutions */
                if ( __SST_RES_OK( platform, sli, mem, res ) ) {
                    break;
                } else {
                    step = -1;
                }
            }
        } else { /* step == -1 */
            if ( __SST_RES_OK( platform, sli, mem, res ) ) {
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

    /* RETRO3DFX Glide-context REUSE (GoldSrc/CS fps fix): GoldSrc switches
    ** between several GL contexts on the SAME window every few frames, and
    ** each MakeCurrent used to do a full grSstWinClose+grSstWinOpen round
    ** trip (70-600ms of fullscreen mode-set EACH) -> seconds per frame.
    ** LoseCurrent no longer closes; here we reuse the open Glide context
    ** when the target window is unchanged and only re-apply state.  A
    ** different window (or resolution request) still closes + reopens. */
    { extern long __r3d_openHwnd;
      extern long __r3d_openRes;
      if ( tacoHackGlideInit && tacoHackContext &&
           __r3d_openHwnd == (long)tacoHackHWND &&
           __r3d_openRes == (long)resolution ) {
          OGLLOG( "MakeCurrent: REUSE Glide ctx=0x%x (same hwnd/res, no WinOpen)",
                  (unsigned)tacoHackContext );
          goto __r3d_glide_ready;
      }
      if ( tacoHackGlideInit && tacoHackContext ) {
          OGLLOG( "MakeCurrent: hwnd/res changed -> grSstWinClose(0x%x)",
                  (unsigned)tacoHackContext );
          grSstWinClose( tacoHackContext );
          tacoHackGlideInit = 0;
          /* 0.3.5: latched world texture is context-lifetime */
          { extern int __r3d_blitValid; __r3d_blitValid = 0; }
      }
      __r3d_openHwnd = (long)tacoHackHWND;
      __r3d_openRes  = (long)resolution;
    }

    OGLLOG( "MakeCurrent: fbmem=%dMB res idx=%d -> maxvp %dx%d, calling "
            "grSstWinOpen(hwnd=0x%x res=%d refresh=GR_REFRESH_60Hz "
            "fmt=ARGB origin=UL nCol=2 nAux=1)",
            grGetInteger( GR_MEMORY_FB ), res,
            gc->constants.maxViewportWidth, gc->constants.maxViewportHeight,
            (unsigned)tacoHackHWND, resolution );

    /* RETRO3DFX EXPERIMENT (env RETRO3DFX_32BPP): open the hw color buffer in
     * true-color ARGB_8888 instead of the default RGB565. Must stay in lockstep
     * with __wglGlideGetDisplayMasks/LockBuffer (WGLGLIDE.C) so the ICD's
     * software buffer depth matches the hw. Env-unset = the original 565 path. */
    if ( getenv("RETRO3DFX_32BPP") ) {
        __pfnWinOpenExt pfnExt = (__pfnWinOpenExt) grGetProcAddress( "grSstWinOpenExt" );
        OGLLOG( "MakeCurrent: RETRO3DFX_32BPP -> grSstWinOpenExt=0x%x (GR_PIXFMT_ARGB_8888)",
                (unsigned)pfnExt );
        if ( pfnExt )
            tacoHackContext = (*pfnExt)( tacoHackHWND, resolution, GR_REFRESH_60Hz,
                                         GR_COLORFORMAT_ARGB, GR_ORIGIN_UPPER_LEFT,
                                         GR_PIXFMT_ARGB_8888, 2, 1 );
        else                          /* fallback to 16bpp if the ext isn't resolvable */
            tacoHackContext = grSstWinOpen( tacoHackHWND, resolution, GR_REFRESH_60Hz,
                                            GR_COLORFORMAT_ARGB, GR_ORIGIN_UPPER_LEFT, 2, 1 );
    } else {
        tacoHackContext = grSstWinOpen( tacoHackHWND,
                                           resolution,
                                           GR_REFRESH_60Hz,
                                           GR_COLORFORMAT_ARGB,
                                           GR_ORIGIN_UPPER_LEFT,
                                           2,1 );         /* double-buffer 60Hz = optimal;
                                              triple-buffer(3,1) -> 55.8fps and 85Hz destabilized
                                              (the swap does necessary GPU/SLI sync, not idle wait) */
    }
    if ( !tacoHackContext ) {
        OGLLOG( "MakeCurrent: ** grSstWinOpen FAILED (returned 0) **" );
        return GL_FALSE;
    }
    OGLLOG( "MakeCurrent: grSstWinOpen OK ctx=0x%x",
            (unsigned)tacoHackContext );
    tacoHackGlideInit = 1;

__r3d_glide_ready:
    if ( !gc->modes.doubleBufferMode || getenv( "SST_SINGLEBUFFER" ) ) {
        grRenderBuffer( GR_BUFFER_FRONTBUFFER );
    } else {
        /* explicit: a reused context may have been left on FRONT by a
        ** single-buffered sibling context */
        grRenderBuffer( GR_BUFFER_BACKBUFFER );
    }

    /* XXXshui glide initialization; may need to be moved to the place */
    /* where we create and init a glide context */
    grReset( GR_VERTEX_PARAMETER );
    grVertexLayout( GR_PARAM_XY,  offsetof( GrVertex, x   ), GR_PARAM_ENABLE );
    grVertexLayout( GR_PARAM_RGB, offsetof( GrVertex, r   ), GR_PARAM_ENABLE );
    grVertexLayout( GR_PARAM_Z,   offsetof( GrVertex, ooz ), GR_PARAM_ENABLE );
    grVertexLayout( GR_PARAM_A,   offsetof( GrVertex, a   ), GR_PARAM_ENABLE );

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

    /* RETRO3DFX 2D-text garble A/B: the VSA-100 4x4 ordered dither mottles
    ** flat-color UI text (Q3 proportional menu font, CS HUD) — thin glyph
    ** strokes with every-other-column darkened read as "sliced". The GDI
    ** Generic software-GL reference (no dither) renders the same quads solid.
    ** Env RETRO3DFX_NODITHER forces dither off so one binary can A/B; the
    ** live default is decided after measuring against the GDI oracle. */
    { extern int __r3d_nodither;
      if ( getenv( "RETRO3DFX_NODITHER" ) ) {
          grDitherMode( GR_DITHER_DISABLE );
          __r3d_nodither = 1;
      } }

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
    /* OPT 0.1.4 overbright: the direct combine calls above bypassed the
    ** SST_TEX.C dedup caches -- resync them for this fresh context. */
    __glSSTResetCombineCache();

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
            /* QUALITY FIX menu-text (baseline): do NOT advertise
            ** GL_ARB_multitexture.  That was the unshipped OPT 0.1.4-0.1.6
            ** single-pass/overbright experiment (world lighting collapse +
            ** vertex-color doubling) which never validated clean on the live
            ** Voodoo5 and is not in the kept 0.1.0/0.1.1/0.1.3 set.  With ARB
            ** unadvertised, Q3 falls back to its classic two-pass world path
            ** (identical to shipped 0.1.3 -- in-game q3dm1 stays clean) while
            ** all the dormant multitexture plumbing compiles but never fires.
            ** Reverted to the legacy SGIS-only advertisement. */
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

    OGLLOG( "MakeCurrent: done, returning GL_TRUE" );
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


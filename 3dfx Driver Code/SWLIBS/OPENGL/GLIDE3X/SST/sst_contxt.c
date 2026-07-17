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
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdio.h>
#include "render.h"
#include "context.h"
#include "global.h"
#include "imports.h"
#include "pixel.h"
#include "image.h"
#include "g_imfncs.h"
#if defined(_GL_PC_FAST_RAST)
#include "frastenb.h"
#endif
#include "histogram.h"

//#define __GL_CODEGEN_PMON
#ifdef __GL_CODEGEN_PMON
#include "pmonstat.h"
#endif

#include "sst_globals.h"

extern int __glCanMmx(__GLcontext *gc);

#if defined(__GL_OLD_THREAD)
__GLcontextArea __gl_contextArea;
#endif

static GLfloat DefaultAmbient[4] = { 0.2F, 0.2F, 0.2F, 1.0F };
static GLfloat DefaultDiffuse[4] = { 0.8F, 0.8F, 0.8F, 1.0F };
static GLfloat DefaultBlack[4] = { 0.0F, 0.0F, 0.0F, 1.0F };
static GLfloat DefaultWhite[4] = { 1.0F, 1.0F, 1.0F, 1.0F };

/************************************************************************/

/* Dispatch table override control for external agents like libGLS */

static __GLdispatchState* __glDispatchExec(__GLcontext *gc) {
    return &gc->currentDispatchState;
}

static void __glBeginDispatchOverride(__GLcontext *gc) {
    gc->currentDispatchState = __gl_dispatch;
    if (!gc->dlist.currentList) {
        gc->dispatchState = &gc->currentDispatchState;
    }
}

static void __glEndDispatchOverride(__GLcontext *gc) {
    if (!gc->dlist.currentList) {
        gc->dispatchState = &__gl_dispatch;
    }
}

/************************************************************************/


/*
** Early initialization of context.  Very little is done here, just enough
** to make a context viable.
*/
void __glSSTEarlyInitContext(__GLcontext *gc)
{
    GLint numLights;

    gc->constants.fviewportXAdjust = (__GLfloat) gc->constants.viewportXAdjust;
    gc->constants.fviewportYAdjust = (__GLfloat) gc->constants.viewportYAdjust;
    gc->constants.one = (__GLfloat) 1.0;
    gc->constants.half = (__GLfloat) 0.5;

    /* initialize current color scales to 1.0 */

    gc->constants.oldRedScale   = 1.0;
    gc->constants.oldGreenScale = 1.0;
    gc->constants.oldBlueScale  = 1.0;
    gc->constants.oldAlphaScale = 1.0;

    gc->exports.copyContext = __glCopyContext;
    gc->exports.destroyContext = __glSSTDestroyContext;
    gc->exports.dispatchExec = __glDispatchExec;
    gc->exports.beginDispatchOverride = __glBeginDispatchOverride;
    gc->exports.endDispatchOverride = __glEndDispatchOverride;
    gc->procs.pickColorMaterialProcs =  (void (*)(__GLcontext*))__glNop;
    gc->procs.applyColor = (void (*)(__GLcontext*)) __glNop;

    /* Allocate memory to hold variable sized things */
    numLights = gc->constants.numberOfLights;
    gc->state.light.source = (__GLlightSourceState*)
        (*gc->imports.calloc)(gc, (size_t) numLights,
                              sizeof(__GLlightSourceState));
    gc->light.lutCache = NULL;
    gc->light.source = (__GLlightSourceMachine*)
        (*gc->imports.calloc)(gc, (size_t) numLights,
                              sizeof(__GLlightSourceMachine));
    gc->attributes.stack = (__GLattribute**)
        (*gc->imports.calloc)(gc, (size_t) gc->constants.maxAttribStackDepth,
                              sizeof(__GLattribute*));
    gc->attributes.clientStack = (__GLclientAttribute**)
        (*gc->imports.calloc)(gc,
                              (size_t) gc->constants.maxClientAttribStackDepth,
                              sizeof(__GLclientAttribute*));
    gc->select.stack = (GLuint*)
        (*gc->imports.calloc)(gc, (size_t) gc->constants.maxNameStackDepth,
                              sizeof(GLuint));

#ifdef _WIN32
    /*
    ** Allocate shader buffers.  This was formerly allocated on the
    ** stack during rasterization, but that is expensive on Win95.
    */
    gc->polygon.shader.colors = (__GLcolor *)
        (*gc->imports.calloc)(gc, (size_t) __GL_MAX_MAX_VIEWPORT,
                              sizeof(__GLcolor));
    gc->polygon.shader.fbcolors = (__GLcolor *)
        (*gc->imports.calloc)(gc, (size_t) __GL_MAX_MAX_VIEWPORT,
                              sizeof(__GLcolor));
    gc->polygon.shader.stipplePat = (__GLstippleWord *)
        (*gc->imports.calloc)(gc, (size_t) __GL_MAX_STIPPLE_WORDS,
                              sizeof(__GLstippleWord));
#endif /* WIN32 */

    __glSSTEarlyInitTextureState(gc);
    __glEarlyInitTransformState(gc);
    __glSSTEarlyInitPrimitiveState(gc);

#if __GL_NUMBER_OF_AUX_BUFFERS > 0
    /*
    ** Allocate aux color buffer records
    ** Note: Does not allocate the actual buffer memory, this is done 
    **  elsewhere.
    */
    gc->auxBuffer = (__GLcolorBuffer *)
        (*gc->imports.calloc)(gc, gc->modes.numAuxBuffers,
                              sizeof(__GLcolorBuffer));
#endif

    gc->procs.memory.newArena = __glNewArena;
    gc->procs.memory.deleteArena = __glDeleteArena;
    gc->procs.memory.alloc = __glArenaAlloc;
    gc->procs.memory.freeAll = __glArenaFreeAll;

#ifdef __GL_CODEGEN
    gc->constants.textureReadCacheCount = 4;
    gc->constants.textureReadCacheSize = 1024;
    gc->constants.depthTestCacheCount = 4;
    gc->constants.depthTestCacheSize = 256;
    gc->constants.triSetupCacheCount = 40;
    gc->constants.triSetupCacheSize = 4096;
    gc->constants.triSetupDataSize = 1024;
    gc->constants.triRasterCacheCount = 40;
    gc->constants.triRasterCacheSize = (4 * 4096);

    __glInitializeCodeCache(gc);

    gc->geomOGdata.last_hit = &gc->geomOGdata.slots[0];
    memset(gc->geomOGdata.slots, 0, sizeof(gc->geomOGdata.slots));

    gc->ogState.fpconstant_size = 1000;
    gc->ogState.fpconstant_next = 0;
    gc->ogState.fpconstant_pool = calloc(sizeof(float), gc->ogState.fpconstant_size);

    gc->ogState.mxconstant_size = 1000;
    gc->ogState.mxconstant_next = 0;
    gc->ogState.mxconstant_pool = calloc(sizeof(double), gc->ogState.mxconstant_size);

#endif

#ifdef _WIN32
    {
        if ( getenv( "GL_PERF_HISTOGRAM" ) )
        {
            unsigned long lowest_latency = 10000;
            int           i;
            unsigned long size;

            size = atoi( getenv( "GL_PERF_HISTOGRAM" ) );

            if ( size < 2 )
               size = 500;
            else if ( size > 1000 )
               size = 1000;
            
            /*
            ** measure RDTSC overhead
            */
            for ( i = 0; i < 100; i++ )
            {
               unsigned long a, b;

               RDTSC( a );
               RDTSC( b );
               if ( b - a < lowest_latency )
                  lowest_latency = b - a;
            }

            gc->perf_histogram = ( void * ) __glHistogramCreate( gc, size, lowest_latency );
        }
        else
        {
            gc->perf_histogram = 0;
        }
    }
#endif

#ifdef __GL_CODEGEN_PMON
    {
        static int cold = 1;
        
        if (cold) {
            InitPmonCounters("pmon.log", "/pmonstat/example/pmonstat.cfg", 34, 35, Ring123);
            cold = 0;
        }
    }
#endif

    __glInitDlistState(gc);

}

/*
** Destroy a context.  If it's the current context then the
** current context is set to GL_NULL.
*/
GLboolean __glSSTDestroyContext(__GLcontext *gc)
{
#ifdef _WIN32
    if (gc->perf_histogram )
    {
        FILE *fp = fopen( "hist.out", "wt" );

        __glHistogramSummarize( gc->perf_histogram );
        if ( fp )
        {
            __glHistogramPrint( gc->perf_histogram, fp, 0.01F );
            fclose( fp );
        }
        else
        {
            OutputDebugString( "__glDestroyContext() - could not open 'hist.out'\n" );
        }
        __glHistogramDestroy( gc, gc->perf_histogram );
        gc->perf_histogram = 0;
    }
#endif

#ifdef __GL_CODEGEN
    __glReleaseCodeCache(gc);
#endif

    if (gc->attributes.stack) {
        __glFreeAttributeState(gc);
        (*gc->imports.free)(gc, gc->attributes.stack);
    }

    if (gc->attributes.clientStack) {
        (*gc->imports.free)(gc, gc->attributes.clientStack);
    }

    if(gc->state.light.source) (*gc->imports.free)(gc, gc->state.light.source);
    if(gc->light.source) (*gc->imports.free)(gc, gc->light.source);
    if(gc->select.stack) (*gc->imports.free)(gc, gc->select.stack);

    if(gc->state.transform.eyeClipPlanes) (*gc->imports.free)(gc, gc->state.transform.eyeClipPlanes);
    if(gc->transform.modelViewStack) (*gc->imports.free)(gc, gc->transform.modelViewStack);
    if(gc->transform.projectionStack) (*gc->imports.free)(gc, gc->transform.projectionStack);
    if(gc->transform.textureStack[0]) (*gc->imports.free)(gc, gc->transform.textureStack[0]);
        if(gc->transform.textureStack[1]) (*gc->imports.free)(gc, gc->transform.textureStack[1]);
    if(gc->transform.clipTemp) (*gc->imports.free)(gc, gc->transform.clipTemp);

    if (gc->polygon.shader.colors)
        (*gc->imports.free)(gc, gc->polygon.shader.colors);
    if (gc->polygon.shader.fbcolors)
        (*gc->imports.free)(gc, gc->polygon.shader.fbcolors);
    if (gc->polygon.shader.stipplePat)
        (*gc->imports.free)(gc, gc->polygon.shader.stipplePat);

    /* XXX: The following should be freed in *_rgb.c or *_ci.c.  */
    /* FYI: It is allocated lazylily in so_alphatst.c */
    if( gc->frontBuffer.alphaTestFuncTable ) {
        (*gc->imports.free)(gc, gc->frontBuffer.alphaTestFuncTable);
    }

    /*
    ** Free other malloc'd data associated with the context
    */
    __glFreeColorTables(gc);
    __glFreeEvaluatorState(gc);
    __glFreePixelState(gc);
    __glFreeVertexArrayState(gc);
    if (gc->dlist.dlistArray) __glFreeDlistState(gc);
    if (gc->texture.texture) __glSSTFreeTextureState(gc);
    /*
    ** The material state may have references to the LUT cache, so
    ** free the material state entries to decrement the refcounts
    ** and allow the whole LUT cache to be freed.
    */
    if (gc->light.front.cache) __glFreeSpecLUT(gc, gc->light.front.cache);
    if (gc->light.back.cache) __glFreeSpecLUT(gc, gc->light.back.cache);
    if (gc->light.lutCache) __glFreeLUTCache(gc);
    __glFreeVertexCacheState(gc);

#if __GL_NUMBER_OF_AUX_BUFFERS > 0
    /*
    ** Free any aux color buffer records
    ** Note: Does not free the actual buffer memory, this is done elsewhere.
    */
    if (gc->auxBuffer) (*gc->imports.free)(gc, gc->auxBuffer);
#endif

    /*
    ** Note: We do not free the software buffers here.  They are attached
    ** to the drawable, and is the glx extension's responsibility to free
    ** them when the drawable is destroyed.
    */

    return GL_TRUE;
}


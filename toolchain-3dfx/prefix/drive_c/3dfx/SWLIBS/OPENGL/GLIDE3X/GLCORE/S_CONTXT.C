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
void __glEarlyInitContext(__GLcontext *gc)
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
    gc->exports.destroyContext = __glDestroyContext;
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

    __glEarlyInitTextureState(gc);
    __glEarlyInitTransformState(gc);

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

void __glContextSetColorScales(__GLcontext *gc)
{
    __GLfloat one = __glOne;
    __GLattribute **spp;
    __GLattribute *sp;
    GLfloat tmpRedScale, tmpGreenScale, tmpBlueScale, tmpAlphaScale;
    GLuint mask;
    GLint i;
    
    if((gc->constants.oldRedScale == gc->frontBuffer.redScale) &&
       (gc->constants.oldGreenScale == gc->frontBuffer.greenScale) &&
       (gc->constants.oldBlueScale == gc->frontBuffer.blueScale) &&
       (gc->constants.oldAlphaScale == gc->frontBuffer.alphaScale)) {
      return; /* color scale factor didn't change, don't do anything */
    }
    
    /*
    ** We'll temporarly set gc->frontBuffer.*Scale values to the ratio
    ** of: redScale/oldRedScale, greenScale/oldGreenScale etc. We
    ** do this because the following code uses the frontBuffer.*Scale
    ** values to rescale OpenGL's various constants.
    **
    ** Other code, outside this function, expects these variables to
    ** contain the current scaling, not a ratio, so we restore the
    ** variables to their correct values at the end of this function.
    */

    /* save current scale values */
    tmpRedScale   = gc->frontBuffer.redScale;
    tmpGreenScale = gc->frontBuffer.greenScale;
    tmpBlueScale  = gc->frontBuffer.blueScale;
    tmpAlphaScale = gc->frontBuffer.alphaScale;

    /* compute correct inverse (not used in this function) */
    gc->frontBuffer.oneOverRedScale = one / gc->frontBuffer.redScale;
    gc->frontBuffer.oneOverGreenScale = one / gc->frontBuffer.greenScale;
    gc->frontBuffer.oneOverBlueScale = one / gc->frontBuffer.blueScale;
    gc->frontBuffer.oneOverAlphaScale = one / gc->frontBuffer.alphaScale;

    /* calculate temporary, combined scale factor for use in this function */
    gc->frontBuffer.redScale   /= gc->constants.oldRedScale;
    gc->frontBuffer.greenScale /= gc->constants.oldGreenScale;
    gc->frontBuffer.blueScale  /= gc->constants.oldBlueScale;
    gc->frontBuffer.alphaScale /= gc->constants.oldAlphaScale;

    /*
    ** update current, correct scale values, for next time.
    */
    for (spp = &gc->attributes.stack[0]; spp < gc->attributes.stackPointer; 
            spp++) {
        sp = *spp;
        mask = sp->mask;

        if (mask & GL_CURRENT_BIT) {
            if (gc->modes.rgbMode) {
                __glScaleColorf(gc,
                    &sp->current.rasterPos.colors[__GL_FRONTFACE],
                    &sp->current.rasterPos.colors[__GL_FRONTFACE].r);
                __glScaleColorf(gc,
                    &sp->current.color,
                    &sp->current.color.r);
            }
        }
        if (mask & GL_LIGHTING_BIT) {
            __glScaleColorf(gc,
                &sp->light.model.ambient,
                &sp->light.model.ambient.r);
            for (i=0; i<gc->constants.numberOfLights; i++) {
                __glScaleColorf(gc,
                    &sp->light.source[i].ambient,
                    &sp->light.source[i].ambient.r);
                __glScaleColorf(gc,
                    &sp->light.source[i].diffuse,
                    &sp->light.source[i].diffuse.r);
                __glScaleColorf(gc,
                    &sp->light.source[i].specular,
                    &sp->light.source[i].specular.r);
            }
            __glScaleColorf(gc,
                &sp->light.front.emissive,
                &sp->light.front.emissive.r);
            __glScaleColorf(gc,
                &sp->light.back.emissive,
                &sp->light.back.emissive.r);
        }
    }

    if (gc->modes.rgbMode) {
        __glScaleColorf(gc, 
                &gc->state.current.rasterPos.colors[__GL_FRONTFACE], 
                &gc->state.current.rasterPos.colors[__GL_FRONTFACE].r);
        __glScaleColorf(gc,
                &gc->state.current.color,
                &gc->state.current.color.r);
    } 

    __glScaleColorf(gc, 
            &gc->state.light.model.ambient,
            &gc->state.light.model.ambient.r);
    for (i=0; i<gc->constants.numberOfLights; i++) {
        __glScaleColorf(gc,
                &gc->state.light.source[i].ambient,
                &gc->state.light.source[i].ambient.r);
        __glScaleColorf(gc,
                &gc->state.light.source[i].diffuse,
                &gc->state.light.source[i].diffuse.r);
        __glScaleColorf(gc,
                &gc->state.light.source[i].specular,
                &gc->state.light.source[i].specular.r);
    }
    __glScaleColorf(gc,
            &gc->state.light.front.emissive, 
            &gc->state.light.front.emissive.r);
    __glScaleColorf(gc,
            &gc->state.light.back.emissive, 
            &gc->state.light.back.emissive.r);


    /* restore frontBuffer.*Scale variables to their proper values */
    gc->frontBuffer.redScale   = tmpRedScale;  
    gc->frontBuffer.greenScale = tmpGreenScale; 
    gc->frontBuffer.blueScale  = tmpBlueScale;  
    gc->frontBuffer.alphaScale = tmpAlphaScale;


    /* update contants.*Scale variables to their new values */
    gc->constants.oldRedScale   = gc->frontBuffer.redScale;
    gc->constants.oldGreenScale = gc->frontBuffer.greenScale;
    gc->constants.oldBlueScale  = gc->frontBuffer.blueScale;
    gc->constants.oldAlphaScale = gc->frontBuffer.alphaScale;

    /*
    ** this function doesn't need to be "unscaled", so it is called
    ** after frontBufer.*Scale have been restored to proper values
    */
    __glPixelSetColorScales(gc);
}

/*
** Initialize all user controllable state, plus any computed state that
** is only set by user commands.  For example, light source position
** is converted immediately into eye coordinates.
**
** Any state that would be initialized to zero is not done here because
** the memory assigned to the context has already been block zeroed.
*/
void __glSoftResetContext(__GLcontext *gc)
{
    __GLlightSourceState *lss;
    __GLlightSourceMachine *lsm;
    __GLvertex *vx;
    GLint i, numLights;
    __GLfloat one = __glOne;

    /*
    ** Initialize constant values first so that they will
    ** be valid if needed by subsequent initialization steps.
    */

    /* Setup generic values for get strings */
    /* XXXshui since this is dev dep, we may make a sst version of this routine */    
    gc->constants.vendor = "3Dfx Interactive Inc.";
    gc->constants.renderer = "3Dfx [retro3dfx 0.3.8]";
    gc->constants.version = "1.1.0 3Dfx Beta 3.00";
    /*
    ** Put new extension names in alphabetical order; make sure to include
    ** a space after the name.
    */
    gc->constants.extensions = 
#if 0 /* XXXwheeler don't enable until optimized */
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
#endif      
        ;

    /*
    ** Not quite 2^31-1 because of possible floating point errors.  4294965000
    ** is a much safer number to use.
    */
    gc->constants.val255 = (__GLfloat) 255.0;
    gc->constants.val65535 = (__GLfloat) 65535.0;
    gc->constants.val4294965000 = (__GLfloat) 4294965000.0;
    gc->constants.oneOver255 = one / gc->constants.val255;
    gc->constants.oneOver65535 = one / gc->constants.val65535;
    gc->constants.oneOver4294965000 = one / gc->constants.val4294965000;

    if (gc->constants.alphaTestSize == 0) {
        gc->constants.alphaTestSize = 256;      /* A default */
    }
    gc->constants.alphaTableConv = (gc->constants.alphaTestSize - 1) / 
            gc->frontBuffer.alphaScale;

    gc->frontBuffer.oneOverRedScale = one / gc->frontBuffer.redScale;
    gc->frontBuffer.oneOverGreenScale = one / gc->frontBuffer.greenScale;
    gc->frontBuffer.oneOverBlueScale = one / gc->frontBuffer.blueScale;
    gc->frontBuffer.oneOverAlphaScale = one / gc->frontBuffer.alphaScale;

    /* Lookup table used in macro __GL_UB_TO_FLOAT */
    for (i = 0; i < 256; i++) {
        gc->constants.uByteToFloat[i] = i * gc->constants.oneOver255;
    }

#if defined(_GL_PC_FAST_RAST)
    /* Reciprocal table used in fast rasterizer code. */
    InitReciprocalTable();
#endif


    /* XXX - Ick! */
    /* XXX This almost certainly does the wrong thing on a PC.     */
    /* The (temp2 != temp1) test will be carried out within double */
    /* precision floating point registers, which will then make    */
    /* besteps a double precision epsilon, with the effect that    */
    /* gc->constants.viewportAlmostHalf == __glHalf                */ 
      
    {
        /* Compute some fixed point viewport constants */
        volatile __GLfloat temp1, temp2;
        volatile __GLfloat epsilon, besteps;

        temp1 = gc->constants.fviewportXAdjust;
        epsilon = one;
        besteps = one;
        for (;;) {
            temp2 = temp1 + epsilon;
            if (temp2 != temp1) {
                besteps = epsilon;
            } else
                break;
            epsilon = epsilon/2;
        }

        /* XXXwheeler: this makes conform happier */
        besteps = 0.0625;
        gc->constants.viewportEpsilon = besteps;
        gc->constants.viewportAlmostHalf = __glHalf - besteps;
    }

    /* Allocate memory to hold variable sized things */
    numLights = gc->constants.numberOfLights;

    /* Misc machine state */
    __gl_beginMode = __GL_NEED_VALIDATE;
    gc->dirtyMask = __GL_DIRTY_ALL;
    gc->validateMask = ~0;
    gc->validateTexture = 1;
    gc->cdrsTexture = 0;
    gc->glideAALineFunc = (void (__stdcall *)(const void *, const void *))
      grGetProcAddress("grDrawTextureLineExt");
    gc->glideLineFunc = grDrawLine;
    gc->slowPath = 0;
    gc->attributes.stackPointer = &gc->attributes.stack[0];
    gc->attributes.clientStackPointer = &gc->attributes.clientStack[0];
    gc->vertex.v0 = &gc->vertex.vbuf[0];

    vx = &gc->vertex.vbuf[0];
    for (i = 0; i < __GL_NVBUF; i++, vx++) {
        vx->color = &vx->colors[__GL_FRONTFACE];
    }

    /* GL_LIGHTING_BIT state */
    gc->state.light.model.ambient.r = DefaultAmbient[0];
    gc->state.light.model.ambient.g = DefaultAmbient[1];
    gc->state.light.model.ambient.b = DefaultAmbient[2];
    gc->state.light.model.ambient.a = DefaultAmbient[3];
    gc->state.light.front.ambient.r = DefaultAmbient[0];
    gc->state.light.front.ambient.g = DefaultAmbient[1];
    gc->state.light.front.ambient.b = DefaultAmbient[2];
    gc->state.light.front.ambient.a = DefaultAmbient[3];
    gc->state.light.front.diffuse.r = DefaultDiffuse[0];
    gc->state.light.front.diffuse.g = DefaultDiffuse[1];
    gc->state.light.front.diffuse.b = DefaultDiffuse[2];
    gc->state.light.front.diffuse.a = DefaultDiffuse[3];
    gc->state.light.front.specular.r = DefaultBlack[0];
    gc->state.light.front.specular.g = DefaultBlack[1];
    gc->state.light.front.specular.b = DefaultBlack[2];
    gc->state.light.front.specular.a = DefaultBlack[3];
    gc->state.light.front.emissive.r = DefaultBlack[0];
    gc->state.light.front.emissive.g = DefaultBlack[1];
    gc->state.light.front.emissive.b = DefaultBlack[2];
    gc->state.light.front.emissive.a = DefaultBlack[3];
    gc->state.light.front.cmapa = 0;
    gc->state.light.front.cmaps = 1;
    gc->state.light.front.cmapd = 1;
    gc->state.light.back = gc->state.light.front;

    gc->light.front.specularExponent = -1;
    gc->light.front.specTable = NULL;
    gc->light.front.cache = NULL;
    gc->light.back.specularExponent = -1;
    gc->light.back.specTable = NULL;
    gc->light.back.cache = NULL;

    /* Initialize the individual lights */
    lss = &gc->state.light.source[0];
    lsm = &gc->light.source[0];
    for (i = 0; i < numLights; i++, lss++, lsm++) {
        lss->ambient.r = DefaultBlack[0];
        lss->ambient.g = DefaultBlack[1];
        lss->ambient.b = DefaultBlack[2];
        lss->ambient.a = DefaultBlack[3];
        if (i == 0) {
            lss->diffuse.r = DefaultWhite[0];
            lss->diffuse.g = DefaultWhite[1];
            lss->diffuse.b = DefaultWhite[2];
            lss->diffuse.a = DefaultWhite[3];
        } else {
            lss->diffuse.r = DefaultBlack[0];
            lss->diffuse.g = DefaultBlack[1];
            lss->diffuse.b = DefaultBlack[2];
            lss->diffuse.a = DefaultBlack[3];
        }
        lss->specular = lss->diffuse;
        lss->position.z = __glOne;
        lss->positionEye.z = __glOne;
        lsm->position.z = __glOne;
        lss->direction.z = __glMinusOne;
        lsm->direction.z = __glMinusOne;
        lss->spotLightCutOffAngle = 180;
        lss->constantAttenuation = __glOne;
        lsm->spotTable = NULL;
        lsm->spotLightExponent = -1;
        lsm->cache = NULL;
    }
    gc->state.light.colorMaterialFace = GL_FRONT_AND_BACK;
    gc->state.light.colorMaterialParam = GL_AMBIENT_AND_DIFFUSE;
    gc->state.light.indexMaterialFace = GL_FRONT_AND_BACK;
    gc->state.light.indexMaterialParam = GL_INDEX_OFFSET;
    gc->state.light.shadingModel = GL_SMOOTH;

    /* GL_HINT_BIT state */
    gc->state.hints.perspectiveCorrection = GL_DONT_CARE;
    gc->state.hints.pointSmooth = GL_DONT_CARE;
    gc->state.hints.lineSmooth = GL_DONT_CARE;
    gc->state.hints.polygonSmooth = GL_DONT_CARE;
    gc->state.hints.fog = GL_DONT_CARE;

    /* GL_CURRENT_BIT state */
    gc->state.current.rasterPos.window.x = 
                        gc->constants.fviewportXAdjust + __glHalf;
        /*
        ** Note that window.y will be adjusted in MakeCurrent if 
        ** yInvert is needed. Don't know window height at this point.
        */
    gc->state.current.rasterPos.window.y = 
                        gc->constants.fviewportYAdjust + __glHalf;
    
    if (gc->constants.yInverted)
        gc->state.current.rasterPos.window.y -= gc->constants.viewportEpsilon;                     
    gc->state.current.rasterPos.clip.w = __glOne;
    gc->state.current.rasterPos.texture[0].w = __glOne;
    gc->state.current.rasterPos.color
        = &gc->state.current.rasterPos.colors[__GL_FRONTFACE];
    if (gc->modes.rgbMode) {
        gc->state.current.rasterPos.colors[__GL_FRONTFACE].r = DefaultWhite[0];
        gc->state.current.rasterPos.colors[__GL_FRONTFACE].g = DefaultWhite[1];
        gc->state.current.rasterPos.colors[__GL_FRONTFACE].b = DefaultWhite[2];
        gc->state.current.rasterPos.colors[__GL_FRONTFACE].a = DefaultWhite[3];
    } else {
        gc->state.current.rasterPos.colors[__GL_FRONTFACE].r = __glOne;
    }
    gc->state.current.validRasterPos = GL_TRUE;
    gc->state.current.edgeTag = GL_TRUE;

    /* GL_FOG_BIT state */
    gc->state.fog.mode = GL_EXP;
    gc->state.fog.density = __glOne;
    gc->state.fog.end = 1.0;

    /* GL_POINT_BIT state */
    gc->state.point.requestedSize = 1.0;
    gc->state.point.smoothSize = 1.0;
    gc->state.point.aliasedSize = 1;
    gc->state.point.a = 1.0;
    gc->state.point.fadeThreshold = 1.0;
    gc->state.point.max = gc->constants.pointSizeMaximum;

    /* GL_LINE_BIT state */
    gc->state.line.requestedWidth = 1.0;
    gc->state.line.smoothWidth = 1.0;
    gc->state.line.aliasedWidth = 1;
    gc->state.line.stipple = 0xFFFF;
    gc->state.line.stippleRepeat = 1;

    /* GL_POLYGON_BIT state */
    gc->state.polygon.frontMode = GL_FILL;
    gc->state.polygon.backMode = GL_FILL;
    gc->state.polygon.cull = GL_BACK;
    gc->state.polygon.frontFaceDirection = GL_CCW;

    /* GL_POLYGON_STIPPLE_BIT state */
    for (i = 0; i < 4*32; i++) {
        gc->state.polygonStipple.stipple[i] = 0xFF;
    }
    for (i = 0; i < 32; i++) {
        gc->polygon.stipple[i] = 0xFFFFFFFF;
    }

    /* GL_ACCUM_BUFFER_BIT state */

    /* GL_STENCIL_BUFFER_BIT state */
    gc->state.stencil.testFunc = GL_ALWAYS;
    gc->state.stencil.mask = __GL_MAX_STENCIL_VALUE;
    gc->state.stencil.fail = GL_KEEP;
    gc->state.stencil.depthFail = GL_KEEP;
    gc->state.stencil.depthPass = GL_KEEP;
    gc->state.stencil.writeMask = __GL_MAX_STENCIL_VALUE;

    /* GL_DEPTH_BUFFER_BIT state */
    gc->state.depth.writeEnable = GL_TRUE;
    gc->state.depth.testFunc = GL_LESS;
    gc->state.depth.clear = __glOne;

    /* GL_COLOR_BUFFER_BIT state */
    gc->renderMode = GL_RENDER;
    gc->state.raster.alphaFunction = GL_ALWAYS;
    gc->state.raster.blendSrc = GL_ONE;
    gc->state.raster.blendDst = GL_ZERO;
    gc->state.raster.indexFunction = GL_ALWAYS;
    gc->state.raster.logicOp = GL_COPY;
    gc->state.raster.rMask = GL_TRUE;
    gc->state.raster.gMask = GL_TRUE;
    gc->state.raster.bMask = GL_TRUE;
    gc->state.raster.aMask = GL_TRUE;
    if (gc->modes.doubleBufferMode) {
        gc->state.raster.drawBuffer = GL_BACK;
    } else {
        gc->state.raster.drawBuffer = GL_FRONT;
    }
    gc->state.raster.drawBufferReturn = gc->state.raster.drawBuffer;
    gc->state.current.userColor.r = 1.0;
    gc->state.current.userColor.g = 1.0;
    gc->state.current.userColor.b = 1.0;
    gc->state.current.userColor.a = 1.0;
    gc->state.current.userColorIndex = 1.0;
    if (gc->modes.colorIndexMode) {
        gc->state.raster.writeMask = gc->frontBuffer.redMax;
        gc->state.current.color.r = gc->state.current.userColorIndex;
    } else {
        gc->state.current.color = gc->state.current.userColor;
    }
    gc->state.enables.general |= __GL_DITHER_ENABLE;

    gc->select.hit = GL_FALSE;
    gc->select.sp = gc->select.stack;

    /*
    ** Initialize larger subsystems by calling their init codes.
    */
    __glInitColorTables(gc);
    __glInitEvaluatorState(gc);
    __glInitTextureState(gc);
    __glInitTransformState(gc);
    __glInitPixelState(gc);
    __glInitLUTCache(gc);
    __glInitVertexArrayState(gc);
    __glInitVertexCacheState(gc);
}

/*
** Destroy a context.  If it's the current context then the
** current context is set to GL_NULL.
*/
GLboolean __glDestroyContext(__GLcontext *gc)
{
    extern void __glSSTFreeTextureState(__GLcontext *gc);
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
    if (gc->texture.texture[0]||
        gc->texture.texture[1] ) __glSSTFreeTextureState(gc);
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

void __glSetError(GLenum code)
{
    __GL_SETUP();

    {
        extern char* getenv(const char*);

        if (getenv("GLERRORABORT")) {
            fprintf(stderr, "__glSetError(): GLERRORABORT set, aborting.\n");
            abort();
        }
    }

    if (!gc->error) {
        gc->error = code;
    }
    if (gc->procs.error) (*gc->procs.error)(gc, code);
}

GLint APIENTRY __glim_RenderMode(GLenum mode)
{
    GLint rv;
    __GL_SETUP_NOT_IN_BEGIN2();

    switch (mode) {
      case GL_RENDER:
          gc->slowPath &= ~__GL_RENDERMODE_SLOWPATH;
          break;
      case GL_FEEDBACK:
      case GL_SELECT:
          gc->slowPath |= __GL_RENDERMODE_SLOWPATH;
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return 0;
    }

    __GL_API_STATE();

    /* Switch out of old render mode.  Get return value. */
    switch (gc->renderMode) {
      case GL_RENDER:
        rv = 0;
        break;
      case GL_FEEDBACK:
        rv = gc->feedback.overFlowed ? -1 :
            (GLint)(gc->feedback.result - gc->feedback.resultBase);
        break;
      case GL_SELECT:
        rv = gc->select.overFlowed ? -1 : gc->select.hits;
        break;
    }

    /* Switch to new render mode */
    gc->renderMode = mode;
    __GL_DELAY_VALIDATE(gc);
    switch (mode) {
      case GL_FEEDBACK:
        if (!gc->feedback.resultBase) {
            __glSetError(GL_INVALID_OPERATION);
            return rv;
        }
        gc->feedback.result = gc->feedback.resultBase;
        gc->feedback.overFlowed = GL_FALSE;
        break;
      case GL_SELECT:
        if (!gc->select.resultBase) {
            __glSetError(GL_INVALID_OPERATION);
            return rv;
        }
        gc->select.result = gc->select.resultBase;
        gc->select.overFlowed = GL_FALSE;
        gc->select.sp = gc->select.stack;
        gc->select.hit = GL_FALSE;
        gc->select.hits = 0;
        gc->select.z = 0;
        break;
    }
    return rv;
}

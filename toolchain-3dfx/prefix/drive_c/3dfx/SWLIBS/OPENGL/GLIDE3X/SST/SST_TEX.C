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
*/
#include <windows.h>
#include <stddef.h>
#include "context.h"
#include "imports.h"
#include "global.h"
#include "g_imfncs.h"
#include "types.h"
#include "namesint.h"
#include "pixel.h"
#include "image.h"
#include "glmath.h"
#include <memory.h>

#include <glide.h>
#include <g3ext.h>   /* OPT 0.1.4 overbright: GR_CMBX_* / GR_FUNC_MODE_* */
#include <string.h>
#include "sst_globals.h"

/* RETRO3DFX grTexSource logging (GoldSrc color hunt): every source op with
** tmu/addr/format for the first 300 calls after the marker-created log
** engages -- shows exactly what each TMU samples at draw time.  Wrapped
** via self-referential macro like the download counters below. */
static void __r3dLogTexSource(int tmu, unsigned long addr, void *info);
#define grTexSource(a,b,c,d) (__r3dLogTexSource((int)(a),(unsigned long)(b),(void*)(d)), \
                              grTexSource(a,b,c,d))

/* RETRO3DFX_PERFLOG counters (defined in sst_export.c, dumped each 100
** frames).  Self-referential macros: the inner name is not re-expanded, so
** every grTexDownload* call site in this file is counted transparently. */
extern long __r3d_cTexDl, __r3d_cTexDlPart;
#define grTexDownloadMipMapLevel(a,b,c,d,e,f,g,h) \
        (__r3d_cTexDl++, grTexDownloadMipMapLevel(a,b,c,d,e,f,g,h))
#define grTexDownloadMipMapLevelPartial(a,b,c,d,e,f,g,h,i,j) \
        (__r3d_cTexDlPart++, grTexDownloadMipMapLevelPartial(a,b,c,d,e,f,g,h,i,j))

/* crash-robust append logger to C:\3dfxogl.log (defined in WGL/WGLCMDS.C);
** declared early for the FILT@ instrumentation of grTexFilterMode sites. */
extern void OGLLOG( const char *fmt, ... );
extern void OGLLOGV( const char *fmt, ... );

/* body for the grTexSource wrapper macro above (GrTexInfo layout: smallLod,
** largeLod, aspect, format at offsets 0/4/8/12). */
static void __r3dLogTexSource(int tmu, unsigned long addr, void *info)
{
    /* tmu=0 sources are the rare interesting ones (unit-1/lightmap TMU on
    ** the inverted mapping) -- log them all (cap 100).  tmu=1 floods, log
    ** every 500th as a heartbeat so the in-map era is still visible. */
    static int logged0 = 0; static long n1 = 0;
    unsigned long *gi = (unsigned long *)info;
    if (tmu == 0) {
        if (logged0 >= 100) return;
        logged0++;
        OGLLOGV("TEXSRC tmu=0 addr=0x%lx lodS=%lu lodL=%lu asp=%lu fmt=0x%lx",
                addr, gi ? gi[0] : 0, gi ? gi[1] : 0, gi ? gi[2] : 0, gi ? gi[3] : 0);
    } else {
        n1++;
        if (n1 % 500) return;
        OGLLOGV("TEXSRC tmu=1 (heartbeat %ld) addr=0x%lx fmt=0x%lx",
                n1, addr, gi ? gi[3] : 0);
    }
}

#define __GL_TEXTURE_INDEX_1D 0
#define __GL_TEXTURE_INDEX_2D 1
#define __GL_PROXY_TEXTURE_INDEX_1D 2
#define __GL_PROXY_TEXTURE_INDEX_2D 3

static GLfloat Clampf(GLfloat fval, __GLfloat zero, __GLfloat one)
{
    if (fval < zero) return zero;
    else if (fval > one) return one;
    else return fval;
}

/************************************************************************/

/*ARGSUSED*/
GLvoid __glSSTCopyTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        __GLtexture *tex, GLint lod)
{
    (*gc->procs.copyImage)(gc, spanInfo, GL_TRUE);
}


/*ARGSUSED*/
GLvoid __glSSTReadTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        __GLtexture *tex, GLint lod)
{
    (*gc->procs.readImage)(gc, spanInfo);
}

/************************************************************************/

static void freenode(__GLcontext *gc, __GLSSTtexCacheNode *f);
GLvoid __glSSTFreeTexObj(__GLcontext *gc, __GLtextureObject *texobj);
__GLtextureBuffer *__glSSTTexCreateProxyLevel(__GLcontext *gc, __GLtexture *tex,
                                           GLint lod, GLint components,
                                           GLsizei w, GLsizei h, GLsizei d,
                                           GLint border, GLint dim);
__GLtextureBuffer *__glSSTTexCreateLevel(__GLcontext *gc, __GLtexture *tex,
                                      GLint lod, GLint components,
                                      GLsizei w, GLsizei h, GLsizei d,
                                      GLint border, GLint dim);

/*
** Initialize everything in a texture object except the textureMachine.
*/
/* ARGSUSED */
GLvoid __glSSTTexInitTextureObject(__GLcontext *gc, __GLtextureObject *texobj, 
                             GLuint name, GLuint targetIndex)
{
    assert(NULL != texobj);
    texobj->free = __glSSTFreeTexObj;
    texobj->targetIndex = targetIndex;
    texobj->resident = GL_FALSE;
    texobj->texture.map.copyTexImage = __glSSTCopyTexImage;
    texobj->texture.map.readTexImage = __glSSTReadTexImage;
    texobj->texture.map.texobjs.name = name;
    texobj->texture.map.texobjs.priority = 1.0;

#ifdef GL_EXT_paletted_texture
    switch (targetIndex) {
    case __GL_TEXTURE_INDEX_1D:
        texobj->texture.map.CT.target = GL_TEXTURE_1D;
        break;
    case __GL_TEXTURE_INDEX_2D:
        texobj->texture.map.CT.target = GL_TEXTURE_2D;
        break;
    }

    texobj->texture.map.CT.format = GL_RGBA;
#endif
}

/************************************************************************/

/*
** Used to share texture objects between two different contexts.
*/
void __glSSTShareTextureObjects(__GLcontext *gc, __GLcontext *shareMe)
{
    /* First get rid of our private texture object state */
    gc->texture.namesArray->refcount--;
    if (gc->texture.namesArray->refcount == 0) {
        __glNamesFreeArray(gc, gc->texture.namesArray);
    }
    gc->texture.namesArray = NULL;
    gc->texture.namesArray = shareMe->texture.namesArray;
    gc->texture.namesArray->refcount++;
}

void __glSSTEarlyInitTextureState(__GLcontext *gc)
{
    GLint numTextures, numEnvs;
    GLint i,maxMipMapLevel;
    __GLtextureObject *texobj;
#ifdef __GL_SST
    GLuint texUnit;
#endif

    gc->texture.currentTexUnit = 0;

    /* initial routine pick */
    if (gc->texture.initTextureObject == NULL) {
        gc->texture.initTextureObject = __glSSTTexInitTextureObject;
    }
    if (gc->texture.createLevel == NULL) {
        gc->texture.createLevel = __glSSTTexCreateLevel;
    }
    if (gc->texture.createProxyLevel == NULL) {
        gc->texture.createProxyLevel = __glSSTTexCreateProxyLevel;
    }

    /* XXX Override device dependent values */
    gc->constants.numberOfTextures = 4;
    gc->constants.maxTextureSize = 1 << (gc->constants.maxMipMapLevel - 1);

    /* Allocate memory based on number of textures supported */
    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;

    /*
    ** Init texture object structures.
    ** Normally a texture object has only one textureMachine allocated
    ** with it because it supports only one object.  The default texture
    ** texture object is special in that its textureMachine is an array
    ** of textureMachines, one for each target.
    */

    if (NULL == gc->texture.namesArray) {
        gc->texture.namesArray = __glNamesNewArray(gc, __GL_NAMES_TEXOBJ);
        assert(NULL != gc->texture.namesArray);
    }

    maxMipMapLevel = gc->constants.maxMipMapLevel;

    /* per tex unit state init */
    for( texUnit = 0; texUnit < __GL_MAX_TEX_UNITS; texUnit++ ) {
        gc->state.texture[texUnit].texture = (__GLperTextureState*)
            (*gc->imports.calloc)(gc, (size_t) numTextures,
                                  sizeof(__GLperTextureState));
        gc->texture.texture[texUnit] = (__GLperTextureMachine**)
            (*gc->imports.calloc)(gc, (size_t) numTextures,
                                  sizeof(__GLperTextureMachine*));
        gc->state.texture[texUnit].env = (__GLtextureEnvState*)
            (*gc->imports.calloc)(gc, (size_t) numEnvs,
                                  sizeof(__GLtextureEnvState));
        /*
        ** Set up the dummy texture objects for the default textures. 
        ** Because the default textures are not shared, they should
        ** not be hung off of the namesArray structure.
        */
        gc->texture.defaultTextures[texUnit] = (__GLtextureObject *)(*gc->imports.calloc)
            (gc, numTextures, sizeof(__GLtextureObject));
        assert(NULL != gc->texture.defaultTextures[texUnit]);

        /* allocate the boundTextures array */
        gc->texture.boundTextures[texUnit] = (__GLtextureObject **)(*gc->imports.calloc)
            (gc, numTextures, sizeof(__GLtextureObject *));
        assert(NULL != gc->texture.boundTextures[texUnit]);

        texobj = gc->texture.defaultTextures[texUnit];
        for (i=0; i < numTextures; i++, texobj++) {
            (*gc->texture.initTextureObject)(gc, texobj, 0/*name*/, i/*targetIndex*/);
            assert(texobj->texture.map.texobjs.name == 0);
            /*
            ** The refcount is unused because default textures aren't
            ** shared.
            */
            texobj->refcount = 1;
            /*
            ** Install the default textures into the gc.
            */
            gc->texture.texture[texUnit][i] = &(texobj->texture);
            gc->texture.boundTextures[texUnit][i] = texobj;
            
            /* Allocate memory based on max mipmap level supported */
            texobj->texture.map.level = (__GLmipMapLevel*)
                (*gc->imports.calloc)(gc, (size_t) maxMipMapLevel,
                                      sizeof(__GLmipMapLevel));
        }
    }

    gc->texture.sst.currentTMU = GR_TMU0;
}

/*
** This routine is used to initialize a texture object. 
** Texture objects must be initialized exactly the way the default
** textures are initialized at startup of the library.
** TBD--- it's currently a copy of the code in InitTextureState;
** it should be shared code, if performance considerations allow it.
*/
static
void InitTextureMachine(__GLcontext *gc, GLuint targetIndex, 
                        __GLperTextureMachine *ptm)
{
    GLint level, maxMipMapLevel;

    ptm->map.gc = gc;
    /*
    ** Can't copy the params currently in the gc state.texture params,
    ** because they might not be at init conditions.
    */
    ptm->map.params.sWrapMode = GL_REPEAT;
    ptm->map.params.tWrapMode = GL_REPEAT;
    ptm->map.params.minFilter = GL_NEAREST_MIPMAP_LINEAR;
    ptm->map.params.magFilter = GL_LINEAR;

    switch (targetIndex) {
      case __GL_TEXTURE_INDEX_1D:
        ptm->map.dim = 1;
        ptm->map.createLevel = gc->texture.createLevel;
        break;
      case __GL_TEXTURE_INDEX_2D:
        ptm->map.dim = 2;
        ptm->map.createLevel = gc->texture.createLevel;
        break;
      case __GL_PROXY_TEXTURE_INDEX_1D:
        ptm->map.dim = 1;
        ptm->map.createLevel = gc->texture.createProxyLevel;
        break;
      case __GL_PROXY_TEXTURE_INDEX_2D:
        ptm->map.dim = 2;
        ptm->map.createLevel = gc->texture.createProxyLevel;
        break;
      default:
        break;
    }

    maxMipMapLevel = gc->constants.maxMipMapLevel;

    ptm->map.level = (__GLmipMapLevel*)
            (*gc->imports.calloc)(gc, (size_t) maxMipMapLevel,
                                  sizeof(__GLmipMapLevel));

    /* Init each texture level */
    for (level = 0; level < maxMipMapLevel; level++) {
        ptm->map.level[level].requestedFormat = 1;
    }

#ifdef __GL_SST
    /* XXXshui maybe need to init other things */
    ptm->map.sst.needMask   = 0;
    ptm->map.sst.allocation = __GL_SST_TEXALLOC_NONE;
    ptm->map.sst.cacheMask = 0;

    ptm->map.sst.texUnit[0].allocSize = 0;
    ptm->map.sst.texUnit[1].allocSize = 0;
#endif    
}

/* XXXshui never gets called! */
#ifdef __TACO_NOTDEF__
void __glSSTInitTextureState(__GLcontext *gc)
{
    __GLperTextureState *pts;
    __GLtextureEnvState *tes;
    __GLperTextureMachine **ptm;
    GLint i, level, maxMipMapLevel, numTextures, numEnvs;

    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;
    maxMipMapLevel = gc->constants.maxMipMapLevel;

    gc->state.current.texture[0].w = __glOne;
    gc->state.current.texture[1].w = __glOne;

    /* Init each texture environment state */
    tes = &gc->state.texture.env[0];
    for (i = 0; i < numEnvs; i++, tes++) {
        tes->mode = GL_MODULATE;
    }

    /* Init each textures state */
    pts = &gc->state.texture.texture[0];
    ptm = gc->texture.texture;
    for (i = 0; i < numTextures; i++, pts++, ptm++) {
        /* Init client state */
        pts->params.sWrapMode = GL_REPEAT;
        pts->params.tWrapMode = GL_REPEAT;
        pts->params.minFilter = GL_NEAREST_MIPMAP_LINEAR;
        pts->params.magFilter = GL_LINEAR;

        pts->texobjs.name = 0;
        pts->texobjs.priority = 1.0;

        /* Init machine state */
        (*ptm)->map.gc = gc;
        (*ptm)->map.params = pts->params;
        switch (i) {
          case __GL_TEXTURE_INDEX_1D:
            (*ptm)->map.dim = 1;
            (*ptm)->map.createLevel = gc->texture.createLevel;
            break;
          case __GL_TEXTURE_INDEX_2D:
            (*ptm)->map.dim = 2;
            (*ptm)->map.createLevel = gc->texture.createLevel;
            break;
          case __GL_PROXY_TEXTURE_INDEX_1D:
            (*ptm)->map.dim = 1;
            (*ptm)->map.createLevel = gc->texture.createProxyLevel;
            break;
          case __GL_PROXY_TEXTURE_INDEX_2D:
            (*ptm)->map.dim = 2;
            (*ptm)->map.createLevel = gc->texture.createProxyLevel;
            break;
          default:
            break;
        }
        /* Init each texture level */
        for (level = 0; level < maxMipMapLevel; level++) {
            (*ptm)->map.level[level].requestedFormat = 1;
        }
    }

    /* Init rest of texture state */
    gc->state.texture.s.mode = GL_EYE_LINEAR;
    gc->state.texture.s.eyePlaneEquation.x = __glOne;
    gc->state.texture.s.objectPlaneEquation.x = __glOne;
    gc->state.texture.t.mode = GL_EYE_LINEAR;
    gc->state.texture.t.eyePlaneEquation.y = __glOne;
    gc->state.texture.t.objectPlaneEquation.y = __glOne;
    gc->state.texture.r.mode = GL_EYE_LINEAR;
    gc->state.texture.q.mode = GL_EYE_LINEAR;

    gc->state.texture.scale[0] = __glOne;
    gc->state.texture.scale[1] = __glOne;
    gc->state.texture.scale[2] = __glOne;
    gc->state.texture.scale[3] = __glOne;

#ifndef __GL_SST
    __glInitTextureEnvCache(gc);
#endif
}
#endif

void __glSSTFreeTextureState(__GLcontext *gc)
{
    GLint i, level, numTextures, maxLevel, texUnit;

    /*
    ** Clean up all allocs associated with texture objects.
    */

    for( texUnit = 0; texUnit < __GL_MAX_TEX_UNITS; texUnit++ ) {
        __GLperTextureMachine **ptm = gc->texture.texture[texUnit];

        gc->texture.currentTexUnit = texUnit;

        maxLevel = gc->constants.maxMipMapLevel;
        numTextures = gc->constants.numberOfTextures;
        for (i = 0; i < numTextures; i++, ptm++) {
            /* Unbind all non-default textures. */
            __glSSTBindTexture(gc, i, 0, GL_TRUE);
            /* free levels for default textures */
            for (level = 0; level < maxLevel; level++) {
                if (NULL == (*ptm)->map.level[level].buffer) continue;
                assert((*ptm)->map.texobjs.name == 0);
                (*gc->imports.free)(gc, (*ptm)->map.level[level].buffer);
            }
            (*gc->imports.free)(gc, (*ptm)->map.level);
        }
    }

    gc->texture.currentTexUnit = 0;

    gc->texture.namesArray->refcount--;
    if (gc->texture.namesArray->refcount == 0) {
        __glNamesFreeArray(gc, gc->texture.namesArray);
    }
    gc->texture.namesArray = NULL;

    for( texUnit = 0; texUnit < __GL_MAX_TEX_UNITS; texUnit++ ) {
        (*gc->imports.free)(gc, gc->texture.texture[texUnit]);
        (*gc->imports.free)(gc, gc->texture.boundTextures[texUnit]);
        (*gc->imports.free)(gc, gc->texture.defaultTextures[texUnit]);
        (*gc->imports.free)(gc, gc->state.texture[texUnit].texture);
        (*gc->imports.free)(gc, gc->state.texture[texUnit].env);

        gc->texture.texture[texUnit] = NULL;
        gc->texture.boundTextures[texUnit] = NULL;
        gc->texture.defaultTextures[texUnit] = NULL;
        gc->state.texture[texUnit].texture = NULL;
        gc->state.texture[texUnit].env = NULL;
    }

#ifndef __GL_SST
    __glFreeTextureEnvCache(gc);
#endif
}

/************************************************************************/

void __glSSTFreeTextureMemory(__GLcontext *gc, __GLtexture *tex);
void __glSSTAllocateTextureMemory(__GLcontext *gc, __GLtexture *tex, int len);


/* ================= RETRO3DFX FILTER FIX (0.2.0) ========================
   Honor the GL min/mag filters in hardware.  The ICD set
   grTexFilterMode(GR_TMU0, POINT_SAMPLED, BILINEAR) once at context init
   (sst_export.c) and NEVER updated it from glTexParameter, so every
   MINIFIED texture was point-sampled: sliced/garbled scaled text (Q3 menu
   proportional font, CS HUD text) and software-renderer-like pixel shimmer
   on distant/minified surfaces.  Called at every grTexSource bind since the
   hw filter is per-TMU state while GL's is per-texture. */
static void __glSSTApplyGrFilter(GrChipID_t tmu, __GLtexture *tex)
{
    GrTextureFilterMode_t minf, magf;
    switch (tex->params.minFilter) {
    case GL_NEAREST:
    case GL_NEAREST_MIPMAP_NEAREST:
    case GL_NEAREST_MIPMAP_LINEAR:
        minf = GR_TEXTUREFILTER_POINT_SAMPLED; break;
    default:                       /* GL_LINEAR / GL_LINEAR_MIPMAP_* */
        minf = GR_TEXTUREFILTER_BILINEAR; break;
    }
    magf = (tex->params.magFilter == GL_NEAREST)
         ? GR_TEXTUREFILTER_POINT_SAMPLED : GR_TEXTUREFILTER_BILINEAR;
    /* single-texture lane: cover BOTH TMUs (binds land on tmu=1 while the
       init default set only TMU0 - the sampling TMU must get the filter).
       Refine to per-unit when ARB multitexture ships. */
    OGLLOGV( "FILT@456 tmu/min/mag= %d %d %d", (int)(GR_TMU0), (int)(minf), (int)(magf) );
    grTexFilterMode(GR_TMU0, minf, magf);
    OGLLOGV( "FILT@457 tmu/min/mag= %d %d %d", (int)(GR_TMU1), (int)(minf), (int)(magf) );
    grTexFilterMode(GR_TMU1, minf, magf);
    (void)tmu;
}
/* =============== end RETRO3DFX FILTER FIX ============================= */

/* ============ RETRO3DFX MINIFICATION FIX (text-garble root cause) ==========
** A GL_LINEAR (non-mipmapped) 2D texture drawn MINIFIED aliases hard on the
** VSA-100 (bilinear only taps 2x2 texels) -> thin soft-alpha glyph strokes drop
** columns = the Q3 menu / CS HUD "garbled text". Fix: build a box-filtered mip
** chain on the fly and let the hardware LOD-dither-blend between levels
** (GR_MIPMAP_NEAREST_DITHER) so minified fonts are properly downfiltered.
** Handles the two 16-bit texel formats fonts use (RGB_565, ARGB_4444). Gated
** on env RETRO3DFX_FONTMIP for A/B; harmless when off. */
static FxU16 __r3d_avg565(FxU16 a, FxU16 b, FxU16 c, FxU16 d)
{
    int r = ((a>>11)&31)+((b>>11)&31)+((c>>11)&31)+((d>>11)&31);
    int g = ((a>>5)&63)+((b>>5)&63)+((c>>5)&63)+((d>>5)&63);
    int bl= (a&31)+(b&31)+(c&31)+(d&31);
    return (FxU16)(((r>>2)<<11)|((g>>2)<<5)|(bl>>2));
}
static FxU16 __r3d_avg4444(FxU16 a, FxU16 b, FxU16 c, FxU16 d)
{
    int A=((a>>12)&15)+((b>>12)&15)+((c>>12)&15)+((d>>12)&15);
    int R=((a>>8)&15)+((b>>8)&15)+((c>>8)&15)+((d>>8)&15);
    int G=((a>>4)&15)+((b>>4)&15)+((c>>4)&15)+((d>>4)&15);
    int B=(a&15)+(b&15)+(c&15)+(d&15);
    return (FxU16)(((A>>2)<<12)|((R>>2)<<8)|((G>>2)<<4)|(B>>2));
}
static int __r3d_wantFontMip(void)
{
    static int c = -1;
    if (c < 0) c = getenv("RETRO3DFX_FONTMIP") ? 1 : 0;
    return c;
}
/* Build a full box-filtered chain (large->small, concatenated) from lp->buffer
** into scratch; return total FxU16 count, or 0 if unsupported. */
static int __r3d_buildMipChain(__GLtexture *tex, FxU16 *scratch, int maxwords)
{
    __GLmipMapLevel *lp = &tex->level[0];
    int fmt = lp->internalFormat;
    int w = lp->width, h = lp->height;
    FxU16 *src, *dst;
    int total, sw, sh, dw, dh, x, y;
    if (fmt != __GL_SST_RGB_565 && fmt != __GL_SST_ARGB_4444) return 0;
    if (w < 2 || h < 2) return 0;
    /* LOD0 verbatim */
    total = w * h;
    if (total > maxwords) return 0;
    for (x = 0; x < total; x++) scratch[x] = ((FxU16 *)lp->buffer)[x];
    src = scratch; sw = w; sh = h;
    dst = scratch + total;
    while (sw > 1 || sh > 1) {
        dw = sw > 1 ? sw >> 1 : 1;
        dh = sh > 1 ? sh >> 1 : 1;
        if (total + dw*dh > maxwords) return 0;
        for (y = 0; y < dh; y++) {
            int sy0 = (sh > 1) ? y*2 : 0, sy1 = (sh > 1) ? y*2+1 : 0;
            for (x = 0; x < dw; x++) {
                int sx0 = (sw > 1) ? x*2 : 0, sx1 = (sw > 1) ? x*2+1 : 0;
                FxU16 a = src[sy0*sw+sx0], b = src[sy0*sw+sx1];
                FxU16 c = src[sy1*sw+sx0], e = src[sy1*sw+sx1];
                dst[y*dw+x] = (fmt == __GL_SST_RGB_565)
                    ? __r3d_avg565(a,b,c,e) : __r3d_avg4444(a,b,c,e);
            }
        }
        total += dw*dh;
        src = dst; dst = scratch + total; sw = dw; sh = dh;
    }
    return total;
}
/* ============ end RETRO3DFX MINIFICATION FIX ============================= */

static void applyTexParameter(__GLcontext *gc, __GLtexture *tex)
{
    GrTexInfo grTex;
    GLint n, memsize;
    GLuint txu;

    txu = gc->texture.currentTexUnit;

    /* Begin sst specific stuff. */
    if (tex->params.minFilter != GL_NEAREST && tex->params.minFilter != GL_LINEAR) {
        if (tex->level[0].width && tex->level[0].height) { /* XXXshui check a bit instead */
            grTex.format      = tex->sst.grformat;
            grTex.smallLodLog2    = GR_LOD_LOG2_1;
            grTex.largeLodLog2    = tex->sst.lod;
            grTex.aspectRatioLog2 = tex->sst.aspect;
            grTex.data        = tex->level[0].buffer;
            if (tex->sst.allocation == __GL_SST_TEXALLOC_BASE) {
                __glSSTFreeTextureMemory(gc, tex);
                tex->sst.allocation = __GL_SST_TEXALLOC_NONE;
                tex->sst.texUnit[txu].allocSize = 0;
                tex->sst.cacheMask = 0;
                /* XXX taco hack - push texture object out of other texture cache*/
                if ( gc->grNTexelFx == 2 ) {
                    int ntxu = !txu;
                    if ( tex->sst.texUnit[ntxu].cache ) {
                        freenode( gc, tex->sst.texUnit[ntxu].cache );
                        tex->sst.texUnit[ntxu].cache = NULL;
                    }
                }
            }
            if (tex->sst.allocation != __GL_SST_TEXALLOC_STACK) {
                memsize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &grTex);
                __glSSTAllocateTextureMemory(gc, tex, memsize);
                tex->sst.allocation = __GL_SST_TEXALLOC_STACK;
                tex->sst.texUnit[txu].allocSize = memsize;
                
                /* load all suitable levels */
                tex->sst.cacheMask = 0;
                for (n=0; n < tex->sst.numLevels; n++) {
                    if ((tex->level[n].width == tex->sst.allocWidth[n]) &&
                        (tex->level[n].height == tex->sst.allocHeight[n]) &&
                        (tex->level[n].requestedFormat == tex->level[0].requestedFormat)) {
                        tex->sst.cacheMask |= (1L << n);
                        grTexDownloadMipMapLevel(gc->texture.sst.currentTMU,
                                                 (FxU32)tex->sst.texUnit[txu].cache->addr,
                                                 tex->sst.lod - n, tex->sst.lod, 
                                                 tex->sst.aspect, 
                                                 tex->sst.grformat, 
                                                 GR_MIPMAPLEVELMASK_BOTH, 
                                                 tex->level[n].buffer);
                    }
                }
            }
            if (tex->sst.cacheMask == tex->sst.needMask) {
                grTexSource(gc->texture.sst.currentTMU,
                            (FxU32)tex->sst.texUnit[txu].cache->addr,
                            GR_MIPMAPLEVELMASK_BOTH,
                            &grTex);
                __glSSTApplyGrFilter(gc->texture.sst.currentTMU, tex);
                gc->validateTexture = 1;
            } else {
                gc->validateTexture = 1;
            }
        } else {
            /* do nothing */
        }
    } else {
        if (tex->level[0].width && tex->level[0].height) {
            /* RETRO3DFX MINIFICATION FIX: for a GL_LINEAR non-mipmapped 2D
            ** texture, synthesize a box-filtered mip chain + LOD-dither blend so
            ** minified soft-alpha fonts stop aliasing (the text-garble fix). */
            static FxU16 __r3dMipBuf[256*256 + 128*128 + 64*64 + 8192];
            int __r3dMipWords = 0;
            if (__r3d_wantFontMip() && tex->params.minFilter == GL_LINEAR &&
                tex->level[0].width >= 4 && tex->level[0].height >= 4) {
                __r3dMipWords = __r3d_buildMipChain(tex, __r3dMipBuf,
                                    sizeof(__r3dMipBuf)/sizeof(FxU16));
            }
            OGLLOG("APPLYTEX(LINpath) %dx%d minF=0x%x ifmt=0x%x mipWords=%d",
                   (int)tex->level[0].width,(int)tex->level[0].height,
                   (unsigned)tex->params.minFilter,(unsigned)tex->level[0].internalFormat,
                   __r3dMipWords);
            grTex.format      = tex->sst.grformat;
            if (__r3dMipWords) {
                grTex.smallLodLog2 = GR_LOD_LOG2_1;
                grTex.largeLodLog2 = tex->sst.lod;
                grTex.data         = __r3dMipBuf;
            } else {
            grTex.smallLodLog2    = tex->sst.lod;
            grTex.largeLodLog2    = tex->sst.lod;
            grTex.data        = tex->level[0].buffer;
            }
            grTex.aspectRatioLog2 = tex->sst.aspect;
            if (tex->sst.allocation == __GL_SST_TEXALLOC_STACK) {
                __glSSTFreeTextureMemory(gc, tex);
                tex->sst.allocation = __GL_SST_TEXALLOC_NONE;
                tex->sst.texUnit[txu].allocSize = 0;
                tex->sst.cacheMask = 0;
                /* XXX taco hack - push texture object out of other texture cache*/
                if ( gc->grNTexelFx == 2 ) {
                    int ntxu = !txu;
                    if ( tex->sst.texUnit[ntxu].cache ) {
                        freenode( gc, tex->sst.texUnit[ntxu].cache );
                        tex->sst.texUnit[ntxu].cache = NULL;
                    }
                }
            }
            if (tex->sst.allocation != __GL_SST_TEXALLOC_BASE) {
                memsize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &grTex);
                __glSSTAllocateTextureMemory(gc, tex, memsize);

                tex->sst.allocation = __GL_SST_TEXALLOC_BASE;
                tex->sst.texUnit[txu].allocSize = memsize;
                if (__r3dMipWords) {
                    /* full box-filtered chain (large->small, concatenated) */
                    grTexDownloadMipMap(gc->texture.sst.currentTMU,
                                        (FxU32)tex->sst.texUnit[txu].cache->addr,
                                        GR_MIPMAPLEVELMASK_BOTH, &grTex);
                } else {
                grTexDownloadMipMapLevel(gc->texture.sst.currentTMU,
                                         (FxU32)tex->sst.texUnit[txu].cache->addr,
                                         tex->sst.lod,
                                         tex->sst.lod,
                                         tex->sst.aspect,
                                         tex->sst.grformat,
                                         GR_MIPMAPLEVELMASK_BOTH,
                                         tex->level[0].buffer);
                }
            }
            grTexSource(gc->texture.sst.currentTMU,
                        (FxU32)tex->sst.texUnit[txu].cache->addr,
                        GR_MIPMAPLEVELMASK_BOTH,
                        &grTex);
                __glSSTApplyGrFilter(gc->texture.sst.currentTMU, tex);
            if (__r3dMipWords) {
                /* LOD-dither blend between levels = trilinear-ish minification */
                grTexMipMapMode(gc->texture.sst.currentTMU,
                                GR_MIPMAP_NEAREST_DITHER, FXFALSE);
            }
            gc->validateTexture = 1;
        } else {
            /* do nothing */
        }
    }
}

void APIENTRY __glsstim_TexParameterfv(GLenum target, GLenum pname, const GLfloat pv[])
{
    unsigned long tmp;
    int txu;
    __GLtextureParamState *pts;
    GLenum e;
    __GLtexture *tex;
    __GLtextureObject *pto;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();


    txu = gc->texture.currentTexUnit;

    pts = __glLookUpTextureParams(gc, target, txu );
    tex = __glLookUpTexture(gc, target, txu );

    if (!pts) {
      bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    pto = __glLookUpTextureObject(gc, target, txu );
#if 0
    /* taco - this doesn't work with multitexture */
    if (pto->refcount > 2) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
#endif

    switch (pname) {
      case GL_TEXTURE_WRAP_S:
        switch (e = (GLenum) pv[0]) {
          case GL_REPEAT:
            tex->sst.clamps = GR_TEXTURECLAMP_WRAP;
            break;
          case GL_CLAMP:
            tex->sst.clamps = GR_TEXTURECLAMP_CLAMP;
            break;
          default:
            goto bad_enum;
        }
        tmp = ( ( tex->sst.clamps << 8 ) | ( tex->sst.clampt ) );
        if ( gc->texture.hwSTWrap[txu] != tmp ) {
            grTexClampMode(gc->texture.sst.texUnits[txu], tex->sst.clamps, tex->sst.clampt);
            gc->texture.hwSTWrap[txu] = tmp;
        }
        tex->params.sWrapMode = pts->sWrapMode = e;
        break;
      case GL_TEXTURE_WRAP_T:
        switch (e = (GLenum) pv[0]) {
          case GL_REPEAT:
            tex->sst.clampt = GR_TEXTURECLAMP_WRAP;
            break;
          case GL_CLAMP:
            tex->sst.clampt = GR_TEXTURECLAMP_CLAMP;
            break;
          default:
            goto bad_enum;
        }
        tmp = ( ( tex->sst.clamps << 8 ) | ( tex->sst.clampt ) );
        if ( gc->texture.hwSTWrap[txu] != tmp ) {
            grTexClampMode(gc->texture.sst.texUnits[txu], tex->sst.clamps, tex->sst.clampt);
            gc->texture.hwSTWrap[txu] = tmp;
        }
        tex->params.tWrapMode = pts->tWrapMode = e;
        break;
      case GL_TEXTURE_MIN_FILTER:
        switch (e = (GLenum) pv[0]) {
          case GL_NEAREST:
            tex->sst.min = GR_TEXTUREFILTER_POINT_SAMPLED;
            tex->sst.mip = GR_MIPMAP_DISABLE;
            break;
          case GL_LINEAR:
            tex->sst.min = GR_TEXTUREFILTER_BILINEAR;
            tex->sst.mip = GR_MIPMAP_DISABLE;
            break;
          case GL_NEAREST_MIPMAP_NEAREST:
            tex->sst.min = GR_TEXTUREFILTER_POINT_SAMPLED;
            tex->sst.mip = GR_MIPMAP_NEAREST;
            break;
          case GL_LINEAR_MIPMAP_NEAREST:
            tex->sst.min = GR_TEXTUREFILTER_BILINEAR;
            tex->sst.mip = GR_MIPMAP_NEAREST; 
            break;
          case GL_NEAREST_MIPMAP_LINEAR: /* XXXshui unsupported by sst */
            tex->sst.min = GR_TEXTUREFILTER_POINT_SAMPLED;
            tex->sst.mip = GR_MIPMAP_NEAREST; 
            break;
          case GL_LINEAR_MIPMAP_LINEAR: /* XXXshui unsupported by sst */
            tex->sst.min = GR_TEXTUREFILTER_BILINEAR;
            tex->sst.mip = GR_MIPMAP_NEAREST; 
            break;
          default:
            goto bad_enum;
        }

        /* RETRO3DFX diagnostic (CS green-world): file marker C:\icd_nomip.on
        ** forces GR_MIPMAP_DISABLE so the HW samples only the base LOD.  If
        ** the world then renders tan, the corruption is in the mip chain
        ** (sub-level address/alloc); if still green, the base LOD itself. */
        { static int nomip = -1;
          if ( nomip < 0 ) {
              HANDLE g = CreateFileA( "C:\\icd_nomip.on", GENERIC_READ,
                                      FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 );
              nomip = ( g != INVALID_HANDLE_VALUE ) ? 1 : 0;
              if ( nomip ) CloseHandle( g );
          }
          if ( nomip ) tex->sst.mip = GR_MIPMAP_DISABLE;
        }

        tmp = ( ( tex->sst.min << 8 ) | ( tex->sst.mag ) );
        if ( gc->texture.hwMinMag[txu] != tmp ) {
            OGLLOGV( "FILT@676 tmu/min/mag= %d %d %d", (int)(gc->texture.sst.texUnits[txu]), (int)(tex->sst.min), (int)(tex->sst.mag) );
            grTexFilterMode(gc->texture.sst.texUnits[txu], tex->sst.min, tex->sst.mag);
            gc->texture.hwMinMag[txu] = tmp;
        }

        tmp = tex->sst.mip;
        if ( gc->texture.hwMMMode[txu] != tmp ) {
            grTexMipMapMode(gc->texture.sst.texUnits[txu], tex->sst.mip, FXFALSE);
            gc->texture.hwMMMode[txu] = tmp;
        }

        tex->params.minFilter = pts->minFilter = e;
        break;
      case GL_TEXTURE_MAG_FILTER:
        switch (e = (GLenum) pv[0]) {
          case GL_NEAREST:
            tex->sst.mag = GR_TEXTUREFILTER_POINT_SAMPLED;
            break;
          case GL_LINEAR:
            tex->sst.mag = GR_TEXTUREFILTER_BILINEAR;
            break;
          default:
            goto bad_enum;
        }
        tmp = ( ( tex->sst.min << 8 ) | ( tex->sst.mag ) );
        if ( gc->texture.hwMinMag[txu] != tmp ) {
            OGLLOGV( "FILT@701 tmu/min/mag= %d %d %d", (int)(gc->texture.sst.texUnits[txu]), (int)(tex->sst.min), (int)(tex->sst.mag) );
            grTexFilterMode(gc->texture.sst.texUnits[txu], tex->sst.min, tex->sst.mag); 
            gc->texture.hwMinMag[txu] = tmp;
        }
        tex->params.magFilter = pts->magFilter = e;
        break;
      case GL_TEXTURE_BORDER_COLOR:
        __glClampColorf(gc, &pts->borderColor, pv);
        tex->params.borderColor = pts->borderColor;
        break;
      
      case GL_TEXTURE_PRIORITY:
        {
            __GLtextureObjectState *ptos;
            ptos = __glLookUpTextureTexobjs(gc, target, txu );
            ptos->priority = Clampf(pv[0], __glZero, __glOne);
        }
        break;

      default:
        goto bad_enum;
    }
    applyTexParameter(gc, tex);
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_TexParameterf(GLenum target, GLenum pname, GLfloat f)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
        __glsstim_TexParameterfv(target, pname, &f);
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

void APIENTRY __glsstim_TexParameteriv(GLenum target, GLenum pname, const GLint pv[])
{
    __GLtextureParamState *pts;
    GLenum e;
    int txu;
    unsigned long tmp;
    __GLtexture *tex;
    __GLtextureObject *pto;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();


    txu = gc->texture.currentTexUnit;

    pts = __glLookUpTextureParams(gc, target, txu );
    tex = __glLookUpTexture(gc, target, txu );

    if (!pts) {
      bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    pto = __glLookUpTextureObject( gc, target, txu );
#if 0
    /* xxxTaxo - this doesn't work with multitexture */
    if (pto->refcount > 2) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
#endif
    
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
        switch (e = (GLenum) pv[0]) {
          case GL_REPEAT:
            tex->sst.clamps = GR_TEXTURECLAMP_WRAP;
            break;
          case GL_CLAMP:
            tex->sst.clamps = GR_TEXTURECLAMP_CLAMP;
            break;
          default:
            goto bad_enum;
        }
        tmp = ( ( tex->sst.clamps << 8 ) | ( tex->sst.clampt ) );
        if ( gc->texture.hwSTWrap[txu] != tmp ) {
            grTexClampMode(gc->texture.sst.texUnits[txu], tex->sst.clamps, tex->sst.clampt);
            gc->texture.hwSTWrap[txu] = tmp;
        }
        tex->params.sWrapMode = pts->sWrapMode = e;
        break;
      case GL_TEXTURE_WRAP_T:
        switch (e = (GLenum) pv[0]) {
          case GL_REPEAT:
            tex->sst.clampt = GR_TEXTURECLAMP_WRAP;
            break;
          case GL_CLAMP:
            tex->sst.clampt = GR_TEXTURECLAMP_CLAMP;
            break;
          default:
            goto bad_enum;
        }
        tmp = ( ( tex->sst.clamps << 8 ) | ( tex->sst.clampt ) );
        if ( gc->texture.hwSTWrap[txu] != tmp ) {
            grTexClampMode(gc->texture.sst.texUnits[txu], tex->sst.clamps, tex->sst.clampt);
            gc->texture.hwSTWrap[txu] = tmp;
        }
        tex->params.tWrapMode = pts->tWrapMode = e;
        break;
      case GL_TEXTURE_MIN_FILTER:
        switch (e = (GLenum) pv[0]) {
          case GL_NEAREST:
            tex->sst.min = GR_TEXTUREFILTER_POINT_SAMPLED;
            tex->sst.mip = GR_MIPMAP_DISABLE;
            break;
          case GL_LINEAR:
            tex->sst.min = GR_TEXTUREFILTER_BILINEAR;
            tex->sst.mip = GR_MIPMAP_DISABLE;
            break;
          case GL_NEAREST_MIPMAP_NEAREST:
            tex->sst.min = GR_TEXTUREFILTER_POINT_SAMPLED;
            tex->sst.mip = GR_MIPMAP_NEAREST;
            break;
          case GL_LINEAR_MIPMAP_NEAREST:
            tex->sst.min = GR_TEXTUREFILTER_BILINEAR;
            tex->sst.mip = GR_MIPMAP_NEAREST; 
            break;
          case GL_NEAREST_MIPMAP_LINEAR: /* XXXshui unsupported by sst: use sw */
            tex->sst.min = GR_TEXTUREFILTER_POINT_SAMPLED;
            tex->sst.mip = GR_MIPMAP_NEAREST; 
            break;
          case GL_LINEAR_MIPMAP_LINEAR: /* XXXshui unsupported by sst: use sw */
            tex->sst.min = GR_TEXTUREFILTER_BILINEAR;
            tex->sst.mip = GR_MIPMAP_NEAREST; 
            break;
          default:
            goto bad_enum;
        }
        tmp = ( ( tex->sst.min << 8 ) | ( tex->sst.mag ) );
        if ( gc->texture.hwMinMag[txu] != tmp ) {
            OGLLOGV( "FILT@842 tmu/min/mag= %d %d %d", (int)(gc->texture.sst.texUnits[txu]), (int)(tex->sst.min), (int)(tex->sst.mag) );
            grTexFilterMode(gc->texture.sst.texUnits[txu], tex->sst.min, tex->sst.mag); 
            gc->texture.hwMinMag[txu] = tmp;
        }

        tmp = tex->sst.mip;
        if ( gc->texture.hwMMMode[txu] != tmp ) {
            grTexMipMapMode(gc->texture.sst.texUnits[txu], tex->sst.mip, FXFALSE);
            gc->texture.hwMMMode[txu] = tmp;
        }
        tex->params.minFilter = pts->minFilter = e;
        break;
      case GL_TEXTURE_MAG_FILTER:
        switch (e = (GLenum) pv[0]) {
          case GL_NEAREST:
            tex->sst.mag = GR_TEXTUREFILTER_POINT_SAMPLED;
            break;
          case GL_LINEAR:
            tex->sst.mag = GR_TEXTUREFILTER_BILINEAR;
            break;
          default:
            goto bad_enum;
        }
        tmp = ( ( tex->sst.min << 8 ) | ( tex->sst.mag ) );
        if ( gc->texture.hwMinMag[txu] != tmp ) {
            OGLLOGV( "FILT@866 tmu/min/mag= %d %d %d", (int)(gc->texture.sst.texUnits[txu]), (int)(tex->sst.min), (int)(tex->sst.mag) );
            grTexFilterMode(gc->texture.sst.texUnits[txu], tex->sst.min, tex->sst.mag); 
            gc->texture.hwMinMag[txu] = tmp;
        }
        tex->params.magFilter = pts->magFilter = e;
        break;
      case GL_TEXTURE_BORDER_COLOR:
        __glClampColori(gc, &pts->borderColor, pv);
        tex->params.borderColor = pts->borderColor;
        break;
      case GL_TEXTURE_PRIORITY:
        {
            __GLfloat priority;
            __GLtextureObjectState *ptos;

            ptos = __glLookUpTextureTexobjs( gc, target, txu );
            priority = __GL_I_TO_FLOAT(pv[0]);
            ptos->priority = Clampf(priority, __glZero, __glOne);
        }
        break;
      default:
        goto bad_enum;
    }
    applyTexParameter(gc, tex);
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_TexParameteri(GLenum target, GLenum pname, GLint i)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
        __glsstim_TexParameteriv(target, pname, &i);
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

/************************************************************************/

void __glSSTEnableTexturing(__GLcontext *gc)
{
    __GLperTextureMachine *ptm;
    __GLtexture *tex;
    int enableState, txu, minfilter, enable, cachemask, maxTxu;

    enable = 0;
    maxTxu = gc->grNTexelFx;
    for( txu = 0; txu < maxTxu; txu++ ) {
        /* TEXEL-CENTER FIX: default bias 0 (no texture bound) */
        __glSSTHalfTexelS[txu] = 0.0f;
        __glSSTHalfTexelT[txu] = 0.0f;
        enableState = gc->state.enables.texture[txu];
        if ( enableState & __GL_TEXTURE_2D_ENABLE ) {
            ptm = gc->texture.texture[txu][__GL_TEXTURE_INDEX_2D];
            if ( ptm ) {
                tex = &ptm->map;
                /* TEXEL-CENTER FIX (root cause of the 2D text garble, proven
                ** by vertex-level tracing 2026-07-19): the VSA-100 samples
                ** texel centers at INTEGER texel coords, OpenGL at .5 --
                ** every textured draw was half a texel off in s and t.
                ** Bias the normalized coords by -0.5 texel (linear, so
                ** pre-scale bias == post-scale -0.5 exactly). Applied in ALL
                ** texcoord copy paths (S_TAPI + every S_VARRAY variant). */
                /* TEXEL-CENTER CALIBRATION RESULT (gltest, 2026-07-19): the
                ** VSA-100 + our stack sample GL-conformant .5-centers with NO
                ** bias -- pre-fix c03 [141,111] == exact GL math, and a -0.5
                ** bias breaks 1:1 integer draws to 50/50 gray (c02=128).
                ** Biases stay 0; the entry-site adds are no-ops kept for
                ** future per-texture calibration. */
                minfilter = tex->params.minFilter;
                cachemask = tex->sst.cacheMask;
                if ( minfilter == GL_LINEAR || minfilter == GL_NEAREST ) {
                    /* not mipmapping, verify presence of lvl 1 */
                    if ( tex->sst.texUnit[txu].cache && tex->level[0].width && tex->level[0].height ) {
                        enable = 1;
                    }
                } else if ( cachemask != 0 ) {
                    /* mipmapping, verify presence of all mipmap levels */
                    if ( !(tex->sst.needMask & ~cachemask ) ) {
                        enable = 1;
                    }
                }       
            }
        } 
    }

    if ( enable ) {
        __glSSTLoadCombineFunction(gc);
#       if __GL_SST_GLIDE_VTX
        gc->texture.sst.w = tex->level[0].width2f;
        gc->texture.sst.h = tex->level[0].height2f;
#       endif
    } else {
        __glSSTDisableTexturing(gc);
    }
}

void __glSSTDisableTexturing(__GLcontext *gc)
{
    GLuint enableState;
    GLuint    texUnit;

    texUnit     = gc->texture.currentTexUnit;
    enableState = gc->state.enables.texture[texUnit];

    /* temporarily push texture unit disabled state */
    gc->state.enables.texture[texUnit] &= 
        ~( __GL_TEXTURE_2D_ENABLE | __GL_TEXTURE_1D_ENABLE );

    /* assert correct acu/ccu/tcu state */
    __glSSTLoadCombineFunction( gc );

    gc->state.enables.texture[texUnit] = enableState;
}

#define MAKE_CUWORD( A, B, C, D ) \
    ( ( A ) << 24 ) |             \
    ( ( B ) << 16 ) |             \
    ( ( C ) << 8  ) |             \
    ( ( D ) << 0  );

#define USE_CUWORD( X ) \
    ( ( ( unsigned long ) ( X ) ) >> 24 ) & 0xff, \
    ( ( ( unsigned long ) ( X ) ) >> 16 ) & 0xff, \
    ( ( ( unsigned long ) ( X ) ) >> 8  ) & 0xff, \
    ( ( ( unsigned long ) ( X ) ) >> 0  ) & 0xff

static unsigned long s_ACWord  = ~0;
static unsigned long s_CCWord  = ~0;
static unsigned long s_TC0Word = ~0;
static unsigned long s_TC1Word = ~0;

/* QUALITY FIX menu-text: definition of the per-unit normalized texel-center
** bias declared in texture.h.  Set by __glSSTEnableTexturing below. */
__GLfloat __glSSTHalfTexelS[__GL_MAX_TEX_UNITS] = { 0 };
__GLfloat __glSSTHalfTexelT[__GL_MAX_TEX_UNITS] = { 0 };

/* OPT 0.1.4 overbright: Quake3-style world lighting draws base*lightmap in
** TWO passes at baseline, and the lightmap pass's framebuffer blend
** (GL_DST_COLOR,GL_SRC_COLOR) doubles the product -- that is how Q3
** implements r_overBrightBits=1 (2x overbright).  Our single-pass
** GL_ARB_multitexture collapse computes iterated*T0*T1 with no doubling,
** which renders uniformly ~50% too dark.  Fix: on hardware with the Napalm
** COMBINE extension (Voodoo4/5), program the color combine through
** grColorCombineExt with an output shift of 1 (2x) for the two-TMU
** MODULATE(base) x MODULATE(lightmap) path only.  grColorCombineExt is not
** a static glide3x export -- it must be fetched via grGetProcAddress.
** Single-texture and REPLACE paths keep the legacy (unscaled) combine. */
typedef void ( FX_CALL *__glSSTPfnCombineExt )( FxU32  a, FxU32 a_mode,
                                                FxU32  b, FxU32 b_mode,
                                                FxU32  c, FxBool c_invert,
                                                FxU32  d, FxBool d_invert,
                                                FxU32  shift, FxBool invert );
static __glSSTPfnCombineExt s_pfnColorCombineExt = 0;
static __glSSTPfnCombineExt s_pfnAlphaCombineExt = 0;
static int s_combineExtProbed = 0;
static int s_combineExtActive = 0;

/* crash-robust append logger to C:\3dfxogl.log (defined in WGL/WGLCMDS.C) */
extern void OGLLOG( const char *fmt, ... );

/* OPT 0.1.5 overbright (vertex-double mechanism): on-hardware logging on the
** live Voodoo5 PROVED that grColorCombineExt's output-shift=1 does NOT double
** the color combine result on Napalm (branch engaged, pfn resolved, texenvs
** correct -- render stayed half-bright), so the ext-shift 2x is a dead end.
** New mechanism: when the two-TMU MODULATE(base) x MODULATE(lightmap) world
** path is active, the render procs (SST/sst_pgmode.c) DOUBLE the iterated
** vertex RGB (clamped to 255) before building the GrVertex, and the combine
** stays LEGACY 1x (iterated * T0*T1).  Q3 world vertex color is
** identityLight = 0.5 with r_overBrightBits=1, so 2*0.5=1.0 reproduces the
** free 2x the 2-pass (GL_DST_COLOR,GL_SRC_COLOR) lightmap blend provides.
** This flag is the gate, published by __glSSTLoadCombineFunction and read by
** the triangle fill procs. */
int __glSSTOverbright2xVtx = 0;

/* OPT 0.1.4b overbright gate hardening: gate ONLY on grGetProcAddress
** resolving both combine-ext entry points -- that is the real capability
** test.  The previous ' COMBINE ' GR_EXTENSION substring requirement is
** DROPPED: our Napalm glide3x resolves grColorCombineExt from a static
** FX_GLIDE_NAPALM table regardless of what grGetString(GR_EXTENSION)
** advertises, so the string check could (and on the live Voodoo5 did)
** veto a perfectly working extension -> no 2x -> half-bright world. */
static void __glSSTProbeCombineExt( void ) {
    const char *ext;
    char extbuf[400]; /* OGLLOG fmt buffer is 512; keep '%s' bounded */
    if ( s_combineExtProbed ) {
        return;
    }
    s_combineExtProbed = 1;
    ext = grGetString( GR_EXTENSION );
    s_pfnColorCombineExt = ( __glSSTPfnCombineExt )
        grGetProcAddress( "grColorCombineExt" );
    s_pfnAlphaCombineExt = ( __glSSTPfnCombineExt )
        grGetProcAddress( "grAlphaCombineExt" );
    extbuf[0] = '\0';
    if ( ext ) {
        lstrcpynA( extbuf, ext, sizeof( extbuf ) );
    }
    OGLLOG( "CombineExt probe: GR_EXTENSION='%s'",
            ext ? extbuf : "(null)" );
    OGLLOG( "CombineExt probe: grColorCombineExt=0x%x "
            "grAlphaCombineExt=0x%x",
            (unsigned)s_pfnColorCombineExt,
            (unsigned)s_pfnAlphaCombineExt );
    if ( !s_pfnColorCombineExt || !s_pfnAlphaCombineExt ) {
        s_pfnColorCombineExt = 0;
        s_pfnAlphaCombineExt = 0;
        OGLLOG( "CombineExt probe: ** FAILED ** (grGetProcAddress NULL) "
                "-> legacy combine, NO 2x overbright; need another "
                "2x mechanism" );
    } else {
        OGLLOG( "CombineExt probe: OK -> 2x overbright available" );
    }
}

/* OPT 0.1.4 overbright: color = ( itrgb * (T0*T1 chain) ) << 1, alpha
** unscaled (italpha * texalpha).  While the extended combine mode is
** enabled glide derives BOTH the color and alpha units from the ext args,
** so both must be programmed here.  TMU combines stay legacy. */
static void __glSSTSetCombineExt2x( void ) {
    if ( !s_combineExtActive ) {
        (*s_pfnColorCombineExt)( GR_CMBX_ITRGB,       GR_FUNC_MODE_X,
                                 GR_CMBX_ZERO,        GR_FUNC_MODE_X,
                                 GR_CMBX_TEXTURE_RGB, FXFALSE,
                                 GR_CMBX_ZERO,        FXFALSE,
                                 /* shift arg -> cc_outshift: glide3
                                 ** _grCCExtcombineMode (GGLIDE.C) maps
                                 ** 1 -> SST_CM_CC_OUTSHIFT_2X and
                                 ** 2 -> SST_CM_CC_OUTSHIFT_4X, so 1 is
                                 ** the correct 2x (verified in H5 src) */
                                 1 /* output << 1 == 2x overbright */,
                                 FXFALSE );
        (*s_pfnAlphaCombineExt)( GR_CMBX_ITALPHA,       GR_FUNC_MODE_X,
                                 GR_CMBX_ZERO,          GR_FUNC_MODE_X,
                                 GR_CMBX_TEXTURE_ALPHA, FXFALSE,
                                 GR_CMBX_ZERO,          FXFALSE,
                                 0, FXFALSE );
        s_combineExtActive = 1;
        /* force the next legacy path to actually re-issue its combine
        ** calls (they are what exit GR_COMBINEEXT_MODE in glide) */
        s_CCWord = ~0;
        s_ACWord = ~0;
    }
}

/* Single emission point for the legacy color/alpha combine.  Leaving the
** extended combine mode requires the legacy calls to really be issued
** (glide's grColorCombine/grAlphaCombine disable GR_COMBINEEXT_MODE), so
** the dedup words are invalidated on the ext->legacy transition. */
static void __glSSTSetLegacyCombine( unsigned long CCWord,
                                     unsigned long ACWord ) {
    if ( s_combineExtActive ) {
        s_combineExtActive = 0;
        s_CCWord = ~0;
        s_ACWord = ~0;
    }
    if ( s_CCWord != CCWord ) {
        grColorCombine( USE_CUWORD( CCWord ), FXFALSE );
        s_CCWord = CCWord;
    }
    if ( s_ACWord != ACWord ) {
        grAlphaCombine( USE_CUWORD( ACWord ), FXFALSE );
        s_ACWord = ACWord;
    }
}

/* OPT 0.1.4 overbright: called from context init (MakeCurrent programs the
** legacy combine directly, bypassing the dedup caches) so a stale
** ext-active/dedup state can never leak across a glide context. */
void __glSSTResetCombineCache( void ) {
    s_ACWord  = ~0;
    s_CCWord  = ~0;
    s_TC0Word = ~0;
    s_TC1Word = ~0;
    s_combineExtActive = 0;
    s_combineExtProbed = 0;
    s_pfnColorCombineExt = 0;
    s_pfnAlphaCombineExt = 0;
    __glSSTOverbright2xVtx = 0; /* OPT 0.1.5 overbright vertex-double gate */
}

static GrTexInfo cdrsTex;
void __glSSTSetCDRSTexture(__GLcontext *gc) {
    unsigned long ACWord;
    unsigned long CCWord;
    unsigned long TC0Word;
    unsigned long TC1Word;
    unsigned int tmu = 0;

    if ( gc->grNTexelFx == 1 ) {
        grVertexLayout( GR_PARAM_Q,  offsetof( GrVertex, oow ), GR_PARAM_ENABLE );
        grVertexLayout( GR_PARAM_Q0, offsetof( GrVertex, oow ), GR_PARAM_ENABLE );
        grVertexLayout( GR_PARAM_ST0, offsetof( GrVertex, tmuvtx[0].sow ), GR_PARAM_ENABLE );
        TC0Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                               GR_COMBINE_FACTOR_NONE,
                               GR_COMBINE_FUNCTION_LOCAL,
                               GR_COMBINE_FACTOR_NONE );
    } else { /* two tmu configuration */
        tmu = 1;
        grVertexLayout( GR_PARAM_ST0, offsetof( GrVertex, tmuvtx[0].sow ),
			GR_PARAM_ENABLE );
        grVertexLayout( GR_PARAM_Q,   offsetof( GrVertex, oow ),
			GR_PARAM_ENABLE );
        grVertexLayout( GR_PARAM_Q0,  offsetof( GrVertex, oow ),
			GR_PARAM_ENABLE );
        grVertexLayout( GR_PARAM_ST1, 0, GR_PARAM_DISABLE );
        grVertexLayout( GR_PARAM_Q1,  0, GR_PARAM_DISABLE );

        TC0Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                               GR_COMBINE_FACTOR_ONE,
                               GR_COMBINE_FUNCTION_SCALE_OTHER,
                               GR_COMBINE_FACTOR_ONE );
        TC1Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                               GR_COMBINE_FACTOR_NONE,
                               GR_COMBINE_FUNCTION_LOCAL,
                               GR_COMBINE_FACTOR_NONE );

        if ( s_TC1Word != TC1Word ) {
            grTexCombine( GR_TMU1, USE_CUWORD( TC1Word ), 0, 0 );
            s_TC1Word = TC1Word;
        }
    }
    if ( s_TC0Word != TC0Word ) {
        grTexCombine( GR_TMU0, USE_CUWORD( TC0Word ), 0, 0 );
        s_TC0Word = TC0Word;
    }
    CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                          GR_COMBINE_FACTOR_NONE,
                          GR_COMBINE_LOCAL_ITERATED,
                          GR_COMBINE_OTHER_NONE );
    ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                          GR_COMBINE_FACTOR_LOCAL,
                          GR_COMBINE_LOCAL_ITERATED,
                          GR_COMBINE_OTHER_TEXTURE );
    /* OPT 0.1.5 overbright: CDRS programs its own combine; the world-path
    ** vertex-double gate must not leak into CDRS draws. */
    __glSSTOverbright2xVtx = 0;
    __glSSTSetLegacyCombine( CCWord, ACWord );
    grTexSource(0, 0, GR_MIPMAPLEVELMASK_BOTH, &cdrsTex);
}

/* This function examines the GC and asserts the
   correct color combine, alpha combine, and
   texture combine states. */
void __glSSTLoadCombineFunction( __GLcontext *gc ) {
    GLenum format;
    GLenum texEnv, texEnv0, texEnv1, enTex0, enTex1;
    GLuint setCCU;
    GLuint overbright2x; /* OPT 0.1.4 overbright */

    unsigned long ACWord;
    unsigned long CCWord;
    unsigned long TC0Word;
    unsigned long TC1Word;

    texEnv = texEnv0 = gc->state.texture[0].env[0].mode;
    texEnv1 = gc->state.texture[1].env[0].mode;
    enTex0  = gc->state.enables.texture[0] & 
              ( __GL_TEXTURE_2D_ENABLE | 
                __GL_TEXTURE_1D_ENABLE );
    enTex1  = gc->state.enables.texture[1] &
              ( __GL_TEXTURE_2D_ENABLE |
                __GL_TEXTURE_1D_ENABLE );

    /* RETRO3DFX diagnostic (CS green-world): file marker C:\icd_nomt.on
    ** forces single-texture (drop TMU1/lightmap).  If the live world then
    ** renders correct-tan, the bug is in the 2-TMU combine path; if it
    ** stays green, it is base-texture sampling.  Checked once. */
    { static int nomt = -1;
      if ( nomt < 0 ) {
          HANDLE g = CreateFileA( "C:\\icd_nomt.on", GENERIC_READ,
                                  FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 );
          nomt = ( g != INVALID_HANDLE_VALUE ) ? 1 : 0;
          if ( nomt ) CloseHandle( g );
      }
      if ( nomt ) enTex1 = 0;
    }
    /* XXXTaco the format is not properly considered for 
       texture-texture combination at this time */
    format  = GL_RGBA;

    /* XXXtaco set default state */
    setCCU  = 0;
    overbright2x = 0;

    ACWord = 
    CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                           GR_COMBINE_FACTOR_NONE,
                           GR_COMBINE_LOCAL_ITERATED,
                           GR_COMBINE_OTHER_NONE );
    TC0Word = 
    TC1Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_NONE,
                           GR_COMBINE_FACTOR_NONE,
                           GR_COMBINE_FUNCTION_NONE,
                           GR_COMBINE_FACTOR_NONE );
    
    if ( gc->grNTexelFx == 1 ) {
        if ( enTex0 ) {
            grVertexLayout( GR_PARAM_Q,  offsetof( GrVertex, oow ), GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q0, offsetof( GrVertex, oow ), GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_ST0, offsetof( GrVertex, tmuvtx[0].sow ), GR_PARAM_ENABLE );
            TC0Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                   GR_COMBINE_FACTOR_NONE,
                                   GR_COMBINE_FUNCTION_LOCAL,
                                   GR_COMBINE_FACTOR_NONE );
            texEnv = texEnv0;
            setCCU = 1;
            if ( gc->texture.currentTexture[0] ) {
                format = gc->texture.currentTexture[0]->level[0].baseFormat;
            } else {
                format = GL_RGBA;
            }
        } else {
            grVertexLayout( GR_PARAM_ST0, 0, GR_PARAM_DISABLE );
            grVertexLayout( GR_PARAM_Q0,  0, GR_PARAM_DISABLE );
            grVertexLayout( GR_PARAM_Q,   0, GR_PARAM_DISABLE );
        }
    } else { /* two tmu configuration */
        if ( enTex0 && enTex1 ) { /* both tmus enabled */
            grVertexLayout( GR_PARAM_ST0, offsetof( GrVertex, tmuvtx[0].sow ), GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q,   offsetof( GrVertex, oow ),           GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q0,  offsetof( GrVertex, oow ),           GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_ST1, offsetof( GrVertex, tmuvtx[1].sow ), GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q1,  offsetof( GrVertex, tmuvtx[1].oow ), GR_PARAM_ENABLE );

            switch( texEnv0 ) {
            case GL_REPLACE:
            case GL_MODULATE: /* OPT 0.1.4 GL_ARB_multitexture: unit-0
                              ** MODULATE differs from REPLACE only in that
                              ** the iterated color re-enters at the color
                              ** combine (set below); TMU words identical. */
                TC1Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                       GR_COMBINE_FACTOR_NONE,
                                       GR_COMBINE_FUNCTION_LOCAL,
                                       GR_COMBINE_FACTOR_NONE );
                switch( texEnv1 ) {
                case GL_REPLACE: /* wasteful */
                    TC0Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                           GR_COMBINE_FACTOR_NONE,
                                           GR_COMBINE_FUNCTION_LOCAL,
                                           GR_COMBINE_FACTOR_NONE );
                    break;
                case GL_MODULATE:
                    TC0Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                           GR_COMBINE_FACTOR_LOCAL,
                                           GR_COMBINE_FUNCTION_SCALE_OTHER,
                                           GR_COMBINE_FACTOR_LOCAL );
                    break;
                case GL_DECAL: /* OTHER + LOCAL_ALPHA * ( LOCAL - OTHER ) */
                    TC0Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_BLEND,
                                           GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA,
                                           GR_COMBINE_FUNCTION_SCALE_OTHER,
                                           GR_COMBINE_FACTOR_ONE );
                    break;
                case GL_BLEND:
                    TC0Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                           GR_COMBINE_FACTOR_ONE_MINUS_LOCAL,
                                           GR_COMBINE_FUNCTION_SCALE_OTHER,
                                           GR_COMBINE_FACTOR_ONE_MINUS_LOCAL );
                    break;
                }
                break;
            case GL_BLEND:
            case GL_DECAL:
                /* not implemented */
                break;
            }
            if ( ( texEnv0 == GL_MODULATE ) && ( texEnv1 != GL_REPLACE ) ) {
                /* OPT 0.1.4 GL_ARB_multitexture: unit-0 GL_MODULATE --
                ** final = iterated * (TMU chain).  Q3 world surfaces are
                ** MODULATE (base) x MODULATE (lightmap): chain = T0*T1,
                ** then modulate by the iterated vertex color here. */
                ACWord  =
                CCWord  = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                       GR_COMBINE_FACTOR_LOCAL,
                                       GR_COMBINE_LOCAL_ITERATED,
                                       GR_COMBINE_OTHER_TEXTURE );
                /* OPT 0.1.4 overbright: MODULATE x MODULATE is the
                ** collapsed Q3 base-x-lightmap world path; the 2-pass
                ** equivalent gains a free 2x from the lightmap pass's
                ** (GL_DST_COLOR,GL_SRC_COLOR) blend (r_overBrightBits=1).
                ** GL exposes no overbright signal to the ICD, so the 2x
                ** is keyed on this texenv combination -- the standard
                ** 3dfx behavior.  Requires the Napalm COMBINE extension;
                ** without it (Voodoo1/2/3/Banshee) the legacy unscaled
                ** combine below is kept. */
                if ( texEnv1 == GL_MODULATE ) {
                    static int s_ob2xLogged = 0; /* first-hit log only */
                    __glSSTProbeCombineExt(); /* kept for its diagnostics */
                    /* OPT 0.1.5: the 2x now lives in the ITERATED vertex
                    ** color (doubled+clamped in sst_pgmode.c), NOT in the
                    ** ext output shift (proven a no-op on live Napalm).
                    ** No hardware extension needed -> unconditional. */
                    overbright2x = 1;
                    if ( !s_ob2xLogged ) {
                        s_ob2xLogged = 1;
                        OGLLOG( "overbright2x branch: ext_active=%d "
                                "applying 2x via vertex-color double "
                                "(texEnv0=0x%x texEnv1=0x%x)",
                                (int)( s_pfnColorCombineExt != 0 ),
                                (unsigned)texEnv0, (unsigned)texEnv1 );
                    }
                }
            } else {
                ACWord  =
                CCWord  = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                       GR_COMBINE_FACTOR_ONE,
                                       GR_COMBINE_LOCAL_NONE,
                                       GR_COMBINE_OTHER_TEXTURE );
            }
        } else if ( enTex0 ) {
            grVertexLayout( GR_PARAM_ST1, offsetof( GrVertex, tmuvtx[0].sow ), GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q,   offsetof( GrVertex, oow ),           GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q1,  offsetof( GrVertex, oow ),           GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_ST0, 0, GR_PARAM_DISABLE );
            grVertexLayout( GR_PARAM_Q0,  0, GR_PARAM_DISABLE );

            TC0Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                   GR_COMBINE_FACTOR_ONE,
                                   GR_COMBINE_FUNCTION_SCALE_OTHER,
                                   GR_COMBINE_FACTOR_ONE );

            TC1Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                   GR_COMBINE_FACTOR_NONE,
                                   GR_COMBINE_FUNCTION_LOCAL,
                                   GR_COMBINE_FACTOR_NONE );

            setCCU = 1;
            texEnv = texEnv0;
            if ( gc->texture.currentTexture[0] ) {
                format = gc->texture.currentTexture[0]->level[0].baseFormat;
            } else {
                format = GL_RGBA;
            }
        } else if ( enTex1 ) {
            grVertexLayout( GR_PARAM_ST0, offsetof( GrVertex, tmuvtx[0].sow ), GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q,   offsetof( GrVertex, oow ),           GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_Q0,  offsetof( GrVertex, oow ),           GR_PARAM_ENABLE );
            grVertexLayout( GR_PARAM_ST1, 0, GR_PARAM_DISABLE );
            grVertexLayout( GR_PARAM_Q1,  0, GR_PARAM_DISABLE );

            TC1Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_NONE,
                                   GR_COMBINE_FACTOR_NONE,
                                   GR_COMBINE_FUNCTION_NONE,
                                   GR_COMBINE_FACTOR_NONE );

            TC0Word = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                   GR_COMBINE_FACTOR_NONE,
                                   GR_COMBINE_FUNCTION_LOCAL,
                                   GR_COMBINE_FACTOR_NONE );

            setCCU = 1;
            texEnv = texEnv1;
            if ( gc->texture.currentTexture[1] ) {
                format = gc->texture.currentTexture[1]->level[0].baseFormat;
            } else {
                format = GL_RGBA;
            }
        } else {
            grVertexLayout( GR_PARAM_ST1, 0, GR_PARAM_DISABLE );
            grVertexLayout( GR_PARAM_Q1,  0, GR_PARAM_DISABLE );
            grVertexLayout( GR_PARAM_ST0, 0, GR_PARAM_DISABLE );
            grVertexLayout( GR_PARAM_Q0,  0, GR_PARAM_DISABLE );
            grVertexLayout( GR_PARAM_Q,   0, GR_PARAM_DISABLE );
        }
        if ( s_TC1Word != TC1Word ) {
            grTexCombine( GR_TMU1, USE_CUWORD( TC1Word ), 0, 0 );
            s_TC1Word = TC1Word;
        }
    }
    if ( s_TC0Word != TC0Word ) {
        grTexCombine( GR_TMU0, USE_CUWORD( TC0Word ), 0, 0 );
        s_TC0Word = TC0Word;
    }

    if ( setCCU ) {
        switch (texEnv) {
        case GL_MODULATE:
            switch (format) {
            case GL_RGB:
            case GL_LUMINANCE:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_LOCAL,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_TEXTURE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                      GR_COMBINE_FACTOR_NONE,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_NONE );
                break;
            case GL_RGBA:
            case GL_LUMINANCE_ALPHA:
            case GL_INTENSITY:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_LOCAL,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_TEXTURE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_LOCAL,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_TEXTURE );
                break;
            case GL_ALPHA:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                      GR_COMBINE_FACTOR_NONE,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_NONE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_LOCAL,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_TEXTURE );
                break;
            }
            break;
        case GL_DECAL:
            switch (format) {
            case GL_RGB:
            case GL_LUMINANCE: /* undefined - treat just like RGB */
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_ONE,
                                      GR_COMBINE_LOCAL_NONE,
                                      GR_COMBINE_OTHER_TEXTURE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                      GR_COMBINE_FACTOR_NONE,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_NONE );
                break;
            case GL_RGBA:
            case GL_LUMINANCE_ALPHA: /* undefined - treat just like RGBA */
            case GL_INTENSITY: /* undefined - treat just like RGBA */
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_BLEND,
                                      GR_COMBINE_FACTOR_TEXTURE_ALPHA,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_TEXTURE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                      GR_COMBINE_FACTOR_NONE,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_NONE );
                break;
            case GL_ALPHA: /* undefined - treat just like REPLACE */
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                      GR_COMBINE_FACTOR_NONE,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_NONE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_LOCAL,
                                      GR_COMBINE_LOCAL_NONE,
                                      GR_COMBINE_OTHER_TEXTURE );
                break;
            }
            break;
        case GL_REPLACE:
            switch (format) {
            case GL_RGB:
            case GL_LUMINANCE:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_ONE,
                                      GR_COMBINE_LOCAL_NONE,
                                      GR_COMBINE_OTHER_TEXTURE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                      GR_COMBINE_FACTOR_NONE,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_NONE );
                break;
            case GL_RGBA:
            case GL_LUMINANCE_ALPHA:
            case GL_INTENSITY:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_ONE,
                                      GR_COMBINE_LOCAL_NONE,
                                      GR_COMBINE_OTHER_TEXTURE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_ONE,
                                      GR_COMBINE_LOCAL_NONE,
                                      GR_COMBINE_OTHER_TEXTURE );
                break;
            case GL_ALPHA:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                      GR_COMBINE_FACTOR_NONE,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_NONE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_ONE,
                                      GR_COMBINE_LOCAL_NONE,
                                      GR_COMBINE_OTHER_TEXTURE );
                break;
            }
            break;
        case GL_BLEND: /* XXXtaco unsupported by sst1, supported on Voodoo 2 and higher */
            switch (format) {
            case GL_ALPHA:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                      GR_COMBINE_FACTOR_NONE,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_NONE );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_LOCAL,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_TEXTURE );
                break;
            case GL_RGB:
            case GL_LUMINANCE:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_BLEND,
                                      GR_COMBINE_FACTOR_TEXTURE_RGB,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_CONSTANT );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_LOCAL,
                                      GR_COMBINE_FACTOR_NONE,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_NONE );
                break;
            case GL_RGBA:
            case GL_LUMINANCE_ALPHA:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_BLEND,
                                      GR_COMBINE_FACTOR_TEXTURE_RGB,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_CONSTANT );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_SCALE_OTHER,
                                      GR_COMBINE_FACTOR_LOCAL,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_TEXTURE );
                break;
            case GL_INTENSITY:
                CCWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_BLEND,
                                      GR_COMBINE_FACTOR_TEXTURE_RGB,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_CONSTANT );
                ACWord = MAKE_CUWORD( GR_COMBINE_FUNCTION_BLEND,
                                      GR_COMBINE_FACTOR_TEXTURE_ALPHA,
                                      GR_COMBINE_LOCAL_ITERATED,
                                      GR_COMBINE_OTHER_CONSTANT );
                break;
            }
            break;
        }
    }

    /* OPT 0.1.5 overbright: publish the vertex-double gate for the render
    ** procs and ALWAYS program the LEGACY 1x combine.  The doubling lives
    ** in the iterated vertex color now, so applying the ext output-shift
    ** 2x on top would be 4x (and it was proven a no-op on live Napalm
    ** anyway) -- __glSSTSetCombineExt2x is intentionally NOT called. */
    __glSSTOverbright2xVtx = overbright2x;
    __glSSTSetLegacyCombine( CCWord, ACWord );
    gc->cdrsTexture = 0;
}

void APIENTRY __glsstim_TexEnvfv(GLenum target, GLenum pname, const GLfloat pv[])
{
    __GLtextureEnvState *tes;
    GLenum e;
    unsigned long ccolor;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();
    
    if(target < GL_TEXTURE_ENV) {
      __glSetError(GL_INVALID_ENUM);
      return;
    }
    target -= GL_TEXTURE_ENV;
    if (target >= (GLuint) gc->constants.numberOfTextureEnvs) {
      bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    tes = &gc->state.texture[gc->texture.currentTexUnit].env[target];

    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
        switch(e = (GLenum) pv[0]) {
          case GL_MODULATE:
          case GL_DECAL:
          case GL_REPLACE:
          case GL_BLEND: /* XXXtaco supported only on Voodoo2, this needs to be conditional on V1 */
              tes->mode = e;
              break;
          case GL_ADD: /* XXXshui where is this mode documented? */
              return;
          default:
              goto bad_enum;
        }
        break;
      case GL_TEXTURE_ENV_COLOR:
        __glClampAndScaleColorf(gc, &tes->color, pv);
        ccolor = ( ((((unsigned long)(tes->color.a))&0xff)<<24) |
                   ((((unsigned long)(tes->color.r))&0xff)<<16) |
                   ((((unsigned long)(tes->color.g))&0xff)<<8) |
                   ((((unsigned long)(tes->color.b))&0xff)<<0) );
        grConstantColorValue( ccolor );
        break;
      default:
        goto bad_enum;
    }
    gc->validateTexture = 1;
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_TexEnvf(GLenum target, GLenum pname, GLfloat f)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
        __glsstim_TexEnvfv(target, pname, &f);
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

void APIENTRY __glsstim_TexEnviv(GLenum target, GLenum pname, const GLint pv[])
{
    __GLtextureEnvState *tes;
    GLenum e;
    unsigned long ccolor;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    if(target < GL_TEXTURE_ENV) {
      __glSetError(GL_INVALID_ENUM);
      return;
    }
    target -= GL_TEXTURE_ENV;
    if (target >= (GLuint) gc->constants.numberOfTextureEnvs) {
      bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    tes = &gc->state.texture[gc->texture.currentTexUnit].env[target];

    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
        switch(e = (GLenum) pv[0]) {
          case GL_MODULATE:
          case GL_DECAL:
          case GL_REPLACE:
          case GL_ADD:
            tes->mode = e;
            break;
          case GL_BLEND:
            return;
          default:
            goto bad_enum;
        }
        break;
      case GL_TEXTURE_ENV_COLOR:
        __glClampAndScaleColori(gc, &tes->color, pv);
        ccolor = ( ((((unsigned long)(tes->color.a))&0xff)<<24) |
                   ((((unsigned long)(tes->color.r))&0xff)<<16) |
                   ((((unsigned long)(tes->color.g))&0xff)<<8) |
                   ((((unsigned long)(tes->color.b))&0xff)<<0) );
        grConstantColorValue( ccolor );
        break;
      default:
        goto bad_enum;
    }
    gc->validateTexture = 1;
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_TexEnvi(GLenum target, GLenum pname, GLint i)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
        __glsstim_TexEnviv(target, pname, &i);
        break;
      default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
}

/************************************************************************/

GLboolean __glSSTIsTextureConsistent(__GLcontext *gc, __GLtexture *tex)
{
    __GLtextureParamState *params = &tex->params;
    GLint i, width, height, depth;
    GLint maxLevel;
    GLint border;
    GLenum baseFormat;
    GLenum requestedFormat;

    if ((tex->level[0].width == 0) ||
        (tex->level[0].height == 0) ||
        (tex->level[0].depth == 0)) {
        return GL_FALSE;
    }

    border = tex->level[0].border;
    width = tex->level[0].width - border*2;
    height = tex->level[0].height - border*2;
    depth = tex->level[0].depth - border*2;
    maxLevel = gc->constants.maxMipMapLevel;

    baseFormat = tex->level[0].baseFormat;
    if (gc->modes.rgbMode) {
        if (baseFormat == GL_COLOR_INDEX) {
#ifdef GL_EXT_paletted_texture
            baseFormat = tex->CT.baseFormat;
            if (baseFormat == 0 || baseFormat == GL_COLOR_INDEX) {
                return GL_FALSE;
            }
#else
            return GL_FALSE;
#endif
        }
    } else {
        if (baseFormat != GL_COLOR_INDEX) {
            return GL_FALSE;
        }
    }

    requestedFormat = tex->level[0].requestedFormat;

    switch(gc->state.texture[gc->texture.currentTexUnit].env[0].mode) {
      case GL_DECAL:
        if (!(gc->modes.rgbMode &&
             (baseFormat == GL_RGB || baseFormat == GL_RGBA))) {
            return GL_FALSE;
        }
        break;
      case GL_BLEND:
      case GL_MODULATE:
        if (!gc->modes.rgbMode) {
            return GL_FALSE;
        }
        break;
      case GL_ADD:
        if (gc->modes.rgbMode) {
            return GL_FALSE;
        }
      default:
        break;
    }

    /* If not-mipmapping, we are ok */
    switch (params->minFilter) {
      case GL_NEAREST:
      case GL_LINEAR:
        return GL_TRUE;
      default:
        break;
    }

    i = 0;
    while (++i < maxLevel) {
        if (width == 1 && height == 1) break;
        width >>= 1;
        if (width == 0) width = 1;
        height >>= 1;
        if (height == 0) height = 1;
        depth >>= 1;
        if (depth == 0) depth = 1;

        if (tex->level[i].border != border ||
                tex->level[i].requestedFormat != requestedFormat ||
                tex->level[i].width != width + border*2 ||
                tex->level[i].height != height + border*2 ||
                tex->level[i].depth != depth + border*2) {
            return GL_FALSE;
        }
    }

    return GL_TRUE;
}

static __GLtexture *CheckTexImageArgs(__GLcontext *gc, GLenum target, GLint lod,
                                      GLint components, GLint border,
                                      GLenum format, GLenum type, GLint dim)
{
    __GLtexture *tex = __glLookUpTexture(gc, target, gc->texture.currentTexUnit );

    /* one-shot diagnostics: the first 14 TexImage calls' exact shape --
    ** which internalformat/format/type combos the live app really uses
    ** (GoldSrc color hunt: probes only covered format=GL_RGBA). */
    { static int logged = 0;
      if (logged < 14) {
          logged++;
          OGLLOG("TexImage: ifmt=0x%x fmt=0x%x type=0x%x dim=%d",
                  (unsigned)components, (unsigned)format, (unsigned)type, dim);
      } }

    if (!tex || (tex->dim != dim)) {
      bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return 0;
    }

    switch (type) {
      case GL_BITMAP:
        if (format != GL_COLOR_INDEX) goto bad_enum;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
        break;
      case GL_UNSIGNED_BYTE_3_3_2_EXT:
        switch (format) {
          case GL_RGB:
          case GL_BGR_EXT:
            break;
          default:
            __glSetError(GL_INVALID_OPERATION);
            return 0;
        }
        break;
      case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
      case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
      case GL_UNSIGNED_INT_8_8_8_8_EXT:
      case GL_UNSIGNED_INT_10_10_10_2_EXT:
        switch (format) {
          case GL_RGBA:
          case GL_ABGR_EXT:
          case GL_BGRA_EXT:
            break;
          default:
            __glSetError(GL_INVALID_OPERATION);
            return 0;
        }
        break;

      default:
        goto bad_enum;
    }

    switch (format) {
      case GL_COLOR_INDEX:      case GL_RED:
      case GL_GREEN:            case GL_BLUE:
      case GL_ALPHA:            case GL_RGB:
      case GL_RGBA:             case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:  case GL_ABGR_EXT:
      case GL_BGRA_EXT:         case GL_BGR_EXT:
        break;
      default:
        goto bad_enum;
    }

    if ((lod < 0) || (lod >= gc->constants.maxMipMapLevel)) {
      bad_value:
        __glSetError(GL_INVALID_VALUE);
        return 0;
    }

    switch (components) {
      case 1: case 2: case 3: case 4:
      case GL_LUMINANCE:
      case GL_LUMINANCE4:       case GL_LUMINANCE8:
      case GL_LUMINANCE12:      case GL_LUMINANCE16:
        break;
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:        case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:        case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:      case GL_LUMINANCE16_ALPHA16:
        break;
      case GL_RGB:
      case GL_R3_G3_B2:         case GL_RGB4:
      case GL_RGB5:             case GL_RGB8:
      case GL_RGB10:            case GL_RGB12:
      case GL_RGB16:
        break;
      case GL_RGBA:
      case GL_RGBA2:            case GL_RGBA4:
      case GL_RGBA8:            case GL_RGBA12:
      case GL_RGBA16:           case GL_RGB5_A1:
      case GL_RGB10_A2:
        break;
      case GL_ALPHA:
      case GL_ALPHA4:           case GL_ALPHA8:
      case GL_ALPHA12:          case GL_ALPHA16:
        break;
      case GL_INTENSITY:
      case GL_INTENSITY4:       case GL_INTENSITY8:
      case GL_INTENSITY12:      case GL_INTENSITY16:
        break;
#ifdef GL_EXT_paletted_texture
    case GL_COLOR_INDEX1_EXT:   case GL_COLOR_INDEX2_EXT:
    case GL_COLOR_INDEX4_EXT:   case GL_COLOR_INDEX8_EXT:
    case GL_COLOR_INDEX12_EXT:  case GL_COLOR_INDEX16_EXT:
        if (format != GL_COLOR_INDEX)
            goto bad_value;
        switch (type) {
        case GL_BYTE:   case GL_UNSIGNED_BYTE:
        case GL_SHORT:  case GL_UNSIGNED_SHORT:
        case GL_INT:    case GL_UNSIGNED_INT:
            break;
        default:
            goto bad_value;
        }
        
        break;
#endif
      default:
        goto bad_enum;
    }

    if ((border < 0) || (border > 1)) {
        goto bad_value;
    }

    return tex;
}

/* ARGSUSED */
static GLint ComputeTexLevelSize(__GLcontext *gc, __GLtexture *tex,
                                 __GLmipMapLevel *lp, GLint lod,
                                 GLint components,
                                 GLsizei w, GLsizei h, GLsizei d,
                                 GLint border, GLint dim)
{
    GLint numTexels, texelStorageSize = 0;
    GLint baseWidth= (w-border*2) * (1 << lod);
    GLint baseHeight= (h-border*2) * (1 << lod);
    GLint baseDepth= (d-border*2) * (1 << lod);

    if (baseWidth == 0 || baseHeight == 0) {
        return 0;
    }
    if (baseWidth > gc->constants.maxTextureSize ||
        baseHeight > gc->constants.maxTextureSize) {
        return -1;
    }
    if (baseWidth/baseHeight > 8 || baseHeight/baseWidth > 8) {
        return -1;
    }
                                                                
    numTexels = w * h;

    lp->requestedFormat = (GLenum) components;
    lp->redSize = 0;
    lp->greenSize = 0;
    lp->blueSize = 0;
    lp->alphaSize = 0;
    lp->luminanceSize = 0;
    lp->intensitySize = 0;

    switch (lp->requestedFormat) {
      case GL_RGB:              case 3:
      case GL_RGB8:
      case GL_RGB10:            case GL_RGB12:
      case GL_RGB16:
        lp->baseFormat = GL_RGB;
        lp->internalFormat = __GL_SST_RGB_565;
        lp->redSize = 5;
        lp->greenSize = 6;
        lp->blueSize = 5;
        texelStorageSize = 1 * sizeof(GLushort);
        if (border) {
            lp->extract = __glExtractTexelRGB_B;
        } else {
            lp->extract = __glExtractTexelRGB;
        }
        break;
      case GL_R3_G3_B2:         case GL_RGB4:
      case GL_RGB5:
        lp->baseFormat = GL_RGB;
        lp->internalFormat = __GL_SST_RGB_332;
        lp->redSize = 3;
        lp->greenSize = 3;
        lp->blueSize = 2;
        texelStorageSize = 1 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelRGB_B;
        } else {
            lp->extract = __glExtractTexelRGB;
        }
        break;
      case GL_RGBA:             case 4:
      case GL_RGBA2:            case GL_RGBA4:
      case GL_RGBA8:            case GL_RGBA12:
      case GL_RGBA16:           case GL_RGB5_A1:
      case GL_RGB10_A2:
        lp->baseFormat = GL_RGBA;
        lp->internalFormat = __GL_SST_ARGB_4444;
        lp->redSize = 4;
        lp->greenSize = 4;
        lp->blueSize = 4;
        texelStorageSize = 1 * sizeof(GLushort);
        if (border) {
            lp->extract = __glExtractTexelRGB_B;
        } else {
            lp->extract = __glExtractTexelRGB;
        }
        break;
      case GL_LUMINANCE:        case 1:
      case GL_LUMINANCE4:       case GL_LUMINANCE8:
      case GL_LUMINANCE12:      case GL_LUMINANCE16:
        lp->baseFormat = GL_LUMINANCE;
        lp->internalFormat = __GL_SST_I_8;
        lp->luminanceSize = 8;
        texelStorageSize = 1 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelL_B;
        } else {
            lp->extract = __glExtractTexelL;
        }
        break;
      case GL_LUMINANCE_ALPHA:  case 2:
      case GL_LUMINANCE8_ALPHA8:        case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:      case GL_LUMINANCE16_ALPHA16:
        lp->baseFormat = GL_LUMINANCE_ALPHA;
        lp->internalFormat = __GL_SST_AI_88;
        lp->luminanceSize = 8;
        lp->alphaSize = 8;
        texelStorageSize = 1 * sizeof(GLushort);
        if (border) {
            lp->extract = __glExtractTexelLA_B;
        } else {
            lp->extract = __glExtractTexelLA;
        }
        break;
      case GL_LUMINANCE4_ALPHA4:        case GL_LUMINANCE6_ALPHA2:
        lp->baseFormat = GL_LUMINANCE_ALPHA;
        lp->internalFormat = __GL_SST_AI_44;
        lp->luminanceSize = 4;
        lp->alphaSize = 4;
        texelStorageSize = 1 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelLA_B;
        } else {
            lp->extract = __glExtractTexelLA;
        }
        break;
      case GL_ALPHA:
      case GL_ALPHA4:   case GL_ALPHA8:
      case GL_ALPHA12:  case GL_ALPHA16:
        lp->baseFormat = GL_ALPHA;
        lp->internalFormat = __GL_SST_A_8;
        lp->alphaSize = 8;
        texelStorageSize = 1 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelA_B;
        } else {
            lp->extract = __glExtractTexelA;
        }
        break;
      case GL_COLOR_INDEX8_EXT:
        lp->baseFormat = GL_RGB; 
        lp->internalFormat = __GL_SST_P_8;
        lp->alphaSize = 0;
        texelStorageSize = 1 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelA_B;
        } else {
            lp->extract = __glExtractTexelA;
        }
        break;
      default:
        break;
    }
    lp->texelSize = texelStorageSize;
    return (numTexels * texelStorageSize);
}

__GLtextureBuffer *__glSSTTexCreateProxyLevel(__GLcontext *gc, __GLtexture *tex,
                                           GLint lod, GLint components,
                                           GLsizei w, GLsizei h, GLsizei d,
                                           GLint border, GLint dim)
{
    __GLmipMapLevel template, *lp = &tex->level[lod];
    GLint bufferSize;

    bufferSize = ComputeTexLevelSize(gc, tex, &template, lod, components,
                                     w, h, d, border, dim);

    if (bufferSize < 0) {
        /* Proxy allocation failed */
        lp->width = 0;
        lp->height = 0;
        lp->depth = 0;
        lp->border = 0;
        lp->requestedFormat = 0;
        lp->baseFormat = 0;
        lp->internalFormat = 0;
        lp->redSize = 0;
        lp->greenSize = 0;
        lp->blueSize = 0;
        lp->alphaSize = 0;
        lp->luminanceSize = 0;
        lp->intensitySize = 0;
        lp->extract = (void *) __glNop;
    } else {
        /* Proxy allocation succeeded */
        lp->width = w;
        lp->height = h;
        lp->depth = d;
        lp->border = border;
        lp->requestedFormat = template.requestedFormat;
        lp->baseFormat = template.baseFormat;
        lp->internalFormat = template.internalFormat;
        lp->redSize = template.redSize;
        lp->greenSize = template.greenSize;
        lp->blueSize = template.blueSize;
        lp->alphaSize = template.alphaSize;
        lp->luminanceSize = template.luminanceSize;
        lp->intensitySize = template.intensitySize;
        lp->extract = template.extract;
    }
    return 0;
}

__GLtextureBuffer *__glSSTTexCreateLevel(__GLcontext *gc, __GLtexture *tex,
                                      GLint lod, GLint components,
                                      GLsizei w, GLsizei h, GLsizei d,
                                      GLint border, GLint dim)
{
    __GLmipMapLevel template, *lp = &tex->level[lod];
    GLint bufferSize;

    bufferSize = ComputeTexLevelSize(gc, tex, &template, lod, components,
                                    w, h, d, border, dim);
    if (bufferSize < 0) {
        /* Texture allocation failed */
        __glSetError(GL_INVALID_VALUE);
        return 0;
    } else if (bufferSize > 0) {
        /* Texture allocation succeeded, fill in new level info */

#ifdef __GL_SST
    bufferSize = (bufferSize + 7) & ~0x7; /* align to 8 */ 
    bufferSize += 8; /* add two DWORDS of bloat to guard against bad accesses from Glide*/
    tex->sst.bufferSize = bufferSize; /* XXXshui */
#endif
        lp->buffer = (__GLtextureBuffer*)
            (*gc->imports.realloc)(gc, lp->buffer, (size_t) bufferSize);
        
        if (lp->buffer == NULL) {
            __glSetError(GL_OUT_OF_MEMORY);
            return 0;
        }

        /* This is allocated lazily */
        lp->pixelBuffer = NULL;
        lp->width = w;
        lp->height = h;
        lp->depth = d;
        lp->imageSize = w * h;
        lp->width2 = w - border*2;
        lp->widthLog2 = __glFloorLog2(lp->width2);
        lp->height2 = h - border*2;
        lp->heightLog2 =__glFloorLog2(lp->height2);
        lp->depth2 = d - border*2;
        lp->depthLog2 =__glFloorLog2(lp->depth2);
#ifdef __GL_SST
        lp->width2f = tex->sst.sScale;
        lp->height2f = tex->sst.tScale;
        lp->depth2f = 256.0; /* no 3D textures yet */
#else
        lp->width2f = lp->width2;
        lp->height2f = lp->height2;
        lp->depth2f = lp->depth2;
#endif
        lp->border = border;
        lp->requestedFormat = template.requestedFormat;
        lp->baseFormat = template.baseFormat;
        lp->internalFormat = template.internalFormat;
        lp->redSize = template.redSize;
        lp->greenSize = template.greenSize;
        lp->blueSize = template.blueSize;
        lp->alphaSize = template.alphaSize;
        lp->luminanceSize = template.luminanceSize;
        lp->intensitySize = template.intensitySize;
        lp->texelSize = template.texelSize;
        lp->extract = template.extract;
    } else {
        /* The texture level is being freed */
        if (lp->buffer != NULL) {
            (*gc->imports.free)(gc, lp->buffer);
            lp->buffer = NULL;
        }
        if (lp->pixelBuffer != NULL) {
            (*gc->imports.free)(gc, lp->pixelBuffer);
            lp->buffer = NULL;
        }
        lp->width = 0;
        lp->height = 0;
        lp->depth = 0;
        lp->imageSize = 0;
        lp->width2 = 0;
        lp->height2 = 0;
        lp->depth2 = 0;
        lp->widthLog2 = 0;
        lp->heightLog2 = 0;
        lp->depthLog2 = 0;
        lp->border = 0;
        lp->requestedFormat = 1;
        lp->baseFormat = 0;
        lp->internalFormat = 0;
        lp->redSize = 0;
        lp->greenSize = 0;
        lp->blueSize = 0;
        lp->alphaSize = 0;
        lp->luminanceSize = 0;
        lp->intensitySize = 0;
        lp->extract = (void *) __glNop;
    }

    if (lod == 0) {
        tex->p = lp->heightLog2;
        if (lp->widthLog2 > tex->p) {
            tex->p = lp->widthLog2;
        }
        if (lp->depthLog2 > tex->p) {
            tex->p = lp->depthLog2;
        }
    }
    return lp->buffer;
}

/************************************************************************/

void APIENTRY __glsstim_TexImage1D(GLenum target, GLint lod, 
                       GLint components, GLsizei length,
                       GLint border, GLenum format,
                       GLenum type, const GLvoid *buf)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    __GLtextureObject *pto;

    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    /* XXXshui 1D textures unsupported for now */
    return;
    
    /* Check arguments and get the right texture being changed */
    tex = __glCheckTexImage1DArgs(gc, target, lod, components, length,
                                  border, format, type);
    if (!tex) {
        return;
    }
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_1D, gc->texture.currentTexUnit );
#if 0    
    /* this doesn't work with multitexture */
    if (pto->refcount > 2) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
#endif

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, lod, components,
                               length, 1+border*2, 1+border*2, border, 1);
    /* Copy image data */
    if (buf && dest) {
        __glInitTexSourceUnpack(gc, &spanInfo, length, 1, 1,
                                format, type, buf, GL_FALSE);
        __glInitTexImageStore(gc, &spanInfo, tex, lod);
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*tex->copyTexImage)(gc, &spanInfo, tex, lod);
    }

    /* Mark the bound texture as resident. */
    pto->resident = GL_TRUE;

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

void __glsstlei_TexImage1D(__GLcontext *gc, GLenum target, GLint lod,
                        GLint components, GLsizei length, GLint border,
                        GLenum format, GLenum type, const GLubyte *image)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;
    __GLtextureObject *pto;

    /* XXXshui unsupported for now */
    return;
    
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    beginMode = __gl_beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
        if (beginMode == __GL_NEED_VALIDATE) {
            (*gc->procs.validate)(gc);
            __gl_beginMode = __GL_NOT_IN_BEGIN;
        } else {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    /* Check arguments and get the right texture being changed */
    tex = __glCheckTexImage1DArgs(gc, target, lod, components, length,
                                  border, format, type);
    if (!tex) {
        return;
    }

    pto = __glLookUpTextureObject(gc, GL_TEXTURE_1D, gc->texture.currentTexUnit );
#if 0
    /* xxxtaco - this doesn't work with multitexture  */
    if (pto->refcount > 2) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
#endif

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, lod, components,
                               length, 1+border*2, 1+border*2, border, 1);

    /* Copy image data */
    if (image && dest) {
        __glInitTexSourceUnpack(gc, &spanInfo, length, 1, 1,
                                format, type, image, GL_TRUE);
        __glInitTexImageStore(gc, &spanInfo, tex, lod);
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*tex->copyTexImage)(gc, &spanInfo, tex, lod);
    }

    /* Mark the bound texture object as resident. */
    pto->resident = GL_TRUE;

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

/************************************************************************/

static GLubyte aatexture[] = {0, 80, 160, 240, 240, 160, 80, 0};
void __glSSTInitTextureManager(__GLcontext *gc)
{
    GLuint maxAddr;
    __GLSSTtexCacheNode *first;
    int texUnit;
    int memsize;
    
    for( texUnit = 0; texUnit < __GL_MAX_TEX_UNITS; texUnit++ ) {
        first = (__GLSSTtexCacheNode *)
            (*gc->imports.calloc)(gc, 1, (size_t) sizeof(__GLSSTtexCacheNode));
        first->tex = NULL;
        first->addr = grTexMinAddress(GR_TMU0);
        if (texUnit == 0) {
            /* Reserve room for the 8-byte cdrs/aa ramp texture downloaded at
            ** grTexMinAddress below -- but keep the heap start aligned to the
            ** NAPALM texBaseAddr granularity of 16 bytes.  The historical +8
            ** (SST1 had 8-byte granularity) put EVERY TMU0 texture at an
            ** address with bit 3 set; SST_TEXTURE_MUNGE_ADDRESS drops addr
            ** bits [3:0] on Napalm, so the sampler read 8 bytes below the
            ** download address: all texture content shifted +4 texels in S.
            ** That was the Q3/CS 2D "garbled text" (glyph sub-rect quads clip
            ** the shifted content into sliced strokes).  Proven on .143 V5500
            ** with a texel-ruler probe: constant +4-texel shift, row-end
            ** wrap-around at quad left edges, GDI reference at 0. */
            first->addr += 16;
        }
        
        /* for 4MB systems, we clamp to 2MB for now */
        maxAddr = grTexMaxAddress(GR_TMU0);
#if 0
        if (maxAddr > 0x1ffff8) maxAddr = 0x1ffff8;
#endif
    
        /* add 8 because max and min addresses are inclusive */
        first->len = maxAddr - first->addr + 8;
        first->next = NULL;
        gc->texture.sst.mru[texUnit] = first;
        gc->texture.sst.first[texUnit] = first;
        gc->texture.sst.endAddr[texUnit] = maxAddr;
    }
    cdrsTex.format = GR_TEXFMT_ALPHA_8;
    cdrsTex.smallLodLog2 = GR_LOD_LOG2_8;
    cdrsTex.largeLodLog2 = GR_LOD_LOG2_8;
    cdrsTex.aspectRatioLog2 = GR_ASPECT_LOG2_8x1;
    cdrsTex.data = aatexture;
    memsize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &cdrsTex);
    grTexDownloadMipMapLevel(0,
			     gc->texture.sst.first[0]->addr - 16,
			     GR_LOD_LOG2_8, GR_LOD_LOG2_8,
			     GR_ASPECT_LOG2_8x1,
                             GR_TEXFMT_ALPHA_8,
                             GR_MIPMAPLEVELMASK_BOTH,
                             aatexture);
}

static __GLSSTtexCacheNode *split(__GLcontext *gc, __GLSSTtexCacheNode *p, GLint len)
{
    __GLSSTtexCacheNode *new = (__GLSSTtexCacheNode *) 
      (*gc->imports.calloc)(gc, 1, (size_t) sizeof(__GLSSTtexCacheNode));

    new->next = p->next;
    new->len = p->len - len;
    new->addr = p->addr + len;
    new->tex = NULL;
    p->next = new;
    p->len = len;
    /* addr should already be filled; tex to be filled by caller */
    return new;
}

static void coallesce(__GLcontext *gc, __GLSSTtexCacheNode *p) 
{
    __GLSSTtexCacheNode *tmp = p->next;
    
    p->len += tmp->len;
    p->next = tmp->next;
#if 0
    if (gc->texture.sst.mru == tmp) {
        gc->texture.sst.mru = p;
    }
#endif
    (*gc->imports.free)(gc, tmp);
}

static void freenode(__GLcontext *gc, __GLSSTtexCacheNode *f)
{
    __GLSSTtexCacheNode *p, *prev = NULL;

    p = gc->texture.sst.first[gc->texture.currentTexUnit];
    while (p != NULL) {
        if (p == f) {
            break;
        }
        prev = p;
        p = p->next;
    }
    if (p) {
        p->tex = NULL;
        if (p->next && p->next->tex==NULL) {
            coallesce(gc, p);
        }
        if (prev && prev->tex==NULL) {
            coallesce(gc, prev);
        }
    }
}

void __glSSTFreeTextureMemory(__GLcontext *gc, __GLtexture *tex)
{
    __GLSSTtexCacheNode *p;

    p = tex->sst.texUnit[gc->texture.currentTexUnit].cache;
    if (p) {
        freenode(gc, p);
        tex->sst.texUnit[gc->texture.currentTexUnit].cache = NULL;
    }
}

void __glSSTAllocateTextureMemory(__GLcontext *gc, __GLtexture *tex, int len)
{
    __GLSSTtexCacheNode *p, *start;
    int need;
    GLuint txu;

    /* Keep every cache node 16-byte aligned (Napalm texBaseAddr granularity;
    ** MUNGE drops addr bits [3:0]).  grTexTextureMemRequired can return
    ** lengths that are only 8-aligned (e.g. small ALPHA_8 strips), which
    ** would knock all subsequent nodes onto +8 addresses.
    ** A/B: C:\icd_heap8.on reverts to the pre-0.3.1 unrounded len. */
    { static int heap8 = -1;
      if ( heap8 < 0 ) {
          HANDLE g = CreateFileA( "C:\\icd_heap8.on", GENERIC_READ,
                                  FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0 );
          heap8 = ( g != INVALID_HANDLE_VALUE ) ? 1 : 0;
          if ( heap8 ) CloseHandle( g );
      }
      if ( !heap8 ) len = (len + 15) & ~15;
    }

    { extern long __r3d_cTexAlloc; __r3d_cTexAlloc++; }

    txu = gc->texture.currentTexUnit;

    p = tex->sst.texUnit[txu].cache;
    if (p) {
        if (p->len == len) {
            /* do nothing */
            return;
        } else {
            freenode(gc, p);
            tex->sst.texUnit[txu].cache = NULL;
        }
    }

    /* look at node after mru, assuming that's the lru */
    start = gc->texture.sst.mru[txu]->next;
    if (start==NULL || (int)(gc->texture.sst.endAddr[txu] - start->addr) < len) {
        /* go back to the front */
        start = gc->texture.sst.first[txu];
    }

    /* compensate for 2M tex boundary quirk */
    if ( ( start->addr < 0x200000 ) && ( start->addr + len > 0x200000 ) ) {
        /* allocate dummy texture */
        p = start;
        need = 0x200000 - start->addr;
        while (need > 0) {
            if (p->tex) {
                p->tex->sst.texUnit[txu].cache = NULL;
            }
            if (p->len > need) {
                split(gc, p, need);
            }
            need -= p->len;
            if (p != start) {
                coallesce(gc,start);
            }
            p = start->next;
        }
        start->tex = 0; 
        start = start->next;
        assert( start->addr == 0x200000 );
    }

    /* grab nodes until we have enough */
    p = start;
    need = len;
    while (need > 0) {
        if (p->tex) {
            p->tex->sst.texUnit[txu].cache = NULL;
        }
        if (p->len > need) {
            split(gc, p, need);
        }
        need -= p->len;
        if (p != start) {
            coallesce(gc,start);
        }
        p = start->next;
    }
    start->tex = tex;
    tex->sst.texUnit[txu].cache = start;
    gc->texture.sst.mru[txu] = start;
}

void __glSSTShadowTexImage(__GLtexture *tex, GLint lod, GLenum format, GLenum type,
                          const GLvoid *buf)
{
    __GLmipMapLevel *lp = &tex->level[lod];
    GLushort *sp;
    GLubyte *bp, *src;
    GLfloat *fsrc;
    int i, j;
    int skip;

    switch (format) {
    case GL_LUMINANCE:
    case GL_RED:
    case GL_BLUE:
    case GL_GREEN:
    case GL_ALPHA:
        skip = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        skip = 2;
        break;
    case GL_RGB:
        skip = 3;
        break;
    case GL_RGBA:
        skip = 4;
        break;
    case GL_COLOR_INDEX:
        skip = 1;
        break;
    }
    
    /* XXXshui need to make this work for :
                cases where host format has fewer components than internal format
                cases where host format orders components differently than internal format
                cases where host is LA and internal is RGBA, or vice versa
    */
    src = (GLubyte *) buf;
    fsrc = (GLfloat *) buf;

    /* RETRO3DFX diag: gated dump of 256x256 RGBA/ubyte uploads (font atlases)
     * exactly as the app provided them. Gate C:\icd_trace ->
     * C:\texdump_NN.raw (raw RGBA). First 8 such textures. */
    if (format == GL_RGBA && type == GL_UNSIGNED_BYTE &&
        lp->width == 256 && lp->height == 256 && lod == 0) {
        static HANDLE gate = (HANDLE)-2;
        static int ndump = 0;
        if (gate == (HANDLE)-2) {
            HANDLE g = CreateFileA("C:\\icd_trace", GENERIC_READ,
                                   FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                                   OPEN_EXISTING, 0, 0);
            if (g != INVALID_HANDLE_VALUE) { CloseHandle(g); gate = (HANDLE)1; }
            else gate = INVALID_HANDLE_VALUE;
        }
        if (gate == (HANDLE)1 && ndump < 8) {
            char nm[64];
            HANDLE h;
            DWORD wr;
            wsprintfA(nm, "C:\\texdump_%02d.raw", ndump);
            h = CreateFileA(nm, GENERIC_WRITE, FILE_SHARE_READ, 0,
                            CREATE_ALWAYS, 0, 0);
            if (h != INVALID_HANDLE_VALUE) {
                WriteFile(h, buf, 256 * 256 * 4, &wr, 0);
                CloseHandle(h);
                ndump++;
            }
        }
    }

    switch (lp->internalFormat) {
    case __GL_SST_RGB_332:
        bp = (GLubyte *) lp->buffer;
        switch (type) {
        case GL_UNSIGNED_BYTE:
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *bp = (src[0] & 0xE0) | ((src[1] & 0xE0) >> 3) | ((src[2] & 0xC0) >> 6);
                    bp++;
                    src += skip;
                }
            }
            break;
        case GL_UNSIGNED_SHORT:
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *bp = (src[1] & 0xE0) | ((src[3] & 0xE0) >> 3) | ((src[5] & 0xC0) >> 6);
                    bp++;
                    src += skip * 2;
                }
            }
            break;
        case GL_FLOAT:
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *bp = (((GLint)(fsrc[0] * 7.0) << 5) |
                           ((GLint)(fsrc[1] * 7.0) << 2) |
                           ((GLint)(fsrc[2] * 3.0)));
                    bp++;
                    fsrc += skip;
                }
            }
            break;
        }
        break;
    case __GL_SST_RGB_565:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            sp = (GLushort *) lp->buffer;
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *sp = ((src[0] & 0xF8) << 8) | ((src[1] & 0xFC) << 3) |
                        ((src[2] & 0xF8) >> 3);
                    sp++;
                    src += skip;
                }
            } 
            break;
        case GL_UNSIGNED_SHORT:
            sp = (GLushort *) lp->buffer;
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *sp = ((src[1] & 0xF8) << 8) | ((src[3] & 0xFC) << 3) |
                        ((src[5] & 0xF8) >> 3);
                    sp++;
                    src += skip * 2;
                }
            } 
            break;
        case GL_FLOAT:
            sp = (GLushort *) lp->buffer;
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *sp = (((GLuint)(fsrc[0] * 31.0) << 11) |
                           ((GLuint)(fsrc[1] * 63.0) << 5) |
                           ((GLuint)(fsrc[2] * 31.0)));
                    sp++;
                    fsrc += skip;
                }
            } 
            break;
        }
        break;
    case __GL_SST_ARGB_4444:
        switch (type) {
        case GL_UNSIGNED_BYTE:
            sp = (GLushort *) lp->buffer;
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *sp = ((src[0] & 0xF0) << 4) | ((src[1] & 0xF0) << 0) |
                        ((src[2] & 0xF0) >> 4) | ((src[3] & 0xF0) << 8);
                    sp++;
                    src += skip;
                }
            } 
            break;
        case GL_UNSIGNED_SHORT:
            sp = (GLushort *) lp->buffer;
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *sp = ((src[1] & 0xF0) << 4) | ((src[3] & 0xF0) << 0) |
                        ((src[5] & 0xF0) >> 4) | ((src[7] & 0xF0) << 8);
                    sp++;
                    src += skip * 2;
                }
            } 
            break;
        case GL_FLOAT:
            sp = (GLushort *) lp->buffer;
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *sp = (((GLuint)(fsrc[0] * 15.0) <<  8) |
                           ((GLuint)(fsrc[1] * 15.0) <<  4) |
                           ((GLuint)(fsrc[2] * 15.0)      ) |
                           ((GLuint)(fsrc[3] * 15.0) << 12));
                    sp++;
                    fsrc += skip;
                }
            } 
            break;
        }
        break;
    case __GL_SST_AI_88: /* host format GL_LUMINANCE_ALPHA */
        sp = (GLushort *) lp->buffer;
        switch (format) {
        case GL_LUMINANCE_ALPHA:
            switch (type) {
            case GL_UNSIGNED_BYTE:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *sp = src[0] | (src[1] << 8);
                        sp++;
                        src += skip; 
                    }
                }
                break;
            case GL_UNSIGNED_SHORT:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *sp = src[1] | (src[3] << 8);
                        sp++;
                        src += skip * 2; 
                    }
                }
                break;
            case GL_FLOAT:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *sp = ((GLint)(fsrc[0] * 255.0)) |
                              ((GLint)(fsrc[1] * 255.0) << 8);
                        sp++;
                        fsrc += skip; 
                    }
                }
                break;
            }
            break;
        case GL_RGBA:
            switch (type) {
            case GL_UNSIGNED_BYTE:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *sp = src[0] | (src[3] << 8);
                        sp++;
                        src += skip; 
                    }
                }
                break;
            case GL_UNSIGNED_SHORT:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *sp = src[1] | (src[7] << 8);
                        sp++;
                        src += skip * 2; 
                    }
                }
                break;
            case GL_FLOAT:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *sp = ((GLint)(fsrc[0] * 255.0)) |
                              ((GLint)(fsrc[3] * 255.0) << 8);
                        sp++;
                        fsrc += skip; 
                    }
                }
                break;
            }
            break;
        }
        break;
    case __GL_SST_I_8: /* host format GL_LUMINANCE */
        bp = (GLubyte *) lp->buffer;
        switch (type) {
        case GL_UNSIGNED_BYTE:
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *bp++ = src[0];
                    src += skip; 
                }
            }
            break;
        case GL_UNSIGNED_SHORT:
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *bp++ = src[1];
                    src += skip * 2; 
                }
            }
            break;
        case GL_FLOAT:
            for (i=0; i < lp->height; i++) {
                for (j=0; j < lp->width; j++) {
                    *bp++ = (GLint)(fsrc[0] * 255.0);
                    fsrc += skip; 
                }
            }
            break;
        }
        break;
    case __GL_SST_A_8: /* host format GL_ALPHA */
        bp = (GLubyte *) lp->buffer;
        switch (format) {
        case GL_ALPHA:
            switch (type) {
            case GL_UNSIGNED_BYTE:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *bp++ = src[0];
                        src += skip; 
                    }
                }
                break;
            case GL_UNSIGNED_SHORT:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *bp++ = src[1];
                        src += skip * 2; 
                    }
                }
                break;
            case GL_FLOAT:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *bp++ = (GLint)(fsrc[0] * 255.0);
                        fsrc += skip; 
                    }
                }
                break;
            }
            break;
        case GL_RGBA:
            switch (type) {
            case GL_UNSIGNED_BYTE:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *bp++ = src[3];
                        src += skip; 
                    }
                }
                break;
            case GL_UNSIGNED_SHORT:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *bp++ = src[7];
                        src += skip * 2; 
                    }
                }
                break;
            case GL_FLOAT:
                for (i=0; i < lp->height; i++) {
                    for (j=0; j < lp->width; j++) {
                        *bp++ = (GLint)(fsrc[0] * 255.0);
                        fsrc += skip; 
                    }
                }
                break;
            }
            break;
        }
        break;
    case __GL_SST_P_8: /* host format GL_COLOR_INDEX */
        bp = (GLubyte *) lp->buffer;
        for (i=0; i < lp->height; i++) {
            for (j=0; j < lp->width; j++) {
                *bp++ = *src;
                src += skip; 
            }
        } 
        break;
    }
}

void __glSSTTexImage2D(GLenum target, GLint lod, GLint components,
                       GLsizei width, GLsizei height, GLint border, GLenum format,
                       GLenum type, const GLvoid *buf)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
#ifndef __GL_SST
    __GLpixelSpanInfo spanInfo;
#endif
    __GLtextureObject *pto;
    GrTexInfo grTex;
    GLint n, major, aspect, memsize;
    GLuint txu;

    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    /* one-shot diagnostics (GoldSrc color hunt): first 16 uploads' exact
    ** shape through the LIVE SST path (__glim_/GLCORE variants are NOT on
    ** the dispatch - earlier probes there logged nothing). */
    { static int logged = 0;
      /* width>=64 filters out the 16x16 font-glyph spam that exhausted the
      ** budget before the map's world textures ever uploaded. */
      if (logged < 400 && width >= 64 && lod == 0) {
          const unsigned char *p = (const unsigned char *)buf;
          logged++;
          OGLLOG("SSTTexImage2D: ifmt=0x%x %dx%d fmt=0x%x type=0x%x lod=%d bytes=%02x %02x %02x %02x %02x %02x %02x %02x",
                 (unsigned)components, (int)width, (int)height, (unsigned)format,
                 (unsigned)type, (int)lod,
                 p?p[0]:0, p?p[1]:0, p?p[2]:0, p?p[3]:0,
                 p?p[4]:0, p?p[5]:0, p?p[6]:0, p?p[7]:0);
      } }

    txu = gc->texture.currentTexUnit;

    /* intensity formats are not supported by sst1 and are nops */
    /* XXXTACO - GL_TEXFMT_A8 is identical to INTENSITY */
    if (border != 0) {
        return;
    }
    switch (components) {
      case GL_INTENSITY:
      case GL_INTENSITY4:       case GL_INTENSITY8:
      case GL_INTENSITY12:      case GL_INTENSITY16:
        return;
    }
    /* Check arguments and get the right texture being changed */
    tex = __glCheckTexImage2DArgs(gc, target, lod, components, width, height,
                                  border, format, type);
    if (!tex) {
        return;
    }
    
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_2D, gc->texture.currentTexUnit );
#if 0
    /* xxxtaco - this doesn't work with multitexture */
    if (pto->refcount > 2) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
#endif

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, lod, components,
                               width, height, 1+border*2, border, 2);

    if (!dest) {
        if (width == 0 || height == 0) {
            /* zero size, i.e. no texture */
            /* XXXshui should really free only if caused stack to be inconsistent */
            __glSSTFreeTextureMemory(gc,tex);
            tex->sst.allocation = __GL_SST_TEXALLOC_NONE;
            tex->sst.texUnit[txu].allocSize = 0;
            tex->sst.cacheMask = 0;
            __glSSTDisableTexturing(gc);
        } else {
            /* Failed because texture is too large */
        }
        goto doneTexture2D;
    }
      
    /* Copy image data */
    if (buf && dest) {
#ifdef __GL_SST
        __glSSTShadowTexImage(tex, lod, format, type, buf);
#else
        __glInitTexSourceUnpack(gc, &spanInfo, w, h, 1,
                                format, type, buf, GL_FALSE);
        __glInitTexImageStore(gc, &spanInfo, tex, lod);
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);
        (*tex->copyTexImage)(gc, &spanInfo, tex, lod);
#endif
    }

    /* Begin SST specific stuff. */

    if (lod == 0) {
        GLint nlod, w, h;

        if (width > height) {
            major = width;
        } else {
            major = height;
        }
        switch (major) {
        case 256:
            tex->sst.lod = GR_LOD_LOG2_256;
            break;
        case 128:
            tex->sst.lod = GR_LOD_LOG2_128;
            break;
        case 64:
            tex->sst.lod = GR_LOD_LOG2_64;
            break;
        case 32:
            tex->sst.lod = GR_LOD_LOG2_32;
            break;
        case 16:
            tex->sst.lod = GR_LOD_LOG2_16;
            break;
        case 8:
            tex->sst.lod = GR_LOD_LOG2_8;
            break;
        case 4:
            tex->sst.lod = GR_LOD_LOG2_4;
            break;
        case 2:
            tex->sst.lod = GR_LOD_LOG2_2;
            break;
        case 1:
            tex->sst.lod = GR_LOD_LOG2_1;
            break;
        }

        aspect = (8 * width) / height;
        switch (aspect) {
        case 64:
            tex->sst.aspect = GR_ASPECT_LOG2_8x1;
            tex->sst.sScale = 256.0f;
            tex->sst.tScale =  32.0f;
            break;
        case 32:
            tex->sst.aspect = GR_ASPECT_LOG2_4x1;
            tex->sst.sScale = 256.0f;
            tex->sst.tScale =  64.0f;
            break;
        case 16:
            tex->sst.aspect = GR_ASPECT_LOG2_2x1;
            tex->sst.sScale = 256.0f;
            tex->sst.tScale = 128.0f;
            break;
        case 8:
            tex->sst.aspect = GR_ASPECT_LOG2_1x1;
            tex->sst.sScale = 256.0f;
            tex->sst.tScale = 256.0f;
            break;
        case 4:
            tex->sst.aspect = GR_ASPECT_LOG2_1x2;
            tex->sst.sScale = 128.0f;
            tex->sst.tScale = 256.0f;
            break;
        case 2:
            tex->sst.aspect = GR_ASPECT_LOG2_1x4;
            tex->sst.sScale =  64.0f;
            tex->sst.tScale = 256.0f;
            break;
        case 1:
            tex->sst.aspect = GR_ASPECT_LOG2_1x8;
            tex->sst.sScale =  32.0f;
            tex->sst.tScale = 256.0f;
            break;
        default:
            return; /* XXXsimon never happens */
            break;
        }
        /* XXXshui This stomps on the values set earlier by CreateLevel; 
           should perhaps make CreateLevel machine dependent, and set it there.
           To make the software path work, will need a validator routine that
           takes user state and rederives machine state such as this. Or we could
           make CalcTexture be machine dependent and use a different piece of state. */
        tex->level[0].width2f = tex->sst.sScale;
        tex->level[0].height2f = tex->sst.tScale;

        switch (tex->level[0].internalFormat) {
        case __GL_SST_RGB_565:
            tex->sst.grformat = GR_TEXFMT_RGB_565;
            break;
        case __GL_SST_ARGB_4444:
            tex->sst.grformat = GR_TEXFMT_ARGB_4444;
            break;
        case __GL_SST_RGB_332:
            tex->sst.grformat = GR_TEXFMT_RGB_332;
            break;
        case __GL_SST_I_8:
            tex->sst.grformat = GR_TEXFMT_INTENSITY_8;
            break;
        case __GL_SST_AI_88:
            tex->sst.grformat = GR_TEXFMT_ALPHA_INTENSITY_88;
            break;
        case __GL_SST_A_8:
            tex->sst.grformat = GR_TEXFMT_ALPHA_8;
            break;
        case __GL_SST_P_8:
                tex->sst.grformat = GR_TEXFMT_P_8;
                break;
        }
        
        nlod = 0;
        w = width;
        h = height;
        while (w || h) {
            tex->sst.allocWidth[nlod] = w ? w : 1;
            tex->sst.allocHeight[nlod] = h ? h : 1;
            tex->sst.resident[nlod] = GL_FALSE;
            w >>= 1;
            h >>= 1;
            nlod++;
        }
        for (n=nlod; n < __GL_SST_MAX_LOD; n++) {
            tex->sst.allocWidth[n] = 0;
            tex->sst.allocHeight[n] = 0;
            tex->sst.resident[n] = 0;
        }
        tex->sst.numLevels = nlod;
        tex->sst.needMask = (1L << nlod) - 1;
    }

    if (tex->params.minFilter != GL_NEAREST && tex->params.minFilter != GL_LINEAR) {
        if (lod == 0) {     /* XXXshui have to disable texture if width/height is zero */
            grTex.format      = tex->sst.grformat;
            grTex.smallLodLog2    = GR_LOD_LOG2_1;
            grTex.largeLodLog2    = tex->sst.lod;
            grTex.aspectRatioLog2 = tex->sst.aspect;
            grTex.data        = tex->level[0].buffer;
            memsize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &grTex);

            /* allocate the full stack */
            if (tex->sst.allocation != __GL_SST_TEXALLOC_STACK ||
                tex->sst.texUnit[txu].allocSize != memsize) {
                /* XXX taco hack - push texture object out of other texture cache*/
                if ( gc->grNTexelFx == 2 ) {
                    int ntxu = !txu;
                    if ( tex->sst.texUnit[ntxu].cache ) {
                        freenode( gc, tex->sst.texUnit[ntxu].cache );
                        tex->sst.texUnit[ntxu].cache = NULL;
                    }
                }
                __glSSTAllocateTextureMemory(gc,tex, memsize);
                tex->sst.allocation = __GL_SST_TEXALLOC_STACK;
                tex->sst.texUnit[txu].allocSize = memsize;
            }

            /* load all suitable levels */
            tex->sst.cacheMask = 0;
            for (n=0; n < tex->sst.numLevels; n++) {
                if ((tex->level[n].width == tex->sst.allocWidth[n]) &&
                    (tex->level[n].height == tex->sst.allocHeight[n]) &&
                    (tex->level[n].requestedFormat == tex->level[0].requestedFormat)) {
                    tex->sst.cacheMask |= (1L << n);
                    /* one-shot: dump the 565 shadow words we hand the HW for
                    ** large base levels (CS green-world: is our data green?) */
                    { static int dl = 0;
                      if (n == 0 && dl < 30 && tex->sst.grformat == GR_TEXFMT_RGB_565
                          && tex->level[0].width >= 64) {
                          unsigned short *sw = (unsigned short *)tex->level[0].buffer;
                          dl++;
                          OGLLOGV("SHADOW565 %dx%d asp=%d 565[0..5]=%04x %04x %04x %04x %04x %04x",
                                  (int)tex->level[0].width, (int)tex->level[0].height,
                                  (int)tex->sst.aspect,
                                  sw?sw[0]:0, sw?sw[1]:0, sw?sw[2]:0,
                                  sw?sw[3]:0, sw?sw[4]:0, sw?sw[5]:0);
                      } }
                    grTexDownloadMipMapLevel(gc->texture.sst.currentTMU,
                                             (FxU32)tex->sst.texUnit[txu].cache->addr,
                                             tex->sst.lod - n, tex->sst.lod,
                                             tex->sst.aspect,
                                             grTex.format,
                                             GR_MIPMAPLEVELMASK_BOTH,
                                             tex->level[n].buffer);
                }
            }
            if (tex->sst.cacheMask == tex->sst.needMask) {
                grTexSource(gc->texture.sst.currentTMU,
                            (FxU32)tex->sst.texUnit[txu].cache->addr,
                            GR_MIPMAPLEVELMASK_BOTH,
                            &grTex);
                __glSSTApplyGrFilter(gc->texture.sst.currentTMU, tex);
                gc->validateTexture = 1;
            } else {
                gc->validateTexture = 1;
            }

        } else { /* not base level */

            if (tex->sst.allocation == __GL_SST_TEXALLOC_STACK) {
                if (tex->level[lod].width == tex->sst.allocWidth[lod] &&
                    tex->level[lod].height == tex->sst.allocHeight[lod]) {
                    tex->sst.cacheMask |= (1L << lod);
                    grTexDownloadMipMapLevel(gc->texture.sst.currentTMU, 
                                             (FxU32)tex->sst.texUnit[txu].cache->addr,
                                             tex->sst.lod - lod, tex->sst.lod, 
                                             tex->sst.aspect, 
                                             tex->sst.grformat, GR_MIPMAPLEVELMASK_BOTH, 
                                             tex->level[lod].buffer);
                    if (tex->sst.cacheMask == tex->sst.needMask) {
                        grTex.format      = tex->sst.grformat;
                        grTex.smallLodLog2    = GR_LOD_LOG2_1;
                        grTex.largeLodLog2    = tex->sst.lod;
                        grTex.aspectRatioLog2 = tex->sst.aspect;
                        grTex.data        = tex->level[0].buffer;
                        grTexSource(gc->texture.sst.currentTMU,
                                    (FxU32)tex->sst.texUnit[txu].cache->addr,
                                    GR_MIPMAPLEVELMASK_BOTH,
                                    &grTex);
                __glSSTApplyGrFilter(gc->texture.sst.currentTMU, tex);
                        gc->validateTexture = 1;
                    }
                } else {
                    /* inconsistent level causes deallocation */
                    __glSSTFreeTextureMemory(gc,tex);
                    tex->sst.allocation = __GL_SST_TEXALLOC_NONE;
                    tex->sst.texUnit[txu].allocSize = 0;
                    tex->sst.cacheMask = 0;
                    __glSSTDisableTexturing(gc);
                }
            } else {
                /* do nothing; nothing needs to be cached */
            }
        }
    } else {
        /* nonmipmap filter */
        if (lod == 0) {
            /* XXXshui have to disable texture if width/height is zero */
            /* RETRO3DFX MINIFICATION FIX (primary upload path): synthesize a
            ** box-filtered mip chain for GL_LINEAR 2D textures + LOD-dither
            ** blend -> minified soft-alpha fonts stop aliasing (text-garble). */
            static FxU16 __r3dMipBuf2[256*256 + 128*128 + 64*64 + 8192];
            int __r3dMipW2 = 0;
            if (__r3d_wantFontMip() && tex->params.minFilter == GL_LINEAR &&
                tex->level[0].width >= 4 && tex->level[0].height >= 4) {
                __r3dMipW2 = __r3d_buildMipChain(tex, __r3dMipBuf2,
                                 sizeof(__r3dMipBuf2)/sizeof(FxU16));
            }

            grTex.format      = tex->sst.grformat;
            if (__r3dMipW2) {
                grTex.smallLodLog2 = GR_LOD_LOG2_1;
                grTex.largeLodLog2 = tex->sst.lod;
                grTex.data         = __r3dMipBuf2;
            } else {
            grTex.smallLodLog2    = tex->sst.lod;
            grTex.largeLodLog2    = tex->sst.lod;
            grTex.data        = tex->level[0].buffer;
            }
            grTex.aspectRatioLog2 = tex->sst.aspect;
            memsize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &grTex);
            if (tex->sst.allocation != __GL_SST_TEXALLOC_BASE ||
                tex->sst.texUnit[txu].allocSize != memsize) {
                /* XXX taco hack - push texture object out of other texture cache*/
                if ( gc->grNTexelFx == 2 ) {
                    int ntxu = !txu;
                    if ( tex->sst.texUnit[ntxu].cache ) {
                        freenode( gc, tex->sst.texUnit[ntxu].cache );
                        tex->sst.texUnit[ntxu].cache = NULL;
                    }
                }
                /* allocate level zero */
                __glSSTAllocateTextureMemory(gc,tex, memsize);
                tex->sst.allocation = __GL_SST_TEXALLOC_BASE;
                tex->sst.texUnit[txu].allocSize = memsize;
            }
            if (__r3dMipW2) {
                OGLLOG("TEXIMG2D-MIP %dx%d minF=0x%x words=%d",
                       (int)tex->level[0].width,(int)tex->level[0].height,
                       (unsigned)tex->params.minFilter,__r3dMipW2);
                grTexDownloadMipMap(gc->texture.sst.currentTMU,
                                    (FxU32)tex->sst.texUnit[txu].cache->addr,
                                    GR_MIPMAPLEVELMASK_BOTH, &grTex);
            } else {
            grTexDownloadMipMapLevel(gc->texture.sst.currentTMU,
                                     (FxU32)tex->sst.texUnit[txu].cache->addr,
                                     tex->sst.lod, tex->sst.lod,
                                     tex->sst.aspect,
                                     tex->sst.grformat,
                                     GR_MIPMAPLEVELMASK_BOTH,
                                     tex->level[0].buffer);
            }
            grTexSource(gc->texture.sst.currentTMU,
                        (FxU32)tex->sst.texUnit[txu].cache->addr,
                        GR_MIPMAPLEVELMASK_BOTH,
                        &grTex);
                __glSSTApplyGrFilter(gc->texture.sst.currentTMU, tex);
            if (__r3dMipW2)
                grTexMipMapMode(gc->texture.sst.currentTMU,
                                GR_MIPMAP_NEAREST_DITHER, FXFALSE);
            gc->validateTexture = 1;
        } else {
            /* do nothing; nothing needs to be cached */
        }
    }
    /* Mark the bound texture object as resident. */
    pto->resident = GL_TRUE;

doneTexture2D:

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_TexImage2D(GLenum target, GLint lod, GLint components,
                       GLsizei width, GLsizei height, GLint border, GLenum format,
                       GLenum type, const GLvoid *buf)
{
    __glSSTTexImage2D(target, lod, components, width, height, border,
                      format, type, buf);
}

void __glsstlei_TexImage2D(__GLcontext *gc, GLenum target, GLint lod, 
                        GLint components, GLsizei w, GLsizei h, 
                        GLint border, GLenum format, GLenum type,
                        const GLubyte *image)
{
    __glSSTTexImage2D(target, lod, components, w, h, border,
                      format, type, image);
}

/************************************************************************/

void APIENTRY __glsstim_TexSubImage1D(GLenum target, GLint lod, 
                       GLint xoffset, GLint length,
                       GLenum format, GLenum type, const GLvoid *buf)
{
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckTexSubImage1DArgs(gc, target, lod, xoffset, length,
                                     format, type);
    if (!tex) {
        return;
    }

    /* Copy sub-image data */
    __glInitTexSourceUnpack(gc, &spanInfo, length, 1, 1,
                            format, type, buf, GL_FALSE);
    __glInitTexSubImageStore(gc, &spanInfo, tex, lod, xoffset, 0, 0);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*tex->copyTexImage)(gc, &spanInfo, tex, lod);
}

void __glsstlei_TexSubImage1D(__GLcontext *gc, GLenum target, GLint lod,
                        GLint xoffset, GLint length,
                        GLenum format, GLenum type, const GLubyte *image)
{
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
        if (beginMode == __GL_NEED_VALIDATE) {
            (*gc->procs.validate)(gc);
            gc->beginMode = __GL_NOT_IN_BEGIN;
        } else {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckTexSubImage1DArgs(gc, target, lod, xoffset, length,
                                     format, type);
    if (!tex) {
        return;
    }

    /* Copy sub-image data */
    __glInitTexSourceUnpack(gc, &spanInfo, length, 1, 1,
                            format, type, image, GL_TRUE);
    __glInitTexSubImageStore(gc, &spanInfo, tex, lod, xoffset, 0, 0);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*tex->copyTexImage)(gc, &spanInfo, tex, lod);
}

static void
memCopy(char *dst, const char *src, int size)
{
    /* unroll loop to maximize throughput */
    while (size > 15) {
        register long p0, p1, p2, p3;

        p0 = *(long *) (src  );
        p1 = *(long *) (src+ 4);
        p2 = *(long *) (src+ 8);
        p3 = *(long *) (src+12);
        src += 16;
        *(long *) (dst   ) = p0;
        *(long *) (dst+ 4) = p1;
        *(long *) (dst+ 8) = p2;
        *(long *) (dst+12) = p3;
        dst += 16;
        size -= 16;
    }

    while (size--) *dst++ = *src++;
}

void __glSSTShadowTexSubImage(__GLcontext *gc, __GLtexture *tex, GLint lod, GLint xoffset, GLint yoffset,
                              GLint w, GLint h, GLenum format, GLenum type,
                              const GLvoid *buf)
{
    __GLmipMapLevel *lp = &tex->level[lod];
    GLushort *sp, *srow;
    GLubyte *bp, *src, *old_src;
    int i, j;
    int element_size;
    int skipLines, skipPixels, rowLength;

    switch (format) {
    case GL_LUMINANCE:
        element_size = 1;
        break;
    case GL_LUMINANCE_ALPHA:
        element_size = 2;
        break;
    case GL_RGB:
        element_size = 3;
        break;
    case GL_RGBA:
        element_size = 4;
        break;
    default:
        element_size = 4;   /* defensive: never leave it uninitialized */
        break;
    }

    skipPixels = gc->state.pixel.unpackModes.skipPixels * element_size;
    skipLines = gc->state.pixel.unpackModes.skipLines;
    rowLength = gc->state.pixel.unpackModes.lineLength ? gc->state.pixel.unpackModes.lineLength : w;
    rowLength *= element_size;
    src = (GLubyte *) buf + skipLines * rowLength + skipPixels;
    switch (lp->internalFormat) {
    case __GL_SST_I_8: /* GL_LUMINANCE, GL_UNSIGNED_BYTE */
        bp = (GLubyte *) lp->buffer + (lp->width * yoffset) + xoffset;
        for (i=0; i < h; i++) {
            memCopy(bp, src, w);
            bp += lp->width;
            src += rowLength;
        }
        break;
    case __GL_SST_RGB_565: /* GL_RGB or GL_RGBA, GL_UNSIGNED_BYTE */
        srow = (GLushort *) lp->buffer + (lp->width * yoffset + xoffset);
        for (i=0; i < h; i++) {
            old_src = src;
            sp = srow;
            for (j=0; j < w; j++) {
                *sp = ((src[0] & 0xF8) << 8) | ((src[1] & 0xFC) << 3) |
                    ((src[2] & 0xF8) >> 3);
                sp++;
                src += element_size;
            }
            src = old_src + rowLength;
            srow += lp->width;
        } 
        break;
    case __GL_SST_ARGB_4444:
        srow = (GLushort *) lp->buffer + (lp->width * yoffset + xoffset);
        for (i=0; i < h; i++) {
            old_src = src;
            sp = srow;
            for (j=0; j < w; j++) {
                *sp = ((src[0] & 0xF0) << 4) | ((src[1] & 0xF0) << 0) |
                    ((src[2] & 0xF0) >> 4) | ((src[3] & 0xF0) << 8);
                sp++;
                src += element_size;
            }
            src = old_src + rowLength;
            srow += lp->width;
        } 
        break;
    }
}

void APIENTRY __glsstim_TexSubImage2D(GLenum target, GLint lod,
                       GLint xoffset, GLint yoffset,
                       GLsizei w, GLsizei h, GLenum format,
                       GLenum type, const GLvoid *buf)
{
    __GLtexture *tex;

    /* XXXshui not true anymore */
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    /* one-shot diagnostics (GoldSrc color hunt): lightmap updates come
    ** through here. */
    { static int logged = 0;
      if (logged < 16) {
          const unsigned char *p = (const unsigned char *)buf;
          logged++;
          OGLLOG("SSTTexSubImage2D: %d,%d %dx%d fmt=0x%x type=0x%x bytes=%02x %02x %02x %02x %02x %02x %02x %02x",
                 (int)xoffset, (int)yoffset, (int)w, (int)h,
                 (unsigned)format, (unsigned)type,
                 p?p[0]:0, p?p[1]:0, p?p[2]:0, p?p[3]:0,
                 p?p[4]:0, p?p[5]:0, p?p[6]:0, p?p[7]:0);
      } }

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckTexSubImage2DArgs(gc, target, lod, xoffset, yoffset, w, h,
                                     format, type);
    if (!tex) {
        return;
    }

    /* Always update the software shadow copy of the texture first. */
    __glSSTShadowTexSubImage(gc, tex, lod, xoffset, yoffset, w, h, format, type, buf);

    /* SUB-IMAGE CRASH FIX (UT / glTexSubImage2D GPF): only push a *partial*
     * download to the hardware texture cache when this texture actually HAS a
     * resident cache entry. When the texture is not resident (cache == NULL),
     * the original code dereferenced cache->addr -> NULL pointer GPF (this is
     * exactly UT's create-empty-then-glTexSubImage2D pattern for lightmaps).
     * Skipping the partial download is safe and correct: the shadow buffer now
     * holds the updated texels, and the whole level is downloaded when the
     * texture is next made resident at bind/validate time. */
    if (tex->sst.texUnit[gc->texture.currentTexUnit].cache) {
        grTexDownloadMipMapLevelPartial(gc->texture.sst.currentTMU,
                                    (FxU32) tex->sst.texUnit[gc->texture.currentTexUnit].cache->addr,
                                    tex->sst.lod - lod,
                                    tex->sst.lod,
                                    tex->sst.aspect,
                                    tex->sst.grformat,
                                    GR_MIPMAPLEVELMASK_BOTH,
                                    tex->level[lod].buffer + tex->level[lod].width * yoffset * tex->level[lod].texelSize,
                                    yoffset,
                                    yoffset + h - 1);
    }
}

void __glsstlei_TexSubImage2D(__GLcontext *gc, GLenum target, GLint lod, 
                        GLint xoffset, GLint yoffset,
                        GLsizei w, GLsizei h, GLenum format, GLenum type,
                        const GLubyte *image)
{
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    beginMode = gc->beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
        if (beginMode == __GL_NEED_VALIDATE) {
            (*gc->procs.validate)(gc);
            gc->beginMode = __GL_NOT_IN_BEGIN;
        } else {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
    }

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckTexSubImage2DArgs(gc, target, lod, xoffset, yoffset, w, h,
                                     format, type);
    if (!tex) {
        return;
    }

    /* Copy sub-image data */
    __glInitTexSourceUnpack(gc, &spanInfo, w, h, 1,
                            format, type, image, GL_TRUE);
    __glInitTexSubImageStore(gc, &spanInfo, tex, lod, xoffset, yoffset, 0);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*tex->copyTexImage)(gc, &spanInfo, tex, lod);
}

/************************************************************************/
/*
** Texture Object extension routines.
*/
/************************************************************************/

#define __GL_CHECK_VALID_N_PARAM(failStatement)                         \
    if (n < 0) {                                                        \
        __glSetError(GL_INVALID_VALUE);                                 \
    }                                                                   \
    if (n == 0) {                                                       \
        failStatement;                                                  \
    }                                                                   \

/*
 * The null texture object free function.
 */
/*ARGSUSED*/
GLvoid __glSSTEmptyFreeTexObj(__GLcontext *gc, __GLtextureObject *texobj)
{
}

/*
 * Our implementation-specific texture object free function.
 */
GLvoid __glSSTFreeTexObj(__GLcontext *gc, __GLtextureObject *texobj)
{
    GLint level, maxLevel;
    GLuint txu;

    txu = gc->texture.currentTexUnit;

    maxLevel = gc->constants.maxMipMapLevel;
    for (level = 0; level < maxLevel; level++) {
        if (NULL == texobj->texture.map.level[level].buffer) continue;
        assert(texobj->texture.map.texobjs.name != 0);
        (*gc->imports.free)(gc, texobj->texture.map.level[level].buffer);
    }
    (*gc->imports.free)(gc, texobj->texture.map.level);
    /* Clear pointer from texture cache node to this texture */
    if (texobj->texture.map.sst.texUnit[txu].cache) {
        texobj->texture.map.sst.texUnit[txu].cache->tex = NULL;
    }
    (*gc->imports.free)(gc, texobj);
}

/*
 * Generic texture object free function; called from so_names.c
 */
GLvoid __glSSTDisposeTexObj(__GLcontext *gc, void *pData)
{
    __GLtextureObject *texobj = (__GLtextureObject *)pData;

    texobj->refcount--;
    assert(texobj->refcount >= 0);

    if (texobj->refcount == 0) {
        texobj->free(gc, texobj);
    }
}

GLvoid APIENTRY __glsstim_GenTextures(GLsizei n, GLuint* textures)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);
    __GL_API_BLAND();

    if (NULL == textures) return;

    assert(NULL != gc->texture.namesArray);

    __glNamesGenNames(gc, gc->texture.namesArray, n, textures);

}

GLvoid APIENTRY __glsstim_DeleteTextures(GLsizei n, const GLuint* textures)
{
    GLuint start, rangeVal;
    GLint numTextures, targetIndex, i;
    __GLnamesArray *array;
    __GLtextureObject *texobj, **pBoundTexture;

    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);
    __GL_API_STATE();

    array = gc->texture.namesArray;
    numTextures = gc->constants.numberOfTextures;

    /*
    ** Send the texture names in ranges to the names module to be
    ** deleted.  Ignore any references to default textures.
    ** If a texture that is being deleted is currently bound,
    ** bind the default texture to its target.
    ** The names routine ignores any names that don't refer to
    ** textures.
    */
    start = rangeVal = textures[0];
    for (i=0; i < n; i++, rangeVal++) {
        if (0 == textures[i]) {         /* skip default textures */
            /* delete up to this one */
            __glNamesDeleteRange(gc,array,start,rangeVal-start);
            /* skip over this one by setting start to the next one */
            start = textures[i+1];
            rangeVal = start-1;         /* because it gets incremented later */
            continue;
        }
        /*
        ** If the texture is currently bound, bind the defaultTexture
        ** to its target.  The problem here is identifying the target.
        ** One way is to look up the texobj with the name.  Another is
        ** to look through all of the currently bound textures and
        ** check each for the name.  It has been implemented with the
        ** assumption that looking through the currently bound textures
        ** is faster than retrieving the texobj that corresponds to
        ** the name.
        */
        for (targetIndex=0, pBoundTexture = gc->texture.boundTextures[gc->texture.currentTexUnit]; 
                targetIndex < numTextures; targetIndex++, pBoundTexture++) {

            /* Is the texture currently bound? */
            if ((*pBoundTexture)->texture.map.texobjs.name == textures[i]) {
                __GLperTextureState *pts;
                pts = &gc->state.texture[gc->texture.currentTexUnit].texture[targetIndex];
                /* if we don't unlock it, it won't get deleted */
                __glNamesUnlockData(gc, *pBoundTexture);

                /* bind the default texture to this target */
                texobj = gc->texture.defaultTextures[gc->texture.currentTexUnit] + targetIndex;
                assert(texobj->texture.map.texobjs.name == 0);
                gc->texture.texture[gc->texture.currentTexUnit][targetIndex] = &(texobj->texture);          
                *pBoundTexture = texobj;
                pts->texobjs = texobj->texture.map.texobjs;
                pts->params = texobj->texture.map.params;

                /* Need to reset the current texture and such. */
                __GL_DELAY_VALIDATE(gc);
                break;
            }
        }
        if (textures[i] != rangeVal) {
            /* delete up to this one */
            __glNamesDeleteRange(gc,array,start,rangeVal-start);
            start = rangeVal = textures[i];
        }
    }
    __glNamesDeleteRange(gc,array,start,rangeVal-start);
}

/*
** This routine is used by the pick routines to actually perform
** the bind.  
*/
void __glSSTBindTexture(__GLcontext *gc, GLuint targetIndex, GLuint texture, 
                        GLboolean freeingContext)
{
    __GLtextureObject *texobj;
    __GLtexture *tex;
    GrTexInfo grTex;
    GLint n;
    GLuint txu, tmu, tmp;

    assert(NULL != gc->texture.namesArray);

    txu = gc->texture.currentTexUnit;
    tmu = gc->texture.sst.currentTMU;

    /*
    ** Retrieve the texture object from the namesArray structure.
    */
    if (texture == 0) {
        texobj = gc->texture.defaultTextures[txu] + targetIndex;
        assert(NULL != texobj);
        assert(texobj->texture.map.texobjs.name == 0);
    }
    else {
        texobj = (__GLtextureObject *)
                __glNamesLockData(gc, gc->texture.namesArray, texture);
    }

    /*
    ** Is this the first time this name has been bound?
    ** If so, create a new texture object and initialize it.
    */
    if (NULL == texobj) {
        texobj = (__GLtextureObject *)
                        (*gc->imports.calloc)(gc, 1, sizeof(*texobj));
        assert(NULL != texobj);
        (*gc->texture.initTextureObject)(gc, texobj, texture, targetIndex);
        InitTextureMachine(gc, targetIndex, &(texobj->texture));
        __glNamesNewData(gc, gc->texture.namesArray, texture, texobj);
        /*
        ** Shortcut way to lock without doing another lookup.
        */
        texobj->refcount++;
    }
    else {
        /*
        ** Retrieved an existing texture object.  Do some
        ** sanity checks.
        */
        if (texobj->targetIndex != targetIndex) {
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        assert(texture == texobj->texture.map.texobjs.name);
    }

    {
        __GLperTextureState *pts;
        __GLtexture *ptm;
        __GLtextureObject *boundTexture;

        pts = &(gc->state.texture[txu].texture[targetIndex]);
        ptm = &(gc->texture.texture[txu][targetIndex]->map);
        boundTexture = gc->texture.boundTextures[txu][targetIndex];

#       if 0
        /* tacohack - this breaks MT in subtle ways */
        /* Copy the current stackable state into the bound texture. */
        ptm->params = pts->params;
        ptm->texobjs = pts->texobjs;
#endif

        if (boundTexture->texture.map.texobjs.name != 0) {
            /* Unlock the texture that is being unbound.  */
            __glNamesUnlockData(gc, boundTexture);
        }

        /*
        ** Install the new texture into the correct target and save
        ** its pointer so it can be unlocked easily when it is unbound.
        */
        gc->texture.texture[txu][targetIndex] = &(texobj->texture);          
        gc->texture.boundTextures[txu][targetIndex] = texobj;

        /* Copy the new texture's stackable state into the context state. */
        pts->params = texobj->texture.map.params;
        pts->texobjs = texobj->texture.map.texobjs;

        /* If in context destruction, we don't want/need to call glide below. */
        if (freeingContext) return;

        tmp = ( ( texobj->texture.map.sst.clamps << 8 ) | ( texobj->texture.map.sst.clampt ) );
        if ( gc->texture.hwSTWrap[txu] != tmp ) {
            grTexClampMode(tmu, texobj->texture.map.sst.clamps, texobj->texture.map.sst.clampt);
            gc->texture.hwSTWrap[txu] = tmp;
        }
        tmp = ( ( texobj->texture.map.sst.min << 8 ) | ( texobj->texture.map.sst.mag ) );
        if ( gc->texture.hwMinMag[txu] != tmp ) {
            OGLLOGV( "FILT@3767 tmu/min/mag= %d %d %d", (int)(tmu), (int)(texobj->texture.map.sst.min), (int)(texobj->texture.map.sst.mag) );
            grTexFilterMode(tmu, texobj->texture.map.sst.min, texobj->texture.map.sst.mag); 
            gc->texture.hwMinMag[txu] = tmp;
        }
        tmp = texobj->texture.map.sst.mip;
        if ( gc->texture.hwMMMode[txu] != tmp ) {
            grTexMipMapMode(tmu, texobj->texture.map.sst.mip, FXFALSE);
            gc->texture.hwMMMode[txu] = tmp;
        }

        /* XXXwheeler: punt on non-2D textures for now */
        if (targetIndex != __GL_TEXTURE_INDEX_2D) {
            gc->validateTexture = 1;
            return;
        }

        /* XXXshui not necessarily 2D is active */
        tex = &gc->texture.texture[txu][__GL_TEXTURE_INDEX_2D]->map;

        grTex.format      = tex->sst.grformat;
        grTex.smallLodLog2    = GR_LOD_LOG2_1;
        grTex.largeLodLog2    = tex->sst.lod;
        grTex.aspectRatioLog2 = tex->sst.aspect;
        grTex.data        = tex->level[0].buffer;
        if (tex->params.minFilter != GL_NEAREST && tex->params.minFilter != GL_LINEAR) {
            /* restore the cache state when this texture was last bound */
            if (tex->sst.allocation == __GL_SST_TEXALLOC_STACK) {
                if (!tex->sst.texUnit[txu].cache) {
                    if ( tex->sst.texUnit[txu].allocSize == 0 ) {
                        int memsize;
                        memsize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &grTex);
                        tex->sst.texUnit[txu].allocSize = memsize;
                    }
                    __glSSTAllocateTextureMemory(gc,tex, tex->sst.texUnit[txu].allocSize);
                    /* reload all cached levels */
                    for (n=0; n < tex->sst.numLevels; n++) {
                        if (tex->sst.cacheMask & (1L << n)) {
                            grTexDownloadMipMapLevel(tmu, 
                                                     (FxU32)tex->sst.texUnit[txu].cache->addr,
                                                     tex->sst.lod - n, 
                                                     tex->sst.lod, 
                                                     tex->sst.aspect, 
                                                     tex->sst.grformat, 
                                                     GR_MIPMAPLEVELMASK_BOTH, 
                                                     tex->level[n].buffer);
                        }
                    }
                }
                if (tex->sst.cacheMask == tex->sst.needMask) {
                    grTex.smallLodLog2 = GR_LOD_LOG2_1;
                    grTexSource(tmu,
                                (FxU32)tex->sst.texUnit[txu].cache->addr,
                                GR_MIPMAPLEVELMASK_BOTH,
                                &grTex);
                __glSSTApplyGrFilter(tmu, tex);
                    gc->validateTexture = 1;
                } else {
                    gc->validateTexture = 1;
                }
            } else {
                gc->validateTexture = 1;
            }
        } else {
            if (tex->sst.allocation == __GL_SST_TEXALLOC_BASE) {
                grTex.smallLodLog2 = tex->sst.lod;
                if (!tex->sst.texUnit[txu].cache) {
                    if ( tex->sst.texUnit[txu].allocSize == 0 ) {
                        int memsize;
                        memsize = grTexTextureMemRequired(GR_MIPMAPLEVELMASK_BOTH, &grTex);
                        tex->sst.texUnit[txu].allocSize = memsize;
                    }

                    __glSSTAllocateTextureMemory(gc, tex, tex->sst.texUnit[txu].allocSize);
                    grTexDownloadMipMapLevel(tmu, 
                                             (FxU32)tex->sst.texUnit[txu].cache->addr,
                                             tex->sst.lod, 
                                             tex->sst.lod, 
                                             tex->sst.aspect,
                                             tex->sst.grformat, 
                                             GR_MIPMAPLEVELMASK_BOTH, 
                                             tex->level[0].buffer);
                }
                grTexSource(tmu,
                            (FxU32)tex->sst.texUnit[txu].cache->addr,
                            GR_MIPMAPLEVELMASK_BOTH,
                            &grTex);
                __glSSTApplyGrFilter(tmu, tex);
                gc->validateTexture = 1;
            } else {
                gc->validateTexture = 1;
            }
        }
    }
}

GLvoid APIENTRY __glsstim_BindTexture(GLenum target, GLuint texture)
{
    GLuint targetIndex;
    /*
    ** Need to validate in case a new texture was popped into
    ** the state immediately prior to this call.
    */
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch (target) {
    case GL_TEXTURE_1D:
        targetIndex = __GL_TEXTURE_INDEX_1D;
        break;
    case GL_TEXTURE_2D:
        targetIndex = __GL_TEXTURE_INDEX_2D;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    __glSSTBindTexture(gc, targetIndex, texture, GL_FALSE);

    /* Need to reset the current texture and such. */
    /* XXXshui much too much work is done here than we need */
    __GL_DELAY_VALIDATE(gc);
}


GLvoid APIENTRY __glsstim_PrioritizeTextures(GLsizei n,
                           const GLuint* textures,
                           const GLclampf* priorities)
{
    int i;
    __GLtextureObject *texobj;

    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);
    __GL_API_STATE();

    for (i=0; i < n; i++) {
        /* silently ignore default texture */
        if (0 == textures[i]) continue;

        texobj = (__GLtextureObject *)
            __glNamesLockData(gc, gc->texture.namesArray, textures[i]);

        /* silently ignore non-texture */
        if (NULL == texobj) continue;

        texobj->texture.map.texobjs.priority = 
                        Clampf(priorities[i], __glZero, __glOne);

        /* Is it currently bound? */
        if (texobj->texture.map.texobjs.name == 
            gc->texture.texture[gc->texture.currentTexUnit][texobj->targetIndex]->map.texobjs.name) {
            /* Make sure the priority is set in the current texture. */
            gc->state.texture[gc->texture.currentTexUnit].texture[texobj->targetIndex].texobjs.priority =
            gc->texture.texture[gc->texture.currentTexUnit][texobj->targetIndex]->map.texobjs.priority = 
                texobj->texture.map.texobjs.priority;
        }
        __glNamesUnlockData(gc, texobj);
    }
}


static
GLboolean IsTextureResident(__GLcontext *gc, __GLtexture *tex)
{
    return (tex->sst.texUnit[gc->texture.currentTexUnit].cache && tex->sst.texUnit[gc->texture.currentTexUnit].cache->addr);
#if 0    
    GLint i;
    int maxMipMapLevel = gc->constants.maxMipMapLevel;

    for (i=0; i < maxMipMapLevel; i++) {
        if (!tex->level[i].buffer) {
            return GL_FALSE;
        }
        /* If not-mipmapping, return after checking the first level. */
        switch (tex->params.minFilter) {
          case GL_NEAREST:
          case GL_LINEAR:
            return GL_TRUE;
          default:
            break;
        }
        if (tex->level[i].width == 1 && tex->level[i].height == 1) break;
    }

    return GL_TRUE;
#endif    
}

GLboolean APIENTRY __glsstim_AreTexturesResident(GLsizei n,
                                                 const GLuint* textures,
                                                 GLboolean* residences)
{
    int i;
    __GLtextureObject *texobj;
    GLboolean allResident = GL_TRUE;

    __GL_SETUP_NOT_IN_BEGIN2();
    __GL_CHECK_VALID_N_PARAM(return GL_FALSE);
    __GL_API_GET();

    for (i=0; i < n; i++) {
        /* Can't query a default texture. */
        if (0 == textures[i]) {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }
        texobj = (__GLtextureObject *)
            __glNamesLockData(gc, gc->texture.namesArray, textures[i]);
        /*
        ** Ensure that all of the names have corresponding textures.
        */
        if (NULL == texobj) {
            __glSetError(GL_INVALID_VALUE);
            return GL_FALSE;
        }
        if (!__glSSTIsTextureConsistent(gc, &texobj->texture.map) || 
            !IsTextureResident(gc, &texobj->texture.map)) {
            allResident = GL_FALSE;
            residences[i] = GL_FALSE;
        }
        else {
            residences[i] = GL_TRUE;
        }
        __glNamesUnlockData(gc, texobj);
    }

    return allResident;
}

GLboolean APIENTRY __glsstim_IsTexture(GLuint texture)
{
    __GLtextureObject *texobj;
    __GL_SETUP_NOT_IN_BEGIN2();
    __GL_API_GET();

    if (0 == texture) return GL_FALSE;

    texobj = (__GLtextureObject *)
            __glNamesLockData(gc, gc->texture.namesArray, texture);

    if (NULL == texobj) return GL_FALSE;

    __glNamesUnlockData(gc, texobj);

    return GL_TRUE;
}

/************************************************************************/
/*
** Copy Texture extension routines.
*/
/************************************************************************/

void APIENTRY __glsstim_CopyTexImage1D(GLenum target,
                           GLint level,
                           GLenum internalformat,
                           GLint x,
                           GLint y,
                           GLsizei width,
                           GLint border)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    __GLtextureObject *pto;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_PIXEL_OP();

    /* not implemented yet */
    return;

    /* Check arguments and get the right texture being changed */
    tex = __glCheckCopyTexImageArgs(gc, target, level, internalformat, 
                                        x, y, width, 1, border, 1);
    if (!tex) {
        return;
    }
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_1D, gc->texture.currentTexUnit );
#if 0
    if (pto->refcount > 2) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
#endif

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, level, internalformat,
                               width, 1+border*2, 1+border*2, border, 1);

    /* Copy image data */
    if (dest) {

        __glInitReadImageSrcInfo(gc, &spanInfo, x, y, width, 1);
        __glInitTexImageStore(gc, &spanInfo, tex, level);
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);

        if (!__glClipReadPixels(gc, &spanInfo)) return;
        (*tex->readTexImage)(gc, &spanInfo, tex, level);

        /* Mark the bound texture object as resident. */
        pto->resident = GL_TRUE;
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_CopyTexImage2D(GLenum target,
                               GLint level,
                               GLenum internalformat,
                               GLint x, GLint y,
                               GLsizei w, GLsizei h,
                               GLint border)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    __GLtextureObject *pto;

    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_PIXEL_OP();

    /* not implemented yet */
    return;

    /* Check arguments and get the right texture being changed */
    tex = __glCheckCopyTexImageArgs(gc, target, level, internalformat, 
                                        x, y, w, h, border, 2);
    if (!tex) {
        return;
    }
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_2D, gc->texture.currentTexUnit );
#if 0
    /* XXXtaco - this doesn't work with multitexture */
    if (pto->refcount > 2) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
#endif

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, level, internalformat,
                               w, h, 1+border*2, border, 2);

    /* Copy image data */
    if (dest) {

        __glInitReadImageSrcInfo(gc, &spanInfo, x, y, w, h);
        __glInitTexImageStore(gc, &spanInfo, tex, level);
        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);

        if (!__glClipReadPixels(gc, &spanInfo)) return;
        (*tex->readTexImage)(gc, &spanInfo, tex, level);

        /* Mark the bound texture object as resident. */
        pto->resident = GL_TRUE;
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glsstim_CopyTexSubImage1D(GLenum target,
                              GLint level,
                              GLint xoffset,
                              GLint x,
                              GLint y,
                              GLsizei width)
{
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;
    __GLtextureObject *pto;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_PIXEL_OP();

    /* not implemented yet */
    return;

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckCopyTexSubImageArgs(gc, target, level,
                                        xoffset, 0, 0, x, y, width, 1, 1);
    if (!tex) {
        return;
    }
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_1D, gc->texture.currentTexUnit );
#if 0
    if (pto->refcount > 2) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
#endif

    /* Copy sub-image data */
    __glInitReadImageSrcInfo(gc, &spanInfo, x, y, width, 1);
    __glInitTexSubImageStore(gc, &spanInfo, tex, level, xoffset, 0, 0);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);

    if (!__glClipReadPixels(gc, &spanInfo)) return;
    (*tex->readTexImage)(gc, &spanInfo, tex, level);
}

void APIENTRY __glsstim_CopyTexSubImage2D(GLenum target,
                              GLint level,
                              GLint xoffset,
                              GLint yoffset,
                              GLint x,
                              GLint y,
                              GLsizei width,
                              GLsizei height)
{
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;
    __GLtextureObject *pto;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_PIXEL_OP();

    /* not implemented yet */
    return;

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckCopyTexSubImageArgs(gc, target, level,
                                        xoffset, yoffset, 0,
                                        x, y, width, height, 2);
    if (!tex) {
        return;
    }
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_2D, gc->texture.currentTexUnit );
#if 0
    if (pto->refcount > 2) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }
#endif

    /* Copy sub-image data */
    __glInitReadImageSrcInfo(gc, &spanInfo, x, y, width, height);
    __glInitTexSubImageStore(gc, &spanInfo, tex, level, xoffset, yoffset, 0);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);

    if (!__glClipReadPixels(gc, &spanInfo)) return;
    (*tex->readTexImage)(gc, &spanInfo, tex, level);
}

/************************************************************************/

void APIENTRY __glsstim_GetTexImage(GLenum target, GLint level, GLenum format, GLenum type,
                                    GLvoid *texels)
{
    GLint width, height, depth;
    __GLtexture *tex;
    __GLmipMapLevel *lp;
#if 0
    __GLpixelSpanInfo spanInfo;
#endif
    GLubyte *dst, *bp;
    GLushort *sp;
    GLint i, j;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_GET();

    tex = __glLookUpTexture(gc, target, gc->texture.currentTexUnit );

    if (!tex || (target == GL_PROXY_TEXTURE_1D) ||
                (target == GL_PROXY_TEXTURE_2D))
    {
      bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if ((level < 0) || (level >= gc->constants.maxMipMapLevel)) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    switch (format) {
      case GL_RGBA:
      case GL_RGB:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
#if 0 /* XXXshui don't support these yet */
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_ABGR_EXT:
      case GL_BGR_EXT:
      case GL_BGRA_EXT:
#endif    
        break;
      default:
        goto bad_enum;
    }
    switch (type) {
      case GL_UNSIGNED_BYTE:
#if 0 /* XXXshui don't support these yet */       
      case GL_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
        break;
      case GL_UNSIGNED_BYTE_3_3_2_EXT:
        switch (format) {
          case GL_RGB:
          case GL_BGR_EXT:
            break;
          default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
        break;
      case GL_UNSIGNED_SHORT_4_4_4_4_EXT:
      case GL_UNSIGNED_SHORT_5_5_5_1_EXT:
      case GL_UNSIGNED_INT_8_8_8_8_EXT:
      case GL_UNSIGNED_INT_10_10_10_2_EXT:
        switch (format) {
          case GL_RGBA:
          case GL_ABGR_EXT:
          case GL_BGRA_EXT:
            break;
          default:
            __glSetError(GL_INVALID_OPERATION);
            return;
        }
#endif  
        break;
      default:
        goto bad_enum;
    }

    lp = &tex->level[level];

    width = lp->width;
    if (tex->dim < 2) {
        height = lp->height - lp->border*2;
    } else {
        height = lp->height;
    }

    /* If no texture has been loaded */
    if (!width || !height) return;

    depth = lp->depth - lp->border*2;

    /* XXXshui only support this for now */
    if (type != GL_UNSIGNED_BYTE) return;
    dst = (GLubyte *) texels;
    
    switch (lp->internalFormat) {
    case __GL_SST_RGB_332:
        if (format != GL_RGB) return;
        bp = (GLubyte *) lp->buffer;
        for (i=0; i < lp->height; i++) {
            for (j=0; j < lp->width; j++) {
                dst[0] = (*bp & 0xe0);
                dst[1] = (*bp & 0x1c) << 3;
                dst[2] = (*bp & 0x03) << 6;
                bp++;
                dst += 3;
            }
        }
        break;
    case __GL_SST_RGB_565:
        if (format != GL_RGB) return;
        sp = (GLushort *) lp->buffer;
        for (i=0; i < lp->height; i++) {
            for (j=0; j < lp->width; j++) {
                dst[0] = (*sp & 0xf100) >> 8;
                dst[1] = (*sp & 0x07e0) >> 3;
                dst[2] = (*sp & 0x001f) << 3;
                sp++;
                dst += 3;
            }
        } 
        break;
    case __GL_SST_ARGB_4444:
        if (format != GL_RGBA) return;
        sp = (GLushort *) lp->buffer;
        for (i=0; i < lp->height; i++) {
            for (j=0; j < lp->width; j++) {
                dst[3] = (*sp & 0xf000) >> 8;
                dst[0] = (*sp & 0x0f00) >> 4;
                dst[1] = (*sp & 0x00f0);
                dst[2] = (*sp & 0x000f) << 4;
                sp++;
                dst += 4;
            }
        } 
        break;
    case __GL_SST_AI_88:
        if (format != GL_LUMINANCE_ALPHA) return;
        sp = (GLushort *) lp->buffer;
        for (i=0; i < lp->height; i++) {
            for (j=0; j < lp->width; j++) {
                dst[0] = (*sp & 0x00ff);
                dst[1] = (*sp & 0xff00) >> 8;
                sp++;
                dst += 2; 
            }
        }
        break;
    case __GL_SST_I_8:
        if (format != GL_LUMINANCE) return;
        bp = (GLubyte *) lp->buffer;
        for (i=0; i < lp->height; i++) {
            for (j=0; j < lp->width; j++) {
                *dst++ = *bp++;
            }
        }
        break;
    case __GL_SST_A_8:
        if (format != GL_ALPHA) return;
        bp = (GLubyte *) lp->buffer;
        for (i=0; i < lp->height; i++) {
            for (j=0; j < lp->width; j++) {
                *dst++ = *bp++;
            }
        } 
        break;
    case __GL_SST_P_8:
        if (format != GL_COLOR_INDEX) return;
        bp = (GLubyte *) lp->buffer;
        for (i=0; i < lp->height; i++) {
            for (j=0; j < lp->width; j++) {
                *dst++ = *bp++;
            }
        } 
        break;
    }
#if 0    
    __glInitTexDestPack(gc, &spanInfo, width, height, depth,
                        format, type, texels);
    __glInitTexImageGet(gc, &spanInfo, tex, level);
    __glInitPacker(gc, &spanInfo);
    __glInitUnpacker(gc, &spanInfo);
    (*gc->procs.copyImage)(gc, &spanInfo, GL_TRUE);
#endif    
}


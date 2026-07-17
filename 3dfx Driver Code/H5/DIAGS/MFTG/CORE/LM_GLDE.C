/*
 * lm_glde.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <env.h>

#include "lua.h"
#include "luadebug.h"
#include "lualib.h"
#include "lauxlib.h"

#ifndef DIAG_BUILD
#define DIAG_BUILD 1
#endif

#undef DEBUG_PRINT_TMU_PARAMS


#include <3dfx.h>
#include <fxpci.h>
#include "glide.h"
#include <h3regs.h>
#include <h3cinit.h>

#include "mdclua.h"
#include "gplay\gplay.h"


#define GLDE_CHECKARG(x,y) ((y)luaL_check_number(x))

/* glide2x */
void lm_glde_checksumGlideFb(void);
void lm_glde_grSplash(void); 
void lm_glde_loadTextureFile(void);
void lm_glde_guTexSource(void);
void lm_glde_grSstIdle(void);
void lm_glde_grSstSelect(void); /* int */
void lm_glde_grGlideInit(void);
void lm_glde_grConstantColorValue(void); /* h */
void lm_glde_grColorCombine(void); /* color combine parameters */
#if 0
void lm_glde_tlConOutput(void); /* string */
void lm_glde_tlConSet(void); /* f, f, f, f, i, i, h */
void lm_glde_tlConRender(void);
#endif
void lm_glde_grDrawPoint(void); /* vertex */
void lm_glde_grDrawLine(void); /* v,v */
void lm_glde_grDrawTriangle(void); /* v,v,v */
void lm_glde_grBufferSwap(void); /* int */
void lm_glde_grGlideShutdown(void); 
void lm_glde_grBufferClear(void);
void lm_glde_startglide(void);

void lm_glde_grTexCombine(void);    /* GrChipID_t, GrCombineFunction_t, GrCombineFactor_t, 
                                       GrCombineFunction_t, GrCombineFactor, FxBool, FxBool */
void lm_glde_grTexMipMapMode(void); /* GrChipID_t, GrMipMapMode_t  , FxBool */
void lm_glde_grTexFilterMode(void); /* GrChipID_t, GrTextureFilterMode_t, GrTextureFilterMode_t */

void lm_glde_grHints(void); /* GrHint_t, FxU32 hintMask */
void lm_glde_grSliCtrl(void);
void lm_glde_grDisableSliCtrl(void);
void lm_glde_grAASetup(void);

/* glide2x - gplay */
#define GPLAY_DONE 1
#ifdef GPLAY_DONE
void lm_glde_gplayTrace(void);
#endif

static struct luafn_reg_struct luafn_list[] = {
  /* Glide2x */
  { "startglide", lm_glde_startglide },
  { "grGlideInit", lm_glde_grGlideInit },
  { "grSstSelect", lm_glde_grSstSelect },
#if 0
  { "tlConOutput", lm_glde_tlConOutput },
  { "tlConSet", lm_glde_tlConSet },
  { "tlConRender", lm_glde_tlConRender },
#endif
  { "grDrawPoint", lm_glde_grDrawPoint },
  { "grDrawLine", lm_glde_grDrawLine },
  { "grDrawTriangle", lm_glde_grDrawTriangle },
  { "grBufferSwap", lm_glde_grBufferSwap },
  { "grGlideShutdown", lm_glde_grGlideShutdown },
  { "grConstantColorValue", lm_glde_grConstantColorValue },
  { "grColorCombine", lm_glde_grColorCombine },
  { "grBufferClear", lm_glde_grBufferClear },
  { "grSplash", lm_glde_grSplash },
  { "grSstIdle", lm_glde_grSstIdle },
  { "loadTextureFile", lm_glde_loadTextureFile },
  { "guTexSource", lm_glde_guTexSource },
  { "checksumGlideFb", lm_glde_checksumGlideFb },
  
  { "grTexFilterMode", lm_glde_grTexFilterMode },
  { "grTexCombine", lm_glde_grTexCombine },
  { "grTexMipMapMode", lm_glde_grTexMipMapMode },
  { "grSliCtrl", lm_glde_grSliCtrl },
  { "grDisableSliCtrl", lm_glde_grDisableSliCtrl },
  { "grAASetup", lm_glde_grAASetup },
  { "grHints", lm_glde_grHints },

#ifdef GPLAY_DONE
  { "gplayTrace", lm_glde_gplayTrace },
#endif
  { NULL, NULL }};



static struct luadbl_reg_struct luadbl_list[] = {
  {"FXTRUE", 1},
  {"FXFALSE", 0 },

  {"MAX_NUM_SST", MAX_NUM_SST},
  {"MAX_MIPMAPS_PER_SST", MAX_MIPMAPS_PER_SST},

  {"GR_LODBIAS_BILINEAR", GR_LODBIAS_BILINEAR},
  {"GR_LODBIAS_TRILINEAR", GR_LODBIAS_TRILINEAR},

  {"GR_FOG_TABLE_SIZE", GR_FOG_TABLE_SIZE },
  {"GR_NULL_MIPMAP_HANDLE", GR_NULL_MIPMAP_HANDLE},
  {"GR_ZDEPTHVALUE_NEAREST", GR_ZDEPTHVALUE_NEAREST},
  {"GR_ZDEPTHVALUE_FARTHEST", GR_ZDEPTHVALUE_FARTHEST},
  {"GR_WDEPTHVALUE_NEAREST", GR_WDEPTHVALUE_NEAREST},
  {"GR_WDEPTHVALUE_FARTHEST", GR_WDEPTHVALUE_FARTHEST},
  
  {"GR_MIPMAPLEVELMASK_EVEN", GR_MIPMAPLEVELMASK_EVEN},
  {"GR_MIPMAPLEVELMASK_ODD", GR_MIPMAPLEVELMASK_ODD},
  {"GR_MIPMAPLEVELMASK_BOTH", GR_MIPMAPLEVELMASK_BOTH},
  
  
// typedef FxI32 GrChipID_t;
        {"GR_TMU0", GR_TMU0},
        {"GR_TMU1", GR_TMU1},
        {"GR_TMU2", GR_TMU2},
        {"GR_FBI", GR_FBI},

// typedef FxI32 GrCombineFunction_t;
        {"GR_COMBINE_FUNCTION_ZERO", GR_COMBINE_FUNCTION_ZERO},
        {"GR_COMBINE_FUNCTION_NONE", GR_COMBINE_FUNCTION_NONE},
        {"GR_COMBINE_FUNCTION_LOCAL", GR_COMBINE_FUNCTION_LOCAL},
        {"GR_COMBINE_FUNCTION_LOCAL_ALPHA", GR_COMBINE_FUNCTION_LOCAL_ALPHA},
        {"GR_COMBINE_FUNCTION_SCALE_OTHER", GR_COMBINE_FUNCTION_SCALE_OTHER},
        {"GR_COMBINE_FUNCTION_BLEND_OTHER", GR_COMBINE_FUNCTION_BLEND_OTHER},
        {"GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL", GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL},
        {"GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA", GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA},
        {"GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL", GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL},
        {"GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL", GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL},
        {"GR_COMBINE_FUNCTION_BLEND", GR_COMBINE_FUNCTION_BLEND},
        {"GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA", GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA},
        {"GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL", GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL},
        {"GR_COMBINE_FUNCTION_BLEND_LOCAL", GR_COMBINE_FUNCTION_BLEND_LOCAL},
        {"GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA", GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA},

// typedef FxI32 GrCombineFactor_t;
        {"GR_COMBINE_FACTOR_ZERO", GR_COMBINE_FACTOR_ZERO},
        {"GR_COMBINE_FACTOR_NONE", GR_COMBINE_FACTOR_NONE},
        {"GR_COMBINE_FACTOR_LOCAL", GR_COMBINE_FACTOR_LOCAL},
        {"GR_COMBINE_FACTOR_OTHER_ALPHA", GR_COMBINE_FACTOR_OTHER_ALPHA},
        {"GR_COMBINE_FACTOR_LOCAL_ALPHA", GR_COMBINE_FACTOR_LOCAL_ALPHA},
        {"GR_COMBINE_FACTOR_TEXTURE_ALPHA", GR_COMBINE_FACTOR_TEXTURE_ALPHA},
        {"GR_COMBINE_FACTOR_TEXTURE_RGB", GR_COMBINE_FACTOR_TEXTURE_RGB},
        {"GR_COMBINE_FACTOR_DETAIL_FACTOR", GR_COMBINE_FACTOR_DETAIL_FACTOR},
        {"GR_COMBINE_FACTOR_LOD_FRACTION", GR_COMBINE_FACTOR_LOD_FRACTION},
        {"GR_COMBINE_FACTOR_ONE", GR_COMBINE_FACTOR_ONE},
        {"GR_COMBINE_FACTOR_ONE_MINUS_LOCAL", GR_COMBINE_FACTOR_ONE_MINUS_LOCAL},
        {"GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA", GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA},
        {"GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA", GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA},
        {"GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA", GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA},
        {"GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR", GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR},
        {"GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION", GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION},


// typedef FxI32 GrCombineLocal_t;
        {"GR_COMBINE_LOCAL_ITERATED", GR_COMBINE_LOCAL_ITERATED},
        {"GR_COMBINE_LOCAL_CONSTANT", GR_COMBINE_LOCAL_CONSTANT},
        {"GR_COMBINE_LOCAL_NONE", GR_COMBINE_LOCAL_NONE},
        {"GR_COMBINE_LOCAL_DEPTH", GR_COMBINE_LOCAL_DEPTH},

// typedef FxI32 GrCombineOther_t;
        {"GR_COMBINE_OTHER_ITERATED", GR_COMBINE_OTHER_ITERATED},
        {"GR_COMBINE_OTHER_TEXTURE", GR_COMBINE_OTHER_TEXTURE},
        {"GR_COMBINE_OTHER_CONSTANT", GR_COMBINE_OTHER_CONSTANT},
        {"GR_COMBINE_OTHER_NONE", GR_COMBINE_OTHER_NONE},


// typedef FxI32 GrAlphaSource_t;
        {"GR_ALPHASOURCE_CC_ALPHA", GR_ALPHASOURCE_CC_ALPHA},
        {"GR_ALPHASOURCE_ITERATED_ALPHA", GR_ALPHASOURCE_ITERATED_ALPHA},
        {"GR_ALPHASOURCE_TEXTURE_ALPHA", GR_ALPHASOURCE_TEXTURE_ALPHA},
        {"GR_ALPHASOURCE_TEXTURE_ALPHA_TIMES_ITERATED_ALPHA", GR_ALPHASOURCE_TEXTURE_ALPHA_TIMES_ITERATED_ALPHA},


// typedef FxI32 GrColorCombineFnc_t;
        {"GR_COLORCOMBINE_ZERO", GR_COLORCOMBINE_ZERO},
        {"GR_COLORCOMBINE_CCRGB", GR_COLORCOMBINE_CCRGB},
        {"GR_COLORCOMBINE_ITRGB", GR_COLORCOMBINE_ITRGB},
        {"GR_COLORCOMBINE_ITRGB_DELTA0", GR_COLORCOMBINE_ITRGB_DELTA0},
        {"GR_COLORCOMBINE_DECAL_TEXTURE", GR_COLORCOMBINE_DECAL_TEXTURE},
        {"GR_COLORCOMBINE_TEXTURE_TIMES_CCRGB", GR_COLORCOMBINE_TEXTURE_TIMES_CCRGB},
        {"GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB", GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB},
        {"GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_DELTA0", GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_DELTA0},
        {"GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_ADD_ALPHA", GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_ADD_ALPHA},
        {"GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA", GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA},
        {"GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA_ADD_ITRGB", GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA_ADD_ITRGB},
        {"GR_COLORCOMBINE_TEXTURE_ADD_ITRGB", GR_COLORCOMBINE_TEXTURE_ADD_ITRGB},
        {"GR_COLORCOMBINE_TEXTURE_SUB_ITRGB", GR_COLORCOMBINE_TEXTURE_SUB_ITRGB},
        {"GR_COLORCOMBINE_CCRGB_BLEND_ITRGB_ON_TEXALPHA", GR_COLORCOMBINE_CCRGB_BLEND_ITRGB_ON_TEXALPHA},
        {"GR_COLORCOMBINE_DIFF_SPEC_A", GR_COLORCOMBINE_DIFF_SPEC_A},
        {"GR_COLORCOMBINE_DIFF_SPEC_B", GR_COLORCOMBINE_DIFF_SPEC_B},
        {"GR_COLORCOMBINE_ONE", GR_COLORCOMBINE_ONE},

// typedef FxI32 GrAlphaBlendFnc_t;
        {"GR_BLEND_ZERO", GR_BLEND_ZERO},
        {"GR_BLEND_SRC_ALPHA", GR_BLEND_SRC_ALPHA},
        {"GR_BLEND_SRC_COLOR", GR_BLEND_SRC_COLOR},
        {"GR_BLEND_DST_COLOR", GR_BLEND_DST_COLOR},
        {"GR_BLEND_DST_ALPHA", GR_BLEND_DST_ALPHA},
        {"GR_BLEND_ONE", GR_BLEND_ONE},
        {"GR_BLEND_ONE_MINUS_SRC_ALPHA", GR_BLEND_ONE_MINUS_SRC_ALPHA},
        {"GR_BLEND_ONE_MINUS_SRC_COLOR", GR_BLEND_ONE_MINUS_SRC_COLOR},
        {"GR_BLEND_ONE_MINUS_DST_COLOR", GR_BLEND_ONE_MINUS_DST_COLOR},
        {"GR_BLEND_ONE_MINUS_DST_ALPHA", GR_BLEND_ONE_MINUS_DST_ALPHA},
        {"GR_BLEND_RESERVED_8", GR_BLEND_RESERVED_8},
        {"GR_BLEND_RESERVED_9", GR_BLEND_RESERVED_9},
        {"GR_BLEND_RESERVED_A", GR_BLEND_RESERVED_A},
        {"GR_BLEND_RESERVED_B", GR_BLEND_RESERVED_B},
        {"GR_BLEND_RESERVED_C", GR_BLEND_RESERVED_C},
        {"GR_BLEND_RESERVED_D", GR_BLEND_RESERVED_D},
        {"GR_BLEND_RESERVED_E", GR_BLEND_RESERVED_E},
        {"GR_BLEND_ALPHA_SATURATE", GR_BLEND_ALPHA_SATURATE},
        {"GR_BLEND_PREFOG_COLOR", GR_BLEND_PREFOG_COLOR},

// typedef FxI32 GrAspectRatio_t;
#if defined(GLIDE3) && defined(GLIDE3_ALPHA)
        {"GR_ASPECT_8x1", GR_ASPECT_8x1},
        {"GR_ASPECT_4x1", GR_ASPECT_4x1},
        {"GR_ASPECT_2x1", GR_ASPECT_2x1},
        {"GR_ASPECT_1x1", GR_ASPECT_1x1},
        {"GR_ASPECT_1x2", GR_ASPECT_1x2},
        {"GR_ASPECT_1x4", GR_ASPECT_1x4},
        {"GR_ASPECT_1x8", GR_ASPECT_1x8},
#endif

// typedef FxI32 GrBuffer_t;
        {"GR_BUFFER_FRONTBUFFER", GR_BUFFER_FRONTBUFFER},
        {"GR_BUFFER_BACKBUFFER", GR_BUFFER_BACKBUFFER},
        {"GR_BUFFER_AUXBUFFER", GR_BUFFER_AUXBUFFER},
        {"GR_BUFFER_DEPTHBUFFER", GR_BUFFER_DEPTHBUFFER},
        {"GR_BUFFER_ALPHABUFFER", GR_BUFFER_ALPHABUFFER},
        {"GR_BUFFER_TRIPLEBUFFER", GR_BUFFER_TRIPLEBUFFER},

#ifdef CHRIS_DENIS_ANTHONY_HACK
        {"GR_BUFFER_DENIS_HACK_ON", GR_BUFFER_DENIS_HACK_ON},
        {"GR_BUFFER_DENIS_HACK_OFF", GR_BUFFER_DENIS_HACK_OFF},
#endif

// typedef FxI32 GrChromakeyMode_t;
        {"GR_CHROMAKEY_DISABLE", GR_CHROMAKEY_DISABLE},
        {"GR_CHROMAKEY_ENABLE", GR_CHROMAKEY_ENABLE},

#if defined(GLIDE3) && defined(GLIDE3_ALPHA)
// typedef FxI32 GrChromaRangeMode_t;
        {"GR_CHROMARANGE_RGB_ANY", GR_CHROMARANGE_RGB_ANY},
        {"GR_CHROMARANGE_RGb_ANY", GR_CHROMARANGE_RGb_ANY},
        {"GR_CHROMARANGE_RgB_ANY", GR_CHROMARANGE_RgB_ANY},
        {"GR_CHROMARANGE_Rgb_ANY", GR_CHROMARANGE_Rgb_ANY},
        {"GR_CHROMARANGE_rGB_ANY", GR_CHROMARANGE_rGB_ANY},
        {"GR_CHROMARANGE_rGb_ANY", GR_CHROMARANGE_rGb_ANY},
        {"GR_CHROMARANGE_rgB_ANY", GR_CHROMARANGE_rgB_ANY},
        {"GR_CHROMARANGE_rgb_ANY", GR_CHROMARANGE_rgb_ANY},
        {"GR_CHROMARANGE_RGB_ALL", GR_CHROMARANGE_RGB_ALL},
        {"GR_CHROMARANGE_RGb_ALL", GR_CHROMARANGE_RGb_ALL},
        {"GR_CHROMARANGE_RgB_ALL", GR_CHROMARANGE_RgB_ALL},
        {"GR_CHROMARANGE_Rgb_ALL", GR_CHROMARANGE_Rgb_ALL},
        {"GR_CHROMARANGE_rGB_ALL", GR_CHROMARANGE_rGB_ALL},
        {"GR_CHROMARANGE_rGb_ALL", GR_CHROMARANGE_rGb_ALL},
        {"GR_CHROMARANGE_rgB_ALL", GR_CHROMARANGE_rgB_ALL},
        {"GR_CHROMARANGE_rgb_ALL", GR_CHROMARANGE_rgb_ALL},
#endif

// typedef FxI32 GrCmpFnc_t;
        {"GR_CMP_NEVER", GR_CMP_NEVER},
        {"GR_CMP_LESS", GR_CMP_LESS},
        {"GR_CMP_EQUAL", GR_CMP_EQUAL},
        {"GR_CMP_LEQUAL", GR_CMP_LEQUAL},
        {"GR_CMP_GREATER", GR_CMP_GREATER},
        {"GR_CMP_NOTEQUAL", GR_CMP_NOTEQUAL},
        {"GR_CMP_GEQUAL", GR_CMP_GEQUAL},
        {"GR_CMP_ALWAYS", GR_CMP_ALWAYS},

// typedef FxI32 GrColorFormat_t;
        {"GR_COLORFORMAT_ARGB", GR_COLORFORMAT_ARGB},
        {"GR_COLORFORMAT_ABGR", GR_COLORFORMAT_ABGR},

        {"GR_COLORFORMAT_RGBA", GR_COLORFORMAT_RGBA},
        {"GR_COLORFORMAT_BGRA", GR_COLORFORMAT_BGRA},

// typedef FxI32 GrCullMode_t;
        {"GR_CULL_DISABLE", GR_CULL_DISABLE},
        {"GR_CULL_NEGATIVE", GR_CULL_NEGATIVE},
        {"GR_CULL_POSITIVE", GR_CULL_POSITIVE},

// typedef FxI32 GrDepthBufferMode_t;
        {"GR_DEPTHBUFFER_DISABLE", GR_DEPTHBUFFER_DISABLE},
        {"GR_DEPTHBUFFER_ZBUFFER", GR_DEPTHBUFFER_ZBUFFER},
        {"GR_DEPTHBUFFER_WBUFFER", GR_DEPTHBUFFER_WBUFFER},
        {"GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS", GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS},
        {"GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS", GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS},

// typedef FxI32 GrDitherMode_t;
        {"GR_DITHER_DISABLE", GR_DITHER_DISABLE},
        {"GR_DITHER_2x2", GR_DITHER_2x2},
        {"GR_DITHER_4x4", GR_DITHER_4x4},

// typedef FxI32 GrFogMode_t;
        {"GR_FOG_DISABLE", GR_FOG_DISABLE},
        {"GR_FOG_WITH_ITERATED_ALPHA", GR_FOG_WITH_ITERATED_ALPHA},
        {"GR_FOG_WITH_TABLE", GR_FOG_WITH_TABLE},
        {"GR_FOG_WITH_ITERATED_Z", GR_FOG_WITH_ITERATED_Z},
        {"GR_FOG_MULT2", GR_FOG_MULT2},
        {"GR_FOG_ADD2", GR_FOG_ADD2},

// typedef FxU32 GrLock_t;
        {"GR_LFB_READ_ONLY", GR_LFB_READ_ONLY},
        {"GR_LFB_WRITE_ONLY", GR_LFB_WRITE_ONLY},
        {"GR_LFB_IDLE", GR_LFB_IDLE},
        {"GR_LFB_NOIDLE", GR_LFB_NOIDLE},

// typedef FxI32 GrLfbBypassMode_t;
        {"GR_LFBBYPASS_DISABLE", GR_LFBBYPASS_DISABLE},
        {"GR_LFBBYPASS_ENABLE", GR_LFBBYPASS_ENABLE},

// typedef FxI32 GrLfbWriteMode_t;
        {"GR_LFBWRITEMODE_565", GR_LFBWRITEMODE_565},
        {"GR_LFBWRITEMODE_555", GR_LFBWRITEMODE_555},
        {"GR_LFBWRITEMODE_1555", GR_LFBWRITEMODE_1555},
        {"GR_LFBWRITEMODE_RESERVED1", GR_LFBWRITEMODE_RESERVED1},
        {"GR_LFBWRITEMODE_888", GR_LFBWRITEMODE_888},
        {"GR_LFBWRITEMODE_8888", GR_LFBWRITEMODE_8888},
        {"GR_LFBWRITEMODE_RESERVED2", GR_LFBWRITEMODE_RESERVED2},
        {"GR_LFBWRITEMODE_RESERVED3", GR_LFBWRITEMODE_RESERVED3},
        {"GR_LFBWRITEMODE_RESERVED4", GR_LFBWRITEMODE_RESERVED4},
        {"GR_LFBWRITEMODE_RESERVED5", GR_LFBWRITEMODE_RESERVED5},
        {"GR_LFBWRITEMODE_RESERVED6", GR_LFBWRITEMODE_RESERVED6},
        {"GR_LFBWRITEMODE_RESERVED7", GR_LFBWRITEMODE_RESERVED7},
        {"GR_LFBWRITEMODE_565_DEPTH", GR_LFBWRITEMODE_565_DEPTH},
        {"GR_LFBWRITEMODE_555_DEPTH", GR_LFBWRITEMODE_555_DEPTH},
        {"GR_LFBWRITEMODE_1555_DEPTH", GR_LFBWRITEMODE_1555_DEPTH},
        {"GR_LFBWRITEMODE_ZA16", GR_LFBWRITEMODE_ZA16},
        {"GR_LFBWRITEMODE_ANY", GR_LFBWRITEMODE_ANY},

// typedef FxI32 GrOriginLocation_t;
        {"GR_ORIGIN_UPPER_LEFT", GR_ORIGIN_UPPER_LEFT},
        {"GR_ORIGIN_LOWER_LEFT", GR_ORIGIN_LOWER_LEFT},
        {"GR_ORIGIN_ANY", GR_ORIGIN_ANY},

// typedef FxI32 GrLOD_t;
        {"GR_LOD_256", GR_LOD_256},
        {"GR_LOD_128", GR_LOD_128},
        {"GR_LOD_64", GR_LOD_64},
        {"GR_LOD_32", GR_LOD_32},
        {"GR_LOD_16", GR_LOD_16},
        {"GR_LOD_8", GR_LOD_8},
        {"GR_LOD_4", GR_LOD_4},
        {"GR_LOD_2", GR_LOD_2},
        {"GR_LOD_1", GR_LOD_1},

// typedef FxI32 GrMipMapMode_t;
        {"GR_MIPMAP_DISABLE", GR_MIPMAP_DISABLE},
        {"GR_MIPMAP_NEAREST", GR_MIPMAP_NEAREST},
        {"GR_MIPMAP_NEAREST_DITHER", GR_MIPMAP_NEAREST_DITHER},


// typedef GrHint_t
  { "GR_HINT_STWHINT" , GR_HINT_STWHINT },

// typedef GrSTWHint_t

  { "GR_STWHINT_ST_DIFF_TMU0" , GR_STWHINT_ST_DIFF_TMU0 },
  { "GR_STWHINT_ST_DIFF_TMU1" , GR_STWHINT_ST_DIFF_TMU1 },
  { "GR_STWHINT_ST_DIFF_TMU2" , GR_STWHINT_ST_DIFF_TMU2 },
  { "GR_STWHINT_W_DIFF_TMU0"  , GR_STWHINT_W_DIFF_TMU0 },
  { "GR_STWHINT_W_DIFF_TMU1"  , GR_STWHINT_W_DIFF_TMU1 },
  { "GR_STWHINT_W_DIFF_TMU2"  , GR_STWHINT_W_DIFF_TMU2 },

// typedef FxI32 GrSmoothingMode_t;
        {"GR_SMOOTHING_DISABLE", GR_SMOOTHING_DISABLE},
        {"GR_SMOOTHING_ENABLE", GR_SMOOTHING_ENABLE},

// typedef FxI32 GrTextureClampMode_t;
        {"GR_TEXTURECLAMP_WRAP", GR_TEXTURECLAMP_WRAP},
        {"GR_TEXTURECLAMP_CLAMP", GR_TEXTURECLAMP_CLAMP},

// typedef FxI32 GrTextureCombineFnc_t;
        {"GR_TEXTURECOMBINE_ZERO", GR_TEXTURECOMBINE_ZERO},
        {"GR_TEXTURECOMBINE_DECAL", GR_TEXTURECOMBINE_DECAL},
        {"GR_TEXTURECOMBINE_OTHER", GR_TEXTURECOMBINE_OTHER},
        {"GR_TEXTURECOMBINE_ADD", GR_TEXTURECOMBINE_ADD},
        {"GR_TEXTURECOMBINE_MULTIPLY", GR_TEXTURECOMBINE_MULTIPLY},
        {"GR_TEXTURECOMBINE_SUBTRACT", GR_TEXTURECOMBINE_SUBTRACT},
        {"GR_TEXTURECOMBINE_DETAIL", GR_TEXTURECOMBINE_DETAIL},
        {"GR_TEXTURECOMBINE_DETAIL_OTHER", GR_TEXTURECOMBINE_DETAIL_OTHER},
        {"GR_TEXTURECOMBINE_TRILINEAR_ODD", GR_TEXTURECOMBINE_TRILINEAR_ODD},
        {"GR_TEXTURECOMBINE_TRILINEAR_EVEN", GR_TEXTURECOMBINE_TRILINEAR_EVEN},
        {"GR_TEXTURECOMBINE_ONE", GR_TEXTURECOMBINE_ONE},

// typedef FxI32 GrTextureFilterMode_t;
        {"GR_TEXTUREFILTER_POINT_SAMPLED", GR_TEXTUREFILTER_POINT_SAMPLED},
        {"GR_TEXTUREFILTER_BILINEAR", GR_TEXTUREFILTER_BILINEAR},

// typedef FxI32 GrTextureFormat_t;
        {"GR_TEXFMT_8BIT", GR_TEXFMT_8BIT},
        {"GR_TEXFMT_RGB_332", GR_TEXFMT_RGB_332},
        {"GR_TEXFMT_YIQ_422", GR_TEXFMT_YIQ_422},
        {"GR_TEXFMT_ALPHA_8", GR_TEXFMT_ALPHA_8},
        {"GR_TEXFMT_INTENSITY_8", GR_TEXFMT_INTENSITY_8},
        {"GR_TEXFMT_ALPHA_INTENSITY_44", GR_TEXFMT_ALPHA_INTENSITY_44},
        {"GR_TEXFMT_P_8", GR_TEXFMT_P_8},
        {"GR_TEXFMT_RSVD0", GR_TEXFMT_RSVD0},
        {"GR_TEXFMT_RSVD1", GR_TEXFMT_RSVD1},
        {"GR_TEXFMT_16BIT", GR_TEXFMT_16BIT},
        {"GR_TEXFMT_ARGB_8332", GR_TEXFMT_ARGB_8332},
        {"GR_TEXFMT_AYIQ_8422", GR_TEXFMT_AYIQ_8422},
        {"GR_TEXFMT_RGB_565", GR_TEXFMT_RGB_565},
        {"GR_TEXFMT_ARGB_1555", GR_TEXFMT_ARGB_1555},
        {"GR_TEXFMT_ARGB_4444", GR_TEXFMT_ARGB_4444},
        {"GR_TEXFMT_ALPHA_INTENSITY_88", GR_TEXFMT_ALPHA_INTENSITY_88},
        {"GR_TEXFMT_AP_88", GR_TEXFMT_AP_88},
        {"GR_TEXFMT_RSVD2", GR_TEXFMT_RSVD2},

// typedef FxU32 GrTexTable_t;
        {"GR_TEXTABLE_NCC0", GR_TEXTABLE_NCC0},
        {"GR_TEXTABLE_NCC1", GR_TEXTABLE_NCC1},
        {"GR_TEXTABLE_PALETTE", GR_TEXTABLE_PALETTE},

// typedef FxU32 GrNCCTable_t;
        {"GR_NCCTABLE_NCC0", GR_NCCTABLE_NCC0},
        {"GR_NCCTABLE_NCC1", GR_NCCTABLE_NCC1},

// typedef FxU32 GrTexBaseRange_t;
        {"GR_TEXBASE_256", GR_TEXBASE_256},
        {"GR_TEXBASE_128", GR_TEXBASE_128},
        {"GR_TEXBASE_64", GR_TEXBASE_64},
        {"GR_TEXBASE_32_TO_1", GR_TEXBASE_32_TO_1},

#ifdef GLIDE3
// typedef FxU32 GrEnableMode_t;
        {"GR_MODE_DISABLE", GR_MODE_DISABLE},
        {"GR_MODE_ENABLE", GR_MODE_ENABLE},

        {"GR_AA_ORDERED", GR_AA_ORDERED},
        {"GR_ALLOW_MIPMAP_DITHER", GR_ALLOW_MIPMAP_DITHER},
        {"GR_SHAMELESS_PLUG", GR_SHAMELESS_PLUG},
        {"GR_VIDEO_SMOOTHING", GR_VIDEO_SMOOTHING},

// typedef FxU32 GrCoordinateSpaceMode_t;
        {"GR_WINDOW_COORDS", GR_WINDOW_COORDS},
        {"GR_CLIP_COORDS", GR_CLIP_COORDS},

/* Types of data in strips */
        {"GR_FLOAT", GR_FLOAT},
        {"GR_U8", GR_U8},

/* Parameters for strips */
        {"GR_PARAM_XY", GR_PARAM_XY},
        {"GR_PARAM_Z", GR_PARAM_Z},
        {"GR_PARAM_W", GR_PARAM_W},
        {"GR_PARAM_Q", GR_PARAM_Q},

        {"GR_PARAM_A", GR_PARAM_A},
        {"GR_PARAM_A0", GR_PARAM_A0},
        {"GR_PARAM_A1", GR_PARAM_A1},
        {"GR_PARAM_A2", GR_PARAM_A2},
        {"GR_PARAM_A3", GR_PARAM_A3},
        {"GR_PARAM_A4", GR_PARAM_A4},
        {"GR_PARAM_A5", GR_PARAM_A5},
        {"GR_PARAM_A6", GR_PARAM_A6},
        {"GR_PARAM_A7", GR_PARAM_A7},

        {"GR_PARAM_RGB", GR_PARAM_RGB},
        {"GR_PARAM_RGB0", GR_PARAM_RGB0},
        {"GR_PARAM_RGB1", GR_PARAM_RGB1},
        {"GR_PARAM_RGB2", GR_PARAM_RGB2},
        {"GR_PARAM_RGB3", GR_PARAM_RGB3},
        {"GR_PARAM_RGB4", GR_PARAM_RGB4},
        {"GR_PARAM_RGB5", GR_PARAM_RGB5},
        {"GR_PARAM_RGB6", GR_PARAM_RGB6},
        {"GR_PARAM_RGB7", GR_PARAM_RGB7},

        {"GR_PARAM_PARGB", GR_PARAM_PARGB},
        {"GR_PARAM_PARGB0", GR_PARAM_PARGB0},
        {"GR_PARAM_PARGB1", GR_PARAM_PARGB1},
        {"GR_PARAM_PARGB2", GR_PARAM_PARGB2},
        {"GR_PARAM_PARGB3", GR_PARAM_PARGB3},
        {"GR_PARAM_PARGB4", GR_PARAM_PARGB4},
        {"GR_PARAM_PARGB5", GR_PARAM_PARGB5},
        {"GR_PARAM_PARGB6", GR_PARAM_PARGB6},
        {"GR_PARAM_PARGB7", GR_PARAM_PARGB7},

        {"GR_PARAM_ST0", GR_PARAM_ST0},
        {"GR_PARAM_ST1", GR_PARAM_ST1},
        {"GR_PARAM_ST2", GR_PARAM_ST2},
        {"GR_PARAM_ST3", GR_PARAM_ST3},
        {"GR_PARAM_ST4", GR_PARAM_ST4},
        {"GR_PARAM_ST5", GR_PARAM_ST5},
        {"GR_PARAM_ST6", GR_PARAM_ST6},
        {"GR_PARAM_ST7", GR_PARAM_ST7},

        {"GR_PARAM_Q0", GR_PARAM_Q0},
        {"GR_PARAM_Q1", GR_PARAM_Q1},
        {"GR_PARAM_Q2", GR_PARAM_Q2},
        {"GR_PARAM_Q3", GR_PARAM_Q3},
        {"GR_PARAM_Q4", GR_PARAM_Q4},
        {"GR_PARAM_Q5", GR_PARAM_Q5},
        {"GR_PARAM_Q6", GR_PARAM_Q6},
        {"GR_PARAM_Q7", GR_PARAM_Q7},

        {"GR_PARAM_DISABLE", GR_PARAM_DISABLE},
        {"GR_PARAM_ENABLE", GR_PARAM_ENABLE},

/* Componenets for strips */
/* vertex */
        {"GR_VERTEX_XYZ", GR_VERTEX_XYZ},
        {"GR_VERTEX_XYZW", GR_VERTEX_XYZW},
/* Color */
        {"GR_COLOR_RGB", GR_COLOR_RGB},
        {"GR_COLOR_RGBA", GR_COLOR_RGBA},
/* Texture */
        {"GR_TEX_NONE", GR_TEX_NONE},
        {"GR_TEX_ST", GR_TEX_ST},
        {"GR_TEX_STW", GR_TEX_STW},

/* grDrawVertexArray primitive type */
        {"GR_POINTS", GR_POINTS},
        {"GR_LINE_STRIP", GR_LINE_STRIP},
        {"GR_LINES", GR_LINES},
        {"GR_POLYGON", GR_POLYGON},
        {"GR_TRIANGLE_STRIP", GR_TRIANGLE_STRIP},
        {"GR_TRIANGLE_FAN", GR_TRIANGLE_FAN},
        {"GR_TRIANGLES", GR_TRIANGLES},


/* Stuff for grGet/grReset */
        {"GR_BITS_DEPTH", GR_BITS_DEPTH},
        {"GR_BITS_RGBA", GR_BITS_RGBA},
        {"GR_FIFO_FULLNESS", GR_FIFO_FULLNESS},
        {"GR_FOG_TABLE_ENTRIES", GR_FOG_TABLE_ENTRIES},
        {"GR_GAMMA_TABLE_ENTRIES", GR_GAMMA_TABLE_ENTRIES},
        {"GR_IS_BUSY", GR_IS_BUSY},
        {"GR_LFB_PIXEL_PIPE", GR_LFB_PIXEL_PIPE},
        {"GR_MAX_TEXTURE_SIZE", GR_MAX_TEXTURE_SIZE},
        {"GR_MAX_TEXTURE_ASPECT_RATIO", GR_MAX_TEXTURE_ASPECT_RATIO},
        {"GR_MEMORY_FB", GR_MEMORY_FB},
        {"GR_MEMORY_TMU", GR_MEMORY_TMU},
        {"GR_MEMORY_UMA", GR_MEMORY_UMA},
        {"GR_NUM_BOARDS", GR_NUM_BOARDS},
        {"GR_NUM_POWER_OF_TWO_TEXTURES", GR_NUM_POWER_OF_TWO_TEXTURES},
        {"GR_NUM_FB", GR_NUM_FB},
        {"GR_NUM_TMU", GR_NUM_TMU},
        {"GR_PENDING_BUFFERSWAPS", GR_PENDING_BUFFERSWAPS},
        {"GR_REVISION_FB", GR_REVISION_FB},
        {"GR_REVISION_TMU", GR_REVISION_TMU},
        {"GR_STATS_LINES", GR_STATS_LINES},
        {"GR_STATS_PIXELS_AFUNC_FAIL", GR_STATS_PIXELS_AFUNC_FAIL},
        {"GR_STATS_PIXELS_CHROMA_FAIL", GR_STATS_PIXELS_CHROMA_FAIL},
        {"GR_STATS_PIXELS_DEPTHFUNC_FAIL", GR_STATS_PIXELS_DEPTHFUNC_FAIL},
        {"GR_STATS_PIXELS_IN", GR_STATS_PIXELS_IN},
        {"GR_STATS_PIXELS_OUT", GR_STATS_PIXELS_OUT},
        {"GR_STATS_PIXELS", GR_STATS_PIXELS},
        {"GR_STATS_POINTS", GR_STATS_POINTS},
        {"GR_STATS_TRIANGLES_IN", GR_STATS_TRIANGLES_IN},
        {"GR_STATS_TRIANGLES_OUT", GR_STATS_TRIANGLES_OUT},
        {"GR_STATS_TRIANGLES", GR_STATS_TRIANGLES},
        {"GR_SWAP_HISTORY", GR_SWAP_HISTORY},
        {"GR_TEXTURE_ALIGN", GR_TEXTURE_ALIGN},
        {"GR_VIDEO_POSITION", GR_VIDEO_POSITION},
        {"GR_VIEWPORT", GR_VIEWPORT},
        {"GR_WDEPTH_MIN_MAX", GR_WDEPTH_MIN_MAX},
        {"GR_ZDEPTH_MIN_MAX", GR_ZDEPTH_MIN_MAX},

/* stuff for grGetString */
        {"GR_EXTENSION", GR_EXTENSION},
        {"GR_HARDWARE", GR_HARDWARE},
        {"GR_RENDERER", GR_RENDERER},
        {"GR_VENDOR", GR_VENDOR},
        {"GR_VERSION", GR_VERSION},
#endif /* GLIDE3 */
  { NULL, NULL }};


void lm_glde_init() {
   mdcl_register_fns(luafn_list);
   mdcl_register_dbls(luadbl_list);
}


void mdc_glideErrorCallback(const char *string, FxBool fatal);
void lm_glde_grGlideInit(void) {

  grErrorSetCallback(mdc_glideErrorCallback);
  grGlideInit();
}

void lm_glde_grSstSelect(void) {
  int num = lua_getnumber(lua_getparam(1));

  grSstSelect(num);
}


#if 0
void lm_glde_tlConOutput(void) { /* string */
  char *string = luaL_check_string(1);
  tlConOutput(string);
}


void lm_glde_tlConSet(void) { /* f, f, f, f, i, i, h */
  float a = luaL_check_number(1);
  float b = luaL_check_number(2);
  float c = luaL_check_number(3);
  float d = luaL_check_number(4);
  int e = luaL_check_number(5);
  int f = luaL_check_number(6);
  unsigned long int g = luaL_check_number(7);

  tlConSet(a,b,c,d,e,f,g);
}

void lm_glde_tlConRender(void) {
  tlConRender();
}

#endif

struct vertex_data_pull_struct {
  char *var_name;
  unsigned int offset;
} vertex_data_pull_list[] = {
  { "x",      GR_VERTEX_X_OFFSET },
  { "y",      GR_VERTEX_Y_OFFSET },
  { "z",      GR_VERTEX_Z_OFFSET },
  { "r",      GR_VERTEX_R_OFFSET },
  { "g",      GR_VERTEX_G_OFFSET },
  { "b",      GR_VERTEX_B_OFFSET },
  { "ooz",    GR_VERTEX_OOZ_OFFSET },
  { "a",      GR_VERTEX_A_OFFSET },
  { "oow",    GR_VERTEX_OOW_OFFSET },
  { NULL, NULL }};  


struct vertex_data_pull_struct tmu_vertex_data_pull_list[] = {
  { "sow",    ((GrTmuVertex *)0)->sow },
  { "tow",    ((GrTmuVertex *)0)->tow },
  { "oow",    ((GrTmuVertex *)0)->oow },
  { NULL, NULL}};

GrTmuVertex *lm_glde_parse_tmuvertex(lua_Object obj, GrTmuVertex *tvtx) {
  lua_Object tmpobj;
  unsigned char *vtx_param = (unsigned char *)((float *)tvtx);
  struct vertex_data_pull_struct *walker = tmu_vertex_data_pull_list;

  while (walker->var_name) {
    lua_pushobject(obj);
    lua_pushstring(walker->var_name);
    tmpobj = lua_gettable();
    if (lua_isnumber(tmpobj)) {

      *((float *)(&vtx_param[walker->offset])) = lua_getnumber(tmpobj);


#ifdef DEBUG_PRINT_TMU_PARAMS
      printf("setup tmuvertex parameter %s:%d = %f\n",walker->var_name,
           walker->offset,lua_getnumber(tmpobj));
#endif
    }

    walker++;
  }
  return (tvtx);
}


GrVertex *lm_glde_parse_vertex(lua_Object obj, GrVertex *vtx) {
  lua_Object tmpobj;
  float *vtx_param = (float *)vtx;

  struct vertex_data_pull_struct *walker = vertex_data_pull_list;

  while (walker->var_name) {
    lua_pushobject(obj);
    lua_pushstring(walker->var_name);
    tmpobj = lua_gettable();
    if (lua_isnumber(tmpobj)) {
      vtx_param[walker->offset] = lua_getnumber(tmpobj);
#ifdef DEBUG_PRINT_TMU_PARAMS
      printf("setup vertex parameter %s = %f\n",walker->var_name,lua_getnumber(tmpobj));
#endif
    }

    walker++;
  }

    // look for tmu setup parameters

    lua_pushobject(obj);
    lua_pushstring("tmuvtx");
    tmpobj = lua_gettable();
    if (lua_istable(tmpobj)) {
      int tblref;
      int iter = 0;

      lua_pushobject(tmpobj);     
      tblref = lua_ref(0);

      do {
        lua_pushobject(lua_getref(tblref));
        lua_pushnumber(iter + 1);
        tmpobj = lua_gettable();

        if (lua_istable(tmpobj)) {
          lm_glde_parse_tmuvertex(tmpobj,&(vtx->tmuvtx[iter]));
        }

        iter++;
      } while (!lua_isnil(tmpobj));
    }
  return (vtx);
}

void lm_glde_grBufferClear(void) {
  grBufferClear( 0x0, 0, GR_WDEPTHVALUE_FARTHEST);
}

void lm_glde_grColorCombine(void) { /* color combine parameters */
  grColorCombine(GLDE_CHECKARG(1,GrCombineFunction_t),
                 GLDE_CHECKARG(2,GrCombineFactor_t),
                 GLDE_CHECKARG(3,GrCombineLocal_t),
                 GLDE_CHECKARG(4,GrCombineOther_t),
                 GLDE_CHECKARG(5,FxBool)
    );
}

void lm_glde_grConstantColorValue(void) { /* h */
  grConstantColorValue(GLDE_CHECKARG(1,GrColor_t));
}


void lm_glde_grDrawPoint(void) { /* vertex */
  GrVertex vtx_a;
  GrVertex *pvtx_a = lm_glde_parse_vertex(lua_getparam(1),&vtx_a);

  grDrawPoint(pvtx_a);
}

void lm_glde_grDrawLine(void) { /* v , v */
  GrVertex vtx_a, vtx_b;
  GrVertex *pvtx_a, *pvtx_b;

  pvtx_a = lm_glde_parse_vertex(lua_getparam(1),&vtx_a);
  pvtx_b = lm_glde_parse_vertex(lua_getparam(2),&vtx_b);

  grDrawLine(pvtx_a,pvtx_b);
}

void lm_glde_grDrawTriangle(void) { /* v , v ,v */
  GrVertex vtx_a, vtx_b, vtx_c;
  GrVertex *pvtx_a, *pvtx_b, *pvtx_c;

  pvtx_a = lm_glde_parse_vertex(lua_getparam(1),&vtx_a);
  pvtx_b = lm_glde_parse_vertex(lua_getparam(2),&vtx_b);
  pvtx_c = lm_glde_parse_vertex(lua_getparam(3),&vtx_c);

  grDrawTriangle(pvtx_a,pvtx_b,pvtx_c);
}


void lm_glde_grBufferSwap(void) { /* int */
  int a = luaL_check_number(1);
  
  grBufferSwap(a);
}

void lm_glde_grGlideShutdown(void) {
  grGlideShutdown();
}


void mdcl_chksumCallback(int frame) {
  if (lua_callback_fn && lua_isfunction(lua_callback_fn)) {
    // printf("calling lua Callback frame number = %d\n",frame);
    lua_pushnumber(frame);
    lua_callfunction(lua_callback_fn);
  } 
}

void lm_glde_grSplash(void) {
  float arg[4];
  FxU32 frame;


  arg[0] = luaL_check_number(1);  // x
  arg[1] = luaL_check_number(2);  // y 
  arg[2] = luaL_check_number(3);  // w
  arg[3] = luaL_check_number(4);  // h
  frame = luaL_check_number(5);   // frame
  lua_callback_fn = lua_getparam(6);  // callback function

  grSplashCb(arg[0], arg[1], arg[2], arg[3], frame, mdcl_chksumCallback);
}


/* should return the FxU32 checksum of the buffer */
void lm_glde_checksumGlideFb(void) {
  FxU32 screenX = 640;
  FxU32 screenY = 480;
  FxU32 checksum = 0;
  FxU32 i;
  int p;
  FxU16 *buf,*cmp_buf = 0;
  char *fname = NULL;
  int generate = 0;
  FxU32 pixel_errors = 0;
  FxU32 pixel_count = 0;
  FILE *fp = NULL;
  FxU16 maskbyte, maskfilebyte = 0;

  if (lua_isstring(lua_getparam(1))) {
     fname = lua_getstring(lua_getparam(1));
     generate = luaL_check_number(2);
     fp = fopen(fname,(generate ? "wb" : "rb+"));
     if (fp == 0) {
        printf("\n.SBI file (%s) not found in GLD subdirectory.\n",fname);
	return;
     }
  }

  buf = (FxU16 *) malloc(sizeof(FxU16) * screenX);
  if (!generate) {
      cmp_buf = (FxU16 *) malloc(sizeof(FxU16) * screenX);
  }

  for (i = 0; i < screenY; i ++) {
      if (grLfbReadRegion(GR_BUFFER_FRONTBUFFER,
              0, 			/* x-loc of start pixel */
              i, 			/* y-loc of start pixel */
              screenX,  		/* width in pixels */
              1, 			/* height in lines */
              screenX * sizeof(FxU16), 	/* line stride */
              buf   			/* destination buf */
              ) != FXTRUE) {
         printf("grLfbReadRegion() returned badness\n");
      }

      /* compute checksum for this line */
      for (p = 0; p < screenX; p++) {
        checksum += buf[p];
        pixel_count++;
      }

      if (fp) {
         if (generate) {
            if (fwrite(buf,screenX * sizeof(FxU16),1,fp) != 1) {
             lua_error("fwrite failed!");
            }
         } else {
            if (fread(cmp_buf,screenX * sizeof(FxU16),1,fp) != 1) {
             lua_error("fread failed!");
            } else {
             for (p=0;p<screenX;p++) {
              	maskbyte = (buf[p] & 0xFFF0);
		maskfilebyte = (cmp_buf[p] & 0xFFF0);
		if(maskbyte != maskfilebyte){
                   pixel_errors++;
                   printf("pixel err was %x sb= %x\n", buf[p], cmp_buf[p]);
               }
            } 
         }
      }
     }
  }

  free(buf);
  if (fp) { 
     fclose(fp); 
  }

  if (checksum != (FxU32)((double)checksum)) {
    /* error!! the checksum is not representable 
        as a double!, this should never happen */
    printf("checksum not representable as a double! (%X)\n",checksum);
    lua_pushnumber(0);
    return;
  }
  if (pixel_errors)	{
    printf("chksumFB %s (pixels = 0x%X,checksum = 0x%X, pixel_errors = 0x%X)\n",
	fname,pixel_count,checksum, pixel_errors);
  }

  lua_pushnumber((double)checksum);
  lua_pushnumber((double)pixel_errors);
}

void lm_glde_grSstIdle(void) {
  grSstIdle();
}


lua_Object glideTextureTag = LUA_NOOBJECT;

void lm_glde_loadTextureFile(void) { 
  
  /* arg1 = string filename, returns handle to a texture, or nil */
  /* arg2 = TMU number */

  char *filename = luaL_check_string(1); // filename
  GrMipMapId_t    triTexMap;
  Gu3dfInfo       fileInfo;
  GrChipID_t      tmu = GLDE_CHECKARG(2,GrChipID_t);
  

    /* Read in texture file  */
  if ( gu3dfGetInfo( filename, &fileInfo ) )  {
    fileInfo.data = (void *)malloc( fileInfo.mem_required );

    if ( fileInfo.data == 0 ) {
      fprintf( stderr, "out of memory for texture file %s\n", filename );
      free( fileInfo.data );
      return;
    }

    if ( !gu3dfLoad( filename, &fileInfo ) ) {
      fprintf( stderr, "could not load texture file %s\n", filename );
      free( fileInfo.data );
      return;
    }

    triTexMap = guTexAllocateMemory(  tmu, GR_MIPMAPLEVELMASK_BOTH,
                                  fileInfo.header.width, fileInfo.header.height,
                                  fileInfo.header.format,
                                  GR_MIPMAP_NEAREST,
                                  fileInfo.header.small_lod, fileInfo.header.large_lod,
                                  fileInfo.header.aspect_ratio,
                                  GR_TEXTURECLAMP_WRAP, GR_TEXTURECLAMP_WRAP,
                                  GR_TEXTUREFILTER_BILINEAR, GR_TEXTUREFILTER_BILINEAR,
                                  0.0F,
                                  FXFALSE );
    if ( triTexMap == GR_NULL_MIPMAP_HANDLE ) {
      fprintf( stderr, "could not allocate memory for texture file %s\n", filename );
      free( fileInfo.data );
      return;
    }

    guTexDownloadMipMap( triTexMap, fileInfo.data, &fileInfo.table.nccTable );
    free( fileInfo.data );
  } else {
    fprintf( stderr, "could not get info on %s\n", filename );
    return;
  }

  /* everything worked! so we need to setup the lua userdata */

#if 0  
  if (glideTextureTag == LUA_NOOBJECT) {
    glideTextureTag=lua_newtag();           /* sets up generic function interception tag */
    lua_pushCclosure(lm_glde_freeTexture, 0);
    lua_settagmethod(x, "gc");
  }
    
  tb = lua_createtable();       /* create a table with two elements */

  lua_pushobject(tb);
  lua_settag(glideTextureTag);

  lua_pushobject(tb);
  lua_pushnumber(1.0);
  lua_pushuserdata(triTexMap); // glide texture handle
  lua_settable();
#endif

  lua_pushuserdata( (void *)triTexMap );
}

void lm_glde_guTexSource(void) { /* setup a texture source */
  lua_Object      texObj;
  GrMipMapId_t    texHandle;  

  texObj = lua_getparam(1); // texture handle

  if (!lua_isuserdata(texObj)) {
    lua_error("guTexSource() needs a texture handle");
    return;
  }
  texHandle = (GrMipMapId_t) lua_getuserdata(texObj);

  grAlphaBlendFunction (GR_BLEND_ONE, GR_BLEND_ZERO, GR_BLEND_ONE, GR_BLEND_ZERO);
  grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,  GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_NONE,
    GR_COMBINE_OTHER_TEXTURE, FXFALSE  );
  
 
  guColorCombineFunction(GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB);
  grTexCombineFunction(GR_TMU0, GR_TEXTURECOMBINE_DECAL);
  grTexCombineFunction(GR_TMU1, GR_TEXTURECOMBINE_DECAL);
  guTexSource(texHandle);

}


#ifdef GPLAY_DONE
void lm_glde_gplayTrace(void) {
  char *dirname;
  FxU32 start, end;

  dirname = luaL_check_string(1);
  start = luaL_check_number(2);
  end = luaL_check_number(3);
  lua_callback_fn = lua_getparam(4);  // callback function

  mdc_l_push_fxbool(gplayTrace(dirname,start,end,mdcl_chksumCallback));
}
#endif


/* here comes the glide stuff! */

#include <glide.h>
#include <gdebug.h>

void mdc_glideErrorCallback(const char *string, FxBool fatal) {
  if (fatal == FXTRUE) {
    printf("GLD_FATAL_ERR: %s", string);
    exit(1);
  } else {
    printf("GLD_ERR: %s",string);
  }
  
}

// #include "env.h"

void bansheeStartGlide(LPCARDINFO card) {
  GrVertex vtx;

  printf("Trying to start glide...\n");
  grErrorSetCallback(mdc_glideErrorCallback);


  setenv("FX_GLIDE_NO_SPLASH","1",0);
  grGlideInit();
  grSstSelect( 0 );
  
  grSstWinOpen( 0, GR_RESOLUTION_640x480, GR_REFRESH_60Hz, GR_COLORFORMAT_ABGR,
		GR_ORIGIN_UPPER_LEFT, 2, 1);

  grBufferClear( 0x0, 0, GR_WDEPTHVALUE_FARTHEST);
  
  grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
                 GR_COMBINE_FACTOR_NONE,
                 GR_COMBINE_LOCAL_CONSTANT,
                 GR_COMBINE_OTHER_NONE,
                 FXFALSE);
  grConstantColorValue(0xFFFFFF);

  vtx.x = 10; vtx.y = 10;
  grDrawPoint( &vtx );

  grBufferSwap( 1 );

}

void lm_glde_startglide(void) {
    bansheeStartGlide(card);
}

void lm_glde_grTexCombine(void) {    /* GrChipID_t, GrCombineFunction_t, GrCombineFactor_t, 
                                       GrCombineFunction_t, GrCombineFactor, FxBool, FxBool */
  grTexCombine(
    GLDE_CHECKARG(1,GrChipID_t),
    GLDE_CHECKARG(2,GrCombineFunction_t),
    GLDE_CHECKARG(3,GrCombineFactor_t),
    GLDE_CHECKARG(4,GrCombineFunction_t),
    GLDE_CHECKARG(5,GrCombineFactor_t),
    GLDE_CHECKARG(6,FxBool),
    GLDE_CHECKARG(7,FxBool)
    );

}

void lm_glde_grTexMipMapMode(void) { /* GrChipID_t,   , FxBool */
  grTexMipMapMode(
    GLDE_CHECKARG(1,GrChipID_t),
    GLDE_CHECKARG(2,GrMipMapMode_t),
    GLDE_CHECKARG(3,FxBool)
    );
}

void lm_glde_grTexFilterMode(void) { /* GrChipID_t, GrTextureFilterMode_t, GrTextureFilterMode_t */
  grTexFilterMode(
    GLDE_CHECKARG(1,GrChipID_t),
    GLDE_CHECKARG(2,GrTextureFilterMode_t),
    GLDE_CHECKARG(3,GrTextureFilterMode_t)
    );
};

void lm_glde_grHints(void) {
  grHints(
    GLDE_CHECKARG(1,GrHint_t),              // type
    GLDE_CHECKARG(2,GrSTWHint_t)            // hintMask
   );
}

void lm_glde_grSliCtrl(void) {
     printf("Enabling SLI...\n");
//   grSliCtrl();
}

void lm_glde_grDisableSliCtrl(void) {
     printf("Disabling SLI...\n");
//   grDisableSliCtrl();
}

void lm_glde_grAASetup(void) {
  FxU32 aaCtrl_status;

  aaCtrl_status = luaL_check_number(1);

  if (aaCtrl_status) {		// Enable Anti-aliasing
     printf("Enabling Anti-Aliasing...\n");
//     grAAOffsetValue(xOffset, yOffset, minchipid, maxchipid, 1);
  }
  else   {			// Disable Anti-aliasing
     printf("Disabling Anti-Aliasing...\n");
//     grAAOffsetValue(xOffset, yOffset, minchipid, maxchipid, 0);
  }

}


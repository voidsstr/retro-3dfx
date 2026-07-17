/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
*/

/*
**  state.h
*/

#define MAX_TMU                 8

#define MAX_ENABLE_DISABLE     16
#define MAX_FOG_TABLE_SIZE    128
#define MAX_GAMMA_TABLE_SIZE  256
#define MAX_RESET             256
#define MAX_VERTEX_LAYOUT     256

typedef struct {
  FxBool set;
  GrAlphaBlendFnc_t rgb_sf;
  GrAlphaBlendFnc_t rgb_df;
  GrAlphaBlendFnc_t alpha_sf;
  GrAlphaBlendFnc_t alpha_df;
} grAlphaBlendFunctionState;

typedef struct {
  FxBool set;
  GrCombineFunction_t function;
  GrCombineFactor_t factor;
  GrCombineLocal_t local;
  GrCombineOther_t other;
  FxBool invert;
} grAlphaCombineState;

typedef struct {
  FxBool set;
  FxBool enable;
} grAlphaControlsITRGBLightingState;

typedef struct {
  FxBool set;
  GrCmpFnc_t function;
} grAlphaTestFunctionState;

typedef struct {
  FxBool set;
  GrAlpha_t value;
} grAlphaTestReferenceValueState;

typedef struct {
  FxBool set;
  GrColor_t color;
  GrAlpha_t alpha;
  FxU32 depth;
} grBufferClearState;

typedef struct {
  FxBool set;
  int swap_interval;
} grBufferSwapState;

typedef struct {
  FxBool set;
  GrChromakeyMode_t mode;
} grChromakeyModeState;

typedef struct {
  FxBool set;
  GrColor_t value;
} grChromakeyValueState;

typedef struct {
  FxBool set;
  FxU32 minx;
  FxU32 miny;
  FxU32 maxx;
  FxU32 maxy;
} grClipWindowState;

typedef struct {
  FxBool set;
  GrCombineFunction_t function;
  GrCombineFactor_t factor;
  GrCombineLocal_t local;
  GrCombineOther_t other;
  FxBool invert;
} grColorCombineState;

typedef struct {
  FxBool set;
  FxBool rgb;
  FxBool a;
} grColorMaskState;

typedef struct {
  FxBool set;
  GrColor_t value;
} grConstantColorValueState;

typedef struct {
  FxBool set;
  GrCoordinateSpaceMode_t mode;
} grCoordinateSpaceState;

typedef struct {
  FxBool set;
  GrCullMode_t mode;
} grCullModeState;

typedef struct {
  FxBool set;
  FxI32 level;
} grDepthBiasLevelState;

typedef struct {
  FxBool set;
  GrCmpFnc_t function;
} grDepthBufferFunctionState;

typedef struct {
  FxBool set;
  GrDepthBufferMode_t mode;
} grDepthBufferModeState;

typedef struct {
  FxBool set;
  FxBool mask;
} grDepthMaskState;

typedef struct {
  FxBool set;
  FxFloat n;
  FxFloat f;
} grDepthRangeState;

typedef struct {
  FxBool set;
} grDisableAllEffectsState;

typedef struct {
  FxBool set;
  GrDitherMode_t mode;
} grDitherModeState;

typedef struct {
  FxBool set[MAX_ENABLE_DISABLE];
  FxBool enable[MAX_ENABLE_DISABLE];
} grEnableDisableState;

typedef struct {
  FxBool set;
  GrColor_t fogcolor;
} grFogColorValueState;

typedef struct {
  FxBool set;
  GrFogMode_t mode;
} grFogModeState;

typedef struct {
  FxBool set;
  FxU32 tsize;
  GrFog_t data[MAX_FOG_TABLE_SIZE];
} grFogTableState;

typedef struct {
  FxBool set;
  GrAlpha_t alpha;
} grLfbConstantAlphaState;

typedef struct {
  FxBool set;
  FxU32 depth;
} grLfbConstantDepthState;

typedef struct {
  FxBool set;
  GrColorFormat_t colorFormat;
} grLfbWriteColorFormatState;

typedef struct {
  FxBool set;
  FxBool swizzleBytes;
  FxBool swapWords;
} grLfbWriteColorSwizzleState;

typedef struct {
  FxBool set;
  FxU32 nentries;
  FxU32 data_r[MAX_GAMMA_TABLE_SIZE];
  FxU32 data_g[MAX_GAMMA_TABLE_SIZE];
  FxU32 data_b[MAX_GAMMA_TABLE_SIZE];
} grLoadGammaTableState;

typedef struct {
  FxBool set;
  GrBuffer_t buffer;
} grRenderBufferState;

typedef struct {
  FxBool set[MAX_RESET];
} grResetState;

typedef struct {
  FxBool set;
  GrContext_t context;
} grSelectContextState;

typedef struct {
  FxBool set;
  GrOriginLocation_t origin;
} grSstOriginState;

typedef struct {
  FxBool set;
  GrTextureClampMode_t s_clampmode;
  GrTextureClampMode_t t_clampmode;
} grTexClampModeState;

typedef struct {
  FxBool set;
  GrCombineFunction_t rgb_function;
  GrCombineFactor_t rgb_factor;
  GrCombineFunction_t alpha_function;
  GrCombineFactor_t alpha_factor;
  FxBool rgb_invert;
  FxBool alpha_invert;
} grTexCombineState;

typedef struct {
  FxBool set;
  int lod_bias;
  FxU8 detail_scale;
  float detail_max;
} grTexDetailControlState;

typedef struct {
  FxBool set;
  GrTexTable_t type;   
  GuTexPalette texpalette;
  GuNccTable   ncctable;
} grTexDownloadTableState;

typedef struct {
  FxBool set;
  GrTextureFilterMode_t minfilter_mode;
  GrTextureFilterMode_t magfilter_mode;
} grTexFilterModeState;

typedef struct {
  FxBool set;
  float bias;
} grTexLodBiasValueState;

typedef struct {
  FxBool set;
  GrMipMapMode_t mode;
  FxBool lodBlend;
} grTexMipMapModeState;

typedef struct {
  FxBool set;
  FxBool enable;
} grTexMultibaseState;

typedef struct {
  FxBool set;
  GrTexBaseRange_t range;
  FxU32 startAddress;
  FxU32 evenOdd;
  GrTexInfo info;
} grTexMultibaseAddressState;

typedef struct {
  FxBool set;
  GrNCCTable_t table;
} grTexNCCTableState;

typedef struct {
  FxBool set;
  FxU32 startAddress;
  FxU32 evenOdd;
  GrTexInfo info;
} grTexSourceState;

typedef struct {
  FxBool set;
  FxI32 offset;
  FxU32 mode;
} grVertexLayoutState;

typedef struct {
  FxBool set;
  FxI32 x;
  FxI32 y;
  FxI32 width;
  FxI32 height;
} grViewportState;

typedef struct {
  FxBool set;
  FxFloat red;
  FxFloat green;
  FxFloat blue;
} guGammaCorrectionRGBState;

typedef struct {
  FxBool set;
  GrChromakeyMode_t mode;
} grChromaRangeModeExtState;

typedef struct {
  FxBool set;
  GrColor_t color;
  GrColor_t range;
  GrChromaRangeMode_t match_mode;
} grChromaRangeExtState;

typedef struct {
  FxBool set;
  GrTexChromakeyMode_t mode;
} grTexChromaModeExtState;

typedef struct {
  FxBool set;
  GrColor_t min;
  GrColor_t max;
  GrTexChromakeyMode_t mode;
} grTexChromaRangeExtState;

typedef struct {

  grAlphaBlendFunctionState         grABF;
  grAlphaCombineState               grAC;
  grAlphaControlsITRGBLightingState grACIL;
  grAlphaTestFunctionState          grATF;
  grAlphaTestReferenceValueState    grATRV;

  grBufferClearState grBC;
  grBufferSwapState  grBS;

  grChromakeyModeState      grCKM;
  grChromakeyValueState     grCKV;
  grClipWindowState         grCW;
  grColorCombineState       grCC;
  grColorMaskState          grColorMask;
  grConstantColorValueState grCCV;
  grCoordinateSpaceState    grCS;
  grCullModeState           grCullMode;

  grDepthBiasLevelState      grDBL;
  grDepthBufferFunctionState grDBF;
  grDepthBufferModeState     grDBM;
  grDepthMaskState           grDepthMask;
  grDepthRangeState          grDR;
  grDisableAllEffectsState   grDAE;
  grDitherModeState          grDitherMode;

  grEnableDisableState grED;

  grFogColorValueState grFCV;
  grFogModeState       grFM;
  grFogTableState      grFT;

  grLfbConstantAlphaState     grLCA;
  grLfbConstantDepthState     grLCD;
  grLfbWriteColorFormatState  grLWCF;
  grLfbWriteColorSwizzleState grLWCS;
  grLoadGammaTableState       grLGT;

  grRenderBufferState  grRB;
  grResetState         grR;

  grSstOriginState      grSO;

  grTexClampModeState        grTCM[MAX_TMU];
  grTexCombineState          grTC[MAX_TMU];
  grTexDetailControlState    grTDC[MAX_TMU];
  grTexDownloadTableState    grTDT;
  grTexFilterModeState       grTFM[MAX_TMU];
  grTexLodBiasValueState     grTLBV[MAX_TMU];
  grTexMipMapModeState       grTMMM[MAX_TMU];
  grTexMultibaseState        grTMB[MAX_TMU];
  grTexMultibaseAddressState grTMBA[MAX_TMU];
  grTexNCCTableState         grTNT;
  grTexSourceState           grTS[MAX_TMU];

  grVertexLayoutState  grVL[MAX_VERTEX_LAYOUT];
  grViewportState      grVP;

  guGammaCorrectionRGBState guGCRGB;

  grChromaRangeExtState     grCRE;
  grChromaRangeModeExtState grCRME;
  grTexChromaModeExtState   grTCME[MAX_TMU];
  grTexChromaRangeExtState  grTCRE[MAX_TMU];

} GlideState;

typedef struct {
  int token;
  GrChipID_t tmu;
  FxU32 startAddress;
  FxU32 evenOdd;
  GrTexInfo info;
  char filename[TEXNAMELEN+1];
  int size;
} grTexDownloadMipMapState;

typedef struct {
  int token;
  GrChipID_t tmu;
  FxU32 startAddress;
  GrLOD_t thisLod;
  GrLOD_t largeLod;
  GrAspectRatio_t aspectRatio;
  GrTextureFormat_t format;
  FxU32 evenOdd;
  char filename[TEXNAMELEN+1];
  int size;
} grTexDownloadMipMapLevelState;

typedef struct {
  int token;
  GrChipID_t tmu;
  FxU32 startAddress;
  GrLOD_t thisLod;
  GrLOD_t largeLod;
  GrAspectRatio_t aspectRatio;
  GrTextureFormat_t format;
  FxU32 evenOdd;
  int start;
  int end;
  char filename[TEXNAMELEN+1];
  int size;
} grTexDownloadMipMapLevelPartialState;

extern GlideState gstate;

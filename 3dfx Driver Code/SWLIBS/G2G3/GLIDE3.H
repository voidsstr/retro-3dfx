/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
*/

/*
** GLIDE.H
**
** The following #defines are relevant when using Glide:
**
** One of the following "platform constants" must be defined during
** compilation:
**
**            __DOS__           Defined for 32-bit DOS applications
**            __WIN32__         Defined for 32-bit Windows applications
**            __sparc__         Defined for Sun Solaris/SunOS
**            __linux__         Defined for Linux applications
**            __IRIX__          Defined for SGI Irix applications
**
*/
#ifndef __GLIDE3_H__
#define __GLIDE3_H__

#include <3dfx.h>
#include <glidesys.h>
#include <sst1vid.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** -----------------------------------------------------------------------
** TYPE DEFINITIONS
** -----------------------------------------------------------------------
*/
typedef FxU32 G3GrColor_t;
typedef FxU8  G3GrAlpha_t;
typedef FxU32 G3GrMipMapId_t;
typedef FxU8  G3GrFog_t;
typedef FxU32 G3GrContext_t;
typedef int (__stdcall *G3GrProc)();

/*
** -----------------------------------------------------------------------
** CONSTANTS AND TYPES
** -----------------------------------------------------------------------
*/
#define MAX_NUM_SST            4

#define G3GR_NULL_MIPMAP_HANDLE  ((G3GrMipMapId_t) -1)

#define G3GR_MIPMAPLEVELMASK_EVEN  FXBIT(0)
#define G3GR_MIPMAPLEVELMASK_ODD  FXBIT(1)
#define G3GR_MIPMAPLEVELMASK_BOTH (G3GR_MIPMAPLEVELMASK_EVEN | G3GR_MIPMAPLEVELMASK_ODD )

#define G3GR_LODBIAS_BILINEAR     0.5
#define G3GR_LODBIAS_TRILINEAR    0.0

typedef FxI32 G3GrChipID_t;
#define G3GR_TMU0         0x0
#define G3GR_TMU1         0x1
#define G3GR_TMU2         0x2

#define G3GR_FBI          0x0

typedef FxI32 G3GrCombineFunction_t;
#define G3GR_COMBINE_FUNCTION_ZERO        0x0
#define G3GR_COMBINE_FUNCTION_NONE        G3GR_COMBINE_FUNCTION_ZERO
#define G3GR_COMBINE_FUNCTION_LOCAL       0x1
#define G3GR_COMBINE_FUNCTION_LOCAL_ALPHA 0x2
#define G3GR_COMBINE_FUNCTION_SCALE_OTHER 0x3
#define G3GR_COMBINE_FUNCTION_BLEND_OTHER G3GR_COMBINE_FUNCTION_SCALE_OTHER
#define G3GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL 0x4
#define G3GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL_ALPHA 0x5 
#define G3GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL 0x6
#define G3GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL 0x7
#define G3GR_COMBINE_FUNCTION_BLEND G3GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL
#define G3GR_COMBINE_FUNCTION_SCALE_OTHER_MINUS_LOCAL_ADD_LOCAL_ALPHA 0x8
#define G3GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL 0x9
#define G3GR_COMBINE_FUNCTION_BLEND_LOCAL G3GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL
#define G3GR_COMBINE_FUNCTION_SCALE_MINUS_LOCAL_ADD_LOCAL_ALPHA 0x10

typedef FxI32 G3GrCombineFactor_t;
#define G3GR_COMBINE_FACTOR_ZERO          0x0
#define G3GR_COMBINE_FACTOR_NONE          G3GR_COMBINE_FACTOR_ZERO
#define G3GR_COMBINE_FACTOR_LOCAL         0x1
#define G3GR_COMBINE_FACTOR_OTHER_ALPHA   0x2
#define G3GR_COMBINE_FACTOR_LOCAL_ALPHA   0x3
#define G3GR_COMBINE_FACTOR_TEXTURE_ALPHA 0x4
#define G3GR_COMBINE_FACTOR_TEXTURE_RGB   0x5
#define G3GR_COMBINE_FACTOR_DETAIL_FACTOR G3GR_COMBINE_FACTOR_TEXTURE_ALPHA
#define G3GR_COMBINE_FACTOR_LOD_FRACTION  0x5
#define G3GR_COMBINE_FACTOR_ONE           0x8
#define G3GR_COMBINE_FACTOR_ONE_MINUS_LOCAL 0x9
#define G3GR_COMBINE_FACTOR_ONE_MINUS_OTHER_ALPHA 0xa
#define G3GR_COMBINE_FACTOR_ONE_MINUS_LOCAL_ALPHA 0xb
#define G3GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA 0xc
#define G3GR_COMBINE_FACTOR_ONE_MINUS_DETAIL_FACTOR G3GR_COMBINE_FACTOR_ONE_MINUS_TEXTURE_ALPHA
#define G3GR_COMBINE_FACTOR_ONE_MINUS_LOD_FRACTION 0xd


typedef FxI32 G3GrCombineLocal_t;
#define G3GR_COMBINE_LOCAL_ITERATED 0x0
#define G3GR_COMBINE_LOCAL_CONSTANT 0x1
#define G3GR_COMBINE_LOCAL_NONE G3GR_COMBINE_LOCAL_CONSTANT
#define G3GR_COMBINE_LOCAL_DEPTH  0x2

typedef FxI32 G3GrCombineOther_t;
#define G3GR_COMBINE_OTHER_ITERATED 0x0
#define G3GR_COMBINE_OTHER_TEXTURE 0x1
#define G3GR_COMBINE_OTHER_CONSTANT 0x2
#define G3GR_COMBINE_OTHER_NONE G3GR_COMBINE_OTHER_CONSTANT


typedef FxI32 G3GrAlphaSource_t;
#define G3GR_ALPHASOURCE_CC_ALPHA 0x0
#define G3GR_ALPHASOURCE_ITERATED_ALPHA 0x1
#define G3GR_ALPHASOURCE_TEXTURE_ALPHA 0x2
#define G3GR_ALPHASOURCE_TEXTURE_ALPHA_TIMES_ITERATED_ALPHA 0x3


typedef FxI32 G3GrColorCombineFnc_t;
#define G3GR_COLORCOMBINE_ZERO 0x0
#define G3GR_COLORCOMBINE_CCRGB 0x1
#define G3GR_COLORCOMBINE_ITRGB 0x2
#define G3GR_COLORCOMBINE_ITRGB_DELTA0 0x3
#define G3GR_COLORCOMBINE_DECAL_TEXTURE 0x4
#define G3GR_COLORCOMBINE_TEXTURE_TIMES_CCRGB 0x5
#define G3GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB 0x6
#define G3GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_DELTA0 0x7
#define G3GR_COLORCOMBINE_TEXTURE_TIMES_ITRGB_ADD_ALPHA 0x8
#define G3GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA 0x9
#define G3GR_COLORCOMBINE_TEXTURE_TIMES_ALPHA_ADD_ITRGB 0xa
#define G3GR_COLORCOMBINE_TEXTURE_ADD_ITRGB 0xb
#define G3GR_COLORCOMBINE_TEXTURE_SUB_ITRGB 0xc
#define G3GR_COLORCOMBINE_CCRGB_BLEND_ITRGB_ON_TEXALPHA 0xd
#define G3GR_COLORCOMBINE_DIFF_SPEC_A 0xe
#define G3GR_COLORCOMBINE_DIFF_SPEC_B 0xf
#define G3GR_COLORCOMBINE_ONE 0x10

typedef FxI32 G3GrAlphaBlendFnc_t;
#define G3GR_BLEND_ZERO 0x0
#define G3GR_BLEND_SRC_ALPHA 0x1
#define G3GR_BLEND_SRC_COLOR 0x2
#define G3GR_BLEND_DST_COLOR G3GR_BLEND_SRC_COLOR
#define G3GR_BLEND_DST_ALPHA 0x3 
#define G3GR_BLEND_ONE 0x4
#define G3GR_BLEND_ONE_MINUS_SRC_ALPHA 0x5
#define G3GR_BLEND_ONE_MINUS_SRC_COLOR 0x6
#define G3GR_BLEND_ONE_MINUS_DST_COLOR G3GR_BLEND_ONE_MINUS_SRC_COLOR 
#define G3GR_BLEND_ONE_MINUS_DST_ALPHA 0x7
#define G3GR_BLEND_RESERVED_8 0x8
#define G3GR_BLEND_RESERVED_9 0x9
#define G3GR_BLEND_RESERVED_A 0xa
#define G3GR_BLEND_RESERVED_B 0xb
#define G3GR_BLEND_RESERVED_C 0xc
#define G3GR_BLEND_RESERVED_D 0xd
#define G3GR_BLEND_RESERVED_E 0xe
#define G3GR_BLEND_ALPHA_SATURATE 0xf
#define G3GR_BLEND_PREFOG_COLOR G3GR_BLEND_ALPHA_SATURATE

typedef FxI32 G3GrAspectRatio_t;
#define G3GR_ASPECT_LOG2_8x1        3       /* 8W x 1H */
#define G3GR_ASPECT_LOG2_4x1        2       /* 4W x 1H */
#define G3GR_ASPECT_LOG2_2x1        1       /* 2W x 1H */
#define G3GR_ASPECT_LOG2_1x1        0       /* 1W x 1H */
#define G3GR_ASPECT_LOG2_1x2       -1       /* 1W x 2H */
#define G3GR_ASPECT_LOG2_1x4       -2       /* 1W x 4H */
#define G3GR_ASPECT_LOG2_1x8       -3       /* 1W x 8H */

typedef FxI32 G3GrBuffer_t;
#define G3GR_BUFFER_FRONTBUFFER   0x0
#define G3GR_BUFFER_BACKBUFFER    0x1
#define G3GR_BUFFER_AUXBUFFER     0x2
#define G3GR_BUFFER_DEPTHBUFFER   0x3
#define G3GR_BUFFER_ALPHABUFFER   0x4
#define G3GR_BUFFER_TRIPLEBUFFER  0x5

typedef FxI32 G3GrChromakeyMode_t;
#define G3GR_CHROMAKEY_DISABLE    0x0
#define G3GR_CHROMAKEY_ENABLE     0x1

typedef FxI32 G3GrChromaRangeMode_t;
#define G3GR_CHROMARANGE_RGB_ALL_EXT  0x0

#define G3GR_CHROMARANGE_DISABLE_EXT  0x00
#define G3GR_CHROMARANGE_ENABLE_EXT   0x01

typedef FxI32 G3GrTexChromakeyMode_t;
#define G3GR_TEXCHROMA_DISABLE_EXT               0x0
#define G3GR_TEXCHROMA_ENABLE_EXT                0x1

#define G3GR_TEXCHROMARANGE_RGB_ALL_EXT  0x0

typedef FxI32 G3GrCmpFnc_t;
#define G3GR_CMP_NEVER    0x0
#define G3GR_CMP_LESS     0x1
#define G3GR_CMP_EQUAL    0x2
#define G3GR_CMP_LEQUAL   0x3
#define G3GR_CMP_GREATER  0x4
#define G3GR_CMP_NOTEQUAL 0x5
#define G3GR_CMP_GEQUAL   0x6
#define G3GR_CMP_ALWAYS   0x7

typedef FxI32 G3GrColorFormat_t;
#define G3GR_COLORFORMAT_ARGB     0x0
#define G3GR_COLORFORMAT_ABGR     0x1

#define G3GR_COLORFORMAT_RGBA     0x2
#define G3GR_COLORFORMAT_BGRA     0x3

typedef FxI32 G3GrCullMode_t;
#define G3GR_CULL_DISABLE         0x0
#define G3GR_CULL_NEGATIVE        0x1
#define G3GR_CULL_POSITIVE        0x2

typedef FxI32 G3GrDepthBufferMode_t;
#define G3GR_DEPTHBUFFER_DISABLE                  0x0
#define G3GR_DEPTHBUFFER_ZBUFFER                  0x1
#define G3GR_DEPTHBUFFER_WBUFFER                  0x2
#define G3GR_DEPTHBUFFER_ZBUFFER_COMPARE_TO_BIAS  0x3
#define G3GR_DEPTHBUFFER_WBUFFER_COMPARE_TO_BIAS  0x4

typedef FxI32 G3GrDitherMode_t;
#define G3GR_DITHER_DISABLE       0x0
#define G3GR_DITHER_2x2           0x1
#define G3GR_DITHER_4x4           0x2

typedef FxI32 G3GrFogMode_t;
#define G3GR_FOG_DISABLE                     0x0
#define G3GR_FOG_WITH_TABLE_ON_FOGCOORD_EXT  0x1
#define G3GR_FOG_WITH_TABLE_ON_Q             0x2
#define G3GR_FOG_WITH_TABLE_ON_W             G3GR_FOG_WITH_TABLE_ON_Q
#define G3GR_FOG_WITH_ITERATED_Z             0x3
#define G3GR_FOG_MULT2                       0x100
#define G3GR_FOG_ADD2                        0x200

typedef FxU32 G3GrLock_t;
#define G3GR_LFB_READ_ONLY  0x00
#define G3GR_LFB_WRITE_ONLY 0x01
#define G3GR_LFB_IDLE       0x00
#define G3GR_LFB_NOIDLE     0x10

typedef FxI32 G3GrLfbBypassMode_t;
#define G3GR_LFBBYPASS_DISABLE    0x0
#define G3GR_LFBBYPASS_ENABLE     0x1

typedef FxI32 G3GrLfbWriteMode_t;
#define G3GR_LFBWRITEMODE_565        0x0 /* RGB:RGB */
#define G3GR_LFBWRITEMODE_555        0x1 /* RGB:RGB */
#define G3GR_LFBWRITEMODE_1555       0x2 /* ARGB:ARGB */
#define G3GR_LFBWRITEMODE_RESERVED1  0x3
#define G3GR_LFBWRITEMODE_888        0x4 /* RGB */
#define G3GR_LFBWRITEMODE_8888       0x5 /* ARGB */
#define G3GR_LFBWRITEMODE_RESERVED2  0x6
#define G3GR_LFBWRITEMODE_RESERVED3  0x7
#define G3GR_LFBWRITEMODE_RESERVED4  0x8
#define G3GR_LFBWRITEMODE_RESERVED5  0x9
#define G3GR_LFBWRITEMODE_RESERVED6  0xa
#define G3GR_LFBWRITEMODE_RESERVED7  0xb
#define G3GR_LFBWRITEMODE_565_DEPTH  0xc /* RGB:DEPTH */
#define G3GR_LFBWRITEMODE_555_DEPTH  0xd /* RGB:DEPTH */
#define G3GR_LFBWRITEMODE_1555_DEPTH 0xe /* ARGB:DEPTH */
#define G3GR_LFBWRITEMODE_ZA16       0xf /* DEPTH:DEPTH */
#define G3GR_LFBWRITEMODE_ANY        0xFF


typedef FxI32 G3GrOriginLocation_t;
#define G3GR_ORIGIN_UPPER_LEFT    0x0
#define G3GR_ORIGIN_LOWER_LEFT    0x1
#define G3GR_ORIGIN_ANY           0xFF

typedef struct {
    int                size;
    void               *lfbPtr;
    FxU32              strideInBytes;        
    G3GrLfbWriteMode_t   writeMode;
    G3GrOriginLocation_t origin;
} G3GrLfbInfo_t;

typedef FxI32 G3GrLOD_t;
#define G3GR_LOD_LOG2_256         0x8
#define G3GR_LOD_LOG2_128         0x7
#define G3GR_LOD_LOG2_64          0x6
#define G3GR_LOD_LOG2_32          0x5
#define G3GR_LOD_LOG2_16          0x4
#define G3GR_LOD_LOG2_8           0x3
#define G3GR_LOD_LOG2_4           0x2
#define G3GR_LOD_LOG2_2           0x1
#define G3GR_LOD_LOG2_1           0x0

typedef FxI32 G3GrMipMapMode_t;
#define G3GR_MIPMAP_DISABLE               0x0 /* no mip mapping  */
#define G3GR_MIPMAP_NEAREST               0x1 /* use nearest mipmap */
#define G3GR_MIPMAP_NEAREST_DITHER        0x2 /* G3GR_MIPMAP_NEAREST + LOD dith */

typedef FxI32 G3GrSmoothingMode_t;
#define G3GR_SMOOTHING_DISABLE    0x0
#define G3GR_SMOOTHING_ENABLE     0x1

typedef FxI32 G3GrTextureClampMode_t;
#define G3GR_TEXTURECLAMP_WRAP        0x0
#define G3GR_TEXTURECLAMP_CLAMP       0x1
#define G3GR_TEXTURECLAMP_MIRROR_EXT  0x2

typedef FxI32 G3GrTextureCombineFnc_t;
#define G3GR_TEXTURECOMBINE_ZERO          0x0 /* texout = 0 */
#define G3GR_TEXTURECOMBINE_DECAL         0x1 /* texout = texthis */
#define G3GR_TEXTURECOMBINE_OTHER         0x2 /* this TMU in passthru mode */
#define G3GR_TEXTURECOMBINE_ADD           0x3 /* tout = tthis + t(this+1) */
#define G3GR_TEXTURECOMBINE_MULTIPLY      0x4 /* texout = tthis * t(this+1) */
#define G3GR_TEXTURECOMBINE_SUBTRACT      0x5 /* Sutract from upstream TMU */
#define G3GR_TEXTURECOMBINE_DETAIL        0x6 /* detail--detail on tthis */
#define G3GR_TEXTURECOMBINE_DETAIL_OTHER  0x7 /* detail--detail on tthis+1 */
#define G3GR_TEXTURECOMBINE_TRILINEAR_ODD 0x8 /* trilinear--odd levels tthis*/
#define G3GR_TEXTURECOMBINE_TRILINEAR_EVEN 0x9 /*trilinear--even levels tthis*/
#define G3GR_TEXTURECOMBINE_ONE           0xa /* texout = 0xFFFFFFFF */

typedef FxI32 G3GrTextureFilterMode_t;
#define G3GR_TEXTUREFILTER_POINT_SAMPLED  0x0
#define G3GR_TEXTUREFILTER_BILINEAR       0x1

typedef FxI32 G3GrTextureFormat_t;
#define G3GR_TEXFMT_8BIT                  0x0
#define G3GR_TEXFMT_RGB_332 G3GR_TEXFMT_8BIT
#define G3GR_TEXFMT_YIQ_422               0x1
#define G3GR_TEXFMT_ALPHA_8               0x2 /* (0..0xFF) alpha     */
#define G3GR_TEXFMT_INTENSITY_8           0x3 /* (0..0xFF) intensity */
#define G3GR_TEXFMT_ALPHA_INTENSITY_44    0x4
#define G3GR_TEXFMT_P_8                   0x5 /* 8-bit palette */
#define G3GR_TEXFMT_RSVD0                 0x6
#define G3GR_TEXFMT_RSVD1                 0x7
#define G3GR_TEXFMT_16BIT                 0x8
#define G3GR_TEXFMT_ARGB_8332 G3GR_TEXFMT_16BIT
#define G3GR_TEXFMT_AYIQ_8422             0x9
#define G3GR_TEXFMT_RGB_565               0xa
#define G3GR_TEXFMT_ARGB_1555             0xb
#define G3GR_TEXFMT_ARGB_4444             0xc
#define G3GR_TEXFMT_ALPHA_INTENSITY_88    0xd
#define G3GR_TEXFMT_AP_88                 0xe /* 8-bit alpha 8-bit palette */
#define G3GR_TEXFMT_RSVD2                 0xf

typedef FxU32 G3GrTexTable_t;
#define G3GR_TEXTABLE_NCC0                 0x0
#define G3GR_TEXTABLE_NCC1                 0x1
#define G3GR_TEXTABLE_PALETTE              0x2
#define G3GR_TEXTABLE_PALETTE_6666_EXT     0x3

typedef FxU32 G3GrNCCTable_t;
#define G3GR_NCCTABLE_NCC0    0x0
#define G3GR_NCCTABLE_NCC1    0x1

typedef FxU32 G3GrTexBaseRange_t;
#define G3GR_TEXBASE_256      0x3
#define G3GR_TEXBASE_128      0x2
#define G3GR_TEXBASE_64       0x1
#define G3GR_TEXBASE_32_TO_1  0x0


typedef FxU32 G3GrEnableMode_t;
#define G3GR_MODE_DISABLE     0x0
#define G3GR_MODE_ENABLE      0x1

#define G3GR_AA_ORDERED            0x01
#define G3GR_ALLOW_MIPMAP_DITHER   0x02
#define G3GR_PASSTHRU              0x03
#define G3GR_SHAMELESS_PLUG        0x04
#define G3GR_VIDEO_SMOOTHING       0x05

typedef FxU32 G3GrCoordinateSpaceMode_t;
#define G3GR_WINDOW_COORDS    0x00
#define G3GR_CLIP_COORDS      0x01

/* Types of data in strips */
#define G3GR_FLOAT        0
#define G3GR_U8           1

/* Parameters for strips */
#define G3GR_PARAM_XY       0x01
#define G3GR_PARAM_Z        0x02
#define G3GR_PARAM_W        0x03
#define G3GR_PARAM_Q        0x04
#define G3GR_PARAM_FOG_EXT  0x05

#define G3GR_PARAM_A        0x10

#define G3GR_PARAM_RGB      0x20

#define G3GR_PARAM_PARGB    0x30

#define G3GR_PARAM_ST0      0x40
#define G3GR_PARAM_ST1      G3GR_PARAM_ST0+1
#define G3GR_PARAM_ST2      G3GR_PARAM_ST0+2

#define G3GR_PARAM_Q0       0x50
#define G3GR_PARAM_Q1       G3GR_PARAM_Q0+1
#define G3GR_PARAM_Q2       G3GR_PARAM_Q0+2

#define G3GR_PARAM_DISABLE  0x00
#define G3GR_PARAM_ENABLE   0x01

/*
** grDrawVertexArray/grDrawVertexArrayContiguous primitive type
*/
#define G3GR_POINTS                        0
#define G3GR_LINE_STRIP                    1
#define G3GR_LINES                         2
#define G3GR_POLYGON                       3
#define G3GR_TRIANGLE_STRIP                4
#define G3GR_TRIANGLE_FAN                  5
#define G3GR_TRIANGLES                     6
#define G3GR_TRIANGLE_STRIP_CONTINUE       7
#define G3GR_TRIANGLE_FAN_CONTINUE         8

/* 
** grGet/grReset types
*/
#define G3GR_BITS_DEPTH                   0x01
#define G3GR_BITS_RGBA                    0x02
#define G3GR_FIFO_FULLNESS                0x03
#define G3GR_FOG_TABLE_ENTRIES            0x04
#define G3GR_GAMMA_TABLE_ENTRIES          0x05
#define G3GR_GLIDE_STATE_SIZE             0x06
#define G3GR_GLIDE_VERTEXLAYOUT_SIZE      0x07
#define G3GR_IS_BUSY                      0x08
#define G3GR_LFB_PIXEL_PIPE               0x09
#define G3GR_MAX_TEXTURE_SIZE             0x0a
#define G3GR_MAX_TEXTURE_ASPECT_RATIO     0x0b
#define G3GR_MEMORY_FB                    0x0c
#define G3GR_MEMORY_TMU                   0x0d
#define G3GR_MEMORY_UMA                   0x0e
#define G3GR_NUM_BOARDS                   0x0f
#define G3GR_NON_POWER_OF_TWO_TEXTURES    0x10
#define G3GR_NUM_FB                       0x11
#define G3GR_NUM_SWAP_HISTORY_BUFFER      0x12
#define G3GR_NUM_TMU                      0x13
#define G3GR_PENDING_BUFFERSWAPS          0x14
#define G3GR_REVISION_FB                  0x15
#define G3GR_REVISION_TMU                 0x16
#define G3GR_STATS_LINES                  0x17  /* grGet/grReset */
#define G3GR_STATS_PIXELS_AFUNC_FAIL      0x18
#define G3GR_STATS_PIXELS_CHROMA_FAIL     0x19
#define G3GR_STATS_PIXELS_DEPTHFUNC_FAIL  0x1a
#define G3GR_STATS_PIXELS_IN              0x1b
#define G3GR_STATS_PIXELS_OUT             0x1c
#define G3GR_STATS_PIXELS                 0x1d  /* grReset */
#define G3GR_STATS_POINTS                 0x1e  /* grGet/grReset */
#define G3GR_STATS_TRIANGLES_IN           0x1f
#define G3GR_STATS_TRIANGLES_OUT          0x20
#define G3GR_STATS_TRIANGLES              0x21  /* grReset */
#define G3GR_SWAP_HISTORY                 0x22
#define G3GR_SUPPORTS_PASSTHRU            0x23
#define G3GR_TEXTURE_ALIGN                0x24
#define G3GR_VIDEO_POSITION               0x25
#define G3GR_VIEWPORT                     0x26
#define G3GR_WDEPTH_MIN_MAX               0x27
#define G3GR_ZDEPTH_MIN_MAX               0x28
#define G3GR_VERTEX_PARAMETER             0x29

/*
** grGetString types
*/
#define G3GR_EXTENSION                    0xa0
#define G3GR_HARDWARE                     0xa1
#define G3GR_RENDERER                     0xa2
#define G3GR_VENDOR                       0xa3
#define G3GR_VERSION                      0xa4

/*
** -----------------------------------------------------------------------
** STRUCTURES
** -----------------------------------------------------------------------
*/

typedef struct {
    G3GrLOD_t           smallLodLog2;
    G3GrLOD_t           largeLodLog2;
    G3GrAspectRatio_t   aspectRatioLog2;
    G3GrTextureFormat_t format;
    void              *data;
} G3GrTexInfo;

typedef struct G3GrSstPerfStats_s {
  FxU32  pixelsIn;              /* # pixels processed (minus buffer clears) */
  FxU32  chromaFail;            /* # pixels not drawn due to chroma key */ 
  FxU32  zFuncFail;             /* # pixels not drawn due to Z comparison */
  FxU32  aFuncFail;             /* # pixels not drawn due to alpha comparison */
  FxU32  pixelsOut;             /* # pixels drawn (including buffer clears) */
} G3GrSstPerfStats_t;

typedef struct {
  GrScreenResolution_t resolution;
  GrScreenRefresh_t    refresh;
  int                  numColorBuffers;
  int                  numAuxBuffers;
} G3GrResolution;

typedef G3GrResolution GlideResolution;

#define G3GR_QUERY_ANY  ((FxU32)(~0))

typedef FxU32 G3GrLfbSrcFmt_t;
#define G3GR_LFB_SRC_FMT_565          0x00
#define G3GR_LFB_SRC_FMT_555          0x01
#define G3GR_LFB_SRC_FMT_1555         0x02
#define G3GR_LFB_SRC_FMT_888          0x04
#define G3GR_LFB_SRC_FMT_8888         0x05
#define G3GR_LFB_SRC_FMT_565_DEPTH    0x0c
#define G3GR_LFB_SRC_FMT_555_DEPTH    0x0d
#define G3GR_LFB_SRC_FMT_1555_DEPTH   0x0e
#define G3GR_LFB_SRC_FMT_ZA16         0x0f
#define G3GR_LFB_SRC_FMT_RLE16        0x80

#ifdef H3D
#define G3GR_HINT_H3DENABLE               4
#undef  G3GR_HINTTYPE_MAX
#define G3GR_HINTTYPE_MAX 4
#endif


#ifdef __cplusplus
}
#endif

#include <glideutl.h>

#endif /* __GLIDE_H__ */

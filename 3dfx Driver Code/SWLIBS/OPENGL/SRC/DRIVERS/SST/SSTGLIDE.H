/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
**
**
** $Revision: 2$ 
** $Date: 10/11/00 8:03:06 PM$ 
**
*/


#ifndef _SSTGLIDE_H_
#define _SSTGLIDE_H_

typedef struct {
    GrAlphaBlendFnc_t src; 
    GrAlphaBlendFnc_t dst; 
} __GlideAlphaBlendFunction;

typedef struct {
    FxU32 func;
} __GlideDepthBufferFunction;

typedef struct {
    FxU32 mode;
} __GlideDepthBufferMode;

typedef struct {
    FxU32 mask;
} __GlideDepthMask;

typedef struct {
    FxU32 mag;
    FxU32 min;
} __GlideTexFilterMode;

typedef struct {
    FxU32 s;
    FxU32 t;
} __GlideTexClampMode;

typedef struct {
    FxU32 mode;
} __GlideTexMipMapMode;

typedef struct {
    FxU32 function;
    FxU32 factor;
} __GlideTexCombine;

typedef struct {
    FxU32 function;
    FxU32 factor;
    FxU32 local;
    FxU32 other;
} __GlideColorCombine;

typedef struct {
    FxU32 function;
    FxU32 factor;
    FxU32 local;
    FxU32 other;
} __GlideAlphaCombine;

typedef struct {
    FxU32 buf;
} __GlideRenderBuffer;

typedef struct {
    FxU32 mode;
} __GlideCullMode;

typedef struct {
    FxU32 func;
} __GlideAlphaTestFunction;

typedef struct {
    FxU32 value;
} __GlideAlphaTestReferenceValue;

typedef struct {
    FxU32 level;
} __GlideDepthBiasLevel;

typedef struct {
    FxBool rgb;
    FxBool a;
} __GlideColorMask;

typedef struct {
    FxU32 mode;
} __GlideDitherMode;

typedef struct {
    GrColor_t value;
} __GlideFogColor;

typedef struct {
    GLint mode;
    float density, start, end;
} __GlideFogParms;

/* 
 * This struct and enum are for the hw validation mechanism.
 * Ideally, the list and the struct should be sorted such that
 * commonly altered hw modes are towards the top of the list.
 * If you change one, you must change the other.
 */
enum {
    ALPHA_BLEND_FUNCTION,
    DEPTH_BUFFER_FUNCTION,
    DEPTH_BUFFER_MODE,
    DEPTH_MASK,
    TEX_FILTER_MODE0,
    TEX_CLAMP_MODE0,
    TEX_MIPMAP_MODE0,
    TEX_COMBINE0,
    TEX_FILTER_MODE1,
    TEX_CLAMP_MODE1,
    TEX_MIPMAP_MODE1,
    TEX_COMBINE1,
    COLOR_COMBINE,
    ALPHA_COMBINE,
    RENDER_BUFFER,
    CULL_MODE,
    ALPHA_TEST_FUNCTION,
    ALPHA_TEST_REFERENCE_VALUE,
    DEPTH_BIAS_LEVEL,
    COLOR_MASK,
    DITHER_MODE,
    FOG_COLOR,
    FOG_PARMS
};
typedef struct {
    __GlideAlphaBlendFunction  *alphaBlendFunction;
    __GlideDepthBufferFunction *depthBufferFunction;
    __GlideDepthBufferMode     *depthBufferMode;
    __GlideDepthMask           *depthMask;
    __GlideTexFilterMode       *texFilterMode0;
    __GlideTexClampMode        *texClampMode0;
    __GlideTexMipMapMode       *texMipMapMode0;
    __GlideTexCombine          *texCombine0;
    __GlideTexFilterMode       *texFilterMode1;
    __GlideTexClampMode        *texClampMode1;
    __GlideTexMipMapMode       *texMipMapMode1;
    __GlideTexCombine          *texCombine1;
    __GlideColorCombine        *colorCombine;
    __GlideAlphaCombine        *alphaCombine;
    __GlideRenderBuffer        *renderBuffer;
    __GlideCullMode            *cullMode;
    __GlideAlphaTestFunction   *alphaTestFunction;
    __GlideAlphaTestReferenceValue *alphaTestReferenceValue;
    __GlideDepthBiasLevel      *depthBiasLevel;
    __GlideColorMask           *colorMask;
    __GlideDitherMode          *ditherMode;
    __GlideFogColor            *fogColor;
    __GlideFogParms            *fogParms;
    FxU32                       dirty;
} __GLglideShadow;

/* This struct holds glide versions of opengl state and other assorted data */
typedef struct {
    GrContext_t context;
    GrAlphaBlendFnc_t blendSrc;
    GrAlphaBlendFnc_t blendDst;
} __GLglideInternalState;

/*
 * Hardware enumerations
 */
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

/* This struct holds information on the hw configuration */
typedef struct {
    FxU32 numTMUs;
    FxU32 platform;
    FxU32 resolution;
    FxU32 sli;
    FxU32 mem;
} __GLglideHWConfig;

typedef struct {
    __GLglideShadow shadow;
    __GLglideHWConfig config;
    __GLglideInternalState state;
} __GLglideState;

/* 
 * This macro sets a piece of glide state in the shadow and sets a dirty
 * bit for validation if the new value is different from the current 
 * shadowed state.
 */
#define __GL_GLIDE_SET( function, param, value ) {                  \
    __GLSSTcontext *hwcx = (__GLSSTcontext *) gc;                   \
    if ( hwcx->glide.shadow. ## function ## -> ## param != value ) {\
        hwcx->glide.shadow. ## function ## -> ## param = value;     \
        hwcx->glide.shadow.dirty |=                                 \
          ( 0x1 << ( (int *)(&hwcx->glide.shadow. ## function ) -   \
                     (int *)(&hwcx->glide.shadow ) ) );             \
    }                                                               \
}

/*
 * This macro just sets the dirty bit for a function so that it will be
 * validated next time hw validation occurs.
 */
#define __GL_GLIDE_SET_DIRTY( function ) {                          \
    __GLSSTcontext *hwcx = (__GLSSTcontext *) gc;                   \
    hwcx->glide.shadow.dirty |=                                     \
      ( 0x1 << ( (int *)(&hwcx->glide.shadow. ## function ) -       \
                 (int *)(&hwcx->glide.shadow ) ) );                 \
}

#define __GL_UINTCAST( X ) (*(unsigned int*)&(X))
#define __GL_SIGNMASK( X ) (((int) (__GL_UINTCAST(X))) >> 31)
#define __GL_CLAMP_255(X) {                                               \
        unsigned mask = ~((int)(__GL_UINTCAST(X)-1)>>31);                 \
        __GL_UINTCAST(X) += (unsigned) (0x04000000 - 0x437f0000);         \
        __GL_UINTCAST(X) &= __GL_SIGNMASK(X);                             \
        __GL_UINTCAST(X) += (unsigned) 0x437f0000;                        \
        __GL_UINTCAST(X) &= mask;                                         \
        }

#endif

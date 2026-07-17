#ifndef __gl_texture_h_
#define __gl_texture_h_

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
** $Revision: 4$
** $Date: 10/11/00 7:52:32 PM$
*/
#include "types.h"
#include "ctable.h"

/*
** Client state set with glTexGen
*/
typedef struct __GLtextureCoordStateRec {
    /* How coordinates are being generated */
    GLenum mode;

    /* eye plane equation (used iff mode == GL_EYE_LINEAR) */
    __GLcoord eyePlaneEquation;

    /* object plane equation (used iff mode == GL_OBJECT_LINEAR) */
    __GLcoord objectPlaneEquation;
} __GLtextureCoordState;

/*
** Client state set with glTexEnv
*/
typedef struct __GLtextureEnvStateRec {
    /* environment "blend" function */
    GLenum mode;

    /* environment color */
    __GLcolor color;
} __GLtextureEnvState;

/*
** Client state set with glTexParameter
*/
typedef struct __GLtextureParamStateRec {
    /* wrap modes */
    GLenum sWrapMode;
    GLenum tWrapMode;

    /* min and mag filter */
    GLenum minFilter;
    GLenum magFilter;

    /* border color */
    __GLcolor borderColor;      /* Unscaled! */

} __GLtextureParamState;

/*
** Stackable texture object state.
*/
typedef struct __GLtextureObjectStateRec {
    GLuint name;        /* name of the texture */
    GLfloat priority;   /* priority of the texture object */
} __GLtextureObjectState;

/*
** Client state per texture map per dimension.
*/
typedef struct __GLperTextureStateRec {
    /*
    ** Texture parameter state (set with glTexParameter).
    */
    __GLtextureParamState params;

    /*
    ** Texture object bindings and priorities.
    */
    __GLtextureObjectState texobjs;
} __GLperTextureState;

/*
** Stackable client texture state. This does not include
** the mipmaps, or level dependent state.  Only state which is
** stackable via glPushAttrib/glPopAttrib is here.  The rest of the
** state is in the machine structure below.
*/
typedef struct __GLtextureStateRec {
    /* Per coordinate texture state (set with glTexGen) */
    __GLtextureCoordState s;
    __GLtextureCoordState t;
    __GLtextureCoordState r;
    __GLtextureCoordState q;

    /* Per texture state */
    __GLperTextureState *texture;

    /* Per texture environment state */
    __GLtextureEnvState *env;

    /* Scale and bias used before applying texture color table */
    __GLfloat scale[4];
    __GLfloat bias[4];
} __GLtextureState;

/************************************************************************/

/*
** Lookup tables used to compute texture environment functions.
*/
typedef struct __GLtextureEnvTableRec {
    GLint refcount;
    GLenum mode;
    GLenum internalFormat;
    GLvoid *red, *green, *blue, *alpha;
} __GLtextureEnvTable;

/************************************************************************/

typedef GLubyte __GLtextureBuffer;

typedef struct __GLtexelRec {
    __GLfloat r, g, b;
    __GLfloat luminance;
    __GLfloat alpha;
    __GLfloat intensity;
} __GLtexel;

/************************************************************************/

typedef struct __GLmipMapLevelRec __GLmipMapLevel;
typedef struct __GLtextureRec __GLtexture;

struct __GLmipMapLevelRec {
    __GLtextureBuffer *buffer;

    /* Alternate representation in pixel format */
    GLvoid *pixelBuffer;

    /* Image dimensions, including border */
    GLint width, height, depth, imageSize;

    /* Image dimensions, doesn't include border */
    GLint width2, height2, depth2;
    __GLfloat width2f, height2f, depth2f;

    /* log2 of width2 & height2 */
    GLint widthLog2, heightLog2, depthLog2;

    /* Border size */
    GLint border;

    /* Requested internal format */
    GLenum requestedFormat;

    /* Base internal format */
    GLenum baseFormat;

    /* Actual internal format */
    GLenum internalFormat;

    /* Component resolution */
    GLint redSize;
    GLint greenSize;
    GLint blueSize;
    GLint alphaSize;
    GLint luminanceSize;
    GLint intensitySize;
    GLint texelSize;

    /* Extract function for this mipmap level */
    void (*extract)(__GLmipMapLevel *level, __GLtexture *tex,
                    GLint img, GLint row, GLint col, __GLtexel *result);
};

#ifdef __GL_SST

struct __GLSSTtexCacheNodeRec {
    __GLtexture *tex;
    GLuint addr;
    GLint len;
    struct __GLSSTtexCacheNodeRec *next;
};
typedef struct __GLSSTtexCacheNodeRec __GLSSTtexCacheNode;

typedef struct __GLSSTperTexUnitRec {
    __GLSSTtexCacheNode *cache; 
    GLint allocSize;
} __GLSSTperTexUnit;

struct __GLSSTperTextureMachineRec {
    GLint numLevels;
    GLboolean resident[20]; /* XXXshui after we move into dd code, use #define constant */
    GLint allocWidth[20];
    GLint allocHeight[20];
    GLuint needMask;
    GLuint cacheMask;
    GLuint allocation; /* should be enum: NONE, BASE, or STACK */

    __GLSSTperTexUnit texUnit[__GL_MAX_TEX_UNITS];

    GLfloat sScale, tScale;
    GLint bufferSize;
    GLint lod; /* XXXshui */
    GLint aspect;
    GLint min; /* glide token for min filter */
    GLint mag; /* glide token for mag filter */
    GLint mip; /* glide token for mipmap mode */
    GLint grformat;
    GLint clamps;
    GLint clampt;

};
typedef struct __GLSSTperTextureMachineRec __GLSSTperTextureMachine;
#endif

struct __GLtextureRec {
    /* Back pointer to context */
    __GLcontext *gc;

    /* Copy of parameter state */
    __GLtextureParamState params;

    /* Copy of texure object stackable state */
    __GLtextureObjectState texobjs;

    /* Level information */
    __GLmipMapLevel *level;

#ifdef __GL_SST
    /* XXXshui temporary */
    __GLSSTperTextureMachine sst;
#endif
    /* Dimension of this texture (1 or 2) */
    GLint dim;

    /* maximum( log2(level[0].width2), log2(level[0].height2) ) */
    GLint p;

    /* Min/Mag switchover point */
    __GLfloat c;

    /* Color table for EXT_paletted_texture */
    __GLcolorTable CT;

    /* Format of texel after palette (if any) has been applied) */
    GLenum texelFormat;

    /* Create a new mipmap level for this texture */
    __GLtextureBuffer * (*createLevel)(__GLcontext *gc, __GLtexture *tex,
                                       GLint lod, GLint components,
                                       GLsizei w, GLsizei h, GLsizei d,
                                       GLint border, GLint dim);

    /* Copy data to / Read data from texture memory */
    void (*copyTexImage)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         __GLtexture *tex, GLint lod);
    void (*readTexImage)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         __GLtexture *tex, GLint lod);

    /* Texturing function for this texture */
    void (*textureFunc)(__GLcontext *gc, __GLcolor *color,
                        __GLfloat s, __GLfloat t, __GLfloat rho);

    /* Apply current environment function to fragment */
    void (*env)(__GLcontext *gc, __GLcolor *color, __GLtexel *texel);

    /* Magnification routine for this texture */
    void (*magnify)(__GLtexture *tex, __GLfloat lod,
                    __GLfloat s, __GLfloat t, __GLtexel *result);

    /* Minification routine for this texture */
    void (*minnify)(__GLtexture *tex, __GLfloat lod,
                    __GLfloat s, __GLfloat t, __GLtexel *result);

    /* Linear filter for this texture */
    void (*linear)(__GLtexture *tex, __GLmipMapLevel *lp,
                   __GLfloat s, __GLfloat t, __GLtexel *result);

    /* Nearest filter for this texture */
    void (*nearest)(__GLtexture *tex, __GLmipMapLevel *lp,
                    __GLfloat s, __GLfloat t, __GLtexel *result);

    /* platform specific handle */
    void *other;
};

typedef struct __GLperTextureMachineRec {
    __GLtexture map;
} __GLperTextureMachine;


/* needed up here for texobj references */

typedef struct __GLtextureObjectRec __GLtextureObject;

/*
** Implementation-specific function called to free texture object
*/
typedef
void (*__GLTexobjFreeFn)(__GLcontext *gc, __GLtextureObject *texobj);

/*
** Texture object structure.
** refcount field MUST be first in the structure.
*/
struct __GLtextureObjectRec {
    GLint refcount;     /* reference count: create with 1; delete when 0 */
                        /* refcount MUST be first in this structure */
    __GLTexobjFreeFn free;      /* function called to free tex object */
    GLenum targetIndex; /* index of the target it's bound to */
    GLboolean resident; /* residence status of the texture object */
    __GLperTextureMachine texture;      /* actual texture data */
};

#ifdef __GL_SST
typedef struct __GLSSTtextureMachineRec {
    __GLSSTtexCacheNode *first[__GL_MAX_TEX_UNITS];
    __GLSSTtexCacheNode *mru[__GL_MAX_TEX_UNITS];
    GLuint endAddr[__GL_MAX_TEX_UNITS];
    GLuint currentTMU;
    GLuint texUnits[__GL_MAX_TEX_UNITS]; /* lut for GR_TMUx arguments */
#if __GL_SST_GLIDE_VTX
    __GLfloat w[__GL_MAX_TEX_UNITS]; /* size of current texture */
    __GLfloat h[__GL_MAX_TEX_UNITS]; /* size of current texture */
#endif
} __GLSSTtextureMachine;
#endif

typedef struct __GLtextureMachineRec {
    __GLperTextureMachine **(texture[__GL_MAX_TEX_UNITS]);

    /* Array of ptrs to the currently bound texture objects. */
    __GLtextureObject **(boundTextures[__GL_MAX_TEX_UNITS]);

    /* Array of dummy texture objects for the default textures */
    __GLtextureObject *defaultTextures[__GL_MAX_TEX_UNITS];

    /* Current enabled texture */
    __GLtexture *(currentTexture[__GL_MAX_TEX_UNITS]);

    /* Lookup table for computing texture environment functions */
    __GLtextureEnvTable *envTable;

    /* The OR of all texture enable bits */
    GLboolean textureEnabled;

    /* Texture coords will be scaled before interpolation */
    GLboolean interpUVScaled[__GL_MAX_TEX_UNITS];

    /* CreateLevel and CreateProxyLevel  pointers */
    __GLtextureBuffer *(*createLevel)(__GLcontext *gc, __GLtexture *tex,
                                      GLint lod, GLint components,
                                      GLsizei w, GLsizei h, GLsizei d,
                                      GLint border, GLint dim);
    __GLtextureBuffer *(*createProxyLevel)(__GLcontext *gc, __GLtexture *tex,
                                           GLint lod, GLint components,
                                           GLsizei w, GLsizei h, GLsizei d,
                                           GLint border, GLint dim);

    /* pointer to the routinet to initialize texture objects */
    void (*initTextureObject)(__GLcontext *gc, __GLtextureObject *texobj,
                              GLuint name, GLuint targetIndex);

    /* Stores pointers to texture objects, retrieved by name */
    __GLnamesArray *namesArray;

    /* Cache of lookup tables used to compute environment functions */
    __GLtextureEnvTable *envTableCache;

    /* GLubyte version of texture environment color */
    GLubyte envColor[__GL_MAX_TEX_UNITS][4];

#ifdef __GL_SST
    __GLSSTtextureMachine sst;
#endif
    GLuint currentTexUnit;

    GLuint hwMinMag[__GL_MAX_TEX_UNITS];
    GLuint hwSTWrap[__GL_MAX_TEX_UNITS];
    GLuint hwMMMode[__GL_MAX_TEX_UNITS];
} __GLtextureMachine;

/************************************************************************/

/* Check for texture consistency before enabling texturing */
extern GLboolean __glIsTextureConsistent(__GLcontext *gc, __GLtexture *tex);

/* Fragment texturing routines */
extern void __glTextureFragmentUVScale(__GLcontext *gc, __GLcolor *color,
                                __GLfloat s, __GLfloat t,
                                __GLfloat rho);
extern void __glFastTextureFragment(__GLcontext *gc, __GLcolor *color,
                                    __GLfloat s, __GLfloat t,
                                    __GLfloat rho);
extern void __glTextureFragment(__GLcontext *gc, __GLcolor *color,
                                __GLfloat s, __GLfloat t,
                                __GLfloat rho);
extern void __glMipMapFragment(__GLcontext *gc, __GLcolor *color,
                               __GLfloat s, __GLfloat t,
                               __GLfloat rho);

/* Texturing routines */
extern void __glLinearFilterUVScaled(__GLtexture *tex, __GLfloat lod,
                             __GLfloat s, __GLfloat t,
                             __GLtexel *result);
extern void __glNearestFilterUVScaled(__GLtexture *tex, __GLfloat lod,
                              __GLfloat s, __GLfloat t,
                              __GLtexel *result);
extern void __glLinearFilter(__GLtexture *tex, __GLfloat lod,
                             __GLfloat s, __GLfloat t,
                             __GLtexel *result);
extern void __glNearestFilter(__GLtexture *tex, __GLfloat lod,
                              __GLfloat s, __GLfloat t,
                              __GLtexel *result);
extern void __glNMNFilter(__GLtexture *tex, __GLfloat lod,
                          __GLfloat s, __GLfloat t,
                          __GLtexel *result);
extern void __glLMNFilter(__GLtexture *tex, __GLfloat lod,
                          __GLfloat s, __GLfloat t,
                          __GLtexel *result);
extern void __glNMLFilter(__GLtexture *tex, __GLfloat lod,
                          __GLfloat s, __GLfloat t,
                          __GLtexel *result);
extern void __glLMLFilter(__GLtexture *tex, __GLfloat lod,
                          __GLfloat s, __GLfloat t,
                          __GLtexel *result);

/* Filter routines */
extern void __glLinearFilter1(__GLtexture *tex, __GLmipMapLevel *lp,
                              __GLfloat s, __GLfloat t,
                              __GLtexel *result);
extern void __glLinearFilter2(__GLtexture *tex, __GLmipMapLevel *lp,
                              __GLfloat s, __GLfloat t,
                              __GLtexel *result);
extern void __glNearestFilter1(__GLtexture *tex, __GLmipMapLevel *lp,
                               __GLfloat s, __GLfloat t,
                               __GLtexel *result);
extern void __glNearestFilter2(__GLtexture *tex, __GLmipMapLevel *lp,
                               __GLfloat s, __GLfloat t,
                               __GLtexel *result);


/* Texture generation procs */
extern void __glCalcEyeLinear(__GLcontext *gc, __GLvertex *v);
extern void __glCalcObjectLinear(__GLcontext *gc, __GLvertex *v);
extern void __glCalcSphereMap(__GLcontext *gc, __GLvertex *v);
extern void __glCalcMixedTexture(__GLcontext *gc, __GLvertex *v);
extern void __glCalcTexture(__GLcontext *gc, __GLvertex *v);
extern void __glCalcTexturePersp(__GLcontext *gc, __GLvertex *v);
extern void __glCalcTextureUVScale(__GLcontext *gc, __GLvertex *v);
extern void __glCalcTexturePerspUVScale(__GLcontext *gc, __GLvertex *v);
extern void __glFastCalcTexturePersp(__GLcontext *gc, __GLvertex *v);
extern void __glFastCalcTextureUVScale(__GLcontext *gc, __GLvertex *v);
extern void __glFastCalcTexturePerspUVScale(__GLcontext *gc, __GLvertex *v);

/* Texture environment functions */
extern void __glTextureModulateL(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureModulateLA(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureModulateRGB(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureModulateRGBA(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureModulateA(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureModulateI(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureModulateCI(__GLcontext *gc, __GLcolor *color,
                                  __GLtexel *tx);

extern void __glTextureDecalRGB(__GLcontext *gc, __GLcolor *color,
                              __GLtexel *tx);
extern void __glTextureDecalRGBA(__GLcontext *gc, __GLcolor *color,
                              __GLtexel *tx);

extern void __glTextureBlendL(__GLcontext *gc, __GLcolor *color,
                              __GLtexel *tx);
extern void __glTextureBlendLA(__GLcontext *gc, __GLcolor *color,
                              __GLtexel *tx);
extern void __glTextureBlendRGB(__GLcontext *gc, __GLcolor *color,
                              __GLtexel *tx);
extern void __glTextureBlendRGBA(__GLcontext *gc, __GLcolor *color,
                              __GLtexel *tx);
extern void __glTextureBlendA(__GLcontext *gc, __GLcolor *color,
                              __GLtexel *tx);
extern void __glTextureBlendI(__GLcontext *gc, __GLcolor *color,
                              __GLtexel *tx);

extern void __glTextureReplaceL(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureReplaceLA(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureReplaceRGB(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureReplaceRGBA(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureReplaceA(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureReplaceI(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *tx);
extern void __glTextureReplaceCI(__GLcontext *gc, __GLcolor *color,
                                 __GLtexel *texel);
extern void __glTextureAddCI(__GLcontext *gc, __GLcolor *color,
                             __GLtexel *texel);

/* Extract a texel from a texture level (no border) */
extern void __glExtractTexelL(__GLmipMapLevel *level, __GLtexture *tex,
                              GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelLA(__GLmipMapLevel *level, __GLtexture *tex,
                              GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB(__GLmipMapLevel *level, __GLtexture *tex,
                              GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA(__GLmipMapLevel *level, __GLtexture *tex,
                              GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelA(__GLmipMapLevel *level, __GLtexture *tex,
                              GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelI(__GLmipMapLevel *level, __GLtexture *tex,
                              GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB8(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA8(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelCI8(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelCI16(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);

/* Extract a texel from a texture level (the texture has a border) */
extern void __glExtractTexelL_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelLA_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelA_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelI_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB8_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA8_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelCI8_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelCI16_B(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint img, GLint row, GLint col, __GLtexel *res);

/* For getting texture images */
extern void __glInitTexSourceUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                    GLsizei width, GLsizei height, GLsizei depth,
                                    GLenum format, GLenum type, const GLvoid *buf,
                                    GLboolean packed);
extern void __glInitTexDestPack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                GLsizei width, GLsizei height, GLsizei depth,
                                GLenum format, GLenum type,
                                const GLvoid *buf);
extern void __glInitTexImageGet(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                __GLtexture *tex, GLint lod);

extern GLvoid __glCopyTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                               __GLtexture *tex, GLint lod);
extern GLvoid __glReadTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                               __GLtexture *tex, GLint lod);

/* List execution texture image code */
extern void __gllei_TexImage1D(__GLcontext *gc, GLenum target, GLint lod,
                               GLint components,
                               GLsizei length, GLint border,
                               GLenum format, GLenum type,
                               const GLubyte *image);
extern void __gllei_TexImage2D(__GLcontext *gc, GLenum target, GLint lod,
                               GLint components,
                               GLsizei w, GLsizei h, GLint border,
                               GLenum format, GLenum type,
                               const GLubyte *image);


extern void __gllei_TexSubImage1D(__GLcontext *gc, GLenum target, GLint lod,
                                     GLint xoffset, GLsizei length,
                                     GLenum format, GLenum type,
                                     const GLubyte *image);
extern void __gllei_TexSubImage2D(__GLcontext *gc, GLenum target, GLint lod, 
                                     GLint xoffset, GLint yoffset,
                                     GLsizei w, GLsizei h,
                                     GLenum format, GLenum type,
                                     const GLubyte *image);

/* Rho calculation routines */
extern __GLfloat __glComputeLineRho(__GLcontext *gc, 
                                    __GLfloat s, __GLfloat t,
                                    __GLfloat wInv);
extern __GLfloat __glNopLineRho(__GLcontext *gc, 
                                __GLfloat s, __GLfloat t,
                                __GLfloat wInv);
extern __GLfloat __glComputePolygonRho(__GLcontext *gc, const __GLshade *sh,
                                       __GLfloat s, __GLfloat t,
                                       __GLfloat winv);
extern __GLfloat __glNopPolygonRho(__GLcontext *gc, const __GLshade *sh,
                                   __GLfloat s, __GLfloat t,
                                   __GLfloat winv);

extern __GLtexture *__glCheckTexImage1DArgs(__GLcontext *gc, GLenum target,
                                            GLint lod, GLint components,
                                            GLsizei length, GLint border,
                                            GLenum format, GLenum type);

extern __GLtexture *__glCheckTexImage2DArgs(__GLcontext *gc, GLenum target,
                                            GLint lod, GLint components,
                                            GLsizei w, GLsizei h, GLint border,
                                            GLenum format, GLenum type);

extern __GLtexture *__glCheckTexSubImage1DArgs(__GLcontext *gc, GLenum target,
                                               GLint lod,
                                               GLint xoffset, GLint length,
                                               GLenum format, GLenum type);

extern __GLtexture *__glCheckTexSubImage2DArgs(__GLcontext *gc, GLenum target,
                                               GLint lod,
                                               GLint xoffset, GLint yoffset,
                                               GLsizei w, GLsizei h,
                                               GLenum format, GLenum type);

extern __GLtexture *__glCheckCopyTexImageArgs(__GLcontext *gc, GLenum target, 
                                              GLint level,
                                              GLenum internalformat,
                                              GLint x, GLint y, 
                                              GLsizei width, GLsizei height,
                                              GLint border, GLint dim);

extern __GLtexture *__glCheckCopyTexSubImageArgs(__GLcontext *gc, GLenum target,
                                                 GLint lod, 
                                                 GLint xoffset, GLint yoffset, 
                                                 GLint zoffset,
                                                 GLint x, GLint y,
                                                 GLsizei w, GLsizei h, GLint dim);

extern void __glInitTexImageStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                  __GLtexture *tex, GLint lod);

extern void __glInitTexSubImageStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                                     __GLtexture *tex, GLint lod,
                                     GLint xoffset, GLint yoffset, GLint zoffset);

/* Texture Lookup */
extern __GLtextureObjectState *__glLookUpTextureTexobjs(__GLcontext *gc,
                                                        GLenum target,
                                                        int texUnit );
extern __GLtextureParamState *__glLookUpTextureParams(__GLcontext *gc,
                                                      GLenum target,
                                                      int texUnit );
extern __GLtexture *__glLookUpTexture(__GLcontext *gc, GLenum target, int texUnit );

extern __GLtextureObject *__glLookUpTextureObject(__GLcontext *gc,
                                                  GLenum target,
                                                  int texUnit );

/* Texture Initialization */
extern void __glEarlyInitTextureState(__GLcontext *gc);
/* Used to share texture objects. */
extern void __glShareTextureObjects(__GLcontext *gc, __GLcontext *shareMe);

/* Texture Object management */
extern GLvoid __glFreeTexObj(__GLcontext *gc, __GLtextureObject *texobj);
extern GLvoid __glInitTextureObject(__GLcontext *gc, __GLtextureObject *texobj,
                                    GLuint name, GLuint targetIndex);
extern __GLtextureBuffer *
        __glTexCreateProxyLevel(__GLcontext *gc, __GLtexture *tex,
                                GLint lod, GLint components,
                                GLsizei w, GLsizei h, GLsizei d,
                                GLint border, GLint dim);
extern __GLtextureBuffer *
        __glTexCreateLevel(__GLcontext *gc, __GLtexture *tex,
                           GLint lod, GLint components,
                           GLsizei w, GLsizei h, GLsizei d,
                           GLint border, GLint dim);

/* Bind Texture used by pop or entry point. */
extern void __glBindTexture(__GLcontext *gc, GLuint targetIndex, GLuint name);

/* Lookup tables used to compute texture environment functions */
__GLtextureEnvTable *__glCreateTextureEnvTable(GLenum mode, __GLtexture *tex);
void __glInitTextureEnvCache(__GLcontext *gc);
void __glFreeTextureEnvCache(__GLcontext *gc);

/* Create specially formatted texture to match framebuffer format */
void __glCookTexture(__GLcontext *gc);
void __glCookSubTexture(__GLcontext *gc, __GLmipMapLevel *lp,
                        int x, int y, int w, int h);

#endif /* __gl_texture_h_ */

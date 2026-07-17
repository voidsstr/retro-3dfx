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
** $Revision: 2$
** $Date: 10/11/00 8:05:21 PM$
*/
#include "types.h"
#include "ctable.h"
#include "texfmt.h"

#define __GL_NUMBER_OF_TEXTURES     4
#define __GL_TEXTURE_INDEX_1D 0
#define __GL_TEXTURE_INDEX_2D 1
#define __GL_PROXY_TEXTURE_INDEX_1D 2
#define __GL_PROXY_TEXTURE_INDEX_2D 3



#define __GL_NUMBER_OF_TEXTURE_ENVS 1


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
    __GLcolor borderColor;	/* Unscaled! */

} __GLtextureParamState;

/*
** Stackable texture object state.
*/
typedef struct __GLtextureObjectStateRec {
    GLuint name;	/* name of the texture */
    GLfloat priority;	/* priority of the texture object */
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
    __GLperTextureState texture[__GL_NUMBER_OF_TEXTURES];

    /* Per texture environment state */
    __GLtextureEnvState *env;

    /* Scale and bias used before applying texture color table */
    __GLfloat scale[4];
    __GLfloat bias[4];
} __GLtextureState;

/************************************************************************/

typedef GLubyte __GLtextureBuffer;

typedef struct __GLtexelRec {
    GLubyte r, g, b, a;
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

    __GLtextureFormat *texFormat;
    /* Component resolution */
    GLint redSize;
    GLint greenSize;
    GLint blueSize;
    GLint alphaSize;
    GLint luminanceSize;
    GLint intensitySize;

    /* Extract function for this mipmap level */
    void (*extract)(__GLmipMapLevel *level, __GLtexture *tex,
		    GLint img, GLint row, GLint col, __GLtexel *result);
};

/*
** Texture object structure.
** refcount field MUST be first in the structure.
*/
struct __GLtextureRec {
    /* reference count: create with 1; delete when 0 */
    GLint  refcount;

    /* Back pointer to context */
    __GLcontext *gc;

    /* Copy of parameter state */
    __GLtextureParamState params;

    /* Copy of texure object stackable state */
    __GLtextureObjectState texobjs;

    /* Level information */
    GLint numLevels;
    __GLmipMapLevel **level;

    /* 
     * This should only be called by the texture object when it is 
     * deleted. It calls the texture managers delete textureobj func
     */
    void (*free)(__GLcontext *gc, __GLtexture *tex);
	       
    /* 
     * Allocates the space for the texture object. Returns residence 
     * status 
     */
    GLenum (*makeResident)(__GLcontext *gc, __GLtexture *tex, float priority);
      
    /* 
     * Checks the residence status for the texture object. Returns 
     * residence status 
     */
    GLenum (*isResident)(__GLcontext *gc, __GLtexture *tex);
      
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

    /* Release storage for a mipmap level */
    void (*releaseLevel)(__GLcontext *gc, __GLtexture *tex, GLint lod);

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

    __GLtextureFormat *format;

    GLuint  name;       /* name of the texture, alloced by the tm */
    GLuint  targetIndex;
    GLfloat priority;   /* priority of the texture object. Initially 0.5, 
                           or as speced by texture priority */
    GLenum residence;
};


typedef struct __GLtextureMachineRec {
    struct __GLtextureManagerRec *textureManager;

    __GLtexture *texture[__GL_NUMBER_OF_TEXTURES];

    /* Array of dummy texture objects for the default textures */
    __GLtexture *defaultTextures[__GL_NUMBER_OF_TEXTURES];

    /* Current enabled texture */
    __GLtexture *currentTexture;

    /* The OR of all texture enable bits */
    GLboolean textureEnabled;

    /* Stores pointers to texture objects, retrieved by name */
    __GLnamesArray *namesArray;

    /* GLubyte version of texture environment color */
    GLubyte envColor[4];

    /* Create a texture with the given name and target */
    __GLtexture *(*createTexture)(__GLcontext *gc, GLuint name,
				  GLuint targetIndex);
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
extern void __glExtractTexelL8(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelLA8(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB8(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA8(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelA8(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelI8(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelXRGB1555(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB332(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB565(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA4(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelARGB4(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA5551(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelARGB1555(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelCI8(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelCI16(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);

/* Extract a texel from a texture level (the texture has a border) */
extern void __glExtractTexelL8_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelLA8_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB8_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA8_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelA8_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelI8_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelXRGB1555_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB332_B(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGB565_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA4_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelARGB4_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelRGBA5551_B(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelARGB1555_B(__GLmipMapLevel *level, __GLtexture *tex,
			      GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelCI8_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);
extern void __glExtractTexelCI16_B(__GLmipMapLevel *level, __GLtexture *tex,
			       GLint img, GLint row, GLint col, __GLtexel *res);

/* For getting texture images */
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


/* Texture Lookup */
extern __GLtextureObjectState *__glLookUpTextureTexobjs(__GLcontext *gc,
						        GLenum target);
/* Texture Lookup */
extern __GLtextureParamState *__glLookUpTextureParams(__GLcontext *gc,
						      GLenum target);
extern __GLtexture *__glLookUpTexture(__GLcontext *gc, GLenum target);

/* Texture Initialization */
extern void __glEarlyInitTextureState(__GLcontext *gc);
/* Used to share texture objects. */
extern void __glShareTextureObjects(__GLcontext *gc, __GLcontext *shareMe);

/* Texture Object management */
extern GLvoid __glFreeTexObj(__GLtexture *texobj);

/* Bind Texture used by pop or entry point. */
extern void __glBindTexture(__GLcontext *gc, GLuint targetIndex, GLuint name);

/* Create specially formatted texture to match framebuffer format */
void __glCookTexture(__GLcontext *gc);
void __glCookSubTexture(__GLcontext *gc, __GLmipMapLevel *lp,
			int x, int y, int w, int h);

#endif /* __gl_texture_h_ */

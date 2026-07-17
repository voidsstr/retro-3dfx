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
#include "context.h"
#include "imports.h"
#include "global.h"
#include "g_imfncs.h"
#include "types.h"
#include "namesint.h"
#include "pixel.h"
#include "image.h"
#include "glmath.h"
#include "texfmt.h"
#include "texmgr.h"
#include "texture.h"
#include "gldevice.h"
#include <memory.h>


/* The texture manager */
static __GLtextureManager __glTextureManager =
{
    0,
    __glReleaseTextureManager,
    __glInvalidateTextureManager,
};


void __glInitTextureManager(__GLcontext *gc)
{
    __GLtextureManager *texmgr = &__glTextureManager;
    texmgr->refcount++;
    gc->texture.textureManager = texmgr;

    gc->texture.createTexture = __glCreateTexture;
}

void __glReleaseTextureManager(__GLcontext *gc)
{
    __GLtextureManager *texmgr = gc->texture.textureManager;
    texmgr->refcount--;
    if (0 == texmgr->refcount) {
	/* Delete allocated resources */
    }
    gc->texture.textureManager = NULL;
}

/* Notify the texture manager that texture priorities have changed */
void __glInvalidateTextureManager(__GLtextureManager *texmgr)
{
}

/*******************************************************************/
/*                   Texture object management                     */
/*******************************************************************/


__GLtexture *
__glCreateTexture(__GLcontext *gc, GLuint name, GLuint targetIndex)
{
    __GLtexture *tex;
    GLint level, maxMipMapLevel;
    __GLmipMapLevel *lp;

    tex = (__GLtexture *) (*gc->imports.calloc)(gc, 1, sizeof(__GLtexture));
    assert(NULL != tex);

    tex->gc			= gc;
    tex->refcount		= 1;
    tex->targetIndex		= targetIndex;
    tex->residence		= GL_FALSE;
    tex->free			= __glFreeTexture;
    tex->makeResident		= __glTextureMakeResident;
    tex->copyTexImage		= __glCopyTexImage;
    tex->readTexImage		= __glReadTexImage;
    tex->texobjs.name		= name;
    tex->texobjs.priority	= 1.0;
    tex->CT.format		= GL_RGBA;

    /*
    ** Can't copy the params currently in the gc state.texture params,
    ** because they might not be at init conditions.
    */
    tex->params.sWrapMode = GL_REPEAT;
    tex->params.tWrapMode = GL_REPEAT;
    tex->params.minFilter = GL_NEAREST_MIPMAP_LINEAR;
    tex->params.magFilter = GL_LINEAR;

    switch (targetIndex) {
      case __GL_TEXTURE_INDEX_1D:
	tex->dim = 1;
	tex->createLevel  = __glTextureCreateLevel;
	tex->CT.target = GL_TEXTURE_1D;
	break;
      case __GL_TEXTURE_INDEX_2D:
	tex->dim = 2;
	tex->createLevel  = __glTextureCreateLevel;
	tex->CT.target = GL_TEXTURE_2D;
	break;
      case __GL_PROXY_TEXTURE_INDEX_1D:
	tex->dim = 1;
	tex->createLevel  = __glTextureCreateProxyLevel;
	break;
      case __GL_PROXY_TEXTURE_INDEX_2D:
	tex->dim = 2;
	tex->createLevel  = __glTextureCreateProxyLevel;
	break;
      default:
	break;
    }
    tex->releaseLevel = __glTextureDeleteLevel;

    maxMipMapLevel = gc->constants.maxMipMapLevel;
    tex->level = (__GLmipMapLevel**)
	(*gc->imports.calloc)(gc, (size_t) maxMipMapLevel,
			      sizeof(__GLmipMapLevel*));

    tex->level[0] = (__GLmipMapLevel*)
	(*gc->imports.calloc)(gc, (size_t) maxMipMapLevel,
			      sizeof(__GLmipMapLevel));
    /* Init each texture level */
    for (level = 0, lp = tex->level[0];
	 level < maxMipMapLevel; level++, lp++) {
	tex->level[level] = lp;
	tex->level[level]->requestedFormat = 1;
    }

    return (__GLtexture *)tex;
}


/*
 * Texture Object methods
 */
void __glFreeTexture(__GLcontext *gc, __GLtexture *tex)
{
    GLint level, maxLevel;

    assert(tex->refcount == 0);

    maxLevel = gc->constants.maxMipMapLevel;
    for (level = 0; level < maxLevel; level++) {
	(*tex->releaseLevel)(gc, tex, level);
    }
    (*gc->imports.free)(gc, tex->level[0]);
    (*gc->imports.free)(gc, tex);
}


/*ARGSUSED*/
GLvoid __glCopyTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			__GLtexture *tex, GLint lod)
{
    (*gc->procs.copyImage)(gc, spanInfo, GL_TRUE);
    tex->residence = __GL_TEXTURE_RESIDENCE_LOADED;
}


/*ARGSUSED*/
GLvoid __glReadTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			__GLtexture *tex, GLint lod)
{
    (*gc->procs.readImage)(gc, tex, spanInfo);
    tex->residence = __GL_TEXTURE_RESIDENCE_LOADED;
}


GLenum
__glTextureMakeResident(__GLcontext *gc, __GLtexture *tex, float priority)
{
    return 0;
}


GLenum 
__glTextureIsResident(__GLtexture *tex)
{
    return tex->residence;
}


__GLtextureBuffer *
__glTextureCreateLevel(__GLcontext *gc, __GLtexture *tex,
		       GLint lod, GLint components,
		       GLsizei w, GLsizei h, GLsizei d,
		       GLint border, GLint dim)
{
    __GLmipMapLevel   *lp = tex->level[lod];
    __GLtextureFormat *texFormat;
    GLenum baseFormat;
    GLint baseWidth  = (w-border*2) * (1 << lod);
    GLint baseHeight = (h-border*2) * (1 << lod);
    GLint bufferSize;
    GLint maxDim;

    if (baseWidth > baseHeight)
         maxDim = baseWidth;
    else maxDim = baseHeight;

    tex->numLevels = __glFloorLog2(maxDim)+1;

    texFormat = (__GLtextureFormat *)
	(*__glDevice->lookupTextureFormat)(components, &baseFormat);
    assert(NULL != texFormat);
    bufferSize = w * h * (texFormat->bitsPerTexel >> 3);

    if (baseWidth > gc->constants.maxTextureSize ||
	baseHeight > gc->constants.maxTextureSize) {
	/* Texture allocation failed */
	__glSetError(GL_INVALID_VALUE);
	return 0;
    } else if (bufferSize) {
	/* Texture allocation succeeded, fill in new level info */
	lp->buffer = (__GLtextureBuffer*)
	    (*gc->imports.realloc)(gc, lp->buffer, (size_t) bufferSize);
	
	if (lp->buffer == NULL) {
	    __glSetError(GL_OUT_OF_MEMORY);
	    return 0;
	}

	/* This is allocated lazily */
	if (lp->pixelBuffer != NULL) {
	    (*gc->imports.free)(gc, lp->pixelBuffer);
	    lp->pixelBuffer = NULL;
	}

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
	lp->width2f = lp->width2;
	lp->height2f = lp->height2;
	lp->depth2f = lp->depth2;
	lp->border = border;
	lp->texFormat = texFormat;
	lp->requestedFormat = (GLenum) components;
	lp->baseFormat = baseFormat;
	lp->internalFormat = texFormat->internalFormat;
	lp->redSize = texFormat->redSize;
	lp->greenSize = texFormat->greenSize;
	lp->blueSize = texFormat->blueSize;
	lp->alphaSize = texFormat->alphaSize;
	lp->luminanceSize = texFormat->luminanceSize;
	lp->intensitySize = texFormat->intensitySize;
	if (border) {
	    lp->extract = texFormat->extractTexelBorder;
	} else {
	    lp->extract = texFormat->extractTexel;
	}

	tex->format = texFormat;
    } else {
	/* The texture level is being freed */
	if (lp->buffer != NULL) {
	    (*gc->imports.free)(gc, lp->buffer);
	    lp->buffer = NULL;
	}
	if (lp->pixelBuffer != NULL) {
	    (*gc->imports.free)(gc, lp->pixelBuffer);
	    lp->pixelBuffer = NULL;
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
	lp->texFormat = 0;
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


__GLtextureBuffer *
__glTextureCreateProxyLevel(__GLcontext *gc, __GLtexture *tex,
			    GLint lod, GLint components,
			    GLsizei w, GLsizei h, GLsizei d,
			    GLint border, GLint dim)
{
    __GLmipMapLevel *lp = tex->level[lod];
    __GLtextureFormat *texFormat;
    GLenum baseFormat;
    GLint baseWidth= (w-border*2) * (1 << lod);
    GLint baseHeight= (h-border*2) * (1 << lod);

    texFormat = (__GLtextureFormat *)
	(*__glDevice->lookupTextureFormat)(components, &baseFormat);
    assert(NULL != texFormat);

    if (baseWidth > gc->constants.maxTextureSize ||
	baseHeight > gc->constants.maxTextureSize) {
	/* Proxy allocation failed */
	lp->width = 0;
	lp->height = 0;
	lp->depth = 0;
	lp->border = 0;
	lp->texFormat = 0;
	lp->requestedFormat = 0;
	lp->baseFormat = 0;
	lp->internalFormat = 0;
	lp->redSize = 0;
	lp->greenSize = 0;
	lp->blueSize = 0;
	lp->alphaSize = 0;
	lp->luminanceSize = 0;
	lp->intensitySize = 0;
    } else {
	/* Proxy allocation succeeded */
	lp->width = w;
	lp->height = h;
	lp->depth = d;
	lp->border = border;
	lp->texFormat = texFormat;
	lp->requestedFormat = (GLenum) components;
	lp->baseFormat = baseFormat;
	lp->internalFormat = texFormat->internalFormat;
	lp->redSize = texFormat->redSize;
	lp->greenSize = texFormat->greenSize;
	lp->blueSize = texFormat->blueSize;
	lp->alphaSize = texFormat->alphaSize;
	lp->luminanceSize = texFormat->luminanceSize;
	lp->intensitySize = texFormat->intensitySize;
    }
    lp->extract = (void *) __glNop;
    return 0;
}


void __glTextureDeleteLevel(__GLcontext *gc, __GLtexture *tex, GLint lod)
{
    __GLmipMapLevel *lp = tex->level[lod];

    if (lp->buffer) {
	(*gc->imports.free)(gc, lp->buffer);
	lp->buffer = NULL;
    }

    if (lp->pixelBuffer) {
	(*gc->imports.free)(gc, lp->pixelBuffer);
	lp->pixelBuffer = NULL;
    }
}

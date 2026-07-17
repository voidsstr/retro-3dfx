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
#include <memory.h>


#include "texmgr.h"
#include "gldevice.h"

/*
** Some math routines that are optimized in assembly
*/
#ifdef __GL_USE_MIPSASMCODE
#define	__GL_FRAC(f)	__glFrac(f)
#else
#define __GL_FRAC(f)	((f) - __GL_FLOORF(f))
#endif

/************************************************************************/


__GLtextureParamState * __glLookUpTextureParams(__GLcontext *gc, GLenum target)
{
    switch (target) {
      case GL_TEXTURE_1D:
	return &gc->state.texture.texture[__GL_TEXTURE_INDEX_1D].params;
      case GL_TEXTURE_2D:
	return &gc->state.texture.texture[__GL_TEXTURE_INDEX_2D].params;
      default:
	return 0;
    }
}

__GLtextureObjectState * __glLookUpTextureTexobjs(__GLcontext *gc, 
						    GLenum target)
{
    switch (target) {
      case GL_TEXTURE_1D:
	return &gc->state.texture.texture[__GL_TEXTURE_INDEX_1D].texobjs;
      case GL_TEXTURE_2D:
	return &gc->state.texture.texture[__GL_TEXTURE_INDEX_2D].texobjs;
      default:
	return 0;
    }
}

__GLtexture *__glLookUpTexture(__GLcontext *gc, GLenum target)
{
    switch (target) {
      case GL_TEXTURE_1D:
	return gc->texture.texture[__GL_TEXTURE_INDEX_1D];
      case GL_TEXTURE_2D:
	return gc->texture.texture[__GL_TEXTURE_INDEX_2D];
      case GL_PROXY_TEXTURE_1D:
	return gc->texture.texture[__GL_PROXY_TEXTURE_INDEX_1D];
      case GL_PROXY_TEXTURE_2D:
	return gc->texture.texture[__GL_PROXY_TEXTURE_INDEX_2D];
      default:
	return 0;
    }
}

static GLfloat Clampf(GLfloat fval, __GLfloat zero, __GLfloat one)
{
    if (fval < zero) return zero;
    else if (fval > one) return one;
    else return fval;
}

/************************************************************************/

/*
** Used to share texture objects between two different contexts.
*/
void __glShareTextureObjects(__GLcontext *dst, __GLcontext *src)
{
    /* First get rid of our private texture object state */
    dst->texture.namesArray->refcount--;
    if (dst->texture.namesArray->refcount == 0) {
	__glNamesFreeArray(dst, dst->texture.namesArray);
    }
    dst->texture.namesArray = NULL;

    dst->texture.namesArray = src->texture.namesArray;
    dst->texture.namesArray->refcount++;
}

void __glEarlyInitTextureState(__GLcontext *gc)
{
    GLint numTextures, numEnvs;
    GLint i,maxMipMapLevel;
    __GLtexture *texobj, **texobjP;

    /* Initialize the texture manager */
    (*__glDevice->devInitTextureManager)(gc);

    /* XXX Override device dependent values */
    gc->constants.numberOfTextures = 4;
    gc->constants.maxTextureSize = 1 << (gc->constants.maxMipMapLevel - 1);

    /* Allocate memory based on number of textures supported */
    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;

    gc->state.texture.env = (__GLtextureEnvState*)
	(*gc->imports.calloc)(gc, (size_t) numEnvs,
			      sizeof(__GLtextureEnvState));

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

    /*
    ** Set up the dummy texture objects for the default textures. 
    ** Because the default textures are not shared, they should
    ** not be hung off of the namesArray structure.
    */

    texobjP = gc->texture.defaultTextures;
    for (i=0; i < numTextures; i++, texobjP++) {
	texobj = *texobjP =
	    (*gc->texture.createTexture)(gc, 0/*name*/, i/*targetIndex*/);
	assert(texobj->texobjs.name == 0);
	/*
	** The refcount is unused because default textures aren't
	** shared.
	*/
	texobj->refcount = 1;
	/*
	** Install the default textures into the gc.
	*/
	gc->texture.texture[i]       = texobj;
    }
}


void __glInitTextureState(__GLcontext *gc)
{
    __GLperTextureState *pts;
    __GLtextureEnvState *tes;
    __GLtexture  **ptm;
    GLint i, level, maxMipMapLevel, numTextures, numEnvs;

    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;
    maxMipMapLevel = gc->constants.maxMipMapLevel;

    gc->state.current.texture.w = __glOne;

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
	(*ptm)->gc = gc;
	(*ptm)->params = pts->params;
	switch (i) {
	  case __GL_TEXTURE_INDEX_1D:
	    (*ptm)->dim = 1;
	    break;
	  case __GL_TEXTURE_INDEX_2D:
	    (*ptm)->dim = 2;
	    break;
	  case __GL_PROXY_TEXTURE_INDEX_1D:
	    (*ptm)->dim = 1;
	    break;
	  case __GL_PROXY_TEXTURE_INDEX_2D:
	    (*ptm)->dim = 2;
	    break;
	  default:
	    break;
	}
	/* Init each texture level */
	for (level = 0; level < maxMipMapLevel; level++) {
	    (*ptm)->level[level]->requestedFormat = 1;
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
}

void __glFreeTextureState(__GLcontext *gc)
{
    __GLtexture **ptm = gc->texture.texture;
    GLint i, numTextures;

    /*
    ** Clean up all allocs associated with texture objects.
    */

    numTextures = gc->constants.numberOfTextures;
    for (i = 0; i < numTextures; i++, ptm++) {
	/* Unbind all default textures. */
	__glBindTexture(gc, i, 0);
	(*ptm)->refcount--;
        (*(*ptm)->free)(gc, *ptm);
        gc->texture.defaultTextures[i] = NULL;
    }
    gc->texture.namesArray->refcount--;
    if (gc->texture.namesArray->refcount == 0) {
        __glNamesFreeArray(gc, gc->texture.namesArray);
    }
    gc->texture.namesArray = NULL;

    (*gc->imports.free)(gc, gc->state.texture.env);
}

/************************************************************************/

void APIENTRY __glim_TexGenfv(GLenum coord, GLenum pname, const GLfloat pv[])
{
    __GLtextureCoordState *tcs;
    __GLfloat v[4];
    __GLtransform *tr;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch (coord) {
      case GL_S: tcs = &gc->state.texture.s; break;
      case GL_T: tcs = &gc->state.texture.t; break;
      case GL_R: tcs = &gc->state.texture.r; break;
      case GL_Q: tcs = &gc->state.texture.q; break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	switch ((GLenum) pv[0]) {
	  case GL_EYE_LINEAR:
	  case GL_OBJECT_LINEAR:
	    tcs->mode = (GLenum) pv[0];
	    break;
	  case GL_SPHERE_MAP:
	    if ((coord == GL_R) || (coord == GL_Q)) {
		__glSetError(GL_INVALID_ENUM);
		return;
	    }
	    tcs->mode = (GLenum) pv[0];
	    break;
	  default:
	    __glSetError(GL_INVALID_ENUM);
	    return;
	}
	break;
      case GL_OBJECT_PLANE:
	tcs->objectPlaneEquation.x = pv[0];
	tcs->objectPlaneEquation.y = pv[1];
	tcs->objectPlaneEquation.z = pv[2];
	tcs->objectPlaneEquation.w = pv[3];
	break;
      case GL_EYE_PLANE:
	/*XXX transform should not be in generic code */
	v[0] = pv[0]; v[1] = pv[1]; v[2] = pv[2]; v[3] = pv[3];
	tr = gc->transform.modelView;
	if (tr->updateInverse) {
	    (*gc->procs.computeInverseTranspose)(gc, tr);
	}
	(*tr->inverseTranspose.xf4)(&tcs->eyePlaneEquation, v,
				    &tr->inverseTranspose);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glim_TexGenf(GLenum coord, GLenum pname, GLfloat f)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	__glim_TexGenfv(coord, pname, &f);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}

void APIENTRY __glim_TexGendv(GLenum coord, GLenum pname, const GLdouble pv[])
{
    __GLtextureCoordState *tcs;
    __GLfloat v[4];
    __GLtransform *tr;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch (coord) {
      case GL_S: tcs = &gc->state.texture.s; break;
      case GL_T: tcs = &gc->state.texture.t; break;
      case GL_R: tcs = &gc->state.texture.r; break;
      case GL_Q: tcs = &gc->state.texture.q; break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	switch ((GLenum) pv[0]) {
	  case GL_EYE_LINEAR:
	  case GL_OBJECT_LINEAR:
	    tcs->mode = (GLenum) pv[0];
	    break;
	  case GL_SPHERE_MAP:
	    if ((coord == GL_R) || (coord == GL_Q)) {
		__glSetError(GL_INVALID_ENUM);
		return;
	    }
	    tcs->mode = (GLenum) pv[0];
	    break;
	  default:
	    __glSetError(GL_INVALID_ENUM);
	    return;
	}
	break;
      case GL_OBJECT_PLANE:
	tcs->objectPlaneEquation.x = pv[0];
	tcs->objectPlaneEquation.y = pv[1];
	tcs->objectPlaneEquation.z = pv[2];
	tcs->objectPlaneEquation.w = pv[3]; 
	break;
      case GL_EYE_PLANE:
	/*XXX transform should not be in generic code */
	v[0] = pv[0]; v[1] = pv[1]; v[2] = pv[2]; v[3] = pv[3];
	tr = gc->transform.modelView;
	if (tr->updateInverse) {
	    (*gc->procs.computeInverseTranspose)(gc, tr);
	}
	(*tr->inverseTranspose.xf4)(&tcs->eyePlaneEquation, v,
				    &tr->inverseTranspose);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glim_TexGend(GLenum coord, GLenum pname, GLdouble d)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	__glim_TexGendv(coord, pname, &d);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}

void APIENTRY __glim_TexGeniv(GLenum coord, GLenum pname, const GLint pv[])
{
    __GLtextureCoordState *tcs;
    __GLfloat v[4];
    __GLtransform *tr;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    switch (coord) {
      case GL_S: tcs = &gc->state.texture.s; break;
      case GL_T: tcs = &gc->state.texture.t; break;
      case GL_R: tcs = &gc->state.texture.r; break;
      case GL_Q: tcs = &gc->state.texture.q; break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	switch ((GLenum) pv[0]) {
	  case GL_EYE_LINEAR:
	  case GL_OBJECT_LINEAR:
	    tcs->mode = (GLenum) pv[0];
	    break;
	  case GL_SPHERE_MAP:
	    if ((coord == GL_R) || (coord == GL_Q)) {
		__glSetError(GL_INVALID_ENUM);
		return;
	    }
	    tcs->mode = (GLenum) pv[0];
	    break;
	  default:
	    __glSetError(GL_INVALID_ENUM);
	    return;
	}
	break;
      case GL_OBJECT_PLANE:
	tcs->objectPlaneEquation.x = pv[0];
	tcs->objectPlaneEquation.y = pv[1];
	tcs->objectPlaneEquation.z = pv[2];
	tcs->objectPlaneEquation.w = pv[3]; 
	break;
      case GL_EYE_PLANE:
	/*XXX transform should not be in generic code */
	v[0] = pv[0]; v[1] = pv[1]; v[2] = pv[2]; v[3] = pv[3];
	tr = gc->transform.modelView;
	if (tr->updateInverse) {
	    (*gc->procs.computeInverseTranspose)(gc, tr);
	}
	(*tr->inverseTranspose.xf4)(&tcs->eyePlaneEquation, v,
				    &tr->inverseTranspose);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glim_TexGeni(GLenum coord, GLenum pname, GLint i)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_GEN_MODE:
	__glim_TexGeniv(coord, pname, &i);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}

GLint __glTexGendv_size(GLenum e)
{
    switch (e) {
      case GL_TEXTURE_GEN_MODE:
	return 1;
      case GL_OBJECT_PLANE:
      case GL_EYE_PLANE:
	return 4;
      default:
	return -1;
    }
}

GLint __glTexGenfv_size(GLenum e)
{
    return __glTexGendv_size(e);
}

GLint __glTexGeniv_size(GLenum e)
{
    return __glTexGendv_size(e);
}

/************************************************************************/

void APIENTRY __glim_TexParameterfv(GLenum target, GLenum pname, const GLfloat pv[])
{
    __GLtextureParamState *pts;
    GLenum e;
    __GLtexture *tex;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    pts = __glLookUpTextureParams(gc, target);

    if (!pts) {
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    tex = __glLookUpTexture(gc, target);
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

    switch (pname) {
      case GL_TEXTURE_WRAP_S:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    tex->params.sWrapMode = pts->sWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_WRAP_T:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    tex->params.tWrapMode = pts->tWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MIN_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	  case GL_NEAREST_MIPMAP_NEAREST:
	  case GL_LINEAR_MIPMAP_NEAREST:
	  case GL_NEAREST_MIPMAP_LINEAR:
	  case GL_LINEAR_MIPMAP_LINEAR:
	    tex->params.minFilter = pts->minFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MAG_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	    tex->params.magFilter = pts->magFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_BORDER_COLOR:
	__glClampColorf(gc, &pts->borderColor, pv);
	tex->params.borderColor = pts->borderColor;
	break;
      
      case GL_TEXTURE_PRIORITY:
	{
	    __GLtextureObjectState *ptos;
	    ptos = __glLookUpTextureTexobjs(gc, target);
	    ptos->priority = Clampf(pv[0], __glZero, __glOne);
	}
	break;

      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glim_TexParameterf(GLenum target, GLenum pname, GLfloat f)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
	__glim_TexParameterfv(target, pname, &f);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}

void APIENTRY __glim_TexParameteriv(GLenum target, GLenum pname, const GLint pv[])
{
    __GLtextureParamState *pts;
    GLenum e;
    __GLtexture *tex;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    pts = __glLookUpTextureParams(gc, target);
    tex = __glLookUpTexture(gc, target);

    if (!pts) {
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }
    
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    tex->params.sWrapMode = pts->sWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_WRAP_T:
	switch (e = (GLenum) pv[0]) {
	  case GL_REPEAT:
	  case GL_CLAMP:
	    tex->params.tWrapMode = pts->tWrapMode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MIN_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	  case GL_NEAREST_MIPMAP_NEAREST:
	  case GL_LINEAR_MIPMAP_NEAREST:
	  case GL_NEAREST_MIPMAP_LINEAR:
	  case GL_LINEAR_MIPMAP_LINEAR:
	    tex->params.minFilter = pts->minFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_MAG_FILTER:
	switch (e = (GLenum) pv[0]) {
	  case GL_NEAREST:
	  case GL_LINEAR:
	    tex->params.magFilter = pts->magFilter = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_BORDER_COLOR:
	__glClampColori(gc, &pts->borderColor, pv);
	tex->params.borderColor = pts->borderColor;
	break;
      case GL_TEXTURE_PRIORITY:
	{
	    __GLfloat priority;
	    __GLtextureObjectState *ptos;

	    ptos = __glLookUpTextureTexobjs(gc, target);
	    priority = __GL_I_TO_FLOAT(pv[0]);
	    ptos->priority = Clampf(priority, __glZero, __glOne);
	}
	break;
      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glim_TexParameteri(GLenum target, GLenum pname, GLint i)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
	__glim_TexParameteriv(target, pname, &i);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}

GLint __glTexParameterfv_size(GLenum e)
{
    switch (e) {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_PRIORITY:
	return 1;
      case GL_TEXTURE_BORDER_COLOR:
	return 4;
      default:
	return -1;
    }
}

GLint __glTexParameteriv_size(GLenum e)
{
    return __glTexParameterfv_size(e);
}

/************************************************************************/

void APIENTRY __glim_TexEnvfv(GLenum target, GLenum pname, const GLfloat pv[])
{
    __GLtextureEnvState *tes;
    GLenum e;
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
    tes = &gc->state.texture.env[target];

    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	switch(e = (GLenum) pv[0]) {
	  case GL_MODULATE:
	  case GL_DECAL:
	  case GL_BLEND:
	  case GL_REPLACE:
	  case GL_ADD:
	    tes->mode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_ENV_COLOR:
	__glClampAndScaleColorf(gc, &tes->color, pv);
	tes->color.r *= gc->constants.redRescale;
	tes->color.g *= gc->constants.greenRescale;
	tes->color.b *= gc->constants.blueRescale;
	tes->color.a *= gc->constants.alphaRescale;
	break;
      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glim_TexEnvf(GLenum target, GLenum pname, GLfloat f)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	__glim_TexEnvfv(target, pname, &f);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}

void APIENTRY __glim_TexEnviv(GLenum target, GLenum pname, const GLint pv[])
{
    __GLtextureEnvState *tes;
    GLenum e;
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
    tes = &gc->state.texture.env[target];

    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	switch(e = (GLenum) pv[0]) {
	  case GL_MODULATE:
	  case GL_DECAL:
	  case GL_BLEND:
	  case GL_REPLACE:
	  case GL_ADD:
	    tes->mode = e;
	    break;
	  default:
	    goto bad_enum;
	}
	break;
      case GL_TEXTURE_ENV_COLOR:
	__glClampAndScaleColori(gc, &tes->color, pv);
	tes->color.r *= gc->constants.redRescale;
	tes->color.g *= gc->constants.greenRescale;
	tes->color.b *= gc->constants.blueRescale;
	tes->color.a *= gc->constants.alphaRescale;
	break;
      default:
	goto bad_enum;
    }
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glim_TexEnvi(GLenum target, GLenum pname, GLint i)
{
    /* Accept only enumerants that correspond to single values */
    switch (pname) {
      case GL_TEXTURE_ENV_MODE:
	__glim_TexEnviv(target, pname, &i);
	break;
      default:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
}

GLint __glTexEnvfv_size(GLenum e)
{
    switch (e) {
      case GL_TEXTURE_ENV_MODE:
	return 1;
      case GL_TEXTURE_ENV_COLOR:
	return 4;
      default:
	return -1;
    }
}

GLint __glTexEnviv_size(GLenum e)
{
    return __glTexEnvfv_size(e);
}

/************************************************************************/

GLboolean __glIsTextureConsistent(__GLcontext *gc, __GLtexture *tex)
{
    __GLtextureParamState *params = &tex->params;
    GLint i, width, height, depth;
    GLint maxLevel;
    GLint border;
    GLenum baseFormat;
    GLenum requestedFormat;

    if ((tex->level[0]->width == 0) ||
	(tex->level[0]->height == 0) ||
	(tex->level[0]->depth == 0)) {
	return GL_FALSE;
    }

    border   = tex->level[0]->border;
    width    = tex->level[0]->width - border*2;
    height   = tex->level[0]->height - border*2;
    depth    = tex->level[0]->depth - border*2;
    maxLevel = gc->constants.maxMipMapLevel;

    baseFormat = tex->level[0]->baseFormat;
    if (gc->modes.rgbMode) {
	if (baseFormat == GL_COLOR_INDEX) {
	    baseFormat = tex->CT.baseFormat;
	    if (baseFormat == 0 || baseFormat == GL_COLOR_INDEX) {
		return GL_FALSE;
	    }
	}
    } else {
	if (baseFormat != GL_COLOR_INDEX) {
	    return GL_FALSE;
	}
    }

    requestedFormat = tex->level[0]->requestedFormat;

    switch(gc->state.texture.env[0].mode) {
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

	if (tex->level[i]->border != border ||
		tex->level[i]->requestedFormat != requestedFormat ||
		tex->level[i]->width != width + border*2 ||
		tex->level[i]->height != height + border*2 ||
		tex->level[i]->depth != depth + border*2) {
	    return GL_FALSE;
	}
    }

    return GL_TRUE;
}

static __GLtexture *CheckTexImageArgs(__GLcontext *gc, GLenum target, GLint lod,
				      GLint components, GLint border,
				      GLenum format, GLenum type, GLint dim)
{
    __GLtexture *tex = __glLookUpTexture(gc, target);

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
      case GL_COLOR_INDEX:	case GL_RED:
      case GL_GREEN:		case GL_BLUE:
      case GL_ALPHA:		case GL_RGB:
      case GL_RGBA:		case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:  case GL_ABGR_EXT:
      case GL_BGRA_EXT:		case GL_BGR_EXT:
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
      case GL_LUMINANCE4:	case GL_LUMINANCE8:
      case GL_LUMINANCE12:	case GL_LUMINANCE16:
	break;
      case GL_LUMINANCE_ALPHA:
      case GL_LUMINANCE4_ALPHA4:	case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:	case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:	case GL_LUMINANCE16_ALPHA16:
	break;
      case GL_RGB:
      case GL_R3_G3_B2:		case GL_RGB4:
      case GL_RGB5:		case GL_RGB8:
      case GL_RGB10:		case GL_RGB12:
      case GL_RGB16:
	break;
      case GL_RGBA:
      case GL_RGBA2:		case GL_RGBA4:
      case GL_RGBA8:		case GL_RGBA12:
      case GL_RGBA16:		case GL_RGB5_A1:
      case GL_RGB10_A2:
	break;
      case GL_ALPHA:
      case GL_ALPHA4:		case GL_ALPHA8:
      case GL_ALPHA12:		case GL_ALPHA16:
	break;
      case GL_INTENSITY:
      case GL_INTENSITY4:	case GL_INTENSITY8:
      case GL_INTENSITY12:	case GL_INTENSITY16:
	break;
    case GL_COLOR_INDEX1_EXT:	case GL_COLOR_INDEX2_EXT:
    case GL_COLOR_INDEX4_EXT:	case GL_COLOR_INDEX8_EXT:
    case GL_COLOR_INDEX12_EXT:	case GL_COLOR_INDEX16_EXT:
	if (format != GL_COLOR_INDEX)
	    goto bad_value;
	switch (type) {
	case GL_BYTE:	case GL_UNSIGNED_BYTE:
	case GL_SHORT:	case GL_UNSIGNED_SHORT:
	case GL_INT:	case GL_UNSIGNED_INT:
	    break;
	default:
	    goto bad_value;
	}
	
	break;
      default:
	goto bad_enum;
    }

    if ((border < 0) || (border > 1)) {
	goto bad_value;
    }

    return tex;
}


/*
** Used to store texture images.
*/
/* ARGSUSED */
void __glInitTexImageStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			  __GLtexture *tex, GLint lod)
{
    __GLmipMapLevel *lp = tex->level[lod];

    spanInfo->dstImage = lp->buffer;
    spanInfo->dstSkipPixels = 0;
    spanInfo->dstSkipLines = 0;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstLsbFirst = GL_TRUE;
    spanInfo->dstLineLength = lp->width;
    spanInfo->dim = tex->dim;
    if (tex->dim == 1) {
	spanInfo->dstSkipLines += lp->border;
    }

    spanInfo->dstFormat = lp->texFormat->pxFormat;
    spanInfo->dstType = lp->texFormat->pxType;
    spanInfo->dstAlignment = lp->texFormat->pxAlignment;
}

/*
** Used to get texture images.
*/
/*ARGSUSED*/
void __glInitTexImageGet(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 __GLtexture *tex, GLint lod)
{
    __GLmipMapLevel *lp = tex->level[lod];

    spanInfo->srcImage = lp->buffer;
    spanInfo->srcSkipPixels = 0;
    spanInfo->srcSkipLines = 0;
    spanInfo->srcSwapBytes = GL_FALSE;
    spanInfo->srcLsbFirst = GL_TRUE;
    spanInfo->srcLineLength = lp->width;
    spanInfo->dim = tex->dim;
    if (tex->dim == 1) {
	spanInfo->srcSkipLines += lp->border;
    }

    spanInfo->srcFormat = lp->texFormat->pxFormat;
    spanInfo->srcType = lp->texFormat->pxType;
    spanInfo->srcAlignment = lp->texFormat->pxAlignment;
}

/*
** Used to store texture images.  "packed" is set to GL_TRUE if this
** image is being pulled out of a display list, and GL_FALSE if it is
** being pulled directly out of an application.
*/
void __glInitTexSourceUnpack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			   GLsizei width, GLsizei height, GLsizei depth,
			   GLenum format, GLenum type, const GLvoid *buf,
			   GLboolean packed)
{
    spanInfo->x = 0;
    spanInfo->zoomx = __glOne;
    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->depth = depth;
    spanInfo->srcFormat = format;
    spanInfo->srcType = type;
    spanInfo->srcImage = buf;

    /* Set the remaining source modes according to the pixel state */
    __glLoadUnpackModes(gc, spanInfo, packed);
}

/*
** Used to get texture images.
*/
void __glInitTexDestPack(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			 GLsizei width, GLsizei height, GLsizei depth,
			 GLenum format, GLenum type, const GLvoid *buf)
{
    spanInfo->x = 0;
    spanInfo->zoomx = __glOne;
    spanInfo->width = width;
    spanInfo->height = height;
    spanInfo->depth = depth;
    spanInfo->dstFormat = format;
    spanInfo->dstType = type;
    spanInfo->dstImage = buf;

    /* Set the remaining source modes according to the pixel state */
    __glLoadPackModes(gc, spanInfo);
}

/*
** Return GL_TRUE if the given range (length or width/height) is a legal
** power of 2, taking into account the border.  The range is not allowed
** to be negative either.
*/
static GLboolean IsLegalRange(__GLcontext *gc, GLsizei r, GLint border)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    r -= border * 2;
    if ((r < 0) || (r & (r - 1))) {
	__glSetError(GL_INVALID_VALUE);
	return GL_FALSE;
    }
    return GL_TRUE;
}

__GLtexture *__glCheckTexImage1DArgs(__GLcontext *gc, GLenum target, GLint lod,
				     GLint components, GLsizei length,
				     GLint border, GLenum format, GLenum type)
{
    __GLtexture *tex;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexImageArgs(gc, target, lod, components, border,
			    format, type, 1);
    if (!tex) {
	return 0;
    }
    if (!IsLegalRange(gc, length, border)) {
	return 0;
    }
    return tex;
}

__GLtexture *__glCheckTexImage2DArgs(__GLcontext *gc, GLenum target, GLint lod,
				     GLint components, GLsizei w, GLsizei h,
				     GLint border, GLenum format, GLenum type)
{
    __GLtexture *tex;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexImageArgs(gc, target, lod, components, border,
			    format, type, 2);
    if (!tex) {
	return 0;
    }
    if (!IsLegalRange(gc, w, border)) {
	return 0;
    }
    if (!IsLegalRange(gc, h, border)) {
	return 0;
    }
    return tex;
}

/*ARGSUSED*/
__GLtexture *__glCheckCopyTexImageArgs(__GLcontext *gc, GLenum target, 
					 GLint level,
					 GLenum internalformat,
					 GLint x, GLint y, 
					 GLsizei width, GLsizei height,
					 GLint border, GLint dim)
{
    __GLtexture *tex;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexImageArgs(gc, target, level, internalformat, border, 
			    GL_RGBA, GL_FLOAT, dim);

    switch(target) {
      case GL_PROXY_TEXTURE_1D:
      case GL_PROXY_TEXTURE_2D:
	__glSetError(GL_INVALID_ENUM);
	return NULL;
    }
    switch(internalformat) {
	case 1: case 2: case 3: case 4:
	    __glSetError(GL_INVALID_ENUM);
	    return NULL;
	default:
	    break;
    }
    if (!tex) {
	return 0;
    }
    if (!IsLegalRange(gc, width, border)) {
	return 0;
    }
    if ((dim > 1) && 
	!IsLegalRange(gc, height, border)) {
	return 0;
    }
    return tex;
}

/************************************************************************/

void APIENTRY __glim_TexImage1D(GLenum target, GLint lod, 
		       GLint components, GLsizei length,
		       GLint border, GLenum format,
		       GLenum type, const GLvoid *buf)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_STATE();

    /* Check arguments and get the right texture being changed */
    tex = __glCheckTexImage1DArgs(gc, target, lod, components, length,
				  border, format, type);
    if (!tex) {
	return;
    }
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

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

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

void __gllei_TexImage1D(__GLcontext *gc, GLenum target, GLint lod,
		        GLint components, GLsizei length, GLint border,
		        GLenum format, GLenum type, const GLubyte *image)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

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
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

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

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

/************************************************************************/

void APIENTRY __glim_TexImage2D(GLenum target, GLint lod, GLint components,
		       GLsizei w, GLsizei h, GLint border, GLenum format,
		       GLenum type, const GLvoid *buf)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_STATE();

    /* Check arguments and get the right texture being changed */
    tex = __glCheckTexImage2DArgs(gc, target, lod, components, w, h,
				  border, format, type);
    if (!tex) {
	return;
    }
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, lod, components,
			       w, h, 1+border*2, border, 2);

    /* Copy image data */
    if (buf && dest) {
	__glInitTexSourceUnpack(gc, &spanInfo, w, h, 1,
				format, type, buf, GL_FALSE);
	__glInitTexImageStore(gc, &spanInfo, tex, lod);
	__glInitUnpacker(gc, &spanInfo);
	__glInitPacker(gc, &spanInfo);
	(*tex->copyTexImage)(gc, &spanInfo, tex, lod);
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

void __gllei_TexImage2D(__GLcontext *gc, GLenum target, GLint lod, 
		        GLint components, GLsizei w, GLsizei h, 
		        GLint border, GLenum format, GLenum type,
		        const GLubyte *image)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    GLuint beginMode;

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
    tex = __glCheckTexImage2DArgs(gc, target, lod, components, w, h,
				  border, format, type);
    if (!tex) {
	return;
    }
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

    /* Allocate memory for the level data */
    dest = (*tex->createLevel)(gc, tex, lod, components,
			       w, h, 1+border*2, border, 2);

    /* Copy image data */
    if (image && dest) {
	__glInitTexSourceUnpack(gc, &spanInfo, w, h, 1,
				format, type, image, GL_TRUE);
	__glInitTexImageStore(gc, &spanInfo, tex, lod);
	__glInitUnpacker(gc, &spanInfo);
	__glInitPacker(gc, &spanInfo);
	(*tex->copyTexImage)(gc, &spanInfo, tex, lod);
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

/************************************************************************/

static __GLfloat Dot(const __GLcoord *v1, const __GLcoord *v2)
{
    return (v1->x * v2->x + v1->y * v2->y + v1->z * v2->z);
}

/*
** Compute the s & t coordinates for a sphere map.  The s & t values
** are stored in "result" even if both coordinates are not being
** generated.  The caller picks the right values out.
*/
static void SphereGen(__GLcontext *gc, __GLvertex *vx, __GLcoord *result)
{
    __GLcoord u, r;
    __GLfloat m, ndotu;

    /* Get unit vector from origin to the vertex in eye coordinates into u */
    (*gc->procs.normalize)(&u.x, &vx->eye.x);

    /* Dot the normal with the unit position u */
    ndotu = Dot(&vx->normal, &u);

    /* Compute r */
    r.x = u.x - 2 * vx->normal.x * ndotu;
    r.y = u.y - 2 * vx->normal.y * ndotu;
    r.z = u.z - 2 * vx->normal.z * ndotu;

    /* Compute m */
    m = 2 * __GL_SQRTF(r.x*r.x + r.y*r.y + (r.z + 1) * (r.z + 1));

    if (m) {
	result->x = r.x / m + __glHalf;
	result->y = r.y / m + __glHalf;
    } else {
	result->x = __glHalf;
	result->y = __glHalf;
    }
}

/*
** Transform or compute the texture coordinates for this vertex.
*/
void __glCalcMixedTexture(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord sphereCoord, gen, *c;
    GLboolean didSphereGen = GL_FALSE;
    GLuint texenables = gc->state.enables.texture;
    __GLmatrix *m;

    /* Generate/copy s coordinate */
    if (texenables & __GL_TEXTURE_GEN_S_ENABLE) {
	switch (gc->state.texture.s.mode) {
	  case GL_EYE_LINEAR:
	    c = &gc->state.texture.s.eyePlaneEquation;
	    gen.x = c->x * vx->eye.x + c->y * vx->eye.y
		+ c->z * vx->eye.z + c->w * vx->eye.w;
	    break;
	  case GL_OBJECT_LINEAR:
	    c = &gc->state.texture.s.objectPlaneEquation;
	    gen.x = c->x * vx->obj.x + c->y * vx->obj.y
		+ c->z * vx->obj.z + c->w * vx->obj.w;
	    break;
	  case GL_SPHERE_MAP:
	    SphereGen(gc, vx, &sphereCoord);
	    gen.x = sphereCoord.x;
	    didSphereGen = GL_TRUE;
	    break;
	}
    } else {
	gen.x = vx->texture.x;
    }

    /* Generate/copy t coordinate */
    if (texenables & __GL_TEXTURE_GEN_T_ENABLE) {
	switch (gc->state.texture.t.mode) {
	  case GL_EYE_LINEAR:
	    c = &gc->state.texture.t.eyePlaneEquation;
	    gen.y = c->x * vx->eye.x + c->y * vx->eye.y
		+ c->z * vx->eye.z + c->w * vx->eye.w;
	    break;
	  case GL_OBJECT_LINEAR:
	    c = &gc->state.texture.t.objectPlaneEquation;
	    gen.y = c->x * vx->obj.x + c->y * vx->obj.y
		+ c->z * vx->obj.z + c->w * vx->obj.w;
	    break;
	  case GL_SPHERE_MAP:
	    if (!didSphereGen) {
		SphereGen(gc, vx, &sphereCoord);
	    }
	    gen.y = sphereCoord.y;
	    break;
	}
    } else {
	gen.y = vx->texture.y;
    }

    /* Generate/copy r coordinate */
    if (texenables & __GL_TEXTURE_GEN_R_ENABLE) {
	switch (gc->state.texture.r.mode) {
	  case GL_EYE_LINEAR:
	    c = &gc->state.texture.r.eyePlaneEquation;
	    gen.z = c->x * vx->eye.x + c->y * vx->eye.y
		+ c->z * vx->eye.z + c->w * vx->eye.w;
	    break;
	  case GL_OBJECT_LINEAR:
	    c = &gc->state.texture.r.objectPlaneEquation;
	    gen.z = c->x * vx->obj.x + c->y * vx->obj.y
		+ c->z * vx->obj.z + c->w * vx->obj.w;
	    break;
	}
    } else {
	gen.z = vx->texture.z;
    }

    /* Generate/copy q coordinate */
    if (texenables & __GL_TEXTURE_GEN_Q_ENABLE) {
	switch (gc->state.texture.q.mode) {
	  case GL_EYE_LINEAR:
	    c = &gc->state.texture.q.eyePlaneEquation;
	    gen.w = c->x * vx->eye.x + c->y * vx->eye.y
		+ c->z * vx->eye.z + c->w * vx->eye.w;
	    break;
	  case GL_OBJECT_LINEAR:
	    c = &gc->state.texture.q.objectPlaneEquation;
	    gen.w = c->x * vx->obj.x + c->y * vx->obj.y
		+ c->z * vx->obj.z + c->w * vx->obj.w;
	    break;
	}
    } else {
	gen.w = vx->texture.w;
    }

    /* Finally, apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &gen.x, m);
}

void __glCalcEyeLinear(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord gen, *c;
    __GLmatrix *m;

    /* Generate texture coordinates from eye coordinates */
    c = &gc->state.texture.s.eyePlaneEquation;
    gen.x = c->x * vx->eye.x + c->y * vx->eye.y + c->z * vx->eye.z
	+ c->w * vx->eye.w;
    c = &gc->state.texture.t.eyePlaneEquation;
    gen.y = c->x * vx->eye.x + c->y * vx->eye.y + c->z * vx->eye.z
	+ c->w * vx->eye.w;
    gen.z = vx->texture.z;
    gen.w = vx->texture.w;

    /* Finally, apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &gen.x, m);
}

void __glCalcObjectLinear(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord gen, *c;
    __GLmatrix *m;

    /* Generate texture coordinates from object coordinates */
    c = &gc->state.texture.s.objectPlaneEquation;
    gen.x = c->x * vx->obj.x + c->y * vx->obj.y + c->z * vx->obj.z
	+ c->w * vx->obj.w;
    c = &gc->state.texture.t.objectPlaneEquation;
    gen.y = c->x * vx->obj.x + c->y * vx->obj.y + c->z * vx->obj.z
	+ c->w * vx->obj.w;
    gen.z = vx->texture.z;
    gen.w = vx->texture.w;

    /* Finally, apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &gen.x, m);
}

void __glCalcSphereMap(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord sphereCoord;
    __GLmatrix *m;

    SphereGen(gc, vx, &sphereCoord);
    sphereCoord.z = vx->texture.z;
    sphereCoord.w = vx->texture.w;

    /* Finally, apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &sphereCoord.x, m);
}

void __glCalcTexture(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord copy;
    __GLmatrix *m;

    copy.x = vx->texture.x;
    copy.y = vx->texture.y;
    copy.z = vx->texture.z;
    copy.w = vx->texture.w;

    /* Apply texture matrix */
    m = &gc->transform.texture->matrix;
    (*m->xf4)(&vx->texture, &copy.x, m);
}

void __glCalcTexturePersp(__GLcontext *gc, __GLvertex *vx)
{
    (*gc->procs.calcTexture2)(gc, vx);

    if (!(vx->hasAndClipCode & __GL_CLIP_MASK)) {
	vx->texture.x *= vx->window.w;
	vx->texture.y *= vx->window.w;
	vx->texture.w *= vx->window.w;
    }
}

void __glFastCalcTexturePersp(__GLcontext *gc, __GLvertex *vx)
{
    if (gc->transform.texture->matrix.matrixType != __GL_MT_IDENTITY) {
	(*gc->procs.calcTexture2)(gc, vx);
    }

    if (!(vx->hasAndClipCode & __GL_CLIP_MASK)) {
	vx->texture.x *= vx->window.w;
	vx->texture.y *= vx->window.w;
	vx->texture.w *= vx->window.w;
    }
}

void __glCalcTextureUVScale(__GLcontext *gc, __GLvertex *vx)
{
    __GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

    (*gc->procs.calcTexture2)(gc, vx);

    vx->texture.x *= lp->width2f;
    vx->texture.y *= lp->height2f;
}

void __glFastCalcTextureUVScale(__GLcontext *gc, __GLvertex *vx)
{
    __GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

    if (gc->transform.texture->matrix.matrixType != __GL_MT_IDENTITY) {
	(*gc->procs.calcTexture2)(gc, vx);
    }

    vx->texture.x *= lp->width2f;
    vx->texture.y *= lp->height2f;
}

void __glCalcTexturePerspUVScale(__GLcontext *gc, __GLvertex *vx)
{
    __GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

    (*gc->procs.calcTexture2)(gc, vx);

    if (!(vx->hasAndClipCode & __GL_CLIP_MASK)) {
	vx->texture.x *= vx->window.w*lp->width2f;
	vx->texture.y *= vx->window.w*lp->height2f;
	vx->texture.w *= vx->window.w;
    } else {
	vx->texture.x *= lp->width2f;
	vx->texture.y *= lp->height2f;
    }
}

void __glFastCalcTexturePerspUVScale(__GLcontext *gc, __GLvertex *vx)
{
    __GLmipMapLevel *lp = gc->texture.currentTexture->level[0];

    if (gc->transform.texture->matrix.matrixType != __GL_MT_IDENTITY) {
	(*gc->procs.calcTexture2)(gc, vx);
    }

    if (!(vx->hasAndClipCode & __GL_CLIP_MASK)) {
	vx->texture.x *= vx->window.w*lp->width2f;
	vx->texture.y *= vx->window.w*lp->height2f;
	vx->texture.w *= vx->window.w;
    } else {
	vx->texture.x *= lp->width2f;
	vx->texture.y *= lp->height2f;
    }
}

/************************************************************************/

static __GLtexture *CheckTexSubImageArgs(__GLcontext *gc, GLenum target,
					 GLint lod, GLenum format,
					 GLenum type, GLint dim)
{
    __GLtexture *tex = __glLookUpTexture(gc, target);

    if (!tex || (target == GL_PROXY_TEXTURE_1D) ||
		(target == GL_PROXY_TEXTURE_2D))
    {
      bad_enum:
	__glSetError(GL_INVALID_ENUM);
	return 0;
    }

    if (tex->dim != dim) {
	goto bad_enum;
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
      case GL_COLOR_INDEX:	case GL_RED:
      case GL_GREEN:		case GL_BLUE:
      case GL_ALPHA:		case GL_RGB:
      case GL_RGBA:		case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:  case GL_ABGR_EXT:
      case GL_BGRA_EXT:		case GL_BGR_EXT:
	break;
      default:
	goto bad_enum;
    }

    if ((lod < 0) || (lod >= gc->constants.maxMipMapLevel)) {
	__glSetError(GL_INVALID_VALUE);
	return 0;
    }

    return tex;
}

void __glInitTexSubImageStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
			     __GLtexture *tex, GLint lod,
			     GLint xoffset, GLint yoffset, GLint zoffset)
{
    __GLmipMapLevel *lp = tex->level[lod];

    __glInitTexImageStore(gc, spanInfo, tex, lod);
    spanInfo->dstSkipPixels = xoffset + lp->border;
    spanInfo->dstSkipLines = yoffset + lp->border;
    spanInfo->dstLineLength = lp->width;
}

static GLboolean CheckTexSubImageRange(__GLcontext *gc, __GLmipMapLevel *lp,
				       GLint offset, GLsizei size, GLsizei max)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    if ((size < 0) || (offset < -lp->border) || (offset+size > max-lp->border))
    {
	__glSetError(GL_INVALID_VALUE);
	return GL_FALSE;
    }
    return GL_TRUE;
}

__GLtexture *__glCheckTexSubImage1DArgs(__GLcontext *gc, GLenum target,
					GLint lod,
					GLint xoffset, GLint length,
					GLenum format, GLenum type)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexSubImageArgs(gc, target, lod, format, type, 1);
    if (!tex) {
	return NULL;
    }
    lp = tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return NULL;
    }
    if (!CheckTexSubImageRange(gc, lp, xoffset, length, lp->width)) {
	return NULL;
    }
    return tex;
}

__GLtexture *__glCheckTexSubImage2DArgs(__GLcontext *gc, GLenum target,
					GLint lod,
					GLint xoffset, GLint yoffset,
					GLsizei w, GLsizei h,
					GLenum format, GLenum type)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexSubImageArgs(gc, target, lod, format, type, 2);
    if (!tex) {
	return NULL;
    }
    lp = tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return NULL;
    }
    if (!CheckTexSubImageRange(gc, lp, xoffset, w, lp->width)) {
	return NULL;
    }
    if (!CheckTexSubImageRange(gc, lp, yoffset, h, lp->height)) {
	return NULL;
    }
    return tex;
}

/*ARGSUSED*/
__GLtexture *__glCheckCopyTexSubImageArgs(__GLcontext *gc, GLenum target,
					GLint lod,
					GLint xoffset, GLint yoffset, GLint zoffset,
					GLint x, GLint y,
					GLsizei w, GLsizei h, GLint dim)
{
    __GLtexture *tex;
    __GLmipMapLevel *lp;

    /* Check arguments and get the right texture being changed */
    tex = CheckTexSubImageArgs(gc, target, lod, GL_RGBA, GL_FLOAT, dim);
    if (!tex) {
	return NULL;
    }
    lp = tex->level[lod];
    if (lp->buffer == NULL) {
	__glSetError(GL_INVALID_OPERATION);
	return NULL;
    }
    if (!CheckTexSubImageRange(gc, lp, xoffset, w, lp->width)) {
	return NULL;
    }
    if (!CheckTexSubImageRange(gc, lp, yoffset, h, lp->height)) {
	return NULL;
    }
    if (!CheckTexSubImageRange(gc, lp, zoffset, 1, lp->depth)) {
	return NULL;
    }
    return tex;
}

void APIENTRY __glim_TexSubImage1D(GLenum target, GLint lod, 
		       GLint xoffset, GLint length,
		       GLenum format, GLenum type, const GLvoid *buf)
{
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
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

void __gllei_TexSubImage1D(__GLcontext *gc, GLenum target, GLint lod,
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

void APIENTRY __glim_TexSubImage2D(GLenum target, GLint lod,
		       GLint xoffset, GLint yoffset,
		       GLsizei w, GLsizei h, GLenum format,
		       GLenum type, const GLvoid *buf)
{
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_STATE();

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckTexSubImage2DArgs(gc, target, lod, xoffset, yoffset, w, h,
				     format, type);
    if (!tex) {
	return;
    }

    /* Copy sub-image data */
    __glInitTexSourceUnpack(gc, &spanInfo, w, h, 1,
			    format, type, buf, GL_FALSE);
    __glInitTexSubImageStore(gc, &spanInfo, tex, lod, xoffset, yoffset, 0);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);
    (*tex->copyTexImage)(gc, &spanInfo, tex, lod);

    if (tex->level[lod]->pixelBuffer)
	__glCookSubTexture(gc, tex->level[lod], xoffset, yoffset, w, h);
}

void __gllei_TexSubImage2D(__GLcontext *gc, GLenum target, GLint lod, 
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

    if (tex->level[lod]->pixelBuffer)
	__glCookSubTexture(gc, tex->level[lod], xoffset, yoffset, w, h);
}

/************************************************************************/
/*
** Texture Object extension routines.
*/
/************************************************************************/


#define __GL_CHECK_VALID_N_PARAM(failStatement)				\
    if (n < 0) {							\
	__glSetError(GL_INVALID_VALUE);					\
    }									\
    if (n == 0) {							\
	failStatement;							\
    }									\


/*
 * Generic texture object free function; called from so_names.c
 */
GLvoid __glDisposeTexObj(__GLcontext *gc, void *pData)
{
    __GLtexture *texobj = (__GLtexture *)pData;

    texobj->refcount--;
    assert(texobj->refcount >= 0);

    if (texobj->refcount == 0) {
	(*texobj->free)(gc, texobj);
    }
}

GLvoid APIENTRY __glim_GenTextures(GLsizei n, GLuint* textures)
{
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);
    __GL_API_BLAND();

    if (NULL == textures) return;

    assert(NULL != gc->texture.namesArray);

    __glNamesGenNames(gc, gc->texture.namesArray, n, textures);

}

GLvoid APIENTRY __glim_DeleteTextures(GLsizei n, const GLuint* textures)
{
    GLuint start, rangeVal;
    GLint numTextures, targetIndex, i;
    __GLnamesArray *array;
    __GLtexture *texobj, **pBoundTexture;

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
	if (0 == textures[i]) { 	/* skip default textures */
	    /* delete up to this one */
	    __glNamesDeleteRange(gc,array,start,rangeVal-start);
	    /* skip over this one by setting start to the next one */
	    start = textures[i+1];
	    rangeVal = start-1; 	/* because it gets incremented later */
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
	for (targetIndex=0, pBoundTexture = gc->texture.texture; 
		targetIndex < numTextures; targetIndex++, pBoundTexture++) {

	    /* Is the texture currently bound? */
	    if ((*pBoundTexture)->texobjs.name == textures[i]) {
		__GLperTextureState *pts;
		pts = &gc->state.texture.texture[targetIndex];
		/* if we don't unlock it, it won't get deleted */
		__glNamesUnlockData(gc, *pBoundTexture);

		/* bind the default texture to this target */
		texobj = gc->texture.defaultTextures[targetIndex];
		assert(texobj->texobjs.name == 0);

		gc->texture.texture[targetIndex] = texobj;
		*pBoundTexture = texobj;
		pts->texobjs   = texobj->texobjs;
		pts->params    = texobj->params;

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
void __glBindTexture(__GLcontext *gc, GLuint targetIndex, GLuint texture)
{
    __GLtexture *texobj;
    __GLperTextureState *pts;
    __GLtexture *boundTexture;

    assert(NULL != gc->texture.namesArray);

    /*
    ** Retrieve the texture object from the namesArray structure.
    */
    if (texture == 0) {
	texobj = gc->texture.defaultTextures[targetIndex];
	assert(NULL != texobj);
	assert(texobj->texobjs.name == 0);
    }
    else {
	texobj = (__GLtexture *)
		__glNamesLockData(gc, gc->texture.namesArray, texture);
    }

    /*
    ** Is this the first time this name has been bound?
    ** If so, create a new texture object and initialize it.
    */
    if (NULL == texobj) {
	texobj = (*gc->texture.createTexture)(gc, texture, targetIndex);
	assert(NULL != texobj);
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
	assert(texture == texobj->texobjs.name);
    }

    pts = &(gc->state.texture.texture[targetIndex]);
    boundTexture = gc->texture.texture[targetIndex];
    boundTexture->params  = pts->params;
    boundTexture->texobjs = pts->texobjs;
    if (boundTexture->texobjs.name != 0) {
        /* Unlock the texture that is being unbound.  */
        __glNamesUnlockData(gc, boundTexture);
    }

    /*
    ** Install the new texture into the correct target and save
    ** its pointer so it can be unlocked easily when it is unbound.
    */
    gc->texture.texture[targetIndex]       = texobj;		

   /* Now we adopt the params and texobj state of the newly bound texture */
    pts->params  = texobj->params;
    pts->texobjs = texobj->texobjs;

}

GLvoid APIENTRY __glim_BindTexture(GLenum target, GLuint texture)
{
    GLuint targetIndex;
    /*
    ** Need to validate in case a new texture was popped into
    ** the state immediately prior to this call.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
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

    __glBindTexture(gc, targetIndex, texture);

    /* Need to reset the current texture and such. */
    __GL_DELAY_VALIDATE(gc);
}


GLvoid APIENTRY __glim_PrioritizeTextures(GLsizei n,
                           const GLuint* textures,
                           const GLclampf* priorities)
{
    int i;
    __GLtexture *texobj;

    __GL_SETUP_NOT_IN_BEGIN();
    __GL_CHECK_VALID_N_PARAM(return);
    __GL_API_STATE();

    for (i=0; i < n; i++) {
	/* silently ignore default texture */
	if (0 == textures[i]) continue;

	texobj = (__GLtexture *)
	    __glNamesLockData(gc, gc->texture.namesArray, textures[i]);

	/* silently ignore non-texture */
	if (NULL == texobj) continue;

	texobj->texobjs.priority = 
			Clampf(priorities[i], __glZero, __glOne);

	/* Is it currently bound? */
	if (texobj->texobjs.name == 
	    gc->texture.texture[texobj->targetIndex]->texobjs.name) {
	    /* Make sure the priority is set in the current texture. */
	    gc->state.texture.texture[texobj->targetIndex].texobjs.priority =
	    gc->texture.texture[texobj->targetIndex]->texobjs.priority = 
		texobj->texobjs.priority;
	}
	__glNamesUnlockData(gc, texobj);
    }

/* XXX reshuffle textures now */
/* __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_TEXTURE_PRIORITIES);*/
}


static
GLboolean IsTextureResident(__GLcontext *gc, __GLtexture *tex)
{
    GLint i;
    int maxMipMapLevel = gc->constants.maxMipMapLevel;

    for (i=0; i < maxMipMapLevel; i++) {
	if (!tex->level[i]->buffer) {
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
        if (tex->level[i]->width == 1 && tex->level[i]->height == 1) break;
    }

    return GL_TRUE;
}

GLboolean APIENTRY __glim_AreTexturesResident(GLsizei n,
                               const GLuint* textures,
                               GLboolean* residences)
{
    int i;
    __GLtexture *texobj;
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
	texobj = (__GLtexture *)
	    __glNamesLockData(gc, gc->texture.namesArray, textures[i]);
	/*
	** Ensure that all of the names have corresponding textures.
	*/
	if (NULL == texobj) {
	    __glSetError(GL_INVALID_VALUE);
	    return GL_FALSE;
	}
	if (!__glIsTextureConsistent(gc, texobj) || 
	    !IsTextureResident(gc, texobj)) {
	    allResident   = GL_FALSE;
	    residences[i] = GL_FALSE;
	}
	else {
	    residences[i] = GL_TRUE;
	}
	__glNamesUnlockData(gc, texobj);
    }

    return allResident;
}

GLboolean APIENTRY __glim_IsTexture(GLuint texture)
{
    __GLtexture *texobj;
    __GL_SETUP_NOT_IN_BEGIN2();
    __GL_API_GET();

    if (0 == texture) return GL_FALSE;

    texobj = (__GLtexture *)
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


void APIENTRY __glim_CopyTexImage1D(GLenum target,
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
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_PIXEL_OP();

    /* Check arguments and get the right texture being changed */
    tex = __glCheckCopyTexImageArgs(gc, target, level, internalformat, 
					x, y, width, 1, border, 1);
    if (!tex) {
	return;
    }
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

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
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

void APIENTRY __glim_CopyTexImage2D(GLenum target,
			       GLint level,
			       GLenum internalformat,
			       GLint x, GLint y,
			       GLsizei w, GLsizei h,
			       GLint border)
{
    __GLtexture *tex;
    __GLtextureBuffer *dest;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_PIXEL_OP();

    /* Check arguments and get the right texture being changed */
    tex = __glCheckCopyTexImageArgs(gc, target, level, internalformat, 
					x, y, w, h, border, 2);
    if (!tex) {
	return;
    }
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

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
    }

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}


void APIENTRY __glim_CopyTexSubImage1D(GLenum target,
			      GLint level,
			      GLint xoffset,
			      GLint x,
			      GLint y,
			      GLsizei width)
{
    __GLtexture *tex;
    __GLpixelSpanInfo spanInfo;
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_PIXEL_OP();

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckCopyTexSubImageArgs(gc, target, level,
					xoffset, 0, 0, x, y, width, 1, 1);
    if (!tex) {
	return;
    }
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

    /* Copy sub-image data */
    __glInitReadImageSrcInfo(gc, &spanInfo, x, y, width, 1);
    __glInitTexSubImageStore(gc, &spanInfo, tex, level, xoffset, 0, 0);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);

    if (!__glClipReadPixels(gc, &spanInfo)) return;
    (*tex->readTexImage)(gc, &spanInfo, tex, level);
}

void APIENTRY __glim_CopyTexSubImage2D(GLenum target,
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
    /*
    ** Validate because we use the copyImage proc which may be affected
    ** by the pickers.
    */
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();
    __GL_API_PIXEL_OP();

    /* Check arguments and get the right texture level being changed */
    tex = __glCheckCopyTexSubImageArgs(gc, target, level,
					xoffset, yoffset, 0,
					x, y, width, height, 2);
    if (!tex) {
	return;
    }
    if (tex->refcount > 2) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }

    /* Copy sub-image data */
    __glInitReadImageSrcInfo(gc, &spanInfo, x, y, width, height);
    __glInitTexSubImageStore(gc, &spanInfo, tex, level, xoffset, yoffset, 0);
    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);

    if (!__glClipReadPixels(gc, &spanInfo)) return;
    (*tex->readTexImage)(gc, &spanInfo, tex, level);
}

/************************************************************************/

/* Reformat textures into framebuffer format for fast undithered REPLACE */
void __glCookSubTexture(__GLcontext *gc, __GLmipMapLevel *lp,
			int x, int y, int w, int h)
{
    GLint row, col, rs, gs, bs, as;
    GLubyte *srcPtr, *dstPtr;
    GLuint srcSkip, dstSkip;
    GLuint dst_bpp;
    __GLcolorBuffer *cfb = gc->drawBuffer;

    rs = 8 - gc->modes.redBits;
    gs = 8 - gc->modes.greenBits;
    bs = 8 - gc->modes.blueBits;
    as = 8 - gc->modes.alphaBits;

    dst_bpp = gc->drawBuffer->buf.elementSize;

    srcPtr = ((GLubyte *)lp->buffer) + w*y + x;
    dstPtr = ((GLubyte *)lp->pixelBuffer) + (w*y + x) * dst_bpp;

    srcSkip = lp->width - w;
    dstSkip = srcSkip * dst_bpp;

    for (row = 0; row < h; row++) {
	for (col = 0; col < w; col++) {
	    GLuint pixel;
	    GLubyte l, r, g, b, a;

	    switch (lp->internalFormat) {
	    case __GL_FORMAT_LUMINANCE8:
		l = *srcPtr++;
		r = l;
		g = l;
		b = l;
		a = 0; /* Uses fragment alpha */ 
		break;

	    case __GL_FORMAT_LUMINANCE_ALPHA8:
		l = srcPtr[0];
		r = l;
		g = l;
		b = l;
		a = srcPtr[1];
		srcPtr += 2;
		break;

	    case __GL_FORMAT_RGB8:
		r = srcPtr[0];
		g = srcPtr[1];
		b = srcPtr[2];
		a = 0; /* uses fragment alpha */
		srcPtr += 3;
		break;
	    case __GL_FORMAT_RGBA8:
		r = srcPtr[0];
		g = srcPtr[1];
		b = srcPtr[2];
		a = srcPtr[3];
		srcPtr += 4;
		break;
	    case __GL_FORMAT_INTENSITY8:
		l = *srcPtr++;
		r = l;
		g = l;
		b = l;
		a = l;
		break;
	    }

	    r >>= rs;
	    g >>= gs;
	    b >>= bs;
	    a >>= as;

	    pixel =
		(r << cfb->redShift) |
		(g << cfb->greenShift) |
		(b << cfb->blueShift) |
		(a << cfb->alphaShift);

	    switch (dst_bpp) {
	    case 1:
		*((GLubyte*)dstPtr) = pixel;
		dstPtr += sizeof(GLubyte);
		break;
	    case 2:
		*((GLushort*)dstPtr) = pixel;
		dstPtr += sizeof(GLushort);
		break;
	    case 4:
		*((GLuint*)dstPtr) = pixel;
		dstPtr += sizeof(GLuint);
		break;
	    }
	}
	srcPtr += srcSkip;
	dstPtr += dstSkip;
    }
}

void __glCookTexture(__GLcontext *gc)
{
    __GLmipMapLevel *lp = gc->texture.currentTexture->level[0];
    GLuint pixelCount;
    GLuint bufferSize;
    __GLcolorBuffer *cfb = gc->drawBuffer;

    /* Already cooked! */
    if (lp->pixelBuffer)
	return;

    pixelCount = lp->width * lp->height;
    bufferSize = pixelCount * gc->drawBuffer->buf.elementSize;

    lp->pixelBuffer = (GLvoid *)
	(*gc->imports.realloc)(gc, lp->pixelBuffer, (size_t) bufferSize);

    __glCookSubTexture(gc, lp, 0, 0, lp->width, lp->height);
}

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

void __glCookSubTexture(__GLcontext *gc, __GLmipMapLevel *lp,
                        int x, int y, int w, int h);

/*
** Some math routines that are optimized in assembly
*/
#ifdef __GL_USE_MIPSASMCODE
#define __GL_FRAC(f)    __glFrac(f)
#else
#define __GL_FRAC(f)    ((f) - __GL_FLOORF(f))
#endif

/************************************************************************/

#define __GL_TEXTURE_INDEX_1D 0
#define __GL_TEXTURE_INDEX_2D 1
#define __GL_PROXY_TEXTURE_INDEX_1D 2
#define __GL_PROXY_TEXTURE_INDEX_2D 3

__GLtextureParamState * __glLookUpTextureParams(__GLcontext *gc, GLenum target, int texUnit )
{

    switch (target) {
      case GL_TEXTURE_1D:
        return &gc->state.texture[texUnit].texture[__GL_TEXTURE_INDEX_1D].params;
      case GL_TEXTURE_2D:
        return &gc->state.texture[texUnit].texture[__GL_TEXTURE_INDEX_2D].params;
      default:
        return 0;
    }
}

__GLtextureObjectState * __glLookUpTextureTexobjs(__GLcontext *gc, 
                                                    GLenum target,
                                                  int texUnit )
{
    switch (target) {
      case GL_TEXTURE_1D:
        return &gc->state.texture[texUnit].texture[__GL_TEXTURE_INDEX_1D].texobjs;
      case GL_TEXTURE_2D:
        return &gc->state.texture[texUnit].texture[__GL_TEXTURE_INDEX_2D].texobjs;
      default:
        return 0;
    }
}

__GLtexture * __glLookUpTexture(__GLcontext *gc, GLenum target, int texUnit )
{
    switch (target) {
      case GL_TEXTURE_1D:
        return &gc->texture.texture[texUnit][__GL_TEXTURE_INDEX_1D]->map;
      case GL_TEXTURE_2D:
        return &gc->texture.texture[texUnit][__GL_TEXTURE_INDEX_2D]->map;
      case GL_PROXY_TEXTURE_1D:
        return &gc->texture.texture[texUnit][__GL_PROXY_TEXTURE_INDEX_1D]->map;
      case GL_PROXY_TEXTURE_2D:
        return &gc->texture.texture[texUnit][__GL_PROXY_TEXTURE_INDEX_2D]->map;
      default:
        return 0;
    }
}

__GLtextureObject *__glLookUpTextureObject(__GLcontext *gc, GLenum target, int texUnit )
{
    switch (target) {
      case GL_TEXTURE_1D:
        return gc->texture.boundTextures[texUnit][__GL_TEXTURE_INDEX_1D];
      case GL_TEXTURE_2D:
        return gc->texture.boundTextures[texUnit][__GL_TEXTURE_INDEX_2D];
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

/*ARGSUSED*/
GLvoid __glCopyTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        __GLtexture *tex, GLint lod)
{
    (*gc->procs.copyImage)(gc, spanInfo, GL_TRUE);
}


/*ARGSUSED*/
GLvoid __glReadTexImage(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                        __GLtexture *tex, GLint lod)
{
    (*gc->procs.readImage)(gc, spanInfo);
}

/************************************************************************/

GLvoid __glFreeTexObj(__GLcontext *gc, __GLtextureObject *texobj);
__GLtextureBuffer *__glTexCreateProxyLevel(__GLcontext *gc, __GLtexture *tex,
                                           GLint lod, GLint components,
                                           GLsizei w, GLsizei h, GLsizei d,
                                           GLint border, GLint dim);
__GLtextureBuffer *__glTexCreateLevel(__GLcontext *gc, __GLtexture *tex,
                                      GLint lod, GLint components,
                                      GLsizei w, GLsizei h, GLsizei d,
                                      GLint border, GLint dim);

/*
** Initialize everything in a texture object except the textureMachine.
*/
/* ARGSUSED */
GLvoid __glTexInitTextureObject(__GLcontext *gc, __GLtextureObject *texobj, 
                             GLuint name, GLuint targetIndex)
{
    assert(NULL != texobj);
    texobj->free = __glFreeTexObj;
    texobj->targetIndex = targetIndex;
    texobj->resident = GL_FALSE;
    texobj->texture.map.copyTexImage = __glCopyTexImage;
    texobj->texture.map.readTexImage = __glReadTexImage;
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
void __glShareTextureObjects(__GLcontext *gc, __GLcontext *shareMe)
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

void __glEarlyInitTextureState(__GLcontext *gc)
{
    GLint numTextures, numEnvs;
    GLint i,maxMipMapLevel;
    __GLtextureObject *texobj;

    /* initial routine pick */
    if (gc->texture.initTextureObject == NULL) {
        gc->texture.initTextureObject = __glTexInitTextureObject;
    }
    if (gc->texture.createLevel == NULL) {
        gc->texture.createLevel = __glTexCreateLevel;
    }
    if (gc->texture.createProxyLevel == NULL) {
        gc->texture.createProxyLevel = __glTexCreateProxyLevel;
    }

    /* XXX Override device dependent values */
    gc->constants.numberOfTextures = 4;
    gc->constants.maxTextureSize = 1 << (gc->constants.maxMipMapLevel - 1);

    /* Allocate memory based on number of textures supported */
    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;
    gc->state.texture[gc->texture.currentTexUnit].texture = (__GLperTextureState*)
        (*gc->imports.calloc)(gc, (size_t) numTextures,
                              sizeof(__GLperTextureState));
    gc->texture.texture[gc->texture.currentTexUnit] = (__GLperTextureMachine**)
        (*gc->imports.calloc)(gc, (size_t) numTextures,
                              sizeof(__GLperTextureMachine*));
    gc->state.texture[gc->texture.currentTexUnit].env = (__GLtextureEnvState*)
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
    gc->texture.defaultTextures[gc->texture.currentTexUnit] = (__GLtextureObject *)(*gc->imports.calloc)
                    (gc, numTextures, sizeof(__GLtextureObject));
    assert(NULL != gc->texture.defaultTextures[gc->texture.currentTexUnit]);

    /* allocate the boundTextures array */
    gc->texture.boundTextures[gc->texture.currentTexUnit] = (__GLtextureObject **)(*gc->imports.calloc)
                    (gc, numTextures, sizeof(__GLtextureObject *));
    assert(NULL != gc->texture.boundTextures[gc->texture.currentTexUnit]);

    texobj = gc->texture.defaultTextures[gc->texture.currentTexUnit];
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
        gc->texture.texture[gc->texture.currentTexUnit][i] = &(texobj->texture);
        gc->texture.boundTextures[gc->texture.currentTexUnit][i] = texobj;

        /* Allocate memory based on max mipmap level supported */
        texobj->texture.map.level = (__GLmipMapLevel*)
            (*gc->imports.calloc)(gc, (size_t) maxMipMapLevel,
                                  sizeof(__GLmipMapLevel));
    }
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

}

void __glInitTextureState(__GLcontext *gc)
{
    __GLperTextureState *pts;
    __GLtextureEnvState *tes;
    __GLperTextureMachine **ptm;
    GLint i, level, maxMipMapLevel, numTextures, numEnvs;

    numTextures = gc->constants.numberOfTextures;
    numEnvs = gc->constants.numberOfTextureEnvs;
    maxMipMapLevel = gc->constants.maxMipMapLevel;

    gc->state.current.texture[0].w = __glOne;

    /* Init each texture environment state */
    tes = &gc->state.texture[gc->texture.currentTexUnit].env[0];
    for (i = 0; i < numEnvs; i++, tes++) {
        tes->mode = GL_MODULATE;
    }

    /* Init each textures state */
    pts = &gc->state.texture[gc->texture.currentTexUnit].texture[0];
    ptm = gc->texture.texture[gc->texture.currentTexUnit];
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
    gc->state.texture[gc->texture.currentTexUnit].s.mode = GL_EYE_LINEAR;
    gc->state.texture[gc->texture.currentTexUnit].s.eyePlaneEquation.x = __glOne;
    gc->state.texture[gc->texture.currentTexUnit].s.objectPlaneEquation.x = __glOne;
    gc->state.texture[gc->texture.currentTexUnit].t.mode = GL_EYE_LINEAR;
    gc->state.texture[gc->texture.currentTexUnit].t.eyePlaneEquation.y = __glOne;
    gc->state.texture[gc->texture.currentTexUnit].t.objectPlaneEquation.y = __glOne;
    gc->state.texture[gc->texture.currentTexUnit].r.mode = GL_EYE_LINEAR;
    gc->state.texture[gc->texture.currentTexUnit].q.mode = GL_EYE_LINEAR;

    gc->state.texture[gc->texture.currentTexUnit].scale[0] = __glOne;
    gc->state.texture[gc->texture.currentTexUnit].scale[1] = __glOne;
    gc->state.texture[gc->texture.currentTexUnit].scale[2] = __glOne;
    gc->state.texture[gc->texture.currentTexUnit].scale[3] = __glOne;

    __glInitTextureEnvCache(gc);
}

void __glFreeTextureState(__GLcontext *gc)
{
    __GLperTextureMachine **ptm = gc->texture.texture[gc->texture.currentTexUnit];
    GLint i, level, numTextures, maxLevel;

    /*
    ** Clean up all allocs associated with texture objects.
    */

    maxLevel = gc->constants.maxMipMapLevel;
    numTextures = gc->constants.numberOfTextures;
    for (i = 0; i < numTextures; i++, ptm++) {
        /* Unbind all non-default textures. */
        __glBindTexture(gc, i, 0);
        /* free levels for default textures */
        for (level = 0; level < maxLevel; level++) {
            if (NULL == (*ptm)->map.level[level].buffer) continue;
            assert((*ptm)->map.texobjs.name == 0);
            (*gc->imports.free)(gc, (*ptm)->map.level[level].buffer);
        }
        (*gc->imports.free)(gc, (*ptm)->map.level);
    }
    gc->texture.namesArray->refcount--;
    if (gc->texture.namesArray->refcount == 0) {
        __glNamesFreeArray(gc, gc->texture.namesArray);
    }
    gc->texture.namesArray = NULL;

    (*gc->imports.free)(gc, gc->texture.texture);
    (*gc->imports.free)(gc, gc->texture.boundTextures);
    (*gc->imports.free)(gc, gc->texture.defaultTextures);
    (*gc->imports.free)(gc, gc->state.texture[gc->texture.currentTexUnit].texture);
    (*gc->imports.free)(gc, gc->state.texture[gc->texture.currentTexUnit].env);
    gc->texture.texture[gc->texture.currentTexUnit] = NULL;
    gc->texture.boundTextures[gc->texture.currentTexUnit] = NULL;
    gc->texture.defaultTextures[gc->texture.currentTexUnit] = NULL;
    gc->state.texture[gc->texture.currentTexUnit].texture = NULL;
    gc->state.texture[gc->texture.currentTexUnit].env = NULL;

    __glFreeTextureEnvCache(gc);
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
      case GL_S: tcs = &gc->state.texture[gc->texture.currentTexUnit].s; break;
      case GL_T: tcs = &gc->state.texture[gc->texture.currentTexUnit].t; break;
      case GL_R: tcs = &gc->state.texture[gc->texture.currentTexUnit].r; break;
      case GL_Q: tcs = &gc->state.texture[gc->texture.currentTexUnit].q; break;
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
      case GL_S: tcs = &gc->state.texture[gc->texture.currentTexUnit].s; break;
      case GL_T: tcs = &gc->state.texture[gc->texture.currentTexUnit].t; break;
      case GL_R: tcs = &gc->state.texture[gc->texture.currentTexUnit].r; break;
      case GL_Q: tcs = &gc->state.texture[gc->texture.currentTexUnit].q; break;
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
      case GL_S: tcs = &gc->state.texture[gc->texture.currentTexUnit].s; break;
      case GL_T: tcs = &gc->state.texture[gc->texture.currentTexUnit].t; break;
      case GL_R: tcs = &gc->state.texture[gc->texture.currentTexUnit].r; break;
      case GL_Q: tcs = &gc->state.texture[gc->texture.currentTexUnit].q; break;
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
    __GLtextureObject *pto;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    pts = __glLookUpTextureParams(gc, target, gc->texture.currentTexUnit );
    tex = __glLookUpTexture(gc, target, gc->texture.currentTexUnit );

    if (!pts) {
      bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    pto = __glLookUpTextureObject(gc, target, gc->texture.currentTexUnit );
    if (pto->refcount > 2) {
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
            ptos = __glLookUpTextureTexobjs(gc, target, gc->texture.currentTexUnit );
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
    __GLtextureObject *pto;
    __GL_SETUP_NOT_IN_BEGIN();
    __GL_API_STATE();

    pts = __glLookUpTextureParams(gc, target, gc->texture.currentTexUnit );
    tex = __glLookUpTexture(gc, target, gc->texture.currentTexUnit );

    if (!pts) {
      bad_enum:
        __glSetError(GL_INVALID_ENUM);
        return;
    }
    pto = __glLookUpTextureObject(gc, target, gc->texture.currentTexUnit );
    if (pto->refcount > 2) {
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

            ptos = __glLookUpTextureTexobjs(gc, target, gc->texture.currentTexUnit );
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
    tes = &gc->state.texture[gc->texture.currentTexUnit].env[target];

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
    tes = &gc->state.texture[gc->texture.currentTexUnit].env[target];

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

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelL(__GLmipMapLevel *level, __GLtexture *tex,
                       GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result->luminance = tex->params.borderColor.r;
    } else {
        image = level->buffer + ((row << level->widthLog2) + col);
        result->luminance = __GL_UB_TO_FLOAT(image[0]);
    }
}

/*
** Get a texture element out of the two component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelLA(__GLmipMapLevel *level, __GLtexture *tex,
                       GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result->luminance = tex->params.borderColor.r;
        result->alpha = tex->params.borderColor.a;
    } else {
        image = level->buffer + ((row << level->widthLog2) + col) * 2;
        result->luminance = __GL_UB_TO_FLOAT(image[0]);
        result->alpha = __GL_UB_TO_FLOAT(image[1]);
    }
}

/*
** Get a texture element out of the three component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelRGB(__GLmipMapLevel *level, __GLtexture *tex,
                       GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;
    GLushort texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result->r = tex->params.borderColor.r;
        result->g = tex->params.borderColor.g;
        result->b = tex->params.borderColor.b;
    } else {
        image = level->buffer + ((row << level->widthLog2) + col) * 2;
        texel = *(GLushort *)image;
        result->r = __GL_UB_TO_FLOAT((texel & 0xf800)>>8);
        result->g = __GL_UB_TO_FLOAT((texel & 0x07e0)>>3);
        result->b = __GL_UB_TO_FLOAT((texel & 0x001f)<<3);
    }
}

/*
** Get a texture element out of the four component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelRGBA(__GLmipMapLevel *level, __GLtexture *tex,
                       GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result->r = tex->params.borderColor.r;
        result->g = tex->params.borderColor.g;
        result->b = tex->params.borderColor.b;
        result->alpha = tex->params.borderColor.a;
    } else {
        image = level->buffer + ((row << level->widthLog2) + col) * 4;
        result->r = __GL_UB_TO_FLOAT(image[0]);
        result->g = __GL_UB_TO_FLOAT(image[1]);
        result->b = __GL_UB_TO_FLOAT(image[2]);
        result->alpha = __GL_UB_TO_FLOAT(image[3]);
    }
}

/* ARGSUSED */
void __glExtractTexelA(__GLmipMapLevel *level, __GLtexture *tex,
                       GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result->alpha = tex->params.borderColor.a;
    } else {
        image = level->buffer + ((row << level->widthLog2) + col);
        result->alpha = __GL_UB_TO_FLOAT(image[0]);
    }
}

/* ARGSUSED */
void __glExtractTexelI(__GLmipMapLevel *level, __GLtexture *tex,
                       GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result->intensity = tex->params.borderColor.r;
    } else {
        image = level->buffer + ((row << level->widthLog2) + col);
        result->intensity = __GL_UB_TO_FLOAT(image[0]);
    }
}

/* ARGSUSED */
void __glExtractTexelRGB1555(__GLmipMapLevel *level, __GLtexture *tex,
                       GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result->r = tex->params.borderColor.r;
        result->g = tex->params.borderColor.g;
        result->b = tex->params.borderColor.b;
    } else {
        image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);

        texel = image[0];
        result->r = (__GLfloat) ((texel >> 10) & 0x1F) / 31.0F;
        result->g = (__GLfloat) ((texel >>  5) & 0x1F) / 31.0F;
        result->b = (__GLfloat) ((texel      ) & 0x1F) / 31.0F;
    }
}

/* ARGSUSED */
void __glExtractTexelRGB565(__GLmipMapLevel *level, __GLtexture *tex,
                       GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result->r = tex->params.borderColor.r;
        result->g = tex->params.borderColor.g;
        result->b = tex->params.borderColor.b;
    } else {
        image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
        texel = image[0];
        result->r = (__GLfloat) ((texel >> 11) & 0x1F) / 31.0F;
        result->g = (__GLfloat) ((texel >>  5) & 0x3F) / 63.0F;
        result->b = (__GLfloat) ((texel      ) & 0x1F) / 31.0F;
    }
}

#ifdef GL_EXT_paletted_texture
/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelCI8(__GLmipMapLevel *level, __GLtexture *tex,
                         GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    GLuint index;
    __GLcolorTable *ct;

    ct = &tex->CT;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        index = tex->params.borderColor.r;
    } else {
        image = ((GLubyte*)level->buffer) + ((row << level->widthLog2) + col);
        index = image[0];
    }

    if (tex->texelFormat == GL_COLOR_INDEX) {
        result->r = index;
    } else {
        switch (ct->baseFormat) {
        case GL_LUMINANCE:
            result->luminance = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        case GL_LUMINANCE_ALPHA:
            index <<= 1;
            result->luminance = __GL_UB_TO_FLOAT(ct->table[index]);
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index+1]);
            break;
        case GL_RGB:
            index += index<<1;
            result->r = __GL_UB_TO_FLOAT(ct->table[index]);
            result->g = __GL_UB_TO_FLOAT(ct->table[index+1]);
            result->b = __GL_UB_TO_FLOAT(ct->table[index+2]);
            break;
        case GL_RGBA:
            index <<= 2;
            result->r = __GL_UB_TO_FLOAT(ct->table[index]);
            result->g = __GL_UB_TO_FLOAT(ct->table[index+1]);
            result->b = __GL_UB_TO_FLOAT(ct->table[index+2]);
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index+3]);
            break;
        case GL_ALPHA:
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        case GL_INTENSITY:
            result->intensity = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        default:
            assert(0);
        }
    }
}

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelCI16(__GLmipMapLevel *level, __GLtexture *tex,
                          GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    GLuint index;
    __GLcolorTable *ct;

    ct = &tex->CT;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        index = tex->params.borderColor.r;
    } else {
        image = ((GLushort*)level->buffer) + ((row << level->widthLog2) + col);
        index = image[0];
    }

    if (tex->texelFormat == GL_COLOR_INDEX) {
        result->r = index;
    } else {
        switch (ct->baseFormat) {
        case GL_LUMINANCE:
            result->luminance = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        case GL_LUMINANCE_ALPHA:
            index <<= 1;
            result->luminance = __GL_UB_TO_FLOAT(ct->table[index]);
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index+1]);
            break;
        case GL_RGB:
            index += index<<1;
            result->r = __GL_UB_TO_FLOAT(ct->table[index]);
            result->g = __GL_UB_TO_FLOAT(ct->table[index+1]);
            result->b = __GL_UB_TO_FLOAT(ct->table[index+2]);
            break;
        case GL_RGBA:
            index <<= 2;
            result->r = __GL_UB_TO_FLOAT(ct->table[index]);
            result->g = __GL_UB_TO_FLOAT(ct->table[index+1]);
            result->b = __GL_UB_TO_FLOAT(ct->table[index+2]);
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index+3]);
            break;
        case GL_ALPHA:
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        case GL_INTENSITY:
            result->intensity = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        default:
            assert(0);
        }
    }
}
#endif

/*
** Get a texture element out of the one component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glExtractTexelL_B(__GLmipMapLevel *level, __GLtexture *tex,
                        GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result->luminance = __GL_UB_TO_FLOAT(image[0]);
}

/*
** Get a texture element out of the two component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glExtractTexelLA_B(__GLmipMapLevel *level, __GLtexture *tex,
                        GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 2;
    result->luminance = __GL_UB_TO_FLOAT(image[0]);
    result->alpha = __GL_UB_TO_FLOAT(image[1]);
}

/*
** Get a texture element out of the three component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glExtractTexelRGB_B(__GLmipMapLevel *level, __GLtexture *tex,
                        GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 3;
    result->r = __GL_UB_TO_FLOAT(image[0]);
    result->g = __GL_UB_TO_FLOAT(image[1]);
    result->b = __GL_UB_TO_FLOAT(image[2]);
}

/*
** Get a texture element out of the four component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glExtractTexelRGBA_B(__GLmipMapLevel *level, __GLtexture *tex,
                        GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 4;
    result->r = __GL_UB_TO_FLOAT(image[0]);
    result->g = __GL_UB_TO_FLOAT(image[1]);
    result->b = __GL_UB_TO_FLOAT(image[2]);
    result->alpha = __GL_UB_TO_FLOAT(image[3]);
}

/* ARGSUSED */
void __glExtractTexelA_B(__GLmipMapLevel *level, __GLtexture *tex,
                        GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + ((row << level->widthLog2) + col);
    result->alpha = __GL_UB_TO_FLOAT(image[0]);
}

/* ARGSUSED */
void __glExtractTexelI_B(__GLmipMapLevel *level, __GLtexture *tex,
                        GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + ((row << level->widthLog2) + col);
    result->intensity = __GL_UB_TO_FLOAT(image[0]);
}

/* ARGSUSED */
void __glExtractTexelRGB1555_B(__GLmipMapLevel *level, __GLtexture *tex,
                        GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result->r = (__GLfloat) ((texel >> 10) & 0x1F) / 31.0F;
    result->g = (__GLfloat) ((texel >>  5) & 0x1F) / 31.0F;
    result->b = (__GLfloat) ((texel      ) & 0x1F) / 31.0F;
}

/* ARGSUSED */
void __glExtractTexelRGB565_B(__GLmipMapLevel *level, __GLtexture *tex,
                        GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result->r = (__GLfloat) ((texel >> 11) & 0x1F) / 31.0F;
    result->g = (__GLfloat) ((texel >>  5) & 0x3F) / 63.0F;
    result->b = (__GLfloat) ((texel      ) & 0x1F) / 31.0F;
}

#ifdef GL_EXT_paletted_texture
/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelCI8_B(__GLmipMapLevel *level, __GLtexture *tex,
                           GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    GLuint index;
    __GLcolorTable *ct;

    ct = &tex->CT;

    row++;
    col++;
    image = ((GLubyte*)level->buffer) + ((row << level->widthLog2) + col);
    index = image[0];

    if (tex->texelFormat == GL_COLOR_INDEX) {
        result->r = index;
    } else {
        switch (ct->baseFormat) {
        case GL_LUMINANCE:
            result->luminance = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        case GL_LUMINANCE_ALPHA:
            index <<= 1;
            result->luminance = __GL_UB_TO_FLOAT(ct->table[index]);
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index+1]);
            break;
        case GL_RGB:
            index += index<<1;
            result->r = __GL_UB_TO_FLOAT(ct->table[index]);
            result->g = __GL_UB_TO_FLOAT(ct->table[index+1]);
            result->b = __GL_UB_TO_FLOAT(ct->table[index+2]);
            break;
        case GL_RGBA:
            index <<= 2;
            result->r = __GL_UB_TO_FLOAT(ct->table[index]);
            result->g = __GL_UB_TO_FLOAT(ct->table[index+1]);
            result->b = __GL_UB_TO_FLOAT(ct->table[index+2]);
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index+3]);
            break;
        case GL_ALPHA:
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        case GL_INTENSITY:
            result->intensity = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        default:
            assert(0);
        }
    }
}

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelCI16_B(__GLmipMapLevel *level, __GLtexture *tex,
                            GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    GLuint index;
    __GLcolorTable *ct;

    ct = &tex->CT;

    row++;
    col++;
    image = ((GLushort *)level->buffer) + ((row << level->widthLog2) + col);
    index = image[0];

    if (tex->texelFormat == GL_COLOR_INDEX) {
        result->r = index;
    } else {
        switch (ct->baseFormat) {
        case GL_LUMINANCE:
            result->luminance = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        case GL_LUMINANCE_ALPHA:
            index <<= 1;
            result->luminance = __GL_UB_TO_FLOAT(ct->table[index]);
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index+1]);
            break;
        case GL_RGB:
            index += index<<1;
            result->r = __GL_UB_TO_FLOAT(ct->table[index]);
            result->g = __GL_UB_TO_FLOAT(ct->table[index+1]);
            result->b = __GL_UB_TO_FLOAT(ct->table[index+2]);
            break;
        case GL_RGBA:
            index <<= 2;
            result->r = __GL_UB_TO_FLOAT(ct->table[index]);
            result->g = __GL_UB_TO_FLOAT(ct->table[index+1]);
            result->b = __GL_UB_TO_FLOAT(ct->table[index+2]);
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index+3]);
            break;
        case GL_ALPHA:
            result->alpha = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        case GL_INTENSITY:
            result->intensity = __GL_UB_TO_FLOAT(ct->table[index]);
            break;
        default:
            assert(0);
        }
    }
}
#endif

/************************************************************************/

GLboolean __glIsTextureConsistent(__GLcontext *gc, __GLtexture *tex)
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
    GLint numTexels, texelStorageSize;
    GLint baseWidth= (w-border*2) * (1 << lod);
    GLint baseHeight= (h-border*2) * (1 << lod);
    GLint baseDepth= (d-border*2) * (1 << lod);

    if (baseWidth > gc->constants.maxTextureSize ||
        baseHeight > gc->constants.maxTextureSize)
    {
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
      case GL_LUMINANCE:        case 1:
      case GL_LUMINANCE4:       case GL_LUMINANCE8:
      case GL_LUMINANCE12:      case GL_LUMINANCE16:
        lp->baseFormat = GL_LUMINANCE;
        lp->internalFormat = GL_LUMINANCE;
        lp->luminanceSize = 8;
        texelStorageSize = 1 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelL_B;
        } else {
            lp->extract = __glExtractTexelL;
        }
        break;
      case GL_LUMINANCE_ALPHA:  case 2:
      case GL_LUMINANCE4_ALPHA4:        case GL_LUMINANCE6_ALPHA2:
      case GL_LUMINANCE8_ALPHA8:        case GL_LUMINANCE12_ALPHA4:
      case GL_LUMINANCE12_ALPHA12:      case GL_LUMINANCE16_ALPHA16:
        lp->baseFormat = GL_LUMINANCE_ALPHA;
        lp->internalFormat = GL_LUMINANCE_ALPHA;
        lp->luminanceSize = 8;
        lp->alphaSize = 8;
        texelStorageSize = 2 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelLA_B;
        } else {
            lp->extract = __glExtractTexelLA;
        }
        break;
      case GL_RGB:              case 3:
      case GL_R3_G3_B2:         case GL_RGB4:
      case GL_RGB5:
#ifndef __GL_PC_RAST
        lp->baseFormat = GL_RGB;
        lp->internalFormat = GL_RGB5;
        lp->redSize = 5;
        lp->greenSize = 6;
        lp->blueSize = 5;
        texelStorageSize = 1 * sizeof(GLushort);
        if (border) {
            lp->extract = __glExtractTexelRGB565_B;
        } else {
            lp->extract = __glExtractTexelRGB565;
        }
        break;
#endif
      case GL_RGB8:
      case GL_RGB10:            case GL_RGB12:
      case GL_RGB16:
        lp->baseFormat = GL_RGB;
        lp->internalFormat = GL_RGB;
        lp->redSize = 8;
        lp->greenSize = 8;
        lp->blueSize = 8;
        texelStorageSize = 3 * sizeof(GLubyte);
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
        lp->internalFormat = GL_RGBA;
        lp->redSize = 8;
        lp->greenSize = 8;
        lp->blueSize = 8;
        lp->alphaSize = 8;
        texelStorageSize = 4 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelRGBA_B;
        } else {
            lp->extract = __glExtractTexelRGBA;
        }
        break;
      case GL_ALPHA:
      case GL_ALPHA4:   case GL_ALPHA8:
      case GL_ALPHA12:  case GL_ALPHA16:
        lp->baseFormat = GL_ALPHA;
        lp->internalFormat = GL_ALPHA;
        lp->alphaSize = 8;
        texelStorageSize = 1 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelA_B;
        } else {
            lp->extract = __glExtractTexelA;
        }
        break;
      case GL_INTENSITY:
      case GL_INTENSITY4:       case GL_INTENSITY8:
      case GL_INTENSITY12:      case GL_INTENSITY16:
        lp->baseFormat = GL_INTENSITY;
        lp->internalFormat = GL_INTENSITY;
        lp->intensitySize = 8;
        texelStorageSize = 1 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelI_B;
        } else {
            lp->extract = __glExtractTexelI;
        }
        break;
#ifdef GL_EXT_paletted_texture
    case GL_COLOR_INDEX1_EXT:   case GL_COLOR_INDEX2_EXT:
    case GL_COLOR_INDEX4_EXT:   case GL_COLOR_INDEX8_EXT:
        lp->baseFormat = GL_COLOR_INDEX;
        lp->internalFormat = GL_COLOR_INDEX8_EXT;
        texelStorageSize = 1 * sizeof(GLubyte);
        if (border) {
            lp->extract = __glExtractTexelCI8_B;
        } else {
            lp->extract = __glExtractTexelCI8;
        }
        break;
    case GL_COLOR_INDEX12_EXT:  case GL_COLOR_INDEX16_EXT:
        lp->baseFormat = GL_COLOR_INDEX;
        lp->internalFormat = GL_COLOR_INDEX16_EXT;
        texelStorageSize = 1 * sizeof(GLushort);
        if (border) {
            lp->extract = __glExtractTexelCI16_B;
        } else {
            lp->extract = __glExtractTexelCI16;
        }
        break;
#endif
      default:
        break;
    }

    return (numTexels * texelStorageSize);
}

__GLtextureBuffer *__glTexCreateProxyLevel(__GLcontext *gc, __GLtexture *tex,
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

__GLtextureBuffer *__glTexCreateLevel(__GLcontext *gc, __GLtexture *tex,
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
        lp->width2f = lp->width2;
        lp->height2f = lp->height2;
        lp->depth2f = lp->depth2;
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

/*
** Used to store texture images.
*/
/* ARGSUSED */
void __glInitTexImageStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                          __GLtexture *tex, GLint lod)
{
    __GLmipMapLevel *lp = &tex->level[lod];

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

    switch(lp->internalFormat) {
      case GL_LUMINANCE:
        spanInfo->dstFormat = GL_RED;
        spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 1;
        break;
      case GL_LUMINANCE_ALPHA:
        spanInfo->dstFormat = __GL_RED_ALPHA;
        spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 1;
        break;
      case GL_RGB:
        spanInfo->dstFormat = GL_RGB;
        spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 1;
        break;
      case GL_RGB5:
        spanInfo->dstFormat = GL_RGB;
        spanInfo->dstType = __GL_UNSIGNED_SHORT_5_6_5;
        spanInfo->dstAlignment = 2;
        break;
      case GL_RGBA:
        spanInfo->dstFormat = GL_RGBA;
        spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 1;
        break;
      case GL_ALPHA:
        spanInfo->dstFormat = GL_ALPHA;
        spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 1;
        break;
      case GL_INTENSITY:
        spanInfo->dstFormat = GL_RED;
        spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 1;
        break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX8_EXT:
        spanInfo->dstFormat = GL_COLOR_INDEX;
        spanInfo->dstType = GL_UNSIGNED_BYTE;
        spanInfo->dstAlignment = 1;
        break;
      case GL_COLOR_INDEX16_EXT:
        spanInfo->dstFormat = GL_COLOR_INDEX;
        spanInfo->dstType = GL_UNSIGNED_SHORT;
        spanInfo->dstAlignment = 2;
        break;
#endif
      default:
        assert(0);
        break;
    }
}

/*
** Used to get texture images.
*/
/*ARGSUSED*/
void __glInitTexImageGet(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                         __GLtexture *tex, GLint lod)
{
    __GLmipMapLevel *lp = &tex->level[lod];

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

    switch(lp->internalFormat) {
      case GL_LUMINANCE:
        spanInfo->srcFormat = GL_RED;
        spanInfo->srcType = GL_UNSIGNED_BYTE;
        spanInfo->srcAlignment = 1;
        break;
      case GL_LUMINANCE_ALPHA:
        spanInfo->srcFormat = __GL_RED_ALPHA;
        spanInfo->srcType = GL_UNSIGNED_BYTE;
        spanInfo->srcAlignment = 1;
        break;
      case GL_RGB:
        spanInfo->srcFormat = GL_RGB;
        spanInfo->srcType = GL_UNSIGNED_BYTE;
        spanInfo->srcAlignment = 1;
        break;
      case GL_RGB5:
        spanInfo->srcFormat = GL_RGB;
        spanInfo->srcType = __GL_UNSIGNED_SHORT_5_6_5;
        spanInfo->srcAlignment = 2;
        break;
      case GL_RGBA:
        spanInfo->srcFormat = GL_RGBA;
        spanInfo->srcType = GL_UNSIGNED_BYTE;
        spanInfo->srcAlignment = 1;
        break;
      case GL_ALPHA:
        spanInfo->srcFormat = GL_ALPHA;
        spanInfo->srcType = GL_UNSIGNED_BYTE;
        spanInfo->srcAlignment = 1;
        break;
      case GL_INTENSITY:
        spanInfo->srcFormat = GL_RED;
        spanInfo->srcType = GL_UNSIGNED_BYTE;
        spanInfo->srcAlignment = 1;
        break;
#ifdef GL_EXT_paletted_texture
      case GL_COLOR_INDEX8_EXT:
        spanInfo->srcFormat = GL_COLOR_INDEX;
        spanInfo->srcType = GL_UNSIGNED_BYTE;
        spanInfo->srcAlignment = 1;
        break;
      case GL_COLOR_INDEX16_EXT:
        spanInfo->srcFormat = GL_COLOR_INDEX;
        spanInfo->srcType = GL_UNSIGNED_SHORT;
        spanInfo->srcAlignment = 2;
        break;
#endif
      default:
        assert(0);
        break;
    }
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
    __GLtextureObject *pto;
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
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_1D, gc->texture.currentTexUnit );
    if (pto->refcount > 2) {
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

    /* Mark the bound texture as resident. */
    pto->resident = GL_TRUE;

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
    __GLtextureObject *pto;

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
    if (pto->refcount > 2) {
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

    /* Mark the bound texture object as resident. */
    pto->resident = GL_TRUE;

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
    __GLtextureObject *pto;
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
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_2D, gc->texture.currentTexUnit );
    if (pto->refcount > 2) {
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

    /* Mark the bound texture object as resident. */
    pto->resident = GL_TRUE;

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
    __GLtextureObject *pto;

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
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_2D, gc->texture.currentTexUnit );
    if (pto->refcount > 2) {
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

    /* Mark the bound texture object as resident. */
    pto->resident = GL_TRUE;

    /* Might have just disabled texturing... */
    __GL_DELAY_VALIDATE(gc);
}

/************************************************************************/

/*
** Return texel nearest the s coordinate.  s is converted to u
** implicitly during this step.
*/
/* ARGSUSED */
void __glNearestFilter1(__GLtexture *tex, __GLmipMapLevel *lp,
                        __GLfloat s, __GLfloat t, __GLtexel *result)
{
    GLint col = (GLint) s;
    GLint w2 = lp->width2;

    /* Find texel index */
    if (tex->params.sWrapMode == GL_REPEAT) {
        col &= w2-1;
    } else {
        if (col < 0) col = 0;
        else if (col >= w2) col = w2 - 1;
    }

    /* Lookup texel */
    (*lp->extract)(lp, tex, 0, 0, col, result);
}

/*
** Return texel nearest the s,t coordinates.  s,t are converted to u,v
** implicitly during this step.
*/
/* ARGSUSED */
void __glNearestFilter2(__GLtexture *tex, __GLmipMapLevel *lp,
                        __GLfloat s, __GLfloat t, __GLtexel *result)
{
    GLint row = (GLint) t;
    GLint col = (GLint) s;
    GLint w2 = lp->width2;
    GLint h2 = lp->height2;

    /* Find texel column address */
    if (tex->params.sWrapMode == GL_REPEAT) {
        col &= w2-1;
    } else {
        if (col < 0) col = 0;
        else if (col >= w2) col = w2 - 1;
    }

    /* Find texel row address */
    if (tex->params.tWrapMode == GL_REPEAT) {
        row &= h2-1;
    } else {
        if (row < 0) row = 0;
        else if (row >= h2) row = h2 - 1;
    }

    /* Lookup texel */
    (*lp->extract)(lp, tex, 0, row, col, result);
}

/*
** Return texel which is a linear combination of texels near s.
*/
/* ARGSUSED */
void __glLinearFilter1(__GLtexture *tex, __GLmipMapLevel *lp,
                       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLfloat u, alpha, omalpha;
    GLint col0, col1;
    __GLtexel t0, t1;

    /* Find col0 and col1 */
    u = s;
    if (tex->params.sWrapMode == GL_REPEAT) {
        u -= __glHalf;
        col0 = ((GLint) floor(u)) & (lp->width2 - 1);
        col1 = (col0 + 1) & (lp->width2 - 1);
    } else {
        if (u < __glZero) u = __glZero;
        else if (u > lp->width2) u = lp->width2;
        u -= __glHalf;
        col0 = (GLint) floor(u);
        col1 = col0 + 1;
    }

    /* Compute alpha and beta */
    alpha = __GL_FRAC(u);

    /* Calculate the final texel value as a combination of the two texels */
    (*lp->extract)(lp, tex, 0, 0, col0, &t0);
    (*lp->extract)(lp, tex, 0, 0, col1, &t1);

    omalpha = __glOne - alpha;

    switch (tex->texelFormat) {
      case GL_LUMINANCE_ALPHA:
        result->alpha = omalpha * t0.alpha + alpha * t1.alpha;
        /* FALLTHROUGH */
      case GL_LUMINANCE:
        result->luminance = omalpha * t0.luminance + alpha * t1.luminance;
        break;
      case GL_RGBA:
        result->alpha = omalpha * t0.alpha + alpha * t1.alpha;
        /* FALLTHROUGH */
      case GL_RGB:
        result->r = omalpha * t0.r + alpha * t1.r;
        result->g = omalpha * t0.g + alpha * t1.g;
        result->b = omalpha * t0.b + alpha * t1.b;
        break;
      case GL_ALPHA:
        result->alpha = omalpha * t0.alpha + alpha * t1.alpha;
        break;
      case GL_INTENSITY:
        result->intensity = omalpha * t0.intensity + alpha * t1.intensity;
        break;
    }
}

/*
** Return texel which is a linear combination of texels near s,t.
*/
/* ARGSUSED */
void __glLinearFilter2(__GLtexture *tex, __GLmipMapLevel *lp,
                       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLfloat u, v, alpha, beta, half;
    GLint col0, row0, col1, row1, w2f, h2f;
    __GLtexel t00, t01, t10, t11;
    __GLfloat omalpha, ombeta, m00, m01, m10, m11;

    /* Find col0, col1 */
    w2f = (GLint)lp->width2f;
    u = s;
    half = __glHalf;
    if (tex->params.sWrapMode == GL_REPEAT) {
        GLint w2mask = lp->width2 - 1;
        u -= half;
        col0 = ((GLint) floor(u)) & w2mask;
        col1 = (col0 + 1) & w2mask;
    } else {
        if (u < __glZero) u = __glZero;
        else if (u > w2f) u = w2f;
        u -= half;
        col0 = (GLint) floor(u);
        col1 = col0 + 1;
    }

    /* Find row0, row1 */
    h2f = (GLint)lp->height2f;
    v = t;
    if (tex->params.tWrapMode == GL_REPEAT) {
        GLint h2mask = lp->height2 - 1;
        v -= half;
        row0 = ((GLint) floor(v)) & h2mask;
        row1 = (row0 + 1) & h2mask;
    } else {
        if (v < __glZero) v = __glZero;
        else if (v > h2f) v = h2f;
        v -= half;
        row0 = (GLint) floor(v);
        row1 = row0 + 1;
    }

    /* Compute alpha and beta */
    alpha = __GL_FRAC(u);
    beta = __GL_FRAC(v);

    /* Calculate the final texel value as a combination of the square chosen */
    (*lp->extract)(lp, tex, 0, row0, col0, &t00);
    (*lp->extract)(lp, tex, 0, row0, col1, &t10);
    (*lp->extract)(lp, tex, 0, row1, col0, &t01);
    (*lp->extract)(lp, tex, 0, row1, col1, &t11);

    omalpha = __glOne - alpha;
    ombeta = __glOne - beta;

    m00 = omalpha * ombeta;
    m10 = alpha * ombeta;
    m01 = omalpha * beta;
    m11 = alpha * beta;

    switch (tex->texelFormat) {
      case GL_LUMINANCE_ALPHA:
        result->alpha = m00*t00.alpha + m10*t10.alpha + m01*t01.alpha
            + m11*t11.alpha;
        /* FALLTHROUGH */
      case GL_LUMINANCE:
        result->luminance = m00*t00.luminance + m10*t10.luminance
            + m01*t01.luminance + m11*t11.luminance;
        break;
      case GL_RGBA:
        result->alpha = m00*t00.alpha + m10*t10.alpha + m01*t01.alpha
            + m11*t11.alpha;
        /* FALLTHROUGH */
      case GL_RGB:
        result->r = m00*t00.r + m10*t10.r + m01*t01.r + m11*t11.r;
        result->g = m00*t00.g + m10*t10.g + m01*t01.g + m11*t11.g;
        result->b = m00*t00.b + m10*t10.b + m01*t01.b + m11*t11.b;
        break;
      case GL_ALPHA:
        result->alpha = m00*t00.alpha + m10*t10.alpha + m01*t01.alpha
            + m11*t11.alpha;
        break;
      case GL_INTENSITY:
        result->intensity = m00*t00.intensity + m10*t10.intensity
            + m01*t01.intensity + m11*t11.intensity;
        break;
    }
}

/*
** Linear min/mag filter
*/
void __glLinearFilterUVScaled(__GLtexture *tex, __GLfloat lod,
                      __GLfloat s, __GLfloat t, __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    (*tex->linear)(tex, &tex->level[0], s, t, result);
}

/*
** Nearest min/mag filter
*/
void __glNearestFilterUVScaled(__GLtexture *tex, __GLfloat lod,
                       __GLfloat s, __GLfloat t, __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    (*tex->nearest)(tex, &tex->level[0], s, t, result);
}

/*
** Linear min/mag filter
*/
void __glLinearFilter(__GLtexture *tex, __GLfloat lod,
                      __GLfloat s, __GLfloat t, __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    __GLmipMapLevel *lp = &tex->level[0];
    s *= lp->width2f;
    t *= lp->height2f;
    (*tex->linear)(tex, &tex->level[0], s, t, result);
}

/*
** Nearest min/mag filter
*/
void __glNearestFilter(__GLtexture *tex, __GLfloat lod,
                       __GLfloat s, __GLfloat t, __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    __GLmipMapLevel *lp = &tex->level[0];
    s *= lp->width2f;
    t *= lp->height2f;
    (*tex->nearest)(tex, &tex->level[0], s, t, result);
}

/*
** Apply minification rules to find the texel value.
*/
void __glNMNFilter(__GLtexture *tex, __GLfloat lod,
                   __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLint p, d;

    if (lod <= __glHalf) {
        d = 0;
    } else {
        p = tex->p;
        d = (GLint) (lod + ((__GLfloat)0.49995)); /* NOTE: .5 minus epsilon */
        if (d > p) {
            d = p;
        }
    }
    lp = &tex->level[d];
    s *= lp->width2f;
    t *= lp->height2f;
    (*tex->nearest)(tex, lp, s, t, result);
}

/*
** Apply minification rules to find the texel value.
*/
void __glLMNFilter(__GLtexture *tex, __GLfloat lod,
                   __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLint p, d;

    if (lod <= __glHalf) {
        d = 0;
    } else {
        p = tex->p;
        d = (GLint) (lod + ((__GLfloat) 0.49995)); /* NOTE: .5 minus epsilon */
        if (d > p) {
            d = p;
        }
    }
    lp = &tex->level[d];
    s *= lp->width2f;
    t *= lp->height2f;
    (*tex->linear)(tex, lp, s, t, result);
}

/*
** Apply minification rules to find the texel value.
*/
void __glNMLFilter(__GLtexture *tex, __GLfloat lod,
                   __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLint p, d;
    __GLtexel td, td1;
    __GLfloat f, omf;

    p = tex->p;
    d = ((GLint) lod) + 1;
    if (d > p || d < 0) {
        /* Clamp d to last available mipmap */
        lp = &tex->level[p];
        s *= lp->width2f;
        t *= lp->height2f;
        (*tex->nearest)(tex, lp, s, t, result);
    } else {
        __GLfloat s1, t1;

        lp = &tex->level[d];
        s1 = s * lp->width2f;
        t1 = t * lp->height2f;
        (*tex->nearest)(tex, lp, s1, t1, &td);

        lp = &tex->level[d-1];
        s1 = s * lp->width2f;
        t1 = t * lp->height2f;
        (*tex->nearest)(tex, lp, s1, t1, &td1);

        f = __GL_FRAC(lod);
        omf = __glOne - f;
        switch (tex->texelFormat) {
          case GL_LUMINANCE_ALPHA:
            result->alpha = omf * td1.alpha + f * td.alpha;
            /* FALLTHROUGH */
          case GL_LUMINANCE:
            result->luminance = omf * td1.luminance + f * td.luminance;
            break;
          case GL_RGBA:
            result->alpha = omf * td1.alpha + f * td.alpha;
            /* FALLTHROUGH */
          case GL_RGB:
            result->r = omf * td1.r + f * td.r;
            result->g = omf * td1.g + f * td.g;
            result->b = omf * td1.b + f * td.b;
            break;
          case GL_ALPHA:
            result->alpha = omf * td1.alpha + f * td.alpha;
            break;
          case GL_INTENSITY:
            result->intensity = omf * td1.intensity + f * td.intensity;
            break;
        }
    }
}

/*
** Apply minification rules to find the texel value.
*/
void __glLMLFilter(__GLtexture *tex, __GLfloat lod,
                   __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLint p, d;
    __GLtexel td, td1;
    __GLfloat f, omf;

    p = tex->p;
    d = ((GLint) lod) + 1;
    if (d > p || d < 0) {
        /* Clamp d to last available mipmap */
        lp = &tex->level[p];
        s *= lp->width2f;
        t *= lp->height2f;
        (*tex->linear)(tex, lp, s, t, result);
    } else {
        __GLfloat s1, t1;

        lp = &tex->level[d];
        s1 = s * lp->width2f;
        t1 = t * lp->height2f;
        (*tex->linear)(tex, lp, s1, t1, &td);

        lp = &tex->level[d-1];
        s1 = s * lp->width2f;
        t1 = t * lp->height2f;
        (*tex->linear)(tex, lp, s1, t1, &td1);

        f = __GL_FRAC(lod);
        omf = __glOne - f;
        switch (tex->texelFormat) {
          case GL_LUMINANCE_ALPHA:
            result->alpha = omf * td1.alpha + f * td.alpha;
            /* FALLTHROUGH */
          case GL_LUMINANCE:
            result->luminance = omf * td1.luminance + f * td.luminance;
            break;
          case GL_RGBA:
            result->alpha = omf * td1.alpha + f * td.alpha;
            /* FALLTHROUGH */
          case GL_RGB:
            result->r = omf * td1.r + f * td.r;
            result->g = omf * td1.g + f * td.g;
            result->b = omf * td1.b + f * td.b;
            break;
          case GL_ALPHA:
            result->alpha = omf * td1.alpha + f * td.alpha;
            break;
          case GL_INTENSITY:
            result->intensity = omf * td1.intensity + f * td.intensity;
            break;
        }
    }
}


/***********************************************************************/

/* 1 Component modulate */
void __glTextureModulateL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->luminance * color->r;
    color->g = texel->luminance * color->g;
    color->b = texel->luminance * color->b;
}

/* 2 Component modulate */
void __glTextureModulateLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->luminance * color->r;
    color->g = texel->luminance * color->g;
    color->b = texel->luminance * color->b;
    color->a = texel->alpha * color->a;
}

/* 3 Component modulate */
void __glTextureModulateRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->r * color->r;
    color->g = texel->g * color->g;
    color->b = texel->b * color->b;
}

/* 4 Component modulate */
void __glTextureModulateRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->r * color->r;
    color->g = texel->g * color->g;
    color->b = texel->b * color->b;
    color->a = texel->alpha * color->a;
}

/* Alpha modulate */
void __glTextureModulateA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->a = texel->alpha * color->a;
}

/* Intensity modulate */
void __glTextureModulateI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->intensity * color->r;
    color->g = texel->intensity * color->g;
    color->b = texel->intensity * color->b;
    color->a = texel->intensity * color->a;
}

/* Color Index modulate */
void __glTextureModulateCI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->r = texel->r  * color->r;
}

/***********************************************************************/

/* 3 Component decal */
void __glTextureDecalRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r * gc->frontBuffer.redScale;
    color->g = texel->g * gc->frontBuffer.greenScale;
    color->b = texel->b * gc->frontBuffer.blueScale;
}

/* 4 Component decal */
void __glTextureDecalRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat a = texel->alpha;
    __GLfloat oma = __glOne - a;

    color->r = oma * color->r
        + a * texel->r * gc->frontBuffer.redScale;
    color->g = oma * color->g
        + a * texel->g * gc->frontBuffer.greenScale;
    color->b = oma * color->b
        + a * texel->b * gc->frontBuffer.blueScale;
}

/***********************************************************************/

/* 1 Component blend */
void __glTextureBlendL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat l = texel->luminance;
    __GLfloat oml = __glOne - l;
    __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

    color->r = oml * color->r + l * cc->r;
    color->g = oml * color->g + l * cc->g;
    color->b = oml * color->b + l * cc->b;
}

/* 2 Component blend */
void __glTextureBlendLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat l = texel->luminance;
    __GLfloat oml = __glOne - l;
    __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

    color->r = oml * color->r + l * cc->r;
    color->g = oml * color->g + l * cc->g;
    color->b = oml * color->b + l * cc->b;
    color->a = texel->alpha * color->a;
}

/* 3 Component blend */
void __glTextureBlendRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat r = texel->r;
    __GLfloat g = texel->g;
    __GLfloat b = texel->b;
    __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

    color->r = (__glOne - r) * color->r + r * cc->r;
    color->g = (__glOne - g) * color->g + g * cc->g;
    color->b = (__glOne - b) * color->b + b * cc->b;
}

/* 4 Component blend */
void __glTextureBlendRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat r = texel->r;
    __GLfloat g = texel->g;
    __GLfloat b = texel->b;
    __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

    color->r = (__glOne - r) * color->r + r * cc->r;
    color->g = (__glOne - g) * color->g + g * cc->g;
    color->b = (__glOne - b) * color->b + b * cc->b;
    color->a = texel->alpha * color->a;
}

/* Alpha blend */
void __glTextureBlendA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
#ifdef __GL_LINT
    gc = gc;
#endif
    color->a = texel->alpha * color->a;
}

/* Intensity blend */
void __glTextureBlendI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat i = texel->intensity;
    __GLfloat omi = __glOne - i;
    __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

    color->r = omi * color->r + i * cc->r;
    color->g = omi * color->g + i * cc->g;
    color->b = omi * color->b + i * cc->b;
    color->a = omi * color->a + i * cc->a;
}

/***********************************************************************/

/* 1 Component replace */
void __glTextureReplaceL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->luminance * gc->frontBuffer.redScale;
    color->g = texel->luminance * gc->frontBuffer.greenScale;
    color->b = texel->luminance * gc->frontBuffer.blueScale;
}

/* 2 Component replace */
void __glTextureReplaceLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->luminance * gc->frontBuffer.redScale;
    color->g = texel->luminance * gc->frontBuffer.greenScale;
    color->b = texel->luminance * gc->frontBuffer.blueScale;
    color->a = texel->alpha * gc->frontBuffer.alphaScale;
}

/* 3 Component replace */
void __glTextureReplaceRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r * gc->frontBuffer.redScale;
    color->g = texel->g * gc->frontBuffer.greenScale;
    color->b = texel->b * gc->frontBuffer.blueScale;
}

/* 4 Component replace */
void __glTextureReplaceRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r * gc->frontBuffer.redScale;
    color->g = texel->g * gc->frontBuffer.greenScale;
    color->b = texel->b * gc->frontBuffer.blueScale;
    color->a = texel->alpha * gc->frontBuffer.alphaScale;
}

/* Alpha replace */
void __glTextureReplaceA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->a = texel->alpha * gc->frontBuffer.alphaScale;
}

/* Intensity replace */
void __glTextureReplaceI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->intensity * gc->frontBuffer.redScale;
    color->g = texel->intensity * gc->frontBuffer.greenScale;
    color->b = texel->intensity * gc->frontBuffer.blueScale;
    color->a = texel->intensity * gc->frontBuffer.alphaScale;
}

/* Color Index replace */
void __glTextureReplaceCI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r;
}

/***********************************************************************/

/* Color Index add */
void __glTextureAddCI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = texel->r + color->r;
}

/***********************************************************************/

/* ARGSUSED */
__GLfloat __glNopPolygonRho(__GLcontext *gc, const __GLshade *sh,
                            __GLfloat s, __GLfloat t, __GLfloat winv)
{
    return __glZero;
}

/*
** Compute the "rho" (level of detail) parameter used by the texturing code.
** Instead of fully computing the derivatives compute nearby texture coordinates
** and discover the derivative.  The incoming s & t arguments have not
** been divided by winv yet.
*/
__GLfloat __glComputePolygonRho(__GLcontext *gc, const __GLshade *sh,
                                __GLfloat s, __GLfloat t, __GLfloat qw)
{
    __GLfloat qw0, qw1, p0, p1;
    __GLfloat pupx, pupy, pvpx, pvpy;
    __GLfloat px, py, one;
    const __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];

    /* Compute partial of u with respect to x */
    one = __glOne;
    qw0 = one / (qw - sh->dqwdx);
    qw1 = one / (qw + sh->dqwdx);
    p0 = (s - sh->dsdx) * qw0;
    p1 = (s + sh->dsdx) * qw1;
    pupx = p1 - p0;
    if (!gc->texture.interpUVScaled)
        pupx *= tex->level[0].width2f;

    /* Compute partial of v with respect to x */
    p0 = (t - sh->dtdx) * qw0;
    p1 = (t + sh->dtdx) * qw1;
    pvpx = p1 - p0;
    if (!gc->texture.interpUVScaled)
        pvpx *= tex->level[0].height2f;

    /* Compute partial of u with respect to y */
    qw0 = one / (qw - sh->dqwdy);
    qw1 = one / (qw + sh->dqwdy);
    p0 = (s - sh->dsdy) * qw0;
    p1 = (s + sh->dsdy) * qw1;
    pupy = p1 - p0;
    if (!gc->texture.interpUVScaled)
        pupy *= tex->level[0].width2f;

    /* Compute partial of v with respect to y */
    p0 = (t - sh->dtdy) * qw0;
    p1 = (t + sh->dtdy) * qw1;
    pvpy = p1 - p0;
    if (!gc->texture.interpUVScaled)
        pvpy *= tex->level[0].height2f;

    /* Finally, figure sum of squares */
    px = pupx * pupx + pvpx * pvpx;
    py = pupy * pupy + pvpy * pvpy;

    /* Return largest value as the level of detail */
    if (px > py) {
        return px * ((__GLfloat) 0.25);
    } else {
        return py * ((__GLfloat) 0.25);
    }
}

/* ARGSUSED */
__GLfloat __glNopLineRho(__GLcontext *gc,
                         __GLfloat s, __GLfloat t, __GLfloat qw)
{
    return __glZero;
}

__GLfloat __glComputeLineRho(__GLcontext *gc,
                             __GLfloat s, __GLfloat t, __GLfloat qw)
{
    __GLfloat pspx, pspy, ptpx, ptpy;
    __GLfloat pupx, pupy, pvpx, pvpy;
    __GLfloat temp, pu, pv, p2;
    __GLfloat magnitude, invMag, invMag2;
    __GLfloat dx, dy, invqw;
    const __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    const __GLvertex *v0 = gc->line.options.v0;
    const __GLvertex *v1 = gc->line.options.v1;

    /* Compute the length of the line (its magnitude) */
    dx = v1->window.x - v0->window.x;
    dy = v1->window.y - v0->window.y;
    magnitude = __GL_SQRTF(dx*dx + dy*dy);
    invMag = __glOne / magnitude;
    invMag2 = invMag * invMag;

    invqw = __glOne / qw;

    /* Compute s partials */
    temp = ((v1->texture[0].x - v0->texture[0].x) - s) * invqw;
    pspx = temp * dx * invMag2;
    pspy = temp * dy * invMag2;

    /* Compute t partials */
    temp = ((v1->texture[0].y - v0->texture[0].y) - t) * invqw;
    ptpx = temp * dx * invMag2;
    ptpy = temp * dy * invMag2;

    pupx = pspx * tex->level[0].width2f;
    pupy = pspy * tex->level[0].width2f;
    pvpx = ptpx * tex->level[0].height2f;
    pvpy = ptpy * tex->level[0].height2f;

    /* Now compute rho */
    pu = pupx * dx + pupy * dy;
    pv = pvpx * dx + pvpy * dy;
    p2 = pu * pu + pv * pv;

    return (p2 * invMag2);
}

/************************************************************************/

void __glTextureFragmentUVScale(__GLcontext *gc, __GLcolor *color,
                             __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    __GLmipMapLevel *lp = &tex->level[0];

    (*gc->procs.texture)(gc, color, s*lp->width2f, t*lp->width2f, rho);
}

/*
** Fast texture a fragment assumes that rho is noise - this is true
** when no mipmapping is being done and the min and mag filters are
** the same.
*/
void __glFastTextureFragment(__GLcontext *gc, __GLcolor *color,
                             __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    __GLtexel texel;

#ifdef __GL_LINT
    rho = rho;
#endif
    (*tex->magnify)(tex, __glZero, s, t, &texel);
    (*tex->env)(gc, color, &texel);
}

/*
** Non-mipmapping texturing function.
*/
void __glTextureFragment(__GLcontext *gc, __GLcolor *color,
                         __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    __GLtexel texel;

    if (rho <= tex->c) {
        (*tex->magnify)(tex, __glZero, s, t, &texel);
    } else {
        (*tex->minnify)(tex, __glZero, s, t, &texel);
    }

    /* Now apply texture environment to get final color */
    (*tex->env)(gc, color, &texel);
}

void __glMipMapFragment(__GLcontext *gc, __GLcolor *color,
                        __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    __GLtexel texel;

    /* In the spec c is given in terms of lambda.
    ** Here c is compared to rho (really rho^2) and adjusted accordingly.
    */
    if (rho <= tex->c) {
        /* NOTE: rho is ignored by magnify proc */
        (*tex->magnify)(tex, rho, s, t, &texel);
    } else {
        if (rho) {
#if 0
            __GLfloat oldrho;
#endif
            __GLfloat twotolev;
            GLuint irho, lev;
            /* Convert rho to lambda */
#if 0
            oldrho = __GL_LOGF(rho) * (__GL_M_LN2_INV * 0.5);
#endif

            /* this is an approximation of log base 2 */
            irho = rho;
            lev = 0;
            while( irho >>= 1 ) lev++;
            twotolev = 1<<lev;
            rho = (lev + ( (rho-twotolev) / twotolev ) ) * 0.5;
        } else {
            rho = __glZero;
        }
        (*tex->minnify)(tex, rho, s, t, &texel);
    }

    /* Now apply texture environment to get final color */
    (*tex->env)(gc, color, &texel);
}


/**************************************************************************/

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
    GLboolean didSphereGen;
    __GLmatrix *m;
    GLuint texenables, txu, maxTxu;

    maxTxu = gc->grNTexelFx;
    for( txu = 0; txu < maxTxu; txu++ ) {
        didSphereGen = GL_FALSE;
        texenables = gc->state.enables.texture[txu];

        /* Generate/copy s coordinate */
        if (texenables & __GL_TEXTURE_GEN_S_ENABLE) {
            switch (gc->state.texture[txu].s.mode) {
            case GL_EYE_LINEAR:
                c = &gc->state.texture[txu].s.eyePlaneEquation;
                gen.x = c->x * vx->eye.x + c->y * vx->eye.y
                    + c->z * vx->eye.z + c->w * vx->eye.w;
                break;
            case GL_OBJECT_LINEAR:
                c = &gc->state.texture[txu].s.objectPlaneEquation;
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
            gen.x = vx->texture[txu].x;
        }
        
        /* Generate/copy t coordinate */
        if (texenables & __GL_TEXTURE_GEN_T_ENABLE) {
            switch (gc->state.texture[txu].t.mode) {
            case GL_EYE_LINEAR:
                c = &gc->state.texture[txu].t.eyePlaneEquation;
                gen.y = c->x * vx->eye.x + c->y * vx->eye.y
                    + c->z * vx->eye.z + c->w * vx->eye.w;
                break;
            case GL_OBJECT_LINEAR:
                c = &gc->state.texture[txu].t.objectPlaneEquation;
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
            gen.y = vx->texture[txu].y;
        }
        
        /* Generate/copy r coordinate */
        if (texenables & __GL_TEXTURE_GEN_R_ENABLE) {
            switch (gc->state.texture[txu].r.mode) {
            case GL_EYE_LINEAR:
                c = &gc->state.texture[txu].r.eyePlaneEquation;
                gen.z = c->x * vx->eye.x + c->y * vx->eye.y
                    + c->z * vx->eye.z + c->w * vx->eye.w;
                break;
            case GL_OBJECT_LINEAR:
                c = &gc->state.texture[txu].r.objectPlaneEquation;
                gen.z = c->x * vx->obj.x + c->y * vx->obj.y
                    + c->z * vx->obj.z + c->w * vx->obj.w;
                break;
            }
        } else {
            gen.z = vx->texture[txu].z;
        }

        /* Generate/copy q coordinate */
        if (texenables & __GL_TEXTURE_GEN_Q_ENABLE) {
            switch (gc->state.texture[txu].q.mode) {
            case GL_EYE_LINEAR:
                c = &gc->state.texture[txu].q.eyePlaneEquation;
                gen.w = c->x * vx->eye.x + c->y * vx->eye.y
                    + c->z * vx->eye.z + c->w * vx->eye.w;
                break;
            case GL_OBJECT_LINEAR:
                c = &gc->state.texture[txu].q.objectPlaneEquation;
                gen.w = c->x * vx->obj.x + c->y * vx->obj.y
                    + c->z * vx->obj.z + c->w * vx->obj.w;
                break;
            }
        } else {
            gen.w = vx->texture[txu].w;
        }

        /* Finally, apply texture matrix */
        m = &gc->transform.texture[txu]->matrix;
        (*m->xf4)(&vx->texture[txu], &gen.x, m);
    }
}

void __glCalcEyeLinear(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord gen, *c;
    __GLmatrix *m;
    int maxTxu, txu;
    maxTxu = gc->grNTexelFx;
    for( txu = 0; txu < maxTxu; txu++ ) {
        /* Generate texture coordinates from eye coordinates */
        c = &gc->state.texture[txu].s.eyePlaneEquation;
        gen.x = c->x * vx->eye.x + c->y * vx->eye.y + c->z * vx->eye.z
            + c->w * vx->eye.w;
        c = &gc->state.texture[txu].t.eyePlaneEquation;
        gen.y = c->x * vx->eye.x + c->y * vx->eye.y + c->z * vx->eye.z
            + c->w * vx->eye.w;
        gen.z = vx->texture[txu].z;
        gen.w = vx->texture[txu].w;
        
        /* Finally, apply texture matrix */
        m = &gc->transform.texture[txu]->matrix;
        (*m->xf4)(&vx->texture[txu], &gen.x, m);
    }
}

void __glCalcObjectLinear(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord gen, *c;
    __GLmatrix *m;
    int maxTxu, txu;
    maxTxu = gc->grNTexelFx;
    for( txu = 0; txu < maxTxu; txu++ ) {
        /* Generate texture coordinates from object coordinates */
        c = &gc->state.texture[txu].s.objectPlaneEquation;
        gen.x = c->x * vx->obj.x + c->y * vx->obj.y + c->z * vx->obj.z
            + c->w * vx->obj.w;
        c = &gc->state.texture[txu].t.objectPlaneEquation;
        gen.y = c->x * vx->obj.x + c->y * vx->obj.y + c->z * vx->obj.z
            + c->w * vx->obj.w;
        gen.z = vx->texture[txu].z;
        gen.w = vx->texture[txu].w;
        
        /* Finally, apply texture matrix */
        m = &gc->transform.texture[txu]->matrix;
        (*m->xf4)(&vx->texture[txu], &gen.x, m);
    }
}

void __glCalcSphereMap(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord sphereCoord;
    __GLmatrix *m;

    int maxTxu, txu;
    maxTxu = gc->grNTexelFx;
    for( txu = 0; txu < maxTxu; txu++ ) {
        SphereGen(gc, vx, &sphereCoord);
        sphereCoord.z = vx->texture[txu].z;
        sphereCoord.w = vx->texture[txu].w;

        /* Finally, apply texture matrix */
        m = &gc->transform.texture[txu]->matrix;
        (*m->xf4)(&vx->texture[txu], &sphereCoord.x, m);
    }
}

void __glCalcTexture(__GLcontext *gc, __GLvertex *vx)
{
    __GLcoord copy;
    __GLmatrix *m;

    int maxTxu, txu;
    maxTxu = gc->grNTexelFx;
    for( txu = 0; txu < maxTxu; txu++ ) {
        copy.x = vx->texture[txu].x;
        copy.y = vx->texture[txu].y;
        copy.z = vx->texture[txu].z;
        copy.w = vx->texture[txu].w;

        /* Apply texture matrix */
        m = &gc->transform.texture[txu]->matrix;
        (*m->xf4)(&vx->texture[txu], &copy.x, m);
    }
}

void __glCalcTexturePersp(__GLcontext *gc, __GLvertex *vx)
{
    int maxTxu, txu;

    (*gc->procs.calcTexture2)(gc, vx);

    maxTxu = gc->grNTexelFx;

    if (!(vx->hasAndClipCode & __GL_CLIP_MASK)) {
        for( txu = 0; txu < maxTxu; txu++ ) {
            vx->texture[txu].x *= vx->window.w;
            vx->texture[txu].y *= vx->window.w;
            vx->texture[txu].w *= vx->window.w;
        }
        vx->hasAndClipCode |= __GL_HAS_WINDOW_TEXTURE;
    }
}

void __glFastCalcTexturePersp(__GLcontext *gc, __GLvertex *vx)
{
    int maxTxu, txu;
    
    maxTxu = gc->grNTexelFx;
    
    if ( gc->transform.texture[0]->matrix.matrixType != __GL_MT_IDENTITY ) {
        (*gc->procs.calcTexture2)(gc, vx);
    } else if ( ( maxTxu > 1 ) &&
                ( gc->transform.texture[1]->matrix.matrixType != __GL_MT_IDENTITY ) ) {
        (*gc->procs.calcTexture2)(gc, vx);
    }
    
    if (!(vx->hasAndClipCode & __GL_CLIP_MASK)) {
        for( txu = 0; txu < maxTxu; txu++ ) {
            vx->texture[txu].x *= vx->window.w;
            vx->texture[txu].y *= vx->window.w;
            vx->texture[txu].w *= vx->window.w;
        }
        vx->hasAndClipCode |= __GL_HAS_WINDOW_TEXTURE;
    }
}

void __glCalcTextureUVScale(__GLcontext *gc, __GLvertex *vx)
{
    int maxTxu, txu;
    __GLtexture     *tex;
    __GLmipMapLevel *lp;

    (*gc->procs.calcTexture2)(gc, vx);

    maxTxu = gc->grNTexelFx;
    for( txu = 0; txu < maxTxu; txu++ ) {
        tex = gc->texture.currentTexture[txu];
        if ( tex ) {
            lp = &tex->level[0];
            vx->texture[txu].x *= lp->width2f;
            vx->texture[txu].y *= lp->height2f; 
        }
    }
}

void __glFastCalcTextureUVScale(__GLcontext *gc, __GLvertex *vx)
{
    int maxTxu, txu;
    __GLtexture     *tex;
    __GLmipMapLevel *lp;

    maxTxu = gc->grNTexelFx;

    if ( gc->transform.texture[0]->matrix.matrixType != __GL_MT_IDENTITY ) {
        (*gc->procs.calcTexture2)(gc, vx);
    } else if ( ( maxTxu > 1 ) &&
                ( gc->transform.texture[1]->matrix.matrixType != __GL_MT_IDENTITY ) ) {
        (*gc->procs.calcTexture2)(gc, vx);
    }
        
    for( txu = 0; txu < maxTxu; txu++ ) {
        tex = gc->texture.currentTexture[txu];
        if ( tex ) {
            lp = &tex->level[0];
            vx->texture[txu].x *= lp->width2f;
            vx->texture[txu].y *= lp->height2f; 
        }
    }
}

void __glCalcTexturePerspUVScale(__GLcontext *gc, __GLvertex *vx)
{
    int maxTxu, txu;
    __GLtexture     *tex;
    __GLmipMapLevel *lp;

    maxTxu = gc->grNTexelFx;

    (*gc->procs.calcTexture2)(gc, vx);

    for( txu = 0; txu < maxTxu; txu++ ) {
        tex = gc->texture.currentTexture[txu];
        if ( tex ) {
            lp = &tex->level[0];
            if (!(vx->hasAndClipCode & __GL_CLIP_MASK)) {
                vx->texture[txu].x *= vx->window.w*lp->width2f;
                vx->texture[txu].y *= vx->window.w*lp->height2f;
                vx->texture[txu].w *= vx->window.w;
                vx->hasAndClipCode |= __GL_HAS_WINDOW_TEXTURE;
            } else {
                vx->texture[txu].x *= lp->width2f;
                vx->texture[txu].y *= lp->height2f;
            }
        }
    }
}

void __glFastCalcTexturePerspUVScale(__GLcontext *gc, __GLvertex *vx)
{
    int maxTxu, txu;
    __GLtexture     *tex;
    __GLmipMapLevel *lp;

    maxTxu = gc->grNTexelFx;

    if ( gc->transform.texture[0]->matrix.matrixType != __GL_MT_IDENTITY ) {
        (*gc->procs.calcTexture2)(gc, vx);
    } else if ( ( maxTxu > 1 ) &&
                ( gc->transform.texture[1]->matrix.matrixType != __GL_MT_IDENTITY ) ) {
        (*gc->procs.calcTexture2)(gc, vx);
    }

    for( txu = 0; txu < maxTxu; txu++ ) {
        tex = gc->texture.currentTexture[txu];
        if ( tex ) {
            lp = &tex->level[0];
            if (!(vx->hasAndClipCode & __GL_CLIP_MASK)) {
#               if !__GL_SST_GLIDE_VTX
                vx->texture[txu].x *= vx->window.w*lp->width2f;
                vx->texture[txu].y *= vx->window.w*lp->height2f;
                vx->texture[txu].w *= vx->window.w;
#               endif
                vx->hasAndClipCode |= __GL_HAS_WINDOW_TEXTURE;
            } else {
#               if !__GL_SST_GLIDE_VTX
                vx->texture[txu].x *= lp->width2f;
                vx->texture[txu].y *= lp->height2f;
#               endif
            }
        }
    }
}

/************************************************************************/

static __GLtexture *CheckTexSubImageArgs(__GLcontext *gc, GLenum target,
                                         GLint lod, GLenum format,
                                         GLenum type, GLint dim)
{
    __GLtexture *tex = __glLookUpTexture(gc, target, gc->texture.currentTexUnit );

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
        __glSetError(GL_INVALID_VALUE);
        return 0;
    }

    return tex;
}

void __glInitTexSubImageStore(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                             __GLtexture *tex, GLint lod,
                             GLint xoffset, GLint yoffset, GLint zoffset)
{
    __GLmipMapLevel *lp = &tex->level[lod];

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
    lp = &tex->level[lod];
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
    lp = &tex->level[lod];
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
    lp = &tex->level[lod];
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

    if (tex->level[lod].pixelBuffer)
        __glCookSubTexture(gc, &tex->level[lod], xoffset, yoffset, w, h);
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

    if (tex->level[lod].pixelBuffer)
        __glCookSubTexture(gc, &tex->level[lod], xoffset, yoffset, w, h);
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
GLvoid __glEmptyFreeTexObj(__GLcontext *gc, __GLtextureObject *texobj)
{
}

/*
 * Our implementation-specific texture object free function.
 */
GLvoid __glFreeTexObj(__GLcontext *gc, __GLtextureObject *texobj)
{
    GLint level, maxLevel;

    maxLevel = gc->constants.maxMipMapLevel;
    for (level = 0; level < maxLevel; level++) {
        if (NULL == texobj->texture.map.level[level].buffer) continue;
        assert(texobj->texture.map.texobjs.name != 0);
        (*gc->imports.free)(gc, texobj->texture.map.level[level].buffer);
    }
    (*gc->imports.free)(gc, texobj->texture.map.level);
    (*gc->imports.free)(gc, texobj);
}

/*
 * Generic texture object free function; called from so_names.c
 */
GLvoid __glDisposeTexObj(__GLcontext *gc, void *pData)
{
    __GLtextureObject *texobj = (__GLtextureObject *)pData;

    texobj->refcount--;
    assert(texobj->refcount >= 0);

    if (texobj->refcount == 0) {
        texobj->free(gc, texobj);
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
void __glBindTexture(__GLcontext *gc, GLuint targetIndex, GLuint texture)
{
    __GLtextureObject *texobj;

    assert(NULL != gc->texture.namesArray);

    /*
    ** Retrieve the texture object from the namesArray structure.
    */
    if (texture == 0) {
        texobj = gc->texture.defaultTextures[gc->texture.currentTexUnit] + targetIndex;
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

        pts = &(gc->state.texture[gc->texture.currentTexUnit].texture[targetIndex]);
        ptm = &(gc->texture.texture[gc->texture.currentTexUnit][targetIndex]->map);
        boundTexture = gc->texture.boundTextures[gc->texture.currentTexUnit][targetIndex];

        /* Copy the current stackable state into the bound texture. */
        ptm->params = pts->params;
        ptm->texobjs = pts->texobjs;

        if (boundTexture->texture.map.texobjs.name != 0) {
            /* Unlock the texture that is being unbound.  */
            __glNamesUnlockData(gc, boundTexture);
        }

        /*
        ** Install the new texture into the correct target and save
        ** its pointer so it can be unlocked easily when it is unbound.
        */
        gc->texture.texture[gc->texture.currentTexUnit][targetIndex] = &(texobj->texture);          
        gc->texture.boundTextures[gc->texture.currentTexUnit][targetIndex] = texobj;

        /* Copy the new texture's stackable state into the context state. */
        pts->params = texobj->texture.map.params;
        pts->texobjs = texobj->texture.map.texobjs;
    }

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
}

GLboolean APIENTRY __glim_AreTexturesResident(GLsizei n,
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
        if (!__glIsTextureConsistent(gc, &texobj->texture.map) || 
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

GLboolean APIENTRY __glim_IsTexture(GLuint texture)
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
    __GLtextureObject *pto;
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
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_1D, gc->texture.currentTexUnit);
    if (pto->refcount > 2) {
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

        /* Mark the bound texture object as resident. */
        pto->resident = GL_TRUE;
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
    __GLtextureObject *pto;
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
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_2D, gc->texture.currentTexUnit);
    if (pto->refcount > 2) {
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

        /* Mark the bound texture object as resident. */
        pto->resident = GL_TRUE;
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
    __GLtextureObject *pto;
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
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_1D, gc->texture.currentTexUnit);
    if (pto->refcount > 2) {
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
    __GLtextureObject *pto;
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
    pto = __glLookUpTextureObject(gc, GL_TEXTURE_2D, gc->texture.currentTexUnit);
    if (pto->refcount > 2) {
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

    if (tex->level[level].pixelBuffer)
        __glCookSubTexture(gc, &tex->level[level], xoffset, yoffset,
                           width, height);
}

/************************************************************************/

/*
** Framework for maintaining a cache of lookup tables used
** to compute texture environment function results.
**
** Currently this is only used for RGB5/MODULATE.
*/

void __glInitTextureEnvCache(__GLcontext *gc)
{
    gc->texture.envTableCache = (__GLtextureEnvTable *)
                (*gc->imports.calloc)(gc, 1, sizeof(__GLtextureEnvTable));
}

void __glFreeTextureEnvCache(__GLcontext *gc)
{
    __GLtextureEnvTable *envTable = gc->texture.envTableCache;

    if (envTable) {
        if (envTable->red) (*gc->imports.free)(gc, envTable->red);
        if (envTable->green) (*gc->imports.free)(gc, envTable->green);
        if (envTable->blue) (*gc->imports.free)(gc, envTable->blue);
        if (envTable->alpha) (*gc->imports.free)(gc, envTable->alpha);
        (*gc->imports.free)(gc, envTable);
        gc->texture.envTableCache = NULL;
    }
}

__GLtextureEnvTable *__glCreateTextureEnvTable(GLenum mode, __GLtexture *tex)
{
    __GLcontext *gc = tex->gc;
    __GLtextureEnvTable *envTable = NULL;

    if (mode == GL_MODULATE &&
        tex->level[0].internalFormat == GL_RGB5)
    {
        envTable = gc->texture.envTableCache;

        if (envTable->mode == GL_MODULATE &&
            envTable->internalFormat == GL_RGB5)
        {
            /* already have a RGB5/Modulate table */
            envTable->refcount++;
        } else {
            /* create a new RGB5/Modulate table */
            GLushort *red, *green, *blue;
            int i, j;

            assert(envTable->refcount == 0);
            if (envTable->red) (*gc->imports.free)(gc, envTable->red);
            if (envTable->green) (*gc->imports.free)(gc, envTable->green);
            if (envTable->blue) (*gc->imports.free)(gc, envTable->blue);
            if (envTable->alpha) (*gc->imports.free)(gc, envTable->alpha);

            envTable->refcount++;
            envTable->mode = GL_MODULATE;
            envTable->internalFormat = GL_RGB5;
            envTable->red = (*gc->imports.malloc)(gc, 32*32*sizeof(GLushort));
            envTable->green = (*gc->imports.malloc)(gc, 64*64*sizeof(GLushort));
            envTable->blue = (*gc->imports.malloc)(gc, 32*32*sizeof(GLushort));
            envTable->alpha = NULL;

            red = (GLushort *) envTable->red;
            green = (GLushort *) envTable->green;
            blue = (GLushort *) envTable->blue;

            /*
            ** These are simple pre-computed multiplication tables.
            **
            ** result =
            **   red[(Rf<<5) + Rt] | green[(Gf<<6) + Gt] | blue[(Bf<<5) + Bt];
            */
            for (i=0; i<32; ++i) {
                for (j=0; j<32; ++j) {
                    GLushort val = (GLushort) ((i * j) / 31);

                    red[(i<<5) + j] = val << 11;
                    blue[(i<<5) + j] = val;
                }
            }
            for (i=0; i<64; ++i) {
                for (j=0; j<64; ++j) {
                    GLushort val = (GLushort) ((i * j) / 63);

                    green[(i<<6) + j] = val << 5;
                }
            }
        }
    }
    return envTable;
}

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
            case GL_LUMINANCE:
                l = *srcPtr++;
                r = l;
                g = l;
                b = l;
                a = 0; /* Uses fragment alpha */ 
                break;

            case GL_LUMINANCE_ALPHA:
                l = srcPtr[0];
                r = l;
                g = l;
                b = l;
                a = srcPtr[1];
                srcPtr += 2;
                break;

            case GL_RGB:
                r = srcPtr[0];
                g = srcPtr[1];
                b = srcPtr[2];
                a = 0; /* uses fragment alpha */
                srcPtr += 3;
                break;
            case GL_RGBA:
                r = srcPtr[0];
                g = srcPtr[1];
                b = srcPtr[2];
                a = srcPtr[3];
                srcPtr += 4;
                break;
            case GL_INTENSITY:
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
    __GLmipMapLevel *lp = &gc->texture.currentTexture[gc->texture.currentTexUnit]->level[0];
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

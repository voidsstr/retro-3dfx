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
  
#include <stdlib.h>
#include "context.h"
#include "global.h"
#include "glmath.h"
#include "g_imfncs.h"

#include "geom_og.h"

#if defined(WIN32)
#pragma	optimize("a", on)
#endif

#define __GL_ENVVARS
#ifdef __GL_ENVVARS
static GLboolean disable_og_vcache;
#endif
	   
/**********************************************************************************
**
** Initialise the vertex cache and the array for building primitives up. 
**
**********************************************************************************/

void __glInitVertexCacheState(__GLcontext *gc)
{							   
    __GLvcacheMachine *vc = &gc->vertexCache;

    vc->vertexCacheEnabled = GL_FALSE;
#if 1
    {
	char *env = getenv("__GL_VCACHE");
	if (env)
	    vc->vertexCacheEnabled = (GLboolean) atoi(env);
    }
#if defined(__GL_CODEGEN)
    {
	char *env = getenv("__GL_DISABLE_OG_VCACHE");
	if (env)
	    disable_og_vcache = (GLboolean) atoi(env);
    }
#endif
#endif
}

void __glFreeVertexCacheState(__GLcontext *gc)
{
}

/**********************************************************************************
**
** Process the Vertex Cache. 
**
**********************************************************************************/

static void (*__glProcessCacheProcs[])(__GLcontext *, unsigned int) = {
    __glProcessCachedPoints,
    __glProcessCachedLines,
    __glProcessCachedLineLoop,
    __glProcessCachedLineStrip,
    __glProcessCachedTriangles,
    __glProcessCachedTriangleStrip,
    __glProcessCachedTriangleFan,
    __glProcessCachedQuads,
    __glProcessCachedQuadStrip,
    __glProcessCachedPolygon,
};

void __glProcessVertexCache(__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;
    void (*old_xf)(__GLcoord *res, const __GLfloat *v, const __GLmatrix *m);
    GLuint (*old_clipCheck)(__GLcontext *gc, __GLvertex *vx);
    void (*old_validateVertex)(__GLcontext *gc, __GLvertex *v, GLuint needs);
    void (*old_compile)(__GLcontext *, int, int, int);
    void (*old_compileSilhouette)(__GLcontext *, __GLvertex *, int);
    __GLvertex *old_varrayPtr;
    GLbitfield old_controlWord;
    GLuint vp_size;

    if (!vc->vertexCount) return;

    old_xf = gc->vertexArray.xf;
    old_clipCheck = gc->vertexArray.clipCheck;
    old_validateVertex = gc->vertexArray.validateVertex;

    if (vc->vertexType >= CACHE_VERTEX_TYPE_4D) {
	gc->vertexArray.xf = gc->transform.modelView->mvp.xf4;
	gc->vertexArray.clipCheck = gc->procs.clipCheck4;
	gc->vertexArray.validateVertex = gc->procs.validateVertex4;
	vp_size = 4;
    } else if (vc->vertexType >= CACHE_VERTEX_TYPE_3D) {
	gc->vertexArray.xf = gc->transform.modelView->mvp.xf3;
	gc->vertexArray.clipCheck = gc->procs.clipCheck3;
	gc->vertexArray.validateVertex = gc->procs.validateVertex3;
	vp_size = 3;
    } else {
	gc->vertexArray.xf = gc->transform.modelView->mvp.xf2;
	gc->vertexArray.clipCheck = gc->procs.clipCheck2;
	gc->vertexArray.validateVertex = gc->procs.validateVertex2;
	vp_size = 2;
    }

    old_controlWord = gc->vertexArray.controlWord;
    old_compile = gc->vertexArray.compileElements;
    old_compileSilhouette = gc->vertexArray.compileElementsSilhouette;
    old_varrayPtr = gc->vertexArray.varrayPtr;

    /* No precompile or copy when processing the vertex cache */
    gc->vertexArray.controlWord &= ~VERTARRAY_CW_ALLOW_PRECOMPILE;

    gc->vertexArray.compileElements =
	(vc->vertexCacheState & VC_MATERIAL_VALIDATE) ?
	__glCompileElements_NoCopy :
	vc->compileElements[vp_size-2];

    gc->vertexArray.compileElementsSilhouette = __glCompileElementsSilhouette_NoCopy;
    gc->vertexArray.varrayPtr = gc->vertexCache.vertexCache;

    __GL_LOCK_RENDER_BUFFERS(gc);

    (*__glProcessCacheProcs[vc->cachedPrimitiveType])(gc, flushed);

    __GL_UNLOCK_RENDER_BUFFERS(gc);

    gc->vertexArray.xf = old_xf;
    gc->vertexArray.clipCheck = old_clipCheck;
    gc->vertexArray.validateVertex = old_validateVertex;
    gc->vertexArray.controlWord = old_controlWord;
    gc->vertexArray.compileElements = old_compile;
    gc->vertexArray.compileElementsSilhouette = old_compileSilhouette;
    gc->vertexArray.varrayPtr = old_varrayPtr;
}

/**********************************************************************************
**
** Routines for updating the contents of the vertex cache buffers.  Note that
** the Test to see if the buffer needs to be flushed is done at the beginning
** of the routines.  This is important and is done in this order so that if a
** polygon is constructed which has the same number of vertices as the buffer,
** then the cache will be flushed by an GL_END_VCACHE as opposed to a
** GL_FLUSH_CACHE.  This will ensure that the cache handling code can detect
** the fact that primitive has actually ended. 
**
**********************************************************************************/

#define VX_INIT(dim) {					\
    vx->hasAndClipCode = __GL_HAS_VERTEX_##dim##D;	\
    vx->boundaryEdge = gc->state.current.edgeTag;	\
}

void APIENTRY __glim_VertexCache2fv(const GLfloat v[2])
{
    __GL_SETUP();
    __GLvertex 	*vx;
    __GLvcacheMachine *vc = &gc->vertexCache;

    /* If the array is full, process the lot */
    if (vc->vertexCount == MAX_VERTEX_CACHE)
	__glProcessVertexCache(gc, GL_FLUSH_VCACHE);

    /* Store Vertex in Array */	 
    vx = vc->vertexCache + vc->vertexCount;
    assert((vc->vertexIncrement == 0) || (vc->vertexIncrement == 1));
    vc->vertexCount += vc->vertexIncrement;
    vc->vertexType |= CACHE_VERTEX_TYPE_2D;
	
    VX_INIT(2);
    vx->obj.x = v[0];
    vx->obj.y = v[1];
    vx->obj.z = __glZero;
    vx->obj.w = __glOne;

    (*vc->save)(gc, vx);
}

void APIENTRY __glim_VertexCache3fv(const GLfloat v[3])
{
    __GL_SETUP();
    __GLvertex 	*vx;
    __GLvcacheMachine *vc = &gc->vertexCache;

    /* If the array is full, process the lot */
    if (vc->vertexCount == MAX_VERTEX_CACHE)
	__glProcessVertexCache(gc, GL_FLUSH_VCACHE);

    /* Store Vertex in Array */	 
    vx = vc->vertexCache + vc->vertexCount;
    assert((vc->vertexIncrement == 0) || (vc->vertexIncrement == 1));
    vc->vertexCount += vc->vertexIncrement;
    vc->vertexType |= CACHE_VERTEX_TYPE_3D;

    VX_INIT(3);
    vx->obj.x = v[0];
    vx->obj.y = v[1];
    vx->obj.z = v[2];
    vx->obj.w = __glOne;

    (*vc->save)(gc, vx);
}

void APIENTRY __glim_VertexCache4fv(const GLfloat v[4])
{
    __GL_SETUP();
    __GLvertex 	*vx;
    __GLvcacheMachine *vc = &gc->vertexCache;

    if (vc->vertexCount == MAX_VERTEX_CACHE)
	__glProcessVertexCache(gc, GL_FLUSH_VCACHE);

    vc->vertexType |= CACHE_VERTEX_TYPE_4D;
    vx = vc->vertexCache + vc->vertexCount;
    assert((vc->vertexIncrement == 0) || (vc->vertexIncrement == 1));
    vc->vertexCount += vc->vertexIncrement;

    VX_INIT(4);
    vx->obj.x = v[0];
    vx->obj.y = v[1];
    vx->obj.z = v[2];
    vx->obj.w = v[3];

    (*vc->save)(gc, vx);
}

/**********************************************************************************
**
** Vertex save procs 
**
**********************************************************************************/

static void __fastcall __glVCSaveN(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x,y,z;

    x = gc->state.current.normal.x;
    y = gc->state.current.normal.y;
    z = gc->state.current.normal.z;
    vx->normal.x = x;
    vx->normal.y = y;
    vx->normal.z = z;
}

static void __fastcall __glVCSaveCI(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat index;

    index = gc->state.current.userColorIndex;
    vx->colors[__GL_FRONTFACE].r = index;
}

static void __fastcall __glVCSaveNCI(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x,y,z;
    __GLfloat index;

    x = gc->state.current.normal.x;
    y = gc->state.current.normal.y;
    z = gc->state.current.normal.z;
    vx->normal.x = x;
    vx->normal.y = y;
    vx->normal.z = z;

    index = gc->state.current.userColorIndex;
    vx->colors[__GL_FRONTFACE].r = index;
}

static void __fastcall __glVCSaveC(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat r,g,b,a;

    r = gc->state.current.color.r;
    g = gc->state.current.color.g;
    b = gc->state.current.color.b;
    a = gc->state.current.color.a;
    vx->colors[__GL_FRONTFACE].r = r;
    vx->colors[__GL_FRONTFACE].g = g;
    vx->colors[__GL_FRONTFACE].b = b;
    vx->colors[__GL_FRONTFACE].a = a;
}

static void __fastcall __glVCSaveNC(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x,y,z;
    __GLfloat r,g,b,a;

    x = gc->state.current.normal.x;
    y = gc->state.current.normal.y;
    z = gc->state.current.normal.z;
    vx->normal.x = x;
    vx->normal.y = y;
    vx->normal.z = z;

    r = gc->state.current.color.r;
    g = gc->state.current.color.g;
    b = gc->state.current.color.b;
    a = gc->state.current.color.a;
    vx->colors[__GL_FRONTFACE].r = r;
    vx->colors[__GL_FRONTFACE].g = g;
    vx->colors[__GL_FRONTFACE].b = b;
    vx->colors[__GL_FRONTFACE].a = a;
}

static void __fastcall __glVCSaveT(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x,y,z,w;

    x = gc->state.current.texture.x;
    y = gc->state.current.texture.y;
    z = gc->state.current.texture.z;
    w = gc->state.current.texture.w;
    vx->texture.x = x;
    vx->texture.y = y;
    vx->texture.z = z;
    vx->texture.w = w;
}

static void __fastcall __glVCSaveCT(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat r,g,b,a;
    __GLfloat x,y,z,w;

    r = gc->state.current.color.r;
    g = gc->state.current.color.g;
    b = gc->state.current.color.b;
    a = gc->state.current.color.a;
    vx->colors[__GL_FRONTFACE].r = r;
    vx->colors[__GL_FRONTFACE].g = g;
    vx->colors[__GL_FRONTFACE].b = b;
    vx->colors[__GL_FRONTFACE].a = a;
    x = gc->state.current.texture.x;
    y = gc->state.current.texture.y;
    z = gc->state.current.texture.z;
    w = gc->state.current.texture.w;
    vx->texture.x = x;
    vx->texture.y = y;
    vx->texture.z = z;
    vx->texture.w = w;
}

static void __fastcall __glVCSaveNT(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x,y,z,w;

    x = gc->state.current.normal.x;
    y = gc->state.current.normal.y;
    z = gc->state.current.normal.z;
    vx->normal.x = x;
    vx->normal.y = y;
    vx->normal.z = z;
    x = gc->state.current.texture.x;
    y = gc->state.current.texture.y;
    z = gc->state.current.texture.z;
    w = gc->state.current.texture.w;
    vx->texture.x = x;
    vx->texture.y = y;
    vx->texture.z = z;
    vx->texture.w = w;
}

static void __fastcall __glVCSaveCIAll(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x,y,z,w;
    __GLfloat index;

    index = gc->state.current.userColorIndex;
    x = gc->state.current.normal.x;
    y = gc->state.current.normal.y;
    z = gc->state.current.normal.z;
    vx->colors[__GL_FRONTFACE].r = index;
    vx->normal.x = x;
    vx->normal.y = y;
    vx->normal.z = z;
    x = gc->state.current.texture.x;
    y = gc->state.current.texture.y;
    z = gc->state.current.texture.z;
    w = gc->state.current.texture.w;
    vx->texture.x = x;
    vx->texture.y = y;
    vx->texture.z = z;
    vx->texture.w = w;
}

static void __fastcall __glVCSaveCAll(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat r,g,b,a;
    __GLfloat x,y,z,w;

    x = gc->state.current.normal.x;
    y = gc->state.current.normal.y;
    z = gc->state.current.normal.z;
    r = gc->state.current.color.r;
    g = gc->state.current.color.g;
    b = gc->state.current.color.b;
    a = gc->state.current.color.a;
    vx->colors[__GL_FRONTFACE].r = r;
    vx->colors[__GL_FRONTFACE].g = g;
    vx->colors[__GL_FRONTFACE].b = b;
    vx->colors[__GL_FRONTFACE].a = a;
    vx->normal.x = x;
    vx->normal.y = y;
    vx->normal.z = z;
    x = gc->state.current.texture.x;
    y = gc->state.current.texture.y;
    z = gc->state.current.texture.z;
    w = gc->state.current.texture.w;
    vx->texture.x = x;
    vx->texture.y = y;
    vx->texture.z = z;
    vx->texture.w = w;
}

static void __fastcall __glVCSaveCIT(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat index;
    __GLfloat x,y,z,w;

    index = gc->state.current.userColorIndex;
    vx->colors[__GL_FRONTFACE].r = index;
    x = gc->state.current.texture.x;
    y = gc->state.current.texture.y;
    z = gc->state.current.texture.z;
    w = gc->state.current.texture.w;
    vx->texture.x = x;
    vx->texture.y = y;
    vx->texture.z = z;
    vx->texture.w = w;
}

/************************************************************************/


#define __NORMAL 1
#define __COLOR 2
#define __TEX 4

static void (__fastcall *CISaveProcs[8])(__GLcontext*, __GLvertex*) = {
    0, 			/* none */
    __glVCSaveN,	/* __NORMAL */
    __glVCSaveCI,	/* __COLOR */
    __glVCSaveNCI, 	/* __NORMAL | __COLOR */

    __glVCSaveT, 	/* __TEX */
    __glVCSaveNT, 	/* __NORMAL | __TEX */

    __glVCSaveCIT, 	/* __COLOR | __TEX */
    __glVCSaveCIAll, 	/* __NORMAL | __COLOR | __TEX */
};

static void (__fastcall *RGBSaveProcs[8])(__GLcontext*, __GLvertex*) = {
    0, 			/* none */
    __glVCSaveN, 	/* __NORMAL */

    __glVCSaveC, 	/* __COLOR */
    __glVCSaveNC, 	/* __NORMAL | __COLOR */

    __glVCSaveT, 	/* __TEX */
    __glVCSaveNT, 	/* __NORMAL | __TEX */

    __glVCSaveCT, 	/* __COLOR | __TEX */
    __glVCSaveCAll, 	/* __NORMAL | __COLOR | __TEX */
};

/******************************************************************************
**
** VertexCache Begin / End procs
**
******************************************************************************/

void APIENTRY __glim_VertexCacheBegin(GLenum mode)
{
    __GL_SETUP();
    GLuint beginMode;

    beginMode = __gl_beginMode;
    if (beginMode != __GL_NOT_IN_BEGIN) {
	if (beginMode == __GL_NEED_VALIDATE) {
	    (*gc->procs.validate)(gc);
	    __gl_beginMode = __GL_NOT_IN_BEGIN;
	    glBegin(mode);
	    return;
	} else {
	    __glSetError(GL_INVALID_OPERATION);
	    return;
	}
    }

    if ((GLuint)mode > GL_POLYGON) {
	__glSetError(GL_INVALID_ENUM);
	return;
    }

    __gl_beginMode = __GL_IN_BEGIN;
    __GL_API_BGN_RENDER();

#if defined(__GL_VCACHE_ALLOW_CAT_PRIMITIVE)
    if (gc->vertexCache.vertexCacheState & VC_CAT_PRIMITIVE) {
	if (gc->vertexCache.cachedPrimitiveType == mode) {
	    return;
	} else {
	    /*
	    ** Change in primitive type...flush the current
	    ** buffer out and then continue afresh.
	    */
	    __glProcessVertexCache(gc, GL_END_VCACHE);
	    gc->vertexCache.vertexCacheState &= ~(VC_CAT_PRIMITIVE|VC_MATERIAL_VALIDATE;
	}
    }
#else
    assert(0 == gc->vertexCache.vertexCount);
#endif

#if defined(__GL_VCACHE_ALLOW_CAT_PRIMITIVE)
    /*
    ** See if the primitive being rendered is one suitable 
    ** for primitive concatenation.
    */
    if (mode == GL_POINTS    ||
	mode == GL_LINES     ||
	mode == GL_TRIANGLES ||
	mode == GL_QUADS)
    {
	gc->vertexCache.vertexCacheState |= VC_CAT_PRIMITIVE;
    }
#endif

    gc->vertexCache.cachedPrimitiveType = mode;
    gc->vertexCache.vertexType = 0;
    gc->vertexCache.vertexIncrement = 1;
    gc->vertexCache.colorChanged = GL_FALSE;
    /* force vertex array recompile after vertex cache */
    gc->vertexArray.controlWord |= VERTARRAY_CW_NEEDS_COMPILE;
    gc->vertexArray.continuation = GL_FALSE;
}

void APIENTRY __glim_VertexCacheEnd(void)
{
    __GL_SETUP();
    GLuint beginMode;

    beginMode = __gl_beginMode;
    if (beginMode == __GL_NOT_IN_BEGIN || beginMode == __GL_NEED_VALIDATE) {
	__glSetError(GL_INVALID_OPERATION);
	return;
    }
    assert(beginMode == __GL_IN_BEGIN);
    __GL_API_END_RENDER();

#if defined(__GL_VCACHE_ALLOW_CAT_PRIMITIVE)
    /*
    ** If we are rendering any of the independent primitives
    ** then don't flush...thereby concatenating successive 
    ** primitives.
    */
    if (gc->vertexCache.vertexCacheState & VC_CAT_PRIMITIVE) {
	/* Fix up the vertex count according to the primitive type */
	switch (gc->vertexCache.cachedPrimitiveType) {
	case GL_LINES:
	    gc->vertexCache.vertexCount -= gc->vertexCache.vertexCount % 2;
	    break;
	case GL_TRIANGLES:
	    gc->vertexCache.vertexCount -= gc->vertexCache.vertexCount % 3;
	    break;
	case GL_QUADS:
	    gc->vertexCache.vertexCount -= gc->vertexCache.vertexCount % 4;
	    break;
	}
    } else {
	__glProcessVertexCache(gc, GL_END_VCACHE);
    }
#else
    __glProcessVertexCache(gc, GL_END_VCACHE);
#endif

    gc->vertexCache.vertexIncrement = 0;

    gc->vertexArray.continuation = GL_FALSE;
    __gl_beginMode = __GL_NOT_IN_BEGIN;
}

/**********************************************************************************
**
** Validate Cache Vertices. 
**
**********************************************************************************/

#if defined(__GL_CODEGEN)
static void __glCompileElementsVcache2Wrapper(__GLcontext *gc, GLint offset,
					      GLint first, GLsizei count)
{
    __GLVertArrayMachine *va = &gc->vertexArray;
    __GLvcacheMachine *vc = &gc->vertexCache;
    vc->compileElements[0] = GenerateCompile(gc, 2, __GL_GEOM_OG_NOCOPY);
    va->compileElements = vc->compileElements[0];
    (*va->compileElements)(gc, offset, first, count);
}

static void __glCompileElementsVcache3Wrapper(__GLcontext *gc, GLint offset,
					      GLint first, GLsizei count)
{
    __GLVertArrayMachine *va = &gc->vertexArray;
    __GLvcacheMachine *vc = &gc->vertexCache;
    vc->compileElements[1] = GenerateCompile(gc, 3, __GL_GEOM_OG_NOCOPY);
    va->compileElements = vc->compileElements[1];
    (*va->compileElements)(gc, offset, first, count);
}

static void __glCompileElementsVcache4Wrapper(__GLcontext *gc, GLint offset,
					      GLint first, GLsizei count)
{
    __GLVertArrayMachine *va = &gc->vertexArray;
    __GLvcacheMachine *vc = &gc->vertexCache;
    vc->compileElements[2] = GenerateCompile(gc, 4, __GL_GEOM_OG_NOCOPY);
    va->compileElements = vc->compileElements[2];
    (*va->compileElements)(gc, offset, first, count);
}
#endif

void __glPickVcacheProcs (__GLcontext *gc)
{
    __GLvcacheMachine *vc = &gc->vertexCache;
    GLuint enables = gc->state.enables.general;
    GLint ix;

    gc->dispatchState->dispatch.Begin = __glim_VertexCacheBegin;
    gc->dispatchState->dispatch.End = __glim_VertexCacheEnd;
    gc->dispatchState->dispatch.Vertex2fv = __glim_VertexCache2fv;
    gc->dispatchState->dispatch.Vertex3fv = __glim_VertexCache3fv;
    gc->dispatchState->dispatch.Vertex4fv = __glim_VertexCache4fv;
    gc->procs.matValidate = __glMatValidateVcache;

#if defined(__GL_CODEGEN)
    if (gc->renderMode == GL_RENDER) {
#ifdef __GL_ENVVARS
	if (disable_og_vcache) {
	    vc->compileElements[0] = __glCompileElements_NoCopy;
	    vc->compileElements[1] = __glCompileElements_NoCopy;
	    vc->compileElements[2] = __glCompileElements_NoCopy;
	} else {
	    vc->compileElements[0] = __glCompileElementsVcache2Wrapper;
	    vc->compileElements[1] = __glCompileElementsVcache3Wrapper;
	    vc->compileElements[2] = __glCompileElementsVcache4Wrapper;
	}
#else
	vc->compileElements[0] = __glCompileElementsVcache2Wrapper;
	vc->compileElements[1] = __glCompileElementsVcache3Wrapper;
	vc->compileElements[2] = __glCompileElementsVcache4Wrapper;
#endif
    } else {
	vc->compileElements[0] = __glCompileElements_NoCopy;
	vc->compileElements[1] = __glCompileElements_NoCopy;
	vc->compileElements[2] = __glCompileElements_NoCopy;
    }
#else
    vc->compileElements[0] = __glCompileElements_NoCopy;
    vc->compileElements[1] = __glCompileElements_NoCopy;
    vc->compileElements[2] = __glCompileElements_NoCopy;
#endif

    if (gc->renderMode == GL_FEEDBACK) {
	ix = __NORMAL | __COLOR | __TEX;
    } else {
	ix = 0;
	if (enables & __GL_LIGHTING_ENABLE) {
	    ix |= __NORMAL;
	} else {
	    ix |= __COLOR;
	}
	if (enables & __GL_CULL_VERTEX_ENABLE) {
	    ix |= __NORMAL;
	}
	if (gc->texture.textureEnabled) {
	    ix |= __TEX;
	    if (gc->state.enables.texture & __GL_TEXTURE_GEN_S_ENABLE) {
		if (gc->state.texture.s.mode == GL_SPHERE_MAP) {
		    ix |= __NORMAL;
		}
	    }
	    if (gc->state.enables.texture & __GL_TEXTURE_GEN_T_ENABLE) {
		if (gc->state.texture.t.mode == GL_SPHERE_MAP) {
		    ix |= __NORMAL;
		}
	    }
	}
    }

    if (gc->modes.rgbMode) {
	vc->save = RGBSaveProcs[ix];
    } else {
	vc->save = CISaveProcs[ix];
    }
    assert(vc->save != 0);
}


/*
** Validate vertexes in the vcache
*/
void __glMatValidateVcache(__GLcontext *gc)
{
    __GLvcacheMachine *vc = &gc->vertexCache;
    __GLvertex *v, *last;
    GLuint needs;

    needs = gc->vertex.materialNeeds;

    v = vc->vertexCache + vc->vertexStart;
    last = vc->vertexCache + vc->vertexCount;
    for (; v < last; v++) {
	if (~v->hasAndClipCode & needs) DO_VALIDATE(gc, v, needs);
    }

    vc->vertexStart = vc->vertexCount;
    vc->vertexCacheState |= VC_MATERIAL_VALIDATE;
}

#if defined(WIN32)
#pragma	optimize("", on)
#endif

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
/************************************************************************
 *                                                                      *
 *                               GLiNT                                  *
 *                                                                      *
 *                                                                      *
 *                   Copyright (C) 1994  3Dlabs Ltd.                    *
 *                                                                      *
 *                                                                      *
 * This software and its associated documentation contains proprietary, *
 * confidential and trade secret information of 3Dlabs Ltd. and except  *
 * as provided by written agreement with 3Dlabs Ltd.                    *
 *                                                                      *
 * a) no part may be disclosed, distributed, reproduced, transmitted,   *
 *    transcribed, stored in a retieval system, adapted or translated   *
 *    in any form or by any means electronic, mechanical, magnetic,     *
 *    optical, chemical, manual or otherwise,                           *
 *                                                                      *
 *    and                                                               *
 *                                                                      *
 * b) the recipient is not entitled to discover through reverse         *
 *    engineering or reverse compiling or other such techniques or      *
 *    processes the trade secrets contained therein or in the           *
 *    documentation.                                                    *
 *                                                                      *
 ************************************************************************/
  
#include <stdlib.h>
#include "context.h"
#include "global.h"
#include "glmath.h"
#include "g_imfncs.h"

#if defined(WIN32)
#pragma	optimize("a", on)
#endif

/**********************************************************************************
**
** Process triangle strips. All of the cached polygon routines use the same
** mechanisms.  The differences between them are how they handle flushing of
** the cached buffer and building the generic triangle list.
**
**********************************************************************************/

void __glProcessCachedTriangleStrip (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;

    if (vc->vertexCount >= 3) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);

	drawVertexes = gc->vertexArray.drawVertexes[GL_TRIANGLE_STRIP];
	assert(NULL != drawVertexes);
	(*drawVertexes)(gc, 0, vc->vertexCount, NULL);

	/*
	** We were flushed, so we have to copy the last two vertices in the 
	** list over and set the start and count values appropriately.
	*/
	if (flushed == GL_FLUSH_VCACHE) {
	    __GLvertex *src = vc->vertexCache + vc->vertexCount - 2;
	    __GLvertex *dst = vc->vertexCache;
	    dst[0] = src[0]; dst[0].color = &dst[0].colors[__GL_FRONTFACE];
	    dst[1] = src[1]; dst[1].color = &dst[1].colors[__GL_FRONTFACE];
	    vc->vertexStart = vc->vertexCount = 2;
	    gc->vertexArray.continuation = GL_TRUE;
	    return;
	}
    }

    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}


/**********************************************************************************
**
** Process Quad strips, see the triangle mesh code above for more comments.
** The only real main difference between the two is that the triangle list
** has to be built up in a different way.
**
**********************************************************************************/

void __glProcessCachedQuadStrip (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;

    if (vc->vertexCount >= 4) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);

	drawVertexes = gc->vertexArray.drawVertexes[GL_QUAD_STRIP];
	assert(NULL != drawVertexes);
	(*drawVertexes)(gc, 0, vc->vertexCount, NULL);

	/*
	** We were flushed, so we have to copy the last two vertices in the 
	** list over and set the start and count values appropriately.
	*/
	if (flushed == GL_FLUSH_VCACHE) {
	    __GLvertex *src = vc->vertexCache + vc->vertexCount - 2;
	    __GLvertex *dst = vc->vertexCache;
	    dst[0] = src[0]; dst[0].color = &dst[0].colors[__GL_FRONTFACE];
	    dst[1] = src[1]; dst[1].color = &dst[1].colors[__GL_FRONTFACE];
	    vc->vertexStart = vc->vertexCount = 2;
	    gc->vertexArray.continuation = GL_TRUE;
	    return;
	}
    }

    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}


/**********************************************************************************
**
** Polygons.
**
**********************************************************************************/

void __glProcessCachedPolygon (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;
    GLboolean alreadyInPolygon = vc->vertexCacheState & VC_IN_POLYGON;

    if (vc->vertexCount >= 3) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);

	if (flushed == GL_FLUSH_VCACHE || alreadyInPolygon) {
	    /*
	    ** There is a partial polygon in the cache and this is either
	    ** the first, last, or some middle piece, so we must set edge
	    ** flags appropriately.
	    */
	    if (!alreadyInPolygon) {
		/* first piece of polygon */
		vc->vertexCacheState |= VC_IN_POLYGON;
		vc->vertexCache[vc->vertexCount - 1].boundaryEdge = GL_FALSE;

	    } else if (flushed == GL_END_VCACHE) {
		/* last piece of polygon */
		vc->vertexCacheState &= ~VC_IN_POLYGON;

	    } else {
		/* some middle piece of polygon */
		vc->vertexCache[vc->vertexCount - 1].boundaryEdge = GL_FALSE;
	    }
	}

	drawVertexes = gc->vertexArray.drawVertexes[GL_POLYGON];
	assert(NULL != drawVertexes);

	(*drawVertexes)(gc, 0, vc->vertexCount, NULL);

	/*
	** We have been flushed, so copy the last vertex into the second
	** vertex of the buffer. The first vertex remains intact.
	*/
	if (flushed == GL_FLUSH_VCACHE) {
	    __GLvertex *src = vc->vertexCache + vc->vertexCount - 1;
	    __GLvertex *dst = vc->vertexCache + 1;
	    dst[0] = src[0]; dst[0].color = &dst[0].colors[__GL_FRONTFACE];
	    vc->vertexStart = vc->vertexCount = 2;
	    gc->vertexArray.continuation = GL_TRUE;

	    vc->vertexCache[0].boundaryEdge = GL_FALSE;
	    vc->vertexCache[1].boundaryEdge = gc->state.current.edgeTag;
	    return;
	}
    }

    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}


/**********************************************************************************
**
** Triangle Fans.
**
**********************************************************************************/

void __glProcessCachedTriangleFan (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;

    if (vc->vertexCount >= 3) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);

	drawVertexes = gc->vertexArray.drawVertexes[GL_TRIANGLE_FAN];
	assert(NULL != drawVertexes);
	(*drawVertexes)(gc, 0, vc->vertexCount, NULL);

	/*
	** We have been flushed, so copy the last vertex into the second
	** vertex of the buffer. The first vertex remains intact.
	*/
	if (flushed == GL_FLUSH_VCACHE) {
	    __GLvertex *src = vc->vertexCache + vc->vertexCount - 1;
	    __GLvertex *dst = vc->vertexCache + 1;
	    dst[0] = src[0]; dst[0].color = &dst[0].colors[__GL_FRONTFACE];
	    vc->vertexStart = vc->vertexCount = 2;
	    gc->vertexArray.continuation = GL_TRUE;
	    return;
	}
    }

    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}


/**********************************************************************************
**
** Independent Triangles.
**
**********************************************************************************/

//#define __GL_CODEGEN_PMON
#ifdef __GL_CODEGEN_PMON
#include "pmonstat.h"
#endif

void __glProcessCachedTriangles (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;

    if (vc->vertexCount >= 3) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
	drawVertexes = gc->vertexArray.drawVertexes[GL_TRIANGLES];
	assert(NULL != drawVertexes);
#ifdef __GL_CODEGEN_PMON
	do {
	    (*drawVertexes)(gc, 0, vc->vertexCount, NULL);
	} while (PrintPerformanceCounters());
#else
	(*drawVertexes)(gc, 0, vc->vertexCount, NULL);
#endif
    }

    /*
    ** Whether we have been flushed or have hit an end it makes no difference
    ** because the cache buffer size is divisible by 3.
    */
    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}


/**********************************************************************************
**
** Independent Quads.
**
**********************************************************************************/

void __glProcessCachedQuads (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;

    if (vc->vertexCount >= 4) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
	drawVertexes = gc->vertexArray.drawVertexes[GL_QUADS];
	assert(NULL != drawVertexes);
	(*drawVertexes)(gc, 0, vc->vertexCount, NULL);
    }

    /*
    ** Whether we have been flushed or have hit an end it makes no difference
    ** because the cache buffer size is divisible by 4.
    */
    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}

#if defined(WIN32)
#pragma optimize("", on)
#endif

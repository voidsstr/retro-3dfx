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
 *                   Copyright (C) 1995  3Dlabs Ltd.                    *
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
#include "render.h"

#if defined(WIN32)
#pragma	optimize("a", on)
#endif

/**********************************************************************************
**
** Process line strips. 
**
**********************************************************************************/

void __glProcessCachedLineStrip (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;

    if (vc->vertexCount >= 2) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);

	drawVertexes = gc->vertexArray.drawVertexes[GL_LINE_STRIP];
	assert(NULL != drawVertexes);
	(*drawVertexes)(gc, 0, gc->vertexCache.vertexCount, NULL);

	/*
	** We were flushed, so we have to copy the last  vertex in the 
	** list over and set the start and count values appropriately.
	*/
	if (flushed == GL_FLUSH_VCACHE) {
	    __GLvertex *src = vc->vertexCache + vc->vertexCount - 1;
	    __GLvertex *dst = vc->vertexCache;
	    dst[0] = src[0]; dst[0].color = &dst[0].colors[__GL_FRONTFACE];
	    vc->vertexStart = vc->vertexCount = 1;
	    return;
	}
    }

    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}

/**********************************************************************************
**
** Process lines. 
**
**********************************************************************************/

void __glProcessCachedLines (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;

    if (vc->vertexCount >= 2) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
	drawVertexes = gc->vertexArray.drawVertexes[GL_LINES];
	assert(NULL != drawVertexes);
	(*drawVertexes)(gc, 0, gc->vertexCache.vertexCount, NULL);
    }
						
    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}												 
	  
/**********************************************************************************
**
** Process line loops. 
**
**********************************************************************************/

void __glProcessCachedLineLoop (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;
    GLsizei count, start;
    GLboolean alreadyInLoop = vc->vertexCacheState & VC_IN_LINE_LOOP;

    if (vc->vertexCount >= 2) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);

	if (flushed == GL_FLUSH_VCACHE || alreadyInLoop) {
	    /*
	    ** There is a partial loop in the cache and this is either
	    ** the first, last, or some middle piece, so we must render
	    ** it as a line strip.
	    */
	    drawVertexes = gc->vertexArray.drawVertexes[GL_LINE_STRIP];
	    if (!alreadyInLoop) {
		/* first piece of line loop */
		start = 0;
		count = vc->vertexCount;
		vc->vertexCacheState |= VC_IN_LINE_LOOP;

	    } else if (flushed == GL_END_VCACHE) {
		/* last piece of line loop */
		__GLvertex *src = vc->vertexCache + 0;
		__GLvertex *dst = vc->vertexCache + vc->vertexCount;
		dst[0] = src[0]; dst[0].color = &dst[0].colors[__GL_FRONTFACE];
		start = 1;
		count = vc->vertexCount;
		vc->vertexCacheState &= ~VC_IN_LINE_LOOP;

	    } else {
		/* some middle piece of line loop */
		start = 1;
		count = vc->vertexCount - 1;
	    }
	} else {
	    /*
	    ** There is one complete loop in the cache, so we can
	    ** process it as a line loop.
	    */
	    drawVertexes = gc->vertexArray.drawVertexes[GL_LINE_LOOP];
	    start = 0;
	    count = vc->vertexCount;
	}

	assert(NULL != drawVertexes);
	(*drawVertexes)(gc, start, count, NULL);

	if (flushed == GL_FLUSH_VCACHE) {
	    __GLvertex *src = vc->vertexCache + vc->vertexCount - 1;
	    __GLvertex *dst = vc->vertexCache + 1;
	    dst[0] = src[0]; dst[0].color = &dst[0].colors[__GL_FRONTFACE];
	    vc->vertexStart = vc->vertexCount = 2;
	    return;
	}
    }

    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}
	  
#if defined(WIN32)
#pragma	optimize("", on)
#endif

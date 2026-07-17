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

/**********************************************************************************
**
** Process the points in the cache
**
**********************************************************************************/

void __glProcessCachedPoints (__GLcontext *gc, GLuint flushed)
{
    __GLvcacheMachine *vc = &gc->vertexCache;

    if (vc->vertexCount >= 1) {
	void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
	drawVertexes = gc->vertexArray.drawVertexes[GL_POINTS];
	assert(NULL != drawVertexes);
	(*drawVertexes)(gc, 0, gc->vertexCache.vertexCount, NULL);
    }
    /* Reset the vertex counters. */
    vc->vertexStart = vc->vertexCount = 0;
    vc->vertexCacheState &= ~VC_MATERIAL_VALIDATE;
}


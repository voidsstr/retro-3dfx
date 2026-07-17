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
** $Date: 10/11/00 8:04:22 PM$
*/
#include "context.h"
#include "global.h"
#include "render.h"

void __glBeginPoints(__GLcontext *gc)
{
    __GLvertex *v0 = &gc->vertex.vbuf[0];

    gc->procs.vertex = gc->procs.vertexPoints;
    gc->vertex.v0 = v0;
    gc->procs.endPrim = __glEndPoints;
    gc->procs.matValidate = (void (*)(__GLcontext *gc)) __glNop;
}

void __glEndPoints(__GLcontext *gc)
{
    gc->procs.vertex = (void (*)(__GLcontext*, __GLvertex*)) __glNop;
    gc->procs.endPrim = __glEndPrim;
}

#ifndef __GL_USE_MIPSASMCODE
void __glPoint(__GLcontext *gc, __GLvertex *vx)
{
    if ((vx->hasAndClipCode & __GL_ALL_CLIP_MASK) == 0) {
	DO_VALIDATE(gc, vx, gc->vertex.faceNeeds[__GL_FRONTFACE] | __GL_HAS_FRONT_COLOR);
#if !defined(__GL_SST)
	__GL_LOCK_RENDER_BUFFERS(gc);
	(*gc->procs.renderPoint)(gc, vx);
	__GL_UNLOCK_RENDER_BUFFERS(gc);
#else
	(*gc->procs.renderPoint)(gc, vx);
#endif
    }
}

void __glPointFast(__GLcontext *gc, __GLvertex *vx)
{
    if ((vx->hasAndClipCode & __GL_ALL_CLIP_MASK) == 0) {
#if !defined(__GL_SST)
	__GL_LOCK_RENDER_BUFFERS(gc);
	(*gc->procs.renderPoint)(gc, vx);
	__GL_UNLOCK_RENDER_BUFFERS(gc);
#else
	(*gc->procs.renderPoint)(gc, vx);
#endif
    }
}
#endif

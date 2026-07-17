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
** $Date: 10/11/00 7:51:23 PM$
*/
#include "context.h"

void __glFirstLinesVertex(__GLcontext*, __GLvertex*);

void __glSecondLinesVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->line.notResetStipple = GL_FALSE;

    gc->vertex.v0 = v0 - 1;
    gc->procs.vertex = __glFirstLinesVertex;
    (*gc->procs.clipLine)(gc, v0 - 1, v0);
}

void __glFirstLinesVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = gc->procs.vertex2ndLines;
}

void __glBeginLines(__GLcontext *gc)
{
    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = __glFirstLinesVertex;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

/************************************************************************/

/*
** input  v0  v1    v0'  v1'   result
** -----  --  --    ---  ---   ------
** begin  --  --    -0   --
** A      A0  --    -1   A0
** B      B1  A0    A0   B1    draw AB
** C      C0  B1    B1   C0    draw BC
*/
void __glOtherLStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *v1 = gc->vertex.v1;

    gc->vertex.v0 = v1;
    gc->vertex.v1 = v0;

    if ((v1->hasAndClipCode | v0->hasAndClipCode) & __GL_ALL_CLIP_MASK) {
	/*
	** The line must be clipped more carefully.  Cannot trivially
	** accept the lines.
	*/
	if (((v1->hasAndClipCode & v0->hasAndClipCode) & __GL_ALL_CLIP_MASK) != 0) {
	    /*
	    ** Trivially reject the line.  If anding the codes is non-zero then
	    ** every vertex in the line is outside of the same set of
	    ** clipping planes (at least one).
	    */
	    return;
	}
	__glClipLine(gc, v1, v0);
	return;
    }

    /* Validate provoking vertex color */
    DO_VALIDATE(gc, v0, gc->vertex.faceNeeds[__GL_FRONTFACE]);

    /* Draw the line */
    (*gc->procs.renderLine)(gc, v1, v0);
}

#ifndef __GL_USE_MIPSASMCODE

void __glOtherLStripVertexFast(__GLcontext *gc, __GLvertex *v0)
{
    __GLvertex *v1 = gc->vertex.v1;

    gc->vertex.v0 = v1;
    gc->vertex.v1 = v0;

    if ((v0->hasAndClipCode | v1->hasAndClipCode) & __GL_ALL_CLIP_MASK) {
	(*gc->procs.clipLine)(gc, v1, v0);
    } else {
	(*gc->procs.renderLine)(gc, v1, v0);
    }
}

#endif /* !__GL_USE_MIPSASMCODE */

void __glFirstLStripVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v1 = v0;
    gc->procs.vertex = gc->procs.vertexLStrip;
    gc->procs.matValidate = __glMatValidateV1;

    /* Validate first vertex, if needed */
    if (gc->vertex.faceNeeds[__GL_FRONTFACE])
	DO_VALIDATE(gc, v0, gc->vertex.faceNeeds[__GL_FRONTFACE]);
}

void __glBeginLStrip(__GLcontext *gc)
{
    gc->line.notResetStipple = GL_FALSE;

    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = __glFirstLStripVertex;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

/************************************************************************/

/*
** Here is a three vertex example of a line loop:
**     input   v0  v1    v0'  v1'    result
**     -----   --  --    ---  ---    ------
**     begin   --  --    -0   --
**     A       A0  --    -1   --
**     B       B1  --    -2   B1     draw AB
**     C       C2  B1    B1   C2     draw BC
**     end     B1  C2    --   --     draw CA
**
** Here is a two vertex example of a line loop:
**     input   v0  v1    v0'  v1'    result
**     -----   --  --    ---  ---    ------
**     begin   --  --    -0   --
**     A       A0  --    -1   --
**     B       B1  --    -2   B1     draw AB
**     end     -2  B1    --   --     draw BA
**
** Here is a one vertex example of a line loop:
**     input   v0  v1    v0'  v1'    result
**     -----   --  --    ---  ---    ------
**     begin   --  --    -0   --
**     A       A0  --    -1   --
**     end     -1  --    --   --     nothing drawn
*/
static void SecondLLoopVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->vertex.v0 = v0 + 1;
    gc->vertex.v1 = v0;
    gc->procs.vertex = gc->procs.vertexLStrip;
    gc->procs.matValidate = __glMatValidateVbuf0V1;
    (*gc->procs.clipLine)(gc, v0 - 1, v0);
}

static void FirstLLoopVertex(__GLcontext *gc, __GLvertex *v0)
{
    gc->vertex.v0 = v0 + 1;
    gc->procs.vertex = SecondLLoopVertex;
}

void __glEndLLoop(__GLcontext *gc)
{
    /*
    ** This isn't a terribly kosher way of checking if we have gotten 
    ** two vertices already, but it is the best I can think of.
    */
    if (gc->procs.vertex != FirstLLoopVertex &&
	    gc->procs.vertex != SecondLLoopVertex) {
	/*
	** Close off the loop by drawing a final line segment back to the
	** first vertex.  The first vertex was saved in vbuf[0].
	*/
	(*gc->procs.clipLine)(gc, gc->vertex.v1, &gc->vertex.vbuf[0]);
    }
    gc->procs.vertex = (void (*)(__GLcontext*, __GLvertex*)) __glNop;
    gc->procs.endPrim = __glEndPrim;
}

void __glBeginLLoop(__GLcontext *gc)
{
    gc->line.notResetStipple = GL_FALSE;

    gc->vertex.v0 = &gc->vertex.vbuf[0];
    gc->procs.vertex = FirstLLoopVertex;
    gc->procs.endPrim = __glEndLLoop;
    gc->procs.matValidate = __glMatValidateVbuf0N;
}

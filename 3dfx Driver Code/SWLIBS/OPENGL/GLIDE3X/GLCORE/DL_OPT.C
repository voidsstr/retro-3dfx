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
#include <stdio.h>
#include "global.h"
#include "context.h"
#include "dlistopt.h"
#include "g_imfncs.h"
#include "g_disp.h"
#include <GL/gl.h>
#include "g_listop.h"
#include "g_lcomp.h"
#include "imports.h"

struct __gllc_Normal3fvVertex3fv_Rec {
    GLfloat n[3];
    GLfloat v[3];
};

struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec {
    GLfloat t[2];
    GLfloat n[3];
    GLfloat v[3];
};

struct __gllc_Color3fvNormal3fvVertex3fv_Rec {
    GLfloat c[3];
    GLfloat n[3];
    GLfloat v[3];
};

const GLubyte *__glle_Normal3fvVertex3fv(const GLubyte *PC)
{
    struct __gllc_Normal3fvVertex3fv_Rec *data;

    data = (struct __gllc_Normal3fvVertex3fv_Rec *) PC;
    (*__gl_dispatch.normal.Normal3fv)(data->n);
    (*__gl_dispatch.vertex.Vertex3fv)(data->v);
    return PC + sizeof(struct __gllc_Normal3fvVertex3fv_Rec);
}

const GLubyte *__glle_TexCoord2fvNormal3fvVertex3fv(const GLubyte *PC)
{
    struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec *data;

    data = (struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec *) PC;
    (*__gl_dispatch.texCoord.TexCoord2fv)(data->t);
    (*__gl_dispatch.normal.Normal3fv)(data->n);
    (*__gl_dispatch.vertex.Vertex3fv)(data->v);
    return PC + sizeof(struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec);
}

const GLubyte *__glle_Color3fvNormal3fvVertex3fv(const GLubyte *PC)
{
    struct __gllc_Color3fvNormal3fvVertex3fv_Rec *data;

    data = (struct __gllc_Color3fvNormal3fvVertex3fv_Rec *) PC;
    (*__gl_dispatch.color.Color3fv)(data->c);
    (*__gl_dispatch.normal.Normal3fv)(data->n);
    (*__gl_dispatch.vertex.Vertex3fv)(data->v);
    return PC + sizeof(struct __gllc_Color3fvNormal3fvVertex3fv_Rec);
}

const GLubyte *__glle_MultiTexCoord2fvNormal3fvVertex3fv(const GLubyte *PC)
{
    struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec *data;
    int i, count = *(int *)PC;
    data = (struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec *) (PC+sizeof(int));
    for (i = 0; i < count; i++) {
        (*__gl_dispatch.texCoord.TexCoord2fv)(data->t);
        (*__gl_dispatch.normal.Normal3fv)(data->n);
        (*__gl_dispatch.vertex.Vertex3fv)(data->v);
        data++;
    }
    return (GLubyte *)data;
}

const GLubyte *__glle_MultiColor3fvNormal3fvVertex3fv(const GLubyte *PC)
{
    struct __gllc_Color3fvNormal3fvVertex3fv_Rec *data;
    int i, count = *(int *)PC;
    data = (struct __gllc_Color3fvNormal3fvVertex3fv_Rec *) (PC+sizeof(int));
    for (i = 0; i < count; i++) {
        (*__gl_dispatch.color.Color3fv)(data->c);
        (*__gl_dispatch.normal.Normal3fv)(data->n);
        (*__gl_dispatch.vertex.Vertex3fv)(data->v);
        data++;
    }
    return (GLubyte *)data;
}

const GLubyte *__glle_MultiNormal3fvVertex3fv(const GLubyte *PC)
{
    struct __gllc_Normal3fvVertex3fv_Rec *data;
    int i, count = *(int *)PC;
    data = (struct __gllc_Normal3fvVertex3fv_Rec *) (PC+sizeof(int));
    for (i = 0; i < count; i++) {
        (*__gl_dispatch.normal.Normal3fv)(data->n);
        (*__gl_dispatch.vertex.Vertex3fv)(data->v);
        data++;
    }
    return (GLubyte *)data;
}

const GLubyte *__glle_MultiVertex3fv(const GLubyte *PC)
{
    struct __gllc_Vertex3fv_Rec *data;
    int i, count = *(int *)PC;
    data = (struct __gllc_Vertex3fv_Rec *) (PC+sizeof(int));
    for (i = 0; i < count; i++) {
        (*__gl_dispatch.vertex.Vertex3fv)(data->v);
        data++;
    }
    return (GLubyte *)data;
}

static void optimizeTextureNormalVertex(__GLcontext *gc,
					__GLcompiledDlist *cdlist) {
    __GLdlistOp **first;
    __GLdlistOp *one, *two, *three, *dlop;
    struct __gllc_TexCoord2fv_Rec *targs;
    struct __gllc_Normal3fv_Rec *nargs;
    struct __gllc_Vertex3fv_Rec *vargs;
    struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec *tnvargs;

    first = &cdlist->dlist;
    while (*first) {
        if ((*first)->opcode == __glop_TexCoord2fv) {
	    one = *first;
            two = one->next;
            if (two && two->opcode == __glop_Normal3fv) {
                three = two->next;
                if (three && three->opcode == __glop_Vertex3fv) {

  	            // Create a new dlist op and add it in place
                    dlop = __glDlistAllocOp2(gc,
 		     sizeof(struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec));
                    dlop->opcode = __glop_TexCoord2fvNormal3fvVertex3fv;
                    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_NORMAL;
                    dlop->next = three->next;
                    *first = dlop;
                    first = &(dlop->next);
		    targs = (struct __gllc_TexCoord2fv_Rec *)one->data;
                    nargs = (struct __gllc_Normal3fv_Rec *)two->data;
                    vargs = (struct __gllc_Vertex3fv_Rec *)three->data;
                    tnvargs = (struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec *)&dlop->data;
                    tnvargs->t[0] = targs->v[0];
                    tnvargs->t[1] = targs->v[1];
                    tnvargs->n[0] = nargs->v[0];
                    tnvargs->n[1] = nargs->v[1];
                    tnvargs->n[2] = nargs->v[2];
                    tnvargs->v[0] = vargs->v[0];
                    tnvargs->v[1] = vargs->v[1];
                    tnvargs->v[2] = vargs->v[2];

                    // Free previous dlist ops
                    __glDlistFreeOp(gc, one);
                    __glDlistFreeOp(gc, two);
                    __glDlistFreeOp(gc, three);
                } else {
                    first = &((*first)->next);
		}
            } else {
                first = &((*first)->next);
            }
        } else {
            first = &((*first)->next);
	}
    }
}

static void optimizeColorNormalVertex(__GLcontext *gc,
				      __GLcompiledDlist *cdlist) {
    __GLdlistOp **first;
    __GLdlistOp *one, *two, *three, *dlop;
    struct __gllc_Color3fv_Rec *cargs;
    struct __gllc_Normal3fv_Rec *nargs;
    struct __gllc_Vertex3fv_Rec *vargs;
    struct __gllc_Color3fvNormal3fvVertex3fv_Rec *tnvargs;

    first = &cdlist->dlist;
    while (*first) {
        if ((*first)->opcode == __glop_Color3fv) {
	    one = *first;
            two = one->next;
            if (two && two->opcode == __glop_Normal3fv) {
                three = two->next;
                if (three && three->opcode == __glop_Vertex3fv) {

  	            // Create a new dlist op and add it in place
                    dlop = __glDlistAllocOp2(gc,
 		     sizeof(struct __gllc_Color3fvNormal3fvVertex3fv_Rec));
                    dlop->opcode = __glop_Color3fvNormal3fvVertex3fv;
                    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_NORMAL;
                    dlop->next = three->next;
                    *first = dlop;
                    first = &(dlop->next);
		    cargs = (struct __gllc_Color3fv_Rec *)one->data;
                    nargs = (struct __gllc_Normal3fv_Rec *)two->data;
                    vargs = (struct __gllc_Vertex3fv_Rec *)three->data;
                    tnvargs = (struct __gllc_Color3fvNormal3fvVertex3fv_Rec *)&dlop->data;
                    tnvargs->c[0] = cargs->v[0];
                    tnvargs->c[1] = cargs->v[1];
                    tnvargs->c[2] = cargs->v[2];
                    tnvargs->n[0] = nargs->v[0];
                    tnvargs->n[1] = nargs->v[1];
                    tnvargs->n[2] = nargs->v[2];
                    tnvargs->v[0] = vargs->v[0];
                    tnvargs->v[1] = vargs->v[1];
                    tnvargs->v[2] = vargs->v[2];

                    // Free previous dlist ops
                    __glDlistFreeOp(gc, one);
                    __glDlistFreeOp(gc, two);
                    __glDlistFreeOp(gc, three);
                } else {
                    first = &((*first)->next);
		}
            } else {
                first = &((*first)->next);
            }
        } else {
            first = &((*first)->next);
	}
    }
}

static void optimizeNormalVertex(__GLcontext *gc, __GLcompiledDlist *cdlist) {
    __GLdlistOp **first;
    __GLdlistOp *start, *next, *dlop;
    struct __gllc_Normal3fv_Rec *nargs;
    struct __gllc_Vertex3fv_Rec *vargs;
    struct __gllc_Normal3fvVertex3fv_Rec *nvargs;

    first = &cdlist->dlist;
    while (*first) {
        if ((*first)->opcode == __glop_Normal3fv) {
	    start = *first;
            next = start->next;
            if (next && next->opcode == __glop_Vertex3fv) {

	        // Create a new dlist op and add it in place
                dlop = __glDlistAllocOp2(gc,
			   sizeof(struct __gllc_Normal3fvVertex3fv_Rec));
                dlop->opcode = __glop_Normal3fvVertex3fv;
                gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_NORMAL;
                dlop->next = next->next;
                *first = dlop;
                first = &(dlop->next);
                nargs = (struct __gllc_Normal3fv_Rec *)start->data;
                vargs = (struct __gllc_Vertex3fv_Rec *)next->data;
                nvargs = (struct __gllc_Normal3fvVertex3fv_Rec *)&dlop->data;
                nvargs->n[0] = nargs->v[0];
                nvargs->n[1] = nargs->v[1];
                nvargs->n[2] = nargs->v[2];
                nvargs->v[0] = vargs->v[0];
                nvargs->v[1] = vargs->v[1];
                nvargs->v[2] = vargs->v[2];

                // Free previous dlist ops
                __glDlistFreeOp(gc, start);
                __glDlistFreeOp(gc, next);
            } else {
                first = &((*first)->next);
            }
        } else {
            first = &((*first)->next);
	}
    }
}

static void optimizeMultiNormalVertex(__GLcontext *gc, __GLcompiledDlist *cdlist) {
    __GLdlistOp **first;
    __GLdlistOp *start, *next, *dlop, *node;
    int count = 0;
    int i;
    struct __gllc_Normal3fvVertex3fv_Rec *cp;

    first = &cdlist->dlist;
    while (*first) {
        if ((*first)->opcode == __glop_Normal3fvVertex3fv) {
	    start = *first;
            count = 1;
            for (next = start->next; next; next = next->next, count++) {
	        if (next->opcode != __glop_Normal3fvVertex3fv) {
                    break;
                }
            }

            if (count > 1) {
	        // Create a new dlist op and add it in place
                dlop = __glDlistAllocOp2(gc,
			 sizeof(int) +
			 count*sizeof(struct __gllc_Normal3fvVertex3fv_Rec));
                dlop->opcode = __glop_MultiNormal3fvVertex3fv;
                gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_NORMAL;
                dlop->next = next;
                *first = dlop;
                first = &(dlop->next);
                *(int *) dlop->data = count;
                cp = (struct __gllc_Normal3fvVertex3fv_Rec *)
		  (dlop->data + sizeof(int));
                node = start;
                for (i = 0; i < count; i++) {
                    __GL_MEMCOPY(cp, node->data,
				 sizeof(struct __gllc_Normal3fvVertex3fv_Rec));
		    node = node->next;
                    cp++;
                }

                // Free previous dlist ops
                dlop = start;
                while (dlop != next) {
                    start = dlop;
                    dlop = dlop->next;
                    __glDlistFreeOp(gc, start);
		}
            } else {
                first = &((*first)->next);
            }
        } else {
            first = &((*first)->next);
	}
    }
}

static void optimizeMultiTextureNormalVertex(__GLcontext *gc, __GLcompiledDlist *cdlist) {
    __GLdlistOp **first;
    __GLdlistOp *start, *next, *dlop, *node;
    int count = 0;
    int i;
    struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec *cp;

    first = &cdlist->dlist;
    while (*first) {
        if ((*first)->opcode == __glop_TexCoord2fvNormal3fvVertex3fv) {
	    start = *first;
            count = 1;
            for (next = start->next; next; next = next->next, count++) {
	        if (next->opcode != __glop_TexCoord2fvNormal3fvVertex3fv) {
                    break;
                }
            }

            if (count > 1) {
	        // Create a new dlist op and add it in place
                dlop = __glDlistAllocOp2(gc,
			 sizeof(int) +
			 count*sizeof(struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec));
                dlop->opcode = __glop_MultiTexCoord2fvNormal3fvVertex3fv;
                gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_NORMAL;
                dlop->next = next;
                *first = dlop;
                first = &(dlop->next);
                *(int *) dlop->data = count;
                cp = (struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec *)
		  (dlop->data + sizeof(int));
                node = start;
                for (i = 0; i < count; i++) {
                    __GL_MEMCOPY(cp, node->data,
				 sizeof(struct __gllc_TexCoord2fvNormal3fvVertex3fv_Rec));
		    node = node->next;
                    cp++;
                }

                // Free previous dlist ops
                dlop = start;
                while (dlop != next) {
                    start = dlop;
                    dlop = dlop->next;
                    __glDlistFreeOp(gc, start);
		}
            } else {
                first = &((*first)->next);
            }
        } else {
            first = &((*first)->next);
	}
    }
}

static void optimizeMultiColorNormalVertex(__GLcontext *gc, __GLcompiledDlist *cdlist) {
    __GLdlistOp **first;
    __GLdlistOp *start, *next, *dlop, *node;
    int count = 0;
    int i;
    struct __gllc_Color3fvNormal3fvVertex3fv_Rec *cp;

    first = &cdlist->dlist;
    while (*first) {
        if ((*first)->opcode == __glop_Color3fvNormal3fvVertex3fv) {
	    start = *first;
            count = 1;
            for (next = start->next; next; next = next->next, count++) {
	        if (next->opcode != __glop_Color3fvNormal3fvVertex3fv) {
                    break;
                }
            }

            if (count > 1) {
	        // Create a new dlist op and add it in place
                dlop = __glDlistAllocOp2(gc,
			 sizeof(int) +
			 count*sizeof(struct __gllc_Color3fvNormal3fvVertex3fv_Rec));
                dlop->opcode = __glop_MultiColor3fvNormal3fvVertex3fv;
                gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_NORMAL;
                dlop->next = next;
                *first = dlop;
                first = &(dlop->next);
                *(int *) dlop->data = count;
                cp = (struct __gllc_Color3fvNormal3fvVertex3fv_Rec *)
		  (dlop->data + sizeof(int));
                node = start;
                for (i = 0; i < count; i++) {
                    __GL_MEMCOPY(cp, node->data,
				 sizeof(struct __gllc_Color3fvNormal3fvVertex3fv_Rec));
		    node = node->next;
                    cp++;
                }

                // Free previous dlist ops
                dlop = start;
                while (dlop != next) {
                    start = dlop;
                    dlop = dlop->next;
                    __glDlistFreeOp(gc, start);
		}
            } else {
                first = &((*first)->next);
            }
        } else {
            first = &((*first)->next);
	}
    }
}

static void optimizeMultiVertex(__GLcontext *gc, __GLcompiledDlist *cdlist) {
    __GLdlistOp **first;
    __GLdlistOp *start, *next, *dlop, *node;
    int count = 0;
    int i;
    struct __gllc_Vertex3fv_Rec *cp;

    first = &cdlist->dlist;
    while (*first) {
        if ((*first)->opcode == __glop_Vertex3fv) {
	    start = *first;
            count = 1;
            for (next = start->next; next; next = next->next, count++) {
	        if (next->opcode != __glop_Vertex3fv) {
                    break;
                }
            }

            if (count > 1) {
	        // Create a new dlist op and add it in place
                dlop = __glDlistAllocOp2(gc,
			 sizeof(int) +
			 count*sizeof(struct __gllc_Vertex3fv_Rec));
                dlop->opcode = __glop_MultiVertex3fv;
                gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_NORMAL;
                dlop->next = next;
                *first = dlop;
                first = &(dlop->next);
                *(int *) dlop->data = count;
                cp = (struct __gllc_Vertex3fv_Rec *)
		  (dlop->data + sizeof(int));
                node = start;
                for (i = 0; i < count; i++) {
                    __GL_MEMCOPY(cp, node->data,
				 sizeof(struct __gllc_Vertex3fv_Rec));
		    node = node->next;
                    cp++;
                }

                // Free previous dlist ops
                dlop = start;
                while (dlop != next) {
                    start = dlop;
                    dlop = dlop->next;
                    __glDlistFreeOp(gc, start);
		}
            } else {
                first = &((*first)->next);
            }
        } else {
            first = &((*first)->next);
	}
    }
}

void __glDlistOptimizeCDRS(__GLcontext *gc, __GLcompiledDlist *cdlist) {
    optimizeTextureNormalVertex(gc, cdlist);
    optimizeColorNormalVertex(gc, cdlist);
    optimizeNormalVertex(gc, cdlist);
    optimizeMultiTextureNormalVertex(gc, cdlist);
    optimizeMultiColorNormalVertex(gc, cdlist);
    optimizeMultiNormalVertex(gc, cdlist);
    optimizeMultiVertex(gc, cdlist);
}

/*
** The default display list optimizer.  By default, consecutive material
** calls stored in a display list are optimized.
*/
void __glGenericDlistOptimizer(__GLcontext *gc, __GLcompiledDlist *cdlist)
{
#ifdef __GL_LINT
    gc = gc;
    cdlist = cdlist;
#endif
    __glDlistOptimizeCDRS(gc, cdlist);
    __glDlistOptimizeMaterial(gc, cdlist);
}

__GLlistExecFunc *__gl_GenericDlOps[] = {
    __glle_Begin_LineLoop,
    __glle_Begin_LineStrip,
    __glle_Begin_Lines,
    __glle_Begin_Points,
    __glle_Begin_Polygon,
    __glle_Begin_TriangleStrip,
    __glle_Begin_TriangleFan,
    __glle_Begin_Triangles,
    __glle_Begin_QuadStrip,
    __glle_Begin_Quads,
    __glle_InvalidValue,
    __glle_InvalidEnum,
    __glle_InvalidOperation,
    __glle_UnimplementedExtension,
    __glle_FastMaterial,
    __glle_TableTooLarge,
    __glle_Normal3fvVertex3fv,
    __glle_TexCoord2fvNormal3fvVertex3fv,
    __glle_Color3fvNormal3fvVertex3fv,
    __glle_MultiNormal3fvVertex3fv,
    __glle_MultiTexCoord2fvNormal3fvVertex3fv,
    __glle_MultiColor3fvNormal3fvVertex3fv,
    __glle_MultiVertex3fv,
};

/*
** This is the compilation routine for Begin.  It doesn't actually serve
** any terribly important purpose.  It simply stores the type of begin
** in the type of display list entry rather than in the entry itself.
*/
void APIENTRY __gllc_Begin(GLenum mode)
{
    __GLdlistOp *dlop;
    __GLlistExecFunc *func;
    GLint opcode;
    __GL_SETUP();

    dlop = __glDlistAllocOp2(gc, 0);
    if (dlop == NULL) return;

    switch(mode) {
      case GL_LINE_LOOP:
	opcode = __glop_Begin_LineLoop;
	func = __glle_Begin_LineLoop;
	break;
      case GL_LINE_STRIP:
	opcode = __glop_Begin_LineStrip;
	func = __glle_Begin_LineStrip;
	break;
      case GL_LINES:
	opcode = __glop_Begin_Lines;
	func = __glle_Begin_Lines;
	break;
      case GL_POINTS:
	opcode = __glop_Begin_Points;
	func = __glle_Begin_Points;
	break;
      case GL_POLYGON:
	opcode = __glop_Begin_Polygon;
	func = __glle_Begin_Polygon;
	break;
      case GL_TRIANGLE_STRIP:
	opcode = __glop_Begin_TriangleStrip;
	func = __glle_Begin_TriangleStrip;
	break;
      case GL_TRIANGLE_FAN:
	opcode = __glop_Begin_TriangleFan;
	func = __glle_Begin_TriangleFan;
	break;
      case GL_TRIANGLES:
	opcode = __glop_Begin_Triangles;
	func = __glle_Begin_Triangles;
	break;
      case GL_QUAD_STRIP:
	opcode = __glop_Begin_QuadStrip;
	func = __glle_Begin_QuadStrip;
	break;
      case GL_QUADS:
	opcode = __glop_Begin_Quads;
	func = __glle_Begin_Quads;
	break;
      default:
	dlop->opcode = __glop_InvalidEnum;
	__glDlistAppendOp(gc, dlop, __glle_InvalidEnum);
	return;
    }

    gc->dlist.listData.genericFlags |= __GL_DLFLAG_HAS_BEGIN;
    dlop->opcode = opcode;
    __glDlistAppendOp(gc, dlop, func);
}

/************************************************************************/

const GLubyte *__glle_Begin_LineLoop(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_LINE_LOOP);
    return PC;
}

const GLubyte *__glle_Begin_LineStrip(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_LINE_STRIP);
    return PC;
}

const GLubyte *__glle_Begin_Lines(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_LINES);
    return PC;
}

const GLubyte *__glle_Begin_Points(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_POINTS);
    return PC;
}

const GLubyte *__glle_Begin_Polygon(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_POLYGON);
    return PC;
}

const GLubyte *__glle_Begin_TriangleStrip(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_TRIANGLE_STRIP);
    return PC;
}

const GLubyte *__glle_Begin_TriangleFan(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_TRIANGLE_FAN);
    return PC;
}

const GLubyte *__glle_Begin_Triangles(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_TRIANGLES);
    return PC;
}

const GLubyte *__glle_Begin_QuadStrip(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_QUAD_STRIP);
    return PC;
}

const GLubyte *__glle_Begin_Quads(const GLubyte *PC)
{
    (*__gl_dispatch.dispatch.Begin)(GL_QUADS);
    return PC;
}

/************************************************************************/

/*
** Optimized errors.  Strange but true.  These are called to save an error
** in the display list.
*/
void APIENTRY __gllc_InvalidValue(__GLcontext *gc)
{
    __GLdlistOp *dlop;

    dlop = __glDlistAllocOp2(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_InvalidValue;
    __glDlistAppendOp(gc, dlop, __glle_InvalidValue);
}

void APIENTRY __gllc_InvalidEnum(__GLcontext *gc)
{
    __GLdlistOp *dlop;

    dlop = __glDlistAllocOp2(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_InvalidEnum;
    __glDlistAppendOp(gc, dlop, __glle_InvalidEnum);
}

void APIENTRY __gllc_InvalidOperation(__GLcontext *gc)
{
    __GLdlistOp *dlop;

    dlop = __glDlistAllocOp2(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_InvalidOperation;
    __glDlistAppendOp(gc, dlop, __glle_InvalidOperation);
}

void APIENTRY __gllc_TableTooLarge(__GLcontext *gc)
{
    __GLdlistOp *dlop;
    dlop = __glDlistAllocOp2(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_TableTooLarge;
    __glDlistAppendOp(gc, dlop, __glle_TableTooLarge);
}

/*
** Special case function call. Needs void argument to be plugged into
** dispatch tables.
*/
void APIENTRY __gllc_UnimplementedExtension(void)
{
    __GLdlistOp *dlop;
    __GL_SETUP();
    dlop = __glDlistAllocOp2(gc, 0);
    if (dlop == NULL) return;
    dlop->opcode = __glop_UnimplementedExtension;
    __glDlistAppendOp(gc, dlop, __glle_UnimplementedExtension);
}


void APIENTRY __gllc_Error(__GLcontext *gc, GLenum error)
{
    switch(error) {
      case GL_INVALID_VALUE:
	__gllc_InvalidValue(gc);
	break;
      case GL_INVALID_ENUM:
	__gllc_InvalidEnum(gc);
	break;
      case GL_INVALID_OPERATION:
	__gllc_InvalidOperation(gc);
	break;
      case GL_TABLE_TOO_LARGE_EXT:
	__gllc_TableTooLarge(gc);
	break;

    }
}

/*
** These routines execute an error stored in a display list.
*/
const GLubyte *__glle_InvalidValue(const GLubyte *PC)
{
    __glSetError(GL_INVALID_VALUE);
    return PC;
}

const GLubyte *__glle_InvalidEnum(const GLubyte *PC)
{
    __glSetError(GL_INVALID_ENUM);
    return PC;
}

const GLubyte *__glle_InvalidOperation(const GLubyte *PC)
{
    __glSetError(GL_INVALID_OPERATION);
    return PC;
}

const GLubyte *__glle_TableTooLarge(const GLubyte *PC)
{
    __glSetError(GL_TABLE_TOO_LARGE_EXT);
    return PC;
}



/*
** Although it returns the same error condition, I've created a
** new error routine for unimplemented extensions to help with debugging.
*/

const GLubyte *__glle_UnimplementedExtension(const GLubyte *PC)
{
  __glSetError(GL_INVALID_OPERATION);
  return PC;
}




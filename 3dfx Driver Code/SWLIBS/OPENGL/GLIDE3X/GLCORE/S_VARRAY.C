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
** $Date: 10/11/00 7:56:52 PM$
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "context.h"
#include "global.h"
#include "varray.h"

#include "glnew.h"
#include "g_imfncs.h"

#include "geom_og.h"

#define __GL_CODEGEN_STATS

#include "timer.h"

static GLboolean AllocateVbuf(__GLcontext *gc, int first, int count);

void __glDrawVertexes_Points(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
void __glDrawVertexes_Lines(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
void __glDrawVertexes_Lloop(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
void __glDrawVertexes_Lstrip(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
void __glDrawVertexes_Triangles(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
void __glDrawVertexes_Tstrip(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
void __glDrawVertexes_Tfan(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
void __glDrawVertexes_Quads(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
void __glDrawVertexes_Qstrip(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);
void __glDrawVertexes_Polygon(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);

#pragma optimize("aw", on)

static void __fastcall CopyVertex2s(__GLcontext *gc, const GLshort *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = 0.0F; v->obj.w = 1.0F;
}
static void __fastcall CopyVertex2i(__GLcontext *gc, const GLint *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = 0.0F; v->obj.w = 1.0F;
}
static void __fastcall CopyVertex2f(__GLcontext *gc, const GLfloat *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = 0.0F; v->obj.w = 1.0F;
}
static void __fastcall CopyVertex2d(__GLcontext *gc, const GLdouble *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = 0.0F; v->obj.w = 1.0F;
}
static void __fastcall CopyVertex3s(__GLcontext *gc, const GLshort *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = vp[2]; v->obj.w = 1.0F;
}
static void __fastcall CopyVertex3i(__GLcontext *gc, const GLint *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = vp[2]; v->obj.w = 1.0F;
}
static void __fastcall CopyVertex3f(__GLcontext *gc, const GLfloat *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = vp[2]; v->obj.w = 1.0F;
}
static void __fastcall CopyVertex3d(__GLcontext *gc, const GLdouble *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = vp[2]; v->obj.w = 1.0F;
}
static void __fastcall CopyVertex4s(__GLcontext *gc, const GLshort *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = vp[2]; v->obj.w = vp[3];
}
static void __fastcall CopyVertex4i(__GLcontext *gc, const GLint *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = vp[2]; v->obj.w = vp[3];
}
static void __fastcall CopyVertex4f(__GLcontext *gc, const GLfloat *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = vp[2]; v->obj.w = vp[3];
}
static void __fastcall CopyVertex4d(__GLcontext *gc, const GLdouble *vp, __GLvertex *v)
{
    v->obj.x = vp[0]; v->obj.y = vp[1]; v->obj.z = vp[2]; v->obj.w = vp[3];
}

static void __fastcall CopyNormal3b(__GLcontext *gc, const GLbyte *np, __GLvertex *v)
{
    v->normal.x = np[0]; v->normal.y = np[1]; v->normal.z = np[2];
}
static void __fastcall CopyNormal3s(__GLcontext *gc, const GLshort *np, __GLvertex *v)
{
    v->normal.x = np[0]; v->normal.y = np[1]; v->normal.z = np[2];
}
static void __fastcall CopyNormal3i(__GLcontext *gc, const GLint *np, __GLvertex *v)
{
    v->normal.x = np[0]; v->normal.y = np[1]; v->normal.z = np[2];
}
static void __fastcall CopyNormal3f(__GLcontext *gc, const GLfloat *np, __GLvertex *v)
{
    v->normal.x = np[0]; v->normal.y = np[1]; v->normal.z = np[2];
}
static void __fastcall CopyNormal3d(__GLcontext *gc, const GLdouble *np, __GLvertex *v)
{
    v->normal.x = np[0]; v->normal.y = np[1]; v->normal.z = np[2];
}
static void __fastcall CopyCurrentNormal(__GLcontext *gc, const GLdouble *np, __GLvertex *v)
{
    v->normal.x = gc->state.current.normal.x;
    v->normal.y = gc->state.current.normal.y;
    v->normal.z = gc->state.current.normal.z;
}

static void __fastcall CopyColor3b(__GLcontext *gc, const GLbyte *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_B_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_B_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_B_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = 1.0F;
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor3ub(__GLcontext *gc, const GLubyte *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_UB_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_UB_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_UB_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = 1.0F;
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor3s(__GLcontext *gc, const GLshort *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_S_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_S_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_S_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = 1.0F;
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor3us(__GLcontext *gc, const GLushort *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_US_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_US_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_US_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = 1.0F;
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor3i(__GLcontext *gc, const GLint *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_I_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_I_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_I_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = 1.0F;
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor3ui(__GLcontext *gc, const GLuint *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_UI_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_UI_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_UI_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = 1.0F;
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor3f(__GLcontext *gc, const GLfloat *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = cp[0];
    v->colors[__GL_FRONTFACE].g = cp[1];
    v->colors[__GL_FRONTFACE].b = cp[2];
    v->colors[__GL_FRONTFACE].a = 1.0F;
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor3d(__GLcontext *gc, const GLdouble *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = cp[0];
    v->colors[__GL_FRONTFACE].g = cp[1];
    v->colors[__GL_FRONTFACE].b = cp[2];
    v->colors[__GL_FRONTFACE].a = 1.0F;
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor4b(__GLcontext *gc, const GLbyte *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_B_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_B_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_B_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = __GL_B_TO_FLOAT(cp[3]);
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor4ub(__GLcontext *gc, const GLubyte *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_UB_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_UB_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_UB_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = __GL_UB_TO_FLOAT(cp[3]);
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor4s(__GLcontext *gc, const GLshort *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_S_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_S_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_S_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = __GL_S_TO_FLOAT(cp[3]);
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor4us(__GLcontext *gc, const GLushort *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_US_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_US_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_US_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = __GL_US_TO_FLOAT(cp[3]);
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor4i(__GLcontext *gc, const GLint *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_I_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_I_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_I_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = __GL_I_TO_FLOAT(cp[3]);
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor4ui(__GLcontext *gc, const GLuint *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = __GL_UI_TO_FLOAT(cp[0]);
    v->colors[__GL_FRONTFACE].g = __GL_UI_TO_FLOAT(cp[1]);
    v->colors[__GL_FRONTFACE].b = __GL_UI_TO_FLOAT(cp[2]);
    v->colors[__GL_FRONTFACE].a = __GL_UI_TO_FLOAT(cp[3]);
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor4f(__GLcontext *gc, const GLfloat *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = cp[0];
    v->colors[__GL_FRONTFACE].g = cp[1];
    v->colors[__GL_FRONTFACE].b = cp[2];
    v->colors[__GL_FRONTFACE].a = cp[3];
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyColor4d(__GLcontext *gc, const GLdouble *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = cp[0];
    v->colors[__GL_FRONTFACE].g = cp[1];
    v->colors[__GL_FRONTFACE].b = cp[2];
    v->colors[__GL_FRONTFACE].a = cp[3];
    __glClampAndScaleColorf(gc, &v->colors[__GL_FRONTFACE], &v->colors[__GL_FRONTFACE].r);
}
static void __fastcall CopyCurrentColor(__GLcontext *gc, const GLdouble *cp, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = gc->state.current.color.r;
    v->colors[__GL_FRONTFACE].g = gc->state.current.color.g;
    v->colors[__GL_FRONTFACE].b = gc->state.current.color.b;
    v->colors[__GL_FRONTFACE].a = gc->state.current.color.a;
}

static void __fastcall CopyIndexub(__GLcontext *gc, const GLubyte *ip, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = ip[0];
}
static void __fastcall CopyIndexs(__GLcontext *gc, const GLshort *ip, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = ip[0];
}
static void __fastcall CopyIndexi(__GLcontext *gc, const GLint *ip, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = ip[0];
}
static void __fastcall CopyIndexf(__GLcontext *gc, const GLfloat *ip, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = ip[0];
}
static void __fastcall CopyIndexd(__GLcontext *gc, const GLdouble *ip, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = ip[0];
}
static void __fastcall CopyCurrentIndex(__GLcontext *gc, const GLdouble *ip, __GLvertex *v)
{
    v->colors[__GL_FRONTFACE].r = gc->state.current.userColorIndex;
}

static void __fastcall CopyTexCoord1s(__GLcontext *gc, const GLshort *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = 0.0F; v->texture[0].z = 0.0F; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord1i(__GLcontext *gc, const GLint *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = 0.0F; v->texture[0].z = 0.0F; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord1f(__GLcontext *gc, const GLfloat *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = 0.0F; v->texture[0].z = 0.0F; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord1d(__GLcontext *gc, const GLdouble *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = 0.0F; v->texture[0].z = 0.0F; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord2s(__GLcontext *gc, const GLshort *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = 0.0F; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord2i(__GLcontext *gc, const GLint *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = 0.0F; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord2f(__GLcontext *gc, const GLfloat *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = 0.0F; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord2d(__GLcontext *gc, const GLdouble *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = 0.0F; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord3s(__GLcontext *gc, const GLshort *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = tp[2]; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord3i(__GLcontext *gc, const GLint *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = tp[2]; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord3f(__GLcontext *gc, const GLfloat *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = tp[2]; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord3d(__GLcontext *gc, const GLdouble *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = tp[2]; v->texture[0].w = 1.0F;
}
static void __fastcall CopyTexCoord4s(__GLcontext *gc, const GLshort *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = tp[2]; v->texture[0].w = tp[3];
}
static void __fastcall CopyTexCoord4i(__GLcontext *gc, const GLint *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = tp[2]; v->texture[0].w = tp[3];
}
static void __fastcall CopyTexCoord4f(__GLcontext *gc, const GLfloat *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = tp[2]; v->texture[0].w = tp[3];
}
static void __fastcall CopyTexCoord4d(__GLcontext *gc, const GLdouble *tp, __GLvertex *v)
{
    v->texture[0].x = tp[0]; v->texture[0].y = tp[1]; v->texture[0].z = tp[2]; v->texture[0].w = tp[3];
}
static void __fastcall CopyCurrentTexCoord(__GLcontext *gc, const GLdouble *tp, __GLvertex *v)
{
    v->texture[0].x = gc->state.current.texture[0].x;
    v->texture[0].y = gc->state.current.texture[0].y;
    v->texture[0].z = gc->state.current.texture[0].z;
    v->texture[0].w = gc->state.current.texture[0].w;
}

static void __fastcall CopyEdgeFlag(__GLcontext *gc, const GLboolean *ep, __GLvertex *v)
{
    v->boundaryEdge = ep[0];
}
static void __fastcall CopyCurrentEdgeFlag(__GLcontext *gc, const GLboolean *ep, __GLvertex *v)
{
    v->boundaryEdge = gc->state.current.edgeTag;
}

static void CompileElements(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        v->hasAndClipCode = gc->vertexArray.validateMask;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
        v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
    }
}
static void CompileElementsN(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *np, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    np = (const GLubyte *) gc->vertexArray.normal_pointer +
                                first * gc->vertexArray.np_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyNormal)(gc, np, v);
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot = v->normal.x * (gc->transform.cullEye.x - v->obj.x) +
                      v->normal.y * (gc->transform.cullEye.y - v->obj.y) +
                      v->normal.z * (gc->transform.cullEye.z - v->obj.z);
            } else {
                dot = v->normal.x * gc->transform.cullEye.x +
                      v->normal.y * gc->transform.cullEye.y +
                      v->normal.z * gc->transform.cullEye.z;
            }
            v->hasAndClipCode = gc->vertexArray.cullCode[dot <= 0.0F] | gc->vertexArray.validateMask;
        } else {
            v->hasAndClipCode = gc->vertexArray.validateMask;
        }
        np += gc->vertexArray.np_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
            v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
        }
    }
}
static void CompileElements_N3F_V3F(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    int off = gc->vertexArray.vp_stride * first;
    const GLubyte *np = (const GLubyte *) gc->vertexArray.normal_pointer;
    const GLubyte *vp = (const GLubyte *) gc->vertexArray.vertex_pointer;
    int i;
    GLuint hacc;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        __GLfloat *normalPtr = (GLfloat *) (np + off);
        __GLfloat *vertexPtr = (GLfloat *) (vp + off);

        off += gc->vertexArray.vp_stride;
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot = normalPtr[0] * (gc->transform.cullEye.x - vertexPtr[0]) +
                      normalPtr[1] * (gc->transform.cullEye.y - vertexPtr[1]) +
                      normalPtr[2] * (gc->transform.cullEye.z - vertexPtr[2]);
            } else {
                dot = normalPtr[0] * gc->transform.cullEye.x +
                      normalPtr[1] * gc->transform.cullEye.y +
                      normalPtr[2] * gc->transform.cullEye.z;
            }
            hacc = gc->vertexArray.cullCode[dot <= 0.0F] | gc->vertexArray.validateMask;
        } else {
            hacc = gc->vertexArray.validateMask;
        }
        if (!(hacc & __GL_ALL_CLIP_MASK)) {
            v->normal.x = normalPtr[0];
            v->normal.y = normalPtr[1];
            v->normal.z = normalPtr[2];
            v->obj.x = vertexPtr[0];
            v->obj.y = vertexPtr[1];
            v->obj.z = vertexPtr[2];
            v->obj.w = 1.0F;
        }

        if (!(hacc & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
            hacc |= (*gc->vertexArray.clipCheck)(gc, v);
        }

        v->hasAndClipCode = hacc;
    }
}
static void CompileElementsC(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *cp, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    cp = (const GLubyte *) gc->vertexArray.color_pointer +
                                first * gc->vertexArray.cp_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyColor)(gc, cp, v);
        v->hasAndClipCode = gc->vertexArray.validateMask;
        cp += gc->vertexArray.cp_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
        v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
    }
}
static void CompileElementsI(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *ip, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    ip = (const GLubyte *) gc->vertexArray.index_pointer +
                                first * gc->vertexArray.ip_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyIndex)(gc, ip, v);
        v->hasAndClipCode = gc->vertexArray.validateMask;
        ip += gc->vertexArray.ip_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
        v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
    }
}
static void CompileElementsT(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *tp, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer +
                                first * gc->vertexArray.tp_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyTexCoord)(gc, tp, v);
        v->hasAndClipCode = gc->vertexArray.validateMask;
        tp += gc->vertexArray.tp_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
        v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
    }
}

static void CompileElements_T2F_V3F(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    int off = gc->vertexArray.vp_stride * first;
    const GLubyte *tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer;
    const GLubyte *vp = (const GLubyte *) gc->vertexArray.vertex_pointer;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        __GLfloat *texCoordPtr = (GLfloat *) (tp + off);
        __GLfloat *vertexPtr = (GLfloat *) (vp + off);

        off += gc->vertexArray.vp_stride;
        v->hasAndClipCode = gc->vertexArray.validateMask;
        v->texture[0].x = texCoordPtr[0];
        v->texture[0].y = texCoordPtr[1];
        v->texture[0].z = 0.0F;
        v->texture[0].w = 1.0F;
        v->obj.x = vertexPtr[0];
        v->obj.y = vertexPtr[1];
        v->obj.z = vertexPtr[2];
        v->obj.w = 1.0F;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
        v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
    }
}
static void CompileElementsNC(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *cp, *np, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    cp = (const GLubyte *) gc->vertexArray.color_pointer +
                                first * gc->vertexArray.cp_stride;
    np = (const GLubyte *) gc->vertexArray.normal_pointer +
                                first * gc->vertexArray.np_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyNormal)(gc, np, v);
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot = v->normal.x * (gc->transform.cullEye.x - v->obj.x) +
                      v->normal.y * (gc->transform.cullEye.y - v->obj.y) +
                      v->normal.z * (gc->transform.cullEye.z - v->obj.z);
            } else {
                dot = v->normal.x * gc->transform.cullEye.x +
                      v->normal.y * gc->transform.cullEye.y +
                      v->normal.z * gc->transform.cullEye.z;
            }
            v->hasAndClipCode = gc->vertexArray.cullCode[dot <= 0.0F] | gc->vertexArray.validateMask;
        } else {
            v->hasAndClipCode = gc->vertexArray.validateMask;
        }
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.copyColor)(gc, cp, v);
        }
        cp += gc->vertexArray.cp_stride;
        np += gc->vertexArray.np_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
            v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
        }
    }
}
static void CompileElementsNI(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *ip, *np, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    ip = (const GLubyte *) gc->vertexArray.index_pointer +
                                first * gc->vertexArray.ip_stride;
    np = (const GLubyte *) gc->vertexArray.normal_pointer +
                                first * gc->vertexArray.np_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyNormal)(gc, np, v);
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot = v->normal.x * (gc->transform.cullEye.x - v->obj.x) +
                      v->normal.y * (gc->transform.cullEye.y - v->obj.y) +
                      v->normal.z * (gc->transform.cullEye.z - v->obj.z);
            } else {
                dot = v->normal.x * gc->transform.cullEye.x +
                      v->normal.y * gc->transform.cullEye.y +
                      v->normal.z * gc->transform.cullEye.z;
            }
            v->hasAndClipCode = gc->vertexArray.cullCode[dot <= 0.0F] | gc->vertexArray.validateMask;
        } else {
            v->hasAndClipCode = gc->vertexArray.validateMask;
        }
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.copyIndex)(gc, ip, v);
        }
        ip += gc->vertexArray.ip_stride;
        np += gc->vertexArray.np_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
            v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
        }
    }
}
static void CompileElementsNT(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *tp, *np, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer +
                                first * gc->vertexArray.tp_stride;
    np = (const GLubyte *) gc->vertexArray.normal_pointer +
                                first * gc->vertexArray.np_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyNormal)(gc, np, v);
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot = v->normal.x * (gc->transform.cullEye.x - v->obj.x) +
                      v->normal.y * (gc->transform.cullEye.y - v->obj.y) +
                      v->normal.z * (gc->transform.cullEye.z - v->obj.z);
            } else {
                dot = v->normal.x * gc->transform.cullEye.x +
                      v->normal.y * gc->transform.cullEye.y +
                      v->normal.z * gc->transform.cullEye.z;
            }
            v->hasAndClipCode = gc->vertexArray.cullCode[dot <= 0.0F] | gc->vertexArray.validateMask;
        } else {
            v->hasAndClipCode = gc->vertexArray.validateMask;
        }
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.copyTexCoord)(gc, tp, v);
        }
        tp += gc->vertexArray.tp_stride;
        np += gc->vertexArray.np_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
            v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
        }
    }
}
static void CompileElements_T2F_N3F_V3F(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    int off = gc->vertexArray.vp_stride * first;
    const GLubyte *tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer;
    const GLubyte *np = (const GLubyte *) gc->vertexArray.normal_pointer;
    const GLubyte *vp = (const GLubyte *) gc->vertexArray.vertex_pointer;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        __GLfloat *texCoordPtr = (GLfloat *) (tp + off);
        __GLfloat *normalPtr = (GLfloat *) (np + off);
        __GLfloat *vertexPtr = (GLfloat *) (vp + off);

        off += gc->vertexArray.vp_stride;
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot = normalPtr[0] * (gc->transform.cullEye.x - vertexPtr[0]) +
                      normalPtr[1] * (gc->transform.cullEye.y - vertexPtr[1]) +
                      normalPtr[2] * (gc->transform.cullEye.z - vertexPtr[2]);
            } else {
                dot = normalPtr[0] * gc->transform.cullEye.x +
                      normalPtr[1] * gc->transform.cullEye.y +
                      normalPtr[2] * gc->transform.cullEye.z;
            }
            v->hasAndClipCode = gc->vertexArray.cullCode[dot <= 0.0F] | gc->vertexArray.validateMask;
        } else {
            v->hasAndClipCode = gc->vertexArray.validateMask;
        }
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            v->texture[0].x = texCoordPtr[0];
            v->texture[0].y = texCoordPtr[1];
            v->texture[0].z = 0.0F;
            v->texture[0].w = 1.0F;
            v->normal.x = normalPtr[0];
            v->normal.y = normalPtr[1];
            v->normal.z = normalPtr[2];
            v->obj.x = vertexPtr[0];
            v->obj.y = vertexPtr[1];
            v->obj.z = vertexPtr[2];
            v->obj.w = 1.0F;
        }
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
            v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
        }
    }
}
static void CompileElementsCT(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *cp, *tp, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    cp = (const GLubyte *) gc->vertexArray.color_pointer +
                                first * gc->vertexArray.cp_stride;
    tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer +
                                first * gc->vertexArray.tp_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyTexCoord)(gc, tp, v);
        (*gc->vertexArray.copyColor)(gc, cp, v);
        v->hasAndClipCode = gc->vertexArray.validateMask;
        cp += gc->vertexArray.cp_stride;
        tp += gc->vertexArray.tp_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
        v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
    }
}
static void CompileElementsIT(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *ip, *tp, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    ip = (const GLubyte *) gc->vertexArray.index_pointer +
                                first * gc->vertexArray.ip_stride;
    tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer +
                                first * gc->vertexArray.tp_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyTexCoord)(gc, tp, v);
        (*gc->vertexArray.copyIndex)(gc, ip, v);
        v->hasAndClipCode = gc->vertexArray.validateMask;
        ip += gc->vertexArray.ip_stride;
        tp += gc->vertexArray.tp_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
        v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
    }
}
static void CompileElementsNCT(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *cp, *tp, *np, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    cp = (const GLubyte *) gc->vertexArray.color_pointer +
                                first * gc->vertexArray.cp_stride;
    tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer +
                                first * gc->vertexArray.tp_stride;
    np = (const GLubyte *) gc->vertexArray.normal_pointer +
                                first * gc->vertexArray.np_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyNormal)(gc, np, v);
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot = v->normal.x * (gc->transform.cullEye.x - v->obj.x) +
                      v->normal.y * (gc->transform.cullEye.y - v->obj.y) +
                      v->normal.z * (gc->transform.cullEye.z - v->obj.z);
            } else {
                dot = v->normal.x * gc->transform.cullEye.x +
                      v->normal.y * gc->transform.cullEye.y +
                      v->normal.z * gc->transform.cullEye.z;
            }
            v->hasAndClipCode = gc->vertexArray.cullCode[dot <= 0.0F] | gc->vertexArray.validateMask;
        } else {
            v->hasAndClipCode = gc->vertexArray.validateMask;
        }
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.copyTexCoord)(gc, tp, v);
            (*gc->vertexArray.copyColor)(gc, cp, v);
        }
        cp += gc->vertexArray.cp_stride;
        tp += gc->vertexArray.tp_stride;
        np += gc->vertexArray.np_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
            v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
        }
    }
}
static void CompileElementsNIT(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *ip, *tp, *np, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    ip = (const GLubyte *) gc->vertexArray.index_pointer +
                                first * gc->vertexArray.ip_stride;
    tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer +
                                first * gc->vertexArray.tp_stride;
    np = (const GLubyte *) gc->vertexArray.normal_pointer +
                                first * gc->vertexArray.np_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyNormal)(gc, np, v);
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot = v->normal.x * (gc->transform.cullEye.x - v->obj.x) +
                      v->normal.y * (gc->transform.cullEye.y - v->obj.y) +
                      v->normal.z * (gc->transform.cullEye.z - v->obj.z);
            } else {
                dot = v->normal.x * gc->transform.cullEye.x +
                      v->normal.y * gc->transform.cullEye.y +
                      v->normal.z * gc->transform.cullEye.z;
            }
            v->hasAndClipCode = gc->vertexArray.cullCode[dot <= 0.0F] | gc->vertexArray.validateMask;
        } else {
            v->hasAndClipCode = gc->vertexArray.validateMask;
        }
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.copyTexCoord)(gc, tp, v);
            (*gc->vertexArray.copyIndex)(gc, ip, v);
        }
        ip += gc->vertexArray.ip_stride;
        tp += gc->vertexArray.tp_stride;
        np += gc->vertexArray.np_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
            v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
        }
    }
}
static void CompileElementsAll(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *cp, *ip, *ep, *tp, *np, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    cp = (const GLubyte *) gc->vertexArray.color_pointer +
                                first * gc->vertexArray.cp_stride;
    ip = (const GLubyte *) gc->vertexArray.index_pointer +
                                first * gc->vertexArray.ip_stride;
    tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer +
                                first * gc->vertexArray.tp_stride;
    ep = (const GLubyte *) gc->vertexArray.edge_pointer +
                                first * gc->vertexArray.ep_stride;
    np = (const GLubyte *) gc->vertexArray.normal_pointer +
                                first * gc->vertexArray.np_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyNormal)(gc, np, v);
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot = v->normal.x * (gc->transform.cullEye.x - v->obj.x) +
                      v->normal.y * (gc->transform.cullEye.y - v->obj.y) +
                      v->normal.z * (gc->transform.cullEye.z - v->obj.z);
            } else {
                dot = v->normal.x * gc->transform.cullEye.x +
                      v->normal.y * gc->transform.cullEye.y +
                      v->normal.z * gc->transform.cullEye.z;
            }
            v->hasAndClipCode = gc->vertexArray.cullCode[dot <= 0.0F] | gc->vertexArray.validateMask;
        } else {
            v->hasAndClipCode = gc->vertexArray.validateMask;
        }
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.copyEdgeFlag)(gc, ep, v);
            (*gc->vertexArray.copyTexCoord)(gc, tp, v);
            (*gc->vertexArray.copyIndex)(gc, ip, v);
            (*gc->vertexArray.copyColor)(gc, cp, v);
        }
        cp += gc->vertexArray.cp_stride;
        ip += gc->vertexArray.ip_stride;
        tp += gc->vertexArray.tp_stride;
        ep += gc->vertexArray.ep_stride;
        np += gc->vertexArray.np_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        if (!(v->hasAndClipCode & __GL_ALL_CLIP_MASK)) {
            (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
            v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
        }
    }
}
static void CompileElementsAllNoCull(__GLcontext *gc, GLint offset, GLint first, GLsizei count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    const GLubyte *cp, *ip, *ep, *tp, *np, *vp;
    int i;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    cp = (const GLubyte *) gc->vertexArray.color_pointer +
                                first * gc->vertexArray.cp_stride;
    ip = (const GLubyte *) gc->vertexArray.index_pointer +
                                first * gc->vertexArray.ip_stride;
    tp = (const GLubyte *) gc->vertexArray.tex_coord_pointer +
                                first * gc->vertexArray.tp_stride;
    ep = (const GLubyte *) gc->vertexArray.edge_pointer +
                                first * gc->vertexArray.ep_stride;
    np = (const GLubyte *) gc->vertexArray.normal_pointer +
                                first * gc->vertexArray.np_stride;
    vp = (const GLubyte *) gc->vertexArray.vertex_pointer +
                                first * gc->vertexArray.vp_stride;

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.copyVertex)(gc, vp, v);
        (*gc->vertexArray.copyNormal)(gc, np, v);
        (*gc->vertexArray.copyEdgeFlag)(gc, ep, v);
        (*gc->vertexArray.copyTexCoord)(gc, tp, v);
        (*gc->vertexArray.copyIndex)(gc, ip, v);
        (*gc->vertexArray.copyColor)(gc, cp, v);
        v->hasAndClipCode = gc->vertexArray.validateMask;
        cp += gc->vertexArray.cp_stride;
        ip += gc->vertexArray.ip_stride;
        tp += gc->vertexArray.tp_stride;
        ep += gc->vertexArray.ep_stride;
        np += gc->vertexArray.np_stride;
        vp += gc->vertexArray.vp_stride;
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
        v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v) | __GL_HAS_CLIP;
    }
}

/* wrapper for compileElements when it needs to deal with element indexes */
static void CompileElementsIndexed(__GLcontext *gc, int offset, int first, int count, GLuint *elements)
{
    if (elements) {
        int i;

        for (i=0; i<count; ++i) {
            (*gc->vertexArray.compileElements)(gc, offset+i, elements[first+i], 1);
        }
    } else {
        
                (*gc->vertexArray.compileElements)(gc, offset, first, count);
    }
}

/* wrapper for compileElements when it needs to deal with silhouette vertexes */
static void CompileElementsSilhouette(__GLcontext *gc, __GLvertex *v, GLint index)
{
    int offset = v - gc->vertexArray.varrayPtr;
    GLuint prev_code = v->hasAndClipCode;

    (*gc->vertexArray.compileElementsNoCull)(gc, offset, index, 1);
    v->hasAndClipCode |= (prev_code & __GL_CLIP_VX_CULL);
}

#if 1

/* This is used only by the vertex cache! */
void __glCompileElements_NoCopy(__GLcontext *gc, int offset, int first, int count)
{
    __GLtransform *tr = gc->transform.modelView; 
    __GLvertex *v, *vb = gc->vertexArray.varrayPtr + offset;
    int i;
    GLuint hacc;

    if (tr->updateInverse) {
        (*gc->procs.computeInverseTranspose)(gc, tr);
    }

    for (i=0, v=vb; i<count; ++i, ++v) {
        hacc = v->hasAndClipCode;
        if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) {
            __GLfloat dot;
            if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL_LOCAL) {
                dot =   v->normal.x * (gc->transform.cullEye.x - v->obj.x) + 
                        v->normal.y * (gc->transform.cullEye.y - v->obj.y) +
                        v->normal.z * (gc->transform.cullEye.z - v->obj.z);
            } else {
                dot =   v->normal.x * gc->transform.cullEye.x + 
                        v->normal.y * gc->transform.cullEye.y +
                        v->normal.z * gc->transform.cullEye.z;
            }
            hacc |= (gc->vertexArray.cullCode[dot <= 0.0F] | 
                     gc->vertexArray.validateMask);
        } else {
            hacc |= gc->vertexArray.validateMask;
        }

        if ((hacc & __GL_ALL_CLIP_MASK) == 0) {
          (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
          hacc |= (*gc->vertexArray.clipCheck)(gc, v);
        }

        v->hasAndClipCode = hacc;
    }
}

#endif                  // }

/* This is used only by the vertex cache! */
void __glCompileElementsSilhouette_NoCopy(__GLcontext *gc, __GLvertex *v, GLint index)
{
    __GLtransform *tr = gc->transform.modelView; 

    (*gc->vertexArray.xf)(&v->clip, &v->obj.x, &tr->mvp);
    v->hasAndClipCode |= (*gc->vertexArray.clipCheck)(gc, v);
}

#pragma optimize("", on)

#define __NORMAL 1
#define __COLOR 2
#define __TEX 4

static void (*RGBCompileProcs[8])(__GLcontext*, GLint, GLint, GLsizei) = {
    CompileElements,    /* none */
    CompileElementsN,   /* NORMAL */

    CompileElementsC,   /* COLOR */
    CompileElementsNC,  /* NORMAL | COLOR */

    CompileElementsT,   /* TEX */
    CompileElementsNT,  /* NORMAL | TEX */

    CompileElementsCT,  /* COLOR | TEX */
    CompileElementsNCT, /* NORMAL | COLOR | TEX */
};

static void (*CICompileProcs[8])(__GLcontext*, GLint, GLint, GLsizei) = {
    CompileElements,    /* none */
    CompileElementsN,   /* NORMAL */
    CompileElementsI,   /* COLOR */
    CompileElementsNI,  /* NORMAL | COLOR */

    CompileElementsT,   /* TEX */
    CompileElementsNT,  /* NORMAL | TEX */

    CompileElementsIT,  /* COLOR | TEX */
    CompileElementsNIT, /* NORMAL | COLOR | TEX */
};

static void (*RGBInterleavedCompileProcs[8])(__GLcontext*, GLint, GLint, GLsizei) = {
    CompileElements,            /* none */
    CompileElements_N3F_V3F,    /* NORMAL */

    CompileElementsC,           /* COLOR */
    CompileElementsNC,          /* NORMAL | COLOR */

    CompileElements_T2F_V3F,    /* TEX */
    CompileElements_T2F_N3F_V3F,/* NORMAL | TEX */

    CompileElementsCT,          /* COLOR | TEX */
    CompileElementsNCT,         /* NORMAL | COLOR | TEX */
};

static void (*CIInterleavedCompileProcs[8])(__GLcontext*, GLint, GLint, GLsizei) = {
    CompileElements,            /* none */
    CompileElements_N3F_V3F,    /* NORMAL */
    CompileElementsI,           /* COLOR */
    CompileElementsNI,          /* NORMAL | COLOR */

    CompileElements_T2F_V3F,    /* TEX */
    CompileElements_T2F_N3F_V3F,/* NORMAL | TEX */

    CompileElementsIT,          /* COLOR | TEX */
    CompileElementsNIT,         /* NORMAL | COLOR | TEX */
};

void __glGenericPickVertexArrayEnables(__GLcontext *gc)
{
    __GLVertArrayMachine *va = &gc->vertexArray;
    GLbitfield interleavedMask = 0;

    /*
    ** Pick transform/clipcheck/validate procs
    */
    switch (va->vp_size) {
    case 2:
        va->xf = gc->transform.modelView->mvp.xf2;
        va->clipCheck = gc->procs.clipCheck2;
        va->validateVertex = gc->procs.validateVertex2;
        va->validateMask = __GL_HAS_VERTEX_2D;
        break;
    case 3:
        va->xf = gc->transform.modelView->mvp.xf3;
        va->clipCheck = gc->procs.clipCheck3;
        va->validateVertex = gc->procs.validateVertex3;
        va->validateMask = __GL_HAS_VERTEX_3D;
        break;
    case 4:
        va->xf = gc->transform.modelView->mvp.xf4;
        va->clipCheck = gc->procs.clipCheck4;
        va->validateVertex = gc->procs.validateVertex4;
        va->validateMask = __GL_HAS_VERTEX_4D;
        break;
    default:
        assert(0);
        break;
    }

    if (va->compileMask & va->mask & VERTARRAY_V_MASK) {
        va->copyVertex = va->vp_copy;
        if (va->copyVertex == CopyVertex3f) {
            interleavedMask |= VERTARRAY_V_MASK;
        }
        va->controlWord &= ~VERTARRAY_CW_NO_VERTEX;
    } else {
        va->controlWord |= VERTARRAY_CW_NO_VERTEX;
    }

    if (va->compileMask & va->mask & VERTARRAY_N_MASK) {
        va->copyNormal = va->np_copy;
        if ((va->copyNormal == CopyNormal3f) && (va->np_stride == va->vp_stride)) {
            interleavedMask |= VERTARRAY_N_MASK;
        }
    } else {
        va->copyNormal = CopyCurrentNormal;
    }

    if (va->compileMask & va->mask & VERTARRAY_C_MASK) {
        va->copyColor = va->cp_copy;
        if ((va->copyColor == CopyColor3f) && (va->cp_stride == va->vp_stride)) {
            interleavedMask |= (VERTARRAY_C_MASK | VERTARRAY_I_MASK);
        }
    } else {
        va->copyColor = CopyCurrentColor;
    }

    if (va->compileMask & va->mask & VERTARRAY_I_MASK) {
        va->copyIndex = va->ip_copy;
        if ((va->copyIndex == CopyIndexf) && (va->ip_stride == va->vp_stride)) {
            interleavedMask |= (VERTARRAY_C_MASK | VERTARRAY_I_MASK);
        }
    } else {
        va->copyIndex = CopyCurrentIndex;
    }

    if (va->compileMask & va->mask & VERTARRAY_T_MASK) {
        va->copyTexCoord = va->tp_copy;
        if ((va->copyTexCoord == CopyTexCoord2f) && (va->tp_stride == va->vp_stride)) {
            interleavedMask |= VERTARRAY_T_MASK;
        }
    } else {
        va->copyTexCoord = CopyCurrentTexCoord;
    }

    if (va->compileMask & va->mask & VERTARRAY_E_MASK) {
        va->copyEdgeFlag = va->ep_copy;
    } else {
        va->copyEdgeFlag = CopyCurrentEdgeFlag;
    }

    if (va->compileIndex < 0) {
        va->compileElements = CompileElementsAll;
    } else {
        if (va->compileMask == interleavedMask) {
            if (gc->modes.rgbMode) {
                va->compileElements =
                        RGBInterleavedCompileProcs[va->compileIndex];
            } else {
                va->compileElements =
                        CIInterleavedCompileProcs[va->compileIndex];
            }
        } else {
            if (gc->modes.rgbMode) {
                va->compileElements = RGBCompileProcs[va->compileIndex];
            } else {
                va->compileElements = CICompileProcs[va->compileIndex];
            }
        }
    }

    if (va->controlWord & VERTARRAY_CW_LOCKED) {
        /* resize vertex buffer to allow entrire array to be precompiled */
        if (AllocateVbuf(gc, va->start, va->count)) {
            /* vertex buffer resized okay, allow precompile */
            va->controlWord |= VERTARRAY_CW_ALLOW_PRECOMPILE;
        } else {
            /* vertex buffer resize failed, don't allow precompile */
            va->controlWord &= ~VERTARRAY_CW_ALLOW_PRECOMPILE;
        }
    } else {
        /* restore vertex buffer (possibly free up some memory) */
        (void) AllocateVbuf(gc, 0, 0);
        va->controlWord &= ~VERTARRAY_CW_ALLOW_PRECOMPILE;
    }

    /* Vertex state always needs to be recompiled after validation */
    va->controlWord |= VERTARRAY_CW_NEEDS_COMPILE;
#if __GL_CODEGEN
    if (gc->renderMode == GL_RENDER) {
        va->compileElements =
#if NEW_OG_KEY
            __glSSTGenerateCompile(gc, va->vp_size, 
                                   va->compileIndex & (__GL_GEOM_OG_TEX |
                                                       __GL_GEOM_OG_COLOR));
#else
            GenerateCompile(gc, va->vp_size, 
                            va->compileIndex & (__GL_GEOM_OG_TEX |
                                                __GL_GEOM_OG_COLOR));
#endif
    }
#endif
}

void __glGenericPickVertexArrayProcs(__GLcontext *gc)
{
    __GLVertArrayMachine *va = &gc->vertexArray;
    GLuint enables = gc->state.enables.general;
    GLuint texenables = gc->state.enables.texture[gc->texture.currentTexUnit];

    /*
    ** Check for unsupported modes and that the fastpath isn't disabled.
    ** TBD-- fix colorMaterial for the fast paths.
    */
    if ((va->controlWord & VERTARRAY_CW_DISABLE_FAST) ||
        ((enables & __GL_LIGHTING_ENABLE) &&
         (enables & __GL_COLOR_MATERIAL_ENABLE)))
    {
        va->controlWord |= VERTARRAY_CW_SLOWPATH;
    } else {
        va->controlWord &= ~VERTARRAY_CW_SLOWPATH;
    }

    /*
    ** Set up for vertex culling.
    */
    if (enables & __GL_CULL_VERTEX_ENABLE) {
        if (gc->state.transform.objEyeSpecified) {
            if (gc->state.transform.eyePosObj.w != 0.0F) {
                va->controlWord |= VERTARRAY_CW_VX_CULL_LOCAL;
            } else {
                va->controlWord &= ~VERTARRAY_CW_VX_CULL_LOCAL;
            }
        } else {
            if (gc->state.transform.eyePos.w != 0.0F) {
                va->controlWord |= VERTARRAY_CW_VX_CULL_LOCAL;
            } else {
                va->controlWord &= ~VERTARRAY_CW_VX_CULL_LOCAL;
            }
        }

        switch (gc->state.polygon.cull) {
        case GL_FRONT:
            va->cullCode[__GL_FRONTFACE] = __GL_CLIP_VX_CULL;
            va->cullCode[__GL_BACKFACE] = 0;
            break;
        case GL_BACK:
            va->cullCode[__GL_FRONTFACE] = 0;
            va->cullCode[__GL_BACKFACE] = __GL_CLIP_VX_CULL;
            break;
        case GL_FRONT_AND_BACK:
            va->cullCode[__GL_FRONTFACE] = __GL_CLIP_VX_CULL;
            va->cullCode[__GL_BACKFACE] = __GL_CLIP_VX_CULL;
            break;
        default:
            assert(0);
            break;
        }

        va->controlWord |= VERTARRAY_CW_VX_CULL;
    } else {
        va->controlWord &= ~(VERTARRAY_CW_VX_CULL | VERTARRAY_CW_VX_CULL_LOCAL);
    }

    /*
    ** Pick compile procs
    */
    if ((gc->renderMode == GL_FEEDBACK) ||
        (gc->state.polygon.frontMode != GL_FILL) ||
        (gc->state.polygon.backMode != GL_FILL))
    {
        /* Need everything */
        va->compileMask =
            VERTARRAY_V_MASK | VERTARRAY_N_MASK |
            VERTARRAY_C_MASK | VERTARRAY_I_MASK |
            VERTARRAY_T_MASK | VERTARRAY_E_MASK;
        va->compileIndex = -1;
    } else {
        va->compileMask = VERTARRAY_V_MASK;
        va->compileIndex = 0;
        if (enables & __GL_LIGHTING_ENABLE) {
            va->compileMask |= VERTARRAY_N_MASK;
            va->compileIndex |= __NORMAL;
        } else {
            va->compileMask |= (VERTARRAY_C_MASK | VERTARRAY_I_MASK);
            va->compileIndex |= __COLOR;
        }
        if (enables & __GL_CULL_VERTEX_ENABLE) {
            va->compileMask |= VERTARRAY_N_MASK;
            va->compileIndex |= __NORMAL;
        }
        if (gc->texture.textureEnabled) {
            va->compileMask |= VERTARRAY_T_MASK;
            va->compileIndex |= __TEX;
            if (gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_S_ENABLE) {
                if (gc->state.texture[gc->texture.currentTexUnit].s.mode == GL_SPHERE_MAP) {
                    va->compileMask |= VERTARRAY_N_MASK;
                    va->compileIndex |= __NORMAL;
                }
            }
            if (gc->state.enables.texture[gc->texture.currentTexUnit] & __GL_TEXTURE_GEN_T_ENABLE) {
                if (gc->state.texture[gc->texture.currentTexUnit].t.mode == GL_SPHERE_MAP) {
                    va->compileMask |= VERTARRAY_N_MASK;
                    va->compileIndex |= __NORMAL;
                }
            }
        }
    }

    va->compileElementsIndexed = CompileElementsIndexed;
    va->compileElementsNoCull = CompileElementsAllNoCull;
    va->compileElementsSilhouette = CompileElementsSilhouette;

    __glGenericPickVertexArrayEnables(gc);
}

/************************************************************************/

static GLboolean AllocateVbuf(__GLcontext *gc, int first, int count)
{
    int bufSize, currentBufSize;

    bufSize = count * sizeof(__GLvertex);
    currentBufSize = gc->vertexArray.varrayBufSize;

    if (bufSize > currentBufSize) {
        int blockSize = gc->vertexArray.blockSize;
        __GLvertex *vBuf = gc->vertexArray.varrayBuf;

        /* smallest # of blocks that will contain <count> vertexes */
        bufSize = ((bufSize + (blockSize-1)) / blockSize) * blockSize;

        /* Align to 32byte cache line */
        bufSize += 32;

        /* allocate buffer */
        if (currentBufSize && vBuf) {
            vBuf = (__GLvertex *)
                (*gc->imports.realloc)(gc, vBuf, (size_t)bufSize);
        } else {
            vBuf = (__GLvertex *)
                (*gc->imports.malloc)(gc, (size_t)bufSize);
        }

        /* This breaks 'realloc' and 'free' later */
#if 0
        /* Align to 32byte cache line */
        vBuf = (__GLvertex *)(((unsigned int)vBuf & 0xFFFFFFE0UL) + 32);
#endif

        gc->vertexArray.varrayBuf = vBuf;
        gc->vertexArray.varrayBufSize = bufSize;

        /* Initialize color pointers */
        {
            __GLvertex *vx;
            int i, n = bufSize / sizeof(__GLvertex);

            for (i=0, vx = vBuf; i<n; i++, vx++) {
                vx->color = &vx->colors[__GL_FRONTFACE];
            }
        }

        /* Update vcache pointer */
        gc->vertexCache.vertexCache = gc->vertexArray.varrayBuf;
    }

    /*
    ** If <first> is not zero, then the vertex array data is locked
    ** starting at a non-zero offset.  We could allocate <first>+<count>
    ** elements, but that seems wasteful.  Instead we offset the
    ** array pointer.
    */
    if (first) {
        gc->vertexArray.varrayPtr = gc->vertexArray.varrayBuf - first;
    } else {
        gc->vertexArray.varrayPtr = gc->vertexArray.varrayBuf;
    }

    return GL_TRUE;
}

void APIENTRY __glim_LockArraysSGI(GLint start, GLint count)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (start < 0 || count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (gc->vertexArray.controlWord & VERTARRAY_CW_LOCKED) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __GL_API_BLAND();

    gc->vertexArray.controlWord |= VERTARRAY_CW_LOCKED;
    gc->vertexArray.start = start;
    gc->vertexArray.count = count;

    /*
    ** Compilation is delayed until validation so it catches all of the 
    ** current state at glBegin.
    */
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
}

void APIENTRY __glim_UnlockArraysSGI(void)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (!(gc->vertexArray.controlWord & VERTARRAY_CW_LOCKED)) {
        __glSetError(GL_INVALID_OPERATION);
        return;
    }

    __GL_API_BLAND();

    gc->vertexArray.controlWord &= ~VERTARRAY_CW_LOCKED;
    gc->vertexArray.start = gc->vertexArray.count = 0;

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
}

typedef struct __GLinterleavedFormatRec {
    GLboolean enableTexCoord;
    GLboolean enableColor;
    GLboolean enableIndex;
    GLboolean enableNormal;
    GLint sizeTexCoord;
    GLint sizeColor;
    GLint sizeVertex;
    GLenum typeColor;
    GLint offsetColor;
    GLint offsetIndex;
    GLint offsetNormal;
    GLint offsetVertex;
    GLint stride;
} __GLinterleavedFormat;

#define __GL_SIZE_F \
        sizeof(GLfloat)

#define __GL_SIZE_C \
        (((4*sizeof(GLubyte) + (__GL_SIZE_F-1)) / __GL_SIZE_F) * __GL_SIZE_F)

#define __GL_SIZE_I \
        (((sizeof(GLuint) + (__GL_SIZE_F-1)) / __GL_SIZE_F) * __GL_SIZE_F)

static const __GLinterleavedFormat interleavedFormats[] = {
    {   /* V2F */
        GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, 0, 0, 2, 0,
        0, 0, 0, 0,
        2*__GL_SIZE_F,
    },
    {   /* V3F */
        GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE, 0, 0, 3, 0,
        0, 0, 0, 0,
        3*__GL_SIZE_F,
    },
    {   /* C4UB_V2F */
        GL_FALSE,  GL_TRUE, GL_FALSE, GL_FALSE, 0, 4, 2, GL_UNSIGNED_BYTE,
        0, 0, 0, __GL_SIZE_C,
        __GL_SIZE_C+2*__GL_SIZE_F,
    },
    {   /* C4UB_V3F */
        GL_FALSE,  GL_TRUE, GL_FALSE, GL_FALSE, 0, 4, 3, GL_UNSIGNED_BYTE,
        0, 0, 0, __GL_SIZE_C,
        __GL_SIZE_C+3*__GL_SIZE_F,
    },
    {   /* C3F_V3F */
        GL_FALSE,  GL_TRUE, GL_FALSE, GL_FALSE, 0, 3, 3, GL_FLOAT,
        0, 0, 0, 3*__GL_SIZE_F,
        6*__GL_SIZE_F,
    },
    {   /* N3F_V3F */
        GL_FALSE, GL_FALSE, GL_FALSE,  GL_TRUE, 0, 0, 3, 0,
        0, 0, 0, 3*__GL_SIZE_F,
        6*__GL_SIZE_F,
    },
    {   /* C4F_N3F_V3F */
        GL_FALSE,  GL_TRUE, GL_FALSE,  GL_TRUE, 0, 4, 3, GL_FLOAT,
        0, 0, 4*__GL_SIZE_F, 7*__GL_SIZE_F,
        10*__GL_SIZE_F,
    },
    {   /* T2F_V3F */
         GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, 2, 0, 3, 0,
        0, 0, 0, 2*__GL_SIZE_F,
        5*__GL_SIZE_F,
    },
    {   /* T4F_V4F */
         GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE, 4, 0, 4, 0,
        0, 0, 0, 4*__GL_SIZE_F,
        8*__GL_SIZE_F,
    },
    {   /* T2F_C4UB_V3F */
         GL_TRUE,  GL_TRUE, GL_FALSE, GL_FALSE, 2, 4, 3, GL_UNSIGNED_BYTE,
        2*__GL_SIZE_F, 0, 0, __GL_SIZE_C+2*__GL_SIZE_F,
        __GL_SIZE_C+5*__GL_SIZE_F,
    },
    {   /* T2F_C3F_V3F */
         GL_TRUE,  GL_TRUE, GL_FALSE, GL_FALSE, 2, 3, 3, GL_FLOAT,
        2*__GL_SIZE_F, 0, 0, 5*__GL_SIZE_F, 8*__GL_SIZE_F,
    },
    {   /* T2F_N3F_V3F */
         GL_TRUE, GL_FALSE, GL_FALSE,  GL_TRUE, 2, 0, 3, 0,
        0, 0, 2*__GL_SIZE_F, 5*__GL_SIZE_F,
        8*__GL_SIZE_F,
    },
    {   /* T2F_C4F_N3F_V3F */
         GL_TRUE,  GL_TRUE, GL_FALSE,  GL_TRUE, 2, 4, 3, GL_FLOAT,
        2*__GL_SIZE_F, 0, 6*__GL_SIZE_F, 9*__GL_SIZE_F,
        12*__GL_SIZE_F,
    },
    {   /* T4F_C4F_N3F_V4F */
         GL_TRUE,  GL_TRUE, GL_FALSE,  GL_TRUE, 4, 4, 4, GL_FLOAT,
        4*__GL_SIZE_F, 0, 8*__GL_SIZE_F, 11*__GL_SIZE_F,
        15*__GL_SIZE_F,
    },
    {   /* IUI_V2F */
        GL_FALSE, GL_FALSE,  GL_TRUE, GL_FALSE, 0, 0, 2, 0,
        0, 0, 0, __GL_SIZE_I,
        __GL_SIZE_I+2*__GL_SIZE_F,
    },
    {   /* IUI_V3F */
        GL_FALSE, GL_FALSE,  GL_TRUE, GL_FALSE, 0, 0, 3, 0,
        0, 0, 0, __GL_SIZE_I,
        __GL_SIZE_I+3*__GL_SIZE_F,
    },
    {   /* IUI_N3F_V2F */
        GL_FALSE, GL_FALSE,  GL_TRUE,  GL_TRUE, 0, 0, 2, 0,
        0, 0, __GL_SIZE_I, __GL_SIZE_I+3*__GL_SIZE_F,
        __GL_SIZE_I+5*__GL_SIZE_F,
    },
    {   /* IUI_N3F_V3F */
        GL_FALSE, GL_FALSE,  GL_TRUE,  GL_TRUE, 0, 0, 3, 0,
        0, 0, __GL_SIZE_I, __GL_SIZE_I+3*__GL_SIZE_F,
        __GL_SIZE_I+6*__GL_SIZE_F,
    },
    {   /* T2F_IUI_V2F */
         GL_TRUE, GL_FALSE,  GL_TRUE, GL_FALSE, 2, 0, 2, 0,
        0, 0, 2*__GL_SIZE_F, __GL_SIZE_I+2*__GL_SIZE_F,
        __GL_SIZE_I+4*__GL_SIZE_F,
    },
    {   /* T2F_IUI_V3F */
         GL_TRUE, GL_FALSE,  GL_TRUE, GL_FALSE, 2, 0, 3, 0,
        0, 2*__GL_SIZE_F, 0, __GL_SIZE_I+2*__GL_SIZE_F,
        __GL_SIZE_I+5*__GL_SIZE_F,
    },
    {   /* T2F_IUI_N3F_V2F */
         GL_TRUE, GL_FALSE,  GL_TRUE,  GL_TRUE, 2, 0, 2, 0,
        0, 2*__GL_SIZE_F, __GL_SIZE_I+2*__GL_SIZE_F, __GL_SIZE_I+5*__GL_SIZE_F,
        __GL_SIZE_I+7*__GL_SIZE_F,
    },
    {   /* T2F_IUI_N3F_V3F */
         GL_TRUE, GL_FALSE,  GL_TRUE,  GL_TRUE, 2, 0, 3, 0,
        0, 2*__GL_SIZE_F, __GL_SIZE_I+2*__GL_SIZE_F, __GL_SIZE_I+5*__GL_SIZE_F,
        __GL_SIZE_I+8*__GL_SIZE_F,
    },
};

void APIENTRY __glim_InterleavedArrays(GLenum format, GLsizei stride, 
                        const GLvoid *pointer)
{
    GLubyte *p = (GLubyte *) pointer;
    const __GLinterleavedFormat *f;

    __GL_SETUP_NOT_IN_BEGIN();

    if (stride < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if ((int) format >= GL_V2F &&
        (int) format <= GL_T4F_C4F_N3F_V4F) {
        f = &interleavedFormats[(int) format - (int) GL_V2F];

    } else if ((int) format >= GL_IUI_V2F_SGI &&
               (int) format <= GL_T2F_IUI_N3F_V3F_SGI) {
        f = &interleavedFormats[(int) format - (int) GL_IUI_V2F_SGI];

    } else {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_API_BLAND();

    if (stride == 0) stride = f->stride;

    glDisableClientState(GL_EDGE_FLAG_ARRAY);

    if (f->enableTexCoord) {
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(f->sizeTexCoord, GL_FLOAT, stride, p);
    } else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    if (f->enableColor) {
        glColorPointer(f->sizeColor, f->typeColor, stride, p+f->offsetColor);
        glEnableClientState(GL_COLOR_ARRAY);
    } else {
        glDisableClientState(GL_COLOR_ARRAY);
    }
    if (f->enableIndex) {
        glIndexPointer(GL_INT, stride, p+f->offsetIndex);
        glEnableClientState(GL_INDEX_ARRAY);
    } else {
        glDisableClientState(GL_INDEX_ARRAY);
    }
    if (f->enableNormal) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glNormalPointer(GL_FLOAT, stride, p+f->offsetNormal);
    } else {
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(f->sizeVertex, GL_FLOAT, stride, p+f->offsetVertex);

    gc->vertexArray.interleavedPointer = pointer;
    gc->vertexArray.interleavedFormat = format;

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
}

static struct {
    GLint staticCount;
    GLint linkCount;
} PrimModes[] = {
    { 0, 0 },   /* GL_POINTS */
    { 0, 0 },   /* GL_LINES */
    { 1, 2 },   /* GL_LINE_LOOP */
    { 0, 1 },   /* GL_LINE_STRIP */
    { 0, 0 },   /* GL_TRIANGLES */
    { 0, 2 },   /* GL_TRIANGL_STRIP */
    { 1, 2 },   /* GL_TRIANGLE_FAN */
    { 0, 0 },   /* GL_QUADS */
    { 0, 2 },   /* GL_QUAD_STRIP */
    { 1, 2 },   /* GL_POLYGON */
};

static GLuint
indexToBatchIndex(__GLcontext *gc, GLint index)
{
    __GLVertArrayMachine *va = &gc->vertexArray;

    if ((index >= PrimModes[va->batchMode].staticCount) &&
                                        ((va->batchIndex-va->batchFirst) > 0)) {
        index += va->batchIndex - PrimModes[va->batchMode].linkCount;
    } else {
        index += va->batchFirst;
    }

    if (va->batchElements) {
        return va->batchElements[index];
    } else {
        return index;
    }
}

static void
ClipLine(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1)
{
    {
        if ((v0->hasAndClipCode & __GL_HAS_CLIP) == 0) {
            GLint index = v0 - gc->vertexArray.varrayPtr;

            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                index = indexToBatchIndex(gc, index);
            }
            (*gc->vertexArray.compileElementsSilhouette)(gc, v0, index);
        }
        if ((v1->hasAndClipCode & __GL_HAS_CLIP) == 0) {
            GLint index = v1 - gc->vertexArray.varrayPtr;

            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                index = indexToBatchIndex(gc, index);
            }
            (*gc->vertexArray.compileElementsSilhouette)(gc, v1, index);
        }
    }

    if ((v0->hasAndClipCode & v1->hasAndClipCode & __GL_ALL_CLIP_MASK) == 0) {
        __glClipLine(gc, v1, v0);
    }
}

void
__glDoClipTriangle(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1, __GLvertex *v2)
{

    /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
    {
        if ((v0->hasAndClipCode & __GL_HAS_CLIP) == 0) {
            GLint index = v0 - gc->vertexArray.varrayPtr;

            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                index = indexToBatchIndex(gc, index);
            }
            (*gc->vertexArray.compileElementsSilhouette)(gc, v0, index);
        }
        if ((v1->hasAndClipCode & __GL_HAS_CLIP) == 0) {
            GLint index = v1 - gc->vertexArray.varrayPtr;

            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                index = indexToBatchIndex(gc, index);
            }
            (*gc->vertexArray.compileElementsSilhouette)(gc, v1, index);
        }
        if ((v2->hasAndClipCode & __GL_HAS_CLIP) == 0) {
            GLint index = v2 - gc->vertexArray.varrayPtr;

            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                index = indexToBatchIndex(gc, index);
            }
            (*gc->vertexArray.compileElementsSilhouette)(gc, v2, index);
        }
    }

    if ((v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & __GL_ALL_CLIP_MASK) == 0) {
        GLuint orCodes = (v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK;
        (*gc->procs.clipTriangle)(gc, v0, v1, v2, orCodes);
    }
}
static void
ClipQuad(__GLcontext *gc,
            __GLvertex *v0, __GLvertex *v1, __GLvertex *v2, __GLvertex *v3)
{
     {
        if ((v0->hasAndClipCode & __GL_HAS_CLIP) == 0) {
            GLint index = v0 - gc->vertexArray.varrayPtr;

            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                index = indexToBatchIndex(gc, index);
            }
            (*gc->vertexArray.compileElementsSilhouette)(gc, v0, index);
        }
        if ((v1->hasAndClipCode & __GL_HAS_CLIP) == 0) {
            GLint index = v1 - gc->vertexArray.varrayPtr;

            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                index = indexToBatchIndex(gc, index);
            }
            (*gc->vertexArray.compileElementsSilhouette)(gc, v1, index);
        }
        if ((v2->hasAndClipCode & __GL_HAS_CLIP) == 0) {
            GLint index = v2 - gc->vertexArray.varrayPtr;

            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                index = indexToBatchIndex(gc, index);
            }
            (*gc->vertexArray.compileElementsSilhouette)(gc, v2, index);
        }
        if ((v3->hasAndClipCode & __GL_HAS_CLIP) == 0) {
            GLint index = v3 - gc->vertexArray.varrayPtr;

            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                index = indexToBatchIndex(gc, index);
            }
            (*gc->vertexArray.compileElementsSilhouette)(gc, v3, index);
        }
    }

    if ((v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & v3->hasAndClipCode & __GL_ALL_CLIP_MASK) == 0) {
        GLuint orCodes = (v0->hasAndClipCode|v1->hasAndClipCode|v2->hasAndClipCode|v3->hasAndClipCode) & __GL_ALL_CLIP_MASK;
        __GLvertex *iv[4];
        iv[0] = v0; iv[1] = v1; iv[2] = v2; iv[3] = v3;
        __glDoPolygonClip(gc, &iv[0], 4, orCodes);
    }
}

void __glDrawVertexes_Tstrip2(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1, *v2;
    int i;

    if (count < 3) return;
    assert(count >= 3);
    assert((first == 0) || (elements == 0));

    v0 = vBuf + elements[0];
    v2 = vBuf + elements[1];
    //v0->boundaryEdge = GL_TRUE;
    //v2->boundaryEdge = GL_TRUE;

    for (i=2; i<count;) {
        if (i & 1) {
            v0 = v2;
        } else {
            v1 = v2;
        }
        v2 = vBuf + elements[i++];
        //v2->boundaryEdge = GL_TRUE;

        //gc->line.notResetStipple = GL_FALSE;
        //gc->vertex.provoking = v2;

        if (((v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
            (*gc->procs.renderTriangle)(gc, v0, v1, v2);
        //} else if (v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode) {
        //    continue; /* cull -or- trivial reject */
        //} else {
        //    __glDoClipTriangle(gc, v0, v1, v2);
        }
    }
}

#if __GL_SST_GLIDE_VTX

void __glDrawVertexes_Tstrip(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1, *v2;
    int batchFirst, batchIndex, batchCount, batchOffset;
    GLuint *batchElements;
    int i;

    if (count < 3) return;
    assert(count >= 3);
    assert((first == 0) || (elements == 0));


    batchFirst = batchIndex = first;
    batchCount = count;
    batchOffset = 0;
    batchElements = elements;

    {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
                batchCount = count;

              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset,
                                                        batchIndex,
                                                        batchCount,
                                                        elements);
        }

        v0 = vBuf + batchFirst;
        v2 = vBuf + batchFirst + 1;

        for (i=batchFirst+2; i<batchCount+batchOffset;) {
            if (i & 1) {
                v0 = v2;
            } else {
                v1 = v2;
            }
            v2 = vBuf + i++;

            gc->vertex.provoking = v2;

            if (((v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                (*gc->procs.renderTriangle)(gc, v0, v1, v2);
            } else if (v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                __glDoClipTriangle(gc, v0, v1, v2);
            }
        }
    }
}

#else

void __glDrawVertexes_Tstrip(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1, *v2;
    int batchFirst, batchIndex, batchCount, batchOffset;
    GLuint *batchElements;
    int i;

    if (count < 3) return;
    assert(count >= 3);
    assert((first == 0) || (elements == 0));


    batchFirst = batchIndex = first;
    batchCount = count;
    batchOffset = 0;
    batchElements = elements;

    do {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (batchFirst == batchIndex) {
                /* first batch */
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
                /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
                {
                    gc->vertexArray.batchMode = GL_TRIANGLE_STRIP;
                    gc->vertexArray.batchFirst = first;
                    gc->vertexArray.batchElements = elements;
                }
            } else {
                /* subsequent batch */
                vBuf[0] = vBuf[batchOffset+(batchCount-2)];
                vBuf[0].color = &vBuf[0].colors[__GL_FRONTFACE];
                vBuf[1] = vBuf[batchOffset+(batchCount-1)];
                vBuf[1].color = &vBuf[1].colors[__GL_FRONTFACE];
                batchOffset = 2;
            }
            if (count < batchCount) {
                batchCount = count;
            }
            if (gc->vertexArray.continuation)
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset + 2,
                                                        batchIndex,
                                                        batchCount - 2,
                                                        elements);
            else
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset,
                                                        batchIndex,
                                                        batchCount,
                                                        elements);
            gc->vertexArray.batchIndex = batchIndex;
        }


        if (batchElements) {
            v0 = vBuf + batchElements[0];
            v2 = vBuf + batchElements[1];
        } else {
            v0 = vBuf + batchFirst;
            v2 = vBuf + batchFirst + 1;
        }
        v0->boundaryEdge = GL_TRUE;
        v2->boundaryEdge = GL_TRUE;

        for (i=batchFirst+2; i<batchCount+batchOffset;) {
            if (i & 1) {
                v0 = v2;
            } else {
                v1 = v2;
            }
            if (batchElements) {
                v2 = vBuf + batchElements[i++];
            } else {
                v2 = vBuf + i++;
            }
            v2->boundaryEdge = GL_TRUE;

            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = v2;

            if (((v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                (*gc->procs.renderTriangle)(gc, v0, v1, v2);
            } else if (v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                __glDoClipTriangle(gc, v0, v1, v2);
            }
        }
        batchIndex += batchCount;
        count -= batchCount;
    } while (count > 0);
}

#endif

#if __GL_SST_GLIDE_VTX

void __glDrawVertexes_Tfan(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1, *v2;
    int batchFirst, batchIndex, batchCount, batchOffset;
    GLuint *batchElements;
    int i;

    if (count < 3) return;
    assert(count >= 3);
    assert((first == 0) || (elements == 0));

    batchFirst = batchIndex = first;
    batchCount = count;
    batchOffset = 0;
    batchElements = elements;

    {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                /* first batch */
                batchFirst = 0;
                batchCount = count;

                (*gc->vertexArray.compileElementsIndexed)(gc,
                                                          batchOffset,
                                                          batchIndex,
                                                          batchCount,
                                                          elements);
        }

        v0 = vBuf + batchFirst;
        v2 = vBuf + batchFirst + 1;

        for (i=batchFirst+2; i<batchCount+batchOffset;) {
            v1 = v2;
            if (batchElements) {
                v2 = vBuf + batchElements[i++];
            } else {
                v2 = vBuf + i++;
            }
            gc->vertex.provoking = v2;

            if (((v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                (*gc->procs.renderTriangle)(gc, v0, v1, v2);
            } else if (v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                __glDoClipTriangle(gc, v0, v1, v2);
            }
        }
    }
}

#else

void __glDrawVertexes_Tfan(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1, *v2;
    int batchFirst, batchIndex, batchCount, batchOffset;
    GLuint *batchElements;
    int i;

    if (count < 3) return;
    assert(count >= 3);
    assert((first == 0) || (elements == 0));

    batchFirst = batchIndex = first;
    batchCount = count;
    batchOffset = 0;
    batchElements = elements;

    do {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (batchFirst == batchIndex) {
                /* first batch */
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
                /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
                {
                    gc->vertexArray.batchMode = GL_TRIANGLE_FAN;
                    gc->vertexArray.batchFirst = first;
                    gc->vertexArray.batchElements = elements;
                }
            } else {
                /* subsequent batch */
                vBuf[1] = vBuf[batchOffset+(batchCount-1)];
                vBuf[1].color = &vBuf[1].colors[__GL_FRONTFACE];
                batchOffset = 2;
            }
            if (count < batchCount) {
                batchCount = count;
            }
            if (gc->vertexArray.continuation)
                (*gc->vertexArray.compileElementsIndexed)(gc,
                                                          batchOffset + 2,
                                                          batchIndex,
                                                          batchCount - 2,
                                                          elements);
            else
                (*gc->vertexArray.compileElementsIndexed)(gc,
                                                          batchOffset,
                                                          batchIndex,
                                                          batchCount,
                                                          elements);
            gc->vertexArray.batchIndex = batchIndex;
        }

        if (batchElements) {
            v0 = vBuf + batchElements[0];
            v2 = vBuf + batchElements[1];
        } else {
            v0 = vBuf + batchFirst;
            v2 = vBuf + batchFirst + 1;
        }
        v0->boundaryEdge = GL_TRUE;
        v2->boundaryEdge = GL_TRUE;

        for (i=batchFirst+2; i<batchCount+batchOffset;) {
            v1 = v2;
            if (batchElements) {
                v2 = vBuf + batchElements[i++];
            } else {
                v2 = vBuf + i++;
            }
            v2->boundaryEdge = GL_TRUE;

            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = v2;

            if (((v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                (*gc->procs.renderTriangle)(gc, v0, v1, v2);
            } else if (v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                __glDoClipTriangle(gc, v0, v1, v2);
            }
        }
        batchIndex += batchCount;
        count -= batchCount;
    } while (count > 0);
}

#endif

void __glDrawVertexes_Qstrip(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1, *v2, *v3;
    int batchFirst, batchIndex, batchCount, batchOffset;
    GLuint *batchElements;
    int i;

    if (count < 4) return;
    count = count & 0xFFFFFFFE;
    assert((count >= 4) && (count % 2 == 0));
    assert((first == 0) || (elements == 0));

    batchFirst = batchIndex = first;
    batchCount = count;
    batchOffset = 0;
    batchElements = elements;

    do {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (batchFirst == batchIndex) {
                /* first batch */
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
                /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
                {
                    gc->vertexArray.batchMode = GL_QUAD_STRIP;
                    gc->vertexArray.batchFirst = first;
                    gc->vertexArray.batchElements = elements;
                }
            } else {
                /* subsequent batch */
                vBuf[0] = vBuf[batchOffset+(batchCount-2)];
                vBuf[0].color = &vBuf[0].colors[__GL_FRONTFACE];
                vBuf[1] = vBuf[batchOffset+(batchCount-1)];
                vBuf[1].color = &vBuf[1].colors[__GL_FRONTFACE];
                batchOffset = 2;
            }
            if (count < batchCount) {
                batchCount = count;
            }
            if (gc->vertexArray.continuation)
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset + 2,
                                                        batchIndex,
                                                        batchCount - 2,
                                                        elements);
            else
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset,
                                                        batchIndex,
                                                        batchCount,
                                                        elements);
            gc->vertexArray.batchIndex = batchIndex;
        }

        if (batchElements) {
            v2 = vBuf + batchElements[0];
            v3 = vBuf + batchElements[1];
        } else {
            v2 = vBuf + batchFirst;
            v3 = vBuf + batchFirst + 1;
        }
        v2->boundaryEdge = GL_TRUE;
        v3->boundaryEdge = GL_TRUE;

        for (i=batchFirst+2; i<batchCount+batchOffset;) {
            v0 = v2;
            v1 = v3;
            if (batchElements) {
                v2 = vBuf + batchElements[i++];
                v3 = vBuf + batchElements[i++];
            } else {
                v2 = vBuf + i++;
                v3 = vBuf + i++;
            }
            v2->boundaryEdge = GL_TRUE;
            v3->boundaryEdge = GL_TRUE;

            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = v3;

            if (((v0->hasAndClipCode|v1->hasAndClipCode|v2->hasAndClipCode|v3->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                v1->boundaryEdge = GL_FALSE;
                (*gc->procs.renderTriangle)(gc, v0, v1, v2);
                v1->boundaryEdge = GL_TRUE;
                v2->boundaryEdge = GL_FALSE;
                (*gc->procs.renderTriangle)(gc, v2, v1, v3);
                v2->boundaryEdge = GL_TRUE;
            } else if (v0->hasAndClipCode&v1->hasAndClipCode&v2->hasAndClipCode&v3->hasAndClipCode&__GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                ClipQuad(gc, v0, v1, v3, v2);
            }
        }
        batchIndex += batchCount;
        count -= batchCount;
    } while (count > 0);
}

void __glDrawVertexes_Lstrip(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1;
    int batchFirst, batchIndex, batchCount, batchOffset;
    GLuint *batchElements;
    int i;

    if (count < 2) return;
    assert(count >= 2);
    assert((first == 0) || (elements == 0));

    batchFirst = batchIndex = first;
    batchCount = count;
    batchOffset = 0;
    batchElements = elements;

    do {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (batchFirst == batchIndex) {
                /* first batch */
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
                /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
                {
                    gc->vertexArray.batchMode = GL_LINE_STRIP;
                    gc->vertexArray.batchFirst = first;
                    gc->vertexArray.batchElements = elements;
                }
            } else {
                /* subsequent batch */
                vBuf[0] = vBuf[batchOffset+(batchCount-1)];
                vBuf[0].color = &vBuf[0].colors[__GL_FRONTFACE];
                batchOffset = 1;
            }
            if (count < batchCount) {
                batchCount = count;
            }
            if (gc->vertexArray.continuation)
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset + 1,
                                                        batchIndex,
                                                        batchCount - 1,
                                                        elements);
            else
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset,
                                                        batchIndex,
                                                        batchCount,
                                                        elements);
            gc->vertexArray.batchIndex = batchIndex;
        }

        if (batchElements) {
            v1 = vBuf + batchElements[0];
        } else {
            v1 = vBuf + batchFirst;
        }
        DO_VALIDATE(gc, v1, gc->vertex.faceNeeds[__GL_FRONTFACE]);

        gc->line.notResetStipple = GL_FALSE;

        for (i=batchFirst+1; i<batchCount+batchOffset;) {
            v0 = v1;
            if (batchElements) {
                v1 = vBuf + batchElements[i++];
            } else {
                v1 = vBuf + i++;
            }

            gc->vertex.provoking = v1;

            if (((v0->hasAndClipCode | v1->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                DO_VALIDATE(gc, v1, gc->vertex.faceNeeds[__GL_FRONTFACE]);
                (*gc->procs.renderLine)(gc, v0, v1);
            } else if (v0->hasAndClipCode & v1->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                ClipLine(gc, v0, v1);
            }
        }
        batchIndex += batchCount;
        count -= batchCount;
    } while (count > 0);
}

void __glDrawVertexes_Lloop(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1, *vFirst;
    int batchFirst, batchIndex, batchCount, batchOffset;
    GLuint *batchElements;
    int i;

    if (count < 2) return;
    assert(count >= 2);
    assert((first == 0) || (elements == 0));

    batchFirst = batchIndex = first;
    batchCount = count;
    batchOffset = 0;
    batchElements = elements;

    do {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (batchFirst == batchIndex) {
                /* first batch */
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
                /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
                {
                    gc->vertexArray.batchMode = GL_LINE_LOOP;
                    gc->vertexArray.batchFirst = first;
                    gc->vertexArray.batchElements = elements;
                }
            } else {
                /* subsequent batch */
                vBuf[1] = vBuf[batchOffset+(batchCount-1)];
                vBuf[1].color = &vBuf[1].colors[__GL_FRONTFACE];
                batchFirst = 1;
                batchOffset = 1;
            }
            if (count < batchCount) {
                batchCount = count;
            }
            if (gc->vertexArray.continuation)
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset + 1,
                                                        batchIndex,
                                                        batchCount - 1,
                                                        elements);
            else
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset,
                                                        batchIndex,
                                                        batchCount,
                                                        elements);
            gc->vertexArray.batchIndex = batchIndex;
        }

        if (batchElements) {
            v1 = vBuf + batchElements[0];
        } else {
            v1 = vBuf + batchFirst;
        }
        DO_VALIDATE(gc, v1, gc->vertex.faceNeeds[__GL_FRONTFACE]);

        gc->line.notResetStipple = GL_FALSE;

        vFirst = v1;

        for (i=batchFirst+1; i<batchCount+batchOffset;) {
            v0 = v1;
            if (batchElements) {
                v1 = vBuf + batchElements[i++];
            } else {
                v1 = vBuf + i++;
            }

            gc->vertex.provoking = v1;

            if (((v0->hasAndClipCode | v1->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                DO_VALIDATE(gc, v1, gc->vertex.faceNeeds[__GL_FRONTFACE]);
                (*gc->procs.renderLine)(gc, v0, v1);
            } else if (v0->hasAndClipCode & v1->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                ClipLine(gc, v0, v1);
            }
        }
        batchIndex += batchCount;
        count -= batchCount;
    } while (count > 0);

    /* Draw line from last to first. */
    if (((v1->hasAndClipCode | vFirst->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
        (*gc->procs.renderLine)(gc, v1, vFirst);
    } else if ((v1->hasAndClipCode & vFirst->hasAndClipCode & __GL_ALL_CLIP_MASK) == 0) {
        ClipLine(gc, v1, vFirst);
    }
}

#if __GL_SST_GLIDE_VTX

void __glDrawVertexes_Triangles(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    int batchFirst, batchIndex, batchCount;
    GLuint *batchElements;
    int i;

    if (count < 3) return;
    assert(count >= 3);
    assert((first == 0) || (elements == 0));

        batchFirst = batchIndex = first;
        batchCount = count;
        batchElements = elements;

        {
            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                    batchFirst = 0;
                    batchCount = gc->vertexArray.batchSize;
                    batchElements = NULL;
                    batchCount = count;
                (*gc->vertexArray.compileElementsIndexed)(gc,
                                                          0, batchIndex, batchCount, elements);
            }

            for (i=batchFirst; i+2<batchCount;) {
                __GLvertex *v0, *v1, *v2;

                v0 = vBuf + i++;
                v1 = vBuf + i++;
                v2 = vBuf + i++;

                gc->vertex.provoking = v2;

                if (((v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                    (*gc->procs.renderTriangle)(gc, v0, v1, v2);
                } else if (v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                    continue; /* cull -or- trivial reject */
                } else {
                    __glDoClipTriangle(gc, v0, v1, v2);
                }
            }
        }
}

#else

void __glDrawVertexes_Triangles(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    int batchFirst, batchIndex, batchCount;
    GLuint *batchElements;
    int i;

    if (count < 3) return;
    assert(count >= 3);
    assert((first == 0) || (elements == 0));

        batchFirst = batchIndex = first;
        batchCount = count;
        batchElements = elements;

        do {
            if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
                if (batchFirst == batchIndex) {
                    /* first batch */
                    batchFirst = 0;
                    batchCount = gc->vertexArray.batchSize;
                    batchElements = NULL;
                    /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
                    {
                        gc->vertexArray.batchMode = GL_TRIANGLES;
                        gc->vertexArray.batchFirst = first;
                        gc->vertexArray.batchElements = elements;
                    }
                } else {
                    /* subsequent batch */
                }
                if (count < batchCount) {
                    batchCount = count;
                }
                (*gc->vertexArray.compileElementsIndexed)(gc,
                                                          0, batchIndex, batchCount, elements);
                gc->vertexArray.batchIndex = batchIndex;
            }

            for (i=batchFirst; i+2<batchCount;) {
                __GLvertex *v0, *v1, *v2;

                if (batchElements) {
                    v0 = vBuf + batchElements[i++];
                    v1 = vBuf + batchElements[i++];
                    v2 = vBuf + batchElements[i++];
                } else {
                    v0 = vBuf + i++;
                    v1 = vBuf + i++;
                    v2 = vBuf + i++;
                }

                gc->line.notResetStipple = GL_FALSE;
                gc->vertex.provoking = v2;

                if (((v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                    (*gc->procs.renderTriangle)(gc, v0, v1, v2);
                } else if (v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                    continue; /* cull -or- trivial reject */
                } else {
                    __glDoClipTriangle(gc, v0, v1, v2);
                }
            }
            batchIndex += batchCount;
            count -= batchCount;
        } while (count > 0);
}

#endif

#if __GL_SST_GLIDE_VTX

void __glDrawVertexes_Quads(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    int batchFirst, batchIndex, batchCount;
    GLuint *batchElements;
    int i;

    if (count < 4) return;
    count = count & 0xFFFFFFFC;
    assert((count >= 4) && (count % 4 == 0));
    assert((first == 0) || (elements == 0));

    {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            (*gc->vertexArray.compileElementsIndexed)(gc,
                                                      0,
                                                      0,
                                                      count,
                                                      elements);
        }

        for (i=0; i<count;) {
            __GLvertex *v0, *v1, *v2, *v3;

            v0 = vBuf + i++;
            v1 = vBuf + i++;
            v2 = vBuf + i++;
            v3 = vBuf + i++;

            gc->vertex.provoking = v3;

            if (((v0->hasAndClipCode|v1->hasAndClipCode|v2->hasAndClipCode|v3->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                (*gc->procs.renderTriangle)(gc, v0, v1, v3);
                (*gc->procs.renderTriangle)(gc, v1, v2, v3);
            } else if (v0->hasAndClipCode&v1->hasAndClipCode&v2->hasAndClipCode&v3->hasAndClipCode&__GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                ClipQuad(gc, v0, v1, v2, v3);
            }
        }
    }
}

#else

void __glDrawVertexes_Quads(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    int batchFirst, batchIndex, batchCount;
    GLuint *batchElements;
    int i;

    if (count < 4) return;
    count = count & 0xFFFFFFFC;
    assert((count >= 4) && (count % 4 == 0));
    assert((first == 0) || (elements == 0));

    batchFirst = batchIndex = first;
    batchCount = count;
    batchElements = elements;

    do {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (batchFirst == batchIndex) {
                /* first batch */
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
                /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
                {
                    gc->vertexArray.batchMode = GL_QUADS;
                    gc->vertexArray.batchFirst = first;
                    gc->vertexArray.batchElements = elements;
                }
            } else {
                /* subsequent batch */
            }
            if (count < batchCount) {
                batchCount = count;
            }
            (*gc->vertexArray.compileElementsIndexed)(gc,
                            0, batchIndex, batchCount, elements);
            gc->vertexArray.batchIndex = batchIndex;
        }

        for (i=batchFirst; i<batchCount;) {
            __GLvertex *v0, *v1, *v2, *v3;

            if (batchElements) {
                v0 = vBuf + batchElements[i++];
                v1 = vBuf + batchElements[i++];
                v2 = vBuf + batchElements[i++];
                v3 = vBuf + batchElements[i++];
            } else {
                v0 = vBuf + i++;
                v1 = vBuf + i++;
                v2 = vBuf + i++;
                v3 = vBuf + i++;
            }

            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = v3;

            if (((v0->hasAndClipCode|v1->hasAndClipCode|v2->hasAndClipCode|v3->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                GLboolean saveTag = v1->boundaryEdge;
                v1->boundaryEdge = GL_FALSE;
                (*gc->procs.renderTriangle)(gc, v0, v1, v3);
                v1->boundaryEdge = saveTag;
                saveTag = v3->boundaryEdge;
                v3->boundaryEdge = GL_FALSE;
                (*gc->procs.renderTriangle)(gc, v1, v2, v3);
                v3->boundaryEdge = saveTag;
            } else if (v0->hasAndClipCode&v1->hasAndClipCode&v2->hasAndClipCode&v3->hasAndClipCode&__GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                ClipQuad(gc, v0, v1, v2, v3);
            }
        }
        batchIndex += batchCount;
        count -= batchCount;
    } while (count > 0);
}

#endif

#if __GL_SST_GLIDE_VTX

void __glDrawVertexes_Polygon(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1, *v2;
    int batchFirst, batchIndex, batchCount, batchOffset;
    GLuint *batchElements;
    int i, last = first+count;

    if (count < 3) return;
    assert(count >= 3);
    assert((first == 0) || (elements == 0));

    {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
#if 0
            if (gc->vertexArray.continuation)
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset + 2,
                                                        batchIndex,
                                                        batchCount - 2,
                                                        elements);
            else
#endif
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        0,
                                                        0,
                                                        count,
                                                        elements);
        }
#if 0   
        {
            __GLSSTvertex *vx = gc->vertexArray.sstBuf;
            int i;

            for (i=0; i < count; i++) {
              __GLSSTvertex *tv = &vx[i];

              tv->a = 1.0;
            }
        }
#endif  

        v0 = vBuf;
        v2 = vBuf + 1;
        for (i=2; i<count; ++i) {
            v1 = v2;
            v2 = vBuf + i;

            if (((v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                (*gc->procs.renderTriangle)(gc, v0, v1, v2);
            } else if (v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                __glDoClipTriangle(gc, v0, v1, v2);
            }
        }
    }
}

#else

void __glDrawVertexes_Polygon(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    __GLvertex *v0, *v1, *v2;
    int batchFirst, batchIndex, batchCount, batchOffset;
    GLuint *batchElements;
    int i, last = first+count;

    if (count < 3) return;
    assert(count >= 3);
    assert((first == 0) || (elements == 0));

    batchFirst = batchIndex = first;
    batchCount = count;
    batchOffset = 0;
    batchElements = elements;

    do {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (batchFirst == batchIndex) {
                /* first batch */
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
                /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
                {
                    gc->vertexArray.batchMode = GL_POLYGON;
                    gc->vertexArray.batchFirst = first;
                    gc->vertexArray.batchElements = elements;
                }
            } else {
                /* subsequent batch */
                vBuf[1] = vBuf[batchOffset+(batchCount-1)];
                vBuf[1].color = &vBuf[1].colors[__GL_FRONTFACE];
                batchOffset = 2;
            }
            if (count < batchCount) {
                batchCount = count;
            }
            if (gc->vertexArray.continuation)
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset + 2,
                                                        batchIndex,
                                                        batchCount - 2,
                                                        elements);
            else
              (*gc->vertexArray.compileElementsIndexed)(gc,
                                                        batchOffset,
                                                        batchIndex,
                                                        batchCount,
                                                        elements);
            gc->vertexArray.batchIndex = batchIndex;
        }

        if (batchElements) {
            v0 = vBuf + batchElements[0];
            v2 = vBuf + batchElements[1];
        } else {
            v0 = vBuf + batchFirst;
            v2 = vBuf + batchFirst + 1;
        }

        gc->line.notResetStipple = GL_FALSE;
        gc->vertex.provoking = v0;

        for (i=batchFirst+2; i<batchCount+batchOffset; ++i) {
            GLboolean saveTag0, saveTag2;

            v1 = v2;
            if (batchElements) {
                v2 = vBuf + batchElements[i];
            } else {
                v2 = vBuf + i;
            }

            saveTag0 = v0->boundaryEdge;
            if (i+batchIndex != first+2) {
                v0->boundaryEdge = GL_FALSE;
            }
            saveTag2 = v2->boundaryEdge;
            if (i+batchIndex != last-1) {
                v2->boundaryEdge = GL_FALSE;
            }

            if (((v0->hasAndClipCode | v1->hasAndClipCode | v2->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                (*gc->procs.renderTriangle)(gc, v0, v1, v2);
            } else if (v0->hasAndClipCode & v1->hasAndClipCode & v2->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                __glDoClipTriangle(gc, v0, v1, v2);
            }

            v0->boundaryEdge = saveTag0;
            v2->boundaryEdge = saveTag2;
        }
        batchIndex += batchCount;
        count -= batchCount;
    } while (count > 0);

}

#endif

void __glDrawVertexes_Lines(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    int batchFirst, batchIndex, batchCount;
    GLuint *batchElements;
    int i;

    if (count < 2) return;
    count = count & 0xFFFFFFFE;
    assert((count >= 2) && (count % 2 == 0));
    assert((first == 0) || (elements == 0));

    batchFirst = batchIndex = first;
    batchCount = count;
    batchElements = elements;

    do {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (batchFirst == batchIndex) {
                /* first batch */
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
                /* if (gc->vertexArray.controlWord & VERTARRAY_CW_VX_CULL) */
                {
                    gc->vertexArray.batchMode = GL_LINES;
                    gc->vertexArray.batchFirst = first;
                    gc->vertexArray.batchElements = elements;
                }
            } else {
                /* subsequent batch */
            }
            if (count < batchCount) {
                batchCount = count;
            }
            (*gc->vertexArray.compileElementsIndexed)(gc,
                            0, batchIndex, batchCount, elements);
            gc->vertexArray.batchIndex = batchIndex;
        }

        for (i=batchFirst; i<batchCount;) {
            __GLvertex *v0, *v1;

            if (batchElements) {
                v0 = vBuf + batchElements[i++];
                v1 = vBuf + batchElements[i++];
            } else {
                v0 = vBuf + i++;
                v1 = vBuf + i++;
            }

            gc->line.notResetStipple = GL_FALSE;
            gc->vertex.provoking = v1;

            if (((v0->hasAndClipCode | v1->hasAndClipCode) & __GL_ALL_CLIP_MASK) == 0) {
                DO_VALIDATE(gc, v0, gc->vertex.faceNeeds[__GL_FRONTFACE]);
                DO_VALIDATE(gc, v1, gc->vertex.faceNeeds[__GL_FRONTFACE]);
                (*gc->procs.renderLine)(gc, v0, v1);
            } else if (v0->hasAndClipCode & v1->hasAndClipCode & __GL_ALL_CLIP_MASK) {
                continue; /* cull -or- trivial reject */
            } else {
                ClipLine(gc, v0, v1);
            }
        }
        batchIndex += batchCount;
        count -= batchCount;
    } while (count > 0);
}

void __glDrawVertexes_Points(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements)
{
    __GLvertex *vBuf = (__GLvertex *) gc->vertexArray.varrayPtr;
    int batchFirst, batchIndex, batchCount;
    GLuint *batchElements;
    int i;

    assert((first == 0) || (elements == 0));

    batchFirst = batchIndex = first;
    batchCount = count;
    batchElements = elements;

    do {
        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (batchFirst == batchIndex) {
                /* first batch */
                batchFirst = 0;
                batchCount = gc->vertexArray.batchSize;
                batchElements = NULL;
            } else {
                /* subsequent batch */
            }
            if (count < batchCount) {
                batchCount = count;
            }
            (*gc->vertexArray.compileElementsIndexed)(gc,
                            0, batchIndex, batchCount, elements);
        }

        for (i=batchFirst; i<batchCount;) {
            __GLvertex *v0;

            if (batchElements) {
                v0 = vBuf + batchElements[i++];
            } else {
                v0 = vBuf + i++;
            }

            if ((v0->hasAndClipCode & __GL_ALL_CLIP_MASK) == 0) {
                DO_VALIDATE(gc, v0, 
                  gc->vertex.faceNeeds[__GL_FRONTFACE] | __GL_HAS_FRONT_COLOR);
                (*gc->procs.renderPoint)(gc, v0);
            }
        }
        batchIndex += batchCount;
        count -= batchCount;
    } while (count > 0);
}

void __glInitVertexArrayState(__GLcontext *gc)
{
    /*
     * Set up vertex array default values
     */
    gc->vertexArray.vp_size = 4;
    gc->vertexArray.vp_type = GL_FLOAT;
    gc->vertexArray.vp_call = (void (APIENTRY *)(const char *))__glNop;
    gc->vertexArray.np_type = GL_FLOAT;
    gc->vertexArray.np_call = (void (APIENTRY *)(const char *))__glNop;
    gc->vertexArray.cp_size = 4;
    gc->vertexArray.cp_type = GL_FLOAT;
    gc->vertexArray.cp_call = (void (APIENTRY *)(const char *))__glNop;
    gc->vertexArray.ip_type = GL_FLOAT;
    gc->vertexArray.ip_call = (void (APIENTRY *)(const char *))__glNop;
    gc->vertexArray.tp_size = 4;
    gc->vertexArray.tp_type = GL_FLOAT;
    gc->vertexArray.tp_call = (void (APIENTRY *)(const char *))__glNop;
    gc->vertexArray.ep_call = (void (APIENTRY *)(const char *))__glNop;

    gc->vertexArray.interleavedFormat = GL_NONE;

    gc->vertexArray.drawVertexes[GL_POINTS] = __glDrawVertexes_Points;
    gc->vertexArray.drawVertexes[GL_LINES] = __glDrawVertexes_Lines;
    gc->vertexArray.drawVertexes[GL_LINE_LOOP] = __glDrawVertexes_Lloop;
    gc->vertexArray.drawVertexes[GL_LINE_STRIP] = __glDrawVertexes_Lstrip;
    gc->vertexArray.drawVertexes[GL_TRIANGLES] = __glDrawVertexes_Triangles;
    gc->vertexArray.drawVertexes[GL_TRIANGLE_STRIP] = __glDrawVertexes_Tstrip;
    gc->vertexArray.drawVertexes[GL_TRIANGLE_FAN] = __glDrawVertexes_Tfan;
    gc->vertexArray.drawVertexes[GL_QUADS] = __glDrawVertexes_Quads;
    gc->vertexArray.drawVertexes[GL_QUAD_STRIP] = __glDrawVertexes_Qstrip;
    gc->vertexArray.drawVertexes[GL_POLYGON] = __glDrawVertexes_Polygon;

    /*
    ** The batch size must be a multiple of 2, 3 and 4 to simplify
    ** processing arrays of independent lines, tris, and quads.
    ** It should also be at least as large as the vertex cache, so
    ** that vertexes in the vcache are only batched once.
    */
    gc->vertexArray.batchSize = MAX_VERTEX_CACHE;
    assert(gc->vertexArray.batchSize >= MAX_VERTEX_CACHE);
    assert((gc->vertexArray.batchSize % 3) == 0);
    assert((gc->vertexArray.batchSize % 4) == 0);

    /*
    ** Allocate a block of space for the internal vertex array.
    ** TBD-- this probably needs to be page aligned and sized
    ** appropriately.  For now it is a constant.
    */
    gc->vertexArray.blockSize = 4096;
    (void) AllocateVbuf(gc, 0, gc->vertexArray.batchSize+2);
}

void __glFreeVertexArrayState(__GLcontext *gc)
{
    if (NULL != gc->vertexArray.varrayBuf) {
        (*gc->imports.free)(gc, gc->vertexArray.varrayBuf);
        gc->vertexArray.varrayBuf = NULL;
        gc->vertexArray.varrayPtr = NULL;
        gc->vertexArray.varrayBufSize = 0;
    }
}

void APIENTRY __glim_ArrayElement(GLint i) 
{
    __GL_SETUP();
    GLint index = gc->vertexArray.index;

    if (index & VERTARRAY_N_INDEX) 
        (*gc->vertexArray.np_call)
          (gc->vertexArray.normal_pointer + i*gc->vertexArray.np_stride);
    if (index & VERTARRAY_C_INDEX) 
        (*gc->vertexArray.cp_call)
          (gc->vertexArray.color_pointer + i*gc->vertexArray.cp_stride);
    if (index & VERTARRAY_T_INDEX)
        (*gc->vertexArray.tp_call)
          (gc->vertexArray.tex_coord_pointer + i*gc->vertexArray.tp_stride);
    if (index & VERTARRAY_I_INDEX) 
        (*gc->vertexArray.ip_call)
          (gc->vertexArray.index_pointer + i*gc->vertexArray.ip_stride);
    if (index & VERTARRAY_E_INDEX)
        (*gc->vertexArray.ep_call)
          (gc->vertexArray.edge_pointer + i*gc->vertexArray.ep_stride);
    if (index & VERTARRAY_V_INDEX) 
        (*gc->vertexArray.vp_call)
          (gc->vertexArray.vertex_pointer + i*gc->vertexArray.vp_stride);
}

void APIENTRY __glim_DrawArrays(GLenum mode,
                             GLint first,
                             GLsizei count)
{
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    /* check that first and count are positive */
    if (first < 0 || count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_POLYGON) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (gc->vertexArray.controlWord & 
                (VERTARRAY_CW_SLOWPATH | VERTARRAY_CW_NO_VERTEX)) {
        (*gc->procs.varray_funcs[gc->vertexArray.index])(mode, first, count);

    } else { 
        void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);

        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (gc->vertexArray.controlWord & VERTARRAY_CW_ALLOW_PRECOMPILE) {
                (*gc->vertexArray.compileElements)(gc, gc->vertexArray.start,
                                gc->vertexArray.start, gc->vertexArray.count);
                gc->vertexArray.controlWord &= ~VERTARRAY_CW_NEEDS_COMPILE;
            }
        }

        drawVertexes = gc->vertexArray.drawVertexes[mode];
        assert(NULL != drawVertexes);
        (*drawVertexes)(gc, first, count, NULL);
    }
}
  
void APIENTRY __glim_DrawElements(GLenum mode, GLsizei count, GLenum type, 
                        const GLvoid *indices)
{
    GLuint *elements;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    /* check that count is positive */
    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_POLYGON) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /*
    ** The optimized path is for unsigned int indices, but we
    ** can make the other types go through that path by converting
    ** the indices to unsigned int.  Later we will want to find
    ** a way to propagate the optimized path to all types.
    */
    switch(type) {
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *byteElements = (GLubyte *)indices;
            int i;

            elements = (GLuint *) gc->imports.malloc(gc, count*sizeof(GLuint));
            for (i=0; i<count; i++) {
                elements[i] = byteElements[i];
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *shortElements = (GLushort *)indices;
            int i;

            elements = (GLuint *) gc->imports.malloc(gc, count*sizeof(GLuint));
            for (i=0; i<count; i++) {
                elements[i] = shortElements[i];
            }
        }
        break;
    case GL_UNSIGNED_INT:
        elements = (GLuint *)indices;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    if (gc->vertexArray.controlWord &
                (VERTARRAY_CW_SLOWPATH | VERTARRAY_CW_NO_VERTEX)) {
        int i;

        glBegin(mode);
        for (i=0; i<count; i++) {
            __glim_ArrayElement(elements[i]);
        }
        glEnd();
    } else {
        void (*drawVertexes)(__GLcontext *gc, GLint first, GLsizei count, GLuint *elements);

        if (gc->vertexArray.controlWord & VERTARRAY_CW_NEEDS_COMPILE) {
            if (gc->vertexArray.controlWord & VERTARRAY_CW_ALLOW_PRECOMPILE) {
                (*gc->vertexArray.compileElements)(gc, gc->vertexArray.start,
                                gc->vertexArray.start, gc->vertexArray.count);
                gc->vertexArray.controlWord &= ~VERTARRAY_CW_NEEDS_COMPILE;
            }
        }

        drawVertexes = gc->vertexArray.drawVertexes[mode];
        assert(NULL != drawVertexes);
        (*drawVertexes)(gc, 0, count, elements);
    }

    if (elements != indices) {
        gc->imports.free(gc, elements);
    }
}

/*
 * HOW THE POINTER SETUP CALLS WORK
 * The pointer setup calls are all table-driven whereever possible.  This 
 * is what happens:
 *      stride is checked for <= 0
 *      count is checked for <= 0
 *      size is range-checked (for example, as in [1..4] for tex coords)
 *      type is range-checked for >= GL_BYTE and <= GL_DOUBLE.  This 
 *              assures we will not core dump when we do table lookups 
 *              a few lines hence
 *      The function pointer is looked up in the appropriate table (normal, 
 *              vertex, color, index, or tex coord)
 *      If the function pointer is null, type must not be valid (since we 
 *              already checked for all other errors which could cause this), 
 *              so set an error
 *      Update the pointer, size, type, stride, usr_stride, and count in 
 *              the context (the usr_stride differs from the stride in that 
 *              if the usr passed in a 0 (for tightly packed arrays), the 
 *              usr_stride will still be 0 but the stride will be the 
 *              appropriate value to use in the drawing loops.  This is
 *              needed so that the get commands will return the right thing.
 *      Update the signature.  This is a bitflag used in machine-dependent 
 *              implementations to find special cases.  It encodes the sizes 
 *              and types.
 */

/*
 * Stride table -- indexed by type and size
 */
static int stride_array[GL_DOUBLE - GL_BYTE + 1][5] = {
    0, sizeof(GLbyte),          2*sizeof(GLbyte),       3*sizeof(GLbyte),
    4*sizeof(GLbyte),
    0, sizeof(GLubyte),         2*sizeof(GLubyte),      3*sizeof(GLubyte),
    4*sizeof(GLubyte),
    0, sizeof(GLshort),         2*sizeof(GLshort),      3*sizeof(GLshort), 
    4*sizeof(GLshort),
    0, sizeof(GLushort),        2*sizeof(GLushort),     3*sizeof(GLushort),
    4*sizeof(GLushort),
    0, sizeof(GLint),           2*sizeof(GLint),        3*sizeof(GLint), 
    4*sizeof(GLint),
    0, sizeof(GLuint),          2*sizeof(GLuint),       3*sizeof(GLuint),
    4*sizeof(GLuint),
    0, sizeof(GLfloat),         2*sizeof(GLfloat),      3*sizeof(GLfloat),
    4*sizeof(GLfloat),
    0, 0, 0,                    0,                      0,
    0, 0, 0,                    0,                      0,
    0, 0, 0,                    0,                      0,
    0, sizeof(GLdouble),        2*sizeof(GLdouble),     3*sizeof(GLdouble),
    4*sizeof(GLdouble),
};

/*
 * Pointer table prototype.  Setting the table up as a structure and
 * then casting it to an array will help force people to enter the right 
 * function in the right spot since it will generate compiler warnings 
 * otherwise.  The final table will be indexed by type and size.
 */
typedef struct {
    /* functions used to unpack vertex arrays on the slow path */
    void (APIENTRY *byte_funcs[5])(const GLbyte *);
    void (APIENTRY *ubyte_funcs[5])(const GLubyte *);
    void (APIENTRY *short_funcs[5])(const GLshort *);
    void (APIENTRY *ushort_funcs[5])(const GLushort *);
    void (APIENTRY *int_funcs[5])(const GLint *);
    void (APIENTRY *uint_funcs[5])(const GLuint *);
    void (APIENTRY *float_funcs[5])(const GLfloat *);
    /* skip over GL_2_BYTES...GL_4_BYTES */
    void (APIENTRY *filler[3*5])(void);
    void (APIENTRY *double_funcs[5])(const GLdouble *);

    /* functions used to unpack vertex arrays on the fast path */
    void (__fastcall *byte_copy_funcs[5])(__GLcontext *,
                                        const GLbyte *, __GLvertex *);
    void (__fastcall *ubyte_copy_funcs[5])(__GLcontext *,
                                        const GLubyte *, __GLvertex *);
    void (__fastcall *short_copy_funcs[5])(__GLcontext *,
                                        const GLshort *, __GLvertex *);
    void (__fastcall *ushort_copy_funcs[5])(__GLcontext *,
                                        const GLushort *, __GLvertex *);
    void (__fastcall *int_copy_funcs[5])(__GLcontext *,
                                        const GLint *, __GLvertex *);
    void (__fastcall *uint_copy_funcs[5])(__GLcontext *,
                                        const GLuint *, __GLvertex *);
    void (__fastcall *float_copy_funcs[5])(__GLcontext *,
                                        const GLfloat *, __GLvertex *);
    /* skip over GL_2_BYTES...GL_4_BYTES */
    void (__fastcall *copy_filler[3*5])(void);
    void (__fastcall *double_copy_funcs[5])(__GLcontext *,
                                        const GLdouble *, __GLvertex *);
} func_array;

static func_array vertex_funcs = {
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, glVertex2sv,  glVertex3sv,    glVertex4sv,
    0, 0, 0,            0,              0,
    0, 0, glVertex2iv,  glVertex3iv,    glVertex4iv,
    0, 0, 0,            0,              0,
    0, 0, glVertex2fv,  glVertex3fv,    glVertex4fv,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, glVertex2dv,  glVertex3dv,    glVertex4dv,

    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, CopyVertex2s, CopyVertex3s,   CopyVertex4s,
    0, 0, 0,            0,              0,
    0, 0, CopyVertex2i, CopyVertex3i,   CopyVertex4i,
    0, 0, 0,            0,              0,
    0, 0, CopyVertex2f, CopyVertex3f,   CopyVertex4f,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, CopyVertex2d, CopyVertex3d,    CopyVertex4d,
};
static void (APIENTRY *(*vertex_func_table)[2][11][5])(const char *) = 
     (void (APIENTRY *(*)[2][11][5])(const char *))(&vertex_funcs);

static func_array normal_funcs = {
    0, 0, 0,            glNormal3bv,    0,
    0, 0, 0,            0,              0,
    0, 0, 0,            glNormal3sv,    0,
    0, 0, 0,            0,              0,
    0, 0, 0,            glNormal3iv,    0,
    0, 0, 0,            0,              0,
    0, 0, 0,            glNormal3fv,    0,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            glNormal3dv,    0,

    0, 0, 0,            CopyNormal3b,   0,
    0, 0, 0,            0,              0,
    0, 0, 0,            CopyNormal3s,   0,
    0, 0, 0,            0,              0,
    0, 0, 0,            CopyNormal3i,   0,
    0, 0, 0,            0,              0,
    0, 0, 0,            CopyNormal3f,   0,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            CopyNormal3d,   0,
};
static void (APIENTRY *(*normal_func_table)[2][11][5])(const char *) = 
     (void (APIENTRY *(*)[2][11][5])(const char *))(&normal_funcs);

static func_array color_funcs = {
    0, 0, 0,            glColor3bv,     glColor4bv,
    0, 0, 0,            glColor3ubv,    glColor4ubv,
    0, 0, 0,            glColor3sv,     glColor4sv,
    0, 0, 0,            glColor3usv,    glColor4usv,
    0, 0, 0,            glColor3iv,     glColor4iv,
    0, 0, 0,            glColor3uiv,    glColor4uiv,
    0, 0, 0,            glColor3fv,     glColor4fv,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            glColor3dv,     glColor4dv,

    0, 0, 0,            CopyColor3b,    CopyColor4b,
    0, 0, 0,            CopyColor3ub,   CopyColor4ub,
    0, 0, 0,            CopyColor3s,    CopyColor4s,
    0, 0, 0,            CopyColor3us,   CopyColor4us,
    0, 0, 0,            CopyColor3i,    CopyColor4i,
    0, 0, 0,            CopyColor3ui,   CopyColor4ui,
    0, 0, 0,            CopyColor3f,    CopyColor4f,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            0,              0,
    0, 0, 0,            CopyColor3d,    CopyColor4d,
};
static void (APIENTRY *(*color_func_table)[2][11][5])(const char *) = 
     (void (APIENTRY *(*)[2][11][5])(const char *))(&color_funcs);     

/* eventually, this should not be a full-blown func_array
 * since only one column is used... */
static func_array index_funcs = {
    0, 0,               0,              0,              0,
    0, glIndexubv,      0,              0,              0,
    0, glIndexsv,       0,              0,              0,
    0, 0,               0,              0,              0,
    0, glIndexiv,       0,              0,              0,
    0, 0,               0,              0,              0,
    0, glIndexfv,       0,              0,              0,
    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, glIndexdv,       0,              0,              0,

    0, 0,               0,              0,              0,
    0, CopyIndexub,     0,              0,              0,
    0, CopyIndexs,      0,              0,              0,
    0, 0,               0,              0,              0,
    0, CopyIndexi,      0,              0,              0,
    0, 0,               0,              0,              0,
    0, CopyIndexf,      0,              0,              0,
    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, CopyIndexd,      0,              0,              0,
};
static void (APIENTRY *(*index_func_table)[2][11][5])(const char *) = 
     (void (APIENTRY *(*)[2][11][5])(const char *))(&index_funcs);     

static func_array tex_coord_funcs = {
    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, glTexCoord1sv,   glTexCoord2sv,  glTexCoord3sv,  glTexCoord4sv,
    0, 0,               0,              0,              0,
    0, glTexCoord1iv,   glTexCoord2iv,  glTexCoord3iv,  glTexCoord4iv,
    0, 0,               0,              0,              0,
    0, glTexCoord1fv,   glTexCoord2fv,  glTexCoord3fv,  glTexCoord4fv,
    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, glTexCoord1dv,   glTexCoord2dv,  glTexCoord3dv,  glTexCoord4dv,

    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, CopyTexCoord1s,  CopyTexCoord2s, CopyTexCoord3s, CopyTexCoord4s,
    0, 0,               0,              0,              0,
    0, CopyTexCoord1i,  CopyTexCoord2i, CopyTexCoord3i, CopyTexCoord4i,
    0, 0,               0,              0,              0,
    0, CopyTexCoord1f,  CopyTexCoord2f, CopyTexCoord3f, CopyTexCoord4f,
    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, 0,               0,              0,              0,
    0, CopyTexCoord1d,  CopyTexCoord2d, CopyTexCoord3d, CopyTexCoord4d,
};
static void (APIENTRY *(*tex_coord_func_table)[2][11][5])(const char *) = 
     (void (APIENTRY *(*)[2][11][5])(const char *))(&tex_coord_funcs);     

/* type flag table -- indexed by type */
static int type_flag_table[] = {
    VERTARRAY_BYTE_FLAG,
    VERTARRAY_UNSIGNED_BYTE_FLAG,
    VERTARRAY_SHORT_FLAG,
    VERTARRAY_UNSIGNED_SHORT_FLAG,
    VERTARRAY_INT_FLAG,
    VERTARRAY_UNSIGNED_INT_FLAG,
    VERTARRAY_FLOAT_FLAG,
    0, 
    0,
    0,
    VERTARRAY_DOUBLE_FLAG,
};

void APIENTRY __glim_VertexPointer(GLint size,
                                GLenum type,
                                GLsizei stride,
                                const GLvoid *pointer)
{
    void (APIENTRY *vp_call)(const char *);
    __GL_SETUP_NOT_IN_BEGIN();

    if (stride < 0 || size < 2 || size > 4) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if (type < GL_BYTE || type > GL_DOUBLE) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    vp_call = (*vertex_func_table)[0][type - GL_BYTE][size];
    if (vp_call == (void (APIENTRY *)(const char *))0x0) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_API_BLAND();

    /* update the gc */
    gc->vertexArray.vp_call = vp_call;
    gc->vertexArray.vp_copy =
        (void (__fastcall *)(__GLcontext *, const void *, __GLvertex *))
                        (*vertex_func_table)[1][type - GL_BYTE][size];
    gc->vertexArray.vertex_pointer = pointer;
    gc->vertexArray.vp_size = size;
    gc->vertexArray.vp_type = type;
    gc->vertexArray.vp_stride = 
        stride ? stride : stride_array[type - GL_BYTE][size];
    gc->vertexArray.vp_usr_stride = stride;
    gc->vertexArray.signature = 
        (gc->vertexArray.signature & ~VERTARRAY_V_MASK) | 
            (type_flag_table[type - GL_BYTE] << VERTARRAY_VTYPE_SHIFT) | 
                (size << VERTARRAY_VSIZE_SHIFT);
    gc->vertexArray.vp_count = 0;
    gc->vertexArray.interleavedFormat = GL_NONE;

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
}

void APIENTRY __glim_NormalPointer(GLenum type, 
                                GLsizei stride,
                                const GLvoid *pointer)
{
    void (APIENTRY *np_call)(const char *);
    __GL_SETUP_NOT_IN_BEGIN();

    if (stride < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if (type < GL_BYTE || type > GL_DOUBLE) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    np_call = (*normal_func_table)[0][type - GL_BYTE][3];
    if (np_call == (void (APIENTRY *)(const char *))0x0) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_API_BLAND();

    /* update the gc */
    gc->vertexArray.np_call = np_call;
    gc->vertexArray.np_copy =
        (void (__fastcall *)(__GLcontext *, const void *, __GLvertex *))
                        (*normal_func_table)[1][type - GL_BYTE][3];
    gc->vertexArray.normal_pointer = pointer;
    gc->vertexArray.np_type = type;
    gc->vertexArray.np_stride = 
        stride ? stride : stride_array[type - GL_BYTE][3];
    gc->vertexArray.np_usr_stride = stride;
    gc->vertexArray.signature =
        (gc->vertexArray.signature & ~VERTARRAY_N_MASK) | 
            (type_flag_table[type - GL_BYTE] << VERTARRAY_NTYPE_SHIFT);
    gc->vertexArray.np_count = 0;
    gc->vertexArray.interleavedFormat = GL_NONE;

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
}

void APIENTRY __glim_ColorPointer(GLint size,
                               GLenum type,
                               GLsizei stride,
                               const GLvoid *pointer)
{
    void (APIENTRY *cp_call)(const char *);
    __GL_SETUP_NOT_IN_BEGIN();

    if (stride < 0 || size < 3 || size > 4) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if (type < GL_BYTE || type > GL_DOUBLE) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    cp_call = (*color_func_table)[0][type - GL_BYTE][size];
    if (cp_call == (void (APIENTRY *)(const char *))0x0) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_API_BLAND();

    /* update the gc */
    gc->vertexArray.cp_call = cp_call;
    gc->vertexArray.cp_copy =
        (void (__fastcall *)(__GLcontext *, const void *, __GLvertex *))
                        (*color_func_table)[1][type - GL_BYTE][size];
    gc->vertexArray.color_pointer = pointer;
    gc->vertexArray.cp_size = size;
    gc->vertexArray.cp_type = type;
    gc->vertexArray.cp_stride = 
        stride ? stride : stride_array[type - GL_BYTE][size];
    gc->vertexArray.cp_usr_stride = stride;
    gc->vertexArray.signature = 
        (gc->vertexArray.signature & ~VERTARRAY_C_MASK) |
            (type_flag_table[type - GL_BYTE] << VERTARRAY_CTYPE_SHIFT) |
                (size << VERTARRAY_CSIZE_SHIFT);
    gc->vertexArray.cp_count = 0;
    gc->vertexArray.interleavedFormat = GL_NONE;

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
}

void APIENTRY __glim_IndexPointer(GLenum type,
                               GLsizei stride,
                               const GLvoid *pointer)
{
    void (APIENTRY *ip_call)(const char *);
    __GL_SETUP_NOT_IN_BEGIN();

    if (stride < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if (type < GL_BYTE || type > GL_DOUBLE) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    ip_call = (*index_func_table)[0][type - GL_BYTE][1];
    if (ip_call == (void (APIENTRY *)(const char *))0x0) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_API_BLAND();

    /* update the rest of the gc */
    gc->vertexArray.ip_call = ip_call;
    gc->vertexArray.ip_copy =
        (void (__fastcall *)(__GLcontext *, const void *, __GLvertex *))
                        (*index_func_table)[1][type - GL_BYTE][1];
    gc->vertexArray.index_pointer = pointer;
    gc->vertexArray.ip_type = type;
    gc->vertexArray.ip_stride = 
        stride ? stride : stride_array[type - GL_BYTE][1];
    gc->vertexArray.ip_usr_stride = stride;
    gc->vertexArray.signature = 
        (gc->vertexArray.signature & ~VERTARRAY_I_MASK) | 
            (type_flag_table[type - GL_BYTE] << VERTARRAY_ITYPE_SHIFT);
    gc->vertexArray.ip_count = 0;
    gc->vertexArray.interleavedFormat = GL_NONE;

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
}

void APIENTRY __glim_TexCoordPointer(GLint size,
                                  GLenum type,
                                  GLsizei stride,
                                  const GLvoid *pointer)
{
    void (APIENTRY *tp_call)(const char *);
    __GL_SETUP_NOT_IN_BEGIN();

    if (stride < 0 || size < 1 || size > 4) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    if (type < GL_BYTE || type > GL_DOUBLE) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    tp_call = (*tex_coord_func_table)[0][type - GL_BYTE][size];
    if (tp_call == (void (APIENTRY *)(const char *))0x0) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    __GL_API_BLAND();

    /* update the rest of the gc */
    gc->vertexArray.tp_call = tp_call;
    gc->vertexArray.tp_copy =
        (void (__fastcall *)(__GLcontext *, const void *, __GLvertex *))
                        (*tex_coord_func_table)[1][type - GL_BYTE][size];
    gc->vertexArray.tex_coord_pointer = pointer;
    gc->vertexArray.tp_size = size;
    gc->vertexArray.tp_type = type;
    gc->vertexArray.tp_stride = 
        stride ? stride : stride_array[type - GL_BYTE][size];
    gc->vertexArray.tp_usr_stride = stride;
    gc->vertexArray.signature = 
        (gc->vertexArray.signature & ~VERTARRAY_T_MASK) | 
            (type_flag_table[type - GL_BYTE] << VERTARRAY_TTYPE_SHIFT) |
                (size << VERTARRAY_TSIZE_SHIFT);
    gc->vertexArray.tp_count = 0;
    gc->vertexArray.interleavedFormat = GL_NONE;

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
}

void APIENTRY __glim_EdgeFlagPointer(GLint stride,
                                  const GLboolean *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (stride < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    __GL_API_BLAND();

    gc->vertexArray.ep_call = (void (APIENTRY *)(const char *))glEdgeFlagv;
    gc->vertexArray.ep_copy = CopyEdgeFlag;
    gc->vertexArray.edge_pointer = pointer;
    gc->vertexArray.ep_stride = stride ? stride : sizeof(GLboolean);
    gc->vertexArray.ep_usr_stride = stride;
    gc->vertexArray.signature = 
    gc->vertexArray.signature | VERTARRAY_E_MASK;
    gc->vertexArray.ep_count = 0;
    gc->vertexArray.interleavedFormat = GL_NONE;

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_VERTARRAY);
}

/* list comp */

void APIENTRY __gllc_ArrayElement(GLint i) 
{
    __glim_ArrayElement(i);
}

void APIENTRY __gllc_DrawArrays(GLenum mode,
                             GLint first,
                             GLsizei count)
{
    GLint index;
    int i;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    index = gc->vertexArray.index;

    /* check that first and count are positive */
    if (first < 0 || count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_POLYGON) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* could just call ArrayElement here, but let's try to not do
     * anything that ridiculously slow.  can't share with the immediate
     * mode routine since the special cases may dodge the dispatch table,
     * which would mess things up. */

    glBegin(mode);
    count += first;
    for (i = first; i < count; i++) {
        if (index & VERTARRAY_N_INDEX) 
            (*gc->vertexArray.np_call)
                (gc->vertexArray.normal_pointer + 
                 i*gc->vertexArray.np_stride);
        if (index & VERTARRAY_C_INDEX) 
            (*gc->vertexArray.cp_call)
                (gc->vertexArray.color_pointer + 
                 i*gc->vertexArray.cp_stride);
        if (index & VERTARRAY_T_INDEX)
            (*gc->vertexArray.tp_call)
                (gc->vertexArray.tex_coord_pointer + 
                 i*gc->vertexArray.tp_stride);
        if (index & VERTARRAY_I_INDEX) 
            (*gc->vertexArray.ip_call)
                (gc->vertexArray.index_pointer + 
                 i*gc->vertexArray.ip_stride);
        if (index & VERTARRAY_E_INDEX)
            (*gc->vertexArray.ep_call)
                (gc->vertexArray.edge_pointer + 
                 i*gc->vertexArray.ep_stride);
        if (index & VERTARRAY_V_INDEX) 
            (*gc->vertexArray.vp_call)
                (gc->vertexArray.vertex_pointer + 
                 i*gc->vertexArray.vp_stride);
    }
    glEnd();
}

void APIENTRY __gllc_DrawElements(GLenum mode, GLsizei count, GLenum type, 
                        const GLvoid *indices)
{
    GLint index;
    GLuint *elements;
    int i;

    __GL_SETUP_NOT_IN_BEGIN_VALIDATE();

    index = gc->vertexArray.index;

    /* check that count is positive */
    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }

    if (mode > GL_POLYGON) {
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /*
    ** The optimized path is for unsigned int indices, but we
    ** can make the other types go through that path by converting
    ** the indices to unsigned int.  Later we will want to find
    ** a way to propagate the optimized path to all types.
    */
    switch(type) {
    case GL_UNSIGNED_BYTE:
        {
            GLubyte *byteElements = (GLubyte *)indices;
            elements = (GLuint *)
                    gc->imports.malloc(gc, count*sizeof(GLuint));
            for (i=0; i<count; i++) {
                elements[i] = byteElements[i];
            }
        }
        break;
    case GL_UNSIGNED_SHORT:
        {
            GLushort *shortElements = (GLushort *)indices;
            elements = (GLuint *)
                    gc->imports.malloc(gc, count*sizeof(GLuint));
            for (i=0; i<count; i++) {
                elements[i] = shortElements[i];
            }
        }
        break;
    case GL_UNSIGNED_INT:
        elements = (GLuint *)indices;
        break;
    default:
        __glSetError(GL_INVALID_ENUM);
        return;
    }

    /* could just call ArrayElement here, but let's try to not do
     * anything that ridiculously slow.  can't share with the immediate
     * mode routine since the special cases may dodge the dispatch table,
     * which would mess things up. */

    glBegin(mode);
    for (i = 0; i < count; i++) {
        GLint element = elements[i];

        if (index & VERTARRAY_N_INDEX) 
            (*gc->vertexArray.np_call)
                (gc->vertexArray.normal_pointer + 
                 element*gc->vertexArray.np_stride);
        if (index & VERTARRAY_C_INDEX) 
            (*gc->vertexArray.cp_call)
                (gc->vertexArray.color_pointer + 
                 element*gc->vertexArray.cp_stride);
        if (index & VERTARRAY_T_INDEX)
            (*gc->vertexArray.tp_call)
                (gc->vertexArray.tex_coord_pointer + 
                 element*gc->vertexArray.tp_stride);
        if (index & VERTARRAY_I_INDEX) 
            (*gc->vertexArray.ip_call)
                (gc->vertexArray.index_pointer + 
                 element*gc->vertexArray.ip_stride);
        if (index & VERTARRAY_E_INDEX)
            (*gc->vertexArray.ep_call)
                (gc->vertexArray.edge_pointer + 
                 element*gc->vertexArray.ep_stride);
        if (index & VERTARRAY_V_INDEX) 
            (*gc->vertexArray.vp_call)
                (gc->vertexArray.vertex_pointer + 
                 element*gc->vertexArray.vp_stride);
    }
    glEnd();

    if (elements != indices) {
        gc->imports.free(gc, elements);
    }
}

/* EXT_vertex_array */

void APIENTRY __glim_VertexPointerEXT(GLint size, GLenum type, GLsizei stride,
                                    GLsizei count, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    __glim_VertexPointer(size, type, stride, pointer);
    gc->vertexArray.vp_count = count;   /* XXX only if no errors occured */
}

void APIENTRY __glim_NormalPointerEXT(GLenum type, GLsizei stride,
                                    GLsizei count, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    __glim_NormalPointer(type, stride, pointer);
    gc->vertexArray.np_count = count;   /* XXX only if no errors occured */
}

void APIENTRY __glim_ColorPointerEXT(GLint size, GLenum type, GLsizei stride,
                                   GLsizei count, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    __glim_ColorPointer(size, type, stride, pointer);
    gc->vertexArray.cp_count = count;   /* XXX only if no errors occured */
}

void APIENTRY __glim_IndexPointerEXT(GLenum type, GLsizei stride,
                                   GLsizei count, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    __glim_IndexPointer(type, stride, pointer);
    gc->vertexArray.ip_count = count;   /* XXX only if no errors occured */
}

void APIENTRY __glim_TexCoordPointerEXT(GLint size, GLenum type, GLsizei stride,
                                      GLsizei count, const GLvoid *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    __glim_TexCoordPointer(size, type, stride, pointer);
    gc->vertexArray.tp_count = count;   /* XXX only if no errors occured */
}

void APIENTRY __glim_EdgeFlagPointerEXT(GLsizei stride,
                                        GLsizei count, const GLboolean *pointer)
{
    __GL_SETUP_NOT_IN_BEGIN();

    if (count < 0) {
        __glSetError(GL_INVALID_VALUE);
        return;
    }
    __glim_EdgeFlagPointer(stride, pointer);
    gc->vertexArray.ep_count = count;   /* XXX only if no errors occured */
}

GLboolean __glVertexArrayBackDoor(__GLcontext *gc, GLenum pname, GLfloat *params)
{
    switch(pname) {
        case __GL_VARRAY_DISABLE_FAST:
            if (0.0F != params[0]) {
                gc->vertexArray.controlWord |= VERTARRAY_CW_DISABLE_FAST;
            } else {
                gc->vertexArray.controlWord &= ~VERTARRAY_CW_DISABLE_FAST;
            }
            return GL_TRUE;
        case __GL_VARRAY_DISABLE_RASTER:
            if (0.0F != params[0]) {
                gc->vertexArray.controlWord |= VERTARRAY_CW_DISABLE_RASTER;
            } else {
                gc->vertexArray.controlWord &= ~VERTARRAY_CW_DISABLE_RASTER;
            }
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_GENERIC);
            return GL_TRUE;
        case __GL_VARRAY_BATCH_SIZE:
            gc->vertexArray.batchSize = (GLint)params[0];
            return GL_TRUE;
        case __GL_VARRAY_SAME_BUF:
            if (0.0F != params[0]) {
                gc->vertexArray.controlWord |= VERTARRAY_CW_SAME_BUF;
            } else {
                gc->vertexArray.controlWord &= ~VERTARRAY_CW_SAME_BUF;
            }
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_GENERIC);
            return GL_TRUE;
        case __GL_VARRAY_DISABLE_VCACHE:
            if (0.0F != params[0]) {
                gc->vertexCache.vertexCacheEnabled = GL_FALSE;
            } else {
                gc->vertexCache.vertexCacheEnabled = GL_TRUE;
            }
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_GENERIC);
            return GL_TRUE;
        case __GL_VARRAY_SMALL_GLVERTEX:
            if (0.0F != params[0]) {
                gc->vertexArray.controlWord |= VERTARRAY_CW_SMALL_GLVERTEX;
            } else {
                gc->vertexArray.controlWord &= ~VERTARRAY_CW_SMALL_GLVERTEX;
            }
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_GENERIC);
            return GL_TRUE;
        case __GL_VARRAY_SKIP_VXCULL:
            if (0.0F != params[0]) {
                gc->vertexArray.controlWord |= VERTARRAY_CW_SKIP_VXCULL;
            } else {
                gc->vertexArray.controlWord &= ~VERTARRAY_CW_SKIP_VXCULL;
            }
            __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_GENERIC);
            return GL_TRUE;
        default:
            return GL_FALSE;
    }
}

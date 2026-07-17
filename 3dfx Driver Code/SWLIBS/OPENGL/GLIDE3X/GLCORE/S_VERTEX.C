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
#include "global.h"
#include "g_imfncs.h"
#include "mips.h"
#include "intel.h"

#ifndef __GL_USE_MIPSASMCODE

#ifndef __GL_USE_INTEL_ASM

/*
** Clip check the clip coordinates against the frustum planes.
*/
GLuint __glClipCheckFrustum(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x, y, z, w, negW, invW;
    GLuint code;
    
    /* Set clip codes */
    code = 0;

        w = vx->clip.w;
        invW = __glOne / w;
        x = vx->clip.x;
        y = vx->clip.y;
        z = vx->clip.z;
        negW = -w;
    
        if (x < negW) code |= __GL_CLIP_LEFT;
        if (x > w) code |= __GL_CLIP_RIGHT;
        if (y < negW) code |= __GL_CLIP_BOTTOM;
        if (y > w) code |= __GL_CLIP_TOP;
        if (z < negW) code |= __GL_CLIP_NEAR;
        if (z > w) code |= __GL_CLIP_FAR;

    vx->window.w = invW;
    vx->hasAndClipCode |= __GL_HAS_CLIP;
    
    /* Compute window coordinates if not clipped */
    if (!code) {
        __GLviewport *vp = &gc->state.viewport;
        __GLfloat wx, wy, wz;
        
        wx = x * vp->xScale * invW + vp->xCenter;
        wy = y * vp->yScale * invW + vp->yCenter;
        wz = z * vp->zScale * invW + vp->zCenter;
        vx->window.x = wx;
        vx->window.y = wy;
        vx->window.z = wz;
    }
    
    return code;
}
#else
GLuint __glClipCheckFrustum(__GLcontext *gc, __GLvertex *pVertex )
{
    __GLviewport *vp = &gc->state.viewport;
    GLuint code;

    if ( -pVertex->clip.w < 0.0F )
    {
        
        __asm mov edi, pVertex

        __asm mov eax, dword ptr [edi+V_CLIP_W]
        __asm mov esi, 0
        
        __asm cmp eax, 03f800000h
        __asm jne do_divide
        __asm fld dword ptr [edi+V_CLIP_W]
        __asm jmp do_clip_check
do_divide:
        __asm fld1
        __asm fdiv dword ptr [edi+V_CLIP_W]
            
do_clip_check:

        __asm mov ebx, dword ptr [edi+V_CLIP_X]
        __asm mov ecx, ebx
        __asm and ebx, 7fffffffh
        __asm sar ecx, 31
        __asm cmp eax, ebx
        __asm sbb ebx, ebx
        __asm xor ecx, __GL_CLIP_RIGHT
        __asm and ecx, __GL_CLIP_RIGHT | __GL_CLIP_LEFT
        __asm and ebx, ecx
        __asm or  esi, ebx

        __asm mov ebx, dword ptr [edi+V_CLIP_Y]
        __asm mov ecx, ebx
        __asm and ebx, 7fffffffh
        __asm sar ecx, 31
        __asm cmp eax, ebx
        __asm sbb ebx, ebx
        __asm xor ecx, __GL_CLIP_TOP
        __asm and ecx, __GL_CLIP_TOP | __GL_CLIP_BOTTOM
        __asm and ebx, ecx
        __asm or  esi, ebx

        __asm mov ebx, dword ptr [edi+V_CLIP_Z]
        __asm mov ecx, ebx
        __asm and ebx, 7fffffffh
        __asm sar ecx, 31
        __asm cmp eax, ebx
        __asm sbb ebx, ebx
        __asm xor ecx, __GL_CLIP_FAR
        __asm and ecx, __GL_CLIP_FAR | __GL_CLIP_NEAR
        __asm and ebx, ecx
        __asm or  esi, ebx

        __asm mov code, esi

        __asm fstp dword ptr [edi+V_WINDOW_W]
        __asm or   dword ptr [edi+V_HAS], __GL_HAS_CLIP
        __asm cmp  esi, 0
        __asm jnz  skip_rest
            
        __asm mov  esi, vp
        __asm fld  dword ptr [edi+V_CLIP_X]    ; x
        __asm fmul dword ptr [esi+VP_XSCALE]   ; x*xs
        __asm fld  dword ptr [edi+V_CLIP_Y]    ; y | x*xs
        __asm fmul dword ptr [esi+VP_YSCALE]   ; y*ys | x*xs
        __asm fld  dword ptr [edi+V_CLIP_Z]    ; z | y*ys | x*xs
        __asm fmul dword ptr [esi+VP_ZSCALE]   ; z*zs | y*ys | x*xs
        __asm fxch st(2)                       ; x*xs | y*ys | z*zs
        __asm fmul dword ptr [edi+V_WINDOW_W]  ; x*xs+xc | y*ys | z*zs
        __asm fxch st(1)                       ; y*ys | x*xs*invw | z*zs
        __asm fmul dword ptr [edi+V_WINDOW_W]  ; y*ys*invw | x*xs*invw | z*zs
        __asm fxch st(2)                       ; z*zs | x*xs*invw | y*ys*invw 
        __asm fmul dword ptr [edi+V_WINDOW_W]  ; z*zs*invw | x*xs*invw | y*ys*invw 
        __asm fxch st(1)                       ; x*xs*invw | z*zs*invw | y*ys*invw 
        __asm fadd dword ptr [esi+VP_XCENTER]  ; X' | z*zs*invw | y*ys*invw 
        __asm fxch st(2)                       ; y*ys*invw | z*zs*invw | X'
        __asm fadd dword ptr [esi+VP_YCENTER]  ; Y' | z*zs*invw | X'
        __asm fxch st(1)                       ; z*zs*invw | Y' | X'
        __asm fadd dword ptr [esi+VP_ZCENTER]  ; Z' | Y' | X'
        __asm fxch st(2)                       ; X' | Y' | Z'
        __asm fstp dword ptr [edi+V_WINDOW_X]  ; Y' | Z'
        __asm fstp dword ptr [edi+V_WINDOW_Y]  ; Z'
        __asm fstp dword ptr [edi+V_WINDOW_Z]  ; (empty)
            
skip_rest:
        return code;
    }
    else
    {
        __GLfloat x, y, z, w, invW, negW;
        GLuint code;
        
        w = pVertex->clip.w;
        x = pVertex->clip.x;
        y = pVertex->clip.y;
        z = pVertex->clip.z;
        negW = -w;
        invW = (w == (__GLfloat)1.0 ? (__GLfloat)1.0 : (__GLfloat)1.0 / w);
        
        /* Set clip codes */
        code = 0;
        
        if ( w > 0.0F )
        {
            if ( x < negW ) code |= __GL_CLIP_LEFT;
            else if (x > w) code |= __GL_CLIP_RIGHT;
            if (y < negW) code |= __GL_CLIP_BOTTOM;
            else if (y > w) code |= __GL_CLIP_TOP;
            if (z < negW) code |= __GL_CLIP_NEAR;
            else if (z > w) code |= __GL_CLIP_FAR;
        }
        else
        {
            if ( x < negW ) code |= __GL_CLIP_LEFT;
            if (x > w) code |= __GL_CLIP_RIGHT;
            if (y < negW) code |= __GL_CLIP_BOTTOM;
            if (y > w) code |= __GL_CLIP_TOP;
            if (z < negW) code |= __GL_CLIP_NEAR;
            if (z > w) code |= __GL_CLIP_FAR;
        }
        
        pVertex->hasAndClipCode |= __GL_HAS_CLIP;
        if (code) return code;
        
        /* Compute window coordinates if not clipped */
        {
            __GLfloat wx, wy, wz;
            
            wx = x * vp->xScale * invW + vp->xCenter;
            wy = y * vp->yScale * invW + vp->yCenter;
            wz = z * vp->zScale * invW + vp->zCenter;
            pVertex->window.w = invW;
            pVertex->window.x = wx;
            pVertex->window.y = wy;
            pVertex->window.z = wz;
        }
    }
}
#endif

/*
** Clip check the clip coordinates against the frustum planes.
*/
GLuint __glClipCheckFrustum2D(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x, y, z, w, negW, invW;
    GLuint code;

    w = vx->clip.w;
    invW = w;           /* W is 1.0 */
    x = vx->clip.x;
    y = vx->clip.y;
    negW = -w;

    /* Set clip codes */
    code = 0;

    /*
    ** NOTE: it is possible for x to be less than negW and greater than w
    ** (if w is negative).  Otherwise there would be "else" clauses here.
    */
    if (x < negW) code |= __GL_CLIP_LEFT;
    if (x > w) code |= __GL_CLIP_RIGHT;
    if (y < negW) code |= __GL_CLIP_BOTTOM;
    if (y > w) code |= __GL_CLIP_TOP;

    vx->window.w = invW;
    vx->hasAndClipCode |= __GL_HAS_CLIP;

    if (!code) {
        __GLviewport *vp = &gc->state.viewport;
        __GLfloat wx, wy, wz;

        z = vx->clip.z;
        wx = x * vp->xScale + vp->xCenter;
        wy = y * vp->yScale + vp->yCenter;
        wz = z * vp->zScale + vp->zCenter;
        vx->window.x = wx;
        vx->window.y = wy;
        vx->window.z = wz;
    }

    return code;
}
#endif

GLuint __glClipCheckAll2(__GLcontext *gc, __GLvertex *vx)
{
    vx->obj.z = __glZero;
    vx->obj.w = __glOne;
    return __glClipCheckAll(gc, vx);
}

GLuint __glClipCheckAll3(__GLcontext *gc, __GLvertex *vx)
{
    vx->obj.w = __glOne;
    return __glClipCheckAll(gc, vx);
}


#ifdef NOTUSED
/*
** Clip check if matrix is 2D.  Convert straight to window coords,
** bypassing clip coords and compute clip codes.  Incoming vertex was
** from a glVertex2*() call.
*/
GLuint __glClipCheck2D_2(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x, y, ox, oy;
    GLuint code;
    __GLmatrix *m;

    m = &(gc->transform.matrix2D);

    ox = vx->obj.x;
    oy = vx->obj.y;

    x = ox * m->matrix[0][0] + oy * m->matrix[1][0] + m->matrix[3][0];
    y = ox * m->matrix[0][1] + oy * m->matrix[1][1] + m->matrix[3][1];

    code = 0;
    if (x < gc->transform.fminx) code |= __GL_CLIP_LEFT;
    if (x >= gc->transform.fmaxx) code |= __GL_CLIP_RIGHT;
    if (y < gc->transform.fminy) {
        if (gc->constants.yInverted) {
            code |= __GL_CLIP_TOP;
        } else {
            code |= __GL_CLIP_BOTTOM;
        }
    }
    if (y >= gc->transform.fmaxy) {
        if (gc->constants.yInverted) {
            code |= __GL_CLIP_BOTTOM;
        } else {
            code |= __GL_CLIP_TOP;
        }
    }

    vx->window.x = x;
    vx->window.y = y;
    vx->hasAndClipCode |= __GL_HAS_CLIP;
    /* z is not needed */

    return code;
}


/*
** Clip check if matrix is 2D.  Convert straight to window coords,
** bypassing clip coords and compute clip codes.  Incoming vertex was
** from a glVertex3*() call.
*/
GLuint __glClipCheck2D_3(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat ox, oy, x, y, z;
    GLuint code;
    __GLmatrix *m;

    m = &(gc->transform.matrix2D);

    ox = vx->obj.x;
    oy = vx->obj.y;
    z = vx->obj.z;

    x = ox * m->matrix[0][0] + oy * m->matrix[1][0] + m->matrix[3][0];
    y = ox * m->matrix[0][1] + oy * m->matrix[1][1] + m->matrix[3][1];
    z =  z * m->matrix[2][2] + m->matrix[3][2];

    code = 0;
    if (x < gc->transform.fminx) code |= __GL_CLIP_LEFT;
    if (x >= gc->transform.fmaxx) code |= __GL_CLIP_RIGHT;
    if (y < gc->transform.fminy) {
        if (gc->constants.yInverted) {
            code |= __GL_CLIP_TOP;
        } else {
            code |= __GL_CLIP_BOTTOM;
        }
    }
    if (y >= gc->transform.fmaxy) {
        if (gc->constants.yInverted) {
            code |= __GL_CLIP_BOTTOM;
        } else {
            code |= __GL_CLIP_TOP;
        }
    }
    if (z > __glOne) code |= __GL_CLIP_FAR;
    if (z < -1.0f) code |= __GL_CLIP_NEAR;
    vx->hasAndClipCode |= __GL_HAS_CLIP;

    if (!code) {
        vx->window.x = x;
        vx->window.y = y;
        /* z is not needed */
    }

    return code;
}
#endif /* NOTUSED */

/*
** Clip check against the frustum and user clipping planes.
*/
GLuint __glClipCheckAll(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x, y, z, w, negW, invW, zero, dot;
    GLuint code, bit, clipPlanesMask;
    __GLcoord *plane;

    /* We need eye coordinates to do user clip plane clipping */
    DO_VALIDATE(gc, vx, __GL_HAS_EYE);

    /*
    ** Do frustum checks.
    **
    ** NOTE: it is possible for x to be less than negW and greater than w
    ** (if w is negative).  Otherwise there would be "else" clauses here.
    */
    w = vx->clip.w;
    invW = __glOne / w;
    x = vx->clip.x;
    y = vx->clip.y;
    z = vx->clip.z;
    negW = -w;
    code = 0;
    if (x < negW) code |= __GL_CLIP_LEFT;
    if (x > w) code |= __GL_CLIP_RIGHT;
    if (y < negW) code |= __GL_CLIP_BOTTOM;
    if (y > w) code |= __GL_CLIP_TOP;
    if (z < negW) code |= __GL_CLIP_NEAR;
    if (z > w) code |= __GL_CLIP_FAR;
    vx->window.w = invW;

    /*
    ** Now do user clip plane checks
    */
    x = vx->eye.x;
    y = vx->eye.y;
    z = vx->eye.z;
    w = vx->eye.w;
    clipPlanesMask = gc->state.enables.clipPlanes;
    plane = &gc->state.transform.eyeClipPlanes[0];
    bit = __GL_CLIP_USER0;
    zero = __glZero;
    while (clipPlanesMask) {
        if (clipPlanesMask & 1) {
            /*
            ** Dot the vertex clip coordinate against the clip plane and see
            ** if the sign is negative.  If so, then the point is out.
            */
            dot = x * plane->x + y * plane->y + z * plane->z + w * plane->w;
            if (dot < zero) {
                code |= bit;
            }
        }
        clipPlanesMask >>= 1;
        bit <<= 1;
        plane++;
    }
    vx->hasAndClipCode |= __GL_HAS_CLIP;

    /* Compute window coordinates if not clipped */
    if (!code) {
        __GLviewport *vp = &gc->state.viewport;
        __GLfloat wx, wy, wz;

        x = vx->clip.x;
        y = vx->clip.y;
        z = vx->clip.z;
        wx = x * vp->xScale * invW + vp->xCenter;
        wy = y * vp->yScale * invW + vp->yCenter;
        wz = z * vp->zScale * invW + vp->zCenter;
        vx->window.x = wx;
        vx->window.y = wy;
        vx->window.z = wz;
    }

    return code;
}

#ifndef __GL_USE_MIPSASMCODE
/************************************************************************
** NOTE: This code is written like this so the compiler will realize that 
** there is no aliasing going on here, and it will do something reasonable
** (as opposed to the code it usually generates).
*/

void __glSaveN(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x,y,z;
    void (*vertex)(__GLcontext *gc, __GLvertex *vx);

    vertex = gc->procs.vertex;
    x = gc->state.current.normal.x;
    y = gc->state.current.normal.y;
    z = gc->state.current.normal.z;
    vx->normal.x = x;
    vx->normal.y = y;
    vx->normal.z = z;
    (*vertex)(gc, vx);
}

void __glSaveCI(__GLcontext *gc, __GLvertex *vx)
{
    void (*vertex)(__GLcontext *gc, __GLvertex *vx);
    __GLfloat index;

    vertex = gc->procs.vertex;
    index = gc->state.current.userColorIndex;
    vx->colors[__GL_FRONTFACE].r = index;
    (*vertex)(gc, vx);
}

void __glSaveC(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat r,g,b,a;
    void (*vertex)(__GLcontext *gc, __GLvertex *vx);

    vertex = gc->procs.vertex;
    r = gc->state.current.color.r;
    g = gc->state.current.color.g;
    b = gc->state.current.color.b;
    a = gc->state.current.color.a;
    vx->colors[__GL_FRONTFACE].r = r;
    vx->colors[__GL_FRONTFACE].g = g;
    vx->colors[__GL_FRONTFACE].b = b;
    vx->colors[__GL_FRONTFACE].a = a;
    (*vertex)(gc, vx);
}

void __glSaveT(__GLcontext *gc, __GLvertex *vx)
{
    void (*vertex)(__GLcontext *gc, __GLvertex *vx);
    __GLfloat x,y,z,w;

    vertex = gc->procs.vertex;
    x = gc->state.current.texture[0].x;
    y = gc->state.current.texture[0].y;
    z = gc->state.current.texture[0].z;
    w = gc->state.current.texture[0].w;
    vx->texture[0].x = x;
    vx->texture[0].y = y;
    vx->texture[0].z = z;
    vx->texture[0].w = w;
    x = gc->state.current.texture[1].x;
    y = gc->state.current.texture[1].y;
    z = gc->state.current.texture[1].z;
    w = gc->state.current.texture[1].w;
    vx->texture[1].x = x;
    vx->texture[1].y = y;
    vx->texture[1].z = z;
    vx->texture[1].w = w;
    (*vertex)(gc, vx);
}

void __glSaveCT(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat r,g,b,a;
    __GLfloat x,y,z,w;
    void (*vertex)(__GLcontext *gc, __GLvertex *vx);

    vertex = gc->procs.vertex;
    r = gc->state.current.color.r;
    g = gc->state.current.color.g;
    b = gc->state.current.color.b;
    a = gc->state.current.color.a;
    vx->colors[__GL_FRONTFACE].r = r;
    vx->colors[__GL_FRONTFACE].g = g;
    vx->colors[__GL_FRONTFACE].b = b;
    vx->colors[__GL_FRONTFACE].a = a;
    x = gc->state.current.texture[0].x;
    y = gc->state.current.texture[0].y;
    z = gc->state.current.texture[0].z;
    w = gc->state.current.texture[0].w;
    vx->texture[0].x = x;
    vx->texture[0].y = y;
    vx->texture[0].z = z;
    vx->texture[0].w = w;
    x = gc->state.current.texture[1].x;
    y = gc->state.current.texture[1].y;
    z = gc->state.current.texture[1].z;
    w = gc->state.current.texture[1].w;
    vx->texture[1].x = x;
    vx->texture[1].y = y;
    vx->texture[1].z = z;
    vx->texture[1].w = w;
    (*vertex)(gc, vx);
}

void __glSaveNT(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x,y,z,w;
    void (*vertex)(__GLcontext *gc, __GLvertex *vx);

    vertex = gc->procs.vertex;
    x = gc->state.current.normal.x;
    y = gc->state.current.normal.y;
    z = gc->state.current.normal.z;
    vx->normal.x = x;
    vx->normal.y = y;
    vx->normal.z = z;
    x = gc->state.current.texture[0].x;
    y = gc->state.current.texture[0].y;
    z = gc->state.current.texture[0].z;
    w = gc->state.current.texture[0].w;
    vx->texture[0].x = x;
    vx->texture[0].y = y;
    vx->texture[0].z = z;
    vx->texture[0].w = w;
    x = gc->state.current.texture[1].x;
    y = gc->state.current.texture[1].y;
    z = gc->state.current.texture[1].z;
    w = gc->state.current.texture[1].w;
    vx->texture[1].x = x;
    vx->texture[1].y = y;
    vx->texture[1].z = z;
    vx->texture[1].w = w;
    (*vertex)(gc, vx);
}

void __glSaveCIAll(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat x,y,z,w;
    void (*vertex)(__GLcontext *gc, __GLvertex *vx);
    __GLfloat index;

    index = gc->state.current.userColorIndex;
    vertex = gc->procs.vertex;
    x = gc->state.current.normal.x;
    y = gc->state.current.normal.y;
    z = gc->state.current.normal.z;
    vx->colors[__GL_FRONTFACE].r = index;
    vx->normal.x = x;
    vx->normal.y = y;
    vx->normal.z = z;
    x = gc->state.current.texture[0].x;
    y = gc->state.current.texture[0].y;
    z = gc->state.current.texture[0].z;
    w = gc->state.current.texture[0].w;
    vx->texture[0].x = x;
    vx->texture[0].y = y;
    vx->texture[0].z = z;
    vx->texture[0].w = w;
    x = gc->state.current.texture[1].x;
    y = gc->state.current.texture[1].y;
    z = gc->state.current.texture[1].z;
    w = gc->state.current.texture[1].w;
    vx->texture[1].x = x;
    vx->texture[1].y = y;
    vx->texture[1].z = z;
    vx->texture[1].w = w;
    (*vertex)(gc, vx);
}

void __glSaveCAll(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat r,g,b,a;
    __GLfloat x,y,z,w;
    void (*vertex)(__GLcontext *gc, __GLvertex *vx);

    vertex = gc->procs.vertex;
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
    x = gc->state.current.texture[0].x;
    y = gc->state.current.texture[0].y;
    z = gc->state.current.texture[0].z;
    w = gc->state.current.texture[0].w;
    vx->texture[0].x = x;
    vx->texture[0].y = y;
    vx->texture[0].z = z;
    vx->texture[0].w = w;
    x = gc->state.current.texture[1].x;
    y = gc->state.current.texture[1].y;
    z = gc->state.current.texture[1].z;
    w = gc->state.current.texture[1].w;
    vx->texture[1].x = x;
    vx->texture[1].y = y;
    vx->texture[1].z = z;
    vx->texture[1].w = w;
    (*vertex)(gc, vx);
}
#endif  /* !__GL_USE_MIPSASMCODE */

void __glSaveCIT(__GLcontext *gc, __GLvertex *vx)
{
    __GLfloat index;
    __GLfloat x,y,z,w;
    void (*vertex)(__GLcontext *gc, __GLvertex *vx);

    vertex = gc->procs.vertex;
    index = gc->state.current.userColorIndex;
    vx->colors[__GL_FRONTFACE].r = index;
    x = gc->state.current.texture[0].x;
    y = gc->state.current.texture[0].y;
    z = gc->state.current.texture[0].z;
    w = gc->state.current.texture[0].w;
    vx->texture[0].x = x;
    vx->texture[0].y = y;
    vx->texture[0].z = z;
    vx->texture[0].w = w;
    x = gc->state.current.texture[1].x;
    y = gc->state.current.texture[1].y;
    z = gc->state.current.texture[1].z;
    w = gc->state.current.texture[1].w;
    vx->texture[1].x = x;
    vx->texture[1].y = y;
    vx->texture[1].z = z;
    vx->texture[1].w = w;
    (*vertex)(gc, vx);
}

/************************************************************************/


#define __NORMAL 1
#define __COLOR 2
#define __TEX 4

static void (*CISaveProcs[8])(__GLcontext*, __GLvertex*) = {
    0,                  /* none */
    __glSaveN,          /* __NORMAL */
    __glSaveCI,         /* __COLOR */
    0,                  /* __NORMAL | __COLOR */

    __glSaveT,          /* __TEX */
    __glSaveNT,         /* __NORMAL | __TEX */

    __glSaveCIT,        /* __COLOR | __TEX */
    __glSaveCIAll,      /* __NORMAL | __COLOR | __TEX */
};

static void (*RGBSaveProcs[8])(__GLcontext*, __GLvertex*) = {
    0,                  /* none */
    __glSaveN,          /* __NORMAL */

    __glSaveC,          /* __COLOR */
    0,                  /* __NORMAL | __COLOR */

    __glSaveT,          /* __TEX */
    __glSaveNT,         /* __NORMAL | __TEX */

    __glSaveCT,         /* __COLOR | __TEX */
    __glSaveCAll,       /* __NORMAL | __COLOR | __TEX */
};

void __glGenericPickVertexProcs(__GLcontext *gc)
{
    GLuint enables = gc->state.enables.general;
    GLuint texenables = gc->state.enables.texture[0] | gc->state.enables.texture[1];
    GLint ix;
    GLenum mvpMatrixType;
    __GLmatrix *m;

    m = &(gc->transform.modelView->mvp);
    mvpMatrixType = m->matrixType;

    /* Pick clipCheck proc */
#ifdef __GL_USE_MIPSASMCODE
    if (gc->state.enables.clipPlanes) {
        /*
        ** Transform and clip routines that are called from the assemblerized
        ** vertex handlers are supposed to preserve t0, t1, t2 and t3.
        ** Since we don't have assembly code versions for ClipCheckAll
        ** routines we call assembly-code wrappers that save those registers
        ** and then call the equivalent c code routine.
        */
        gc->procs.clipCheck2 = __glMipsClipCheckAll2;
        gc->procs.clipCheck3 = __glMipsClipCheckAll3;
        gc->procs.clipCheck4 = __glMipsClipCheckAll;
    } else {
        /*
        ** Pick faster clip checkers if we know that incoming W is 1 and 
        ** mvp matrix doesn't change it.
        */
        if (mvpMatrixType >= __GL_MT_W0001) {
            if (mvpMatrixType >= __GL_MT_IS2D &&
                    m->matrix[3][2] >= -1.0f && m->matrix[3][2] <= 1.0f) {
                gc->procs.clipCheck2 = __glMipsClipCheckFrustum2D;
            } else {
                gc->procs.clipCheck2 = __glMipsClipCheckFrustum_W;
            }
            gc->procs.clipCheck3 = __glMipsClipCheckFrustum_W;
        } else {
            gc->procs.clipCheck2 = __glMipsClipCheckFrustum;
            gc->procs.clipCheck3 = __glMipsClipCheckFrustum;
        }

        gc->procs.clipCheck4 = __glMipsClipCheckFrustum;
    }
#else
    if (gc->state.enables.clipPlanes) {
        gc->procs.clipCheck2 = __glClipCheckAll2;
        gc->procs.clipCheck3 = __glClipCheckAll3;
        gc->procs.clipCheck4 = __glClipCheckAll;
    } else {
        gc->procs.clipCheck3 = __glClipCheckFrustum;
        gc->procs.clipCheck4 = __glClipCheckFrustum;
        if (mvpMatrixType >= __GL_MT_IS2D &&
                m->matrix[3][2] >= -1.0f && m->matrix[3][2] <= 1.0f) {
            gc->procs.clipCheck2 = __glClipCheckFrustum2D;
        } else {
            gc->procs.clipCheck2 = __glClipCheckFrustum;
        }
    }
#endif

    /* Pick vertex validation routines */
    if (gc->vertex.faceNeeds[__GL_FRONTFACE] == 0) {
        gc->procs.validateVertex0 = 
                (void (*)(__GLcontext *, __GLvertex *, GLuint)) __glNop;
        gc->procs.validateVertex2 = 
                (void (*)(__GLcontext *, __GLvertex *, GLuint)) __glNop;
        gc->procs.validateVertex3 =
                (void (*)(__GLcontext *, __GLvertex *, GLuint)) __glNop;
        gc->procs.validateVertex4 = 
                (void (*)(__GLcontext *, __GLvertex *, GLuint)) __glNop;
    } else {
        gc->procs.validateVertex0 = 
                (void (*)(__GLcontext *, __GLvertex *, GLuint)) __glNop;
        gc->procs.validateVertex2 = __glValidateVertex2;
        gc->procs.validateVertex3 = __glValidateVertex3;
        gc->procs.validateVertex4 = __glValidateVertex4;
    }

    if (gc->renderMode == GL_FEEDBACK) {
        if (gc->modes.colorIndexMode) {
            gc->procs.v = __glSaveCIAll;
        } else {
            gc->procs.v = __glSaveCAll;
        }
    } else {
        ix = 0;
        if (enables & __GL_LIGHTING_ENABLE) {
            ix |= __NORMAL;
        } else {
            ix |= __COLOR;
        }
        if (gc->texture.textureEnabled) {
            ix |= __TEX;
            if (texenables & __GL_TEXTURE_GEN_S_ENABLE) {
                if (gc->state.texture[gc->texture.currentTexUnit].s.mode == GL_SPHERE_MAP) {
                    ix |= __NORMAL;
                }
            }
            if (texenables & __GL_TEXTURE_GEN_T_ENABLE) {
                if (gc->state.texture[gc->texture.currentTexUnit].t.mode == GL_SPHERE_MAP) {
                    ix |= __NORMAL;
                }
            }
        }
        if (gc->modes.rgbMode) {
            gc->procs.v = RGBSaveProcs[ix];
        } else {
            gc->procs.v = CISaveProcs[ix];
        }
        assert(gc->procs.v != 0);
    }
}

/************************************************************************/

#ifndef __GL_USE_MIPSASMCODE
void __glValidateVertex2(__GLcontext *gc, __GLvertex *vx, GLuint needs)
{
    GLuint has = vx->hasAndClipCode;
    GLuint wants = needs & ~has;

    if (wants & __GL_HAS_EYE) {
        __GLtransform *tr = gc->transform.modelView;
        (*tr->matrix.xf2)(&vx->eye, &vx->obj.x, &tr->matrix);
    }
    if (wants & __GL_HAS_NORMAL) {
        __GLcoord ne;
        __GLtransform *tr = gc->transform.modelView;

        if (tr->updateInverse) {
            (*gc->procs.computeInverseTranspose)(gc, tr);
        }
        if (tr->inverseTranspose.matrixType == __GL_MT_GENERAL) {
            /* this is needed only if there is projection info */
            vx->normal.w =
                -(vx->normal.x * vx->obj.x + vx->normal.y * vx->obj.y);
        }
        if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
            (*tr->inverseTranspose.xf3)(&ne, &vx->normal.x, 
                    &tr->inverseTranspose);
            (*gc->procs.normalize)(&vx->normal.x, &ne.x);
        } else {
            (*tr->inverseTranspose.xf3)(&vx->normal,
                    &vx->normal.x, &tr->inverseTranspose);
        }
    }
    if (wants & __GL_HAS_FOG) {
        vx->fog = (*gc->procs.fogVertex)(gc, vx);
    }
    if (wants & __GL_HAS_TEXTURE) {
        vx->obj.z = __glZero;
        vx->obj.w = __glOne;
        (*gc->procs.calcTexture)(gc, vx);
    }
    if (wants & __GL_HAS_FRONT_COLOR) {
        (*gc->procs.calcColor)(gc, __GL_FRONTFACE, vx);
    } 
    if (wants & __GL_HAS_BACK_COLOR) {
        (*gc->procs.calcColor)(gc, __GL_BACKFACE, vx);
    }

    vx->hasAndClipCode = has | wants;
}

void __glValidateVertex3(__GLcontext *gc, __GLvertex *vx, GLuint needs)
{
    GLuint has = vx->hasAndClipCode;
    GLuint wants = needs & ~has;

    if (wants & __GL_HAS_EYE) {
        __GLtransform *tr = gc->transform.modelView;
        (*tr->matrix.xf3)(&vx->eye, &vx->obj.x, &tr->matrix);
    }

    if (wants & __GL_HAS_NORMAL) {
        __GLcoord ne;
        __GLtransform *tr = gc->transform.modelView;

        if (tr->updateInverse) {
            (*gc->procs.computeInverseTranspose)(gc, tr);
        }
        if (tr->inverseTranspose.matrixType == __GL_MT_GENERAL) {
            /* this is needed only if there is projection info */
            vx->normal.w =
                -(vx->normal.x * vx->obj.x + vx->normal.y * vx->obj.y
                         + vx->normal.z * vx->obj.z);
        }
        if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
            (*tr->inverseTranspose.xf3)(&ne, &vx->normal.x, 
                    &tr->inverseTranspose);
            (*gc->procs.normalize)(&vx->normal.x, &ne.x);
        } else {
            (*tr->inverseTranspose.xf3)(&vx->normal,
                    &vx->normal.x, &tr->inverseTranspose);
        }
    }
    if (wants & __GL_HAS_FOG) {
        vx->fog = (*gc->procs.fogVertex)(gc, vx);
    }
    if (wants & __GL_HAS_TEXTURE) {
        vx->obj.w = __glOne;
        (*gc->procs.calcTexture)(gc, vx);
    }
    if (wants & __GL_HAS_FRONT_COLOR) {
        (*gc->procs.calcColor)(gc, __GL_FRONTFACE, vx);
    } 
    if (wants & __GL_HAS_BACK_COLOR) {
        (*gc->procs.calcColor)(gc, __GL_BACKFACE, vx);
    }

    vx->hasAndClipCode = has | wants;
}

void __glValidateVertex4(__GLcontext *gc, __GLvertex *vx, GLuint needs)
{
    GLuint has = vx->hasAndClipCode;
    GLuint wants = needs & ~has;

    if (wants & __GL_HAS_EYE) {
        __GLtransform *tr = gc->transform.modelView;
        (*tr->matrix.xf4)(&vx->eye, &vx->obj.x, &tr->matrix);
    }
    if (wants & __GL_HAS_NORMAL) {
        __GLcoord ne;
        __GLtransform *tr = gc->transform.modelView;

        if (tr->updateInverse) {
            (*gc->procs.computeInverseTranspose)(gc, tr);
        }
        if (tr->inverseTranspose.matrixType == __GL_MT_GENERAL && vx->obj.w) {
            /* this is needed only if there is projection info */
            vx->normal.w = -(vx->normal.x * vx->obj.x + vx->normal.y * vx->obj.y
                             + vx->normal.z * vx->obj.z) / vx->obj.w;
        } else {
            vx->normal.w = __glZero;
        }
        if (gc->state.enables.general & __GL_NORMALIZE_ENABLE) {
            (*tr->inverseTranspose.xf3)(&ne, &vx->normal.x, 
                    &tr->inverseTranspose);
            (*gc->procs.normalize)(&vx->normal.x, &ne.x);
        } else {
            (*tr->inverseTranspose.xf3)(&vx->normal,
                    &vx->normal.x, &tr->inverseTranspose);
        }
    }
    if (wants & __GL_HAS_FOG) {
        vx->fog = (*gc->procs.fogVertex)(gc, vx);
    }
    if (wants & __GL_HAS_TEXTURE) {
        (*gc->procs.calcTexture)(gc, vx);
    }
    if (wants & __GL_HAS_FRONT_COLOR) {
        (*gc->procs.calcColor)(gc, __GL_FRONTFACE, vx);
    } 
    if (wants & __GL_HAS_BACK_COLOR) {
        (*gc->procs.calcColor)(gc, __GL_BACKFACE, vx);
    }

    vx->hasAndClipCode = has | wants;
}
#endif

void __glValidateVertex2Clip(__GLcontext *gc, __GLvertex *vx, GLuint needs)
{
    __GLtransform *tr;

    tr = gc->transform.modelView;
    if (needs & __GL_HAS_CLIP) {
        (*tr->mvp.xf2)(&vx->clip, &vx->obj.x, &tr->mvp);
        vx->hasAndClipCode |= __GL_HAS_CLIP;
    }
}

void __glValidateVertex3Clip(__GLcontext *gc, __GLvertex *vx, GLuint needs)
{
    __GLtransform *tr;

    tr = gc->transform.modelView;
    if (needs & __GL_HAS_CLIP) {
        (*tr->mvp.xf3)(&vx->clip, &vx->obj.x, &tr->mvp);
        vx->hasAndClipCode |= __GL_HAS_CLIP;
    }
}

void __glValidateVertex4Clip(__GLcontext *gc, __GLvertex *vx, GLuint needs)
{
    __GLtransform *tr;

    tr = gc->transform.modelView;
    if (needs & __GL_HAS_CLIP) {
        (*tr->mvp.xf4)(&vx->clip, &vx->obj.x, &tr->mvp);
        vx->hasAndClipCode |= __GL_HAS_CLIP;
    }
}

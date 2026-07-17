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
** $Date: 10/11/00 7:51:03 PM$
*/
#include "context.h"
#include "global.h"

/*
** Clipping macros.  These are used to reduce the amount of code
** hand written below.
*/
#define __GL_CLIP_POS(d,a,b,t) \
    d->clip.w = t*(a->clip.w - b->clip.w) + b->clip.w;  \
    d->window.w = ((__GLfloat) 1.0) / d->clip.w;        \
    d->clip.x = t*(a->clip.x - b->clip.x) + b->clip.x;  \
    d->clip.y = t*(a->clip.y - b->clip.y) + b->clip.y;  \
    d->clip.z = t*(a->clip.z - b->clip.z) + b->clip.z;  \
    d->hasAndClipCode = __GL_CLIP_MASK

/*
** The following is done this way since when we are slow fogging we want to
** clip the eye.z coordinate only, while when we are cheap fogging we want
** to clip the fog value.  This way we avoid doubling the number of clip
** routines.
*/
#define __GL_CLIP_FOG(d,a,b,t)  \
    if (a->hasAndClipCode & __GL_HAS_FOG) \
        d->fog = t * (a->fog - b->fog) + b->fog; \
    else \
        d->eye.z = t*(a->eye.z - b->eye.z) + b->eye.z

#define __GL_CLIP_COLOR(d,a,b,t)                                      \
    d->colors[__GL_FRONTFACE].r = t*(a->colors[__GL_FRONTFACE].r      \
        - b->colors[__GL_FRONTFACE].r) + b->colors[__GL_FRONTFACE].r; \
    d->colors[__GL_FRONTFACE].g = t*(a->colors[__GL_FRONTFACE].g      \
        - b->colors[__GL_FRONTFACE].g) + b->colors[__GL_FRONTFACE].g; \
    d->colors[__GL_FRONTFACE].b = t*(a->colors[__GL_FRONTFACE].b      \
        - b->colors[__GL_FRONTFACE].b) + b->colors[__GL_FRONTFACE].b; \
    d->colors[__GL_FRONTFACE].a = t*(a->colors[__GL_FRONTFACE].a      \
        - b->colors[__GL_FRONTFACE].a) + b->colors[__GL_FRONTFACE].a

#define __GL_CLIP_INDEX(d,a,b,t)                                     \
    d->colors[__GL_FRONTFACE].r = t*(a->colors[__GL_FRONTFACE].r     \
        - b->colors[__GL_FRONTFACE].r) + b->colors[__GL_FRONTFACE].r

#define __GL_CLIP_BACKCOLOR(d,a,b,t)                                \
    d->colors[__GL_BACKFACE].r = t*(a->colors[__GL_BACKFACE].r      \
        - b->colors[__GL_BACKFACE].r) + b->colors[__GL_BACKFACE].r; \
    d->colors[__GL_BACKFACE].g = t*(a->colors[__GL_BACKFACE].g      \
        - b->colors[__GL_BACKFACE].g) + b->colors[__GL_BACKFACE].g; \
    d->colors[__GL_BACKFACE].b = t*(a->colors[__GL_BACKFACE].b      \
        - b->colors[__GL_BACKFACE].b) + b->colors[__GL_BACKFACE].b; \
    d->colors[__GL_BACKFACE].a = t*(a->colors[__GL_BACKFACE].a      \
        - b->colors[__GL_BACKFACE].a) + b->colors[__GL_BACKFACE].a

#define __GL_CLIP_BACKINDEX(d,a,b,t)                               \
    d->colors[__GL_BACKFACE].r = t*(a->colors[__GL_BACKFACE].r     \
        - b->colors[__GL_BACKFACE].r) + b->colors[__GL_BACKFACE].r

#if __GL_SST_GLIDE_VTX
#define __GL_CLIP_TEXTURE(d,a,b,t) \
    { \
        __GLfloat ax, ay, az, aw; \
        __GLfloat bx, by, bz, bw; \
            ax = a->texture.x; \
            ay = a->texture.y; \
            az = a->texture.z; \
            aw = a->texture.w; \
            bx = b->texture.x; \
            by = b->texture.y; \
            bz = b->texture.z; \
            bw = b->texture.w; \
        d->texture.x = t*(ax - bx) + bx; \
        d->texture.y = t*(ay - by) + by; \
        d->texture.z = t*(az - bz) + bz; \
        d->texture.w = t*(aw - bw) + bw; \
    }
#else
#define __GL_CLIP_TEXTURE(d,a,b,t) \
    { \
        __GLfloat ax0, ay0, az0, aw0; \
        __GLfloat bx0, by0, bz0, bw0; \
        __GLfloat ax1, ay1, az1, aw1; \
        __GLfloat bx1, by1, bz1, bw1; \
        if (!(a->hasAndClipCode & __GL_CLIP_MASK) && \
                (a->hasAndClipCode & __GL_HAS_WINDOW_TEXTURE)) { \
            ax0 = a->texture[0].x * a->clip.w; \
            ay0 = a->texture[0].y * a->clip.w; \
            az0 = a->texture[0].z * a->clip.w; \
            aw0 = a->texture[0].w * a->clip.w; \
            ax1 = a->texture[1].x * a->clip.w; \
            ay1 = a->texture[1].y * a->clip.w; \
            az1 = a->texture[1].z * a->clip.w; \
            aw1 = a->texture[1].w * a->clip.w; \
        } else { \
            ax0 = a->texture[0].x; \
            ay0 = a->texture[0].y; \
            az0 = a->texture[0].z; \
            aw0 = a->texture[0].w; \
            ax1 = a->texture[1].x; \
            ay1 = a->texture[1].y; \
            az1 = a->texture[1].z; \
            aw1 = a->texture[1].w; \
        } \
        if (!(b->hasAndClipCode & __GL_CLIP_MASK) && \
                (b->hasAndClipCode & __GL_HAS_WINDOW_TEXTURE)) { \
            bx0 = b->texture[0].x * b->clip.w; \
            by0 = b->texture[0].y * b->clip.w; \
            bz0 = b->texture[0].z * b->clip.w; \
            bw0 = b->texture[0].w * b->clip.w; \
            bx1 = b->texture[1].x * b->clip.w; \
            by1 = b->texture[1].y * b->clip.w; \
            bz1 = b->texture[1].z * b->clip.w; \
            bw1 = b->texture[1].w * b->clip.w; \
        } else { \
            bx0 = b->texture[0].x; \
            by0 = b->texture[0].y; \
            bz0 = b->texture[0].z; \
            bw0 = b->texture[0].w; \
            bx1 = b->texture[1].x; \
            by1 = b->texture[1].y; \
            bz1 = b->texture[1].z; \
            bw1 = b->texture[1].w; \
        } \
        d->texture[0].x = t*(ax0 - bx0) + bx0; \
        d->texture[0].y = t*(ay0 - by0) + by0; \
        d->texture[0].z = t*(az0 - bz0) + bz0; \
        d->texture[0].w = t*(aw0 - bw0) + bw0; \
        d->texture[1].x = t*(ax1 - bx1) + bx1; \
        d->texture[1].y = t*(ay1 - by1) + by1; \
        d->texture[1].z = t*(az1 - bz1) + bz1; \
        d->texture[1].w = t*(aw1 - bw1) + bw1; \
    }
#endif

/************************************************************************/

static void Clip(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
}

static void ClipC(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                  __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
}

static void ClipI(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                  __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
}

static void ClipBC(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                   __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
}

static void ClipBI(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                   __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
}

static void ClipT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void ClipIT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void ClipBIT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void ClipCT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void ClipBCT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void ClipF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void ClipIF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void ClipBIF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void ClipCF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void ClipBCF(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                 __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
}

static void ClipFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                   __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void ClipIFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                    __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void ClipBIFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                     __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_INDEX(dst,a,b,t);
    __GL_CLIP_BACKINDEX(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void ClipCFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                    __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void ClipBCFT(__GLvertex *dst, const __GLvertex *a, const __GLvertex *b,
                     __GLfloat t)
{
    __GL_CLIP_POS(dst,a,b,t);
    __GL_CLIP_COLOR(dst,a,b,t);
    __GL_CLIP_BACKCOLOR(dst,a,b,t);
    __GL_CLIP_FOG(dst,a,b,t);
    __GL_CLIP_TEXTURE(dst,a,b,t);
}

static void (*clipProcs[20])(__GLvertex*, const __GLvertex*, const __GLvertex*,
                             __GLfloat) =
{
    Clip, ClipI, ClipC, ClipBI, ClipBC,
    ClipF, ClipIF, ClipCF, ClipBIF, ClipBCF,
    ClipT, ClipIT, ClipCT, ClipBIT, ClipBCT,
    ClipFT, ClipIFT, ClipCFT, ClipBIFT, ClipBCFT,
};

void __glGenericPickParameterClipProcs(__GLcontext *gc)
{
    GLint line = 0, poly = 0;
    GLuint enables = gc->state.enables.general;
    GLboolean twoSided = (enables & __GL_LIGHTING_ENABLE)
        && gc->state.light.model.twoSided;
    GLuint modeFlags = gc->polygon.shader.modeFlags;

    if (gc->modes.rgbMode) {
        if (gc->state.light.shadingModel != GL_FLAT) {
            line = 2;
            if (twoSided) {
                poly = 4;
            } else {
                poly = 2;
            }
        }
    } else {
        if (gc->state.light.shadingModel != GL_FLAT) {
            line = 1;
            if (twoSided) {
                poly = 3;
            } else {
                poly = 1;
            }
        }
    }
    if ((modeFlags & __GL_SHADE_SLOW_FOG) ||
            ((modeFlags & __GL_SHADE_CHEAP_FOG) && 
            !(modeFlags & __GL_SHADE_SMOOTH_LIGHT))) {
        line += 5;
        poly += 5;
    }
    if (gc->texture.textureEnabled) { /*XXX - don't change this (see Derrick)*/
        line += 10;
        poly += 10;
    }
    gc->procs.lineClipParam = clipProcs[line];
    gc->procs.polyClipParam = clipProcs[poly];
}

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
*/

#include "context.h"
#include "global.h"
#include "types.h"
#include "glmath.h"
#include <math.h>


/************************************************************************/

/*
** Texture mapping optimization is composed of 2 parts:
** - optimizing span processing
** - optimizing rho (level-of-detail) calculation
**
** + Span Processing optimization is done the usual way..  By inlining
**   and performing low-level optimization...  Here, combinations
**   of mag and min filters, and texture format types  are combined together 
**   with the span processing, to produce routines that are hardwired
**   for these cases..  Experiments have shown that using per-span special
**   routines instead of per-pixel special routines provide 30% more speedup..
**
** + LOD was performed by calculating a rho on a per-pixel basis.  Now,
**   rho is calculated on a per-vertex basis, and rho/w is linearly
**   interpolated across the triangle.  This is done in order to give
**   rho the necessery perspective correction.  1/w is already been
**   calculated, so the overhead for calculating rho per-pixel is minimal.
**
** Future work:
**
** - Assemblerize the span routines.  The following trick could be also
**   used: Use per-pixel texturing routines that do not save-restore 
**   registers, and let the (assemblerized) span routines deal with the
**   saving and restoring.  The span routines use much fewer register and
**   can therefore juggle register usage better.
**
** - Optimize the (per-vertex) rho calculation.  This topic is open
**   for discussion.
**
*/

/************************************************************************/

/*
** Some math routines that are optimized in assembly
*/

#ifdef __GL_USE_MIPSASMCODE
#define __GL_FRAC(f)    __glFrac(f)
#define __GL_FLOOR(f)   __glFloor(f)
#else
#define __GL_FRAC(f)    ((f) - __GL_FLOORF(f))
#define __GL_FLOOR(f)   ((GLint) __GL_FLOORF(f))
#endif

/************************************************************************/

/*
** The following are optimized span texture span routines for certain cases
*/


/*
**  non-stippled spans.
**  RGB mip-mapped.
**  Max filter: GL_LINEAR
**  Min filter: GL_NEAREST_MIPMAP_LINEAR
*/
GLboolean __glTextureRGB_L_NML_Span(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat S, T, qw;
    GLint w;
    __GLfloat rhow, rho;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    __GLfloat w2f, h2f;
    __GLfloat half = __glHalf;
    __GLfloat u, v, alpha, beta, omalpha, ombeta;
    __GLfloat m00, m10, m01, m11;
    GLint row, col, row0, col0, row1, col1;
    __GLtexel texel;
    GLint p, d;
    __GLfloat f, omf;
    __GLtextureBuffer *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint w2mask, h2mask;
    GLint wlog2;
    GLint repeatState;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */
    rhow = gc->polygon.shader.frag.rhow;

    while( --w >= 0 ) {
        qw = __glOne / qwinv;
        s = S * qw;
        t = T * qw;
        rho = rhow * qw;

        /* mipmap the fragment */
        if( rho <= tex->c ) {
            /* magnify..  Use bilinear (GL_LINEAR) */
            repeatState = GL_FALSE;

            lp = &tex->level[0];
            wlog2 = lp->widthLog2;
            buffer = lp->buffer;
            w2mask = lp->width2 - 1;
            h2mask = lp->height2 - 1;

            w2f = lp->width2f;
            u = s * w2f;
            if (tex->params.sWrapMode == GL_REPEAT) {
                u -= half;
                col0 = (__GL_FLOOR(u)) & w2mask;
                col1 = (col0 + 1) & w2mask;
                repeatState++;
            } else {
                if (u < __glZero) u = __glZero;
                else if (u > w2f) u = w2f;
                u -= half;
                col0 = __GL_FLOOR(u);
                col1 = col0 + 1;
            }

            h2f = lp->height2f;
            v = t * h2f;
            if (tex->params.tWrapMode == GL_REPEAT) {
                v -= half;
                row0 = (__GL_FLOOR(v)) & h2mask;
                row1 = (row0 + 1) & h2mask;
                repeatState++;
            } else {
                if (v < __glZero) v = __glZero;
                else if (v > h2f) v = h2f;
                v -= half;
                row0 = __GL_FLOOR(v);
                row1 = row0 + 1;
            }

            alpha = __GL_FRAC(u);
            beta = __GL_FRAC(v);
            omalpha = __glOne - alpha;
            ombeta = __glOne - beta;
            m00 = omalpha * ombeta;
            m10 = alpha * ombeta;
            m01 = omalpha * beta;
            m11 = alpha * beta;

            /* extract texels */
            if( repeatState == 2 ) {
                /* both coords are GL_REPEAT..  No borders */
                offset = (row0 << wlog2) + col0;
                image = buffer + (offset << 1) + offset;
                texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);

                offset = (row0 << wlog2) + col1;
                image = buffer + (offset << 1) + offset;
                texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);

                offset = (row1 << wlog2) + col0;
                image = buffer + (offset << 1) + offset;
                texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);

                offset = (row1 << wlog2) + col1;
                image = buffer + (offset << 1) + offset;
                texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
            } else {
                /* could have borders */
                h2mask = ~h2mask;
                w2mask = ~w2mask;

                if( (row0 & h2mask) | (col0 & w2mask) ) {
                    texel.r = m00 * tex->params.borderColor.r;
                    texel.g = m00 * tex->params.borderColor.g;
                    texel.b = m00 * tex->params.borderColor.b;
                } else {
                    offset = (row0 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);
                }
                if( (row0 & h2mask) | (col1 & w2mask) ) {
                    texel.r += m10 * tex->params.borderColor.r;
                    texel.g += m10 * tex->params.borderColor.g;
                    texel.b += m10 * tex->params.borderColor.b;
                } else {
                    offset = (row0 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                }
                if( (row1 & h2mask) | (col0 & w2mask) ) {
                    texel.r += m01 * tex->params.borderColor.r;
                    texel.g += m01 * tex->params.borderColor.g;
                    texel.b += m01 * tex->params.borderColor.b;
                } else {
                    offset = (row1 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                }
                if( (row1 & h2mask) | (col1 & w2mask) ) {
                    texel.r += m11 * tex->params.borderColor.r;
                    texel.g += m11 * tex->params.borderColor.g;
                    texel.b += m11 * tex->params.borderColor.b;
                } else {
                    offset = (row1 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                }
            }

        } else {
            /* convert rho to lambda */
            if (rho) {
                GLuint irho, lev;
                __GLfloat twotolev;

                irho = rho;
                lev = 0;
                while (irho >>= 1) lev++;
                twotolev = 1<<lev;
                rho = (lev + ( (rho-twotolev) / twotolev ) ) * half;
            } else {
                rho = __glZero;
            }

            /* minify */
            p = tex->p;
            d = (GLint)rho;
            f = rho - d;        /* f is frac(rho) */
            d++;
            omf = __glOne - f;
            if ( (d > p) | (d < 0) ) {
                lp = &tex->level[p];
                w2f = lp->width2f;
                if (tex->params.sWrapMode == GL_REPEAT) {
                    col = (GLint)(__GL_FRAC(s) * w2f);
                } else {
                    GLint w2 = lp->width2;
                    col = (GLint)(s * w2f);
                    if (col < 0) col = 0;
                    else if (col >= w2) col = w2 - 1;
                }
                h2f = lp->height2f;
                if (tex->params.tWrapMode == GL_REPEAT) {
                    row = (GLint)(__GL_FRAC(t) * h2f);
                } else {
                    GLint h2 = lp->height2;
                    row = (GLint)(t * h2f);
                    if (row < 0) row = 0;
                    else if (row >= h2) row = h2 - 1;
                }

                if ( (row & ~(lp->height2 - 1)) | (col & ~(lp->width2 - 1)) ) {
                    texel.r = tex->params.borderColor.r;
                    texel.g = tex->params.borderColor.g;
                    texel.b = tex->params.borderColor.b;
                } else {
                    offset = (row << lp->widthLog2) + col;
                    image = lp->buffer + (offset << 1) + offset;

                    texel.r = __GL_UB_TO_FLOAT(image[0]);
                    texel.g = __GL_UB_TO_FLOAT(image[1]);
                    texel.b = __GL_UB_TO_FLOAT(image[2]);
                }
            } else {
                lp = &tex->level[d-1];
                w2f = lp->width2f;
                if (tex->params.sWrapMode == GL_REPEAT) {
                    col = (GLint)(__GL_FRAC(s) * w2f);
                } else {
                    GLint w2 = lp->width2;
                    col = (GLint)(s * w2f);
                    if (col < 0) col = 0;
                    else if (col >= w2) col = w2 - 1;
                }
                h2f = lp->height2f;
                if (tex->params.tWrapMode == GL_REPEAT) {
                    row = (GLint)(__GL_FRAC(t) * h2f);
                } else {
                    GLint h2 = lp->height2;
                    row = (GLint)(t * h2f);
                    if (row < 0) row = 0;
                    else if (row >= h2) row = h2 - 1;
                }
                if ( (row & ~(lp->height2 - 1)) | (col & ~(lp->width2 - 1)) ) {
                    /* he hit the border..  Same color for both LODs */
                    texel.r = tex->params.borderColor.r;
                    texel.g = tex->params.borderColor.g;
                    texel.b = tex->params.borderColor.b;
                } else {
                    offset = (row << lp->widthLog2) + col;
                    image = lp->buffer + (offset << 1) + offset;
                    texel.r = omf * __GL_UB_TO_FLOAT(image[0]);
                    texel.g = omf * __GL_UB_TO_FLOAT(image[1]);
                    texel.b = omf * __GL_UB_TO_FLOAT(image[2]);

                    /* now load the lower level of detail */
                    lp = &tex->level[d];
                    row >>= 1;
                    col >>= 1;
                    offset = (row << lp->widthLog2) + col;
                    image = lp->buffer + (offset << 1) + offset;
                    texel.r += f * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += f * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += f * __GL_UB_TO_FLOAT(image[2]);
                }
            }
        }

        /* final pixel processing */
        if( texenvMode == GL_MODULATE ) {
            /* modulate */
            cp->r *= texel.r;
            cp->g *= texel.g;
            cp->b *= texel.b;
        } else {
            if ((texenvMode == GL_DECAL) | 
                (texenvMode == GL_REPLACE)) {
                /* decal or replace (RGB) */
                cp->r = texel.r * gc->frontBuffer.redScale;
                cp->g = texel.g * gc->frontBuffer.greenScale;
                cp->b = texel.b * gc->frontBuffer.blueScale;
            } else {
                /* blend */
                __GLfloat r = texel.r;
                __GLfloat g = texel.g;
                __GLfloat b = texel.b;
                __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

                cp->r = (__glOne - r) * cp->r + r * cc->r;
                cp->g = (__glOne - g) * cp->g + g * cc->g;
                cp->b = (__glOne - b) * cp->b + b * cc->b;
            }
        }

        /* iterate to next pixel */
        S += gc->polygon.shader.dsdx;
        T += gc->polygon.shader.dtdx;
        qwinv += gc->polygon.shader.dqwdx;
        rhow += gc->polygon.shader.drhowdx;
        cp++;
    }

    return GL_FALSE;
}

/*
**  stippled spans.
**  RGB mip-mapped.
**  Max filter: GL_LINEAR
**  Min filter: GL_NEAREST_MIPMAP_LINEAR
*/
GLboolean __glTextureRGB_L_NML_StippledSpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLstippleWord inMask, bit, *sp;
    __GLfloat S, T, qw;
    GLint w, count;
    __GLfloat rhow, rho;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    __GLfloat w2f, h2f;
    __GLfloat half = __glHalf;
    __GLfloat u, v, alpha, beta, omalpha, ombeta;
    __GLfloat m00, m10, m01, m11;
    GLint row, col, row0, col0, row1, col1;
    __GLtexel texel;
    GLint p, d;
    __GLfloat f, omf;
    __GLtextureBuffer *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint w2mask, h2mask;
    GLint wlog2;
    GLint repeatState;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    sp = gc->polygon.shader.stipplePat;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */
    rhow = gc->polygon.shader.frag.rhow;

    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
        
                qw = __glOne / qwinv;
                s = S * qw;
                t = T * qw;
                rho = rhow * qw;

                /* mipmap the fragment */
                if( rho <= tex->c ) {
                    /* magnify..  Use bilinear (GL_LINEAR) */
                    repeatState = GL_FALSE;

                    lp = &tex->level[0];
                    wlog2 = lp->widthLog2;
                    buffer = lp->buffer;
                    w2mask = lp->width2 - 1;
                    h2mask = lp->height2 - 1;

                    w2f = lp->width2f;
                    u = s * w2f;
                    if (tex->params.sWrapMode == GL_REPEAT) {
                        u -= half;
                        col0 = (__GL_FLOOR(u)) & w2mask;
                        col1 = (col0 + 1) & w2mask;
                        repeatState++;
                    } else {
                        if (u < __glZero) u = __glZero;
                        else if (u > w2f) u = w2f;
                        u -= half;
                        col0 = __GL_FLOOR(u);
                        col1 = col0 + 1;
                    }
                    
                    h2f = lp->height2f;
                    v = t * h2f;
                    if (tex->params.tWrapMode == GL_REPEAT) {
                        v -= half;
                        row0 = (__GL_FLOOR(v)) & h2mask;
                        row1 = (row0 + 1) & h2mask;
                        repeatState++;
                    } else {
                        if (v < __glZero) v = __glZero;
                        else if (v > h2f) v = h2f;
                        v -= half;
                        row0 = __GL_FLOOR(v);
                        row1 = row0 + 1;
                    }

                    alpha = __GL_FRAC(u);
                    beta = __GL_FRAC(v);
                    omalpha = __glOne - alpha;
                    ombeta = __glOne - beta;
                    m00 = omalpha * ombeta;
                    m10 = alpha * ombeta;
                    m01 = omalpha * beta;
                    m11 = alpha * beta;

                    /* extract texels */
                    if( repeatState == 2 ) {
                        /* both coords are GL_REPEAT..  No borders */
                        offset = (row0 << wlog2) + col0;
                        image = buffer + (offset << 1) + offset;
                        texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);

                        offset = (row0 << wlog2) + col1;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);

                        offset = (row1 << wlog2) + col0;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);

                        offset = (row1 << wlog2) + col1;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                    } else {
                        /* could have borders */
                        h2mask = ~h2mask;
                        w2mask = ~w2mask;

                        if( (row0 & h2mask) | (col0 & w2mask) ) {
                            texel.r = m00 * tex->params.borderColor.r;
                            texel.g = m00 * tex->params.borderColor.g;
                            texel.b = m00 * tex->params.borderColor.b;
                        } else {
                            offset = (row0 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);
                        }
                        if( (row0 & h2mask) | (col1 & w2mask) ) {
                            texel.r += m10 * tex->params.borderColor.r;
                            texel.g += m10 * tex->params.borderColor.g;
                            texel.b += m10 * tex->params.borderColor.b;
                        } else {
                            offset = (row0 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                        }
                        if( (row1 & h2mask) | (col0 & w2mask) ) {
                            texel.r += m01 * tex->params.borderColor.r;
                            texel.g += m01 * tex->params.borderColor.g;
                            texel.b += m01 * tex->params.borderColor.b;
                        } else {
                            offset = (row1 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                        }
                        if( (row1 & h2mask) | (col1 & w2mask) ) {
                            texel.r += m11 * tex->params.borderColor.r;
                            texel.g += m11 * tex->params.borderColor.g;
                            texel.b += m11 * tex->params.borderColor.b;
                        } else {
                            offset = (row1 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                        }
                    }

                } else {
                    /* convert rho to lambda */
                    if (rho) {
                        GLuint irho, lev;
                        __GLfloat twotolev;

                        irho = rho;
                        lev = 0;
                        while (irho >>= 1) lev++;
                        twotolev = 1<<lev;
                        rho = (lev + ( (rho-twotolev) / twotolev ) ) * half;
                    } else {
                        rho = __glZero;
                    }

                    /* minify */
                    p = tex->p;
                    d = (GLint)rho;
                    f = rho - d;        /* f is frac(rho) */
                    d++;
                    omf = __glOne - f;
                    if ( (d > p) | (d < 0) ) {
                        lp = &tex->level[p];
                        w2f = lp->width2f;
                        if (tex->params.sWrapMode == GL_REPEAT) {
                            col = (GLint)(__GL_FRAC(s) * w2f);
                        } else {
                            GLint w2 = lp->width2;
                            col = (GLint)(s * w2f);
                            if (col < 0) col = 0;
                            else if (col >= w2) col = w2 - 1;
                        }
                        h2f = lp->height2f;
                        if (tex->params.tWrapMode == GL_REPEAT) {
                            row = (GLint)(__GL_FRAC(t) * h2f);
                        } else {
                            GLint h2 = lp->height2;
                            row = (GLint)(t * h2f);
                            if (row < 0) row = 0;
                            else if (row >= h2) row = h2 - 1;
                        }

                        if ( (row & ~(lp->height2 - 1)) | (col & ~(lp->width2 - 1)) ) {
                            texel.r = tex->params.borderColor.r;
                            texel.g = tex->params.borderColor.g;
                            texel.b = tex->params.borderColor.b;
                        } else {
                            offset = (row << lp->widthLog2) + col;
                            image = lp->buffer + (offset << 1) + offset;
                            
                            texel.r = __GL_UB_TO_FLOAT(image[0]);
                            texel.g = __GL_UB_TO_FLOAT(image[1]);
                            texel.b = __GL_UB_TO_FLOAT(image[2]);
                        }
                    } else {
                        lp = &tex->level[d-1];
                        w2f = lp->width2f;
                        if (tex->params.sWrapMode == GL_REPEAT) {
                            col = (GLint)(__GL_FRAC(s) * w2f);
                        } else {
                            GLint w2 = lp->width2;
                            col = (GLint)(s * w2f);
                            if (col < 0) col = 0;
                            else if (col >= w2) col = w2 - 1;
                        }
                        h2f = lp->height2f;
                        if (tex->params.tWrapMode == GL_REPEAT) {
                            row = (GLint)(__GL_FRAC(t) * h2f);
                        } else {
                            GLint h2 = lp->height2;
                            row = (GLint)(t * h2f);
                            if (row < 0) row = 0;
                            else if (row >= h2) row = h2 - 1;
                        }
                        if ( (row & ~(lp->height2 - 1)) | (col & ~(lp->width2 - 1)) ) {
                            /* he hit the border..  Same color for both LODs */
                            texel.r = tex->params.borderColor.r;
                            texel.g = tex->params.borderColor.g;
                            texel.b = tex->params.borderColor.b;
                        } else {
                            offset = (row << lp->widthLog2) + col;
                            image = lp->buffer + (offset << 1) + offset;
                            texel.r = omf * __GL_UB_TO_FLOAT(image[0]);
                            texel.g = omf * __GL_UB_TO_FLOAT(image[1]);
                            texel.b = omf * __GL_UB_TO_FLOAT(image[2]);
                            
                            /* now load the lower level of detail */
                            lp = &tex->level[d];
                            row >>= 1;
                            col >>= 1;
                            offset = (row << lp->widthLog2) + col;
                            image = lp->buffer + (offset << 1) + offset;
                            texel.r += f * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += f * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += f * __GL_UB_TO_FLOAT(image[2]);
                        }
                    }
                }

                /* final pixel processing */
                if( texenvMode == GL_MODULATE ) {
                    /* modulate */
                    cp->r *= texel.r;
                    cp->g *= texel.g;
                    cp->b *= texel.b;
                } else {
                    if ((texenvMode == GL_DECAL) | 
                        (texenvMode == GL_REPLACE)) {
                        /* decal or replace */
                        cp->r = texel.r * gc->frontBuffer.redScale;
                        cp->g = texel.g * gc->frontBuffer.greenScale;
                        cp->b = texel.b * gc->frontBuffer.blueScale;
                    } else {
                        /* blend */
                        __GLfloat r = texel.r;
                        __GLfloat g = texel.g;
                        __GLfloat b = texel.b;
                        __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;
                        
                        cp->r = (__glOne - r) * cp->r + r * cc->r;
                        cp->g = (__glOne - g) * cp->g + g * cc->g;
                        cp->b = (__glOne - b) * cp->b + b * cc->b;
                    }
                }
            }

            /* iterate to next pixel */
            S += gc->polygon.shader.dsdx;
            T += gc->polygon.shader.dtdx;
            qwinv += gc->polygon.shader.dqwdx;
            rhow += gc->polygon.shader.drhowdx;
            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }

    return GL_FALSE;
}


/*
**  non-stippled spans.
**  RGB mip-mapped.
**  Max filter: GL_LINEAR
**  Min filter: GL_LINEAR_MIPMAP_LINEAR
*/
GLboolean __glTextureRGB_L_LML_Span(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat S, T, qw;
    GLint w;
    __GLfloat rhow, rho;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    __GLfloat w2f, h2f;
    __GLfloat half = __glHalf;
    __GLfloat u, v, alpha, beta, omalpha, ombeta;
    __GLfloat m00, m10, m01, m11;
    GLint row0, col0, row1, col1;
    __GLtexel texel;
    GLint p, d;
    __GLfloat f, omf;
    __GLtextureBuffer *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint w2mask, h2mask;
    GLint wlog2;
    GLint repeatState;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */
    rhow = gc->polygon.shader.frag.rhow;

    while( --w >= 0 ) {
        qw = __glOne / qwinv;
        s = S * qw;
        t = T * qw;
        rho = rhow * qw;

        /* mipmap the fragment */
        if( rho <= tex->c ) {
            /* magnify..  Use bilinear (GL_LINEAR) */
            repeatState = 0;

            lp = &tex->level[0];
            wlog2 = lp->widthLog2;
            buffer = lp->buffer;
            w2mask = lp->width2 - 1;
            h2mask = lp->height2 - 1;

            w2f = lp->width2f;
            u = s * w2f;
            if (tex->params.sWrapMode == GL_REPEAT) {
                u -= half;
                col0 = (__GL_FLOOR(u)) & w2mask;
                col1 = (col0 + 1) & w2mask;
                repeatState++;
            } else {
                if (u < __glZero) u = __glZero;
                else if (u > w2f) u = w2f;
                u -= half;
                col0 = __GL_FLOOR(u);
                col1 = col0 + 1;
            }

            h2f = lp->height2f;
            v = t * h2f;
            if (tex->params.tWrapMode == GL_REPEAT) {
                v -= half;
                row0 = (__GL_FLOOR(v)) & h2mask;
                row1 = (row0 + 1) & h2mask;
                repeatState++;
            } else {
                if (v < __glZero) v = __glZero;
                else if (v > h2f) v = h2f;
                v -= half;
                row0 = __GL_FLOOR(v);
                row1 = row0 + 1;
            }

            alpha = __GL_FRAC(u);
            beta = __GL_FRAC(v);
            omalpha = __glOne - alpha;
            ombeta = __glOne - beta;
            m00 = omalpha * ombeta;
            m10 = alpha * ombeta;
            m01 = omalpha * beta;
            m11 = alpha * beta;

            /* extract texels */
            if( repeatState == 2 ) {
                /* both coords are GL_REPEAT..  No borders */
                offset = (row0 << wlog2) + col0;
                image = buffer + (offset << 1) + offset;
                texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);

                offset = (row0 << wlog2) + col1;
                image = buffer + (offset << 1) + offset;
                texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);

                offset = (row1 << wlog2) + col0;
                image = buffer + (offset << 1) + offset;
                texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);

                offset = (row1 << wlog2) + col1;
                image = buffer + (offset << 1) + offset;
                texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
            } else {
                /* could have borders */
                h2mask = ~h2mask;
                w2mask = ~w2mask;

                if( (row0 & h2mask) | (col0 & w2mask) ) {
                    texel.r = m00 * tex->params.borderColor.r;
                    texel.g = m00 * tex->params.borderColor.g;
                    texel.b = m00 * tex->params.borderColor.b;
                } else {
                    offset = (row0 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);
                }
                if( (row0 & h2mask) | (col1 & w2mask) ) {
                    texel.r += m10 * tex->params.borderColor.r;
                    texel.g += m10 * tex->params.borderColor.g;
                    texel.b += m10 * tex->params.borderColor.b;
                } else {
                    offset = (row0 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                }
                if( (row1 & h2mask) | (col0 & w2mask) ) {
                    texel.r += m01 * tex->params.borderColor.r;
                    texel.g += m01 * tex->params.borderColor.g;
                    texel.b += m01 * tex->params.borderColor.b;
                } else {
                    offset = (row1 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                }
                if( (row1 & h2mask) | (col1 & w2mask) ) {
                    texel.r += m11 * tex->params.borderColor.r;
                    texel.g += m11 * tex->params.borderColor.g;
                    texel.b += m11 * tex->params.borderColor.b;
                } else {
                    offset = (row1 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                }
            }

        } else {
            /* convert rho to lambda */
            if (rho) {
                GLuint irho, lev;
                __GLfloat twotolev;

                irho = rho;
                lev = 0;
                while (irho >>= 1) lev++;
                twotolev = 1<<lev;
                rho = (lev + ( (rho-twotolev) / twotolev ) ) * half;
            } else {
                rho = __glZero;
            }

            /* minify */
            p = tex->p;
            d = (GLint)rho;
            f = rho - d;        /* f is frac(rho) */
            d++;
            omf = __glOne - f;
            if ( (d > p) | (d < 0) ) {
                /* at the edge..  still, use bilinear */
                lp = &tex->level[p];
                wlog2 = lp->widthLog2;
                buffer = lp->buffer;
                w2mask = lp->width2 - 1;
                h2mask = lp->height2 - 1;

                w2f = lp->width2f;
                u = s * w2f;
                if (tex->params.sWrapMode == GL_REPEAT) {
                    u -= half;
                    col0 = (__GL_FLOOR(u)) & w2mask;
                    col1 = (col0 + 1) & w2mask;
                } else {
                    if (u < __glZero) u = __glZero;
                    else if (u > w2f) u = w2f;
                    u -= half;
                    col0 = __GL_FLOOR(u);
                    col1 = col0 + 1;
                }

                h2f = lp->height2f;
                v = t * h2f;
                if (tex->params.tWrapMode == GL_REPEAT) {
                    v -= half;
                    row0 = (__GL_FLOOR(v)) & h2mask;
                    row1 = (row0 + 1) & h2mask;
                } else {
                    if (v < __glZero) v = __glZero;
                    else if (v > h2f) v = h2f;
                    v -= half;
                    row0 = __GL_FLOOR(v);
                    row1 = row0 + 1;
                }

                alpha = __GL_FRAC(u);
                beta = __GL_FRAC(v);
                omalpha = __glOne - alpha;
                ombeta = __glOne - beta;
                m00 = omalpha * ombeta;
                m10 = alpha * ombeta;
                m01 = omalpha * beta;
                m11 = alpha * beta;

                /* extract texels */
                h2mask = ~h2mask;
                w2mask = ~w2mask;

                if( (row0 & h2mask) | (col0 & w2mask) ) {
                    texel.r = m00 * tex->params.borderColor.r;
                    texel.g = m00 * tex->params.borderColor.g;
                    texel.b = m00 * tex->params.borderColor.b;
                } else {
                    offset = (row0 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);
                }
                if( (row0 & h2mask) | (col1 & w2mask) ) {
                    texel.r += m10 * tex->params.borderColor.r;
                    texel.g += m10 * tex->params.borderColor.g;
                    texel.b += m10 * tex->params.borderColor.b;
                } else {
                    offset = (row0 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                }
                if( (row1 & h2mask) | (col0 & w2mask) ) {
                    texel.r += m01 * tex->params.borderColor.r;
                    texel.g += m01 * tex->params.borderColor.g;
                    texel.b += m01 * tex->params.borderColor.b;
                } else {
                    offset = (row1 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                }
                if( (row1 & h2mask) | (col1 & w2mask) ) {
                    texel.r += m11 * tex->params.borderColor.r;
                    texel.g += m11 * tex->params.borderColor.g;
                    texel.b += m11 * tex->params.borderColor.b;
                } else {
                    offset = (row1 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                }
            } else {
                /* 
                ** tri-linear interpolation... 
                ** bilinear on each level, and linear between levels
                */

                /* first do level d-1 */
                repeatState = 0;
                lp = &tex->level[d-1];
                wlog2 = lp->widthLog2;
                buffer = lp->buffer;
                w2mask = lp->width2 - 1;
                h2mask = lp->height2 - 1;

                w2f = lp->width2f;
                u = s * w2f;
                if (tex->params.sWrapMode == GL_REPEAT) {
                    u -= half;
                    col0 = (__GL_FLOOR(u)) & w2mask;
                    col1 = (col0 + 1) & w2mask;
                    repeatState++;
                } else {
                    if (u < __glZero) u = __glZero;
                    else if (u > w2f) u = w2f;
                    u -= half;
                    col0 = __GL_FLOOR(u);
                    col1 = col0 + 1;
                }

                h2f = lp->height2f;
                v = t * h2f;
                if (tex->params.tWrapMode == GL_REPEAT) {
                    v -= half;
                    row0 = (__GL_FLOOR(v)) & h2mask;
                    row1 = (row0 + 1) & h2mask;
                    repeatState++;
                } else {
                    if (v < __glZero) v = __glZero;
                    else if (v > h2f) v = h2f;
                    v -= half;
                    row0 = __GL_FLOOR(v);
                    row1 = row0 + 1;
                }

                alpha = __GL_FRAC(u);
                beta = __GL_FRAC(v);
                omalpha = __glOne - alpha;
                ombeta = __glOne - beta;
                m00 = omf * omalpha * ombeta;
                m10 = omf * alpha * ombeta;
                m01 = omf * omalpha * beta;
                m11 = omf * alpha * beta;

                /* extract texels */
                if (repeatState == 2) {
                    /* both coords are GL_REPEAT..  no borders */
                    offset = (row0 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);

                    offset = (row0 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);

                    offset = (row1 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);

                    offset = (row1 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                } else {
                    /* could have borders */
                    h2mask = ~h2mask;
                    w2mask = ~w2mask;

                    if( (row0 & h2mask) | (col0 & w2mask) ) {
                        texel.r = m00 * tex->params.borderColor.r;
                        texel.g = m00 * tex->params.borderColor.g;
                        texel.b = m00 * tex->params.borderColor.b;
                    } else {
                        offset = (row0 << wlog2) + col0;
                        image = buffer + (offset << 1) + offset;
                        texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);
                    }
                    if( (row0 & h2mask) | (col1 & w2mask) ) {
                        texel.r += m10 * tex->params.borderColor.r;
                        texel.g += m10 * tex->params.borderColor.g;
                        texel.b += m10 * tex->params.borderColor.b;
                    } else {
                        offset = (row0 << wlog2) + col1;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                    }
                    if( (row1 & h2mask) | (col0 & w2mask) ) {
                        texel.r += m01 * tex->params.borderColor.r;
                        texel.g += m01 * tex->params.borderColor.g;
                        texel.b += m01 * tex->params.borderColor.b;
                    } else {
                        offset = (row1 << wlog2) + col0;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                    }
                    if( (row1 & h2mask) | (col1 & w2mask) ) {
                        texel.r += m11 * tex->params.borderColor.r;
                        texel.g += m11 * tex->params.borderColor.g;
                        texel.b += m11 * tex->params.borderColor.b;
                    } else {
                        offset = (row1 << wlog2) + col1;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                    }
                }

                /* now do level d */
                repeatState = 0;
                lp = &tex->level[d];
                wlog2 = lp->widthLog2;
                buffer = lp->buffer;
                w2mask = lp->width2 - 1;
                h2mask = lp->height2 - 1;

                w2f = lp->width2f;
                u = s * w2f;
                if (tex->params.sWrapMode == GL_REPEAT) {
                    u -= half;
                    col0 = (__GL_FLOOR(u)) & w2mask;
                    col1 = (col0 + 1) & w2mask;
                    repeatState++;
                } else {
                    if (u < __glZero) u = __glZero;
                    else if (u > w2f) u = w2f;
                    u -= half;
                    col0 = __GL_FLOOR(u);
                    col1 = col0 + 1;
                }

                h2f = lp->height2f;
                v = t * h2f;
                if (tex->params.tWrapMode == GL_REPEAT) {
                    v -= half;
                    row0 = (__GL_FLOOR(v)) & h2mask;
                    row1 = (row0 + 1) & h2mask;
                    repeatState++;
                } else {
                    if (v < __glZero) v = __glZero;
                    else if (v > h2f) v = h2f;
                    v -= half;
                    row0 = __GL_FLOOR(v);
                    row1 = row0 + 1;
                }

                alpha = __GL_FRAC(u);
                beta = __GL_FRAC(v);
                omalpha = __glOne - alpha;
                ombeta = __glOne - beta;
                m00 = f * omalpha * ombeta;
                m10 = f * alpha * ombeta;
                m01 = f * omalpha * beta;
                m11 = f * alpha * beta;

                /* extract texels */
                if (repeatState == 2) {
                    /* both coords are GL_REPEAT..  no borders */
                    offset = (row0 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m00 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m00 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m00 * __GL_UB_TO_FLOAT(image[2]);

                    offset = (row0 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);

                    offset = (row1 << wlog2) + col0;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);

                    offset = (row1 << wlog2) + col1;
                    image = buffer + (offset << 1) + offset;
                    texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                    texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                    texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                } else {
                    /* could have borders */
                    h2mask = ~h2mask;
                    w2mask = ~w2mask;

                    if( (row0 & h2mask) | (col0 & w2mask) ) {
                        texel.r += m00 * tex->params.borderColor.r;
                        texel.g += m00 * tex->params.borderColor.g;
                        texel.b += m00 * tex->params.borderColor.b;
                    } else {
                        offset = (row0 << wlog2) + col0;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m00 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m00 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m00 * __GL_UB_TO_FLOAT(image[2]);
                    }
                    if( (row0 & h2mask) | (col1 & w2mask) ) {
                        texel.r += m10 * tex->params.borderColor.r;
                        texel.g += m10 * tex->params.borderColor.g;
                        texel.b += m10 * tex->params.borderColor.b;
                    } else {
                        offset = (row0 << wlog2) + col1;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                    }
                    if( (row1 & h2mask) | (col0 & w2mask) ) {
                        texel.r += m01 * tex->params.borderColor.r;
                        texel.g += m01 * tex->params.borderColor.g;
                        texel.b += m01 * tex->params.borderColor.b;
                    } else {
                        offset = (row1 << wlog2) + col0;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                    }
                    if( (row1 & h2mask) | (col1 & w2mask) ) {
                        texel.r += m11 * tex->params.borderColor.r;
                        texel.g += m11 * tex->params.borderColor.g;
                        texel.b += m11 * tex->params.borderColor.b;
                    } else {
                        offset = (row1 << wlog2) + col1;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                    }
                }
            }
        }

        /* final pixel processing */
        if( texenvMode == GL_MODULATE ) {
            /* modulate */
            cp->r *= texel.r;
            cp->g *= texel.g;
            cp->b *= texel.b;
        } else {
            if ((texenvMode == GL_DECAL) | 
                (texenvMode == GL_REPLACE)) {
                /* decal or replace */
                cp->r = texel.r * gc->frontBuffer.redScale;
                cp->g = texel.g * gc->frontBuffer.greenScale;
                cp->b = texel.b * gc->frontBuffer.blueScale;
            } else {
                /* blend */
                __GLfloat r = texel.r;
                __GLfloat g = texel.g;
                __GLfloat b = texel.b;
                __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

                cp->r = (__glOne - r) * cp->r + r * cc->r;
                cp->g = (__glOne - g) * cp->g + g * cc->g;
                cp->b = (__glOne - b) * cp->b + b * cc->b;
            }
        }

        /* iterate to next pixel */
        S += gc->polygon.shader.dsdx;
        T += gc->polygon.shader.dtdx;
        qwinv += gc->polygon.shader.dqwdx;
        rhow += gc->polygon.shader.drhowdx;
        cp++;
    }

    return GL_FALSE;
}



/*
**  stippled spans.
**  RGB mip-mapped.
**  Max filter: GL_LINEAR
**  Min filter: GL_LINEAR_MIPMAP_LINEAR
*/
GLboolean __glTextureRGB_L_LML_StippledSpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLstippleWord inMask, bit, *sp;
    __GLfloat S, T, qw;
    GLint w, count;
    __GLfloat rhow, rho;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    __GLfloat w2f, h2f;
    __GLfloat half = __glHalf;
    __GLfloat u, v, alpha, beta, omalpha, ombeta;
    __GLfloat m00, m10, m01, m11;
    GLint row0, col0, row1, col1;
    __GLtexel texel;
    GLint p, d;
    __GLfloat f, omf;
    __GLtextureBuffer *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint w2mask, h2mask;
    GLint wlog2;
    GLint repeatState;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    sp = gc->polygon.shader.stipplePat;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */
    rhow = gc->polygon.shader.frag.rhow;

    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
                qw = __glOne / qwinv;
                s = S * qw;
                t = T * qw;
                rho = rhow * qw;

                /* mipmap the fragment */
                if( rho <= tex->c ) {
                    /* magnify..  Use bilinear (GL_LINEAR) */
                    repeatState = 0;

                    lp = &tex->level[0];
                    wlog2 = lp->widthLog2;
                    buffer = lp->buffer;
                    w2mask = lp->width2 - 1;
                    h2mask = lp->height2 - 1;

                    w2f = lp->width2f;
                    u = s * w2f;
                    if (tex->params.sWrapMode == GL_REPEAT) {
                        u -= half;
                        col0 = (__GL_FLOOR(u)) & w2mask;
                        col1 = (col0 + 1) & w2mask;
                        repeatState++;
                    } else {
                        if (u < __glZero) u = __glZero;
                        else if (u > w2f) u = w2f;
                        u -= half;
                        col0 = __GL_FLOOR(u);
                        col1 = col0 + 1;
                    }
                    
                    h2f = lp->height2f;
                    v = t * h2f;
                    if (tex->params.tWrapMode == GL_REPEAT) {
                        v -= half;
                        row0 = (__GL_FLOOR(v)) & h2mask;
                        row1 = (row0 + 1) & h2mask;
                        repeatState++;
                    } else {
                        if (v < __glZero) v = __glZero;
                        else if (v > h2f) v = h2f;
                        v -= half;
                        row0 = __GL_FLOOR(v);
                        row1 = row0 + 1;
                    }

                    alpha = __GL_FRAC(u);
                    beta = __GL_FRAC(v);
                    omalpha = __glOne - alpha;
                    ombeta = __glOne - beta;
                    m00 = omalpha * ombeta;
                    m10 = alpha * ombeta;
                    m01 = omalpha * beta;
                    m11 = alpha * beta;

                    /* extract texels */
                    if( repeatState == 2 ) {
                        /* both coords are GL_REPEAT..  No borders */
                        offset = (row0 << wlog2) + col0;
                        image = buffer + (offset << 1) + offset;
                        texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);

                        offset = (row0 << wlog2) + col1;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);

                        offset = (row1 << wlog2) + col0;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);

                        offset = (row1 << wlog2) + col1;
                        image = buffer + (offset << 1) + offset;
                        texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                        texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                        texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                    } else {
                        /* could have borders */
                        h2mask = ~h2mask;
                        w2mask = ~w2mask;

                        if( (row0 & h2mask) | (col0 & w2mask) ) {
                            texel.r = m00 * tex->params.borderColor.r;
                            texel.g = m00 * tex->params.borderColor.g;
                            texel.b = m00 * tex->params.borderColor.b;
                        } else {
                            offset = (row0 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);
                        }
                        if( (row0 & h2mask) | (col1 & w2mask) ) {
                            texel.r += m10 * tex->params.borderColor.r;
                            texel.g += m10 * tex->params.borderColor.g;
                            texel.b += m10 * tex->params.borderColor.b;
                        } else {
                            offset = (row0 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                        }
                        if( (row1 & h2mask) | (col0 & w2mask) ) {
                            texel.r += m01 * tex->params.borderColor.r;
                            texel.g += m01 * tex->params.borderColor.g;
                            texel.b += m01 * tex->params.borderColor.b;
                        } else {
                            offset = (row1 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                        }
                        if( (row1 & h2mask) | (col1 & w2mask) ) {
                            texel.r += m11 * tex->params.borderColor.r;
                            texel.g += m11 * tex->params.borderColor.g;
                            texel.b += m11 * tex->params.borderColor.b;
                        } else {
                            offset = (row1 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                        }
                    }

                } else {
                    /* convert rho to lambda */
                    if (rho) {
                        GLuint irho, lev;
                        __GLfloat twotolev;
                        
                        irho = rho;
                        lev = 0;
                        while (irho >>= 1) lev++;
                        twotolev = 1<<lev;
                        rho = (lev + ( (rho-twotolev) / twotolev ) ) * half;
                    } else {
                        rho = __glZero;
                    }

                    /* minify */
                    p = tex->p;
                    d = (GLint)rho;
                    f = rho - d;        /* f is frac(rho) */
                    d++;
                    omf = __glOne - f;
                    if ( (d > p) | (d < 0) ) {
                        /* at the edge..  still, use bilinear */
                        lp = &tex->level[p];
                        wlog2 = lp->widthLog2;
                        buffer = lp->buffer;
                        w2mask = lp->width2 - 1;
                        h2mask = lp->height2 - 1;

                        w2f = lp->width2f;
                        u = s * w2f;
                        if (tex->params.sWrapMode == GL_REPEAT) {
                            u -= half;
                            col0 = (__GL_FLOOR(u)) & w2mask;
                            col1 = (col0 + 1) & w2mask;
                        } else {
                            if (u < __glZero) u = __glZero;
                            else if (u > w2f) u = w2f;
                            u -= half;
                            col0 = __GL_FLOOR(u);
                            col1 = col0 + 1;
                        }

                        h2f = lp->height2f;
                        v = t * h2f;
                        if (tex->params.tWrapMode == GL_REPEAT) {
                            v -= half;
                            row0 = (__GL_FLOOR(v)) & h2mask;
                            row1 = (row0 + 1) & h2mask;
                        } else {
                            if (v < __glZero) v = __glZero;
                            else if (v > h2f) v = h2f;
                            v -= half;
                            row0 = __GL_FLOOR(v);
                            row1 = row0 + 1;
                        }

                        alpha = __GL_FRAC(u);
                        beta = __GL_FRAC(v);
                        omalpha = __glOne - alpha;
                        ombeta = __glOne - beta;
                        m00 = omalpha * ombeta;
                        m10 = alpha * ombeta;
                        m01 = omalpha * beta;
                        m11 = alpha * beta;

                        /* extract texels */
                        h2mask = ~h2mask;
                        w2mask = ~w2mask;

                        if( (row0 & h2mask) | (col0 & w2mask) ) {
                            texel.r = m00 * tex->params.borderColor.r;
                            texel.g = m00 * tex->params.borderColor.g;
                            texel.b = m00 * tex->params.borderColor.b;
                        } else {
                            offset = (row0 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);
                        }
                        if( (row0 & h2mask) | (col1 & w2mask) ) {
                            texel.r += m10 * tex->params.borderColor.r;
                            texel.g += m10 * tex->params.borderColor.g;
                            texel.b += m10 * tex->params.borderColor.b;
                        } else {
                            offset = (row0 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                        }
                        if( (row1 & h2mask) | (col0 & w2mask) ) {
                            texel.r += m01 * tex->params.borderColor.r;
                            texel.g += m01 * tex->params.borderColor.g;
                            texel.b += m01 * tex->params.borderColor.b;
                        } else {
                            offset = (row1 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                        }
                        if( (row1 & h2mask) | (col1 & w2mask) ) {
                            texel.r += m11 * tex->params.borderColor.r;
                            texel.g += m11 * tex->params.borderColor.g;
                            texel.b += m11 * tex->params.borderColor.b;
                        } else {
                            offset = (row1 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                        }
                    } else {
                        /* 
                         ** tri-linear interpolation... 
                         ** bilinear on each level, and linear between levels
                         */

                        /* first do level d-1 */
                        repeatState = 0;
                        lp = &tex->level[d-1];
                        wlog2 = lp->widthLog2;
                        buffer = lp->buffer;
                        w2mask = lp->width2 - 1;
                        h2mask = lp->height2 - 1;

                        w2f = lp->width2f;
                        u = s * w2f;
                        if (tex->params.sWrapMode == GL_REPEAT) {
                            u -= half;
                            col0 = (__GL_FLOOR(u)) & w2mask;
                            col1 = (col0 + 1) & w2mask;
                            repeatState++;
                        } else {
                            if (u < __glZero) u = __glZero;
                            else if (u > w2f) u = w2f;
                            u -= half;
                            col0 = __GL_FLOOR(u);
                            col1 = col0 + 1;
                        }

                        h2f = lp->height2f;
                        v = t * h2f;
                        if (tex->params.tWrapMode == GL_REPEAT) {
                            v -= half;
                            row0 = (__GL_FLOOR(v)) & h2mask;
                            row1 = (row0 + 1) & h2mask;
                            repeatState++;
                        } else {
                            if (v < __glZero) v = __glZero;
                            else if (v > h2f) v = h2f;
                            v -= half;
                            row0 = __GL_FLOOR(v);
                            row1 = row0 + 1;
                        }

                        alpha = __GL_FRAC(u);
                        beta = __GL_FRAC(v);
                        omalpha = __glOne - alpha;
                        ombeta = __glOne - beta;
                        m00 = omf * omalpha * ombeta;
                        m10 = omf * alpha * ombeta;
                        m01 = omf * omalpha * beta;
                        m11 = omf * alpha * beta;

                        /* extract texels */
                        if (repeatState == 2) {
                            /* both coords are GL_REPEAT..  no borders */
                            offset = (row0 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);

                            offset = (row0 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);

                            offset = (row1 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);

                            offset = (row1 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                        } else {
                            /* could have borders */
                            h2mask = ~h2mask;
                            w2mask = ~w2mask;

                            if( (row0 & h2mask) | (col0 & w2mask) ) {
                                texel.r = m00 * tex->params.borderColor.r;
                                texel.g = m00 * tex->params.borderColor.g;
                                texel.b = m00 * tex->params.borderColor.b;
                            } else {
                                offset = (row0 << wlog2) + col0;
                                image = buffer + (offset << 1) + offset;
                                texel.r = m00 * __GL_UB_TO_FLOAT(image[0]);
                                texel.g = m00 * __GL_UB_TO_FLOAT(image[1]);
                                texel.b = m00 * __GL_UB_TO_FLOAT(image[2]);
                            }
                            if( (row0 & h2mask) | (col1 & w2mask) ) {
                                texel.r += m10 * tex->params.borderColor.r;
                                texel.g += m10 * tex->params.borderColor.g;
                                texel.b += m10 * tex->params.borderColor.b;
                            } else {
                                offset = (row0 << wlog2) + col1;
                                image = buffer + (offset << 1) + offset;
                                texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                                texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                                texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                            }
                            if( (row1 & h2mask) | (col0 & w2mask) ) {
                                texel.r += m01 * tex->params.borderColor.r;
                                texel.g += m01 * tex->params.borderColor.g;
                                texel.b += m01 * tex->params.borderColor.b;
                            } else {
                                offset = (row1 << wlog2) + col0;
                                image = buffer + (offset << 1) + offset;
                                texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                                texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                                texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                            }
                            if( (row1 & h2mask) | (col1 & w2mask) ) {
                                texel.r += m11 * tex->params.borderColor.r;
                                texel.g += m11 * tex->params.borderColor.g;
                                texel.b += m11 * tex->params.borderColor.b;
                            } else {
                                offset = (row1 << wlog2) + col1;
                                image = buffer + (offset << 1) + offset;
                                texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                                texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                                texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                            }
                        }

                        /* now do level d */
                        repeatState = 0;
                        lp = &tex->level[d];
                        wlog2 = lp->widthLog2;
                        buffer = lp->buffer;
                        w2mask = lp->width2 - 1;
                        h2mask = lp->height2 - 1;

                        w2f = lp->width2f;
                        u = s * w2f;
                        if (tex->params.sWrapMode == GL_REPEAT) {
                            u -= half;
                            col0 = (__GL_FLOOR(u)) & w2mask;
                            col1 = (col0 + 1) & w2mask;
                            repeatState++;
                        } else {
                            if (u < __glZero) u = __glZero;
                            else if (u > w2f) u = w2f;
                            u -= half;
                            col0 = __GL_FLOOR(u);
                            col1 = col0 + 1;
                        }

                        h2f = lp->height2f;
                        v = t * h2f;
                        if (tex->params.tWrapMode == GL_REPEAT) {
                            v -= half;
                            row0 = (__GL_FLOOR(v)) & h2mask;
                            row1 = (row0 + 1) & h2mask;
                            repeatState++;
                        } else {
                            if (v < __glZero) v = __glZero;
                            else if (v > h2f) v = h2f;
                            v -= half;
                            row0 = __GL_FLOOR(v);
                            row1 = row0 + 1;
                        }

                        alpha = __GL_FRAC(u);
                        beta = __GL_FRAC(v);
                        omalpha = __glOne - alpha;
                        ombeta = __glOne - beta;
                        m00 = f * omalpha * ombeta;
                        m10 = f * alpha * ombeta;
                        m01 = f * omalpha * beta;
                        m11 = f * alpha * beta;

                        /* extract texels */
                        if (repeatState == 2) {
                            /* both coords are GL_REPEAT..  no borders */
                            offset = (row0 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m00 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m00 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m00 * __GL_UB_TO_FLOAT(image[2]);
                            
                            offset = (row0 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);

                            offset = (row1 << wlog2) + col0;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);

                            offset = (row1 << wlog2) + col1;
                            image = buffer + (offset << 1) + offset;
                            texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                            texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                            texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                        } else {
                            /* could have borders */
                            h2mask = ~h2mask;
                            w2mask = ~w2mask;

                            if( (row0 & h2mask) | (col0 & w2mask) ) {
                                texel.r += m00 * tex->params.borderColor.r;
                                texel.g += m00 * tex->params.borderColor.g;
                                texel.b += m00 * tex->params.borderColor.b;
                            } else {
                                offset = (row0 << wlog2) + col0;
                                image = buffer + (offset << 1) + offset;
                                texel.r += m00 * __GL_UB_TO_FLOAT(image[0]);
                                texel.g += m00 * __GL_UB_TO_FLOAT(image[1]);
                                texel.b += m00 * __GL_UB_TO_FLOAT(image[2]);
                            }
                            if( (row0 & h2mask) | (col1 & w2mask) ) {
                                texel.r += m10 * tex->params.borderColor.r;
                                texel.g += m10 * tex->params.borderColor.g;
                                texel.b += m10 * tex->params.borderColor.b;
                            } else {
                                offset = (row0 << wlog2) + col1;
                                image = buffer + (offset << 1) + offset;
                                texel.r += m10 * __GL_UB_TO_FLOAT(image[0]);
                                texel.g += m10 * __GL_UB_TO_FLOAT(image[1]);
                                texel.b += m10 * __GL_UB_TO_FLOAT(image[2]);
                            }
                            if( (row1 & h2mask) | (col0 & w2mask) ) {
                                texel.r += m01 * tex->params.borderColor.r;
                                texel.g += m01 * tex->params.borderColor.g;
                                texel.b += m01 * tex->params.borderColor.b;
                            } else {
                                offset = (row1 << wlog2) + col0;
                                image = buffer + (offset << 1) + offset;
                                texel.r += m01 * __GL_UB_TO_FLOAT(image[0]);
                                texel.g += m01 * __GL_UB_TO_FLOAT(image[1]);
                                texel.b += m01 * __GL_UB_TO_FLOAT(image[2]);
                            }
                            if( (row1 & h2mask) | (col1 & w2mask) ) {
                                texel.r += m11 * tex->params.borderColor.r;
                                texel.g += m11 * tex->params.borderColor.g;
                                texel.b += m11 * tex->params.borderColor.b;
                            } else {
                                offset = (row1 << wlog2) + col1;
                                image = buffer + (offset << 1) + offset;
                                texel.r += m11 * __GL_UB_TO_FLOAT(image[0]);
                                texel.g += m11 * __GL_UB_TO_FLOAT(image[1]);
                                texel.b += m11 * __GL_UB_TO_FLOAT(image[2]);
                            }
                        }
                    }
                }
                
                /* final pixel processing */
                if( texenvMode == GL_MODULATE ) {
                    /* modulate */
                    cp->r *= texel.r;
                    cp->g *= texel.g;
                    cp->b *= texel.b;
                } else {
                    if ((texenvMode == GL_DECAL) | 
                        (texenvMode == GL_REPLACE)) {
                        /* decal or replace */
                        cp->r = texel.r * gc->frontBuffer.redScale;
                        cp->g = texel.g * gc->frontBuffer.greenScale;
                        cp->b = texel.b * gc->frontBuffer.blueScale;
                    } else {
                        /* blend */
                        __GLfloat r = texel.r;
                        __GLfloat g = texel.g;
                        __GLfloat b = texel.b;
                        __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;
                        
                        cp->r = (__glOne - r) * cp->r + r * cc->r;
                        cp->g = (__glOne - g) * cp->g + g * cc->g;
                        cp->b = (__glOne - b) * cp->b + b * cc->b;
                    }
                }
            }

            /* iterate to next pixel */
            S += gc->polygon.shader.dsdx;
            T += gc->polygon.shader.dtdx;
            qwinv += gc->polygon.shader.dqwdx;
            rhow += gc->polygon.shader.drhowdx;
            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }

    return GL_FALSE;
}



/*
**  non-stippled spans.
**  RGB non-mip-mapped.
**  Max filter: GL_NEAREST
**  Min filter: GL_NEAREST
*/
GLboolean __glTextureRGB_N_N_Span(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat S, T, qw;
    GLint w;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    GLint row, col;
    __GLtexel texel;
    __GLtextureBuffer *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint wlog2;
    GLint w2mask, h2mask;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */

    while( --w >= 0 ) {
        qw = __glOne / qwinv;
        s = S * qw;
        t = T * qw;

        /* do it! */
        lp = &tex->level[0];
        wlog2 = lp->widthLog2;
        buffer = lp->buffer;
        w2mask = lp->width2 - 1;
        h2mask = lp->height2 - 1;

        if (tex->params.sWrapMode == GL_REPEAT) {
            col = (GLint) s & w2mask;
        } else {
            GLint w2 = lp->width2;
            col = (GLint) s;
            if (col < 0) col = 0;
            else if (col >= w2) col = w2mask;
        }
        
        if (tex->params.tWrapMode == GL_REPEAT) {
            row = (GLint) t & h2mask;
        } else {
            GLint h2 = lp->height2;
            row = (GLint) t;
            if (row < 0) row = 0;
            else if (row >= h2) row = h2mask;
        }

        /* extract texel */
        h2mask = ~h2mask;
        w2mask = ~w2mask;

        if( (row & h2mask) | (col & w2mask) ) {
            texel.r = tex->params.borderColor.r;
            texel.g = tex->params.borderColor.g;
            texel.b = tex->params.borderColor.b;
        } else {
            offset = (row << wlog2) + col;
            image = buffer + (offset << 1) + offset;
            texel.r = __GL_UB_TO_FLOAT(image[0]);
            texel.g = __GL_UB_TO_FLOAT(image[1]);
            texel.b = __GL_UB_TO_FLOAT(image[2]);
        }

        /* final pixel processing */
        if( texenvMode == GL_MODULATE ) {
            /* modulate */
            cp->r *= texel.r;
            cp->g *= texel.g;
            cp->b *= texel.b;
        } else {
            if ((texenvMode == GL_DECAL) | 
                (texenvMode == GL_REPLACE)) {
                /* decal or replace */
                cp->r = texel.r * gc->frontBuffer.redScale;
                cp->g = texel.g * gc->frontBuffer.greenScale;
                cp->b = texel.b * gc->frontBuffer.blueScale;
            } else {
                /* blend */
                __GLfloat r = texel.r;
                __GLfloat g = texel.g;
                __GLfloat b = texel.b;
                __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

                cp->r = (__glOne - r) * cp->r + r * cc->r;
                cp->g = (__glOne - g) * cp->g + g * cc->g;
                cp->b = (__glOne - b) * cp->b + b * cc->b;
            }
        }

        /* iterate to next pixel */
        S += gc->polygon.shader.dsdx;
        T += gc->polygon.shader.dtdx;
        qwinv += gc->polygon.shader.dqwdx;
        cp++;
    }

    return GL_FALSE;
}

/*
**  stippled spans.
**  RGB non-mip-mapped.
**  Max filter: GL_NEAREST
**  Min filter: GL_NEAREST
*/
GLboolean __glTextureRGB_N_N_StippledSpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLstippleWord inMask, bit, *sp;
    __GLfloat S, T, qw;
    GLint w, count;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    GLint row, col;
    __GLtexel texel;
    __GLtextureBuffer *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint w2mask, h2mask;
    GLint wlog2;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    sp = gc->polygon.shader.stipplePat;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */

    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
        
                qw = __glOne / qwinv;
                s = S * qw;
                t = T * qw;

                /* do it! */
                lp = &tex->level[0];
                wlog2 = lp->widthLog2;
                buffer = lp->buffer;
                w2mask = lp->width2 - 1;
                h2mask = lp->height2 - 1;


                if (tex->params.sWrapMode == GL_REPEAT) {
                    col = (GLint) s & w2mask;
                } else {
                    GLint w2 = lp->width2;
                    col = (GLint) s;
                    if (col < 0) col = 0;
                    else if (col >= w2) col = w2mask;
                }
                
                if (tex->params.tWrapMode == GL_REPEAT) {
                    row = (GLint) t & h2mask;
                } else {
                    GLint h2 = lp->height2;
                    row = (GLint) t;
                    if (row < 0) row = 0;
                    else if (row >= h2) row = h2mask;
                }


                /* extract texel */
                h2mask = ~h2mask;
                w2mask = ~w2mask;

                if( (row & h2mask) | (col & w2mask) ) {
                    texel.r = tex->params.borderColor.r;
                    texel.g = tex->params.borderColor.g;
                    texel.b = tex->params.borderColor.b;
                } else {
                    offset = (row << wlog2) + col;
                    image = buffer + (offset << 1) + offset;
                    texel.r = __GL_UB_TO_FLOAT(image[0]);
                    texel.g = __GL_UB_TO_FLOAT(image[1]);
                    texel.b = __GL_UB_TO_FLOAT(image[2]);
                }

                /* final pixel processing */
                if( texenvMode == GL_MODULATE ) {
                    /* modulate */
                    cp->r *= texel.r;
                    cp->g *= texel.g;
                    cp->b *= texel.b;
                } else {
                    if ((texenvMode == GL_DECAL) | 
                        (texenvMode == GL_REPLACE)) {
                        /* decal or replace */
                        cp->r = texel.r * gc->frontBuffer.redScale;
                        cp->g = texel.g * gc->frontBuffer.greenScale;
                        cp->b = texel.b * gc->frontBuffer.blueScale;
                    } else {
                        /* blend */
                        __GLfloat r = texel.r;
                        __GLfloat g = texel.g;
                        __GLfloat b = texel.b;
                        __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;
                        
                        cp->r = (__glOne - r) * cp->r + r * cc->r;
                        cp->g = (__glOne - g) * cp->g + g * cc->g;
                        cp->b = (__glOne - b) * cp->b + b * cc->b;
                    }
                }
            }

            /* iterate to next pixel */
            S += gc->polygon.shader.dsdx;
            T += gc->polygon.shader.dtdx;
            qwinv += gc->polygon.shader.dqwdx;
            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }

    return GL_FALSE;
}



/*
**  non-stippled spans.
**  RGB mip-mapped.
**  Max filter: GL_NEAREST
**  Min filter: GL_NEAREST_MIPMAP_NEAREST
*/
GLboolean __glTextureRGB_N_NMN_Span(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat S, T, qw;
    GLint w;
    __GLfloat rhow, rho;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    __GLfloat w2f, h2f;
    __GLfloat half = __glHalf;
    GLint row, col;
    __GLtexel texel;
    GLint p, d;
    __GLtextureBuffer *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint w2mask, h2mask;
    GLint wlog2;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */
    rhow = gc->polygon.shader.frag.rhow;

    while( --w >= 0 ) {
        qw = __glOne / qwinv;
        s = S * qw;
        t = T * qw;
        rho = rhow * qw;

        /* mipmap the fragment */
        if( rho <= tex->c ) {
            /* magnify..  Use GL_NEAREST */
            lp = &tex->level[0];
        } else {
            /* convert rho to lambda */
            if (rho) {
                GLuint irho, lev;
                __GLfloat twotolev;
                
                irho = rho;
                lev = 0;
                while (irho >>= 1) lev++;
                twotolev = 1<<lev;
                rho = (lev + ( (rho-twotolev) / twotolev ) ) * half;
            } else {
                rho = __glZero;
            }

            if (rho <= 0.5f) {
                d = 0;
            } else {
                p = tex->p;
                d = (GLint)(rho + ((__GLfloat)0.49995)); /* NOTE: .5 minus epsilon */
                if (d > p) {
                    d = p;
                }
            }

            lp = &tex->level[d];
        }

        wlog2 = lp->widthLog2;
        buffer = lp->buffer;
        w2mask = lp->width2 - 1;
        h2mask = lp->height2 - 1;

        w2f = lp->width2f;
        if (tex->params.sWrapMode == GL_REPEAT) {
            col = (GLint)(__GL_FRAC(s) * w2f);
        } else {
            GLint w2 = lp->width2;
            col = (GLint)(s * w2f);
            if (col < 0) col = 0;
            else if (col >= w2) col = w2mask;
        }
        
        h2f = lp->height2f;
        if (tex->params.tWrapMode == GL_REPEAT) {
            row = (GLint)(__GL_FRAC(t) * h2f);
        } else {
            GLint h2 = lp->height2;
            row = (GLint)(t * h2f);
            if (row < 0) row = 0;
            else if (row >= h2) row = h2mask;
        }
        

        /* extract texel */
        h2mask = ~h2mask;
        w2mask = ~w2mask;

        if( (row & h2mask) | (col & w2mask) ) {
            texel.r = tex->params.borderColor.r;
            texel.g = tex->params.borderColor.g;
            texel.b = tex->params.borderColor.b;
        } else {
            offset = (row << wlog2) + col;
            image = buffer + (offset << 1) + offset;
            texel.r = __GL_UB_TO_FLOAT(image[0]);
            texel.g = __GL_UB_TO_FLOAT(image[1]);
            texel.b = __GL_UB_TO_FLOAT(image[2]);
        }

        /* final pixel processing */
        if( texenvMode == GL_MODULATE ) {
            /* modulate */
            cp->r *= texel.r;
            cp->g *= texel.g;
            cp->b *= texel.b;
        } else {
            if ((texenvMode == GL_DECAL) | 
                (texenvMode == GL_REPLACE)) {
                /* decal or replace */
                cp->r = texel.r * gc->frontBuffer.redScale;
                cp->g = texel.g * gc->frontBuffer.greenScale;
                cp->b = texel.b * gc->frontBuffer.blueScale;
            } else {
                /* blend */
                __GLfloat r = texel.r;
                __GLfloat g = texel.g;
                __GLfloat b = texel.b;
                __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

                cp->r = (__glOne - r) * cp->r + r * cc->r;
                cp->g = (__glOne - g) * cp->g + g * cc->g;
                cp->b = (__glOne - b) * cp->b + b * cc->b;
            }
        }

        /* iterate to next pixel */
        S += gc->polygon.shader.dsdx;
        T += gc->polygon.shader.dtdx;
        qwinv += gc->polygon.shader.dqwdx;
        rhow += gc->polygon.shader.drhowdx;
        cp++;
    }

    return GL_FALSE;
}

/*
**  stippled spans.
**  RGB mip-mapped.
**  Max filter: GL_NEAREST
**  Min filter: GL_NEAREST_MIPMAP_NEAREST
*/
GLboolean __glTextureRGB_N_NMN_StippledSpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLstippleWord inMask, bit, *sp;
    __GLfloat S, T, qw;
    GLint w, count;
    __GLfloat rhow, rho;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    __GLfloat w2f, h2f;
    __GLfloat half = __glHalf;
    GLint row, col;
    __GLtexel texel;
    GLint p, d;
    __GLtextureBuffer *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint w2mask, h2mask;
    GLint wlog2;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    sp = gc->polygon.shader.stipplePat;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */
    rhow = gc->polygon.shader.frag.rhow;

    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
        
                qw = __glOne / qwinv;
                s = S * qw;
                t = T * qw;
                rho = rhow * qw;

                /* mipmap the fragment */
                if( rho <= tex->c ) {
                    /* magnify..  Use GL_NEAREST */
                    lp = &tex->level[0];
                } else {
                    /* convert rho to lambda */
                    if (rho) {
                        GLuint irho, lev;
                        __GLfloat twotolev;
                        
                        irho = rho;
                        lev = 0;
                        while (irho >>= 1) lev++;
                        twotolev = 1<<lev;
                        rho = (lev + ( (rho-twotolev) / twotolev ) ) * half;
                    } else {
                        rho = __glZero;
                    }

                    if (rho <= 0.5f) {
                        d = 0;
                    } else {
                        p = tex->p;
                        d = (GLint)(rho + ((__GLfloat)0.49995)); /* NOTE: .5 minus epsilon */
                        if (d > p) {
                            d = p;
                        }
                    }

                    lp = &tex->level[d];
                }

                wlog2 = lp->widthLog2;
                buffer = lp->buffer;
                w2mask = lp->width2 - 1;
                h2mask = lp->height2 - 1;

                w2f = lp->width2f;
                if (tex->params.sWrapMode == GL_REPEAT) {
                    col = (GLint)(__GL_FRAC(s) * w2f);
                } else {
                    GLint w2 = lp->width2;
                    col = (GLint)(s * w2f);
                    if (col < 0) col = 0;
                    else if (col >= w2) col = w2mask;
                }
        
                h2f = lp->height2f;
                if (tex->params.tWrapMode == GL_REPEAT) {
                    row = (GLint)(__GL_FRAC(t) * h2f);
                } else {
                    GLint h2 = lp->height2;
                    row = (GLint)(t * h2f);
                    if (row < 0) row = 0;
                    else if (row >= h2) row = h2mask;
                }
        

                /* extract texel */
                h2mask = ~h2mask;
                w2mask = ~w2mask;

                if( (row & h2mask) | (col & w2mask) ) {
                    texel.r = tex->params.borderColor.r;
                    texel.g = tex->params.borderColor.g;
                    texel.b = tex->params.borderColor.b;
                } else {
                    offset = (row << wlog2) + col;
                    image = buffer + (offset << 1) + offset;
                    texel.r = __GL_UB_TO_FLOAT(image[0]);
                    texel.g = __GL_UB_TO_FLOAT(image[1]);
                    texel.b = __GL_UB_TO_FLOAT(image[2]);
                }

                /* final pixel processing */
                if( texenvMode == GL_MODULATE ) {
                    /* modulate */
                    cp->r *= texel.r;
                    cp->g *= texel.g;
                    cp->b *= texel.b;
                } else {
                    if ((texenvMode == GL_DECAL) | 
                        (texenvMode == GL_REPLACE)) {
                        /* decal or replace */
                        cp->r = texel.r * gc->frontBuffer.redScale;
                        cp->g = texel.g * gc->frontBuffer.greenScale;
                        cp->b = texel.b * gc->frontBuffer.blueScale;
                    } else {
                        /* blend */
                        __GLfloat r = texel.r;
                        __GLfloat g = texel.g;
                        __GLfloat b = texel.b;
                        __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;
                        
                        cp->r = (__glOne - r) * cp->r + r * cc->r;
                        cp->g = (__glOne - g) * cp->g + g * cc->g;
                        cp->b = (__glOne - b) * cp->b + b * cc->b;
                    }
                }
            }

            /* iterate to next pixel */
            S += gc->polygon.shader.dsdx;
            T += gc->polygon.shader.dtdx;
            qwinv += gc->polygon.shader.dqwdx;
            rhow += gc->polygon.shader.drhowdx;
            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }

    return GL_FALSE;
}

/*
**  non-stippled spans.
**  COLOR_INDEX8 non-mip-mapped.
**  Max filter: GL_NEAREST
**  Min filter: GL_NEAREST
**  ColorTable: RGB
*/
GLboolean __glTextureCI8_N_N_RGB_Span(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat S, T, qw;
    GLint w;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    GLint row, col;
    __GLtexel texel;
    GLubyte *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint wlog2;
    GLint w2mask, h2mask;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;
    GLubyte *ctable;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */

    while( --w >= 0 ) {
        qw = __glOne / qwinv;
        s = S * qw;
        t = T * qw;

        /* do it! */
        lp = &tex->level[0];
        wlog2 = lp->widthLog2;
        buffer = (GLubyte *) lp->buffer;
        ctable = tex->CT.table;
        w2mask = lp->width2 - 1;
        h2mask = lp->height2 - 1;

        if (tex->params.sWrapMode == GL_REPEAT) {
            col = (GLint) s & w2mask;
        } else {
            GLint w2 = lp->width2;
            col = (GLint) s;
            if (col < 0) col = 0;
            else if (col >= w2) col = w2mask;
        }
        
        if (tex->params.tWrapMode == GL_REPEAT) {
            row = (GLint) t & h2mask;
        } else {
            GLint h2 = lp->height2;
            row = (GLint) t;
            if (row < 0) row = 0;
            else if (row >= h2) row = h2mask;
        }

        /* extract texel */
        h2mask = ~h2mask;
        w2mask = ~w2mask;

        if( (row & h2mask) | (col & w2mask) ) {
            offset = tex->params.borderColor.r;
        } else {
            image = buffer + (row << wlog2) + col;
            offset = image[0];
        }

        offset += offset<<1;
        texel.r = __GL_UB_TO_FLOAT(ctable[offset]);
        texel.g = __GL_UB_TO_FLOAT(ctable[offset+1]);
        texel.b = __GL_UB_TO_FLOAT(ctable[offset+2]);

        /* final pixel processing */
        if( texenvMode == GL_MODULATE ) {
            /* modulate */
            cp->r *= texel.r;
            cp->g *= texel.g;
            cp->b *= texel.b;
        } else {
            if ((texenvMode == GL_DECAL) | 
                (texenvMode == GL_REPLACE)) {
                /* decal or replace */
                cp->r = texel.r * gc->frontBuffer.redScale;
                cp->g = texel.g * gc->frontBuffer.greenScale;
                cp->b = texel.b * gc->frontBuffer.blueScale;
            } else {
                /* blend */
                __GLfloat r = texel.r;
                __GLfloat g = texel.g;
                __GLfloat b = texel.b;
                __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;

                cp->r = (__glOne - r) * cp->r + r * cc->r;
                cp->g = (__glOne - g) * cp->g + g * cc->g;
                cp->b = (__glOne - b) * cp->b + b * cc->b;
            }
        }

        /* iterate to next pixel */
        S += gc->polygon.shader.dsdx;
        T += gc->polygon.shader.dtdx;
        qwinv += gc->polygon.shader.dqwdx;
        cp++;
    }

    return GL_FALSE;
}

/*
**  stippled spans.
**  COLOR_INDEX8 non-mip-mapped.
**  Max filter: GL_NEAREST
**  Min filter: GL_NEAREST
**  ColorTable: RGB
*/
GLboolean __glTextureCI8_N_N_RGB_StippledSpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLstippleWord inMask, bit, *sp;
    __GLfloat S, T, qw;
    GLint w, count;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    GLint row, col;
    __GLtexel texel;
    GLubyte *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint w2mask, h2mask;
    GLint wlog2;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;
    GLubyte *ctable;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    sp = gc->polygon.shader.stipplePat;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */

    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
        
                qw = __glOne / qwinv;
                s = S * qw;
                t = T * qw;

                /* do it! */
                lp = &tex->level[0];
                wlog2 = lp->widthLog2;
                buffer = (GLubyte *)lp->buffer;
                ctable = tex->CT.table;
                w2mask = lp->width2 - 1;
                h2mask = lp->height2 - 1;

                if (tex->params.sWrapMode == GL_REPEAT) {
                    col = (GLint) s & w2mask;
                } else {
                    GLint w2 = lp->width2;
                    col = (GLint) s;
                    if (col < 0) col = 0;
                    else if (col >= w2) col = w2mask;
                }
                
                if (tex->params.tWrapMode == GL_REPEAT) {
                    row = (GLint) t & h2mask;
                } else {
                    GLint h2 = lp->height2;
                    row = (GLint) t;
                    if (row < 0) row = 0;
                    else if (row >= h2) row = h2mask;
                }


                /* extract texel */
                h2mask = ~h2mask;
                w2mask = ~w2mask;

                if( (row & h2mask) | (col & w2mask) ) {
                    offset = tex->params.borderColor.r;
                } else {
                    image = buffer + (row << wlog2) + col;
                    offset = image[0];
                }

                offset += offset<<1;
                texel.r = __GL_UB_TO_FLOAT(ctable[offset]);
                texel.g = __GL_UB_TO_FLOAT(ctable[offset+1]);
                texel.b = __GL_UB_TO_FLOAT(ctable[offset+2]);

                /* final pixel processing */
                if( texenvMode == GL_MODULATE ) {
                    /* modulate */
                    cp->r *= texel.r;
                    cp->g *= texel.g;
                    cp->b *= texel.b;
                } else {
                    if ((texenvMode == GL_DECAL) | 
                        (texenvMode == GL_REPLACE)) {
                        /* decal or replace */
                        cp->r = texel.r * gc->frontBuffer.redScale;
                        cp->g = texel.g * gc->frontBuffer.greenScale;
                        cp->b = texel.b * gc->frontBuffer.blueScale;
                    } else {
                        /* blend */
                        __GLfloat r = texel.r;
                        __GLfloat g = texel.g;
                        __GLfloat b = texel.b;
                        __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;
                        
                        cp->r = (__glOne - r) * cp->r + r * cc->r;
                        cp->g = (__glOne - g) * cp->g + g * cc->g;
                        cp->b = (__glOne - b) * cp->b + b * cc->b;
                    }
                }
            }

            /* iterate to next pixel */
            S += gc->polygon.shader.dsdx;
            T += gc->polygon.shader.dtdx;
            qwinv += gc->polygon.shader.dqwdx;
            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }

    return GL_FALSE;
}

/*
**  non-stippled spans.
**  COLOR_INDEX8 non-mip-mapped.
**  Max filter: GL_NEAREST
**  Min filter: GL_NEAREST
**  ColorTable: RGBA
*/
GLboolean __glTextureCI8_N_N_RGBA_Span(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLfloat S, T, qw;
    GLint w;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    GLint row, col;
    __GLtexel texel;
    GLubyte *image, *buffer;
    __GLmipMapLevel *lp;
    GLint offset;
    GLint wlog2;
    GLint w2mask, h2mask;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;
    GLubyte *ctable;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */

    while( --w >= 0 ) {
        qw = __glOne / qwinv;
        s = S * qw;
        t = T * qw;

        /* do it! */
        lp = &tex->level[0];
        wlog2 = lp->widthLog2;
        buffer = (GLubyte *) lp->buffer;
        ctable = tex->CT.table;
        w2mask = lp->width2 - 1;
        h2mask = lp->height2 - 1;

        if (tex->params.sWrapMode == GL_REPEAT) {
            col = (GLint) s & w2mask;
        } else {
            GLint w2 = lp->width2;
            col = (GLint) s;
            if (col < 0) col = 0;
            else if (col >= w2) col = w2mask;
        }
        
        if (tex->params.tWrapMode == GL_REPEAT) {
            row = (GLint) t & h2mask;
        } else {
            GLint h2 = lp->height2;
            row = (GLint) t;
            if (row < 0) row = 0;
            else if (row >= h2) row = h2mask;
        }

        /* extract texel */
        h2mask = ~h2mask;
        w2mask = ~w2mask;

        if( (row & h2mask) | (col & w2mask) ) {
            offset = tex->params.borderColor.r;
        } else {
            image = buffer + (row << wlog2) + col;
            offset = image[0];

        }

        offset <<= 2;
        texel.r = __GL_UB_TO_FLOAT(ctable[offset]);
        texel.g = __GL_UB_TO_FLOAT(ctable[offset+1]);
        texel.b = __GL_UB_TO_FLOAT(ctable[offset+2]);
        texel.alpha = __GL_UB_TO_FLOAT(ctable[offset+3]);

        /* final pixel processing */
        if( texenvMode == GL_MODULATE ) {
            /* modulate */
            cp->r *= texel.r;
            cp->g *= texel.g;
            cp->b *= texel.b;
            cp->a *= texel.alpha;
        } else if (texenvMode == GL_DECAL) {
            /* decal */
            __GLfloat a = texel.alpha;
            __GLfloat oma = __glOne - a;
            cp->r = oma * cp->r
                + a * texel.r * gc->frontBuffer.redScale;
            cp->g = oma * cp->g
                + a * texel.g * gc->frontBuffer.greenScale;
            cp->b = oma * cp->b
                + a * texel.b * gc->frontBuffer.blueScale;
        } else if (texenvMode == GL_REPLACE) {
            /* replace */
            cp->r = texel.r * gc->frontBuffer.redScale;
            cp->g = texel.g * gc->frontBuffer.greenScale;
            cp->b = texel.b * gc->frontBuffer.blueScale;
            cp->a = texel.alpha * gc->frontBuffer.alphaScale;
        } else {
            /* blend */
            __GLfloat r = texel.r;
            __GLfloat g = texel.g;
            __GLfloat b = texel.b;
            __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;
                        
            cp->r = (__glOne - r) * cp->r + r * cc->r;
            cp->g = (__glOne - g) * cp->g + g * cc->g;
            cp->b = (__glOne - b) * cp->b + b * cc->b;
            cp->a = texel.alpha * cp->a;
        }

        /* iterate to next pixel */
        S += gc->polygon.shader.dsdx;
        T += gc->polygon.shader.dtdx;
        qwinv += gc->polygon.shader.dqwdx;
        cp++;
    }

    return GL_FALSE;
}

/*
**  stippled spans.
**  COLOR_INDEX8 non-mip-mapped.
**  Max filter: GL_NEAREST
**  Min filter: GL_NEAREST
**  ColorTable: RGBA
*/
GLboolean __glTextureCI8_N_N_RGBA_StippledSpan(__GLcontext *gc)
{
    __GLcolor *cp;
    __GLstippleWord inMask, bit, *sp;
    __GLfloat S, T, qw;
    GLint w, count;
    __GLfloat s, t, qwinv;
    __GLtexture *tex;
    GLint row, col;
    __GLtexel texel;
    GLubyte *image, *buffer;
    __GLmipMapLevel *lp;
    GLuint offset;
    GLint w2mask, h2mask;
    GLint wlog2;
    GLint texenvMode = gc->state.texture[gc->texture.currentTexUnit].env[0].mode;
    GLubyte *ctable;

    tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    w = gc->polygon.shader.length;
    cp = gc->polygon.shader.colors;
    sp = gc->polygon.shader.stipplePat;

    S = gc->polygon.shader.frag.s;
    T = gc->polygon.shader.frag.t;
    qwinv = gc->polygon.shader.frag.qw; /* frag.qw is actually inverse */

    while (w) {
        count = w;
        if (count > __GL_STIPPLE_BITS) {
            count = __GL_STIPPLE_BITS;
        }
        w -= count;

        inMask = *sp++;
        bit = __GL_STIPPLE_SHIFT(0);
        while (--count >= 0) {
            if (inMask & bit) {
        
                qw = __glOne / qwinv;
                s = S * qw;
                t = T * qw;

                /* do it! */
                lp = &tex->level[0];
                wlog2 = lp->widthLog2;
                buffer = (GLubyte *)lp->buffer;
                ctable = tex->CT.table;
                w2mask = lp->width2 - 1;
                h2mask = lp->height2 - 1;

                if (tex->params.sWrapMode == GL_REPEAT) {
                    col = (GLint) s & w2mask;
                } else {
                    GLint w2 = lp->width2;
                    col = (GLint) s;
                    if (col < 0) col = 0;
                    else if (col >= w2) col = w2mask;
                }
                
                if (tex->params.tWrapMode == GL_REPEAT) {
                    row = (GLint) t & h2mask;
                } else {
                    GLint h2 = lp->height2;
                    row = (GLint) t;
                    if (row < 0) row = 0;
                    else if (row >= h2) row = h2mask;
                }


                /* extract texel */
                h2mask = ~h2mask;
                w2mask = ~w2mask;

                if( (row & h2mask) | (col & w2mask) ) {
                    offset = tex->params.borderColor.r;
                } else {
                    image = buffer + (row << wlog2) + col;
                    offset = image[0];
                }

                offset <<= 2;
                texel.r = __GL_UB_TO_FLOAT(ctable[offset]);
                texel.g = __GL_UB_TO_FLOAT(ctable[offset+1]);
                texel.b = __GL_UB_TO_FLOAT(ctable[offset+2]);
                texel.alpha = __GL_UB_TO_FLOAT(ctable[offset+3]);

                /* final pixel processing */
                if( texenvMode == GL_MODULATE ) {
                    /* modulate */
                    cp->r *= texel.r;
                    cp->g *= texel.g;
                    cp->b *= texel.b;
                    cp->a *= texel.alpha;
                } else if (texenvMode == GL_DECAL) {
                    /* decal */
                    __GLfloat a = texel.alpha;
                    __GLfloat oma = __glOne - a;
                    cp->r = oma * cp->r
                        + a * texel.r * gc->frontBuffer.redScale;
                    cp->g = oma * cp->g
                        + a * texel.g * gc->frontBuffer.greenScale;
                    cp->b = oma * cp->b
                        + a * texel.b * gc->frontBuffer.blueScale;
                } else if (texenvMode == GL_REPLACE) {
                    /* replace */
                    cp->r = texel.r * gc->frontBuffer.redScale;
                    cp->g = texel.g * gc->frontBuffer.greenScale;
                    cp->b = texel.b * gc->frontBuffer.blueScale;
                    cp->a = texel.alpha * gc->frontBuffer.alphaScale;
                } else {
                    /* blend */
                    __GLfloat r = texel.r;
                    __GLfloat g = texel.g;
                    __GLfloat b = texel.b;
                    __GLcolor *cc = &gc->state.texture[gc->texture.currentTexUnit].env[0].color;
                        
                    cp->r = (__glOne - r) * cp->r + r * cc->r;
                    cp->g = (__glOne - g) * cp->g + g * cc->g;
                    cp->b = (__glOne - b) * cp->b + b * cc->b;
                    cp->a = texel.alpha * cp->a;
                }
            }

            /* iterate to next pixel */
            S += gc->polygon.shader.dsdx;
            T += gc->polygon.shader.dtdx;
            qwinv += gc->polygon.shader.dqwdx;
            cp++;
#ifdef __GL_STIPPLE_MSB
            bit >>= 1;
#else
            bit <<= 1;
#endif
        }
    }

    return GL_FALSE;
}


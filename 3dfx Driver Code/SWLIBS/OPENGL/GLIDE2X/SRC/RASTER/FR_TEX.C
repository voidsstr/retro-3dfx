/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
#include "imports.h"
#include "global.h"
#include "g_imfncs.h"
#include "types.h"
#include "namesint.h"
#include "pixel.h"
#include "image.h"
#include "glmath.h"
#include <memory.h>
#include "fr_tri.h"
#include "fr_tex.h"
#include "fr_modes.h"

/* Fixed point contants */
#define STRQ_ONE  (1<<STRQ_FRAC_BITS)
#define STRQ_HALF (STRQ_ONE>>1)

#define MIN_FLOAT (1.401298e-45)

#define ExtractBits(f)          (*(int*)&f)
#define ExtractExponent(f)      (((ExtractBits(f)&0x7f800000)>>23)-127)
#define ExtractMantissa(f)      (ExtractBits(f)&0x007fffff)

#define ExtractLOD(rho) ((ExtractExponent(rho)<<(STRQ_FRAC_BITS-1))|\
                         (ExtractMantissa(rho)>>(23-STRQ_FRAC_BITS+1)))

/* For now, we force perspective correction if needRho */
#define AFFINE_RHO 0

/**************************************************************************
 * Extract procs
 **************************************************************************/

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glFRExtractTexelL(__GLmipMapLevel *level, __GLtexture *tex,
                         GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;
    GLuint luminance;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        luminance = __GL_FLOAT_TO_UB(tex->params.borderColor.r);
    } else {
        image = level->buffer + ((row << level->widthLog2) + col);
        luminance = image[0];
    }
    result[0] = luminance;
    result[1] = luminance;
    result[2] = luminance;
}

/*
** Get a texture element out of the two component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glFRExtractTexelLA(__GLmipMapLevel *level, __GLtexture *tex,
                          GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;
    GLuint luminance, alpha;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        luminance = __GL_FLOAT_TO_UB(tex->params.borderColor.r);
        alpha = __GL_FLOAT_TO_UB(tex->params.borderColor.a);
    } else {
        image = level->buffer + ((row << level->widthLog2) + col) * 2;
        luminance = image[0];
        alpha = image[1];
    }
    result[0] = luminance;
    result[1] = luminance;
    result[2] = luminance;
    result[3] = alpha;
}

/*
** Get a texture element out of the three component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glFRExtractTexelRGB(__GLmipMapLevel *level, __GLtexture *tex,
                           GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result[0] = __GL_FLOAT_TO_UB(tex->params.borderColor.r);
        result[1] = __GL_FLOAT_TO_UB(tex->params.borderColor.g);
        result[2] = __GL_FLOAT_TO_UB(tex->params.borderColor.b);
    } else {
        image = level->buffer + ((row << level->widthLog2) + col) * 3;
        result[0] = image[0];
        result[1] = image[1];
        result[2] = image[2];
    }
}

/*
** Get a texture element out of the four component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glFRExtractTexelRGBA(__GLmipMapLevel *level, __GLtexture *tex,
                            GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result[0] = __GL_FLOAT_TO_UB(tex->params.borderColor.r);
        result[1] = __GL_FLOAT_TO_UB(tex->params.borderColor.g);
        result[2] = __GL_FLOAT_TO_UB(tex->params.borderColor.b);
        result[3] = __GL_FLOAT_TO_UB(tex->params.borderColor.a);
    } else {
        image = level->buffer + (((row << level->widthLog2) + col) << 2);
        *(GLuint *)result = *(GLuint *) image;
    }
}

void __glFRExtractTexelA(__GLmipMapLevel *level, __GLtexture *tex,
                         GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result[0] = tex->params.borderColor.a;
    } else {
        image = level->buffer + ((row << level->widthLog2) + col);
        result[0] = image[0];
    }
}

/* ARGSUSED */
void __glFRExtractTexelI(__GLmipMapLevel *level, __GLtexture *tex,
                         GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;
    GLuint intensity;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        intensity = __GL_FLOAT_TO_UB(tex->params.borderColor.r);
    } else {
        image = level->buffer + ((row << level->widthLog2) + col);
        intensity = image[0];
    }
    result[0] = intensity;
    result[1] = intensity;
    result[2] = intensity;
    result[3] = intensity;
}

#if 0
/* ARGSUSED */
void __glFRExtractTexelRGB1555(__GLmipMapLevel *level, __GLtexture *tex,
                               GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result[0] = tex->params.borderColor.r;
        result[1] = tex->params.borderColor.g;
        result[2] = tex->params.borderColor.b;
    } else {
        image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);

        texel = image[0];
        result[0] = (__GLfloat) ((texel >> 10) & 0x1F) / 31.0F;
        result[1] = (__GLfloat) ((texel >>  5) & 0x1F) / 31.0F;
        result[2] = (__GLfloat) ((texel      ) & 0x1F) / 31.0F;
    }
}

/* ARGSUSED */
void __glFRExtractTexelRGB565(__GLmipMapLevel *level, __GLtexture *tex,
                              GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        result[0] = tex->params.borderColor.r;
        result[1] = tex->params.borderColor.g;
        result[2] = tex->params.borderColor.b;
    } else {
        image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
        texel = image[0];
        result[0] = (__GLfloat) ((texel >> 11) & 0x1F) / 31.0F;
        result[1] = (__GLfloat) ((texel >>  5) & 0x3F) / 63.0F;
        result[2] = (__GLfloat) ((texel      ) & 0x1F) / 31.0F;
    }
}
#endif

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glFRExtractTexelCI8(__GLmipMapLevel *level, __GLtexture *tex,
                           GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    GLubyte *image;
    GLuint index;
    __GLcolorTable *ct;

    ct = &tex->CT;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        index = __GL_FLOAT_TO_UB(tex->params.borderColor.r);
    } else {
        image = ((GLubyte*)level->buffer) + ((row << level->widthLog2) + col);
        index = image[0];
    }

    switch (ct->baseFormat) {
    case GL_LUMINANCE:
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        break;
    case GL_LUMINANCE_ALPHA:
        index <<= 1;
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        result[3] = ct->table[index+1];
        break;
    case GL_RGB:
        index += index<<1;
        result[0] = ct->table[index];
        result[1] = ct->table[index+1];
        result[2] = ct->table[index+2];
        break;
    case GL_RGBA:
        index <<= 2;
        result[0] = ct->table[index];
        result[1] = ct->table[index+1];
        result[2] = ct->table[index+2];
        result[3] = ct->table[index+3];
        break;
    case GL_ALPHA:
        result[0] = ct->table[index];
        break;
    case GL_INTENSITY:
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        result[3] = result[0];
        break;
    case 0:
        result[0] = index;
        break;
    default:
        assert(0);
    }
}


/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glFRExtractTexelCI16(__GLmipMapLevel *level, __GLtexture *tex,
                            GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image;
    GLuint index;
    __GLcolorTable *ct;

    ct = &tex->CT;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
        (col >= level->width2)) {
        /*
        ** Use border color when the texture supplies no border.
        */
        index = __GL_FLOAT_TO_US(tex->params.borderColor.r);
    } else {
        image = ((GLushort*)level->buffer) + ((row << level->widthLog2) + col);
        index = image[0];
    }

    switch (ct->baseFormat) {
    case GL_LUMINANCE:
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        break;
    case GL_LUMINANCE_ALPHA:
        index <<= 1;
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        result[3] = ct->table[index+1];
        break;
    case GL_RGB:
        index += index<<1;
        result[0] = ct->table[index];
        result[1] = ct->table[index+1];
        result[2] = ct->table[index+2];
        break;
    case GL_RGBA:
        index <<= 2;
        result[0] = ct->table[index];
        result[1] = ct->table[index+1];
        result[2] = ct->table[index+2];
        result[3] = ct->table[index+3];
        break;
    case GL_ALPHA:
        result[0] = ct->table[index];
        break;
    case GL_INTENSITY:
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        result[3] = result[0];
        break;
    case 0:
        /* Warning: this will truncate to GLubyte */
        result[0] = index;
        break;
    default:
        assert(0);
    }
}

/*
** Get a texture element out of the one component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glFRExtractTexelL_B(__GLmipMapLevel *level, __GLtexture *tex,
                           GLint row, GLint col, GLubyte *result)
{
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result[0] = image[0];
    result[1] = image[0];
    result[2] = image[0];
}

/*
** Get a texture element out of the two component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glFRExtractTexelLA_B(__GLmipMapLevel *level, __GLtexture *tex,
                            GLint row, GLint col, GLubyte *result)
{
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + ((row * level->width + col) << 1);
    result[0] = image[0];
    result[1] = image[0];
    result[2] = image[0];
    result[3] = image[1];
}

/*
** Get a texture element out of the three component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glFRExtractTexelRGB_B(__GLmipMapLevel *level, __GLtexture *tex,
                             GLint row, GLint col, GLubyte *result)
{
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 3;
    result[0] = image[0];
    result[1] = image[1];
    result[2] = image[2];
}

/*
** Get a texture element out of the four component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glFRExtractTexelRGBA_B(__GLmipMapLevel *level, __GLtexture *tex,
                              GLint row, GLint col, GLubyte *result)
{
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + ((row * level->width + col) << 2);
    result[0] = image[0];
    result[1] = image[1];
    result[2] = image[2];
    result[3] = image[3];
}

/* ARGSUSED */
void __glFRExtractTexelA_B(__GLmipMapLevel *level, __GLtexture *tex,
                           GLint row, GLint col, GLubyte *result)
{
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result[0] = image[0];
}

/* ARGSUSED */
void __glFRExtractTexelI_B(__GLmipMapLevel *level, __GLtexture *tex,
                           GLint row, GLint col, GLubyte *result)
{
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    result[0] = image[0];
    result[1] = image[0];
    result[2] = image[0];
    result[3] = image[0];
}

#if 0
/* ARGSUSED */
void __glFRExtractTexelRGB1555_B(__GLmipMapLevel *level, __GLtexture *tex,
                                 GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result[0] = (__GLfloat) ((texel >> 10) & 0x1F) / 31.0F;
    result[1] = (__GLfloat) ((texel >>  5) & 0x1F) / 31.0F;
    result[2] = (__GLfloat) ((texel      ) & 0x1F) / 31.0F;
}

/* ARGSUSED */
void __glFRExtractTexelRGB565_B(__GLmipMapLevel *level, __GLtexture *tex,
                                GLint row, GLint col, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result[0] = (__GLfloat) ((texel >> 11) & 0x1F) / 31.0F;
    result[1] = (__GLfloat) ((texel >>  5) & 0x3F) / 63.0F;
    result[2] = (__GLfloat) ((texel      ) & 0x1F) / 31.0F;
}
#endif

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glFRExtractTexelCI8_B(__GLmipMapLevel *level, __GLtexture *tex,
                             GLint row, GLint col, GLubyte *result)
{
    GLubyte *image;
    GLuint index;
    __GLcolorTable *ct;

    ct = &tex->CT;

    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col);
    index = image[0];

    switch (ct->baseFormat) {
    case GL_LUMINANCE:
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        break;
    case GL_LUMINANCE_ALPHA:
        index <<= 1;
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        result[3] = ct->table[index+1];
        break;
    case GL_RGB:
        index += index<<1;
        result[0] = ct->table[index];
        result[1] = ct->table[index+1];
        result[2] = ct->table[index+2];
        break;
    case GL_RGBA:
        index <<= 2;
        result[0] = ct->table[index];
        result[1] = ct->table[index+1];
        result[2] = ct->table[index+2];
        result[3] = ct->table[index+3];
        break;
    case GL_ALPHA:
        result[0] = ct->table[index];
        break;
    case GL_INTENSITY:
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        result[3] = result[0];
        break;
    case 0:
        result[0] = index;
        break;
    default:
        assert(0);
    }
}

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glFRExtractTexelCI16_B(__GLmipMapLevel *level, __GLtexture *tex,
                              GLint row, GLint col, GLubyte *result)
{
    GLushort *image;
    GLuint index;
    __GLcolorTable *ct;

    ct = &tex->CT;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    index = image[0];

    switch (ct->baseFormat) {
    case GL_LUMINANCE:
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        break;
    case GL_LUMINANCE_ALPHA:
        index <<= 1;
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        result[3] = ct->table[index+1];
        break;
    case GL_RGB:
        index += index<<1;
        result[0] = ct->table[index];
        result[1] = ct->table[index+1];
        result[2] = ct->table[index+2];
        break;
    case GL_RGBA:
        index <<= 2;
        result[0] = ct->table[index];
        result[1] = ct->table[index+1];
        result[2] = ct->table[index+2];
        result[3] = ct->table[index+3];
        break;
    case GL_ALPHA:
        result[0] = ct->table[index];
        break;
    case GL_INTENSITY:
        result[0] = ct->table[index];
        result[1] = result[0];
        result[2] = result[0];
        result[3] = result[0];
        break;
    case 0:
        /* Warning: this will truncate to GLubyte */
        result[0] = index;
        break;
    default:
        assert(0);
    }
}

/**************************************************************************
 * Filter procs
 **************************************************************************/

/* When these are called directly for non-mipmapped cases, lod is zero.
 * When called from a mipmap filter, lod is an integer.
 */

void __glFRNearestFilterUVScaled(__GLtexture *tex, GLuint lod,
                                 GLint fs, GLint ft, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp = tex->level + lod;
    GLint row = TruncFixed(ft, STRQ_FRAC_BITS);
    GLint col = TruncFixed(fs, STRQ_FRAC_BITS);
    GLint w2 = lp->width2;
    GLint h2 = lp->height2;

    /* Find texel column address */
    if (tex->params.sWrapMode == GL_REPEAT) {
        col &= w2-1;
    } else {
        if (col < 0) col = 0;
        else if (col >= w2) col = w2 - 1;
    }

    /* Find texel row address */
    if (tex->params.tWrapMode == GL_REPEAT) {
        row &= h2-1;
    } else {
        if (row < 0) row = 0;
        else if (row >= h2) row = h2 - 1;
    }

    /* Lookup texel */
    (*gc->procs.slowExtractTexel)(lp, tex, row, col, result);
}


void __glFRLinearFilterUVScaled(__GLtexture *tex, GLuint lod,
                                GLint fs, GLint ft, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp = tex->level + lod;
    GLuint half, alpha, beta;
    GLuint col0, row0, col1, row1;
    GLint w2mask, h2mask;
    GLuint omalpha, ombeta, m00, m01, m10, m11;
    GLubyte t00[4], t01[4], t10[4], t11[4];
    GLuint r, g, b, a;

    half = STRQ_HALF;

    /* Find col0, col1 */
    if (tex->params.sWrapMode == GL_REPEAT) {
        w2mask = lp->width2 - 1;
        fs -= half;
        col0 = TruncFixed(fs, STRQ_FRAC_BITS) & w2mask;
        col1 = (col0 + 1) & w2mask;
    } else {
        if (fs < 0)
            fs = 0;
        else if (fs > (lp->width2<<STRQ_FRAC_BITS))
            fs = (lp->width2<<STRQ_FRAC_BITS);
        fs -= half;
        col0 = TruncFixed(fs, STRQ_FRAC_BITS);
        col1 = col0 + 1;
    }

    /* Find row0, row1 */
    if (tex->params.tWrapMode == GL_REPEAT) {
        h2mask = lp->height2 - 1;
        ft -= half;
        row0 = TruncFixed(ft, STRQ_FRAC_BITS) & h2mask;
        row1 = (row0 + 1) & h2mask;
    } else {
        if (ft < 0)
            ft = 0;
        else if (ft > (lp->height2<<STRQ_FRAC_BITS))
            ft = (lp->height2<<STRQ_FRAC_BITS);
        ft -= half;
        row0 = TruncFixed(ft, STRQ_FRAC_BITS);
        row1 = row0 + 1;
    }

    /* Compute alpha and beta */
    alpha = ((fs & 0xffff) >> 8);
    beta  = ((ft & 0xffff) >> 8);

    /* Calculate the final texel value as a combination of the square chosen */
    (*gc->procs.slowExtractTexel)(lp, tex, row0, col0, t00);
    (*gc->procs.slowExtractTexel)(lp, tex, row0, col1, t10);
    (*gc->procs.slowExtractTexel)(lp, tex, row1, col0, t01);
    (*gc->procs.slowExtractTexel)(lp, tex, row1, col1, t11);

    omalpha = 255 - alpha;
    ombeta  = 255 - beta;

    m00 = BLEND(omalpha, ombeta);
    m01 = BLEND(omalpha, beta);
    m10 = BLEND(alpha, ombeta);
    m11 = BLEND(alpha, beta);

    /* Why the interleaving?  I'm trying to minimize cache thrashing. */
    a = BLEND(m00,t00[3]);
    r = BLEND(m00,t00[0]);
    g = BLEND(m00,t00[1]);
    b = BLEND(m00,t00[2]);

    a += BLEND(m01,t01[3]);
    r += BLEND(m01,t01[0]);
    g += BLEND(m01,t01[1]);
    b += BLEND(m01,t01[2]);

    a += BLEND(m10,t10[3]);
    r += BLEND(m10,t10[0]);
    g += BLEND(m10,t10[1]);
    b += BLEND(m10,t10[2]);

    a += BLEND(m11,t11[3]);
    r += BLEND(m11,t11[0]);
    g += BLEND(m11,t11[1]);
    b += BLEND(m11,t11[2]);

    result[0] = r;
    result[1] = g;
    result[2] = b;
    result[3] = a;
}

/* Magnification filters: lod is always zero */

void __glFRNearestFilter(__GLtexture *tex, GLuint lod,
                         GLint fs, GLint ft, GLubyte *result)
{
    __GLmipMapLevel *lp = tex->level;

    assert(0 == lod);

    fs <<= lp->widthLog2;
    ft <<= lp->heightLog2;
    __glFRNearestFilterUVScaled(tex, 0, fs, ft, result);
}

void __glFRLinearFilter(__GLtexture *tex, GLuint lod,
                        GLint fs, GLint ft, GLubyte *result)
{
    __GLmipMapLevel *lp = tex->level;

    assert(0 == lod);

    fs <<= lp->widthLog2;
    ft <<= lp->heightLog2;
    __glFRLinearFilterUVScaled(tex, 0, fs, ft, result);
}

/* Minification filters: lod is fixed point */

void __glFR_NMNFilter(__GLtexture *tex, GLuint lod,
                      GLint fs, GLint ft, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLuint d, p;

    d = TruncFixed(lod + STRQ_HALF, STRQ_FRAC_BITS);
    p = (GLuint) tex->p;
    if (d > p) {
        d = p;
    }
    lp = &tex->level[d];
    fs <<= lp->widthLog2;
    ft <<= lp->heightLog2;
    __glFRNearestFilterUVScaled(tex, d, fs, ft, result);
}

/* NOTE: lod is fixed point */
void __glFR_LMNFilter(__GLtexture *tex, GLuint lod,
                      GLint fs, GLint ft, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLuint d, p;

    d = TruncFixed(lod + STRQ_HALF, STRQ_FRAC_BITS);
    p = (GLuint) tex->p;
    if (d > p) {
        d = p;
    }
    lp = &tex->level[d];
    fs <<= lp->widthLog2;
    ft <<= lp->heightLog2;
    __glFRLinearFilterUVScaled(tex, d, fs, ft, result);
}


void __glFR_NMLFilter(__GLtexture *tex, GLuint lod,
                      GLint fs, GLint ft, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLuint p, d, f;
    GLubyte td[4], td1[4];

    d = TruncFixed(lod + STRQ_ONE, STRQ_FRAC_BITS);
    p = (GLuint) tex->p;
    if (d > p) {
        /* Clamp d to last available mipmap */
        lp = &tex->level[p];
        fs <<= lp->widthLog2;
        ft <<= lp->heightLog2;
        __glFRNearestFilterUVScaled(tex, p, fs, ft, result);
    } else {
        GLint s1, t1;

        lp = &tex->level[d];
        s1 = fs << lp->widthLog2;
        t1 = ft << lp->heightLog2;
        __glFRNearestFilterUVScaled(tex, d, s1, t1, td);

        lp = &tex->level[d-1];
        s1 = fs << lp->widthLog2;
        t1 = ft << lp->heightLog2;
        __glFRNearestFilterUVScaled(tex, d-1, s1, t1, td1);

        f = (lod & (STRQ_ONE-1)) >> (STRQ_FRAC_BITS-8);

        result[0] = SA_MSA(td[0], f, td1[0]);
        result[1] = SA_MSA(td[1], f, td1[1]);
        result[2] = SA_MSA(td[2], f, td1[2]);
        result[3] = SA_MSA(td[3], f, td1[3]);
    }
}

void __glFR_LMLFilter(__GLtexture *tex, GLuint lod,
                      GLint fs, GLint ft, GLubyte *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLuint p, d, f;
    GLubyte td[4], td1[4];

    d = TruncFixed(lod + STRQ_ONE, STRQ_FRAC_BITS);
    p = (GLuint) tex->p;
    if (d > p) {
        /* Clamp d to last available mipmap */
        lp = &tex->level[p];
        fs <<= lp->widthLog2;
        ft <<= lp->heightLog2;
        __glFRLinearFilterUVScaled(tex, p, fs, ft, result);
    } else {
        GLint s1, t1;

        lp = &tex->level[d];
        s1 = fs << lp->widthLog2;
        t1 = ft << lp->heightLog2;
        __glFRLinearFilterUVScaled(tex, d, s1, t1, td);

        lp = &tex->level[d-1];
        s1 = fs << lp->widthLog2;
        t1 = ft << lp->heightLog2;
        __glFRLinearFilterUVScaled(tex, d-1, s1, t1, td1);

        f = (lod & (STRQ_ONE-1)) >> (STRQ_FRAC_BITS-8);

        result[0] = SA_MSA(td[0], f, td1[0]);
        result[1] = SA_MSA(td[1], f, td1[1]);
        result[2] = SA_MSA(td[2], f, td1[2]);
        result[3] = SA_MSA(td[3], f, td1[3]);
    }
}

/**************************************************************************
 * Spanlet procs
 **************************************************************************/

void __fastcall __glGenericExtractTexels_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    GLint s, t;

    s = tr->s;
    t = tr->t;

    while (1) {

        while (((int)mask) < 0) {
            (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);

            texelBuf++;

            s += tr->dsdx;
            t += tr->dtdx;
            mask <<= 1;
        }

        if (mask == 0) break;

        s += tr->dsdx;
        t += tr->dtdx;
        mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexels_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    GLint s, t;

    s = tr->s;
    t = tr->t;

    while (1) {

        while (((int)mask) < 0) {
            (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);

            texelBuf += 3;

            s += tr->dsdx;
            t += tr->dtdx;
            mask <<= 1;
        }

        if (mask == 0) break;

        s += tr->dsdx;
        t += tr->dtdx;
        mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexels_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    GLint s, t;

    s = tr->s;
    t = tr->t;

    while (1) {

        while (((int)mask) < 0) {
            (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);

            texelBuf += 4;

            s += tr->dsdx;
            t += tr->dtdx;
            mask <<= 1;
        }

        if (mask == 0) break;

        s += tr->dsdx;
        t += tr->dtdx;
        mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsPC_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int s, t;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float invqw = 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float next_s, next_t, ds, dt;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

        qw += tr->fdqwdxPWL;
#ifdef PWL_OVERLAP
        /* begin the next FDIV so that it overlaps */
        __asm fld  qw
        __asm fld1
        __asm fdivrp st(1), st
#else
        invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
        sw += tr->fdswdxPWL;
        tw += tr->fdtwdxPWL;

#ifdef PWL_OVERLAP
        /* fetch invqw from FPU */
        __asm fstp invqw
#endif /* PWL_OVERLAP */

        /* compute next texture parameters */
        next_s = sw * invqw;
        next_t = tw * invqw;

        ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

        tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
        tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);

        s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
        t = FloatToFixed(prev_t, STRQ_FRAC_BITS);

        while (1) {
            while (((int)mask) < 0) {
                (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);

                texelBuf++;

                s += tr->dsdx;
                t += tr->dtdx;

                mask <<= 1;
            }

            if (mask == 0) break;

            s += tr->dsdx;
            t += tr->dtdx;

            mask <<= 1;
        }

        if (mask2 == 0) break;

        mask = mask2 << 16;
        mask2 = 0;

        prev_s = next_s;
        prev_t = next_t;
    }
}

void __fastcall __glGenericExtractTexelsPC_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int s, t;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float invqw = 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float next_s, next_t, ds, dt;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

        qw += tr->fdqwdxPWL;
#ifdef PWL_OVERLAP
        /* begin the next FDIV so that it overlaps */
        __asm fld  qw
        __asm fld1
        __asm fdivrp st(1), st
#else
        invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
        sw += tr->fdswdxPWL;
        tw += tr->fdtwdxPWL;

#ifdef PWL_OVERLAP
        /* fetch invqw from FPU */
        __asm fstp invqw
#endif /* PWL_OVERLAP */

        /* compute next texture parameters */
        next_s = sw * invqw;
        next_t = tw * invqw;

        ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

        tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
        tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);

        s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
        t = FloatToFixed(prev_t, STRQ_FRAC_BITS);

        while (1) {
            while (((int)mask) < 0) {
                (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);

                texelBuf += 3;

                s += tr->dsdx;
                t += tr->dtdx;

                mask <<= 1;
            }

            if (mask == 0) break;

            s += tr->dsdx;
            t += tr->dtdx;

            mask <<= 1;
        }

        if (mask2 == 0) break;

        mask = mask2 << 16;
        mask2 = 0;

        prev_s = next_s;
        prev_t = next_t;
    }
}

void __fastcall __glGenericExtractTexelsPC_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int s, t;

    float qw = tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float invqw = 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float next_s, next_t, ds, dt;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

        qw += tr->fdqwdxPWL;
#ifdef PWL_OVERLAP
        /* begin the next FDIV so that it overlaps */
        __asm fld  qw
        __asm fld1
        __asm fdivrp st(1), st
#else
        invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
        sw += tr->fdswdxPWL;
        tw += tr->fdtwdxPWL;

#ifdef PWL_OVERLAP
        /* fetch invqw from FPU */
        __asm fstp invqw
#endif /* PWL_OVERLAP */

        /* compute next texture parameters */
        next_s = sw * invqw;
        next_t = tw * invqw;

        ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

        tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
        tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);

        s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
        t = FloatToFixed(prev_t, STRQ_FRAC_BITS);

        while (1) {
            while (((int)mask) < 0) {
                (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);

                texelBuf += 4;

                s += tr->dsdx;
                t += tr->dtdx;

                mask <<= 1;
            }

            if (mask == 0) break;

            s += tr->dsdx;
            t += tr->dtdx;

            mask <<= 1;
        }

        if (mask2 == 0) break;

        mask = mask2 << 16;
        mask2 = 0;

        prev_s = next_s;
        prev_t = next_t;
    }
}

/**************************************************************************
 * Rho-interpolant procs
 **************************************************************************/

#if AFFINE_RHO

void __fastcall __glGenericExtractTexelsRho_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

        while (((int)mask) < 0) {
            if (rho <= tex->c) {
                (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
            } else {
                (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);
            }

            texelBuf += 1;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;
            mask <<= 1;
        }

        if (mask == 0) break;

        s += tr->dsdx;
        t += tr->dtdx;
        rho += tr->drhodx;
        mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsRho_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

        while (((int)mask) < 0) {
            if (rho <= tex->c) {
                (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
            } else {
                (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);
            }

            texelBuf += 3;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;
            mask <<= 1;
        }

        if (mask == 0) break;

        s += tr->dsdx;
        t += tr->dtdx;
        rho += tr->drhodx;
        mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsRho_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

        while (((int)mask) < 0) {
            if (rho <= tex->c) {
                (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
            } else {
                (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);
            }

            texelBuf += 4;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;
            mask <<= 1;
        }

        if (mask == 0) break;

        s += tr->dsdx;
        t += tr->dtdx;
        rho += tr->drhodx;
        mask <<= 1;
    }
}

#endif /* AFFINE_RHO */

void __fastcall __glGenericExtractTexelsPCRho_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int s, t;
    float rho;

    float qw = ExtractBits(tr->fqw) < 0 ? MIN_FLOAT : tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw = 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

        qw += tr->fdqwdxPWL;
        if (ExtractBits(qw) <= 0)
            qw = MIN_FLOAT;
#ifdef PWL_OVERLAP
        /* begin the next FDIV so that it overlaps */
        __asm fld  qw
        __asm fld1
        __asm fdivrp st(1), st
#else
        invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
        sw += tr->fdswdxPWL;
        tw += tr->fdtwdxPWL;
        rhow += tr->fdrhowdxPWL;

#ifdef PWL_OVERLAP
        /* fetch invqw from FPU */
        __asm fstp invqw
#endif /* PWL_OVERLAP */

        /* compute next texture parameters */
        next_s = sw * invqw;
        next_t = tw * invqw;
        next_rho = rhow * invqw;

        ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

        tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
        tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
        tr->drhodx = drho;

        s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
        t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
        rho = prev_rho;

        while (1) {
            while (((int)mask) < 0) {
                if (rho <= tex->c) {
                    (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
                } else {
                    (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);
                }

                texelBuf += 1;

                s += tr->dsdx;
                t += tr->dtdx;
                rho += tr->drhodx;

                mask <<= 1;
            }

            if (mask == 0) break;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;

            mask <<= 1;
        }

        if (mask2 == 0) break;

        mask = mask2 << 16;
        mask2 = 0;

        prev_s = next_s;
        prev_t = next_t;
        prev_rho = next_rho;
    }
}

void __fastcall __glGenericExtractTexelsPCRho_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int s, t;
    float rho;

    float qw = ExtractBits(tr->fqw) < 0 ? MIN_FLOAT : tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw = 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

        qw += tr->fdqwdxPWL;
        if (ExtractBits(qw) <= 0)
            qw = MIN_FLOAT;
#ifdef PWL_OVERLAP
        /* begin the next FDIV so that it overlaps */
        __asm fld  qw
        __asm fld1
        __asm fdivrp st(1), st
#else
        invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
        sw += tr->fdswdxPWL;
        tw += tr->fdtwdxPWL;
        rhow += tr->fdrhowdxPWL;

#ifdef PWL_OVERLAP
        /* fetch invqw from FPU */
        __asm fstp invqw
#endif /* PWL_OVERLAP */

        /* compute next texture parameters */
        next_s = sw * invqw;
        next_t = tw * invqw;
        next_rho = rhow * invqw;

        ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

        tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
        tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
        tr->drhodx = drho;

        s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
        t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
        rho = prev_rho;

        while (1) {
            while (((int)mask) < 0) {
                if (rho <= tex->c) {
                    (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
                } else {
                    (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);
                }

                texelBuf += 3;

                s += tr->dsdx;
                t += tr->dtdx;
                rho += tr->drhodx;

                mask <<= 1;
            }

            if (mask == 0) break;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;

            mask <<= 1;
        }

        if (mask2 == 0) break;

        mask = mask2 << 16;
        mask2 = 0;

        prev_s = next_s;
        prev_t = next_t;
        prev_rho = next_rho;
    }
}

void __fastcall __glGenericExtractTexelsPCRho_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int s, t;
    float rho;

    float qw = ExtractBits(tr->fqw) < 0 ? MIN_FLOAT : tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw = 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

        qw += tr->fdqwdxPWL;
        if (ExtractBits(qw) <= 0)
            qw = MIN_FLOAT;
#ifdef PWL_OVERLAP
        /* begin the next FDIV so that it overlaps */
        __asm fld  qw
        __asm fld1
        __asm fdivrp st(1), st
#else
        invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
        sw += tr->fdswdxPWL;
        tw += tr->fdtwdxPWL;
        rhow += tr->fdrhowdxPWL;

#ifdef PWL_OVERLAP
        /* fetch invqw from FPU */
        __asm fstp invqw
#endif /* PWL_OVERLAP */

        /* compute next texture parameters */
        next_s = sw * invqw;
        next_t = tw * invqw;
        next_rho = rhow * invqw;

        ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

        tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
        tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
        tr->drhodx = drho;

        s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
        t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
        rho = prev_rho;

        while (1) {
            while (((int)mask) < 0) {
                if (rho <= tex->c) {
                    (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
                } else {
                    (*gc->procs.slowMinFilter)(tex, 0, s, t, texelBuf);
                }

                texelBuf += 4;

                s += tr->dsdx;
                t += tr->dtdx;
                rho += tr->drhodx;

                mask <<= 1;
            }

            if (mask == 0) break;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;

            mask <<= 1;
        }

        if (mask2 == 0) break;

        mask = mask2 << 16;
        mask2 = 0;

        prev_s = next_s;
        prev_t = next_t;
        prev_rho = next_rho;
    }
}

/**************************************************************************
 * Mipmap procs
 **************************************************************************/

#if AFFINE_RHO

void __fastcall __glGenericExtractTexelsMipmap_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

        while (((int)mask) < 0) {
            if (rho <= tex->c) {
                (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
            } else {
                GLuint lod = ExtractLOD(rho);
                (*gc->procs.slowMinFilter)(tex, lod, s, t, texelBuf);
            }

            texelBuf += 1;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;
            mask <<= 1;
        }

        if (mask == 0) break;

        s += tr->dsdx;
        t += tr->dtdx;
        rho += tr->drhodx;
        mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsMipmap_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

        while (((int)mask) < 0) {
            if (rho <= tex->c) {
                (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
            } else {
                GLuint lod = ExtractLOD(rho);
                (*gc->procs.slowMinFilter)(tex, lod, s, t, texelBuf);
            }

            texelBuf += 3;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;
            mask <<= 1;
        }

        if (mask == 0) break;

        s += tr->dsdx;
        t += tr->dtdx;
        rho += tr->drhodx;
        mask <<= 1;
    }
}

void __fastcall __glGenericExtractTexelsMipmap_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture;
    int s, t;
    float rho;

    s = tr->s;
    t = tr->t;
    rho = tr->rho;

    while (1) {

        while (((int)mask) < 0) {
            if (rho <= tex->c) {
                (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
            } else {
                GLuint lod = ExtractLOD(rho);
                (*gc->procs.slowMinFilter)(tex, lod, s, t, texelBuf);
            }

            texelBuf += 4;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;
            mask <<= 1;
        }

        if (mask == 0) break;

        s += tr->dsdx;
        t += tr->dtdx;
        rho += tr->drhodx;
        mask <<= 1;
    }
}

#endif /* AFFINE_RHO */

void __fastcall
__glGenericExtractTexelsPCMipmap_1(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int s, t;
    float rho;

    float qw = ExtractBits(tr->fqw) < 0 ? MIN_FLOAT : tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw = 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

        qw += tr->fdqwdxPWL;
        if (ExtractBits(qw) <= 0)
            qw = MIN_FLOAT;
#ifdef PWL_OVERLAP
        /* begin the next FDIV so that it overlaps */
        __asm fld  qw
        __asm fld1
        __asm fdivrp st(1), st
#else
        invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
        sw += tr->fdswdxPWL;
        tw += tr->fdtwdxPWL;
        rhow += tr->fdrhowdxPWL;

#ifdef PWL_OVERLAP
        /* fetch invqw from FPU */
        __asm fstp invqw
#endif /* PWL_OVERLAP */

        /* compute next texture parameters */
        next_s = sw * invqw;
        next_t = tw * invqw;
        next_rho = rhow * invqw;

        ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

        tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
        tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
        tr->drhodx = drho;

        s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
        t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
        rho = prev_rho;

        while (1) {
            while (((int)mask) < 0) {
                if (rho <= tex->c) {
                    (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
                } else {
                    GLuint lod = ExtractLOD(rho);
                    (*gc->procs.slowMinFilter)(tex, lod, s, t, texelBuf);
                }

                texelBuf += 1;

                s += tr->dsdx;
                t += tr->dtdx;
                rho += tr->drhodx;

                mask <<= 1;
            }

            if (mask == 0) break;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;

            mask <<= 1;
        }

        if (mask2 == 0) break;

        mask = mask2 << 16;
        mask2 = 0;

        prev_s = next_s;
        prev_t = next_t;
        prev_rho = next_rho;
    }
}

void __fastcall
__glGenericExtractTexelsPCMipmap_3(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int s, t;
    float rho;

    float qw = ExtractBits(tr->fqw) < 0 ? MIN_FLOAT : tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw = 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

        qw += tr->fdqwdxPWL;
        if (ExtractBits(qw) <= 0)
            qw = MIN_FLOAT;
#ifdef PWL_OVERLAP
        /* begin the next FDIV so that it overlaps */
        __asm fld  qw
        __asm fld1
        __asm fdivrp st(1), st
#else
        invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
        sw += tr->fdswdxPWL;
        tw += tr->fdtwdxPWL;
        rhow += tr->fdrhowdxPWL;

#ifdef PWL_OVERLAP
        /* fetch invqw from FPU */
        __asm fstp invqw
#endif /* PWL_OVERLAP */

        /* compute next texture parameters */
        next_s = sw * invqw;
        next_t = tw * invqw;
        next_rho = rhow * invqw;

        ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

        tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
        tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
        tr->drhodx = drho;

        s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
        t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
        rho = prev_rho;

        while (1) {
            while (((int)mask) < 0) {
                if (rho <= tex->c) {
                    (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
                } else {
                    GLuint lod = ExtractLOD(rho);
                    (*gc->procs.slowMinFilter)(tex, lod, s, t, texelBuf);
                }

                texelBuf += 3;

                s += tr->dsdx;
                t += tr->dtdx;
                rho += tr->drhodx;

                mask <<= 1;
            }

            if (mask == 0) break;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;

            mask <<= 1;
        }

        if (mask2 == 0) break;

        mask = mask2 << 16;
        mask2 = 0;

        prev_s = next_s;
        prev_t = next_t;
        prev_rho = next_rho;
    }
}

void __fastcall
__glGenericExtractTexelsPCMipmap_4(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *texelBuf = (GLubyte *) tr->texelBuf;
    __GLtexture *tex = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int s, t;
    float rho;

    float qw = ExtractBits(tr->fqw) < 0 ? MIN_FLOAT : tr->fqw;
    float sw = tr->fsw;
    float tw = tr->ftw;
    float rhow = tr->frhow;
    float invqw = 1.0F / qw;
    float prev_s = sw * invqw;
    float prev_t = tw * invqw;
    float prev_rho = rhow * invqw;
    float next_s, next_t, next_rho, ds, dt, drho;
    double dtmp;
    GLbitfield mask2;


    mask2 = mask & 0x0000ffff; /* Save bottom half for second sub-span */
    mask  = mask & 0xffff0000; /* Clear bottom half for first sub-span */

    while (1) {

        qw += tr->fdqwdxPWL;
        if (ExtractBits(qw) <= 0)
            qw = MIN_FLOAT;
#ifdef PWL_OVERLAP
        /* begin the next FDIV so that it overlaps */
        __asm fld  qw
        __asm fld1
        __asm fdivrp st(1), st
#else
        invqw = 1.0F / qw;
#endif /* PWL_OVERLAP */
        sw += tr->fdswdxPWL;
        tw += tr->fdtwdxPWL;
        rhow += tr->fdrhowdxPWL;

#ifdef PWL_OVERLAP
        /* fetch invqw from FPU */
        __asm fstp invqw
#endif /* PWL_OVERLAP */

        /* compute next texture parameters */
        next_s = sw * invqw;
        next_t = tw * invqw;
        next_rho = rhow * invqw;

        ds = (next_s - prev_s) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        dt = (next_t - prev_t) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);
        drho = (next_rho - prev_rho) * (1.0F / __FR_TEXEL_PWL_SPAN_WIDTH);

        tr->dsdx = FloatToFixed(ds, STRQ_FRAC_BITS);
        tr->dtdx = FloatToFixed(dt, STRQ_FRAC_BITS);
        tr->drhodx = drho;

        s = FloatToFixed(prev_s, STRQ_FRAC_BITS);
        t = FloatToFixed(prev_t, STRQ_FRAC_BITS);
        rho = prev_rho;

        while (1) {
            while (((int)mask) < 0) {
                if (rho <= tex->c) {
                    (*gc->procs.slowMagFilter)(tex, 0, s, t, texelBuf);
                } else {
                    GLuint lod = ExtractLOD(rho);
                    (*gc->procs.slowMinFilter)(tex, lod, s, t, texelBuf);
                }

                texelBuf += 4;

                s += tr->dsdx;
                t += tr->dtdx;
                rho += tr->drhodx;

                mask <<= 1;
            }

            if (mask == 0) break;

            s += tr->dsdx;
            t += tr->dtdx;
            rho += tr->drhodx;

            mask <<= 1;
        }

        if (mask2 == 0) break;

        mask = mask2 << 16;
        mask2 = 0;

        prev_s = next_s;
        prev_t = next_t;
        prev_rho = next_rho;
    }
}

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
#ifdef __GL_PC_RAST

#include "context.h"
#include "render.h"
#include "global.h"
#include "image.h"
#include "fr_modes.h"
#include "fr_tri.h"
#include "g_rgbtri.h"
#include "g_citri.h"
#include "g_rgbpix.h"
#include "g_cipix.h"
#include "g_rgbclr.h"
#include "g_ciclr.h"
#include "g_ztest.h"
#include "g_texel.h"
#include "g_rgbcmb.h"
#include "g_cicmb.h"
#include "fr_fbtype.h"
#include "fr_tex.h"
#include "fr_stncl.h"
#include "fr_store.h"
#include "imports.h" /* for __GL_POWF */

#ifdef __GL_CODEGEN
#include "fr_og.h"
#include "ras_og.h"
#endif

/* For now, we force perspective correction if needRho */
#define AFFINE_RHO 0

extern void  __glFRRenderTriangle(__GLcontext *gc, __GLvertex *v0,
                                  __GLvertex *v1, __GLvertex *v2);


static __GLspanlet *__glGenPixTab[] = {
    0,
    0,
    __fr_rgb_pix_2, /* RGB332 */
    0,
    __fr_rgb_pix_4, /* RGB5 */
    __fr_rgb_pix_5, /* RGB565 */
    0,
    0,
    0,
    0,
    __fr_rgb_pix_a, /* XRGB8 */
    0
};

static __GLspanlet *__glGenColorTab[] = {
    0,
    0,
    __fr_rgb_genclr_2, /* RGB332 */
    0,
    __fr_rgb_genclr_4, /* RGB5 */
    __fr_rgb_genclr_5, /* RGB565 */
    0,
    __fr_rgb_genclr_a, /* RGB8 */
    0,
    0,
    __fr_rgb_genclr_a, /* XRGB8 */
    0
};

static __GLspanlet *__glCombineTab[] = {
    0,
    0,
    __fr_rgb_combine_2, /* RGB332 */
    0,
    __fr_rgb_combine_4, /* RGB5 */
    __fr_rgb_combine_5, /* RGB565 */
    0,
    0,
    0,
    0,
    __fr_rgb_combine_a, /* XRGB8 */
    0
};

#define __GL_ENVVARS 1

#ifdef __GL_ENVVARS
/* The following allows a runtime switch to disable the fast path */
static GLboolean disable_fastpath = GL_FALSE;
static GLboolean check_env = GL_TRUE;
static GLboolean null_triangle = GL_FALSE;
static GLboolean null_spanlets = GL_FALSE;
static GLboolean generic_store = GL_FALSE;
#if __GL_SST
static GLboolean disable_og = GL_TRUE;
static GLboolean generic_setup = GL_TRUE;
#else
static GLboolean disable_og = GL_FALSE;
static GLboolean generic_setup = GL_FALSE;
#endif

void __glNullRenderTriangle(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1,
                            __GLvertex *v2)
{
}
#endif

const unsigned int __glFRMaskTable[33] = {
    0x00000000, 0x80000000, 0xc0000000, 0xe0000000,
    0xf0000000, 0xf8000000, 0xfc000000, 0xfe000000,
    0xff000000, 0xff800000, 0xffc00000, 0xffe00000,
    0xfff00000, 0xfff80000, 0xfffc0000, 0xfffe0000,
    0xffff0000, 0xffff8000, 0xffffc000, 0xffffe000,
    0xfffff000, 0xfffff800, 0xfffffc00, 0xfffffe00,
    0xffffff00, 0xffffff80, 0xffffffc0, 0xffffffe0,
    0xfffffff0, 0xfffffff8, 0xfffffffc, 0xfffffffe,
    0xffffffff
};

const int __glFRDitherTable[4][35] = {
    {
         0<<DITHER_SHIFT,  8<<DITHER_SHIFT,  2<<DITHER_SHIFT, 10<<DITHER_SHIFT,
         0<<DITHER_SHIFT,  8<<DITHER_SHIFT,  2<<DITHER_SHIFT, 10<<DITHER_SHIFT,
         0<<DITHER_SHIFT,  8<<DITHER_SHIFT,  2<<DITHER_SHIFT, 10<<DITHER_SHIFT,
         0<<DITHER_SHIFT,  8<<DITHER_SHIFT,  2<<DITHER_SHIFT, 10<<DITHER_SHIFT,
         0<<DITHER_SHIFT,  8<<DITHER_SHIFT,  2<<DITHER_SHIFT, 10<<DITHER_SHIFT,
         0<<DITHER_SHIFT,  8<<DITHER_SHIFT,  2<<DITHER_SHIFT, 10<<DITHER_SHIFT,
         0<<DITHER_SHIFT,  8<<DITHER_SHIFT,  2<<DITHER_SHIFT, 10<<DITHER_SHIFT,
         0<<DITHER_SHIFT,  8<<DITHER_SHIFT,  2<<DITHER_SHIFT, 10<<DITHER_SHIFT,
         0<<DITHER_SHIFT,  8<<DITHER_SHIFT,  2<<DITHER_SHIFT
    },
    {
        12<<DITHER_SHIFT,  4<<DITHER_SHIFT, 14<<DITHER_SHIFT,  6<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  4<<DITHER_SHIFT, 14<<DITHER_SHIFT,  6<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  4<<DITHER_SHIFT, 14<<DITHER_SHIFT,  6<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  4<<DITHER_SHIFT, 14<<DITHER_SHIFT,  6<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  4<<DITHER_SHIFT, 14<<DITHER_SHIFT,  6<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  4<<DITHER_SHIFT, 14<<DITHER_SHIFT,  6<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  4<<DITHER_SHIFT, 14<<DITHER_SHIFT,  6<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  4<<DITHER_SHIFT, 14<<DITHER_SHIFT,  6<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  4<<DITHER_SHIFT, 14<<DITHER_SHIFT
    },
    {
         3<<DITHER_SHIFT, 11<<DITHER_SHIFT,  1<<DITHER_SHIFT,  9<<DITHER_SHIFT,
         3<<DITHER_SHIFT, 11<<DITHER_SHIFT,  1<<DITHER_SHIFT,  9<<DITHER_SHIFT,
         3<<DITHER_SHIFT, 11<<DITHER_SHIFT,  1<<DITHER_SHIFT,  9<<DITHER_SHIFT,
         3<<DITHER_SHIFT, 11<<DITHER_SHIFT,  1<<DITHER_SHIFT,  9<<DITHER_SHIFT,
         3<<DITHER_SHIFT, 11<<DITHER_SHIFT,  1<<DITHER_SHIFT,  9<<DITHER_SHIFT,
         3<<DITHER_SHIFT, 11<<DITHER_SHIFT,  1<<DITHER_SHIFT,  9<<DITHER_SHIFT,
         3<<DITHER_SHIFT, 11<<DITHER_SHIFT,  1<<DITHER_SHIFT,  9<<DITHER_SHIFT,
         3<<DITHER_SHIFT, 11<<DITHER_SHIFT,  1<<DITHER_SHIFT,  9<<DITHER_SHIFT,
         3<<DITHER_SHIFT, 11<<DITHER_SHIFT,  1<<DITHER_SHIFT
    },
    {
        15<<DITHER_SHIFT,  7<<DITHER_SHIFT, 13<<DITHER_SHIFT,  5<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  7<<DITHER_SHIFT, 13<<DITHER_SHIFT,  5<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  7<<DITHER_SHIFT, 13<<DITHER_SHIFT,  5<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  7<<DITHER_SHIFT, 13<<DITHER_SHIFT,  5<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  7<<DITHER_SHIFT, 13<<DITHER_SHIFT,  5<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  7<<DITHER_SHIFT, 13<<DITHER_SHIFT,  5<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  7<<DITHER_SHIFT, 13<<DITHER_SHIFT,  5<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  7<<DITHER_SHIFT, 13<<DITHER_SHIFT,  5<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  7<<DITHER_SHIFT, 13<<DITHER_SHIFT
    },
};

const int __glFRDitherTableRtoL[4][35] = {
    {
         0<<DITHER_SHIFT, 10<<DITHER_SHIFT,  2<<DITHER_SHIFT,  8<<DITHER_SHIFT,
         0<<DITHER_SHIFT, 10<<DITHER_SHIFT,  2<<DITHER_SHIFT,  8<<DITHER_SHIFT,
         0<<DITHER_SHIFT, 10<<DITHER_SHIFT,  2<<DITHER_SHIFT,  8<<DITHER_SHIFT,
         0<<DITHER_SHIFT, 10<<DITHER_SHIFT,  2<<DITHER_SHIFT,  8<<DITHER_SHIFT,
         0<<DITHER_SHIFT, 10<<DITHER_SHIFT,  2<<DITHER_SHIFT,  8<<DITHER_SHIFT,
         0<<DITHER_SHIFT, 10<<DITHER_SHIFT,  2<<DITHER_SHIFT,  8<<DITHER_SHIFT,
         0<<DITHER_SHIFT, 10<<DITHER_SHIFT,  2<<DITHER_SHIFT,  8<<DITHER_SHIFT,
         0<<DITHER_SHIFT, 10<<DITHER_SHIFT,  2<<DITHER_SHIFT,  8<<DITHER_SHIFT,
         0<<DITHER_SHIFT, 10<<DITHER_SHIFT,  2<<DITHER_SHIFT
    },
    {
        12<<DITHER_SHIFT,  6<<DITHER_SHIFT, 14<<DITHER_SHIFT,  4<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  6<<DITHER_SHIFT, 14<<DITHER_SHIFT,  4<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  6<<DITHER_SHIFT, 14<<DITHER_SHIFT,  4<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  6<<DITHER_SHIFT, 14<<DITHER_SHIFT,  4<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  6<<DITHER_SHIFT, 14<<DITHER_SHIFT,  4<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  6<<DITHER_SHIFT, 14<<DITHER_SHIFT,  4<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  6<<DITHER_SHIFT, 14<<DITHER_SHIFT,  4<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  6<<DITHER_SHIFT, 14<<DITHER_SHIFT,  4<<DITHER_SHIFT,
        12<<DITHER_SHIFT,  6<<DITHER_SHIFT, 14<<DITHER_SHIFT
    },
    {
         3<<DITHER_SHIFT,  9<<DITHER_SHIFT,  1<<DITHER_SHIFT, 11<<DITHER_SHIFT,
         3<<DITHER_SHIFT,  9<<DITHER_SHIFT,  1<<DITHER_SHIFT, 11<<DITHER_SHIFT,
         3<<DITHER_SHIFT,  9<<DITHER_SHIFT,  1<<DITHER_SHIFT, 11<<DITHER_SHIFT,
         3<<DITHER_SHIFT,  9<<DITHER_SHIFT,  1<<DITHER_SHIFT, 11<<DITHER_SHIFT,
         3<<DITHER_SHIFT,  9<<DITHER_SHIFT,  1<<DITHER_SHIFT, 11<<DITHER_SHIFT,
         3<<DITHER_SHIFT,  9<<DITHER_SHIFT,  1<<DITHER_SHIFT, 11<<DITHER_SHIFT,
         3<<DITHER_SHIFT,  9<<DITHER_SHIFT,  1<<DITHER_SHIFT, 11<<DITHER_SHIFT,
         3<<DITHER_SHIFT,  9<<DITHER_SHIFT,  1<<DITHER_SHIFT, 11<<DITHER_SHIFT,
         3<<DITHER_SHIFT,  9<<DITHER_SHIFT,  1<<DITHER_SHIFT
    },
    {
        15<<DITHER_SHIFT,  5<<DITHER_SHIFT, 13<<DITHER_SHIFT,  7<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  5<<DITHER_SHIFT, 13<<DITHER_SHIFT,  7<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  5<<DITHER_SHIFT, 13<<DITHER_SHIFT,  7<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  5<<DITHER_SHIFT, 13<<DITHER_SHIFT,  7<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  5<<DITHER_SHIFT, 13<<DITHER_SHIFT,  7<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  5<<DITHER_SHIFT, 13<<DITHER_SHIFT,  7<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  5<<DITHER_SHIFT, 13<<DITHER_SHIFT,  7<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  5<<DITHER_SHIFT, 13<<DITHER_SHIFT,  7<<DITHER_SHIFT,
        15<<DITHER_SHIFT,  5<<DITHER_SHIFT, 13<<DITHER_SHIFT
    },
};

const void *__glFRDitherTables[] = {
    __glFRDitherTable, __glFRDitherTableRtoL
};

/* Used for masking the color buffer, and measuring NULL rasterization */
void __fastcall __glNullSpanlet(GLbitfield mask, __GLtri *tr)
{
}

#define REVERSE_BITORDER(inMask) \
{ \
    GLuint tmp; \
    GLubyte *src = (GLubyte *) &(inMask); \
    GLubyte *dst = (GLubyte *) &tmp; \
    dst[0] = __glMsbToLsbTable[src[3]]; \
    dst[1] = __glMsbToLsbTable[src[2]]; \
    dst[2] = __glMsbToLsbTable[src[1]]; \
    dst[3] = __glMsbToLsbTable[src[0]]; \
    (inMask) = tmp; \
}

void __fastcall __glGenStipple(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;

    if (mask) {
        GLuint x = tr->dx < 0 ? -tr->x - 31 : tr->x;
        GLuint y =
            gc->constants.yInverted ? gc->constants.height - tr->y - 1 : tr->y;

        GLuint row = gc->polygon.stipple[y & 31];
        GLuint shift = x & 31;
        GLuint result = (row << shift) | (row >> (32 - shift));

        /* reverse order for right-to-left rendering! */
        if (tr->dx < 0) {
            REVERSE_BITORDER(result);
        }
        mask &= result;
    }

    if (mask)
        (*gc->procs.afterStipple)(mask, tr);
}

void __fastcall __glGenMask(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLbitfield scissorMask = ~0UL;
    int x, y, y0, leftClip = 0, rightClip = 0;

    y = tr->y;
    x = tr->dx < 0 ? -tr->x - 31 : tr->x;  /* left edge of spanlet */

    /* apply scissor to mask */
    y0 = y + gc->constants.viewportYAdjust;

    if (y0 < gc->transform.clipY0 || y0 >= gc->transform.clipY1) {
        return;
    } else {
        int x0, x1;

        x0 = x + gc->constants.viewportXAdjust;
        x1 = x0 + 31;

        if (x1 < gc->transform.clipX0 || x0 >= gc->transform.clipX1) {
            return;
        }

        /* spanlet crosses left edge */
        if (x0 < gc->transform.clipX0) {
            leftClip = gc->transform.clipX0 - x0;

            scissorMask &= ~0UL >> leftClip;
        }

        /* spanlet crosses right edge */
        if (x1 >= gc->transform.clipX1) {
            rightClip = (x1 - gc->transform.clipX1) + 1;

            scissorMask &= ~0UL << rightClip;
        }
    }

    /* apply window clip to mask (pixel ownership) */
    if (gc->polygon.shader.modeFlags & __GL_SHADE_OWNERSHIP_TEST) {
        __GLownershipBuffer *mfb = &gc->ownershipBuffer;
        int xBytes = x >> 3;
        int xBits = x & 7;
        GLubyte *p;
        GLbitfield clipMask;

        p = (GLubyte *) mfb->buf.base + (y * mfb->buf.byteWidth) + xBytes;

        if (leftClip && rightClip) {
            int left, right, ii;

            left = gc->transform.clipX0 - gc->constants.viewportXAdjust;
            right = gc->transform.clipX1 - gc->constants.viewportXAdjust - 1;
            left = (left>>3) - xBytes;
            right = (right>>3) - xBytes;

            clipMask = 0;
            for (ii = left; ii < right; ii++) {
                clipMask |= p[ii] << ((3-ii)<<3);
            }
            if (ii < 4) {
                clipMask |= p[ii] << ((3-ii)<<3);
                clipMask <<= xBits;
            } else {
                clipMask <<= xBits;
                clipMask |= p[4] >> 8-xBits;
            }

        } else if (leftClip) {
            int left, bytes;

            left = gc->transform.clipX0 - gc->constants.viewportXAdjust;
            bytes = (left >> 3) - xBytes;

            if (bytes == 0) {

                clipMask = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];

                if (xBits != 0) {
                    clipMask <<= xBits;
                    clipMask |= p[4] >> 8-xBits;
                }

            } else {

                clipMask = p[4];

                switch (bytes) {
                case 1:
                    clipMask |= (p[1] << 24);
                    /*FALLTHRU*/
                case 2:
                    clipMask |= (p[2] << 16);
                    /*FALLTHRU*/
                case 3:
                    clipMask |= (p[3] << 8);
                    break;
                }

                clipMask >>= 8-xBits;
            }

        } else if (rightClip) {
            int right, bytes;

            right = gc->transform.clipX1 - gc->constants.viewportXAdjust - 1;
            bytes = (right >> 3) - xBytes;

            if (bytes == 4) {

                clipMask = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];

                if (xBits != 0) {
                    clipMask <<= xBits;
                    clipMask |= p[4] >> 8-xBits;
                }

            } else {

                clipMask = (p[0] << 24);

                switch (bytes) {
                case 3:
                    clipMask |= (p[3] << 0);
                    /*FALLTHRU*/
                case 2:
                    clipMask |= (p[2] << 8);
                    /*FALLTHRU*/
                case 1:
                    clipMask |= (p[1] << 16);
                    break;
                }

                clipMask <<= xBits;
            }

        } else {

            clipMask = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];

            if (xBits != 0) {
                clipMask <<= xBits;
                clipMask |= p[4] >> 8-xBits;
            }
        }

        scissorMask &= clipMask;
        if (!scissorMask)
            return;
    }

    if (tr->dx < 0) {
        REVERSE_BITORDER(scissorMask);
    }

    mask &= scissorMask;

    if (mask)
        (*gc->procs.afterGenMask)(mask, tr);
}

__GLspanlet __glFRPickStencilOpProc(GLenum op)
{
    __GLspanlet proc;

    switch (op) {
    case GL_ZERO:
        proc = __glFRStencilZero;
        break;
    case GL_KEEP:
        proc = __glFRStencilKeep;
        break;
    case GL_REPLACE:
        proc = __glFRStencilReplace;
        break;
    case GL_INCR:
        proc = __glFRStencilIncrement;
        break;
    case GL_DECR:
        proc = __glFRStencilDecrement;
        break;
    case GL_INVERT:
        proc = __glFRStencilInvert;
        break;
    default:
        proc = (__GLspanlet) NULL;
    }

    return proc;
}

void __fastcall __glFRAlphaTest(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *atft = gc->frontBuffer.alphaTestFuncTable;
    GLbitfield mask_out = ~mask;
    GLbitfield bit = HIBIT;
    GLubyte *cp = tr->colorBuf;

    while (1) {
        while (((int)mask) < 0) {
            if (0 == atft[cp[3]])
                mask_out |= bit;
            mask <<= 1;
            bit >>= 1;
            cp += 4;
        }
        if (mask == 0) break;

        mask <<= 1;
        bit >>= 1;
        cp += 4;
    }

    if (~mask_out)
        (*gc->procs.afterAlphaTest)(~mask_out, tr);
}

void __fastcall __glFRIndexTest(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLubyte *itft = gc->frontBuffer.indexTestFuncTable;
    GLbitfield mask_out = ~mask;
    GLbitfield bit = HIBIT;
    GLubyte *cp = tr->colorBuf;

    while (1) {
        while (((int)mask) < 0) {
            if (0 == itft[*cp])
                mask_out |= bit;
            mask <<= 1;
            bit >>= 1;
            cp++;
        }
        if (mask == 0) break;

        mask <<= 1;
        bit >>= 1;
        cp++;
    }

    if (~mask_out)
        (*gc->procs.afterIndexTest)(~mask_out, tr);
}

void __fastcall __glFRFogCI(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLbitfield save_mask = mask;
    GLuint *cp = (GLuint *)tr->colorBuf;
    GLint f = tr->f;
    GLint fog;
    GLubyte fog_i = gc->state.fog.i;

    while (1) {
        while (((int)mask) < 0) {

            fog = TruncFixed(f, COLOR_FRAC_BITS);
            if (fog < 0) fog = 0;
            if (fog > 255) fog = 255;

            cp[0] = cp[0] + (BLEND(255 - fog, fog_i) << COLOR_FRAC_BITS);

            mask <<= 1;
            f += tr->dfdx;
            cp += 1;
        }
        if (mask == 0) break;

        mask <<= 1;
        f += tr->dfdx;
        cp += 1;
    }

    (*gc->procs.afterFog)(save_mask, tr);
}

void __fastcall __glFRFogRGB(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLbitfield save_mask = mask;
    GLubyte *cp = tr->colorBuf;
    GLint f = tr->f;
    GLint fog;
    GLubyte fog_r = gc->state.fog.r;
    GLubyte fog_g = gc->state.fog.g;
    GLubyte fog_b = gc->state.fog.b;

    while (1) {
        while (((int)mask) < 0) {

            fog = TruncFixed(f, COLOR_FRAC_BITS);
            if (fog < 0) fog = 0;
            if (fog > 255) fog = 255;

            cp[0] = SA_MSA(cp[0], fog, fog_r);
            cp[1] = SA_MSA(cp[1], fog, fog_g);
            cp[2] = SA_MSA(cp[2], fog, fog_b);

            mask <<= 1;
            f += tr->dfdx;
            cp += 4;
        }
        if (mask == 0) break;

        mask <<= 1;
        f += tr->dfdx;
        cp += 4;
    }

    (*gc->procs.afterFog)(save_mask, tr);
}

void __fastcall __glFRFogSlowCI(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLbitfield save_mask = mask;
    GLuint *cp = (GLuint *)tr->colorBuf;
    GLint f = tr->f;
    GLfloat fog_i = gc->state.fog.index;
    __GLfloat ffog, eyeZ, density, density2, end;
    double dtmp;

    density = gc->state.fog.density;
    density2 = density * density;
    end = gc->state.fog.end;

    while (1) {
        while (((int)mask) < 0) {

            /* Compute fog value */
            eyeZ = f * (1.0f/(float)(1<<COLOR_FRAC_BITS));
            if (eyeZ < 0) eyeZ = -eyeZ;

            /* Convert to a table!!! */
            switch (gc->state.fog.mode) {
            case GL_EXP:
                ffog = __GL_POWF(__glE, -density * eyeZ);
                break;
            case GL_EXP2:
                ffog = __GL_POWF(__glE, -(density2 * eyeZ * eyeZ));
                break;
            case GL_LINEAR:
                ffog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
                break;
            }

            if (ffog < 0.0f)
                ffog = 0.0f;
            else if (ffog > 1.0f)
                ffog = 1.0f;

            cp[0] = cp[0] + FloatToFixed(fog_i * (1.0f-ffog), COLOR_FRAC_BITS);

            mask <<= 1;
            f += tr->dfdx;
            cp++;
        }
        if (mask == 0) break;

        mask <<= 1;
        f += tr->dfdx;
        cp++;
    }

    (*gc->procs.afterFog)(save_mask, tr);
}

void __fastcall __glFRFogSlowRGB(GLbitfield mask, __GLtri *tr)
{
    __GLcontext *gc = tr->gc;
    GLbitfield save_mask = mask;
    GLubyte *cp = tr->colorBuf;
    GLint f = tr->f;
    GLfloat fog_r = gc->state.fog.r;
    GLfloat fog_g = gc->state.fog.g;
    GLfloat fog_b = gc->state.fog.b;
    __GLfloat ffog, eyeZ, density, density2, end;
    double dtmp;

    density = gc->state.fog.density;
    density2 = density * density;
    end = gc->state.fog.end;

    while (1) {
        while (((int)mask) < 0) {

            /* Compute fog value */
            eyeZ = f * (1.0f/(float)(1<<COLOR_FRAC_BITS));
            if (eyeZ < 0) eyeZ = -eyeZ;

            /* Convert to a table!!! */
            switch (gc->state.fog.mode) {
            case GL_EXP:
                ffog = __GL_POWF(__glE, -density * eyeZ);
                break;
            case GL_EXP2:
                ffog = __GL_POWF(__glE, -(density2 * eyeZ * eyeZ));
                break;
            case GL_LINEAR:
                ffog = (end - eyeZ) * gc->state.fog.oneOverEMinusS;
                break;
            }

            /* clamp fog value */
            if (ffog < 0.0f)
                ffog = 0.0f;
            else if (ffog > 1.0f)
                ffog = 1.0f;

            /* Blend incoming color against the fog color */
            cp[0] =
                FloatToFixed(((float)cp[0]) * ffog + (1.0f-ffog) * fog_r, 0);
            cp[1] =
                FloatToFixed(((float)cp[1]) * ffog + (1.0f-ffog) * fog_g, 0);
            cp[2] =
                FloatToFixed(((float)cp[2]) * ffog + (1.0f-ffog) * fog_b, 0);

            mask <<= 1;
            f += tr->dfdx;
            cp += 4;
        }
        if (mask == 0) break;

        mask <<= 1;
        f += tr->dfdx;
        cp += 4;
    }

    (*gc->procs.afterFog)(save_mask, tr);
}

/* Slow spanlet path for DrawBuffer(FRONT_AND_BACK) */
void __fastcall __glFRDoubleStore(GLbitfield mask, __GLtri *tr)
{
    GLvoid *cp = tr->cp;
    const int *dither = tr->dither;
    __GLcontext *gc = tr->gc;
    __GLcolorBuffer *cfb = gc->drawBuffer;
    __GLspanlet storeProc = gc->procs.doubleStoreProc;

    /* Draw to the backbuffer */
    gc->drawBuffer = gc->back;
    (*storeProc)(mask, tr);

    /* Set color buffer to front, and reset dither */
    tr->cp = tr->cp2;
    tr->dither = dither;

    /* Draw to the frontbuffer */
    gc->drawBuffer = gc->front;
    (*storeProc)(mask, tr);

    /* Reset color buffer to back */
    tr->cp = cp;
    gc->drawBuffer = cfb;
}

int __glPCPickTriangleProcs(__GLcontext *gc)
{
    extern int __glCanMmx( __GLcontext *gc );

    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint pixFlags, cmbFlags;
    GLuint texFlags = 0;
    GLboolean needAlpha, combineOp;
    GLboolean genericStore = GL_FALSE;
    GLboolean genericSetup = GL_FALSE;
    GLboolean genericTexture = GL_FALSE;
    GLboolean cookTexture = GL_FALSE;
    GLboolean noOutput = GL_FALSE;
#ifdef __GL_CODEGEN
    GLboolean usingSpanlets = GL_FALSE;
#endif
#ifndef __GL_CODEGEN
    GLuint triFlags;
    GLuint wl2, hl2;
#endif
    __GLspanlet *sp, *pixProcs, *cmbProcs;
    void (**triProcs)(__GLcontext *gc, __GLvertex *v0, __GLvertex *v1,
                      __GLvertex *v2);

    /* HACK!!! See comments in fr_sqrs.c */
    extern GLubyte __glNegativeSquareTable[256];
    assert((__glNegativeSquareTable + 256) == __glSquareTable);

    /* JCB's Quake hacks */
    if (0) {
        GLuint TO_CLEAR = (__GL_SHADE_DITHER /* | __GL_SHADE_DEPTH_TEST */);
        modeFlags &= ~TO_CLEAR;
        gc->polygon.shader.modeFlags &= ~TO_CLEAR;

        gc->state.texture[gc->texture.currentTexUnit].env[0].mode = GL_REPLACE;
    }


#ifdef __GL_ENVVARS
    if (check_env) {
        char *env;

        check_env = GL_FALSE;

        env = getenv("__GL_DISABLE_FASTPATH");
        if (env)
            disable_fastpath = atoi(env) ? GL_TRUE : GL_FALSE;

        env = getenv("__GL_NULL_TRIANGLE_RENDER");
        if (env)
            null_triangle = atoi(env) ? GL_TRUE : GL_FALSE;

        env = getenv("__GL_NULL_TRIANGLE_FILL");
        if (env)
            null_spanlets = atoi(env) ? GL_TRUE : GL_FALSE;

        env = getenv("__GL_DISABLE_OG");
        if (env)
            disable_og = atoi(env) ? GL_TRUE : GL_FALSE;

        env = getenv("__GL_GENERIC_STORE");
        if (env)
            generic_store = atoi(env) ? GL_TRUE : GL_FALSE;

        env = getenv("__GL_GENERIC_SETUP");
        if (env)
            generic_setup = atoi(env) ? GL_TRUE : GL_FALSE;
    }
    if (disable_fastpath) {
        return GL_FALSE;
    }
    genericStore = generic_store;
    genericSetup = generic_setup;
#endif /* __GL_ENVVARS */

    /**********************************************************************
     * Check for modes which are not yet supported by this rasterizer
     **********************************************************************/

    if (modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL) {
        genericSetup = GL_TRUE;
#ifndef __GL_CODEGEN
        /* Not supported in the static version */
        return GL_FALSE;
#endif
    }

    if ((modeFlags & __GL_SHADE_SLOW_FOG) &&
        (gc->state.hints.fog == GL_NICEST)) {
        genericSetup = GL_TRUE;
    }

    /* Check for pixel formats not yet supported by this rasterizer */
    if (modeFlags & __GL_SHADE_RGB) {
        switch (gc->drawBuffer->fbtype) {
        case RGB332:
        case RGB5:
        case RGB565:
        case XRGB8:
            break;
        case RGB8:
            genericStore = GL_TRUE;
            break;
        default:
            assert(0);  /* We don't support any other framebuffer formats!!! */
            break;
        }
    } else {
        /* Color index rendering is supported only for 8-bit framebuffers */
        assert(1 == gc->drawBuffer->buf.elementSize);
    }

    /* Check for depth formats not yet supported by this rasterizer */
    if (modeFlags & __GL_SHADE_DEPTH_TEST) {
        assert ((gc->depthBuffer.buf.depth == 32) ||
                (gc->depthBuffer.buf.depth == 16));
    }

    if (gc->state.polygon.frontMode != GL_FILL ||
        gc->state.polygon.backMode != GL_FILL) {
        genericSetup = GL_TRUE;
    }

#if 0
    /* XXX - Quite simply, we do not support anti-aliased polygons.  Ouch. */
    if (gc->state.enables.general & __GL_POLYGON_SMOOTH_ENABLE) {
        return GL_FALSE;
    }
#endif

    if (gc->buffers.doubleStore) {
        genericSetup = GL_TRUE;
        genericStore = GL_TRUE;
    }

    /**********************************************************************
     * Set up flags for picking color generation procs
     **********************************************************************/

#ifndef __GL_CODEGEN
    triFlags = 0;
#endif
    pixFlags = 0;
    cmbFlags = 0;
    needAlpha = GL_FALSE;
    combineOp = GL_FALSE;

    if (modeFlags & __GL_SHADE_RGB) {

        triProcs = __fr_rgb_tri_rast_table;
        pixProcs = __glGenPixTab[gc->drawBuffer->fbtype];

        if (modeFlags & __GL_SHADE_MASK) {
            if (gc->state.raster.rMask |
                gc->state.raster.gMask |
                gc->state.raster.bMask |
                gc->state.raster.aMask) {
                genericStore = GL_TRUE;
            } else {
                noOutput = GL_TRUE;
            }
        }

        if (genericStore) {
            pixProcs = __glGenColorTab[gc->drawBuffer->fbtype];
        }

        if (modeFlags & (__GL_SHADE_LOGICOP |
                          __GL_SHADE_SLOW_FOG |
                          __GL_SHADE_ALPHA_TEST |
                         __GL_SHADE_BLEND) ||
            genericStore) {
            pixProcs = __glGenColorTab[gc->drawBuffer->fbtype];
            cmbProcs = __glCombineTab[gc->drawBuffer->fbtype];
            combineOp = GL_TRUE;
        }

        if (modeFlags & __GL_SHADE_BLEND) {
#ifndef __GL_CODEGEN
            triFlags |= __FR_SETUP_ALPHA;
#endif
            pixFlags |= __FR_PIXEL_ALPHA;
            needAlpha = GL_TRUE;
        }

        if ((modeFlags & __GL_SHADE_ALPHA_TEST) ||
            (gc->modes.alphaMask && gc->state.raster.aMask)) {
#ifndef __GL_CODEGEN
            triFlags |= __FR_SETUP_ALPHA;
#endif
            pixFlags |= __FR_PIXEL_ALPHA;
            needAlpha = GL_TRUE;
        }
    } else {
        /* color index mode */
        triProcs = __fr_ci_tri_rast_table;
        pixProcs = __fr_ci_pix_table;

        if (modeFlags & __GL_SHADE_MASK) {
            if (gc->state.raster.writeMask != 0) {
                genericStore = GL_TRUE;
            } else {
                noOutput = GL_TRUE;
            }
        }

        if (modeFlags & (__GL_SHADE_LOGICOP |
                         __GL_SHADE_SLOW_FOG | __GL_SHADE_INDEX_TEST) |
            genericStore) {
            combineOp = GL_TRUE;
            pixProcs = __fr_ci_genclr_table;
            cmbProcs = __fr_ci_combine_table;
        }
    }

    if (modeFlags & __GL_SHADE_SMOOTH) {
#ifndef __GL_CODEGEN
        triFlags |= __FR_SETUP_SMOOTH;
#endif
        pixFlags |= __FR_PIXEL_SMOOTH;
    }

    if (modeFlags & __GL_SHADE_DITHER) {
#ifndef __GL_CODEGEN
        triFlags |= __FR_SETUP_DITHER;
#endif
        if (combineOp) {
            cmbFlags |= __FR_COMBINE_DITHER;
        } else {
            pixFlags |= __FR_PIXEL_DITHER;
        }
    }

    if (modeFlags & __GL_SHADE_TEXTURE) {
        __GLtexture *current = gc->texture.currentTexture[gc->texture.currentTexUnit];
        __GLmipMapLevel *lp = current->level;
        GLenum baseFormat;
        GLboolean needRho, mipmap = GL_FALSE;

        cookTexture = !combineOp && !needAlpha &&
            (0 == (modeFlags & __GL_SHADE_DITHER));

        needRho = current->params.minFilter != current->params.magFilter;

        if (needRho) {
            genericTexture = GL_TRUE;
        }

        switch (current->params.minFilter) {
        case GL_NEAREST:
            gc->procs.slowMinFilter = __glFRNearestFilterUVScaled;
            break;
        case GL_LINEAR:
            gc->procs.slowMinFilter = __glFRLinearFilterUVScaled;
            genericTexture = GL_TRUE;
            break;
        case GL_NEAREST_MIPMAP_NEAREST:
            gc->procs.slowMinFilter = __glFR_NMNFilter;
            genericTexture = GL_TRUE;
            mipmap = GL_TRUE;
            break;
        case GL_LINEAR_MIPMAP_NEAREST:
            gc->procs.slowMinFilter = __glFR_LMNFilter;
            genericTexture = GL_TRUE;
            mipmap = GL_TRUE;
            break;
        case GL_NEAREST_MIPMAP_LINEAR:
            gc->procs.slowMinFilter = __glFR_NMLFilter;
            genericTexture = GL_TRUE;
            mipmap = GL_TRUE;
            break;
        case GL_LINEAR_MIPMAP_LINEAR:
            gc->procs.slowMinFilter = __glFR_LMLFilter;
            genericTexture = GL_TRUE;
            mipmap = GL_TRUE;
            break;
        default:
            assert(0);
            break;
        }

        switch (current->params.magFilter) {
        case GL_NEAREST:
            gc->procs.slowMagFilter = mipmap ?
                __glFRNearestFilter :
                __glFRNearestFilterUVScaled;
            break;
        case GL_LINEAR:
            gc->procs.slowMagFilter = mipmap ?
                __glFRLinearFilter :
                __glFRLinearFilterUVScaled;
            genericTexture = GL_TRUE;
            break;
        default:
            assert(0);
            break;
        };

        if (current->params.sWrapMode != GL_REPEAT ||
            current->params.tWrapMode != GL_REPEAT) {
            genericTexture = GL_TRUE;
        }

        if (lp->border != 0) {
            genericTexture = GL_TRUE;
        }

#ifndef __GL_CODEGEN
        triFlags |= __FR_SETUP_TEXTURE;

        /* Generated code has no texture size restrictions! */
        wl2 = lp->widthLog2 - __FR_TEXEL_WIDTH_BIAS;
        if (wl2 < 0 || wl2 > __FR_TEXEL_LOG_WIDTH)
            genericTexture = GL_TRUE;
        else
            texFlags |= wl2;

        hl2 = lp->heightLog2 - __FR_TEXEL_WIDTH_BIAS;
        if (hl2 < 0 || hl2 > __FR_TEXEL_LOG_WIDTH)
            genericTexture = GL_TRUE;

        if (wl2 == hl2)
            texFlags |= __FR_TEXEL_SQUARE;
#endif

        if (modeFlags & __GL_SHADE_TEXTURE_PERSP) {
#ifndef __GL_CODEGEN
            triFlags |= __FR_SETUP_PERSPECTIVE;
#endif
            texFlags |= __FR_TEXEL_PERSPECTIVE;
        }

        baseFormat = current->texelFormat;
        if (baseFormat == GL_COLOR_INDEX) {
            if (lp->internalFormat != GL_COLOR_INDEX8_EXT) {
                assert(lp->internalFormat == GL_COLOR_INDEX16_EXT);
                genericTexture = GL_TRUE;
            }

            baseFormat = current->CT.baseFormat;
            texFlags |= __FR_TEXEL_HAS_INDEX;

            if (gc->modes.rgbMode)
                cookTexture = GL_FALSE;
        }

        if (gc->modes.rgbMode) {
            texFlags |= __FR_TEXEL_NEED_RGB;

            switch (baseFormat) {
            case GL_LUMINANCE:
                if (lp->border)
                    gc->procs.slowExtractTexel = __glFRExtractTexelL_B;
                else
                    gc->procs.slowExtractTexel = __glFRExtractTexelL;
                break;
            case GL_LUMINANCE_ALPHA:
                texFlags |= __FR_TEXEL_HAS_ALPHA;
                if (needAlpha) {
                    texFlags |= __FR_TEXEL_NEED_ALPHA;
                    pixFlags |= __FR_PIXEL_TEXTURE_ALPHA;
                }
                if (lp->border)
                    gc->procs.slowExtractTexel = __glFRExtractTexelLA_B;
                else
                    gc->procs.slowExtractTexel = __glFRExtractTexelLA;
                break;
            case GL_RGB:
                texFlags |= __FR_TEXEL_HAS_RGB;
                if (lp->border)
                    gc->procs.slowExtractTexel = __glFRExtractTexelRGB_B;
                else
                    gc->procs.slowExtractTexel = __glFRExtractTexelRGB;
                break;
            case GL_RGBA:
                texFlags |= __FR_TEXEL_HAS_RGB | __FR_TEXEL_HAS_ALPHA;
                if (needAlpha | (gc->state.texture[gc->texture.currentTexUnit].env[0].mode == GL_DECAL)) {
                    texFlags |= __FR_TEXEL_NEED_ALPHA;
                    pixFlags |= __FR_PIXEL_TEXTURE_ALPHA;
                }
                if (lp->border)
                    gc->procs.slowExtractTexel = __glFRExtractTexelRGBA_B;
                else
                    gc->procs.slowExtractTexel = __glFRExtractTexelRGBA;
                break;
            case GL_INTENSITY:
                if (needAlpha) {
                    texFlags |= __FR_TEXEL_NEED_ALPHA;
                    pixFlags |= __FR_PIXEL_TEXTURE_ALPHA;
                }
                if (lp->border)
                    gc->procs.slowExtractTexel = __glFRExtractTexelI_B;
                else
                    gc->procs.slowExtractTexel = __glFRExtractTexelI;
#ifndef __GL_CODEGEN
                /* Static code does not support INTENSITY textures */
                genericTexture = GL_TRUE;
#endif
                break;

            case GL_ALPHA:
                if (lp->border)
                    gc->procs.slowExtractTexel = __glFRExtractTexelA_B;
                else
                    gc->procs.slowExtractTexel = __glFRExtractTexelA;
#ifndef __GL_CODEGEN
                genericTexture = GL_TRUE;
#endif
                break;
            default:
                assert(0);
                break;
            }
        }

        if (lp->internalFormat == GL_COLOR_INDEX8_EXT) {
            if (lp->border)
                gc->procs.slowExtractTexel = __glFRExtractTexelCI8_B;
            else
                gc->procs.slowExtractTexel = __glFRExtractTexelCI8;
        } else if (lp->internalFormat == GL_COLOR_INDEX16_EXT) {
            if (lp->border)
                gc->procs.slowExtractTexel = __glFRExtractTexelCI16_B;
            else
                gc->procs.slowExtractTexel = __glFRExtractTexelCI16;
        }

        switch (gc->state.texture[gc->texture.currentTexUnit].env[0].mode) {
        case GL_MODULATE:
            if (baseFormat == GL_ALPHA) {
                if (needAlpha) {
                    pixFlags |=
                        __FR_PIXEL_MODULATE_ALPHA |
                        __FR_PIXEL_TEXTURE_ALPHA;
                } else {
                    /* Turn off texturing! */
                    /* XXX this should happen at a higher level! */
                    gc->polygon.shader.modeFlags &=
                        ~(__GL_SHADE_TEXTURE|__GL_SHADE_TEXTURE_PERSP);
                }

            } else {
                pixFlags |= __FR_PIXEL_MODULATE;
            }
            break;
        case GL_DECAL:
            /* There is no support for DECAL w/o ALPHA since it
             * is identical to REPLACE.
             */
            if (texFlags & __FR_TEXEL_HAS_ALPHA) {
                pixFlags |= __FR_PIXEL_DECAL;
            } else {
                pixFlags |= __FR_PIXEL_REPLACE;
            }
            break;
        case GL_BLEND:
            if (baseFormat == GL_ALPHA) {
                if (needAlpha) {
                    pixFlags |=
                        __FR_PIXEL_MODULATE_ALPHA |
                        __FR_PIXEL_TEXTURE_ALPHA;
                } else {
                    /* Turn off texturing! */
                    /* XXX this should happen at a higher level! */
                    gc->polygon.shader.modeFlags &=
                        ~(__GL_SHADE_TEXTURE|__GL_SHADE_TEXTURE_PERSP);
                }

            } else if (baseFormat == GL_INTENSITY && needAlpha) {
                /* The BLEND behavior for the alpha component is unique,
                 * and requires special handling.
                 */
                pixFlags |= __FR_PIXEL_BLEND_INTENSITY;
            } else {
                pixFlags |= __FR_PIXEL_BLEND;
            }
            /* Convert texture environment color */
            gc->texture.envColor[gc->texture.currentTexUnit][0] =
                __GL_FLOAT_TO_UB(gc->state.texture[gc->texture.currentTexUnit].env[0].color.r *
                                 gc->frontBuffer.oneOverRedScale);
            gc->texture.envColor[gc->texture.currentTexUnit][1] =
                __GL_FLOAT_TO_UB(gc->state.texture[gc->texture.currentTexUnit].env[0].color.g *
                                 gc->frontBuffer.oneOverGreenScale);
            gc->texture.envColor[gc->texture.currentTexUnit][2] =
                __GL_FLOAT_TO_UB(gc->state.texture[gc->texture.currentTexUnit].env[0].color.b *
                                 gc->frontBuffer.oneOverBlueScale);
            gc->texture.envColor[gc->texture.currentTexUnit][3] =
                __GL_FLOAT_TO_UB(gc->state.texture[gc->texture.currentTexUnit].env[0].color.a *
                                 gc->frontBuffer.oneOverAlphaScale);
            /* If dithering, scale to component max to avoid overflow */
            if (modeFlags & __GL_SHADE_DITHER) {
                gc->texture.envColor[gc->texture.currentTexUnit][0] -=
                    (gc->texture.envColor[gc->texture.currentTexUnit][0] >> gc->modes.redBits);
                gc->texture.envColor[gc->texture.currentTexUnit][1] -=
                    (gc->texture.envColor[gc->texture.currentTexUnit][1] >> gc->modes.greenBits);
                gc->texture.envColor[gc->texture.currentTexUnit][2] -=
                    (gc->texture.envColor[gc->texture.currentTexUnit][2] >> gc->modes.blueBits);
                if (gc->modes.alphaBits) {
                    gc->texture.envColor[gc->texture.currentTexUnit][3] -=
                        (gc->texture.envColor[gc->texture.currentTexUnit][3] >> gc->modes.alphaBits);
                }
            }
            break;
        case GL_REPLACE:
            if (baseFormat == GL_ALPHA) {
                if (needAlpha) {
                    genericSetup = GL_TRUE;
                    pixFlags |=
                        __FR_PIXEL_MODULATE_ALPHA |
                        __FR_PIXEL_TEXTURE_ALPHA;
                } else {
                    /* Turn off texturing! */
                    /* XXX this should happen at a higher level! */
                    gc->polygon.shader.modeFlags &=
                        ~(__GL_SHADE_TEXTURE|__GL_SHADE_TEXTURE_PERSP);
                }
            } else {
                pixFlags |= __FR_PIXEL_REPLACE;
            }
            break;
        case GL_ADD:
            pixFlags |= __FR_PIXEL_ADD;
            break;
        default:
            assert(0);
            break;
        }

        if (genericTexture ||
            (__FR_PIXEL_REPLACE != (pixFlags & __FR_PIXEL_TEXENV))) {
            cookTexture = GL_FALSE;
        }

        /* We should never hit an invalid texenv (e.g. BLEND in CI mode)
         * since that should be caught by IsTextureConsistent()
         */
        if (genericTexture) {
            if (
#if !AFFINE_RHO
                needRho ||
#endif
                (texFlags & __FR_TEXEL_PERSPECTIVE)) {
                if (!gc->modes.rgbMode || current->texelFormat == GL_ALPHA) {
                    gc->procs.extractTexels = mipmap ?
                        __glGenericExtractTexelsPCMipmap_1 :
                        (needRho ?
                         __glGenericExtractTexelsPCRho_1 :
                         __glGenericExtractTexelsPC_1);
                } else if (texFlags & __FR_TEXEL_NEED_ALPHA) {
                    gc->procs.extractTexels = mipmap ?
                        __glGenericExtractTexelsPCMipmap_4 :
                        (needRho ?
                         __glGenericExtractTexelsPCRho_4 :
                         __glGenericExtractTexelsPC_4);
                } else {
                    gc->procs.extractTexels = mipmap ?
                        __glGenericExtractTexelsPCMipmap_3 :
                        (needRho ?
                         __glGenericExtractTexelsPCRho_3 :
                         __glGenericExtractTexelsPC_3);
                }
            } else {
#if AFFINE_RHO
                if (!gc->modes.rgbMode || current->texelFormat == GL_ALPHA) {
                    gc->procs.extractTexels = mipmap ?
                        __glGenericExtractTexelsMipmap_1 :
                        (needRho ?
                         __glGenericExtractTexelsRho_1 :
                         __glGenericExtractTexels_1);
                } else if (texFlags & __FR_TEXEL_NEED_ALPHA) {
                    gc->procs.extractTexels = mipmap ?
                        __glGenericExtractTexelsMipmap_4 :
                        (needRho ?
                         __glGenericExtractTexelsRho_4 :
                         __glGenericExtractTexels_4);
                } else {
                    gc->procs.extractTexels = mipmap ?
                        __glGenericExtractTexelsMipmap_3 :
                        (needRho ?
                         __glGenericExtractTexelsRho_3 :
                         __glGenericExtractTexels_3);
                }
#else
                if (!gc->modes.rgbMode || current->texelFormat == GL_ALPHA) {
                    gc->procs.extractTexels = 
                        __glGenericExtractTexels_1;
                } else if (texFlags & __FR_TEXEL_NEED_ALPHA) {
                    gc->procs.extractTexels =
                        __glGenericExtractTexels_4;
                } else {
                    gc->procs.extractTexels =
                        __glGenericExtractTexels_3;
                }
#endif
            }
        } else {
#ifndef __GL_CODEGEN
            gc->procs.extractTexels = __fr_texel_table[texFlags];
#else /* __GL_CODEGEN */
            gc->procs.extractTexels =
                OGGenerateTR(gc, texFlags, lp->widthLog2, lp->heightLog2);
#endif /* __GL_CODEGEN */
        }
        assert(gc->procs.extractTexels);
    }

    /**********************************************************************
     * Set up the rendering pipeline by chaining procs in processing order
     **********************************************************************/

    /* The OG code does not handle the case where we have SHADE_MASK and no
     * depth test.  (This is a rather peculiar case anyway).
     * It _does_ handle the case where we have SHADE_MASK and DEPTH_TEST.
     * Hence the term below.
     */

#ifdef __GL_CODEGEN
#define __GL_CODEGEN_NUM 1
#else
#define __GL_CODEGEN_NUM 0
#endif

    if (!__GL_CODEGEN_NUM ||
#ifdef __GL_ENVVARS
        disable_og ||
#endif
        (modeFlags & (__GL_SHADE_OWNERSHIP_TEST |
                      __GL_SHADE_STIPPLE |
                      __GL_SHADE_ALPHA_TEST |
                      __GL_SHADE_INDEX_TEST |
                      __GL_SHADE_STENCIL_TEST |
                      __GL_SHADE_SLOW_FOG |
                      __GL_SHADE_POLYGON_OFFSET_FILL |
                      __GL_SHADE_LOGICOP)) ||
        ((modeFlags & __GL_SHADE_MASK) && !(modeFlags & __GL_SHADE_DEPTH_TEST)) ||
        ((modeFlags & __GL_SHADE_TEXTURE) &&
         /* DECAL and ADD are identical bit masks since they are mutually
          * exclusive
          */
         ((pixFlags & __FR_PIXEL_TEXENV) == __FR_PIXEL_DECAL &&
          gc->modes.rgbMode) ||
         ((pixFlags & __FR_PIXEL_TEXENV) == __FR_PIXEL_BLEND) ||
         ((pixFlags & __FR_PIXEL_TEXENV) == __FR_PIXEL_BLEND_INTENSITY) ||
         ((pixFlags & __FR_PIXEL_TEXENV) == __FR_PIXEL_REPLACE_ALPHA) ||
         ((pixFlags & __FR_PIXEL_TEXENV) == __FR_PIXEL_MODULATE_ALPHA) ||
         (gc->texture.currentTexture[gc->texture.currentTexUnit] &&
          gc->texture.currentTexture[gc->texture.currentTexUnit]->dim != 2)) ||
        (!__glCanMmx(gc) && combineOp) ||
        genericTexture || genericSetup || genericStore ||
        !gc->transform.reasonableViewport)
        {
#ifdef __GL_CODEGEN
            usingSpanlets = GL_TRUE;
            gc->procs.renderTrap = GenerateRasterizer(gc, GL_TRUE);
#endif

            sp = &gc->procs.renderSpan;

            /* Pick triangle and span procs */
            if (!gc->transform.reasonableViewport ||
                modeFlags & __GL_SHADE_OWNERSHIP_TEST) {
                *sp = __glGenMask;
                sp = &gc->procs.afterGenMask;
            }

            if (modeFlags & __GL_SHADE_STIPPLE) {
                *sp = __glGenStipple;
                sp = &gc->procs.afterStipple;
            }

            if (modeFlags & (__GL_SHADE_ALPHA_TEST | __GL_SHADE_INDEX_TEST)) {

                assert(pixProcs[pixFlags]);
                *sp = pixProcs[pixFlags];
                sp = &gc->procs.afterGenColors;

                if (modeFlags & __GL_SHADE_ALPHA_TEST) {
                    *sp = __glFRAlphaTest;
                    sp = &gc->procs.afterAlphaTest;
                } else {
                    /* Aarghh!!  Stupid special cases... */
                    if (modeFlags & __GL_SHADE_SLOW_FOG) {
                        if (gc->state.hints.fog == GL_NICEST)
                            *sp = __glFRFogSlowCI;
                        else
                            *sp = __glFRFogCI;
                        sp = &gc->procs.afterFog;
                    }

                    *sp = __glFRIndexTest;
                    sp = &gc->procs.afterIndexTest;
                }
            }

            if (modeFlags & __GL_SHADE_STENCIL_TEST) {

#ifndef __GL_CODEGEN
                triFlags |= __FR_SETUP_STENCIL;
#endif

                *sp = __glFRStencilTest;
                sp = &gc->procs.afterStencilTest;

                gc->procs.stencilOpSFail =
                    __glFRPickStencilOpProc(gc->state.stencil.fail);
                gc->procs.stencilOpZFail =
                    __glFRPickStencilOpProc(gc->state.stencil.depthFail);
                gc->procs.stencilOpZPass =
                    __glFRPickStencilOpProc(gc->state.stencil.depthPass);
            }

            /* If writing is disabled and the test is GL_ALWAYS we can effectively
             * skip depth testing.
             */
            if (modeFlags & __GL_SHADE_DEPTH_TEST &&
                (gc->state.depth.writeEnable ||
                 gc->state.depth.testFunc != GL_ALWAYS)) {
#ifndef __GL_CODEGEN
                /* Pick ztest procs */
                GLuint ztestFlags = gc->state.depth.testFunc & 7;

                triFlags |= __FR_SETUP_DEPTH;

                if (gc->depthBuffer.buf.depth > 16) {
                    ztestFlags |= __FR_DEPTH32;
                }
                if (gc->state.depth.writeEnable) {
                    ztestFlags |= __FR_DEPTH_WRITE;
                }

                if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                    ztestFlags |= __FR_DEPTH_STENCIL;
                }

                assert(__fr_ztest_table[ztestFlags]);
                *sp = __fr_ztest_table[ztestFlags];
#else /* __GL_CODEGEN */
                *sp = OGGenerateZT(gc);
#endif /* __GL_CODEGEN */

                sp = &gc->procs.afterDepthTest;
            }

            /* Post depth-test stencil cleanup */
            if (modeFlags & __GL_SHADE_STENCIL_TEST) {
                *sp = __glFRStencilPostDepthCleanup;
                sp = &gc->procs.afterStencilDepthTest;
            }

            if (noOutput) {
                /* If all channels are masked, stop here */
                *sp = __glNullSpanlet;
            } else if (combineOp || genericStore) {

                if (0 == (modeFlags & (__GL_SHADE_ALPHA_TEST|__GL_SHADE_INDEX_TEST))) {
                    assert(pixProcs[pixFlags]);
                    *sp = pixProcs[pixFlags];
                    sp = &gc->procs.afterGenColors;
                }

                /* Special case: if INDEX_TEST is enabled, fog has already
                 * been applied.
                 */
                if (modeFlags & __GL_SHADE_SLOW_FOG) {
                    if (gc->modes.rgbMode) {
                        if (gc->state.hints.fog == GL_NICEST)
                            *sp = __glFRFogSlowRGB;
                        else
                            *sp = __glFRFogRGB;
                    } else if (0 == (modeFlags & __GL_SHADE_INDEX_TEST)) {
                        if (gc->state.hints.fog == GL_NICEST)
                            *sp = __glFRFogSlowCI;
                        else
                            *sp = __glFRFogCI;
                    }
                    sp = &gc->procs.afterFog;
                }

                if (needAlpha) {
                    cmbFlags |= __FR_COMBINE_ALPHA;
                }
                if (modeFlags & __GL_SHADE_LOGICOP) {
                    cmbFlags |= gc->state.raster.logicOp - GL_CLEAR;
                } else if (modeFlags & __GL_SHADE_BLEND) {
                    GLuint blendMask = 0;

                    switch (gc->state.raster.blendSrc) {
                    case GL_SRC_ALPHA:
                        blendMask |= __FR_BLEND_SRC_SA;
                        break;
                    case GL_ONE_MINUS_SRC_ALPHA:
                        blendMask |= __FR_BLEND_SRC_MSA;
                        break;
                    case GL_SRC_ALPHA_SATURATE:
                        blendMask |= __FR_BLEND_SRC_SAS;
                        break;
                    default:
                        genericStore = GL_TRUE;
                        break;
                    }
                    switch (gc->state.raster.blendDst) {
                    case GL_ZERO:
                        blendMask |= __FR_BLEND_DST_ZERO;
                        break;
                    case GL_ONE:
                        blendMask |= __FR_BLEND_DST_ONE;
                        break;
                    case GL_SRC_ALPHA:
                        blendMask |= __FR_BLEND_DST_SA;
                        break;
                    case GL_ONE_MINUS_SRC_ALPHA:
                        blendMask |= __FR_BLEND_DST_MSA;
                        break;
                    default:
                        genericStore = GL_TRUE;
                        break;
                    }

                    switch (blendMask) {
                    case __FR_BLEND_SA_ZERO:
                    case __FR_BLEND_SA_ONE:
                    case __FR_BLEND_SA_MSA:
                    case __FR_BLEND_MSA_SA:
                    case __FR_BLEND_SAS_ONE:
                        cmbFlags |= __FR_COMBINE_BLEND | blendMask;
                        break;
                    default:
                        genericStore = GL_TRUE;
                        break;
                    }
                } else {

                    /* If neither BLEND nor LOGICOP are enabled, treat it as
                     * logicop GL_COPY.
                     */
                    cmbFlags |= GL_COPY - GL_CLEAR;
                }
                if (genericStore) {
                    __GLspanlet storeProc;
                    assert(__glStoreTab[gc->drawBuffer->fbtype]);
                    storeProc = __glStoreTab[gc->drawBuffer->fbtype];
                    if (gc->buffers.doubleStore) {
                        *sp = __glFRDoubleStore;
                        gc->procs.doubleStoreProc = storeProc;
                    } else {
                        *sp = storeProc;
                    }
                } else {
                    assert(cmbProcs[cmbFlags]);
                    *sp = cmbProcs[cmbFlags];
                }
            } else {
                assert(pixProcs[pixFlags]);
                *sp = pixProcs[pixFlags];
            }
        } else {
#ifdef __GL_CODEGEN
            if (cookTexture) {
                __glCookTexture(gc);
            }
            gc->procs.renderTrap = GenerateRasterizer(gc, GL_FALSE);
#endif
        }

#if defined(__GL_CODEGEN)
    if (genericSetup)
        gc->procs.renderTriangle = __glFRRenderTriangle;
    else
        gc->procs.renderTriangle = GenerateSetup(gc, usingSpanlets);
#else
    gc->procs.renderTriangle = triProcs[triFlags];
#endif
    assert(gc->procs.renderTriangle);

#ifdef __GL_ENVVARS
    if (null_triangle)
        gc->procs.renderTriangle = __glNullRenderTriangle;
    if (null_spanlets)
        gc->procs.renderSpan = __glNullSpanlet;
#endif

    return GL_TRUE;
}

#endif /* __GL_PC_RAST */

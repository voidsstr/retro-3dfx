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
#ifdef __GL_CODEGEN

#include <stdio.h>
#include <malloc.h>
#include <assert.h>

#include "context.h"
#include "render.h"
#include "global.h"
#include "fr_modes.h"
#include "fr_tri.h"
#include "fr_fbtype.h"

#include "og.h"

//#define __GL_CODEGEN_STATS
#ifdef __GL_CODEGEN_STATS

#include "timer.h"

static struct {
  int num_texture_reads;
  int num_textureread_changes;
} stats;

#endif /* __GL_CODEGEN_STATS */

/* fptr (Floating-Point TRiangle) holds memory variables used 
 * used by the floating-point rasterizer.
 * (There is a much larger MMX equivalent, 'mmxtr').
 */

struct __GLfptri {
    struct celem *dstdx;
    struct celem *dstdx2;

    struct celem *qis;
    struct celem *qit;
    struct celem *st;
    struct celem *ist;
    struct celem *is;
    struct celem *it;
    struct celem *prev_is;
    struct celem *prev_it;
    struct celem *converters[16];
};

static void
this_warmup(__GLcontext *gc,
            struct __GLfptri *fptr,
            float *data)
{
    GLuint i;

    for (i = 0; i < 16; i++) {
        fptr->converters[i] = fpk(gc, ((float)(1 << 20) * (float)(3 << (15 + (i)))));
    }

    i = 0;
    fptr->qis = mem2(gc, value(gc, (int)&(data[i++])), NULL); /* allocate qword */
    i++;
    fptr->qit = mem2(gc, value(gc, (int)&(data[i++])), NULL); /* allocate qword */
    i++;

    fptr->st = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->ist = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->dstdx = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->dstdx2 = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->is = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->it = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->prev_is = mem2(gc, value(gc, (int)&(data[i++])), NULL);
    fptr->prev_it = mem2(gc, value(gc, (int)&(data[i++])), NULL);
}

/************************************************************************/


/************************************************************************/

/* Given
 *      1/q     q       s       (t)
 * on the stack, calculates new s (and t), stores them in is (and it),
 * and fixed-point versions in qis (and qit).
 * Increments the two (or three) interpolants, and leaves the stack:
 *      q       q       s       (t)
 */

static unsigned char* pcstep(__GLcontext *gc,
                             unsigned char *cb,
                             int wl2,
                             int hl2,
                             int two_d,
                             struct __GLfptri *fptr)
{
    if (two_d) {
cb = emit_code(gc, cb, fld(gc, st3));

cb = emit_code(gc, cb, fmul(gc, st1));
cb = emit_code(gc, cb, fxch(gc, st1));
                
cb = emit_code(gc, cb, fmul(gc, st3));
cb = emit_code(gc, cb, fst(gc, fptr->is));
cb = emit_code(gc, cb, fadd(gc, fptr->converters[wl2]));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fst(gc, fptr->it));
cb = emit_code(gc, cb, fadd(gc, fptr->converters[hl2]));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, offsetof(fdqwdxPWL, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, offsetof(fdswdxPWL, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st4));
cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, offsetof(fdtwdxPWL, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st4));
cb = emit_code(gc, cb, fxch(gc, st3));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fstpq(gc, fptr->qit));
cb = emit_code(gc, cb, fstpq(gc, fptr->qis));
cb = emit_code(gc, cb, fld(gc, st0));
    } else {
cb = emit_code(gc, cb, fmul(gc, st2));
                
cb = emit_code(gc, cb, fst(gc, fptr->is));
cb = emit_code(gc, cb, fadd(gc, fptr->converters[wl2]));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, offsetof(fdqwdxPWL, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fadd(gc, mem(gc, value(gc, offsetof(fdswdxPWL, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, fxch(gc, st2));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fstpq(gc, fptr->qis));
cb = emit_code(gc, cb, fld(gc, st0));
    }

  return cb;
}

/*
 * Reads is,it and prev_is,prev_it.  Calculates dstdx, trashing qis,qit.
 */

static unsigned char*
pcstepnext(__GLcontext *gc,
           unsigned char *cb,
           int wl2,
           int hl2,
           int two_d,
           struct __GLfptri *fptr)
{
    if (two_d) {
cb = emit_code(gc, cb, fld(gc, fptr->is));
cb = emit_code(gc, cb, fsub(gc, fptr->prev_is));
cb = emit_code(gc, cb, fld(gc, fptr->it));
cb = emit_code(gc, cb, fsub(gc, fptr->prev_it));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fadd(gc, fptr->converters[wl2 + 4]));
cb = emit_code(gc, cb, fxch(gc, st1));
cb = emit_code(gc, cb, fadd(gc, fptr->converters[hl2 + 4]));
cb = emit_code(gc, cb, fxch(gc, st1));

cb = emit_code(gc, cb, mov(gc, eax, fptr->is));
cb = emit_code(gc, cb, mov(gc, fptr->prev_is, eax));
cb = emit_code(gc, cb, mov(gc, eax, fptr->it));
cb = emit_code(gc, cb, mov(gc, fptr->prev_it, eax));

cb = emit_code(gc, cb, fstpq(gc, fptr->qis));
cb = emit_code(gc, cb, fstpq(gc, fptr->qit));

cb = emit_code(gc, cb, mov(gc, eax, fptr->qit));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, eax, fptr->qis));

cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, mov(gc, ecx, eax));
cb = emit_code(gc, cb, and(gc, ecx, value(gc, 0x8000)));
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, 1)));
cb = emit_code(gc, cb, sub(gc, eax, ecx));
cb = emit_code(gc, cb, pop(gc, ecx));

cb = emit_code(gc, cb, mov(gc, fptr->dstdx, eax));
    } else {
cb = emit_code(gc, cb, fld(gc, fptr->is));
cb = emit_code(gc, cb, fsub(gc, fptr->prev_is));
cb = emit_code(gc, cb, fadd(gc, fptr->converters[wl2 + 4]));

cb = emit_code(gc, cb, mov(gc, eax, fptr->is));
cb = emit_code(gc, cb, mov(gc, fptr->prev_is, eax));

cb = emit_code(gc, cb, fstpq(gc, fptr->qis));

cb = emit_code(gc, cb, mov(gc, eax, fptr->qis));
cb = emit_code(gc, cb, mov(gc, fptr->dstdx, eax));
    }

  return cb;
}


static unsigned char*
GenerateInner(__GLcontext *gc,
              unsigned char*cb,
              __GLtexture *current,
              int texelFlags,
              int wl2,
              int hl2,
              int texel_width,
              int dest_width,
              int two_d,
              struct __GLfptri *fptr)
{
    struct celem
        *top = label(gc),
        *top0 = label(gc), 
        *maskpos = label(gc),
        *done = label(gc),
        *n1 = label(gc),
        *alldone = label(gc);
    GLenum baseFormat;

cb = emit_code(gc, cb, push(gc, ebp));
cb = emit_code(gc, cb, and(gc, ebp, value(gc, 0xffff0000)));
cb = emit_code(gc, cb, je(gc, done));

top->spec.label.where = cb; resolve(gc, top, cb);

cb = emit_code(gc, cb, cmp(gc, ebp, value(gc, 0)));
cb = emit_code(gc, cb, jge(gc, maskpos));

top0->spec.label.where = cb; resolve(gc, top0, cb);

        if (wl2 <= 8) {
cb = emit_code(gc, cb, mov(gc, eax, ecx));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 24 - hl2)));
cb = emit_code(gc, cb, mov(gc, al, ch));
                if (wl2 < 8)
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 8 - wl2)));
        } else {
                /* Got to do the more expensive 16-bit version */
cb = emit_code(gc, cb, mov(gc, eax, ecx));
                if (hl2 < 16)
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16 - hl2)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, eax, ecx));
                if (wl2 < 16)
cb = emit_code(gc, cb, shr(gc, eax, value(gc, 16 - wl2)));
        }

        /* EAX now holds (unscaled) offset into texture */

        baseFormat = current->texelFormat;
        if ((texelFlags & (__FR_TEXEL_HAS_INDEX|__FR_TEXEL_NEED_RGB)) == 
            (__FR_TEXEL_HAS_INDEX|__FR_TEXEL_NEED_RGB)) {
                /* Indirect, turn it into offset in palette (which looks like
                 * a 256x1 texture).
                 */
cb = emit_code(gc, cb, add(gc, eax, mem(gc, value(gc, offsetof(tp, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, ebx, eax));
cb = emit_code(gc, cb, mov(gc, eax, value(gc, 0)));
cb = emit_code(gc, cb, mov(gc, al, mem(gc, NULL, ebx, NULL, 1)));
                /* EAX now holds (unscaled) offset into palette */
                baseFormat = current->CT.baseFormat;
        }

        switch (baseFormat) {
        case GL_COLOR_INDEX:
cb = emit_code(gc, cb, mov(gc, bl, mem(gc, NULL, esi, eax, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), bl));
                break;

        case GL_LUMINANCE:
cb = emit_code(gc, cb, mov(gc, bl, mem(gc, NULL, esi, eax, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), bl));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 1), edi, NULL, 1), bl));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), bl));
                break;

        case GL_INTENSITY:
cb = emit_code(gc, cb, mov(gc, bl, mem(gc, NULL, esi, eax, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), bl));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 1), edi, NULL, 1), bl));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), bl));
                if (dest_width == 4) { /* need_alpha */
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 3), edi, NULL, 1), bl));
                }
                break;

        case GL_LUMINANCE_ALPHA:
                if (dest_width == 4) { /* need_alpha */
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, NULL, esi, eax, 2)));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), bl));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 1), edi, NULL, 1), bl));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), bl));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 3), edi, NULL, 1), bh));
                } else {
cb = emit_code(gc, cb, mov(gc, bl, mem(gc, NULL, esi, eax, 2)));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), bl));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 1), edi, NULL, 1), bl));
cb = emit_code(gc, cb, mov(gc, mem(gc, value(gc, 2), edi, NULL, 1), bl));
                }
                break;
       
        case GL_RGB:
cb = emit_code(gc, cb, lea(gc, eax, mem(gc, NULL, eax, eax, 2)));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, NULL, esi, eax, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), ebx));
                break;

        case GL_RGBA:
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, NULL, esi, eax, 4)));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), ebx));
                break;
        case GL_ALPHA:
cb = emit_code(gc, cb, mov(gc, bl, mem(gc, NULL, esi, eax, 1)));
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, edi, NULL, 1), bl));
                break;
        }

cb = emit_code(gc, cb, add(gc, edi, value(gc, dest_width)));

cb = emit_code(gc, cb, shl(gc, ebp, value(gc, 1)));
cb = emit_code(gc, cb, add(gc, ecx, fptr->dstdx));
cb = emit_code(gc, cb, cmp(gc, ebp, value(gc, 0)));
cb = emit_code(gc, cb, jl(gc, top0));

cb = emit_code(gc, cb, cmp(gc, ebp, value(gc, 0)));
cb = emit_code(gc, cb, je(gc, done));

maskpos->spec.label.where = cb; resolve(gc, maskpos, cb);

cb = emit_code(gc, cb, shl(gc, ebp, value(gc, 1)));
cb = emit_code(gc, cb, add(gc, ecx, fptr->dstdx));
cb = emit_code(gc, cb, jmp(gc, top));

done->spec.label.where = cb; resolve(gc, done, cb);

cb = emit_code(gc, cb, pop(gc, ebp));
cb = emit_code(gc, cb, xor(gc, eax, eax));
cb = emit_code(gc, cb, shl(gc, ebp, value(gc, 16)));

cb = emit_code(gc, cb, test(gc, ebp, ebp));
        if (texelFlags & __FR_TEXEL_PERSPECTIVE) {
cb = emit_code(gc, cb, jne(gc, n1));
cb = emit_code(gc, cb, jmp(gc, alldone));
n1->spec.label.where = cb; resolve(gc, n1, cb);
cb = emit_code(gc, cb, push(gc, eax));

cb = emit_code(gc, cb, mov(gc, eax, fptr->is));
cb = emit_code(gc, cb, mov(gc, fptr->prev_is, eax));
    if (two_d) {
cb = emit_code(gc, cb, mov(gc, eax, fptr->it));
cb = emit_code(gc, cb, mov(gc, fptr->prev_it, eax));
    }
cb = pcstep(gc, cb, wl2, hl2, two_d, fptr);
cb = pcstepnext(gc, cb, wl2, hl2, two_d, fptr);

cb = emit_code(gc, cb, mov(gc, ecx, fptr->ist));
cb = emit_code(gc, cb, jmp(gc, top));
        } else {
cb = emit_code(gc, cb, jne(gc, n1));
cb = emit_code(gc, cb, jmp(gc, alldone));
n1->spec.label.where = cb; resolve(gc, n1, cb);
cb = emit_code(gc, cb, push(gc, eax));
cb = emit_code(gc, cb, jmp(gc, top));
        }

alldone->spec.label.where = cb; resolve(gc, alldone, cb);

        return cb;
}

__GLspanlet OGGenerateTR(__GLcontext *gc, int texelFlags, int wl2, int hl2)
{
    __GLtexture *current = gc->texture.currentTexture[gc->texture.currentTexUnit];
    int texel_width, dest_width;
    char *cb, *code;
    unsigned int signature;
    int two_d = (current->dim == 2);
    struct __GLfptri fptr;

    /* Build unique texture signature for this procedure:
     * bits  0..15      internal format
     * bits 16..20      texelFlags (low three bits are ignored)
     * bits 21..22      dimensions (e.g. TEXTURE_?D)
     * bits 23..26      log2(width)
     * bits 27..30      log2(height)
     * bit  31          unused
     */
    signature =
        current->texelFormat |
        ((texelFlags>>4)<<16) |
        (current->dim << 21) |
        (wl2 << 23) |
        (hl2 << 27);

    if (__glAllocCodeSpace(&gc->textureReadCache, 0, signature, 0, &code)) {
        return (__GLspanlet) code;
    }

    cb = code;

#ifdef __GL_CODEGEN_STATS
    __glSTSectionBegin(9, "TR Generator");

    stats.num_textureread_changes++;
#endif

    og_warmup(gc);
    this_warmup(gc, &fptr, (float *)(((int)gc->triSetupData + 7) & ~7));

    if (texelFlags & __FR_TEXEL_NEED_RGB) {

        if (texelFlags & __FR_TEXEL_HAS_RGB) {
            texel_width = 3;
        } else {
            texel_width = 1;
        }

        if (texelFlags & __FR_TEXEL_HAS_ALPHA) {
            texel_width += 1;
        }

        if (texelFlags & __FR_TEXEL_NEED_ALPHA)
            dest_width = 4;
        else if (current->texelFormat != GL_ALPHA)
            dest_width = 3;
        else
            dest_width = 1;

    } else {
        texel_width = 1;
        dest_width = 1;
    }

cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, ecx));
cb = emit_code(gc, cb, push(gc, esi));
cb = emit_code(gc, cb, push(gc, edi));
cb = emit_code(gc, cb, push(gc, ebp));

cb = emit_code(gc, cb, mov(gc, esi, ecx));

        /* Texture setup: load ecx with starting s,t, dstdx
         *      with change per pixel.
         */
        if (texelFlags & __FR_TEXEL_PERSPECTIVE) {
            if (two_d) {
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, offsetof(ftw, __GLtri)), edx, NULL, 1)));
            }
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, offsetof(fsw, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, fld(gc, mem(gc, value(gc, offsetof(fqw, __GLtri)), edx, NULL, 1)));

cb = emit_code(gc, cb, fld(gc, st0));

cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));

cb = pcstep(gc, cb, wl2, hl2, two_d, &fptr);

cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));

            if (two_d) {
cb = emit_code(gc, cb, mov(gc, eax, fptr.qit));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
            }
cb = emit_code(gc, cb, mov(gc, eax, fptr.qis));
cb = emit_code(gc, cb, mov(gc, fptr.ist, eax));

cb = emit_code(gc, cb, mov(gc, eax, fptr.is));
cb = emit_code(gc, cb, mov(gc, fptr.prev_is, eax));
            if (two_d) {
cb = emit_code(gc, cb, mov(gc, eax, fptr.it));
cb = emit_code(gc, cb, mov(gc, fptr.prev_it, eax));
            }
cb = pcstep(gc, cb, wl2, hl2, two_d, &fptr);

            /* Now need to calculate ist for the second span */
            if (two_d) {
cb = emit_code(gc, cb, mov(gc, eax, fptr.qit));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
            }
cb = emit_code(gc, cb, mov(gc, eax, fptr.qis));

cb = emit_code(gc, cb, mov(gc, ecx, fptr.ist));
cb = emit_code(gc, cb, mov(gc, fptr.ist, eax));

cb = pcstepnext(gc, cb, wl2, hl2, two_d, &fptr);

cb = emit_code(gc, cb, fdivr(gc, fpk(gc, 1.0)));
        } else {
            if (two_d) {
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, offsetof(dtdx, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, 16 - hl2)));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, offsetof(dsdx, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, wl2)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, ecx, eax));

cb = emit_code(gc, cb, mov(gc, eax, ecx));
cb = emit_code(gc, cb, and(gc, eax, value(gc, 0x8000)));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 1)));
cb = emit_code(gc, cb, sub(gc, ecx, eax));

cb = emit_code(gc, cb, mov(gc, fptr.dstdx, ecx));

cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, offsetof(t, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, shl(gc, ecx, value(gc, 16 - hl2)));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, offsetof(s, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, shr(gc, eax, value(gc, wl2)));
cb = emit_code(gc, cb, size16(gc));
cb = emit_code(gc, cb, mov(gc, ecx, eax));
            } else {
cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, offsetof(dsdx, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, shr(gc, ecx, value(gc, wl2)));
cb = emit_code(gc, cb, mov(gc, fptr.dstdx, ecx));

cb = emit_code(gc, cb, mov(gc, ecx, mem(gc, value(gc, offsetof(s, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, shr(gc, ecx, value(gc, wl2)));
            }
        }

cb = emit_code(gc, cb, mov(gc, ebp, esi));

        /* Load the texture pointer into ESI */
        if ((texelFlags & (__FR_TEXEL_HAS_INDEX | __FR_TEXEL_NEED_RGB)) == 
            (__FR_TEXEL_HAS_INDEX | __FR_TEXEL_NEED_RGB)) {
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, offsetof(gc, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, offsetof(texture.currentTexture, __GLcontext)), esi, NULL, 1)));
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, offsetof(CT.table, __GLtexture)), esi, NULL, 1)));
        } else {
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, offsetof(tp, __GLtri)), edx, NULL, 1)));
        }
cb = emit_code(gc, cb, lea(gc, edi, mem(gc, value(gc, offsetof(texelBuf, __GLtri)), edx, NULL, 1)));

        cb = GenerateInner(gc, cb, current, texelFlags, wl2, hl2,
                           texel_width, dest_width, two_d, &fptr);

cb = emit_code(gc, cb, pop(gc, ebp));
cb = emit_code(gc, cb, pop(gc, edi));
cb = emit_code(gc, cb, pop(gc, esi));
cb = emit_code(gc, cb, pop(gc, ecx));
cb = emit_code(gc, cb, pop(gc, ebx));

        if (texelFlags & __FR_TEXEL_PERSPECTIVE) {
            if (two_d) {
cb = emit_code(gc, cb, fstp(gc, st0));
            }
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fstp(gc, st0));
cb = emit_code(gc, cb, fstp(gc, st0));
        }

cb = emit_code(gc, cb, ret(gc));

#ifdef __GL_CODEGEN_STATS
    __glSTSectionEnd(9);
#endif /* __GL_CODEGEN_STATS */

    assert((unsigned)(cb - code) <= gc->constants.textureReadCacheSize);
    return (__GLspanlet) code;
}

/* Depth test signature bits */
#define __OG_ZTEST_ZFUNC        0x7
#define __OG_ZTEST_STENCIL      (1<<3)
#define __OG_ZTEST_WRITE        (1<<4)
#define __OG_ZTEST_16BIT        (1<<5)
#define __OG_ZTEST_OFFSET       (1<<6)

__GLspanlet OGGenerateZT(__GLcontext *gc)
{
    unsigned char *cb;
    unsigned char *code;
    struct celem *top0, *top, *zfail, *step, *maskpos, *done, *allfailed,
        *noclamp;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLboolean always = (gc->state.depth.testFunc == GL_ALWAYS);
    GLboolean z16bit = (gc->depthBuffer.buf.depth == 16);
    GLboolean stencil = (0 != (modeFlags & __GL_SHADE_STENCIL_TEST));
    GLboolean offset = (0 != (modeFlags & __GL_SHADE_POLYGON_OFFSET_FILL));
    GLuint signature;

    /* Build signature */
    signature = gc->state.depth.testFunc & __OG_ZTEST_ZFUNC;
    if (stencil)
        signature |= __OG_ZTEST_STENCIL;
    if (gc->state.depth.writeEnable)
        signature |= __OG_ZTEST_WRITE;
    if (z16bit)
        signature |= __OG_ZTEST_16BIT;
    if (offset)
        signature |= __OG_ZTEST_OFFSET;

    if (__glAllocCodeSpace(&gc->depthTestCache, 0, signature, 0, &code)) {
        /* Found existing depth test which satisfies request! */
        return (__GLspanlet) code;
    }

    cb = code;

    og_warmup(gc);

    top0 = label(gc);
    top = label(gc);
    zfail = label(gc);
    step = label(gc);
    maskpos = label(gc);
    done = label(gc);
    allfailed = label(gc);
    if (offset) {
        noclamp = label(gc);
    }

    /* Picking should eliminate this case */
    assert(gc->state.depth.writeEnable || !always);

    if (gc->state.depth.testFunc == GL_NEVER) {
        if (stencil) {
cb = emit_code(gc, cb, mov(gc, ecx, value(gc, 0)));
cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, offsetof(gc, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, jmpi(gc, mem(gc, value(gc, offsetof(procs.afterDepthTest, __GLcontext)), eax, NULL, 1)));
        } else {
cb = emit_code(gc, cb, ret(gc));
        }
    } else {

cb = emit_code(gc, cb, push(gc, ebx));
cb = emit_code(gc, cb, push(gc, esi));
        if (!always) {
cb = emit_code(gc, cb, push(gc, edi));
        }
cb = emit_code(gc, cb, push(gc, ebp));

        /*
         * eax  mask
         * ebx  z
         * ecx  maskout
         * edx  tr
         * esi  zp
         * edi  bit
         * ebp  *
         */

cb = emit_code(gc, cb, mov(gc, eax, ecx));
cb = emit_code(gc, cb, mov(gc, ebx, mem(gc, value(gc, offsetof(z, __GLtri)), edx, NULL, 1)));
        if (!always) {
cb = emit_code(gc, cb, xor(gc, ecx, value(gc, ~0)));
cb = emit_code(gc, cb, mov(gc, edi, value(gc, 0x80000000)));
        }
cb = emit_code(gc, cb, mov(gc, esi, mem(gc, value(gc, offsetof(zp, __GLtri)), edx, NULL, 1)));

top0->spec.label.where = cb; resolve(gc, top0, cb);

cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 0)));
cb = emit_code(gc, cb, jge(gc, maskpos));

top->spec.label.where = cb; resolve(gc, top, cb);

cb = emit_code(gc, cb, mov(gc, ebp, ebx));
        if (offset) {
cb = emit_code(gc, cb, sub(gc, ebp, value(gc, 0x80000000)));
cb = emit_code(gc, cb, add(gc, ebp, mem(gc, value(gc, offsetof(depthOffset, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, jno(gc, noclamp));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, offsetof(depthOffset, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, sar(gc, ebp, value(gc, 31)));
cb = emit_code(gc, cb, xor(gc, ebp, value(gc, 0x7fffffff)));
noclamp->spec.label.where = cb; resolve(gc, noclamp, cb);
cb = emit_code(gc, cb, add(gc, ebp, value(gc, 0x80000000)));
        }
        if (z16bit) {
cb = emit_code(gc, cb, shr(gc, ebp, value(gc, 16)));
cb = emit_code(gc, cb, size16(gc));
        }
        if (!always) {
cb = emit_code(gc, cb, cmp(gc, ebp, mem(gc, NULL, esi, NULL, 1)));
            switch (gc->state.depth.testFunc) {
            case GL_LESS:
cb = emit_code(gc, cb, jae(gc, zfail));
                break;
            case GL_EQUAL:
cb = emit_code(gc, cb, jne(gc, zfail));
                break;
            case GL_LEQUAL:
cb = emit_code(gc, cb, ja(gc, zfail));
                break;
            case GL_GREATER:
cb = emit_code(gc, cb, jbe(gc, zfail));
                break;
            case GL_NOTEQUAL:
cb = emit_code(gc, cb, je(gc, zfail));
                break;
            case GL_GEQUAL:
cb = emit_code(gc, cb, jb(gc, zfail));
                break;
            default:
                assert(0);
                break;
            }
        }
        if (gc->state.depth.writeEnable) {
            if (z16bit) {
cb = emit_code(gc, cb, size16(gc));
            }
cb = emit_code(gc, cb, mov(gc, mem(gc, NULL, esi, NULL, 1), ebp));
        }
        if (!always) {
cb = emit_code(gc, cb, jmp(gc, step));

zfail->spec.label.where = cb; resolve(gc, zfail, cb);

cb = emit_code(gc, cb, or(gc, ecx, edi));

step->spec.label.where = cb; resolve(gc, step, cb);

cb = emit_code(gc, cb, shr(gc, edi, value(gc, 1)));
        }
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, offsetof(dx, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 1)));
        if (z16bit) {
cb = emit_code(gc, cb, lea(gc, esi, mem(gc, NULL, esi, ebp, 2)));
        } else {
cb = emit_code(gc, cb, lea(gc, esi, mem(gc, NULL, esi, ebp, 4)));
        }
cb = emit_code(gc, cb, add(gc, ebx, mem(gc, value(gc, offsetof(dzdx, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, cmp(gc, eax, value(gc, 0)));
cb = emit_code(gc, cb, jl(gc, top));

maskpos->spec.label.where = cb; resolve(gc, maskpos, cb);

        /* if (mask == 0) break; */
cb = emit_code(gc, cb, test(gc, eax, eax));
cb = emit_code(gc, cb, je(gc, done));

        /* Step interpolants */
        if (!always) {
cb = emit_code(gc, cb, shr(gc, edi, value(gc, 1)));
        }
cb = emit_code(gc, cb, shl(gc, eax, value(gc, 1)));
cb = emit_code(gc, cb, mov(gc, ebp, mem(gc, value(gc, offsetof(dx, __GLtri)), edx, NULL, 1)));
        if (z16bit) {
cb = emit_code(gc, cb, lea(gc, esi, mem(gc, NULL, esi, ebp, 2)));
        } else {
cb = emit_code(gc, cb, lea(gc, esi, mem(gc, NULL, esi, ebp, 4)));
        }
cb = emit_code(gc, cb, add(gc, ebx, mem(gc, value(gc, offsetof(dzdx, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, jmp(gc, top0));

done->spec.label.where = cb; resolve(gc, done, cb);

cb = emit_code(gc, cb, pop(gc, ebp));
        if (!always) {
cb = emit_code(gc, cb, pop(gc, edi));
        }
cb = emit_code(gc, cb, pop(gc, esi));
cb = emit_code(gc, cb, pop(gc, ebx));

        if (!always) {
cb = emit_code(gc, cb, xor(gc, ecx, value(gc, ~0)));
            if (!stencil) {
cb = emit_code(gc, cb, je(gc, allfailed));
            }
        }

cb = emit_code(gc, cb, mov(gc, eax, mem(gc, value(gc, offsetof(gc, __GLtri)), edx, NULL, 1)));
cb = emit_code(gc, cb, jmpi(gc, mem(gc, value(gc, offsetof(procs.afterDepthTest, __GLcontext)), eax, NULL, 1)));

allfailed->spec.label.where = cb; resolve(gc, allfailed, cb);

cb = emit_code(gc, cb, ret(gc));
    }

    assert((unsigned)(cb - code) <= gc->constants.depthTestCacheSize);
    return (__GLspanlet) code;
}

/************************************************************************/


#ifdef __GL_CODEGEN_STATS
#include "timer.h"

static struct {
  double started;
  double total;
  int num;
  char* name;
} timer[50];

extern double readmachineclock();

static void dumptimers(void)
{
  FILE* f;
  int i;
  double total;

  f = fopen("timers.log", "wt");

  for (i = 0; i < 1; i++) {
    if (timer[i].started)
      __glSTSectionEnd(i);
  }
  total = timer[0].total;

#define REPORT(X)  fprintf(f, "%30s %d\n", #X, stats.X)

  REPORT(num_texture_reads);
  REPORT(num_textureread_changes);

  fprintf(f, "Sample duration: %.1f seconds\n\n", total / 166000000.0);

  for (i = 1; i < 10; i++) {
    if (timer[i].num)
      fprintf(f, "% 5.1f % 7.0f [%d] %s\n", 
              100.0 * (timer[i].total / total),
              timer[i].total / timer[i].num,
              i,
              timer[i].name);
  }

  fclose(f);
}

void __glSTSectionBegin(int ident, char* name)
{
  static int cold = 1;

  if (cold) {
    cold = 0;
    atexit(dumptimers);
    __glSTSectionBegin(0, "Total");
  }
  timer[ident].started = readmachineclock();
  timer[ident].name = name;
}

void __glSTSectionEnd(int ident)
{
    timer[ident].total += (readmachineclock() - timer[ident].started);
    timer[ident].num++;
    timer[ident].started = 0.0;
}

#endif /* __GL_CODEGEN_STATS */

#endif /* __GL_CODEGEN */

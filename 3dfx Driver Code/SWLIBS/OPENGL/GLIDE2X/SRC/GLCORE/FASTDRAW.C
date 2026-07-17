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
** $Date: 10/11/00 7:50:26 PM$
*/
/*
 * fastdraw.c
 *
 * Optimized glDrawPixels and glBitmap functions.
 */


#include "context.h"
#include "global.h"
#include "imports.h"
#include "pixel.h"



/*
 * glDrawPixels for 8-bit CI mode.
 * zoomx = zoomy = +/-1.0
 * No raster ops (dither, depth test, etc)
 * No pixel transfer ops (scale, bias, mapping)
 */
void __glDrawPixels_FastCI8(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    GLubyte *fp, *src;
    GLint destCol, destRow, destRowInc;
    GLint height, width;
    GLint i, j;

    __GL_LOCK_BUFFERS(gc);

    destRow = spanInfo->startRow;
    if (spanInfo->zoomy<0.0f) {
       destRowInc = -1;
    }
    else {
       destRowInc = 1;
    }

    height = spanInfo->height;
    if (spanInfo->endCol > spanInfo->startCol) {
       /* zoomx > 0 */
        width = spanInfo->endCol - spanInfo->startCol;
        destCol = spanInfo->startCol;

        /* get pointer to first src fragment */
        src = (GLubyte *) spanInfo->srcCurrent;

        for (i=0; i<height; i++) {
           fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLubyte *),
                                 destCol, destRow );

           __GL_MEMCOPY( fp, src, width );

           src += spanInfo->srcRowIncrement;
           destRow += destRowInc;
        }
    }
    else {
        /* zoomx < 0 */
        width = spanInfo->startCol - spanInfo->endCol;
        destCol = spanInfo->startCol;

        /* get pointer to first src fragment */
        src = (GLubyte *) spanInfo->srcCurrent;

        for (i=0; i<height; i++) {
           fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLubyte *),
                                 destCol, destRow );

           for (j=0;j<width;j++) {
               *fp = src[j];
               fp--;
           }

           src += spanInfo->srcRowIncrement;
           destRow += destRowInc;
        }
    }

    __GL_UNLOCK_BUFFERS(gc);
}



/*
 * glDrawPixels for 16-bit RGB mode.
 * zoomx = zoomy = +/-1.0
 * No raster ops (dither, depth test, etc)
 * No pixel transfer ops (scale, bias, mapping)
 */
void __glDrawPixels_FastRGB16(__GLcontext *gc, __GLpixelSpanInfo *spanInfo)
{
    __GLcolorBuffer *cfb = gc->drawBuffer;
    GLubyte *src, *srcrow;
    GLushort *fp;
    GLint destCol, destRow, destRowInc;
    GLint height, width;
    GLint i, j;
    GLint rRightShift = 8 - gc->modes.redBits;
    GLint gRightShift = 8 - gc->modes.greenBits;
    GLint bRightShift = 8 - gc->modes.blueBits;
    GLint rLeftShift = cfb->redShift;
    GLint gLeftShift = cfb->greenShift;
    GLint bLeftShift = cfb->blueShift;

    __GL_LOCK_BUFFERS(gc);

    destRow = spanInfo->startRow;
    if (spanInfo->zoomy<0.0f) {
       destRowInc = -1;
    }
    else {
       destRowInc = 1;
    }

    height = spanInfo->height;
    if (spanInfo->endCol > spanInfo->startCol) {
       /* zoomx > 0 */
        width = spanInfo->endCol - spanInfo->startCol;
        destCol = spanInfo->startCol;

        /* get pointer to first src fragment */
        srcrow = (GLubyte *) spanInfo->srcCurrent;

        for (i=0; i<height; i++) {
           fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLushort *),
                                 destCol, destRow );

           src = srcrow;
           for (j=0;j<width;j++) {
              GLint r = *src++ >> rRightShift;
              GLint g = *src++ >> gRightShift;
              GLint b = *src++ >> bRightShift;
              *fp = (r << rLeftShift) |
                    (g << gLeftShift) |
                    (b << bLeftShift);
              fp++;
           }

           srcrow += spanInfo->srcRowIncrement;
           destRow += destRowInc;
        }
    }
    else {
        /* zoomx < 0 */
        width = spanInfo->startCol - spanInfo->endCol;
        destCol = spanInfo->startCol;

        /* get pointer to first src fragment */
        srcrow = (GLubyte *) spanInfo->srcCurrent;

        for (i=0; i<height; i++) {
           fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLushort *),
                                 destCol, destRow );

           src = srcrow;
           for (j=0;j<width;j++) {
              GLint r = *src++ >> rRightShift;
              GLint g = *src++ >> gRightShift;
              GLint b = *src++ >> bRightShift;
              *fp = (r << rLeftShift) |
                    (g << gLeftShift) |
                    (b << bLeftShift);
              fp--;
           }

           srcrow += spanInfo->srcRowIncrement;
           destRow += destRowInc;
        }
    }

    __GL_UNLOCK_BUFFERS(gc);
}



static GLint __glIntDitherTable[16] = {
    0 *17, 8 *17, 2 *17, 10*17,
    12*17, 4 *17, 14*17, 6 *17,
    3 *17, 11*17, 1 *17, 9 *17,
    15*17, 7 *17, 13*17, 5 *17,
};


/*
 * glDrawPixels for 16-bit RGB mode with dithering.
 * zoomx = zoomy = +/-1.0
 * Only dither, no other raster ops (depth test, etc)
 * No pixel transfer ops (scale, bias, mapping)
 */
void __glDrawPixels_FastRGB16_Dither(__GLcontext *gc,
                                     __GLpixelSpanInfo *spanInfo)
{
    __GLcolorBuffer *cfb = gc->drawBuffer;
    GLubyte *src, *srcrow;
    GLushort *fp;
    GLint destCol, destRow, destRowInc;
    GLint height, width;
    GLint i, j;
    __GLfloat rscale = cfb->redScale / 255.0f;
    __GLfloat gscale = cfb->greenScale / 255.0f;
    __GLfloat bscale = cfb->blueScale / 255.0f;
    GLint rLeftShift = cfb->redShift;
    GLint gLeftShift = cfb->greenShift;
    GLint bLeftShift = cfb->blueShift;
    GLint rMax = cfb->iRedScale;
    GLint gMax = cfb->iGreenScale;
    GLint bMax = cfb->iBlueScale;
    GLint rBits = gc->modes.redBits;
    GLint gBits = gc->modes.greenBits;
    GLint bBits = gc->modes.blueBits;


    __GL_LOCK_BUFFERS(gc);

    destRow = spanInfo->startRow;
    if (spanInfo->zoomy<0.0f) {
       destRowInc = -1;
    }
    else {
       destRowInc = 1;
    }

    height = spanInfo->height;
    if (spanInfo->endCol > spanInfo->startCol) {
       /* zoomx > 0 */
        width = spanInfo->endCol - spanInfo->startCol;
        destCol = spanInfo->startCol;

        /* get pointer to first src fragment */
        srcrow = (GLubyte *) spanInfo->srcCurrent;

        for (i=0; i<height; i++) {
           fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLushort *),
                                 destCol, destRow );

           src = srcrow;
           for (j=0;j<width;j++) {
              GLint ix = __GL_DITHER_INDEX(destCol+j, destRow);
              GLint inc = __glIntDitherTable[ix];
              GLint r = *src++;
              GLint g = *src++;
              GLint b = *src++;
              /* NOTE: (X << S) - X  =  X * (2^S - 1) */
              r = (((r << rBits) - r) + inc) >> 8;
              g = (((g << gBits) - g) + inc) >> 8;
              b = (((b << bBits) - b) + inc) >> 8;
              *fp = (r << rLeftShift) |
                    (g << gLeftShift) |
                    (b << bLeftShift);
              fp++;
           }

           srcrow += spanInfo->srcRowIncrement;
           destRow += destRowInc;
        }
    }
    else {
        /* zoomx < 0 */
        width = spanInfo->startCol - spanInfo->endCol;
        destCol = spanInfo->startCol;

        /* get pointer to first src fragment */
        srcrow = (GLubyte *) spanInfo->srcCurrent;

        for (i=0; i<height; i++) {
           fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLushort *),
                                 destCol, destRow );

           src = srcrow;
           for (j=0;j<width;j++) {
              GLint ix = __GL_DITHER_INDEX(destCol-j, destRow);
              GLint inc = __glIntDitherTable[ix];
              GLint r = *src++;
              GLint g = *src++;
              GLint b = *src++;
              /* NOTE: (X << S) - X  =  X * (2^S - 1) */
              r = (((r << rBits) - r) + inc) >> 8;
              g = (((g << gBits) - g) + inc) >> 8;
              b = (((b << bBits) - b) + inc) >> 8;
              *fp = (r << rLeftShift) |
                    (g << gLeftShift) |
                    (b << bLeftShift);
              fp--;
           }

           srcrow += spanInfo->srcRowIncrement;
           destRow += destRowInc;
        }
    }

    __GL_UNLOCK_BUFFERS(gc);
}




/*
 * glDrawPixels for 16-bit RGBA/ABGR/BGRA images with Alpha testing.
 * zoomx = zoomy = +/-1.0
 * Alpha test, no other raster ops
 * No pixel transfer ops (scale, bias, mapping)
 *
 * Note:  Luckily, the size of the alpha test table is 256, the same as the
 * range of alpha values in a GL_UBYTE image!
 */
void __glDrawPixels_FastRGBA16_Atest(__GLcontext *gc,
                                     __GLpixelSpanInfo *spanInfo)
{
    __GLcolorBuffer *cfb = gc->drawBuffer;
    GLubyte *src, *srcrow;
    GLushort *fp;
    GLint destCol, destRow, destRowInc;
    GLint height, width;
    GLint i, j;
    GLint rRightShift = 8 - gc->modes.redBits;
    GLint gRightShift = 8 - gc->modes.greenBits;
    GLint bRightShift = 8 - gc->modes.blueBits;
    GLint rLeftShift = cfb->redShift;
    GLint gLeftShift = cfb->greenShift;
    GLint bLeftShift = cfb->blueShift;
    GLint rpos, gpos, bpos, apos;
    GLubyte *atft = &gc->frontBuffer.alphaTestFuncTable[0];

    if (spanInfo->srcFormat==GL_RGBA) {
       rpos = 0;  gpos = 1;  bpos = 2;  apos = 3;
    }
    else if (spanInfo->srcFormat==GL_ABGR_EXT) {
       rpos = 3;  gpos = 2;  bpos = 1;  apos = 0;
    }
    else if (spanInfo->srcFormat==GL_BGRA_EXT) {
       rpos = 2;  gpos = 1;  bpos = 0;  apos = 3;
    }
    else {
       abort();
    }


    __GL_LOCK_BUFFERS(gc);

    destRow = spanInfo->startRow;
    if (spanInfo->zoomy<0.0f) {
       destRowInc = -1;
    }
    else {
       destRowInc = 1;
    }

    height = spanInfo->height;
    if (spanInfo->endCol > spanInfo->startCol) {
       /* zoomx > 0 */
        width = spanInfo->endCol - spanInfo->startCol;
        destCol = spanInfo->startCol;

        /* get pointer to first src fragment */
        srcrow = (GLubyte *) spanInfo->srcCurrent;

        for (i=0; i<height; i++) {
           fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLushort *),
                                 destCol, destRow );

           src = srcrow;
           for (j=0;j<width;j++) {
              if (atft[src[apos]]) {
                 /* passed alpha test */
                 GLint r = src[rpos] >> rRightShift;
                 GLint g = src[gpos] >> gRightShift;
                 GLint b = src[bpos] >> bRightShift;
                 *fp = (r << rLeftShift) |
                       (g << gLeftShift) |
                       (b << bLeftShift);
              }
              src += 4;
              fp++;
           }

           srcrow += spanInfo->srcRowIncrement;
           destRow += destRowInc;
        }
    }
    else {
        /* zoomx < 0 */
        width = spanInfo->startCol - spanInfo->endCol;
        destCol = spanInfo->startCol;

        /* get pointer to first src fragment */
        srcrow = (GLubyte *) spanInfo->srcCurrent;

        for (i=0; i<height; i++) {
           fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLushort *),
                                 destCol, destRow );

           src = srcrow;
           for (j=0;j<width;j++) {
              if (atft[src[apos]]) {
                 /* passed alpha test */
                 GLint r = src[rpos] >> rRightShift;
                 GLint g = src[gpos] >> gRightShift;
                 GLint b = src[bpos] >> bRightShift;
                 *fp = (r << rLeftShift) |
                       (g << gLeftShift) |
                       (b << bLeftShift);
              }
              src += 4;
              fp--;
           }

           srcrow += spanInfo->srcRowIncrement;
           destRow += destRowInc;
        }
    }

    __GL_UNLOCK_BUFFERS(gc);
}



/**********************************************************************/
/*****                     Bitmap functions                       *****/
/**********************************************************************/



/*
 * glBitmap for 16-bit RGB color mode.
 * No raster ops (depth test, dither, etc).
 * Texture and fog is supported though.
 */
void __glRenderBitmap_FastRGB16(__GLcontext *gc, const __GLbitmap *bitmap,
                                const GLubyte *data)
{
    __GLcolorBuffer *cfb = gc->drawBuffer;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLfragment frag;
    __GLvertex *rp;
    GLint x, y, w, h;
    GLubyte bitmask, firstmask;
    GLint ySign;
    GLushort pixel;
    GLint skipBits, nextRow;

    ySign = gc->constants.ySign;

    /*
    ** Check if current raster position is valid.  Do not render if invalid.
    ** Also, if selection is in progress skip the rendering of the
    ** bitmap.  Bitmaps are invisible to selection and do not generate
    ** selection hits.
    */
    rp = &gc->state.current.rasterPos;
    if (!gc->state.current.validRasterPos) {
        return;
    }

    if (gc->renderMode == GL_SELECT) {
        rp->window.x += bitmap->xmove;
        rp->window.y += ySign * bitmap->ymove;
        return;
    }

    if (gc->renderMode == GL_FEEDBACK) {
        __glFeedbackBitmap(gc, rp);
        /*
        ** Advance the raster position as if the bitmap had been rendered.
        */
        rp->window.x += bitmap->xmove;
        rp->window.y += ySign * bitmap->ymove;
        return;
    }

    frag.color = *rp->color;
    if (modeFlags & __GL_SHADE_TEXTURE) {
        __GLfloat qInv = __glOne / rp->texture[0].w;
        (*gc->procs.textureRaster)(gc, &frag.color, rp->texture[0].x * qInv,
                               rp->texture[0].y * qInv, __glOne);
    }
    /* XXX - is this the correct test */
    if (gc->state.enables.general & __GL_FOG_ENABLE) {
        (*gc->procs.fogPoint)(gc, &frag, rp->eye.z);
    }

    frag.x = (GLint) (rp->window.x - __glHalf - bitmap->xorig);
    frag.y = (GLint) (rp->window.y - __glHalf - ySign * bitmap->yorig);
    frag.z = rp->window.z;

    /* compute pixel value to store */
    pixel = (((GLuint) (frag.color.r + 0.5F)) << cfb->redShift) |
            (((GLuint) (frag.color.g + 0.5F)) << cfb->greenShift) |
            (((GLuint) (frag.color.b + 0.5F)) << cfb->blueShift);

    /* advance current raster position */
    rp->window.x += bitmap->xmove;
    rp->window.y += ySign * bitmap->ymove;

    w = bitmap->width;
    h = bitmap->height;

    skipBits = 0;

    /* clip bitmap */
    if (ySign > 0) {
        if (frag.y < gc->transform.clipY0) {
            int dy = gc->transform.clipY0 - frag.y;
            h -= dy;
            if (h <= 0) return;

            /* skip rows */
            frag.y += dy;
            data += dy * ((bitmap->width + 7) >> 3);
        }
        if (gc->transform.clipY1 <= (frag.y + h)) {
            int dy = (frag.y + h) - gc->transform.clipY1;
            h -= dy;
            if (h <= 0) return;
        }
    } else {
        if ((frag.y - h) <  gc->transform.clipY0) {
            int dy = (gc->transform.clipY0 - 1) - (frag.y - h);
            h -= dy;
            if (h <= 0) return;
        }
        if (gc->transform.clipY1 <= frag.y) {
            int dy = frag.y - (gc->transform.clipY1 - 1);
            h -= dy;
            if (h <= 0) return;

            /* skip rows */
            frag.y -= dy;
            data += dy * ((bitmap->width + 7) >> 3);
        }
    }
    if (frag.x < gc->transform.clipX0) {
        int dx = gc->transform.clipX0 - frag.x;
        w -= dx;
        if (w <= 0) return;

        /* skip pixels */
        frag.x += dx;
        data += dx >> 3;
        skipBits = dx & 7;
    }
    if (gc->transform.clipX1 <= (frag.x + w)) {
        int dx = (frag.x + w) - gc->transform.clipX1;
        w -= dx;
        if (w <= 0) return;
    }

    firstmask = 0x80 >> skipBits;
    nextRow = 1 + ((bitmap->width + 7) >> 3) - ((w + skipBits + 7) >> 3);

    __GL_LOCK_BUFFERS(gc);

    bitmask = firstmask;
    for (y = 0; y < h; y++) {
        GLushort *fp;
        fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLushort *), frag.x, frag.y );

        for (x = 0; x < w; x++) {
            if (*data & bitmask) {
                /* store */
                *fp = pixel;
            }
            fp++;
            bitmask = bitmask >> 1;
            if (bitmask == 0) {
                bitmask = 0x80;
                data++;
            }
        }
        frag.y += ySign;
        if (bitmask == 0x80) {
            bitmask = firstmask;
            data += nextRow - 1;
        } else {
            bitmask = firstmask;
            data += nextRow;
        }
    }

    __GL_UNLOCK_BUFFERS(gc);
}



/*
 * glBitmap for 8-bit CI mode.
 * Texture, and fog are handled but no other raster ops
 */
void __glRenderBitmap_FastCI8(__GLcontext *gc, const __GLbitmap *bitmap,
                              const GLubyte *data)
{
    __GLcolorBuffer *cfb = gc->drawBuffer;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    __GLfragment frag;
    __GLvertex *rp;
    GLint x, y, w, h;
    GLubyte bitmask, firstmask;
    GLint ySign;
    GLubyte pixel;
    GLint skipBits, nextRow;

    ySign = gc->constants.ySign;

    /*
    ** Check if current raster position is valid.  Do not render if invalid.
    ** Also, if selection is in progress skip the rendering of the
    ** bitmap.  Bitmaps are invisible to selection and do not generate
    ** selection hits.
    */
    rp = &gc->state.current.rasterPos;
    if (!gc->state.current.validRasterPos) {
        return;
    }

    if (gc->renderMode == GL_SELECT) {
        rp->window.x += bitmap->xmove;
        rp->window.y += ySign * bitmap->ymove;
        return;
    }

    if (gc->renderMode == GL_FEEDBACK) {
        __glFeedbackBitmap(gc, rp);
        /*
        ** Advance the raster position as if the bitmap had been rendered.
        */
        rp->window.x += bitmap->xmove;
        rp->window.y += ySign * bitmap->ymove;
        return;
    }

    frag.color = *rp->color;
    if (modeFlags & __GL_SHADE_TEXTURE) {
        __GLfloat qInv = __glOne / rp->texture[0].w;
        (*gc->procs.textureRaster)(gc, &frag.color, rp->texture[0].x * qInv,
                               rp->texture[0].y * qInv, __glOne);
    }
    /* XXX - is this the correct test */
    if (gc->state.enables.general & __GL_FOG_ENABLE) {
        (*gc->procs.fogPoint)(gc, &frag, rp->eye.z);
    }

    frag.x = (GLint) (rp->window.x - __glHalf - bitmap->xorig);
    frag.y = (GLint) (rp->window.y - __glHalf - ySign * bitmap->yorig);
    frag.z = rp->window.z;

    /* compute pixel value to store */
    pixel = (GLubyte) (GLint) frag.color.r;

    /* advance current raster position */
    rp->window.x += bitmap->xmove;
    rp->window.y += ySign * bitmap->ymove;

    w = bitmap->width;
    h = bitmap->height;

    skipBits = 0;

    /* clip bitmap */
    if (ySign > 0) {
        if (frag.y < gc->transform.clipY0) {
            int dy = gc->transform.clipY0 - frag.y;
            h -= dy;
            if (h <= 0) return;

            /* skip rows */
            frag.y += dy;
            data += dy * ((bitmap->width + 7) >> 3);
        }
        if (gc->transform.clipY1 <= (frag.y + h)) {
            int dy = (frag.y + h) - gc->transform.clipY1;
            h -= dy;
            if (h <= 0) return;
        }
    } else {
        if ((frag.y - h) <  gc->transform.clipY0) {
            int dy = (gc->transform.clipY0 - 1) - (frag.y - h);
            h -= dy;
            if (h <= 0) return;
        }
        if (gc->transform.clipY1 <= frag.y) {
            int dy = frag.y - (gc->transform.clipY1 - 1);
            h -= dy;
            if (h <= 0) return;

            /* skip rows */
            frag.y -= dy;
            data += dy * ((bitmap->width + 7) >> 3);
        }
    }
    if (frag.x < gc->transform.clipX0) {
        int dx = gc->transform.clipX0 - frag.x;
        w -= dx;
        if (w <= 0) return;

        /* skip pixels */
        frag.x += dx;
        data += dx >> 3;
        skipBits = dx & 7;
    }
    if (gc->transform.clipX1 <= (frag.x + w)) {
        int dx = (frag.x + w) - gc->transform.clipX1;
        w -= dx;
        if (w <= 0) return;
    }

    firstmask = 0x80 >> skipBits;
    nextRow = 1 + ((bitmap->width + 7) >> 3) - ((w + skipBits + 7) >> 3);

    __GL_LOCK_BUFFERS(gc);

    bitmask = firstmask;
    for (y = 0; y < h; y++) {
        GLubyte *fp;
        fp = __GL_FB_ADDRESS( gc->drawBuffer, (GLubyte *), frag.x, frag.y );

        for (x = 0; x < w; x++) {
            if (*data & bitmask) {
                /* store */
                *fp = pixel;
            }
            fp++;
            bitmask = bitmask >> 1;
            if (bitmask == 0) {
                bitmask = 0x80;
                data++;
            }
        }
        frag.y += ySign;
        if (bitmask == 0x80) {
            bitmask = firstmask;
            data += nextRow - 1;
        } else {
            bitmask = firstmask;
            data += nextRow;
        }
    }

    __GL_UNLOCK_BUFFERS(gc);
}



/**********************************************************************/
/*****                Optimized glDrawPixels picker               *****/
/**********************************************************************/


/*
 * This function determines if an optimized glDrawPixels implementation
 * can be used.  If not, we call the __glSlowPickDrawPixels function.
 */
void __glOptPickDrawPixels(__GLcontext *gc, GLint width, GLint height,
                            GLenum format, GLenum type, const GLvoid *pixels,
                            GLboolean packed)
{
    GLboolean doSlowPick = GL_TRUE;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint cant_do = __GL_SHADE_DEPTH_TEST
                   | __GL_SHADE_STIPPLE
                   | __GL_SHADE_STENCIL_TEST
                 /*| __GL_SHADE_DITHER  can do this */
                   | __GL_SHADE_LOGICOP
                   | __GL_SHADE_BLEND
                /* | __GL_SHADE_ALPHA_TEST  can do this*/
                   | __GL_SHADE_INDEX_TEST
                   | __GL_SHADE_MASK
                   | __GL_SHADE_TEXTURE
                   | __GL_SHADE_OWNERSHIP_TEST
                   ;


    if (type!=GL_UNSIGNED_BYTE
        || (modeFlags & cant_do)
        || (gc->state.pixel.transferMode.zoomX!=1.0F
            && gc->state.pixel.transferMode.zoomX!=-1.0F)
        || (gc->state.pixel.transferMode.zoomY!=1.0F
            && gc->state.pixel.transferMode.zoomY!=-1.0F)
        || gc->state.pixel.transferMode.mapColor
        || gc->state.pixel.transferMode.r_bias!=__glZero
        || gc->state.pixel.transferMode.g_bias!=__glZero
        || gc->state.pixel.transferMode.b_bias!=__glZero
        || gc->state.pixel.transferMode.a_bias!=__glZero
        || gc->state.pixel.transferMode.r_scale!=__glOne
        || gc->state.pixel.transferMode.g_scale!=__glOne
        || gc->state.pixel.transferMode.b_scale!=__glOne
        || gc->state.pixel.transferMode.a_scale!=__glOne
        || gc->state.pixel.transferMode.indexOffset!=__glZero
        || gc->state.pixel.transferMode.indexShift!=__glZero ) {
       /* can't handle this rasterization mode */
       doSlowPick = GL_TRUE;
    }
    else {
        /* On our way to optimized glDrawPixels... */
        __GLpixelSpanInfo spanInfo;
    
        __glInitDrawPixelsInfo(gc, &spanInfo, width, height, format,
                               type, pixels);
        __glLoadUnpackModes(gc, &spanInfo, packed);
        if(!__glClipDrawPixels(gc, &spanInfo)) return;
        __glInitUnpacker(gc, &spanInfo);


        /* Determine which, if any, optmized glDrawPixels can be used */
        if (gc->modes.rgbMode && gc->drawBuffer->buf.elementSize==2) {
            /* 16-bit RGB mode */
            int atest = __GL_SHADE_ALPHA_TEST;
            if (modeFlags & __GL_SHADE_DITHER) {
                /* dithering enabled */
                if (modeFlags & __GL_SHADE_ALPHA_TEST) {
                    /* alpha test enabled */
                    /* DITHERED, ALPHA-TESTED NOT IMPLEMENTED */
                }
                else {
                    /* alpha test disabled */
                    if (format==GL_RGB) {
                        __glDrawPixels_FastRGB16_Dither( gc, &spanInfo );
                        return;
                    }
                }
            }
            else {
                /* dithering disabled */
                if (modeFlags & __GL_SHADE_ALPHA_TEST) {
                    /* alpha test enabled */
                    if (format==GL_RGBA || format==GL_BGRA_EXT
                        || format==GL_ABGR_EXT) {
                        __glDrawPixels_FastRGBA16_Atest( gc, &spanInfo );
                        return;
                    }
                }
                else {
                    /* alpha test disabled */
                    if (format==GL_RGB) {
                        __glDrawPixels_FastRGB16( gc, &spanInfo );
                        return;
                    }
                }
            }
        }
        else if (!gc->modes.rgbMode && gc->drawBuffer->buf.elementSize==1
                 && format==GL_COLOR_INDEX) {
            /* 8-bit CI mode */
            if (modeFlags & __GL_SHADE_DITHER) {
                /* dithering enabled */
                /* dithering is a no-op for GL_UBYTE */
                __glDrawPixels_FastCI8( gc, &spanInfo );
                return;                
            }
            else {
                /* dithering disabled */
                __glDrawPixels_FastCI8( gc, &spanInfo );
                return;                
            }
        }
    }

    if (doSlowPick) {
       __glSlowPickDrawPixels( gc, width, height, format, type,
                               pixels, packed );
    }
}




/*
 * Setup the gc->procs.renderBitmap pointer.
 *
 * This function checks if we can use an optimized bitmap function.
 * If not, call the regular __glGenericPickRenderBitmapProcs function.
 *
 */
void __glOptPickRenderBitmapProcs(__GLcontext *gc)
{
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLuint cant_do = __GL_SHADE_DEPTH_TEST
                   | __GL_SHADE_STIPPLE
                   | __GL_SHADE_STENCIL_TEST
                /* | __GL_SHADE_DITHER   can do this */
                   | __GL_SHADE_LOGICOP
                   | __GL_SHADE_BLEND
                   | __GL_SHADE_ALPHA_TEST
                   | __GL_SHADE_INDEX_TEST
                   | __GL_SHADE_MASK
                   | __GL_SHADE_OWNERSHIP_TEST
                   ;

    gc->procs.renderBitmap = NULL;

    if (modeFlags & cant_do) {
        /* can't handle this rasterization state */
    }
    else {
        if (gc->modes.rgbMode && gc->drawBuffer->buf.elementSize==2) {
            /* 16-bit RGB mode */
            if (modeFlags & __GL_SHADE_DITHER) {
                /* dithering enabled */
            }
            else {
                /* dithering disabled */
                gc->procs.renderBitmap = __glRenderBitmap_FastRGB16;
                return;
            }
        }
        else if (!gc->modes.rgbMode && gc->drawBuffer->buf.elementSize==1) {
            /* 8-bit CI mode */
            if (modeFlags & __GL_SHADE_DITHER) {
                /* dithering enabled */
            }
            else {
                /* dithering disabled */
                gc->procs.renderBitmap = __glRenderBitmap_FastCI8;
                return;
            }
        }
    }

    /* if we couldn't get an optimized glBitmap then fall back to this */
    if (!gc->procs.renderBitmap) {
       __glGenericPickRenderBitmapProcs( gc );
    }
}


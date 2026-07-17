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
** $Date: 10/11/00 7:55:58 PM$
*/
#include "render.h"
#include "context.h"
#include "global.h"
#include "image.h"

#define __GL_BITS_PER_UINT32 (sizeof(GLuint) * __GL_BITS_PER_BYTE)

void __glDrawBitmap(__GLcontext *gc, GLsizei width, GLsizei height,
                    GLfloat xOrig, GLfloat yOrig, 
                    GLfloat xMove, GLfloat yMove, 
                    const GLubyte oldbits[])
{
    __GLbitmap bitmap;

    bitmap.width = width;
    bitmap.height = height;
    bitmap.xorig = xOrig;
    bitmap.yorig = yOrig;
    bitmap.xmove = xMove;
    bitmap.ymove = yMove;

    /* 
    ** Could check the pixel transfer modes and see if we can maybe just 
    ** render oldbits directly rather than converting it first.
    */
    if (width > 0 && height > 0) {
        GLubyte *newbits;

        newbits = (GLubyte *) (*gc->imports.malloc)(gc, (size_t)
                                __glImageSize(width, height, GL_COLOR_INDEX, GL_BITMAP));
        
        __glFillImage(gc, width, height, GL_COLOR_INDEX, GL_BITMAP,
                      oldbits, newbits);

        (*gc->procs.renderBitmap)(gc, &bitmap, newbits);

        (*gc->imports.free)(gc, newbits);
    } else {
        /*
        ** Nothing to draw, but we still need to update the current
        ** raster position, and we might be in selection or feedback mode.
        */
        (*gc->procs.renderBitmap)(gc, &bitmap, NULL);
    }
}

void __glRenderBitmap(__GLcontext *gc, const __GLbitmap *bitmap,
                      const GLubyte *data)
{
    __GLfragment frag;
    __GLvertex *rp;
    __GLfloat fx;
    GLint x, y, bit;
    GLint ySign;
    GLuint modeFlags = gc->polygon.shader.modeFlags;
    GLint x0 = gc->transform.clipX0;
    GLint x1 = gc->transform.clipX1;
    GLint y0 = gc->transform.clipY0;
    GLint y1 = gc->transform.clipY1;

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
#if !__GL_SST
    if (modeFlags & __GL_SHADE_TEXTURE) {
        __GLfloat qInv = __glOne / rp->texture[0].w;
        (*gc->procs.textureRaster)(gc, &frag.color, rp->texture[0].x * qInv,
                               rp->texture[0].y * qInv, __glOne);
    }
#endif
    /* XXX - is this the correct test */
    if (gc->state.enables.general & __GL_FOG_ENABLE) {
        (*gc->procs.fogPoint)(gc, &frag, rp->eye.z);
    }

    frag.z = rp->window.z;
    fx = (GLint) (rp->window.x - __glHalf - bitmap->xorig);
    frag.y = (GLint) (rp->window.y - __glHalf - ySign * bitmap->yorig);

    __GL_LOCK_BUFFERS(gc);

    bit = 7;
    for (y = 0; y < bitmap->height; y++) {
        frag.x = fx;
        for (x = 0; x < bitmap->width; x++) {
            if (y0 <= frag.y && frag.y < y1 && x0 <= frag.x && frag.x < x1) {
                if (*data & (1<<bit)) {
                    (*gc->procs.store)(gc->drawBuffer, &frag);
                }
            }
            frag.x++;
            bit--;
            if (bit < 0) {
                bit = 7;
                data++;
            }
        }
        frag.y += ySign;
        if (bit != 7) {
            bit = 7;
            data++;
        }
    }

    __GL_UNLOCK_BUFFERS(gc);

    /*
    ** Advance current raster position.
    */
    rp->window.x += bitmap->xmove;
    rp->window.y += ySign * bitmap->ymove;
}

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
#include <windows.h>
#include "global.h"
#include "types.h"
#include "imports.h"
#include "ctable.h"
#include <string.h>
#include "g_imfncs.h"

#include <glide.h>
#include "sst_globals.h"


/*
** This routine is used by both the __glim and __gllc versions of ColorTableEXT
** it assumes error checking has been done, and requires a packed argument,
** used to determine the source of the pixel storage mode state.
*/
void __glSSTColorTableEXT(__GLcontext *gc, GLenum target, GLenum internalformat, 
                                  GLsizei width, GLenum format, GLenum type, 
                                  const void *table, GLboolean packed)
{
    __GLcolorTable *ct;
    GLboolean proxy = GL_FALSE;
//    __GLpixelSpanInfo spanInfo;
//    int oldWidth;

    /* load the table */
    switch(target) {
    case GL_PROXY_TEXTURE_1D:
    case GL_PROXY_TEXTURE_2D:
        proxy = GL_TRUE;
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
        {
            __GLtexture *tex = __glLookUpTexture(gc, target, gc->texture.currentTexUnit );
            ct = &tex->CT;
        }
                break;
    case GL_SHARED_TEXTURE_PALETTE_EXT:
        break;
    }

#if 0
    if(internalformat == (GLenum)0) { /* proxy width too large, zero entries */
        assert(proxy); /* is error checking code correct? */
        ct->format = 0;
        ct->width = 0;
        ct->type = 0;
        ct->redSize = 0;
        ct->greenSize = 0;
        ct->blueSize = 0;
        ct->alphaSize = 0;
        ct->luminanceSize = 0;
        ct->intensitySize = 0;
        return;
    }

    oldWidth = ct->width * __glElementsPerGroup(ct->baseFormat, ct->type);

    __glLoadColorTableParams(ct, target, internalformat, width);
#endif
    /* Update the array */
    if(!proxy) {
#if 1
        /* Build the Glide GuTexPalette (LE FxU32 = A<<24|R<<16|G<<8|B per
        ** entry).  The original code hardcoded a 3-byte-RGB source stride;
        ** GoldSrc/Half-Life (the main EXT_paletted_texture user) passes
        ** format=GL_RGBA tables, so every entry read one byte early --
        ** de_dust's tan world rendered GREEN (proven by the gfix case-I
        ** probe: output entry k == src[3k..3k+2] exactly).  Honor the
        ** declared source format; only UNSIGNED_BYTE tables reach here in
        ** practice (checked by __glCheckColorTableArgs). */
        static unsigned char pal[256][4];
        const unsigned char *src = table;
        unsigned long i, n;
        int rOfs = 0, gOfs = 1, bOfs = 2, aOfs = -1, stride;
        switch (format) {
          case GL_RGB:
            stride = 3; break;
          case 0x80E1: /* GL_BGRA_EXT */
            stride = 4; rOfs = 2; bOfs = 0; aOfs = 3; break;
          case GL_RGBA:
          default:
            stride = 4; aOfs = 3; break;
        }
        n = (width > 0 && width < 256) ? (unsigned long)width : 256;
        for( i = 0; i < 256; i++ ) {
           pal[i][0] = pal[i][1] = pal[i][2] = 0; pal[i][3] = 255;
        }
        for( i = 0; i < n; i++ ) {
           pal[i][0] = src[i*stride + bOfs];
           pal[i][1] = src[i*stride + gOfs];
           pal[i][2] = src[i*stride + rOfs];
           pal[i][3] = (aOfs >= 0) ? src[i*stride + aOfs] : 255;
        }
        { extern long __r3d_cTableDl; __r3d_cTableDl++; }
        grTexDownloadTable( GR_TEXTABLE_PALETTE, (void*)pal );
#else
        /* malloc table if it needs it */
        if(oldWidth < width * __glElementsPerGroup(ct->baseFormat, ct->type)) {
            ct->table = (*gc->imports.realloc)(gc, ct->table, width * 
                         __glElementsPerGroup(ct->baseFormat, ct->type) *
                         __glBytesPerElement(ct->type));
        }
        __glInitMemUnpack(gc, &spanInfo, width, 1, 
                            0, format, type, table, packed);
        __glInitColorTableStore(ct, &spanInfo);

        __glInitUnpacker(gc, &spanInfo);
        __glInitPacker(gc, &spanInfo);

        spanInfo.applyFbScale = GL_FALSE;

        (*gc->procs.copyImage)(gc, &spanInfo, GL_FALSE);
#endif
    }
}


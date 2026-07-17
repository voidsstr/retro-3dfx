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
#include <ddraw.h>

#include "context.h"
#include "imports.h"
#include "global.h"
#include "g_imfncs.h"
#include "types.h"
#include "namesint.h"
#include "pixel.h"
#include "image.h"
#include "glmath.h"
#include "texfmt.h"
#include "texmgr.h"
#include "texture.h"
#include "gldevice.h"

#include "s3vcontext.h"
#include "s3virge.h"
#include "ddtexmgr.h"
#include "ddtexfmt.h"


#define __DEBUG_PRINT 
#include "dbg.h"

extern HINSTANCE hDirectDraw;
extern struct IDirectDraw *lpDirectDraw;


/* XXX remove this later */
extern LPDIRECTDRAW __wglAllocateDDrawObject(GLvoid);
extern void __wglFreeDDrawObject(LPDIRECTDRAW);

/*
 * DDraw base machine-independent texture manager implementation 
 *
 * This texture manager is a generic implementation. It can be used one of
 * three ways:
 *
 *  1. Without modification, in cases where it suffices
 *  2. Use the manager as-is, but substitute a new (or enhanced) texture object
 *  3. Rewrite the whole thing to take advantage of an underlying texture
 *     management system.
 *
 */

/* Local prototypes */

/* These functions define the standard texture object interface */

__GLDDrawTextureFormat *__glS3VLookupDDrawTextureFormat(int internalStorageFormat);



void __glS3VDDrawInitTextureManager(__GLcontext *gc)
{
    __GLDDrawTextureManager *ddmgr = &__glDDrawTextureManager;

    __glDDrawInitTextureManager(gc);
    if (ddmgr->texmgr.refcount == 1) {
        ddmgr->lookupDDrawTextureFormat = __glS3VLookupDDrawTextureFormat;
    }

}








/*****************************************************************/
/*                       Texture formats                         */
/*****************************************************************/

__GLDDrawTextureFormat *
__glS3VLookupDDrawTextureFormat(int internalStorageFormat)
{
    switch(internalStorageFormat) {
    case __GL_FORMAT_LUMINANCE8:
       return &__glDDrawTexFormatLuminanceBGRA8;
    case __GL_FORMAT_LUMINANCE_ALPHA8:
        return &__glDDrawTexFormatLuminanceAlphaBGRA8;
    case __GL_FORMAT_RGB8:
        return &__glDDrawTexFormatRGB8BGRA8;
    case __GL_FORMAT_RGB332:
        return &__glDDrawTexFormatRGB332BGRA8;
    case __GL_FORMAT_XRGB1555:
        return &__glDDrawTexFormatRGB5BGRA8;
    case __GL_FORMAT_RGB565:
        return &__glDDrawTexFormatRGB565BGRA8;
    case __GL_FORMAT_RGBA8:
        return &__glDDrawTexFormatRGBA8BGRA8;
    case __GL_FORMAT_RGBA4:
        return &__glDDrawTexFormatRGBA4BGRA8;
    case __GL_FORMAT_ARGB4:
        return &__glDDrawTexFormatARGB4BGRA8;
    case __GL_FORMAT_RGBA5551:
        return &__glDDrawTexFormatRGB5_A1BGRA8;
    case __GL_FORMAT_ARGB1555:
        return &__glDDrawTexFormatARGB5_1BGRA8;
    case __GL_FORMAT_ALPHA8:
        return &__glDDrawTexFormatAlphaBGRA8;
    case __GL_FORMAT_INTENSITY8:
        return &__glDDrawTexFormatIntensityBGRA8;
    case __GL_FORMAT_COLOR_INDEX8:
        return &__glDDrawTexFormatColorIndex8BGRA8;
    case __GL_FORMAT_COLOR_INDEX16:
        return &__glDDrawTexFormatColorIndex16BGRA8;
    default:
    case __GL_FORMAT_VENDOR_SPECIFIC:
        return NULL;
    }
}


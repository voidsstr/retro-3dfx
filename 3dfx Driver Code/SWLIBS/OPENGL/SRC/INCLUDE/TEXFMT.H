#ifndef __gl_texfmt_h_
#define __gl_texfmt_h_

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
#include "types.h"


/* 
 * 2 mag methods and 6 min methods
 */
#define __GL_NUM_FILTER_METHODS	  8
/* Texture Format table data structures, for drivers.
 *
 * A table of these is pointed at by the context.  
 */

/*
 *
 */
struct __GLtextureFormatRec {
    GLint internalFormat;
    GLint indexSize;
    GLint redSize;
    GLint greenSize;
    GLint blueSize;
    GLint alphaSize;
    GLint luminanceSize;
    GLint intensitySize;

    GLint bitsPerTexel;

    /* pixel path storage information */
    GLenum pxFormat;
    GLenum pxType;
    GLenum pxAlignment;

    /* fetch a single texel (slow path used for software rendering) */

    void (*extractTexel)(struct __GLmipMapLevelRec *level, 
			 struct __GLtextureRec *tex, GLint img, GLint,
			 GLint col, struct __GLtexelRec *res);

    void (*extractTexelBorder)(struct __GLmipMapLevelRec *level, 
			       struct __GLtextureRec *tex, GLint img,
			       GLint, GLint col, struct __GLtexelRec *res);

    /* get and set rows of texels (used to download and read back textures) */

#if 0
    void (*store)(__GLcontext *gc,
                  struct __GLmipMapLevelRec *level, struct __GLtextureRec *tex,
                  GLint x, GLint y, GLint w, struct __GLtexelRec *src);

    void (*fetch)(__GLcontext *gc, struct __GLmipMapLevelRec *level,
		  struct __GLtextureRec *tex,
                  GLint  x, GLint y, GLint w, struct __GLtexelRec *dest);
#else
    void (*store)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);

    void (*fetch)(__GLcontext *gc, __GLpixelSpanInfo *spanInfo,
                       GLvoid *inspan, GLvoid *outspan);
#endif

    /* Get and set rects of texels (used to download and read back textures) */

    void (*storeRect)(__GLcontext *gc, struct __GLmipMapLevelRec *level,
		      struct __GLtextureRec *tex, GLint x, GLint y, GLint w,
		      GLint h, GLint srcStride, struct __GLtexelRec *src);

    void (*fetchRect)(__GLcontext *gc, struct __GLmipMapLevelRec *level,
		      struct __GLtextureRec *tex, GLint x, GLint y, GLint w,
		      GLint h, GLint dstStride, struct __GLtexelRec *dest);

};

typedef struct __GLtextureFormatRec  __GLtextureFormat;


/* The built-in texture internal formats
 * custom formats may be added as offsets from __GL_FORMAT_VENDOR_SPECIFIC
 */
enum {
    __GL_FORMAT_LUMINANCE8,
    __GL_FORMAT_LUMINANCE_ALPHA8,
    __GL_FORMAT_RGB8,
    __GL_FORMAT_RGB332,
    __GL_FORMAT_XRGB1555,
    __GL_FORMAT_RGB565,
    __GL_FORMAT_RGBA8,
    __GL_FORMAT_RGBA4,
    __GL_FORMAT_ARGB4,
    __GL_FORMAT_RGBA5551,
    __GL_FORMAT_ARGB1555,
    __GL_FORMAT_ALPHA8,
    __GL_FORMAT_INTENSITY8,
    __GL_FORMAT_COLOR_INDEX8,
    __GL_FORMAT_COLOR_INDEX16,
    __GL_FORMAT_VENDOR_SPECIFIC
};

extern const __GLtextureFormat *
__glLookupTextureFormat(GLenum components, GLenum *baseFormat);

extern const __GLtextureFormat __glTexFormatLuminance8;
extern const __GLtextureFormat __glTexFormatLuminanceAlpha8;
extern const __GLtextureFormat __glTexFormatRGB8;
extern const __GLtextureFormat __glTexFormatRGB332;
extern const __GLtextureFormat __glTexFormatXRGB1555;
extern const __GLtextureFormat __glTexFormatRGB565;
extern const __GLtextureFormat __glTexFormatRGBA8;
extern const __GLtextureFormat __glTexFormatRGBA4;
extern const __GLtextureFormat __glTexFormatARGB4;
extern const __GLtextureFormat __glTexFormatRGBA5551;
extern const __GLtextureFormat __glTexFormatARGB1555;
extern const __GLtextureFormat __glTexFormatAlpha8;
extern const __GLtextureFormat __glTexFormatIntensity8;
extern const __GLtextureFormat __glTexFormatColorIndex8;
extern const __GLtextureFormat __glTexFormatColorIndex16;

#endif

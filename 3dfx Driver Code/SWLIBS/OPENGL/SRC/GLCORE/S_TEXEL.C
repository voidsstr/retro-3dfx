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

/*
** Some math routines that are optimized in assembly
*/
#ifdef __GL_USE_MIPSASMCODE
#define	__GL_FRAC(f)	__glFrac(f)
#else
#define __GL_FRAC(f)	((f) - __GL_FLOORF(f))
#endif


/************************************************************************/

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelL8(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;
    GLubyte luminance;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	luminance = (GLubyte) (255.0F * tex->params.borderColor.r);
    } else {
	image = level->buffer + ((row << level->widthLog2) + col);
	luminance = image[0];
    }
    result->r = luminance;
    result->g = luminance;
    result->b = luminance;
}

/*
** Get a texture element out of the two component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelLA8(__GLmipMapLevel *level, __GLtexture *tex,
			 GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;
    GLubyte luminance, alpha;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	luminance = (GLubyte) (255.0F * tex->params.borderColor.r);
	alpha     = (GLubyte) (255.0F * tex->params.borderColor.a);
    } else {
	image = level->buffer + ((row << level->widthLog2) + col) * 2;
	luminance = image[0];
	alpha     = image[1];
    }
    result->r = luminance;
    result->g = luminance;
    result->b = luminance;
    result->a = alpha;
}

/*
** Get a texture element out of the three component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelRGB8(__GLmipMapLevel *level, __GLtexture *tex,
			  GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = (GLubyte) (255.0F * tex->params.borderColor.r);
	result->g = (GLubyte) (255.0F * tex->params.borderColor.g);
	result->b = (GLubyte) (255.0F * tex->params.borderColor.b);
    } else {
	image = level->buffer + ((row << level->widthLog2) + col) * 3;
	result->r = image[0];
	result->g = image[1];
	result->b = image[2];
    }
}

/*
** Get a texture element out of the four component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelRGBA8(__GLmipMapLevel *level, __GLtexture *tex,
			   GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = (GLubyte) (255.0F * tex->params.borderColor.r);
	result->g = (GLubyte) (255.0F * tex->params.borderColor.g);
	result->b = (GLubyte) (255.0F * tex->params.borderColor.b);
	result->a = (GLubyte) (255.0F * tex->params.borderColor.a);
    } else {
	image = level->buffer + ((row << level->widthLog2) + col) * 4;
	result->r = image[0];
	result->g = image[1];
	result->b = image[2];
	result->a = image[3];
    }
}

/* ARGSUSED */
void __glExtractTexelA8(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->a = (GLubyte) (255.0F * tex->params.borderColor.a);
    } else {
	image = level->buffer + ((row << level->widthLog2) + col);
	result->a = image[0];
    }
}

/* ARGSUSED */
void __glExtractTexelI8(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;
    GLubyte intensity;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	intensity = (GLubyte) (255.0F * tex->params.borderColor.r);
    } else {
	image = level->buffer + ((row << level->widthLog2) + col);
	intensity = image[0];
    }
    result->r = intensity;
    result->g = intensity;
    result->b = intensity;
    result->a = intensity;
}

/* ARGSUSED */
void __glExtractTexelXRGB1555(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = (GLubyte) (255.0F * tex->params.borderColor.r);
	result->g = (GLubyte) (255.0F * tex->params.borderColor.g);
	result->b = (GLubyte) (255.0F * tex->params.borderColor.b);
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);

	texel = image[0];
	result->r = (GLubyte) ((texel >> 10) & 0x1F) * (255.0F/31.0F);
	result->g = (GLubyte) ((texel >>  5) & 0x1F) * (255.0F/31.0F);
	result->b = (GLubyte) ((texel      ) & 0x1F) * (255.0F/31.0F);
    }
}

/* ARGSUSED */
void __glExtractTexelRGB332(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLubyte *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = (GLubyte) (255.0F * tex->params.borderColor.r);
	result->g = (GLubyte) (255.0F * tex->params.borderColor.g);
	result->b = (GLubyte) (255.0F * tex->params.borderColor.b);
    } else {
	image = (GLubyte *)level->buffer + ((row << level->widthLog2) + col);

	texel = image[0];
	result->r = (GLubyte) ((texel >> 5) & 0x7) * (255.0F/7.0F);
	result->g = (GLubyte) ((texel >> 2) & 0x7) * (255.0F/7.0F);
	result->b = (GLubyte) ((texel     ) & 0x3) * (255.0F/3.0F);
    }
}

/* ARGSUSED */
void __glExtractTexelRGB565(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = (GLubyte) (255.0F * tex->params.borderColor.r);
	result->g = (GLubyte) (255.0F * tex->params.borderColor.g);
	result->b = (GLubyte) (255.0F * tex->params.borderColor.b);
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);

	texel = image[0];
	result->r = (GLubyte) ((texel >> 11) & 0x1F) * (255.0F/31.0F);
	result->g = (GLubyte) ((texel >>  5) & 0x3F) * (255.0F/63.0F);
	result->b = (GLubyte) ((texel      ) & 0x1F) * (255.0F/31.0F);
    }
}

/* ARGSUSED */
void __glExtractTexelRGBA4(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = (GLubyte) (255.0F * tex->params.borderColor.r);
	result->g = (GLubyte) (255.0F * tex->params.borderColor.g);
	result->b = (GLubyte) (255.0F * tex->params.borderColor.b);
	result->a = (GLubyte) (255.0F * tex->params.borderColor.a);
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
	texel = image[0];
	result->r = (GLubyte) ((texel >> 12) & 0x0F) * (255.0F/15.0F);
	result->g = (GLubyte) ((texel >>  8) & 0x0F) * (255.0F/15.0F);
	result->b = (GLubyte) ((texel >>  4) & 0x0F) * (255.0F/15.0F);
	result->a = (GLubyte) ((texel >>  0) & 0x0F) * (255.0F/15.0F);
    }
}

/* ARGSUSED */
void __glExtractTexelARGB4(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = (GLubyte) (255.0F * tex->params.borderColor.r);
	result->g = (GLubyte) (255.0F * tex->params.borderColor.g);
	result->b = (GLubyte) (255.0F * tex->params.borderColor.b);
	result->a = (GLubyte) (255.0F * tex->params.borderColor.a);
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
	texel = image[0];
	result->r = (GLubyte) ((texel >>  8) & 0x0F) * (255.0F/15.0F);
	result->g = (GLubyte) ((texel >>  4) & 0x0F) * (255.0F/15.0F);
	result->b = (GLubyte) ((texel      ) & 0x0F) * (255.0F/15.0F);
	result->a = (GLubyte) ((texel >> 12) & 0x0F) * (255.0F/15.0F);
    }
}

/* ARGSUSED */
void __glExtractTexelRGBA5551(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = (GLubyte) (255.0F * tex->params.borderColor.r);
	result->g = (GLubyte) (255.0F * tex->params.borderColor.g);
	result->b = (GLubyte) (255.0F * tex->params.borderColor.b);
	result->a = (GLubyte) (255.0F * tex->params.borderColor.a);
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
	texel = image[0];
	result->r = (__GLfloat) ((texel >> 11) & 0x1F) * (255.0F/31.0F);
	result->g = (__GLfloat) ((texel >>  6) & 0x1F) * (255.0F/31.0F);
	result->b = (__GLfloat) ((texel >>  1) & 0x1F) * (255.0F/31.0F);
	result->a = (__GLfloat) ((texel >>  0) & 0x01) * 255.0F;
    }
}

/* ARGSUSED */
void __glExtractTexelARGB1555(__GLmipMapLevel *level, __GLtexture *tex,
		       GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	result->r = (GLubyte) (255.0F * tex->params.borderColor.r);
	result->g = (GLubyte) (255.0F * tex->params.borderColor.g);
	result->b = (GLubyte) (255.0F * tex->params.borderColor.b);
	result->a = (GLubyte) (255.0F * tex->params.borderColor.a);
    } else {
	image = (GLushort *)level->buffer + ((row << level->widthLog2) + col);
	texel = image[0];
	result->r = (__GLfloat) ((texel >> 10) & 0x1F) * (255.0F/31.0F);
	result->g = (__GLfloat) ((texel >>  5) & 0x1F) * (255.0F/31.0F);
	result->b = (__GLfloat) ((texel      ) & 0x1F) * (255.0F/31.0F);
	result->a = (__GLfloat) ((texel >> 15) & 0x01) * 255.0F;
    }
}

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelCI8(__GLmipMapLevel *level, __GLtexture *tex,
			 GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLubyte *image;
    GLuint index;
    __GLcolorTable *ct;
    GLubyte luminance, alpha, intensity;

    ct = &tex->CT;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	index = (GLubyte) (255.0F * tex->params.borderColor.r);
    } else {
	image = ((GLubyte*)level->buffer) + ((row << level->widthLog2) + col);
	index = image[0];
    }

    if (tex->texelFormat == GL_COLOR_INDEX) {
	result->r = index;
    } else {
	switch (ct->baseFormat) {
	case GL_LUMINANCE:
	    luminance = ct->table[index];
	    result->r = luminance;
	    result->g = luminance;
	    result->b = luminance;
	    break;
	case GL_LUMINANCE_ALPHA:
	    index <<= 1;
	    luminance = ct->table[index];
	    alpha = ct->table[index+1];
	    result->r = luminance;
	    result->g = luminance;
	    result->b = luminance;
	    result->a = alpha;
	    break;
	case GL_RGB:
	    index += index<<1;
	    result->r = ct->table[index];
	    result->g = ct->table[index+1];
	    result->b = ct->table[index+2];
	    break;
	case GL_RGBA:
	    index <<= 2;
	    result->r = ct->table[index];
	    result->g = ct->table[index+1];
	    result->b = ct->table[index+2];
	    result->a = ct->table[index+3];
	    break;
	case GL_ALPHA:
	    result->a = ct->table[index];
	    break;
	case GL_INTENSITY:
	    intensity = ct->table[index];
	    result->r = intensity;
	    result->g = intensity;
	    result->b = intensity;
	    result->a = intensity;
	    break;
	default:
	    assert(0);
	}
    }
}

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelCI16(__GLmipMapLevel *level, __GLtexture *tex,
			  GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image;
    GLuint index;
    __GLcolorTable *ct;
    GLubyte luminance, alpha, intensity;

    ct = &tex->CT;

    if ((row < 0) || (col < 0) || (row >= level->height2) ||
	(col >= level->width2)) {
	/*
	** Use border color when the texture supplies no border.
	*/
	index = (GLubyte) (255.0F * tex->params.borderColor.r);
    } else {
	image = ((GLushort*)level->buffer) + ((row << level->widthLog2) + col);
	index = image[0];
    }

    if (tex->texelFormat == GL_COLOR_INDEX) {
	/* Warning: this will truncate to GLubyte */
	result->r = index;
    } else {
	switch (ct->baseFormat) {
	case GL_LUMINANCE:
	    luminance = ct->table[index];
	    result->r = luminance;
	    result->g = luminance;
	    result->b = luminance;
	    break;
	case GL_LUMINANCE_ALPHA:
	    index <<= 1;
	    luminance = ct->table[index];
	    alpha = ct->table[index+1];
	    result->r = luminance;
	    result->g = luminance;
	    result->b = luminance;
	    result->a = alpha;
	    break;
	case GL_RGB:
	    index += index<<1;
	    result->r = ct->table[index];
	    result->g = ct->table[index+1];
	    result->b = ct->table[index+2];
	    break;
	case GL_RGBA:
	    index <<= 2;
	    result->r = ct->table[index];
	    result->g = ct->table[index+1];
	    result->b = ct->table[index+2];
	    result->a = ct->table[index+3];
	    break;
	case GL_ALPHA:
	    result->a = ct->table[index];
	    break;
	case GL_INTENSITY:
	    intensity = ct->table[index];
	    result->r = intensity;
	    result->g = intensity;
	    result->b = intensity;
	    result->a = intensity;
	    break;
	default:
	    assert(0);
	}
    }
}


/*
** Get a texture element out of the one component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glExtractTexelL8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;
    GLubyte luminance;

    row++;
    col++;
    image = level->buffer + (row * level->width + col);
    luminance = image[0];
    result->r = luminance;
    result->g = luminance;
    result->b = luminance;
}

/*
** Get a texture element out of the two component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glExtractTexelLA8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;
    GLubyte luminance, alpha;

    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 2;
    luminance = image[0];
    alpha = image[1];
    result->r = luminance;
    result->g = luminance;
    result->b = luminance;
    result->a = alpha;
}

/*
** Get a texture element out of the three component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glExtractTexelRGB8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 3;
    result->r = image[0];
    result->g = image[1];
    result->b = image[2];
}

/*
** Get a texture element out of the four component texture buffer
** with a border.
*/
/* ARGSUSED */
void __glExtractTexelRGBA8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + (row * level->width + col) * 4;
    result->r = image[0];
    result->g = image[1];
    result->b = image[2];
    result->a = image[3];
}

/* ARGSUSED */
void __glExtractTexelA8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;

    row++;
    col++;
    image = level->buffer + ((row << level->widthLog2) + col);
    result->a = image[0];
}

/* ARGSUSED */
void __glExtractTexelI8_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLtextureBuffer *image;
    GLubyte intensity;

    row++;
    col++;
    image = level->buffer + ((row << level->widthLog2) + col);
    intensity = image[0];
    result->r = intensity;
    result->g = intensity;
    result->b = intensity;
    result->a = intensity;
}

/* ARGSUSED */
void __glExtractTexelXRGB1555_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result->r = (GLubyte) ((texel >> 10) & 0x1F) * (255.0F/31.0F);
    result->g = (GLubyte) ((texel >>  5) & 0x1F) * (255.0F/31.0F);
    result->b = (GLubyte) ((texel      ) & 0x1F) * (255.0F/31.0F);
}

/* ARGSUSED */
void __glExtractTexelRGB332_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLubyte *image, texel;

    row++;
    col++;
    image = (GLubyte *)level->buffer + (row * level->width + col);
    texel = image[0];
    result->r = (GLubyte) ((texel >> 5) & 0x7) * (255.0F/7.0F);
    result->g = (GLubyte) ((texel >> 2) & 0x7) * (255.0F/7.0F);
    result->b = (GLubyte) ((texel     ) & 0x3) * (255.0F/3.0F);
}

/* ARGSUSED */
void __glExtractTexelRGB565_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result->r = (GLubyte) ((texel >> 11) & 0x1F) * (255.0F/31.0F);
    result->g = (GLubyte) ((texel >>  5) & 0x3F) * (255.0F/63.0F);
    result->b = (GLubyte) ((texel      ) & 0x1F) * (255.0F/31.0F);
}

/* ARGSUSED */
void __glExtractTexelRGBA4_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result->r = (GLubyte) ((texel >> 12) & 0x0F) * (255.0F/15.0F);
    result->g = (GLubyte) ((texel >>  8) & 0x0F) * (255.0F/15.0F);
    result->b = (GLubyte) ((texel >>  4) & 0x0F) * (255.0F/15.0F);
    result->a = (GLubyte) ((texel >>  0) & 0x0F) * (255.0F/15.0F);
}

/* ARGSUSED */
void __glExtractTexelARGB4_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result->r = (GLubyte) ((texel >>  8) & 0x0F) * (255.0F/15.0F);
    result->g = (GLubyte) ((texel >>  4) & 0x0F) * (255.0F/15.0F);
    result->b = (GLubyte) ((texel      ) & 0x0F) * (255.0F/15.0F);
    result->a = (GLubyte) ((texel >> 12) & 0x0F) * (255.0F/15.0F);
}

/* ARGSUSED */
void __glExtractTexelRGBA5551_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result->r = (GLubyte) ((texel >> 11) & 0x1F) * (255.0F/31.0F);
    result->g = (GLubyte) ((texel >>  6) & 0x1F) * (255.0F/31.0F);
    result->b = (GLubyte) ((texel >>  1) & 0x1F) * (255.0F/31.0F);
    result->a = (GLubyte) ((texel >>  0) & 0x01) * 255.0F;
}

/* ARGSUSED */
void __glExtractTexelARGB1555_B(__GLmipMapLevel *level, __GLtexture *tex,
			GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image, texel;

    row++;
    col++;
    image = (GLushort *)level->buffer + (row * level->width + col);
    texel = image[0];
    result->r = (GLubyte) ((texel >> 10) & 0x1F) * (255.0F/31.0F);
    result->g = (GLubyte) ((texel >>  5) & 0x1F) * (255.0F/31.0F);
    result->b = (GLubyte) ((texel      ) & 0x1F) * (255.0F/31.0F);
    result->a = (GLubyte) ((texel >> 15) & 0x01) * 255.0F;
}

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelCI8_B(__GLmipMapLevel *level, __GLtexture *tex,
			   GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLubyte *image;
    GLuint index;
    __GLcolorTable *ct;
    GLubyte luminance, alpha, intensity;

    ct = &tex->CT;

    row++;
    col++;
    image = ((GLubyte*)level->buffer) + ((row << level->widthLog2) + col);
    index = image[0];

    if (tex->texelFormat == GL_COLOR_INDEX) {
	result->r = index;
    } else {
	switch (ct->baseFormat) {
	case GL_LUMINANCE:
	    luminance = ct->table[index];
	    result->r = luminance;
	    result->g = luminance;
	    result->b = luminance;
	    break;
	case GL_LUMINANCE_ALPHA:
	    index <<= 1;
	    luminance = ct->table[index];
	    alpha = ct->table[index+1];
	    result->r = luminance;
	    result->g = luminance;
	    result->b = luminance;
	    result->a = alpha;
	    break;
	case GL_RGB:
	    index += index<<1;
	    result->r = ct->table[index];
	    result->g = ct->table[index+1];
	    result->b = ct->table[index+2];
	    break;
	case GL_RGBA:
	    index <<= 2;
	    result->r = ct->table[index];
	    result->g = ct->table[index+1];
	    result->b = ct->table[index+2];
	    result->a = ct->table[index+3];
	    break;
	case GL_ALPHA:
	    result->a = ct->table[index];
	    break;
	case GL_INTENSITY:
	    intensity = ct->table[index];
	    result->r = intensity;
	    result->g = intensity;
	    result->b = intensity;
	    result->a = intensity;
	    break;
	default:
	    assert(0);
	}
    }
}

/*
** Get a texture element out of the one component texture buffer
** with no border.
*/
/* ARGSUSED */
void __glExtractTexelCI16_B(__GLmipMapLevel *level, __GLtexture *tex,
			    GLint img, GLint row, GLint col, __GLtexel *result)
{
    GLushort *image;
    GLuint index;
    __GLcolorTable *ct;
    GLubyte luminance, alpha, intensity;

    ct = &tex->CT;

    row++;
    col++;
    image = ((GLushort *)level->buffer) + ((row << level->widthLog2) + col);
    index = image[0];

    if (tex->texelFormat == GL_COLOR_INDEX) {
	/* Warning: this will truncate to GLubyte */
	result->r = index;
    } else {
	switch (ct->baseFormat) {
	case GL_LUMINANCE:
	    luminance = ct->table[index];
	    result->r = luminance;
	    result->g = luminance;
	    result->b = luminance;
	    break;
	case GL_LUMINANCE_ALPHA:
	    index <<= 1;
	    luminance = ct->table[index];
	    alpha = ct->table[index+1];
	    result->r = luminance;
	    result->g = luminance;
	    result->b = luminance;
	    result->a = alpha;
	    break;
	case GL_RGB:
	    index += index<<1;
	    result->r = ct->table[index];
	    result->g = ct->table[index+1];
	    result->b = ct->table[index+2];
	    break;
	case GL_RGBA:
	    index <<= 2;
	    result->r = ct->table[index];
	    result->g = ct->table[index+1];
	    result->b = ct->table[index+2];
	    result->a = ct->table[index+3];
	    break;
	case GL_ALPHA:
	    result->a = ct->table[index];
	    break;
	case GL_INTENSITY:
	    intensity = ct->table[index];
	    result->r = intensity;
	    result->g = intensity;
	    result->b = intensity;
	    result->a = intensity;
	    break;
	default:
	    assert(0);
	}
    }
}

/************************************************************************/

/*
** Return texel nearest the s coordinate.  s is converted to u
** implicitly during this step.
*/
/* ARGSUSED */
void __glNearestFilter1(__GLtexture *tex, __GLmipMapLevel *lp,
			__GLfloat s, __GLfloat t, __GLtexel *result)
{
    GLint col = (GLint) s;
    GLint w2 = lp->width2;

    /* Find texel index */
    if (tex->params.sWrapMode == GL_REPEAT) {
	col &= w2-1;
    } else {
	if (col < 0) col = 0;
	else if (col >= w2) col = w2 - 1;
    }

    /* Lookup texel */
    (*lp->extract)(lp, tex, 0, 0, col, result);
}

/*
** Return texel nearest the s,t coordinates.  s,t are converted to u,v
** implicitly during this step.
*/
/* ARGSUSED */
void __glNearestFilter2(__GLtexture *tex, __GLmipMapLevel *lp,
			__GLfloat s, __GLfloat t, __GLtexel *result)
{
    GLint row = (GLint) t;
    GLint col = (GLint) s;
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
    (*lp->extract)(lp, tex, 0, row, col, result);
}

/*
** Return texel which is a linear combination of texels near s.
*/
/* ARGSUSED */
void __glLinearFilter1(__GLtexture *tex, __GLmipMapLevel *lp,
		       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLfloat u, alpha, omalpha;
    GLint col0, col1;
    __GLtexel t0, t1;

    /* Find col0 and col1 */
    u = s;
    if (tex->params.sWrapMode == GL_REPEAT) {
	u -= __glHalf;
	col0 = ((GLint) floor(u)) & (lp->width2 - 1);
	col1 = (col0 + 1) & (lp->width2 - 1);
    } else {
	if (u < __glZero) u = __glZero;
	else if (u > lp->width2) u = lp->width2;
	u -= __glHalf;
	col0 = (GLint) floor(u);
	col1 = col0 + 1;
    }

    /* Compute alpha and beta */
    alpha = __GL_FRAC(u);

    /* Calculate the final texel value as a combination of the two texels */
    (*lp->extract)(lp, tex, 0, 0, col0, &t0);
    (*lp->extract)(lp, tex, 0, 0, col1, &t1);

    omalpha = __glOne - alpha;

    switch (tex->texelFormat) {
      case GL_LUMINANCE_ALPHA:
	result->a = omalpha * t0.a + alpha * t1.a;
	/* FALLTHROUGH */
      case GL_LUMINANCE:
	result->r = omalpha * t0.r + alpha * t1.r;
	break;
      case GL_RGBA:
	result->a = omalpha * t0.a + alpha * t1.a;
	/* FALLTHROUGH */
      case GL_RGB:
	result->r = omalpha * t0.r + alpha * t1.r;
	result->g = omalpha * t0.g + alpha * t1.g;
	result->b = omalpha * t0.b + alpha * t1.b;
	break;
      case GL_ALPHA:
	result->a = omalpha * t0.a + alpha * t1.a;
	break;
      case GL_INTENSITY:
	result->r = omalpha * t0.r + alpha * t1.r;
	break;
    }
}

/*
** Return texel which is a linear combination of texels near s,t.
*/
/* ARGSUSED */
void __glLinearFilter2(__GLtexture *tex, __GLmipMapLevel *lp,
		       __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLfloat u, v, alpha, beta, half;
    GLint col0, row0, col1, row1, w2f, h2f;
    __GLtexel t00, t01, t10, t11;
    __GLfloat omalpha, ombeta, m00, m01, m10, m11;

    /* Find col0, col1 */
    w2f = (GLint)lp->width2f;
    u = s;
    half = __glHalf;
    if (tex->params.sWrapMode == GL_REPEAT) {
	GLint w2mask = lp->width2 - 1;
	u -= half;
	col0 = ((GLint) floor(u)) & w2mask;
	col1 = (col0 + 1) & w2mask;
    } else {
	if (u < __glZero) u = __glZero;
	else if (u > w2f) u = w2f;
	u -= half;
	col0 = (GLint) floor(u);
	col1 = col0 + 1;
    }

    /* Find row0, row1 */
    h2f = (GLint)lp->height2f;
    v = t;
    if (tex->params.tWrapMode == GL_REPEAT) {
	GLint h2mask = lp->height2 - 1;
	v -= half;
	row0 = ((GLint) floor(v)) & h2mask;
	row1 = (row0 + 1) & h2mask;
    } else {
	if (v < __glZero) v = __glZero;
	else if (v > h2f) v = h2f;
	v -= half;
	row0 = (GLint) floor(v);
	row1 = row0 + 1;
    }

    /* Compute alpha and beta */
    alpha = __GL_FRAC(u);
    beta = __GL_FRAC(v);

    /* Calculate the final texel value as a combination of the square chosen */
    (*lp->extract)(lp, tex, 0, row0, col0, &t00);
    (*lp->extract)(lp, tex, 0, row0, col1, &t10);
    (*lp->extract)(lp, tex, 0, row1, col0, &t01);
    (*lp->extract)(lp, tex, 0, row1, col1, &t11);

    omalpha = __glOne - alpha;
    ombeta = __glOne - beta;

    m00 = omalpha * ombeta;
    m10 = alpha * ombeta;
    m01 = omalpha * beta;
    m11 = alpha * beta;

    switch (tex->texelFormat) {
      case GL_LUMINANCE_ALPHA:
	result->a = m00*t00.a + m10*t10.a + m01*t01.a
	    + m11*t11.a;
	/* FALLTHROUGH */
      case GL_LUMINANCE:
	result->r = m00*t00.r + m10*t10.r
	    + m01*t01.r + m11*t11.r;
	break;
      case GL_RGBA:
	result->a = m00*t00.a + m10*t10.a + m01*t01.a
	    + m11*t11.a;
	/* FALLTHROUGH */
      case GL_RGB:
	result->r = m00*t00.r + m10*t10.r + m01*t01.r + m11*t11.r;
	result->g = m00*t00.g + m10*t10.g + m01*t01.g + m11*t11.g;
	result->b = m00*t00.b + m10*t10.b + m01*t01.b + m11*t11.b;
	break;
      case GL_ALPHA:
	result->a = m00*t00.a + m10*t10.a + m01*t01.a
	    + m11*t11.a;
	break;
      case GL_INTENSITY:
	result->r = m00*t00.r + m10*t10.r
	    + m01*t01.r + m11*t11.r;
	break;
    }
}

/*
** Linear min/mag filter
*/
void __glLinearFilterUVScaled(__GLtexture *tex, __GLfloat lod,
		      __GLfloat s, __GLfloat t, __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    (*tex->linear)(tex, tex->level[0], s, t, result);
}

/*
** Nearest min/mag filter
*/
void __glNearestFilterUVScaled(__GLtexture *tex, __GLfloat lod,
		       __GLfloat s, __GLfloat t, __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    (*tex->nearest)(tex, tex->level[0], s, t, result);
}

/*
** Linear min/mag filter
*/
void __glLinearFilter(__GLtexture *tex, __GLfloat lod,
		      __GLfloat s, __GLfloat t, __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    __GLmipMapLevel *lp = tex->level[0];
    s *= lp->width2f;
    t *= lp->height2f;
    (*tex->linear)(tex, tex->level[0], s, t, result);
}

/*
** Nearest min/mag filter
*/
void __glNearestFilter(__GLtexture *tex, __GLfloat lod,
		       __GLfloat s, __GLfloat t, __GLtexel *result)
{
#ifdef __GL_LINT
    lod = lod;
#endif
    __GLmipMapLevel *lp = tex->level[0];
    s *= lp->width2f;
    t *= lp->height2f;
    (*tex->nearest)(tex, tex->level[0], s, t, result);
}

/*
** Apply minification rules to find the texel value.
*/
void __glNMNFilter(__GLtexture *tex, __GLfloat lod,
		   __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLint p, d;

    if (lod <= __glHalf) {
	d = 0;
    } else {
	p = tex->p;
	d = (GLint) (lod + ((__GLfloat)0.49995)); /* NOTE: .5 minus epsilon */
	if (d > p) {
	    d = p;
	}
    }
    lp = tex->level[d];
    s *= lp->width2f;
    t *= lp->height2f;
    (*tex->nearest)(tex, lp, s, t, result);
}

/*
** Apply minification rules to find the texel value.
*/
void __glLMNFilter(__GLtexture *tex, __GLfloat lod,
		   __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLint p, d;

    if (lod <= __glHalf) {
	d = 0;
    } else {
	p = tex->p;
	d = (GLint) (lod + ((__GLfloat) 0.49995)); /* NOTE: .5 minus epsilon */
	if (d > p) {
	    d = p;
	}
    }
    lp = tex->level[d];
    s *= lp->width2f;
    t *= lp->height2f;
    (*tex->linear)(tex, lp, s, t, result);
}

/*
** Apply minification rules to find the texel value.
*/
void __glNMLFilter(__GLtexture *tex, __GLfloat lod,
		   __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLint p, d;
    __GLtexel td, td1;
    __GLfloat f, omf;

    p = tex->p;
    d = ((GLint) lod) + 1;
    if (d > p || d < 0) {
	/* Clamp d to last available mipmap */
	lp = tex->level[p];
	s *= lp->width2f;
	t *= lp->height2f;
	(*tex->nearest)(tex, lp, s, t, result);
    } else {
	__GLfloat s1, t1;

	lp = tex->level[d];
	s1 = s * lp->width2f;
	t1 = t * lp->height2f;
	(*tex->nearest)(tex, lp, s1, t1, &td);

	lp = tex->level[d-1];
	s1 = s * lp->width2f;
	t1 = t * lp->height2f;
	(*tex->nearest)(tex, lp, s1, t1, &td1);

	f = __GL_FRAC(lod);
	omf = __glOne - f;
	switch (tex->texelFormat) {
	  case GL_LUMINANCE_ALPHA:
	    result->a = omf * td1.a + f * td.a;
	    /* FALLTHROUGH */
	  case GL_LUMINANCE:
	    result->r = omf * td1.r + f * td.r;
	    break;
	  case GL_RGBA:
	    result->a = omf * td1.a + f * td.a;
	    /* FALLTHROUGH */
	  case GL_RGB:
	    result->r = omf * td1.r + f * td.r;
	    result->g = omf * td1.g + f * td.g;
	    result->b = omf * td1.b + f * td.b;
	    break;
	  case GL_ALPHA:
	    result->a = omf * td1.a + f * td.a;
	    break;
	  case GL_INTENSITY:
	    result->r = omf * td1.r + f * td.r;
	    break;
	}
    }
}

/*
** Apply minification rules to find the texel value.
*/
void __glLMLFilter(__GLtexture *tex, __GLfloat lod,
		   __GLfloat s, __GLfloat t, __GLtexel *result)
{
    __GLcontext *gc = tex->gc;
    __GLmipMapLevel *lp;
    GLint p, d;
    __GLtexel td, td1;
    __GLfloat f, omf;

    p = tex->p;
    d = ((GLint) lod) + 1;
    if (d > p || d < 0) {
	/* Clamp d to last available mipmap */
	lp = tex->level[p];
	s *= lp->width2f;
	t *= lp->height2f;
	(*tex->linear)(tex, lp, s, t, result);
    } else {
	__GLfloat s1, t1;

	lp = tex->level[d];
	s1 = s * lp->width2f;
	t1 = t * lp->height2f;
	(*tex->linear)(tex, lp, s1, t1, &td);

	lp = tex->level[d-1];
	s1 = s * lp->width2f;
	t1 = t * lp->height2f;
	(*tex->linear)(tex, lp, s1, t1, &td1);

	f = __GL_FRAC(lod);
	omf = __glOne - f;
	switch (tex->texelFormat) {
	  case GL_LUMINANCE_ALPHA:
	    result->a = omf * td1.a + f * td.a;
	    /* FALLTHROUGH */
	  case GL_LUMINANCE:
	    result->r = omf * td1.r + f * td.r;
	    break;
	  case GL_RGBA:
	    result->a = omf * td1.a + f * td.a;
	    /* FALLTHROUGH */
	  case GL_RGB:
	    result->r = omf * td1.r + f * td.r;
	    result->g = omf * td1.g + f * td.g;
	    result->b = omf * td1.b + f * td.b;
	    break;
	  case GL_ALPHA:
	    result->a = omf * td1.a + f * td.a;
	    break;
	  case GL_INTENSITY:
	    result->r = omf * td1.r + f * td.r;
	    break;
	}
    }
}


/***********************************************************************/

/* 1 Component modulate */
void __glTextureModulateL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat luminance = __GL_UB_TO_FLOAT(texel->r);

    color->r = luminance * color->r;
    color->g = luminance * color->g;
    color->b = luminance * color->b;
}

/* 2 Component modulate */
void __glTextureModulateLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat luminance = __GL_UB_TO_FLOAT(texel->r);
    __GLfloat alpha = __GL_UB_TO_FLOAT(texel->a);

    color->r = luminance * color->r;
    color->g = luminance * color->g;
    color->b = luminance * color->b;
    color->a = alpha * color->a;
}

/* 3 Component modulate */
void __glTextureModulateRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = __GL_UB_TO_FLOAT(texel->r) * color->r;
    color->g = __GL_UB_TO_FLOAT(texel->g) * color->g;
    color->b = __GL_UB_TO_FLOAT(texel->b) * color->b;
}

/* 4 Component modulate */
void __glTextureModulateRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = __GL_UB_TO_FLOAT(texel->r) * color->r;
    color->g = __GL_UB_TO_FLOAT(texel->g) * color->g;
    color->b = __GL_UB_TO_FLOAT(texel->b) * color->b;
    color->a = __GL_UB_TO_FLOAT(texel->a) * color->a;
}

/* Alpha modulate */
void __glTextureModulateA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->a = __GL_UB_TO_FLOAT(texel->a) * color->a;
}

/* Intensity modulate */
void __glTextureModulateI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat intensity = __GL_UB_TO_FLOAT(texel->r);

    color->r = intensity * color->r;
    color->g = intensity * color->g;
    color->b = intensity * color->b;
    color->a = intensity * color->a;
}

/* Color Index modulate */
void __glTextureModulateCI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat index = __GL_UB_TO_FLOAT(texel->r);

    color->r = index * color->r;
}

/***********************************************************************/

/* 3 Component decal */
void __glTextureDecalRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = __GL_UB_TO_FLOAT(texel->r) * gc->frontBuffer.redScale;
    color->g = __GL_UB_TO_FLOAT(texel->g) * gc->frontBuffer.greenScale;
    color->b = __GL_UB_TO_FLOAT(texel->b) * gc->frontBuffer.blueScale;
}

/* 4 Component decal */
void __glTextureDecalRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat a = __GL_UB_TO_FLOAT(texel->a);
    __GLfloat oma = __glOne - a;

    color->r = oma * color->r
	+ a * __GL_UB_TO_FLOAT(texel->r) * gc->frontBuffer.redScale;
    color->g = oma * color->g
	+ a * __GL_UB_TO_FLOAT(texel->g) * gc->frontBuffer.greenScale;
    color->b = oma * color->b
	+ a * __GL_UB_TO_FLOAT(texel->b) * gc->frontBuffer.blueScale;
}

/***********************************************************************/

/* 1 Component blend */
void __glTextureBlendL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat l = __GL_UB_TO_FLOAT(texel->r);
    __GLfloat oml = __glOne - l;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = oml * color->r + l * cc->r;
    color->g = oml * color->g + l * cc->g;
    color->b = oml * color->b + l * cc->b;
}

/* 2 Component blend */
void __glTextureBlendLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat l = __GL_UB_TO_FLOAT(texel->r);
    __GLfloat oml = __glOne - l;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = oml * color->r + l * cc->r;
    color->g = oml * color->g + l * cc->g;
    color->b = oml * color->b + l * cc->b;
    color->a = __GL_UB_TO_FLOAT(texel->a) * color->a;
}

/* 3 Component blend */
void __glTextureBlendRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat r = __GL_UB_TO_FLOAT(texel->r);
    __GLfloat g = __GL_UB_TO_FLOAT(texel->g);
    __GLfloat b = __GL_UB_TO_FLOAT(texel->b);
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = (__glOne - r) * color->r + r * cc->r;
    color->g = (__glOne - g) * color->g + g * cc->g;
    color->b = (__glOne - b) * color->b + b * cc->b;
}

/* 4 Component blend */
void __glTextureBlendRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat r = __GL_UB_TO_FLOAT(texel->r);
    __GLfloat g = __GL_UB_TO_FLOAT(texel->g);
    __GLfloat b = __GL_UB_TO_FLOAT(texel->b);
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = (__glOne - r) * color->r + r * cc->r;
    color->g = (__glOne - g) * color->g + g * cc->g;
    color->b = (__glOne - b) * color->b + b * cc->b;
    color->a = __GL_UB_TO_FLOAT(texel->a) * color->a;
}

/* Alpha blend */
void __glTextureBlendA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->a = __GL_UB_TO_FLOAT(texel->a) * color->a;
}

/* Intensity blend */
void __glTextureBlendI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat i = __GL_UB_TO_FLOAT(texel->r);
    __GLfloat omi = __glOne - i;
    __GLcolor *cc = &gc->state.texture.env[0].color;

    color->r = omi * color->r + i * cc->r;
    color->g = omi * color->g + i * cc->g;
    color->b = omi * color->b + i * cc->b;
    color->a = omi * color->a + i * cc->a;
}

/***********************************************************************/

/* 1 Component replace */
void __glTextureReplaceL(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat luminance = __GL_UB_TO_FLOAT(texel->r);

    color->r = luminance * gc->frontBuffer.redScale;
    color->g = luminance * gc->frontBuffer.greenScale;
    color->b = luminance * gc->frontBuffer.blueScale;
}

/* 2 Component replace */
void __glTextureReplaceLA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat luminance = __GL_UB_TO_FLOAT(texel->r);
    __GLfloat alpha = __GL_UB_TO_FLOAT(texel->a);

    color->r = luminance * gc->frontBuffer.redScale;
    color->g = luminance * gc->frontBuffer.greenScale;
    color->b = luminance * gc->frontBuffer.blueScale;
    color->a = alpha * gc->frontBuffer.alphaScale;
}

/* 3 Component replace */
void __glTextureReplaceRGB(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = __GL_UB_TO_FLOAT(texel->r) * gc->frontBuffer.redScale;
    color->g = __GL_UB_TO_FLOAT(texel->g) * gc->frontBuffer.greenScale;
    color->b = __GL_UB_TO_FLOAT(texel->b) * gc->frontBuffer.blueScale;
}

/* 4 Component replace */
void __glTextureReplaceRGBA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = __GL_UB_TO_FLOAT(texel->r) * gc->frontBuffer.redScale;
    color->g = __GL_UB_TO_FLOAT(texel->g) * gc->frontBuffer.greenScale;
    color->b = __GL_UB_TO_FLOAT(texel->b) * gc->frontBuffer.blueScale;
    color->a = __GL_UB_TO_FLOAT(texel->a) * gc->frontBuffer.alphaScale;
}

/* Alpha replace */
void __glTextureReplaceA(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->a = __GL_UB_TO_FLOAT(texel->a) * gc->frontBuffer.alphaScale;
}

/* Intensity replace */
void __glTextureReplaceI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    __GLfloat intensity = __GL_UB_TO_FLOAT(texel->r);

    color->r = intensity * gc->frontBuffer.redScale;
    color->g = intensity * gc->frontBuffer.greenScale;
    color->b = intensity * gc->frontBuffer.blueScale;
    color->a = intensity * gc->frontBuffer.alphaScale;
}

/* Color Index replace */
void __glTextureReplaceCI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = __GL_UB_TO_FLOAT(texel->r);
}

/***********************************************************************/

/* Color Index add */
void __glTextureAddCI(__GLcontext *gc, __GLcolor *color, __GLtexel *texel)
{
    color->r = __GL_UB_TO_FLOAT(texel->r) + color->r;
}

/***********************************************************************/

/* ARGSUSED */
__GLfloat __glNopPolygonRho(__GLcontext *gc, const __GLshade *sh,
			    __GLfloat s, __GLfloat t, __GLfloat winv)
{
    return __glZero;
}

/*
** Compute the "rho" (level of detail) parameter used by the texturing code.
** Instead of fully computing the derivatives compute nearby texture coordinates
** and discover the derivative.  The incoming s & t arguments have not
** been divided by winv yet.
*/
__GLfloat __glComputePolygonRho(__GLcontext *gc, const __GLshade *sh,
				__GLfloat s, __GLfloat t, __GLfloat qw)
{
    const GLuint modeFlags = gc->polygon.shader.modeFlags;
    const __GLtexture *tex = gc->texture.currentTexture;
    __GLfloat qw0, qw1, p0, p1;
    __GLfloat pupx, pupy, pvpx, pvpy;
    __GLfloat px, py, one;

    /* Compute partial of u with respect to x */
    one = __glOne;
    qw0 = one / (qw - sh->dqwdx);
    qw1 = one / (qw + sh->dqwdx);
    p0 = (s - sh->dsdx) * qw0;
    p1 = (s + sh->dsdx) * qw1;
    pupx = p1 - p0;
    if (!(modeFlags & __GL_SHADE_TEXTURE_UVSCALED)) {
	pupx *= tex->level[0]->width2f;
    }

    /* Compute partial of v with respect to x */
    p0 = (t - sh->dtdx) * qw0;
    p1 = (t + sh->dtdx) * qw1;
    pvpx = p1 - p0;
    if (!(modeFlags & __GL_SHADE_TEXTURE_UVSCALED)) {
	pvpx *= tex->level[0]->height2f;
    }

    /* Compute partial of u with respect to y */
    qw0 = one / (qw - sh->dqwdy);
    qw1 = one / (qw + sh->dqwdy);
    p0 = (s - sh->dsdy) * qw0;
    p1 = (s + sh->dsdy) * qw1;
    pupy = p1 - p0;
    if (!(modeFlags & __GL_SHADE_TEXTURE_UVSCALED)) {
	pupy *= tex->level[0]->width2f;
    }

    /* Compute partial of v with respect to y */
    p0 = (t - sh->dtdy) * qw0;
    p1 = (t + sh->dtdy) * qw1;
    pvpy = p1 - p0;
    if (!(modeFlags & __GL_SHADE_TEXTURE_UVSCALED)) {
	pvpy *= tex->level[0]->height2f;
    }

    /* Finally, figure sum of squares */
    px = pupx * pupx + pvpx * pvpx;
    py = pupy * pupy + pvpy * pvpy;

    /* Return largest value as the level of detail */
    if (px > py) {
	return px * ((__GLfloat) 0.25);
    } else {
	return py * ((__GLfloat) 0.25);
    }
}

/* ARGSUSED */
__GLfloat __glNopLineRho(__GLcontext *gc,
			 __GLfloat s, __GLfloat t, __GLfloat qw)
{
    return __glZero;
}

__GLfloat __glComputeLineRho(__GLcontext *gc,
			     __GLfloat s, __GLfloat t, __GLfloat qw)
{
    __GLfloat pspx, pspy, ptpx, ptpy;
    __GLfloat pupx, pupy, pvpx, pvpy;
    __GLfloat temp, pu, pv, p2;
    __GLfloat magnitude, invMag, invMag2;
    __GLfloat dx, dy, invqw;
    const __GLtexture *tex = gc->texture.currentTexture;
    const __GLvertex *v0 = gc->line.options.v0;
    const __GLvertex *v1 = gc->line.options.v1;

    /* Compute the length of the line (its magnitude) */
    dx = v1->window.x - v0->window.x;
    dy = v1->window.y - v0->window.y;
    magnitude = __GL_SQRTF(dx*dx + dy*dy);
    invMag = __glOne / magnitude;
    invMag2 = invMag * invMag;

    invqw = __glOne / qw;

    /* Compute s partials */
    temp = ((v1->texture.x - v0->texture.x) - s) * invqw;
    pspx = temp * dx * invMag2;
    pspy = temp * dy * invMag2;

    /* Compute t partials */
    temp = ((v1->texture.y - v0->texture.y) - t) * invqw;
    ptpx = temp * dx * invMag2;
    ptpy = temp * dy * invMag2;

    pupx = pspx * tex->level[0]->width2f;
    pupy = pspy * tex->level[0]->width2f;
    pvpx = ptpx * tex->level[0]->height2f;
    pvpy = ptpy * tex->level[0]->height2f;

    /* Now compute rho */
    pu = pupx * dx + pupy * dy;
    pv = pvpx * dx + pvpy * dy;
    p2 = pu * pu + pv * pv;

    return (p2 * invMag2);
}

/************************************************************************/

void __glTextureFragmentUVScale(__GLcontext *gc, __GLcolor *color,
			     __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLmipMapLevel *lp = tex->level[0];

    (*gc->procs.texture)(gc, color, s*lp->width2f, t*lp->width2f, rho);
}

/*
** Fast texture a fragment assumes that rho is noise - this is true
** when no mipmapping is being done and the min and mag filters are
** the same.
*/
void __glFastTextureFragment(__GLcontext *gc, __GLcolor *color,
			     __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLtexel texel;

#ifdef __GL_LINT
    rho = rho;
#endif
    (*tex->magnify)(tex, __glZero, s, t, &texel);
    (*tex->env)(gc, color, &texel);
}

/*
** Non-mipmapping texturing function.
*/
void __glTextureFragment(__GLcontext *gc, __GLcolor *color,
			 __GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLtexel texel;

    if (rho <= tex->c) {
	(*tex->magnify)(tex, __glZero, s, t, &texel);
    } else {
	(*tex->minnify)(tex, __glZero, s, t, &texel);
    }

    /* Now apply texture environment to get final color */
    (*tex->env)(gc, color, &texel);
}

void __glMipMapFragment(__GLcontext *gc, __GLcolor *color,
			__GLfloat s, __GLfloat t, __GLfloat rho)
{
    __GLtexture *tex = gc->texture.currentTexture;
    __GLtexel texel;

    /* In the spec c is given in terms of lambda.
    ** Here c is compared to rho (really rho^2) and adjusted accordingly.
    */
    if (rho <= tex->c) {
	/* NOTE: rho is ignored by magnify proc */
	(*tex->magnify)(tex, rho, s, t, &texel);
    } else {
	if (rho) {
#if 0
	    __GLfloat oldrho;
#endif
            __GLfloat twotolev;
	    GLuint irho, lev;
	    /* Convert rho to lambda */
#if 0
	    oldrho = __GL_LOGF(rho) * (__GL_M_LN2_INV * 0.5);
#endif

	    /* this is an approximation of log base 2 */
	    irho = rho;
	    lev = 0;
	    while( irho >>= 1 ) lev++;
	    twotolev = 1<<lev;
	    rho = (lev + ( (rho-twotolev) / twotolev ) ) * 0.5;
	} else {
	    rho = __glZero;
	}
	(*tex->minnify)(tex, rho, s, t, &texel);
    }

    /* Now apply texture environment to get final color */
    (*tex->env)(gc, color, &texel);
}

/*****************************************************************/
/*                  Texture internal formats                     */
/*****************************************************************/

const __GLtextureFormat __glTexFormatLuminance8 = {
    __GL_FORMAT_LUMINANCE8,	/* internalFormat */
    0,				/* indexSize */
    0,				/* redSize */
    0,				/* greenSize */
    0,				/* blueSize */
    0,				/* alphaSize */
    8,				/* luminanceSize */
    0,				/* intensitySize */
    8,				/* bitsPerTexel */
    GL_RED,			/* pxFormat */
    GL_UNSIGNED_BYTE,		/* pxType */
    1,				/* pxAlignment */
    __glExtractTexelL8,		/* extractTexel */
    __glExtractTexelL8_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatLuminanceAlpha8 = {
    __GL_FORMAT_LUMINANCE_ALPHA8, /* internalFormat */
    0,				/* indexSize */
    0,				/* redSize */
    0,				/* greenSize */
    0,				/* blueSize */
    8,				/* alphaSize */
    8,				/* luminanceSize */
    0,				/* intensitySize */
    16,				/* bitsPerTexel */
    __GL_RED_ALPHA,		/* pxFormat */
    GL_UNSIGNED_BYTE,		/* pxType */
    1,				/* pxAlignment */
    __glExtractTexelLA8,		/* extractTexel */
    __glExtractTexelLA8_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatRGB8 = {
    __GL_FORMAT_RGB8,		/* internalFormat */
    0,				/* indexSize */
    8,				/* redSize */
    8,				/* greenSize */
    8,				/* blueSize */
    0,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    24,				/* bitsPerTexel */
    GL_RGB,			/* pxFormat */
    GL_UNSIGNED_BYTE,		/* pxType */
    1,				/* pxAlignment */
    __glExtractTexelRGB8,	/* extractTexel */
    __glExtractTexelRGB8_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatRGB332 = {
    __GL_FORMAT_RGB332,		/* internalFormat */
    0,				/* indexSize */
    3,				/* redSize */
    3,				/* greenSize */
    2,				/* blueSize */
    0,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    8,				/* bitsPerTexel */
    GL_RGB,			/* pxFormat */
    GL_UNSIGNED_BYTE_3_3_2_EXT,	/* pxType */
    1,				/* pxAlignment */
    __glExtractTexelRGB332,	/* extractTexel */
    __glExtractTexelRGB332_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatXRGB1555 = {
    __GL_FORMAT_XRGB1555,	/* internalFormat */
    0,				/* indexSize */
    5,				/* redSize */
    5,				/* greenSize */
    5,				/* blueSize */
    0,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    16,				/* bitsPerTexel */
    GL_RGB,			/* pxFormat */
    __GL_UNSIGNED_SHORT_X_5_5_5,/* pxType */
    2,				/* pxAlignment */
    __glExtractTexelXRGB1555,	/* extractTexel */
    __glExtractTexelXRGB1555_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatRGB565 = {
    __GL_FORMAT_RGB565,		/* internalFormat */
    0,				/* indexSize */
    5,				/* redSize */
    6,				/* greenSize */
    5,				/* blueSize */
    0,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    16,				/* bitsPerTexel */
    GL_RGB,			/* pxFormat */
    __GL_UNSIGNED_SHORT_5_6_5,	/* pxType */
    2,				/* pxAlignment */
    __glExtractTexelRGB565,	/* extractTexel */
    __glExtractTexelRGB565_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatRGBA8 = {
    __GL_FORMAT_RGBA8,		/* internalFormat */
    0,				/* indexSize */
    8,				/* redSize */
    8,				/* greenSize */
    8,				/* blueSize */
    8,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    32,				/* bitsPerTexel */
    GL_RGBA,			/* pxFormat */
    GL_UNSIGNED_BYTE,		/* pxType */
    1,				/* pxAlignment */
    __glExtractTexelRGBA8,	/* extractTexel */
    __glExtractTexelRGBA8_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatRGBA4 = {
    __GL_FORMAT_RGBA4,		/* internalFormat */
    0,				/* indexSize */
    4,				/* redSize */
    4,				/* greenSize */
    4,				/* blueSize */
    4,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    16,				/* bitsPerTexel */
    GL_RGBA,			/* pxFormat */
    GL_UNSIGNED_SHORT_4_4_4_4_EXT,		/* pxType */
    2,				/* pxAlignment */
    __glExtractTexelRGBA4,	/* extractTexel */
    __glExtractTexelRGBA4_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatARGB4 = {
    __GL_FORMAT_ARGB4,		/* internalFormat */
    0,				/* indexSize */
    4,				/* redSize */
    4,				/* greenSize */
    4,				/* blueSize */
    4,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    16,				/* bitsPerTexel */
    GL_RGBA,			/* pxFormat */
    __GL_UNSIGNED_SHORT_4_4_4_4_ARGB,		/* pxType */
    2,				/* pxAlignment */
    __glExtractTexelARGB4,	/* extractTexel */
    __glExtractTexelARGB4_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatRGBA5551 = {
    __GL_FORMAT_RGBA5551,	/* internalFormat */
    0,				/* indexSize */
    5,				/* redSize */
    5,				/* greenSize */
    5,				/* blueSize */
    1,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    16,				/* bitsPerTexel */
    GL_RGBA,			/* pxFormat */
    GL_UNSIGNED_SHORT_5_5_5_1_EXT,		/* pxType */
    1,				/* pxAlignment */
    __glExtractTexelRGBA5551,	/* extractTexel */
    __glExtractTexelRGBA5551_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatARGB1555 = {
    __GL_FORMAT_ARGB1555,	/* internalFormat */
    0,				/* indexSize */
    5,				/* redSize */
    5,				/* greenSize */
    5,				/* blueSize */
    1,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    16,				/* bitsPerTexel */
    GL_RGBA,			/* pxFormat */
    __GL_UNSIGNED_SHORT_1_5_5_5,		/* pxType */
    2,				/* pxAlignment */
    __glExtractTexelARGB1555,	/* extractTexel */
    __glExtractTexelARGB1555_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatAlpha8 = {
    __GL_FORMAT_ALPHA8,		/* internalFormat */
    0,				/* indexSize */
    0,				/* redSize */
    0,				/* greenSize */
    0,				/* blueSize */
    8,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    8,				/* bitsPerTexel */
    GL_ALPHA,			/* pxFormat */
    GL_UNSIGNED_BYTE,		/* pxType */
    1,				/* pxAlignment */
    __glExtractTexelA8,		/* extractTexel */
    __glExtractTexelA8_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatIntensity8 = {
    __GL_FORMAT_INTENSITY8,	/* internalFormat */
    0,				/* indexSize */
    0,				/* redSize */
    0,				/* greenSize */
    0,				/* blueSize */
    0,				/* alphaSize */
    0,				/* luminanceSize */
    8,				/* intensitySize */
    8,				/* bitsPerTexel */
    GL_RED,			/* pxFormat */
    GL_UNSIGNED_BYTE,		/* pxType */
    1,				/* pxAlignment */
    __glExtractTexelI8,		/* extractTexel */
    __glExtractTexelI8_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatColorIndex8 = {
    __GL_FORMAT_COLOR_INDEX8,	/* internalFormat */
    0,				/* indexSize */
    0,				/* redSize */
    0,				/* greenSize */
    0,				/* blueSize */
    0,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    8,				/* bitsPerTexel */
    GL_COLOR_INDEX,		/* pxFormat */
    GL_UNSIGNED_BYTE,		/* pxType */
    1,				/* pxAlignment */
    __glExtractTexelCI8,	/* extractTexel */
    __glExtractTexelCI8_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};

const __GLtextureFormat __glTexFormatColorIndex16 = {
    __GL_FORMAT_COLOR_INDEX16,	/* internalFormat */
    0,				/* indexSize */
    0,				/* redSize */
    0,				/* greenSize */
    0,				/* blueSize */
    0,				/* alphaSize */
    0,				/* luminanceSize */
    0,				/* intensitySize */
    16,				/* bitsPerTexel */
    GL_COLOR_INDEX,		/* pxFormat */
    GL_UNSIGNED_SHORT,		/* pxType */
    2,				/* pxAlignment */
    __glExtractTexelCI16,	/* extractTexel */
    __glExtractTexelCI16_B,	/* extractTexelBorder */
    NULL,			/* store */
    NULL,			/* fetch */
    NULL,			/* storeRect */
    NULL,			/* fetchRect */
};


/* Given a requested internal format, select the appropriate storage format.
 * The baseFormat is returned as a side effect.
 */
const __GLtextureFormat *
__glLookupTextureFormat(GLenum components, GLenum *baseFormat)
{
    switch (components) {
    case 1:
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
	*baseFormat = GL_LUMINANCE;
	return &__glTexFormatLuminance8;
    case 2:
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
	*baseFormat = GL_LUMINANCE_ALPHA;
	return &__glTexFormatLuminanceAlpha8;
    case 3:
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
	*baseFormat = GL_RGB;
	return &__glTexFormatRGB8;
    case 4:
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
	*baseFormat = GL_RGBA;
	return &__glTexFormatRGBA8;
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
	*baseFormat = GL_ALPHA;
	return &__glTexFormatAlpha8;
    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
    case GL_INTENSITY12:
    case GL_INTENSITY16:
	*baseFormat = GL_INTENSITY;
	return &__glTexFormatIntensity8;
    case GL_COLOR_INDEX1_EXT:
    case GL_COLOR_INDEX2_EXT:
    case GL_COLOR_INDEX4_EXT:
    case GL_COLOR_INDEX8_EXT:
	*baseFormat = GL_COLOR_INDEX;
	return &__glTexFormatColorIndex8;
    case GL_COLOR_INDEX12_EXT:
    case GL_COLOR_INDEX16_EXT:
	*baseFormat = GL_COLOR_INDEX;
	return &__glTexFormatColorIndex16;
    default:
	assert(0);
	return 0;
    }
}

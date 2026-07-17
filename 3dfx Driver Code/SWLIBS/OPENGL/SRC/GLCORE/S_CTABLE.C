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
#include "global.h"
#include "types.h"
#include "imports.h"
#include "ctable.h"
#include <string.h>
#include "g_imfncs.h"


/*
** Helper function to check color table arguments. Return non-zero if
** any argument is invalid. As a side effect, set the base format of
** the internal format, and the number of components to the format,
** as well as the number of bits in each component category.
**
** Note: The soft implementation ignores all but the base format.
*/
/*ARGSUSED*/
GLenum __glCheckColorTableArgs(__GLcontext *gc, GLenum target, 
				GLenum internalformat, GLsizei width, 
				GLenum format, GLenum type)
{
    GLenum baseFormat;

    switch(target) {
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
    case GL_PROXY_TEXTURE_1D:
    case GL_PROXY_TEXTURE_2D:
	break;
    default:
	return GL_INVALID_ENUM;
    }

    switch(internalformat) {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
	baseFormat = GL_ALPHA;
	break;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
	baseFormat = GL_LUMINANCE;
	break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
	baseFormat = GL_LUMINANCE_ALPHA;
	break;
    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
    case GL_INTENSITY12:
    case GL_INTENSITY16:
	baseFormat = GL_INTENSITY;
	break;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
	baseFormat = GL_RGB;
	break;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
	baseFormat = GL_RGBA;
	break;
    default:
	return GL_INVALID_ENUM;
    }

    /* width must be a positive power of two */
    if((width < 0) || (width & (width - 1))) {
	return GL_INVALID_VALUE;
    }

    /* Limit color table size so that pixel operations can be used */
    if((GLuint) width > __GL_MAX_SPAN_SIZE / 
       (__glBytesPerElement(type) * __glElementsPerGroup(baseFormat, type))) {
	return GL_TABLE_TOO_LARGE_EXT;
    }

    switch(format) {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_ABGR_EXT:
    case GL_BGR_EXT:
    case GL_BGRA_EXT:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
	break;
    default:
	return GL_INVALID_ENUM;
    }

    switch(type) {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
	break;
    default:
	return GL_INVALID_ENUM;
    }

    return (GLenum) 0;
}

/*
** Helper function to check color subtable arguments. Return non-zero if
** any argument is invalid. As a side effect, set the base format of
** the internal format, and the number of components to the format,
** as well as the number of bits in each component category.
**
** Note: The soft implementation ignores all but the base format.
*/
/*ARGSUSED*/
GLenum __glCheckColorSubTableArgs(__GLcontext *gc, GLenum target, 
				GLsizei start, GLsizei count, 
				GLenum format, GLenum type)
{
    switch(target) {
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
	break;
    default:
	return GL_INVALID_ENUM;
    }

    if(start < 0 || count < 0) {
	return GL_INVALID_VALUE;
    }

    switch(format) {
    case GL_RED:
    case GL_GREEN:
    case GL_BLUE:
    case GL_ALPHA:
    case GL_RGB:
    case GL_RGBA:
    case GL_ABGR_EXT:
    case GL_BGR_EXT:
    case GL_BGRA_EXT:
    case GL_LUMINANCE:
    case GL_LUMINANCE_ALPHA:
	break;
    default:
	return GL_INVALID_ENUM;
    }

    switch(type) {
    case GL_BYTE:
    case GL_UNSIGNED_BYTE:
    case GL_SHORT:
    case GL_UNSIGNED_SHORT:
    case GL_INT:
    case GL_UNSIGNED_INT:
    case GL_FLOAT:
	break;
    default:
	return GL_INVALID_ENUM;
    }

    return (GLenum) 0;
}


/*
** Load color table with target, internalformat, and derived size info
*/
void __glLoadColorTableParams(__GLcolorTable *ct, GLenum target,
			      GLenum internalformat, GLsizei width)
{
    ct->target = target;
    ct->format = internalformat;
    ct->width = width;

    switch(internalformat) {
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
	ct->redSize = 0;
	ct->greenSize = 0;
	ct->blueSize = 0;
	ct->alphaSize = 8;
	ct->luminanceSize = 0;
	ct->intensitySize = 0;
	ct->baseFormat = GL_ALPHA;
	ct->components = 1;
	ct->type = GL_UNSIGNED_BYTE;
	break;
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
	ct->redSize = 0;
	ct->greenSize = 0;
	ct->blueSize = 0;
	ct->alphaSize = 0;
	ct->luminanceSize = 8;
	ct->intensitySize = 0;
	ct->baseFormat = GL_LUMINANCE;
	ct->components = 1;
	ct->type = GL_UNSIGNED_BYTE;
	break;
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
	ct->redSize = 0;
	ct->greenSize = 0;
	ct->blueSize = 0;
	ct->alphaSize = 8;
	ct->luminanceSize = 8;
	ct->intensitySize = 0;
	ct->baseFormat = GL_LUMINANCE_ALPHA;
	ct->components = 2;
	ct->type = GL_UNSIGNED_BYTE;
	break;
    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
    case GL_INTENSITY12:
    case GL_INTENSITY16:
	ct->redSize = 0;
	ct->greenSize = 0;
	ct->blueSize = 0;
	ct->alphaSize = 0;
	ct->luminanceSize = 0;
	ct->intensitySize = 8;
	ct->baseFormat = GL_INTENSITY;
	ct->components = 1;
	ct->type = GL_UNSIGNED_BYTE;
	break;
    case GL_RGB:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
	ct->redSize = 8;
	ct->greenSize = 8;
	ct->blueSize = 8;
	ct->alphaSize = 0;
	ct->luminanceSize = 0;
	ct->intensitySize = 0;
	ct->baseFormat = GL_RGB;
	ct->components = 3;
	ct->type = GL_UNSIGNED_BYTE;
	break;
    case GL_RGBA:
    case GL_RGBA2:
    case GL_RGBA4:
    case GL_RGBA8:
    case GL_RGBA12:
    case GL_RGBA16:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
	ct->redSize = 8;
	ct->greenSize = 8;
	ct->blueSize = 8;
	ct->alphaSize = 8;
	ct->luminanceSize = 0;
	ct->intensitySize = 0;
	ct->baseFormat = GL_RGBA;
	ct->components = 4;
	ct->type = GL_UNSIGNED_BYTE;
	break;
    default:
	assert(0); /* was internal format type checked before now? */
	break;
    }

}

/*
** Set up spanInfo to pack a color table.
*/			      
static
void __glInitColorTableStore(__GLcolorTable *ct, __GLpixelSpanInfo *spanInfo)
{
    spanInfo->dstImage = ct->table;
    spanInfo->dstSkipPixels = 0;
    spanInfo->dstSkipLines = 0;
    spanInfo->dstSwapBytes = GL_FALSE;
    spanInfo->dstLsbFirst = GL_TRUE;
    spanInfo->dstLineLength = ct->width;
    spanInfo->dstFormat = ct->baseFormat;
    spanInfo->dstType = ct->type;
    spanInfo->dstAlignment = __glBytesPerElement(ct->type);
}

/*
** This routine is used by both the __glim and __gllc versions of ColorTableEXT
** it assumes error checking has been done, and requires a packed argument,
** used to determine the source of the pixel storage mode state.
*/
void __glColorTableEXT(__GLcontext *gc, GLenum target, GLenum internalformat, 
		       GLsizei width, GLenum format, GLenum type, 
		       const void *table, GLboolean packed)
{
    __GLcolorTable *ct;
    GLboolean proxy = GL_FALSE;
    __GLpixelSpanInfo spanInfo;
    int oldWidth;

    /* load the table */
    switch(target) {
    case GL_PROXY_TEXTURE_1D:
    case GL_PROXY_TEXTURE_2D:
	proxy = GL_TRUE;
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
	{
	    __GLtexture *tex = __glLookUpTexture(gc, target);
	    ct = &tex->CT;
	}
	break;
    }

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

    /* Update the array */
    if(!proxy) {
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

    }
}

/*
** This routine is used by both the __glim and __gllc versions of
** ColorSubTableEXT.  It assumes all error checking has been done except
** table size, and requires a packed argument, used to determine the source
** of the pixel storage mode state.
*/
void __glColorSubTableEXT(__GLcontext *gc, GLenum target, GLsizei start, 
			  GLsizei count, GLenum format, GLenum type, 
			  const void *table, GLboolean packed)
{
    __GLcolorTable *ct;
    __GLpixelSpanInfo spanInfo;

    /* load the table */
    switch(target) {
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
	{
	    __GLtexture *tex = __glLookUpTexture(gc, target);
	    ct = &tex->CT;
	}
	break;
    }

    if (start + count > ct->width) {
	__glSetError(GL_INVALID_VALUE);
	return;
    }

    /* Update the array */
    __glInitMemUnpack(gc, &spanInfo, count, 1, 0, format, type, table, packed);
    __glInitColorTableStore(ct, &spanInfo);

    spanInfo.dstSkipPixels = start;

    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);

    spanInfo.applyFbScale = GL_FALSE;

    (*gc->procs.copyImage)(gc, &spanInfo, GL_FALSE);
}


void APIENTRY __glim_ColorSubTableEXT(GLenum target, GLsizei start, GLsizei count,
			  GLenum format, GLenum type, const void *table)
{
    GLenum rvalue;
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE(); /* for pixel paths */
    __GL_API_STATE();
    
    /* check the arguments; compute internalformat numbers */
    if((rvalue = __glCheckColorSubTableArgs(gc, target, start, count, 
					    format, type))) {
	__glSetError(rvalue);
	return;
    }
    (*gc->procs.colorsubtable)(gc, target, start, count, format, type,
			       table, GL_FALSE);

    /* need generic to properly update texture color table changes */
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_PIXEL | __GL_DIRTY_GENERIC);
}

void APIENTRY __glim_ColorTableEXT(GLenum target, GLenum internalformat, GLsizei width,
			  GLenum format, GLenum type, const void *table)
{
    GLenum rvalue;
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE(); /* for pixel paths */
    __GL_API_STATE();
    
    /* check the arguments; compute internalformat numbers */
    if((rvalue = __glCheckColorTableArgs(gc, target, internalformat, width, 
					 format, type))) {
	switch(target) {
	case GL_PROXY_TEXTURE_1D:
	case GL_PROXY_TEXTURE_2D:
	    if(rvalue == GL_TABLE_TOO_LARGE_EXT) {
		internalformat = (GLenum)0; /* proceed with format == 0 */
	    } else {
		__glSetError(rvalue);
		return;
	    }
	    break;
	default: /* normal error behavior */
	    __glSetError(rvalue);
	    return;
	}
    }
    (*gc->procs.colortable)(gc, target, internalformat, width, format, type,
			    table, GL_FALSE);

    /* need generic to properly update texture color table changes */
    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_PIXEL | __GL_DIRTY_GENERIC);
}

void APIENTRY __glim_GetColorTableEXT(GLenum target, GLenum format, GLenum type,
			     void *table)
{
    __GLcolorTable *ct;
    __GLpixelSpanInfo spanInfo;
    GLenum rvalue;
    __GL_SETUP_NOT_IN_BEGIN_VALIDATE(); /* for pixel paths */
    __GL_API_GET();

    /* 
    ** Re-use colortable parameter checking by passing known good values
    ** as some of the arguments.
    */
    rvalue = __glCheckColorTableArgs(gc, target, GL_RGB, 0, format, type);
    if(rvalue) {
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    /* Get the table pointer; proxy not a valid target for GetColorTableEXT */
    switch(target) {
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
	{
	    __GLtexture *tex = __glLookUpTexture(gc, target);
	    ct = &tex->CT;
	}

    case GL_PROXY_TEXTURE_1D:
    case GL_PROXY_TEXTURE_2D:
	__glSetError(GL_INVALID_ENUM);
	return;
    default:
	assert(0); /* did __glCheckColorTableArgs() miss an invalid format? */
	return;
    }
    
    __glInitMemGet(gc, &spanInfo, ct->width, 1, ct->baseFormat, ct->table);
    __glInitMemPack(gc, &spanInfo, ct->width, 1, 0, format, type, table);

    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);

    spanInfo.zeroFillAlpha = GL_TRUE;
    spanInfo.applyFbScale = GL_FALSE;
    
    (*gc->procs.copyImage)(gc, &spanInfo, GL_FALSE);

}


/*
** Return null pointer on failure. Size returns number of words to copy.
*/
GLint *__glColorTableModifiers(__GLcontext *gc, GLenum target, GLenum pname)
{
    __GLcolorTable *ct;

    switch(target) {
    case GL_PROXY_TEXTURE_1D:
    case GL_PROXY_TEXTURE_2D:
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
	{
	    __GLtexture *tex = __glLookUpTexture(gc, target);
	    ct = &tex->CT;
	}
	break;
    default:
	__glSetError(GL_INVALID_ENUM);
	return 0;
    }

    switch(pname) {
    case GL_COLOR_TABLE_FORMAT_EXT:
	return (GLint *) &ct->format;
    case GL_COLOR_TABLE_WIDTH_EXT:
	return &ct->width;
    case GL_COLOR_TABLE_RED_SIZE_EXT:
	return &ct->redSize;
    case GL_COLOR_TABLE_GREEN_SIZE_EXT:
	return &ct->greenSize;
    case GL_COLOR_TABLE_BLUE_SIZE_EXT:
	return &ct->blueSize;
    case GL_COLOR_TABLE_ALPHA_SIZE_EXT:
	return &ct->alphaSize;
    case GL_COLOR_TABLE_LUMINANCE_SIZE_EXT:
	return &ct->luminanceSize;
    case GL_COLOR_TABLE_INTENSITY_SIZE_EXT:
	return &ct->intensitySize;
    default:
	__glSetError(GL_INVALID_ENUM);
	return 0;
    }
}


static
void __glGetColorTableParameterv(__GLcontext *gc, GLenum target, GLenum pname,
				 int *iparams, float *fparams)
{
    GLint *modifiers;

    if(!(modifiers = __glColorTableModifiers(gc, target, pname)))
	return;
    
    if(fparams)
	*fparams = *modifiers;
    if(iparams)
	*iparams = *modifiers;
    return;
}

void APIENTRY __glim_GetColorTableParameterivEXT(GLenum target, GLenum pname,
					int *params)
{
    __GL_SETUP();
    __GL_API_GET();
    __glGetColorTableParameterv(gc, target, pname, params, (float *)0);
}

void APIENTRY __glim_GetColorTableParameterfvEXT(GLenum target, GLenum pname,
					float *params)
{
    __GL_SETUP();
    __GL_API_GET();
    __glGetColorTableParameterv(gc, target, pname, (int *)0, params);
}


void APIENTRY __glim_CopyColorTableEXT(GLenum target, GLenum internalformat,
			      GLint x, GLint y, GLsizei width)
{
    __GLcolorTable *ct;
    GLenum rvalue;
    __GLpixelSpanInfo spanInfo;
    int oldWidth;
    __GLtexture *tex = NULL;

    __GL_SETUP();
    __GL_API_PIXEL_OP();

    /* reuse function by filling in missing args with legal values */
    if((rvalue = __glCheckColorTableArgs(gc, target, internalformat, width, 
					 GL_RGBA, GL_FLOAT))) {
	__glSetError(rvalue);
    }
    switch(target) { /* proxy tables aren't valid targets for copycolortable */
    case GL_PROXY_TEXTURE_1D:
    case GL_PROXY_TEXTURE_2D:
	__glSetError(GL_INVALID_ENUM);
	return;
    }
    /* load the table */
    switch(target) {
    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
	{
	    tex = __glLookUpTexture(gc, target);
	    ct = &tex->CT;
	}
	break;
    }

    oldWidth = ct->width * __glElementsPerGroup(ct->baseFormat, ct->type);

    __glLoadColorTableParams(ct, target, internalformat, width);

    /* malloc table if it needs it */
    if(oldWidth < width * __glElementsPerGroup(ct->baseFormat, ct->type)) {
	ct->table = (*gc->imports.realloc)(gc, ct->table, width * 
		     __glElementsPerGroup(ct->baseFormat, ct->type) *
		     __glBytesPerElement(ct->type));
    }

    __glInitReadImageSrcInfo(gc, &spanInfo, x, y, width, 1);
    __glInitColorTableStore(ct, &spanInfo);

    __glInitUnpacker(gc, &spanInfo);
    __glInitPacker(gc, &spanInfo);

    if (!__glClipReadPixels(gc, &spanInfo)) return;
    (*gc->procs.readImage)(gc, tex, &spanInfo);

    __GL_DELAY_VALIDATE_MASK(gc, __GL_DIRTY_PIXEL);
}

void __glInitColorTables(__GLcontext *gc)
{
    /*
    ** Initialize color tables
    */
}

void __glFreeColorTables(__GLcontext *gc)
{
    /* 
    ** Free color tables if they've been allocated 
    */
}

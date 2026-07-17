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
** $Date: 10/11/00 7:55:08 PM$
*/

#ifdef NEW_LISTS
#include "render.h"
#include "global.h"
#include "context.h"
#include "pixel.h"
#include "g_lcomp.h"
#include "g_listop.h"
#include "g_lcfncs.h"
#include "image.h"
#include "dlist.h"
#include "dlistopt.h"
#include "imports.h"

#include <string.h> /*XXX imports.h please*/

extern GLvoid *__glSPDlalloc(GLint size, GLint func); /*XXXblythe*/
extern GLvoid __glsplc_InvalidEnum(GLvoid);
extern GLvoid __glsplc_InvalidValue(GLvoid);

extern const GLubyte __GLdlsize_tab[];

#define __glCallListsSize(type)				\
	((type) >= GL_BYTE && (type) <= GL_4_BYTES ?	\
	__GLdlsize_tab[(type)-GL_BYTE] : -1)

/*
** Display list compilation and execution versions of CallList and CallLists
** are maintained here for the sake of sanity.  Note that __glle_CallList
** may not call __glim_CallList or it will break the infinite recursive
** display list prevention code.
*/
void __glsplc_CallList(GLuint list)
{
    struct __gllc_CallList_Rec *data;

    if (list == 0) {
	__glsplc_InvalidEnum();
	return;
    }

    data = __glSPDlalloc(sizeof(struct __gllc_CallList_Rec), __glop_CallList);
    if (data == NULL) return;
    data->list = list;
}

void __glsplc_CallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
    GLuint size;
    GLint arraySize;
    struct __gllc_CallLists_Rec *data;

    if (n < 0) {
	__glsplc_InvalidValue();
	return;
    }
    arraySize = n*__glCallListsSize(type);
    if (arraySize < 0) {
	__glsplc_InvalidEnum();
	return;
    }
    size = (GLuint)sizeof(struct __gllc_CallLists_Rec) + __GL_PAD(arraySize);
    data = __glSPDlalloc(size, __glop_CallLists);
    if (data == NULL) return;
    data->n = n;
    data->type = type;
    __GL_MEMCOPY((GLubyte *)data + sizeof(struct __gllc_CallLists_Rec),
		 lists, arraySize);
}

#define __GL_IMAGE_BITMAP	0
#define __GL_IMAGE_INDICES	1
#define __GL_IMAGE_RGBA		2

void __glsplc_Bitmap(GLsizei width, GLsizei height,
		   GLfloat xorig, GLfloat yorig, 
		   GLfloat xmove, GLfloat ymove, 
		   const GLubyte *oldbits)
{
    __GLbitmap *bitmap;
    GLubyte *newbits;
    GLint imageSize;
    __GL_SETUP();

    if ((width < 0) || (height < 0)) {
	__glsplc_InvalidValue();
	return;
    }

    imageSize = height * ((width + 7) >> 3);
    imageSize = __GL_PAD(imageSize);

    bitmap = __glSPDlalloc(imageSize + (GLint)sizeof(__GLbitmap), __glop_Bitmap);
    if (bitmap == NULL) return;

    bitmap->width = width;
    bitmap->height = height;
    bitmap->xorig = xorig;
    bitmap->yorig = yorig;
    bitmap->xmove = xmove;
    bitmap->ymove = ymove;
    bitmap->imageSize = imageSize;

    newbits = (GLubyte *)bitmap + sizeof(__GLbitmap); 
    __glFillImage(gc, width, height, GL_COLOR_INDEX, GL_BITMAP, 
	    oldbits, newbits);
}

void __glsplc_PolygonStipple(const GLubyte *mask)
{
    GLvoid *data;
    __GL_SETUP();
    GLubyte *newbits;

    data = __glSPDlalloc(__glImageSize(32, 32, GL_COLOR_INDEX, GL_BITMAP),
			     __glop_PolygonStipple);
    if (data == NULL) return;

    newbits = (GLubyte *)data;
    __glFillImage(gc, 32, 32, GL_COLOR_INDEX, GL_BITMAP, mask, newbits);
}

struct __gllc_Map1f_Rec {
        GLenum    target;
        __GLfloat u1;
        __GLfloat u2;
        GLint     order;
        /*        points  */
};

void __glsplc_Map1f(GLenum target, 
		  GLfloat u1, GLfloat u2,
		  GLint stride, GLint order,
		  const GLfloat *points)
{
    struct __gllc_Map1f_Rec *map1data;
    GLint k;
    GLint cmdsize;
    __GLfloat *data;
    __GL_SETUP();
    
    k=__glEvalComputeK(target);
    if (k < 0) {
	__glsplc_InvalidEnum();
	return;
    }

    if (order > gc->constants.maxEvalOrder || stride < k ||
	    order < 1 || u1 == u2) {
	__glsplc_InvalidValue();
	return;
    }

    cmdsize = (GLint)sizeof(*map1data) + 
	    __glMap1_size(k, order) * (GLint)sizeof(__GLfloat);

    map1data = __glSPDlalloc(cmdsize, __glop_Map1f);
    if (map1data == NULL) return;

    map1data->target = target;
    map1data->u1 = u1;
    map1data->u2 = u2;
    map1data->order = order;
    data = (__GLfloat *) ((GLubyte *)map1data + sizeof(*map1data));
    __glFillMap1f(k, order, stride, points, data);
}

void __glsplc_Map1d(GLenum target, 
		  GLdouble u1, GLdouble u2,
		  GLint stride, GLint order, 
		  const GLdouble *points)
{
    struct __gllc_Map1f_Rec *map1data;
    GLint k;
    GLint cmdsize;
    __GLfloat *data;
    __GL_SETUP();
    
    k=__glEvalComputeK(target);
    if (k < 0) {
	__glsplc_InvalidEnum();
	return;
    }

    if (order > gc->constants.maxEvalOrder || stride < k ||
	    order < 1 || u1 == u2) {
	__glsplc_InvalidValue();
	return;
    }

    cmdsize = (GLint)sizeof(*map1data) + 
	    __glMap1_size(k, order) * (GLint)sizeof(__GLfloat);

    map1data = __glSPDlalloc(cmdsize, __glop_Map1d);
    if (map1data == NULL) return;

    map1data->target = target;
    map1data->u1 = u1;
    map1data->u2 = u2;
    map1data->order = order;
    data = (__GLfloat *) ((GLubyte *)map1data + sizeof(*map1data));
    __glFillMap1d(k, order, stride, points, data);
}

struct __gllc_Map2f_Rec {
        GLenum    target;
        __GLfloat u1;
        __GLfloat u2;
        GLint     uorder;
        __GLfloat v1;
        __GLfloat v2;
        GLint     vorder;
	__4_BYTE_PAD
        /*        points  */
};

void __glsplc_Map2f(GLenum target, 
		  GLfloat u1, GLfloat u2,
		  GLint ustride, GLint uorder, 
		  GLfloat v1, GLfloat v2,
		  GLint vstride, GLint vorder, 
		  const GLfloat *points)
{
    struct __gllc_Map2f_Rec *map2data;
    GLint k;
    GLint cmdsize;
    __GLfloat *data;
    __GL_SETUP();

    k=__glEvalComputeK(target);
    if (k < 0) {
	__glsplc_InvalidEnum();
	return;
    }

    if (vorder > gc->constants.maxEvalOrder || vstride < k ||
	    vorder < 1 || u1 == u2 || ustride < k ||
	    uorder > gc->constants.maxEvalOrder || uorder < 1 ||
	    v1 == v2) {
	__glsplc_InvalidValue();
	return;
    }

    cmdsize = (GLint)sizeof(*map2data) + 
	    __glMap2_size(k, uorder, vorder) * (GLint)sizeof(__GLfloat);

    map2data = __glSPDlalloc(cmdsize, __glop_Map2f);
    if (map2data == NULL) return;

    map2data->target = target;
    map2data->u1 = u1;
    map2data->u2 = u2;
    map2data->uorder = uorder;
    map2data->v1 = v1;
    map2data->v2 = v2;
    map2data->vorder = vorder;

    data = (__GLfloat *) ((GLubyte *)map2data + sizeof(*map2data));
    __glFillMap2f(k, uorder, vorder, ustride, vstride, points, data);
}

void __glsplc_Map2d(GLenum target, 
		  GLdouble u1, GLdouble u2,
                  GLint ustride, GLint uorder, 
		  GLdouble v1, GLdouble v2,
		  GLint vstride, GLint vorder, 
		  const GLdouble *points)
{
    struct __gllc_Map2f_Rec *map2data;
    GLint k;
    GLint cmdsize;
    __GLfloat *data;
    __GL_SETUP();

    k=__glEvalComputeK(target);
    if (k < 0) {
	__glsplc_InvalidEnum();
	return;
    }

    if (vorder > gc->constants.maxEvalOrder || vstride < k ||
	    vorder < 1 || u1 == u2 || ustride < k ||
	    uorder > gc->constants.maxEvalOrder || uorder < 1 ||
	    v1 == v2) {
	__glsplc_InvalidValue();
	return;
    }

    cmdsize = (GLint)sizeof(*map2data) + 
	    __glMap2_size(k, uorder, vorder) * (GLint)sizeof(__GLfloat);

    map2data = __glSPDlalloc(cmdsize, __glop_Map2d);
    if (map2data == NULL) return;

    map2data->target = target;
    map2data->u1 = u1;
    map2data->u2 = u2;
    map2data->uorder = uorder;
    map2data->v1 = v1;
    map2data->v2 = v2;
    map2data->vorder = vorder;

    data = (__GLfloat *) ((GLubyte *)map2data + sizeof(*map2data));
    __glFillMap2d(k, uorder, vorder, ustride, vstride, points, data);
}

void __glsplc_DrawPixels(GLint width, GLint height, GLenum format, 
		       GLenum type, const GLvoid *pixels)
{
    struct __gllc_DrawPixels_Rec *pixdata;
    GLint imageSize;
    GLboolean index;
    __GL_SETUP();

    if ((width < 0) || (height < 0)) {
	__glsplc_InvalidValue();
	return;
    }
    switch (format) {
      case GL_STENCIL_INDEX:
      case GL_COLOR_INDEX:
	index = GL_TRUE;
	break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_DEPTH_COMPONENT:
      case GL_ABGR_EXT:
      case GL_BGR_EXT:
      case GL_BGRA_EXT:
	index = GL_FALSE;
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }
    switch (type) {
      case GL_BITMAP:
	if (!index) {
	    __glsplc_InvalidEnum();
	    return;
	}
	break;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }

    imageSize = __glImageSize(width, height, format, type);
    imageSize = __GL_PAD(imageSize);

    pixdata = __glSPDlalloc((GLint)sizeof(*pixdata) + imageSize, __glop_DrawPixels);
    if (pixdata == NULL) return;

    pixdata->width = width;
    pixdata->height = height;
    pixdata->format = format;
    pixdata->type = type;

    __glFillImage(gc, width, height, format, type, pixels, 
	    (GLubyte *)pixdata + sizeof(*pixdata));
}

void __glsplc_TexImage1D(GLenum target, GLint level, 
		       GLint components,
		       GLint width, GLint border, GLenum format, 
		       GLenum type, const GLvoid *pixels)
{
    struct __gllc_TexImage1D_Rec *texdata;
    GLint imageSize;
    GLboolean index;
    __GL_SETUP();

    if (target == GL_PROXY_TEXTURE_1D) {
	(*gc->dispatchState->dispatch.TexImage1D)(target, level, components,
				width, border, format, type, pixels);
	return;
    }

    if (border < 0 || border > 1) {
	__glsplc_InvalidValue();
	return;
    }
    if (width < 0) {
	__glsplc_InvalidValue();
	return;
    }
    switch (format) {
      case GL_COLOR_INDEX:
	index = GL_TRUE;
	break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_ABGR_EXT:
      case GL_BGR_EXT:
      case GL_BGRA_EXT:
	index = GL_FALSE;
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }
    switch (type) {
      case GL_BITMAP:
	if (!index) {
	    __glsplc_InvalidEnum();
	    return;
	}
	break;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }

    imageSize = __glImageSize(width, 1, format, type);
    imageSize = __GL_PAD(imageSize);

    texdata = __glSPDlalloc((GLint)sizeof(*texdata) + imageSize, __glop_TexImage1D);
    if (texdata == NULL) return;

    texdata->target = target;
    texdata->level = level;
    texdata->components = components;
    texdata->width = width;
    texdata->border = border;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0 && pixels != NULL) {
	__glFillImage(gc, width, 1, format, type, pixels, 
		(GLubyte *)texdata + sizeof(*texdata));
    }
}

void __glsplc_TexImage2D(GLenum target, GLint level, 
		       GLint components,
		       GLint width, GLint height, GLint border, 
		       GLenum format, GLenum type, 
		       const GLvoid *pixels)
{
    struct __gllc_TexImage2D_Rec *texdata;
    GLint imageSize;
    GLboolean index;
    __GL_SETUP();

    if (target == GL_PROXY_TEXTURE_2D) {
	(*gc->dispatchState->dispatch.TexImage2D)(target, level, components,
				width, height, border, format, type, pixels);
	return;
    }

    if (border < 0 || border > 1) {
	__glsplc_InvalidValue();
	return;
    }
    if ((width < 0) || (height < 0)) {
	__glsplc_InvalidValue();
	return;
    }
    switch (format) {
      case GL_COLOR_INDEX:
	index = GL_TRUE;
	break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_ABGR_EXT:
      case GL_BGR_EXT:
      case GL_BGRA_EXT:
	index = GL_FALSE;
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }
    switch (type) {
      case GL_BITMAP:
	if (!index) {
	    __glsplc_InvalidEnum();
	    return;
	}
	break;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }

    imageSize = __glImageSize(width, height, format, type);
    imageSize = __GL_PAD(imageSize);

    texdata = __glSPDlalloc((GLint)sizeof(*texdata) + imageSize, __glop_TexImage2D);
    if (texdata == NULL) return;

    texdata->target = target;
    texdata->level = level;
    texdata->components = components;
    texdata->width = width;
    texdata->height = height;
    texdata->border = border;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0 && pixels != NULL) {
	__glFillImage(gc, width, height, format, type, pixels, 
		(GLubyte *)texdata + sizeof(*texdata));
    }
}

void __glsplc_TexSubImage1D(GLenum target, GLint level,
			     GLsizei xoffset, GLsizei width,
			     GLenum format, GLenum type, const GLvoid *pixels)
{
    struct __gllc_TexSubImage1D_Rec *texdata;
    GLint imageSize;
    GLboolean index;
    __GL_SETUP();

    if (width < 0) {
	__glsplc_InvalidValue();
	return;
    }
    switch (format) {
      case GL_COLOR_INDEX:
	index = GL_TRUE;
	break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_ABGR_EXT:
      case GL_BGR_EXT:
      case GL_BGRA_EXT:
	index = GL_FALSE;
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }
    switch (type) {
      case GL_BITMAP:
	if (!index) {
	    __glsplc_InvalidEnum();
	    return;
	}
	break;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }

    imageSize = __glImageSize(width, 1, format, type);
    imageSize = __GL_PAD(imageSize);

    texdata = __glSPDlalloc((GLint)sizeof(*texdata) + imageSize,
			 __glop_TexSubImage1D);
    if (texdata == NULL) return;

    texdata->target = target;
    texdata->level = level;
    texdata->xoffset = xoffset;
    texdata->width = width;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0 && pixels != NULL) {
	__glFillImage(gc, width, 1, format, type, pixels, 
		(GLubyte *)texdata + sizeof(*texdata));
    }
}

void __glsplc_TexSubImage2D(GLenum target, GLint level,
			     GLsizei xoffset, GLsizei yoffset,
			     GLsizei width, GLsizei height,
			     GLenum format, GLenum type, const GLvoid *pixels)
{

    struct __gllc_TexSubImage2D_Rec *texdata;
    GLint imageSize;
    GLboolean index;
    __GL_SETUP();

    if ((width < 0) || (height < 0)) {
	__glsplc_InvalidValue();
	return;
    }
    switch (format) {
      case GL_COLOR_INDEX:
	index = GL_TRUE;
	break;
      case GL_RED:
      case GL_GREEN:
      case GL_BLUE:
      case GL_ALPHA:
      case GL_RGB:
      case GL_RGBA:
      case GL_LUMINANCE:
      case GL_LUMINANCE_ALPHA:
      case GL_ABGR_EXT:
      case GL_BGR_EXT:
      case GL_BGRA_EXT:
	index = GL_FALSE;
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }
    switch (type) {
      case GL_BITMAP:
	if (!index) {
	    __glsplc_InvalidEnum();
	    return;
	}
	break;
      case GL_BYTE:
      case GL_UNSIGNED_BYTE:
      case GL_SHORT:
      case GL_UNSIGNED_SHORT:
      case GL_INT:
      case GL_UNSIGNED_INT:
      case GL_FLOAT:
	break;
      default:
	__glsplc_InvalidEnum();
	return;
    }

    imageSize = __glImageSize(width, height, format, type);
    imageSize = __GL_PAD(imageSize);

    texdata = __glSPDlalloc((GLint)sizeof(*texdata) + imageSize,
			 __glop_TexSubImage2D);
    if (texdata == NULL) return;

    texdata->target = target;
    texdata->level = level;
    texdata->xoffset = xoffset;
    texdata->yoffset = yoffset;
    texdata->width = width;
    texdata->height = height;
    texdata->format = format;
    texdata->type = type;
    texdata->imageSize = imageSize;

    if (imageSize > 0 && pixels != NULL) {
	__glFillImage(gc, width, height, format, type, pixels, 
		(GLubyte *) texdata + sizeof(*texdata));
    }
}

void __glsplc_Disable(GLenum cap)
{
    struct __gllc_Disable_Rec *data;

    if (( cap >= GL_VERTEX_ARRAY_EXT ) && ( cap <= GL_EDGE_FLAG_ARRAY_EXT)) {
        __GLcontext *pGC = (__GLcontext *)__gl_context;
	
	switch (cap) {
	  case GL_VERTEX_ARRAY_EXT:
	    pGC->vertexArray.mask &= ~VERTARRAY_V_MASK;
	    pGC->vertexArray.index &= ~VERTARRAY_V_INDEX;
	    return;
	  case GL_NORMAL_ARRAY_EXT:
	    pGC->vertexArray.mask &= ~VERTARRAY_N_MASK;
	    pGC->vertexArray.index &= ~VERTARRAY_N_INDEX;
	    return;
	  case GL_COLOR_ARRAY_EXT:
	    pGC->vertexArray.mask &= ~VERTARRAY_C_MASK;
	    pGC->vertexArray.index &= ~VERTARRAY_C_INDEX;
	    return;
	  case GL_INDEX_ARRAY_EXT:
	    pGC->vertexArray.mask &= ~VERTARRAY_I_MASK;
	    pGC->vertexArray.index &= ~VERTARRAY_I_INDEX;
	    return;
	  case GL_TEXTURE_COORD_ARRAY_EXT:
	    pGC->vertexArray.mask &= ~VERTARRAY_T_MASK;
	    pGC->vertexArray.index &= ~VERTARRAY_T_INDEX;
	    return;
	  case GL_EDGE_FLAG_ARRAY_EXT:
	    pGC->vertexArray.mask &= ~VERTARRAY_E_MASK;
	    pGC->vertexArray.index &= ~VERTARRAY_E_INDEX;
	    return;
	}
    }

    data = (struct __gllc_Disable_Rec *) __glSPDlalloc(sizeof(struct __gllc_Disable_Rec), __glop_Disable);
    if (data == NULL) return;
    data->cap = cap;
}

void __glsplc_Enable(GLenum cap)
{
    struct __gllc_Enable_Rec *data;

    if (( cap >= GL_VERTEX_ARRAY_EXT ) && ( cap <= GL_EDGE_FLAG_ARRAY_EXT)) {
        __GLcontext *pGC = (__GLcontext *)__gl_context;
	
	switch (cap) {
	case GL_VERTEX_ARRAY_EXT:
	    pGC->vertexArray.mask |= VERTARRAY_V_MASK;
	    pGC->vertexArray.index |= VERTARRAY_V_INDEX;
	    return;
	case GL_NORMAL_ARRAY_EXT:
	    pGC->vertexArray.mask |= VERTARRAY_N_MASK;
	    pGC->vertexArray.index |= VERTARRAY_N_INDEX;
	    return;
	case GL_COLOR_ARRAY_EXT:
	    pGC->vertexArray.mask |= VERTARRAY_C_MASK;
	    pGC->vertexArray.index |= VERTARRAY_C_INDEX;
	    return;
	case GL_INDEX_ARRAY_EXT:
	    pGC->vertexArray.mask |= VERTARRAY_I_MASK;
	    pGC->vertexArray.index |= VERTARRAY_I_INDEX;
	    return;
	case GL_TEXTURE_COORD_ARRAY_EXT:
	    pGC->vertexArray.mask |= VERTARRAY_T_MASK;
	    pGC->vertexArray.index |= VERTARRAY_T_INDEX;
	    return;
	case GL_EDGE_FLAG_ARRAY_EXT:
	    pGC->vertexArray.mask |= VERTARRAY_E_MASK;
	    pGC->vertexArray.index |= VERTARRAY_E_INDEX;
	    return;
	}
    }
    data = (struct __gllc_Enable_Rec *) __glSPDlalloc(sizeof(struct __gllc_Enable_Rec), __glop_Enable);
    if (data == NULL) return;
    data->cap = cap;
}

void 
__glsplc_ColorTableEXT(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid* table)
{
    GLuint size;
    GLint tableSize;
    struct __gllc_ColorTableEXT_Rec *data;

    /*XXX isn't some error checking needed here? */

    tableSize = __glImageSize(width, 1, format, type);
    tableSize = __GL_PAD(tableSize);
    size = (int)sizeof(*data) + tableSize;
    data = (struct __gllc_ColorTableEXT_Rec *) __glSPDlalloc(size, __glop_ColorTableEXT);
    if (data == NULL) return;
    data->target = target;
    data->internalformat = internalformat;
    data->width = width;
    data->format = format;
    data->type = type;
    data->imageSize = tableSize;
    __GL_MEMCOPY((GLubyte *)data + sizeof(*data), table, tableSize);
}
#endif /* NEW_LISTS */

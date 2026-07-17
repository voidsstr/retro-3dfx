#ifndef _gl_ctable_h_
#define _gl_ctable_h_

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
#include "types.h"

/*
** Generic color table state
** different tables require different table types
*/
typedef struct __GLcolorTableRec {
    GLenum target; /* equal to target used to access this table */
    GLubyte *table;  /* look-up table contents */
    GLenum format; /* table entry format */
    GLenum type;   /* table element type */
    GLsizei width;
    GLsizei redSize;
    GLsizei greenSize;
    GLsizei blueSize;
    GLsizei alphaSize;
    GLsizei luminanceSize;
    GLsizei intensitySize;
    GLenum baseFormat; /* input format, ignoring component bit sizes */
    GLint components; /* number of components in format */
} __GLcolorTable;

/*
** Function prototypes
*/
void __glColorTableEXT(__GLcontext *gc, GLenum target, GLenum internalformat, 
		       GLsizei width, GLenum format, GLenum type, 
		       const void *table, GLboolean packed);

GLenum __glCheckColorTableArgs(__GLcontext *gc, GLenum target, 
			GLenum internalformat, GLsizei width, 
			GLenum format, GLenum type);

void __glColorSubTableEXT(__GLcontext *gc, GLenum target, GLsizei start, 
		       GLsizei count, GLenum format, GLenum type, 
		       const void *table, GLboolean packed);

GLenum __glCheckColorSubTableArgs(__GLcontext *gc, GLenum target, 
			GLsizei start, GLsizei count, 
			GLenum format, GLenum type);

void __glInitColorTables(__GLcontext *gc);

void __glFreeColorTables(__GLcontext *gc);

void __glLoadColorTableParams(__GLcolorTable *ct, GLenum target,
			      GLenum internalformat, GLsizei width);

#endif /* _gl_ctable_h_ */

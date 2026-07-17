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
#include "context.h"
#include "render.h"

/*
** Initialize a lookup table that is indexed by color index values.
** The table indicates whether the index test passed or failed, based on
** the current index function and the index reference value.
**
*/
void __glValidateIndexTest(__GLcontext *gc)
{
    GLubyte *itft;
    GLint i, limit;
    GLint ref;
    GLenum indexTestFunc = gc->state.raster.indexFunction;

    if (gc->modes.rgbMode)
	return;

    limit = 1 << gc->modes.indexBits;
    ref = (GLint) gc->state.raster.indexReference;

    /*
    ** Allocate index test function table the first time.  It needs
    ** to have at most one entry for each possible index value.
    */
    itft = gc->frontBuffer.indexTestFuncTable;
    if (!itft) {
        itft = (GLubyte*)
	    (*gc->imports.malloc)(gc, (size_t) ((limit) * sizeof(GLubyte)));
	gc->frontBuffer.indexTestFuncTable = itft;
    }

    /*
    ** Build up index test lookup table.  The computed index value is
    ** used as an index into this table to determine if the index
    ** test passed or failed.
    */
    for (i = 0; i < limit; i++) {
	switch (indexTestFunc) {
	  case GL_NEVER:	itft[i] = GL_FALSE; break;
	  case GL_LESS:		itft[i] = (GLubyte) (i <  ref); break;
	  case GL_EQUAL:	itft[i] = (GLubyte) (i == ref); break;
	  case GL_LEQUAL:	itft[i] = (GLubyte) (i <= ref); break;
	  case GL_GREATER:	itft[i] = (GLubyte) (i >  ref); break;
	  case GL_NOTEQUAL:	itft[i] = (GLubyte) (i != ref); break;
	  case GL_GEQUAL:	itft[i] = (GLubyte) (i >= ref); break;
	  case GL_ALWAYS:	itft[i] = GL_TRUE; break;
	}
    }
}

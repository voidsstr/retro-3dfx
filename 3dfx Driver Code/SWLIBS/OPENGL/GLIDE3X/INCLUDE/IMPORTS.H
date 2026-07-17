#ifndef __gl_imports_h_
#define __gl_imports_h_

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
** Imports from outside libraries.
**
** $Revision: 2$
** $Date: 10/11/00 7:57:23 PM$
*/
#include <memory.h>
#include <string.h>
#include <math.h>

#define __GL_MEMCOPY(to,from,count)	memcpy(to,from,(size_t)(count))
#define __GL_MEMZERO(to,count)		memset(to,0,(size_t)(count))

/* Use this for overlapping copies!! Often slower than memcpy */
#define __GL_MEMMOVE(to,from,count)	memmove(to,from,(size_t)(count))

#ifdef __sgi
#define __GL_CEILF(f)			ceilf(f)
#define __GL_SQRTF(f)			sqrtf(f)
#define __GL_POWF(a,b)			powf(a,b)
#define __GL_ABSF(f)			fabsf(f)
#define __GL_FLOORF(f)			floorf(f)
#define __GL_FLOORD(f)			floor(f)
#define __GL_SINF(f)			sinf(f)
#define __GL_COSF(f)			cosf(f)
#define __GL_ATANF(f)			atanf(f)
#define __GL_LOGF(f)			logf(f)
#else
#define __GL_CEILF(f)			((GLfloat)ceil((double) (f)))
#define __GL_SQRTF(f)			((GLfloat)sqrt((double) (f)))
#define __GL_POWF(a,b)			((GLfloat)pow((double) (a),(double) (b)))
#define __GL_ABSF(f)			((GLfloat)fabs((double) (f)))
#define __GL_FLOORF(f)			((GLfloat)floor((double) (f)))
#define __GL_FLOORD(f)			floor(f)
#define __GL_SINF(f)			((GLfloat)sin((double) (f)))
#define __GL_COSF(f)			((GLfloat)cos((double) (f)))
#define __GL_ATANF(f)			((GLfloat)atan((double) (f)))
#define __GL_LOGF(f)			((GLfloat)log((double) (f)))
#endif

#endif /* __gl_imports_h_ */

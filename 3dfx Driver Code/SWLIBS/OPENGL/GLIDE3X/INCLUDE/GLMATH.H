#ifndef __gl_math_h_
#define __gl_math_h_

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
** $Date: 10/11/00 7:57:20 PM$
*/
#include "types.h"
#include "imports.h"

/* extern __GLfloat __glLog2(__GLfloat x); */
extern GLuint __glFloorLog2(GLuint n);

#ifdef __GL_USE_MIPSASMCODE
/*
** routines in mips_texutil.s
*/
extern GLfloat __glFrac( __GLfloat f );
extern GLint __glFloor( __GLfloat f );
#endif /* __GL_USE_MIPSASMCODE */

#endif /* __gl_math_h_ */

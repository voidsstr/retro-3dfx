#ifndef __gl_cpu_h_
#define __gl_cpu_h_

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
** CPU dependent constants.
*/

#ifdef __sgi
/* Mips Big Endian */

#define __GL_BITS_PER_BYTE	8
#define __GL_STIPPLE_MSB	1

/* IEEE Single Precision Floating Point */
typedef union __GLcpuFloatRec {
    __GLfloat f;
    unsigned int ul;
    struct {
	unsigned int sign : 1;
	unsigned int exponent : 8;
	unsigned int mantissa : 23;
    } field;
} __GLcpuFloat;

#define __GL_FLOAT_MANTISSA_BITS	23
#define __GL_FLOAT_MANTISSA_SHIFT	0
#define __GL_FLOAT_EXPONENT_BIAS	127
#define __GL_FLOAT_EXPONENT_BITS	8
#define __GL_FLOAT_EXPONENT_SHIFT	23
#define __GL_FLOAT_SIGN_SHIFT		31

#endif /* __sgi */

#ifdef _M_IX86
/* Intel x86 Little Endian */

#define __GL_BITS_PER_BYTE	8
#define __GL_STIPPLE_MSB	0

/* IEEE Single Precision Floating Point */
typedef union __GLcpuFloatRec {
    __GLfloat f;
    unsigned int ul;
    struct {
	unsigned int sign : 1;
	unsigned int exponent : 8;
	unsigned int mantissa : 23;
    } field;
} __GLcpuFloat;

#define __GL_FLOAT_MANTISSA_BITS	23
#define __GL_FLOAT_MANTISSA_SHIFT	0
#define __GL_FLOAT_EXPONENT_BIAS	127
#define __GL_FLOAT_EXPONENT_BITS	8
#define __GL_FLOAT_EXPONENT_SHIFT	23
#define __GL_FLOAT_SIGN_SHIFT		31

#endif /* _M_IX86 */

#endif /* __gl_cpu_h_ */

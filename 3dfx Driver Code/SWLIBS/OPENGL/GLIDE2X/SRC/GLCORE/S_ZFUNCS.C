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


#ifndef __GL_USE_MIPSASMCODE

#include "render.h"


/*
** this is a series of depth testers written in C
*/

/***********************  non-masked writes ***********************/

/*
** NEVER, no mask
*/
/* ARGSUSED */
GLboolean
__glDT_NEVER( __GLzValue z, __GLzValue *zfb )
{
    return GL_FALSE;
}

/*
** LEQUAL, no mask
*/
GLboolean
__glDT_LEQUAL( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    if( (GLuint)z <= (GLuint)zbv ) {
	zfb[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** LESS, no mask
*/
GLboolean
__glDT_LESS( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    if( (GLuint)z < (GLuint)zbv ) {
	zfb[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** EQUAL, no mask
*/
GLboolean
__glDT_EQUAL( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    if( z == zbv ) {
	zfb[0] = z;	/* why is this there?  Who uses GL_EQUAL anyway? */
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** GREATER, no mask
*/
GLboolean
__glDT_GREATER( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    if( (GLuint)z > (GLuint)zbv ) {
	zfb[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** NOTEQUAL, no mask
*/
GLboolean
__glDT_NOTEQUAL( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    if( z != zbv ) {
	zfb[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** GEQUAL, no mask
*/
GLboolean
__glDT_GEQUAL( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    if( (GLuint)z >= (GLuint)zbv ) {
	zfb[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** ALWAYS, no mask
*/
/* ARGSUSED */
GLboolean
__glDT_ALWAYS( __GLzValue z, __GLzValue *zfb )
{
    zfb[0] = z;
    return GL_TRUE;
}

/***********************  masked writes ***********************/

/*
** LEQUAL, mask
*/
/* ARGSUSED */
GLboolean
__glDT_LEQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    return ((GLuint)z <= (GLuint)zbv);
}

/*
** LESS, mask
*/
/* ARGSUSED */
GLboolean
__glDT_LESS_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    return ((GLuint)z < (GLuint)zbv);
}

/*
** EQUAL, mask
*/
/* ARGSUSED */
GLboolean
__glDT_EQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    return (z == zbv);
}

/*
** GREATER, mask
*/
/* ARGSUSED */
GLboolean
__glDT_GREATER_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    return ((GLuint)z > (GLuint)zbv);
}

/*
** NOTEQUAL, mask
*/
/* ARGSUSED */
GLboolean
__glDT_NOTEQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    return (z != zbv);
}

/*
** GEQUAL, mask
*/
/* ARGSUSED */
GLboolean
__glDT_GEQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue zbv = *zfb;

    return ((GLuint)z >= (GLuint)zbv);
}

/*
** ALWAYS, mask
*/
/* ARGSUSED */
GLboolean
__glDT_ALWAYS_M( __GLzValue z, __GLzValue *zfb )
{
    return GL_TRUE;
}

/***********************  non-masked writes ***********************/

/*
** NEVER, no mask
*/
/* ARGSUSED */
GLboolean
__glDT_NEVER16( __GLzValue z, __GLzValue *zfb )
{
    return GL_FALSE;
}

/*
** LEQUAL, no mask
*/
GLboolean
__glDT_LEQUAL16( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    if( (GLuint)z <= (GLuint)zbv ) {
	zfb16[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** LESS, no mask
*/
GLboolean
__glDT_LESS16( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    if( (GLuint)z < (GLuint)zbv ) {
	zfb16[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** EQUAL, no mask
*/
GLboolean
__glDT_EQUAL16( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    if( z == zbv ) {
	zfb16[0] = z;	/* why is this there?  Who uses GL_EQUAL anyway? */
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** GREATER, no mask
*/
GLboolean
__glDT_GREATER16( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    if( (GLuint)z > (GLuint)zbv ) {
	zfb16[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** NOTEQUAL, no mask
*/
GLboolean
__glDT_NOTEQUAL16( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    if( z != zbv ) {
	zfb16[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** GEQUAL, no mask
*/
GLboolean
__glDT_GEQUAL16( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    if( (GLuint)z >= (GLuint)zbv ) {
	zfb16[0] = z;
	return GL_TRUE;
    } else {
	return GL_FALSE;
    }
}

/*
** ALWAYS, no mask
*/
/* ARGSUSED */
GLboolean
__glDT_ALWAYS16( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    zfb16[0] = z;
    return GL_TRUE;
}

/***********************  masked writes ***********************/

/*
** LEQUAL, mask
*/
/* ARGSUSED */
GLboolean
__glDT_LEQUAL16_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    return ((GLuint)z <= (GLuint)zbv);
}

/*
** LESS, mask
*/
/* ARGSUSED */
GLboolean
__glDT_LESS16_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    return ((GLuint)z < (GLuint)zbv);
}

/*
** EQUAL, mask
*/
/* ARGSUSED */
GLboolean
__glDT_EQUAL16_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    return (z == zbv);
}

/*
** GREATER, mask
*/
/* ARGSUSED */
GLboolean
__glDT_GREATER16_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    return ((GLuint)z > (GLuint)zbv);
}

/*
** NOTEQUAL, mask
*/
/* ARGSUSED */
GLboolean
__glDT_NOTEQUAL16_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    return (z != zbv);
}

/*
** GEQUAL, mask
*/
/* ARGSUSED */
GLboolean
__glDT_GEQUAL16_M( __GLzValue z, __GLzValue *zfb )
{
    __GLzValue16 *zfb16 = (__GLzValue16 *) zfb;
    __GLzValue16 zbv = *zfb16;

    return ((GLuint)z >= (GLuint)zbv);
}

/*
** ALWAYS, mask
*/
/* ARGSUSED */
GLboolean
__glDT_ALWAYS16_M( __GLzValue z, __GLzValue *zfb )
{
    return GL_TRUE;
}

#endif /* __GL_USE_MIPSASMCODE */

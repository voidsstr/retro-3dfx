/*
** Copyright 1996, 1997, Silicon Graphics, Inc.
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
#if   (PIXEL_TYPE == CINDEX)

#define FR_PIXEL GLubyte

#elif (PIXEL_TYPE == RGB332)

#define FR_PIXEL GLubyte
#define RSHIFT	5
#define GSHIFT	2
#define BSHIFT	0
#define ASHIFT	0
#define RSIZE	3
#define GSIZE	3
#define BSIZE	2
#define ASIZE	0

#elif (PIXEL_TYPE == RGB666)

#define FR_PIXEL FR_PIXEL8

#elif (PIXEL_TYPE == RGB5)

#define FR_PIXEL GLushort
#define RSHIFT	10
#define GSHIFT	5
#define BSHIFT	0
#define ASHIFT	0
#define RSIZE	5
#define GSIZE	5
#define BSIZE	5
#define ASIZE	0

#elif (PIXEL_TYPE == RGB565)

#define FR_PIXEL GLushort
#define RSHIFT	11
#define GSHIFT	5
#define BSHIFT	0
#define ASHIFT	0
#define RSIZE	5
#define GSIZE	6
#define BSIZE	5
#define ASIZE	0

#elif (PIXEL_TYPE == A1RGB5)

#define FR_PIXEL GLushort
#define ASHIFT	15
#define RSHIFT	10
#define GSHIFT	5
#define BSHIFT	0
#define RSIZE	5
#define GSIZE	5
#define BSIZE	5
#define ASIZE	1

#elif (PIXEL_TYPE == RGB8)

#define FR_PIXEL GLuint

#elif (PIXEL_TYPE == BGR8)

#define FR_PIXEL GLuint

#elif (PIXEL_TYPE == RGBX8)

#define FR_PIXEL GLuint
#define RSHIFT	24
#define GSHIFT	16
#define BSHIFT	8
#define ASHIFT	0
#define RSIZE	8
#define GSIZE	8
#define BSIZE	8
#define ASIZE	0

#elif (PIXEL_TYPE == XRGB8)

#define FR_PIXEL GLuint
#define RSHIFT	16
#define GSHIFT	8
#define BSHIFT	0
#define ASHIFT	0
#define RSIZE	8
#define GSIZE	8
#define BSIZE	8
#define ASIZE	0

#elif (PIXEL_TYPE == XBGR8)

#define FR_PIXEL GLuint
#define BSHIFT	16
#define GSHIFT	8
#define RSHIFT	0
#define ASHIFT	0
#define RSIZE	8
#define GSIZE	8
#define BSIZE	8
#define ASIZE	0

#elif (PIXEL_TYPE == RGBA8)

#define FR_PIXEL GLuint
#define RSHIFT	24
#define GSHIFT	16
#define BSHIFT	8
#define ASHIFT	0
#define RSIZE	8
#define GSIZE	8
#define BSIZE	8
#define ASIZE	0

#elif (PIXEL_TYPE == ARGB8)

#define FR_PIXEL GLuint
#define ASHIFT	24
#define RSHIFT	16
#define GSHIFT	8
#define BSHIFT	0
#define RSIZE	8
#define GSIZE	8
#define BSIZE	8
#define ASIZE	8

#elif (PIXEL_TYPE == ABGR8)

#define FR_PIXEL GLuint
#define ASHIFT	24
#define BSHIFT	16
#define GSHIFT	8
#define RSHIFT	0
#define RSIZE	8
#define GSIZE	8
#define BSIZE	8
#define ASIZE	8

#endif /* PIXEL_TYPE */

/* Maximum allowable values, per component */
#define MAX_R ((1<<RSIZE)-1)
#define MAX_G ((1<<GSIZE)-1)
#define MAX_B ((1<<BSIZE)-1)
#define MAX_A (ASIZE ? ((1<<ASIZE)-1) : 255)

/* Component mask */
#define RMASK (MAX_R << RSHIFT)
#define GMASK (MAX_G << GSHIFT)
#define BMASK (MAX_B << BSHIFT)
#define AMASK (MAX_A << ASHIFT)

/* Fractional bits in an 8-bit component */
#define RFRAC	(8-RSIZE)
#define GFRAC	(8-GSIZE)
#define BFRAC	(8-BSIZE)
#define AFRAC	(ASIZE ? (8-ASIZE) : 0)

/* Shift to convert between GLuint and Fixed-Point Color */
#define RUB2C	(COLOR_FRAC_BITS-RFRAC)
#define GUB2C	(COLOR_FRAC_BITS-GFRAC)
#define BUB2C	(COLOR_FRAC_BITS-BFRAC)
#define AUB2C	(COLOR_FRAC_BITS-AFRAC)

/* Avoid overflow, assume underflow not possible */
#define RCLAMP(x)		((x) > MAX_R ? MAX_R : (x))
#define GCLAMP(x)		((x) > MAX_G ? MAX_G : (x))
#define BCLAMP(x)		((x) > MAX_B ? MAX_B : (x))
#define ACLAMP(x)		((x) > MAX_A ? MAX_A : (x))

#define RCLAMP_UB(x)		((x) > (MAX_R<<RFRAC) ? (MAX_R<<RFRAC) : (x))
#define GCLAMP_UB(x)		((x) > (MAX_G<<GFRAC) ? (MAX_G<<GFRAC) : (x))
#define BCLAMP_UB(x)		((x) > (MAX_B<<BFRAC) ? (MAX_B<<BFRAC) : (x))
#define ACLAMP_UB(x)		((x) > (MAX_A<<AFRAC) ? (MAX_A<<AFRAC) : (x))

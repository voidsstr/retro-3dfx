#if 0 /* Perl cannot handle C-style comments */
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
#endif
#ifndef __FR_FBTYPE_H
#define __FR_FBTYPE_H

#if 0 /* Perl cannot handle C-style comments */
/* Pixel Types
 *
 * CINDEX	8  bit Ramp  Color index
 * RGB332	8  bit RGB   8x8x4 Color Cube
 * RGB666	8  bit RGB   6x6x6 Color Cube
 * RGB5		16 bit RGB,  5 bits per color component
 * RGB565	16 bit RGB,  5 bits red and blue, 6 bits green
 * A1RGB5	16 bit RGBA, 5 bits per color component, 1 bit alpha
 * RGB8		24 bit RGB,  8 bits per color component
 * BGR8		24 bit RGB,  8 bits per color component
 * RGBX8	32 bit RGB,  8 bits per color component
 * XRGB8	32 bit RGB,  8 bits per color component
 * XBGR8	32 bit RGB,  8 bits per color component
 * RGBA8	32 bit RGBA, 8 bits per color component, 8 bit alpha
 * ARGB8	32 bit RGBA, 8 bits per color component, 8 bit alpha
 * ABGR8	32 bit RGBA, 8 bits per color component, 8 bit alpha
 */
#endif

#define CINDEX	1
#define RGB332	2
#define RGB666	3
#define RGB5	4
#define RGB565	5
#define A1RGB5	6
#define RGB8	7
#define BGR8	8
#define RGBX8	9
#define XRGB8	10
#define XBGR8	11
#define RGBA8	12
#define ARGB8	13
#define ABGR8	14

#endif /* __FR_FBTYPE_H */

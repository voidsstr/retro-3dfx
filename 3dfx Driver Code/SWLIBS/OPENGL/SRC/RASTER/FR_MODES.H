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
#ifndef __FR_MODES_H
#define __FR_MODES_H

#if 0 /* Perl cannot handle C-style comments */
/* Mode bits for PickProcs */
#endif

#if 0
/* Setup mode bits */
#endif

#define __FR_SETUP_DITHER		(1<<0)
#define __FR_SETUP_SMOOTH		(1<<1)
#define __FR_SETUP_TEXTURE		(1<<2)
#define __FR_SETUP_PERSPECTIVE		(1<<3)
#define __FR_SETUP_DEPTH		(1<<4)
#define __FR_SETUP_STENCIL		(1<<5)
#define __FR_SETUP_ALPHA		(1<<6)

#define __FR_SETUP_TABLE_SIZE_CI	(__FR_SETUP_STENCIL<<1)
#define __FR_SETUP_TABLE_SIZE_RGB	(__FR_SETUP_ALPHA<<1)

#if 0
/* GenPixels mode bits
 * Texturing is disabled if TEXENV == 0
 * ALPHA and DITHER are mutally exclusive because ALPHA is used only for
 * GenColors and DITHER is used only for GenPixels.  So we share that bit.
 * Care must be taken at pick time to only set the flag when appropriate!!
 * Also, texenv DECAL (RGBA mode) and texenv ADD (CI mode) share mode bits.
 */
#endif

#define __FR_PIXEL_DITHER		(1<<0)
#define __FR_PIXEL_ALPHA		(1<<0)
#define __FR_PIXEL_SMOOTH		(1<<1)
#define __FR_PIXEL_REPLACE		(1<<2)
#define __FR_PIXEL_ADD			(2<<2)
#define __FR_PIXEL_DECAL		(2<<2)
#define __FR_PIXEL_MODULATE		(3<<2)
#define __FR_PIXEL_BLEND		(4<<2)
#define __FR_PIXEL_BLEND_INTENSITY	(5<<2)
#define __FR_PIXEL_REPLACE_ALPHA	(6<<2)
#define __FR_PIXEL_MODULATE_ALPHA	(7<<2)
#define __FR_PIXEL_TEXENV		(7<<2)
#define __FR_PIXEL_TEXTURE_ALPHA	(1<<5)

#define __FR_PIXEL_TABLE_SIZE_CI	(1<<4)
#define __FR_PIXEL_TABLE_SIZE_RGB	(1<<6)

#if 0
/* DepthTest bits -
 * The low three bits are reserved for the depth test
 * DEPTH_STENCIL is set if STENCIL_TEST is enabled, since depth buffer
 * behavior varies slightly.
 */
#endif

#define __FR_DEPTH32		(1<<3)
#define __FR_DEPTH_WRITE	(1<<4)
#define __FR_DEPTH_STENCIL	(1<<5)
#define __FR_ZTEST_TABLE_SIZE	(__FR_DEPTH_STENCIL<<1)

#if 0
/* ExtractTexels bits -
 * The low three bits represent the log2 of texture width minus 3
 * (e.g. 0 represents a texture of size 2^3=8, 7 represents 2^10 = 1024)
 * INDEX means the texture is in INDEX format;
 * The NEED fields determine whether the return format is INDEX, RGB, RGBA;
 * The HAS fields determine if the texture is L, LA, RGB, RGBA format;
 */
#endif

#define __FR_TEXEL_LOG_WIDTH		(7<<0)
#define __FR_TEXEL_SQUARE		(1<<3)	
#define __FR_TEXEL_PERSPECTIVE		(1<<4)
#define __FR_TEXEL_NEED_RGB		(1<<5)
#define __FR_TEXEL_NEED_ALPHA		(1<<6)
#define __FR_TEXEL_HAS_INDEX		(1<<7)
#define __FR_TEXEL_HAS_RGB		(1<<8)
#define __FR_TEXEL_HAS_ALPHA		(1<<9)

#define __FR_TEXEL_WIDTH_BIAS		3
#define __FR_TEXEL_TABLE_SIZE		(__FR_TEXEL_HAS_ALPHA<<1)

#define __FR_TEXEL_PWL_SPAN_WIDTH	16

#if 0
/* The first 4 bits are the COMBINE_OP.  In CI modes, its the logicop;
 * in RGB mode it depends on the value of COMBINE_OP_BLEND. If set the
 * COMBINE_OP is the blend function; otherwise its the logicop:
 * (logicop-GL_CLEAR)
 */
#endif

#define __FR_LO_CLEAR		0
#define __FR_LO_AND		1
#define __FR_LO_AND_REVERSE	2
#define __FR_LO_COPY		3
#define __FR_LO_AND_INVERTED	4
#define __FR_LO_NOOP		5
#define __FR_LO_XOR		6
#define __FR_LO_OR		7
#define __FR_LO_NOR		8
#define __FR_LO_EQUIV		9
#define __FR_LO_INVERT		10
#define __FR_LO_OR_REVERSE	11
#define __FR_LO_COPY_INVERTED	12
#define __FR_LO_OR_INVERTED	13
#define __FR_LO_NAND		14
#define __FR_LO_SET		15
#define __FR_COMBINE_OPERATION	15

#define __FR_BLEND_SRC_SA	(0 << 2)
#define __FR_BLEND_SRC_MSA	(1 << 2)
#define __FR_BLEND_SRC_SAS	(2 << 2)
#define __FR_BLEND_SRC_FUNC	(3 << 2)

#define __FR_BLEND_DST_ZERO	0
#define __FR_BLEND_DST_ONE	1
#define __FR_BLEND_DST_SA	2
#define __FR_BLEND_DST_MSA	3
#define __FR_BLEND_DST_FUNC	3

#define __FR_BLEND_SA_ZERO	(__FR_BLEND_SRC_SA  | __FR_BLEND_DST_ZERO)
#define __FR_BLEND_SA_ONE	(__FR_BLEND_SRC_SA  | __FR_BLEND_DST_ONE)
#define __FR_BLEND_SA_MSA	(__FR_BLEND_SRC_SA  | __FR_BLEND_DST_MSA)
#define __FR_BLEND_MSA_SA	(__FR_BLEND_SRC_MSA | __FR_BLEND_DST_SA)
#define __FR_BLEND_SAS_ONE	(__FR_BLEND_SRC_SAS | __FR_BLEND_DST_ONE)

#define __FR_COMBINE_DITHER	(1<<4)
#define __FR_COMBINE_BLEND	(1<<5)
#define __FR_COMBINE_ALPHA	(1<<6)

#define __FR_COMBINE_TABLE_SIZE_CI  (__FR_COMBINE_DITHER << 1)
#define __FR_COMBINE_TABLE_SIZE_RGB (__FR_COMBINE_ALPHA  << 1)

#define HIBIT		(1<<31)

#endif /* __FR_MODES_H */

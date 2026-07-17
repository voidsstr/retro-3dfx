#include "vxd.h"
/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
** $Revision: 2$
** $Date: 10/11/00 8:09:27 PM$
*/

#include <h3.h>
#include "h3sim.h"

//----------------------------------------------------------------------
// this file has routines to do YUV to RGB color space conversion
// using fixed point (INTEGER_BITS . FRACTION_BITS) math
//----------------------------------------------------------------------

#define INTEGER_BITS 12
#define FRACTION_BITS (32 - INTEGER_BITS)

// note, these constants are in 12.20 format.  If INTEGER_BITS is
// changed, these need to be recomputed
#define FIXED_0p5	0x00080055
#define FIXED_1p402	0x00166e97
#define FIXED_m0p34414	0xfffa7e68
#define FIXED_m0p71414	0xfff492e2
#define FIXED_0p34414	0x00058198
#define FIXED_0p71414	0x000b6d1e
#define FIXED_1p772	0x001c5a1c

#define INT_TO_FIXED(x) ((x) << FRACTION_BITS)

// the 0x00000100 bias is a hack to get 12.20 to work
// (it needs to round some values up by just a bit)
// if INTEGER_BITS is not 12, this may not work
#if 0
#define FIXED_TO_INT(x) (((x + 0x00000100) >> FRACTION_BITS) + ((x < 0) ? 1 : 0))
#else
long
FIXED_TO_INT(long fixed)
{
    long result = fixed;

    if (fixed >= 0)
    {
	result += 0x00000100;
	result >>= FRACTION_BITS;
    }
    else 
    {
	result >>= FRACTION_BITS;
	if (fixed & ((1 << FRACTION_BITS) - 1))
	    result += 1;
    }

    return result;
}
#endif
    

// multiply two fixed point #s and returns the fixed point results
// uses 64-bit integers specific to MSVC
// (or long longs with GCC)
//
static int
multiply(int x, int y)
{
    INT64 llx, lly, product;
    long _int, fraction;
    
    llx = x;
    lly = y;
    product = llx * lly;
    fraction = (long)(product >> FRACTION_BITS) & ((1 << FRACTION_BITS) - 1);
    _int = (long)(product >> (2*FRACTION_BITS)) & ((1 << INTEGER_BITS) - 1);
    _int <<= FRACTION_BITS;

    return fraction | _int;
}

#undef FLOATYUVTORGB
#if defined(__unix__) && defined(FLOATYUVTORGB)
FxU32
yuvTOrgbFixed(FxU32 yuv)
{
  int y, u, v, r, g, b;
  float rsub, gsub, bsub;

  y = (yuv & 0xFF0000) >> 16;
  u = (yuv & 0xFF00) >> 8;
  v = (yuv & 0xFF);

  rsub = 1.402 * (v - 128);
  if (rsub < 0.0) rsub -= 0.5; else rsub += 0.5;
  r = y + (int)(rsub);

  gsub = 0.34414 * (u - 128);
  if (gsub < 0.0) gsub -= 0.5; else gsub += 0.5;
  g = y - (int)(gsub);

  gsub = 0.71414 * (v - 128);
  if (gsub < 0.0) gsub -= 0.5; else gsub += 0.5;
  g -= (int)(gsub);

  bsub = 1.772 * (u - 128);
  if (bsub < 0.0) bsub -= 0.5; else bsub += 0.5;
  b = y + (int)(bsub);

  if (b < 0) b = 0;
  if (b > 255) b = 255;

  if (r < 0) r = 0;
  if (r > 255) r = 255;

  if (g < 0) g = 0;
  if (g > 255) g = 255;
  return ((r) << 16) | ((g) << 8) | (b);
}
#else

FxU32
yuvTOrgbFixed(FxU32 yuv)
{
    int y, u, v, r, g, b;
    int rf, gf, bf;
  
    y = (yuv & 0xFF0000) >> 16;
    u = (yuv & 0x00FF00) >> 8;
    v = (yuv & 0x0000FF) ;

    GDBG_INFO(170, "YUV conversion: Y=0x%x, U=0x%x, V=0x%x --> ", y, u, v);

    rf = multiply(FIXED_1p402, INT_TO_FIXED(v - 128));
    if (rf < 0) rf -= FIXED_0p5; else rf += FIXED_0p5;
    r = y + FIXED_TO_INT(rf);

    gf =   multiply(FIXED_0p34414, INT_TO_FIXED(u - 128));
    if (gf < 0)
	gf -= FIXED_0p5;
    else
	gf += FIXED_0p5; // round

    g =  y - FIXED_TO_INT(gf);

    gf =  multiply(FIXED_0p71414, INT_TO_FIXED(v - 128));
    if (gf < 0)
	gf -= FIXED_0p5;
    else
	gf += FIXED_0p5; // round

    g -= FIXED_TO_INT(gf);
    if (g < 0)
	g = 0;
    if (g > 255)
	g = 255;
				 
    bf = multiply(FIXED_1p772, INT_TO_FIXED(u - 128));
    if (bf < 0) bf -= FIXED_0p5; else bf += FIXED_0p5;

    b = y + FIXED_TO_INT(bf);
    
    if (b < 0)
	b = 0;
    if (b > 255)
	b = 255;

    if (r < 0)
	r = 0;
    if (r > 255)
	r = 255;
    if (g < 0)
	g = 0;
    if (g > 255)
	g = 255;

    GDBG_INFO_MORE(170, "R=0x%x, G=0x%x, B=0x%x\n", r, g, b);

    return  (r << 16) | (g << 8) | b;
}

#endif


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
** $Date: 10/11/00 8:09:52 PM$
*/

#include "udiag.h"
#include "sstdiag.h"
#include "../csim/h3sim.h"




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


// keep this around -- it is very useful for checking the correctness/
// precision of the fixed point routines
long float_to_fixed(float flt)
{
    unsigned long fixed_int, fixed_frac = 0, float_int;
    float float_frac;
    int frac_bit;
    float float_frac_bit;
    long retval;
    int negative = 0;

    if (flt < 0.0)
    {
	negative = 1;
	flt = -flt;
    }
    
//    printf("input float = %g\n", flt);

    float_int = (unsigned long)flt;
    float_frac = flt - (float)float_int;
//    printf("float int portion = %d, fraction = %g\n", float_int, float_frac);

    fixed_int = float_int << INTEGER_BITS;
    
    for (frac_bit = 0; frac_bit < FRACTION_BITS; frac_bit++)
    {
	float_frac_bit = (float)1.0 / (float)(1 << (frac_bit + 1));
	if (float_frac >= float_frac_bit)
	{
//	    printf("float frac = %g, applying fixed frac bit =%g\n",
//		   float_frac, float_frac_bit);
	    fixed_frac |= 1 << (FRACTION_BITS - frac_bit - 1);
	    float_frac -= float_frac_bit;
//	    printf("remaining float frac = %g\n", float_frac);
	}
	if (float_frac == 0.0)
	    break;
    }
//    printf("left over floating point fraction after conversion is %g\n",
//	   float_frac);

    retval = (float_int << FRACTION_BITS) | fixed_frac;
    
    if (negative)
	retval = -retval;

//    printf("float_int = 0x%08x, fixed_frac = 0x%08x, returnval is 0x%08x\n",
//	   float_int, fixed_frac, retval);
    
    return retval;
}

float fixed_to_float(long fixed)
{
    unsigned long fixed_int, fixed_frac = 0, float_int;
    int frac_bit;
    float float_frac = 0.0F;
    float retval;
    int negative = 0;

    if (fixed < 0)
    {
	negative = 1;
	fixed = -fixed;
    }
    
//    printf("input fixed = 0x%08x\n", fixed);

    fixed_int = fixed >> FRACTION_BITS;
    float_int = (float)fixed_int;
    
    fixed_frac = fixed & ((1 << FRACTION_BITS) - 1);
    
//    printf("fixed int portion = %d, fixed fraction = 0x%08x\n", fixed_int,
//	   fixed_frac);
//    printf("float int = %g\n", float_int);

    for (frac_bit = 0; frac_bit < FRACTION_BITS; frac_bit++)
    {
	if (!(fixed_frac & (1 << (FRACTION_BITS - frac_bit - 1))))
	    continue;
	
	float_frac += (float)1.0 / (float)(1 << (frac_bit + 1));
    }

//    printf("float_frac = %g\n", float_frac);

    retval = float_int + float_frac;
    
    if (negative)
	retval = -retval;

    return retval;
}


FxU32
yuvTOrgbDouble(FxU32 yuv)
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


FxU32
yuvTOrgbFloat(FxU32 yuv)
{
  int y, u, v, r, g, b;
  float rsub, gsub, bsub;

  y = (yuv & 0xFF0000) >> 16;
  u = (yuv & 0xFF00) >> 8;
  v = (yuv & 0xFF);

  rsub = 1.402F * (v - 128);
  if (rsub < 0.0) rsub -= 0.5F; else rsub += 0.5F;
  r = y + (int)(rsub);

  gsub = 0.34414F * (u - 128);
  if (gsub < 0.0) gsub -= 0.5F; else gsub += 0.5F;
  gdbg_info(3, "float:gf=%.8g\n", gsub);
  g = y - (int)(gsub);

  gsub = 0.71414F * (v - 128);
  if (gsub < 0.0) gsub -= 0.5F; else gsub += 0.5F;
  g -= (int)(gsub);

  bsub = 1.772F * (u - 128);
  gdbg_info(4, "bf1 float = %.8g\n", bsub);
  if (bsub < 0.0) bsub -= 0.5F; else bsub += 0.5F;
  gdbg_info(4, "bf2 float = %.8g\n", bsub);
  gdbg_info(4, "bf2 float cast to int = %d\n", (int)bsub);
  b = y + (int)(bsub);

  if (b < 0) b = 0;
  if (b > 255) b = 255;

  if (r < 0) r = 0;
  if (r > 255) r = 255;

  if (g < 0) g = 0;
  if (g > 255) g = 255;
  return ((r) << 16) | ((g) << 8) | (b);
}



FxU32
yuvTOrgbFixed(FxU32 yuv)
{
    int y, u, v, r, g, b;
    int rf, gf, bf;
  
    y = (yuv & 0xFF0000) >> 16;
    u = (yuv & 0x00FF00) >> 8;
    v = (yuv & 0x0000FF) ;

    rf = multiply(FIXED_1p402, INT_TO_FIXED(v - 128));
    if (rf < 0) rf -= FIXED_0p5; else rf += FIXED_0p5;
//    rf += INT_TO_FIXED(y);
//    r =  FIXED_TO_INT(rf);
    r = y + FIXED_TO_INT(rf);

    gf =   multiply(FIXED_0p34414, INT_TO_FIXED(u - 128));
    if (gf < 0)
	gf -= FIXED_0p5;
    else
	gf += FIXED_0p5; // round

    gdbg_info(3, "fixed:gf=%d (0x%08x=%g)\n", FIXED_TO_INT(gf), gf,
	      fixed_to_float(gf));

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
    gdbg_info(4, "bf1 fixed = 0x%08x (%g)\n", bf, fixed_to_float(bf));
    if (bf < 0) bf -= FIXED_0p5; else bf += FIXED_0p5;
    gdbg_info(4, "bf2 fixed = 0x%08x (%g)\n", bf, fixed_to_float(bf));
    gdbg_info(4, "bf2 fixed cast to int = %d\n", FIXED_TO_INT(bf));
//    bf += INT_TO_FIXED(y);
//    b = FIXED_TO_INT(bf);
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

    return  (r << 16) | (g << 8) | b;
}


void
main(int argc, char **argv)
{
    FxU32 yuv;
    FxU32 rgb1, rgb2;

    printf("(fixed)-0.5 = 0x%08x\n", float_to_fixed(-0.5F));
    
    printf("(int)-1.5 = %d, (int)-1.9 = %d, (int)-1.1 = %d\n",
	   (int)-1.5F, (int)-1.9F, (int)-1.1F);
    
    gdbg_info(8, "%g = 0x%08x\n", 0.34414F, float_to_fixed(0.34414F));
    gdbg_info(8, "%g = 0x%08x\n", 0.71414F, float_to_fixed(0.71414F));
    
    for (yuv = 0; yuv <= 0xFFFFFF; yuv++)
    {
	rgb1 = yuvTOrgbFixed(yuv);
	rgb2 = yuvTOrgbFloat(yuv);

	if (rgb1 != rgb2)
	{
	    printf("mismatch: yuv=0x%08x, rgbFixed=0x%08x, rgbFloat=0x%08x\n",
		   yuv, rgb1, rgb2);
	}
    }
}


#if 0
int main(int argc, char **argv)
{
    unsigned long yuv, rgb;

#if 0
    FIXED_0p5 = float_to_fixed(0.5F);
    FIXED_1p402 = float_to_fixed(1.402F);
    FIXED_m0p34414 = float_to_fixed(-0.34414F);
    FIXED_m0p71414 = float_to_fixed(-0.71414F);
    FIXED_1p772 = float_to_fixed(1.772F);
#endif

//doit(1.402F);
//doit(0.5F);
//doit(-0.34414F);
//  doit(-0.71414F);
//  doit(1.772F);
#if 0
    yuv = 0x00123456;
    rgb = yuvTOrgbFloat(yuv);
    printf("yuv = 0x%08x, yuvTOrgbFloat = 0x%08x\n", yuv, rgb);
    
    rgb = yuvTOrgbFixed(yuv);
    printf("yuv = 0x%08x, yuvTOrgbFixed = 0x%08x\n", yuv, rgb);
#endif


    for (yuv = 0; yuv <= 0x00FFFFFF; yuv++)
    {
	if (yuvTOrgbFloat(yuv) != yuvTOrgbFixed(yuv))
	{
	    printf("yuv = 0x%08x doesn't match\n", yuv);
	    break;
	}
    }

    rgb = yuvTOrgbFloat(yuv);
    printf("yuv = 0x%08x, yuvTOrgbFloat = 0x%08x\n", yuv, rgb);
    
    rgb = yuvTOrgbFixed(yuv);
    printf("yuv = 0x%08x, yuvTOrgbFixed = 0x%08x\n", yuv, rgb);

    return 0;
}
#endif /* #if 0 */
#if 0
    printf("float = %g, fixed = 0x%08x\n", (float)1.402,
	   float_to_fixed((float)1.402));
    printf("float = %g, fixed = 0x%08x\n", (float)0.5,
	   float_to_fixed((float)0.5));
    printf("float = %g, fixed = 0x%08x\n", (float)-0.34414,
	   float_to_fixed((float)-0.34414));
    printf("float = %g, fixed = 0x%08x\n", (float)-0.71414,
	   float_to_fixed((float)-0.71414));
    printf("float = %g, fixed = 0x%08x\n", (float)1.772,
	   float_to_fixed((float)1.772));
#endif
#if 0
_12p20_
float = 1.402, fixed = 0x00166e97, 2nd float = 1.402
float = 0.5, fixed = 0x00080000, 2nd float = 0.5
float = -0.34414, fixed = 0xfffa7e68, 2nd float = -0.344139
float = -0.71414, fixed = 0xfff492e2, 2nd float = -0.71414
float = 1.772, fixed = 0x001c5a1c, 2nd float = 1.772

_16p16_
float = 1.402, fixed = 0x000166e9, 2nd float = 1.40199
float = 0.5, fixed = 0x00008000, 2nd float = 0.5
float = -0.34414, fixed = 0xffffa7e7, 2nd float = -0.344131
float = -0.71414, fixed = 0xffff492f, 2nd float = -0.714127
float = 1.772, fixed = 0x0001c5a1, 2nd float = 1.77199
#endif
#if 0
// keep a bunch of stuff around from the test program
#ifdef DO_DPF
#define DPF dpf
#else
#define DPF 1 ? 0 : (void)
#endif

#include <math.h>
#include <stdio.h>
#include <stdarg.h>

dpf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    return (1);
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


void doit(float myfloat)    
{
    printf("float = %g, fixed = 0x%08x, 2nd float = %g\n",
	   myfloat,
	   float_to_fixed(myfloat),
	   fixed_to_float(float_to_fixed(myfloat)));
}

    

unsigned long
yuvTOrgbFloat(unsigned long yuv)
{
    int y, u, v, r, g, b;
    float rf, gf, bf;
  
    y = (yuv & 0xFF0000) >> 16;
    u = (yuv & 0x00FF00) >> 8;
    v = (yuv & 0x0000FF) ;

    rf = 0.5+ ((float)(y) + 1.40200*((float)v - 128.0));

DPF("v-128.0 = %g\n", (float)v-128.0);
DPF("1.40200*((float)v - 128.0) = %g\n",  1.40200*((float)v - 128.0));
DPF("rf (float) = %g\n", rf);
    
    r = (int) rf;
    if(r<0)
	r=0;
    if(r>255)
	r=255;
DPF("r = %d\n", r);

    gf =   -0.34414*((float)u - 128.0);
    if (gf<0.0)
	gf -= 0.5;
    else
	gf+= 0.5; // round
DPF("gf1 = %g\n", gf);
    

    g =  y + (int) gf;
DPF("g1 = %d\n", g);
    
    gf = ( -0.71414*((float)v - 128.0));
    if (gf<0.0)
	gf -= 0.5;
    else
	gf+= 0.5; // round
DPF("gf2 = %g\n", gf);

    g += (int) gf;
    if(g<0)
	g=0;
    if(g>255)
	g=255;
				 
    bf = 0.5+ ((float)(y) + 1.77200*((float)u - 128.0));
DPF("bf = %g\n", bf);

    b = (int) bf;
    if(b<0)
	b=0;
    if(b>255)
	b=255;

    return  (r << 16) | (g << 8) | b;
}

#endif /* #if 0 */


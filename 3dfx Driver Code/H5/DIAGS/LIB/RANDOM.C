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
** $Date: 10/11/00 8:11:57 PM$
*/

#include "udiag.h"

static unsigned long randx = 1;

// set the random number seed
void setSeed(unsigned long seed)
{
    randx = seed;
}

// get the random number seed
unsigned long getSeed(void)
{
    return randx;
}

// generate and return a random number r, where 0 <= r <= maxr
unsigned int iRandom (unsigned int maxr)
{
    unsigned int n,retval;

    if (maxr > 0xFFFFFFF) {
	do {
	    retval = iRandom(0xFFFF);
	    retval |= iRandom(maxr>>16)<<16;
	} while (retval > maxr);
	return retval;
    }
    for (n=1; n<32; n++)
	if (((unsigned)1 << n) > maxr) break;
    do {
	randx = randx*1103515245 + 12345;
	retval = (randx & 0x7fffffff) >> (31-n);
    } while (retval > maxr);
    return retval;
}

// GMT: this routine does not use the 64-bit macros
FxU64 iRandom64(FxU64 maxr)
{
    unsigned int hi;
    FxU64 retval;

    do {
	hi = (unsigned int )(maxr >> 32);	// get high 32 bits
	if (hi) {		// if bigger than 32 bits
	    retval = iRandom(0xFFFFFFFF);	// random lo 32
	    retval |= ((FxU64)iRandom(hi))<<32;	// random hi 32
	}
	else retval = iRandom((unsigned int)maxr);
    } while (retval > maxr);
    return retval;
}

// floating point random between 0 and maxint+1-lsb
float fRandom(int maxint)
{
    int i;
    i = iRandom(maxint);
    return i + iRandom(0xFFFFFFF)/(float)(0x10000000);
}

// floating point random in range [2**loExp, 2**hiExp)
// this generates even probabilities across exponents
float fexpRandom(int loExp, int hiExp)
{
    int e;
    float fe;
    e = rRandom(loExp,hiExp-1);
    fe = (e >= 0) ? (1<<e) : 1.0F/(1<<-e);
    return fe * (fRandom(0) + 1.0F);
}

// NOTE: we subtract one from e-s because the random fractions add 1 to 
//	 the range, this means we never really generate 'e' as a result
float rfRandom(int s, int e)
{
    return s + fRandom(e-s-1);
}

#define M_PI 3.14159265358979323846
#define DEG_TO_RAD(a) ((a) * M_PI / 180.0)

// return a random angle between 0 and 360 degrees in radians
double aRandom(void)
{
    return DEG_TO_RAD(iRandom(360000) * .001);
}

// generate up to 'n' random bytes where the bytes are all 0 or 1
// used for generating random writemasks
unsigned long byteRandom(int n)
{
    int retval = 0;

    while (n-- > 0) {
	retval <<= 8;
	if (iRandom(1))
	    retval |= 0xFF;
    }
    return retval;
}

// return a random 16-bit color that wont dither
unsigned long colRandom16(void)
{
    return iRandom(0xFFFFFF) & (diago.rgb==16 ? 0xF8FCF8 : 0xF8F8F8);
}

// return a random 24-bit color
unsigned long colRandom24(void)
{
    return iRandom(0xFFFFFF);
}

// return a random 32-bit color
FxU32 colRandom32(void)
{
    return iRandom(0xFFFFFFFF);
}

// return an int within a range
int rRandom(int s, int e)
{
  //    if ( s > e ) {
  //      GDBG_ERROR("rRandom", "min > max (min=0x%x, max=0x%x)\n",s,e);
  //      DIAG_FAIL();
  //    }
    return s + iRandom(e-s);
}

FxI64 rRandom64(FxI64 s, FxI64 e)
{
    return FX_ADD64(s , iRandom64(FX_SUB64(e,s)));
}

// initialize an array of unique ints from [0,n-1] and then scramble
// it randomly, by pulling out members one by one into the final array
void scrambleRandom(int n, int *array)
{
    int i, atemp[1024];

    if (n > sizeof(atemp)/sizeof(atemp[0])) {
	GDBG_ERROR("scrambleRandom", "array size exceeded\n");
	DIAG_FAIL();
    }

    for (i=0; i<n; i++)			// init an ordered sequence
	atemp[i] = i;

    while (n-- > 0) {			// for each element
	*array++ = atemp[i=iRandom(n)];	// pull one out randomly
	atemp[i] = atemp[n];		// replace with last element
    }
}

// generate a random (x,y) location on the screen
void xyRandom(long *x, long *y)
{
    *x = iRandom(diago.xmaxscreen-1);		// pick random x,y
    *y = iRandom(diago.ymaxscreen-1);
}

static void gen1(int in, FxU32 src, FxU32 *min, FxU32 *max, FxU32 highest)
{
    int flag;

    src &= highest;			// reduce to 5 bits

    if (in) {				// We want min <= src <= max
	*min  = rRandom(0,src); 		// min <= src
	*max  = rRandom(src,highest);		// src <= max
    }
    else {				// we want csrc OUTside of [min,max]
	flag = iRandom(1);
	if ((src==0 || flag) && src!=highest) {	// src < min <= max
	    *min  = rRandom(src+1,highest);		// src < min
	    *max  = rRandom(*min,highest);		// min <= max
	}
	else {					// min <= max < src
	    *min  = rRandom(0,src-1);			// min < src
	    *max  = rRandom(*min,src-1);		// max < src
	}
    }
}

// generate ckey and crng such that r,g,b of csrc are inside/outside of the 
// chromarange as specified by a,b,c (a=0/1 means r is outside/inside)
void ckeyRandom565(int a, int b, int c, FxU32 csrc, FxU32 *ckey, FxU32 *crng)
{
    FxU32 rkey, gkey, bkey;
    FxU32 rrange, grange, brange;

    // enrange: 1=>inclusive range testing, 2=>exclusive
    // Generate ckey and crng appropriately
    gen1(a, csrc>>11,&rkey,&rrange,31);		// generate each component
    gen1(b, csrc>>5,&gkey,&grange,63);
    gen1(c, csrc>>0,&bkey,&brange,31);
    *ckey = (rkey << 11) | (gkey << 5) | (bkey << 0);
    *crng = (rrange << 11) | (grange << 5) | (brange << 0);
}

void ckeyRandom888(int a, int b, int c, FxU32 csrc, FxU32 *ckey, FxU32 *crng)
{
    FxU32 rkey, gkey, bkey;
    FxU32 rrange, grange, brange;

    // enrange: 1=>inclusive range testing, 2=>exclusive
    // Generate ckey and crng appropriately
    gen1(a, csrc>>16,&rkey,&rrange,255);		// generate each component
    gen1(b, csrc>>8,&gkey,&grange,255);
    gen1(c, csrc>>0,&bkey,&brange,255);
    *ckey = (rkey << 16) | (gkey << 8) | (bkey << 0);
    *crng = (rrange << 16) | (grange << 8) | (brange << 0);
}

// these externs come from the chip dependent module
extern void test32(volatile FxU32 *reg, FxU32 mask, FxU32 val);

void register32test(volatile void *reg, FxU32 maskRead, FxU32 maskWrite)
{
    int i;

    test32(reg,maskRead,maskWrite & iRandom(maskWrite));	// random value
    test32(reg,maskRead,0);			// zero
    test32(reg,maskRead,maskWrite);		// one
    test32(reg,maskRead,maskWrite & iRandom(maskWrite));	// random value
    test32(reg,maskRead,maskWrite);		// one
    test32(reg,maskRead,0);			// zero
    for (i=0; i<32; i++)			// walking 1
	test32(reg,maskRead,maskWrite &(0x00000001<<i));
    for (i=0; i<32; i++)			// walking 0
	test32(reg,maskRead,maskWrite &(~(0x00000001<<i)));
    for (i=0; i<32; i++)			// walking 1
	test32(reg,maskRead,maskWrite &(0x80000000>>i));
    for (i=0; i<32; i++)			// walking 0
	test32(reg,maskRead,maskWrite &(~(0x80000000>>i)));
}


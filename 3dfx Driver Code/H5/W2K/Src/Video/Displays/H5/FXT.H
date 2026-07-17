/*******************************************************************************
*
*	$RCSfile: fxt.h $
*
*	$Date: 10/11/00 8:45:26 PM$
*
*	$Revision: 2$
*
*	$Description: Support header for FXT1 texture encoding $
*
*	Copyright (C) 1999 3dfx Interactive, Inc. All rights reserved.
*
*******************************************************************************/

/***

	Definitions from fx64.h

***/


/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** $Date: 10/11/00 8:45:26 PM$
*/
#ifndef __FX64_H__
#define __FX64_H__

//#include "3dfx.h"

/*
** first figure out what compiler/os we are using and what is supported
*/

/*
** WATCOMC
*/
#if defined(__WATCOMC__)
   #define FX_BIT64(n) ( FX_SHL64( i64_one, n ) )
/*
** GCC
*/
#elif defined(__GNUC__)
   typedef signed long long FxI64;
   typedef unsigned long long FxU64;
   #define NATIVE_64_SUPPORT 1
   #define FX_MASK64(n) (0xFFFFFFFFFFFFFFFFLL >> (64-(n)))
   #define FX_BIT64(n) (((FxI64)1) << (n))
/*
** Microsoft C
*/
#elif defined(_MSC_VER)
   typedef signed __int64 FxI64;
   typedef unsigned __int64 FxU64;

   #define FX_MASK64(n) (0xFFFFFFFFFFFFFFFF >> (64-(n)))
   #define FX_BIT64(n) (((FxI64)1) << (n))
   #define NATIVE_64_SUPPORT 1
#endif

/*
** The following code is for systems with native 64-bit data types
*/
#ifdef NATIVE_64_SUPPORT

#define FX_ADD64( a, b )    ((a) + (b))
#define FX_AND64( a, b )    ((a) & (b))
#define FX_CMP64( a, b )    error
#define FX_COMP64( a )      (~(a))
#define FX_CREATE64( a, b ) ( (((FxI64) (a)) << 32) | (b) )
#define FX_COPY32( a, b )   a = (b)
#define FX_EQ064( a )       ( (a) == 0 )
#define FX_LO64(a)          ((FxI32)(a))
#define FX_HI64(a)          ((FxI32)FX_SHR64(a,32))
#define FX_MUL32( a, b )    (( FxI64 )(  ( FxI64 ) (a) * ( FxI64 ) (b) ) )
#define FX_MUL64( a, b )    ((a) * (b))
#define FX_NEG64( a )       (-(a))
#define FX_OR64( a, b )     ((a) | (b))
#define FX_SET64( a, b, c ) a = (c) | (((FxI64)(b))<<32)
#define FX_SHL64( a, n )    ((a) << (n))             // n < 64
#define FX_SHR64( a, n )    ((a) >> (n))             // n < 64
#define FX_SUB64( a, b )    ((a) - (b))
#define FX_XOR64( a, b )    ((a) ^ (b))
#define FX_FLOATTO64( a )   ((FxI64)(a))
#define FX_64TOFLOAT( a )   (float)(a)
#define FX_64TODBL( a )     (double)(a)

/*
** The following code is for Watcom C
*/
#else

typedef struct
{
  FxI32 hi;
  FxU32 lo;
} FxI64;

typedef struct
{
  FxU32 hi;
  FxU32 lo;
} FxU64;

extern FxI64 i64_one;
extern FxI64 i64_zero;

#define FX_ADD64( a, b )  ( __FX_ADD64( (a).hi, (a).lo, (b).hi, (b).lo ) )
#define FX_AND64( a, b )  ( __FX_AND64( a, b ) )
#define FX_CMP64( a, b )  ( __FX_CMP64( a, b ) )
#define FX_COMP64( a )    ( __FX_COMP64( a ) )
#define FX_CREATE64( a, b ) ( __FX_CREATE64( a, b ) )
#define FX_COPY32( a, b ) ( a = __FX_32TO64( (b) ) )
#define FX_EQ064( a )      ( ( (a).lo == 0 ) && ( (a).hi == 0 ) )
#define FX_HI64( a )      ( (a).hi )
#define FX_LO64( a )      ( (a).lo )
#define FX_MASK64( n )    ( __FX_MASK64( n ) )
#define FX_MUL32( a, b )  ( __FX_MUL32( (a), (b) ) )
#define FX_MUL64( a, b )  ( __FX_MUL64( (a), (b) ) )
#define FX_NEG64( a )     ( __FX_NEG64( (a).hi, (a).lo ) )
#define FX_OR64( a, b )   ( __FX_OR64( a, b ) )
#define FX_SET64( a, b, c ) {(a).hi = b; (a).lo = c;}
#define FX_SHL64( a, n )  ( __FX_SHL64( (a).hi, (a).lo, n ) )
#define FX_SHR64( a, n )  ( __FX_SHR64( (a).hi, (a).lo, n ) )
#define FX_SUB64( a, b )  ( __FX_SUB64( (a).hi, (a).lo, (b).hi, (b).lo ) )
#define FX_FLOATTO64( a ) ( __FX_FLOATTO64((a)) )
#define FX_64TOFLOAT( a )   ( __FX_64TOFLOAT((a)) )

/*
** The following functions are implemented as inline assembly
*/
FxI64 __FX_ADD64( FxI32 a_hi, FxU32 a_lo, FxI32 b_hi, FxU32 b_lo );
FxI64 __FX_32TO64( FxI32 value );
FxI64 __FX_MASK64( int n );
FxI64 __FX_MUL32( FxI32 a, FxI32 b );
FxI64 __FX_NEG64( FxI32 hi, FxU32 lo );
FxI64 __FX_SHL64( FxI32 hi, FxU32 lo, int n );
FxI64 __FX_SHR64( FxI32 hi, FxU32 lo, int n );
FxI64 __FX_SUB64( FxI32 a_hi, FxU32 a_lo, FxI32 b_hi, FxU32 b_lo );

/*
** The following functions are implemented as C functions
*/
FxI64 __FX_AND64( FxI64 a, FxI64 b );
int   __FX_CMP64( FxI64 a, FxI64 b );
FxI64 __FX_COMP64( FxI64 a );
FxI64 __FX_CREATE64( FxI32 a, FxU32 b );
FxI64 __FX_FLOATTO64( float a );
FxI64 __FX_MUL64( FxI64 a, FxI64 b );
FxI64 __FX_OR64( FxI64 a, FxI64 b );
float __FX_64TOFLOAT( FxI64 );

#pragma aux __FX_32TO64 = \
   "cdq" \
   "mov [esi], edx" \
   "mov [esi+4], eax" \
   parm [eax] \
   modify [edx] \
   value struct struct;

#pragma aux __FX_ADD64 = \
   "add  eax, ecx" \
   "adc  edx, ebx" \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   parm [edx] [eax] [ebx] [ecx] \
   value struct struct;

#pragma aux __FX_MUL32 = \
   "imul  edx" \
   "mov   [esi], edx" \
   "mov   [esi+4], eax" \
   parm [edx] [eax] \
   value struct struct;

#pragma aux __FX_SUB64 = \
   "sub  eax, ecx" \
   "sbb  edx, ebx" \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   parm [edx] [eax] [ebx] [ecx] \
   value struct struct;

#pragma aux __FX_SHL64 = \
   "cmp  cl, 32" \
   "jl   SHL_LT32" \
   "sub  cl, 32" \
   "mov  edx, eax " \
   "mov  eax, 0 " \
   "shl  edx, cl" \
   "jmp SHL_DONE" \
   "SHL_LT32:" \
   "shld edx, eax, cl" \
   "sal  eax, cl" \
   "SHL_DONE:" \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   parm [edx] [eax] [cx] \
   value struct struct;

#pragma aux __FX_SHR64 = \
   "cmp  cl, 32" \
   "jl   SHR_LT32" \
   "sub  cl, 32 " \
   "mov  eax, edx" \
   "test edx, 080000000h" \
   "jz  SHR_GE32_POSITIVE" \
   "SHR_GE32_NEGATIVE:" \
   "mov  edx, 0FFFFFFFFh" \
   "or   eax, 080000000h" \
   "sar  eax, cl" \
   "jmp  SHR_DONE" \
   "SHR_GE32_POSITIVE:" \
   "mov  edx, 0" \
   "shr  eax, cl" \
   "jmp  SHR_DONE" \
   "SHR_LT32: " \
   "shrd eax, edx, cl" \
   "sar  edx, cl" \
   "SHR_DONE: " \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   parm [edx] [eax] [cx] \
   value struct struct;

#pragma aux __FX_NEG64 = \
   "neg eax" \
   "adc edx, 0" \
   "neg edx" \
   "mov [esi], edx" \
   "mov [esi+4], eax" \
   parm [edx] [eax] \
   value struct struct;

#pragma aux __FX_MASK64 = \
   "mov edx, 64" \
   "sub edx, ecx" \
   "mov ecx, edx" \
   "mov edx, 0ffffffffh" \
   "mov eax, 0ffffffffh" \
   "cmp  cl, 32" \
   "jl   M64_LT32" \
   "sub  cl, 32 " \
   "mov  eax, edx" \
   "mov  edx, 0" \
   "shr  eax, cl" \
   "jmp  M64_DONE" \
   "M64_LT32: " \
   "shrd eax, edx, cl" \
   "shr  edx, cl" \
   "M64_DONE: " \
   "mov  [esi], edx" \
   "mov  [esi+4], eax" \
   modify [edx eax] \
   parm [ecx] \
   value struct struct;

#endif /* __WATCOMC__ */

/*
** The following is shared between NATIVE64 and Watcom
*/

// a | -(a & (1 << n))  branchless, but may not be good implement for PC
// for constant n, does &, -, |
#define FX_SGNEXT64( a, n ) (FX_OR64((a), FX_NEG64(FX_AND64((a), FX_SHL64(FX_CREATE64(0,1), (n))))))

#endif /* __FX64_H__ */



/***

	Definitions from sst2fxt1.h

***/


#define ALF(x)          (((x) >> 24) & 0xff)
#define RED(x)          (((x) >> 16) & 0xff)
#define GRN(x)          (((x) >>  8) & 0xff)
#define BLU(x)          (((x)      ) & 0xff)
#define ARGB(a, r, g, b) (((a) << 24) | ((r) << 16) | ((g) << 8) |((b)))
#ifndef RGB
#define RGB(r, g, b)    (ARGB( (int) 255, (int) r, (int) g, (int) b) )
#endif

#define TCC_HI          0
#define TCC_MIXED       1
#define TCC_CHROMA      2
#define TCC_ALPHA       3

typedef long            FxI32;
typedef unsigned long   FxU32;


/* in eigen.c */
extern void eigenProject(int n, float *idata, float m[3][3], float *odata);
extern void eigenSpace(int n, float *data, float evec[3][3], float eval[3]);
extern void eigenStatistics(
	int     n, 
	const   float input[][3], 

	float   output[][3],        // after transformation to eigenspace
	float   lo[3][3],           // lo and hi colors for each evector
	float   hi[3][3],

	float   avg[3],             // this is the mean value of this block
	float   min[3],             // min and max values for each evector
	float   max[3], 
	float   err[3]              // max error incurred when dropping each evector
	);

void
printStatistics(
	int     n, 
	const   float input[][3], 

	float   output[][3],        // after transformation to eigenspace
	float   lo[3][3],           // lo and hi colors for each evector
	float   hi[3][3],

	float   avg[3],             // this is the mean value of this block
	float   min[3],             // min and max values for each evector
	float   max[3], 
	float   err[3],             // max error incurred when dropping each evector
	char    *title 
	);


/* In bitcoder.c */
int bitDecoder( void *bits128, FxU32 c[4], int index[32], FxU32 *alpha );
void bitEncoder( int mode, FxU32 c[4], FxU32 alpha, int index[32], void *bits128 );

/* In codec.c */
void FXT1Encode4bpp(int *data, int width, int height, int* encoded);
void FXT1Decode4bpp(int *encoded, int width, int height, int *data);

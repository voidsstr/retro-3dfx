
#ifndef FMACROS_H_
#define FMACROS_H_ 1

/*
** Copyright 1996,1997 Silicon Graphics, Inc.
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
** $Revision: 4$
** $Date: 10/11/00 7:52:09 PM$
*/

#ifdef __cplusplus
extern "C" {
#endif

  /***************************/
 /* FLOATING FORMAT DEFINES */
/***************************/

#define FLOAT_MANTISSA_BITS 23
#define DOUBLE_MANTISSA_BITS 20
#define DOUBLE_MANTISSA_OFFSET ((float) (1<<20))

#define FLOAT_MANTISSA_MASK ((1<<(FLOAT_MANTISSA_BITS))-1)
#define GLFLOAT_MANTISSA_MASK ((1<<(FLOAT_MANTISSA_BITS-1))-1)
#define GLFLOAT_QUANTIZE_MASK ((-1)<<(FIXED_FRAC_BITS - COORD_QUANT_BITS))


  /******************************/
 /* FLOATING POINT UNIT MACROS */
/******************************/

/* Floating point rounding mode control */

#define FP_ROUND_NEAREST 0x0000
#define FP_ROUND_DOWN    0x0400
#define FP_ROUND_UP      0x0800
#define FP_ROUND_ZERO    0x0c00


#define _SAVE_FP_ROUNDING_MODE() __asm { \
        __asm fstcw _fpcntrlOld_         \
}

#define SETUP_FP_ROUNDING_MODE() unsigned int _fpcntrlOld_, _fpcntrlNew_; _SAVE_FP_ROUNDING_MODE();      

#define SET_FP_ROUNDING_MODE(_fprm_) __asm { \
        __asm mov eax, _fpcntrlOld_          \
        __asm and eax, ~0x0c00               \
        __asm or eax, _fprm_                 \
        __asm mov _fpcntrlNew_, eax          \
        __asm fldcw [_fpcntrlNew_]           \
}

#define RESTORE_FP_ROUNDING_MODE() __asm { \
        __asm fldcw _fpcntrlOld_           \
}


  /*****************************/
 /* Generic arithmetic macros */
/*****************************/

/* Float to int conversion */
/*   (Integer unit stalled during fistp) */
#define FLOAT2INT(_flt_, _int_) __asm { \
        __asm fld _flt_                 \
        __asm fistp _int_               \
}

#define Float2Int(_flt_, _int_) \
    (*((float *)(&(_int_)))=(_flt_)+((float)(1<<(FLOAT_MANTISSA_BITS))), \
     (_int_)&=FLOAT_MANTISSA_MASK)

#define SFloat2Int(_flt_, _int_) \
    (*((float *)(&(_int_)))=(_flt_)+((float)(3<<(FLOAT_MANTISSA_BITS-1))), \
     (_int_)=((_int_)&FLOAT_MANTISSA_MASK)-(1<<(FLOAT_MANTISSA_BITS-1)))

/* Integer absolute value */
#define IABS(_intval_) __asm {    \
        __asm  mov eax, _intval_  \
        __asm  sar eax, 31        \
        __asm  xor _intval_, eax  \
        __asm  sub _intval_, eax  \
}


  /**********************/
 /* FIXED POINT MACROS */
/**********************/

/*
  Formats:

  FixedT 12.11 
    Simplest and fast float to int (or unsigned int) conversion.

  DFixedT 12.20
    Double precison analog to FixedT.

  CoordT 11.16  
    Coordinate values are positive, so this gives a range of [0, 2048)
    Coordinate values are rounded to the nearest 1/16
    before being quantized to 4 fractional bits.  The other 12 bits of
    fraction are guard precision for incrementation errors.
    Note:  the floating representation of coords are likewise quantized!
           Areas and gradients *must* be computed with this quantization 
           having been taken into account, since thin triangles can be 
           reduced to zero area by this operation, or sometimes even 
           change sign.

    Differences between coordinates can be signed.

    Both screen coordinates (X and Y) and texture coordinates (S and T)
    use this format, but conversion from floating point values supplied
    by OpenGL proceeds slightly differently.

    Screen CoordT values are offset by .5 before the quantization.  
    This can be accomplished by manipulation of the transform matrix. 
    The .5 offset serves two purposes.  Screen coordinates in OpenGL
    define a pixel as spanning the range [0.0, 1.0).  We prefer to
    have them in the range [-.5, .5) so that the pixel center lands
    on an integer boundary.  We add .5 to keep the coordinates in
    a positive range, and later subtract 1 from raster array x and y
    offsets.  The latter only happens once per triangle.  The second
    purpose is that the .5 offset forces a lower bound on the
    floating point exponent.  This means that the exponent is confined
    to a 12 value range [126, 137] without the need for a check
    to fold small negative exponents (before bias) into a 0 value.  Also,
    we don't need to deal with negative screen coordinates.

    Texture CoordT values are rounded and quantized
    to the nearest 1/16.  The quantization is necessary to prevent an
    unintended overshoot of S and T values in an unwrapped texture
    situation.  However, TCOORDS are *not* offset by .5, so the
    range of a texel is [0, 1.0).  Since texture coordinates are allowed
    to wrap, only the integer bits coresponding to the power of 2
    of the map size (minus one) are significant.  Interestingly, this
    is also the case for gradient values used for incrementing Texture
    CoordT values.
    However since an S or T texture coordinate of 1.0 is valid in an 
    *unwrapped* setting, we need to prevent incremental S (or T) CoordT
    values from actually reaching this value.  This
    is insured if the S (or T) coordinate has
    a non-negative associated gradient, since the right texel boundary
    is not reachable due to the selection rules for which pixels fall
    within the geometric boundaries.  However, if the gradient is
    negative, it is possible to *start* with a S (or T) coordinate
    of 1.0 lying exactly on a pixel center.  Therefore, if the gradient
    of S is negative, then CoordT values are offset negatively by EPSILON,
    where EPSILON represents one lsb in the format. 
  
  DerivT 16.16
    DerivT values are signed values which are the (round toward
    zero) result of a division.  They are used as delta quantities in
    incremental calculations. In the case where the denominator of the
    division is a difference between X or Y coords, the quantization guarantees
    that the minimum Y magnitude is 1/16, which scales the maximum magnitude
    coord of 11 bits to a 15 integer bits, which fits nicely in the 16.16
    representation.  Assuming a 1 lsb error in the fractional bits, an
    incremental quantity can be iterated across 12 bits worth of raster
    (4096 pixels) before contributing to an error significant to the
    subpixel grid.

    The divisions should be carried out in truncation toward zero mode so
    that quantities are not incremented outside of bounds, i.e. we insure
    that we never overshoot.  Otherwise, pointers and indices derived from
    these quantities could wind up making illegal memory references.

    Also, the results of the divisions must be manipulated as a double
    precisions floating point quantity, before conversion to the DERIV
    format.  This is to provide the required 31 bits (+ sign bit) of precision.

    Gradients (derivatives of other quantities with respect to the spatial
    coordinates) also use this format.

  UShortT 16.16
    UShortT are unsigned short values which have 16 bits of fractional precision.

*/

/*==================================================================================*/

typedef int FixedT;             /* fixed-point, two's complement (12.11) */

#define FIXED_INT_BITS   11
#define FIXED_FRAC_BITS  11
#define FIXED_EPSILON    1
#define FIXED_MASK       ((1<<(FIXED_INT_BITS + FIXED_FRAC_BITS + 1)) - FIXED_EPSILON) 
#define FIXED_ONE        (1<<(FIXED_FRAC_BITS))
#define FIXED_HALF       (1<<((FIXED_FRAC_BITS)-1))
#define FIXED_FRAC_MASK  (FIXED_ONE - FIXED_EPSILON)
#define FIXED_INT_MASK   (~FIXED_FRAC_MASK)
#define FIXED_SCALE      ((float)FIXED_ONE)
#define FloatToFixed(f)  ((FixedT) ((f) * FIXED_SCALE))
#define FixedToInt(f)    ((f) >> FIXED_FRAC_BITS)
#define FixedToUns(f)    (((unsigned int)(f)) >> FIXED_FRAC_BITS)
#define FixedCeil(f)     (((f) + FIXED_ONE - FIXED_EPSILON) & FIXED_INT_MASK)
#define FixedFloor(f)    ((f) & FIXED_INT_MASK)
#define FixedToFloat(f)  ((f) * ((float) (1.0f/FIXED_SCALE)))
#define IntToFixed(in)   ((in)<< FIXED_FRAC_BITS)


/*** Use the new, improved versions after we have timed the cost of the DFixed macros */
#if 1
#define PosFloatToFixed(fval) \
    (_tmp=(fval)+((float)(2<<(FIXED_INT_BITS))),((*(int*)(&_tmp))&FIXED_MASK))
#define SignedFloatToFixed(fval) \
    (_tmp=(fval)+((float)(3<<(FIXED_INT_BITS))),(((*(int*)(&_tmp)) \
     &FIXED_MASK)-(1<<(FIXED_INT_BITS + FIXED_FRAC_BITS))))
#define PosScaledFloatToFixed(fval) \
    (_tmp=(fval)+((float)(2<<(FIXED_INT_BITS + FIXED_FRAC_BITS))),((*(int*)(&_tmp))&FIXED_MASK))
#define SignedScaledFloatToFixed(fval) \
    (_tmp=(fval)+((float)(3<<(FIXED_INT_BITS + FIXED_FRAC_BITS))),(((*(int*)(&_tmp)) \
     &FIXED_MASK)-(1<<(FIXED_INT_BITS + FIXED_FRAC_BITS))))
#else
#define PosFloatToFixed(fval) \
    (_tmp=(fval)+((float)(2<<(FIXED_INT_BITS))),_itmp=*(int*)(&_tmp),(_itmp&FIXED_MASK))
#define SignedFloatToFixed(fval) \
    (_tmp=(fval)+((float)(3<<(FIXED_INT_BITS))),_itmp=*(int*)(&_tmp), \
     ((_itmp&FIXED_MASK)-(1<<(FIXED_INT_BITS + FIXED_FRAC_BITS))))
#define PosScaledFloatToFixed(fval) \
    (_tmp=(fval)+((float)(2<<(FIXED_INT_BITS + FIXED_FRAC_BITS))),_itmp=*(int*)(&_tmp),(_itmp&FIXED_MASK))
#define SignedScaledFloatToFixed(fval) \
    (_tmp=(fval)+((float)(3<<(FIXED_INT_BITS + FIXED_FRAC_BITS))),_itmp=*(int*)(&_tmp), \
     ((_itmp&FIXED_MASK)-(1<<(FIXED_INT_BITS + FIXED_FRAC_BITS))))
#endif




/* Round to Minus infinity */

#define FIXED2INT_MINF(_fixedval_) __asm { \
        __asm mov eax, _fixedval_          \
        __asm sar eax, FIXED_FRAC_BITS     \
        __asm mov _fixedval_, eax          \
}

/* Round to Positive infinity */

#define FIXED2INT_PINF(_fixedval_) __asm { \
        __asm mov eax, _fixedval_          \
        __asm add eax, FIXED_FRAC_MASK     \
        __asm sar eax, FIXED_FRAC_BITS     \
        __asm mov _fixedval_, eax          \
}


/* Round to nearest  */
/* sar abd adc both want to pair in the u pipe.
   However, adc gives opportunity for adding in
   another argument. */
/*
#define FIXED2INT_NEAREST(_fixedval_) __asm { \
        __asm mov eax, _fixedval_          \
        __asm sar eax, FIXED_FRAC_BITS     \
        __asm adc eax, 0                   \
        __asm mov _fixedval_, eax          \
}
*/

/* Round to nearest */

#define FIXED2INT_NEAREST(_fixedval_) __asm { \
        __asm mov eax, _fixedval_          \
        __asm add eax, FIXED_HALF          \
        __asm sar eax, FIXED_FRAC_BITS     \
        __asm mov _fixedval_, eax          \
}


/* Round towards zero */
/* If -ve, add 1-epsilon before truncation */
#define FIXED2INT_ZERO(_fixedval_) __asm { \
        __asm mov eax, _fixedval_          \
        __asm sar eax, 31                  \
        __asm and eax, FIXED_FRAC_MASK     \
        __asm add eax, _fixedval_          \
        __asm sar eax, FIXED_FRAC_BITS     \
        __asm mov _fixedval_, eax          \
}

/*==================================================================================*/

typedef int DFixedT;

#define DFIXED_INT_BITS 11
#define DFIXED_FRAC_BITS 20

#define FloatToDFixed(fval) \
    (_dtmp=(fval)+(DOUBLE_MANTISSA_OFFSET*(float)(3<<(DFIXED_INT_BITS))), \
    (DFixedT)(*((int*)(&_dtmp))))

#define ScaledFloatToDFixed(fval) \
    (_dtmp=(fval)+(DOUBLE_MANTISSA_OFFSET*(float)(3<<(DFIXED_INT_BITS + FIXED_FRAC_BITS))), \
    (DFixedT)(*((int*)(&_dtmp))))

#define DoubleToDFixed(dval) \
    (_dtmp=(dval)+(DOUBLE_MANTISSA_OFFSET*(float)(3<<(DFIXED_INT_BITS))), \
    (DFixedT)(*((int*)(&_dtmp))))

#define ScaledDoubleToDFixed(dval) \
    (_dtmp=(dval)+(DOUBLE_MANTISSA_OFFSET*(float)(3<<(DFIXED_INT_BITS + FIXED_FRAC_BITS))), \
    (DFixedT)(*((int*)(&_dtmp))))

/*==================================================================================*/
typedef int CoordT;

#define COORD_INT_BITS   11
#define COORD_FRAC_BITS  16
#define COORD_BITS       (COORD_INT_BITS + COORD_FRAC_BITS + 1)
#define COORD_QUANT_BITS 4
#define COORD_QUANT_FLOAT_LSB (1.0f/((float)(1<<COORD_QUANT_BITS)))
#define COORD_EPSILON    1
#define COORD_MASK       ((1<<(COORD_INT_BITS + COORD_FRAC_BITS + 1)) - COORD_EPSILON) 
#define COORD_ONE        (1<<(COORD_FRAC_BITS))
#define COORD_HALF       (1<<((COORD_FRAC_BITS)-1))
#define COORD_FRAC_MASK  (COORD_ONE - COORD_EPSILON)
#define COORD_INT_MASK   (~COORD_FRAC_MASK)
#define COORD_SCALE      ((float)COORD_ONE)
/* #define FloatToCoord(f)  ((CoordT) ((f) * COORD_SCALE)) */
#define CoordToInt(f)    ((f) >> COORD_FRAC_BITS)
#define CoordToUns(f)    (((unsigned int)(f)) >> COORD_FRAC_BITS)
#define CoordCeil(f)     (((f) + COORD_ONE - COORD_EPSILON) & COORD_INT_MASK)
#define CoordFloor(f)    ((f) & COORD_INT_MASK)
#define CoordToFloat(f)  ((f) * ((float) (1.0f/COORD_SCALE)))
#define IntToCoord(in)   ((in)<< COORD_FRAC_BITS)
#define CoordToReciprocal(f)  (COORD_SCALE/((float)f))

#define FloatToCoord(fval) \
    (_tmp=(fval)+((float)(2<<(COORD_INT_BITS))), \
     (CoordT)(((*(int*)(&_tmp))&FLOAT_MANTISSA_MASK)<<(COORD_BITS-FLOAT_MANTISSA_BITS)))

      /*** Should be just go ahead and make this signed? */
#define ScaledDoubleToCoord(dval) \
    (_dtmp=(dval)+(DOUBLE_MANTISSA_OFFSET*((float)(3<<(COORD_INT_BITS + 4)))*((float)(1<<COORD_FRAC_BITS))), \
     (CoordT)(*((int*)(&_dtmp))))


/* Offset for shifting Double mantissa into subpixel range */
/* If we are in single precison rounding mode, just use 524288.0f */
/* float const QVal = (524288.0f * 536870912); */ /* 2^19 * 2^29 */
extern float const QVal;
  
/* Offset for shifting Float mantissa into fixed point range */
/* float const FVal = 4096.0f; */
extern float const FVal;

/* Rounding quantizer. */
#define QuantizeFloatToCoord(_ptr_, _f_, _i_) __asm {   \
        __asm mov ebx, _ptr_                            \
        __asm fld [ebx]_ptr_._f_                        \
        __asm fadd QVal                                 \
        __asm fsub QVal                                 \
        __asm fst [ebx]_ptr_._f_                        \
        __asm fadd FVal                                 \
        __asm fstp [ebx]_ptr_._i_                       \
        __asm mov eax, [ebx]_ptr_._i_                   \
        __asm and eax, FLOAT_MANTISSA_MASK              \
        __asm shl eax, (COORD_BITS-FLOAT_MANTISSA_BITS) \
        __asm mov [ebx]_ptr_._i_, eax                   \
}


#define QuantizeGLFloatToCoord(_f_, _i_)                     \
    (((*((int *)(&_f_))) &= GLFLOAT_QUANTIZE_MASK),          \
     (_i_ =  (((*((int *)(&_f_))) & GLFLOAT_MANTISSA_MASK)<< \
	      (COORD_BITS-FLOAT_MANTISSA_BITS))))         

/* Rounding quantizer. */
#define QuantizeFloat(_ptr_, _f_) __asm {            \
        __asm mov ebx, _ptr_                         \
        __asm fld [ebx]_ptr_._f_                     \
        __asm fadd QVal                              \
        __asm fsub QVal                              \
        __asm fstp [ebx]_ptr_._f_                    \
}

/* Rounding quantizer. */
#define QuantizeFloatVal(_f_) __asm {                \
        __asm fld _f_                                \
        __asm fadd QVal                              \
        __asm fsub QVal                              \
        __asm fstp _f_                               \
}


/* Truncating quantizer. */ 
/*  Could replace variable shift with a lookup table. */  
#define QuantizeTruncateFloatToCoord(f) \
  { int _if = *(int*)(&f); *(int*)(&f) = _if & (((int)0x80000000)>>((_if>>23)-115)); }
  
           


/*==================================================================================*/
typedef int DerivT;

#define DERIV_INT_BITS 15
#define DERIV_FRAC_BITS 16

#define FloatToDeriv(fval) \
    (_dtmp=(fval)+(DOUBLE_MANTISSA_OFFSET*(float)(3<<(DERIV_INT_BITS))), \
    (DerivT)(*((int*)(&_dtmp))))

#define ScaledFloatToDeriv(fval) \
    (_dtmp=(fval)+(DOUBLE_MANTISSA_OFFSET*(float)(3<<(DERIV_INT_BITS + COORD_FRAC_BITS))), \
    (DerivT)(*((int*)(&_dtmp))))

#define DoubleToDeriv(dval) \
    (_dtmp=(dval)+(DOUBLE_MANTISSA_OFFSET*(float)(3<<(DERIV_INT_BITS))), \
    (DerivT)(*((int*)(&_dtmp))))

#define ScaledDoubleToDeriv(dval) \
    (_dtmp=(dval)+(DOUBLE_MANTISSA_OFFSET*((float)(3<<DERIV_INT_BITS))*((float)(1<<COORD_FRAC_BITS))), \
    (DerivT)(*((int*)(&_dtmp))))

/*==================================================================================*/

typedef unsigned int UShortT;

#define USHORT_INT_BITS   16
#define USHORT_FRAC_BITS  16
#define USHORT_ONE        (1<<(USHORT_FRAC_BITS))
#define USHORT_SCALE      ((float)USHORT_ONE)

#define FloatToUShort(fval) \
    (_dtmp=(fval)+(DOUBLE_MANTISSA_OFFSET*(float)(1<<(USHORT_INT_BITS))), \
    (UShortT)(*((int*)(&_dtmp))))

#define ScaledFloatToUShort(fval) \
    (_dtmp=(fval)+(DOUBLE_MANTISSA_OFFSET*(float)(1<<USHORT_INT_BITS)*(float)(1<< USHORT_FRAC_BITS)), \
    (UShortT)(*((int*)(&_dtmp))))

/*==================================================================================*/

#ifdef __cplusplus
};
#endif


#endif /* FMACROS_H_ */

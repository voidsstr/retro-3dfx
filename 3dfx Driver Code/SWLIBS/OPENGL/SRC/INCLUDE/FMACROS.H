#ifndef __gl_fmacros_h__
#define __gl_fmacros_H__

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
** $Revision: 2$
** $Date: 10/11/00 8:04:58 PM$
*/

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

#endif /* __gl_fmacros_h__ */

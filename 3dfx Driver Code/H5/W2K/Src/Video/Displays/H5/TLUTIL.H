/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** File name:   tlutil.h
**
** Description: Odds and ends from the ref. rasterizer
**
**
*/


#ifdef TnL_HAL
#ifndef  _TLUTIL_H
#define  _TLUTIL_H

#include <math.h>

#ifndef FASTCALL
#ifdef _X86_
#define FASTCALL __fastcall
#else
#define FASTCALL
#endif
#endif

#ifndef CDECL
#ifdef _X86_
#define CDECL __cdecl
#else
#define CDECL
#endif
#endif



// width-specific typedefs for basic types
typedef signed char         INT8;       // NOT the same as char on all systems
typedef short int           INT16;
typedef int                 INT32;
typedef __int64             INT64;
typedef unsigned char       UINT8;
typedef unsigned short int  UINT16;
typedef unsigned int        UINT32;
typedef unsigned __int64    UINT64;


typedef float               FLOAT;
typedef double              DOUBLE;
typedef int                 BOOL;


typedef INT8                   *PINT8;
typedef INT16                  *PINT16;
typedef INT32                  *PINT32;
typedef INT64                  *PINT64;
typedef UINT8                  *PUINT8;
typedef UINT16                 *PUINT16;
typedef UINT32                 *PUINT32;
typedef UINT64                 *PUINT64;
typedef FLOAT                  *PFLOAT;
typedef DOUBLE                 *PDOUBLE;

typedef struct _TLVECTOR4
{
    D3DVALUE x;
    D3DVALUE y;
    D3DVALUE z;
    D3DVALUE w;
} TLVECTOR4, *LPTLVECTOR4;



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Macros                                                                    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))
#define GETABS(a) (((a) < 0) ? (-(a)) : (a))

// Check the return value and return if something wrong.
// Assume hr has been declared
#define HR_RET(exp)                                                           \
{                                                                             \
    hr = (exp);                                                               \
    if (hr != D3D_OK)                                                         \
    {                                                                         \
        return hr;                                                            \
    }                                                                         \
}


//-----------------------------------------------------------------------------
//
// macros for accessing floating point data as 32 bit integers and vice versa
//
// This is used primarily to do floating point to fixed point conversion with
// the unbiased nearest-even rounding that IEEE floating point does internally
// between operations.  Adding a big number slides the mantissa down to where
// the fixed point equivalent is aligned to the LSB.  IEEE applies a nearest-
// even round to the bits it lops off before storing.  The mantissa can then
// be grabbed by the AS_INT* operations.  Note that the sign and exponent are
// still there, so the easiest thing is to do it with doubles and grab the low
// 32 bits.
//
// The snap values (i.e. the "big number") is the sum of 2**n and 2**(n-1),
// which makes the trick return signed numbers (at least within the mantissa).
//
//-----------------------------------------------------------------------------

#if 0
// NOTE: vc5 optimizing compiler bug breaks this pointer casting technique
#define AS_FLOAT(i) ( *(FLOAT*)&(i) )
#define AS_INT32(f) ( *(INT32*)&(f) )
#define AS_INT16(f) ( *(INT16*)&(f) )
#define AS_UINT32(f) ( *(UINT32*)&(f) )

#else

// workaround using union
typedef union { float f; UINT32 u; INT32 i; } VAL32;
typedef union { double d; UINT64 u; INT64 i; } VAL64;
__inline FLOAT AS_FLOAT( long int iVal ) { VAL32 v; v.i = iVal; return v.f; }
__inline FLOAT AS_FLOATU( unsigned long int uVal ) { VAL32 v; v.u = uVal; return v.f; }
__inline INT32 AS_INT32(  FLOAT fVal ) { VAL32 v; v.f = fVal; return v.i; }
__inline INT32 AS_INT32D( DOUBLE dVal ) { VAL64 v; v.d = dVal; return (INT32)(v.u & 0xffffffff); }
__inline INT16 AS_INT16(  FLOAT fVal ) { VAL32 v; v.f = fVal; return (INT16)(v.u & 0xffff); }
__inline INT16 AS_INT16D( DOUBLE dVal ) { VAL64 v; v.d = dVal; return (INT16)(v.u & 0xffff); }
__inline INT32 AS_UINT32( FLOAT fVal ) { VAL32 v; v.f = fVal; return v.u; }

#endif

typedef struct _AlignedBuffer32
{
    LPVOID allocatedBuf;
    LPVOID alignedBuf;
    DWORD  size;
} AlignedBuffer32;


//-----------------------------------------------------------------------------
//
// Some common FP values as constants
// point values
//
//-----------------------------------------------------------------------------
#define g_fZero                 (0.0f)
#define g_fOne                  (1.0f)

// Integer representation of 1.0f.
#define INT32_FLOAT_ONE         0x3f800000

//-----------------------------------------------------------------------------
//
// these are handy to form 'magic' constants to snap real values to fixed
// point values
//
//-----------------------------------------------------------------------------
#define C2POW0 1
#define C2POW1 2
#define C2POW2 4
#define C2POW3 8
#define C2POW4 16
#define C2POW5 32
#define C2POW6 64
#define C2POW7 128
#define C2POW8 256
#define C2POW9 512
#define C2POW10 1024
#define C2POW11 2048
#define C2POW12 4096
#define C2POW13 8192
#define C2POW14 16384
#define C2POW15 32768
#define C2POW16 65536
#define C2POW17 131072
#define C2POW18 262144
#define C2POW19 524288
#define C2POW20 1048576
#define C2POW21 2097152
#define C2POW22 4194304
#define C2POW23 8388608
#define C2POW24 16777216
#define C2POW25 33554432
#define C2POW26 67108864
#define C2POW27 134217728
#define C2POW28 268435456
#define C2POW29 536870912
#define C2POW30 1073741824
#define C2POW31 2147483648
#define C2POW32 4294967296
#define C2POW33 8589934592
#define C2POW34 17179869184
#define C2POW35 34359738368
#define C2POW36 68719476736
#define C2POW37 137438953472
#define C2POW38 274877906944
#define C2POW39 549755813888
#define C2POW40 1099511627776
#define C2POW41 2199023255552
#define C2POW42 4398046511104
#define C2POW43 8796093022208
#define C2POW44 17592186044416
#define C2POW45 35184372088832
#define C2POW46 70368744177664
#define C2POW47 140737488355328
#define C2POW48 281474976710656
#define C2POW49 562949953421312
#define C2POW50 1125899906842624
#define C2POW51 2251799813685248
#define C2POW52 4503599627370496

#define FLOAT_0_SNAP    (FLOAT)(C2POW23+C2POW22)
#define FLOAT_4_SNAP    (FLOAT)(C2POW19+C2POW18)
#define FLOAT_5_SNAP    (FLOAT)(C2POW18+C2POW17)
#define FLOAT_8_SNAP    (FLOAT)(C2POW15+C2POW14)
#define FLOAT_17_SNAP   (FLOAT)(C2POW6 +C2POW5 )
#define FLOAT_18_SNAP   (FLOAT)(C2POW5 +C2POW4 )

#define DOUBLE_0_SNAP   (DOUBLE)(C2POW52+C2POW51)
#define DOUBLE_4_SNAP   (DOUBLE)(C2POW48+C2POW47)
#define DOUBLE_5_SNAP   (DOUBLE)(C2POW47+C2POW46)
#define DOUBLE_8_SNAP   (DOUBLE)(C2POW44+C2POW43)
#define DOUBLE_17_SNAP  (DOUBLE)(C2POW35+C2POW34)
#define DOUBLE_18_SNAP  (DOUBLE)(C2POW34+C2POW33)

//-----------------------------------------------------------------------------
//
// Floating point related macros
//
//-----------------------------------------------------------------------------
#define COSF(fV)        ((FLOAT)cos((double)(fV)))
#define SINF(fV)        ((FLOAT)sin((double)(fV)))
#define SQRTF(fV)       ((FLOAT)sqrt((double)(fV)))
#define POWF(fV, fE)    ((FLOAT)pow((double)(fV), (double)(fE)))

#define FLOAT_CMP_POS(fa, op, fb)       (AS_INT32(fa) op AS_INT32(fb))
#define FLOAT_CMP_PONE(flt, op)         (AS_INT32(flt) op INT32_FLOAT_ONE)

__inline int FLOAT_GTZ(FLOAT f)
{
    VAL32 fi;
    fi.f = f;
    return fi.i > 0;
}
__inline int FLOAT_LTZ(FLOAT f)
{
    VAL32 fi;
    fi.f = f;
    return fi.u > 0x80000000;
}
__inline int FLOAT_GEZ(FLOAT f)
{
    VAL32 fi;
    fi.f = f;
    return fi.u <= 0x80000000;
}
__inline int FLOAT_LEZ(FLOAT f)
{
    VAL32 fi;
    fi.f = f;
    return fi.i <= 0;
}
__inline int FLOAT_EQZ(FLOAT f)
{
    VAL32 fi;
    fi.f = f;
    return (fi.u & 0x7fffffff) == 0;
}
__inline int FLOAT_NEZ(FLOAT f)
{
    VAL32 fi;
    fi.f = f;
    return (fi.u & 0x7fffffff) != 0;
}

// Strip sign bit in integer.
__inline FLOAT
ABSF(FLOAT f)
{
    VAL32 fi;
    fi.f = f;
    fi.u &= 0x7fffffff;
    return fi.f;
}

// Requires chop rounding.
__inline INT
FTOI(FLOAT f)
{
    LARGE_INTEGER i;

#ifdef SSECPP
    __asm
    {
        cvtss2si    eax, f
        mov         i.LowPart, eax
    }
#else
    __asm
    {
        fld f
        fistp i
    }
#endif

    return i.LowPart;
}


///////////////////////////////////////////////////////////////////////////////
// Matrix and Vector routines
///////////////////////////////////////////////////////////////////////////////

__inline void ReverseVector(const D3DVECTOR *in, D3DVECTOR *out)
{
    out->x = -in->x;
    out->y = -in->y;
    out->z = -in->z;
}
__inline void ReverseVectorToSOA(const SOA_XYZ *in, SOA_XYZ *out)
{
    out->x.f[0] = -in->x.f[0];
    out->y.f[0] = -in->y.f[0];
    out->z.f[0] = -in->z.f[0];
}

__inline void AddVector(const D3DVECTOR *v1, const D3DVECTOR *v2, D3DVECTOR *out)
{
    out->x = v1->x + v2->x;
    out->y = v1->y + v2->y;
    out->z = v1->z + v2->z;
}
__inline void AddVectorFromSOA(const SOA_XYZ *v1, const D3DVECTOR *v2, D3DVECTOR *out)
{
    out->x = v1->x.f[0] + v2->x;
    out->y = v1->y.f[0] + v2->y;
    out->z = v1->z.f[0] + v2->z;
}

__inline void SubtractVector(const D3DVECTOR *v1, const D3DVECTOR *v2, D3DVECTOR *out)
{
    out->x = v1->x - v2->x;
    out->y = v1->y - v2->y;
    out->z = v1->z - v2->z;
}
__inline void SubtractVectorFromSOA(const SOA_XYZ *v1, const D3DVECTOR *v2, D3DVECTOR *out)
{
    out->x = v1->x.f[0] - v2->x;
    out->y = v1->y.f[0] - v2->y;
    out->z = v1->z.f[0] - v2->z;
}

__inline void SetIdentity(D3DMATRIX *m)
{
    m->_11 = m->_22 = m->_33 = m->_44 = 1.0f;
    m->_12 = m->_13 = m->_14 = 0.0f;
    m->_21 = m->_23 = m->_24 = 0.0f;
    m->_31 = m->_32 = m->_34 = 0.0f;
    m->_41 = m->_42 = m->_43 = 0.0f;
}


__inline void SetNull(D3DMATRIX *m)
{
    m->_11 = m->_22 = m->_33 = m->_44 = 0.0f;
    m->_12 = m->_13 = m->_14 = 0.0f;
    m->_21 = m->_23 = m->_24 = 0.0f;
    m->_31 = m->_32 = m->_34 = 0.0f;
    m->_41 = m->_42 = m->_43 = 0.0f;
}


__inline void CopyMatrix(D3DMATRIX *s, D3DMATRIX *d)
{
    d->_11 = s->_11;
    d->_12 = s->_12;
    d->_13 = s->_13;
    d->_14 = s->_14;
    d->_21 = s->_21;
    d->_22 = s->_22;
    d->_23 = s->_23;
    d->_24 = s->_24;
    d->_31 = s->_31;
    d->_32 = s->_32;
    d->_33 = s->_33;
    d->_34 = s->_34;
    d->_41 = s->_41;
    d->_42 = s->_42;
    d->_43 = s->_43;
    d->_44 = s->_44;
}

__inline D3DVALUE SquareMagnitude (const D3DVECTOR *v)
{
    return v->x*v->x + v->y*v->y + v->z*v->z;
}

#ifdef SSECPP
__inline void SquareMagnitude_m128 (D3DVALUE *dist, const D3DVECTOR *v)
{
   __declspec(align(16)) __m128 m128tmp;
    __asm
    {
       mov      eax, v
       movaps   xmm0, [eax]
       mov      ecx, dist
       mulps    xmm0, xmm0
       movaps   m128tmp,xmm0
       addss    xmm0, m128tmp+4
       addss    xmm0, m128tmp+8 
       movss    [ecx], xmm0
    }
}
__inline void SqrtSS (D3DVALUE *pout, D3DVALUE *pin)
{
    __asm
    {
       mov      eax, pin
       sqrtss   xmm0, [eax]
       mov      ecx, pout
       movss    [ecx], xmm0
    }
}

__inline void PowSS (D3DVALUE *pout, D3DVALUE *dot, D3DVALUE *shin)
{
    __asm
    {
       mov      eax,  shin
       movss    xmm1, [eax] ;// xmm1 = shin
       mov      edx, dot
       movss    xmm4, [edx]   ;// xmm4 = dot
       mov      ecx, pout
       movaps   xmm0, xmm1  ;// xmm0 = shin
       mulss    xmm1, xmm4  ;// shin*dot
       subss    xmm1, xmm4  ;// shin*dot-dot
       subss    xmm0, xmm1  ;// shin-shin*dot-dot
       rcpss    xmm0, xmm0  ;// 1/(shin-shin*dot-dot)
       mulss    xmm4, xmm0  ;// dot/(shin-shin*dot-dot)
       movss    [ecx], xmm4   
    }
}

#endif  // SSECPP

__inline D3DVALUE Magnitude (const D3DVECTOR *v)
{
    return (D3DVALUE) sqrt(SquareMagnitude(v));
}

__inline void Normalize (D3DVECTOR *v)
{
   #if 0  // VCPP can't align locals for ASM use, don't use for now.
   //#ifdef SSECPP
   __declspec(align(16)) __m128 m128tmp;
   
    // Perform entire Normilize in inline SSE assembler
    __asm
    {
       mov      eax, v
       movlps   xmm0, [eax]     ;// Move X & Y of vector in
       movhps   xmm0, [eax+8]   ;// Move Z of vector in
       movaps   xmm1, xmm0      ;// Duplicate it
       mulps    xmm0, xmm0      ;// Square the vector with itself
       movaps   m128tmp,xmm0    ;// Save the result
       addss    xmm0, m128tmp+4 ;// Add the products
       addss    xmm0, m128tmp+8 ;// XMM now has Mag^2
       rsqrtss  xmm3, xmm0      ;// Take the inverse square root
       shufps   xmm3, xmm3, 0   ;// Broadcast Inv Mag into xmm3
       mulps    xmm1, xmm3      ;// Multiply though by each component in the vec
       movlps   [eax], xmm1     ;// Save X & Y
       shufps   xmm1, xmm1, 0aah ;// Broadcast Z into xmm1
       movss    [eax+8], xmm1   ;// Save Z
    }
   #else
    D3DVALUE mag = Magnitude(v);

    v->x = v->x/mag;
    v->y = v->y/mag;
    v->z = v->z/mag;
   #endif

    return;
}

__inline void NormalizeToSOA(SOA_XYZ *v) {
  float mag = (float)1.0 / (float) sqrt(v->x.f[0] * v->x.f[0] + v->y.f[0] * v->y.f[0] + v->z.f[0] * v->z.f[0]);
  v->x.f[0] *= mag;
  v->y.f[0] *= mag;
  v->z.f[0] *= mag;
}

__inline D3DVECTOR CrossProduct (const D3DVECTOR* v1, const D3DVECTOR* v2)
{
        D3DVECTOR result;

        result.x = v1->y*v2->z - v1->z*v2->y;
        result.y = v1->z*v2->x - v1->x*v2->z;
        result.z = v1->x*v2->y - v1->y*v2->x;

        return result;
}

__inline D3DVALUE DotProduct (const D3DVECTOR *v1, const D3DVECTOR *v2)
{
        return v1->x*v2->x + v1->y*v2->y + v1->z*v2->z;
}
__inline D3DVALUE DotProductToSOA (const SOA_XYZ *v1, const D3DVECTOR *v2)
{
        return v1->x.f[0]*v2->x + v1->y.f[0]*v2->y + v1->z.f[0]*v2->z;
}
__inline D3DVALUE DotProductToSOA2(const D3DVECTOR *v1, const SOA_XYZ *v2)
{
        return v1->x*v2->x.f[0] + v1->y*v2->y.f[0] + v1->z*v2->z.f[0];
}

//---------------------------------------------------------------------
// Multiplies vector (x,y,z,w) by a 4x4 matrix transposed,
// producing a homogeneous vector
//
// res and v should not be the same
//---------------------------------------------------------------------
__inline void XformPlaneBy4x4Transposed(TLVECTOR4 *v, D3DMATRIX *m, TLVECTOR4 *res)
{
    res->x = v->x*m->_11 + v->y*m->_12 + v->z*m->_13 + v->w*m->_14;
    res->y = v->x*m->_21 + v->y*m->_22 + v->z*m->_23 + v->w*m->_24;
    res->z = v->x*m->_31 + v->y*m->_32 + v->z*m->_33 + v->w*m->_34;
    res->w = v->x*m->_41 + v->y*m->_42 + v->z*m->_43 + v->w*m->_44;
}
//---------------------------------------------------------------------
// Multiplies vector (x,y,z,w) by 4x4 matrix, producing a homogeneous vector
//
// res and v should not be the same
//---------------------------------------------------------------------
__inline void XformPlaneBy4x4(TLVECTOR4 *v, D3DMATRIX *m, TLVECTOR4 *res)
{
    res->x = v->x*m->_11 + v->y*m->_21 + v->z*m->_31 + v->w*m->_41;
    res->y = v->x*m->_12 + v->y*m->_22 + v->z*m->_32 + v->w*m->_42;
    res->z = v->x*m->_13 + v->y*m->_23 + v->z*m->_33 + v->w*m->_43;
    res->w = v->x*m->_14 + v->y*m->_24 + v->z*m->_34 + v->w*m->_44;
}
//---------------------------------------------------------------------
// Multiplies vector (x,y,z,1) by 4x4 matrix, producing a homogeneous vector
//
// res and v should not be the same
//---------------------------------------------------------------------
__inline void XformBy4x4(D3DVECTOR *v, D3DMATRIX *m, TLVECTOR4 *res)
{
    res->x = v->x*m->_11 + v->y*m->_21 + v->z*m->_31 + m->_41;
    res->y = v->x*m->_12 + v->y*m->_22 + v->z*m->_32 + m->_42;
    res->z = v->x*m->_13 + v->y*m->_23 + v->z*m->_33 + m->_43;
    res->w = v->x*m->_14 + v->y*m->_24 + v->z*m->_34 + m->_44;
}
//---------------------------------------------------------------------
// Multiplies vector (x,y,z,1) by 4x3 matrix
//
// res and v should not be the same
//---------------------------------------------------------------------
__inline void XformBy4x3(D3DVECTOR *v, D3DMATRIX *m, D3DVECTOR *res)
{
    res->x = v->x*m->_11 + v->y*m->_21 + v->z*m->_31 + m->_41;
    res->y = v->x*m->_12 + v->y*m->_22 + v->z*m->_32 + m->_42;
    res->z = v->x*m->_13 + v->y*m->_23 + v->z*m->_33 + m->_43;
}
//---------------------------------------------------------------------
// Multiplies vector (x,y,z,1) by 4x3 matrix
//
// res and v should not be the same. res must be an SOA vector
//---------------------------------------------------------------------
__inline void XformBy4x3ToSOA(D3DVECTOR *v, D3DMATRIX *m, SOA_XYZ *res)
{
    res->x.f[0] = v->x*m->_11 + v->y*m->_21 + v->z*m->_31 + m->_41;
    res->y.f[0] = v->x*m->_12 + v->y*m->_22 + v->z*m->_32 + m->_42;
    res->z.f[0] = v->x*m->_13 + v->y*m->_23 + v->z*m->_33 + m->_43;
}
//---------------------------------------------------------------------
// Swizzles SOA_FLOAT 
//
//---------------------------------------------------------------------
__inline void SwizzleSOAFLOAT(SOA_FLOAT *f)
{
  f->f[3] = f->f[2] = f->f[1] = f->f[0]; 
}
//---------------------------------------------------------------------
// Swizzles SOA_DWORD
//
//---------------------------------------------------------------------
__inline void SwizzleSOADWORD(SOA_DWORD *f)
{
  f->d[3] = f->d[2] = f->d[1] = f->d[0]; 
}
//---------------------------------------------------------------------
// Swizzles vector (x,y,z) 
//
//---------------------------------------------------------------------
__inline void SwizzleVec3(SOA_XYZ *res)
{
  SwizzleSOAFLOAT(&res->x);
  SwizzleSOAFLOAT(&res->y);
  SwizzleSOAFLOAT(&res->z);
}
//---------------------------------------------------------------------
// Multiplies vector (x,y,z) by 3x3 matrix
//
// res and v should not be the same
//---------------------------------------------------------------------
__inline void Xform3VecBy3x3(D3DVECTOR *v, D3DMATRIX *m, D3DVECTOR *res)
{
    res->x = v->x*m->_11 + v->y*m->_21 + v->z*m->_31;
    res->y = v->x*m->_12 + v->y*m->_22 + v->z*m->_32;
    res->z = v->x*m->_13 + v->y*m->_23 + v->z*m->_33;
}
//---------------------------------------------------------------------
// Multiplies vector (x,y,z) by 3x3 matrix
//
// res and v should not be the same. res must be an SOA vector
//---------------------------------------------------------------------
__inline void Xform3VecBy3x3ToSOA(D3DVECTOR *v, D3DMATRIX *m, SOA_XYZ *res)
{
    res->x.f[0] = v->x*m->_11 + v->y*m->_21 + v->z*m->_31;
    res->y.f[0] = v->x*m->_12 + v->y*m->_22 + v->z*m->_32;
    res->z.f[0] = v->x*m->_13 + v->y*m->_23 + v->z*m->_33;
}



void AB32_Create(NT9XDEVICEDATA *ppdev, AlignedBuffer32 *Buff);
void AB32_Destroy(NT9XDEVICEDATA *ppdev, AlignedBuffer32 *Buff);
HRESULT AB32_Grow(NT9XDEVICEDATA *ppdev, AlignedBuffer32 *Buff, DWORD dwSize); 
HRESULT AB32_CheckAndGrow(NT9XDEVICEDATA *ppdev, AlignedBuffer32 *Buff, DWORD dwSize);



#endif  //_TLUTIL_H
#endif //TnL_HAL
/*
** Copyright (c) 1999, 3Dfx Interactive, Inc.
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
** File name: 
**
** Description: SOA Transformation code
**
** $Revision: 12$
** $Date: 10/11/00 8:50:27 PM$
**
** $Log: 
**  12   3dfx      1.5.1.0.1.4 10/11/00 Brent           Forced check in to enforce
**       branching.
**  11   3dfx      1.5.1.0.1.3 09/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  10   3dfx      1.5.1.0.1.2 09/22/00 Allen Hansen    implemented texgen and
**       texture transformation to the fastpaths
**  9    3dfx      1.5.1.0.1.1 08/21/00 Allen Hansen    changed compiler check from
**       SSECPP to VCPP 
**  8    3dfx      1.5.1.0.1.0 06/25/00 Allen Hansen    removed old code that had
**       long been #ifdef'd out
**  7    3dfx      1.5.1.0     06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  6    Napalm    1.5         04/25/00 Allen Hansen    got rid of old
**       XForm_3/4_SOA functions (were unused), implemented these with inline asm
**       macros in soaxform.h
**  5    Napalm    1.4         04/19/00 Scott Kephart   Big lighting change - Part
**       I
**       Lighting is now split into two parts, diffuse and specular. 
**  4    Napalm    1.3         04/10/00 Allen Hansen    Worked prefetching: Changed
**       FP_IndexedTriangleList2_SOA_Split() to transform 4 soa groups instead of
**       1.  This required modifing AllocateSOAFVF().  I added a version of
**       FP_Xform_4Vert_SOA() that prefetches the next SOA group.  This still needs
**       optimization, but almost all cache-read misses are hidden.
**  3    Napalm    1.2         04/09/00 Allen Hansen    Prefetch work to all but
**       position x/y, some optimization work to the 3x3 and 4x3 transforms
**  2    Napalm    1.1         03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  1    Napalm    1.0         03/08/00 Scott Kephart   
** $
*/



#include "precomp.h"

#if( DX >= 7 )
#if defined(TnL_HAL) && defined(VCPP)

#ifndef WINNT
#include <d3dhal.h>
#include "d6fvf.h"
#include "fxglobal.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "fifomgr.h"
#include "d3tri.h"
#include "d6global.h"
#include "d3contxt.h"
#endif

#include "dxins.h"



#if 0 //unused
/*-------------------------------------------------------------------
Function Name:  Xform_4_SOA
Description:    Transform x, y, z, <1 implied> to x', y', z', w' via a 
                4x4 matrix. This happens in SOA form, so it really 
                translates 4 x's, 4 y's, etc.
Parameters:   
                SOA_XYZW *pD -- pointer to destination
                SOA_XYZ *pV -- pointer to input vertices
                SOA_MATRIX *pMat -- pointer to SOA matrix
Information:    I measured 72 clocks for this function as opposed to 92 
                for the old one
                
Return:         
-------------------------------------------------------------------*/
__declspec(align(32)) void Xform_4_SOA(SOA_XYZW *pD, SOA_XYZ *pV, SOA_MATRIX *pSOAMat4x4x4)
{
#if 0
  for(int i=0; i<SOA_SIZE; ++i)
  {
  	pD->x.f[i] = pV->x.f[i] * pMat4x4x4->_11 + pV->y.f[i] * pMat4x4x4->_21 + pV->z.f[i] * pMat4x4x4->_31 + pMat4x4x4->_41;
  	pD->y.f[i] = pV->x.f[i] * pMat4x4x4->_12 + pV->y.f[i] * pMat4x4x4->_22 + pV->z.f[i] * pMat4x4x4->_32 + pMat4x4x4->_42;
  	pD->z.f[i] = pV->x.f[i] * pMat4x4x4->_13 + pV->y.f[i] * pMat4x4x4->_23 + pV->z.f[i] * pMat4x4x4->_33 + pMat4x4x4->_43;
  	pD->w.f[i] = pV->x.f[i] * pMat4x4x4->_14 + pV->y.f[i] * pMat4x4x4->_24 + pV->z.f[i] * pMat4x4x4->_34 + pMat4x4x4->_44;
  }
#endif
  _asm 
  {
    mov       ecx, [pV]
    mov       edx, [pSOAMat4x4x4]
    mov       eax, [pD]

    movaps    xmm7, [ecx]_SV3.x       // x[i]
    movaps    xmm3, [ecx]_SV3.y       // y[i]
	prefetchnta [ecx]_SV3.z           // z[i]
    movaps    xmm4, xmm7
    mulps     xmm4, [edx]_SM._11      // x[i] * _11
    movaps    xmm0, xmm3
    mulps     xmm0, [edx]_SM._21      // y[i] * _21
    movaps    xmm5, xmm7
    mulps     xmm5, [edx]_SM._12      // x[i] * _12
    movaps    xmm1, xmm3
    mulps     xmm1, [edx]_SM._22      // y[i] * _22
    addps     xmm4, xmm0              // x[i] * _11 + y[i] * _21
    movaps    xmm6, xmm7
    mulps     xmm6, [edx]_SM._13      // x[i] * _13
    movaps    xmm2, xmm3
    mulps     xmm2, [edx]_SM._23      // y[i] * _23
    addps     xmm5, xmm1              // x[i] * _12 + y[i] * _22
    mulps     xmm7, [edx]_SM._14      // x[i] * _14
    movaps    xmm0, [ecx]_SV3.z       // z[i]
    addps     xmm4, [edx]_SM._41      // x[i] * _11 + y[i] * _21 +  _41
    mulps     xmm3, [edx]_SM._24      // y[i] * _24
	prefetchnta [ecx+0x40]            // y of the normals (or 1st blend)
    addps     xmm5, [edx]_SM._42      // x[i] * _12 + y[i] * _22 +  _42
    movaps    xmm1, xmm0
    mulps     xmm1, [edx]_SM._31      // z[i] * _31
    addps     xmm6, xmm2              // x[i] * _13 + y[i] * _23
    movaps    xmm2, xmm0
    addps     xmm7, xmm3              // x[i] * _14 + y[i] * _24
    mulps     xmm2, [edx]_SM._32      // z[i] * _32
    movaps    xmm3, xmm0
    addps     xmm6, [edx]_SM._43      // x[i] * _13 + y[i] * _23 +  _43
    mulps     xmm3, [edx]_SM._33      // z[i] * _33
    addps     xmm7, [edx]_SM._44      // x[i] * _14 + y[i] * _24 +  _44
    mulps     xmm0, [edx]_SM._34      // z[i] * _34
    addps     xmm4, xmm1              // x[i] * _11 + y[i] * _21 + z[i] * _31 + _41
    addps     xmm5, xmm2              // x[i] * _12 + y[i] * _22 + z[i] * _32 + _42
    movaps    [eax]_SV4.x, xmm4       // pD->x
    addps     xmm6, xmm3              // x[i] * _13 + y[i] * _23 + z[i] * _33 + _43
    movaps    [eax]_SV4.y, xmm5       // pD->y
    addps     xmm7, xmm0              // x[i] * _14 + y[i] * _24 + z[i] * _34 + _44
    movaps    [eax]_SV4.z, xmm6       // pD->z
    movaps    [eax]_SV4.w, xmm7       // pD->w
  }
}
#endif //unused


/*-------------------------------------------------------------------
Function Name:  Xform_3_SOA
Description:    Transform x, y, z to x', y', z' via a 
                4x4 matrix. This happens in SOA form, so it really 
                translates 4 x's, 4 y's, etc.
Parameters:   
                SOA_XYZW *pD -- pointer to destination
                SOA_XYZ *pV -- pointer to input vertices
                SOA_MATRIX *pMat -- pointer to SOA matrix
Information:    
                I measured 62 clocks for this function as opposed 
                to 76 for the old one
Return:         
-------------------------------------------------------------------*/
__declspec(align(32)) void Xform_3_SOA(SOA_XYZ *pD, SOA_XYZ *pV, SOA_MATRIX *pSOAMat4x4x4)
{
#if 0
  for(int i=0; i<SOA_SIZE; ++i)
  {
  	pD->x.f[i] = pV->x.f[i] * pMat4x4x4->_11 + pV->y.f[i] * pMat4x4x4->_21 + pV->z.f[i] * pMat4x4x4->_31 + pMat4x4x4->_41;
  	pD->y.f[i] = pV->x.f[i] * pMat4x4x4->_12 + pV->y.f[i] * pMat4x4x4->_22 + pV->z.f[i] * pMat4x4x4->_32 + pMat4x4x4->_42;
  	pD->z.f[i] = pV->x.f[i] * pMat4x4x4->_13 + pV->y.f[i] * pMat4x4x4->_23 + pV->z.f[i] * pMat4x4x4->_33 + pMat4x4x4->_43;
  }
#endif
  _asm 
  {
    mov       ecx, [pV]
    mov       edx, [pSOAMat4x4x4]
    mov       eax, [pD]

    movaps    xmm2, [ecx]_SV3.x     // x[i]
    movaps    xmm5, [ecx]_SV3.y     // y[i]
    movaps    xmm0, xmm2
    movaps    xmm1, xmm2
    mulps     xmm0, [edx]_SM._11    // x[i] * _11
    mulps     xmm1, [edx]_SM._12    // x[i] * _12
    mulps     xmm2, [edx]_SM._13    // x[i] * _13

    addps     xmm0, [edx]_SM._41    // x[i] * _11 + y[i] * _21 + z[i] * _31 + _41
    addps     xmm1, [edx]_SM._42    // x[i] * _12 + y[i] * _22 + z[i] * _32 + _42
    addps     xmm2, [edx]_SM._43    // x[i] * _13 + y[i] * _23 + z[i] * _33 + _43

    movaps    xmm7, [ecx]_SV3.z     // z[i]
    movaps    xmm3, xmm5
    movaps    xmm4, xmm5

    mulps     xmm3, [edx]_SM._21    // y[i] * _21
    mulps     xmm4, [edx]_SM._22    // y[i] * _22
    mulps     xmm5, [edx]_SM._23    // y[i] * _23
    addps     xmm0, xmm3            // x[i] * _11 + y[i] * _21
    addps     xmm1, xmm4            // x[i] * _12 + y[i] * _22
    addps     xmm2, xmm5            // x[i] * _13 + y[i] * _23

    movaps    xmm6, xmm7
    movaps    xmm3, xmm7
    mulps     xmm6, [edx]_SM._31    // z[i] * _31
    mulps     xmm3, [edx]_SM._32    // z[i] * _32
    mulps     xmm7, [edx]_SM._33    // z[i] * _33
    addps     xmm0, xmm6            // x[i] * _11 + y[i] * _21 + z[i] * _31
    addps     xmm1, xmm3            // x[i] * _12 + y[i] * _22 + z[i] * _32
    addps     xmm2, xmm7            // x[i] * _13 + y[i] * _23 + z[i] * _33

    movaps    [eax]_SV3.x, xmm0     // pD->x
    movaps    [eax]_SV3.y, xmm1     // pD->y
    movaps    [eax]_SV3.z, xmm2     // pD->z
  }
}





#endif
#endif
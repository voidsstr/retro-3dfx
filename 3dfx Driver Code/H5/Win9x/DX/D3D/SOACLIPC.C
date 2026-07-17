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
** File name: soaclipc.c
**
** Description: Generate Clip Codes for SOA vertices
**
** $Revision: 10$
** $Date: 10/11/00 8:50:13 PM$
**
** $Log: 
**  10   3dfx      1.2.1.1.1.4 10/11/00 Brent           Forced check in to enforce
**       branching.
**  9    3dfx      1.2.1.1.1.3 09/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  8    3dfx      1.2.1.1.1.2 09/15/00 Allen Hansen    In ComputeClipCodesSOA,
**       load the homogenous coordinates from local regs instead of memory
**  7    3dfx      1.2.1.1.1.1 08/21/00 Allen Hansen    changed compiler check from
**       SSECPP to VCPP 
**  6    3dfx      1.2.1.1.1.0 06/25/00 Allen Hansen    updated TL_SOATMP variable
**       name, wrote hacked up ComputeClipCodes_SOA_C (will go away)
**  5    3dfx      1.2.1.1     06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  4    3dfx      1.2.1.0     05/16/00 Bob Johnston    Consolodating VERT_BUFF and
**       TnL_HAL defines to just use TnL_HAL
**  3    Napalm    1.2         03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  2    Napalm    1.1         03/14/00 Scott Kephart   Use version of clip code
**       generation that matches ref. rast
**  1    Napalm    1.0         03/08/00 Scott Kephart   
** $
** 
** 2     3/12/00 9:52p Skephart
** Fixed 3D Mark clipping bugs - clip code generation matches the ref.
** rast.
** 
** 1     3/08/00 9:26p Skephart
*/

#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL
#if defined(VCPP)

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


#if 0	// unused but don't delete
// This version of ComputeClipCodes_SOA is fast, and has exactly the 
// same behavior with negative w values as does the slow-path
// ComputeClipCodes() function. However, it's not being used currently,
// because really vertices with -w should be clipped anyway, and because
// what ComputeClipCodes() does with negative w isn't correct either!
// However, I'm leaving it in place in case we discover a case where 
// the particular behavior we have here is needed by the clip code, wrong
// or not!
//
// The difference between the two versions of ComputeClipCodes_SOA is that 
// when w and x, y, or z are compared, this version has the same behavior
// as ComputeClipCodes() if both w and x, w and y, or w and z are negative.
// 
// The reason for this difference is that integer compare instructions are 
// used to generate the clip code bits, instead of floating point instructions.
// This works when both values are positive, or when one is positive and the 
// other is negative. But when both negative, an integer compare gives the 
// reverse sense. So we fix that in this routine by checking for both values
// to be negative, and then inverting the result if they are.


/*-------------------------------------------------------------------
Function Name:  ComputeClipCodes_SOA
Description:    Compute Clip Codes for data in SOA format
                All possible cases for Clipping are covered by this
                routine, including user clip planes.
Parameters:   
                RC *pRC -- current rendering context
                pRc->tl.pTL->SOAhv -- vertex transformed to homogeneous 
                    coordinates
                pRc->tl.pTL->fSOAgb11 .. pRc->tl.pTL->fSOAsgb41 -- SOA swizzled
                    versions of pRc->tl.ViewData.gb11...
Information:    
                pRc->tl.pTL->gb_v and pRc->tl.pTL->user_plane_eq
                are temporary variables used in the calculation of the 
                clip codes.

                Clip codes are stored in pRc->tl.pTL->dSOAclip_code

                Execution times:
                6 bit code generation only: ~14 clocks / vertex
                Guard band bits:            ~22 clocks / vertex
                4 User clip planes          ~63 clocks / vertex
Return:         
-------------------------------------------------------------------*/
void ComputePlaneDataC(RC *pRc)	// crappy hack to get this back in C
{
	int i;
    TL_SOATMP *pTlTmp = (TL_SOATMP *)pRc->tl.pTL;
	SOA_DWORD *planeeq = (SOA_DWORD*)&pTlTmp->fSOAuser_planeeq;

	for (i=0; i<SOA_SIZE; ++i)
	{
		if ( ((pTlTmp->SOAhv.x.f[i] * pRc->tl.xfmUserClipPlanes->plane.x) +
			  (pTlTmp->SOAhv.y.f[i] * pRc->tl.xfmUserClipPlanes->plane.y) +
			  (pTlTmp->SOAhv.z.f[i] * pRc->tl.xfmUserClipPlanes->plane.z) +
			  (pTlTmp->SOAhv.w.f[i] * pRc->tl.xfmUserClipPlanes->plane.w)) < 0.0f)
			planeeq->d[i] = 0xffffffff;
		else
			planeeq->d[i] = 0;
	}
}

//#define SSE_CLIPCODES
void ComputeClipCodes_SOA_C(RC *pRc)
{
	int i;
    TL_SOATMP *pTlTmp = (TL_SOATMP *)pRc->tl.pTL;
#ifndef SSE_CLIPCODES
__int64 tmpMmxRegs[8];
#endif

	for (i=0; i<SOA_SIZE; ++i)
	{
		if (pRc->tl.dwTLState & TLPV_GUARDBAND)
		{
		    // First, transform X and Y into guardband clip space for clip check
			//   D3DVALUE xnew = lpClip->x * pRc->tl.ViewData.fSOAgb11 + lpClip->w * pRc->tl.ViewData.fSOAgb41;
			//   D3DVALUE ynew = lpClip->y * pRc->tl.ViewData.fSOAgb22 + lpClip->w * pRc->tl.ViewData.fSOAgb42;
			pTlTmp->SOAgb_v.x.f[i] = (pTlTmp->SOAhv.x.f[i] * pTlTmp->fSOAgb11.f[0]) + (pTlTmp->SOAhv.w.f[i] * pTlTmp->fSOAgb41.f[0]);
			pTlTmp->SOAgb_v.y.f[i] = (pTlTmp->SOAhv.y.f[i] * pTlTmp->fSOAgb22.f[0]) + (pTlTmp->SOAhv.w.f[i] * pTlTmp->fSOAgb42.f[0]);
		}
	}

  __asm {
    mov     eax, pRc
    mov     ecx, [eax]RC.tl.dwTLState;
    mov     eax, [eax]RC.tl.pTL

    // Register usage throughout this code:
    // eax = pRc->tl.pTL
    // ebx = &pRc->tl.xfmUserClipPlanes
    // ecx = pRc->tl.dwTLState
    // mm5 = low part of outcode
    // mm6 = high part of outcode
    // mm7 = TL_clip_mask -- mask bits used for TL

    // Generate the basic 6 bit outcode for X, Y, Z
    // The clip range is: 0 < x < w
    //                    0 < y < w
    //                    0 < z < w

    // if (x < 0)  clip_code |= RRCLIP_LEFTBIT;    
    // if (x >= we) clip_code |= RRCLIP_RIGHTBIT;  
    // if (y < 0)  clip_code |= RRCLIP_BOTTOMBIT;  
    // if (y >= we) clip_code |= RRCLIP_TOPBIT;    
    // if (z < 0)    clip_code |= RRCLIP_FRONTBIT; 
    // if (z >= we) clip_code |= RRCLIP_BACKBIT;   

    movq    mm7, TL_clip_mask                 //mm7 = 80000000 80000000

    // X LO
    movq    mm0, [eax]_STL.SOAhv.x.m.lo       //mm0 = x1       x0
    movq    mm3, [eax]_STL.SOAhv.w.m.lo       //mm3 = w1       w0

    movq    mm5, mm0
    movq    mm2, mm3                          //mm2 = w1      w0

    pand    mm2, mm7                          //mm2 = w < 0?
    movq    mm1, mm2                          //mm1 = w < 0?

    pand    mm0, mm7                          //mm0 = x < 0?
    pand    mm2, mm0                          //mm2 = w < 0? and x < 0?
    psrld   mm0, (32-TLCLIP_LEFTBIT)          //shift sign bits into clip code bits

    pcmpgtd mm5, mm3                          //mm5 = x > w?
    pand    mm5, mm7                          //mm5 = sign bits set if x > w

    pxor    mm5, mm2                          //mm5 = x > w, even if both w&x < 0

    psrld   mm5, (32-TLCLIP_RIGHTBIT)         //shift sign bits into clip code bits

    por     mm5, mm0                          //mm5 = clip code

    // Y LO
    movq    mm0, [eax]_STL.SOAhv.y.m.lo       //mm1 = y1       y0
    movq    mm4, mm0
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //mm1 = y < 0?
    pand    mm2, mm0                          //mm2 = w < 0? and y < 0?
    psrld   mm0, (32-TLCLIP_BOTTOMBIT)        //shift sign bits into clip code bits

    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits set if y ? w

    pxor    mm4, mm2                          //mm4 = y > w, even if both w&y < 0

    psrld   mm4, (32-TLCLIP_TOPBIT)           //shift sign bits into clip code bits
    por     mm5, mm4                          //mm5 = left | bottom clip code

    por     mm5, mm0 

    // Z LO
    movq    mm0, [eax]_STL.SOAhv.z.m.lo       //mm2 = z1       z0
    movq    mm4, mm0                          //mm4 = z > w
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //shift sign bits into clip code bits
    pand    mm2, mm0                          //mm2 = w < 0? and z < 0?
    psrld   mm0, (32-TLCLIP_FRONTBIT)

    pcmpgtd mm4, mm3                          //mm4 = sign bits set if y ? w
    pand    mm4, mm7                          //shift sign bits into clip code bits

    pxor    mm4, mm2                          //mm4 = z > w, even if both w&z < 0

    psrld   mm4, (32-TLCLIP_BACKBIT)          
    por     mm5, mm4                    
                                              //mm1 = z < 0?
    por     mm5, mm0 
                                              //mm7 = 80000000 80000000

    // X HI
    movq    mm0, [eax]_STL.SOAhv.x.m.hi       //mm0 = x1       x0
    movq    mm3, [eax]_STL.SOAhv.w.m.hi       //mm3 = w1       w0

    movq    mm6, mm0                          
    movq    mm2, mm3                          //mm2 = w1      w0

    pand    mm2, mm7                          //mm2 = w < 0?
    movq    mm1, mm2                          //mm1 = w < 0?

    pand    mm0, mm7                          //mm0 = x < 0?
    pand    mm2, mm0                          //mm2 = w < 0? and z < 0?
    psrld   mm0, (32-TLCLIP_LEFTBIT)          //shift sign bits into clip code bits

    pcmpgtd mm6, mm3                          //mm4 = x > w?
    pand    mm6, mm7                          //mm4 = sign bits set if x > w

    pxor    mm6, mm2                          //mm4 = x > w, even if both w&x < 0

    psrld   mm6, (32-TLCLIP_RIGHTBIT)         //shift sign bits into clip code bits

    por     mm6, mm0                          //mm6 = clip code

    // Y HI
    movq    mm0, [eax]_STL.SOAhv.y.m.hi       //mm1 = y1       y0
    movq    mm4, mm0
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //mm1 = y < 0?
    pand    mm2, mm0                          //mm2 = w < 0? and z < 0?
    psrld   mm0, (32-TLCLIP_BOTTOMBIT)        //shift sign bits into clip code bits

    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits set if y > w

    pxor    mm4, mm2                          //mm4 = y > w, even if both w&y < 0

    psrld   mm4, (32-TLCLIP_TOPBIT)           //shift sign bits into clip code bits
    por     mm6, mm4                          //mm6 = left | bottom clip code

    por     mm6, mm0 

    // Z HI
    movq    mm0, [eax]_STL.SOAhv.z.m.hi       //mm2 = z1       z0
    movq    mm4, mm0                          //mm4 = z > w
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //shift sign bits into clip code bits
    pand    mm2, mm0                          //mm2 = w < 0? and z < 0?
    psrld   mm0, (32-TLCLIP_FRONTBIT)

    pcmpgtd mm4, mm3                          //mm4 = sign bits set if z > w
    pand    mm4, mm7                          //shift sign bits into clip code bits

    pxor    mm4, mm2                          //mm4 = z > w, even if both w&z < 0

    psrld   mm4, (32-TLCLIP_BACKBIT)
    por     mm6, mm4                    
                                              //mm1 = z < 0?
    por     mm6, mm0 

    test    ecx, TLPV_GUARDBAND
    jz      NoGuardBand

    // Guardband clipping
//GenGuardBandClipCode:
  //   D3DVALUE xx = lpClip->w - xnew;
  //   D3DVALUE yy = lpClip->w - ynew;
  //   clip_code |= ((AS_INT32(xnew) & 0x80000000) >> (32-TLCLIPGB_LEFTBIT))   |
  //                ((AS_INT32(ynew) & 0x80000000) >> (32-TLCLIPGB_BOTTOMBIT)) |
  //                ((AS_INT32(xx)   & 0x80000000) >> (32-TLCLIPGB_RIGHTBIT))  |
  //                ((AS_INT32(yy)   & 0x80000000) >> (32-TLCLIPGB_TOPBIT));

    movq    mm0, [eax]_STL.SOAgb_v.x.m.lo     //mm0 = x1       x0
    movq    mm3, [eax]_STL.SOAhv.w.m.lo       //mm3 = w1       w0
    movq    mm4, mm0                          //mm4 = x1       x0
    movq    mm2, mm3                          
    pand    mm2, mm7                          //mm2 = w < 0?
    movq    mm1, mm2

    pand    mm0, mm7                          //mm0 = x < 0
    pand    mm2, mm0                          //mm2 = w < 0 && x < 0?
    psrld   mm0, (32-TLCLIPGB_LEFTBIT)

    pcmpgtd mm4, mm3                          //mm4 = x > w
    pand    mm4, mm7                          //mm4 = sign bits of w-x

    pxor    mm4, mm2                          //flip if both w and x are < 0

    psrld   mm4, (32-TLCLIPGB_RIGHTBIT)
    por     mm5, mm4                          //mm5 = clip_code

    por     mm5, mm0 

    movq    mm0, [eax]_STL.SOAgb_v.y.m.lo     //mm1 = y1       y0
    movq    mm4, mm0
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //mm0 = y < 0?
    pand    mm2, mm0                          //mm0 = w < 0 && y < 0?
    psrld   mm0, (32-TLCLIPGB_BOTTOMBIT)


    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits of w-y

    pxor    mm4, mm2                          //flip if both w and y are < 0

    psrld   mm4, (32-TLCLIPGB_TOPBIT)
    por     mm5, mm4                          //mm6 = or in GB Clip code bits

    por     mm5, mm0 

    movq    mm0, [eax]_STL.SOAgb_v.x.m.hi     //mm0 = x1       x0
    movq    mm3, [eax]_STL.SOAhv.w.m.hi       //mm3 = w1       w0
    movq    mm4, mm0
    movq    mm2, mm3
    pand    mm2, mm7                          //mm2 = w < 0?
    movq    mm1, mm2

    pand    mm0, mm7                          //mm0 = x < 0
    pand    mm2, mm0                          //mm2 = w < 0 && x < 0
    psrld   mm0, (32-TLCLIPGB_LEFTBIT)        //mm6 = clip_code

    pcmpgtd mm4, mm3                          //mm4 = x > w
    pand    mm4, mm7                          //mm4 = sign bits of w-x

    pxor    mm4, mm2                          //flip if both w and x are < 0

    psrld   mm4, (32-TLCLIPGB_RIGHTBIT)
    por     mm6, mm4                          //mm6 = clip_code

    por     mm6, mm0 

    movq    mm0, [eax]_STL.SOAgb_v.y.m.hi     //mm1 = y1       y0
    movq    mm4, mm0
    movq    mm2, mm1

    pand    mm0, mm7                          //mm0 = y < 0
    pand    mm2, mm0                          //mm2 = w < 0 && y < 0
    psrld   mm0, (32-TLCLIPGB_BOTTOMBIT)

    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits of w-y

    pxor    mm4, mm2                          //flip if both w and y are < 0

    psrld   mm4, (32-TLCLIPGB_TOPBIT)
    por     mm6, mm4                          //mm6 = left | bottom clip_code

    por     mm6, mm0 

NoGuardBand:
    test    ecx, TLPV_USERCLIPPLANES          // Check for User Clip Planes
    jz      NoUserClipPlanes    


    // User Clip Planes
    mov     ebx, pRc
    lea     ebx, [ebx]RC.tl.xfmUserClipPlanes
    mov     ecx, TLMAX_USER_CLIPPLANES

#ifdef SSE_CLIPCODES
    movaps  xmm4, [eax]_STL.SOAhv.x           //xmm4 = hv.x
    movaps  xmm5, [eax]_STL.SOAhv.y           //xmm5 = hv.y
    movaps  xmm6, [eax]_STL.SOAhv.z           //xmm6 = hv.z
    movaps  xmm7, [eax]_STL.SOAhv.w           //xmm7 = hv.w
#endif
    movq    mm0, TL_userclip_mask             //mm0 = User clip plane bits

ClipPlaneLoop:
//   TLCLIPCODE clipBit = TLCLIP_USERCLIPPLANE0;
//   if (pRc->tl.dwTLState & TLPV_USERCLIPPLANES)
//       for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
//       {
//         if( pRc->tl.xfmUserClipPlanes[j].bActive )
//         {
//           TLVECTOR4 *plane = &pRc->tl.xfmUserClipPlanes[j].plane;
//           FLOAT fComp = 0.0f;
//           if( (lpClip->x*plane->x +
//              lpClip->y*plane->y +
//              lpClip->z*plane->z +
//              lpClip->w*plane->w) < fComp )
//           {
//             clip_code |= clipBit;
//           }
//         }
//         clipBit <<= 1;
//       }
    test    [ebx]TLUSERCLIPPLANE.bActive, 0xff //Is this plane enabled?
    jz      not_enabled_plane

    // Register usage in this loop
    // xmm4 = x
    // xmm5 = y
    // xmm6 = z
    // xmm7 = w
    // mm0 = Current User Clip Plane Bits

#ifdef SSE_CLIPCODES
    // First Swizzle out the plane data. Yes, this is slow.
    movss   xmm0, [ebx]TLUSERCLIPPLANE.plane.x
    shufps  xmm0, xmm0, 0       //broadcast out
    movss   xmm1, [ebx]TLUSERCLIPPLANE.plane.y
    shufps  xmm1, xmm1, 0       //broadcast out
    movss   xmm2, [ebx]TLUSERCLIPPLANE.plane.z
    shufps  xmm2, xmm2, 0       //broadcast out
    movss   xmm3, [ebx]TLUSERCLIPPLANE.plane.w
    shufps  xmm3, xmm3, 0       //broadcast out

    mulps   xmm0, xmm4                      // xmm0 = lpClip->x * plane->x
    mulps   xmm1, xmm5                      // xmm1 = lpClip->y * plane->y
    mulps   xmm2, xmm6                      // xmm2 = lpClip->z * plane->z
    mulps   xmm3, xmm7                      // xmm3 = lpClip->w * plane->w
    addps   xmm0, xmm1                      // xmm0 = lpClip->x * plane->x + lpClip->y * plane->y
    xorps   xmm1, xmm1                      // xmm1 = 0.0
    addps   xmm0, xmm2                      // xmm0 = lpClip->x * plane->x + lpClip->y * plane->y + lpClip->z * plane->z 
    addps   xmm0, xmm3                      // xmm0 = lpClip->x * plane->x + lpClip->y * plane->y + lpClip->z * plane->z + lpClip->w * plane->w
    cmpltps xmm0, xmm1                      // < 0?
    movaps  [eax]_STL.fSOAuser_planeeq, xmm0 // Store out to temp storage
#else
pushad
lea eax, tmpMmxRegs
movq [eax+0x00], mm0
movq [eax+0x08], mm1
movq [eax+0x10], mm2
movq [eax+0x18], mm3
movq [eax+0x20], mm4
movq [eax+0x28], mm5
movq [eax+0x30], mm6
movq [eax+0x38], mm7
emms
	push	pRc
	call    ComputePlaneDataC
	add     esp, 4
lea eax, tmpMmxRegs
movq mm0, [eax+0x00]
//movq mm1, [eax+0x08]
//movq mm2, [eax+0x10]
//movq mm3, [eax+0x18]
//movq mm4, [eax+0x20]
movq mm5, [eax+0x28]
movq mm6, [eax+0x30]
//movq mm7, [eax+0x38]
popad
#endif
    movq    mm1, mm0                        // Restore user clip plane bits   
    movq    mm2, mm0
    pand    mm1, [eax]_STL.fSOAuser_planeeq.m.lo // < 0?
    pand    mm2, [eax]_STL.fSOAuser_planeeq.m.hi
    por     mm5, mm1
    por     mm6, mm2                        // or in clip_code



not_enabled_plane:
    pslld   mm0, 1                          // shift to next clip plane
    add     ebx, SIZE TLUSERCLIPPLANE       // next user clip plane
    dec     ecx
    jnz     ClipPlaneLoop

NoUserClipPlanes:

    movq    [eax]_STL.dSOAclip_code.m.lo, mm5   // save off clip flags
    movq    [eax]_STL.dSOAclip_code.m.hi, mm6
    emms    
  }
}
#endif //0

__declspec(align(32)) void ComputeClipCodes_SOA(RC *pRC)
{
//  #define TEST_CLIP

  __asm {
    mov     eax, pRC
    mov     ecx, [eax]RC.tl.dwTLState;
    mov     eax, [eax]RC.tl.pTL

    // Register usage throughout this code:
    // eax = pRc->tl.pTL
    // ebx = &pRc->tl.xfmUserClipPlanes
    // ecx = pRc->tl.dwTLState
    // mm5 = low part of outcode
    // mm6 = high part of outcode
    // mm7 = TL_clip_mask -- mask bits used for TL


    test    ecx, TLPV_GUARDBAND
    jz      Gen6BitClipCode

    // First, transform X and Y into guardband clip space for clip check

   //   D3DVALUE xnew = lpClip->x * pRc->tl.ViewData.fSOAgb11 +
   //           lpClip->w * pRc->tl.ViewData.fSOAgb41;
   //   D3DVALUE ynew = lpClip->y * pRc->tl.ViewData.fSOAgb22 +
   //           lpClip->w * pRc->tl.ViewData.fSOAgb42;

    movaps  xmm0, [eax]_STL.SOAhv.x         //xmm0 = hv.x
    movaps  xmm1, [eax]_STL.SOAhv.w         //xmm1 = hv.w
    movaps  xmm2, [eax]_STL.SOAhv.y         //xmm2 = hv.y
    movaps  xmm3, xmm1

    mulps   xmm0, [eax]_STL.fSOAgb11        //xmm0 = hv.x * gb11
    mulps   xmm1, [eax]_STL.fSOAgb41        //xmm1 = hv.w * gb41
    mulps   xmm2, [eax]_STL.fSOAgb22        //xmm2 = hv.y * gb22
    mulps   xmm3, [eax]_STL.fSOAgb42        //xmm3 = hv.w * gb42
    addps   xmm0, xmm1                      //xmm0 = hv.x * gb11 + hv.w * gb41
    addps   xmm2, xmm3                      //xmm2 = hv.y * gb22 + hv.w * gb42
    movaps  [eax]_STL.SOAgb_v.x, xmm0       //save off xnew and ynew
    movaps  [eax]_STL.SOAgb_v.y, xmm2

Gen6BitClipCode:
    // Generate the basic 6 bit outcode for X, Y, Z
    // The clip range is: 0 < x < w
    //                    0 < y < w
    //                    0 < z < w

    // if (x < 0)  clip_code |= RRCLIP_LEFTBIT;    
    // if (x >= we) clip_code |= RRCLIP_RIGHTBIT;  
    // if (y < 0)  clip_code |= RRCLIP_BOTTOMBIT;  
    // if (y >= we) clip_code |= RRCLIP_TOPBIT;    
    // if (z < 0)    clip_code |= RRCLIP_FRONTBIT; 
    // if (z >= we) clip_code |= RRCLIP_BACKBIT;   

    movq    mm7, TL_clip_mask                 //mm7 = 80000000 80000000

    // X LO
    movq    mm0, [eax]_STL.SOAhv.x.m.lo       //mm0 = x1       x0
    movq    mm3, [eax]_STL.SOAhv.w.m.lo       //mm3 = w1       w0

    movq    mm5, mm0
    movq    mm2, mm3                          //mm2 = w1      w0

    pand    mm2, mm7                          //mm2 = w < 0?
    movq    mm1, mm2                          //mm1 = w < 0?

    pand    mm0, mm7                          //mm0 = x < 0?
    pand    mm2, mm0                          //mm2 = w < 0? and x < 0?
    psrld   mm0, (32-TLCLIP_LEFTBIT)          //shift sign bits into clip_code bits

    pcmpgtd mm5, mm3                          //mm5 = x > w?
    pand    mm5, mm7                          //mm5 = sign bits set if x > w

    pxor    mm5, mm2                          //mm5 = x > w, even if both w&x < 0

    psrld   mm5, (32-TLCLIP_RIGHTBIT)         //shift sign bits into clip_code bits

    por     mm5, mm0                          //mm5 = clip code

    // Y LO
    movq    mm0, [eax]_STL.SOAhv.y.m.lo       //mm1 = y1       y0
    movq    mm4, mm0
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //mm1 = y < 0?
    pand    mm2, mm0                          //mm2 = w < 0? and y < 0?
    psrld   mm0, (32-TLCLIP_BOTTOMBIT)        //shift sign bits into clip_code bits

    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits set if y ? w

    pxor    mm4, mm2                          //mm4 = y > w, even if both w&y < 0

    psrld   mm4, (32-TLCLIP_TOPBIT)           //shift sign bits into clip_code bits
    por     mm5, mm4                          //mm5 = left | bottom clip_code

    por     mm5, mm0 

    // Z LO
    movq    mm0, [eax]_STL.SOAhv.z.m.lo       //mm2 = z1       z0
    movq    mm4, mm0                          //mm4 = z > w
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //shift sign bits into clip_code bits
    pand    mm2, mm0                          //mm2 = w < 0? and z < 0?
    psrld   mm0, (32-TLCLIP_FRONTBIT)

    pcmpgtd mm4, mm3                          //mm4 = sign bits set if y ? w
    pand    mm4, mm7                          //shift sign bits into clip_code bits

    pxor    mm4, mm2                          //mm4 = z > w, even if both w&z < 0

    psrld   mm4, (32-TLCLIP_BACKBIT)          
    por     mm5, mm4                    
                                              //mm1 = z < 0?
    por     mm5, mm0 
                                              //mm7 = 80000000 80000000

    // X HI
    movq    mm0, [eax]_STL.SOAhv.x.m.hi       //mm0 = x1       x0
    movq    mm3, [eax]_STL.SOAhv.w.m.hi       //mm3 = w1       w0

    movq    mm6, mm0                          
    movq    mm2, mm3                          //mm2 = w1      w0

    pand    mm2, mm7                          //mm2 = w < 0?
    movq    mm1, mm2                          //mm1 = w < 0?

    pand    mm0, mm7                          //mm0 = x < 0?
    pand    mm2, mm0                          //mm2 = w < 0? and z < 0?
    psrld   mm0, (32-TLCLIP_LEFTBIT)          //shift sign bits into clip_code bits

    pcmpgtd mm6, mm3                          //mm4 = x > w?
    pand    mm6, mm7                          //mm4 = sign bits set if x > w

    pxor    mm6, mm2                          //mm4 = x > w, even if both w&x < 0

    psrld   mm6, (32-TLCLIP_RIGHTBIT)         //shift sign bits into clip_code bits

    por     mm6, mm0                          //mm6 = clip_code

    // Y HI
    movq    mm0, [eax]_STL.SOAhv.y.m.hi       //mm1 = y1       y0
    movq    mm4, mm0
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //mm1 = y < 0?
    pand    mm2, mm0                          //mm2 = w < 0? and z < 0?
    psrld   mm0, (32-TLCLIP_BOTTOMBIT)        //shift sign bits into clip_code bits

    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits set if y > w

    pxor    mm4, mm2                          //mm4 = y > w, even if both w&y < 0

    psrld   mm4, (32-TLCLIP_TOPBIT)           //shift sign bits into clip_code bits
    por     mm6, mm4                          //mm6 = left | bottom clip_code

    por     mm6, mm0 

    // Z HI
    movq    mm0, [eax]_STL.SOAhv.z.m.hi       //mm2 = z1       z0
    movq    mm4, mm0                          //mm4 = z > w
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //shift sign bits into clip_code bits
    pand    mm2, mm0                          //mm2 = w < 0? and z < 0?
    psrld   mm0, (32-TLCLIP_FRONTBIT)

    pcmpgtd mm4, mm3                          //mm4 = sign bits set if z > w
    pand    mm4, mm7                          //shift sign bits into clip_code bits

    pxor    mm4, mm2                          //mm4 = z > w, even if both w&z < 0

    psrld   mm4, (32-TLCLIP_BACKBIT)
    por     mm6, mm4                    
                                              //mm1 = z < 0?
    por     mm6, mm0 

    test    ecx, TLPV_GUARDBAND
    jz      NoGuardBand

    // Guardband clipping
//GenGuardBandClipCode:
  //   D3DVALUE xx = lpClip->w - xnew;
  //   D3DVALUE yy = lpClip->w - ynew;
  //   clip_code |= ((AS_INT32(xnew) & 0x80000000) >> (32-TLCLIPGB_LEFTBIT))   |
  //                ((AS_INT32(ynew) & 0x80000000) >> (32-TLCLIPGB_BOTTOMBIT)) |
  //                ((AS_INT32(xx)   & 0x80000000) >> (32-TLCLIPGB_RIGHTBIT))  |
  //                ((AS_INT32(yy)   & 0x80000000) >> (32-TLCLIPGB_TOPBIT));

    movq    mm0, [eax]_STL.SOAgb_v.x.m.lo     //mm0 = x1       x0
    movq    mm3, [eax]_STL.SOAhv.w.m.lo       //mm3 = w1       w0
    movq    mm4, mm0                          //mm4 = x1       x0
    movq    mm2, mm3                          
    pand    mm2, mm7                          //mm2 = w < 0?
    movq    mm1, mm2

    pand    mm0, mm7                          //mm0 = x < 0
    pand    mm2, mm0                          //mm2 = w < 0 && x < 0?
    psrld   mm0, (32-TLCLIPGB_LEFTBIT)

    pcmpgtd mm4, mm3                          //mm4 = x > w
    pand    mm4, mm7                          //mm4 = sign bits of w-x

    pxor    mm4, mm2                          //flip if both w and x are < 0

    psrld   mm4, (32-TLCLIPGB_RIGHTBIT)
    por     mm5, mm4                          //mm5 = clip_code

    por     mm5, mm0 

    movq    mm0, [eax]_STL.SOAgb_v.y.m.lo     //mm1 = y1       y0
    movq    mm4, mm0
    movq    mm2, mm1                          //mm2 = w < 0?

    pand    mm0, mm7                          //mm0 = y < 0?
    pand    mm2, mm0                          //mm0 = w < 0 && y < 0?
    psrld   mm0, (32-TLCLIPGB_BOTTOMBIT)


    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits of w-y

    pxor    mm4, mm2                          //flip if both w and y are < 0

    psrld   mm4, (32-TLCLIPGB_TOPBIT)
    por     mm5, mm4                          //mm6 = or in GB Clip code bits

    por     mm5, mm0 

    movq    mm0, [eax]_STL.SOAgb_v.x.m.hi     //mm0 = x1       x0
    movq    mm3, [eax]_STL.SOAhv.w.m.hi       //mm3 = w1       w0
    movq    mm4, mm0
    movq    mm2, mm3
    pand    mm2, mm7                          //mm2 = w < 0?
    movq    mm1, mm2

    pand    mm0, mm7                          //mm0 = x < 0
    pand    mm2, mm0                          //mm2 = w < 0 && x < 0
    psrld   mm0, (32-TLCLIPGB_LEFTBIT)        //mm6 = clip_code

    pcmpgtd mm4, mm3                          //mm4 = x > w
    pand    mm4, mm7                          //mm4 = sign bits of w-x

    pxor    mm4, mm2                          //flip if both w and x are < 0

    psrld   mm4, (32-TLCLIPGB_RIGHTBIT)
    por     mm6, mm4                          //mm6 = clip_code

    por     mm6, mm0 

    movq    mm0, [eax]_STL.SOAgb_v.y.m.hi     //mm1 = y1       y0
    movq    mm4, mm0
    movq    mm2, mm1

    pand    mm0, mm7                          //mm0 = y < 0
    pand    mm2, mm0                          //mm2 = w < 0 && y < 0
    psrld   mm0, (32-TLCLIPGB_BOTTOMBIT)

    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits of w-y

    pxor    mm4, mm2                          //flip if both w and y are < 0

    psrld   mm4, (32-TLCLIPGB_TOPBIT)
    por     mm6, mm4                          //mm6 = left | bottom clip_code

    por     mm6, mm0 

NoGuardBand:
    test    ecx, TLPV_USERCLIPPLANES          // Check for User Clip Planes
    jz      NoUserClipPlanes    


    // User Clip Planes
    mov     ebx, pRC
    lea     ebx, [ebx]RC.tl.xfmUserClipPlanes
    mov     ecx, TLMAX_USER_CLIPPLANES

    movaps  xmm4, [eax]_STL.SOAhv.x           //xmm4 = hv.x
    movaps  xmm5, [eax]_STL.SOAhv.y           //xmm5 = hv.y
    movaps  xmm6, [eax]_STL.SOAhv.z           //xmm6 = hv.z
    movaps  xmm7, [eax]_STL.SOAhv.w           //xmm7 = hv.w
    movq    mm0, TL_userclip_mask             //mm0 = User clip plane bits

ClipPlaneLoop:
//   TLCLIPCODE clipBit = TLCLIP_USERCLIPPLANE0;
//   if (pRc->tl.dwTLState & TLPV_USERCLIPPLANES)
//       for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
//       {
//         if( pRc->tl.xfmUserClipPlanes[j].bActive )
//         {
//           TLVECTOR4 *plane = &pRc->tl.xfmUserClipPlanes[j].plane;
//           FLOAT fComp = 0.0f;
//           if( (lpClip->x*plane->x +
//              lpClip->y*plane->y +
//              lpClip->z*plane->z +
//              lpClip->w*plane->w) < fComp )
//           {
//             clip_code |= clipBit;
//           }
//         }
//         clipBit <<= 1;
//       }
    test    [ebx]TLUSERCLIPPLANE.bActive, 0xff //Is this plane enabled?
    jz      not_enabled_plane

    // Register usage in this loop
    // xmm4 = x
    // xmm5 = y
    // xmm6 = z
    // xmm7 = w
    // mm0 = Current User Clip Plane Bits

    // First Swizzle out the plane data. Yes, this is slow.
    movss   xmm0, [ebx]TLUSERCLIPPLANE.plane.x
    shufps  xmm0, xmm0, 0       //broadcast out
    movss   xmm1, [ebx]TLUSERCLIPPLANE.plane.y
    shufps  xmm1, xmm1, 0       //broadcast out
    movss   xmm2, [ebx]TLUSERCLIPPLANE.plane.z
    shufps  xmm2, xmm2, 0       //broadcast out
    movss   xmm3, [ebx]TLUSERCLIPPLANE.plane.w
    shufps  xmm3, xmm3, 0       //broadcast out

    mulps   xmm0, xmm4                      // xmm0 = lpClip->x * plane->x
    mulps   xmm1, xmm5                      // xmm1 = lpClip->y * plane->y
    mulps   xmm2, xmm6                      // xmm2 = lpClip->z * plane->z
    mulps   xmm3, xmm7                      // xmm3 = lpClip->w * plane->w
    addps   xmm0, xmm1                      // xmm0 = lpClip->x * plane->x + lpClip->y * plane->y
    xorps   xmm1, xmm1                      // xmm1 = 0.0
    addps   xmm0, xmm2                      // xmm0 = lpClip->x * plane->x + lpClip->y * plane->y + lpClip->z * plane->z 
    addps   xmm0, xmm3                      // xmm0 = lpClip->x * plane->x + lpClip->y * plane->y + lpClip->z * plane->z + lpClip->w * plane->w
    cmpltps xmm0, xmm1                      // < 0?
    movaps  [eax]_STL.fSOAuser_planeeq, xmm0 // Store out to temp storage
    movq    mm1, mm0                        // Restore user clip plane bits   
    movq    mm2, mm0
    pand    mm1, [eax]_STL.fSOAuser_planeeq.m.lo // < 0?
    pand    mm2, [eax]_STL.fSOAuser_planeeq.m.hi
    por     mm5, mm1
    por     mm6, mm2                        // or in clip_code



not_enabled_plane:
    pslld   mm0, 1                          // shift to next clip plane
    add     ebx, SIZE TLUSERCLIPPLANE       // next user clip plane
    dec     ecx
    jnz     ClipPlaneLoop

NoUserClipPlanes:

    movq    [eax]_STL.dSOAclip_code.m.lo, mm5   // save off clip flags
    movq    [eax]_STL.dSOAclip_code.m.hi, mm6
    emms    
  }

  #ifdef TEST_CLIP
  // Test the 1st 6 clip planes
  {
    int i, j;
    TL_SOATMP *pTL = pRC->tl.pTL;
    D3DVALUE xx, xx1;
    D3DVALUE yy, yy1;
    D3DVALUE zz;
    D3DVALUE xnew, ynew;
    TLCLIPCODE clip_code;
    TLCLIPCODE clipBit = TLCLIP_USERCLIPPLANE0;

    for (i = 0; i < 4; i++) 
    {
      xx = pTL->hv.w.f[i] - pTL->SOAhv.x.f[i];
      yy = pTL->hv.w.f[i] - pTL->SOAhv.y.f[i];
      zz = pTL->hv.w.f[i] - pTL->SOAhv.z.f[i];
      clip_code = ((AS_INT32(pTL->SOAhv.x.f[i])  & 0x80000000) >>  (32-TLCLIP_LEFTBIT))   |
                  ((AS_INT32(pTL->SOAhv.y.f[i])  & 0x80000000) >>  (32-TLCLIP_BOTTOMBIT)) |
                  ((AS_INT32(pTL->SOAhv.z.f[i])  & 0x80000000) >>  (32-TLCLIP_FRONTBIT))  |
                  ((AS_INT32(xx)                 & 0x80000000) >>  (32-TLCLIP_RIGHTBIT))  |
                  ((AS_INT32(yy)                 & 0x80000000) >>  (32-TLCLIP_TOPBIT))    |
                  ((AS_INT32(zz)                 & 0x80000000) >>  (32-TLCLIP_BACKBIT));

      clipBit = TLCLIP_USERCLIPPLANE0;
      for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
      {
        if( pRC->tl.xfmUserClipPlanes[j].bActive )
        {
          TLVECTOR4 *plane = &pRC->tl.xfmUserClipPlanes[j].plane;
          FLOAT fComp = 0.0f;
          if( (pTL->SOAhv.x.f[i]*plane->x +
               pTL->SOAhv.y.f[i]*plane->y +
               pTL->SOAhv.z.f[i]*plane->z +
               pTL->SOAhv.w.f[i]*plane->w) < fComp )
          {
            clip_code |= clipBit;
          }
        }
        clipBit <<= 1;
      }

      if (clip_code != 0)  {
        if (pRC->tl.dwTLState & TLPV_GUARDBAND)
        {
          // We do guardband check in the projection space, so
          // we transform X and Y of the vertex there
          xnew = pTL->SOAhv.x.f[i] * pRC->tl.ViewData.fSOAgb11 + pTL->SOAhv.w.f[i] * pRC->tl.ViewData.fSOAgb41;
          ynew = pTL->SOAhv.y.f[i] * pRC->tl.ViewData.fSOAgb22 + pTL->SOAhv.w.f[i] * pRC->tl.ViewData.fSOAgb42;
          xx1 = pTL->SOAhv.w.f[i] - xnew;
          yy1 = pTL->SOAhv.w.f[i] - ynew;
          clip_code |= ((AS_INT32(xnew) & 0x80000000) >> (32-TLCLIPGB_LEFTBIT))   |
                       ((AS_INT32(ynew) & 0x80000000) >> (32-TLCLIPGB_BOTTOMBIT)) |
                       ((AS_INT32(xx1)  & 0x80000000) >> (32-TLCLIPGB_RIGHTBIT))  |
                       ((AS_INT32(yy1)  & 0x80000000) >> (32-TLCLIPGB_TOPBIT));

          if (pTL->dSOAclip_code.d[i] != clip_code) 
          {
            __asm {int 3}
          }
        }
        
      }

      if (pTL->dSOAclip_code.d[i] != clip_code) 
      {
        __asm {int 3}
      }
    
    }
  }
  #endif //TEST_CLIP

}  



#if 0   // unused
// This version of ComputeClipCodes_SOA is faster, but returns different 
// results with -w than does the slow path ComputeClipCodes() function.
//
// The difference between the two versions of ComputeClipCodes_SOA is that 
// when w and x, y, or z are compared, this version has the same behavior
// as ComputeClipCodes() if both w and x, w and y, or w and z are negative.
// 
// The reason for this difference is that integer compare instructions are 
// used to generate the clip_code bits, instead of floating point instructions.
// This works when both values are positive, or when one is positive and the 
// other is negative. But when both negative, an integer compare gives the 
// reverse sense. So we fix that in this routine by checking for both values
// to be negative, and then inverting the result if they are.

/*-------------------------------------------------------------------
Function Name:  ComputeClipCodes_SOA
Description:    Compute Clip Codes for data in SOA format
                All possible cases for Clipping are covered by this
                routine, including user clip planes.
Parameters:   
                RC *pRC -- current rendering context
                pRc->tl.pTL->SOAhv -- vertex transformed to homogeneous 
                    coordinates
                pRc->tl.pTL->fSOAgb11 .. pRc->tl.pTL->fSOAgb41 -- SOA swizzled
                    versions of pRc->tl.ViewData.gb11...
Information:    
                pRc->tl.pTL->SOAgb_v and pRc->tl.pTL->user_plane_eq
                are temporary variables used in the calculation of the 
                clip_codes.

                Clip_code's are stored in pRc->tl.pTL->dSOAclip_code

                Execution times:
                6 bit code generation only: ~14 clocks / vertex
                Guard band bits:            ~22 clocks / vertex
                4 User clip planes          ~63 clocks / vertex
Return:         
-------------------------------------------------------------------*/
__declspec(align(32)) void ComputeClipCodes_SOA(RC *pRC)
{
//  #define TEST_CLIP

  __asm {
    mov     eax, pRC
    mov     ecx, [eax]RC.tl.dwTLState;
    mov     eax, [eax]RC.tl.pTL

    // Register usage throughout this code:
    // eax = pRc->tl.pTL
    // ebx = &pRc->tl.xfmUserClipPlanes
    // ecx = pRc->tl.dwTLState
    // mm5 = low part of outcode
    // mm6 = high part of outcode
    // mm7 = TL_clip_mask -- mask bits used for TL


    test    ecx, TLPV_GUARDBAND
    jz      Gen6BitClipCode

    // First, transform X and Y into guardband clip space for clip check

   //   D3DVALUE xnew = lpClip->x * pRc->tl.ViewData.gb11 +
   //           lpClip->w * pRc->tl.ViewData.gb41;
   //   D3DVALUE ynew = lpClip->y * pRc->tl.ViewData.gb22 +
   //           lpClip->w * pRc->tl.ViewData.gb42;

//    movaps  xmm0, [eax]_STL.SOAhv.x         //xmm0 = hv.x
//    movaps  xmm1, [eax]_STL.SOAhv.w         //xmm1 = hv.w
//    movaps  xmm2, [eax]_STL.SOAhv.y         //xmm2 = hv.y
//    movaps  xmm3, xmm1
    // xmm4-xmm7 holds the homogenous coordinates, no need to reload
	movaps	xmm0, xmm4						//xmm0 = hv.x
	movaps	xmm1, xmm7						//xmm1 = hv.w
	movaps	xmm2, xmm5						//xmm2 = hv.y
	movaps	xmm3, xmm7						//xmm3 = hv.w

    mulps   xmm0, [eax]_STL.fSOAgb11        //xmm0 = hv.x * gb11
    mulps   xmm1, [eax]_STL.fSOAgb41        //xmm1 = hv.w * gb41
    mulps   xmm2, [eax]_STL.fSOAgb22        //xmm2 = hv.y * gb22
    mulps   xmm3, [eax]_STL.fSOAgb42        //xmm3 = hv.w * gb42
    addps   xmm0, xmm1                      //xmm0 = hv.x * gb11 + hv.w * gb41
    addps   xmm2, xmm3                      //xmm2 = hv.y * gb22 + hv.w * gb42
    movaps  [eax]_STL.SOAgb_v.x, xmm0       //save off xnew and ynew
    movaps  [eax]_STL.SOAgb_v.y, xmm2

Gen6BitClipCode:
    // Generate the basic 6 bit outcode for X, Y, Z
    // The clip range is: 0 < x < w
    //                    0 < y < w
    //                    0 < z < w

    // if (x < 0)  clip_code |= RRCLIP_LEFTBIT;    
    // if (x >= we) clip_code |= RRCLIP_RIGHTBIT;  
    // if (y < 0)  clip_code |= RRCLIP_BOTTOMBIT;  
    // if (y >= we) clip_code |= RRCLIP_TOPBIT;    
    // if (z < 0)    clip_code |= RRCLIP_FRONTBIT; 
    // if (z >= we) clip_code |= RRCLIP_BACKBIT;   

    movq    mm7, TL_clip_mask                 //mm7 = 80000000 80000000

    movq    mm0, [eax]_STL.SOAhv.x.m.lo       //mm0 = x1       x0
    movq    mm1, [eax]_STL.SOAhv.y.m.lo       //mm1 = y1       y0
    movq    mm2, [eax]_STL.SOAhv.z.m.lo       //mm2 = z1       z0
    movq    mm3, [eax]_STL.SOAhv.w.m.lo       //mm3 = w1       w0

    movq    mm4, mm0
    pcmpgtd mm4, mm3                          //mm4 = x > w?
    pand    mm4, mm7                          //mm4 = sign bits set if x > w
    psrld   mm4, (32-TLCLIP_RIGHTBIT)         //shift sign bits into clip_code bits
    movq    mm5, mm4                          //mm5 = clip code

    pand    mm0, mm7                          //mm0 = x < 0?
    psrld   mm0, (32-TLCLIP_LEFTBIT)          //shift sign bits into clip_code bits
    por     mm5, mm0 

    movq    mm4, mm1
    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits set if y ? w
    psrld   mm4, (32-TLCLIP_TOPBIT)           //shift sign bits into clip_code bits
    por     mm5, mm4                          //mm5 = left | bottom clip_code

    pand    mm1, mm7                          //mm1 = y < 0?
    psrld   mm1, (32-TLCLIP_BOTTOMBIT)        //shift sign bits into clip_code bits
    por     mm5, mm1 

    movq    mm4, mm2                          //mm4 = z > w
    pcmpgtd mm4, mm3                          //mm4 = sign bits set if y ? w
    pand    mm4, mm7                          //shift sign bits into clip_code bits
    psrld   mm4, (32-TLCLIP_BACKBIT)          
    por     mm5, mm4                    
                                              //mm1 = z < 0?
    pand    mm2, mm7                          //shift sign bits into clip_code bits
    psrld   mm2, (32-TLCLIP_FRONTBIT)
    por     mm5, mm2 
                                              //mm7 = 80000000 80000000

    movq    mm0, [eax]_STL.SOAhv.x.m.hi       //mm0 = x1       x0
    movq    mm1, [eax]_STL.SOAhv.y.m.hi       //mm1 = y1       y0
    movq    mm2, [eax]_STL.SOAhv.z.m.hi       //mm2 = z1       z0
    movq    mm3, [eax]_STL.SOAhv.w.m.hi       //mm3 = w1       w0

    movq    mm4, mm0                          
    pcmpgtd mm4, mm3                          //mm4 = x > w?
    pand    mm4, mm7                          //mm4 = sign bits set if x > w
    psrld   mm4, (32-TLCLIP_RIGHTBIT)         //shift sign bits into clip_code bits
    movq    mm6, mm4                          //mm5 = clip_code

    pand    mm0, mm7                          //mm0 = x < 0?
    psrld   mm0, (32-TLCLIP_LEFTBIT)          //shift sign bits into clip_code bits
    por     mm6, mm0 

    movq    mm4, mm1
    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits set if y > w
    psrld   mm4, (32-TLCLIP_TOPBIT)           //shift sign bits into clip_code bits
    por     mm6, mm4                          //mm5 = left | bottom clip_code

    pand    mm1, mm7                          //mm1 = y < 0?
    psrld   mm1, (32-TLCLIP_BOTTOMBIT)        //shift sign bits into clip_code bits
    por     mm6, mm1 

    movq    mm4, mm2                          //mm4 = z > w
    pcmpgtd mm4, mm3                          //mm4 = sign bits set if z > w
    pand    mm4, mm7                          //shift sign bits into clip_code bits
    psrld   mm4, (32-TLCLIP_BACKBIT)
    por     mm6, mm4                    
                                              //mm1 = z < 0?
    pand    mm2, mm7                          //shift sign bits into clip_code bits
    psrld   mm2, (32-TLCLIP_FRONTBIT)
    por     mm6, mm2 

    test    ecx, TLPV_GUARDBAND
    jz      NoGuardBand

    // Guardband clipping
//GenGuardBandClipCode:
  //   D3DVALUE xx = lpClip->w - xnew;
  //   D3DVALUE yy = lpClip->w - ynew;
  //   clip_code |= ((AS_INT32(xnew) & 0x80000000) >> (32-TLCLIPGB_LEFTBIT))   |
  //                ((AS_INT32(ynew) & 0x80000000) >> (32-TLCLIPGB_BOTTOMBIT)) |
  //                ((AS_INT32(xx)   & 0x80000000) >> (32-TLCLIPGB_RIGHTBIT))  |
  //                ((AS_INT32(yy)   & 0x80000000) >> (32-TLCLIPGB_TOPBIT));

    movq    mm0, [eax]_STL.SOAgb_v.x.m.lo     //mm0 = x1       x0
    movq    mm1, [eax]_STL.SOAgb_v.y.m.lo     //mm1 = y1       y0
    movq    mm3, [eax]_STL.SOAhv.w.m.lo       //mm3 = w1       w0

    movq    mm4, mm0
    pcmpgtd mm4, mm3                          //mm4 = x > w
    pand    mm4, mm7                          //mm4 = sign bits of x
    psrld   mm4, (32-TLCLIPGB_RIGHTBIT)
    por     mm5, mm4                          //mm5 = clip_code

    pand    mm0, mm7                          //mm0 = x < 0
    psrld   mm0, (32-TLCLIPGB_LEFTBIT)
    por     mm5, mm0 

    movq    mm4, mm1
    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits of y
    psrld   mm4, (32-TLCLIPGB_TOPBIT)
    por     mm5, mm4                          //mm6 = or in GB Clip code bits

    pand    mm1, mm7
    psrld   mm1, (32-TLCLIPGB_BOTTOMBIT)
    por     mm5, mm1 

    movq    mm0, [eax]_STL.SOAgb_v.x.m.hi     //mm0 = x1       x0
    movq    mm1, [eax]_STL.SOAgb_v.y.m.hi     //mm1 = y1       y0
    movq    mm3, [eax]_STL.SOAhv.w.m.hi       //mm3 = w1       w0

    movq    mm4, mm0
    pcmpgtd mm4, mm3                          //mm4 = x > w
    pand    mm4, mm7                          //mm4 = sign bits of x
    psrld   mm4, (32-TLCLIPGB_RIGHTBIT)
    por     mm6, mm4                          //mm6 = clip_code

    pand    mm0, mm7                          //mm0 = x < 0
    psrld   mm0, (32-TLCLIPGB_LEFTBIT)        //mm6 = clip_code
    por     mm6, mm0 

    movq    mm4, mm1
    pcmpgtd mm4, mm3                          //mm4 = y > w
    pand    mm4, mm7                          //mm4 = sign bits of y
    psrld   mm4, (32-TLCLIPGB_TOPBIT)
    por     mm6, mm4                          //mm6 = left | bottom clip_code

    pand    mm1, mm7
    psrld   mm1, (32-TLCLIPGB_BOTTOMBIT)
    por     mm6, mm1 

NoGuardBand:
    test    ecx, TLPV_USERCLIPPLANES          // Check for User Clip Planes
    jz      NoUserClipPlanes    


    // User Clip Planes
    mov     ebx, pRC
    lea     ebx, [ebx]RC.tl.xfmUserClipPlanes
    mov     ecx, TLMAX_USER_CLIPPLANES

    movaps  xmm4, [eax]_STL.SOAhv.x           //xmm4 = hv.x
    movaps  xmm5, [eax]_STL.SOAhv.y           //xmm5 = hv.y
    movaps  xmm6, [eax]_STL.SOAhv.z           //xmm6 = hv.z
    movaps  xmm7, [eax]_STL.SOAhv.w           //xmm7 = hv.w
    movq    mm0, TL_userclip_mask             //mm0 = User clip plane bits

ClipPlaneLoop:
//   TLCLIPCODE clipBit = TLCLIP_USERCLIPPLANE0;
//   if (pRc->tl.dwTLState & TLPV_USERCLIPPLANES)
//       for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
//       {
//         if( pRc->tl.xfmUserClipPlanes[j].bActive )
//         {
//           TLVECTOR4 *plane = &pRc->tl.xfmUserClipPlanes[j].plane;
//           FLOAT fComp = 0.0f;
//           if( (lpClip->x*plane->x +
//              lpClip->y*plane->y +
//              lpClip->z*plane->z +
//              lpClip->w*plane->w) < fComp )
//           {
//             clip_code |= clipBit;
//           }
//         }
//         clipBit <<= 1;
//       }
    test    [ebx]TLUSERCLIPPLANES.bActive, 0xff //Is this plane enabled?
    jz      not_enabled_plane

    // Register usage in this loop
    // xmm4 = x
    // xmm5 = y
    // xmm6 = z
    // xmm7 = w
    // mm0 = Current User Clip Plane Bits

    // First Swizzle out the plane data. Yes, this is slow.
    movss   xmm0, [ebx]TLUSERCLIPPLANE.plane.x
    shufps  xmm0, xmm0, 0       //broadcast out
    movss   xmm1, [ebx]TLUSERCLIPPLANE.plane.y
    shufps  xmm1, xmm1, 0       //broadcast out
    movss   xmm2, [ebx]TLUSERCLIPPLANE.plane.z
    shufps  xmm2, xmm2, 0       //broadcast out
    movss   xmm3, [ebx]TLUSERCLIPPLANE.plane.w
    shufps  xmm3, xmm3, 0       //broadcast out

    mulps   xmm0, xmm4                      // xmm0 = lpClip->x * plane->x
    mulps   xmm1, xmm5                      // xmm1 = lpClip->y * plane->y
    mulps   xmm2, xmm6                      // xmm2 = lpClip->z * plane->z
    mulps   xmm3, xmm7                      // xmm3 = lpClip->w * plane->w
    addps   xmm0, xmm1                      // xmm0 = lpClip->x * plane->x + lpClip->y * plane->y
    xorps   xmm1, xmm1                      // xmm1 = 0.0
    addps   xmm0, xmm2                      // xmm0 = lpClip->x * plane->x + lpClip->y * plane->y + lpClip->z * plane->z 
    addps   xmm0, xmm3                      // xmm0 = lpClip->x * plane->x + lpClip->y * plane->y + lpClip->z * plane->z + lpClip->w * plane->w
    cmpltps xmm0, xmm1                      // < 0?
    movaps  [eax]_STL.fSOAuser_planeeq, xmm0 // Store out to temp storage
    movq    mm1, mm0                        // Restore user clip plane bits   
    movq    mm2, mm0
    pand    mm1, [eax]_STL.fSOAuser_planeeq.m.lo // < 0?
    pand    mm2, [eax]_STL.fSOAuser_planeeq.m.hi
    por     mm5, mm1
    por     mm6, mm2                        // or in clip_codes



not_enabled_plane:
    pslld   mm0, 1                          // shift to next clip plane
    add     ebx, SIZE TLUSERCLIPPLANE       // next user clip plane
    dec     ecx
    jnz     ClipPlaneLoop

NoUserClipPlanes:

    movq    [eax]_STL.dSOAclip_code.m.lo, mm5   // save off clip flags
    movq    [eax]_STL.dSOAclip_code.m.hi, mm6
    emms    
  }

  #ifdef TEST_CLIP
  // Test the 1st 6 clip planes
  // Note, this code will fail in this version, because it gives different 
  // results for -w. This checks for the clip_code generation to match that
  // of the slow path.
  {
    int i, j;
    TL_SOATMP *pTL = pRC->tl.pTL;
    D3DVALUE xx, xx1;
    D3DVALUE yy, yy1;
    D3DVALUE zz;
    D3DVALUE xnew, ynew;
    TLCLIPCODE clip_code;
    TLCLIPCODE clipBit = TLCLIP_USERCLIPPLANE0;

    for (i = 0; i < 4; i++) 
    {
      xx = pTL->SOAhv.w.f[i] - pTL->SOAhv.x.f[i];
      yy = pTL->SOAhv.w.f[i] - pTL->SOAhv.y.f[i];
      zz = pTL->SOAhv.w.f[i] - pTL->SOAhv.z.f[i];
      clip_code = ((AS_INT32(pTL->SOAhv.x.f[i])  & 0x80000000) >>  (32-TLCLIP_LEFTBIT))   |
                  ((AS_INT32(pTL->SOAhv.y.f[i])  & 0x80000000) >>  (32-TLCLIP_BOTTOMBIT)) |
                  ((AS_INT32(pTL->SOAhv.z.f[i])  & 0x80000000) >>  (32-TLCLIP_FRONTBIT))  |
                  ((AS_INT32(xx)                 & 0x80000000) >>  (32-TLCLIP_RIGHTBIT))  |
                  ((AS_INT32(yy)                 & 0x80000000) >>  (32-TLCLIP_TOPBIT))    |
                  ((AS_INT32(zz)                 & 0x80000000) >>  (32-TLCLIP_BACKBIT));

      clipBit = TLCLIP_USERCLIPPLANE0;
      for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
      {
        if( pRC->tl.xfmUserClipPlanes[j].bActive )
        {
          TLVECTOR4 *plane = &pRC->tl.xfmUserClipPlanes[j].plane;
          FLOAT fComp = 0.0f;
          if( (pTL->SOAhv.x.f[i]*plane->x +
               pTL->SOAhv.y.f[i]*plane->y +
               pTL->SOAhv.z.f[i]*plane->z +
               pTL->SOAhv.w.f[i]*plane->w) < fComp )
          {
            clip_code |= clipBit;
          }
        }
        clipBit <<= 1;
      }

      if (clip_code != 0)  {
        if (pRC->tl.dwTLState & TLPV_GUARDBAND)
        {
          // We do guardband check in the projection space, so
          // we transform X and Y of the vertex there
          xnew = pTL->SOAhv.x.f[i] * pRC->tl.ViewData.fSOAgb11 + pTL->SOAhv.w.f[i] * pRC->tl.ViewData.fSOAgb41;
          ynew = pTL->SOAhv.y.f[i] * pRC->tl.ViewData.fSOAgb22 + pTL->SOAhv.w.f[i] * pRC->tl.ViewData.fSOAgb42;
          xx1 = pTL->SOAhv.w.f[i] - xnew;
          yy1 = pTL->SOAhv.w.f[i] - ynew;
          clip_code |= ((AS_INT32(xnew) & 0x80000000) >> (32-TLCLIPGB_LEFTBIT))   |
                  ((AS_INT32(ynew) & 0x80000000) >> (32-TLCLIPGB_BOTTOMBIT)) |
                  ((AS_INT32(xx1)  & 0x80000000) >> (32-TLCLIPGB_RIGHTBIT))  |
                  ((AS_INT32(yy1)  & 0x80000000) >> (32-TLCLIPGB_TOPBIT));

          if (pTL->dSOAclip_code.d[i] != clip_code) 
          {
            __asm {int 3}
          }
        }
        
      }

      if (pTL->dSOAclip_code.d[i] != clip_code) 
      {
        __asm {int 3}
      }
    
    }
  }
  #endif //TEST_CLIP

}  
#endif //0 - unused


#endif //VCPP
#endif //TnL_HAL
#endif //DX7
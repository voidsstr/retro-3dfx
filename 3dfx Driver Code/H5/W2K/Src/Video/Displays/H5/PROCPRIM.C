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
** File name: procprim.c
**
** DescriptioMain body of ProcessPrimitive for T & L
**
** $Revision: 20$
** $Date: 10/11/00 8:45:12 PM$
**
** $Log: 
**  20   3dfx      1.18.2.0    10/11/00 Brent           Forced check in to enforce
**       branching.
**  19   Napalm Shared1.18        04/23/00 Matt McClure    Fixed compiler Error
**       when using SW_TNL and Z_ACCESS_OPT
**  18   Napalm Shared1.17        03/29/00 Bob Johnston    Created User Memory
**       Split T&L Path and added index pre calculations.
**  17   Napalm Shared1.16        03/23/00 Bob Johnston    Scott and Bob's changes
**       to split up the tranformation and lighting in the vertex processing loop
**       for improved VB primitive perfromance.
**  16   Napalm Shared1.15        03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  15   Napalm Shared1.14        03/16/00 Bob Johnston    Got Vertex Buffers
**       working correctly
**  14   Napalm Shared1.13        03/14/00 Bob Johnston    Changed User Mem
**       vertices to only use a min amount of memory in the SOAFVF_UM buff. Called
**       the setDX6State() from the FP branch in procprim.  Fixed
**       CanCreateExecBuff32 problem.  Decided to force VB creation to punt until I
**       fix all VB problems.
**  13   Napalm Shared1.12        03/14/00 Scott Kephart   Fixed numerous specular
**       alpha bugs
**  12   Napalm Shared1.11        03/06/00 Bob Johnston    Changes to make Vertex
**       Buffer handleing much more robust and accurate in terms of size.
**  11   Napalm Shared1.10        03/04/00 Bob Johnston    Increased max vertices
**       for VB and User Mem to 8K.  This fixes 3DMark2K's problems.  Also but a
**       fail over check to punt to the slow path  when the driver sees more than
**       8K of vertices inthe T&L HAL.  This number can be easily adjusted
**  10   Napalm Shared1.9         02/22/00 Bob Johnston    Fastpath rendering with
**       non-clipped vertices.  Texture coordinate handleing bug fixes, lots of bug
**       fixes.  Still showing some bugs in the fastpath.
**  9    Napalm Shared1.8         02/11/00 Bob Johnston    Fixed user mem vertex
**       pass/fail bug
**  8    Napalm Shared1.7         02/10/00 Bob Johnston    Added better clipping
**       support for the test code.  It will now use HAL assembly when okay.
**  7    Napalm Shared1.6         02/04/00 Bob Johnston    Changes to support User
**       Memory vertex buffers for the fastpath.  Also cleaned up the vertex
**       processing loops with neater macros for better readablity.
**  6    Napalm Shared1.5         01/28/00 Scott Kephart   Big T&L Merge: Carve out
**       place for fastpath, FVF changes
**  5    Napalm Shared1.4         12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  4    Napalm Shared1.3         11/22/99 Scott Kephart   More vertex buffer
**       support changes. Fix for hang in 3D Mark 2000.
**  3    Napalm Shared1.2         11/15/99 Scott Kephart   Added initial vertex
**       buffer support (more changes to follow)
**       Added support for Intel C Compiler for T&L code
**  2    Napalm Shared1.1         11/10/99 Scott Kephart   Added profiling changes
**       for T&L
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
** 
** 8     3/12/00 5:21p Skephart
** Fixed fogging problems, added more comments
** 
** 7     3/10/00 4:39p Skephart
** Add logic to select Diffuse + specular RGB + A
** 
** 6     3/08/00 9:31p Skephart
** Fixed bug in SetupFVF -- had TLBN.dwFVFFlags &= !TL_FVFFLAG_CLIPPED,
** instead of TLBN.dwFVFFlags &= ~TL_FVFFLAG_CLIPPED
** 
** 5     3/08/00 9:24p Skephart
 * 
 * 10    1/27/00 12:29p Skephart
 * 
 * 9     1/27/00 12:43a Skephart
 * New changes from Bob, Joe, Scott
 * 
 * 8     1/25/00 10:17p Skephart
 * BobJ Merge
 * 
 * 6   1/24/00 4:13p Skephart
 * Beginnings of the fast path
 * 
 * 5   1/20/00 2:25p Skephart
 * Update from BobJ
** 
** 2   1/17/00 9:55p Skephart
** Bob J's FVF_COMP changes
** 
** 1   11/12/99 4:27p Skephart
** 
** 31  10/26/99 12:48a Skephart
** Added #ifdef TnL_HAL
** 
** 30  10/21/99 12:04a Skephart
** Force specular and diffuse components to be stored in every vertex
** 
** 29  10/20/99 3:16p Skephart
** 
** 28  10/20/99 1:43p Skephart
** Added TLPV_VALIDCLIPBUFFER, re-org of FVF setup
** 
** 27  10/19/99 1:40p Skephart
** Fixes for Flare
** 
** 26  10/11/99 1:24p Skephart
** Bob's clipping changes.
** SK - Fixed bug in bend - specular was disabled when it should have been
** enabled in SetupFVFData
** 
** 24  10/06/99 10:27p Skephart
** Cleanup
** 
** 23  10/06/99 3:13p Skephart
** Cleanup
** 
** 22  10/01/99 4:26p Skephart
** Fog Table fixes
** 
** 21  9/28/99 3:13p Skephart
** Fixes from BobJ
** 
** 20  9/27/99 8:32p Skephart
** Move render context variables for TL HAL out of d3global.h, 
** and into tlglobal.h. All TL related variables are gathered under 
** pRc->tl.<variable>
** 
** 19  9/27/99 7:04p Skephart
** Save Primitive Data update from BobJ
** 
** 11  9/16/99 10:59a Skephart
** Clean up gpClipBuf
*/
 
#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL

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
#include "d7fvfext.h"

#ifdef SSECPP
#include "xmmintrin.h"  // intrinsics file
#endif

const D3DVALUE __HUGE_PWR2 = 1024.0f*1024.0f*2.0f;




/*
 ** SavePrimitiveData
 *
 *  FILENAME: C:\project\NAPALM\d3d\procprim.c
 *
 *  PARAMETERS: 
 *          DWORD dwFVFIn    -- Input FVF type
 *
 *  DESCRIPTION:
 *        Initializes important context information for the T&L HAL
 *        to use during ProcessPrimitive.  Called on NON indexed 
 *        Primitives.
 *
 *  RETURNS: Nothing
 *
 */

void SavePrimitiveData( RC *pRc, DWORD dwFVFIn, 
            LPVOID pVtx, DWORD dwVStart, UINT cVertices, 
            D3DPRIMITIVETYPE PrimType, LPVBSURFACEDATA lpVBSurfData )
{
  //
  // 1) Save the incoming information
  //
  pRc->tl.primType = PrimType;

  pRc->tl.InFVF.lpvData = pVtx;

  // Force some state changes if the FVF is different
  if( dwFVFIn != (DWORD) pRc->tl.InFVF.dwFVFType )
  {
    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  }

  pRc->tl.InFVF.dwFVFType = dwFVFIn;
  pRc->tl.dwNumVertices = cVertices;

  // No indices to work with
  pRc->tl.dwNumIndices = 0;
  pRc->tl.pIndices = NULL;
  pRc->tl.InFVF.dwVStart = dwVStart;
  pRc->tl.lpVBSurfData = lpVBSurfData;


} /* SavePrimitiveData */

/*
 ** SaveIdxPrimitiveData
 *
 *  FILENAME: C:\project\NAPALM\d3d\procprim.c
 *
 *  PARAMETERS: 
 *          DWORD dwFVFIn    -- Input FVF type
 *
 *  DESCRIPTION:
 *        Initializes important context information for the T&L HAL
 *        to use during ProcessPrimitive.  Called on Indexed Primitives.
 *
 *  RETURNS: Nothing
 *
 */

void SaveIdxPrimitiveData(    RC *pRc, DWORD dwFVFIn,
                LPVOID pVtx, DWORD dwVStart, UINT cVertices,
                D3DPRIMITIVETYPE PrimType,
                LPWORD pIndices, UINT cIndices, LPVBSURFACEDATA lpVBSurfData  )
 
{
  //
  // 1) Save the incoming information
  //
  pRc->tl.primType = PrimType;

  pRc->tl.InFVF.lpvData = pVtx;

  // Force some state changes if the FVF is different
  if( dwFVFIn != (DWORD) pRc->tl.InFVF.dwFVFType )
  {
    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  }

  pRc->tl.InFVF.dwFVFType = dwFVFIn;
  pRc->tl.dwNumVertices = cVertices - dwVStart;

  pRc->tl.dwNumIndices = cIndices;
  pRc->tl.pIndices = pIndices;
  pRc->tl.InFVF.dwVStart = dwVStart;
  pRc->tl.lpVBSurfData = lpVBSurfData;

  //D3DPRINT(0, ",%08lX,%08lX,%08lX,%08lX,%ld,%ld" , pVtx, dwFVFIn, dwVStart, pIndices, cVertices, cIndices );
} /* SaveIdxPrimitiveData */


/*
 ** MakeRRCOLOR
 *
 *  FILENAME: C:\project\NAPALM\d3d\procprim.c
 *
 *  PARAMETERS: 
 *          TLCOLOR *out   -- where float color result is to be stored
 *          DWORD inputColor  -- input 32 bit RGBA color
 *
 *  DESCRIPTION:
 *        Convert a color from integer RGBA to 4 floats.
 *
 *  RETURNS:
 *
 */


__inline void MakeTLCOLOR( TLCOLOR *out, DWORD inputColor )
{
  out->r = (D3DVALUE)RGBA_GETRED( inputColor );
  out->g = (D3DVALUE)RGBA_GETGREEN( inputColor );
  out->b = (D3DVALUE)RGBA_GETBLUE( inputColor );
}

/*
 ** ComputeClipCodes
 *
 *  FILENAME: C:\project\NAPALM\d3d\procprim.c
 *
 *  PARAMETERS:
 *
 *  DESCRIPTION:
 *
 *  RETURNS:
 *
 */

TLCLIPCODE
ComputeClipCodes(RC *pRc, TLCLIPCODE* pclipIntersection, TLCLIPCODE* pclipUnion, LPPRVCLIPPLANES lpClip)
{
  DWORD j;
  /* if true, need to deal with point size for clipping */
  D3DVALUE xx = lpClip->w - lpClip->x;
  D3DVALUE yy = lpClip->w - lpClip->y;
  D3DVALUE zz = lpClip->w - lpClip->z;

  /*  if (x < 0)  clip |= RRCLIP_LEFTBIT;    */
  /*  if (x >= we) clip |= RRCLIP_RIGHTBIT;  */
  /*  if (y < 0)  clip |= RRCLIP_BOTTOMBIT;  */
  /*  if (y >= we) clip |= RRCLIP_TOPBIT;    */
  /*  if (z < 0)    clip |= RRCLIP_FRONTBIT; */
  /*  if (z >= we) clip |= RRCLIP_BACKBIT;   */
  TLCLIPCODE clip = ((AS_INT32(lpClip->x)  & 0x80000000) >>  (32-TLCLIP_LEFTBIT))  |
       ((AS_INT32(lpClip->y)  & 0x80000000) >>  (32-TLCLIP_BOTTOMBIT))|
       ((AS_INT32(lpClip->z)  & 0x80000000) >>  (32-TLCLIP_FRONTBIT)) |
       ((AS_INT32(xx) & 0x80000000) >>  (32-TLCLIP_RIGHTBIT))  |
       ((AS_INT32(yy) & 0x80000000) >>  (32-TLCLIP_TOPBIT))    |
       ((AS_INT32(zz) & 0x80000000) >>  (32-TLCLIP_BACKBIT));

  TLCLIPCODE clipBit = TLCLIP_USERCLIPPLANE0;
  for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
  {
    if( pRc->tl.xfmUserClipPlanes[j].bActive )
    {
      TLVECTOR4 *plane = &pRc->tl.xfmUserClipPlanes[j].plane;
      FLOAT fComp = 0.0f;
      if( (lpClip->x*plane->x +
         lpClip->y*plane->y +
         lpClip->z*plane->z +
         lpClip->w*plane->w) < fComp )
      {
        clip |= clipBit;
      }
    }
    clipBit <<= 1;
  }

  if (clip == 0)
  {
    *pclipIntersection = 0;
    return clip;
  }
  else
  {
    if (pRc->tl.dwTLState & TLPV_GUARDBAND)
    {
      // We do guardband check in the projection space, so
      // we transform X and Y of the vertex there
      D3DVALUE xnew = lpClip->x * pRc->tl.ViewData.gb11 +
              lpClip->w * pRc->tl.ViewData.gb41;
      D3DVALUE ynew = lpClip->y * pRc->tl.ViewData.gb22 +
              lpClip->w * pRc->tl.ViewData.gb42;
      D3DVALUE xx = lpClip->w - xnew;
      D3DVALUE yy = lpClip->w - ynew;
      clip |= ((AS_INT32(xnew) & 0x80000000) >> (32-TLCLIPGB_LEFTBIT))   |
          ((AS_INT32(ynew) & 0x80000000) >> (32-TLCLIPGB_BOTTOMBIT)) |
          ((AS_INT32(xx)   & 0x80000000) >> (32-TLCLIPGB_RIGHTBIT))  |
          ((AS_INT32(yy)   & 0x80000000) >> (32-TLCLIPGB_TOPBIT));
    }
    else
    {
      *pclipIntersection &= clip;
    }
    *pclipUnion |= clip;
    return clip;
  }
}


///////////////////////////////////////////////////////////////////////////////
// Process primitives implementation:
// 1) Compute FVF info
// 2) Grow buffers to the requisite size
// 3) Initialize clipping state
// 4) Update T&L state
// 5) Transform, Light and compute clipping for vertices
// 6) Clip and Draw the primitives
//
///////////////////////////////////////////////////////////////////////////////

TLCLIPCODE ProcessVertices(RC *pRc)
{
  D3DVERTEX *pin  = (D3DVERTEX*)pRc->tl.InFVF.lpvData;
  DWORD   in_size = pRc->tl.InFVF.dwStride;
  DWORD   inFVF = (DWORD) pRc->tl.InFVF.dwFVFType;
  D3DTLVERTEX *pout  = (D3DTLVERTEX*)pRc->tl.TLFVF.lpvData;
  DWORD   out_size =  pRc->tl.TLFVF.dwStride;
  DWORD   outFVF = pRc->tl.TLFVF.dwFVFType;
  TLCLIPCODE *pclip = pRc->tl.pClipBuf;
  DWORD   flags = pRc->tl.dwTLState;
  TLCLIPCODE  clipIntersection = ~0;
  TLCLIPCODE  clipUnion = 0;
  DWORD   count = pRc->tl.dwNumVertices;
  D3DLIGHTINGELEMENT le;
  BOOL bVertexInEyeSpace = FALSE;
  DWORD i;
  int j;
  FLOAT fPointSize = 0.0f;
  DWORD *pOut;
#ifdef VERT_BUFF
  NT9XDEVICEDATA *ppdev = pRc->ppdev;
#endif
  //
  // Number of vertices to blend. i.e number of blend-matrices to
  // use is numVertexBlends+1.
  //
  int numVertexBlends = pRc->tl.numVertexBlends;


  pRc->tl.lighting.outDiffuse = TL_DEFAULT_DIFFUSE;
  pRc->tl.lighting.outSpecular = TL_DEFAULT_SPECULAR;

  //
  // The main transform loop
  //
  for (i = count; i; i--)
  {
    const D3DVECTOR *pNormal = (D3DVECTOR *)((LPBYTE)pin +
                         pRc->tl.InFVF.dwNormalOffset);

#ifdef SSECPP
    __declspec(align(16)) PRVCLIPPLANES Clip;  
#else
    PRVCLIPPLANES Clip;
#endif
    LPPRVCLIPPLANES lpClip = (LPPRVCLIPPLANES)&Clip;
    float inv_w_clip=0.0f;
    float *pBlendFactors = (float *)((LPBYTE)pin + sizeof( D3DVALUE )*3);

#ifdef  SSECPP // Only call if using the Intel Compiler
    LPBYTE  tmpMatrix;
    D3DVALUE *tmpOut;

     // Prefetch 2 vertices out
    _mm_prefetch((char*) pin + (in_size<<1), 1);
#endif  // SSECPP

    Clip.x = Clip.y = Clip.z = Clip.w = 0.0f;

    if(!numVertexBlends)
    {
      if (pRc->tl.dwTLState & TLPV_DOLIGHTING)
      {

      #if 0  // the VCPP compiler will not align locals for ASM, don't use for now.
      //#ifdef SSECPP
        __declspec(align(16)) __m128 m128temp1;
        __declspec(align(16)) __m128 m128temp2;
        __declspec(align(16)) __m128 m128temp3;

        tmpMatrix = (LPBYTE)pRc->tl.lpxfmToEye[0];
        tmpOut = &le.dvPosition.x;
        __asm 
        {
           mov    eax, pin
           movlps xmm0, [eax]   ;// pull source Vertex x & y
           mov    ecx, tmpOut
           mov    edx, tmpMatrix
           movaps xmm1, xmm0    ;// duplicate it.
           movss  xmm2, [eax+8] ;// pull in z
           movaps xmm3, [edx]   ;// Row 1
           shufps xmm0, xmm0, 0 ;// Broadcast X into xmm0
           shufps xmm1, xmm1, 055h ;// Broadcast Y into xmm1
           movaps xmm4, [edx+16]  ;// Row 2
           shufps xmm2, xmm2, 0 ;// Broadcast Z into xmm1
           mulps  xmm0, xmm3
           movaps xmm5, [edx+32]  ;// Row 3
           mulps  xmm1, xmm4
           mulps  xmm2, xmm5
           movaps xmm6, [edx+48]  ;// Row 4
           addps  xmm0, xmm1
           addps  xmm2, xmm6
           addps  xmm0, xmm2
           movlps [ecx], xmm0   ;//Store le.dvPosition.x and y
           shufps xmm0, xmm2, 0ah
           movss  [ecx+8], xmm0 ;//Store le.dvPosition.z
        }
      #else
        le.dvPosition.x = (pin->x*pRc->tl.lpxfmToEye[0]->_11 +
                  pin->y*pRc->tl.lpxfmToEye[0]->_21 +
                  pin->z*pRc->tl.lpxfmToEye[0]->_31 +
                  pRc->tl.lpxfmToEye[0]->_41);
        le.dvPosition.y = (pin->x*pRc->tl.lpxfmToEye[0]->_12 +
                  pin->y*pRc->tl.lpxfmToEye[0]->_22 +
                  pin->z*pRc->tl.lpxfmToEye[0]->_32 +
                  pRc->tl.lpxfmToEye[0]->_42);
        le.dvPosition.z = (pin->x*pRc->tl.lpxfmToEye[0]->_13 +
                  pin->y*pRc->tl.lpxfmToEye[0]->_23 +
                  pin->z*pRc->tl.lpxfmToEye[0]->_33 +
                  pRc->tl.lpxfmToEye[0]->_43);
      #endif

      #if 0 // The VCPP compiler won't align locals for ASM, don't call for now
      //#ifdef SSECPP
        tmpMatrix = (LPBYTE)pRc->tl.lpxfmToEyeInv[0];
        tmpOut = &le.dvNormal.x;
        __asm 
        {
           mov    eax, pNormal
           movlps xmm0, [eax] ;// pull source Vertex normal x,w,z in
           mov    ecx, tmpOut
           mov    edx, tmpMatrix
           movhps xmm0, [eax+8] ;// pull source Vertex normal x,w,z in
           movaps xmm1, [edx]   ;// Row 1
           mulps  xmm1, xmm0    ;// the X's
           movaps xmm2, [edx+16]  ;// Row 2
           mulps  xmm2, xmm0    ;// the Y's
           movaps xmm3, [edx+32]  ;// Row 3
           mulps  xmm3, xmm0    ;// the Z's
           movaps m128temp1, xmm1
           movaps m128temp2, xmm2
           movaps m128temp3, xmm3
           addss  xmm1, m128temp1+4
           addss  xmm1, m128temp1+8
           addss  xmm2, m128temp2+4
           addss  xmm2, m128temp2+8
           addss  xmm3, m128temp3+4
           addss  xmm3, m128temp3+8
           movss  [ecx], xmm1   ;//Store le.dvNormal.x and y
           movss  [ecx+4], xmm2 ;//Store le.dvNormal.x and y
           movss  [ecx+8], xmm3 ;//Store le.dvNormal.z
        }
      #else

        // Transform vertex normal to the eye space
        // We use inverse transposed matrix
        le.dvNormal.x = (pNormal->x*pRc->tl.lpxfmToEyeInv[0]->_11 +
                  pNormal->y*pRc->tl.lpxfmToEyeInv[0]->_12 +
                  pNormal->z*pRc->tl.lpxfmToEyeInv[0]->_13);
        le.dvNormal.y = (pNormal->x*pRc->tl.lpxfmToEyeInv[0]->_21 +
                  pNormal->y*pRc->tl.lpxfmToEyeInv[0]->_22 +
                  pNormal->z*pRc->tl.lpxfmToEyeInv[0]->_23);
        le.dvNormal.z = (pNormal->x*pRc->tl.lpxfmToEyeInv[0]->_31 +
                  pNormal->y*pRc->tl.lpxfmToEyeInv[0]->_32 +
                  pNormal->z*pRc->tl.lpxfmToEyeInv[0]->_33);
      #endif
      }

      // Apply WORLD
      #ifdef SSECPP
        tmpMatrix = (LPBYTE)pRc->tl.lpxfmCurrent[0];

        __asm 
        {
           mov    eax, pin
           movlps xmm0, [eax]   ;// pull source Vertex x & y
           mov    edx,  tmpMatrix
           mov    ecx,  lpClip
           movaps xmm1, xmm0    ;// duplicate it.
           movss  xmm2, [eax+8] ;// pull in z
           movaps xmm3, [edx]   ;// Row 1
           shufps xmm0, xmm0, 0 ;// Broadcast X into xmm0
           shufps xmm1, xmm1, 055h ;// Broadcast Y into xmm1
           movaps xmm4, [edx+16]  ;// Row 2
           shufps xmm2, xmm2, 0 ;// Broadcast Z into xmm1
           mulps  xmm0, xmm3
           movaps xmm5, [edx+32]  ;// Row 3
           mulps  xmm1, xmm4
           mulps  xmm2, xmm5
           movaps xmm6, [edx+48]  ;// Row 4
           addps  xmm0, xmm1
           addps  xmm2, xmm6
           addps  xmm0, xmm2
           movaps [ecx], xmm0   ;
        }
      #else

      Clip.x = (pin->x*pRc->tl.lpxfmCurrent[0]->_11 +
        pin->y*pRc->tl.lpxfmCurrent[0]->_21 +
        pin->z*pRc->tl.lpxfmCurrent[0]->_31 +
        pRc->tl.lpxfmCurrent[0]->_41);
      Clip.y = (pin->x*pRc->tl.lpxfmCurrent[0]->_12 +
        pin->y*pRc->tl.lpxfmCurrent[0]->_22 +
        pin->z*pRc->tl.lpxfmCurrent[0]->_32 +
        pRc->tl.lpxfmCurrent[0]->_42);
      Clip.z = (pin->x*pRc->tl.lpxfmCurrent[0]->_13 +
        pin->y*pRc->tl.lpxfmCurrent[0]->_23 +
        pin->z*pRc->tl.lpxfmCurrent[0]->_33 +
        pRc->tl.lpxfmCurrent[0]->_43);
      Clip.w = (pin->x*pRc->tl.lpxfmCurrent[0]->_14 +
        pin->y*pRc->tl.lpxfmCurrent[0]->_24 +
        pin->z*pRc->tl.lpxfmCurrent[0]->_34 +
        pRc->tl.lpxfmCurrent[0]->_44);
      #endif

    }
    else
    {

     float cumulBlend = 0; // Blend accumulated so far

       ZeroMemory( &le, sizeof(D3DLIGHTINGELEMENT) );

       for( j=0; j<=numVertexBlends; j++)
       {
         float blend;

         if( j == numVertexBlends )
           blend = 1.0f - cumulBlend;
         else
           blend = pBlendFactors[j];

         cumulBlend += pBlendFactors[j];

         if (pRc->tl.dwTLState & TLPV_DOLIGHTING)
         {
           le.dvPosition.x += (pin->x*pRc->tl.lpxfmToEye[j]->_11 +
                     pin->y*pRc->tl.lpxfmToEye[j]->_21 +
                     pin->z*pRc->tl.lpxfmToEye[j]->_31 +
                     pRc->tl.lpxfmToEye[j]->_41) * blend;
           le.dvPosition.y += (pin->x*pRc->tl.lpxfmToEye[j]->_12 +
                     pin->y*pRc->tl.lpxfmToEye[j]->_22 +
                     pin->z*pRc->tl.lpxfmToEye[j]->_32 +
                     pRc->tl.lpxfmToEye[j]->_42) * blend;
           le.dvPosition.z += (pin->x*pRc->tl.lpxfmToEye[j]->_13 +
                     pin->y*pRc->tl.lpxfmToEye[j]->_23 +
                     pin->z*pRc->tl.lpxfmToEye[j]->_33 +
                     pRc->tl.lpxfmToEye[j]->_43) * blend;
         }

         if (pRc->tl.dwTLState & TLPV_DOLIGHTING)
         {
           // Transform vertex normal to the eye space
           // We use inverse transposed matrix
           le.dvNormal.x += (pNormal->x*pRc->tl.lpxfmToEyeInv[j]->_11 +
                   pNormal->y*pRc->tl.lpxfmToEyeInv[j]->_12 +
                   pNormal->z*pRc->tl.lpxfmToEyeInv[j]->_13) * blend;
           le.dvNormal.y += (pNormal->x*pRc->tl.lpxfmToEyeInv[j]->_21 +
                   pNormal->y*pRc->tl.lpxfmToEyeInv[j]->_22 +
                   pNormal->z*pRc->tl.lpxfmToEyeInv[j]->_23) * blend;
           le.dvNormal.z += (pNormal->x*pRc->tl.lpxfmToEyeInv[j]->_31 +
                   pNormal->y*pRc->tl.lpxfmToEyeInv[j]->_32 +
                   pNormal->z*pRc->tl.lpxfmToEyeInv[j]->_33) * blend;
         }

         // Apply WORLDj
         Clip.x += (pin->x*pRc->tl.lpxfmCurrent[j]->_11 +
           pin->y*pRc->tl.lpxfmCurrent[j]->_21 +
           pin->z*pRc->tl.lpxfmCurrent[j]->_31 +
           pRc->tl.lpxfmCurrent[j]->_41) * blend;
         Clip.y += (pin->x*pRc->tl.lpxfmCurrent[j]->_12 +
           pin->y*pRc->tl.lpxfmCurrent[j]->_22 +
           pin->z*pRc->tl.lpxfmCurrent[j]->_32 +
           pRc->tl.lpxfmCurrent[j]->_42) * blend;
         Clip.z += (pin->x*pRc->tl.lpxfmCurrent[j]->_13 +
           pin->y*pRc->tl.lpxfmCurrent[j]->_23 +
           pin->z*pRc->tl.lpxfmCurrent[j]->_33 +
           pRc->tl.lpxfmCurrent[j]->_43) * blend;
         Clip.w += (pin->x*pRc->tl.lpxfmCurrent[j]->_14 +
           pin->y*pRc->tl.lpxfmCurrent[j]->_24 +
           pin->z*pRc->tl.lpxfmCurrent[j]->_34 +
           pRc->tl.lpxfmCurrent[j]->_44) * blend;
       }
    }

    //
    // Transform vertex to the clipping space, and position and normal
    // into eye space, if needed.
    //

    if ((flags & TLPV_NORMALIZENORMALS) && (pRc->tl.dwTLState & TLPV_DOLIGHTING))
      Normalize(&le.dvNormal);


    //
    // Compute clip codes if needed
    //
    if (pRc->tl.dwTLState & TLPV_DOCLIPPING)
    {
      TLCLIPCODE clip = ComputeClipCodes(pRc, &clipIntersection, &clipUnion, lpClip);
      if (clip == 0)
      {
        *pclip++ = 0;
        inv_w_clip = D3DVAL(1)/Clip.w;
      }
      else
      {
        if (pRc->tl.dwTLState & TLPV_GUARDBAND)
        {
          if ((clip & ~TLCLIP_INGUARDBAND) == 0)
          {
            // If vertex is inside the guardband we have to compute
            // screen coordinates
            inv_w_clip = D3DVAL(1)/Clip.w;
            *pclip++ = (TLCLIPCODE)clip;
            goto l_DoScreenCoord;
          }
        }
        *pclip++ = (TLCLIPCODE)clip;
        // If vertex is outside the frustum we can not compute screen
        // coordinates, hence store the clip coordinates
        pout->sx = Clip.x;
        pout->sy = Clip.y;
        pout->sz = Clip.z;
        pout->rhw = Clip.w;
        goto l_DoLighting;
      }
    }
    else
    {
      // We have to check this only for DONOTCLIP case, because otherwise
      // the vertex with "we = 0" will be clipped and screen coordinates
      // will not be computed
      // "clip" is not zero, if "we" is zero.
      if (!FLOAT_EQZ(Clip.w))
        inv_w_clip = D3DVAL(1)/Clip.w;
      else
        inv_w_clip = __HUGE_PWR2;
    }


l_DoScreenCoord:

    pout->sx = Clip.x * inv_w_clip * pRc->tl.ViewData.scaleX +
      pRc->tl.ViewData.offsetX;
    pout->sy = Clip.y * inv_w_clip * pRc->tl.ViewData.scaleY +
      pRc->tl.ViewData.offsetY;
    pout->sz = Clip.z * inv_w_clip * pRc->tl.ViewData.scaleZ +
      pRc->tl.ViewData.offsetZ;
    pout->rhw = inv_w_clip;

l_DoLighting:

    pOut = (DWORD*)((char*)pout + 4*sizeof(D3DVALUE));


    if (flags & TLPV_DOLIGHTING)
    {
      bVertexInEyeSpace = TRUE;

      //
      // If Diffuse color is needed, extract it for color vertex.
      //
      if (flags & TLPV_VERTEXDIFFUSENEEDED)
      {
        const DWORD color = *(DWORD*)((char*)pin + pRc->tl.InFVF.dwDiffuseOffset);
        MakeTLCOLOR((TLCOLOR *)&pRc->tl.lighting.vertexDiffuse, color);
        pRc->tl.lighting.vertexDiffAlpha = color & 0xff000000;
      }

      //
      // If Specular color is needed and provided
      // , extract it for color vertex.
      //
      if (flags & TLPV_VERTEXSPECULARNEEDED)
      {
        const DWORD color = *(DWORD*)((char*)pin + pRc->tl.InFVF.dwSpecularOffset);
        MakeTLCOLOR((TLCOLOR *)&pRc->tl.lighting.vertexSpecular, color);
        pRc->tl.lighting.vertexSpecAlpha = color & 0xff000000;
      }

      //
      // Light the vertex
      //
      LightVertex( pRc, &le );
    }
    else if (inFVF & (D3DFVF_DIFFUSE | D3DFVF_SPECULAR))
    {
      if (inFVF & D3DFVF_DIFFUSE)
        pRc->tl.lighting.outDiffuse = *(DWORD*)((char*)pin + pRc->tl.InFVF.dwDiffuseOffset);
      if (inFVF & D3DFVF_SPECULAR)
        pRc->tl.lighting.outSpecular = *(DWORD*)((char*)pin + pRc->tl.InFVF.dwSpecularOffset);
    }

    //
    // Compute Vertex Fog if needed
    //
    if (flags & TLPV_DOFOG)
    {
      FogVertex( pRc, (D3DVECTOR*)(pin), &le,  numVertexBlends,
             pBlendFactors, bVertexInEyeSpace );
    }

    if (outFVF & D3DFVF_DIFFUSE)
      *pOut++ = pRc->tl.lighting.outDiffuse;
    if (outFVF & D3DFVF_SPECULAR)
      *pOut++ = pRc->tl.lighting.outSpecular;;

    {
      memcpy(pOut, (char*)pin + pRc->tl.InFVF.dwTexOffset, pRc->tl.InFVF.dwTexCoordSize);
    }
    pin = (D3DVERTEX*) ((char*) pin + in_size);
    pout = (D3DTLVERTEX*) ((char*) pout + out_size);
  }


  if (flags & TLPV_DOCLIPPING)
  {
    pRc->tl.clipIntersection = clipIntersection;
    pRc->tl.clipUnion = clipUnion;
  }
  else
  {
    pRc->tl.clipIntersection = 0;
    pRc->tl.clipUnion = 0;
  }

  if (!pRc->tl.clipUnion) 
  {
    pRc->tl.dwTLState &= ~TLPV_VALIDCLIPBUFFER;
  }

  // Returns whether all the vertices were off screen
  return pRc->tl.clipIntersection;
}

//---------------------------------------------------------------------
// This function should be called every time FVF ID is changed
// All pv flags, input and output FVF id should be set before calling the
// function.
//---------------------------------------------------------------------
void UpdateComponentOffsets (DWORD dwFVFIn,
               LPDWORD pNormalOffset,
               LPDWORD pDiffOffset,
               LPDWORD pSpecOffset,
               LPDWORD pTexOffset)
{
  DWORD dwOffset = 0;

  switch( dwFVFIn & D3DFVF_POSITION_MASK )
  {
  case D3DFVF_XYZ:
    dwOffset = sizeof(D3DVECTOR);
    break;
  case D3DFVF_XYZB1:
    dwOffset = sizeof(D3DVECTOR) + sizeof(D3DVALUE);
    break;
  case D3DFVF_XYZB2:
    dwOffset = sizeof(D3DVECTOR) + 2*sizeof(D3DVALUE);
    break;
  case D3DFVF_XYZB3:
    dwOffset = sizeof(D3DVECTOR) + 3*sizeof(D3DVALUE);
    break;
  case D3DFVF_XYZB4:
    dwOffset = sizeof(D3DVECTOR) + 4*sizeof(D3DVALUE);
    break;
  case D3DFVF_XYZB5:
    dwOffset = sizeof(D3DVECTOR) + 5*sizeof(D3DVALUE);
    break;
  default:
    D3DPRINT(0, "Unable to compute offsets, strange FVF bits set");
  }

  *pNormalOffset = dwOffset;

  if (dwFVFIn & D3DFVF_NORMAL)
    dwOffset += sizeof(D3DVECTOR);
  if (dwFVFIn & D3DFVF_RESERVED1)
    dwOffset += sizeof(D3DVALUE);

  // Offset to the diffuse color
  *pDiffOffset = dwOffset;

  if (dwFVFIn & D3DFVF_DIFFUSE)
    dwOffset += sizeof(DWORD);

  // Offset to the specular color
  *pSpecOffset = dwOffset;

  if (dwFVFIn & D3DFVF_SPECULAR)
    dwOffset += sizeof(DWORD);

  // Offset to the texture data
  *pTexOffset = dwOffset;
} /* UpdateComponentOffsets  */


//---------------------------------------------------------------------
// SetupFVFData() : This function is called when passed FVF types 
//          cahnge.  It's sole purpose is to compute values
//          needed later in the FVF_COMP structure for both
//          the Slowpath and FastPath
//---------------------------------------------------------------------
void SetupFVFData(RC *pRc)
{
  // This function will set values for all four of the following FVF_COMP
  // structures:
  //    InFVF:  The passed untransformed and lit FVF from D3D to DP2 in the HAL.
  //    SOAFVF: The optimized and swizzled SOA untransformed and lit FVF that
  //        we create!
  //    LTFVF:  The D3DFVF compatible Output FVF that the slowpath rendering code 
  //        uses.
  //    LTBN: The Fastpath optimized AOS vertex format. Comes in two flavors, clipped
  //        and unclipped.  This format contains NO FVF info at all, but the 
  //        info required can be housed in the FVF_COMP struture anyway.  
  //        Just ignore all FVF related members of the FVF_COMP structure 
  //        for this pointer.


  // Some of the members of InFVF have already been set earlier in SaveXXXPrimitiveData
  // Now just update some of the informaiton to other FVF_COMP members.
  pRc->tl.SOAFVF.dwFVFType = pRc->tl.InFVF.dwFVFType;
  pRc->tl.SOAFVF.dwFVFFlags = TL_FVFFLAG_SOA;

  // FYI, SOAFVF.lpvData will be assinged later in ProcessVerticesFast() if
  // this primitive can be routed to the fast path.  Allocation has 
  // to occur this way since we had no Vertex info at the original vertex 
  // buffer creation time and we can't put it here since this code is only
  // run when a different FVF type is passed.

  //*****************************************************************
  // Compute number of texture coordinates for each FVF_COMP pointer
  //*****************************************************************
  pRc->tl.InFVF.dwNumTexCoords = FVF_TEXCOORD_NUMBER((DWORD)pRc->tl.InFVF.dwFVFType);
  pRc->tl.SOAFVF.dwNumTexCoords = pRc->tl.InFVF.dwNumTexCoords;
  pRc->tl.TLFVF.dwNumTexCoords = pRc->tl.InFVF.dwNumTexCoords;

#if 0
  // BobJ 2/22/00 -- No need to do this since 
  // texture stage sets can happen outside of FVF changes.
  // We treat everything as two textures anyway fro Napalm

  // The TLBN struct for Napalm holds 0, 1 or 2 textures
  // The Fastpath T&L will assign the correct coord values into
  // these the u0 v0 and u1 v1 arrays.
  if( pRc->state & STATE_REQUIRES_ST_TMU1)
  {
     pRc->tl.TLBN.dwNumTexCoords = 2;
  }
  else if ( pRc->state & STATE_REQUIRES_ST_TMU0 ) 
  {
     pRc->tl.TLBN.dwNumTexCoords = 1;
  }
  else
  {
     pRc->tl.TLBN.dwNumTexCoords = 0;
  }
#endif

  //************************************************************
  // Compute output FVF type for the Slowpath's TLFVF pointer
  //************************************************************
  // Amazing as it seems, we always want to output diffuse and specular components
  // for each vertex. The setup engine is *always* setup to accept at least a diffuse
  // vertex component.
  pRc->tl.TLFVF.dwFVFType = (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR);
  
  // Set up number of texture coordinates and copy texture formats
  pRc->tl.TLFVF.dwFVFType |= (DWORD)((pRc->tl.InFVF.dwNumTexCoords << D3DFVF_TEXCOUNT_SHIFT)  |
           ((DWORD)pRc->tl.InFVF.dwFVFType & 0xFFFF0000));

  //************************************************************
  // Compute size of texture coordinates for all FVF_COMP
  //************************************************************
  pRc->tl.dwTextureCoordSizeTotal = 0;
  ComputeTextureCoordSize((DWORD)pRc->tl.InFVF.dwFVFType, pRc->tl.dwTexCoordSize, 
               &(pRc->tl.dwTextureCoordSizeTotal));
  pRc->tl.InFVF.dwTexCoordSize = pRc->tl.dwTextureCoordSizeTotal; 
  pRc->tl.SOAFVF.dwTexCoordSize = pRc->tl.dwTextureCoordSizeTotal * SOA_SIZE; 
  pRc->tl.TLFVF.dwTexCoordSize = pRc->tl.dwTextureCoordSizeTotal; 
  pRc->tl.TLBN.dwTexCoordSize = (sizeof(AOS_ST) * NAPALM_MAX_TEXTURES);

    
  //  Compute output size
  pRc->tl.InFVF.dwStride = GetFVFVertexSize( pRc->tl.InFVF.dwFVFType );
  pRc->tl.TLFVF.dwStride  = GetFVFVertexSize( pRc->tl.TLFVF.dwFVFType );
  pRc->tl.SOAFVF.dwStride = pRc->tl.InFVF.dwStride * SOA_SIZE;  


  //************************************************************
  // Compute the fastpath TLBN vertex size depending on 
  // the current clip state.
  //************************************************************
  if (pRc->tl.dwTLState & TLPV_DOCLIPPING)
  {
     pRc->tl.TLBN.dwStride  = TLBN_CLIP_SIZE;
     pRc->tl.TLBN.dwFVFFlags  |= TL_FVFFLAG_CLIPPED;
  }
  else
  {
     pRc->tl.TLBN.dwStride  = TLBN_SIZE;
     pRc->tl.TLBN.dwFVFFlags  &= ~TL_FVFFLAG_CLIPPED;
  }

  //************************************************************
  // Update the component offsets for all Input FVF_COMP 
  // strutures
  //************************************************************
  // Now compute the InFVF dependent offsets used by the Geometry loop
  UpdateComponentOffsets ((DWORD)pRc->tl.InFVF.dwFVFType, &(pRc->tl.InFVF.dwNormalOffset),
              &(pRc->tl.InFVF.dwDiffuseOffset), &(pRc->tl.InFVF.dwSpecularOffset),
              &(pRc->tl.InFVF.dwTexOffset));

  // Update the SOAFVF component offsets
  pRc->tl.SOAFVF.dwNormalOffset = pRc->tl.InFVF.dwNormalOffset * SOA_SIZE;
  pRc->tl.SOAFVF.dwDiffuseOffset = pRc->tl.InFVF.dwDiffuseOffset * SOA_SIZE;
  pRc->tl.SOAFVF.dwSpecularOffset = pRc->tl.InFVF.dwSpecularOffset * SOA_SIZE;
  pRc->tl.SOAFVF.dwTexOffset = pRc->tl.InFVF.dwTexOffset * SOA_SIZE;

  //************************************************************
  // Clear our Dirty flags before leaving our update!
  //************************************************************
  pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_FVFOUT;
  pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_FVFIN;

  return;

} /* SetupFVFData */


HRESULT UpdateTLState(RC *pRc)
{
  HRESULT hr = D3D_OK;
  //
  // Update Geometry Loop flags based on the current state set
  //

  pRc->tl.dwTLState &= ~TLPV_FASTPATH_OK;

  // Need to compute the Min of what is in the FVF and the renderstate.
  pRc->tl.numVertexBlends = min( pRc->tl.VertexBlends, 
      (((DWORD)pRc->tl.InFVF.dwFVFType  & D3DFVF_POSITION_MASK) >> 1) - 2 );

  /*-------------------------------------------------------------**
  **  Fog or not:                                                **
  **  Compute fog if: 1) Fogging is enabled                      **
  **          2) VertexFog mode is not FOG_NONE                  **
  **          3) TableFog mode is FOG_NONE                       **
  **  If both table and vertex fog are not FOG_NONE, table fog   **
  **  is applied.                                                **
  **-------------------------------------------------------------*/

  //  if (pRc->useFog & SC_FOGCOLOR && !pRc->useFog & SC_FOGTABLE)
  if (pRc->useFogTable == FOGTABLE_DISABLED && pRc->useFog && pRc->tl.lighting.fog_mode)
  {
    pRc->tl.dwTLState |= TLPV_DOFOG;
  }
  else
  {
    pRc->tl.dwTLState &= ~TLPV_DOFOG;
  }

  /*   Something changed in the transformation state  */
  /*   Recompute digested transform state             */
  HR_RET(UpdateXformData(pRc));

  /*  Something changed in the lighting state */
  if ((pRc->tl.dwTLState & TLPV_DOLIGHTING) &&
      (pRc->tl.dwDirtyFlags & TLPV_DIRTY_LIGHTING))
  {
    /*                                                            */
    /*  Compute Colorvertex flags only if the lighting is enabled */
    /*                                                            */
    pRc->tl.dwTLState &= ~TLPV_COLORVERTEXFLAGS;
    pRc->tl.lighting.pAmbientSrc = &pRc->tl.lighting.matAmb;
    pRc->tl.lighting.pDiffuseSrc = &pRc->tl.lighting.matDiff;
    pRc->tl.lighting.pSpecularSrc = &pRc->tl.lighting.matSpec;
    pRc->tl.lighting.pEmissiveSrc = &pRc->tl.lighting.matEmis;
    pRc->tl.lighting.pDiffuseAlphaSrc = &pRc->tl.lighting.materialDiffAlpha;
    pRc->tl.lighting.pSpecularAlphaSrc = &pRc->tl.lighting.materialSpecAlpha;
    if (pRc->tl.dwTLState & TLPV_COLORVERTEXNEEDED)
    {
      switch( pRc->tl.AmbMaterialSrc )
      {
      case D3DMCS_MATERIAL:
        break;
      case D3DMCS_COLOR1:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_DIFFUSE)
          {
            pRc->tl.dwTLState |=
                (TLPV_VERTEXDIFFUSENEEDED | TLPV_COLORVERTEXAMB);
            pRc->tl.lighting.pAmbientSrc = &pRc->tl.lighting.vertexDiffuse;
          }
        }
        break;
      case D3DMCS_COLOR2:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR)
          {
            pRc->tl.dwTLState |=
                (TLPV_VERTEXSPECULARNEEDED | TLPV_COLORVERTEXAMB);
            pRc->tl.lighting.pAmbientSrc = &pRc->tl.lighting.vertexSpecular;
          }
        }
        break;
      }

      switch( pRc->tl.DfusMaterialSrc )
      {
      case D3DMCS_MATERIAL:
        break;
      case D3DMCS_COLOR1:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_DIFFUSE)
          {
            pRc->tl.dwTLState |=
                (TLPV_VERTEXDIFFUSENEEDED | TLPV_COLORVERTEXDIFF);
            pRc->tl.lighting.pDiffuseSrc = &pRc->tl.lighting.vertexDiffuse;
            pRc->tl.lighting.pDiffuseAlphaSrc =
                &pRc->tl.lighting.vertexDiffAlpha;
          }
        }
        break;
      case D3DMCS_COLOR2:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR)
          {
            pRc->tl.dwTLState |=
                (TLPV_VERTEXSPECULARNEEDED | TLPV_COLORVERTEXDIFF);
            pRc->tl.lighting.pDiffuseSrc = &pRc->tl.lighting.vertexSpecular;
            pRc->tl.lighting.pDiffuseAlphaSrc =
                &pRc->tl.lighting.vertexSpecAlpha;
          }
        }
        break;
      }

      switch( pRc->tl.SpecMaterialSrc )
      {
      case D3DMCS_MATERIAL:
        break;
      case D3DMCS_COLOR1:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_DIFFUSE)
          {
            pRc->tl.dwTLState |=
                (TLPV_VERTEXDIFFUSENEEDED | TLPV_COLORVERTEXSPEC);
            pRc->tl.lighting.pSpecularSrc = &pRc->tl.lighting.vertexDiffuse;
            pRc->tl.lighting.pSpecularAlphaSrc =
                &pRc->tl.lighting.vertexDiffAlpha;
          }
        }
        break;
      case D3DMCS_COLOR2:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR)
          {
            pRc->tl.dwTLState |=
                (TLPV_VERTEXSPECULARNEEDED | TLPV_COLORVERTEXSPEC);
            pRc->tl.lighting.pSpecularSrc = &pRc->tl.lighting.vertexSpecular;
            pRc->tl.lighting.pSpecularAlphaSrc =
                &pRc->tl.lighting.vertexSpecAlpha;
          }
        }
        break;
      }

      switch( pRc->tl.EmisMaterialSrc )
      {
      case D3DMCS_MATERIAL:
        break;
      case D3DMCS_COLOR1:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_DIFFUSE)
          {
            pRc->tl.dwTLState |=
                (TLPV_VERTEXDIFFUSENEEDED | TLPV_COLORVERTEXEMIS);
            pRc->tl.lighting.pEmissiveSrc = &pRc->tl.lighting.vertexDiffuse;
          }
        }
        break;
      case D3DMCS_COLOR2:
        {
          if (pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR)
          {
            pRc->tl.dwTLState |=
                (TLPV_VERTEXSPECULARNEEDED | TLPV_COLORVERTEXEMIS);
            pRc->tl.lighting.pEmissiveSrc = &pRc->tl.lighting.vertexSpecular;
          }
        }
        break;
      }
    }


    /*  If specular is needed in the output and has been provided */
    /*  in the input, force the copy of specular data             */
    if ((pRc->tl.InFVF.dwFVFType & D3DFVF_SPECULAR) && (pRc->specular == FALSE))
    {
      pRc->tl.dwTLState |= TLPV_VERTEXSPECULARNEEDED;
    }

    /*  Update the remaining light state */
    HR_RET(UpdateLightingData(pRc));
  }

  if ((pRc->tl.dwTLState & TLPV_DOFOG) &&
      (pRc->tl.dwDirtyFlags & TLPV_DIRTY_FOG))
  {
    HR_RET(UpdateFogData(pRc));
  }

  //
  // Compute Input and Output FVF and the size of output vertices
  //
  if ((pRc->tl.dwDirtyFlags & TLPV_DIRTY_FVFOUT) ||
      (pRc->tl.dwDirtyFlags & TLPV_DIRTY_FVFIN))
    SetupFVFData(pRc);

  if (pRc->tl.dwTLState & TLPV_DOCLIPPING)
  {
    /*  Figure out which pieces need to be interpolated in new vertices. */
    pRc->tl.clipping.dwInterpolate = 0;
    if (pRc->shadeMode == D3DSHADE_GOURAUD)
    {
      pRc->tl.clipping.dwInterpolate |= TLCLIP_INTERPOLATE_COLOR;
      if (pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR)
      {
        pRc->tl.clipping.dwInterpolate |= TLCLIP_INTERPOLATE_SPECULAR;
      }
    }
    if (pRc->fogEnable)
    {
      pRc->tl.clipping.dwInterpolate |= TLCLIP_INTERPOLATE_SPECULAR;
    }

    if (FVF_TEXCOORD_NUMBER(pRc->tl.InFVF.dwFVFType) != 0)
    {
      pRc->tl.clipping.dwInterpolate |= TLCLIP_INTERPOLATE_TEXTURE;
    }

    /*  Clear clip union and intersection flags */
    pRc->tl.clipIntersection = 0;
    pRc->tl.clipUnion = 0;

    if( pRc->tl.dwDirtyFlags & TLPV_DIRTY_CLIPPLANES )
    {
      HR_RET( UpdateClippingData( pRc, pRc->tl.clipPlaneEnable ));
    }
  }


  if (!(pRc->tl.dwTLState & pRc->tl.dwTLBadFlags))
    pRc->tl.dwTLState |= TLPV_FASTPATH_OK;

  pRc->tl.dwTLDevFlags = 0;
    pRc->tl.dwTLState &= ~(TLPV_DIFF_SRC_VTX | TLPV_SPEC_SRC_VTX) ;


  // ************************************************************************************************  
  // Figure out where Diffuse RGB , Specular RGB, Diffuse Alpha, and Specular Alpha come from

  // RGB colors and alpha for vertices are essentially a big MUX. The RGB color for diffuse and
  // specular can come from different places, depending on the render state and input vertex.
  //
  // DIFFUSE:
  // RGB - Source is one of the following:
  // 0              -     No diffuse
  // Input Vertex   -     Diffuse is given in vertex
  // Lighting       -     Diffuse is computed by lighting code
  //
  // Alpha - Source is one of the following:
  // 0              -     No diffuse
  // Input Vertex   -     Alpha is given in vertex
  // Material       -     Alpha is given in material
  // 
  //
  //
  // SPECULAR:
  // RGB - source is one of the following:
  // 0              -     No specular
  // Input vertex   -     Specular is given in vertex
  // Lighting       -     specular is computed
  //
  // Alpha - source is one of the following:
  // 0              -     No Alpha
  // Input vertex   -     alpha is given in vertex
  // Material       -     alpha is given in material
  // Fog            -     alpha is computed by fog routine
  //
  // When fog is enabled, there are some additional complications.
  // Napalm needs the fog factor given as a float when vertex fog is
  // enabled. The fog is available as a float when;
  // Alpha src = Material, Alpha Src = Fog. When Alpha Src = Input vertex,
  // we have a problem -- the input vertex gets it's fog from the integer
  // alpha portion of the specular RGBA color. So we have to convert this 
  // to a float.
   
#if defined(FASTPATH) && defined(SSECPP)
  // Update Vertex diffuse color and alpha info
  if (pRc->tl.dwTLState & TLPV_DOLIGHTING)  {
    // If lighting , copy diffuse from computed LIGHT
    pRc->tl.dwTLDevFlags |= TLPV_DEV_FTOI_DIFF_RGB | TLPV_DEV_COMBINE_DIFF;
    pRc->tl.lighting.pdSOADiffRGBSrc = &pRc->tl.pTL->dDiffuse;
    pRc->tl.lighting.pdSOADiffAlphaSrc = &pRc->tl.pTL->dMatDiffAlpha;
    
  }
  else  {
    pRc->tl.dwTLDevFlags |= TLPV_DEV_COPY_DIFF;
    if (pRc->tl.SOAFVF.dwFVFType & D3DFVF_DIFFUSE)  {
      // if not lighting, and input vtx has diffuse, copy that
      pRc->tl.dwTLState |= TLPV_DIFF_SRC_VTX;
      pRc->tl.lighting.pdSOADiffRGBSrc = (SOA_DWORD *) &TL_soa_0;   // initialize to something to insure 
      pRc->tl.lighting.pdSOADiffAlphaSrc = (SOA_DWORD *) &TL_soa_0; // a valid pointer - cheap insurance in case this is accessed incorrectly.
    }
    else  {
      // else copy in zeros
      pRc->tl.dwTLDevFlags |= TLPV_DEV_COPY_SPEC;
      pRc->tl.lighting.pdSOADiffRGBSrc = (SOA_DWORD *) &TL_soa_0;
      pRc->tl.lighting.pdSOADiffAlphaSrc = (SOA_DWORD *) &TL_soa_0;
    }
    
  }

  // Update Vertex specular color and alpha info
  if ((pRc->tl.dwTLState & TLPV_DOLIGHTING) && (pRc->tl.dwTLState & TLPV_DOSPECULAR))  {
    // If lighting & specular
    // specular color from computed light
    pRc->tl.dwTLDevFlags |= TLPV_DEV_FTOI_SPEC_RGB | TLPV_DEV_COMBINE_SPEC;
    pRc->tl.lighting.pdSOASpecRGBSrc = &pRc->tl.pTL->dSpecular;

    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
        pRc->tl.dwTLDevFlags |= TLPV_DEV_VERTEX_FOG;

    if (pRc->tl.dwTLState & TLPV_DOFOG)  {
        // Alpha from Fog
        pRc->tl.lighting.pdSOASpecAlphaSrc = &pRc->tl.pTL->dFog;
        pRc->tl.lighting.pfSOASpecAlphaSrc = &pRc->tl.pTL->fFog;
    }
    else  {
        // Alpha from Material
        pRc->tl.lighting.pdSOASpecAlphaSrc = &pRc->tl.pTL->dMatSpecAlpha;
        pRc->tl.lighting.pfSOASpecAlphaSrc = &pRc->tl.pTL->fMatSpecAlpha;
    }

  }
  else  {
    // Lighting, but specular not enabled!
    if (pRc->tl.SOAFVF.dwFVFType & D3DFVF_SPECULAR)  {
      // We have vertex specular
      pRc->tl.dwTLState |= TLPV_SPEC_SRC_VTX;
      // Color Source is from vertex
      pRc->tl.lighting.pdSOASpecRGBSrc = (SOA_DWORD *) &TL_soa_0;
      pRc->tl.lighting.pdSOASpecAlphaSrc = (SOA_DWORD *) &TL_soa_0;
      pRc->tl.lighting.pfSOASpecAlphaSrc = (SOA_FLOAT *) &TL_soa_0;

      if (pRc->state & STATE_REQUIRES_VERTEXFOG)
          pRc->tl.dwTLDevFlags |= TLPV_DEV_VERTEX_FOG;

      // Vertex fog computed?
      if (pRc->tl.dwTLState & TLPV_DOFOG)  {
        // Combine computed vertex fog and RGB from vertex
        pRc->tl.dwTLDevFlags |= TLPV_DEV_COMBINE_SPEC;
        pRc->tl.lighting.pdSOASpecAlphaSrc = &pRc->tl.pTL->dFog;
        pRc->tl.lighting.pfSOASpecAlphaSrc = &pRc->tl.pTL->fFog;
      }
      else  {
        // Specular, but no computed vertex fog.  Just copy the specular

        pRc->tl.dwTLDevFlags |= TLPV_DEV_COPY_SPEC;
        // But wait, what if vertex fog is enabled in the RENDER state?
        if (pRc->state & STATE_REQUIRES_VERTEXFOG)  {
          // We must convert specular alpha (passed in vtx) to float for fog!
          pRc->tl.dwTLDevFlags |= TLPV_DEV_ITOF_SPEC_ALPHA;
          pRc->tl.lighting.pfSOASpecAlphaSrc = &pRc->tl.pTL->fFog;
        }

        
      }

    }
    else  {
      // Lighting, no specular highlights either computed or passed in
      // Copy Zero's for specular!
      pRc->tl.lighting.pdSOASpecRGBSrc = (SOA_DWORD *) &TL_soa_0;
      pRc->tl.lighting.pdSOASpecAlphaSrc = (SOA_DWORD *) &TL_soa_0;
      pRc->tl.lighting.pfSOASpecAlphaSrc = (SOA_FLOAT *) &TL_soa_0;

      if (pRc->state & STATE_REQUIRES_VERTEXFOG)
          pRc->tl.dwTLDevFlags |= TLPV_DEV_VERTEX_FOG;

      if (pRc->tl.dwTLState & TLPV_DOFOG)  {
        // Combine computed vertex fog and RGB from vertex
        pRc->tl.dwTLDevFlags |= TLPV_DEV_COMBINE_SPEC;
        pRc->tl.lighting.pdSOASpecAlphaSrc = &pRc->tl.pTL->dFog;
        pRc->tl.lighting.pfSOASpecAlphaSrc = &pRc->tl.pTL->fFog;
      }
      else  {
        pRc->tl.dwTLDevFlags |= TLPV_DEV_COPY_SPEC;
      }
        
    }
    
  }
#endif    
  // ************************************************************************************************  




  return hr;
}


HRESULT ProcessPrimitive( RC *pRc )
{
  SETUP_PPDEV(pRc)
  HRESULT ret = D3D_OK;
  DWORD dwVertexPoolSize = 0;

  //
  // Update T&L state (must be before FVFData is set up)
  //

  // Update Lighting and related state and flags
  if ((ret = UpdateTLState(pRc)) != D3D_OK)
  return ret;  // There must be a serious problem!  Abort and skip over this Primitive!

  // Adjust the pointer to the input vertices if there is 
  // an offset passed in by DP2
  pRc->tl.InFVF.lpvData = (LPVOID)((LPBYTE)pRc->tl.InFVF.lpvData + 
               (pRc->tl.InFVF.dwVStart * pRc->tl.InFVF.dwStride));


#if defined(FASTPATH) && defined(VERT_BUFF) && defined(SSECPP)

  // This is the Big Check for the FASTPATH
  // If we don't have acccess to a valid Vertex Buffer,
  // we can't do squat!  We also need to qualify the VB too

  // First, determine if our lpVBSurfData pointer is valid
  if(pRc->tl.lpVBSurfData)
  {
    LPVBSURFACEDATA lpVBSD = pRc->tl.lpVBSurfData;

    // We can really only work with Vertex Data that's 
    // marked as write only.  Also, if the VB is 
    // animated or is being updated frequenmtly by the 
    // app, we should just let the SLOWPATH take it
    if((lpVBSD->dwSrcFlags & VBSURF_WRITEONLY) || 
      !(lpVBSD->dwSrcFlags & VBSURF_NOTQUALIFY))
    {

      // If this is the first time seeing this VB
      // Check it out to see if it is too big
      if(!(lpVBSD->pOptAllocAddr)) 
      {
        DWORD VertCnt = ((lpVBSD->dwSrcAllocSize - 319) / pRc->tl.InFVF.dwStride);
        if( VertCnt > TLMAXNUMVERTICES)
        {
          lpVBSD->dwSrcFlags |= VBSURF_NOTQUALIFY;
          pRc->tl.dwTLState &= ~TLPV_FASTPATH_OK;
        }
        else
        {
          D3DPRINT(32, "VB VertCount = %d", VertCnt);
          lpVBSD->dwNumVerts = VertCnt;
        }
      }

      // We need a more elegant way of handling this...
      // Check if is OK to use fast path 
      if ((pRc->tl.primType == D3DPT_TRIANGLELIST) && 
              pRc->tl.pIndices && (pRc->tl.dwTLState & TLPV_FASTPATH_OK))
      {
        if( HW_STATE_CHANGED )
           setDX6state( pRc, 0, 0);         // pRc, primitiveType, #primitives

        //return FP_IndexedTriangleList2_SOA(pRc);       // Does T & L together
        return FP_IndexedTriangleList2_SOA_Split(pRc);   // Splits up T and L
      }
    }
  }
  // Only support UM vertices if less than 2049 
  if ( pRc->tl.dwNumVertices <= TLMAXNUMVERTICES )
  {
     // We need a more elegant way of handling this...
     // Check if is OK to use fast path 
     if ((pRc->tl.primType == D3DPT_TRIANGLELIST) && 
             pRc->tl.pIndices && (pRc->tl.dwTLState & TLPV_FASTPATH_OK))
     {
       if( HW_STATE_CHANGED )
         setDX6state( pRc, 0, 0);        // pRc, primitiveType, #primitives

       //return FP_IndexedTriangleList2_SOA_UM(pRc);
       return FP_IndexedTriangleList2_SOA_UM_Split(pRc);
     }
  }

#endif

  // Set the ClipCode Buffer Valid flag 
  // if clipping is on
  if(pRc->tl.dwTLState & TLPV_DOCLIPPING)
    pRc->tl.dwTLState |= TLPV_VALIDCLIPBUFFER;
  else
    pRc->tl.dwTLState &= ~TLPV_VALIDCLIPBUFFER;
  
  //
  // Transform, Light and compute clipping for vertices
  //
  if (ProcessVertices(pRc))
  {
  // If the entire primitive lies outside the view frustum, quit
  // without drawing anything.  Tell DP2 that we need to skip over
  // this whole primitive
  ret = !D3D_OK;
  }
  return ret;
}

#endif
#endif


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
** File name: soalight.c
**
** Description: Lighting Code for the Pentium III / Structure of Array format Vertex Buffers
**
** $Revision: 33$
** $Date: 10/11/00 8:50:00 PM$
**
** $Log: 
**  33   3dfx      1.22.1.1.1.710/11/00 Brent           Forced check in to enforce
**       branching.
**  32   3dfx      1.22.1.1.1.609/23/00 Allen Hansen    went from 16 to 32 max
**       active lights so match Sage
**  31   3dfx      1.22.1.1.1.509/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  30   3dfx      1.22.1.1.1.409/03/00 Allen Hansen    was loading
**       pRc->tl.lighting.pActiveLightArray twice in the DS functions when we
**       didn't need to
**  29   3dfx      1.22.1.1.1.309/02/00 Allen Hansen    added SSE2 path to
**       converting lights from SOA_FLOATS to packed SOA_DWORDS
**  28   3dfx      1.22.1.1.1.208/29/00 Allen Hansen    moved float-to-int color
**       conversion to lighting functions (from soadev.c), cleaned up the lighting
**       functions
**  27   3dfx      1.22.1.1.1.108/21/00 Allen Hansen    added 8 light functions
**       (support 16 lights now)
**  26   3dfx      1.22.1.1.1.006/25/00 Allen Hansen    updated TL_SOATMP variable
**       name, moved FogVertexK3d to k3dlight.c, fixed bug in taylowpowSSE(); now
**       it saves & restores xmm registers it modifies
**  25   3dfx      1.22.1.1    06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  24   3dfx      1.22.1.0    05/10/00 Allen Hansen    Did a bunch of
**       optimizations to FogVertexSOA, didn't touch any of the lighting functions
**  23   Napalm    1.22        04/24/00 Scott Kephart   Fixed 3D Mark 2000 lighting
**       bug - "Stripes" in game 1
**  22   Napalm    1.21        04/19/00 Scott Kephart   Big lighting change - Part
**       I
**       Lighting is now split into two parts, diffuse and specular. 
**  21   Napalm    1.20        03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  20   Napalm    1.19        03/14/00 Scott Kephart   Fog fixes, got rid of
**       specular alpha in lighting
**  19   Napalm    1.18        03/08/00 Scott Kephart   Re-shuffle of T&L code.
**  18   Napalm    1.17        03/01/00 Allen Hansen    Fixed vertex fog for T&L
**  17   Napalm    1.16        02/28/00 Allen Hansen    Added vertex fog code (it's
**       rem'd out by a return right now)
**  16   Napalm    1.15        02/28/00 Scott Kephart   Lighting fixes. Fixed
**       specular on point lights, added support for dvFalloff on spotlights.
**  15   Napalm    1.14        02/25/00 Scott Kephart   Spotlights work! (As long
**       as dvFalloff == 1.0)
**  14   Napalm    1.13        02/25/00 Scott Kephart   Early out on dot product in
**       TLLV_PointSOA() -- This is a safer bet than it seems!
**  13   Napalm    1.12        02/25/00 Scott Kephart   Added emms to the end of
**       Xform_Light_4Vertex_SOA
**  12   Napalm    1.11        02/25/00 Scott Kephart   Added needed emms
**       instruction to LightVertexSOA
**  11   Napalm    1.10        02/23/00 Scott Kephart   Specular now works for
**       directional and point lights
**  10   Napalm    1.9         02/22/00 Scott Kephart   Minor cleanup
**  9    Napalm    1.8         02/16/00 Scott Kephart   Point lights work. No
**       specular yet.
**  8    Napalm    1.7         02/15/00 Scott Kephart   Added Diffuse Alpha
**  7    Napalm    1.6         02/15/00 Scott Kephart   Updates for SOA lighting
**  6    Napalm    1.5         02/10/00 Scott Kephart   Data structure cleanup for
**       SOA.H -- we're unionized now!
**  5    Napalm    1.4         02/08/00 Scott Kephart   Really simple device
**       coordinate transform
**  4    Napalm    1.3         02/08/00 Scott Kephart   Updated formatting
**  3    Napalm    1.2         02/08/00 Scott Kephart   Added SOA_ARGB_F2I
**  2    Napalm    1.1         02/01/00 Scott Kephart   More lighting changes.
**       Better SSE matrix multiply code.
**  1    Napalm    1.0         01/28/00 Scott Kephart   
** $
** 
** 2     3/12/00 5:21p Skephart
** Fixed FogVertexSOA to use dFog and fFog
** 
** 1     3/08/00 9:26p Skephart
 * 
 * 2     1/24/00 10:40p Skephart
 * Skeleton for SOA T&L
 * 
 * 1     1/19/00 9:49p Skephart
 * Beginnings of SOA T&L code
*/

#include "precomp.h"

#if( DX >= 7 )
#ifdef TnL_HAL

#ifdef VCPP

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

void LightVertexSOA_SSE1_0_D(RC* pRc);		void LightVertexSOA_SSE2_0_D(RC* pRc);
void LightVertexSOA_SSE1_1_D(RC* pRc);		void LightVertexSOA_SSE2_1_D(RC* pRc);
void LightVertexSOA_SSE1_2_D(RC* pRc);		void LightVertexSOA_SSE2_2_D(RC* pRc);
void LightVertexSOA_SSE1_3_D(RC* pRc);		void LightVertexSOA_SSE2_3_D(RC* pRc);
void LightVertexSOA_SSE1_4_D(RC* pRc);		void LightVertexSOA_SSE2_4_D(RC* pRc);
void LightVertexSOA_SSE1_5_D(RC* pRc);		void LightVertexSOA_SSE2_5_D(RC* pRc);
void LightVertexSOA_SSE1_6_D(RC* pRc);		void LightVertexSOA_SSE2_6_D(RC* pRc);
void LightVertexSOA_SSE1_7_D(RC* pRc);		void LightVertexSOA_SSE2_7_D(RC* pRc);
void LightVertexSOA_SSE1_8_D(RC* pRc);		void LightVertexSOA_SSE2_8_D(RC* pRc);
void LightVertexSOA_SSE1_9_D(RC* pRc);		void LightVertexSOA_SSE2_9_D(RC* pRc);
void LightVertexSOA_SSE1_10_D(RC* pRc);		void LightVertexSOA_SSE2_10_D(RC* pRc);
void LightVertexSOA_SSE1_11_D(RC* pRc);		void LightVertexSOA_SSE2_11_D(RC* pRc);
void LightVertexSOA_SSE1_12_D(RC* pRc);		void LightVertexSOA_SSE2_12_D(RC* pRc);
void LightVertexSOA_SSE1_13_D(RC* pRc);		void LightVertexSOA_SSE2_13_D(RC* pRc);
void LightVertexSOA_SSE1_14_D(RC* pRc);		void LightVertexSOA_SSE2_14_D(RC* pRc);
void LightVertexSOA_SSE1_15_D(RC* pRc);		void LightVertexSOA_SSE2_15_D(RC* pRc);
void LightVertexSOA_SSE1_16_D(RC* pRc);		void LightVertexSOA_SSE2_16_D(RC* pRc);

void LightVertexSOA_SSE1_0_DS(RC* pRc);		void LightVertexSOA_SSE2_0_DS(RC* pRc);
void LightVertexSOA_SSE1_1_DS(RC* pRc);		void LightVertexSOA_SSE2_1_DS(RC* pRc);
void LightVertexSOA_SSE1_2_DS(RC* pRc);		void LightVertexSOA_SSE2_2_DS(RC* pRc);
void LightVertexSOA_SSE1_3_DS(RC* pRc);		void LightVertexSOA_SSE2_3_DS(RC* pRc);
void LightVertexSOA_SSE1_4_DS(RC* pRc);		void LightVertexSOA_SSE2_4_DS(RC* pRc);
void LightVertexSOA_SSE1_5_DS(RC* pRc);		void LightVertexSOA_SSE2_5_DS(RC* pRc);
void LightVertexSOA_SSE1_6_DS(RC* pRc);		void LightVertexSOA_SSE2_6_DS(RC* pRc);
void LightVertexSOA_SSE1_7_DS(RC* pRc);		void LightVertexSOA_SSE2_7_DS(RC* pRc);
void LightVertexSOA_SSE1_8_DS(RC* pRc);		void LightVertexSOA_SSE2_8_DS(RC* pRc);
void LightVertexSOA_SSE1_9_DS(RC* pRc);		void LightVertexSOA_SSE2_9_DS(RC* pRc);
void LightVertexSOA_SSE1_10_DS(RC* pRc);	void LightVertexSOA_SSE2_10_DS(RC* pRc);
void LightVertexSOA_SSE1_11_DS(RC* pRc);	void LightVertexSOA_SSE2_11_DS(RC* pRc);
void LightVertexSOA_SSE1_12_DS(RC* pRc);	void LightVertexSOA_SSE2_12_DS(RC* pRc);
void LightVertexSOA_SSE1_13_DS(RC* pRc);	void LightVertexSOA_SSE2_13_DS(RC* pRc);
void LightVertexSOA_SSE1_14_DS(RC* pRc);	void LightVertexSOA_SSE2_14_DS(RC* pRc);
void LightVertexSOA_SSE1_15_DS(RC* pRc);	void LightVertexSOA_SSE2_15_DS(RC* pRc);
void LightVertexSOA_SSE1_16_DS(RC* pRc);	void LightVertexSOA_SSE2_16_DS(RC* pRc);

/************************************************************************************
* Load the the base diffuse color into xmm5-7.  The formula is:
*    ambient * ambient.src + emissive.src
* If the source for both is the material then this is 
* precalculated, so we can just load it.
************************************************************************************/
_inline void SOALightSetupDiffuseCV( RC* pRc )
{
  _asm 
  {
	mov		edx, [pRc]
	test	[edx]RC.tl.dwTLState, (TLPV_COLORVERTEXEMIS | TLPV_COLORVERTEXAMB)
	mov		eax, [edx]RC.tl.pTL
	jz		InitAmbientFromMaterial
	// diffuse base = ambient * ambient.Src + emissive.Src
	mov		ecx, [edx]RC.tl.lighting.pSOACvAmbientSrc
	mov		ebx, [edx]RC.tl.lighting.pSOACvEmissiveSrc
	movaps	xmm5, [eax]_STL.fSOAAmbientIn.red
	movaps	xmm6, [eax]_STL.fSOAAmbientIn.green
	movaps	xmm7, [eax]_STL.fSOAAmbientIn.blue
	mulps	xmm5, [ecx]SOA_RGB.red
	mulps	xmm6, [ecx]SOA_RGB.green
	mulps	xmm7, [ecx]SOA_RGB.blue
	addps	xmm5, [ebx]SOA_RGB.red
	addps	xmm6, [ebx]SOA_RGB.green
	addps	xmm7, [ebx]SOA_RGB.blue
	jmp		AmbientInitialized
  InitAmbientFromMaterial:
	// diffuse base = ambient * mat.ambient + mat.emissive (pre-calculated)
	movaps	xmm5, [eax]_STL.fSOAambEmiss.red
	movaps	xmm6, [eax]_STL.fSOAambEmiss.green
	movaps	xmm7, [eax]_STL.fSOAambEmiss.blue
  AmbientInitialized:
  }
}

_inline void SOALightSetupSpecular()
{
  _asm
  {
    xorps    xmm5, xmm5
    xorps    xmm6, xmm6
    xorps    xmm7, xmm7
  }
}

/************************************************************************************
* The colors are in xmm5-7 and are floats in SOA format.  
* This converts them into packed RGB values (bytes) and 
* stores them as 4-DWORDs at pDst.  Alpha is set to zero.
************************************************************************************/
_inline void ConvertSOAFloatToPackedRGB_SSE1( SOA_DWORD* pDst )
{
  _asm	// sse1 - 20 instructions
  {
	// xmm5=red, xmm6=green, xmm7=blue
	cvtps2pi 	mm2, xmm6       //mm2 = 00 00 00 G1 00 00 00 G0
	movhlps   	xmm6, xmm6
    cvtps2pi 	mm3, xmm6       //mm3 = 00 00 00 G3 00 00 00 G2

    cvtps2pi 	mm0, xmm7       //mm0 = 00 00 00 B1 00 00 00 B0
	movhlps   	xmm7, xmm7
    cvtps2pi 	mm1, xmm7       //mm1 = 00 00 00 B3 00 00 00 B2

    packuswb  	mm2, mm3        //mm2 = 00 G3 00 G2 00 G1 00 G0

    cvtps2pi 	mm6, xmm5       //mm6 = 00 00 00 R1 00 00 00 R0
	movhlps   	xmm5, xmm5
    cvtps2pi 	mm7, xmm5       //mm7 = 00 00 00 R3 00 00 00 R2
    psllq   	mm2, 8          //mm2 = G3 00 G2 00 G1 00 G0 00

    packuswb  	mm0, mm1        //mm0 = 00 B3 00 B2 00 B1 00 B0
    packuswb  	mm6, mm7        //mm6 = 00 R3 00 R2 00 R1 00 R0

    por     	mm0, mm2        //mm0 = G3 B3 G2 B2 G1 B1 G0 B0
    movq      	mm1, mm0        //mm1 = G3 B3 G2 B2 G1 B1 G0 B0

	mov			eax, [pDst]
    punpcklwd 	mm0, mm6        //mm0 = 00 R1 G1 B1 00 R0 G0 B0
    punpckhwd 	mm1, mm6        //mm1 = 00 R3 G3 B3 00 R2 G2 B2
    movq      	[eax]SOA_DWORD.m.lo, mm0
    movq      	[eax]SOA_DWORD.m.hi, mm1
  }
}

_inline void ConvertSOAFloatToPackedRGB_SSE2( SOA_DWORD* pDst )
{
  _asm	// sse2 - 12 instructions
  {
	// xmm5=red, xmm6=green, xmm7=blue
	minps		xmm5, [TL_255]	// clamp to +255.0f
	minps		xmm6, [TL_255]
	minps		xmm7, [TL_255]
	cvtps2dq	xmm5, xmm5		//xmm5 = 00 00 00 r3  00 00 00 r2  00 00 00 r1  00 00 00 r0
	cvtps2dq	xmm6, xmm6		//xmm6 = 00 00 00 g3  00 00 00 g2  00 00 00 g1  00 00 00 g0
	cvtps2dq	xmm7, xmm7		//xmm7 = 00 00 00 b3  00 00 00 b2  00 00 00 b1  00 00 00 b0
	mov			eax, [pDst]
	pslld		xmm5, 16		//xmm5 = 00 r3 00 00  00 r2 00 00  00 r1 00 00  00 r0 00 00
	pslld		xmm6, 8			//xmm6 = 00 00 g3 00  00 00 g2 00  00 00 g1 00  00 00 g0 00
	orps		xmm7, xmm5		//xmm7 = 00 r3 00 b3  00 r2 00 b2  00 r1 00 b1  00 r0 00 b0
	orps		xmm7, xmm6		//xmm7 = 00 r3 g3 b3  00 r2 g2 b2  00 r1 g1 b1  00 r0 g0 b0
	movdqa		[eax], xmm7
  }
}



/*-------------------------------------------------------------------
Function Name:  LightVertexSOA_SSEx_0_D
Description:    Apply no Light to a vertex
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA_SSE1_0_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;

  SOALightSetupDiffuseCV( pRc );
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_0_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;

  SOALightSetupDiffuseCV( pRc );
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA_SSEx_n_D
Description:    Apply n Light(s) to a vertex with Diffuse
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA_SSE1_1_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_1_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_2_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}

ALIGN32 void LightVertexSOA_SSE2_2_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}

ALIGN32 void LightVertexSOA_SSE1_3_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_3_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_4_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}

ALIGN32 void LightVertexSOA_SSE2_4_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}

ALIGN32 void LightVertexSOA_SSE1_5_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_5_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_6_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_6_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_7_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_7_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_8_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_8_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_9_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_9_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_10_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_10_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_11_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_11_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_12_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_12_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_13_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_13_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_14_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_14_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_15_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_15_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_16_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_16_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_17_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_17_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_18_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_18_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_19_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_19_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_20_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_20_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_21_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_21_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_22_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_22_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_23_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_23_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_24_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_24_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_25_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_25_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_26_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_26_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_27_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_27_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_28_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_28_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_29_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_29_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_30_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_30_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_31_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  (pLA[30]->pfnLightVertexSOA_Diff)(pRc, pLA[30]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_31_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  (pLA[30]->pfnLightVertexSOA_Diff)(pRc, pLA[30]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE1_32_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  (pLA[30]->pfnLightVertexSOA_Diff)(pRc, pLA[30]);
  (pLA[31]->pfnLightVertexSOA_Diff)(pRc, pLA[31]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );
}  

ALIGN32 void LightVertexSOA_SSE2_32_D(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  (pLA[30]->pfnLightVertexSOA_Diff)(pRc, pLA[30]);
  (pLA[31]->pfnLightVertexSOA_Diff)(pRc, pLA[31]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );
}  



/*-------------------------------------------------------------------
Function Name:  LightVertexSOA_SSEx_0_DS
Description:    Apply no Light to a vertex
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA_SSE1_0_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;

  SOALightSetupDiffuseCV( pRc );
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_0_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;

  SOALightSetupDiffuseCV( pRc );
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

/*-------------------------------------------------------------------
Function Name:  LightVertexSOA_SSEx_n_DS
Description:    Apply n Light(s) to a vertex with both Diffuse and Specular
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA_SSE1_1_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  //pLA = pRc->tl.lighting.pActiveLightArray;	// don't need to do this with 1 light since it never changed
  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_1_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_2_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}

ALIGN32 void LightVertexSOA_SSE2_2_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}

ALIGN32 void LightVertexSOA_SSE1_3_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_3_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_4_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_4_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_5_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_5_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_6_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_6_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_7_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_7_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_8_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_8_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_9_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_9_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_10_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_10_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_11_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_11_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_12_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_12_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_13_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_13_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_14_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_14_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_15_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_15_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_16_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_16_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_17_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_17_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_18_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_18_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_19_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_19_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_20_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_20_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_21_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_21_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_22_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_22_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_23_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_23_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_24_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_24_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_25_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_25_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_26_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_26_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_27_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_27_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_28_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_28_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_29_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  if (pLA[28]->dwNonZeroDot)   (pLA[28]->pfnLightVertexSOA_Spec)(pRc, pLA[28]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_29_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  if (pLA[28]->dwNonZeroDot)   (pLA[28]->pfnLightVertexSOA_Spec)(pRc, pLA[28]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_30_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  if (pLA[28]->dwNonZeroDot)   (pLA[28]->pfnLightVertexSOA_Spec)(pRc, pLA[28]);
  if (pLA[29]->dwNonZeroDot)   (pLA[29]->pfnLightVertexSOA_Spec)(pRc, pLA[29]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_30_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  if (pLA[28]->dwNonZeroDot)   (pLA[28]->pfnLightVertexSOA_Spec)(pRc, pLA[28]);
  if (pLA[29]->dwNonZeroDot)   (pLA[29]->pfnLightVertexSOA_Spec)(pRc, pLA[29]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_31_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  (pLA[30]->pfnLightVertexSOA_Diff)(pRc, pLA[30]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  if (pLA[28]->dwNonZeroDot)   (pLA[28]->pfnLightVertexSOA_Spec)(pRc, pLA[28]);
  if (pLA[29]->dwNonZeroDot)   (pLA[29]->pfnLightVertexSOA_Spec)(pRc, pLA[29]);
  if (pLA[30]->dwNonZeroDot)   (pLA[30]->pfnLightVertexSOA_Spec)(pRc, pLA[30]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_31_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  (pLA[30]->pfnLightVertexSOA_Diff)(pRc, pLA[30]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  if (pLA[28]->dwNonZeroDot)   (pLA[28]->pfnLightVertexSOA_Spec)(pRc, pLA[28]);
  if (pLA[29]->dwNonZeroDot)   (pLA[29]->pfnLightVertexSOA_Spec)(pRc, pLA[29]);
  if (pLA[30]->dwNonZeroDot)   (pLA[30]->pfnLightVertexSOA_Spec)(pRc, pLA[30]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE1_32_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  (pLA[30]->pfnLightVertexSOA_Diff)(pRc, pLA[30]);
  (pLA[31]->pfnLightVertexSOA_Diff)(pRc, pLA[31]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  if (pLA[28]->dwNonZeroDot)   (pLA[28]->pfnLightVertexSOA_Spec)(pRc, pLA[28]);
  if (pLA[29]->dwNonZeroDot)   (pLA[29]->pfnLightVertexSOA_Spec)(pRc, pLA[29]);
  if (pLA[30]->dwNonZeroDot)   (pLA[30]->pfnLightVertexSOA_Spec)(pRc, pLA[30]);
  if (pLA[31]->dwNonZeroDot)   (pLA[31]->pfnLightVertexSOA_Spec)(pRc, pLA[31]);
  ConvertSOAFloatToPackedRGB_SSE1( &pTLD->dSOASpecular );
}  

ALIGN32 void LightVertexSOA_SSE2_32_DS(RC* pRc)
{
  TL_SOATMP *pTLD = pRc->tl.pTL;
  TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  SOALightSetupDiffuseCV( pRc );
  (pLA[0]->pfnLightVertexSOA_Diff)(pRc, pLA[0]);
  (pLA[1]->pfnLightVertexSOA_Diff)(pRc, pLA[1]);
  (pLA[2]->pfnLightVertexSOA_Diff)(pRc, pLA[2]);
  (pLA[3]->pfnLightVertexSOA_Diff)(pRc, pLA[3]);
  (pLA[4]->pfnLightVertexSOA_Diff)(pRc, pLA[4]);
  (pLA[5]->pfnLightVertexSOA_Diff)(pRc, pLA[5]);
  (pLA[6]->pfnLightVertexSOA_Diff)(pRc, pLA[6]);
  (pLA[7]->pfnLightVertexSOA_Diff)(pRc, pLA[7]);
  (pLA[8]->pfnLightVertexSOA_Diff)(pRc, pLA[8]);
  (pLA[9]->pfnLightVertexSOA_Diff)(pRc, pLA[9]);
  (pLA[10]->pfnLightVertexSOA_Diff)(pRc, pLA[10]);
  (pLA[11]->pfnLightVertexSOA_Diff)(pRc, pLA[11]);
  (pLA[12]->pfnLightVertexSOA_Diff)(pRc, pLA[12]);
  (pLA[13]->pfnLightVertexSOA_Diff)(pRc, pLA[13]);
  (pLA[14]->pfnLightVertexSOA_Diff)(pRc, pLA[14]);
  (pLA[15]->pfnLightVertexSOA_Diff)(pRc, pLA[15]);
  (pLA[16]->pfnLightVertexSOA_Diff)(pRc, pLA[16]);
  (pLA[17]->pfnLightVertexSOA_Diff)(pRc, pLA[17]);
  (pLA[18]->pfnLightVertexSOA_Diff)(pRc, pLA[18]);
  (pLA[19]->pfnLightVertexSOA_Diff)(pRc, pLA[19]);
  (pLA[20]->pfnLightVertexSOA_Diff)(pRc, pLA[20]);
  (pLA[21]->pfnLightVertexSOA_Diff)(pRc, pLA[21]);
  (pLA[22]->pfnLightVertexSOA_Diff)(pRc, pLA[22]);
  (pLA[23]->pfnLightVertexSOA_Diff)(pRc, pLA[23]);
  (pLA[24]->pfnLightVertexSOA_Diff)(pRc, pLA[24]);
  (pLA[25]->pfnLightVertexSOA_Diff)(pRc, pLA[25]);
  (pLA[26]->pfnLightVertexSOA_Diff)(pRc, pLA[26]);
  (pLA[27]->pfnLightVertexSOA_Diff)(pRc, pLA[27]);
  (pLA[28]->pfnLightVertexSOA_Diff)(pRc, pLA[28]);
  (pLA[29]->pfnLightVertexSOA_Diff)(pRc, pLA[29]);
  (pLA[30]->pfnLightVertexSOA_Diff)(pRc, pLA[30]);
  (pLA[31]->pfnLightVertexSOA_Diff)(pRc, pLA[31]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOADiffuse );

  SOALightSetupSpecular();
  if (pLA[0]->dwNonZeroDot)    (pLA[0]->pfnLightVertexSOA_Spec)(pRc, pLA[0]);
  if (pLA[1]->dwNonZeroDot)    (pLA[1]->pfnLightVertexSOA_Spec)(pRc, pLA[1]);
  if (pLA[2]->dwNonZeroDot)    (pLA[2]->pfnLightVertexSOA_Spec)(pRc, pLA[2]);
  if (pLA[3]->dwNonZeroDot)    (pLA[3]->pfnLightVertexSOA_Spec)(pRc, pLA[3]);
  if (pLA[4]->dwNonZeroDot)    (pLA[4]->pfnLightVertexSOA_Spec)(pRc, pLA[4]);
  if (pLA[5]->dwNonZeroDot)    (pLA[5]->pfnLightVertexSOA_Spec)(pRc, pLA[5]);
  if (pLA[6]->dwNonZeroDot)    (pLA[6]->pfnLightVertexSOA_Spec)(pRc, pLA[6]);
  if (pLA[7]->dwNonZeroDot)    (pLA[7]->pfnLightVertexSOA_Spec)(pRc, pLA[7]);
  if (pLA[8]->dwNonZeroDot)    (pLA[8]->pfnLightVertexSOA_Spec)(pRc, pLA[8]);
  if (pLA[9]->dwNonZeroDot)    (pLA[9]->pfnLightVertexSOA_Spec)(pRc, pLA[9]);
  if (pLA[10]->dwNonZeroDot)   (pLA[10]->pfnLightVertexSOA_Spec)(pRc, pLA[10]);
  if (pLA[11]->dwNonZeroDot)   (pLA[11]->pfnLightVertexSOA_Spec)(pRc, pLA[11]);
  if (pLA[12]->dwNonZeroDot)   (pLA[12]->pfnLightVertexSOA_Spec)(pRc, pLA[12]);
  if (pLA[13]->dwNonZeroDot)   (pLA[13]->pfnLightVertexSOA_Spec)(pRc, pLA[13]);
  if (pLA[14]->dwNonZeroDot)   (pLA[14]->pfnLightVertexSOA_Spec)(pRc, pLA[14]);
  if (pLA[15]->dwNonZeroDot)   (pLA[15]->pfnLightVertexSOA_Spec)(pRc, pLA[15]);
  if (pLA[16]->dwNonZeroDot)   (pLA[16]->pfnLightVertexSOA_Spec)(pRc, pLA[16]);
  if (pLA[17]->dwNonZeroDot)   (pLA[17]->pfnLightVertexSOA_Spec)(pRc, pLA[17]);
  if (pLA[18]->dwNonZeroDot)   (pLA[18]->pfnLightVertexSOA_Spec)(pRc, pLA[18]);
  if (pLA[19]->dwNonZeroDot)   (pLA[19]->pfnLightVertexSOA_Spec)(pRc, pLA[19]);
  if (pLA[20]->dwNonZeroDot)   (pLA[20]->pfnLightVertexSOA_Spec)(pRc, pLA[20]);
  if (pLA[21]->dwNonZeroDot)   (pLA[21]->pfnLightVertexSOA_Spec)(pRc, pLA[21]);
  if (pLA[22]->dwNonZeroDot)   (pLA[22]->pfnLightVertexSOA_Spec)(pRc, pLA[22]);
  if (pLA[23]->dwNonZeroDot)   (pLA[23]->pfnLightVertexSOA_Spec)(pRc, pLA[23]);
  if (pLA[24]->dwNonZeroDot)   (pLA[24]->pfnLightVertexSOA_Spec)(pRc, pLA[24]);
  if (pLA[25]->dwNonZeroDot)   (pLA[25]->pfnLightVertexSOA_Spec)(pRc, pLA[25]);
  if (pLA[26]->dwNonZeroDot)   (pLA[26]->pfnLightVertexSOA_Spec)(pRc, pLA[26]);
  if (pLA[27]->dwNonZeroDot)   (pLA[27]->pfnLightVertexSOA_Spec)(pRc, pLA[27]);
  if (pLA[28]->dwNonZeroDot)   (pLA[28]->pfnLightVertexSOA_Spec)(pRc, pLA[28]);
  if (pLA[29]->dwNonZeroDot)   (pLA[29]->pfnLightVertexSOA_Spec)(pRc, pLA[29]);
  if (pLA[30]->dwNonZeroDot)   (pLA[30]->pfnLightVertexSOA_Spec)(pRc, pLA[30]);
  if (pLA[31]->dwNonZeroDot)   (pLA[31]->pfnLightVertexSOA_Spec)(pRc, pLA[31]);
  ConvertSOAFloatToPackedRGB_SSE2( &pTLD->dSOASpecular );
}  

/************************************************************************************
* Two arrays of pointers to these light functions
************************************************************************************/
TLLIGHTFN pLight_D[TLMAX_ACTIVE_LIGHTS+1][2] =  {
  { &LightVertexSOA_SSE1_0_D,  &LightVertexSOA_SSE2_0_D },
  { &LightVertexSOA_SSE1_1_D,  &LightVertexSOA_SSE2_1_D },
  { &LightVertexSOA_SSE1_2_D,  &LightVertexSOA_SSE2_2_D },
  { &LightVertexSOA_SSE1_3_D,  &LightVertexSOA_SSE2_3_D },
  { &LightVertexSOA_SSE1_4_D,  &LightVertexSOA_SSE2_4_D },
  { &LightVertexSOA_SSE1_5_D,  &LightVertexSOA_SSE2_5_D },
  { &LightVertexSOA_SSE1_6_D,  &LightVertexSOA_SSE2_6_D },
  { &LightVertexSOA_SSE1_7_D,  &LightVertexSOA_SSE2_7_D },
  { &LightVertexSOA_SSE1_8_D,  &LightVertexSOA_SSE2_8_D },
  { &LightVertexSOA_SSE1_9_D,  &LightVertexSOA_SSE2_9_D },
  { &LightVertexSOA_SSE1_10_D, &LightVertexSOA_SSE2_10_D },
  { &LightVertexSOA_SSE1_11_D, &LightVertexSOA_SSE2_11_D },
  { &LightVertexSOA_SSE1_12_D, &LightVertexSOA_SSE2_12_D },
  { &LightVertexSOA_SSE1_13_D, &LightVertexSOA_SSE2_13_D },
  { &LightVertexSOA_SSE1_14_D, &LightVertexSOA_SSE2_14_D },
  { &LightVertexSOA_SSE1_15_D, &LightVertexSOA_SSE2_15_D },
  { &LightVertexSOA_SSE1_16_D, &LightVertexSOA_SSE2_16_D },
  { &LightVertexSOA_SSE1_17_D, &LightVertexSOA_SSE2_17_D },
  { &LightVertexSOA_SSE1_18_D, &LightVertexSOA_SSE2_18_D },
  { &LightVertexSOA_SSE1_19_D, &LightVertexSOA_SSE2_19_D },
  { &LightVertexSOA_SSE1_20_D, &LightVertexSOA_SSE2_20_D },
  { &LightVertexSOA_SSE1_21_D, &LightVertexSOA_SSE2_21_D },
  { &LightVertexSOA_SSE1_22_D, &LightVertexSOA_SSE2_22_D },
  { &LightVertexSOA_SSE1_23_D, &LightVertexSOA_SSE2_23_D },
  { &LightVertexSOA_SSE1_24_D, &LightVertexSOA_SSE2_24_D },
  { &LightVertexSOA_SSE1_25_D, &LightVertexSOA_SSE2_25_D },
  { &LightVertexSOA_SSE1_26_D, &LightVertexSOA_SSE2_26_D },
  { &LightVertexSOA_SSE1_27_D, &LightVertexSOA_SSE2_27_D },
  { &LightVertexSOA_SSE1_28_D, &LightVertexSOA_SSE2_28_D },
  { &LightVertexSOA_SSE1_29_D, &LightVertexSOA_SSE2_29_D },
  { &LightVertexSOA_SSE1_30_D, &LightVertexSOA_SSE2_30_D },
  { &LightVertexSOA_SSE1_31_D, &LightVertexSOA_SSE2_31_D },
  { &LightVertexSOA_SSE1_32_D, &LightVertexSOA_SSE2_32_D },
};

TLLIGHTFN pLight_DS[TLMAX_ACTIVE_LIGHTS+1][2] =  {
  { &LightVertexSOA_SSE1_0_DS,  &LightVertexSOA_SSE2_0_DS },
  { &LightVertexSOA_SSE1_1_DS,  &LightVertexSOA_SSE2_1_DS },
  { &LightVertexSOA_SSE1_2_DS,  &LightVertexSOA_SSE2_2_DS },
  { &LightVertexSOA_SSE1_3_DS,  &LightVertexSOA_SSE2_3_DS },
  { &LightVertexSOA_SSE1_4_DS,  &LightVertexSOA_SSE2_4_DS },
  { &LightVertexSOA_SSE1_5_DS,  &LightVertexSOA_SSE2_5_DS },
  { &LightVertexSOA_SSE1_6_DS,  &LightVertexSOA_SSE2_6_DS },
  { &LightVertexSOA_SSE1_7_DS,  &LightVertexSOA_SSE2_7_DS },
  { &LightVertexSOA_SSE1_8_DS,  &LightVertexSOA_SSE2_8_DS },
  { &LightVertexSOA_SSE1_9_DS,  &LightVertexSOA_SSE2_9_DS },
  { &LightVertexSOA_SSE1_10_DS, &LightVertexSOA_SSE2_10_DS },
  { &LightVertexSOA_SSE1_11_DS, &LightVertexSOA_SSE2_11_DS },
  { &LightVertexSOA_SSE1_12_DS, &LightVertexSOA_SSE2_12_DS },
  { &LightVertexSOA_SSE1_13_DS, &LightVertexSOA_SSE2_13_DS },
  { &LightVertexSOA_SSE1_14_DS, &LightVertexSOA_SSE2_14_DS },
  { &LightVertexSOA_SSE1_15_DS, &LightVertexSOA_SSE2_15_DS },
  { &LightVertexSOA_SSE1_16_DS, &LightVertexSOA_SSE2_16_DS },
  { &LightVertexSOA_SSE1_17_DS, &LightVertexSOA_SSE2_17_DS },
  { &LightVertexSOA_SSE1_18_DS, &LightVertexSOA_SSE2_18_DS },
  { &LightVertexSOA_SSE1_19_DS, &LightVertexSOA_SSE2_19_DS },
  { &LightVertexSOA_SSE1_20_DS, &LightVertexSOA_SSE2_20_DS },
  { &LightVertexSOA_SSE1_21_DS, &LightVertexSOA_SSE2_21_DS },
  { &LightVertexSOA_SSE1_22_DS, &LightVertexSOA_SSE2_22_DS },
  { &LightVertexSOA_SSE1_23_DS, &LightVertexSOA_SSE2_23_DS },
  { &LightVertexSOA_SSE1_24_DS, &LightVertexSOA_SSE2_24_DS },
  { &LightVertexSOA_SSE1_25_DS, &LightVertexSOA_SSE2_25_DS },
  { &LightVertexSOA_SSE1_26_DS, &LightVertexSOA_SSE2_26_DS },
  { &LightVertexSOA_SSE1_27_DS, &LightVertexSOA_SSE2_27_DS },
  { &LightVertexSOA_SSE1_28_DS, &LightVertexSOA_SSE2_28_DS },
  { &LightVertexSOA_SSE1_29_DS, &LightVertexSOA_SSE2_29_DS },
  { &LightVertexSOA_SSE1_30_DS, &LightVertexSOA_SSE2_30_DS },
  { &LightVertexSOA_SSE1_31_DS, &LightVertexSOA_SSE2_31_DS },
  { &LightVertexSOA_SSE1_32_DS, &LightVertexSOA_SSE2_32_DS },
};


/*-------------------------------------------------------------------
Function Name:  LightVertexSOA
Description:    Apply all lights to a vertex
Parameters:     
                RC *pRC -- pointer to the rendering context
Information:    
                Currently a maximum of 8 lights are supported. This is
                hard-wired into the code to reduce the number of 
                expensive branch mispredictions in the code.

Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexSOA(RC* pRc)
{
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TLLIGHT  *pLight = Ldata->pActiveLights;
  TL_SOATMP *pTlTmp = (TL_SOATMP *)pRc->tl.pTL;


  // We'll need to add color vertex support here later

  // Initialize the diffuse color

  // Initialize the diffuse and specular color
  //pTLD->fSOASpecularOut = 0.0;
  //pTLD->fSOADiffuseOut = pTLD->fSOAambEmiss;
  __asm
  {
    mov       eax, pTlTmp           // point to TL_SOATMP
    movaps    xmm5, [eax]_STL.fSOAambEmiss.red
    movaps    xmm6, [eax]_STL.fSOAambEmiss.green
    movaps    xmm7, [eax]_STL.fSOAambEmiss.blue
    movaps    [eax]_STL.fSOADiffuseOut.red, xmm5
    movaps    [eax]_STL.fSOADiffuseOut.green, xmm6
    movaps    [eax]_STL.fSOADiffuseOut.blue, xmm7

    xorps     xmm0, xmm0
    movaps    [eax]_STL.fSOASpecularOut.red, xmm0
    movaps    [eax]_STL.fSOASpecularOut.green, xmm0
    movaps    [eax]_STL.fSOASpecularOut.blue, xmm0
  }

  //
  // In a loop accumulate color from the activated lights
  //

  // Light 0
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc, pLight);
    pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 1
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc, pLight);
    pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 2
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc, pLight);
    pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 3
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc, pLight);
    pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }


  // Light 4
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc, pLight);
    pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 5
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc, pLight);
    pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 6
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc, pLight);
    pLight = pLight->Next;
  }
  else
  {
    goto LightFinished;
  }

  // Light 7
  if (pLight)
  {
    if (pLight->dwFlags & TLLIGHT_READY)
      pLight->pfnLightVertexSOA(pRc, pLight);
  }

LightFinished:;


}


/*-------------------------------------------------------------------
Function Name:  FogVertexSOA
Description:    Performs fogging calculation on the input vertex
                Alpha component of pv->lighting.dwSpecular is set
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
#define RRPV_SET_ALPHA(color, a)   ((char*)&color)[3] = (unsigned char)(a);
#if 0
void FogVertexSOA_C(RC* pRc)
{
    int i;
	D3DVALUE fog;
    D3DVALUE dist = 0.0f;
    TL_SOATMP *pTlTmp = (TL_SOATMP *)pRc->tl.pTL;

    // THIS DOESN'T HANDLE VERTEX BLENDS!!!

    // Vertex is already transformed to the camera space
	for(i=0; i<SOA_SIZE; ++i)
	{

        if (pRc->tl.dwTLState & TLPV_RANGEFOG)
			dist = SQRTF(pTlTmp->SOAcv.x.f[i]*pTlTmp->SOAcv.x.f[i] + pTlTmp->SOAcv.y.f[i]*pTlTmp->SOAcv.y.f[i] + pTlTmp->SOAcv.z.f[i]*pTlTmp->SOAcv.z.f[i]);
        else
			dist = pTlTmp->SOAcv.z.f[i];

	    if (pRc->tl.lighting.fog_mode == D3DFOG_LINEAR)
	    {
			if (dist < pRc->tl.lighting.fog_start)
				fog = 255.0f;
			else if (dist >= pRc->tl.lighting.fog_end)
				fog = 0;
			else
				fog = (pRc->tl.lighting.fog_end - dist) * pRc->tl.lighting.fog_factor;
	    }
    	else
    	{
        	D3DVALUE tmp = dist * pRc->tl.lighting.fog_density;
	        if (pRc->tl.lighting.fog_mode == D3DFOG_EXP2)
    	        tmp *= tmp;
        	fog = (D3DVALUE)exp(-tmp) * 255.0f;
    	}

		pTlTmp->fSOAFog.f[i] = fog;
		pTlTmp->dSOAFog.d[i] = 0;					// lower 3 bytes must be cleared!
		RRPV_SET_ALPHA(pTlTmp->dSOAFog.d[i], fog);	// load fog into the upper byte

	}
}
#endif //0

//#define FOG_HIGH_PRECISION_RSQRT
#define CLAMP_FOG

#pragma warning( push )
#pragma warning(disable : 4799)  /* this shuts up the "no emms" warning */
void FogVertexSOA(RC* pRc)
{
//  TL_SOATMP *pTL = pRc->tl.pTL;
  //return;   // Disable this whole thing for now

  //
  // Calculate the distance
  //
  // Vertex is already transformed to the camera space
//  if (pRc->tl.dwTLState & TLPV_RANGEFOG) {
  _asm {
      mov     edx, pRc
      mov     eax, [edx]RC.tl.pTL                 // edx = pointer to pRc->tl
	  test	  [edx]RC.tl.dwTLState, TLPV_RANGEFOG
      movaps  xmm0, [eax]_STL.SOAcv.z
	  je	  FogDistanceDone
      movaps  xmm1, [eax]_STL.SOAcv.y
      movaps  xmm2, [eax]_STL.SOAcv.x
      mulps   xmm0, xmm0              // z*z
      mulps   xmm1, xmm1              // y*y
      mulps   xmm2, xmm2              // x*x
      addps   xmm0, xmm1              // y*y + z*z
      addps   xmm2, xmm0              // x*x + y*y + z*z = d
      rsqrtps   xmm1, xmm2      	  // 1/sqrt(d)
#ifdef FOG_HIGH_PRECISION_RSQRT
      // 0.5 * rsqrtps * (3 - x * rsqrtps(x) * rsqrtps(x))
      mulps   xmm2, xmm1              // d * 1/sqrt(d)
      movaps  xmm3, [TL_soa_0pt5]     // 0.5f
      mulps   xmm2, xmm1              // d * 1/sqrt(d) * 1/sqrt(d)
      mulps   xmm3, xmm1              // 0.5f * 1/sqrt(d)
      addps   xmm2, [TL_soa_neg_3]    // 3 - d * 1/sqrt(d) * 1/sqrt(d)
      mulps   xmm3, xmm2              // 0.5f * 1/sqrt(d) * (3 - d * 1/sqrt(d) * 1/sqrt(d))
#endif
      rcpps   xmm0, xmm1              // sqrt(d)
FogDistanceDone:


  /*
  if (pRc->tl.lighting.fog_mode == D3DFOG_LINEAR) {
  if (dist < pRc->tl.lighting.fog_start)      RRPV_SET_ALPHA(pRc->tl.lighting.dwSpecular, 255);
  else if (dist >= pRc->tl.lighting.fog_end)  RRPV_SET_ALPHA(pRc->tl.lighting.dwSpecular, 0);
  else  RRPV_SET_ALPHA(pRc->tl.lighting.dwSpecular, (int)((pRc->tl.lighting.fog_end - dist) * pRc->tl.lighting.fog_factor));
  } else {
  D3DVALUE tmp = dist * pRc->tl.lighting.fog_density;
  if (pRc->tl.lighting.fog_mode == D3DFOG_EXP2)
  tmp *= tmp;
  RRPV_SET_ALPHA( pRc->tl.lighting.dwSpecular, (int) (exp(-tmp) * 255.0f) )
  }
  */
	  cmp     [edx]RC.tl.lighting.fog_mode, D3DFOG_LINEAR
	  jne	  NonLinearFog
      movss   xmm1, [edx]RC.tl.lighting.fog_start
      movss   xmm2, [edx]RC.tl.lighting.fog_end
      movss   xmm3, [edx]RC.tl.lighting.fog_factor
      shufps  xmm1, xmm1, 0                       // broadcast start
      shufps  xmm2, xmm2, 0                       // broadcast end
      shufps  xmm3, xmm3, 0                       // broadcast fog_factor
      movaps  xmm4, xmm1
      movaps  xmm5, xmm2

      // clear the fog_factor if it's out of range
      cmpltps xmm1, xmm0                          // (dist < start) ? 0 : -1
      cmpnltps xmm2, xmm0                         // (dist >= end ) ? 0 : -1
      andps   xmm3, xmm1                          // (dist < start) ? 0 : fog_factor
      andps   xmm3, xmm2                          // ((dist < start) || (dist >= end)) ? 0 : fog_factor  

      // if fog is in range, fog = (end - dist) * fog_factor (else this result will be zero)
      subps   xmm5, xmm0                          // end - dist
      mulps   xmm5, xmm3                          // (end - dist) * (((dist < start) || (dist >= end)) ? 0 : fog_factor)

      // if (dist < start) fog = 255
      cmpltps xmm0, xmm4                          // (dist < start) ? -1 : 0
      andps   xmm0, [TL_soa_255]                  // (dist < start) ? 255 : 0
      orps    xmm0, xmm5                          // (dist < start) ? 255 : (end - dist) * ((dist >= end) ? 0 : fog_factor)
	  jmp	  FogFactorDone

NonLinearFog:
      movss   xmm1, [edx]RC.tl.lighting.fog_density // density
      shufps  xmm1, xmm1, 0						  // broadcast density
      mulps   xmm0, xmm1                          // distance * density
      movaps  xmm2, [TL_hi_bits]                  // 0x80000000

      test    [edx]RC.tl.lighting.materialDiffAlpha, D3DFOG_EXP2
      jne     SoaFogNotExp2

      mulps   xmm0, xmm0                          // (distance * density)^2

SoaFogNotExp2:
      xorps   xmm0, xmm2                          // -(distance * density)^2
      call    KniExp                              // xmm0 = exp(xmm0)
      movaps  xmm1, [TL_soa_255]
      mulps   xmm0, xmm1

FogFactorDone:

    /*
    int f;
    D3DVALUE tmp = dist * pRc->tl.lighting.fog_density;
    if (pRc->tl.lighting.fog_mode == D3DFOG_EXP2)
    {
    tmp *= tmp;
    }
    tmp = (D3DVALUE)exp(-tmp) * 255.0f;
    f = FTOI(tmp);
    RRPV_SET_ALPHA( pRc->tl.lighting.dwSpecular, f )
    }
    */

    /* Write the fog value back out (do we have to clamp to 255???) */
#ifdef CLAMP_FOG
    maxps     xmm0, [TL_soa_0]                    // clamp negative numbers to zero
    minps     xmm0, [TL_soa_255]                  // clamp over +255 to 255
#endif

    // Convert to int.
	movhlps   xmm1, xmm0
    cvtps2pi  mm0, xmm0
    cvtps2pi  mm1, xmm1
    movaps    [eax]_STL.fSOAFog, xmm0
    pslld     mm0, 24                             // move alpha to hi byte
    pslld     mm1, 24                             // move alpha to hi byte
    movq      [eax]_STL.dSOAFog.m.lo, mm0
    movq      [eax]_STL.dSOAFog.m.hi, mm1
  }
} // end of FogVertexSOA()
#pragma warning( pop ) /* restore the compiler warnings */



/*-------------------------------------------------------------------
Function Name:  taylorpowSSE
Description:    Fairly inaccurate pow() function for SSE,
				only used for falloff on specular
Parameters:     xmm0 = base
Information:    
Return:         xmm0 = pow(fbase, fexp);
-------------------------------------------------------------------*/

__declspec(align(32)) __m128 taylorpowSSE( float *fexp)
{
/*
   float temp;
   float mb1, mb2, mb3, mb5, mb7, mb9;
   float t1, t2, t3, t4, t5;
   
   mb1 = (fbase - 1.0f)/(fbase + 1.0);
   mb2 = mb1 * mb1;
   mb3 = mb2 * mb1;
   mb5 = mb3 * mb2;
   mb7 = mb5 * mb2;
   mb9 = mb7 * mb2;

   temp = 2.0f*( mb1 + mb3*(1.0f/3.0f) + mb5*(1.0f/5.0f) + mb7*(1.0f/7.0f) + mb9*(1.0f/9.0f) );

   temp = -fexp * temp;
   
   t1 = temp;
   t2 = t1 * temp;
   t3 = t2 * temp;
   t4 = t3 * temp;
   t5 = t4 * temp;
   
   fresult = 1.0f + t1 + (t2*(1.0f/2.0f)) + (t3*(1.0f/6.0f)) + 
      (t4*(1.0f/24.0f)) +(t5*(1.0f/120.0f));
   
   return(1.0f/fresult);
*/
//   float  *tmpBase, *tmpExp, *tmpRes;
   
//   tmpExp  = (float *)fexp;
//   tmpBase = (float *)fbase;
//   tmpRes  = (float *)fresult;

// aligned locals to save/restore xmm regs
__declspec(align(32)) SOA_FLOAT xreg1;
__declspec(align(32)) SOA_FLOAT xreg2;
__declspec(align(32)) SOA_FLOAT xreg3;
__declspec(align(32)) SOA_FLOAT xreg4;
//__declspec(align(32)) SOA_FLOAT xreg5;	// unused
__declspec(align(32)) SOA_FLOAT xreg6;
__declspec(align(32)) SOA_FLOAT xreg7;
   
   _asm 
   {
      movups [xreg1], xmm1
      movups [xreg2], xmm2
      movups [xreg3], xmm3
      movups [xreg4], xmm4
//      movups [xreg5], xmm5
      movups [xreg6], xmm6
      movups [xreg7], xmm7

      mov    edx, [fexp]
//      mov    eax, [fbase]
      xorps   xmm6, xmm6
      
      movaps  xmm1, xmm0          // xmm0 = xmm1 = base
      cmpneqps xmm6, xmm0
      subps  xmm0, [TL_soa_1]     // xmm0 = fbase - 1.0f
      addps  xmm1, [TL_soa_1]     // xmm1 = fbase + 1.0f
      rcpps  xmm2, xmm1           // xmm2 = 1.0f/(fbase + 1.0)
      mulps  xmm0, xmm2           // xmm0 = mb1 = (fbase - 1.0f)/(fbase + 1.0f)
      
      movaps xmm7, xmm0           // xmm7 = mb1
      mulps  xmm7, xmm0           // xmm7 = mb2 = mb1 * mb1
    
      movaps xmm1, xmm7           // xmm1 = mb2
      mulps  xmm1, xmm0           // xmm1 = mb3 = mb2 * mb1
      movaps xmm2, xmm1           // xmm2 = mb3
      mulps  xmm2, xmm7           // xmm2 = mb5 = mb3 * mb2
      movaps xmm3, xmm2           // xmm3 = mb5
      mulps  xmm3, xmm7           // xmm3 = mb7 = mb5 * mb2
      movaps xmm4, xmm3           // xmm4 = mb7
      mulps  xmm4, xmm7           // xmm4 = mb9 = mb7 * mb2

      mulps  xmm1, [TL_soa_r3]    // xmm1 = mb3/3.0f
      mulps  xmm2, [TL_soa_r5]    // xmm2 = mb5/5.0f
      mulps  xmm3, [TL_soa_r7]    // xmm3 = mb7/7.0f
      mulps  xmm4, [TL_soa_r9]    // xmm4 = mb9/9.0f
      addps  xmm0, xmm1           // xmm0 = mb1 + mb3/3.0f
      addps  xmm0, xmm2           // xmm0 = mb1 + mb3/3.0f + mb5/5.0f
      addps  xmm0, xmm3           // xmm0 = mb1 + mb3/3.0f + mb5/5.0f + mb7/7.0f
      addps  xmm0, xmm4           // xmm0 = mb1 + mb3/3.0f + mb5/5.0f + mb7/7.0f + mb9/9.0f
      mulps  xmm0, [TL_soa_2]     // xmm0 = ln(fbase) = 2.0f*(mb1 + mb3/3.0f + mb5/5.0f + 
                                  //                       mb7/7.0f + mb9/9.0f)
                                    
      movss xmm7, [edx]           // xmm7 = fexp
      shufps  xmm7, xmm7, 0       // broadcast
      
      mulps  xmm0, xmm7           // xmm0 = fexp * ln(fbase)
      
      //
      // We can negate fexp*ln(fbase) here because it will make the
      // exp portion of the code converge more quickly.  This trick
      // only works because 0.0 < fbase < 1.0 which makes the ln always 
      // negative.  Remember to fix the negation you need to take the 
      // reciprical of the exp at the end.
      //
      
      mulps  xmm0, [TL_soa_neg_1] // xmm0 = temp = t1 = -fexp * ln(fbase)
      movaps xmm1, xmm0           // xmm1 = t1  
      mulps  xmm1, xmm0           // xmm1 = t2 = t1 * temp
      movaps xmm2, xmm1           // xmm2 = t2 
      mulps  xmm2, xmm0           // xmm2 = t3 = t2 * temp
      movaps xmm3, xmm2           // xmm3 = t3
      mulps  xmm3, xmm0           // xmm3 = t4 = t3 * temp
      movaps xmm4, xmm3           // xmm4 = t4
      mulps  xmm4, xmm0           // xmm4 = t5 = t4 * temp
      
      mulps  xmm1, [TL_soa_r2f]   // xmm1 = t2/2!
      mulps  xmm2, [TL_soa_r3f]   // xmm2 = t3/3!
      mulps  xmm3, [TL_soa_r4f]   // xmm3 = t4/4!
      mulps  xmm4, [TL_soa_r5f]   // xmm4 = t5/5!
      
      addps  xmm0, [TL_soa_1]     // xmm0 = 1 + t1
      addps  xmm0, xmm1           // xmm0 = 1 + t1 + t2/2!
      addps  xmm0, xmm2           // xmm0 = 1 + t1 + t2/2! + t3/3! 
      addps  xmm0, xmm3           // xmm0 = 1 + t1 + t2/2! + t3/3! + t4/4!
      addps  xmm0, xmm4           // xmm0 = 1 + t1 + t2/2! + t3/3! + t4/4! + t5/5!

      rcpps  xmm1, xmm0           // xmm1 = fresult = 1/xmm0 to remove negation
      movaps xmm0, xmm1
      andps xmm0, xmm6
      
      movups xmm1, [xreg1]
      movups xmm2, [xreg2]
      movups xmm3, [xreg3]
      movups xmm4, [xreg4]
//      movups xmm5, [xreg5]
      movups xmm6, [xreg6]
      movups xmm7, [xreg7]
   }
}

#endif
#endif
#endif
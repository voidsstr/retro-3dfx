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
** File name: soatnl.c
**
** Description: Transformation and Lighting Code for SOA vertices
**
** $Revision: 25$
** $Date: 10/11/00 8:50:16 PM$
**
** $Log: 
**  25   3dfx      1.8.1.4.1.1010/11/00 Brent           Forced check in to enforce
**       branching.
**  24   3dfx      1.8.1.4.1.9 10/10/00 Allen Hansen    added vertex blends to the
**       optimized paths
**  23   3dfx      1.8.1.4.1.8 09/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  22   3dfx      1.8.1.4.1.7 09/22/00 Allen Hansen    implemented texgen and
**       texture transformation to the fastpaths
**  21   3dfx      1.8.1.4.1.6 09/13/00 Allen Hansen    Fixed bug when using
**       colorvertex; if diffuse comes from the vertex, then the alpha must come
**       from that source.  Was failing the D3DIM sample app "Shadow Volume 2"
**  20   3dfx      1.8.1.4.1.5 09/02/00 Allen Hansen    fixed colorvertex bug
**       w/specular
**  19   3dfx      1.8.1.4.1.4 08/31/00 Allen Hansen    split
**       "ConvertPackedSOAColortoSOA_RGB()" into 2 functions, one for SSE and one
**       for SSE2 (Pentium 4), added support to call the functions indirectly
**  18   3dfx      1.8.1.4.1.3 08/29/00 Allen Hansen    added colorvertex support
**       for sse path, note: P4 instructions in "ConvertPackedSOAColortoSOA_RGB()"
**       aren't enabled yet
**  17   3dfx      1.8.1.4.1.2 08/21/00 Allen Hansen    changed compiler check from
**       SSECPP to VCPP 
**  16   3dfx      1.8.1.4.1.1 07/03/00 Allen Hansen    Changed the name
**       pfnLightVertexSOA to pfnLightVertex  (shared pointer with 3dnow)
**  15   3dfx      1.8.1.4.1.0 06/25/00 Allen Hansen    updated TL_SOATMP variable
**       name, removed 3dnow code to k3dtnl.c
**  14   3dfx      1.8.1.4     06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  13   3dfx      1.8.1.3     05/30/00 Allen Hansen    Added pRc to the 
**       Xform_4_SOA_MAC macro
**  12   3dfx      1.8.1.2     05/22/00 Allen Hansen    Updated transformation
**       macros so the register values aren't hard-coded.
**  11   3dfx      1.8.1.1     05/19/00 Allen Hansen    Wrote asm version of
**       FP_Xform_Light_4Vert_SOA
**  10   3dfx      1.8.1.0     05/16/00 Bob Johnston    Consolodating VERT_BUFF and
**       TnL_HAL defines to just use TnL_HAL
**  9    Napalm    1.8         04/25/00 Allen Hansen    put emms instruction at
**       beginning to TL_RENDER functions (both use the FPU via C code)
**  8    Napalm    1.7         04/19/00 Scott Kephart   Big lighting change - Part
**       I
**       Lighting is now split into two parts, diffuse and specular. 
**  7    Napalm    1.6         04/12/00 Allen Hansen    Uncommented some dumb
**       changes I made
** 
**  6    Napalm    1.5         04/10/00 Allen Hansen    Worked prefetching: Changed
**       FP_IndexedTriangleList2_SOA_Split() to transform 4 soa groups instead of
**       1.  This required modifing AllocateSOAFVF().  I added a version of
**       FP_Xform_4Vert_SOA() that prefetches the next SOA group.  This still needs
**       optimization, but almost all cache-read misses are hidden.
**  5    Napalm    1.4         04/09/00 Allen Hansen    Prefetch work to all but
**       position x/y, some optimization work to the 3x3 and 4x3 transforms
**  4    Napalm    1.3         03/23/00 Bob Johnston    Scott and Bob's changes to
**       split up the tranformation and lighting in the vertex processing loop for
**       improved VB primitive perfromance.
**  3    Napalm    1.2         03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  2    Napalm    1.1         03/14/00 Scott Kephart   Added support for
**       Xform_DevColor_SOA
**  1    Napalm    1.0         03/08/00 Scott Kephart   
** $
** 
** 3     3/12/00 5:20p Skephart
** Changed number of parameters to FogVertexSOA
** 
** 2     3/10/00 4:42p Skephart
** Simplify FP_Xform_Light_4Vert_SOA
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

#include "soaxform.h"




/*-------------------------------------------------------------------
Function Name:  FP_Xform_4Vert_SOA
Description:    Transform 4 vertices in SOA format
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void FP_Xform_4Vert_SOA(RC *pRc, DWORD *pSOA)
{
  _asm {
	mov		esi, dword ptr [pRc]				// pRc
	mov		ecx, dword ptr [pSOA]				// pPos
	mov		edx, [esi]RC.tl.lpSOAxfmCurrent		// pMat4x4x4
	mov		ebx, [esi]RC.tl.pTL
	cmp		[esi]RC.tl.numVertexBlends, 0		// pRc->tl.numVertexBlends
	lea		eax, [ebx]TL_SOATMP.SOAhv	  	   	// &pTlTmp->hv
	jnz		VertexBlends
	Xform_4_SOA_MAC( esi, edx, ecx, eax )		// pRc,pMat,pSrc,pDst   h.xyzw in xmm4567
	test	dword ptr [esi]RC.tl.dwTLState, TLPV_DOCLIPPING
	je		NoClipping
	jmp		ComputeClipCodes

  VertexBlends:
	// pRc,pMat,pSrc,pDst,pcumulBlend,pBlend   h.xyzw in xmm4567, edx incremented to next matrix
    Xform_4_SOA_MAC_BLEND_FIRST( esi, edx, ecx, eax, ([ebx]TL_SOATMP.fSOAcumulBlend), ([ecx+(SIZE SOA_XYZ)+(0*SIZE SOA_FLOAT)]) )
	cmp		[esi]RC.tl.numVertexBlends, 1
	jle		LastVertexBlend

    Xform_4_SOA_MAC_BLEND_MIDDLE( esi, edx, ecx, eax, ([ebx]TL_SOATMP.fSOAcumulBlend), ([ecx+(SIZE SOA_XYZ)+(1*SIZE SOA_FLOAT)]) )
	cmp		[esi]RC.tl.numVertexBlends, 2
	jle		LastVertexBlend

    Xform_4_SOA_MAC_BLEND_MIDDLE( esi, edx, ecx, eax, ([ebx]TL_SOATMP.fSOAcumulBlend), ([ecx+(SIZE SOA_XYZ)+(2*SIZE SOA_FLOAT)]) )

  LastVertexBlend:
    Xform_4_SOA_MAC_BLEND_LAST( esi, edx, ecx, eax, ([ebx]TL_SOATMP.fSOAcumulBlend) )
	test	dword ptr [esi]RC.tl.dwTLState, TLPV_DOCLIPPING
	je		NoClipping

  ComputeClipCodes:
	test	dword ptr [esi]RC.tl.dwTLState, TLPV_DOCLIPPING
	je		NoClipping
	push	esi
	call	ComputeClipCodes_SOA
	add		esp, 4
  NoClipping:
  }
} 


/*-------------------------------------------------------------------
Function Name:  ConvertPackedSOAColortoSOA_RGB_SSE
Description:    Converts 4 packed rgba colors to SOA floats
				SSE version
Parameters:   
Return:         
-------------------------------------------------------------------*/
#pragma warning( push )
#pragma warning(disable : 4799)  /* this shuts up the "no emms" warning */
void ConvertPackedSOAColortoSOA_RGB_SSE( SOA_DWORD *src, SOA_RGB *dst)
{
#if 0
  DWORD i;
  for(i=0; i<SOA_SIZE; ++i)
  {
    DWORD color = src->d[i];
	dst.blue.f[i]  = RGBA_GETBLUE(color);
	dst.green.f[i] = RGBA_GETGREEN(color);
	dst.red.f[i]   = RGBA_GETRED(color);
  }
#endif
  // P3 code - 39 instructions
  _asm 
  {
	mov			eax, [src]
	pxor		mm2, mm2
	movd		mm0, [eax+ 0]	//	00 00 00 00  a0 r0 g0 b0
	pxor		mm5, mm5
	punpcklbw	mm2, [eax+ 4]	//	a1 00 r1 00  g1 00 b1 00
	pxor		mm7, mm7
	movd		mm3, [eax+ 8]	//	00 00 00 00  a2 r2 g2 b2
	punpcklbw	mm5, [eax+12]	//	a3 00 r3 00  g3 00 b3 00
	punpcklbw	mm0, mm7		//  00 a0 00 r0  00 g0 00 b0
	punpcklbw	mm3, mm7		//  00 a2 00 r2  00 g2 00 b2
	por			mm0, mm2		//	a1 a0 r1 r0  g1 g0 b1 b0
	por			mm3, mm5		//	a3 a2 r3 r2  g3 g2 b3 b2
	movq		mm2, mm0		//	a1 a0 r1 r0  g1 g0 b1 b0
	movq		mm5, mm3		//	a3 a2 r3 r2  g3 g2 b3 b2
	punpcklbw	mm0, mm7		//	00 g1 00 g0  00 b1 00 b0
	punpckhbw	mm2, mm7        //  00 a1 00 a0  00 r1 00 r0
	mov			eax, [dst]
	punpcklbw	mm3, mm7		//	00 g3 00 g2  00 b3 00 b2
	punpckhbw	mm5, mm7        //  00 a3 00 a2  00 r3 00 r2
	movq		mm1, mm0		//	00 g1 00 g0  00 b1 00 b0
	movq		mm4, mm3		//	00 g3 00 g2  00 b3 00 b2
	punpcklwd	mm0, mm7		//  00 00 00 b1  00 00 00 b0
	punpckhwd	mm1, mm7		//  00 00 00 g1  00 00 00 g0
	cvtpi2ps	xmm0, mm0		//	00  00  b1  b0
	punpcklwd	mm2, mm7		//  00 00 00 r1  00 00 00 r0
	cvtpi2ps	xmm1, mm1		//	00  00  g1  g0
	punpcklwd	mm3, mm7		//  00 00 00 b3  00 00 00 b2
	cvtpi2ps	xmm2, mm2		//	00  00  r1  r0
	movlps		[eax+0]SOA_RGB.blue, xmm0
	punpckhwd	mm4, mm7		//  00 00 00 g3  00 00 00 g2
	movlps		[eax+0]SOA_RGB.green, xmm1
	cvtpi2ps	xmm3, mm3		//	00  00  b3  b2
	movlps		[eax+0]SOA_RGB.red, xmm2
	punpcklwd	mm5, mm7		//  00 00 00 r3  00 00 00 r2
	cvtpi2ps	xmm4, mm4		//	00  00  g3  g2
	movlps		[eax+8]SOA_RGB.blue, xmm3
	cvtpi2ps	xmm5, mm5		//	00  00  r3  r2
	movlps		[eax+8]SOA_RGB.green, xmm4
	movlps		[eax+8]SOA_RGB.red, xmm5
  }
}
#pragma warning( pop ) /* restore the compiler warnings */

/*-------------------------------------------------------------------
Function Name:  ConvertPackedSOAColortoSOA_RGB_SSE2
Description:    Converts 4 packed rgba colors to SOA floats
				SSE2 version (P4)
Parameters:   
Return:         
-------------------------------------------------------------------*/
void ConvertPackedSOAColortoSOA_RGB_SSE2( SOA_DWORD *src, SOA_RGB *dst)
{
  //P4 code - 27 instructions
  _asm 
  {
	mov			eax, [src]
	xorps		xmm7, xmm7
	movdqa		xmm0, [eax]			// a3 r3 g3 b3  a2 r2 g2 b2  a1 r1 g1 b1  a0 r0 g0 b0
	pshuflw		xmm1, xmm0, 0xd8	// xx xx xx xx  xx xx xx xx  a1 r1 a0 r0  g1 b1 g0 b0
	pshufhw		xmm2, xmm0, 0xd8	// a3 r3 a2 r2  g3 b3 g2 b2  xx xx xx xx  xx xx xx xx
	punpcklbw	xmm1, xmm7			// 00 a1 00 r1  00 a0 00 r0  00 g1 00 b1  00 g0 00 b0
	punpckhbw	xmm2, xmm7			// 00 a3 00 r3  00 a2 00 r2  00 g3 00 b3  00 g2 00 b2

	pshuflw		xmm3, xmm1, 0xd8	// xx xx xx xx  xx xx xx xx  00 g1 00 g0  00 b1 00 b0
	pshufhw		xmm4, xmm1, 0xd8	// 00 a1 00 a0  00 r1 00 r0  xx xx xx xx  xx xx xx xx
	pshuflw		xmm5, xmm2, 0xd8	// xx xx xx xx  xx xx xx xx  00 g3 00 g2  00 b3 00 b2
	pshufhw		xmm6, xmm2, 0xd8	// 00 a3 00 a2  00 r3 00 r2  xx xx xx xx  xx xx xx xx

	punpcklwd	xmm3, xmm7			// 00 00 00 g1  00 00 00 g0  00 00 00 b1  00 00 00 b0
	punpckhwd	xmm4, xmm7			// 00 00 00 a1  00 00 00 a0  00 00 00 r1  00 00 00 r0
	punpcklwd	xmm5, xmm7			// 00 00 00 g3  00 00 00 g2  00 00 00 b3  00 00 00 b2
	punpckhwd	xmm6, xmm7			// 00 00 00 a3  00 00 00 a2  00 00 00 r3  00 00 00 r2
	mov			eax, [dst]
	punpcklqdq	xmm4, xmm6			// 00 00 00 r3  00 00 00 r2  00 00 00 r1  00 00 00 r0

	cvtdq2ps	xmm0, xmm3			//          g1           g0           b1           b0
	cvtdq2ps	xmm1, xmm5			//          g3           g2           b3           b2
	cvtdq2ps	xmm2, xmm4			//          r3           r2           r1           r0
	movlps		[eax+0]SOA_RGB.blue, xmm0	// b1 b0
	movhps		[eax+0]SOA_RGB.green, xmm0	// g1 g0
	movlps		[eax+8]SOA_RGB.blue, xmm1	// b3 b2
	movhps		[eax+8]SOA_RGB.green, xmm1	// g3 g2
	movlps		[eax+0]SOA_RGB.red, xmm2	// r1 r0
	movhps		[eax+8]SOA_RGB.red, xmm2	// r3 r2
  }
}





/*-------------------------------------------------------------------
Function Name:  FP_Light_4Vert_SOA
Description:    Light 4 vertices in SOA format
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void FP_Light_4Vert_SOA(RC *pRc, DWORD *pSOA)
{
  _asm {
	mov		esi, dword ptr [pRc]				// pRc
	mov		ecx, dword ptr [pSOA]				// pPos = pSOA
	mov		ebx, [esi]RC.tl.dwTLState			// dwTLState

	test	ebx, TLPV_DIFF_SRC_VTX
	je		DoNotCopySrcVtxDiff
	mov		eax, [esi]RC.tl.SOAFVF.dwDiffuseOffset
	add		eax, ecx
	mov		[esi]RC.tl.lighting.pdSOADiffRGBSrc, eax	// pDiffuse
// pRc->tl.lighting.pdSOADiffRGBSrc = pDiffuse = (SOA_DWORD *) ((LPBYTE)pSOA + pRc->tl.SOAFVF.dwDiffuseOffset);
DoNotCopySrcVtxDiff:

	test	ebx, TLPV_SPEC_SRC_VTX
	je		DoNotCopySrcVtxSpec
	mov		eax, [esi]RC.tl.SOAFVF.dwSpecularOffset
	add		eax, ecx
	mov		[esi]RC.tl.lighting.pdSOASpecRGBSrc, eax	// pSpecular
// pRc->tl.lighting.pdSOASpecRGBSrc = pSpecular = (SOA_DWORD *) ((LPBYTE)pSOA + pRc->tl.SOAFVF.dwSpecularOffset);
DoNotCopySrcVtxSpec:
	test	ebx, (TLPV_DOLIGHTING | TLPV_DOFOG | TLPV_DOTEXGEN)
	je		NoLightingOrFog

// Lighting Path
	// xform position to eye coordinates
	cmp		[esi]RC.tl.numVertexBlends, 0		// pRc->tl.numVertexBlends
	mov		edx, [esi]RC.tl.lpSOAxfmToEye		// pMat4x4x4
	mov		eax, [esi]RC.tl.pTL					// pTlTmp
	jne		VertexBlends

	// No Vertex Blends ------------------------------------------
	lea		eax, [eax]TL_SOATMP.SOAcv 			// &pTlTmp->SOAcv
	Xform_3_SOA_MAC( edx, ecx, eax )			// pMat, pSrc, pDst
	test	ebx, (TLPV_DOLIGHTING | TLPV_DOTEXGEN)
	je		NoLighting_Fog

	// xform normals
	add		ecx, [esi]RC.tl.SOAFVF.dwNormalOffset
	mov		edx, [esi]RC.tl.lpSOAxfmToEyeInvT	// pMat4x4x4
	Xform_3_SOA_NOSTORE_MAC( edx, ecx )	 		// pMat,pSrc

	// normalize normals if needed
	mov		eax, [esi]RC.tl.pTL					// pTlTmp
	mov		ecx, dword ptr [pSOA]				// reload pPos = pSOA
	test	ebx, TLPV_NORMALIZENORMALS
	je		StoreNormals
	jmp		NormalizeNormals


	// Vertex Blends ---------------------------------------------
VertexBlends:
	// pMat, pSrc, pDst, pcumulBlend, pBlend
	Xform_3_SOA_MAC_BLEND_FIRST( edx, ecx, ([eax]TL_SOATMP.SOAcv), ([eax]TL_SOATMP.fSOAcumulBlend), ([ecx+(SIZE SOA_XYZ)+(0*SIZE SOA_FLOAT)]) )
	cmp		[esi]RC.tl.numVertexBlends, 1
	jle		LastEyeCoordVertexBlend

	Xform_3_SOA_MAC_BLEND_MIDDLE( edx, ecx, ([eax]TL_SOATMP.SOAcv), ([eax]TL_SOATMP.fSOAcumulBlend), ([ecx+(SIZE SOA_XYZ)+(1*SIZE SOA_FLOAT)]) )
	cmp		[esi]RC.tl.numVertexBlends, 2
	jle		LastEyeCoordVertexBlend

	Xform_3_SOA_MAC_BLEND_MIDDLE( edx, ecx, ([eax]TL_SOATMP.SOAcv), ([eax]TL_SOATMP.fSOAcumulBlend), ([ecx+(SIZE SOA_XYZ)+(2*SIZE SOA_FLOAT)]) )

LastEyeCoordVertexBlend:
	Xform_3_SOA_MAC_BLEND_LAST( edx, ecx, ([eax]TL_SOATMP.SOAcv), ([eax]TL_SOATMP.fSOAcumulBlend) )	// xyz in xmm012
    movaps  [eax]TL_SOATMP.SOAcv.x, xmm0	/* SOAcv->x */
    movaps  [eax]TL_SOATMP.SOAcv.y, xmm1	/* SOAcv->y */
    movaps  [eax]TL_SOATMP.SOAcv.z, xmm2	/* SOAcv->z */
	test	ebx, (TLPV_DOLIGHTING | TLPV_DOTEXGEN)
	je		NoLighting_Fog

	// xform normals
	mov		ebx, ecx							// pPos = pSOA
	add		ebx, [esi]RC.tl.SOAFVF.dwNormalOffset
	mov		edx, [esi]RC.tl.lpSOAxfmToEyeInvT	// pMat4x4x4
	Xform_3_SOA_MAC_BLEND_FIRST( edx, ebx, ([eax]TL_SOATMP.SOAcn), ([eax]TL_SOATMP.fSOAcumulBlend), ([ecx+(SIZE SOA_XYZ)+(0*SIZE SOA_FLOAT)]) )
	cmp		[esi]RC.tl.numVertexBlends, 1
	jle		LastNormalVertexBlend

	Xform_3_SOA_MAC_BLEND_MIDDLE( edx, ebx, ([eax]TL_SOATMP.SOAcn), ([eax]TL_SOATMP.fSOAcumulBlend), ([ecx+(SIZE SOA_XYZ)+(1*SIZE SOA_FLOAT)]) )
	cmp		[esi]RC.tl.numVertexBlends, 2
	jle		LastNormalVertexBlend

	Xform_3_SOA_MAC_BLEND_MIDDLE( edx, ebx, ([eax]TL_SOATMP.SOAcn), ([eax]TL_SOATMP.fSOAcumulBlend), ([ecx+(SIZE SOA_XYZ)+(2*SIZE SOA_FLOAT)]) )

LastNormalVertexBlend:
	Xform_3_SOA_MAC_BLEND_LAST( edx, ebx, ([eax]TL_SOATMP.SOAcn), ([eax]TL_SOATMP.fSOAcumulBlend) )	// xyz in xmm012

	// normalize normals if needed
	mov		ebx, [esi]RC.tl.dwTLState			// reload dwTLState
	test	ebx, TLPV_NORMALIZENORMALS
	je		StoreNormals

NormalizeNormals:
	Normalize3SOA_REG_MAC						// doesn't touch integer registers
StoreNormals:
	// store the normals
	lea		eax, [eax]TL_SOATMP.SOAcn			// &pTlTmp->SOAcn
	movaps  [eax]_SV3.x, xmm0
	movaps  [eax]_SV3.y, xmm1
	movaps  [eax]_SV3.z, xmm2


	// prefetch the textures before we call the lighting functions
	mov		edx, [esi]RC.tl.KniRC.RfBits		// RfBits
	add		ecx, [esi]RC.tl.SOAFVF.dwTexOffset	// pTexture
	test	edx, BIT_RC_REQUIRES_TX1
	je		Lighting_Prefetch_NoTx1
	prefetchnta	[ecx]
	add		ecx, 32
Lighting_Prefetch_NoTx1:
	test	edx, BIT_RC_REQUIRES_TX0
	je		Lighting_Prefetch_NoTx0
	prefetchnta	[ecx]
Lighting_Prefetch_NoTx0:


	// if needed, load the colorvertex values from the input vertex
	test	ebx, TLPV_VERTEXDIFFUSENEEDED
	jz		NoColorvertexDiffuse
	mov		edx, [esi]RC.tl.pTL					// pTmp
	mov		ecx, dword ptr [pSOA]				// src SOA vertex
	lea		edx, [edx]_STL.fSOADiffuseIn
	add		ecx, [esi]RC.tl.SOAFVF.dwDiffuseOffset
    mov     eax, [esi]RC.tl.lighting.ConvertPackedSOAColortoSOA_RGB
	push	edx									// dst
	push	ecx									// src
	call	eax									// hoses edx
	add		esp, 8
NoColorvertexDiffuse:

	test	ebx, TLPV_VERTEXSPECULARNEEDED
	jz		NoColorvertexSpecular
	mov		edx, [esi]RC.tl.pTL					// pTmp
	mov		ecx, dword ptr [pSOA]				// src SOA vertex
	lea		edx, [edx]_STL.fSOASpecularIn
	add		ecx, [esi]RC.tl.SOAFVF.dwSpecularOffset
    mov     eax, [esi]RC.tl.lighting.ConvertPackedSOAColortoSOA_RGB
	push	edx									// dst
	push	ecx									// src
	call	eax									// hoses edx
	add		esp, 8
NoColorvertexSpecular:

	mov		ecx, [esi]RC.tl.lighting.dSOAOffsetDiffAlphaSrc
	cmp		ecx, 0
	jz		NoDiffuseAlphaSourceNeeded
	movaps	xmm6, TL_maskHigh8					// ff000000 ...
	add		ecx, dword ptr [pSOA]				// src SOA vertex
	mov		edx, [esi]RC.tl.pTL					// pTmp
	andps	xmm6, [ecx]							// diff.alpha in
	movaps	[edx]_STL.dSOADiffAlpha, xmm6
NoDiffuseAlphaSourceNeeded:


	// call the lighting functions
	mov		eax, [esi]RC.tl.lighting.pfnLightVertex
	push	esi		// pRc
	call	eax		// pfnLightVertex(pRC)
	add		esp, 4

	test	ebx, TLPV_DOFOG
	je		Done
	push	esi		// pRc
	call	FogVertexSOA
	add		esp, 4
	jmp 	Done

// Fog with no lighting path
NoLighting_Fog:

	mov		edx, [esi]RC.tl.KniRC.RfBits		// RfBits
	test	ebx, (TLPV_DIFF_SRC_VTX | TLPV_SPEC_SRC_VTX)
	je		NoLighting_Fog_Prefetch_NoColor
	mov		eax, [esi]RC.tl.SOAFVF.dwDiffuseOffset
	prefetchnta [eax+ecx]
NoLighting_Fog_Prefetch_NoColor:

	add		ecx, [esi]RC.tl.SOAFVF.dwTexOffset	// pTexture
	test	edx, BIT_RC_REQUIRES_TX1
	je		NoLighting_Fog_Prefetch_NoTx1
	prefetchnta	[ecx]
	add		ecx, 32
NoLighting_Fog_Prefetch_NoTx1:
	test	edx, BIT_RC_REQUIRES_TX0
	je		NoLighting_Fog_Prefetch_NoTx0
	prefetchnta	[ecx]
NoLighting_Fog_Prefetch_NoTx0:

	push	esi		// pRc
	call	FogVertexSOA
	add		esp, 4
	jmp 	Done


// no lighting or fog path (prefetch only)
NoLightingOrFog:
	test	ebx, (TLPV_DIFF_SRC_VTX | TLPV_SPEC_SRC_VTX)
	mov		edx, [esi]RC.tl.KniRC.RfBits		// RfBits
	je		NoLightingOrFog_Prefetch_NoColor
	mov		eax, [esi]RC.tl.SOAFVF.dwDiffuseOffset
	prefetchnta [eax+ecx]
NoLightingOrFog_Prefetch_NoColor:

	add		ecx, [esi]RC.tl.SOAFVF.dwTexOffset	// pTexture
	test	edx, BIT_RC_REQUIRES_TX1
	je		NoLightingOrFog_Prefetch_NoTx1
	prefetchnta	[ecx]
	add		ecx, 32
NoLightingOrFog_Prefetch_NoTx1:
	test	edx, BIT_RC_REQUIRES_TX0
	je		NoLightingOrFog_Prefetch_NoTx0
	prefetchnta	[ecx]
NoLightingOrFog_Prefetch_NoTx0:


// Done (it would be really nice to align 16 this)
Done:

  }

#if 0	// old C code, color vertex not added
  SOA_XYZ *pPos = (SOA_XYZ *) pSOA;
  SOA_XYZ *pNorm = (SOA_XYZ *)((LPBYTE)pSOA + pRc->tl.SOAFVF.dwNormalOffset);
  SOA_DWORD *pDiffuse = (SOA_DWORD *) ((LPBYTE)pSOA + pRc->tl.SOAFVF.dwDiffuseOffset);
  SOA_DWORD *pSpecular = (SOA_DWORD *) ((LPBYTE)pSOA + pRc->tl.SOAFVF.dwSpecularOffset);
  TL_SOATMP *pTlTmp = (TL_SOATMP *)pRc->tl.pTL;
  DWORD dwTLState = pRc->tl.dwTLState;

  DWORD *pTexture = (DWORD *)((LPBYTE)pSOA + pRc->tl.SOAFVF.dwTexOffset);
  DWORD *pTex1 = pTexture + pRc->t1CoordIndex * 4;
  DWORD *pTex0 = pTexture + pRc->t0CoordIndex * 4;
  DWORD RcBits = pRc->tl.KniRC.RfBits;

  if (dwTLState & TLPV_DIFF_SRC_VTX)
    // Xform_DevColor_SOA is going to copy diffuse color from input vertex
    pRc->tl.lighting.pdSOADiffRGBSrc = pDiffuse;

  if (dwTLState & TLPV_SPEC_SRC_VTX)
    // Xform_DevColor_SOA is going to copy specular color from input vertex
    pRc->tl.lighting.pdSOASpecRGBSrc = pSpecular;

  if (dwTLState & (TLPV_DOLIGHTING | TLPV_DOFOG)) 
  {
    Xform_3_SOA(&pTlTmp->SOAcv, pPos, pRc->tl.lpSOAxfmToEye);

    if (dwTLState & TLPV_DOLIGHTING) 
    {
      Xform_3_SOA(&pTlTmp->SOAcn, pNorm, pRc->tl.lpSOAxfmToEyeInvT[0]);

      if (dwTLState & TLPV_NORMALIZENORMALS)
        Normalize3SOA(&pTlTmp->SOAcn);

      if (RcBits & BIT_RC_REQUIRES_TX1)
	    SOA_PREFETCH(pTex1);
      if (RcBits & BIT_RC_REQUIRES_TX0)
	    SOA_PREFETCH(pTex0);

      // Perform lighting if needed
       pRc->tl.lighting.pfnLightVertex(pRc);
 //LightVertexSOA(pRc); old stuff
     }
    else
    {
      if (dwTLState & (TLPV_DIFF_SRC_VTX | TLPV_SPEC_SRC_VTX))
	    SOA_PREFETCH(pDiffuse);
      if (RcBits & BIT_RC_REQUIRES_TX1)
	    SOA_PREFETCH(pTex1);
      if (RcBits & BIT_RC_REQUIRES_TX0)
	    SOA_PREFETCH(pTex0);
    }

    if (dwTLState & TLPV_DOFOG)
      FogVertexSOA(pRc);      // Compute vertex fog if needed

  }
  else
  {
    if (dwTLState & (TLPV_DIFF_SRC_VTX | TLPV_SPEC_SRC_VTX))
      SOA_PREFETCH(pDiffuse);
    if (RcBits & BIT_RC_REQUIRES_TX1)
      SOA_PREFETCH(pTex1);
    if (RcBits & BIT_RC_REQUIRES_TX0)
	  SOA_PREFETCH(pTex0);
  }
#endif //0
}


    
#endif 
#endif
#endif
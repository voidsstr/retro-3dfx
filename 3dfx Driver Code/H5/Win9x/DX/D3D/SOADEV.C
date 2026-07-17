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
** File name: soadev.c
**
** Description: Device Coordinate Transform
**
** $Revision: 30$
** $Date: 10/18/00 5:40:45 PM$
**
** $Log: 
**  30   3dfx      1.9.1.5.1.1310/18/00 Allen Hansen    Changed SSE T&L tag scheme
**       from a bitfield to marking the vert T&L'd in one of the textures
**  29   3dfx      1.9.1.5.1.1210/11/00 Brent           Forced check in to enforce
**       branching.
**  28   3dfx      1.9.1.5.1.1110/10/00 Allen Hansen    simplfied utility functions
**       (now all are funct(src,dst))
**  27   3dfx      1.9.1.5.1.1009/28/00 Allen Hansen    Texgen bugfix
**  26   3dfx      1.9.1.5.1.9 09/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  25   3dfx      1.9.1.5.1.8 09/22/00 Allen Hansen    implemented texgen and
**       texture transformation to the fastpaths
**  24   3dfx      1.9.1.5.1.7 09/15/00 Allen Hansen    Got rid of the TLBN_CLIP
**       structure (with homogenous coords), all output verts, clipped or not, use
**       the TLBN structure.
**  23   3dfx      1.9.1.5.1.6 08/29/00 Allen Hansen    moved float-to-int color
**       conversion to lighting functions (saves 3 128-bit stores/loads per 4
**       verts)
**  22   3dfx      1.9.1.5.1.5 08/21/00 Allen Hansen    changed compiler check from
**       SSECPP to VCPP 
**  21   3dfx      1.9.1.5.1.4 07/24/00 Allen Hansen    whql bug fixes (T&L tests)
**  20   3dfx      1.9.1.5.1.3 07/21/00 Allen Hansen    whql failure: spec alpha
**       comes from diffuse alpha
**  19   3dfx      1.9.1.5.1.2 07/08/00 Allen Hansen    replaced hard-wired TLBN
**       and TLBN_CLIP size macros with SIZE xxx
**  18   3dfx      1.9.1.5.1.1 06/29/00 Allen Hansen    moved szTLBN & szTLBNc to
**       soa.h
**  17   3dfx      1.9.1.5.1.0 06/25/00 Allen Hansen    updated a few coments
**  16   3dfx      1.9.1.5     06/01/00 Allen Hansen    Fixed special-case texture
**       bug, added "SOA" to all variables
**  15   3dfx      1.9.1.4     05/30/00 Allen Hansen    Added the
**       XformDeswiz4_DevCoord_SOA_GEOM functions to
**       FP_IndexedTriangleList2_SOA_Split, the transformation code calls one
**       function instead of several, gain's about 1/2 fps
**  14   3dfx      1.9.1.3     05/22/00 Allen Hansen    Updated transformation
**       macros so the register values aren't hard-coded.
**       Implemented XForm & Deswizzle Device Coord's Loop Functions, used by the
**       2-pass outer loop.
** 
**  13   3dfx      1.9.1.2     05/16/00 Bob Johnston    Consolodating VERT_BUFF and
**       TnL_HAL defines to just use TnL_HAL
**  12   3dfx      1.9.1.1     05/10/00 Allen Hansen    Did some minor
**       optimizations to the asm version of Xform_DevColor_SOA
**  11   3dfx      1.9.1.0     05/03/00 Allen Hansen    Moved deswizzling of color,
**       fog, and textures into the texture transform functions, which eliminates
**       storing and loading these params
**  10   Napalm    1.9         04/26/00 Bob Johnston    use of local function
**       pointers to help eliminate branching in the vertex loops.  Setup a Fn
**       pointer for Xform_DevCoord_SOA_GEOM
**  9    Napalm    1.8         04/19/00 Allen Hansen    Fixed bug I introduced in
**       the asm version of Xform_DevColor_SOA (fog was broke), this caused WB2K
**       tst#69 to crash
**  8    Napalm    1.7         04/18/00 Allen Hansen    Converted the color
**       interpolation functions to asm
**  7    Napalm    1.6         04/11/00 Bob Johnston    Optimization's in the
**       Device Coordinate Transform code for retrieving scales and offsets in SOA
**       form, thus minimizing the only fly shuffle and HW offset adds.
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
**  2    Napalm    1.1         03/14/00 Scott Kephart   Added Xform_DevColor_SOA
**  1    Napalm    1.0         03/08/00 Scott Kephart   
** $
** 
** 4     3/12/00 10:10p Skephart
** Fixed Vertex Fog when W Buffering used
** 
** 3     3/12/00 5:21p Skephart
** Fixed fogging problems
** 
** 2     3/10/00 4:40p Skephart
** Add Xform_DevColor_SOA - Do device specific color conversion
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
Function Name:  Xform_DevColor_SOA
Description:    Peform Device Specific Color Processing
                This routine's sole purpose in life is to output
                TL_SOATMP.diff, TL_SOATMP.spec, and TL_SOATMP.fSOAVertFog
                so that these values are ready for Xform_DevCoord_SOA.
Parameters:   
Information:    
                A number of variables control how colors are put 
                together.
                TL_SOATMP.fSOADiffuseOut holds the output from Lighting for diffuse color
                TL_SOATMP.fSOASpecularOut holds the output from Lighting for Specular color

                tl.lighting.pdSOADiffRGBSrc controls the source of 
                integer diffuse RGB color.

                tl.lighting.pdSOADiffAlphaSrc controls the source
                of integer diffuse alpha.

                tl.lighting.pdSOASpecRGBSrc controls the source of
                integer specular RGB Color

                tl.lighting.pdSOASpecAlphaSrc controls the source of
                integer specular alpha.

                tl.lighting.pfSOASpecAlphaSrc controls the source of
                floating point specular alpha.

                Specular alpha is needed as a float because it's used 
                in fogging on Napalm. It's available as a float except
                when the fog factor is passed in the input vertex. In 
                that case, the fog factor has to be converted back to 
                a float. (TLPV_DEV_ITOF_SPEC_ALPHA)

                Specular and diffuse RGB and alpha information can 
                be combined in two ways in this routine. They are 
                either copied from someplace into TL_SOATMP.diff and 
                TL_SOATMP.dSOAspec, or they are OR'd together and then
                stored into TL_SOATMP.dSOAdiff and TL_SOATMP.dSOAspec.
Return:         
-------------------------------------------------------------------*/
#pragma warning( push )
#pragma warning(disable : 4799)  /* this shuts up the "no emms" warning */

void Xform_DevColor_SOA(RC *pRc)
{
  _asm {
	mov			ebx, pRc
	mov			edx, [ebx]RC.tl.dwTLDevFlags
	mov			eax, [ebx]RC.tl.pTL


	test		edx, TLPV_DEV_ITOF_SPEC_ALPHA
	je			ConvertSpecularAlphaDone
    // We have to convert integer specular alpha to float
    mov   		ecx, [ebx]RC.tl.lighting.pdSOASpecAlphaSrc
    movq  		mm0, [ecx]MMX2.lo
    movq  		mm1, [ecx]MMX2.hi
    psrld 		mm0, 24
    psrld 		mm1, 24
    mov   		ecx, [ebx]RC.tl.lighting.pfSOASpecAlphaSrc
    cvtpi2ps  	xmm0, mm0
    cvtpi2ps  	xmm1, mm1
    movlps  	[ecx]MMX2.lo, xmm0
    movlps  	[ecx]MMX2.hi, xmm1
ConvertSpecularAlphaDone:


	test		edx, (TLPV_DEV_COPY_DIFF | TLPV_DEV_COMBINE_DIFF)
	je			DiffuseDone
    // Just copy diffuse RGBA over
    mov   		ecx, [ebx]RC.tl.lighting.pdSOADiffRGBSrc
    movaps  	xmm0, [ecx]
	test		edx, TLPV_DEV_COMBINE_DIFF
    mov   		ecx, [ebx]RC.tl.lighting.pdSOADiffAlphaSrc
	je			CombineDiffuse_StoreResult
    // munge together diffuse RGB & Alpha
	orps		xmm0, [ecx]
CombineDiffuse_StoreResult:
	// test special texture modes
	cmp			[ebx]RC.specialModes, 0
    movaps  	[eax]_STL.dSOAdiff, xmm0
	je			DiffuseDone
	// double (with clamp) r,g,b, don't change alpha
	movq		mm2, TL_maskLow24		// 00ffffff 00ffffff
	movq		mm0, [eax]_STL.dSOAdiff.m.lo
	movq		mm1, [eax]_STL.dSOAdiff.m.hi
	movq		mm3, mm2
	pand		mm2, mm0			// mask alpha
	pand		mm3, mm1
	paddusb		mm0, mm2			// double r,g,b
	paddusb		mm1, mm3			// alpha is unchanged
	movq		[eax]_STL.dSOAdiff.m.lo, mm0
	movq		[eax]_STL.dSOAdiff.m.hi, mm1
DiffuseDone:

	test		edx, (TLPV_DEV_COPY_SPEC | TLPV_DEV_COMBINE_SPEC)
	je			SpecularDone
    mov   		ecx, [ebx]RC.tl.lighting.pdSOASpecRGBSrc
    movaps  	xmm0, [ecx]
	test		edx, TLPV_DEV_COMBINE_SPEC
	//mov  		ecx, [ebx]RC.tl.lighting.pdSOASpecAlphaSrc
	mov	ecx, [ebx]RC.tl.lighting.pdSOADiffAlphaSrc	// spec alpha always comes from diffuse alpha
	je			CombineSpecular_StoreResult
    // munge together specular RGB & Alpha
	orps		xmm0, [ecx]
CombineSpecular_StoreResult:
    movaps  	[eax]_STL.dSOAspec, xmm0
SpecularDone:


	test		edx, TLPV_DEV_VERTEX_FOG
	je			CopyVertexFogDone
    // We have vertex fog -- copy from where-ever to fVertFog
    mov   		ecx, [ebx]RC.tl.lighting.pfSOASpecAlphaSrc
    movaps  	xmm0, [ecx]
    movaps  	[eax]_STL.fSOAVertFog, xmm0
CopyVertexFogDone:

  }
}  
#pragma warning( pop ) /* restore the compiler warnings */



/*-------------------------------------------------------------------
Function Name:  Xform_DevCoord_SOA_GEOM_WBuff
Description:    Convert homogeneous coordinates to device coordinates
Parameters:   
Information:    Implemented for the Split T&L (Xform only)
                Includes w buff calculation
Return:         

x = (((x * 1/w) * scaleX) + offsetX) + pixelOffset
y = (((y * 1/w) * scaleY) + offsetY) + pixelOffset
z = (((z * 1/w) * scaleZ) + offsetZ) * zScale
-------------------------------------------------------------------*/
void Xform_DevCoord_SOA_GEOM_WBuff(RC *pRc)
{
  
  __asm 
  {
    mov     eax, pRc                // eax = pointer to pRc
    mov     ecx, [eax]RC.tl.pTL     // ecx = pointer to pRc->tl
    movaps  xmm7, [ecx]_STL.SOAhv.w // Grab SOA vertex info for all Homogeneous Components
#ifdef DIV_RHW    // Define to use divps to compute 1/w instead of approximation
    movaps  xmm0, TL_one
    divps   xmm0, xmm7
#else
    rcpps   xmm0, xmm7                      // recip approx 
    mulps   xmm7, xmm0                      // x * rcpps(x)
    mulps   xmm7, xmm0                      // (x * rcpps(x)) * rcpps(x)
    addps   xmm0, xmm0                      // 2 * rcpps(x)
    subps   xmm0, xmm7                      // 2 * rcppx(x) - ((x * rcpps(x)) * rcpps(x))
#endif
    movaps  xmm1, [ecx]_STL.SOAhv.x    
    movaps  xmm2, [ecx]_STL.SOAhv.y
    movaps  xmm3, [ecx]_STL.SOAhv.z
    mulps   xmm1, xmm0                          // Mult X, Y and Z through RHW
    mulps   xmm2, xmm0
    mulps   xmm3, xmm0
    movaps  xmm4, [ecx]_STL.fSOAviewDataScaleX  // Load SOA Scale X from TMP
    movaps  xmm5, [ecx]_STL.fSOAviewDataScaleY  // Load SOA Scale Y from TMP
    movaps  xmm6, [ecx]_STL.fSOAviewDataScaleZ  // Load SOA Scale Z from TMP
    mulps   xmm1, xmm4                          // Mult X, Y and Z through scales
    mulps   xmm2, xmm5
    mulps   xmm3, xmm6
    movaps  xmm4, [ecx]_STL.fSOAviewDataOffsetX // Load SOA Offset X from TMP (has HW offset pre-added)
    movaps  xmm5, [ecx]_STL.fSOAviewDataOffsetY // Load SOA Offset Y from TMP (has HW offset pre-added)
    movaps  xmm6, [ecx]_STL.fSOAviewDataOffsetZ // Load SOA Offset Z from TMP
    addps   xmm1, xmm4                          // Add X, Y Z to the scale offsets 
    addps   xmm2, xmm5
    addps   xmm3, xmm6
    movaps  xmm5, [ecx]_STL.fSOAhwZScale            // Get the SOA HW Z Scale
    mulps   xmm3, xmm5
    movaps  [ecx]_STL.SOAsv.w, xmm0             // w
    movaps  [ecx]_STL.SOAsv.x, xmm1             // Write out the Napalm Dev Coords X & Y
    movaps  [ecx]_STL.SOAsv.y, xmm2
    movaps  [ecx]_STL.SOAsv.z, xmm3             // Write out the Napalm Dev Coords Z & wfbi

    movaps  xmm4, [ecx]_STL.fSOAwbuffScale          // Load SOA W buffer scale
    movaps  xmm5, [ecx]_STL.fSOAwbuffOffset         // Load SOA W buffer Offset
    mulps   xmm4, xmm0
    addps   xmm4, xmm5
    movaps  [ecx]_STL.fSOAwfbi, xmm4             // wfbi = RHW * scaleAW + scaleBW
  }
} 



/*-------------------------------------------------------------------
Function Name:  Xform_DevCoord_SOA_GEOM_NoWBuff
Description:    Convert homogeneous coordinates to device coordinates
Parameters:   
Information:    Implemented for the Split T&L (Xform only)
                Excludes w buff calculation
Return:         

x = (((x * 1/w) * scaleX) + offsetX) + pixelOffset
y = (((y * 1/w) * scaleY) + offsetY) + pixelOffset
z = (((z * 1/w) * scaleZ) + offsetZ) * zScale
-------------------------------------------------------------------*/
void Xform_DevCoord_SOA_GEOM_NoWBuff(RC *pRc)
{
  
  __asm 
  {
    mov     eax, pRc                // eax = pointer to pRc
    mov     ecx, [eax]RC.tl.pTL     // ecx = pointer to pRc->tl
    movaps  xmm7, [ecx]_STL.SOAhv.w // Grab SOA vertex info for all Homogeneous Components
#ifdef DIV_RHW    // Define to use divps to compute 1/w instead of approximation
    movaps  xmm0, TL_one
    divps   xmm0, xmm7
#else
    rcpps   xmm0, xmm7                      // recip approx 
    mulps   xmm7, xmm0                      // x * rcpps(x)
    mulps   xmm7, xmm0                      // (x * rcpps(x)) * rcpps(x)
    addps   xmm0, xmm0                      // 2 * rcpps(x)
    subps   xmm0, xmm7                      // rhw = 2 * rcppx(x) - ((x * rcpps(x)) * rcpps(x))
#endif
    movaps  xmm1, [ecx]_STL.SOAhv.x    
    movaps  xmm2, [ecx]_STL.SOAhv.y
    movaps  xmm3, [ecx]_STL.SOAhv.z
    mulps   xmm1, xmm0                              // Mult X, Y and Z through RHW
    mulps   xmm2, xmm0
    mulps   xmm3, xmm0
    movaps  xmm4, [ecx]_STL.fSOAviewDataScaleX      // Load SOA Scale X from TMP
    movaps  xmm5, [ecx]_STL.fSOAviewDataScaleY      // Load SOA Scale Y from TMP
    movaps  xmm6, [ecx]_STL.fSOAviewDataScaleZ      // Load SOA Scale Z from TMP
    mulps   xmm1, xmm4                              // Mult X, Y and Z through scales
    mulps   xmm2, xmm5
    mulps   xmm3, xmm6
    movaps  xmm4, [ecx]_STL.fSOAviewDataOffsetX     // Load SOA Offset X from TMP (has HW offset pre-added)
    movaps  xmm5, [ecx]_STL.fSOAviewDataOffsetY     // Load SOA Offset Y from TMP (has HW offset pre-added)
    movaps  xmm6, [ecx]_STL.fSOAviewDataOffsetZ     // Load SOA Offset Z from TMP
    addps   xmm1, xmm4                              // Add X, Y Z to the scale offsets 
    movaps  xmm7, [ecx]_STL.fSOAhwZScale            // Get the SOA HW Z Scale
    addps   xmm2, xmm5
    addps   xmm3, xmm6
    movaps  [ecx]_STL.fSOAwfbi, xmm0                // wfbi 
    movaps  [ecx]_STL.SOAsv.w, xmm0                 // w
    mulps   xmm3, xmm7
    movaps  [ecx]_STL.SOAsv.x, xmm1                 // Write out the Napalm Dev Coords X & Y
    movaps  [ecx]_STL.SOAsv.y, xmm2
    movaps  [ecx]_STL.SOAsv.z, xmm3                 // Write out the Napalm Dev Coords Z
  }
} 


/* These two macros	do the Napalm device-specific calculations to the vertex */
#define FP_Xform_DevCoord_WBuf_SOA_MAC( pTL, pNext )												\
    _asm rcpps   	xmm3, xmm7                      /* recip approx */								\
	_asm prefetchnta [pNext]_STL.SOAhv.z 															\
    _asm mulps   	xmm7, xmm3                      /* x * rcpps(x)	*/								\
    _asm mulps   	xmm6, [pTL]_STL.fSOAviewDataScaleZ	/* Mult X, Y and Z through scales */		\
    _asm mulps   	xmm4, [pTL]_STL.fSOAviewDataScaleX												\
    _asm mulps   	xmm7, xmm3                      /* (x * rcpps(x)) * rcpps(x) */					\
    _asm mulps   	xmm5, [pTL]_STL.fSOAviewDataScaleY												\
    _asm addps   	xmm3, xmm3                      /* 2 * rcpps(x) */								\
    _asm movaps  	xmm1, [pTL]_STL.fSOAwbuffScale  /* Load SOA W buffer scale */					\
    _asm subps   	xmm3, xmm7                      /* rhw = 2 * rcppx(x) - ((x * rcpps(x)) * rcpps(x)) */\
																									\
    _asm mulps   	xmm6, xmm3                      /* Mult X, Y and Z through RHW */				\
    _asm mulps   	xmm4, xmm3																		\
    _asm mulps   	xmm5, xmm3																		\
    _asm addps   	xmm6, [pTL]_STL.fSOAviewDataOffsetZ												\
    _asm mulps   	xmm1, xmm3																		\
    _asm addps   	xmm4, [pTL]_STL.fSOAviewDataOffsetX	/* Add X, Y Z to the scale offsets */		\
    _asm addps   	xmm5, [pTL]_STL.fSOAviewDataOffsetY												\
    _asm mulps   	xmm6, [pTL]_STL.fSOAhwZScale        /* Get the SOA HW Z Scale */				\
    _asm addps   	xmm1, [pTL]_STL.fSOAwbuffOffset		/* wfbi = w*scale+offset */

#define FP_Xform_DevCoord_NoWBuf_SOA_MAC( pTL, pNext )													\
    _asm rcpps   	xmm3, xmm7                      /* recip approx */									\
   	_asm prefetchnta [pNext]_STL.SOAhv.z																\
    _asm mulps   	xmm7, xmm3                      /* x * rcpps(x)	*/									\
    _asm mulps   	xmm6, [pTL]_STL.fSOAviewDataScaleZ	/* Mult X, Y and Z through scales */			\
    _asm mulps   	xmm7, xmm3                      /* (x * rcpps(x)) * rcpps(x) */						\
    _asm addps   	xmm3, xmm3                      /* 2 * rcpps(x) */									\
    _asm mulps   	xmm4, [pTL]_STL.fSOAviewDataScaleX													\
    _asm mulps   	xmm5, [pTL]_STL.fSOAviewDataScaleY													\
    _asm subps   	xmm3, xmm7                      /* rhw = 2 * rcppx(x) - ((x * rcpps(x)) * rcpps(x)) */\
    _asm mulps   	xmm6, xmm3                      /* Mult X, Y and Z through RHW */					\
    _asm mulps   	xmm4, xmm3																			\
    _asm mulps   	xmm5, xmm3																			\
    _asm addps   	xmm6, [pTL]_STL.fSOAviewDataOffsetZ													\
    _asm addps   	xmm4, [pTL]_STL.fSOAviewDataOffsetX	/* Add X, Y Z to the scale offsets */			\
    _asm addps   	xmm5, [pTL]_STL.fSOAviewDataOffsetY													\
    _asm mulps   	xmm6, [pTL]_STL.fSOAhwZScale        /* Get the SOA HW Z Scale */					



void XformDeswiz4_DevCoord_SOA_GEOM_WBuff(RC *pRc, DWORD *pSrcVert, DWORD *pDstVert)
{
  __asm
  {
	mov			edi, 4 //VertCntSOA
	mov			esi, dword ptr [pRc]			// pRc
	mov			ebx, dword ptr [pSrcVert]		// pPos
	mov			ecx, dword ptr [pDstVert]
	mov			edx, [esi]RC.tl.pTL

LoopStart:
	//FP_Xform_4Vert_SOA(RC *pRc, DWORD *pSrcVert)
	mov			eax, [esi]RC.tl.lpSOAxfmCurrent	// pMat4x4x4
	Xform_4_NOSTORE_SOA_MAC( esi, eax, ebx )		// pRc,pMat,pSrc   h.xyzw in xmm4567

	//Xform_DevCoord
	FP_Xform_DevCoord_WBuf_SOA_MAC(edx, ebx)

	// DeSwizzleSOAtoTLBN_GEOM(RC *pRc, DWORD vAIdx)
	// Don't need to re-read position from the TMP_TL
	movaps		xmm2, xmm4							// xmm2 = x3    x2    x1    x0
	movss		[ecx]TLBN.w+(0 * SIZE TLBN), xmm3	// w0
	movlhps		xmm2, xmm5							// xmm2 = y1    y0    x1    x0
	movhlps		xmm7, xmm3							// xmm7 = --    ---   w3    w2
	mov		    dword ptr [ecx]TLBN.tex+(0 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	movhlps		xmm5, xmm4							// xmm5 = y3    y2    x3    x2
	movss		[ecx]TLBN.w+(2 * SIZE TLBN), xmm7	// w2
	shufps		xmm2, xmm2, 0xd8					// xmm2 = y1    x1    y0    x0
	shufps		xmm5, xmm5, 0xd8					// xmm5 = y3    x3    y2    x2
	mov		    dword ptr [ecx]TLBN.tex+(1 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	movlps		[ecx]TLBN.x+(0 * SIZE TLBN), xmm2	// y0    x0
	movhps		[ecx]TLBN.x+(1 * SIZE TLBN), xmm2	// y1    x1

	movaps		xmm2, xmm6							// xmm2 = z3    z2    z1    z0
	shufps		xmm3, xmm3, 0xb1					// xmm3 = w2    w3    w0    w1
	mov		    dword ptr [ecx]TLBN.tex+(2 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	movlps		[ecx]TLBN.x+(2 * SIZE TLBN), xmm5	// y2    x2
	movhps		[ecx]TLBN.x+(3 * SIZE TLBN), xmm5	// y3    x3
	movhlps		xmm2, xmm1							// xmm2 = z3    z2    wfbi3 wfbi2
	movlhps		xmm6, xmm1							// xmm6 = wfbi1 wfbi0 z1    z0
	movss		[ecx]TLBN.w+(1 * SIZE TLBN), xmm3	// w1
	shufps		xmm6, xmm6, 0xd8					// xmm6 = wfbi1 z1    wfbi0 z0
	shufps		xmm2, xmm2, 0x72					// xmm2 = wfbi3 z3    wfbi2 z2
	mov		    dword ptr [ecx]TLBN.tex+(3 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	movlps		[ecx]TLBN.z+(0 * SIZE TLBN), xmm6	// wfbi0 z0
	movhps		[ecx]TLBN.z+(1 * SIZE TLBN), xmm6	// wfbi1 z1
	movhlps		xmm3, xmm3							// xmm3 = w2    w3    w2    w3
	movlps		[ecx]TLBN.z+(2 * SIZE TLBN), xmm2	// wfbi2 z2
	movhps		[ecx]TLBN.z+(3 * SIZE TLBN), xmm2	// wfbi3 z3
	movss		[ecx]TLBN.w+(3 * SIZE TLBN), xmm3	// w3

	dec			edi
	lea			ecx, [ecx+(SIZE TLBN*4)]
	jnz			LoopStart
  }
} 


void XformDeswiz4_DevCoord_SOA_GEOM_NoWBuff(RC *pRc, DWORD *pSrcVert, DWORD *pDstVert)
{
  __asm
  {
	mov			edi, 4 //VertCntSOA
	mov			esi, dword ptr [pRc]			// pRc
	mov			ebx, dword ptr [pSrcVert]		// pPos
	mov			ecx, dword ptr [pDstVert]
	mov			edx, [esi]RC.tl.pTL

LoopStart:
	//FP_Xform_4Vert_SOA(RC *pRc, DWORD *pSrcVert)
	mov			eax, [esi]RC.tl.lpSOAxfmCurrent	// pMat4x4x4
	Xform_4_NOSTORE_SOA_MAC( esi, eax, ebx )		// pRc,pMat,pSrc   h.xyzw in xmm4567

	//Xform_DevCoord
	FP_Xform_DevCoord_NoWBuf_SOA_MAC(edx, ebx)

	// DeSwizzleSOAtoTLBN_GEOM(RC *pRc, DWORD vAIdx)
	// Don't need to re-read position from the TMP_TL
	// Be carefull here, w and wfbi are the same
	movaps		xmm2, xmm4							// xmm2 = x3    x2    x1    x0
	movss		[ecx]TLBN.w+(0 * SIZE TLBN), xmm3	// w0
	movlhps		xmm2, xmm5							// xmm2 = y1    y0    x1    x0
	movhlps		xmm5, xmm4							// xmm5 = y3    y2    x3    x2
	movhlps		xmm7, xmm3							// xmm7 = --    ---   w3    w2
	shufps		xmm2, xmm2, 0xd8					// xmm2 = y1    x1    y0    x0
	movaps		xmm4, xmm3							// xmm4 = wfbi3 wfbi2 wfbi1 wfbi0
	mov		    dword ptr [ecx]TLBN.tex+(0 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	shufps		xmm5, xmm5, 0xd8					// xmm5 = y3    x3    y2    x2
	movlps		[ecx]TLBN.x+(0 * SIZE TLBN), xmm2	// y0    x0
	movlhps		xmm4, xmm6							// xmm4 = z1    z0    wfbi1 wfbi0
	movhps		[ecx]TLBN.x+(1 * SIZE TLBN), xmm2	// y1    x1
	movhlps		xmm6, xmm3							// xmm6 = z3    z2    wfbi3 wfbi2
	movss		[ecx]TLBN.w+(2 * SIZE TLBN), xmm7	// w2
	shufps		xmm3, xmm3, 0xb1					// xmm3 = z2    z3    z0    z1
	mov		    dword ptr [ecx]TLBN.tex+(1 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	movlps		[ecx]TLBN.x+(2 * SIZE TLBN), xmm5	// y2    x2
	shufps		xmm4, xmm4, 0x72					// xmm4 = wfbi1 z1    wfbi0 z0
	mov		    dword ptr [ecx]TLBN.tex+(2 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	movhps		[ecx]TLBN.x+(3 * SIZE TLBN), xmm5	// y3    x3
	shufps		xmm6, xmm6, 0x72					// xmm6 = wfbi3 z3    wfbi2 z2
	mov		    dword ptr [ecx]TLBN.tex+(3 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	movss		[ecx]TLBN.w+(1 * SIZE TLBN), xmm3	// w1

	movlps		[ecx]TLBN.z+(0 * SIZE TLBN), xmm4	// wfbi0 z0
	movhps		[ecx]TLBN.z+(1 * SIZE TLBN), xmm4	// wfbi1 z1
	movhlps		xmm3, xmm3							// xmm3 = w2    w3    w2    w3
	movlps		[ecx]TLBN.z+(2 * SIZE TLBN), xmm6	// wfbi2 z2
	movhps		[ecx]TLBN.z+(3 * SIZE TLBN), xmm6	// wfbi3 z3
	movss		[ecx]TLBN.w+(3 * SIZE TLBN), xmm3	// w3

	dec			edi
	lea			ecx, [ecx+(SIZE TLBN*4)]
	jnz			LoopStart
  }
} 

void XformDeswiz4_DevCoord_SOA_GEOM_WBuff_Clipped(RC *pRc, DWORD *pSrcVert, DWORD *pDstVert)
{
  __asm
  {
	mov			edi, 4 //VertCntSOA
	mov			esi, dword ptr [pRc]			// pRc
	mov			ebx, dword ptr [pSrcVert]		// pPos
	mov			ecx, dword ptr [pDstVert]
	mov			edx, [esi]RC.tl.pTL

LoopStart:
	//FP_Xform_4Vert_SOA(RC *pRc, DWORD *pSrcVert)
	mov			eax, [esi]RC.tl.lpSOAxfmCurrent	// pMat4x4x4
	Xform_4_SOA_MAC( esi, eax, ebx, edx )		// pRc,pMat,pSrc,pDst   h.xyzw in xmm4567

	sub			esp, 8
	mov			[esp+4], ecx			// push ecx (just need to save it)
	mov			[esp], esi				// push pRc
	call		ComputeClipCodes_SOA
	mov			edx, [esi]RC.tl.pTL
	mov			ecx, [esp+4]			// restore ecx
	add			esp, 8
    movaps  	xmm4, [edx]_STL.SOAhv.x    
    movaps  	xmm5, [edx]_STL.SOAhv.y
    movaps  	xmm6, [edx]_STL.SOAhv.z
    movaps  	xmm7, [edx]_STL.SOAhv.w    		// Grab SOA vertex info for all Homogeneous Components

	//Xform_DevCoord
	FP_Xform_DevCoord_WBuf_SOA_MAC(edx, ebx)

	// DeSwizzleSOAtoTLBN_GEOM(RC *pRc, DWORD vAIdx)
	// Don't need to re-read position from the TMP_TL
	movaps		xmm2, xmm4							// xmm2 = x3    x2    x1    x0
	movlhps		xmm2, xmm5							// xmm2 = y1    y0    x1    x0
	movhlps		xmm5, xmm4							// xmm5 = y3    y2    x3    x2
	movaps		xmm0, [edx]_STL.dSOAclip_code		// xmm0 = cc3   cc2   cc1   cc0
	shufps		xmm2, xmm2, 0xd8					// xmm2 = y1    x1    y0    x0
	mov		    dword ptr [ecx]TLBN.tex+(0 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	shufps		xmm5, xmm5, 0xd8					// xmm5 = y3    x3    y2    x2
	movlps		[ecx]TLBN.x+(0 * SIZE TLBN), xmm2	// y0    x0
	movhps		[ecx]TLBN.x+(1 * SIZE TLBN), xmm2	// y1    x1

	movaps		xmm4, xmm6							// xmm4 = z3    z2    z1    z0
	movlhps		xmm6, xmm1							// xmm6 = wfbi1 wfbi0 z1    z0
	mov		    dword ptr [ecx]TLBN.tex+(1 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	movhlps		xmm4, xmm1							// xmm4 = z3    z2    wfbi3 wfbi2
	movlps		[ecx]TLBN.x+(2 * SIZE TLBN), xmm5	// y2    x2
	movhps		[ecx]TLBN.x+(3 * SIZE TLBN), xmm5	// y3    x3
	shufps		xmm6, xmm6, 0xd8					// xmm6 = wfbi1 z1    wfbi0 z0
	mov		    dword ptr [ecx]TLBN.tex+(2 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	shufps		xmm4, xmm4, 0x72					// xmm4 = wfbi3 z3    wfbi2 z2
	movlps		[ecx]TLBN.z+(0 * SIZE TLBN), xmm6		// wfbi0 z0
	movhps		[ecx]TLBN.z+(1 * SIZE TLBN), xmm6		// wfbi1 z1

 	movaps		xmm2, xmm3							// xmm2 = w3    w2    w1    w0
	movlhps		xmm2, xmm0							// xmm2 = cc1   cc0   w1    w0
	movhlps		xmm3, xmm0							// xmm3 = w3    w2    cc3   cc3
	movlps		[ecx]TLBN.z+(2 * SIZE TLBN), xmm4		// wfbi2 z2
	movhps		[ecx]TLBN.z+(3 * SIZE TLBN), xmm4		// wfbi3 z3
	shufps		xmm2, xmm2, 0xd8					// xmm2 = cc1   w1    cc0   w0
	mov		    dword ptr [ecx]TLBN.tex+(3 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	shufps		xmm3, xmm3, 0x72					// xmm3 = cc3   w3    cc2   w2
	movlps		[ecx]TLBN.w+(0 * SIZE TLBN), xmm2	// w0    cc0
	movhps		[ecx]TLBN.w+(1 * SIZE TLBN), xmm2	// w1    cc1
	movlps		[ecx]TLBN.w+(2 * SIZE TLBN), xmm3	// w2    cc2
	movhps		[ecx]TLBN.w+(3 * SIZE TLBN), xmm3	// w3    cc3

	dec			edi
	lea			ecx, [ecx+(SIZE TLBN * 4)]
	jnz			LoopStart
  }
} 


void XformDeswiz4_DevCoord_SOA_GEOM_NoWBuff_Clipped(RC *pRc, DWORD *pSrcVert, DWORD *pDstVert)
{
  __asm
  {
	mov			edi, 4 //VertCntSOA
	mov			esi, dword ptr [pRc]			// pRc
	mov			ebx, dword ptr [pSrcVert]		// pPos
	mov			ecx, dword ptr [pDstVert]
	mov			edx, [esi]RC.tl.pTL

LoopStart:
	//FP_Xform_4Vert_SOA(RC *pRc, DWORD *pSrcVert)
	mov			eax, [esi]RC.tl.lpSOAxfmCurrent	// pMat4x4x4
	Xform_4_SOA_MAC( esi, eax, ebx, edx )		// pRc,pMat,pSrc,pDst   h.xyzw in xmm4567

	sub			esp, 8
	mov			[esp+4], ecx			// push ecx (just need to save it)
	mov			[esp], esi				// push pRc
	call		ComputeClipCodes_SOA
	mov			edx, [esi]RC.tl.pTL
	mov			ecx, [esp+4]			// restore ecx
	add			esp, 8
    movaps  	xmm4, [edx]_STL.SOAhv.x    
    movaps  	xmm5, [edx]_STL.SOAhv.y
    movaps  	xmm6, [edx]_STL.SOAhv.z
    movaps  	xmm7, [edx]_STL.SOAhv.w    		// Grab SOA vertex info for all Homogeneous Components

	//Xform_DevCoord
	FP_Xform_DevCoord_NoWBuf_SOA_MAC(edx, ebx)

	// DeSwizzleSOAtoTLBN_GEOM(RC *pRc, DWORD vAIdx)
	// Don't need to re-read position from the TMP_TL
	movaps		xmm2, xmm4							// xmm2 = x3    x2    x1    x0
	movlhps		xmm2, xmm5							// xmm2 = y1    y0    x1    x0
	movhlps		xmm5, xmm4							// xmm5 = y3    y2    x3    x2
	movaps		xmm0, [edx]_STL.dSOAclip_code		// xmm0 = cc3   cc2   cc1   cc0
	shufps		xmm2, xmm2, 0xd8					// xmm2 = y1    x1    y0    x0
	mov		    dword ptr [ecx]TLBN.tex+(0 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	shufps		xmm5, xmm5, 0xd8					// xmm5 = y3    x3    y2    x2
	movlps		[ecx]TLBN.x+(0 * SIZE TLBN), xmm2	// y0    x0
	movhps		[ecx]TLBN.x+(1 * SIZE TLBN), xmm2	// y1    x1

	movaps		xmm4, xmm6							// xmm4 = z3    z2    z1    z0
	movlhps		xmm6, xmm3							// xmm6 = wfbi1 wfbi0 z1    z0
	movhlps		xmm4, xmm3							// xmm4 = z3    z2    wfbi3 wfbi2
	movlps		[ecx]TLBN.x+(2 * SIZE TLBN), xmm5	// y2    x2
	movhps		[ecx]TLBN.x+(3 * SIZE TLBN), xmm5	// y3    x3
	shufps		xmm6, xmm6, 0xd8					// xmm6 = wfbi1 z1    wfbi0 z0
	mov		    dword ptr [ecx]TLBN.tex+(1 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	shufps		xmm4, xmm4, 0x72					// xmm4 = wfbi3 z3    wfbi2 z2
	movlps		[ecx]TLBN.z+(0 * SIZE TLBN), xmm6	// wfbi0 z0
	movhps		[ecx]TLBN.z+(1 * SIZE TLBN), xmm6	// wfbi1 z1

	movaps		xmm2, xmm3							// xmm2 = w3    w2    w1    w0
	mov		    dword ptr [ecx]TLBN.tex+(2 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	movlhps		xmm2, xmm0							// xmm2 = cc1   cc0   w1    w0
	movhlps		xmm0, xmm3							// xmm0 = cc3   cc3   w3    w2
	movlps		[ecx]TLBN.z+(2 * SIZE TLBN), xmm4	// wfbi2 z2
	movhps		[ecx]TLBN.z+(3 * SIZE TLBN), xmm4	// wfbi3 z3
	shufps		xmm2, xmm2, 0xd8					// xmm2 = cc1   w1    cc0   w0
	mov		    dword ptr [ecx]TLBN.tex+(3 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
	shufps		xmm0, xmm0, 0xd8					// xmm0 = cc3   w3    cc2   w2
	movlps		[ecx]TLBN.w+(0 * SIZE TLBN), xmm2	// cc0   w0
	movhps		[ecx]TLBN.w+(1 * SIZE TLBN), xmm2	// cc1   w1
	movlps		[ecx]TLBN.w+(2 * SIZE TLBN), xmm0	// cc2   w2
	movhps		[ecx]TLBN.w+(3 * SIZE TLBN), xmm0	// cc3   w3

	dec			edi
	lea			ecx, [ecx+(SIZE TLBN * 4)]
	jnz			LoopStart
  }
} 




/*-------------------------------------------------------------------
Function Name:  Xform_DeSwiz_DevCoord_SOA_TEX
Description:    Convert homogeneous coordinates to device coordinates
Parameters:   
Information:    Non-clipped tri's
Return:         
-------------------------------------------------------------------*/
void Xform_DeSwiz_DevCoord_SOA_TEX(RC *pRc, DWORD dwSOAIdx, SOA_UV* pTex)
{
  __asm 
  {
	mov		eax, dwSOAIdx
    mov     esi, pRc                            	// esi = pointer to pRc
	and		eax, 0xfffffffc							// SOA_IDX( vIdx )
	shl		eax, 4									// SOA_IDX( vIdx ) * TLBN_SIZE (48 bytes)
	lea		eax, [eax+eax*2]

    mov     edi, [esi]RC.tl.pTL                 	// edi = &TL_SOATMP
    mov     edx, [esi]RC.tl.KniRC.RfBits
	add		eax, [esi]RC.tl.TLBN.lpvData			// pTLBN_group

/**************************
* Deswizzle Colors
**************************/
	movlps  	xmm6, [edi]_STL.dSOAdiff+0			// xmm6 = --    --    d1    d0
	movhps  	xmm6, [edi]_STL.dSOAspec+0			// xmm6 = s1    s0    d1    d0
	movlps  	xmm7, [edi]_STL.dSOAdiff+8			// xmm7 = --    --    d3    d2
	movhps  	xmm7, [edi]_STL.dSOAspec+8			// xmm7 = s3    s2    d3    d2
	shufps		xmm6, xmm6, 0xd8	  				// xmm6 = s1    d1    s0    d0
	shufps		xmm7, xmm7, 0xd8	  				// xmm7 = s3    d3    s2    d2
	movlps		[eax]TLBN.diffuse+(0 * SIZE TLBN), xmm6	// diff0 spec0
	movhps		[eax]TLBN.diffuse+(1 * SIZE TLBN), xmm6	// diff1 spec1
	movlps		[eax]TLBN.diffuse+(2 * SIZE TLBN), xmm7	// diff2 spec2
	movhps		[eax]TLBN.diffuse+(3 * SIZE TLBN), xmm7	// diff3 spec3

/**************************
* Fog
**************************/
	test	edx, BIT_RC_VERTEX_FOG
	je		FogDone
    movaps	xmm4, TL_255
	test	edx, BIT_RC_REQUIRES_WBUFFER
    subps	xmm4, [edi]_STL.fSOAVertFog				// wfbi = 255 - spec.alpha
	jne		Wbuffer

	// No Wbuffer
	movhlps	xmm5, xmm4								// xmm5 = --    --    wfbi3 wfbi2
	movss	[eax]TLBN.wfbi+(0 * SIZE TLBN), xmm4	// wfbi0
	shufps	xmm4, xmm4, 0xe1						// xmm4 = --    --    wfbi0 wfbi1
	movss	[eax]TLBN.wfbi+(2 * SIZE TLBN), xmm5	// wfbi2
	shufps	xmm5, xmm5, 0xe1						// xmm5 = --    --    wfbi2 wfbi3
	movss	[eax]TLBN.wfbi+(1 * SIZE TLBN), xmm4	// wfbi1
	movss	[eax]TLBN.wfbi+(3 * SIZE TLBN), xmm5	// wfbi3
	jmp		FogDone

Wbuffer:
    mulps	xmm4, TL_256							// z = (255 - spec.alpha) * 256
	movhlps	xmm5, xmm4								// xmm5 = --    --    z3    z2
	movss	[eax]TLBN.z+(0 * SIZE TLBN), xmm4		// z0
	shufps	xmm4, xmm4, 0xe1						// xmm4 = --    --    z0    z1
	movss	[eax]TLBN.z+(2 * SIZE TLBN), xmm5		// z2
	shufps	xmm5, xmm5, 0xe1						// xmm5 = --    --    z2    z3
	movss	[eax]TLBN.z+(1 * SIZE TLBN), xmm4		// z1
	movss	[eax]TLBN.z+(3 * SIZE TLBN), xmm5		// z3
FogDone:


/*************************************************************************************
*
* Textures
*
*************************************************************************************/
	// eax=*pDst, edx=KniRC.RfBits, esi=*pRc, edi=TL_SOATMP
	test	edx, (BIT_RC_REQUIRES_TX0 | BIT_RC_REQUIRES_TX1)
    jz      NoTextures

    // Load w from the TLBN if we need to perform perspective divide. 
    // We do not do perspective divide with texture wrap (rendering code must do it)
	test	[esi]RC.tl.dwTLState, TLPV_DO_PROSPECTIVE_DIVIDE
	mov		ecx, [esi+0]RC.tl.dwTexCoordIndex	//TCI = pRc->tl.dwTexCoordIndex[0]
	mov		ebx, pTex
	jz		W_Loaded

    // Get the w values from the TLBN
	// use movlhs/movhps rather than movss, it's faster and w is qword aligned
    //mov		ebx, [esi]RC.tl.TLBN.dwStride	// Need the TLBN stride (not anymore, now it's always the same)
    movlps	xmm6, [eax+(0*(SIZE TLBN))]TLBN.w	// --  --  --  w0
    movhps	xmm6, [eax+(1*(SIZE TLBN))]TLBN.w	// --  w1  --  w0
    movlps	xmm7, [eax+(2*(SIZE TLBN))]TLBN.w	// --  --  --  w2
    movhps	xmm7, [eax+(3*(SIZE TLBN))]TLBN.w	// --  w3  --  w2
W_Loaded:

	test	ecx, 0xffff0000						// D3DTSS_TCI_PASSTHRU
	jnz		Tx0_Chk_CameralSpacePosition

	and		ecx, (0x0000ffff & (D3DDP_MAXTEXCOORD-1))
	shl		ecx, 5								// 8*4 = 2^5
	add		ecx, ebx							// ecx = pTex + TCI[0]*32
	test	[esi+0]RC.tl.dwTexXformFlags, (D3DTTFF_PROJECTED-1)
	movaps	xmm0, [ecx]							// xmm0 =	t1		s1		t0		s0
	movaps	xmm1, [ecx+16]						// xmm1 =	t3		s3		t2		s2
	jz		Tx0_SwizzleDone						// special case (most common)
	// swizzle into soa format, load z & w values
	movaps	xmm2, [TL_soa_1]					// xmm2 =	1.0		1.0		1.0		1.0
	movaps	xmm4, xmm0							// xmm4 =	t1		s1		t0		s0
	xorps	xmm3, xmm3							// xmm3 =	0		0		0		0
	shufps	xmm0, xmm1, 0x88					// xmm0 =	s3		s2		s1		s0
	shufps	xmm4, xmm1, 0xdd					// xmm4 =	t3		t2		t1		t0
	movaps	xmm1, xmm4							// xmm1 =	t3		t2		t1		t0
	jmp		Tx0_Loaded

Tx0_Chk_CameralSpacePosition:
	and		ecx, 0xffff0000
	cmp		ecx, D3DTSS_TCI_CAMERASPACENORMAL
	je		Tx0_CameralSpaceNormal

	movaps  xmm0, [edi]_STL.SOAcv.x				// xmm0 =	cv3.x	cv2.x	cv1.x	cv0.x
	movaps  xmm1, [edi]_STL.SOAcv.y				// xmm1 =	cv3.y	cv2.y	cv1.y	cv0.y
	movaps  xmm2, [edi]_STL.SOAcv.z				// xmm2 =	cv3.z	cv2.z	cv1.z	cv0.z
	cmp		ecx, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR
	je		Tx0_CameralSpaceReflectionVector
	movaps	xmm3, [TL_soa_1]					// xmm3 =	1.0		1.0		1.0		1.0
	jmp		Tx0_DoTxXform

Tx0_CameralSpaceReflectionVector:
	test	[ecx]RC.tl.dwTLState, TLPV_LOCALVIEWER
	je		Tx0_CameralSpaceReflectionVector_InfViewer
	mulps	xmm0, xmm0							// xmm0 = x*x
	mulps	xmm1, xmm1							// xmm1 = y*y
	mulps	xmm2, xmm2							// xmm2 = z*z
	addps	xmm1, xmm0
	addps	xmm2, xmm1							// xmm2 = x*x + y*y + z*z
	movaps  xmm3, [edi]_STL.SOAcv.x				// xmm3 =	cv3.x	cv2.x	cv1.x	cv0.x
	movaps  xmm4, [edi]_STL.SOAcv.y				// xmm4 =	cv3.y	cv2.y	cv1.y	cv0.y
	movaps  xmm5, [edi]_STL.SOAcv.z				// xmm5 =	cv3.z	cv2.z	cv1.z	cv0.z
	rsqrtps	xmm2, xmm2							// xmm2 = 1/sqrt(x*x + y*y + z*z)
	mulps	xmm3, xmm2							// xmm3 = norm(cv.x)
	mulps	xmm4, xmm2							// xmm4 = norm(cv.y)
	mulps	xmm5, xmm2							// xmm5 = norm(cv.z)
	movaps	xmm0, xmm3							// xmm0 = norm(cv.x)
	movaps	xmm1, xmm4							// xmm1 = norm(cv.y)
	movaps	xmm2, xmm5							// xmm2 = norm(cv.z)
	mulps  	xmm3, [edi]_STL.SOAcn.x				// xmm3 = norm(cv.x) * cn.x
	mulps  	xmm4, [edi]_STL.SOAcn.y				// xmm4 = norm(cv.y) * cn.y
	mulps  	xmm5, [edi]_STL.SOAcn.z				// xmm5 = norm(cv.z) * cn.z
	addps	xmm4, xmm3
	addps	xmm5, xmm4							// xmm5 = norm(cv.x) * cn.x	+ norm(cv.y) * cn.y + norm(cv.z) * cn.z
	mulps	xmm5, TL_soa_2						// xmm5 = fDot2 = 2.0 * (norm(cv.x) * cn.x	+ norm(cv.y) * cn.y + norm(cv.z) * cn.z)
	movaps	xmm3, xmm5							// xmm3 = fDot2
	movaps	xmm4, xmm5							// xmm4 = fDot2
	mulps	xmm3, [edi]_STL.SOAcn.x				// xmm3 = cn.x * fDot2
	mulps	xmm4, [edi]_STL.SOAcn.y				// xmm4 = cn.y * fDot2
	mulps	xmm5, [edi]_STL.SOAcn.z				// xmm5 = cn.z * fDot2
	subps	xmm0, xmm3							// xmm0 = norm(cv.x) - (cn.x * fDot2)
	subps	xmm1, xmm4							// xmm1 = norm(cv.y) - (cn.y * fDot2)
	subps	xmm2, xmm5							// xmm2 = norm(cv.z) - (cn.z * fDot2)
	movaps	xmm3, TL_soa_1						// xmm3 =	1.0		1.0		1.0		1.0
	jmp		Tx0_Loaded

Tx0_CameralSpaceReflectionVector_InfViewer:
	movaps	xmm4, TL_soa_neg_2					// xmm4 =	-2.0	-2.0	-2.0	-2.0
	movaps  xmm5, [edi]_STL.SOAcn.z				// xmm5 =	cn3.z	cn2.z	cn1.z	cn0.z
	movaps  xmm0, [edi]_STL.SOAcn.x				// xmm0 =	cn3.x	cn2.x	cn1.x	cn0.x
	movaps  xmm1, [edi]_STL.SOAcn.y				// xmm1 =	cn3.y	cn2.y	cn1.y	cn0.y
	mulps	xmm4, xmm5							// xmm4 = -fDot2 = -2.0 * cn.z
	movaps	xmm2, TL_soa_1						// xmm2 =	1.0		1.0		1.0		1.0
	movaps	xmm3, TL_soa_1						// xmm3 =	1.0		1.0		1.0		1.0
	mulps	xmm5, xmm4							// xmm5 = -fDot2 * cn.z
	mulps	xmm0, xmm4							// xmm0 = -fDot2 * cn.x
	mulps	xmm1, xmm4							// xmm1 = -fDot2 * cn.y
	addps	xmm2, xmm5							// xmm2 = 1.0 - (fDot2 * cn.z)
	jmp		Tx0_Loaded

Tx0_CameralSpaceNormal:
	movaps  xmm0, [edi]_STL.SOAcn.x				// xmm0 =	cn3.x	cn2.x	cn1.x	cn0.x
	movaps  xmm1, [edi]_STL.SOAcn.y				// xmm1 =	cn3.y	cn2.y	cn1.y	cn0.y
	movaps  xmm2, [edi]_STL.SOAcn.z				// xmm2 =	cn3.z	cn2.z	cn1.z	cn0.z
	movaps	xmm3, TL_soa_1						// xmm3 =	1.0		1.0		1.0		1.0
Tx0_Loaded:


	// Perform TexTransform
	//if( (pRc->tl.dwTexXformFlags[0] & (D3DTTFF_PROJECTED-1)) != D3DTTFF_DISABLE )
	test	[esi+0]RC.tl.dwTexXformFlags, (D3DTTFF_PROJECTED-1)
	jz		Tx0_TransformDone
Tx0_DoTxXform:
	mov		ecx, [esi+0]RC.tl.lpTexXformMatrixSOA	// pRc->tl.lpxfmTxtrSOA[0]
	movaps	xmm4, xmm0							// xmm4 =	v3.x	v2.x	v1.x	v0.x
	movaps	xmm5, xmm1							// xmm5 =	v3.y	v2.y	v1.y	v0.y
	mulps	xmm0, [ecx]SOA_MATRIX._11			// xmm0 =	x*_11
	mulps	xmm4, [ecx]SOA_MATRIX._12			// xmm4 =	x*_12
	mulps	xmm1, [ecx]SOA_MATRIX._21			// xmm1 =	y*_21
	mulps	xmm5, [ecx]SOA_MATRIX._22			// xmm5 =	y*_22
	addps	xmm0, xmm1							// xmm0 =	x*_11 + y*_21
	addps	xmm4, xmm5							// xmm4 =	x*_12 + y*_22
	movaps	xmm1, xmm2							// xmm1 =	v3.z	v2.z	v1.z	v0.z
	movaps	xmm5, xmm3							// xmm5 =	v3.w	v2.w	v1.w	v0.w
	mulps	xmm2, [ecx]SOA_MATRIX._31			// xmm2 =	z*_31
	mulps	xmm1, [ecx]SOA_MATRIX._32			// xmm1 =	z*_32
	mulps	xmm3, [ecx]SOA_MATRIX._41			// xmm3 =	z*_41
	addps	xmm0, xmm2							// xmm0 =	x*_11 + y*_21 + z*_31
	mulps	xmm5, [ecx]SOA_MATRIX._42			// xmm1 =	z*_42
	addps	xmm1, xmm4							// xmm1 =	x*_12 + y*_22 + z*_32
	addps	xmm0, xmm3							// xmm0 =	s = x*_11 + y*_21 + z*_31 + w*_41
	addps	xmm1, xmm5							// xmm1 =	t = x*_12 + y*_22 + z*_32 + w*_42
Tx0_TransformDone:
	// convert the texture to our modified swizzle format
	movaps	xmm2, xmm0							// xmm2 =	s3		s2		s1		s0
	movlhps	xmm0, xmm1							// xmm0 =	t1		t0		s1		s0
	movhlps	xmm1, xmm2							// xmm1 =	t3		t2		s3		s2
	shufps	xmm0, xmm0, 0xd8					// xmm0 =	t1		s1		t0		s0
	shufps	xmm1, xmm1, 0xd8					// xmm1 =	t3		s3		t2		s2
Tx0_SwizzleDone:

	// scale, add the offset, and do the perspective divide
	mulps	xmm0, [edi]_STL.fSOAscaleST0
	mulps	xmm1, [edi]_STL.fSOAscaleST0
	test	[esi]RC.tl.dwTLState, TLPV_DO_PROSPECTIVE_DIVIDE
	addps	xmm0, [edi]_STL.fSOAcenterST0
	addps	xmm1, [edi]_STL.fSOAcenterST0
	jz		Tx0_PerspectiveCorrectDone
	shufps	xmm6, xmm6, 0xa0					// w1  w1  w0  w0
    shufps  xmm7, xmm7, 0xa0         			// w3  w3  w2  w2
	mulps	xmm0, xmm6
	mulps	xmm1, xmm7
Tx0_PerspectiveCorrectDone:
	mov		ecx, [esi+4]RC.tl.dwTexCoordIndex		//TCI = pRc->tl.dwTexCoordIndex[1]
	movlps	[eax+0]TLBN.tex+(0 * SIZE TLBN), xmm0	//	t0		s0
	movhps	[eax+0]TLBN.tex+(1 * SIZE TLBN), xmm0	//	t1		s1
	movlps	[eax+0]TLBN.tex+(2 * SIZE TLBN), xmm1	//	t2		s2
	movhps	[eax+0]TLBN.tex+(3 * SIZE TLBN), xmm1	//	t3		s3


	test	edx, BIT_RC_REQUIRES_TX1
	jz		Textures_Done

	mov		ecx, [esi+4]RC.tl.dwTexCoordIndex	//TCI = pRc->tl.dwTexCoordIndex[1]
	test	ecx, 0xffff0000						// D3DTSS_TCI_PASSTHRU
	jne		Tx1_Chk_CameralSpacePosition

	and		ecx, (0x0000ffff & (D3DDP_MAXTEXCOORD-1))
	shl		ecx, 5								// 8*4 = 2^5
	add		ecx, ebx							// ecx = pTex + TCI[1]*32
	test	[esi+4]RC.tl.dwTexXformFlags, (D3DTTFF_PROJECTED-1)
	movaps	xmm0, [ecx]							// xmm0 =	t1		s1		t0		s0
	movaps	xmm1, [ecx+16]						// xmm1 =	t3		s3		t2		s2
	jz		Tx1_SwizzleDone						// special case (most common)
	// swizzle into soa format, load z & w values
	movaps	xmm2, [TL_soa_1]					// xmm2 =	1.0		1.0		1.0		1.0
	movaps	xmm4, xmm0							// xmm4 =	t1		s1		t0		s0
	xorps	xmm3, xmm3							// xmm3 =	0		0		0		0
	shufps	xmm0, xmm1, 0x88					// xmm0 =	s3		s2		s1		s0
	shufps	xmm4, xmm1, 0xdd					// xmm4 =	t3		t2		t1		t0
	movaps	xmm1, xmm4							// xmm1 =	t3		t2		t1		t0
	jmp		Tx1_Loaded

Tx1_Chk_CameralSpacePosition:
	and		ecx, 0xffff0000
	cmp		ecx, D3DTSS_TCI_CAMERASPACENORMAL
	je		Tx1_CameralSpaceNormal

	movaps  xmm0, [edi]_STL.SOAcv.x				// xmm0 =	cv3.x	cv2.x	cv1.x	cv0.x
	movaps  xmm1, [edi]_STL.SOAcv.y				// xmm1 =	cv3.y	cv2.y	cv1.y	cv0.y
	movaps  xmm2, [edi]_STL.SOAcv.z				// xmm2 =	cv3.z	cv2.z	cv1.z	cv0.z
	cmp		ecx, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR
	je		Tx1_CameralSpaceReflectionVector
	movaps	xmm3, [TL_soa_1]					// xmm3 =	1.0		1.0		1.0		1.0
	jmp		Tx1_DoTxXform

Tx1_CameralSpaceReflectionVector:
	test	[ecx]RC.tl.dwTLState, TLPV_LOCALVIEWER
	je		Tx1_CameralSpaceReflectionVector_InfViewer
	mulps	xmm0, xmm0							// xmm0 = x*x
	mulps	xmm1, xmm1							// xmm1 = y*y
	mulps	xmm2, xmm2							// xmm2 = z*z
	addps	xmm1, xmm0
	addps	xmm2, xmm1							// xmm2 = x*x + y*y + z*z
	movaps  xmm3, [edi]_STL.SOAcv.x				// xmm3 =	cv3.x	cv2.x	cv1.x	cv0.x
	movaps  xmm4, [edi]_STL.SOAcv.y				// xmm4 =	cv3.y	cv2.y	cv1.y	cv0.y
	movaps  xmm5, [edi]_STL.SOAcv.z				// xmm5 =	cv3.z	cv2.z	cv1.z	cv0.z
	rsqrtps	xmm2, xmm2							// xmm2 = 1/sqrt(x*x + y*y + z*z)
	mulps	xmm3, xmm2							// xmm3 = norm(cv.x)
	mulps	xmm4, xmm2							// xmm4 = norm(cv.y)
	mulps	xmm5, xmm2							// xmm5 = norm(cv.z)
	movaps	xmm0, xmm3							// xmm0 = norm(cv.x)
	movaps	xmm1, xmm4							// xmm1 = norm(cv.y)
	movaps	xmm2, xmm5							// xmm2 = norm(cv.z)
	mulps  	xmm3, [edi]_STL.SOAcn.x				// xmm3 = norm(cv.x) * cn.x
	mulps  	xmm4, [edi]_STL.SOAcn.y				// xmm4 = norm(cv.y) * cn.y
	mulps  	xmm5, [edi]_STL.SOAcn.z				// xmm5 = norm(cv.z) * cn.z
	addps	xmm4, xmm3
	addps	xmm5, xmm4							// xmm5 = norm(cv.x) * cn.x	+ norm(cv.y) * cn.y + norm(cv.z) * cn.z
	mulps	xmm5, TL_soa_2						// xmm5 = fDot2 = 2.0 * (norm(cv.x) * cn.x	+ norm(cv.y) * cn.y + norm(cv.z) * cn.z)
	movaps	xmm3, xmm5							// xmm3 = fDot2
	movaps	xmm4, xmm5							// xmm4 = fDot2
	mulps	xmm3, [edi]_STL.SOAcn.x				// xmm3 = cn.x * fDot2
	mulps	xmm4, [edi]_STL.SOAcn.y				// xmm4 = cn.y * fDot2
	mulps	xmm5, [edi]_STL.SOAcn.z				// xmm5 = cn.z * fDot2
	subps	xmm0, xmm3							// xmm0 = norm(cv.x) - (cn.x * fDot2)
	subps	xmm1, xmm4							// xmm1 = norm(cv.y) - (cn.y * fDot2)
	subps	xmm2, xmm5							// xmm2 = norm(cv.z) - (cn.z * fDot2)
	movaps	xmm3, TL_soa_1						// xmm3 =	1.0		1.0		1.0		1.0
	jmp		Tx1_Loaded

Tx1_CameralSpaceReflectionVector_InfViewer:
	movaps	xmm4, TL_soa_neg_2					// xmm4 =	-2.0	-2.0	-2.0	-2.0
	movaps  xmm5, [edi]_STL.SOAcn.z				// xmm5 =	cn3.z	cn2.z	cn1.z	cn0.z
	movaps  xmm0, [edi]_STL.SOAcn.x				// xmm0 =	cn3.x	cn2.x	cn1.x	cn0.x
	movaps  xmm1, [edi]_STL.SOAcn.y				// xmm1 =	cn3.y	cn2.y	cn1.y	cn0.y
	mulps	xmm4, xmm5							// xmm4 = -fDot2 = -2.0 * cn.z
	movaps	xmm2, TL_soa_1						// xmm2 =	1.0		1.0		1.0		1.0
	movaps	xmm3, TL_soa_1						// xmm3 =	1.0		1.0		1.0		1.0
	mulps	xmm5, xmm4							// xmm5 = -fDot2 * cn.z
	mulps	xmm0, xmm4							// xmm0 = -fDot2 * cn.x
	mulps	xmm1, xmm4							// xmm1 = -fDot2 * cn.y
	addps	xmm2, xmm5							// xmm2 = 1.0 - (fDot2 * cn.z)
	jmp		Tx1_Loaded

Tx1_CameralSpaceNormal:
	movaps  xmm0, [edi]_STL.SOAcn.x				// xmm0 =	cn3.x	cn2.x	cn1.x	cn0.x
	movaps  xmm1, [edi]_STL.SOAcn.y				// xmm1 =	cn3.y	cn2.y	cn1.y	cn0.y
	movaps  xmm2, [edi]_STL.SOAcn.z				// xmm2 =	cn3.z	cn2.z	cn1.z	cn0.z
	movaps	xmm3, TL_soa_1						// xmm3 =	1.0		1.0		1.0		1.0
Tx1_Loaded:


	// Perform TexTransform
	//if( (pRc->tl.dwTexXformFlags[0] & (D3DTTFF_PROJECTED-1)) != D3DTTFF_DISABLE )
	test	[esi+4]RC.tl.dwTexXformFlags, (D3DTTFF_PROJECTED-1)
	jz		Tx1_TransformDone
Tx1_DoTxXform:
	mov		ecx, [esi+4]RC.tl.lpTexXformMatrixSOA	// pRc->tl.lpxfmTxtrSOA[1]
	movaps	xmm4, xmm0							// xmm4 =	v3.x	v2.x	v1.x	v0.x
	movaps	xmm5, xmm1							// xmm5 =	v3.y	v2.y	v1.y	v0.y
	mulps	xmm0, [ecx]SOA_MATRIX._11			// xmm0 =	x*_11
	mulps	xmm4, [ecx]SOA_MATRIX._12			// xmm4 =	x*_12
	mulps	xmm1, [ecx]SOA_MATRIX._21			// xmm1 =	y*_21
	mulps	xmm5, [ecx]SOA_MATRIX._22			// xmm5 =	y*_22
	addps	xmm0, xmm1							// xmm0 =	x*_11 + y*_21
	addps	xmm4, xmm5							// xmm4 =	x*_12 + y*_22
	movaps	xmm1, xmm2							// xmm1 =	v3.z	v2.z	v1.z	v0.z
	movaps	xmm5, xmm3							// xmm5 =	v3.w	v2.w	v1.w	v0.w
	mulps	xmm2, [ecx]SOA_MATRIX._31			// xmm2 =	z*_31
	mulps	xmm1, [ecx]SOA_MATRIX._32			// xmm1 =	z*_32
	mulps	xmm3, [ecx]SOA_MATRIX._41			// xmm3 =	z*_41
	addps	xmm0, xmm2							// xmm0 =	x*_11 + y*_21 + z*_31
	mulps	xmm5, [ecx]SOA_MATRIX._42			// xmm1 =	z*_42
	addps	xmm1, xmm4							// xmm1 =	x*_12 + y*_22 + z*_32
	addps	xmm0, xmm3							// xmm0 =	s = x*_11 + y*_21 + z*_31 + w*_41
	addps	xmm1, xmm5							// xmm1 =	t = x*_12 + y*_22 + z*_32 + w*_42
Tx1_TransformDone:
	// convert the texture to our modified swizzle format
	movaps	xmm2, xmm0							// xmm2 =	s3		s2		s1		s0
	movlhps	xmm0, xmm1							// xmm0 =	t1		t0		s1		s0
	movhlps	xmm1, xmm2							// xmm1 =	t3		t2		s3		s2
	shufps	xmm0, xmm0, 0xd8					// xmm0 =	t1		s1		t0		s0
	shufps	xmm1, xmm1, 0xd8					// xmm1 =	t3		s3		t2		s2
Tx1_SwizzleDone:

	// scale, add the offset, and do the perspective divide
	mulps	xmm0, [edi]_STL.fSOAscaleST1
	mulps	xmm1, [edi]_STL.fSOAscaleST1
	test	[esi]RC.tl.dwTLState, TLPV_DO_PROSPECTIVE_DIVIDE
	addps	xmm0, [edi]_STL.fSOAcenterST1
	addps	xmm1, [edi]_STL.fSOAcenterST1
	jz		Tx1_PerspectiveCorrectDone
	mulps	xmm0, xmm6
	mulps	xmm1, xmm7
Tx1_PerspectiveCorrectDone:
	movlps	[eax+8]TLBN.tex+(0 * SIZE TLBN), xmm0	// v01  u01
	movhps	[eax+8]TLBN.tex+(1 * SIZE TLBN), xmm0	// v11  u11
	movlps	[eax+8]TLBN.tex+(2 * SIZE TLBN), xmm1	// v21  u21
	movhps	[eax+8]TLBN.tex+(3 * SIZE TLBN), xmm1	// v31  u31
    jmp     Textures_Done

NoTextures:
	// store 0's in the 's' of the first texture (means this has been T&L'd)
	mov		dword ptr [eax+0]TLBN.tex+(0 * SIZE TLBN), 0
	mov		dword ptr [eax+0]TLBN.tex+(1 * SIZE TLBN), 0
	mov		dword ptr [eax+0]TLBN.tex+(2 * SIZE TLBN), 0
	mov		dword ptr [eax+0]TLBN.tex+(3 * SIZE TLBN), 0

Textures_Done:
  }
}  


#endif //VCPP
#endif //TnL_HAL
#endif //DX7
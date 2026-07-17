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
** File name: soadswiz.c
**
** Description: De-swizzle code for SOA -> AOS (Napalm)
**
** $Revision: 14$
** $Date: 10/18/00 5:40:49 PM$
**
** $Log: 
**  14   3dfx      1.3.1.3.1.5 10/18/00 Allen Hansen    Changed SSE T&L tag scheme
**       from a bitfield to marking the vert T&L'd in one of the textures
**  13   3dfx      1.3.1.3.1.4 10/11/00 Brent           Forced check in to enforce
**       branching.
**  12   3dfx      1.3.1.3.1.3 09/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  11   3dfx      1.3.1.3.1.2 09/15/00 Allen Hansen    Got rid of the TLBN_CLIP
**       structure (with homogenous coords), all output verts, clipped or not, use
**       the TLBN structure.
**  10   3dfx      1.3.1.3.1.1 07/08/00 Allen Hansen    replaced hard-wired TLBN
**       and TLBN_CLIP size macros with SIZE xxx
**  9    3dfx      1.3.1.3.1.0 06/29/00 Allen Hansen    Put the C version of
**       deswizzle in this file (ifdef'd out because it isn't used)
**  8    3dfx      1.3.1.3     06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  7    3dfx      1.3.1.2     05/28/00 Allen Hansen    Fixed a couple of comments
**  6    3dfx      1.3.1.1     05/10/00 Allen Hansen    Got rid of the C code in
**       the functions (they're all asm now)
**  5    3dfx      1.3.1.0     05/03/00 Allen Hansen    Moved deswizzling of color,
**       fog, and textures into the texture transform functions, which eliminates
**       storing and loading these params
**  4    Napalm    1.3         04/04/00 Bob Johnston    Bug fixes for Split VB path
**       and Full UM
**  3    Napalm    1.2         04/03/00 Allen Hansen    SW T&L Only:
**       Optimizations to asm rendering code,
**       Combined version of clip/cull check optimized cull check,
**       asm version of deswizzle
** 
** 
**  2    Napalm    1.1         03/23/00 Bob Johnston    Scott and Bob's changes to
**       split up the tranformation and lighting in the vertex processing loop for
**       improved VB primitive perfromance.
**  1    Napalm    1.0         03/08/00 Scott Kephart   
** $
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





/*-------------------------------------------------------------------
Function Name:  DeSwizzleSOAtoTLBN_GEOM
Description:    DeSwizzle's the Temp SOA down to the TLBN
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void DeSwizzleSOAtoTLBN_GEOM(RC *pRc, DWORD dwSOAIdx)
{
  __asm {
	mov			ecx, pRc
	mov			edx, dwSOAIdx
	mov			eax, [ecx]RC.tl.pTL
	lea			edx, [edx+edx*2]				// dwSOAIdx * TLBN_SIZE
	shl			edx, 4							// (TLBN_SIZE = 0x30)

	movlps  	xmm0, [eax]_STL.SOAsv.z+0	  	// xmm0 = --    --    z1    z0
	movlps  	xmm1, [eax]_STL.SOAsv.z+8	  	// xmm1 = --    --    z3    z2
	movhps  	xmm0, [eax]_STL.fSOAwfbi+0	  	// xmm0 = wfbi1 wfbi0 z1    z0
	movhps  	xmm1, [eax]_STL.fSOAwfbi+8	  	// xmm1 = wfbi3 wfbi2 z3    z2
	shufps		xmm0, xmm0, 0xd8				// xmm0 = wfbi1 z1    wfbi0 z0
	add			edx, [ecx]RC.tl.TLBN.lpvData	// *pDst
	shufps		xmm1, xmm1, 0xd8				// xmm1 = wfbi3 z3    wfbi2 z2

	movlps		xmm2, [eax]_STL.SOAsv.x+0	  	// xmm2 = --    --    x1    x0
	movlps		xmm3, [eax]_STL.SOAsv.x+8	  	// xmm3 = --    --    x3    x2
	movhps		xmm2, [eax]_STL.SOAsv.y+0	  	// xmm2 = y1    y0    x1    x0
	movhps		xmm3, [eax]_STL.SOAsv.y+8	  	// xmm3 = y3    y2    x3    x2
	shufps		xmm2, xmm2, 0xd8				// xmm2 = y1    x1    y0    x0
	shufps		xmm3, xmm3, 0xd8				// xmm3 = y3    x3    y2    x2

	movlps  	xmm4, [eax]_STL.SOAsv.w+0	  	// xmm4 = --    --    w1    w0
	movlps  	xmm5, [eax]_STL.SOAsv.w+8	  	// xmm5 = --    --    w3    w2

	movlps		[edx]TLBN.z+(0 * SIZE TLBN), xmm0	// wfbi0 z0
	movlps		[edx]TLBN.x+(0 * SIZE TLBN), xmm2	// y0    x0
	movss		[edx]TLBN.w+(0 * SIZE TLBN), xmm4	// w0
	shufps		xmm4, xmm4, 0xe1				// xmm4 = --    --    w0    w1
	mov		    dword ptr [edx]TLBN.tex+(0 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT

	movhps		[edx]TLBN.z+(1 * SIZE TLBN), xmm0	// wfbi1 z1
	movhps		[edx]TLBN.x+(1 * SIZE TLBN), xmm2	// y1    x1
	movss		[edx]TLBN.w+(1 * SIZE TLBN), xmm4	// w1
	mov		    dword ptr [edx]TLBN.tex+(1 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT

	movlps		[edx]TLBN.z+(2 * SIZE TLBN), xmm1	// wfbi2 z2
	movlps		[edx]TLBN.x+(2 * SIZE TLBN), xmm3	// y2    x2
	movss		[edx]TLBN.w+(2 * SIZE TLBN), xmm5	// w2
	shufps		xmm5, xmm5, 0xe1				// xmm5 = --    --    w2    w3
	mov		    dword ptr [edx]TLBN.tex+(2 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT

	movhps		[edx]TLBN.z+(3 * SIZE TLBN), xmm1	// wfbi3 z3
	movhps		[edx]TLBN.x+(3 * SIZE TLBN), xmm3	// y3    x3
	movss		[edx]TLBN.w+(3 * SIZE TLBN), xmm5	// w3
	mov		    dword ptr [edx]TLBN.tex+(3 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
  }
}






/*-------------------------------------------------------------------
Function Name:  DeSwizzleSOAtoTLBNClipped_GEOM
Description:    DeSwizzle's the Temp SOA down to the TLBN
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void DeSwizzleSOAtoTLBNClipped_GEOM(RC *pRc, DWORD dwSOAIdx)
{
  __asm {
	mov			ecx, pRc
	mov			edx, dwSOAIdx
	mov			eax, [ecx]RC.tl.pTL
	lea			edx, [edx+edx*2]					// dwSOAIdx * TLBN_SIZE
	shl			edx, 4								// (TLBN_SIZE = 0x30)

	movlps  	xmm0, [eax]_STL.SOAsv.z+0			// xmm0 = --    --    z1    z0
	movlps  	xmm1, [eax]_STL.SOAsv.z+8			// xmm1 = --    --    z3    z2
	movhps  	xmm0, [eax]_STL.fSOAwfbi+0			// xmm0 = wfbi1 wfbi0 z1    z0
	movhps  	xmm1, [eax]_STL.fSOAwfbi+8			// xmm1 = wfbi3 wfbi2 z3    z2
	shufps		xmm0, xmm0, 0xd8					// xmm0 = wfbi1 z1    wfbi0 z0
	add			edx, [ecx]RC.tl.TLBN.lpvData		// *pDst
	shufps		xmm1, xmm1, 0xd8					// xmm1 = wfbi3 z3    wfbi2 z2

	movlps		xmm2, [eax]_STL.SOAsv.x+0			// xmm2 = --    --    x1    x0
	movlps		xmm3, [eax]_STL.SOAsv.x+8			// xmm3 = --    --    x3    x2
	movhps		xmm2, [eax]_STL.SOAsv.y+0			// xmm2 = y1    y0    x1    x0
	movhps		xmm3, [eax]_STL.SOAsv.y+8			// xmm3 = y3    y2    x3    x2
	shufps		xmm2, xmm2, 0xd8					// xmm2 = y1    x1    y0    x0
	shufps		xmm3, xmm3, 0xd8					// xmm3 = y3    x3    y2    x2

	movlps  	xmm4, [eax]_STL.SOAsv.w+0			// xmm4 = --    --    w1    w0
	movlps  	xmm5, [eax]_STL.SOAsv.w+8			// xmm5 = --    --    w3    w2
	movhps		xmm4, [eax]_STL.dSOAclip_code+0		// xmm4 = cc1	cc0   w1    w0
	movhps		xmm5, [eax]_STL.dSOAclip_code+8		// xmm5 = cc3	cc2   w3    w2
	shufps		xmm4, xmm4, 0xd8					// xmm4 = cc1	w1    cc0   w0
	shufps		xmm5, xmm5, 0xd8					// xmm5 = cc3	w3    cc2   w2

	movlps		[edx]TLBN.z+(0 * SIZE TLBN), xmm0	// wfbi0 z0
	movlps		[edx]TLBN.x+(0 * SIZE TLBN), xmm2	// y0    x0
	movlps		[edx]TLBN.w+(0 * SIZE TLBN), xmm4	// cc0   w0
	mov		    dword ptr [edx]TLBN.tex+(0 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT

	movhps		[edx]TLBN.z+(1 * SIZE TLBN), xmm0	// wfbi1 z1
	movhps		[edx]TLBN.x+(1 * SIZE TLBN), xmm2	// y1    x1
	movhps		[edx]TLBN.w+(1 * SIZE TLBN), xmm4	// cc1   w1
	mov		    dword ptr [edx]TLBN.tex+(1 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT

	movlps		[edx]TLBN.z+(2 * SIZE TLBN), xmm1	// wfbi2 z2
	movlps		[edx]TLBN.x+(2 * SIZE TLBN), xmm3	// y2    x2
	movlps		[edx]TLBN.w+(2 * SIZE TLBN), xmm5	// cc2   w2
	mov		    dword ptr [edx]TLBN.tex+(2 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT

	movhps		[edx]TLBN.z+(3 * SIZE TLBN), xmm1	// wfbi3 z3
	movhps		[edx]TLBN.x+(3 * SIZE TLBN), xmm3	// y3    x3
	movhps		[edx]TLBN.w+(3 * SIZE TLBN), xmm5	// cc3   w3
	mov		    dword ptr [edx]TLBN.tex+(3 * SIZE TLBN), TL_VERT_XFORMED_NOT_LIT
  }
}




#endif // TnL_HAL
#endif // DX7

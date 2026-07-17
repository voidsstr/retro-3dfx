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
** File name: soaswiz.c
**
** Description: Swizzle code for SOA
**
** $Revision: 17$
** $Date: 10/11/00 8:50:14 PM$
**
** $Log: 
**  17   3dfx      1.8.1.2.1.4 10/11/00 Brent           Forced check in to enforce
**       branching.
**  16   3dfx      1.8.1.2.1.3 09/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  15   3dfx      1.8.1.2.1.2 09/05/00 Allen Hansen    Fixed bug in the swizzle
**       code: we were always swizzling blocks of 4 verts.  If the # of verts
**       wasn't divisible by 4, we always swizzled the last group of 4.  We can
**       page fault (or GPF) if the end of a VB is near the end of a page (or
**       segment). 
**  14   3dfx      1.8.1.2.1.1 08/21/00 Allen Hansen    changed from sse intrinsics
**       to inline assembly
**  13   3dfx      1.8.1.2.1.0 06/25/00 Allen Hansen    SwizzleSingleD3DtoSOA to
**       SwizzleSingleD3DtoSOA_C
**  12   3dfx      1.8.1.2     06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  11   3dfx      1.8.1.1     05/11/00 Allen Hansen    Wrote asm version of
**       SwizzleD3DtoSOA(), gets used quite a bit by 3DMark
**  10   3dfx      1.8.1.0     05/11/00 Allen Hansen    Changed prefetch from
**       Prefetch0 to PrefetchNTA
**  9    Napalm    1.8         04/17/00 Allen Hansen    Oops, my bad ... last
**       checkin wouldn't compile!
**  8    Napalm    1.7         04/17/00 Allen Hansen    Fixed 2 bugs in
**       swizzlesingle: first texture offsets were wrong, wasn't checking the
**       D3DFVF_RESERVED1 flags
**  7    Napalm    1.6         04/14/00 Allen Hansen    Removed hack to not
**       swizzle/copy more than 3 textures in the ASM code.  (Broke test #6 in
**       WB2K)
**  6    Napalm    1.5         04/12/00 Allen Hansen    1) added prefetching to
**       single-swizzle code, requires new file asoaswiz.asm
**       2) this required the SwizzleSingleD3DtoSOA_Setup() function
** 
**  5    Napalm    1.4         03/29/00 Bob Johnston    Created User Memory Split
**       T&L Path and added index pre calculations.
**  4    Napalm    1.3         03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  3    Napalm    1.2         03/16/00 Bob Johnston    Got Vertex Buffers working
**       correctly
**  2    Napalm    1.1         03/14/00 Bob Johnston    Changed User Mem vertices
**       to only use a min amount of memory in the SOAFVF_UM buff. Called the
**       setDX6State() from the FP branch in procprim.  Fixed CanCreateExecBuff32
**       problem.  Decided to force VB creation to punt until I fix all VB
**       problems.
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


extern KniSwizSingle_SwizParms_FunctionType *SwizzleSngl_SwizParmsFunctions;
extern KniSwizSingle_CopyTxtrs_FunctionType *SwizzleSngl_CopyTxtrsFunctions;

/*-------------------------------------------------------------------
Function Name:  SwizzleD3DtoSOA
Description:    Swizzle's D3DFVF to SOAFVF
Parameters:   
Information:    This version calls the asm code (which uses KNI regs)
Return:         
-------------------------------------------------------------------*/
void __cdecl SwizzleD3DtoSOA_Asm ( DWORD *pSrc, DWORD *pDst, DWORD dwSrcStride, DWORD dwDstStride, DWORD dwNumSOAVecs, DWORD dwFVFEntries, DWORD dwNumTxtures );
void SwizzleD3DtoSOA( RC *pRc, DWORD dwNumSOAVecs )
{
	DWORD *pSrc = (DWORD*) pRc->tl.lpVBSurfData->pSrcAlignAddr;
	DWORD *pDst = (DWORD*) pRc->tl.SOAFVF.lpvData;
	DWORD dwSrcStride = pRc->tl.InFVF.dwStride;
	DWORD dwDstStride = pRc->tl.SOAFVF.dwStride;
	DWORD dwSrcTexOffset = pRc->tl.InFVF.dwTexOffset;
	DWORD dwDstTexOffset = pRc->tl.SOAFVF.dwTexOffset;
	DWORD dwFVFType = pRc->tl.InFVF.dwFVFType;
	DWORD dwNumTxtures = (dwFVFType & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	DWORD dwFVFEntries;
	DWORD dwNumVerts = pRc->tl.lpVBSurfData->dwNumVerts;
	DWORD dwNumSOAVerts = dwNumVerts / 4;

	switch (dwFVFType & D3DFVF_POSITION_MASK)
	{
		case D3DFVF_XYZ:	
			if (! (dwFVFType & D3DFVF_RESERVED1))	dwFVFEntries = 3;
			else									dwFVFEntries = 4;	// D3DLVERTEX (a special case)
			break;
		case D3DFVF_XYZRHW:	dwFVFEntries = 4;	break;
		case D3DFVF_XYZB1:	dwFVFEntries = 4;	break;
		case D3DFVF_XYZB2:	dwFVFEntries = 5;	break;
		case D3DFVF_XYZB3:	dwFVFEntries = 6;	break;
		case D3DFVF_XYZB4:	dwFVFEntries = 7;	break;
		case D3DFVF_XYZB5:	dwFVFEntries = 8;	break;
		default:			dwFVFEntries = 0;	break;	// no position ... BAD
	}
    if (dwFVFType & D3DFVF_NORMAL)		dwFVFEntries += 3;
    if (dwFVFType & D3DFVF_DIFFUSE)		dwFVFEntries += 1;
    if (dwFVFType & D3DFVF_SPECULAR)	dwFVFEntries += 1;

	if (dwFVFEntries == 0)
		return;

	//SwizzleD3DtoSOA_Asm( pSrc, pDst, dwSrcStride, dwDstStride, dwNumSOAVecs, dwFVFEntries, dwNumTxtures );
	if (dwNumSOAVerts)
	  SwizzleD3DtoSOA_Asm( pSrc, pDst, dwSrcStride, dwDstStride, dwNumSOAVerts, dwFVFEntries, dwNumTxtures );

	// Check for a partial SOA group left, we can't just blindly read past the end or we could page fault
	if (dwNumVerts & 3)
	{
		DWORD *pSrcTmp = (DWORD*)( (DWORD)pSrc + ((dwNumVerts & ~3) * dwSrcStride) );
		DWORD *pDstTmp = (DWORD*)( (DWORD)pDst + ((dwNumVerts & ~3) * dwDstStride / SOA_SIZE) );
		DWORD dwNumVertsInLastSOAGroup = dwNumVerts & 3;
		SwizzlePartialSingleD3DtoSOA( pRc, pSrcTmp, pDstTmp, dwNumVertsInLastSOAGroup );
	}
}



/*-------------------------------------------------------------------
Function Name:  SwizzlePartialSingleD3DtoSOA
Description:    Swizzle's D3DFVF to SOAFVF for a partial SOA group
Parameters:   
Information:    Use this when the group you need to swizzle has less
				than 4 verts.  We have to do this otherwize we'll
				try to read past the end of the VB, which could cause
				a Page or GP fault.  (They're rare but I have seen
				them occur - don't take this out!).
Return:         
-------------------------------------------------------------------*/
void SwizzlePartialSingleD3DtoSOA( RC *pRc, DWORD *pSrc, DWORD *pDst, DWORD numVerts )
{
	DWORD dwSrcStride = pRc->tl.InFVF.dwStride >> 2;		// Divide by four to match pointer math
	DWORD dwFVFType = pRc->tl.InFVF.dwFVFType;
	DWORD dwNumTxtures = (dwFVFType & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	DWORD dwFVFEntries;
	DWORD i, j;
	DWORD *pSrcTmp = pSrc;
	DWORD *pDstTmp = pDst;

	switch (dwFVFType & D3DFVF_POSITION_MASK)
	{
		case D3DFVF_XYZ:	
			if (! (dwFVFType & D3DFVF_RESERVED1))	dwFVFEntries = 3;
			else									dwFVFEntries = 4;	// D3DLVERTEX (a special case)
			break;
		case D3DFVF_XYZRHW:	dwFVFEntries = 4;	break;
		case D3DFVF_XYZB1:	dwFVFEntries = 4;	break;
		case D3DFVF_XYZB2:	dwFVFEntries = 5;	break;
		case D3DFVF_XYZB3:	dwFVFEntries = 6;	break;
		case D3DFVF_XYZB4:	dwFVFEntries = 7;	break;
		case D3DFVF_XYZB5:	dwFVFEntries = 8;	break;
		default:			dwFVFEntries = 0;	break;	// no position ... BAD
	}
    if (dwFVFType & D3DFVF_NORMAL)		dwFVFEntries += 3;
    if (dwFVFType & D3DFVF_DIFFUSE)		dwFVFEntries += 1;
    if (dwFVFType & D3DFVF_SPECULAR)	dwFVFEntries += 1;

	if (dwFVFEntries == 0)
		return;

    for(i=0; i<numVerts; ++i)
	{
		for(j=0; j<dwFVFEntries; ++j)
		{
          pDstTmp[j*4] = pSrcTmp[j];
		}
		pSrcTmp = (DWORD*)&pSrcTmp[dwSrcStride];
		pDstTmp++;
	}

	// set the src and dst ptrs to the textures
    pSrcTmp = pSrc;
	pSrcTmp = (DWORD*)&pSrcTmp[dwFVFEntries*1];
	pDstTmp = pDst;
	pDstTmp = (DWORD*)&pDstTmp[dwFVFEntries*4];
    for(i=0; i<numVerts; ++i)
	{
		for(j=0; j<dwNumTxtures; ++j)
		{
          pDstTmp[(j*2*4) + 0] = pSrcTmp[(j*2)+0];
          pDstTmp[(j*2*4) + 1] = pSrcTmp[(j*2)+1];
		}

		pSrcTmp = (DWORD*)&pSrcTmp[dwSrcStride];
		pDstTmp += 2;
	}
}


#if 0 //UNUSED - don't delete this
/*-------------------------------------------------------------------
Function Name:  SwizzleSingleD3DtoSOA_C
Description:    Swizzle's D3DFVF to SOAFVF for one SOA group
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void SwizzleSingleD3DtoSOA_C( RC *pRc, DWORD vIndx, DWORD dwSOAOffset )
{
  // Copy all this stuff to local stack variables so we 
  // don't have to do so much pointer chasing
  DWORD *pSrc = (DWORD*) ((LPBYTE)pRc->tl.InFVF.lpvData + (vIndx * pRc->tl.InFVF.dwStride));
  DWORD *pDst = (DWORD*) ((LPBYTE)pRc->tl.SOAFVF.lpvData + (dwSOAOffset * pRc->tl.SOAFVF.dwStride));
  DWORD *pSrcTmp = pSrc;
  DWORD *pDstTmp = pDst;
  DWORD dwFVFType = pRc->tl.InFVF.dwFVFType; 
  DWORD dwSrcStride = (pRc->tl.InFVF.dwStride>>2);  // Divide by four to match pointer math

    switch (dwFVFType & D3DFVF_POSITION_MASK)
    {
      case D3DFVF_XYZ:
        pDstTmp[4*0 + 0] = pSrcTmp[dwSrcStride*0 + 0];  // vertex 0 x
        pDstTmp[4*1 + 0] = pSrcTmp[dwSrcStride*0 + 1];  // vertex 0 y
        pDstTmp[4*2 + 0] = pSrcTmp[dwSrcStride*0 + 2];  // vertex 0 z
        pDstTmp[4*0 + 1] = pSrcTmp[dwSrcStride*1 + 0];  // vertex 1 x
        pDstTmp[4*1 + 1] = pSrcTmp[dwSrcStride*1 + 1];  // vertex 1 y
        pDstTmp[4*2 + 1] = pSrcTmp[dwSrcStride*1 + 2];  // vertex 1 z
        pDstTmp[4*0 + 2] = pSrcTmp[dwSrcStride*2 + 0];  // vertex 2 x
        pDstTmp[4*1 + 2] = pSrcTmp[dwSrcStride*2 + 1];  // vertex 2 y
        pDstTmp[4*2 + 2] = pSrcTmp[dwSrcStride*2 + 2];  // vertex 2 z
        pDstTmp[4*0 + 3] = pSrcTmp[dwSrcStride*3 + 0];  // vertex 3 x
        pDstTmp[4*1 + 3] = pSrcTmp[dwSrcStride*3 + 1];  // vertex 3 y
        pDstTmp[4*2 + 3] = pSrcTmp[dwSrcStride*3 + 2];  // vertex 3 z
	    if (! (dwFVFType & D3DFVF_RESERVED1)) {		// D3DLVERTEX emulation, only valid with D3DFVF_XYZ
			pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 3];	  		pDstTmp = (DWORD *)&pDstTmp[4*3 + 0];
		} else {
			pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 4];	  		pDstTmp = (DWORD *)&pDstTmp[4*4 + 0];
		}
        break;

      case D3DFVF_XYZRHW:
      case D3DFVF_XYZB1:
        pDstTmp[4*0 + 0] = pSrcTmp[dwSrcStride*0 + 0];  // vertex 0 x
        pDstTmp[4*1 + 0] = pSrcTmp[dwSrcStride*0 + 1];  // vertex 0 y
        pDstTmp[4*2 + 0] = pSrcTmp[dwSrcStride*0 + 2];  // vertex 0 z
        pDstTmp[4*3 + 0] = pSrcTmp[dwSrcStride*0 + 3];  // vertex 0 1/w or blend 1
        pDstTmp[4*0 + 1] = pSrcTmp[dwSrcStride*1 + 0];  // vertex 1 x
        pDstTmp[4*1 + 1] = pSrcTmp[dwSrcStride*1 + 1];  // vertex 1 y
        pDstTmp[4*2 + 1] = pSrcTmp[dwSrcStride*1 + 2];  // vertex 1 z
        pDstTmp[4*3 + 1] = pSrcTmp[dwSrcStride*1 + 3];  // vertex 1 1/w or blend 1
        pDstTmp[4*0 + 2] = pSrcTmp[dwSrcStride*2 + 0];  // vertex 2 x
        pDstTmp[4*1 + 2] = pSrcTmp[dwSrcStride*2 + 1];  // vertex 2 y
        pDstTmp[4*2 + 2] = pSrcTmp[dwSrcStride*2 + 2];  // vertex 2 z
        pDstTmp[4*3 + 2] = pSrcTmp[dwSrcStride*2 + 3];  // vertex 2 1/w or blend 1
        pDstTmp[4*0 + 3] = pSrcTmp[dwSrcStride*3 + 0];  // vertex 3 x
        pDstTmp[4*1 + 3] = pSrcTmp[dwSrcStride*3 + 1];  // vertex 3 y
        pDstTmp[4*2 + 3] = pSrcTmp[dwSrcStride*3 + 2];  // vertex 3 z
        pDstTmp[4*3 + 3] = pSrcTmp[dwSrcStride*3 + 3];  // vertex 3 1/w or blend 1
		pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 4];
		pDstTmp = (DWORD *)&pDstTmp[4*4 + 0];
        break;

      case D3DFVF_XYZB2:
        pDstTmp[4*0 + 0] = pSrcTmp[dwSrcStride*0 + 0];  // vertex 0 x
        pDstTmp[4*1 + 0] = pSrcTmp[dwSrcStride*0 + 1];  // vertex 0 y
        pDstTmp[4*2 + 0] = pSrcTmp[dwSrcStride*0 + 2];  // vertex 0 z
        pDstTmp[4*3 + 0] = pSrcTmp[dwSrcStride*0 + 3];  // vertex 0 blend 1
        pDstTmp[4*4 + 0] = pSrcTmp[dwSrcStride*0 + 4];  // vertex 0 blend 2
        pDstTmp[4*0 + 1] = pSrcTmp[dwSrcStride*1 + 0];  // vertex 1 x
        pDstTmp[4*1 + 1] = pSrcTmp[dwSrcStride*1 + 1];  // vertex 1 y
        pDstTmp[4*2 + 1] = pSrcTmp[dwSrcStride*1 + 2];  // vertex 1 z
        pDstTmp[4*3 + 1] = pSrcTmp[dwSrcStride*1 + 3];  // vertex 1 blend 1
        pDstTmp[4*4 + 1] = pSrcTmp[dwSrcStride*1 + 4];  // vertex 1 blend 2
        pDstTmp[4*0 + 2] = pSrcTmp[dwSrcStride*2 + 0];  // vertex 2 x
        pDstTmp[4*1 + 2] = pSrcTmp[dwSrcStride*2 + 1];  // vertex 2 y
        pDstTmp[4*2 + 2] = pSrcTmp[dwSrcStride*2 + 2];  // vertex 2 z
        pDstTmp[4*3 + 2] = pSrcTmp[dwSrcStride*2 + 3];  // vertex 2 blend 1
        pDstTmp[4*4 + 2] = pSrcTmp[dwSrcStride*2 + 4];  // vertex 2 blend 2
        pDstTmp[4*0 + 3] = pSrcTmp[dwSrcStride*3 + 0];  // vertex 3 x
        pDstTmp[4*1 + 3] = pSrcTmp[dwSrcStride*3 + 1];  // vertex 3 y
        pDstTmp[4*2 + 3] = pSrcTmp[dwSrcStride*3 + 2];  // vertex 3 z
        pDstTmp[4*3 + 3] = pSrcTmp[dwSrcStride*3 + 3];  // vertex 3 blend 1
        pDstTmp[4*4 + 3] = pSrcTmp[dwSrcStride*3 + 4];  // vertex 3 blend 2
		pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 5];
		pDstTmp = (DWORD *)&pDstTmp[4*5 + 0];
        break;

      case D3DFVF_XYZB3:
        pDstTmp[4*0 + 0] = pSrcTmp[dwSrcStride*0 + 0];  // vertex 0 x
        pDstTmp[4*1 + 0] = pSrcTmp[dwSrcStride*0 + 1];  // vertex 0 y
        pDstTmp[4*2 + 0] = pSrcTmp[dwSrcStride*0 + 2];  // vertex 0 z
        pDstTmp[4*3 + 0] = pSrcTmp[dwSrcStride*0 + 3];  // vertex 0 blend 1
        pDstTmp[4*4 + 0] = pSrcTmp[dwSrcStride*0 + 4];  // vertex 0 blend 2
        pDstTmp[4*5 + 0] = pSrcTmp[dwSrcStride*0 + 5];  // vertex 0 blend 3
        pDstTmp[4*0 + 1] = pSrcTmp[dwSrcStride*1 + 0];  // vertex 1 x
        pDstTmp[4*1 + 1] = pSrcTmp[dwSrcStride*1 + 1];  // vertex 1 y
        pDstTmp[4*2 + 1] = pSrcTmp[dwSrcStride*1 + 2];  // vertex 1 z
        pDstTmp[4*3 + 1] = pSrcTmp[dwSrcStride*1 + 3];  // vertex 1 blend 1
        pDstTmp[4*4 + 1] = pSrcTmp[dwSrcStride*1 + 4];  // vertex 1 blend 2
        pDstTmp[4*5 + 1] = pSrcTmp[dwSrcStride*1 + 5];  // vertex 1 blend 3
        pDstTmp[4*0 + 2] = pSrcTmp[dwSrcStride*2 + 0];  // vertex 2 x
        pDstTmp[4*1 + 2] = pSrcTmp[dwSrcStride*2 + 1];  // vertex 2 y
        pDstTmp[4*2 + 2] = pSrcTmp[dwSrcStride*2 + 2];  // vertex 2 z
        pDstTmp[4*3 + 2] = pSrcTmp[dwSrcStride*2 + 3];  // vertex 2 blend 1
        pDstTmp[4*4 + 2] = pSrcTmp[dwSrcStride*2 + 4];  // vertex 2 blend 2
        pDstTmp[4*5 + 2] = pSrcTmp[dwSrcStride*2 + 5];  // vertex 2 blend 3
        pDstTmp[4*0 + 3] = pSrcTmp[dwSrcStride*3 + 0];  // vertex 3 x
        pDstTmp[4*1 + 3] = pSrcTmp[dwSrcStride*3 + 1];  // vertex 3 y
        pDstTmp[4*2 + 3] = pSrcTmp[dwSrcStride*3 + 2];  // vertex 3 z
        pDstTmp[4*3 + 3] = pSrcTmp[dwSrcStride*3 + 3];  // vertex 3 blend 1
        pDstTmp[4*4 + 3] = pSrcTmp[dwSrcStride*3 + 4];  // vertex 3 blend 2
        pDstTmp[4*5 + 3] = pSrcTmp[dwSrcStride*3 + 5];  // vertex 3 blend 3
		pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 6];
		pDstTmp = (DWORD *)&pDstTmp[4*6 + 0];
        break;

      case D3DFVF_XYZB4:
        pDstTmp[4*0 + 0] = pSrcTmp[dwSrcStride*0 + 0];  // vertex 0 x
        pDstTmp[4*1 + 0] = pSrcTmp[dwSrcStride*0 + 1];  // vertex 0 y
        pDstTmp[4*2 + 0] = pSrcTmp[dwSrcStride*0 + 2];  // vertex 0 z
        pDstTmp[4*3 + 0] = pSrcTmp[dwSrcStride*0 + 3];  // vertex 0 blend 1
        pDstTmp[4*4 + 0] = pSrcTmp[dwSrcStride*0 + 4];  // vertex 0 blend 2
        pDstTmp[4*5 + 0] = pSrcTmp[dwSrcStride*0 + 5];  // vertex 0 blend 3
        pDstTmp[4*6 + 0] = pSrcTmp[dwSrcStride*0 + 6];  // vertex 0 blend 4
        pDstTmp[4*0 + 1] = pSrcTmp[dwSrcStride*1 + 0];  // vertex 1 x
        pDstTmp[4*1 + 1] = pSrcTmp[dwSrcStride*1 + 1];  // vertex 1 y
        pDstTmp[4*2 + 1] = pSrcTmp[dwSrcStride*1 + 2];  // vertex 1 z
        pDstTmp[4*3 + 1] = pSrcTmp[dwSrcStride*1 + 3];  // vertex 1 blend 1
        pDstTmp[4*4 + 1] = pSrcTmp[dwSrcStride*1 + 4];  // vertex 1 blend 2
        pDstTmp[4*5 + 1] = pSrcTmp[dwSrcStride*1 + 5];  // vertex 1 blend 3
        pDstTmp[4*6 + 1] = pSrcTmp[dwSrcStride*1 + 6];  // vertex 1 blend 4
        pDstTmp[4*0 + 2] = pSrcTmp[dwSrcStride*2 + 0];  // vertex 2 x
        pDstTmp[4*1 + 2] = pSrcTmp[dwSrcStride*2 + 1];  // vertex 2 y
        pDstTmp[4*2 + 2] = pSrcTmp[dwSrcStride*2 + 2];  // vertex 2 z
        pDstTmp[4*3 + 2] = pSrcTmp[dwSrcStride*2 + 3];  // vertex 2 blend 1
        pDstTmp[4*4 + 2] = pSrcTmp[dwSrcStride*2 + 4];  // vertex 2 blend 2
        pDstTmp[4*5 + 2] = pSrcTmp[dwSrcStride*2 + 5];  // vertex 2 blend 3
        pDstTmp[4*6 + 2] = pSrcTmp[dwSrcStride*2 + 6];  // vertex 2 blend 4
        pDstTmp[4*0 + 3] = pSrcTmp[dwSrcStride*3 + 0];  // vertex 3 x
        pDstTmp[4*1 + 3] = pSrcTmp[dwSrcStride*3 + 1];  // vertex 3 y
        pDstTmp[4*2 + 3] = pSrcTmp[dwSrcStride*3 + 2];  // vertex 3 z
        pDstTmp[4*3 + 3] = pSrcTmp[dwSrcStride*3 + 3];  // vertex 3 blend 1
        pDstTmp[4*4 + 3] = pSrcTmp[dwSrcStride*3 + 4];  // vertex 3 blend 2
        pDstTmp[4*5 + 3] = pSrcTmp[dwSrcStride*3 + 5];  // vertex 3 blend 3
        pDstTmp[4*6 + 3] = pSrcTmp[dwSrcStride*3 + 6];  // vertex 3 blend 4
		pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 7];
		pDstTmp = (DWORD *)&pDstTmp[4*7 + 0];
        break;

      case D3DFVF_XYZB5:
        pDstTmp[4*0 + 0] = pSrcTmp[dwSrcStride*0 + 0];  // vertex 0 x
        pDstTmp[4*1 + 0] = pSrcTmp[dwSrcStride*0 + 1];  // vertex 0 y
        pDstTmp[4*2 + 0] = pSrcTmp[dwSrcStride*0 + 2];  // vertex 0 z
        pDstTmp[4*3 + 0] = pSrcTmp[dwSrcStride*0 + 3];  // vertex 0 blend 1
        pDstTmp[4*4 + 0] = pSrcTmp[dwSrcStride*0 + 4];  // vertex 0 blend 2
        pDstTmp[4*5 + 0] = pSrcTmp[dwSrcStride*0 + 5];  // vertex 0 blend 3
        pDstTmp[4*6 + 0] = pSrcTmp[dwSrcStride*0 + 6];  // vertex 0 blend 4
        pDstTmp[4*7 + 0] = pSrcTmp[dwSrcStride*0 + 7];  // vertex 0 blend 5
        pDstTmp[4*0 + 1] = pSrcTmp[dwSrcStride*1 + 0];  // vertex 1 x
        pDstTmp[4*1 + 1] = pSrcTmp[dwSrcStride*1 + 1];  // vertex 1 y
        pDstTmp[4*2 + 1] = pSrcTmp[dwSrcStride*1 + 2];  // vertex 1 z
        pDstTmp[4*3 + 1] = pSrcTmp[dwSrcStride*1 + 3];  // vertex 1 blend 1
        pDstTmp[4*4 + 1] = pSrcTmp[dwSrcStride*1 + 4];  // vertex 1 blend 2
        pDstTmp[4*5 + 1] = pSrcTmp[dwSrcStride*1 + 5];  // vertex 1 blend 3
        pDstTmp[4*6 + 1] = pSrcTmp[dwSrcStride*1 + 6];  // vertex 1 blend 4
        pDstTmp[4*7 + 1] = pSrcTmp[dwSrcStride*1 + 7];  // vertex 1 blend 5
        pDstTmp[4*0 + 2] = pSrcTmp[dwSrcStride*2 + 0];  // vertex 2 x
        pDstTmp[4*1 + 2] = pSrcTmp[dwSrcStride*2 + 1];  // vertex 2 y
        pDstTmp[4*2 + 2] = pSrcTmp[dwSrcStride*2 + 2];  // vertex 2 z
        pDstTmp[4*3 + 2] = pSrcTmp[dwSrcStride*2 + 3];  // vertex 2 blend 1
        pDstTmp[4*4 + 2] = pSrcTmp[dwSrcStride*2 + 4];  // vertex 2 blend 2
        pDstTmp[4*5 + 2] = pSrcTmp[dwSrcStride*2 + 5];  // vertex 2 blend 3
        pDstTmp[4*6 + 2] = pSrcTmp[dwSrcStride*2 + 6];  // vertex 2 blend 4
        pDstTmp[4*7 + 2] = pSrcTmp[dwSrcStride*2 + 7];  // vertex 2 blend 5
        pDstTmp[4*0 + 3] = pSrcTmp[dwSrcStride*3 + 0];  // vertex 3 x
        pDstTmp[4*1 + 3] = pSrcTmp[dwSrcStride*3 + 1];  // vertex 3 y
        pDstTmp[4*2 + 3] = pSrcTmp[dwSrcStride*3 + 2];  // vertex 3 z
        pDstTmp[4*3 + 3] = pSrcTmp[dwSrcStride*3 + 3];  // vertex 3 blend 1
        pDstTmp[4*4 + 3] = pSrcTmp[dwSrcStride*3 + 4];  // vertex 3 blend 2
        pDstTmp[4*5 + 3] = pSrcTmp[dwSrcStride*3 + 5];  // vertex 3 blend 3
        pDstTmp[4*6 + 3] = pSrcTmp[dwSrcStride*3 + 6];  // vertex 3 blend 4
        pDstTmp[4*7 + 3] = pSrcTmp[dwSrcStride*3 + 7];  // vertex 3 blend 5
		pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 8];
		pDstTmp = (DWORD *)&pDstTmp[4*8 + 0];
        break;

      default:          // no position ... BAD
	    if (dwFVFType & D3DFVF_RESERVED1) {
			pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 1];			pDstTmp = (DWORD *)&pDstTmp[4*1 + 0];
		}
        break;
    } // end of position/blend switch()

    if (dwFVFType & D3DFVF_NORMAL) {
//      pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwNormalOffset); 
//      pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwNormalOffset);
      pDstTmp[4*0 + 0] = pSrcTmp[dwSrcStride*0 + 0];  // vertex 0 nx
      pDstTmp[4*1 + 0] = pSrcTmp[dwSrcStride*0 + 1];  // vertex 0 ny
      pDstTmp[4*2 + 0] = pSrcTmp[dwSrcStride*0 + 2];  // vertex 0 nz
      pDstTmp[4*0 + 1] = pSrcTmp[dwSrcStride*1 + 0];  // vertex 1 nx
      pDstTmp[4*1 + 1] = pSrcTmp[dwSrcStride*1 + 1];  // vertex 1 ny
      pDstTmp[4*2 + 1] = pSrcTmp[dwSrcStride*1 + 2];  // vertex 1 nz
      pDstTmp[4*0 + 2] = pSrcTmp[dwSrcStride*2 + 0];  // vertex 2 nx
      pDstTmp[4*1 + 2] = pSrcTmp[dwSrcStride*2 + 1];  // vertex 2 ny
      pDstTmp[4*2 + 2] = pSrcTmp[dwSrcStride*2 + 2];  // vertex 2 nz
      pDstTmp[4*0 + 3] = pSrcTmp[dwSrcStride*3 + 0];  // vertex 3 nx
      pDstTmp[4*1 + 3] = pSrcTmp[dwSrcStride*3 + 1];  // vertex 3 ny
      pDstTmp[4*2 + 3] = pSrcTmp[dwSrcStride*3 + 2];  // vertex 3 nz
      pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 3];
      pDstTmp = (DWORD *)&pDstTmp[4*3 + 0];
    }

    if (dwFVFType & D3DFVF_DIFFUSE) {
//      pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwDiffuseOffset); 
//      pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwDiffuseOffset);
      pDstTmp[0] = pSrcTmp[dwSrcStride*0 + 0];  // vertex 0 diff
      pDstTmp[1] = pSrcTmp[dwSrcStride*1 + 0];  // vertex 1 diff
      pDstTmp[2] = pSrcTmp[dwSrcStride*2 + 0];  // vertex 2 diff
      pDstTmp[3] = pSrcTmp[dwSrcStride*3 + 0];  // vertex 3 diff
      pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 1];
      pDstTmp = (DWORD *)&pDstTmp[4*1 + 0];
    }

    if (dwFVFType & D3DFVF_SPECULAR) {
//      pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwSpecularOffset); 
//      pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwSpecularOffset);
      pDstTmp[0] = pSrcTmp[dwSrcStride*0 + 0];  // vertex 0 spec
      pDstTmp[1] = pSrcTmp[dwSrcStride*1 + 0];  // vertex 1 spec
      pDstTmp[2] = pSrcTmp[dwSrcStride*2 + 0];  // vertex 2 spec
      pDstTmp[3] = pSrcTmp[dwSrcStride*3 + 0];  // vertex 3 spec
      pSrcTmp = (DWORD *)&pSrcTmp[dwSrcStride*0 + 1];
      pDstTmp = (DWORD *)&pDstTmp[4*1 + 0];
    }

    switch ((dwFVFType & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT)  // Number of texture coordinate sets for this vertex
    {
      case (D3DFVF_TEX0 >> D3DFVF_TEXCOUNT_SHIFT):  // no textures
        break;

      case (D3DFVF_TEX1 >> D3DFVF_TEXCOUNT_SHIFT):  // 1 texture
//        pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwTexOffset); 
//        pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwTexOffset);
        pDstTmp[1*2*0 +  0] = pSrcTmp[dwSrcStride*0 +  0];  // vertex 0 texture coord 0 u
        pDstTmp[1*2*0 +  1] = pSrcTmp[dwSrcStride*0 +  1];  // vertex 0 texture coord 0 v
        pDstTmp[1*2*1 +  0] = pSrcTmp[dwSrcStride*1 +  0];  // vertex 1 texture coord 0 u
        pDstTmp[1*2*1 +  1] = pSrcTmp[dwSrcStride*1 +  1];  // vertex 1 texture coord 0 v
        pDstTmp[1*2*2 +  0] = pSrcTmp[dwSrcStride*2 +  0];  // vertex 2 texture coord 0 u
        pDstTmp[1*2*2 +  1] = pSrcTmp[dwSrcStride*2 +  1];  // vertex 2 texture coord 0 v
        pDstTmp[1*2*3 +  0] = pSrcTmp[dwSrcStride*3 +  0];  // vertex 3 texture coord 0 u
        pDstTmp[1*2*3 +  1] = pSrcTmp[dwSrcStride*3 +  1];  // vertex 3 texture coord 0 v
        break;

      case (D3DFVF_TEX2 >> D3DFVF_TEXCOUNT_SHIFT):  // 2 textures
//        pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwTexOffset); 
//        pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwTexOffset);
        pDstTmp[2*2*0 +  0] = pSrcTmp[dwSrcStride*0 +  0];  // vertex 0 texture coord 0 u
        pDstTmp[2*2*0 +  1] = pSrcTmp[dwSrcStride*0 +  1];  // vertex 0 texture coord 0 v
        pDstTmp[2*2*0 +  2] = pSrcTmp[dwSrcStride*1 +  0];  // vertex 1 texture coord 0 u
        pDstTmp[2*2*0 +  3] = pSrcTmp[dwSrcStride*1 +  1];  // vertex 1 texture coord 0 v
        pDstTmp[2*2*1 +  0] = pSrcTmp[dwSrcStride*2 +  0];  // vertex 2 texture coord 0 u
        pDstTmp[2*2*1 +  1] = pSrcTmp[dwSrcStride*2 +  1];  // vertex 2 texture coord 0 v
        pDstTmp[2*2*1 +  2] = pSrcTmp[dwSrcStride*3 +  0];  // vertex 3 texture coord 0 u
        pDstTmp[2*2*1 +  3] = pSrcTmp[dwSrcStride*3 +  1];  // vertex 3 texture coord 0 v
        pDstTmp[2*2*2 +  0] = pSrcTmp[dwSrcStride*0 +  2];  // vertex 0 texture coord 1 u
        pDstTmp[2*2*2 +  1] = pSrcTmp[dwSrcStride*0 +  3];  // vertex 0 texture coord 1 v
        pDstTmp[2*2*2 +  2] = pSrcTmp[dwSrcStride*1 +  2];  // vertex 1 texture coord 1 u
        pDstTmp[2*2*2 +  3] = pSrcTmp[dwSrcStride*1 +  3];  // vertex 1 texture coord 1 v
        pDstTmp[2*2*3 +  0] = pSrcTmp[dwSrcStride*2 +  2];  // vertex 2 texture coord 1 u
        pDstTmp[2*2*3 +  1] = pSrcTmp[dwSrcStride*2 +  3];  // vertex 2 texture coord 1 v
        pDstTmp[2*2*3 +  2] = pSrcTmp[dwSrcStride*3 +  2];  // vertex 3 texture coord 1 u
        pDstTmp[2*2*3 +  3] = pSrcTmp[dwSrcStride*3 +  3];  // vertex 3 texture coord 1 v
        break;

      case (D3DFVF_TEX3 >> D3DFVF_TEXCOUNT_SHIFT):  // 3 textures
//        pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwTexOffset); 
//        pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwTexOffset);
        pDstTmp[3*2*0 +  0] = pSrcTmp[dwSrcStride*0 +  0];  // vertex 0 texture coord 0 u
        pDstTmp[3*2*0 +  1] = pSrcTmp[dwSrcStride*0 +  1];  // vertex 0 texture coord 0 v
        pDstTmp[3*2*0 +  2] = pSrcTmp[dwSrcStride*1 +  0];  // vertex 1 texture coord 0 u
        pDstTmp[3*2*0 +  3] = pSrcTmp[dwSrcStride*1 +  1];  // vertex 1 texture coord 0 v
        pDstTmp[3*2*0 +  4] = pSrcTmp[dwSrcStride*2 +  0];  // vertex 2 texture coord 0 u
        pDstTmp[3*2*0 +  5] = pSrcTmp[dwSrcStride*2 +  1];  // vertex 2 texture coord 0 v
        pDstTmp[3*2*1 +  0] = pSrcTmp[dwSrcStride*3 +  0];  // vertex 3 texture coord 0 u
        pDstTmp[3*2*1 +  1] = pSrcTmp[dwSrcStride*3 +  1];  // vertex 3 texture coord 0 v
        pDstTmp[3*2*1 +  2] = pSrcTmp[dwSrcStride*0 +  2];  // vertex 0 texture coord 1 u
        pDstTmp[3*2*1 +  3] = pSrcTmp[dwSrcStride*0 +  3];  // vertex 0 texture coord 1 v
        pDstTmp[3*2*1 +  4] = pSrcTmp[dwSrcStride*1 +  2];  // vertex 1 texture coord 1 u
        pDstTmp[3*2*1 +  5] = pSrcTmp[dwSrcStride*1 +  3];  // vertex 1 texture coord 1 v
        pDstTmp[3*2*2 +  0] = pSrcTmp[dwSrcStride*2 +  2];  // vertex 2 texture coord 1 u
        pDstTmp[3*2*2 +  1] = pSrcTmp[dwSrcStride*2 +  3];  // vertex 2 texture coord 1 v
        pDstTmp[3*2*2 +  2] = pSrcTmp[dwSrcStride*3 +  2];  // vertex 3 texture coord 1 u
        pDstTmp[3*2*2 +  3] = pSrcTmp[dwSrcStride*3 +  3];  // vertex 3 texture coord 1 v
        pDstTmp[3*2*2 +  4] = pSrcTmp[dwSrcStride*0 +  4];  // vertex 0 texture coord 2 u
        pDstTmp[3*2*2 +  5] = pSrcTmp[dwSrcStride*0 +  5];  // vertex 0 texture coord 2 v
        pDstTmp[3*2*3 +  0] = pSrcTmp[dwSrcStride*1 +  4];  // vertex 1 texture coord 2 u
        pDstTmp[3*2*3 +  1] = pSrcTmp[dwSrcStride*1 +  5];  // vertex 1 texture coord 2 v
        pDstTmp[3*2*3 +  2] = pSrcTmp[dwSrcStride*2 +  4];  // vertex 2 texture coord 2 u
        pDstTmp[3*2*3 +  3] = pSrcTmp[dwSrcStride*2 +  5];  // vertex 2 texture coord 2 v
        pDstTmp[3*2*3 +  4] = pSrcTmp[dwSrcStride*3 +  4];  // vertex 3 texture coord 2 u
        pDstTmp[3*2*3 +  5] = pSrcTmp[dwSrcStride*3 +  5];  // vertex 3 texture coord 2 v
        break;

      case (D3DFVF_TEX4 >> D3DFVF_TEXCOUNT_SHIFT):  // 4 textures
//        pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwTexOffset); 
//        pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwTexOffset);
        pDstTmp[4*2*0 +  0] = pSrcTmp[dwSrcStride*0 +  0];  // vertex 0 texture coord 0 u
        pDstTmp[4*2*0 +  1] = pSrcTmp[dwSrcStride*0 +  1];  // vertex 0 texture coord 0 v
        pDstTmp[4*2*0 +  2] = pSrcTmp[dwSrcStride*1 +  0];  // vertex 1 texture coord 0 u
        pDstTmp[4*2*0 +  3] = pSrcTmp[dwSrcStride*1 +  1];  // vertex 1 texture coord 0 v
        pDstTmp[4*2*0 +  4] = pSrcTmp[dwSrcStride*2 +  0];  // vertex 2 texture coord 0 u
        pDstTmp[4*2*0 +  5] = pSrcTmp[dwSrcStride*2 +  1];  // vertex 2 texture coord 0 v
        pDstTmp[4*2*0 +  6] = pSrcTmp[dwSrcStride*3 +  0];  // vertex 3 texture coord 0 u
        pDstTmp[4*2*0 +  7] = pSrcTmp[dwSrcStride*3 +  1];  // vertex 3 texture coord 0 v
        pDstTmp[4*2*1 +  0] = pSrcTmp[dwSrcStride*0 +  2];  // vertex 0 texture coord 1 u
        pDstTmp[4*2*1 +  1] = pSrcTmp[dwSrcStride*0 +  3];  // vertex 0 texture coord 1 v
        pDstTmp[4*2*1 +  2] = pSrcTmp[dwSrcStride*1 +  2];  // vertex 1 texture coord 1 u
        pDstTmp[4*2*1 +  3] = pSrcTmp[dwSrcStride*1 +  3];  // vertex 1 texture coord 1 v
        pDstTmp[4*2*1 +  4] = pSrcTmp[dwSrcStride*2 +  2];  // vertex 2 texture coord 1 u
        pDstTmp[4*2*1 +  5] = pSrcTmp[dwSrcStride*2 +  3];  // vertex 2 texture coord 1 v
        pDstTmp[4*2*1 +  6] = pSrcTmp[dwSrcStride*3 +  2];  // vertex 3 texture coord 1 u
        pDstTmp[4*2*1 +  7] = pSrcTmp[dwSrcStride*3 +  3];  // vertex 3 texture coord 1 v
        pDstTmp[4*2*2 +  0] = pSrcTmp[dwSrcStride*0 +  4];  // vertex 0 texture coord 2 u
        pDstTmp[4*2*2 +  1] = pSrcTmp[dwSrcStride*0 +  5];  // vertex 0 texture coord 2 v
        pDstTmp[4*2*2 +  2] = pSrcTmp[dwSrcStride*1 +  4];  // vertex 1 texture coord 2 u
        pDstTmp[4*2*2 +  3] = pSrcTmp[dwSrcStride*1 +  5];  // vertex 1 texture coord 2 v
        pDstTmp[4*2*2 +  4] = pSrcTmp[dwSrcStride*2 +  4];  // vertex 2 texture coord 2 u
        pDstTmp[4*2*2 +  5] = pSrcTmp[dwSrcStride*2 +  5];  // vertex 2 texture coord 2 v
        pDstTmp[4*2*2 +  6] = pSrcTmp[dwSrcStride*3 +  4];  // vertex 3 texture coord 2 u
        pDstTmp[4*2*2 +  7] = pSrcTmp[dwSrcStride*3 +  5];  // vertex 3 texture coord 2 v
        pDstTmp[4*2*3 +  0] = pSrcTmp[dwSrcStride*0 +  6];  // vertex 0 texture coord 3 u
        pDstTmp[4*2*3 +  1] = pSrcTmp[dwSrcStride*0 +  7];  // vertex 0 texture coord 3 v
        pDstTmp[4*2*3 +  2] = pSrcTmp[dwSrcStride*1 +  6];  // vertex 1 texture coord 3 u
        pDstTmp[4*2*3 +  3] = pSrcTmp[dwSrcStride*1 +  7];  // vertex 1 texture coord 3 v
        pDstTmp[4*2*3 +  4] = pSrcTmp[dwSrcStride*2 +  6];  // vertex 2 texture coord 3 u
        pDstTmp[4*2*3 +  5] = pSrcTmp[dwSrcStride*2 +  7];  // vertex 2 texture coord 3 v
        pDstTmp[4*2*3 +  6] = pSrcTmp[dwSrcStride*3 +  6];  // vertex 3 texture coord 3 u
        pDstTmp[4*2*3 +  7] = pSrcTmp[dwSrcStride*3 +  7];  // vertex 3 texture coord 3 v
        break;

      case (D3DFVF_TEX5 >> D3DFVF_TEXCOUNT_SHIFT):  // 5 textures
//        pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwTexOffset); 
//        pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwTexOffset);
        pDstTmp[5*2*0 +  0] = pSrcTmp[dwSrcStride*0 +  0];  // vertex 0 texture coord 0 u
        pDstTmp[5*2*0 +  1] = pSrcTmp[dwSrcStride*0 +  1];  // vertex 0 texture coord 0 v
        pDstTmp[5*2*0 +  2] = pSrcTmp[dwSrcStride*1 +  0];  // vertex 1 texture coord 0 u
        pDstTmp[5*2*0 +  3] = pSrcTmp[dwSrcStride*1 +  1];  // vertex 1 texture coord 0 v
        pDstTmp[5*2*0 +  4] = pSrcTmp[dwSrcStride*2 +  0];  // vertex 2 texture coord 0 u
        pDstTmp[5*2*0 +  5] = pSrcTmp[dwSrcStride*2 +  1];  // vertex 2 texture coord 0 v
        pDstTmp[5*2*0 +  6] = pSrcTmp[dwSrcStride*3 +  0];  // vertex 3 texture coord 0 u
        pDstTmp[5*2*0 +  7] = pSrcTmp[dwSrcStride*3 +  1];  // vertex 3 texture coord 0 v
        pDstTmp[5*2*0 +  8] = pSrcTmp[dwSrcStride*0 +  2];  // vertex 0 texture coord 1 u
        pDstTmp[5*2*0 +  9] = pSrcTmp[dwSrcStride*0 +  3];  // vertex 0 texture coord 1 v
        pDstTmp[5*2*1 +  0] = pSrcTmp[dwSrcStride*1 +  2];  // vertex 1 texture coord 1 u
        pDstTmp[5*2*1 +  1] = pSrcTmp[dwSrcStride*1 +  3];  // vertex 1 texture coord 1 v
        pDstTmp[5*2*1 +  2] = pSrcTmp[dwSrcStride*2 +  2];  // vertex 2 texture coord 1 u
        pDstTmp[5*2*1 +  3] = pSrcTmp[dwSrcStride*2 +  3];  // vertex 2 texture coord 1 v
        pDstTmp[5*2*1 +  4] = pSrcTmp[dwSrcStride*3 +  2];  // vertex 3 texture coord 1 u
        pDstTmp[5*2*1 +  5] = pSrcTmp[dwSrcStride*3 +  3];  // vertex 3 texture coord 1 v
        pDstTmp[5*2*1 +  6] = pSrcTmp[dwSrcStride*0 +  4];  // vertex 0 texture coord 2 u
        pDstTmp[5*2*1 +  7] = pSrcTmp[dwSrcStride*0 +  5];  // vertex 0 texture coord 2 v
        pDstTmp[5*2*1 +  8] = pSrcTmp[dwSrcStride*1 +  4];  // vertex 1 texture coord 2 u
        pDstTmp[5*2*1 +  9] = pSrcTmp[dwSrcStride*1 +  5];  // vertex 1 texture coord 2 v
        pDstTmp[5*2*2 +  0] = pSrcTmp[dwSrcStride*2 +  4];  // vertex 2 texture coord 2 u
        pDstTmp[5*2*2 +  1] = pSrcTmp[dwSrcStride*2 +  5];  // vertex 2 texture coord 2 v
        pDstTmp[5*2*2 +  2] = pSrcTmp[dwSrcStride*3 +  4];  // vertex 3 texture coord 2 u
        pDstTmp[5*2*2 +  3] = pSrcTmp[dwSrcStride*3 +  5];  // vertex 3 texture coord 2 v
        pDstTmp[5*2*2 +  4] = pSrcTmp[dwSrcStride*0 +  6];  // vertex 0 texture coord 3 u
        pDstTmp[5*2*2 +  5] = pSrcTmp[dwSrcStride*0 +  7];  // vertex 0 texture coord 3 v
        pDstTmp[5*2*2 +  6] = pSrcTmp[dwSrcStride*1 +  6];  // vertex 1 texture coord 3 u
        pDstTmp[5*2*2 +  7] = pSrcTmp[dwSrcStride*1 +  7];  // vertex 1 texture coord 3 v
        pDstTmp[5*2*2 +  8] = pSrcTmp[dwSrcStride*2 +  6];  // vertex 2 texture coord 3 u
        pDstTmp[5*2*2 +  9] = pSrcTmp[dwSrcStride*2 +  7];  // vertex 2 texture coord 3 v
        pDstTmp[5*2*3 +  0] = pSrcTmp[dwSrcStride*3 +  6];  // vertex 3 texture coord 3 u
        pDstTmp[5*2*3 +  1] = pSrcTmp[dwSrcStride*3 +  7];  // vertex 3 texture coord 3 v
        pDstTmp[5*2*3 +  2] = pSrcTmp[dwSrcStride*0 +  8];  // vertex 0 texture coord 4 u
        pDstTmp[5*2*3 +  3] = pSrcTmp[dwSrcStride*0 +  9];  // vertex 0 texture coord 4 v
        pDstTmp[5*2*3 +  4] = pSrcTmp[dwSrcStride*1 +  8];  // vertex 1 texture coord 4 u
        pDstTmp[5*2*3 +  5] = pSrcTmp[dwSrcStride*1 +  9];  // vertex 1 texture coord 4 v
        pDstTmp[5*2*3 +  6] = pSrcTmp[dwSrcStride*2 +  8];  // vertex 2 texture coord 4 u
        pDstTmp[5*2*3 +  7] = pSrcTmp[dwSrcStride*2 +  9];  // vertex 2 texture coord 4 v
        pDstTmp[5*2*3 +  8] = pSrcTmp[dwSrcStride*3 +  8];  // vertex 3 texture coord 4 u
        pDstTmp[5*2*3 +  9] = pSrcTmp[dwSrcStride*3 +  9];  // vertex 3 texture coord 4 v
        break;

      case (D3DFVF_TEX6 >> D3DFVF_TEXCOUNT_SHIFT):  // 6 textures
//        pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwTexOffset); 
//        pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwTexOffset);
        pDstTmp[6*2*0 +  0] = pSrcTmp[dwSrcStride*0 +  0];  // vertex 0 texture coord 0 u
        pDstTmp[6*2*0 +  1] = pSrcTmp[dwSrcStride*0 +  1];  // vertex 0 texture coord 0 v
        pDstTmp[6*2*0 +  2] = pSrcTmp[dwSrcStride*1 +  0];  // vertex 1 texture coord 0 u
        pDstTmp[6*2*0 +  3] = pSrcTmp[dwSrcStride*1 +  1];  // vertex 1 texture coord 0 v
        pDstTmp[6*2*0 +  4] = pSrcTmp[dwSrcStride*2 +  0];  // vertex 2 texture coord 0 u
        pDstTmp[6*2*0 +  5] = pSrcTmp[dwSrcStride*2 +  1];  // vertex 2 texture coord 0 v
        pDstTmp[6*2*0 +  6] = pSrcTmp[dwSrcStride*3 +  0];  // vertex 3 texture coord 0 u
        pDstTmp[6*2*0 +  7] = pSrcTmp[dwSrcStride*3 +  1];  // vertex 3 texture coord 0 v
        pDstTmp[6*2*0 +  8] = pSrcTmp[dwSrcStride*0 +  2];  // vertex 0 texture coord 1 u
        pDstTmp[6*2*0 +  9] = pSrcTmp[dwSrcStride*0 +  3];  // vertex 0 texture coord 1 v
        pDstTmp[6*2*0 + 10] = pSrcTmp[dwSrcStride*1 +  2];  // vertex 1 texture coord 1 u
        pDstTmp[6*2*0 + 11] = pSrcTmp[dwSrcStride*1 +  3];  // vertex 1 texture coord 1 v
        pDstTmp[6*2*1 +  0] = pSrcTmp[dwSrcStride*2 +  2];  // vertex 2 texture coord 1 u
        pDstTmp[6*2*1 +  1] = pSrcTmp[dwSrcStride*2 +  3];  // vertex 2 texture coord 1 v
        pDstTmp[6*2*1 +  2] = pSrcTmp[dwSrcStride*3 +  2];  // vertex 3 texture coord 1 u
        pDstTmp[6*2*1 +  3] = pSrcTmp[dwSrcStride*3 +  3];  // vertex 3 texture coord 1 v
        pDstTmp[6*2*1 +  4] = pSrcTmp[dwSrcStride*0 +  4];  // vertex 0 texture coord 2 u
        pDstTmp[6*2*1 +  5] = pSrcTmp[dwSrcStride*0 +  5];  // vertex 0 texture coord 2 v
        pDstTmp[6*2*1 +  6] = pSrcTmp[dwSrcStride*1 +  4];  // vertex 1 texture coord 2 u
        pDstTmp[6*2*1 +  7] = pSrcTmp[dwSrcStride*1 +  5];  // vertex 1 texture coord 2 v
        pDstTmp[6*2*1 +  8] = pSrcTmp[dwSrcStride*2 +  4];  // vertex 2 texture coord 2 u
        pDstTmp[6*2*1 +  9] = pSrcTmp[dwSrcStride*2 +  5];  // vertex 2 texture coord 2 v
        pDstTmp[6*2*1 + 10] = pSrcTmp[dwSrcStride*3 +  4];  // vertex 3 texture coord 2 u
        pDstTmp[6*2*1 + 11] = pSrcTmp[dwSrcStride*3 +  5];  // vertex 3 texture coord 2 v
        pDstTmp[6*2*2 +  0] = pSrcTmp[dwSrcStride*0 +  6];  // vertex 0 texture coord 3 u
        pDstTmp[6*2*2 +  1] = pSrcTmp[dwSrcStride*0 +  7];  // vertex 0 texture coord 3 v
        pDstTmp[6*2*2 +  2] = pSrcTmp[dwSrcStride*1 +  6];  // vertex 1 texture coord 3 u
        pDstTmp[6*2*2 +  3] = pSrcTmp[dwSrcStride*1 +  7];  // vertex 1 texture coord 3 v
        pDstTmp[6*2*2 +  4] = pSrcTmp[dwSrcStride*2 +  6];  // vertex 2 texture coord 3 u
        pDstTmp[6*2*2 +  5] = pSrcTmp[dwSrcStride*2 +  7];  // vertex 2 texture coord 3 v
        pDstTmp[6*2*2 +  6] = pSrcTmp[dwSrcStride*3 +  6];  // vertex 3 texture coord 3 u
        pDstTmp[6*2*2 +  7] = pSrcTmp[dwSrcStride*3 +  7];  // vertex 3 texture coord 3 v
        pDstTmp[6*2*2 +  8] = pSrcTmp[dwSrcStride*0 +  8];  // vertex 0 texture coord 4 u
        pDstTmp[6*2*2 +  9] = pSrcTmp[dwSrcStride*0 +  9];  // vertex 0 texture coord 4 v
        pDstTmp[6*2*2 + 10] = pSrcTmp[dwSrcStride*1 +  8];  // vertex 1 texture coord 4 u
        pDstTmp[6*2*2 + 11] = pSrcTmp[dwSrcStride*1 +  9];  // vertex 1 texture coord 4 v
        pDstTmp[6*2*3 +  0] = pSrcTmp[dwSrcStride*2 +  8];  // vertex 2 texture coord 4 u
        pDstTmp[6*2*3 +  1] = pSrcTmp[dwSrcStride*2 +  9];  // vertex 2 texture coord 4 v
        pDstTmp[6*2*3 +  2] = pSrcTmp[dwSrcStride*3 +  8];  // vertex 3 texture coord 4 u
        pDstTmp[6*2*3 +  3] = pSrcTmp[dwSrcStride*3 +  9];  // vertex 3 texture coord 4 v
        pDstTmp[6*2*3 +  4] = pSrcTmp[dwSrcStride*0 + 10];  // vertex 0 texture coord 5 u
        pDstTmp[6*2*3 +  5] = pSrcTmp[dwSrcStride*0 + 11];  // vertex 0 texture coord 5 v
        pDstTmp[6*2*3 +  6] = pSrcTmp[dwSrcStride*1 + 10];  // vertex 1 texture coord 5 u
        pDstTmp[6*2*3 +  7] = pSrcTmp[dwSrcStride*1 + 11];  // vertex 1 texture coord 5 v
        pDstTmp[6*2*3 +  8] = pSrcTmp[dwSrcStride*2 + 10];  // vertex 2 texture coord 5 u
        pDstTmp[6*2*3 +  9] = pSrcTmp[dwSrcStride*2 + 11];  // vertex 2 texture coord 5 v
        pDstTmp[6*2*3 + 10] = pSrcTmp[dwSrcStride*3 + 10];  // vertex 3 texture coord 5 u
        pDstTmp[6*2*3 + 11] = pSrcTmp[dwSrcStride*3 + 11];  // vertex 3 texture coord 5 v
        break;

      case (D3DFVF_TEX7 >> D3DFVF_TEXCOUNT_SHIFT):  // 7 textures
//        pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwTexOffset); 
//        pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwTexOffset);
        pDstTmp[7*2*0 +  0] = pSrcTmp[dwSrcStride*0 +  0];  // vertex 0 texture coord 0 u
        pDstTmp[7*2*0 +  1] = pSrcTmp[dwSrcStride*0 +  1];  // vertex 0 texture coord 0 v
        pDstTmp[7*2*0 +  2] = pSrcTmp[dwSrcStride*1 +  0];  // vertex 1 texture coord 0 u
        pDstTmp[7*2*0 +  3] = pSrcTmp[dwSrcStride*1 +  1];  // vertex 1 texture coord 0 v
        pDstTmp[7*2*0 +  4] = pSrcTmp[dwSrcStride*2 +  0];  // vertex 2 texture coord 0 u
        pDstTmp[7*2*0 +  5] = pSrcTmp[dwSrcStride*2 +  1];  // vertex 2 texture coord 0 v
        pDstTmp[7*2*0 +  6] = pSrcTmp[dwSrcStride*3 +  0];  // vertex 3 texture coord 0 u
        pDstTmp[7*2*0 +  7] = pSrcTmp[dwSrcStride*3 +  1];  // vertex 3 texture coord 0 v
        pDstTmp[7*2*0 +  8] = pSrcTmp[dwSrcStride*0 +  2];  // vertex 0 texture coord 1 u
        pDstTmp[7*2*0 +  9] = pSrcTmp[dwSrcStride*0 +  3];  // vertex 0 texture coord 1 v
        pDstTmp[7*2*0 + 10] = pSrcTmp[dwSrcStride*1 +  2];  // vertex 1 texture coord 1 u
        pDstTmp[7*2*0 + 11] = pSrcTmp[dwSrcStride*1 +  3];  // vertex 1 texture coord 1 v
        pDstTmp[7*2*0 + 12] = pSrcTmp[dwSrcStride*2 +  2];  // vertex 2 texture coord 1 u
        pDstTmp[7*2*0 + 13] = pSrcTmp[dwSrcStride*2 +  3];  // vertex 2 texture coord 1 v
        pDstTmp[7*2*1 +  0] = pSrcTmp[dwSrcStride*3 +  2];  // vertex 3 texture coord 1 u
        pDstTmp[7*2*1 +  1] = pSrcTmp[dwSrcStride*3 +  3];  // vertex 3 texture coord 1 v
        pDstTmp[7*2*1 +  2] = pSrcTmp[dwSrcStride*0 +  4];  // vertex 0 texture coord 2 u
        pDstTmp[7*2*1 +  3] = pSrcTmp[dwSrcStride*0 +  5];  // vertex 0 texture coord 2 v
        pDstTmp[7*2*1 +  4] = pSrcTmp[dwSrcStride*1 +  4];  // vertex 1 texture coord 2 u
        pDstTmp[7*2*1 +  5] = pSrcTmp[dwSrcStride*1 +  5];  // vertex 1 texture coord 2 v
        pDstTmp[7*2*1 +  6] = pSrcTmp[dwSrcStride*2 +  4];  // vertex 2 texture coord 2 u
        pDstTmp[7*2*1 +  7] = pSrcTmp[dwSrcStride*2 +  5];  // vertex 2 texture coord 2 v
        pDstTmp[7*2*1 +  8] = pSrcTmp[dwSrcStride*3 +  4];  // vertex 3 texture coord 2 u
        pDstTmp[7*2*1 +  9] = pSrcTmp[dwSrcStride*3 +  5];  // vertex 3 texture coord 2 v
        pDstTmp[7*2*1 + 10] = pSrcTmp[dwSrcStride*0 +  6];  // vertex 0 texture coord 3 u
        pDstTmp[7*2*1 + 11] = pSrcTmp[dwSrcStride*0 +  7];  // vertex 0 texture coord 3 v
        pDstTmp[7*2*1 + 12] = pSrcTmp[dwSrcStride*1 +  6];  // vertex 1 texture coord 3 u
        pDstTmp[7*2*1 + 13] = pSrcTmp[dwSrcStride*1 +  7];  // vertex 1 texture coord 3 v
        pDstTmp[7*2*2 +  0] = pSrcTmp[dwSrcStride*2 +  6];  // vertex 2 texture coord 3 u
        pDstTmp[7*2*2 +  1] = pSrcTmp[dwSrcStride*2 +  7];  // vertex 2 texture coord 3 v
        pDstTmp[7*2*2 +  2] = pSrcTmp[dwSrcStride*3 +  6];  // vertex 3 texture coord 3 u
        pDstTmp[7*2*2 +  3] = pSrcTmp[dwSrcStride*3 +  7];  // vertex 3 texture coord 3 v
        pDstTmp[7*2*2 +  4] = pSrcTmp[dwSrcStride*0 +  8];  // vertex 0 texture coord 4 u
        pDstTmp[7*2*2 +  5] = pSrcTmp[dwSrcStride*0 +  9];  // vertex 0 texture coord 4 v
        pDstTmp[7*2*2 +  6] = pSrcTmp[dwSrcStride*1 +  8];  // vertex 1 texture coord 4 u
        pDstTmp[7*2*2 +  7] = pSrcTmp[dwSrcStride*1 +  9];  // vertex 1 texture coord 4 v
        pDstTmp[7*2*2 +  8] = pSrcTmp[dwSrcStride*2 +  8];  // vertex 2 texture coord 4 u
        pDstTmp[7*2*2 +  9] = pSrcTmp[dwSrcStride*2 +  9];  // vertex 2 texture coord 4 v
        pDstTmp[7*2*2 + 10] = pSrcTmp[dwSrcStride*3 +  8];  // vertex 3 texture coord 4 u
        pDstTmp[7*2*2 + 11] = pSrcTmp[dwSrcStride*3 +  9];  // vertex 3 texture coord 4 v
        pDstTmp[7*2*2 + 12] = pSrcTmp[dwSrcStride*0 + 10];  // vertex 0 texture coord 5 u
        pDstTmp[7*2*2 + 13] = pSrcTmp[dwSrcStride*0 + 11];  // vertex 0 texture coord 5 v
        pDstTmp[7*2*3 +  0] = pSrcTmp[dwSrcStride*1 + 10];  // vertex 1 texture coord 5 u
        pDstTmp[7*2*3 +  1] = pSrcTmp[dwSrcStride*1 + 11];  // vertex 1 texture coord 5 v
        pDstTmp[7*2*3 +  2] = pSrcTmp[dwSrcStride*2 + 10];  // vertex 2 texture coord 5 u
        pDstTmp[7*2*3 +  3] = pSrcTmp[dwSrcStride*2 + 11];  // vertex 2 texture coord 5 v
        pDstTmp[7*2*3 +  4] = pSrcTmp[dwSrcStride*3 + 10];  // vertex 3 texture coord 5 u
        pDstTmp[7*2*3 +  5] = pSrcTmp[dwSrcStride*3 + 11];  // vertex 3 texture coord 5 v
        pDstTmp[7*2*3 +  6] = pSrcTmp[dwSrcStride*0 + 12];  // vertex 0 texture coord 6 u
        pDstTmp[7*2*3 +  7] = pSrcTmp[dwSrcStride*0 + 13];  // vertex 0 texture coord 6 v
        pDstTmp[7*2*3 +  8] = pSrcTmp[dwSrcStride*1 + 12];  // vertex 1 texture coord 6 u
        pDstTmp[7*2*3 +  9] = pSrcTmp[dwSrcStride*1 + 13];  // vertex 1 texture coord 6 v
        pDstTmp[7*2*3 + 10] = pSrcTmp[dwSrcStride*2 + 12];  // vertex 2 texture coord 6 u
        pDstTmp[7*2*3 + 11] = pSrcTmp[dwSrcStride*2 + 13];  // vertex 2 texture coord 6 v
        pDstTmp[7*2*3 + 12] = pSrcTmp[dwSrcStride*3 + 12];  // vertex 3 texture coord 6 u
        pDstTmp[7*2*3 + 13] = pSrcTmp[dwSrcStride*3 + 13];  // vertex 3 texture coord 6 v
        break;

      case (D3DFVF_TEX8 >> D3DFVF_TEXCOUNT_SHIFT):  // 8 textures
//        pSrcTmp = (DWORD *) ((LPBYTE)pSrc + pRc->tl.InFVF.dwTexOffset); 
//        pDstTmp = (DWORD *) ((LPBYTE)pDst + pRc->tl.SOAFVF.dwTexOffset);
        pDstTmp[8*2*0 +  0] = pSrcTmp[dwSrcStride*0 +  0];  // vertex 0 texture coord 0 u
        pDstTmp[8*2*0 +  1] = pSrcTmp[dwSrcStride*0 +  1];  // vertex 0 texture coord 0 v
        pDstTmp[8*2*0 +  2] = pSrcTmp[dwSrcStride*1 +  0];  // vertex 1 texture coord 0 u
        pDstTmp[8*2*0 +  3] = pSrcTmp[dwSrcStride*1 +  1];  // vertex 1 texture coord 0 v
        pDstTmp[8*2*0 +  4] = pSrcTmp[dwSrcStride*2 +  0];  // vertex 2 texture coord 0 u
        pDstTmp[8*2*0 +  5] = pSrcTmp[dwSrcStride*2 +  1];  // vertex 2 texture coord 0 v
        pDstTmp[8*2*0 +  6] = pSrcTmp[dwSrcStride*3 +  0];  // vertex 3 texture coord 0 u
        pDstTmp[8*2*0 +  7] = pSrcTmp[dwSrcStride*3 +  1];  // vertex 3 texture coord 0 v
        pDstTmp[8*2*0 +  8] = pSrcTmp[dwSrcStride*0 +  2];  // vertex 0 texture coord 1 u
        pDstTmp[8*2*0 +  9] = pSrcTmp[dwSrcStride*0 +  3];  // vertex 0 texture coord 1 v
        pDstTmp[8*2*0 + 10] = pSrcTmp[dwSrcStride*1 +  2];  // vertex 1 texture coord 1 u
        pDstTmp[8*2*0 + 11] = pSrcTmp[dwSrcStride*1 +  3];  // vertex 1 texture coord 1 v
        pDstTmp[8*2*0 + 12] = pSrcTmp[dwSrcStride*2 +  2];  // vertex 2 texture coord 1 u
        pDstTmp[8*2*0 + 13] = pSrcTmp[dwSrcStride*2 +  3];  // vertex 2 texture coord 1 v
        pDstTmp[8*2*0 + 14] = pSrcTmp[dwSrcStride*3 +  2];  // vertex 3 texture coord 1 u
        pDstTmp[8*2*0 + 15] = pSrcTmp[dwSrcStride*3 +  3];  // vertex 3 texture coord 1 v
        pDstTmp[8*2*1 +  0] = pSrcTmp[dwSrcStride*0 +  4];  // vertex 0 texture coord 2 u
        pDstTmp[8*2*1 +  1] = pSrcTmp[dwSrcStride*0 +  5];  // vertex 0 texture coord 2 v
        pDstTmp[8*2*1 +  2] = pSrcTmp[dwSrcStride*1 +  4];  // vertex 1 texture coord 2 u
        pDstTmp[8*2*1 +  3] = pSrcTmp[dwSrcStride*1 +  5];  // vertex 1 texture coord 2 v
        pDstTmp[8*2*1 +  4] = pSrcTmp[dwSrcStride*2 +  4];  // vertex 2 texture coord 2 u
        pDstTmp[8*2*1 +  5] = pSrcTmp[dwSrcStride*2 +  5];  // vertex 2 texture coord 2 v
        pDstTmp[8*2*1 +  6] = pSrcTmp[dwSrcStride*3 +  4];  // vertex 3 texture coord 2 u
        pDstTmp[8*2*1 +  7] = pSrcTmp[dwSrcStride*3 +  5];  // vertex 3 texture coord 2 v
        pDstTmp[8*2*1 +  8] = pSrcTmp[dwSrcStride*0 +  6];  // vertex 0 texture coord 3 u
        pDstTmp[8*2*1 +  9] = pSrcTmp[dwSrcStride*0 +  7];  // vertex 0 texture coord 3 v
        pDstTmp[8*2*1 + 10] = pSrcTmp[dwSrcStride*1 +  6];  // vertex 1 texture coord 3 u
        pDstTmp[8*2*1 + 11] = pSrcTmp[dwSrcStride*1 +  7];  // vertex 1 texture coord 3 v
        pDstTmp[8*2*1 + 12] = pSrcTmp[dwSrcStride*2 +  6];  // vertex 2 texture coord 3 u
        pDstTmp[8*2*1 + 13] = pSrcTmp[dwSrcStride*2 +  7];  // vertex 2 texture coord 3 v
        pDstTmp[8*2*1 + 14] = pSrcTmp[dwSrcStride*3 +  6];  // vertex 3 texture coord 3 u
        pDstTmp[8*2*1 + 15] = pSrcTmp[dwSrcStride*3 +  7];  // vertex 3 texture coord 3 v
        pDstTmp[8*2*2 +  0] = pSrcTmp[dwSrcStride*0 +  8];  // vertex 0 texture coord 4 u
        pDstTmp[8*2*2 +  1] = pSrcTmp[dwSrcStride*0 +  9];  // vertex 0 texture coord 4 v
        pDstTmp[8*2*2 +  2] = pSrcTmp[dwSrcStride*1 +  8];  // vertex 1 texture coord 4 u
        pDstTmp[8*2*2 +  3] = pSrcTmp[dwSrcStride*1 +  9];  // vertex 1 texture coord 4 v
        pDstTmp[8*2*2 +  4] = pSrcTmp[dwSrcStride*2 +  8];  // vertex 2 texture coord 4 u
        pDstTmp[8*2*2 +  5] = pSrcTmp[dwSrcStride*2 +  9];  // vertex 2 texture coord 4 v
        pDstTmp[8*2*2 +  6] = pSrcTmp[dwSrcStride*3 +  8];  // vertex 3 texture coord 4 u
        pDstTmp[8*2*2 +  7] = pSrcTmp[dwSrcStride*3 +  9];  // vertex 3 texture coord 4 v
        pDstTmp[8*2*2 +  8] = pSrcTmp[dwSrcStride*0 + 10];  // vertex 0 texture coord 5 u
        pDstTmp[8*2*2 +  9] = pSrcTmp[dwSrcStride*0 + 11];  // vertex 0 texture coord 5 v
        pDstTmp[8*2*2 + 10] = pSrcTmp[dwSrcStride*1 + 10];  // vertex 1 texture coord 5 u
        pDstTmp[8*2*2 + 11] = pSrcTmp[dwSrcStride*1 + 11];  // vertex 1 texture coord 5 v
        pDstTmp[8*2*2 + 12] = pSrcTmp[dwSrcStride*2 + 10];  // vertex 2 texture coord 5 u
        pDstTmp[8*2*2 + 13] = pSrcTmp[dwSrcStride*2 + 11];  // vertex 2 texture coord 5 v
        pDstTmp[8*2*2 + 14] = pSrcTmp[dwSrcStride*3 + 10];  // vertex 3 texture coord 5 u
        pDstTmp[8*2*2 + 15] = pSrcTmp[dwSrcStride*3 + 11];  // vertex 3 texture coord 5 v
        pDstTmp[8*2*3 +  0] = pSrcTmp[dwSrcStride*0 + 12];  // vertex 0 texture coord 6 u
        pDstTmp[8*2*3 +  1] = pSrcTmp[dwSrcStride*0 + 13];  // vertex 0 texture coord 6 v
        pDstTmp[8*2*3 +  2] = pSrcTmp[dwSrcStride*1 + 12];  // vertex 1 texture coord 6 u
        pDstTmp[8*2*3 +  3] = pSrcTmp[dwSrcStride*1 + 13];  // vertex 1 texture coord 6 v
        pDstTmp[8*2*3 +  4] = pSrcTmp[dwSrcStride*2 + 12];  // vertex 2 texture coord 6 u
        pDstTmp[8*2*3 +  5] = pSrcTmp[dwSrcStride*2 + 13];  // vertex 2 texture coord 6 v
        pDstTmp[8*2*3 +  6] = pSrcTmp[dwSrcStride*3 + 12];  // vertex 3 texture coord 6 u
        pDstTmp[8*2*3 +  7] = pSrcTmp[dwSrcStride*3 + 13];  // vertex 3 texture coord 6 v
        pDstTmp[8*2*3 +  8] = pSrcTmp[dwSrcStride*0 + 14];  // vertex 0 texture coord 7 u
        pDstTmp[8*2*3 +  9] = pSrcTmp[dwSrcStride*0 + 15];  // vertex 0 texture coord 7 v
        pDstTmp[8*2*3 + 10] = pSrcTmp[dwSrcStride*1 + 14];  // vertex 1 texture coord 7 u
        pDstTmp[8*2*3 + 11] = pSrcTmp[dwSrcStride*1 + 15];  // vertex 1 texture coord 7 v
        pDstTmp[8*2*3 + 12] = pSrcTmp[dwSrcStride*2 + 14];  // vertex 2 texture coord 7 u
        pDstTmp[8*2*3 + 13] = pSrcTmp[dwSrcStride*2 + 15];  // vertex 2 texture coord 7 v
        pDstTmp[8*2*3 + 14] = pSrcTmp[dwSrcStride*3 + 14];  // vertex 3 texture coord 7 u
        pDstTmp[8*2*3 + 15] = pSrcTmp[dwSrcStride*3 + 15];  // vertex 3 texture coord 7 v
        break;

      default:
        break;
    } // end of texture switch()
}
#endif //0 - unused

/* Swizzles a 4x4 matrix into a 4x4x4 matrix
OLD MATRIX (4x4)         NEW MATRIX (4x4x4)
 _11 _12 _13 _14         _11 _11 _11 _11   _12 _12 _12 _12   _13 _13 _13 _13   _14 _14 _14 _14
 _21 _22 _23 _24         _21 _21 _21 _21   _22 _22 _22 _22   _23 _23 _23 _23   _24 _24 _24 _24
 _31 _32 _33 _34         _31 _31 _31 _31   _32 _32 _32 _32   _33 _33 _33 _33   _34 _34 _34 _34
 _41 _42 _43 _44         _41 _41 _41 _41   _42 _42 _42 _42   _43 _43 _43 _43   _44 _44 _44 _44
*/
void SwizzleMatrix(D3DMATRIX *pMat, SOA_MATRIX *pSOAMat4x4x4)
{
  _asm
  {
    mov     eax, pMat             // source 4x4 matrix, can be misaligned
    mov     edx, pSOAMat4x4x4     // 16-byte aligned destination 4x4x4 matrix

    movups  xmm0, [eax]D3DMATRIX._11  // _14 _13 _12 _11
    movups  xmm1, [eax]D3DMATRIX._21  // _24 _23 _22 _21
    movups  xmm2, [eax]D3DMATRIX._31  // _34 _33 _32 _31
    movups  xmm3, [eax]D3DMATRIX._41  // _44 _43 _42 _41

    movss   xmm4, xmm0            // --- --- --- _11
    movlhps xmm5, xmm0            // _12 _11 --- ---
    movhlps xmm6, xmm0            // --- --- _14 _13
    shufps  xmm4, xmm4, 0         // _11 _11 _11 _11
    shufps  xmm5, xmm5, 0xff      // _12 _12 _12 _12
    shufps  xmm6, xmm6, 0         // _13 _13 _13 _13
    shufps  xmm0, xmm0, 0xff      // _14 _14 _14 _14
    movaps  [edx]_SM._11, xmm4
    movaps  [edx]_SM._12, xmm5
    movaps  [edx]_SM._13, xmm6
    movaps  [edx]_SM._14, xmm0

    movss   xmm4, xmm1            // --- --- --- _21
    movlhps xmm5, xmm1            // _22 _21 --- ---
    movhlps xmm6, xmm1            // --- --- _24 _23
    shufps  xmm4, xmm4, 0         // _21 _21 _21 _21
    shufps  xmm5, xmm5, 0xff      // _22 _22 _22 _22
    shufps  xmm6, xmm6, 0         // _23 _23 _23 _23
    shufps  xmm1, xmm1, 0xff      // _24 _24 _24 _24
    movaps  [edx]_SM._21, xmm4
    movaps  [edx]_SM._22, xmm5
    movaps  [edx]_SM._23, xmm6
    movaps  [edx]_SM._24, xmm1

    movss   xmm4, xmm2            // --- --- --- _31
    movlhps xmm5, xmm2            // _32 _31 --- ---
    movhlps xmm6, xmm2            // --- --- _34 _33
    shufps  xmm4, xmm4, 0         // _31 _31 _31 _31
    shufps  xmm5, xmm5, 0xff      // _32 _32 _32 _32
    shufps  xmm6, xmm6, 0         // _33 _33 _33 _33
    shufps  xmm2, xmm2, 0xff      // _34 _34 _34 _34
    movaps  [edx]_SM._31, xmm4
    movaps  [edx]_SM._32, xmm5
    movaps  [edx]_SM._33, xmm6
    movaps  [edx]_SM._34, xmm2

    movss   xmm4, xmm3            // --- --- --- _41
    movlhps xmm5, xmm3            // _42 _41 --- ---
    movhlps xmm6, xmm3            // --- --- _44 _43
    shufps  xmm4, xmm4, 0         // _41 _41 _41 _41
    shufps  xmm5, xmm5, 0xff      // _42 _42 _42 _42
    shufps  xmm6, xmm6, 0         // _43 _43 _43 _43
    shufps  xmm3, xmm3, 0xff      // _44 _44 _44 _44
    movaps  [edx]_SM._41, xmm4
    movaps  [edx]_SM._42, xmm5
    movaps  [edx]_SM._43, xmm6
    movaps  [edx]_SM._44, xmm3
  }
}


/*-------------------------------------------------------------------
Function Name:  SwizzleSingleD3DtoSOA_Setup
Description:    Sets up the asm special-case swizzle/copy functions
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
void SwizzleSingleD3DtoSOA_Setup( RC *pRc )
{
	DWORD dwFVFType = pRc->tl.InFVF.dwFVFType;
	DWORD dwFVFEntries;
	DWORD numTxtures;
	DWORD *lpFnctPtr;

	switch (dwFVFType & D3DFVF_POSITION_MASK)
	{
		case D3DFVF_XYZ:	
			if (! (dwFVFType & D3DFVF_RESERVED1))	dwFVFEntries = 3;
			else									dwFVFEntries = 4;	// D3DLVERTEX (a special case)
			break;
		case D3DFVF_XYZRHW:	dwFVFEntries = 4;	break;
		case D3DFVF_XYZB1:	dwFVFEntries = 4;	break;
		case D3DFVF_XYZB2:	dwFVFEntries = 5;	break;
		case D3DFVF_XYZB3:	dwFVFEntries = 6;	break;
		case D3DFVF_XYZB4:	dwFVFEntries = 7;	break;
		case D3DFVF_XYZB5:	dwFVFEntries = 8;	break;
		default:			dwFVFEntries = 0;	break;	// no position ... BAD
	}
    if (dwFVFType & D3DFVF_NORMAL)		dwFVFEntries += 3;
    if (dwFVFType & D3DFVF_DIFFUSE)		dwFVFEntries += 1;
    if (dwFVFType & D3DFVF_SPECULAR)	dwFVFEntries += 1;

	lpFnctPtr = (DWORD*) &SwizzleSngl_SwizParmsFunctions;
	pRc->tl.KniSwizParmsFunct = (KniSwizSingle_SwizParms_FunctionType*) lpFnctPtr[dwFVFEntries];

	numTxtures = (dwFVFType & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	lpFnctPtr = (DWORD*) &SwizzleSngl_CopyTxtrsFunctions;
	pRc->tl.KniCpyTxtrFunct = (KniSwizSingle_CopyTxtrs_FunctionType*) lpFnctPtr[numTxtures];
}


#endif	//TnL_HAL && VCPP
#endif	//DX7
/*
** Copyright (c) 2000, 3Dfx Interactive, Inc.
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
** File name: k3dtnl.c
**
** Description: Transformation and Lighting Code for 3DNow verticies
**
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




/*-------------------------------------------------------------------
Function Name:  ComputeClipCodesK3D_C
Description:    Computes clip codes from homogeneous coordinates
Caller:			FP_XformPositionClippedK3d_C
Parameters:   
Information:    
Return:         
-------------------------------------------------------------------*/
#if 0 // UNUSED - don't delete
__inline void ComputeClipCodesK3D_C(RC *pRc, TLBN *pDst)
{
	TL_K3DTMP *pTlkTmp = (TL_K3DTMP *)pRc->tl.pTLK;
	D3DVECTOR	v1, v2;
	D3DVALUE	we;
	TLCLIPCODE	clip_code, clipBit;
	int j;

	v1.x = pDst->x;
	v1.y = pDst->y;
	v1.z = pDst->z;
	we   = pDst->w;
	v2.x = we - v1.x;
	v2.y = we - v1.y;
	v2.z = we - v1.z;

	/*  if (x < 0)  clip |= RRCLIP_LEFTBIT;    */
	/*  if (x >= we) clip |= RRCLIP_RIGHTBIT;  */
	/*  if (y < 0)  clip |= RRCLIP_BOTTOMBIT;  */
	/*  if (y >= we) clip |= RRCLIP_TOPBIT;    */
	/*  if (z < 0)    clip |= RRCLIP_FRONTBIT; */
	/*  if (z >= we) clip |= RRCLIP_BACKBIT;   */
	clip_code = ((AS_INT32(v1.x) & 0x80000000) >> (32-TLCLIP_LEFTBIT))   |
                ((AS_INT32(v1.y) & 0x80000000) >> (32-TLCLIP_BOTTOMBIT)) |
                ((AS_INT32(v1.z) & 0x80000000) >> (32-TLCLIP_FRONTBIT))  |
                ((AS_INT32(v2.x) & 0x80000000) >> (32-TLCLIP_RIGHTBIT))  |
                ((AS_INT32(v2.y) & 0x80000000) >> (32-TLCLIP_TOPBIT))    |
                ((AS_INT32(v2.z) & 0x80000000) >> (32-TLCLIP_BACKBIT));

	clipBit = TLCLIP_USERCLIPPLANE0;
	for( j=0; j<TLMAX_USER_CLIPPLANES; j++)
	{
		if( pRc->tl.xfmUserClipPlanes[j].bActive )
		{
			TLVECTOR4 *plane = &pRc->tl.xfmUserClipPlanes[j].plane;
			if( (v1.x*plane->x + v1.y*plane->y + v1.z*plane->z + we*plane->w) < 0.0f )
				clip_code |= clipBit;
		}
		clipBit <<= 1;
	}

	if ((clip_code != 0) && (pRc->tl.dwTLState & TLPV_GUARDBAND))
	{
		// We do guardband check in the projection space, so
		// we transform X and Y of the vertex there
		D3DVALUE xnew = v1.x * pTlkTmp->gb11 + we * pTlkTmp->gb41;
		D3DVALUE ynew = v1.y * pTlkTmp->gb22 + we * pTlkTmp->gb42;
		D3DVALUE xx = we - xnew;
		D3DVALUE yy = we - ynew;
		clip_code |= ((AS_INT32(xnew) & 0x80000000) >> (32-TLCLIPGB_LEFTBIT))   |
		             ((AS_INT32(ynew) & 0x80000000) >> (32-TLCLIPGB_BOTTOMBIT)) |
		             ((AS_INT32(xx)   & 0x80000000) >> (32-TLCLIPGB_RIGHTBIT))  |
		             ((AS_INT32(yy)   & 0x80000000) >> (32-TLCLIPGB_TOPBIT));
    }

	// write to the dst vtx
	pDst->clip_code = clip_code;

} //ComputeClipCodesK3D_C()
#endif // 0

#if 0 // UNUSED - don't delete
// This is now inlined in the clipped special case code, so don't use
_inline void ComputeClipCodesK3D_Asm(RC *pRc, TLBN *pDst)
{
	_asm {
	// this is already loaded
		mov			ecx, [pRc]					// will have to reload this after calling ComputeClipCodesK3D
		mov			ebx, [ecx]RC.tl.pTLK		// TL_K3DTMP *pTlkTmp
		mov			edi, pDst;					// output vertex list

		movq		MM0, [edi]TLBN.x			/*	hy	hx	*/
		movd		MM1, [edi]TLBN.w			/*  hz	hw 	*/  // not really
		movd		MM4, [edi]TLBN.z			/*	hz	*/
		punpckldq	MM1, MM4					/*  hz	hw 	*/
	// end of preloaded stuff

		pxor		mm6, mm6
		pxor		mm7, mm7
		punpckldq	mm1, mm1					/*	hw		hw		*/
		movq		mm2, mm0					/*	hy		yx		*/
		movq		mm3, mm4					/*	--		hz		*/

		pfcmpgt		mm6, mm0					/*	y < 0	x < 0	*/
		pfcmpgt		mm7, mm4					/*	-		z < 0	*/
		pfcmpgt		mm2, mm1					/*	y > w	x > w	*/
		pfcmpgt		mm3, mm1					/*	-		z > w	*/

		pand		mm6, [TLCLIPMASK_LEFT_BOTTOM]
		pand		mm7, [TLCLIPMASK_FRONT_0]
		pand		mm2, [TLCLIPMASK_RIGHT_TOP]
		pand		mm3, [TLCLIPMASK_BACK_0]
		por			mm7, mm6
		por			mm3, mm2
		por			mm7, mm3
		movq		mm6, mm7
		punpckhdq	mm7, mm7
		por			mm7, mm6					/* clip_code */

		movd		mm5, [TL_userclip_mask]		/*	0		TLCLIP_USERCLIPPLANE0	*/
		punpckldq	mm4, mm1					/* hw		hz		*/
		// mm0=x,y  mm4=z,w  mm5=TLCLIP_USERCLIPPLANE0  mm7=clip_code

		mov			eax, TLMAX_USER_CLIPPLANES
		lea			edx, [ecx]RC.tl.xfmUserClipPlanes
		jmp			CheckClipBit

	NextUserClipPlane:
		add			edx, SIZE TLUSERCLIPPLANE
		pslld		mm5, 1
		dec			eax
		jz			UserClipPlanesDone

	CheckClipBit:
		cmp			[edx]TLUSERCLIPPLANE.bActive, 0	//Is this plane enabled?
		jz			NextUserClipPlane

		movq		mm2, [edx]TLUSERCLIPPLANE.plane.x	/*	h.y		h.x		*/
		movq		mm3, [edx]TLUSERCLIPPLANE.plane.z	/*	h.w		h.z		*/
		pfmul		mm2, mm0					/*	h.y*plane.y	h.x*plane.x	*/
		pfmul		mm3, mm4					/*	h.w*plane.w	h.z*plane.z	*/
		pfadd		mm2, mm3
		pxor		mm6, mm6					/*	0		0		*/
		pfacc		mm2, mm2					/*	x*x+y*y+z*z+w*w	*/
		pcmpgtd		mm6, mm2					/*	x*x+y*y+z*z+w*w < 0.0f */
		pand		mm6, mm5					/*	x*x+y*y+z*z+w*w < 0.0f ? clipbit : 0	*/
		por			mm7, mm6					/*	clip_code |= clipbit	*/
		jmp			NextUserClipPlane

	UserClipPlanesDone:


		//if ((clip_code != 0) && (pRc->tl.dwTLState & TLPV_GUARDBAND))
		test		[ecx]RC.tl.dwTLState, TLPV_GUARDBAND
		movd		eax, mm7					/* eax = mm7 = clip_code	*/
		jz			GuardbandDone
		cmp			eax, 0
		jz			GuardbandDone

		// We do guardband check in the projection space, so
		// we transform X and Y of the vertex there
		punpckhdq	mm4, mm4					/*	w		w		*/
		pfmul		mm0, [ebx]TL_K3DTMP.gb11	/*	y*gb12	x*gb11	*/
		movq		mm5, mm4					/*	w		w		*/
		pfmul		mm4, [ebx]TL_K3DTMP.gb41	/*	w*gb42	w*gb41	*/
		pxor		mm1, mm1					/*	0		0		*/
		pfadd		mm0, mm4					/*	 y*gb22+w*gb42   x*gb11*w*gb41  */
		pfcmpgt		mm1, mm0					/*	y*gb22+w*gb42<0  x*gb11*w*gb41<0 */
		pfcmpgt		mm0, mm5					/*	y*gb22+w*gb42>w  x*gb11*w*gb41>w */

		pand		mm1, [TLCLIPGBMASK_LEFT_BOTTOM]
		pand		mm0, [TLCLIPGBMASK_RIGHT_TOP]
		por			mm0, mm1
		por			mm7, mm0
		punpckhdq	mm0, mm0
		por			mm7, mm0

	GuardbandDone:

		movd		[edi]TLBN.clip_code, mm7
	FEMMS
	}
} //ComputeClipCodesK3D_Asm()
#endif // 0




/*-------------------------------------------------------------------
Function Name:  FP_XformPositionK3d_C and FP_XformPositionClippedK3d_C
Description:    Transforms input vertex position to 
				homogeneous coordinates, and converts these to
				device coordinates.
				Also copies the colors and textures from the input 
				vertex to the output vertex.  Therefore, we'll never 
				need the input vertex again (unless we light) which 
				simplifies the cacheability and prefetching.
Parameters:   
Information:    x = (((x * 1/w) * scaleX) + offsetX) + pixelOffset
				y = (((y * 1/w) * scaleY) + offsetY) + pixelOffset
				z = hz (fix it later)
Return:         
-------------------------------------------------------------------*/
#if 0 // UNUSED - don't delete
void FP_XformPositionK3d_C(RC *pRc, DWORD *pSrc, TLBN *pDst, DWORD numVerts)
{
	TLVECTOR4 *phv;		// ptr to the homogeneous coordinates
	DWORD dwStride = pRc->tl.InFVF.dwStride;
	DWORD dwDiffuseOffset = pRc->tl.InFVF.dwDiffuseOffset;
	DWORD dwSpecularOffset = pRc->tl.InFVF.dwSpecularOffset;
	DWORD *pDiff, *pSpec;
	DWORD dwTexOffset = pRc->tl.InFVF.dwTexOffset;
	DWORD dwTexOffset0 = dwTexOffset + (pRc->t0CoordIndex << 2);
	DWORD dwTexOffset1 = dwTexOffset + (pRc->t1CoordIndex << 2);
	DWORD RfBits = pRc->tl.KniRC.RfBits;
	D3DVALUE w;
	DWORD i;


	for(i=0; i<numVerts; ++i)
	{
		// Transform the position to clipping space, store it in the dest vertex
		phv = (TLVECTOR4*)&pDst->x;
		XformBy4x4( (D3DVECTOR*)pSrc, pRc->tl.lpxfmCurrent[0], phv );

		//Xform_DevPositionK3d_C(pRc, pDst);
		w = 1.0f / phv->w;
		pDst->x = (w * phv->x * pRc->tl.ViewData.scaleX) + pRc->tl.ViewData.viewDataOffsetX;
		pDst->y = (w * phv->y * pRc->tl.ViewData.scaleY) + pRc->tl.ViewData.viewDataOffsetY;
		pDst->z = phv->z;		// don't bother scaleing yet, just store it
		pDst->w = w;
		*(DWORD*)(&pDst->wfbi) = TL_VERT_XFORMED_NOT_LIT;

		// copy the colors
		pDiff = (DWORD*)((LPBYTE)pSrc + dwDiffuseOffset);
		pDst->diffuse = *pDiff;
		pSpec = (DWORD*)((LPBYTE)pSrc + dwSpecularOffset);
		pDst->specular = *pSpec;

		// increment the pointers
		pSrc = (DWORD*)((LPBYTE)pSrc + dwStride);
		pDst = (TLBN*)((LPBYTE)pDst + TLBN_SIZE);
	}
}

void FP_XformPositionClippedK3d_C(RC *pRc, DWORD *pSrc, TLBN *pDst, DWORD numVerts)
{
	TLVECTOR4 *phv;		// ptr to the homogeneous coordinates
	DWORD dwStride = pRc->tl.InFVF.dwStride;
	DWORD dwDiffuseOffset = pRc->tl.InFVF.dwDiffuseOffset;
	DWORD dwSpecularOffset = pRc->tl.InFVF.dwSpecularOffset;
	DWORD *pDiff, *pSpec;
	DWORD dwTexOffset = pRc->tl.InFVF.dwTexOffset;
	DWORD dwTexOffset0 = dwTexOffset + (pRc->t0CoordIndex << 2);
	DWORD dwTexOffset1 = dwTexOffset + (pRc->t1CoordIndex << 2);
	DWORD RfBits = pRc->tl.KniRC.RfBits;
	D3DVALUE w;
	DWORD i;


	for(i=0; i<(int)numVerts; ++i)
	{
		// Transform the position to clipping space, store it in the homogeneous coordinates
		//phv = (TLVECTOR4*)&pDst->hx;
		phv = (TLVECTOR4*)&pDst->x;	// scratch that, put them in screen coords until we generate clip codes
		XformBy4x4( (D3DVECTOR*)pSrc, pRc->tl.lpxfmCurrent[0], phv );
		pDst->w = phv->w;	// load w (these w's aren't the same)

    	ComputeClipCodesK3D_C(pRc, pDst);

		//Xform_DevPositionClippedK3d_C(pRc, pDst);
		w = 1.0f / pDst->w;
		pDst->x = (w * phv->x * pRc->tl.ViewData.scaleX) + pRc->tl.ViewData.viewDataOffsetX;
		pDst->y = (w * phv->y * pRc->tl.ViewData.scaleY) + pRc->tl.ViewData.viewDataOffsetY;
		pDst->z = phv->z;		// don't bother scaleing yet, just store it
		pDst->w = w;
		*(DWORD*)(&pDst->wfbi) = TL_VERT_XFORMED_NOT_LIT;

		// copy the colors
		pDiff = (DWORD*)((LPBYTE)pSrc + dwDiffuseOffset);
		pDst->diffuse = *pDiff;
		pSpec = (DWORD*)((LPBYTE)pSrc + dwSpecularOffset);
		pDst->specular = *pSpec;

		// increment the pointers
		pSrc = (DWORD*) ((LPBYTE)pSrc + pRc->tl.InFVF.dwStride);
		pDst = (TLBN*)((LPBYTE)pDst + TLBN_SIZE);
	}
}
#endif //0



/*----------------------------------------------------------------------------------
Function Name:  FP_XformPositionK3d_Asm_XXX
Description:    Special cased versions
Parameters:   
Information:    
Return:         
----------------------------------------------------------------------------------*/
// The MSVC compiler seems to have an intermittant bug where it complains about
// macro redefinition.  These macros ARE #undef'd in k3dtnl.h and since it works 
// intermittantly, I figure it's a compiler bug.
#pragma warning( push )
#pragma warning(disable : 4005)  /* "macro redefinition" */

// Case 1
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C0_U0_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 2
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C0_U0_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 3
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C0_U0_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 4
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C0_U0_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES  	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 5
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C0_U1_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 6
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C0_U1_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 7
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C0_U1_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 8
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C0_U1_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES  	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 9
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C1_U0_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 10
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C1_U0_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 11
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C1_U0_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 12
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C1_U0_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 13
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C1_U1_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 14
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C1_U1_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 15
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C1_U1_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 16
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B0_C1_U1_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	0
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

///// 1 vertex blend
// Case 17
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C0_U0_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 18
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C0_U0_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 19
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C0_U0_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 20
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C0_U0_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES  	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 21
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C0_U1_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 22
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C0_U1_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 23
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C0_U1_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 24
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C0_U1_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES  	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 25
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C1_U0_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 26
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C1_U0_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 27
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C1_U0_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 28
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C1_U0_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 29
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C1_U1_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 30
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C1_U1_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 31
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C1_U1_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 32
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B1_C1_U1_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	1
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

///// 2 vertex blends
// Case 33
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C0_U0_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 34
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C0_U0_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 35
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C0_U0_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 36
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C0_U0_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES  	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 37
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C0_U1_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 38
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C0_U1_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 39
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C0_U1_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 40
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C0_U1_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES  	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 41
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C1_U0_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 42
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C1_U0_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 43
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C1_U0_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 44
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C1_U0_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 45
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C1_U1_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 46
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C1_U1_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 47
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C1_U1_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 48
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B2_C1_U1_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	2
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

///// 3 vertex blends
// Case 49
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C0_U0_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 50
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C0_U0_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 51
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C0_U0_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 52
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C0_U0_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES  	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 53
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C0_U1_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 54
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C0_U1_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 55
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C0_U1_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES 	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 56
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C0_U1_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			0
#define K3DXFORM_USERCLIPPPLANES  	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 57
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C1_U0_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 58
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C1_U0_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 59
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C1_U0_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 60
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C1_U0_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	0
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 61
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C1_U1_D0_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 62
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C1_U1_D0_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		0
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

// Case 63
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C1_U1_D1_S0
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		0
#include "k3dtnl.h"

// Case 64
#define K3DXFORM_NAME				FP_XformPositionK3d_Asm_B3_C1_U1_D1_S1
#define K3DXFORM_NUM_VERTEX_BLENDS	3
#define K3DXFORM_CLIPPED			1
#define K3DXFORM_USERCLIPPPLANES	1
#define K3DXFORM_DIFFUSE_COLOR		1
#define K3DXFORM_SPECULAR_COLOR		1
#include "k3dtnl.h"

#pragma warning( pop ) /* restore the compiler warnings */

/*
*	Just about every possibility is valid here, even non-clipped 
*	with user clip planes (found that with WHQL).  
*/
const XFORMFNP K3D_Xform_fns[] = { 
	// no vertex blends, no clipping, no user clip planes
	&FP_XformPositionK3d_Asm_B0_C0_U0_D0_S0,	&FP_XformPositionK3d_Asm_B0_C0_U0_D0_S1,
	&FP_XformPositionK3d_Asm_B0_C0_U0_D1_S0,	&FP_XformPositionK3d_Asm_B0_C0_U0_D1_S1,
	// no vertex blends, no clipping, user clip planes
	&FP_XformPositionK3d_Asm_B0_C0_U1_D0_S0,	&FP_XformPositionK3d_Asm_B0_C0_U1_D0_S1,
	&FP_XformPositionK3d_Asm_B0_C0_U1_D1_S0,	&FP_XformPositionK3d_Asm_B0_C0_U1_D1_S1,
	// no vertex blends, clipping w/o user clip planes
	&FP_XformPositionK3d_Asm_B0_C1_U0_D0_S0,	&FP_XformPositionK3d_Asm_B0_C1_U0_D0_S1,
	&FP_XformPositionK3d_Asm_B0_C1_U0_D1_S0,	&FP_XformPositionK3d_Asm_B0_C1_U0_D1_S1,
	// no vertex blends, clipping w/user clip planes
	&FP_XformPositionK3d_Asm_B0_C1_U1_D0_S0,	&FP_XformPositionK3d_Asm_B0_C1_U1_D0_S1,
	&FP_XformPositionK3d_Asm_B0_C1_U1_D1_S0,	&FP_XformPositionK3d_Asm_B0_C1_U1_D1_S1,
	// 1 vertex blend, no clipping, no user clip planes
	&FP_XformPositionK3d_Asm_B1_C0_U0_D0_S0,	&FP_XformPositionK3d_Asm_B1_C0_U0_D0_S1,
	&FP_XformPositionK3d_Asm_B1_C0_U0_D1_S0,	&FP_XformPositionK3d_Asm_B1_C0_U0_D1_S1,
	// no vertex blends, no clipping, user clip planes
	&FP_XformPositionK3d_Asm_B1_C0_U1_D0_S0,	&FP_XformPositionK3d_Asm_B1_C0_U1_D0_S1,
	&FP_XformPositionK3d_Asm_B1_C0_U1_D1_S0,	&FP_XformPositionK3d_Asm_B1_C0_U1_D1_S1,
	// no vertex blends, clipping w/o user clip planes
	&FP_XformPositionK3d_Asm_B1_C1_U0_D0_S0,	&FP_XformPositionK3d_Asm_B1_C1_U0_D0_S1,
	&FP_XformPositionK3d_Asm_B1_C1_U0_D1_S0,	&FP_XformPositionK3d_Asm_B1_C1_U0_D1_S1,
	// no vertex blends, clipping w/user clip planes
	&FP_XformPositionK3d_Asm_B1_C1_U1_D0_S0,	&FP_XformPositionK3d_Asm_B1_C1_U1_D0_S1,
	&FP_XformPositionK3d_Asm_B1_C1_U1_D1_S0,	&FP_XformPositionK3d_Asm_B1_C1_U1_D1_S1,
	// 2 vertex blends, no clipping, no user clip planes
	&FP_XformPositionK3d_Asm_B2_C0_U0_D0_S0,	&FP_XformPositionK3d_Asm_B2_C0_U0_D0_S1,
	&FP_XformPositionK3d_Asm_B2_C0_U0_D1_S0,	&FP_XformPositionK3d_Asm_B2_C0_U0_D1_S1,
	// no vertex blends, no clipping, user clip planes
	&FP_XformPositionK3d_Asm_B2_C0_U1_D0_S0,	&FP_XformPositionK3d_Asm_B2_C0_U1_D0_S1,
	&FP_XformPositionK3d_Asm_B2_C0_U1_D1_S0,	&FP_XformPositionK3d_Asm_B2_C0_U1_D1_S1,
	// no vertex blends, clipping w/o user clip planes
	&FP_XformPositionK3d_Asm_B2_C1_U0_D0_S0,	&FP_XformPositionK3d_Asm_B2_C1_U0_D0_S1,
	&FP_XformPositionK3d_Asm_B2_C1_U0_D1_S0,	&FP_XformPositionK3d_Asm_B2_C1_U0_D1_S1,
	// no vertex blends, clipping w/user clip planes
	&FP_XformPositionK3d_Asm_B2_C1_U1_D0_S0,	&FP_XformPositionK3d_Asm_B2_C1_U1_D0_S1,
	&FP_XformPositionK3d_Asm_B2_C1_U1_D1_S0,	&FP_XformPositionK3d_Asm_B2_C1_U1_D1_S1,
	// 3 vertex blends, no clipping, no user clip planes
	&FP_XformPositionK3d_Asm_B3_C0_U0_D0_S0,	&FP_XformPositionK3d_Asm_B3_C0_U0_D0_S1,
	&FP_XformPositionK3d_Asm_B3_C0_U0_D1_S0,	&FP_XformPositionK3d_Asm_B3_C0_U0_D1_S1,
	// no vertex blends, no clipping, user clip planes
	&FP_XformPositionK3d_Asm_B3_C0_U1_D0_S0,	&FP_XformPositionK3d_Asm_B3_C0_U1_D0_S1,
	&FP_XformPositionK3d_Asm_B3_C0_U1_D1_S0,	&FP_XformPositionK3d_Asm_B3_C0_U1_D1_S1,
	// no vertex blends, clipping w/o user clip planes
	&FP_XformPositionK3d_Asm_B3_C1_U0_D0_S0,	&FP_XformPositionK3d_Asm_B3_C1_U0_D0_S1,
	&FP_XformPositionK3d_Asm_B3_C1_U0_D1_S0,	&FP_XformPositionK3d_Asm_B3_C1_U0_D1_S1,
	// no vertex blends, clipping w/user clip planes
	&FP_XformPositionK3d_Asm_B3_C1_U1_D0_S0,	&FP_XformPositionK3d_Asm_B3_C1_U1_D0_S1,
	&FP_XformPositionK3d_Asm_B3_C1_U1_D1_S0,	&FP_XformPositionK3d_Asm_B3_C1_U1_D1_S1,
};



#if 0
// This was used to get the 3DNow path working
extern void Xform_4_SOA_C(SOA_XYZW *pD, SOA_XYZ *pV, TLMATRIX *pMat4x4x4);
extern void Xform_3_SOA_C(SOA_XYZ *pD, SOA_XYZ *pV, TLMATRIX *pMat4x4x4);
extern void Normalize3SOA_C(SOA_XYZ *pV);
extern void ComputeClipCodes_SOA_C(RC *pRc);
extern void FogVertexSOA_C(RC* pRc);
extern void Xform_DevColor_SOA_C(RC *pRc);
extern void Xform_DevCoord_SOA_C(RC *pRc, DWORD *pTex0, DWORD *pTex1);
void FP_Xform_Light_4Vert_SOA_C(RC *pRc, DWORD *pSOA)
{
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

  // First, Transform the position to clipping space
  Xform_4_SOA_C(&pTlTmp->SOAhv, pPos, pRc->tl.lpxfmCurrent[0]);

  // Compute the clipping codes -- this will likely be inlined with 
  // transformation to camera space
  if (dwTLState & TLPV_DOCLIPPING)
    ComputeClipCodes_SOA_C(pRc);

  if (dwTLState & TLPV_DIFF_SRC_VTX)
    // Xform_DevColor_SOA is going to copy diffuse color from input vertex
    pRc->tl.lighting.pdSOADiffRGBSrc = pDiffuse;

  if (dwTLState & TLPV_SPEC_SRC_VTX)
    // Xform_DevColor_SOA is going to copy specular color from input vertex
    pRc->tl.lighting.pdSOASpecRGBSrc = pSpecular;

  if (dwTLState & (TLPV_DOLIGHTING | TLPV_DOFOG)) 
  {
    Xform_3_SOA_C(&pTlTmp->SOAcv, pPos, pRc->tl.lpxfmToEye[0]);

    if (dwTLState & TLPV_DOLIGHTING) 
    {
      Xform_3_SOA_C(&pTlTmp->SOAcn, pNorm, pRc->tl.lpxfmToEyeInvT);

      if (dwTLState & TLPV_NORMALIZENORMALS)
        Normalize3SOA_C(&pTlTmp->SOAcn);

       // Perform lighting if needed
       //pRc->tl.lighting.pfnLightVertex(pRc);
       LightVertexSOA(pRc); //old stuff
     }

	// Compute vertex fog if needed
    if (dwTLState & TLPV_DOFOG)
      FogVertexSOA_C(pRc);
  }

  Xform_DevColor_SOA_C(pRc);

  // Finally, convert to device coordinates
  Xform_DevCoord_SOA_C(pRc, pTex0, pTex1);
  _asm emms
}
#endif //0

/*-------------------------------------------------------------------
Function Name:  FP_XformLightK3d_C
Description:    Handles the rest of the transformations, lights
				the verticies, and puts everything in the proper
				devicde coordintates in the output verts
Parameters:   
Information:    
	z = (((z * 1/w) * scaleZ) + offsetZ) * zScale
	diff & specular come from lighting or source vertex (or are 0)
	specular may require alpha from fog or material
	fog comes from FogVertex, material, or specular and either 
	goes in z or wfbi
	wfbi has either fog or w.  
	Bit 0 of wfbi is the non-lit bit.  If it's set, this function has 
	not processed the vertex.  This function clears this bit.  Since 
	it's the lsb of the mantissa, the error will be 1/8M.  
Return:         
-------------------------------------------------------------------*/
#if 0 // UNUSED - don't delete yet
extern float FogVertexK3D_C(RC* pRc);
void FP_XformLightK3d_C( RC *pRc, DWORD idx, TLBN *pDst )	
{
	TL_K3DTMP *pTlkTmp = (TL_K3DTMP *)pRc->tl.pTLK;
	TLLIGHTING *Ldata = &pRc->tl.lighting;
	DWORD dwTLState = pRc->tl.dwTLState;
	DWORD RfBits = pRc->tl.KniRC.RfBits;
	DWORD diff;
	float fFog;
	D3DVALUE w;
	TLBN *pSrc = (TLBN*)((LPBYTE)pRc->tl.InFVF.lpvData + (idx * pRc->tl.InFVF.dwStride));	// input vertex
	TextureType *pTextures = (TextureType*)((DWORD)pSrc + pRc->tl.InFVF.dwTexOffset);

	if (dwTLState & (TLPV_DOLIGHTING | TLPV_DOFOG | TLPV_DOTEXGEN))
	{
		XformBy4x3( (D3DVECTOR*)&pSrc->x, pRc->tl.lpxfmToEye[0], &pTlkTmp->cv );

		if (dwTLState & TLPV_DOLIGHTING) 
		{
			D3DVECTOR *pvNorm = (D3DVECTOR*)((LPBYTE)pSrc + pRc->tl.InFVF.dwNormalOffset);
			XformBy4x3( pvNorm, pRc->tl.lpxfmToEyeInvT, &pTlkTmp->cn );

			if (dwTLState & TLPV_NORMALIZENORMALS)
				Normalize( &pTlkTmp->cn );

	        if (dwTLState & TLPV_COLORVERTEXFLAGS)
	        {
				// These offsets are the position in the TLBN where we get the respective colors. 
				// (The color has already been copied when we transformed.)  If an offset is
				// zero, then we get the color from the material.  This seemed like the way to 
				// do it with the fewest branches.
			  	if (pRc->tl.lighting.dwK3dOffsetCvAmbientSrc)
					pRc->tl.lighting.dwAmbientSrc = *(DWORD*)((BYTE*)pDst + pRc->tl.lighting.dwK3dOffsetCvAmbientSrc);
				else
					pRc->tl.lighting.dwAmbientSrc = pRc->tl.lighting.dwMaterialAmbient;
				if (pRc->tl.lighting.dwK3dOffsetCvDiffuseSrc)
					pRc->tl.lighting.dwDiffuseSrc = *(DWORD*)((BYTE*)pDst + pRc->tl.lighting.dwK3dOffsetCvDiffuseSrc);
				else
					pRc->tl.lighting.dwDiffuseSrc = pRc->tl.lighting.dwMaterialDiffuse;
				if (pRc->tl.lighting.dwK3dOffsetCvSpecularSrc)
					pRc->tl.lighting.dwSpecularSrc = *(DWORD*)((BYTE*)pDst + pRc->tl.lighting.dwK3dOffsetCvSpecularSrc);
				else
					pRc->tl.lighting.dwSpecularSrc = pRc->tl.lighting.dwMaterialSpecular;
				if (pRc->tl.lighting.dwK3dOffsetCvEmissiveSrc)
					pRc->tl.lighting.dwEmissiveSrc = *(DWORD*)((BYTE*)pDst + pRc->tl.lighting.dwK3dOffsetCvEmissiveSrc);
				else
					pRc->tl.lighting.dwEmissiveSrc = pRc->tl.lighting.dwMaterialEmissive;

				if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
				{
					pTlkTmp->p8AmbientSrc_U12pt0.uw.b = (unsigned short)RGBA_GETBLUE(pRc->tl.lighting.dwAmbientSrc) << 4;
					pTlkTmp->p8AmbientSrc_U12pt0.uw.g = (unsigned short)RGBA_GETGREEN(pRc->tl.lighting.dwAmbientSrc) << 4;
					pTlkTmp->p8AmbientSrc_U12pt0.uw.r = (unsigned short)RGBA_GETRED(pRc->tl.lighting.dwAmbientSrc) << 4;
					pTlkTmp->p8AmbientSrc_U12pt0.uw.a = 0;
					pTlkTmp->p8DiffuseSrc_U12pt0.uw.b = (unsigned short)RGBA_GETBLUE(pRc->tl.lighting.dwDiffuseSrc) << 4;
					pTlkTmp->p8DiffuseSrc_U12pt0.uw.g = (unsigned short)RGBA_GETGREEN(pRc->tl.lighting.dwDiffuseSrc) << 4;
					pTlkTmp->p8DiffuseSrc_U12pt0.uw.r = (unsigned short)RGBA_GETRED(pRc->tl.lighting.dwDiffuseSrc) << 4;
					pTlkTmp->p8DiffuseSrc_U12pt0.uw.a = (unsigned short)RGBA_GETALPHA(pRc->tl.lighting.dwDiffuseSrc) << 4;
					pTlkTmp->p8SpecularSrc_U16pt0.uw.b = (unsigned short)RGBA_GETBLUE(pRc->tl.lighting.dwSpecularSrc) << 8;
					pTlkTmp->p8SpecularSrc_U16pt0.uw.g = (unsigned short)RGBA_GETGREEN(pRc->tl.lighting.dwSpecularSrc) << 8;
					pTlkTmp->p8SpecularSrc_U16pt0.uw.r = (unsigned short)RGBA_GETRED(pRc->tl.lighting.dwSpecularSrc) << 8;
					pTlkTmp->p8SpecularSrc_U16pt0.uw.a = (unsigned short)RGBA_GETALPHA(pRc->tl.lighting.dwSpecularSrc) << 8;
					pTlkTmp->p8EmissiveSrc_U8pt0.uw.b = (unsigned short)RGBA_GETBLUE(pRc->tl.lighting.dwEmissiveSrc) << 0;
					pTlkTmp->p8EmissiveSrc_U8pt0.uw.g = (unsigned short)RGBA_GETGREEN(pRc->tl.lighting.dwEmissiveSrc) << 0;
					pTlkTmp->p8EmissiveSrc_U8pt0.uw.r = (unsigned short)RGBA_GETRED(pRc->tl.lighting.dwEmissiveSrc) << 0;
					pTlkTmp->p8EmissiveSrc_U8pt0.uw.a = 0;
				}

				if (dwTLState & (TLPV_COLORVERTEXEMIS | TLPV_COLORVERTEXAMB | TLPV_COLORVERTEXDIFF))
				{
					DWORD dwAmbientSrc = pRc->tl.lighting.dwAmbientSrc;
					DWORD dwEmissiveSrc = pRc->tl.lighting.dwEmissiveSrc;

					Ldata->fDiffuse.r = Ldata->ambient_red   * (D3DVALUE)RGBA_GETRED(dwAmbientSrc)   + (D3DVALUE)RGBA_GETRED(dwEmissiveSrc);
					Ldata->fDiffuse.g = Ldata->ambient_green * (D3DVALUE)RGBA_GETGREEN(dwAmbientSrc) + (D3DVALUE)RGBA_GETGREEN(dwEmissiveSrc);
					Ldata->fDiffuse.b = Ldata->ambient_blue  * (D3DVALUE)RGBA_GETBLUE(dwAmbientSrc)  + (D3DVALUE)RGBA_GETBLUE(dwEmissiveSrc);

					if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
					{
						//  Ldata->ambient_xxx = Ldata->ambient_save/255 (so it's really 0-1), and p8Ambient = ambient_save*256 
   						Ldata->p8Diffuse.uw.b = (unsigned short)(((DWORD)pRc->tl.pTLK->p8Ambient_U12pt0.uw.b * (DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.b) >> 16)
   										+ pRc->tl.pTLK->p8EmissiveSrc_U8pt0.uw.b;
			   			Ldata->p8Diffuse.uw.g = (unsigned short)(((DWORD)pRc->tl.pTLK->p8Ambient_U12pt0.uw.g * (DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.g) >> 16)
   										+ pRc->tl.pTLK->p8EmissiveSrc_U8pt0.uw.g;
   						Ldata->p8Diffuse.uw.r = (unsigned short)(((DWORD)pRc->tl.pTLK->p8Ambient_U12pt0.uw.r * (DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.r) >> 16)
   										+ pRc->tl.pTLK->p8EmissiveSrc_U8pt0.uw.r;
			   			Ldata->p8Diffuse.uw.a = (unsigned short)((DWORD)pTlkTmp->p8DiffuseSrc_U12pt0.uw.a >> 4);
    					Ldata->p8Specular.qw = Ldata->p8AmbEmissPlusSpecAlpha.qw;
					}
				}
				else
				{
					// Initialize the diffuse and specular color
					Ldata->fDiffuse.r = (float) Ldata->p8AmbEmissPlusDiffAlpha.uw.r;
					Ldata->fDiffuse.g = (float) Ldata->p8AmbEmissPlusDiffAlpha.uw.g;
					Ldata->fDiffuse.b = (float) Ldata->p8AmbEmissPlusDiffAlpha.uw.b;
					Ldata->fSpecular.r = 0;
					Ldata->fSpecular.g = 0;
					Ldata->fSpecular.b = 0;

					if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
					{
    					Ldata->p8Diffuse.qw = Ldata->p8AmbEmissPlusDiffAlpha.qw;
					    Ldata->p8Specular.qw = Ldata->p8AmbEmissPlusSpecAlpha.qw;
					}
				}
			}
			else //not color vertex, init colors from material
			{
				// Initialize the diffuse and specular color
				Ldata->fDiffuse.r = (float) Ldata->p8AmbEmissPlusDiffAlpha.uw.r;
				Ldata->fDiffuse.g = (float) Ldata->p8AmbEmissPlusDiffAlpha.uw.g;
				Ldata->fDiffuse.b = (float) Ldata->p8AmbEmissPlusDiffAlpha.uw.b;
				Ldata->fSpecular.r = 0;
				Ldata->fSpecular.g = 0;
				Ldata->fSpecular.b = 0;

				if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
				{
   					Ldata->p8Diffuse.qw = Ldata->p8AmbEmissPlusDiffAlpha.qw;
				    Ldata->p8Specular.qw = Ldata->p8AmbEmissPlusSpecAlpha.qw;
				}
			}

			_asm {
	    		mov         eax,Ldata
				movq		mm6, [eax]TLLIGHTING.p8Diffuse
				movq		mm7, [eax]TLLIGHTING.p8Specular
			}
			LightVertexK3D_C( pRc );
			_asm {
	    		mov         eax,Ldata
				packuswb	mm6, mm6
				packuswb	mm7, mm7
				movd		[eax]TLLIGHTING.dwDiffuse, mm6
				movd		[eax]TLLIGHTING.dwSpecular, mm7
				femms
			}
		}
	}


	/*
	* Apply the colors and compute the fog
	*/
	if (dwTLState & TLPV_DOLIGHTING)
	{
		// copy diffuse from computed LIGHT, munge in alpha from material
		diff = Ldata->dwDiffuse;
		if (pRc->specialModes)
			CLAMP888( diff, diff, diff );
		pDst->diffuse = diff;

		if (! (RfBits & BIT_RC_VERTEX_FOG))
		{
			fFog = 0;
			pDst->specular = Ldata->dwSpecular;
		}
		else
		{	// vertex fog, so the source is...
		    if (dwTLState & TLPV_DOFOG)
			{
				fFog = FogVertexK3D_C( pRc );
				pDst->specular = (((DWORD)(FTOI(fFog))) << 24) | Ldata->dwSpecular;
				fFog = 255.0f - fFog;
			}
			else	// fog from source vertex
			{
				fFog = Ldata->fMatSpecAlphaDev;			// precalculated 255-a
				pDst->specular = Ldata->materialSpecAlpha | Ldata->dwSpecular;
			}
		}
	}
	else //TLPV_DOLIGHTING
	{ 	// we didn't light the vert
		if (dwTLState & TLPV_DIFF_SRC_VTX)
		{
			if (pRc->specialModes)
			{
				diff = pDst->diffuse;	// copy from source vertex
				CLAMP888( diff, diff, diff );
				pDst->diffuse = diff;
			}
			else
			{
				//pDst->diffuse = pDst->diffuse; //it's already there
			}
		}
		else
		{
		 	pDst->diffuse = TL_DEFAULT_DIFFUSE;
		}


		if (! (RfBits & BIT_RC_VERTEX_FOG))
		{
			fFog = 0;
			if (dwTLState & TLPV_SPEC_SRC_VTX)
				pDst->specular = (pDst->diffuse & 0xff000000) | (pDst->specular & 0x00ffffff);
			else
				pDst->specular = (pDst->diffuse & 0xff000000) | (TL_DEFAULT_SPECULAR & 0x00ffffff);
		}
		else	// vertex fog, so the source is...
		{
		    if (! (dwTLState & TLPV_DOFOG))
			{
				if (dwTLState & TLPV_SPEC_SRC_VTX)
				{	// fog from the alpha in the source vertex's specular
					fFog = (float)(255 - (pDst->specular >> 24));	// copy from source vertex
					pDst->specular = (pDst->diffuse & 0xff000000) | (pDst->specular & 0x00ffffff);
				}
				else
				{
					fFog = (float)(255 - (TL_DEFAULT_SPECULAR >> 24));
					pDst->specular = (pDst->diffuse & 0xff000000) | (TL_DEFAULT_SPECULAR & 0x00ffffff);
				}
			}
			else
			{	// doing fog without T&L
				fFog = FogVertexK3D_C( pRc );
				fFog = 255.0f - fFog;
				if (dwTLState & TLPV_SPEC_SRC_VTX)
					pDst->specular = (pDst->diffuse & 0xff000000) | (pDst->specular & 0x00ffffff);
				else
					pDst->specular = (pDst->diffuse & 0xff000000) | (TL_DEFAULT_SPECULAR & 0x00ffffff);
			}
		}
	} //TLPV_DOLIGHTING


	/*
	* Z and WFBI
	*/
	w = pDst->w;	// W is needed for z, wfbi, and textures
	if (! (RfBits & BIT_RC_REQUIRES_WBUFFER))
	{
		//pDst->z = ((w * pDst->z * pRc->tl.ViewData.scaleZ) + pRc->tl.ViewData.offsetZ) * pRc->zScale;
		pDst->z = (w * pDst->z * pTlkTmp->viewDataScaleZ) + pTlkTmp->viewDataOffsetZ;
		if (RfBits & BIT_RC_VERTEX_FOG)
			pDst->wfbi = fFog;
		else
			pDst->wfbi = w;
	}
	else // WBUFFER
	{
		if (RfBits & BIT_RC_VERTEX_FOG)
			pDst->z = AS_FLOAT((AS_UINT32(fFog) + (8 << 23)));	// (255-fog)*256
		else
			//pDst->z = ((w * pDst->z * pRc->tl.ViewData.scaleZ) + pRc->tl.ViewData.offsetZ) * pRc->zScale;
			pDst->z = (w * pDst->z * pTlkTmp->viewDataScaleZ) + pTlkTmp->viewDataOffsetZ;
		pDst->wfbi = (w * pRc->aW) + pRc->bW;
	}


	/*
	* Textures
	*/
	if (RfBits & BIT_RC_REQUIRES_TX0)
	{
 		TLVECTOR4 v4;
		DWORD TCI = pRc->tl.dwTexCoordIndex[0];
		DWORD texXformFlags = pRc->tl.dwTexXformFlags[0];

    	switch( TCI & 0xffff0000 )
        {
        	case D3DTSS_TCI_CAMERASPACENORMAL:
			{ 
			    v4.x = pTlkTmp->cn.x;
			    v4.y = pTlkTmp->cn.y;
			    v4.z = pTlkTmp->cn.z;
		    	v4.w = 1.0f;
	            break;
			}
			case D3DTSS_TCI_CAMERASPACEPOSITION:
			{ 
		    	v4.x = pTlkTmp->cv.x;
			    v4.y = pTlkTmp->cv.y;
			    v4.z = pTlkTmp->cv.z;
			    v4.w = 1.0f;
            	break;
			}
			case D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR:
			{ 
			    if( dwTLState & TLPV_LOCALVIEWER )
				{ 
					D3DVECTOR v = pTlkTmp->cv;
					D3DVALUE fDot2;
					Normalize( &v );	// have to normalize before we reflect, result will be normalized
					fDot2 = 2.0f * (v.x*pTlkTmp->cn.x + v.y*pTlkTmp->cn.y + v.z*pTlkTmp->cn.z);
					v4.x = v.x - (pTlkTmp->cn.x * fDot2);
					v4.y = v.y - (pTlkTmp->cn.y * fDot2);
					v4.z = v.z - (pTlkTmp->cn.z * fDot2);
					v4.w = 1.0f;
				}
	            else
				{ 
					D3DVALUE fDot2 = 2.0f * pTlkTmp->cn.z;
					v4.x = 0.0f - (pTlkTmp->cn.x * fDot2);
					v4.y = 0.0f - (pTlkTmp->cn.y * fDot2);
					v4.z = 1.0f - (pTlkTmp->cn.z * fDot2);
					v4.w = 1.0f;
    	        }
				break;
			}
			case D3DTSS_TCI_PASSTHRU: // No TexGen
			{ 
            	//TLVECTOR4 *texIn = (TLVECTOR4*)((DWORD)pTextures + (pRc->t0CoordIndex << 2)); //assume all input textures are 2d
				TLVECTOR4 *texIn = (TLVECTOR4*)((DWORD)pTextures + TCI*sizeof(AOS_UV)); //assume all input textures are 2d
	 		    v4.x = texIn->x;    
 			    v4.y = texIn->y;	
 			    v4.z = 1.0f;		
 		    	v4.w = 0.0f;
				break;
			}
        }

        // Perform TexTransform
        if( (texXformFlags & (D3DTTFF_PROJECTED-1)) != D3DTTFF_DISABLE )
        { 
			LPD3DMATRIX pM = pRc->tl.lpTexXformMatrix[0];
			pDst->tex[0].s = v4.x*pM->_11 + v4.y*pM->_21 + v4.z*pM->_31 + v4.w*pM->_41;
			pDst->tex[0].t = v4.x*pM->_12 + v4.y*pM->_22 + v4.z*pM->_32 + v4.w*pM->_42;
		}
		else
		{
			pDst->tex[0].s = v4.x;
			pDst->tex[0].t = v4.y;
		}

		// This wasn't in the procprim.c in the refrast, but the sdk says for projected
		// you divide the elements passed in by the last element  (LEAVE IT OUT!)
		//if( pRc->textureStage[dwTexStage].texXformFlags & D3DTTFF_PROJECTED )
		//texOut->u /= z;
		//texOut->v /= z;

		// scale and offset
		pDst->tex[0].s = (pDst->tex[0].s * pTlkTmp->TxScale[0].u) + pTlkTmp->TxCenter[0].u;
		pDst->tex[0].t = (pDst->tex[0].t * pTlkTmp->TxScale[0].v) + pTlkTmp->TxCenter[0].v;

        if( pRc->tl.dwTLState & TLPV_DO_PROSPECTIVE_DIVIDE )
		{
			pDst->tex[0].s *= w;
			pDst->tex[0].t *= w;
		}
	}

	if (RfBits & BIT_RC_REQUIRES_TX1)
	{
 		TLVECTOR4 v4;
		DWORD TCI = pRc->tl.dwTexCoordIndex[1];
		DWORD texXformFlags = pRc->tl.dwTexXformFlags[1];

    	switch( TCI & 0xffff0000 )
        {
        	case D3DTSS_TCI_CAMERASPACENORMAL:
			{ 
			    v4.x = pTlkTmp->cn.x;
			    v4.y = pTlkTmp->cn.y;
			    v4.z = pTlkTmp->cn.z;
		    	v4.w = 1.0f;
	            break;
			}
			case D3DTSS_TCI_CAMERASPACEPOSITION:
			{ 
		    	v4.x = pTlkTmp->cv.x;
			    v4.y = pTlkTmp->cv.y;
			    v4.z = pTlkTmp->cv.z;
			    v4.w = 1.0f;
            	break;
			}
			case D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR:
			{ 
			    if( dwTLState & TLPV_LOCALVIEWER )
				{ 
					D3DVECTOR v = pTlkTmp->cv;
					D3DVALUE fDot2;
					Normalize( &v );	// have to normalize before we reflect, result will be normalized
					fDot2 = 2.0f * (v.x*pTlkTmp->cn.x + v.y*pTlkTmp->cn.y + v.z*pTlkTmp->cn.z);
					v4.x = v.x - (pTlkTmp->cn.x * fDot2);
					v4.y = v.y - (pTlkTmp->cn.y * fDot2);
					v4.z = v.z - (pTlkTmp->cn.z * fDot2);
					v4.w = 1.0f;
				}
	            else
				{ 
					D3DVALUE fDot2 = 2.0f * pTlkTmp->cn.z;
					v4.x = 0.0f - (pTlkTmp->cn.x * fDot2);
					v4.y = 0.0f - (pTlkTmp->cn.y * fDot2);
					v4.z = 1.0f - (pTlkTmp->cn.z * fDot2);
					v4.w = 1.0f;
    	        }
				break;
			}
			case D3DTSS_TCI_PASSTHRU: // No TexGen
			{ 
            	//TLVECTOR4 *texIn = (TLVECTOR4*)((DWORD)pTextures + (pRc->t1CoordIndex << 2)); //assume all input textures are 2d
				TLVECTOR4 *texIn = (TLVECTOR4*)((DWORD)pTextures + TCI*sizeof(AOS_UV)); //assume all input textures are 2d
	 		    v4.x = texIn->x;    
 			    v4.y = texIn->y;	
 			    v4.z = 1.0f;		
 		    	v4.w = 0.0f;
				break;
			}
        }

        // Perform TexTransform
        if( (texXformFlags & (D3DTTFF_PROJECTED-1)) != D3DTTFF_DISABLE )
        { 
			LPD3DMATRIX pM = pRc->tl.lpTexXformMatrix[1];
			pDst->tex[1].s = v4.x*pM->_11 + v4.y*pM->_21 + v4.z*pM->_31 + v4.w*pM->_41;
			pDst->tex[1].t = v4.x*pM->_12 + v4.y*pM->_22 + v4.z*pM->_32 + v4.w*pM->_42;
		}
		else
		{
			pDst->tex[1].s = v4.x;
			pDst->tex[1].t = v4.y;
		}

		// This wasn't in the procprim.c in the refrast, but the sdk says for projected
		// you divide the elements passed in by the last element  (LEAVE IT OUT!)
		//if( pRc->textureStage[dwTexStage].texXformFlags & D3DTTFF_PROJECTED )
		//texOut->u /= z;
		//texOut->v /= z;

		// scale and offset
		pDst->tex[1].s = (pDst->tex[1].s * pTlkTmp->TxScale[1].u) + pTlkTmp->TxCenter[1].u;
		pDst->tex[1].t = (pDst->tex[1].t * pTlkTmp->TxScale[1].v) + pTlkTmp->TxCenter[1].v;

        if( pRc->tl.dwTLState & TLPV_DO_PROSPECTIVE_DIVIDE )
		{
			pDst->tex[1].s *= w;
			pDst->tex[1].t *= w;
		}
	}

}
#endif //0

/*********************************************************
* 3DNow! assembly language version of FP_XformLightK3d()
*********************************************************/
void FP_XformLightK3d_Asm( RC *pRc, DWORD idx, TLBN* pDst )
{
	_asm {
		mov			ecx, [pRc]
		mov			esi, [idx]
		imul		esi, [ecx]RC.tl.InFVF.dwStride		// idx * pRc->tl.InFVF.dwStride
		mov			eax, [ecx]RC.tl.dwTLState			// dwTLState
		mov			ebx, [ecx]RC.tl.pTLK				// TL_K3DTMP *pTlkTmp
		add			esi, [ecx]RC.tl.InFVF.lpvData		// pSrc = input vertex

	/*************************************************************************************
	*
	* Lighting
	*
	*************************************************************************************/
		test		eax, (TLPV_DOLIGHTING | TLPV_DOFOG | TLPV_DOTEXGEN)
		jz			LightingAndFogDone

		movd		mm3, [esi]D3DVECTOR.z				/*	0			z			*/
		movq		mm0, [esi]D3DVECTOR.x				/*	y			x			*/
		mov			edx, dword ptr [ecx]RC.tl.numVertexBlends
		punpckldq	mm3, [TL_one]						/*	1.0			z			*/

		cmp			edx, 0
		jnz			VertexBlends

		/*******************************************************
		* Transform the position into eye (camera) space
		*******************************************************/
		// XformBy4x3( (D3DVECTOR*)&pSrc->x, pRc->tl.lpxfmToEye[0], &pTlkTmp->cv );
		movq		mm2, mm0							/*	y			x			*/
		movq		mm5, mm3							/*	1.0			z			*/
		pfmul		mm0, [ebx]TL_K3DTMP.xfmToEyeT._11	/*	y*_21		x*_11		*/
		movq		mm1, mm2							/*	y			x			*/
		pfmul		mm3, [ebx]TL_K3DTMP.xfmToEyeT._13	/*	_41			z*_31		*/
		movq		mm4, mm5							/*	1.0			z			*/
		pfmul		mm2, [ebx]TL_K3DTMP.xfmToEyeT._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx]TL_K3DTMP.xfmToEyeT._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3							/*	z*_31+_41	x*_11+y*_21	*/
		pfmul		mm1, [ebx]TL_K3DTMP.xfmToEyeT._31	/*	y*_23		x*_13		*/
		pfacc		mm2, mm5							/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm4, [ebx]TL_K3DTMP.xfmToEyeT._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4							/*	z*_33+_43	x*_13+y*_23	*/
		prefetch	[esi+128]							/*	prefetch a src vert	*/
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		mov			edi, [ecx]RC.tl.InFVF.dwNormalOffset	// pvNorm
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		movq		[ebx]TL_K3DTMP.cv.x, mm0			/*	cv.y		cv.x		*/
		movd		[ebx]TL_K3DTMP.cv.z, mm1			/*	-			cv.z		*/

		test		eax, (TLPV_DOLIGHTING | TLPV_DOTEXGEN)
		jz			LightingAndFogDone

		/*******************************************************
		* Transform the normal, normzlize if needed, and light
		*******************************************************/
		//XformBy4x3( pvNorm, pRc->tl.lpxfmToEyeInvT[0], &pTlkTmp->cn );
		movd		mm3, [esi+edi]D3DVECTOR.z				/*	0			z			*/
		movq		mm0, [esi+edi]D3DVECTOR.x				/*	y			x			*/
		punpckldq	mm3, [TL_one]							/*	1.0			z			*/
		movq		mm2, mm0								/*	y			x			*/
		movq		mm5, mm3								/*	1.0			z			*/
		pfmul		mm0, [ebx]TL_K3DTMP.lpxfmToEyeInv._11	/*	y*_21		x*_11		*/
		movq		mm1, mm2								/*	y			x			*/
		pfmul		mm3, [ebx]TL_K3DTMP.lpxfmToEyeInv._13	/*	_41			z*_31		*/
		movq		mm4, mm5								/*	1.0			z			*/
		pfmul		mm2, [ebx]TL_K3DTMP.lpxfmToEyeInv._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx]TL_K3DTMP.lpxfmToEyeInv._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3								/*	z*_31+_41	x*_11+y*_21	*/
		pfmul		mm1, [ebx]TL_K3DTMP.lpxfmToEyeInv._31	/*	y*_23		x*_13		*/
		pfacc		mm2, mm5								/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm4, [ebx]TL_K3DTMP.lpxfmToEyeInv._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4								/*	z*_33+_43	x*_13+y*_23	*/
		test		eax, TLPV_NORMALIZENORMALS
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		jz			WriteNormals
		jmp			NormalizeNormals


	// Vertex Blends ---------------------------------------------
	VertexBlends:
		/*******************************************************
		* Blend the position into eye (camera) space
		*******************************************************/

		// Camera Blend #0		mm3=x|y, mm0=z|1
		shl			edx, 6								/* numVertexBlends *= sizeof(D3DMATRIX) = 64 */
		cmp			dword ptr [esi+FVF_OFFSET_BLEND0], 0 /* check first blend for early out	*/
		jnz			CameraBlend0
		// first blend is zero, initialize the vert and cumulBlend 
		pxor		mm7, mm7							/*	0			cz.x		*/
		pxor		mm6, mm6							/*	cv.y		cv.x		*/
		punpckldq	mm7, [TL_one]						/*	1.0			cz.x		*/
		jmp			CameraBlend0Done
	CameraBlend0:
		movq		mm2, mm0							/*	y			x			*/
		movq		mm5, mm3							/*	1.0			z			*/
		pfmul		mm0, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._11	/*	y*_21		x*_11		*/
		pfmul		mm3, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._13	/*	_41			z*_31		*/
		movq		mm1, mm2							/*	y			x			*/
		movq		mm4, mm5							/*	1.0			z			*/
		pfmul		mm2, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm2, mm5							/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm1, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._31	/*	y*_23		x*_13		*/
		pfmul		mm4, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4							/*	z*_33+_43	x*_13+y*_23	*/
		movd		mm7, [esi+FVF_OFFSET_BLEND0]		/*	0			blend[0]	*/
		punpckldq	mm7, mm7							/*	blend[0]	blend[0]	*/
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		movq		mm6, mm7
		movq		mm4, mm7
		cmp			dword ptr [esi+FVF_OFFSET_BLEND0], INT32_FLOAT_ONE	/* if blend0=1.0, cumulBlend=0 so early out	*/
		pfmul		mm6, mm0							/*	cv.y*blend	cv.x*blend	*/
		pfmul		mm7, mm1							/*	cv.z*blend	cv.z*blend	*/
		jz			CameraBlendsDone					/*	this vertex wasn't blended */
		movd		mm5, [TL_one]
		movd		mm3, [esi]D3DVECTOR.z				/*	0			z			*/
		movq		mm0, [esi]D3DVECTOR.x				/*	y			x			*/
		punpckldq	mm3, [TL_one]						/*	1.0			z			*/
		pfsub		mm5, mm4							/*	cumulBlend=1.0-blend[0]	*/
		punpckldq	mm7, mm5							/*	cumulBlend	cv.z		*/
	CameraBlend0Done:

		// Camera Blend #1 	mm3=x|y, mm0=z|1, mm6=cv.x|cv.y, mm7=cv.z|cumulBlend
		cmp			edx, (1 * (SIZE D3DMATRIX))			/* if(numVertexBlends == 1)	*/
		jle			CameraBlendLast
		cmp			dword ptr [esi+FVF_OFFSET_BLEND1], 0 /* check second blend for early out	*/
		jz			CameraBlend1Done
		movq		mm2, mm0							/*	y			x			*/
		movq		mm5, mm3							/*	1.0			z			*/
		pfmul		mm0, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._11	/*	y*_21		x*_11		*/
		pfmul		mm3, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._13	/*	_41			z*_31		*/
		movq		mm1, mm2							/*	y			x			*/
		movq		mm4, mm5							/*	1.0			z			*/
		pfmul		mm2, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm2, mm5							/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm1, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._31	/*	y*_23		x*_13		*/
		pfmul		mm4, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4							/*	z*_33+_43	x*_13+y*_23	*/
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		movd		mm4, [esi+FVF_OFFSET_BLEND1]		/*	0			blend[1]	*/
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		punpckldq	mm4, mm4							/*	blend[1]	blend[1]	*/
		pfmul		mm0, mm4							/*	cv.y*blend	cv.x*blend	*/
		pfmul		mm1, mm4							/*	cv.z*blend	cv.z*blend	*/
		movq		mm5, mm7							/*	cumulBlend	cv.z		*/
		punpckhdq	mm5, mm5							/*	cumulBlend	cumulBlend	*/
		movd		mm3, [esi]D3DVECTOR.z				/*	0			z			*/
		pfsub		mm5, mm4							/*	cumulBlend -= blend[1]	*/
		pfadd		mm6, mm0							/*	cv.y		cv.x		*/
		movq		mm0, [esi]D3DVECTOR.x				/*	y			x			*/
		punpckldq	mm3, [TL_one]						/*	1.0			z			*/
		pfadd		mm7, mm1							/*	-			cv.z		*/
		punpckldq	mm7, mm5							/*	cumulBlend	cv.z		*/
	CameraBlend1Done:
		// mm3=x|y, mm0=z|1, mm6=cv.x|cv.y, mm7=cv.z|cumulBlend

		// Camera Blend #2 	mm3=x|y, mm0=z|1, mm6=cv.x|cv.y, mm7=cv.z|cumulBlend
		cmp			edx, (2 * (SIZE D3DMATRIX))			/* if(numVertexBlends == 2)	*/
		jle			CameraBlendLast
		cmp			dword ptr [esi+FVF_OFFSET_BLEND2], 0 /* check third blend for early out	*/
		jz			CameraBlend2Done
		movq		mm2, mm0							/*	y			x			*/
		movq		mm5, mm3							/*	1.0			z			*/
		pfmul		mm0, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._11	/*	y*_21		x*_11		*/
		pfmul		mm3, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._13	/*	_41			z*_31		*/
		movq		mm1, mm2							/*	y			x			*/
		movq		mm4, mm5							/*	1.0			z			*/
		pfmul		mm2, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm2, mm5							/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm1, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._31	/*	y*_23		x*_13		*/
		pfmul		mm4, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmToEyeT._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4							/*	z*_33+_43	x*_13+y*_23	*/
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		movd		mm4, [esi+FVF_OFFSET_BLEND2]		/*	0			blend[1]	*/
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		punpckldq	mm4, mm4							/*	blend[1]	blend[1]	*/
		movq		mm5, mm7							/*	cumulBlend	cv.z		*/
		pfmul		mm0, mm4							/*	cv.y*blend	cv.x*blend	*/
		pfmul		mm1, mm4							/*	cv.z*blend	cv.z*blend	*/
		punpckhdq	mm5, mm5							/*	cumulBlend	cumulBlend	*/
		movd		mm3, [esi]D3DVECTOR.z				/*	0			z			*/
		pfsub		mm5, mm4							/*	cumulBlend -= blend[2]	*/
		pfadd		mm6, mm0							/*	cv.y		cv.x		*/
		movq		mm0, [esi]D3DVECTOR.x				/*	y			x			*/
		punpckldq	mm3, [TL_one]						/*	1.0			z			*/
		pfadd		mm7, mm1							/*	-			cv.z		*/
		punpckldq	mm7, mm5							/*	cumulBlend	cv.z		*/
	CameraBlend2Done:
		// mm3=x|y, mm0=z|1, mm6=cv.x|cv.y, mm7=cv.z|cumulBlend

	CameraBlendLast:
		// Camera Blend #3 (or last) 	mm3=x|y, mm0=z|1, mm6=cv.x|cv.y, mm7=cv.z|cumulBlend
		movq		mm2, mm0							/*	y			x			*/
		movq		mm5, mm3							/*	1.0			z			*/
		pfmul		mm0, [ebx+edx]TL_K3DTMP.xfmToEyeT._11	/*	y*_21		x*_11		*/
		pfmul		mm3, [ebx+edx]TL_K3DTMP.xfmToEyeT._13	/*	_41			z*_31		*/
		movq		mm1, mm2							/*	y			x			*/
		movq		mm4, mm5							/*	1.0			z			*/
		pfmul		mm2, [ebx+edx]TL_K3DTMP.xfmToEyeT._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx+edx]TL_K3DTMP.xfmToEyeT._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm2, mm5							/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm1, [ebx+edx]TL_K3DTMP.xfmToEyeT._31	/*	y*_23		x*_13		*/
		pfmul		mm4, [ebx+edx]TL_K3DTMP.xfmToEyeT._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4							/*	z*_33+_43	x*_13+y*_23	*/
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		movq		mm4, mm7							/*	cumulBlend	cv.z		*/
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		punpckhdq	mm4, mm4							/*	cumulBlend	cumulBlend	*/
		pfmul		mm0, mm4							/*	cv.y*blend	cv.x*blend	*/
		pfmul		mm1, mm4							/*	cv.z*blend	cv.z*blend	*/
		pfadd		mm6, mm0							/*	cv.y		cv.x		*/
		pfadd		mm7, mm1							/*	-			cz.x		*/
		// mmm6=cv.x|cv.y, mm7=cv.z|-

	CameraBlendsDone:
		mov			edi, [ecx]RC.tl.InFVF.dwNormalOffset	// pvNorm
		movq		[ebx]TL_K3DTMP.cv.x, mm6			/*	cv.y		cv.x		*/
		movd		[ebx]TL_K3DTMP.cv.z, mm7			/*	-			cv.z		*/


		test		eax, (TLPV_DOLIGHTING | TLPV_DOTEXGEN)
		jz			LightingAndFogDone

		/*******************************************************
		* Blend the normal
		*******************************************************/
		movd		mm3, [esi+edi]D3DVECTOR.z			/*	0			nz			*/
		movq		mm0, [esi+edi]D3DVECTOR.x			/*	ny			nx			*/
		punpckldq	mm3, [TL_one]						/*	1.0			nz			*/

		// Camera Blend #0		mm3=nx|ny, mm0=nz|1
		cmp			dword ptr [esi+FVF_OFFSET_BLEND0], 0 /* check first blend for early out	*/
		movd		[ebx]TL_K3DTMP.cn.x, mm4			/*	cumulBlend (don't recalculate */
		jnz			NormalBlend0
		// first blend is zero, initialize the vert and cumulBlend 
		pxor		mm6, mm6							/*	cn.y		cn.x		*/
		pxor		mm7, mm7							/*	0			cn.z		*/
		jmp			NormalBlend0Done
	NormalBlend0:
		movq		mm2, mm0							/*	y			x			*/
		movq		mm5, mm3							/*	1.0			z			*/
		pfmul		mm0, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._11	/*	y*_21		x*_11		*/
		pfmul		mm3, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._13	/*	_41			z*_31		*/
		movq		mm1, mm2							/*	y			x			*/
		movq		mm4, mm5							/*	1.0			z			*/
		pfmul		mm2, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm2, mm5							/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm1, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._31	/*	y*_23		x*_13		*/
		pfmul		mm4, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4							/*	z*_33+_43	x*_13+y*_23	*/
		movd		mm6, [esi+FVF_OFFSET_BLEND0]		/*	0			blend[0]	*/
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		cmp			dword ptr [esi+FVF_OFFSET_BLEND0], INT32_FLOAT_ONE	/* if blend0=1.0, cumulBlend=0 so early out	*/
		punpckldq	mm6, mm6							/*	blend[0]	blend[0]	*/
		movq		mm7, mm6
		movq		mm4, mm6
		pfmul		mm6, mm0							/*	cn.y*blend	cn.x*blend	*/
		pfmul		mm7, mm1							/*	cn.z*blend	cn.z*blend	*/
		jz			NormalBlendsDone					/*	this vertex wasn't blended */
		movd		mm3, [esi+edi]D3DVECTOR.z			/*	0			nz			*/
		movq		mm0, [esi+edi]D3DVECTOR.x			/*	ny			nx			*/
		punpckldq	mm3, [TL_one]						/*	1.0			nz			*/
	NormalBlend0Done:

		// Normal Blend #1 	mm3=nx|ny, mm0=nz|1, mm6=cn.x|cn.y, mm7=cn.z|-
		cmp			edx, (1 * (SIZE D3DMATRIX))			/* if(numVertexBlends == 1)	*/
		jle			NormalBlendLast
		cmp			dword ptr [esi+FVF_OFFSET_BLEND1], 0 /* check second blend for early out	*/
		jz			NormalBlend1Done
		movq		mm2, mm0							/*	y			x			*/
		movq		mm5, mm3							/*	1.0			z			*/
		pfmul		mm0, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._11	/*	y*_21		x*_11		*/
		pfmul		mm3, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._13	/*	_41			z*_31		*/
		movq		mm1, mm2							/*	y			x			*/
		movq		mm4, mm5							/*	1.0			z			*/
		pfmul		mm2, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm2, mm5							/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm1, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._31	/*	y*_23		x*_13		*/
		pfmul		mm4, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4							/*	z*_33+_43	x*_13+y*_23	*/
		movd		mm5, [esi+FVF_OFFSET_BLEND1]		/*	0			blend[1]	*/
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		punpckldq	mm5, mm5							/*	blend[1]	blend[1]	*/
		pfmul		mm0, mm5							/*	cv.y*blend	cv.x*blend	*/
		pfmul		mm1, mm5							/*	cv.z*blend	cv.z*blend	*/
		movd		mm3, [esi+edi]D3DVECTOR.z			/*	0			nz			*/
		pfadd		mm6, mm0							/*	cn.y		cn.x		*/
		movq		mm0, [esi+edi]D3DVECTOR.x			/*	ny			nx			*/
		punpckldq	mm3, [TL_one]						/*	1.0			nz			*/
		pfadd		mm7, mm1							/*	-			cn.z		*/
	NormalBlend1Done:

		// Normal Blend #2 	mm3=nx|ny, mm0=nz|1, mm6=cn.x|cn.y, mm7=cn.z|cumulBlend
		cmp			edx, (2 * (SIZE D3DMATRIX))			/* if(numVertexBlends == 2)	*/
		jle			NormalBlendLast
		cmp			dword ptr [esi+FVF_OFFSET_BLEND2], 0 /* check third blend for early out	*/
		jz			NormalBlend2Done
		movq		mm2, mm0							/*	y			x			*/
		movq		mm5, mm3							/*	1.0			z			*/
		pfmul		mm0, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._11	/*	y*_21		x*_11		*/
		pfmul		mm3, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._13	/*	_41			z*_31		*/
		movq		mm1, mm2							/*	y			x			*/
		movq		mm4, mm5							/*	1.0			z			*/
		pfmul		mm2, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm2, mm5							/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm1, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._31	/*	y*_23		x*_13		*/
		pfmul		mm4, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.lpxfmToEyeInv._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4							/*	z*_33+_43	x*_13+y*_23	*/
		movd		mm5, [esi+FVF_OFFSET_BLEND2]		/*	0			blend[1]	*/
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		punpckldq	mm5, mm5							/*	blend[1]	blend[1]	*/
		pfmul		mm0, mm5							/*	cv.y*blend	cv.x*blend	*/
		pfmul		mm1, mm5							/*	cv.z*blend	cv.z*blend	*/
		movd		mm3, [esi+edi]D3DVECTOR.z			/*	0			nz			*/
		pfadd		mm6, mm0							/*	cn.y		cn.x		*/
		movq		mm0, [esi+edi]D3DVECTOR.x			/*	ny			nx			*/
		punpckldq	mm3, [TL_one]						/*	1.0			nz			*/
		pfadd		mm7, mm1							/*	-			cn.x		*/
	NormalBlend2Done:

	NormalBlendLast:
		// Normal Blend #3 (or last) 	mm3=nx|ny, mm0=nz|1, mm6=cn.x|cn.y, mm7=cn.z|cumulBlend
		movq		mm2, mm0							/*	y			x			*/
		movq		mm5, mm3							/*	1.0			z			*/
		pfmul		mm0, [ebx+edx]TL_K3DTMP.lpxfmToEyeInv._11	/*	y*_21		x*_11		*/
		pfmul		mm3, [ebx+edx]TL_K3DTMP.lpxfmToEyeInv._13	/*	_41			z*_31		*/
		movq		mm1, mm2							/*	y			x			*/
		movq		mm4, mm5							/*	1.0			z			*/
		pfmul		mm2, [ebx+edx]TL_K3DTMP.lpxfmToEyeInv._21	/*	y*_22		x*_12		*/
		pfmul		mm5, [ebx+edx]TL_K3DTMP.lpxfmToEyeInv._23	/*	_42			z*_32		*/
		pfacc		mm0, mm3							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm2, mm5							/*	z*_32+_42	x*_12+y*_22	*/
		pfmul		mm1, [ebx+edx]TL_K3DTMP.lpxfmToEyeInv._31	/*	y*_23		x*_13		*/
		pfmul		mm4, [ebx+edx]TL_K3DTMP.lpxfmToEyeInv._33	/*	_43			z*_33		*/
		pfacc		mm1, mm4							/*	z*_33+_43	x*_13+y*_23	*/
		movd		mm3, [ebx]TL_K3DTMP.cn.x			/*	0			cumulBlend	*/
		pfacc		mm0, mm2							/*	y=x*_12+y*_22+z*_32+_42  x=x*_11+y*_21+z*_31+_41 */
		pfacc		mm1, mm1							/*	z=x*_13+y*_23+z*_33+_43  z=x*_13+y*_23+z*_33+_43 */
		punpckldq	mm3, mm3							/*	cumulBlend	cumulBlend	*/
		pfmul		mm0, mm3							/*	cv.y*blend	cv.x*blend	*/
		pfmul		mm1, mm3							/*	cv.z*blend	cv.z*blend	*/
		pfadd		mm0, mm6							/*	cv.y		cv.x		*/
		pfadd		mm1, mm7							/*	-			cz.x		*/
	NormalBlendsDone:
		test		eax, TLPV_NORMALIZENORMALS
		jz			WriteNormals

	// Vertex Blends Done ---------------------------------------------

		// mm0=cn.x|cn.y, mm1=cn.z|-
	NormalizeNormals:
		movq		mm2, mm0					/*	y			x			*/
		movq		mm3, mm1					/*	z			z			*/
		pfmul		mm2, mm2					/*	y*y			x*x			*/
		pfmul		mm3, mm3					/*	z*z			z*z			*/
		prefetch	[esi+128]					/*	prefetch a src vert		*/
		pfacc		mm2, mm2					/*	x*x+y*y		x*x+y*y		*/
		pfadd		mm2, mm3					/*	mag = x*x+y*y+z*z		*/
		pfrsqrt		mm2, mm2					/*	1/sqrt(mag)	1/sqrt(mag)	*/
		pfmul		mm0, mm2					/*	y			x			*/
		pfmul		mm1, mm2					/*	z			z			*/
	WriteNormals:
		// do this a little later...

		test		eax, TLPV_COLORVERTEXFLAGS
		jnz			InitColorsFromInputVertex

		// load the initial colors from the material (no colorvertex)
		movq		mm6, [ecx]RC.tl.lighting.p8AmbEmissPlusDiffAlpha
		movq		mm7, [ecx]RC.tl.lighting.p8AmbEmissPlusSpecAlpha
		mov			eax, [ecx]RC.tl.lighting.pfnLightVertex
 		// now store the normals
		movq		[ebx]TL_K3DTMP.cn.x, mm0	/*	cn.y		cn.x		*/
		movd		[ebx]TL_K3DTMP.cn.z, mm1	/*	-			cn.z		*/
		jmp			InitColorsDone

	InitColorsFromInputVertex:
		mov			eax, [ecx]RC.tl.lighting.dwK3dOffsetCvAmbientSrc
		mov			edi, [pDst]					// colors are already in dest vertex
		mov			edx, [ecx]RC.tl.lighting.dwK3dOffsetCvDiffuseSrc
		movd		mm4, TL_maskLow24			// 00 00 00 00  00 ff ff ff
		pxor		mm2, mm2
		pxor		mm5, mm5
		// now store the normals
		movq		[ebx]TL_K3DTMP.cn.x, mm0	/*	cn.y		cn.x		*/
		movd		[ebx]TL_K3DTMP.cn.z, mm1	/*	-			cn.z		*/

		cmp			eax, 0
		jne			AmbientFromVertex
		movd		mm0, [ecx]RC.tl.lighting.dwMaterialAmbient
		jmp			AmbientLoaded
	AmbientFromVertex:
		movd		mm0, [edi+eax]
	AmbientLoaded:
		pand		mm0, mm4			// 00 00 00 00  00 ar ag ab		(mask off amb.alpha)
		punpcklbw	mm0, mm5			// 00 00 00 ar  00 ag 00 ab		zero extend from bytes to words
		psllw		mm0, 4				// convert ambient 12.0

		cmp			edx, 0
		mov			eax, [ecx]RC.tl.lighting.dwK3dOffsetCvSpecularSrc
		movd		mm7, TL_maskHigh8	// 00000000  ff000000		need this to mask off diff.alpha
		jne			DiffuseFromVertex
		movd		mm1, [ecx]RC.tl.lighting.dwMaterialDiffuse	// 00 00 00 00  da dr dg db
		jmp			DiffuseLoaded
	DiffuseFromVertex:
		movd		mm1, [edi+edx]		// 00 00 00 00  da dr dg db
	DiffuseLoaded:
		pand		mm7, mm1			// 00 00 00 00  da 00 00 00		we need diff.alpha for the base diff & spec
		pand		mm1, mm4			// 00 00 00 00  00 dr dg db		(mask off diff.alpha)
		psllq		mm7, 24				// 00 da 00 00  00 00 00 00		put diffuse.alpha in upper word as 8.0
		punpcklbw	mm1, mm5			// 00 00 00 dr  00 dg 00 db		zero extend from bytes to words
		psllw		mm1, 4				// convert diffuse 12.0

		mov			edx, [ecx]RC.tl.lighting.dwK3dOffsetCvEmissiveSrc
		cmp			eax, 0
		jne			SpecularFromVertex
		movd		mm3, [ecx]RC.tl.lighting.dwMaterialSpecular	// 00 00 00 00  sa sr sg sb
		jmp			SpecularLoaded
	SpecularFromVertex:
		movd		mm3, [edi+eax]		// 00 00 00 00  sa sr sg sb
	SpecularLoaded:
		pand		mm3, mm4			// 00 00 00 00  00 sr sg sb		mask off spec.alpha
		punpcklbw	mm2, mm3			// 00 00 sr 00  sg 00 sb 00		converts spec to 16.0

		cmp			edx, 0
		jne			EmissiveFromVertex
		movd		mm6, [ecx]RC.tl.lighting.dwMaterialEmissive
		jmp			EmissiveLoaded
	EmissiveFromVertex:
		movd		mm6, [edi+edx]
	EmissiveLoaded:
		pand		mm6, mm4			// 00 00 00 00  00 er eg eb		(mask off emmis.alpha)
		movq		mm3, [ebx]TL_K3DTMP.p8Ambient_U12pt0
		punpcklbw	mm6, mm5			// 00 00 00 er  00 eg 00 eb		zero extend from bytes to words
		mov			eax, [ecx]RC.tl.lighting.pfnLightVertex
		//psllw		mm6, 0				// emmisive stays at 8.0

		// store ambient, diffuse, and specular sources
		movq		[ebx]TL_K3DTMP.p8AmbientSrc_U12pt0, mm0
		movq		[ebx]TL_K3DTMP.p8DiffuseSrc_U12pt0, mm1
		movq		[ebx]TL_K3DTMP.p8SpecularSrc_U16pt0, mm2

		pmulhuw		mm3, mm0			// ambient * ambSrc
		por			mm6, mm7			// emissiveSrc + diff.alpha
		paddusw		mm6, mm3			// ambient * ambSrc + emissiveSrc + diff.alpha (now our base diffuse)
		//movq		mm7, mm7			// 00 da 00 00  00 00 00 00		base specular

	InitColorsDone:

		// Call the lighting functions
		mov			edi, ecx					// save pRc
		push		ecx							// pRc
		call		eax							// pRc->tl.lighting.pfnLightVertex(pRc)
		add			esp, 4
		mov			ecx, edi					// reload pRc
		packuswb	mm6, mm6					// xx xx xx xx da dr dg db
		mov			eax, [edi]RC.tl.dwTLState	// reload dwTLState
		packuswb	mm7, mm7					// xx xx xx xx sa sr sg sb
		// mm6 = diff, mm7 = spec

	LightingAndFogDone:


	/*************************************************************************************
	*
	* Apply the colors and compute the fog
	*
	*************************************************************************************/
		lea			ebx, [ecx]RC.tl.lighting			// TLLIGHTING *Ldata
		mov			edx, [ecx]RC.tl.KniRC.RfBits		// RfBits
		mov			edi, [pDst]

		test		eax, TLPV_DOLIGHTING
		jz			Colors_NoLighting

		/******************
		* Computed Light
		******************/
		// copy diffuse from computed light
		cmp			dword ptr [ecx]RC.specialModes, 0
		jz			Colors_Lighting_WriteDiffuse
		paddusb		mm6, mm6
	Colors_Lighting_WriteDiffuse:
		movd		[edi]TLBN.specular, mm7
		movd		[edi]TLBN.diffuse, mm6

		// get specular from computed light
		test		edx, BIT_RC_VERTEX_FOG
		jnz			Colors_Lighting_VFog
		jmp			Colors_Done

	Colors_Lighting_VFog:
		test		eax, TLPV_DOFOG
		jz			Colors_Lighting_SrcVertFog

		// vertex fog computed
		push		ecx									// pRc
		call		FogVertexK3D_Asm					// returns fog in mm0
		add			esp, 4

		movd		mm1, [TL_255]						// 255.0
		mov			ecx, [pRc]							// reload pRc
		pfsub		mm1, mm0							// 255.0 - fog
		mov			edx, [ecx]RC.tl.KniRC.RfBits		// reload RfBits
		mov			eax, [ecx]RC.tl.dwTLState			// dwTLState
		jmp			Colors_Done

	Colors_Lighting_SrcVertFog:
		movd		mm1, [ebx]TLLIGHTING.fMatSpecAlphaDev	// fog = precalculated 255-alpha
		movd		[edi]TLBN.specular, mm7
		jmp			Colors_Done

		/********************
		* Pre-Computed Light
		********************/
		// get colors from source vert (if available, else diffuse = ffffffff and spec = 0)
		// copy diff.alpha into spec.alpha
		// fog can still be calculated but will probably come from the source vert
	Colors_NoLighting:

		//No lighting, diffuse
		test		eax, TLPV_DIFF_SRC_VTX
		movd		mm3, TL_maskHigh8					// 0xff000000
		movd		mm1, [edi]TLBN.specular				// sa sr sg sb
		jz			Colors_NoLighting_NoSrcDiffuse
		cmp			dword ptr [ecx]RC.specialModes, 0
		jz			Colors_NoLighting_DiffuseDone				
		movd		mm2, [edi]TLBN.diffuse
		paddusb		mm2, mm2
		movd		[edi]TLBN.diffuse, mm2
		pand		mm3, mm2							// da 00 00 00 00
		jmp			Colors_NoLighting_DiffuseDone
	Colors_NoLighting_NoSrcDiffuse:
		mov			dword ptr [edi]TLBN.diffuse, TL_DEFAULT_DIFFUSE	// ffffffff
	Colors_NoLighting_DiffuseDone:

		//No lighting, specular
		test		edx, BIT_RC_VERTEX_FOG
		pslld		mm1, 8								// sr sg sb 00
		jnz			Colors_NoLighting_VertexFog
		test		eax, TLPV_SPEC_SRC_VTX
		jz			Colors_NoLighting_NoFog_WriteSpecular
		psrld		mm1, 8								// 00 sr sg sb
		por			mm3, mm1							// da sr sg sb
	Colors_NoLighting_NoFog_WriteSpecular:
		movd		[edi]TLBN.specular, mm3
		jmp			Colors_Done

	Colors_NoLighting_VertexFog:
		test		eax, TLPV_DOFOG
		jnz			Colors_NoLighting_VertexFog_DoFog
		test		eax, TLPV_SPEC_SRC_VTX
		jz			Colors_NoLighting_VertexFog_NoSrcSpec

		// vertex fog from the source vertex
		pcmpeqd		mm4, mm4							// ffffffff ffffffff
		movq		mm2, mm1							// sa sr sg sb
		pxor		mm1, mm4							// ^spec
		pand		mm2, TL_maskLow24					// 00 sr sg sb
		psrld		mm1, 24								// ^spec.alpha = ^spec >> 24 = 255-spec.alpha
		por			mm3, mm2							// da sr sg sb
		pf2id		mm1, mm1							// fog = (float)(255-spec.alpha)
		movd		[edi]TLBN.specular, mm3
		jmp			Colors_Done

	Colors_NoLighting_VertexFog_NoSrcSpec:
		movd		mm1, [TL_255]						// fog = 255.0 - 0.0
		movd		[edi]TLBN.specular, mm3
		jmp			Colors_Done

	Colors_NoLighting_VertexFog_DoFog:
		// vertex fog computed
		push		ecx									// pRc
		call		FogVertexK3D_Asm					// returns fog in mm0
		add			esp, 4
		mov			eax, (0xff000000 & TL_DEFAULT_SPECULAR)
		mov			ecx, [pRc]							// reload pRc

		movd		mm1, [TL_255]						// 255.0
		and			eax, [edi]TLBN.diffuse				// da 00 00 00
		test		[ecx]RC.tl.dwTLState, TLPV_SPEC_SRC_VTX // dwTLState
		mov			edx, [ecx]RC.tl.KniRC.RfBits		// reload RfBits (FogVertex hosed edx)
		pfsub		mm1, mm0							// 255.0 - fog
		jz			Colors_NoLighting_VertexFog_DoFog_WriteSpecular
		mov			ebx, 0x00ffffff
		and			ebx, [edi]TLBN.specular				// 00 sr sg sb
		or			eax, ebx							// da sr sg sb
	Colors_NoLighting_VertexFog_DoFog_WriteSpecular:
		mov			[edi]TLBN.specular, eax

	Colors_Done:
		prefetchw	[edi+128]							/*	prefetch a dst vert	*/


	/*************************************************************************************
	*
	* Z and WFBI
	*
	*	if (! (RfBits & BIT_RC_REQUIRES_WBUFFER))	{
	*		z = (w * z * viewDataScaleZ) + viewDataOffsetZ;
	*		if (RfBits & BIT_RC_VERTEX_FOG)			wfbi = fFog;
	*		else						   			wfbi = w;
	*	} else { // WBUFFER
	*		if (RfBits & BIT_RC_VERTEX_FOG)			z = (255-fog)*256
	*		else									z = (w * z * viewDataScaleZ) + viewDataOffsetZ;
	*		wfbi = (w * aW) + bW;
	*	}
	*************************************************************************************/
		// mm1 = fog							   	   
		test		edx, BIT_RC_REQUIRES_WBUFFER
		movd		mm7, [edi]TLBN.w				// rhw from destination vertex, needed for z, wfbi, and textures
		movd		mm6, [edi]TLBN.z				// z from destination vertex
		mov			ebx, [ecx]RC.tl.pTLK				// load ebx with TL_K3DTMP *pTlkTmp (overwrite Ldata)
		jne			WBuffer

		//NoWBuffer:
		pfmul		mm6, mm7							// rwh * z
		pfmul		mm6, [ebx]TL_K3DTMP.viewDataScaleZ	// rwh * z * (scaleZ * zScale)
		test		edx, BIT_RC_VERTEX_FOG
		pfadd		mm6, [ebx]TL_K3DTMP.viewDataOffsetZ	// (rwh * z * (scaleZ * zScale)) + (offsetZ * zScale)
		jne			NoWBuffer_VertexFog

		movd		[edi]TLBN.wfbi, mm7			// hw fog
		movd		[edi]TLBN.z, mm6
		jmp			ZandFog_Done

	NoWBuffer_VertexFog:
		movd		[edi]TLBN.wfbi, mm1
		movd		[edi]TLBN.z, mm6
		jmp			ZandFog_Done

	WBuffer:
		movd		mm3, [ecx]RC.aW
		pfmul		mm3, mm7							// rhw * aW
		test		edx, BIT_RC_VERTEX_FOG
		pfadd		mm3, [ecx]RC.bW						// (rhw * aW) + bW
		je			WBuffer_NoVertexFog

		paddd		mm1, [exp_mul256]					// z = fog * 256
		movd		[edi]TLBN.wfbi, mm3
		movd		[edi]TLBN.z, mm1
		jmp			ZandFog_Done

	WBuffer_NoVertexFog:
		pfmul		mm6, mm7							// rwh * z
		pfmul		mm6, [ebx]TL_K3DTMP.viewDataScaleZ	// rwh * z * (scaleZ * zScale)
		pfadd		mm6, [ebx]TL_K3DTMP.viewDataOffsetZ	// z = (rwh * z * (scaleZ * zScale)) + (offsetZ * zScale)
		movd		[edi]TLBN.wfbi, mm3
		movd		[edi]TLBN.z, mm6

	ZandFog_Done:


	/*************************************************************************************
	*
	* Textures
	*
	*************************************************************************************/
		// ebx=*TL_K3DTMP, ecx=*pRc, edx = KniRC.RfBits, esi=*pSrcVertex, edi=*pDst
		mov			eax, [ecx+0]RC.tl.dwTexCoordIndex	//TCI = pRc->tl.dwTexCoordIndex[0];
		test		edx, BIT_RC_REQUIRES_TX0
		jz			Textures_Done
		add			esi, [ecx]RC.tl.InFVF.dwTexOffset	// esi = pSrc->tex

    	// Load Texture 0
		test		eax, 0xffff0000						// D3DTSS_TCI_PASSTHRU
		jne			Tx0_Chk_CameralSpacePosition
		and			eax, (0x0000ffff & (D3DDP_MAXTEXCOORD-1))
		test		[ecx+0]RC.tl.dwTexXformFlags, (D3DTTFF_PROJECTED-1)
		movq		mm4, [ebx+0]TL_K3DTMP.TxScale		// scale.t			scale.s
		movq		mm0, [esi+eax*8]					// t		s		*pTex = ptextures + TCI*sizeof(AOS_UV)
		jz			Tx0_TransformDone					// special case (most common)
		movd		mm1, TL_one							// 0		1.0
		jmp			Tx0_Loaded
	Tx0_Chk_CameralSpacePosition:
		and			eax, 0xffff0000
		cmp			eax, D3DTSS_TCI_CAMERASPACENORMAL
		je			Tx0_CameralSpaceNormal
		movd		mm1, [ebx]TL_K3DTMP.cv.z			// 0				cv.z
		cmp			eax, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR
		movq		mm0, [ebx]TL_K3DTMP.cv.x			// cv.y				cv.x
		je			Tx0_CameralSpaceReflectionVector
		punpckldq	mm1, TL_one							// 1.0				cv.z
		movq		mm4, [ebx+0]TL_K3DTMP.TxScale		// scale.t			scale.s
		jmp			Tx0_DoTxXform
	Tx0_CameralSpaceReflectionVector:
		test		[ecx]RC.tl.dwTLState, TLPV_LOCALVIEWER
		je			Tx0_CameralSpaceReflectionVector_InfViewer
		pfmul		mm0, mm0							// cv.y*cv.y		cv.x*cv.x
		pfmul		mm1, mm1							// 0				cv.z*cv.z
		movd		mm3, [ebx]TL_K3DTMP.cn.z			// 0				cn.z
		movq		mm2, [ebx]TL_K3DTMP.cn.x			// cn.y				cn.x
		pfacc		mm0, mm0							// cv.x*cv.x + cv.y*cv.y
		movq		mm6, TL_soa_neg_2					// -2.0				-2.0
		pfadd		mm0, mm1							// mag = cv.x*cv.x + cv.y*cv.y + cv.z*cv.z
		pfmul		mm3, mm6							// 0				-2*cn.z
		movd		mm1, [ebx]TL_K3DTMP.cv.z			// 0				cv.z
		pfmul		mm2, mm6							// -2*cn.y			-2*cn.x
		pfrsqrt		mm0, mm0							// 1/sqrt(mag)		1/sqrt(mag)
		pfmul		mm1, mm0							// 0				n_cv.z
		pfmul		mm0, [ebx]TL_K3DTMP.cv.x			// n_cv.y			n_cv.x
		pfmul		mm3, mm1							// 0				-2*n_cv.z*cn.z
		pfmul		mm2, mm0							// -2*n_cv.y*cn.y	-2*n_cv.x*cn.x
		movq		mm4, [ebx]TL_K3DTMP.cn.x			// cn.y				cn.x
		movd		mm5, [ebx]TL_K3DTMP.cn.z			// 0				cn.z
		pfacc		mm2, mm2							// -2*n_cv.x*cn.x + -2*n_cv.y*cn.y
		punpckldq	mm3, mm3							// -2*n_cv.z*cn.z	-2*n_cv.z*cn.z
		pfadd		mm2, mm3							// -fDot2 = -2*(n_cv.x*cn.x + n_cv.y*cn.y + n_cv.z*cn.z)
		punpckldq	mm1, TL_one							// 1.0				n_cv.z
		pfmul		mm4, mm2							// -cn.y*fDot2		-cn.x*fDot2
		pfmul		mm5, mm2							// 0				-cn.z*fDot2
		pfadd		mm0, mm4							// n_cv.y-cn.y*fDot2 n_cv.x-cn.x*fDot2
		pfadd		mm1, mm5							// 1.0				n_cv.z-cn.z*fDot2
		movq		mm4, [ebx+0]TL_K3DTMP.TxScale		// scale.t			scale.s
		jmp			Tx0_Loaded
	Tx0_CameralSpaceReflectionVector_InfViewer:
		movd		mm2, TL_soa_neg_2					// 0				-2.0
		movq		mm3, TL_one							// 1.0				2.0
		pfmul		mm2, mm1							// 0				fdot2 = -2*cv.z
		pfmul		mm0, mm2							// -cn.y*fdot2		-cn.x*fdot2
		pfmul		mm1, mm2							// 0				-cn.z*fdot2
		movq		mm4, [ebx+0]TL_K3DTMP.TxScale		// scale.t			scale.s
		pfadd		mm1, mm3							// 1.0				1-cn.z*fdot2
		jmp			Tx0_Loaded
	Tx0_CameralSpaceNormal:
		movd		mm1, [ebx]TL_K3DTMP.cn.z			// 0				cn.z
		movq		mm0, [ebx]TL_K3DTMP.cn.x			// cn.y				cn.x
		punpckldq	mm1, TL_one							// 1.0				cn.z
		movq		mm4, [ebx+0]TL_K3DTMP.TxScale		// scale.t			scale.s
	Tx0_Loaded:

		// Perform TexTransform
		//if( (pRc->tl.dwTexXformFlags[0] & (D3DTTFF_PROJECTED-1)) != D3DTTFF_DISABLE )
		test		[ecx+0]RC.tl.dwTexXformFlags, (D3DTTFF_PROJECTED-1)
		jz			Tx0_TransformDone
	Tx0_DoTxXform:
		mov			eax, [ecx+0]RC.tl.lpTexXformMatrixT	// pRc->tl.lpxfmTxtrT[0]
		movq		mm2, mm0							// v.y				v.x
		movq		mm3, mm1							// v.w				v.z
		pfmul		mm0, [eax+0x00]						// v.y*21			v.x*11
		pfmul		mm1, [eax+0x08]						// v.w*41			v.z*31
		pfmul		mm2, [eax+0x10]						// v.y*22			v.x*12
		pfmul		mm3, [eax+0x18]						// v.w*42			v.z*32
		pfadd		mm0, mm1							// v.y*21+v.w*41	v.x*11+v.z*31
		pfadd		mm2, mm3							// v.y*22+v.w*42	v.x*12+v.z*32
		pfacc		mm0, mm2							// t=v.x*_12+v.y*_22+v.z*_32+v.w*_42	s=v.x*_11+v.y*_21+v.z*_31+v.w*_41
	Tx0_TransformDone:

		pfmul		mm0, mm4							// t*scale			s*scale
		test		[ecx]RC.tl.dwTLState, TLPV_DO_PROSPECTIVE_DIVIDE
		punpckldq	mm7, mm7							// rhw				rhw
		pfadd		mm0, [ebx+0]TL_K3DTMP.TxCenter		// (t/s*Scale)+Center
		jz			Tx0_PerspectiveCorrectDone
		pfmul		mm0, mm7							// (((t/s*Scale)+Center)*rhw
	Tx0_PerspectiveCorrectDone:
		mov			eax, [ecx+4]RC.tl.dwTexCoordIndex	//TCI = pRc->tl.dwTexCoordIndex[1];
		test		edx, BIT_RC_REQUIRES_TX1
		movq		[edi+0]TLBN.tex, mm0				// store the texture
		jz			Textures_Done

    	// Load Texture 1
		test		eax, 0xffff0000						// D3DTSS_TCI_PASSTHRU
		jne			Tx1_Chk_CameralSpacePosition
		and			eax, (0x0000ffff & (D3DDP_MAXTEXCOORD-1))
		test		[ecx+4]RC.tl.dwTexXformFlags, (D3DTTFF_PROJECTED-1)
		movq		mm4, [ebx+8]TL_K3DTMP.TxScale		// scale.t			scale.s
		movq		mm0, [esi+eax*8]					// t		s		*pTex = ptextures + TCI*sizeof(AOS_UV)
		jz			Tx1_TransformDone					// special case (most common)
		movd		mm1, TL_one							// 0		1.0
		jmp			Tx1_DoTxXform
	Tx1_Chk_CameralSpacePosition:
		and			eax, 0xffff0000
		cmp			eax, D3DTSS_TCI_CAMERASPACENORMAL
		je			Tx1_CameralSpaceNormal
		movd		mm1, [ebx]TL_K3DTMP.cv.z			// 0		cv.z
		cmp			eax, D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR
		movq		mm0, [ebx]TL_K3DTMP.cv.x			// cv.y		cv.x
		je			Tx1_CameralSpaceReflectionVector
		punpckldq	mm1, TL_one							// 1.0		cv.z
		movq		mm4, [ebx+8]TL_K3DTMP.TxScale		// scale.t			scale.s
		jmp			Tx1_Loaded
	Tx1_CameralSpaceReflectionVector:
		test		[ecx]RC.tl.dwTLState, TLPV_LOCALVIEWER
		je			Tx1_CameralSpaceReflectionVector_InfViewer
		pfmul		mm0, mm0							// cv.y*cv.y		cv.x*cv.x
		pfmul		mm1, mm1							// 0				cv.z*cv.z
		movd		mm3, [ebx]TL_K3DTMP.cn.z			// 0				cn.z
		movq		mm2, [ebx]TL_K3DTMP.cn.x			// cn.y				cn.x
		pfacc		mm0, mm0							// cv.x*cv.x + cv.y*cv.y
		movq		mm6, TL_soa_neg_2					// -2.0				-2.0
		pfadd		mm0, mm1							// mag = cv.x*cv.x + cv.y*cv.y + cv.z*cv.z
		pfmul		mm3, mm6							// 0				-2*cn.z
		movd		mm1, [ebx]TL_K3DTMP.cv.z			// 0				cv.z
		pfmul		mm2, mm6							// -2*cn.y			-2*cn.x
		pfrsqrt		mm0, mm0							// 1/sqrt(mag)		1/sqrt(mag)
		pfmul		mm1, mm0							// 0				n_cv.z
		pfmul		mm0, [ebx]TL_K3DTMP.cv.x			// n_cv.y			n_cv.x
		pfmul		mm3, mm1							// 0				-2*n_cv.z*cn.z
		pfmul		mm2, mm0							// -2*n_cv.y*cn.y	-2*n_cv.x*cn.x
		movq		mm4, [ebx]TL_K3DTMP.cn.x			// cn.y				cn.x
		movd		mm5, [ebx]TL_K3DTMP.cn.z			// 0				cn.z
		pfacc		mm2, mm2							// -2*n_cv.x*cn.x + -2*n_cv.y*cn.y
		punpckldq	mm3, mm3							// -2*n_cv.z*cn.z	-2*n_cv.z*cn.z
		pfadd		mm2, mm3							// -fDot2 = -2*(n_cv.x*cn.x + n_cv.y*cn.y + n_cv.z*cn.z)
		punpckldq	mm1, TL_one							// 1.0				n_cv.z
		pfmul		mm4, mm2							// -cn.y*fDot2		-cn.x*fDot2
		pfmul		mm5, mm2							// 0				-cn.z*fDot2
		pfadd		mm0, mm4							// n_cv.y-cn.y*fDot2 n_cv.x-cn.x*fDot2
		pfadd		mm1, mm5							// 1.0				n_cv.z-cn.z*fDot2
		movq		mm4, [ebx+8]TL_K3DTMP.TxScale		// scale.t			scale.s
		jmp			Tx1_Loaded
	Tx1_CameralSpaceReflectionVector_InfViewer:
		movd		mm2, TL_soa_neg_2					// 0				-2.0
		movq		mm3, TL_one							// 1.0				2.0
		pfmul		mm2, mm1							// 0				fdot2 = -2*cv.z
		pfmul		mm0, mm2							// -cn.y*fdot2		-cn.x*fdot2
		pfmul		mm1, mm2							// 0				-cn.z*fdot2
		movq		mm4, [ebx+8]TL_K3DTMP.TxScale		// scale.t			scale.s
		pfadd		mm1, mm3							// 1.0				1-cn.z*fdot2
		jmp			Tx1_Loaded
	Tx1_CameralSpaceNormal:
		movd		mm1, [ebx]TL_K3DTMP.cn.z			// 0		cn.z
		movq		mm0, [ebx]TL_K3DTMP.cn.x			// cn.y		cn.x
		punpckldq	mm1, TL_one							// 1.0		cn.z
		movq		mm4, [ebx+8]TL_K3DTMP.TxScale		// scale.t			scale.s
	Tx1_Loaded:

		// Perform TexTransform
		//if( (pRc->tl.dwTexXformFlags[1] & (D3DTTFF_PROJECTED-1)) != D3DTTFF_DISABLE )
		test		[ecx+4]RC.tl.dwTexXformFlags, (D3DTTFF_PROJECTED-1)
		jz			Tx1_TransformDone
	Tx1_DoTxXform:
		mov			eax, [ecx+4]RC.tl.lpTexXformMatrixT	// pRc->tl.lpxfmTxtrT[1]
		movq		mm2, mm0							// v.y				v.x
		movq		mm3, mm1							// v.w				v.z
		pfmul		mm0, [eax+0x00]						// v.y*21			v.x*11
		pfmul		mm1, [eax+0x08]						// v.w*41			v.z*31
		pfmul		mm2, [eax+0x10]						// v.y*22			v.x*12
		pfmul		mm3, [eax+0x18]						// v.w*42			v.z*32
		pfadd		mm0, mm1							// v.y*21+v.w*41	v.x*11+v.z*31
		pfadd		mm2, mm3							// v.y*22+v.w*42	v.x*12+v.z*32
		pfacc		mm0, mm2							// t=v.x*_12+v.y*_22+v.z*_32+v.w*_42	s=v.x*_11+v.y*_21+v.z*_31+v.w*_41
	Tx1_TransformDone:

		pfmul		mm0, mm4							// t*scale			s*scale
		test		[ecx]RC.tl.dwTLState, TLPV_DO_PROSPECTIVE_DIVIDE
		pfadd		mm0, [ebx+8]TL_K3DTMP.TxCenter		// (t/s*Scale)+Center
		jz			Tx1_PerspectiveCorrectDone
		pfmul		mm0, mm7							// (((t/s*Scale)+Center)*rhw
	Tx1_PerspectiveCorrectDone:
		movq		[edi+8]TLBN.tex, mm0				// store the texture

	Textures_Done:

		femms
	}
}

    
#endif //VCPP
#endif //TnL_HAL
#endif //DX7
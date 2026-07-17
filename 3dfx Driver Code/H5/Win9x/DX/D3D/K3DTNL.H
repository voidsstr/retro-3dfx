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
** File name: k3dtnl.h
**
** Description: Transformation and Lighting Code for 3DNow verticies
**
*/

/**********************************************************************************************************
**************************************  Pseudo Code w/Vertex Blends  **************************************
***********************************************************************************************************

  if K3DXFORM_NUM_VERTEX_BLENDS == 0  no vertex blending, use previous function
  if K3DXFORM_NUM_VERTEX_BLENDS == 1  use 1 blend factor, blend between 2 matrices
  if K3DXFORM_NUM_VERTEX_BLENDS == 2  use 2 blend factor, blend between 3 matrices
  if K3DXFORM_NUM_VERTEX_BLENDS == 3  use 3 blend factor, blend between 4 matrices

  float cumulBlend = 1.0;
  int numVertexBlends = pRc->tl.numVertexBlends;
  TLVERTEX4 hv;

  if(pBlendFactors[0] != 0) {
    hv.x = (pSrc->x*pRc->tl.lpxfmCurrent[0]->_11 + pSrc->y*pRc->tl.lpxfmCurrent[0]->_21 + pSrc->z*pRc->tl.lpxfmCurrent[0]->_31 + pRc->tl.lpxfmCurrent[0]->_41);
    hv.y = (pSrc->x*pRc->tl.lpxfmCurrent[0]->_12 + pSrc->y*pRc->tl.lpxfmCurrent[0]->_22 + pSrc->z*pRc->tl.lpxfmCurrent[0]->_32 + pRc->tl.lpxfmCurrent[0]->_42);
    hv.z = (pSrc->x*pRc->tl.lpxfmCurrent[0]->_13 + pSrc->y*pRc->tl.lpxfmCurrent[0]->_23 + pSrc->z*pRc->tl.lpxfmCurrent[0]->_33 + pRc->tl.lpxfmCurrent[0]->_43);
    hv.w = (pSrc->x*pRc->tl.lpxfmCurrent[0]->_14 + pSrc->y*pRc->tl.lpxfmCurrent[0]->_24 + pSrc->z*pRc->tl.lpxfmCurrent[0]->_34 + pRc->tl.lpxfmCurrent[0]->_44);
	if(pBlendFactors[0] == 1.0f)
	  goto BlendsDone;  // we're done
    float blend = pBlendFactors[0];
	Clip.x *= blend;    Clip.y *= blend;    Clip.z *= blend;    Clip.w *= blend;
	cumulBlend = 1.0f - blend;
  } else {
	cumulBlend = 1.0f;
	Clip.x = Clip.y = Clip.z = Clip.w = 0;
  }
    
  for( int j=1; j<=numVertexBlends; j++)  {
    if( j == numVertexBlends ) {
	  blend = cumulBlend;
	} else {
	  blend = pBlendFactors[j];
	  if(blend == 0)
	    continue;
	  cumulBlend -= blend;
	}
    hv.x += (pSrc->x*pRc->tl.lpxfmCurrent[j]->_11 + pSrc->y*pRc->tl.lpxfmCurrent[j]->_21 + pSrc->z*pRc->tl.lpxfmCurrent[j]->_31 + pRc->tl.lpxfmCurrent[j]->_41) * blend;
    hv.y += (pSrc->x*pRc->tl.lpxfmCurrent[j]->_12 + pSrc->y*pRc->tl.lpxfmCurrent[j]->_22 + pSrc->z*pRc->tl.lpxfmCurrent[j]->_32 + pRc->tl.lpxfmCurrent[j]->_42) * blend;
    hv.z += (pSrc->x*pRc->tl.lpxfmCurrent[j]->_13 + pSrc->y*pRc->tl.lpxfmCurrent[j]->_23 + pSrc->z*pRc->tl.lpxfmCurrent[j]->_33 + pRc->tl.lpxfmCurrent[j]->_43) * blend;
    hv.w += (pSrc->x*pRc->tl.lpxfmCurrent[j]->_14 + pSrc->y*pRc->tl.lpxfmCurrent[j]->_24 + pSrc->z*pRc->tl.lpxfmCurrent[j]->_34 + pRc->tl.lpxfmCurrent[j]->_44) * blend;
  }
  BlendsDone:

  if(Clipping)
    ComputeClipCodes()

  pout->z = hv.z;  // we'll scale this later
  pout->w = 1/hv.w;
  pout->x = (hv.x * pout->w * scaleX) + offsetX;
  pout->y = (hv.y * pout->w * scaleY) + offsetY;
***********************************************************************************************************
**********************************************************************************************************/


#if K3DXFORM_CLIPPED
#if (K3DXFORM_NUM_VERTEX_BLENDS == 0)

/****************************************************************************
********************* Clipped Version - No Vertex Blends ********************
****************************************************************************/
__declspec(align(16)) void K3DXFORM_NAME(RC *pRc, DWORD *pSrc, TLBN *pDst, DWORD numVerts)
{
	_asm {
		mov			esi, [pSrc]							// input vertex list
		mov			edi, [pDst]							// output vertex list
		mov			eax, [numVerts]
		mov			ecx, [pRc]							// will have to reload this after calling ComputeClipCodesK3D
		prefetch	[esi+0]
		prefetch	[esi+64]
		prefetchw	[edi+0]				  
		prefetchw	[edi+64]
		dec			eax
		js			Done
		movd		mm2, [TL_one]						/*	0			1.0			*/
		mov			ebx, [ecx]RC.tl.pTLK				// TL_K3DTMP *pTlkTmp

		// align NextVert to a 16-byte boundary (this sucks but it works!)
		jmp			NextVert
		mov			ecx, ecx	// 2-byte nop
		_asm _emit 0x8d _asm _emit 0x9c _asm _emit 0x23 _asm _emit 0x00 _asm _emit 0x00 _asm _emit 0x00 _asm _emit 0x00	// lea ebx,[ebx+00000000] 7-byte nop

	NextVert:
		movd		mm5, [esi]D3DVECTOR.z				/*	0			z			*/
		movq		mm1, [esi]D3DVECTOR.x				/*	y			x			*/
		mov			[numVerts], eax
		punpckldq	mm5, mm2							/*	1.0			z			*/
		movq		mm3, mm1							/*	y			x			*/
#if K3DXFORM_DIFFUSE_COLOR | K3DXFORM_SPECULAR_COLOR
		mov			eax, esi
#if K3DXFORM_DIFFUSE_COLOR
		add			eax, [ecx]RC.tl.InFVF.dwDiffuseOffset
#else
		add			eax, [ecx]RC.tl.InFVF.dwSpecularOffset
#endif
#endif
		movq		mm7, mm5							/*	1.0			z			*/
		pfmul		mm1, [ebx]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		movq		mm0, mm3							/*	y			x			*/
		pfmul		mm5, [ebx]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm4, mm7							/*	1.0			z			*/
		pfmul		mm3, [ebx]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		movq		mm2, mm0							/*	y			x			*/
		pfmul		mm7, [ebx]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm6, mm4							/*	1.0			z			*/
		pfmul		mm0, [ebx]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfacc		mm1, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm4, [ebx]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm3, mm7							/*	z*_34+_44	x*_14+y*_24	*/
		pfmul		mm2, [ebx]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfacc		mm0, mm4							/*	z*_31+_41	x*_11+y*_21	*/
		pfmul		mm6, [ebx]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
#if K3DXFORM_DIFFUSE_COLOR && K3DXFORM_SPECULAR_COLOR
		movq		mm7, [eax]							/* spec			diffuse		*/
#elif K3DXFORM_DIFFUSE_COLOR
		movd		mm7, [eax]							/* 0			diff		*/
#elif K3DXFORM_SPECULAR_COLOR
		movd		mm7, [eax]							/* 0			spec		*/
		punpckldq	mm7, mm7							/* spec			spec		*/
#endif
		prefetchnta	[esi+192]							/* +192 seems to be the sweet spot */
		pfacc		mm3, mm1	/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44 */
		prefetchw	[edi+256]							/* write prefetch */
		pfacc		mm2, mm6							/*	z*_32+_42	x*_12+y*_22	*/
		movq		mm1, [ecx]RC.tl.ViewData.scaleX		/* scaleY	scaleX	*/

		pfrcp		mm5, mm3					/* 1/w approx	1/w approx	*/
		movq		mm4, mm3					/*	hz			hw			*/
#if K3DXFORM_DIFFUSE_COLOR | K3DXFORM_SPECULAR_COLOR
		movq		[edi]TLBN.diffuse, mm7		/* spec			diffuse		*/
#endif
		punpckldq	mm3, mm3					/*	hw			hw			*/
		pfacc		mm0, mm2	/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		movq		mm6, mm3					/*	hw		hw		*/
		pfrcpit1	mm3, mm5					/* 		intermediate 		*/
		pxor		mm7, mm7					/*	0			0			*/
		mov			dword ptr [edi]TLBN.wfbi, TL_VERT_XFORMED_NOT_LIT
		pfrcpit2	mm3, mm5					/* 1/w			1/w			*/
		punpckhdq	mm4, mm4					/*	hz			hz			*/
		pfmul		mm1, mm0					/* y*scaleY		x*scaleX	*/
		pxor		mm5, mm5					/*	0			0			*/
		movd		[edi]TLBN.w, mm3			/*	rhw	*/
		pfmul		mm1, mm3					/* w*y*scaleY	w*x*scaleX	*/
		movq		mm3, mm4					/*	hz			hz			*/
		// mm0=hx,hy	mm1=sx,sy	mm4=hz,hz   mm6=hw,hw

		//ComputeClipCodesK3D_Asm
		/*  if (x <  0)	RRCLIP_LEFTBIT		if (x >= w)	RRCLIP_RIGHTBIT	*/
		/*  if (y <  0)	RRCLIP_BOTTOMBIT	if (y >= w)	RRCLIP_TOPBIT	*/
		/*  if (z <  0)	RRCLIP_FRONTBIT		if (z >= w)	RRCLIP_BACKBIT	*/
		pfcmpgt		mm5, mm4					/*	z < 0	z < 0	*/
		pfcmpge		mm3, mm6					/*	z >= w	z >= w	*/
		movq		mm2, mm0					/*	hy			yx			*/
		pfcmpgt		mm7, mm0					/*	y < 0	x < 0	*/
		movd		[edi]TLBN.z, mm4			/*	hz (not sz)	*/
		pfcmpge		mm2, mm6					/*	y >= w	x >= w	*/
		pfadd		mm1, [ecx]RC.tl.ViewData.viewDataOffsetX	/* sy=w*y*scaleY+offsetY	sx=w*x*scaleX+offsetX	*/
		punpckldq	mm5, mm3					/*	z >= w	z < 0	*/

		pand		mm7, [TLCLIPMASK_LEFT_BOTTOM]/*	BOTTOM	LEFT	*/
		pand		mm2, [TLCLIPMASK_RIGHT_TOP]	/*	TOP		RIGHT	*/
		pand		mm5, [TLCLIPMASK_FRONT_BACK]/*	BACK	FRONT	*/
		movq		[edi]TLBN.x, mm1			/*	sy		sx		*/
		por			mm7, mm2
		por			mm7, mm5
		movq		mm1, mm7
		punpckhdq	mm7, mm7
		por			mm7, mm1					/* clip_code */
		// mm0=hx,hy	mm4=hz,hz,  mm6=hw,hw	mm7=clip_code

#if K3DXFORM_USERCLIPPPLANES
		movd		mm5, [TL_userclip_mask]		/*	0		TLCLIP_USERCLIPPLANE0	*/
		punpckldq	mm4, mm6					/* hw		hz		*/
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
		pxor		mm1, mm1					/*	0		0		*/
		pfacc		mm2, mm2					/*	x*x+y*y+z*z+w*w	*/
		pcmpgtd		mm1, mm2					/*	x*x+y*y+z*z+w*w < 0.0f */
		pand		mm1, mm5					/*	x*x+y*y+z*z+w*w < 0.0f ? clipbit : 0	*/
		por			mm7, mm1					/*	clip_code |= clipbit	*/
		add			edx, SIZE TLUSERCLIPPLANE
		pslld		mm5, 1
		dec			eax
		jnz			CheckClipBit

	UserClipPlanesDone:
		// mm0=hx,hy	mm4=hz,hw,  mm6=hw,hw	mm7=clip_code
#endif	//K3DXFORM_USERCLIPPPLANES


		//if ((clip_code != 0) && (pRc->tl.dwTLState & TLPV_GUARDBAND))
		// Didn't special case this: it takes 7 clocks to get "clip_code" into eax so
		// in that time I think we can read TLState, test it, and branch
		movd		eax, mm7					/* eax = mm7 = clip_code	*/
		test		[ecx]RC.tl.dwTLState, TLPV_GUARDBAND
		jz			GuardbandDone
		cmp			eax, 0
		jz			GuardbandDone

		// We do guardband check in the projection space, so
		// we transform X and Y of the vertex there
		pfmul		mm0, [ebx]TL_K3DTMP.gb11	/*	y*gb12	x*gb11	*/
		movq		mm2, mm6					/*	w		w		*/
		pfmul		mm6, [ebx]TL_K3DTMP.gb41	/*	w*gb42	w*gb41	*/
		pxor		mm3, mm3					/*	0		0		*/
		pfadd		mm0, mm6					/*	 y*gb22+w*gb42   x*gb11*w*gb41  */
		pfcmpgt		mm3, mm0					/*	y*gb22+w*gb42<0  x*gb11*w*gb41<0 */
		pfcmpgt		mm0, mm2					/*	y*gb22+w*gb42>w  x*gb11*w*gb41>w */

		pand		mm3, [TLCLIPGBMASK_LEFT_BOTTOM]
		pand		mm0, [TLCLIPGBMASK_RIGHT_TOP]
		por			mm0, mm3
		por			mm7, mm0
		punpckhdq	mm0, mm0
		por			mm7, mm0

	GuardbandDone:
		mov			eax, [numVerts]
		movd		mm2, [TL_one]					/*	0			1.0			*/
		add			esi, [ecx]RC.tl.InFVF.dwStride
		movd		[edi]TLBN.clip_code, mm7


		// increment the pointers
		dec			eax
		lea			edi, [edi + SIZE TLBN]
		jns			NextVert

	Done:
		femms
	}
}

#else	//K3DXFORM_NUM_VERTEX_BLENDS != 0

/****************************************************************************
********************* Clipped Version with Vertex Blends ********************
****************************************************************************/
__declspec(align(16)) void K3DXFORM_NAME(RC *pRc, DWORD *pSrc, TLBN *pDst, DWORD numVerts)
{
	float cumulBlend;
	_asm {
		mov			esi, [pSrc]							// input vertex list
		mov			edi, [pDst]							// output vertex list
		mov			eax, [numVerts]
		mov			ecx, [pRc]							// will have to reload this after calling ComputeClipCodesK3D
		prefetch	[esi+0]
		prefetch	[esi+64]
		prefetchw	[edi+0]				  
		prefetchw	[edi+64]
		dec			eax
		js			Done
		mov			ebx, [ecx]RC.tl.pTLK				// TL_K3DTMP *pTlkTmp

	NextVert:
		mov			[numVerts], eax
		movd		mm6, [esi]D3DVECTOR.z				/*	0			z			*/
		movq		mm7, [esi]D3DVECTOR.x				/*	y			x			*/
		punpckldq	mm6, [TL_one]						/*	1.0			z			*/

		// BLEND #0		mm6=x|y, mm7=z|1
		cmp			dword ptr [esi+FVF_OFFSET_BLEND0], 0 /* check first blend for early out	*/
#if (K3DXFORM_NUM_VERTEX_BLENDS >= 2)
		mov			eax, [esi+FVF_OFFSET_BLEND1]
#endif
		jnz			Blend0
		// first blend is zero, initialize the vert and cumulBlend 
		pxor		mm0, mm0							/*	hy			hx			*/
		pxor		mm3, mm3							/*	hz			hw			*/
		movd		mm5, TL_soa_1						/* cumulBlend = 1.0			*/
		jmp			Blend0Done
	Blend0:
		movq		mm3, mm7							/*	y			x			*/
		movq		mm2, mm6							/*	1.0			z			*/
		pfmul		mm3, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		pfmul		mm2, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm4, mm7							/*	y			x			*/
		movq		mm5, mm6							/*	1.0			z			*/
		pfmul		mm4, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		pfmul		mm5, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm0, mm7							/*	y			x			*/
		movq		mm1, mm6							/*	1.0			z			*/
		pfmul		mm7, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfmul		mm6, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
		pfacc		mm3, mm2							/*	z*_34+_44	x*_14+y*_24	*/
		pfacc		mm4, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm0, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfmul		mm1, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm7, mm6							/*	z*_32+_42	x*_12+y*_22	*/
		cmp			dword ptr [esi+FVF_OFFSET_BLEND0], INT32_FLOAT_ONE	/* if blend0=1.0, cumulBlend=0 so early out	*/
		pfacc		mm0, mm1							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm3, mm4				/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44	*/
		movd		mm2, [esi+FVF_OFFSET_BLEND0]		/*	blend = blendFactor[0]		*/
		pfacc		mm0, mm7				/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		je			BlendsDone							/* blend0=1.0 so cumulBlend=0, early out */
		punpckldq	mm2, mm2
		movd		mm5, TL_soa_1						/*	0			1.0				*/
		pfmul		mm3, mm2							/*	hz*blend	hw*blend		*/
		pfmul		mm0, mm2							/*	hy*blend	hx*blend		*/
		movd		mm6, [esi]D3DVECTOR.z				/*	0			z				*/
		movq		mm7, [esi]D3DVECTOR.x				/*	y			x				*/
		pfsub		mm5, mm2							/* cumulBlend = 1-blendfactor[0] */
		punpckldq	mm6, [TL_one]						/*	1.0			z				*/
	Blend0Done:

		// BLEND #1		mm0=hx|hy, mm3=hw|hz, mm5=cumulBlend, mm6=x|y, mm7=z|1, eax=blend[1]
#if (K3DXFORM_NUM_VERTEX_BLENDS >= 2)
		cmp			eax, 0								/* check blend1 for early out	*/
  #if (K3DXFORM_NUM_VERTEX_BLENDS >= 3)
		mov			eax, [esi+FVF_OFFSET_BLEND2]
  #endif
		jz			Blend1Done
#endif
		movq		[ebx]TL_K3DTMP.hv.z, mm3			/* z and w are swapped!			*/
		movq		[ebx]TL_K3DTMP.hv.x, mm0
		movd		[cumulBlend], mm5
		movq		mm3, mm7							/*	y			x			*/
		movq		mm2, mm6							/*	1.0			z			*/
		pfmul		mm3, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		pfmul		mm2, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm4, mm7							/*	y			x			*/
		movq		mm5, mm6							/*	1.0			z			*/
		pfmul		mm4, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		pfmul		mm5, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm0, mm7							/*	y			x			*/
		movq		mm1, mm6							/*	1.0			z			*/
		pfmul		mm7, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfmul		mm6, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
		pfacc		mm3, mm2							/*	z*_34+_44	x*_14+y*_24	*/
		pfacc		mm4, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm0, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfmul		mm1, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm7, mm6							/*	z*_32+_42	x*_12+y*_22	*/
		pfacc		mm0, mm1							/*	z*_31+_41	x*_11+y*_21	*/
#if (K3DXFORM_NUM_VERTEX_BLENDS >= 2)
		movd		mm2, [esi+FVF_OFFSET_BLEND1]		/*	blend = blendFactor[1]		*/
		movd		mm5, [cumulBlend]					/*	0			cumulBlend		*/
#else
		movd		mm2, [cumulBlend]					/*	0			cumulBlend		*/
#endif
		pfacc		mm3, mm4				/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44	*/
		pfacc		mm0, mm7				/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		punpckldq	mm2, mm2
		pfmul		mm3, mm2							/*	hz*blend	hw*blend		*/
		pfmul		mm0, mm2							/*	hy*blend	hx*blend		*/
		pfadd		mm3, [ebx]TL_K3DTMP.hv.z
		pfadd		mm0, [ebx]TL_K3DTMP.hv.x
#if (K3DXFORM_NUM_VERTEX_BLENDS >= 2)
		movd		mm6, [esi]D3DVECTOR.z				/*	0			z				*/
		movq		mm7, [esi]D3DVECTOR.x				/*	y			x				*/
		pfsub		mm5, mm2							/* cumulBlend -= blendfactor[1]	*/
		punpckldq	mm6, [TL_one]						/*	1.0			z				*/
	Blend1Done:

		// BLEND #2		mm0=hx|hy, mm3=hw|hz, mm5=cumulBlend, mm6=x|y, mm7=z|1, eax=blend[2]
  #if (K3DXFORM_NUM_VERTEX_BLENDS >= 3)
		cmp			eax, 0								/* check blend2 for early out	*/
		jz			Blend2Done
  #endif
		movq		[ebx]TL_K3DTMP.hv.z, mm3
		movq		[ebx]TL_K3DTMP.hv.x, mm0
		movd		[cumulBlend], mm5
		movq		mm3, mm7							/*	y			x			*/
		movq		mm2, mm6							/*	1.0			z			*/
		pfmul		mm3, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		pfmul		mm2, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm4, mm7							/*	y			x			*/
		movq		mm5, mm6							/*	1.0			z			*/
		pfmul		mm4, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		pfmul		mm5, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm0, mm7							/*	y			x			*/
		movq		mm1, mm6							/*	1.0			z			*/
		pfmul		mm7, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfmul		mm6, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
		pfacc		mm3, mm2							/*	z*_34+_44	x*_14+y*_24	*/
		pfacc		mm4, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm0, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfmul		mm1, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm7, mm6							/*	z*_32+_42	x*_12+y*_22	*/
		pfacc		mm0, mm1							/*	z*_31+_41	x*_11+y*_21	*/
  #if (K3DXFORM_NUM_VERTEX_BLENDS >= 3)
		movd		mm5, [cumulBlend]
		movd		mm2, [esi+FVF_OFFSET_BLEND2]		/* cumulBlend = blendFactor[2]	*/
  #else
		movd		mm2, [cumulBlend]					/* 1.0 - (blendfactor[0] + blendfactor[1]) */
  #endif
		pfacc		mm3, mm4				/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44	*/
		pfacc		mm0, mm7				/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		punpckldq	mm2, mm2
		pfmul		mm3, mm2							/*	hz*blend	hw*blend		*/
		pfmul		mm0, mm2							/*	hy*blend	hx*blend		*/
		pfadd		mm3, [ebx]TL_K3DTMP.hv.z
		pfadd		mm0, [ebx]TL_K3DTMP.hv.x
  #if (K3DXFORM_NUM_VERTEX_BLENDS >= 3)
		movd		mm6, [esi]D3DVECTOR.z				/*	0			z				*/
		movq		mm7, [esi]D3DVECTOR.x				/*	y			x				*/
		pfsub		mm5, mm2							/* cumulBlend -= blendfactor[2]	*/
		punpckldq	mm6, [TL_one]						/*	1.0			z				*/
	Blend2Done:

		// BLEND #3		mm0=hx|hy, mm3=hw|hz, mm5=cumulBlend, mm6=x|y, mm7=z|1
		movq		[ebx]TL_K3DTMP.hv.z, mm3
		movq		[ebx]TL_K3DTMP.hv.x, mm0
		movd		[cumulBlend], mm5
		movq		mm3, mm7							/*	y			x			*/
		movq		mm2, mm6							/*	1.0			z			*/
		pfmul		mm3, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		pfmul		mm2, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm4, mm7							/*	y			x			*/
		movq		mm5, mm6							/*	1.0			z			*/
		pfmul		mm4, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		pfmul		mm5, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm0, mm7							/*	y			x			*/
		movq		mm1, mm6							/*	1.0			z			*/
		pfmul		mm7, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfmul		mm6, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
		pfacc		mm3, mm2							/*	z*_34+_44	x*_14+y*_24	*/
		pfacc		mm4, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm0, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfmul		mm1, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm7, mm6							/*	z*_32+_42	x*_12+y*_22	*/
		pfacc		mm0, mm1							/*	z*_31+_41	x*_11+y*_21	*/
		movd		mm2, [cumulBlend]					/* 1.0 - (blendfactor[0] + blendfactor[1] - blendfactor[2]) */
		pfacc		mm3, mm4				/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44	*/
		pfacc		mm0, mm7				/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		punpckldq	mm2, mm2
		pfmul		mm3, mm2							/*	hz*blend	hw*blend	*/
		pfmul		mm0, mm2							/*	hy*blend	hx*blend	*/
		pfadd		mm3, [ebx]TL_K3DTMP.hv.z
		pfadd		mm0, [ebx]TL_K3DTMP.hv.x
  #endif //(K3DXFORM_NUM_VERTEX_BLENDS >= 3)
#endif //(K3DXFORM_NUM_VERTEX_BLENDS >= 2)
	BlendsDone:
		// mm0=hx|hy, mm3=hw|hz


#if K3DXFORM_DIFFUSE_COLOR | K3DXFORM_SPECULAR_COLOR
		mov			eax, esi
  #if K3DXFORM_DIFFUSE_COLOR
		add			eax, [ecx]RC.tl.InFVF.dwDiffuseOffset
  #else
		add			eax, [ecx]RC.tl.InFVF.dwSpecularOffset
  #endif
#endif

		pfrcp		mm5, mm3							/* 1/w approx	1/w approx	*/
		movq		mm4, mm3							/*	hz			hw			*/
		punpckldq	mm3, mm3							/*	hw			hw			*/
		movq		mm6, mm3							/*	hw			hw			*/
		pfrcpit1	mm3, mm5							/* 		intermediate 		*/
		movq		mm1, [ecx]RC.tl.ViewData.scaleX		/* scaleY		scaleX		*/

#if K3DXFORM_DIFFUSE_COLOR && K3DXFORM_SPECULAR_COLOR
		movq		mm2, [eax]							/* spec			diffuse		*/
#elif K3DXFORM_DIFFUSE_COLOR
		movd		mm2, [eax]							/* 0			diff		*/
#elif K3DXFORM_SPECULAR_COLOR
		movd		mm2, [eax]							/* 0			spec		*/
		punpckldq	mm2, mm2							/* spec			spec		*/
#endif

		pxor		mm7, mm7							/*	0			0			*/
		pfrcpit2	mm3, mm5							/* 1/w			1/w			*/
		mov			dword ptr [edi]TLBN.wfbi, TL_VERT_XFORMED_NOT_LIT
		punpckhdq	mm4, mm4							/*	hz			hz			*/
		pfmul		mm1, mm0							/* y*scaleY		x*scaleX	*/
#if K3DXFORM_DIFFUSE_COLOR | K3DXFORM_SPECULAR_COLOR
		movq		[edi]TLBN.diffuse, mm2				/* spec			diffuse		*/
#endif
		pxor		mm5, mm5							/*	0			0			*/
		movd		[edi]TLBN.z, mm4					/*	hz (not sz)	*/
		pfmul		mm1, mm3							/* w*y*scaleY	w*x*scaleX	*/
		movd		[edi]TLBN.w, mm3					/*	rhw	*/
		movq		mm3, mm4							/*	hz			hz			*/
		// mm0=hx,hy	mm1=sx,sy	mm2=colors	mm4=hz,hz   mm6=hw,hw

		prefetchnta	[esi+192]							/* +192 seems to be the sweet spot */
		prefetchw	[edi+256]							/* write prefetch */

		//ComputeClipCodesK3D_Asm
		/*  if (x <  0)	RRCLIP_LEFTBIT		if (x >= w)	RRCLIP_RIGHTBIT	*/
		/*  if (y <  0)	RRCLIP_BOTTOMBIT	if (y >= w)	RRCLIP_TOPBIT	*/
		/*  if (z <  0)	RRCLIP_FRONTBIT		if (z >= w)	RRCLIP_BACKBIT	*/
		pfcmpgt		mm5, mm4					/*	z < 0	z < 0	*/
		pfcmpge		mm3, mm6					/*	z >= w	z >= w	*/
		movq		mm2, mm0					/*	hy			yx			*/
		pfcmpgt		mm7, mm0					/*	y < 0	x < 0	*/
		pfcmpge		mm2, mm6					/*	y >= w	x >= w	*/
		pfadd		mm1, [ecx]RC.tl.ViewData.viewDataOffsetX	/* sy=w*y*scaleY+offsetY	sx=w*x*scaleX+offsetX	*/
		punpckldq	mm5, mm3					/*	z >= w	z < 0	*/

		pand		mm7, [TLCLIPMASK_LEFT_BOTTOM]/*	BOTTOM	LEFT	*/
		pand		mm2, [TLCLIPMASK_RIGHT_TOP]	/*	TOP		RIGHT	*/
		pand		mm5, [TLCLIPMASK_FRONT_BACK]/*	BACK	FRONT	*/
		movq		[edi]TLBN.x, mm1			/*	sy		sx		*/
		por			mm7, mm2
		por			mm7, mm5
		movq		mm1, mm7
		punpckhdq	mm7, mm7
		por			mm7, mm1					/* clip_code */
		// mm0=hx,hy	mm4=hz,hz,  mm6=hw,hw	mm7=clip_code

#if K3DXFORM_USERCLIPPPLANES
		movd		mm5, [TL_userclip_mask]		/*	0		TLCLIP_USERCLIPPLANE0	*/
		punpckldq	mm4, mm6					/* hw		hz		*/
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
		pxor		mm1, mm1					/*	0		0		*/
		pfacc		mm2, mm2					/*	x*x+y*y+z*z+w*w	*/
		pcmpgtd		mm1, mm2					/*	x*x+y*y+z*z+w*w < 0.0f */
		pand		mm1, mm5					/*	x*x+y*y+z*z+w*w < 0.0f ? clipbit : 0	*/
		por			mm7, mm1					/*	clip_code |= clipbit	*/
		add			edx, SIZE TLUSERCLIPPLANE
		pslld		mm5, 1
		dec			eax
		jnz			CheckClipBit

	UserClipPlanesDone:
		// mm0=hx,hy	mm4=hz,hw,  mm6=hw,hw	mm7=clip_code
#endif	//K3DXFORM_USERCLIPPPLANES


		//if ((clip_code != 0) && (pRc->tl.dwTLState & TLPV_GUARDBAND))
		// Didn't special case this: it takes 7 clocks to get "clip_code" into eax so
		// in that time I think we can read TLState, test it, and branch
		movd		eax, mm7					/* eax = mm7 = clip_code	*/
		test		[ecx]RC.tl.dwTLState, TLPV_GUARDBAND
		jz			GuardbandDone
		cmp			eax, 0
		jz			GuardbandDone

		// We do guardband check in the projection space, so
		// we transform X and Y of the vertex there
		pfmul		mm0, [ebx]TL_K3DTMP.gb11	/*	y*gb12	x*gb11	*/
		movq		mm2, mm6					/*	w		w		*/
		pfmul		mm6, [ebx]TL_K3DTMP.gb41	/*	w*gb42	w*gb41	*/
		pxor		mm3, mm3					/*	0		0		*/
		pfadd		mm0, mm6					/*	y*gb22+w*gb42	x*gb11*w*gb41  */
		pfcmpgt		mm3, mm0					/*	y*gb22+w*gb42<0	x*gb11*w*gb41<0 */
		pfcmpgt		mm0, mm2					/*	y*gb22+w*gb42>w	x*gb11*w*gb41>w */

		pand		mm3, [TLCLIPGBMASK_LEFT_BOTTOM]
		pand		mm0, [TLCLIPGBMASK_RIGHT_TOP]
		por			mm0, mm3
		por			mm7, mm0
		punpckhdq	mm0, mm0
		por			mm7, mm0

	GuardbandDone:
		mov			eax, [numVerts]
		movd		mm2, [TL_one]					/*	0			1.0			*/
		add			esi, [ecx]RC.tl.InFVF.dwStride
		movd		[edi]TLBN.clip_code, mm7


		// increment the pointers
		dec			eax
		lea			edi, [edi + SIZE TLBN]
		jns			NextVert

	Done:
		femms
	}
}

#endif	//K3DXFORM_NUM_VERTEX_BLENDS

#else	// ------------------------------- !K3DXFORM_CLIPPED -------------------------------------

#if (K3DXFORM_NUM_VERTEX_BLENDS == 0)

/****************************************************************************
********************** Non-Clipped Version - No Vertex Blends ***************
****************************************************************************/
__declspec(align(16)) void K3DXFORM_NAME(RC *pRc, DWORD *pSrc, TLBN *pDst, DWORD numVerts)
{
	_asm {
		mov			esi, [pSrc]							// input vertex list
		mov			edi, [pDst]							// output vertex list
		mov			eax, [numVerts]
		mov			ecx, [pRc]							// will have to reload this after calling ComputeClipCodesK3D
		prefetch	[esi+0]
		prefetch	[esi+64]
		prefetchw	[edi+0]				  
		prefetchw	[edi+64]
		dec			eax
		js			Done
		movd		mm2, [TL_one]						/*	0			1.0			*/
		mov			ebx, [ecx]RC.tl.pTLK				// TL_K3DTMP *pTlkTmp

		// align NextVert to a 16-byte boundary (this sucks but it works!)
		jmp			NextVert
		mov			ecx, ecx	// 2-byte nop
		_asm _emit 0x8d _asm _emit 0x9c _asm _emit 0x23 _asm _emit 0x00 _asm _emit 0x00 _asm _emit 0x00 _asm _emit 0x00	// lea ebx,[ebx+00000000] 7-byte nop

	NextVert:
		// these comments for the matrix params are correct because it's transposed
		mov			[numVerts], eax				
		movd		mm5, [esi]D3DVECTOR.z				/*	0			z			*/
		movq		mm1, [esi]D3DVECTOR.x				/*	y			x			*/
		punpckldq	mm5, mm2							/*	1.0			z			*/
		movq		mm3, mm1							/*	y			x			*/
#if K3DXFORM_DIFFUSE_COLOR | K3DXFORM_SPECULAR_COLOR
		mov			eax, esi
#if K3DXFORM_DIFFUSE_COLOR
		add			eax, [ecx]RC.tl.InFVF.dwDiffuseOffset
#else
		add			eax, [ecx]RC.tl.InFVF.dwSpecularOffset
#endif
#endif
		movq		mm7, mm5							/*	1.0			z			*/
		pfmul		mm1, [ebx]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		movq		mm0, mm3							/*	y			x			*/
		pfmul		mm5, [ebx]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm4, mm7							/*	1.0			z			*/
		pfmul		mm3, [ebx]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		movq		mm2, mm0							/*	y			x			*/
		pfmul		mm7, [ebx]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm6, mm4							/*	1.0			z			*/
		pfmul		mm0, [ebx]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfacc		mm1, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm4, [ebx]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm3, mm7							/*	z*_34+_44	x*_14+y*_24	*/
		pfmul		mm2, [ebx]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfacc		mm0, mm4							/*	z*_31+_41	x*_11+y*_21	*/
		pfmul		mm6, [ebx]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
		pfacc		mm3, mm1	/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44	*/
		movq		mm7, [ecx]RC.tl.ViewData.scaleX		/* scaleY		scaleX		*/
		pfacc		mm2, mm6							/*	z*_32+_42	x*_12+y*_22	*/

		prefetchnta	[esi+192]							/* +192 seems to be the sweet spot */

#if K3DXFORM_DIFFUSE_COLOR && K3DXFORM_SPECULAR_COLOR
		movq		mm6, [eax]							/* spec			diffuse		*/
#elif K3DXFORM_DIFFUSE_COLOR
		movd		mm6, [eax]							/* 0			diff		*/
#elif K3DXFORM_SPECULAR_COLOR
		movd		mm6, [eax]							/* 0			spec		*/
		punpckldq	mm6, mm6							/* spec			spec		*/
#endif

		pfacc		mm0, mm2	/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		pfrcp		mm5, mm3						/* 1/w approx	1/w approx	*/
		movq		mm4, mm3						/* temporary copy (for z)	*/
		punpckldq	mm3, mm3						/*	hw			hw			*/
		add			esi, [ecx]RC.tl.InFVF.dwStride
		pfrcpit1	mm3, mm5						/* 		intermediate 		*/
		prefetchw	[edi+128]						/* +128 seems to be the sweet spot */
		punpckhdq	mm4, mm4						/*	hz			hz			*/
		pfmul		mm7, mm0						/* y*scaleY		x*scaleX	*/
		pfrcpit2	mm3, mm5						/* 1/w			1/w			*/

		movq		mm5, [ecx]RC.tl.ViewData.viewDataOffsetX	/* offsetY	offsetX	*/
		pfmul		mm7, mm3						/* w*y*scaleY	w*x*scaleX	*/

		movd		mm2, [TL_one]					/*	0			1.0			*/
		mov			eax, [numVerts]

#if K3DXFORM_DIFFUSE_COLOR | K3DXFORM_SPECULAR_COLOR
		movq		[edi]TLBN.diffuse, mm6			/* spec			diffuse		*/
#endif

		pfadd		mm7, mm5			/* y=w*y*scaleY+offsetY		x=w*x*scaleX+offsetX	*/

		mov			dword ptr [edi]TLBN.wfbi, TL_VERT_XFORMED_NOT_LIT

		movd		[edi]TLBN.z, mm4				/*	hz (not sz)	*/
		movd		[edi]TLBN.w, mm3				/*	rhw	*/
		movq		[edi]TLBN.x, mm7				/*	y			x			*/

		// increment the pointers
		dec			eax
		lea			edi, [edi + SIZE TLBN]
		jns			NextVert

	Done:
		femms
	}
}

#else	//K3DXFORM_NUM_VERTEX_BLENDS != 0

/****************************************************************************
******************* Non-clipped Version with Vertex Blends ******************
****************************************************************************/
__declspec(align(16)) void K3DXFORM_NAME(RC *pRc, DWORD *pSrc, TLBN *pDst, DWORD numVerts)
{
	float cumulBlend;
	_asm {
		mov			esi, [pSrc]							// input vertex list
		mov			edi, [pDst]							// output vertex list
		mov			eax, [numVerts]
		mov			ecx, [pRc]							// will have to reload this after calling ComputeClipCodesK3D
		prefetch	[esi+0]
		prefetch	[esi+64]
		prefetchw	[edi+0]				  
		prefetchw	[edi+64]
		dec			eax
		js			Done
		mov			ebx, [ecx]RC.tl.pTLK				// TL_K3DTMP *pTlkTmp

	NextVert:
		mov			[numVerts], eax
		movd		mm6, [esi]D3DVECTOR.z				/*	0			z			*/
		movq		mm7, [esi]D3DVECTOR.x				/*	y			x			*/
		punpckldq	mm6, [TL_one]						/*	1.0			z			*/

		// BLEND #0		mm6=x|y, mm7=z|1
		cmp			dword ptr [esi+FVF_OFFSET_BLEND0], 0 /* check first blend for early out	*/
#if (K3DXFORM_NUM_VERTEX_BLENDS >= 2)
		mov			eax, [esi+FVF_OFFSET_BLEND1]
#endif
		jnz			Blend0
		// first blend is zero, initialize the vert and cumulBlend 
		pxor		mm0, mm0							/*	hy			hx			*/
		pxor		mm3, mm3							/*	hz			hw			*/
		movd		mm5, TL_soa_1						/* cumulBlend = 1.0			*/
		jmp			Blend0Done
	Blend0:
		movq		mm3, mm7							/*	y			x			*/
		movq		mm2, mm6							/*	1.0			z			*/
		pfmul		mm3, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		pfmul		mm2, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm4, mm7							/*	y			x			*/
		movq		mm5, mm6							/*	1.0			z			*/
		pfmul		mm4, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		pfmul		mm5, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm0, mm7							/*	y			x			*/
		movq		mm1, mm6							/*	1.0			z			*/
		pfmul		mm7, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfmul		mm6, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
		pfacc		mm3, mm2							/*	z*_34+_44	x*_14+y*_24	*/
		pfacc		mm4, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm0, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfmul		mm1, [ebx+(0*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm7, mm6							/*	z*_32+_42	x*_12+y*_22	*/
		cmp			dword ptr [esi+FVF_OFFSET_BLEND0], INT32_FLOAT_ONE	/* if blend0=1.0, cumulBlend=0 so early out	*/
		pfacc		mm0, mm1							/*	z*_31+_41	x*_11+y*_21	*/
		pfacc		mm3, mm4				/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44	*/
		movd		mm2, [esi+FVF_OFFSET_BLEND0]		/*	blend = blendFactor[0]		*/
		pfacc		mm0, mm7				/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		je			BlendsDone							/* blend0=1.0 so cumulBlend=0, early out */
		punpckldq	mm2, mm2
		movd		mm5, TL_soa_1						/*	0			1.0				*/
		pfmul		mm3, mm2							/*	hz*blend	hw*blend		*/
		pfmul		mm0, mm2							/*	hy*blend	hx*blend		*/
		movd		mm6, [esi]D3DVECTOR.z				/*	0			z				*/
		movq		mm7, [esi]D3DVECTOR.x				/*	y			x				*/
		pfsub		mm5, mm2							/* cumulBlend = 1-blendfactor[0] */
		punpckldq	mm6, [TL_one]						/*	1.0			z				*/
	Blend0Done:

		// BLEND #1		mm0=hx|hy, mm3=hw|hz, mm5=cumulBlend, mm6=x|y, mm7=z|1, eax=blend[1]
#if (K3DXFORM_NUM_VERTEX_BLENDS >= 2)
		cmp			eax, 0								/* check blend1 for early out	*/
  #if (K3DXFORM_NUM_VERTEX_BLENDS >= 3)
		mov			eax, [esi+FVF_OFFSET_BLEND2]
  #endif
		jz			Blend1Done
#endif
		movq		[ebx]TL_K3DTMP.hv.z, mm3			/* z and w are swapped!			*/
		movq		[ebx]TL_K3DTMP.hv.x, mm0
		movd		[cumulBlend], mm5
		movq		mm3, mm7							/*	y			x			*/
		movq		mm2, mm6							/*	1.0			z			*/
		pfmul		mm3, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		pfmul		mm2, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm4, mm7							/*	y			x			*/
		movq		mm5, mm6							/*	1.0			z			*/
		pfmul		mm4, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		pfmul		mm5, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm0, mm7							/*	y			x			*/
		movq		mm1, mm6							/*	1.0			z			*/
		pfmul		mm7, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfmul		mm6, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
		pfacc		mm3, mm2							/*	z*_34+_44	x*_14+y*_24	*/
		pfacc		mm4, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm0, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfmul		mm1, [ebx+(1*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm7, mm6							/*	z*_32+_42	x*_12+y*_22	*/
		pfacc		mm0, mm1							/*	z*_31+_41	x*_11+y*_21	*/
#if (K3DXFORM_NUM_VERTEX_BLENDS >= 2)
		movd		mm2, [esi+FVF_OFFSET_BLEND1]		/*	blend = blendFactor[1]		*/
		movd		mm5, [cumulBlend]					/*	0			cumulBlend		*/
#else
		movd		mm2, [cumulBlend]					/*	0			cumulBlend		*/
#endif
		pfacc		mm3, mm4				/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44	*/
		pfacc		mm0, mm7				/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		punpckldq	mm2, mm2
		pfmul		mm3, mm2							/*	hz*blend	hw*blend		*/
		pfmul		mm0, mm2							/*	hy*blend	hx*blend		*/
		pfadd		mm3, [ebx]TL_K3DTMP.hv.z
		pfadd		mm0, [ebx]TL_K3DTMP.hv.x
#if (K3DXFORM_NUM_VERTEX_BLENDS >= 2)
		movd		mm6, [esi]D3DVECTOR.z				/*	0			z				*/
		movq		mm7, [esi]D3DVECTOR.x				/*	y			x				*/
		pfsub		mm5, mm2							/* cumulBlend -= blendfactor[1]	*/
		punpckldq	mm6, [TL_one]						/*	1.0			z				*/
	Blend1Done:

		// BLEND #2		mm0=hx|hy, mm3=hw|hz, mm5=cumulBlend, mm6=x|y, mm7=z|1, eax=blend[2]
  #if (K3DXFORM_NUM_VERTEX_BLENDS >= 3)
		cmp			eax, 0								/* check blend2 for early out	*/
		jz			Blend2Done
  #endif
		movq		[ebx]TL_K3DTMP.hv.z, mm3
		movq		[ebx]TL_K3DTMP.hv.x, mm0
		movd		[cumulBlend], mm5
		movq		mm3, mm7							/*	y			x			*/
		movq		mm2, mm6							/*	1.0			z			*/
		pfmul		mm3, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		pfmul		mm2, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm4, mm7							/*	y			x			*/
		movq		mm5, mm6							/*	1.0			z			*/
		pfmul		mm4, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		pfmul		mm5, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm0, mm7							/*	y			x			*/
		movq		mm1, mm6							/*	1.0			z			*/
		pfmul		mm7, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfmul		mm6, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
		pfacc		mm3, mm2							/*	z*_34+_44	x*_14+y*_24	*/
		pfacc		mm4, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm0, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfmul		mm1, [ebx+(2*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm7, mm6							/*	z*_32+_42	x*_12+y*_22	*/
		pfacc		mm0, mm1							/*	z*_31+_41	x*_11+y*_21	*/
  #if (K3DXFORM_NUM_VERTEX_BLENDS >= 3)
		movd		mm5, [cumulBlend]
		movd		mm2, [esi+FVF_OFFSET_BLEND2]		/* cumulBlend = blendFactor[2]	*/
  #else
		movd		mm2, [cumulBlend]					/* 1.0 - (blendfactor[0] + blendfactor[1]) */
  #endif
		pfacc		mm3, mm4				/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44	*/
		pfacc		mm0, mm7				/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		punpckldq	mm2, mm2
		pfmul		mm3, mm2							/*	hz*blend	hw*blend		*/
		pfmul		mm0, mm2							/*	hy*blend	hx*blend		*/
		pfadd		mm3, [ebx]TL_K3DTMP.hv.z
		pfadd		mm0, [ebx]TL_K3DTMP.hv.x
  #if (K3DXFORM_NUM_VERTEX_BLENDS >= 3)
		movd		mm6, [esi]D3DVECTOR.z				/*	0			z				*/
		movq		mm7, [esi]D3DVECTOR.x				/*	y			x				*/
		pfsub		mm5, mm2							/* cumulBlend -= blendfactor[2]	*/
		punpckldq	mm6, [TL_one]						/*	1.0			z				*/
	Blend2Done:

		// BLEND #3		mm0=hx|hy, mm3=hw|hz, mm5=cumulBlend, mm6=x|y, mm7=z|1
		movq		[ebx]TL_K3DTMP.hv.z, mm3
		movq		[ebx]TL_K3DTMP.hv.x, mm0
		movd		[cumulBlend], mm5
		movq		mm3, mm7							/*	y			x			*/
		movq		mm2, mm6							/*	1.0			z			*/
		pfmul		mm3, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._41	/*	y*_24		x*_14		*/
		pfmul		mm2, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._43	/*	_44			z*_34		*/
		movq		mm4, mm7							/*	y			x			*/
		movq		mm5, mm6							/*	1.0			z			*/
		pfmul		mm4, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._31	/*	y*_23		x*_13		*/
		pfmul		mm5, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._33	/*	_43			z*_33		*/
		movq		mm0, mm7							/*	y			x			*/
		movq		mm1, mm6							/*	1.0			z			*/
		pfmul		mm7, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._21	/*	y*_22		x*_12		*/
		pfmul		mm6, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._23	/*	_42			z*_32		*/
		pfacc		mm3, mm2							/*	z*_34+_44	x*_14+y*_24	*/
		pfacc		mm4, mm5							/*	z*_33+_43	x*_13+y*_23	*/
		pfmul		mm0, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._11	/*	y*_21		x*_11		*/
		pfmul		mm1, [ebx+(3*SIZE D3DMATRIX)]TL_K3DTMP.xfmCurrentT._13	/*	_41			z*_31		*/
		pfacc		mm7, mm6							/*	z*_32+_42	x*_12+y*_22	*/
		pfacc		mm0, mm1							/*	z*_31+_41	x*_11+y*_21	*/
		movd		mm2, [cumulBlend]					/* 1.0 - (blendfactor[0] + blendfactor[1] - blendfactor[2]) */
		pfacc		mm3, mm4				/*	hz=x*_13+y*_23+z*_33+_43	hw=x*_14+y*_24+z*_34+_44	*/
		pfacc		mm0, mm7				/*	hy=x*_12+y*_22+z*_32+_42	hx=x*_11+y*_21+z*_31+_41	*/
		punpckldq	mm2, mm2
		pfmul		mm3, mm2							/*	hz*blend	hw*blend	*/
		pfmul		mm0, mm2							/*	hy*blend	hx*blend	*/
		pfadd		mm3, [ebx]TL_K3DTMP.hv.z
		pfadd		mm0, [ebx]TL_K3DTMP.hv.x
  #endif //(K3DXFORM_NUM_VERTEX_BLENDS >= 3)
#endif //(K3DXFORM_NUM_VERTEX_BLENDS >= 2)
	BlendsDone:
		// mm0=hx|hy, mm3=hw|hz


#if K3DXFORM_DIFFUSE_COLOR | K3DXFORM_SPECULAR_COLOR
		mov			eax, esi
#if K3DXFORM_DIFFUSE_COLOR
		add			eax, [ecx]RC.tl.InFVF.dwDiffuseOffset
#else
		add			eax, [ecx]RC.tl.InFVF.dwSpecularOffset
#endif
#endif

		pfrcp		mm5, mm3						/* 1/w approx	1/w approx	*/
		movq		mm4, mm3						/* temporary copy (for z)	*/
		punpckldq	mm3, mm3						/*	hw			hw			*/
		pfrcpit1	mm3, mm5						/* 		intermediate 		*/
		pfmul		mm0, [ecx]RC.tl.ViewData.scaleX	/* y*scaleY		x*scaleX	*/

		prefetchnta	[esi+192]							/* +192 seems to be the sweet spot */
		prefetchw	[edi+128]						/* +128 seems to be the sweet spot */

		punpckhdq	mm4, mm4						/*	hz			hz			*/
		pfrcpit2	mm3, mm5						/* 1/w			1/w			*/

#if K3DXFORM_DIFFUSE_COLOR && K3DXFORM_SPECULAR_COLOR
		movq		mm6, [eax]						/* spec			diffuse		*/
#elif K3DXFORM_DIFFUSE_COLOR
		movd		mm6, [eax]						/* 0			diff		*/
#elif K3DXFORM_SPECULAR_COLOR
		movd		mm6, [eax]						/* 0			spec		*/
		punpckldq	mm6, mm6						/* spec			spec		*/
#endif

		movq		mm5, [ecx]RC.tl.ViewData.viewDataOffsetX	/* offsetY	offsetX	*/
		pfmul		mm0, mm3						/* w*y*scaleY	w*x*scaleX	*/

		movd		mm2, [TL_one]					/*	0			1.0			*/
		mov			eax, [numVerts]

		add			esi, [ecx]RC.tl.InFVF.dwStride

		pfadd		mm0, mm5			/* y=w*y*scaleY+offsetY		x=w*x*scaleX+offsetX	*/

		mov			dword ptr [edi]TLBN.wfbi, TL_VERT_XFORMED_NOT_LIT
#if K3DXFORM_DIFFUSE_COLOR | K3DXFORM_SPECULAR_COLOR
		movq		[edi]TLBN.diffuse, mm6			/* spec			diffuse		*/
#endif
		movd		[edi]TLBN.z, mm4				/*	hz (not sz)	*/
		movd		[edi]TLBN.w, mm3				/*	rhw	*/
		movq		[edi]TLBN.x, mm0				/*	y			x			*/

		// increment the pointers
		dec			eax
		lea			edi, [edi + SIZE TLBN]
		jns			NextVert

	Done:
		femms
	}
}

#endif	//K3DXFORM_NUM_VERTEX_BLENDS

#endif	// K3DXFORM_CLIPPED

#undef K3DXFORM_NAME
#undef K3DXFORM_NUM_VERTEX_BLENDS
#undef K3DXFORM_CLIPPED
#undef K3DXFORM_USERCLIPPPLANES
#undef K3DXFORM_DIFFUSE_COLOR
#undef K3DXFORM_SPECULAR_COLOR

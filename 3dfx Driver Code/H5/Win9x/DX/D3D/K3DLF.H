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
** File name: k3dlf.h
**
** Description: K3D Lighting code
**
** $
*/


/*-------------------------------------------------------------------
Function Name:  LightK3d_Dir_XXX
Description:    Directional Lights 
Parameters:   	RC *pRC -- pointer to the current rendering context
                TLLIGHT *pL -- pointer to the current light

                mm6 = diffuse, mm7 = specular

Information:    This function is included in k3dlitfn.c. It will handle 
                complete calculation for the lighting of a vertex.  The
				diffuse and specular components are acculated in mm6
				and mm7, respectively.

                NAME - the name of the generated directional specular lighting 
				DIRECTIONAL - Directional light
				DO_DIFFUSE_AMBIENT - add the diffuse ambient component
					(this is always on)
				HAS_NORMALS - the FVF has a normal
				DO_DIFFUSE - do the diffuse component
				DO_SPECULAR - do the specular component
                DO_LOCALVIEWER - 1 = local lighting, 0 = infinite lighting


Return:       	mm6 = diffuse, mm7 = specular
-------------------------------------------------------------------*/
#if DIRECTIONAL
__declspec(align(64)) void NAME( RC *pRc, TLLIGHT *pL) 
{
	_asm {
		mov			ecx, [pRc]
		mov			edx, [pL]
		mov			ebx, [ecx]RC.tl.pTLK				// TL_K3DTMP *pTlkTmp

	    // Add the material's ambient component
#if DO_DIFFUSE_AMBIENT
#if DO_COLORVERTEX
		movq		mm2, [edx]TLLIGHT.p8La_U12pt0
		pmulhuw		mm2, [ebx]TL_K3DTMP.p8AmbientSrc_U12pt0	// La*AmbSrc
		paddusw		mm6, mm2							// Diffuse += La*AmbSrc
#else
		paddusw		mm6, [edx]TLLIGHT.p8Ma_La_U16pt0	// Diffuse += Ma_La
#endif	//DO_COLORVERTEX
#endif	// DO_DIFFUSE_AMBIENT


		// SPECIAL CASE D3DFVF_NORMAL
		//if ((pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL) == 0)	return;
#if HAS_NORMALS

		// Dot the light vector with the normal, if it's negative then
		// the light source is behind the surface, so we're done
		movq		mm0, [ebx]TL_K3DTMP.cn.x			//	cn.y			cn.x
		movq		mm4, [edx]TLLIGHT.direction_in_eye.x //	pos.y			pos.x
		movd		mm1, [ebx]TL_K3DTMP.cn.z			//	0				cn.z
		movd		mm5, [edx]TLLIGHT.direction_in_eye.z //	0				pos.z
		pfmul		mm0, mm4							//	pos.y-cn.y	pos.x-cn.x
		pfmul		mm1, mm5							//	0			pos.z-cn.z
#if DO_DIFFUSE
#if DO_COLORVERTEX
		movd		mm2, [exp_X64K]
		movq		mm3, [ebx]TL_K3DTMP.p8DiffuseSrc_U12pt0	// DiffSrc
#else
		movd		mm2, [exp_X4K]
#endif	//DO_COLORVERTEX
#endif	// DO_DIFFUSE
		pfacc		mm0, mm0
		pfadd		mm0, mm1							// dot = DOT(direction_in_eye, cn)
		movd		dword ptr [dwK3dCmpTmp], mm0
#if DO_DIFFUSE
		paddd		mm2, mm0			// dot << 12 (or << 16 for colorvertex)
#if DO_COLORVERTEX
		pmulhuw		mm3, [edx]TLLIGHT.p8Ld_U12pt0		// Ld * DiffSrc
#endif	//DO_COLORVERTEX
#endif	// DO_DIFFUSE
		// mm0 = dot, mm4,mm5=direction_in_eye, mm6=diff, mm7=spec
		
		cmp			dword ptr [dwK3dCmpTmp], 0
		jng			Done	// ReturnEarly

#if DO_DIFFUSE
		pf2iw		mm2, mm2			// (int)(dot << 12)  (<<16 for colorvertex
		pshufw		mm2, mm2, 0			// broadcast
#if DO_COLORVERTEX
		pmulhuw		mm2, mm3			// diff = dot * Ld * DiffSrc
#else
		pmulhuw		mm2, [edx]TLLIGHT.p8Md_Ld_U12pt4	// diff = dot * Md_Ld
#endif	//DO_COLORVERTEX
		paddusw		mm6, mm2			// Diffuse += diff
#endif	// DO_DIFFUSE
		// mm4,mm5=direction_in_eye, mm6=diff, mm7=spec


		/*********************
		*	Specular
		*********************/
		// SPECIAL CASE TLPV_DOSPECULAR
//		TEST		[ECX]RC.tl.dwTLState, TLPV_DOSPECULAR
//		je			Done
#if DO_SPECULAR

		// SPECIAL CASE TLPV_LOCALVIEWER
		//TEST		[ECX]RC.tl.dwTLState, TLPV_LOCALVIEWER
		//JE			NonLocalViewer
#if DO_LOCALVIEWER

		// calc vector from vertex to the eye
		//SubtractVector( &Ldata->eye_in_eye, &pTlkTmp->cv, &eye);
		//Normalize( &eye );
		//AddVector( &pL->direction_in_eye, &eye, &half );
		movq		mm0, [ecx]RC.tl.lighting.eye_in_eye.x		// e.y		e.x		// MIGHT NOT BE
		movd		mm1, [ecx]RC.tl.lighting.eye_in_eye.z		// 0		e.z		// ALIGNED!!!
		pfsub		mm0, [ebx]TL_K3DTMP.cv.x					// e.y-cv.y	e.x-cv.x
		pfsub		mm1, [ebx]TL_K3DTMP.cv.z					// -		e.z-cv.z
		movq		mm3, mm0
		movq		mm2, mm1
		pfmul		mm0, mm0									// y*y		x*x
		pfmul		mm1, mm1									// -		z*z
		pfacc		mm0, mm0
		pfadd		mm0, mm1									// x*x+y+y+z*z
		pfrsqrt		mm0, mm0									// eye = 1/sqrt(x*x+y+y+z*z)
		pfmul		mm3, mm0									// eye.y	eye.x
		pfmul		mm2, mm0									// -		eye.z
		// calc halfway vector
		pfadd		mm4, mm3									// eye.y+dirE.y	eye.x+dirE.xy
		pfadd		mm5, mm2									// -			eye.z+dirE.xz
//		JMP		NormalizeHalfwayVector
#else	// DO_LOCALVIEWER
//	NonLocalViewer:		// eye = 0,0,-1
		pfadd		mm5, [TL_soa_neg_1]							// -			half.z

// mm1=att, mm4,mm5 = half, mm6=diff, mm7=spec
//	NormalizeHalfwayVector:
#endif	// DO_LOCALVIEWER
		//Normalize( &half );
		movq		mm2, mm4
		movq		mm3, mm5
		pfmul		mm4, mm4									// y*y			x*x
#if DO_COLORVERTEX
		movq		mm1, [ebx]TL_K3DTMP.p8SpecularSrc_U16pt0	// SpecSrc
#endif	//DO_COLORVERTEX
		pfmul		mm5, mm5									// -			z*z
		pfacc		mm4, mm4
#if DO_COLORVERTEX
		pmulhuw		mm1, [edx]TLLIGHT.p8Ls_U12pt0				// Ls * SpecSrc
#endif	//DO_COLORVERTEX
		pfadd		mm4, mm5									// 		x*x+y+y+z*z
		movq		mm0, [ebx]TL_K3DTMP.cn.x					// cn.y			cn.x
		pfrsqrt		mm4, mm4									// half = 1/sqrt(x*x+y+y+z*z)
		movd		mm5, [ebx]TL_K3DTMP.cn.z					// 0			cn.z
		pfmul		mm2, mm4									// half.y		half.x
		pfmul		mm3, mm4									// -			half.z

		//dot = DotProduct( &pTlkTmp->cn, &half );
		mov			eax, [ecx]RC.tl.lighting.specThreshold
		pfmul		mm0, mm2									// cn.y*half.y	cn.x*half.x
		pfmul		mm5, mm3									// -			cn.z*half.z
		movd		mm4, [exp_X4K]								// same for colorvertex
		pfacc		mm0, mm0
		pfadd		mm0, mm5									// dot = DOT(cn, half)

		//if(dot >= pRc->tl.lighting.specThreshold)		// early out
		movd		dword ptr [dwK3dCmpTmp], mm0
		cmp			eax, dword ptr [dwK3dCmpTmp]
		jge			Done				// THIS BRANCH STAYS, EVEN W/SPECIAL CASE
#if DO_COLORVERTEX
		movq		mm5, mm1									// Ls * SpecSrc
#endif	//DO_COLORVERTEX

		//coeff = pow(dot, material.power);
		call		[ecx]RC.tl.lighting.K3dPowerFunction

		//spec += coef*Ms_Ls
		paddd		mm0, mm4			// coeff << 12
		pf2iw		mm0, mm0			// (int)coeff
		pshufw		mm0, mm0, 0			// broadcast
#if DO_COLORVERTEX
		pfmul		mm0, mm5			// spec = coeff * Ls * SpecSrc
#else
		pmulhuw		mm0, [edx]TLLIGHT.p8Ms_Ls_U12pt4	// spec = coeff * Ms_Ls
#endif	//DO_COLORVERTEX
		paddusw		mm7, mm0			// Specular += spec


#endif	// DO_SPECULAR
	Done:
#endif // HAS_NORMALS
	}
}
#endif // DIRECTIONAL



/*-------------------------------------------------------------------
Function Name:  LightK3d_Point_XXX and LightK3d_Spot_XXX
Description:    Point and Spot Lights 
Parameters:   	RC *pRC -- pointer to the current rendering context
                TLLIGHT *pL -- pointer to the current light

                mm6 = diffuse, mm7 = specular

Information:    This function is included in k3dlitfn.c. It will handle 
                complete calculation for the lighting of a vertex.  The
				diffuse and specular components are acculated in mm6
				and mm7, respectively.

                NAME - the name of the generated directional specular lighting 
				POINT - Point light
				SPOT - Spot light
				DO_DIFFUSE_AMBIENT - add the diffuse ambient component
					(this is always on)
				FALLOFF - 0 = falloff = 1.0, 1 = falloff != 1.0 (have to calculate)
				DO_QUAD - Calculate the quadradic, else att0=1.0, att1=0,
							and att2=0
				HAS_NORMALS - the FVF has a normal
				DO_DIFFUSE - do the diffuse component
				DO_SPECULAR - do the specular component
                DO_LOCALVIEWER - 1 = local lighting, 0 = infinite lighting


Return:       	mm6 = diffuse, mm7 = specular
-------------------------------------------------------------------*/
#if defined(POINT) || defined(SPOT)
__declspec(align(64)) void NAME( RC *pRc, TLLIGHT *pL)
{
	_asm {
		mov			ecx, [pRc]
		mov			edx, [pL]
		mov			ebx, [ecx]RC.tl.pTLK				// TL_K3DTMP *pTlkTmp

		//  d = pL->poistion_in_eye - pTL->cv
		//SubtractVector( &pL->position_in_eye, &pTlkTmp->cv, &d);
		movq		mm0, [edx]TLLIGHT.position_in_eye.x	//	pos.y		pos.x
		movd		mm3, [edx]TLLIGHT.position_in_eye.z	//	0			pos.z
		pfsub		mm0, [ebx]TL_K3DTMP.cv.x			//	pos.y-cv.y	pos.x-cv.x
		pfsub		mm3, [ebx]TL_K3DTMP.cv.z			//	-			pos.z-cv.z


		movq		mm4, mm0							//	dirL.y		dirL.x
		movq		mm5, mm3							//	-			dirL.z
		// mm4,mm5 = dirL, mm6=diff, mm7=spec

		// early out if out of range or exactly on the vertex
		// Wrong! if it's exactly on the vertex then the vert should get the
		// max light so don't bother checking.  I think MS did this in the 
		// refrast so they wouldn't have to deal with divide-by-zero exceptions.
		//distSquared = SquareMagnitude( &d );
		pfmul		mm0, mm0							//	y*y			x*x
		mov			eax, [edx]TLLIGHT.range_squared		// range_squared
		pfmul		mm3, mm3							//	-			z*z
		pfacc		mm0, mm0							//	x*x+y*y		x*x+y*y
#if DO_QUAD
		movd		mm1, [edx]TLLIGHT.Attenuation0		// 0		Att0
#endif	// DO_QUAD
		pfadd		mm0, mm3							//	distSquared = x*x+y*y+z*z (always positive)
		// mm0=distSquared, mm4,mm5 = dirL, mm6=diff, mm7=spec, eax=range_squared


		//if (FLOAT_CMP_POS(distSquared, >=, pL->range_squared) || FLOAT_EQZ(distSquared))	return;
		// Blow off the FLOAT_EQZ(distSquared) check, I think this is techically more correct.
		//dist = SQRTF( distSquared );
		movd		dword ptr [dwK3dCmpTmp], mm0
		pfrsqrt		mm3, mm0							// 1/sqrt(distSquared) = 1/dist

		cmp			eax, dword ptr [dwK3dCmpTmp]
		jng			Done

#if DO_QUAD
		pfrcp		mm2, mm3							// dist = sqrt(distSquared)
#endif
		// mm0=distSquared, mm1=Attenuation0, mm2=dist (if DO_QUAD), mm3=1/dist, mm4,mm5 = dirL, mm6=diff, mm7=spec 


#if DO_QUAD
		// Compute the attenuation while we're seeing if distance is 0 or too far
		// att = pL->Attenuation0 + pL->Attenuation1 * dist + pL->Attenuation2 * distSquared;
		punpckldq	mm2, mm0							// distSquared		dist
		pfmul		mm2, [edx]TLLIGHT.Attenuation1		// distSquared*Att2	dist*Att1
		pfadd		mm2, mm1							// distSquared*Att2	Att0 + dist*Att1
		pfacc		mm2, mm2							// att = Att0 + dist*Att1 + distSquared*Att2
		// att = (D3DVALUE) 1.0/att;
		pfrcp		mm2, mm2							// att = 1/att
#else
		movd		mm2, [TL_one]						// special case where att0=1.0, att1=0.0, att2=0.0
#endif	// DO_QUAD
		// mm2=att (if DO_QUAD), mm3=1/dist, mm4,mm5 = dirL, mm6=diff, mm7=spec 


	    // If the light is a spotlight compute the spot-light factor
		// SPECIAL CASE SPOTLIGHT
		//TEST	[EDX]TLLIGHT.dltType, D3DLIGHT_SPOT
		//JE		SpotDone
#if SPOT
		// Calc dot product of direction to light with light direction to
		// be compared anganst the cone angles to see if we are in the light.
		// Note that cone_dot is still scaled by dist
		//D3DVALUE cone_dot = DotProduct(&dirL, &pL->direction_in_eye) * dist;
		movq		mm0, [edx]TLLIGHT.direction_in_eye.x //	pos.y			pos.x
		movd		mm1, [edx]TLLIGHT.direction_in_eye.z //	0				pos.z
		pfmul		mm0, mm4							//	pos.y*dirL.y	pos.x*dirL.x
		pfmul		mm1, mm5							//	0				pos.z*dirL.z
		pfacc		mm0, mm0							//	x*x+y*y			x*x+y*y
		pfadd		mm0, mm1							//	-				x*x+y*y+z*z
		pfmul		mm0, mm3							//	cone_dot = DOT(dirL, direction_in_eye) * 1/dist

		//if (FLOAT_CMP_POS(cone_dot, <=, pL->cos_phi_by_2))	return;
		movd		eax, mm0
		cmp			eax, [edx]TLLIGHT.cos_phi_by_2		// early out
		jle			Done

		// modify att if in the region between phi and theta
		cmp			eax, [edx]TLLIGHT.cos_theta_by_2
		jge			SpotDone

		pfsub		mm0, [edx]TLLIGHT.cos_phi_by_2
		movd		mm1, [edx]TLLIGHT.inv_theta_minus_phi
		pfmul		mm0, mm1			// val = (cone_dot - pL->cos_phi_by_2) * pL->inv_theta_minus_phi

		// SPECIAL CASE FALLOFF
		//MOV			EAX, [edx]TLLIGHT.Falloff
		//CMP			EAX, INT32_FLOAT_ONE
		//JE			FalloffEqualsOne
#if FALLOFF
		//val = (D3DVALUE) pow( (double)val, (double)pL->Falloff )
		movd		mm1, [edx]TLLIGHT.Falloff
		call		k3dpow_amd		// ~110 clocks, destroys mm0 and mm1 
	//FalloffEqualsOne:
#endif	// FALLOFF

#if DO_QUAD
		pfmul		mm2, mm0			// att *= val
#else // DO_QUAD
		movq		mm2, mm0			// att = 1.0, so att = val
#endif	// DO_QUAD

	SpotDone:
#endif	// SPOT
		// mm2=att, mm3=dist, mm4,mm5 = dirL, mm6=diff, mm7=spec 

#if DO_DIFFUSE_AMBIENT
#if DO_COLORVERTEX
		movd		mm1, [exp_X64K]
		movq		mm0, [ebx]TL_K3DTMP.p8AmbientSrc_U12pt0	// AmbSrc
		paddd		mm1, mm2								// att << 16
		pmulhuw		mm0, [edx]TLLIGHT.p8La_U12pt0			// La * AmbSrc
		pf2iw		mm1, mm1			// (int)att << 16
		pshufw		mm1, mm1, 0			// broadcast
		pmulhuw		mm1, mm0			// amb = att * La * AmbSrc, result an U16.16 (U16.0)
#else
		// Add the material's ambient component
		movd		mm1, [exp_X4K]
		paddd		mm1, mm2			// att << 12
		pf2iw		mm1, mm1			// (int)att << 12
		pshufw		mm1, mm1, 0			// broadcast
		pmulhuw		mm1, [edx]TLLIGHT.p8Ma_La_U12pt4	// amb = att * Ma_La, result an U16.16 (U16.0)
#endif	//DO_COLORVERTEX
		paddusw		mm6, mm1			// diff += amb
#endif //DO_DIFFUSE_AMBIENT


#if HAS_NORMALS
		// Calc dot product of light dir with normal.  Note that since we
		// didn't normalize the direction the result is scaled by the distance.
		// If no normals are present, bail out since we cannot perform the
		// normal-dependent computations
		// SPECIAL CASE D3DFVF_NORMAL
		//if ((pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL) == 0)	return;
		//TEST		[ECX]RC.tl.InFVF.dwFVFType, D3DFVF_NORMAL
		//JE		Done

		// Dot the light vector with the normal, if it's negative then
		// the light source is behind the surface, so we're done
		//dot = DotProduct( &pTlkTmp->cn, &dirL );
		movq		mm0, [ebx]TL_K3DTMP.cn.x			//	cn.y		cn.x
		movd		mm1, [ebx]TL_K3DTMP.cn.z			//	0			cn.z
		pfmul		mm0, mm4							//	dirL.y-cn.y	dirL.x-cn.x
		pfmul		mm1, mm5							//	0			dirL.z-cn.z
		pfadd		mm0, mm1							//	y			x+z
		movq		mm1, mm2
		pfacc		mm0, mm0							//	dot = x*x+y*y+z*z = DOT(cn, dirL)
		pfmul		mm1, mm3							//	dist*att

		movd		dword ptr [dwK3dCmpTmp], mm0
		cmp			dword ptr [dwK3dCmpTmp], 0
		jng			Done
		pfmul		mm1, mm0							//	dot = dot*att*dist
		// mm1=dot, mm2=att, mm3=dist, mm4,mm5 = dirL, mm6=diff, mm7=spec 

#if DO_DIFFUSE
		// range of dot is huge, carefull here!!!
#if DO_COLORVERTEX
		movq		mm0, [ebx]TL_K3DTMP.p8DiffuseSrc_U12pt0	// DiffSrc
		paddd		mm1, [exp_X64K]		// dot << 16
		pmulhuw		mm0, [edx]TLLIGHT.p8Ld_U12pt0			// Ld * DiffSrc
#else
		paddd		mm1, [exp_X4K]		// dot << 12
#endif	//DO_COLORVERTEX
		pf2iw		mm1, mm1			// (int)dot
#if DO_SPECULAR
		pfmul		mm4, mm3			// dirL.y*dist	dirL.x*dist
		pfmul		mm5, mm3			// -			dirL.z*dist
#endif	// DO_SPECULAR
		pshufw		mm1, mm1, 0			// broadcast
#if DO_COLORVERTEX
		pmulhuw		mm1, mm0							// diff = dot * Ld * DiffSrc
#else
		pmulhuw		mm1, [edx]TLLIGHT.p8Md_Ld_U12pt4	// diff = dot * Md_Ld
#endif	//DO_COLORVERTEX
		paddusw		mm6, mm1			// Diffuse += diff
#endif	// DO_DIFFUSE
		// mm2=att, mm3=dist, mm4,mm5 = dirL, mm6=diff, mm7=spec 


		/*********************
		*	Specular
		*********************/
		// SPECIAL CASE TLPV_DOSPECULAR
		//TEST		[ECX]RC.tl.dwTLState, TLPV_DOSPECULAR
		//JE			Done
#if DO_SPECULAR

		// normalize light direction
		//dirL *= dist;
//		pfmul		mm4, mm3			// dirL.y*dist	dirL.x*dist
//		pfmul		mm5, mm3			// -			dirL.z*dist
		// mm2=att, mm4,mm5 = dirL, mm6=diff, mm7=spec


		// SPECIAL CASE TLPV_LOCALVIEWER
		//TEST		[ECX]RC.tl.dwTLState, TLPV_LOCALVIEWER
		//JE			NonLocalViewer
#if DO_LOCALVIEWER

		// calc vector from vertex to the eye
		//SubtractVector( &Ldata->eye_in_eye, &pTlkTmp->cv, &eye);
		//Normalize( &eye );
		movq		mm0, [ecx]RC.tl.lighting.eye_in_eye.x		// e.y		e.x		// NEEDS ALIGNMENT!!!
		movd		mm1, [ecx]RC.tl.lighting.eye_in_eye.z		// 0		e.z
		pfsub		mm0, [ebx]TL_K3DTMP.cv.x					// e.y-cv.y	e.x-cv.x
		pfsub		mm1, [ebx]TL_K3DTMP.cv.z					// -		e.z-cv.z
		punpckldq	mm5, mm2									// att		dirL.z	(ran out of registers)
		movq		mm3, mm0
		movq		mm2, mm1
		pfmul		mm0, mm0									// y*y		x*x
		pfmul		mm1, mm1									// -		z*z
		pfacc		mm0, mm0
		pfadd		mm0, mm1									// x*x+y+y+z*z
		pfrsqrt		mm0, mm0									// eye = 1/sqrt(x*x+y+y+z*z)
		pfmul		mm3, mm0									// eye.y	eye.x
		pfmul		mm2, mm0									// -		eye.z
		// calc halfway vector
		//AddVector( &dirL, &eye, &half );
		movq		mm1, mm5									// att			dirL.z
		pfadd		mm4, mm3									// eye.y+dirL.y	eye.x+dirL.xy
		pfadd		mm5, mm2									// -			eye.z+dirL.xz
		punpckhdq	mm1, mm1									// -			att
		//JMP			NormalizeHalfwayVector

#else	// DO_LOCALVIEWER
	//NonLocalViewer:		// eye = 0,0,-1
		movq		mm1, mm2									// -			att
		// calc halfway vector
		//AddVector( &dirL, &eye, &half );
		pfadd		mm5, [TL_soa_neg_1]							// -			half.z

		// mm1=att, mm4,mm5 = half, mm6=diff, mm7=spec
#endif	// DO_LOCALVIEWER
	//NormalizeHalfwayVector:
		//Normalize( &half );
		movq		mm2, mm4
		movq		mm3, mm5
		pfmul		mm4, mm4									// y*y			x*x
		pfmul		mm5, mm5									// -			z*z
		movq		mm0, [ebx]TL_K3DTMP.cn.x					// cn.y			cn.x
		pfacc		mm4, mm4
		pfadd		mm4, mm5									// 		x*x+y+y+z*z
		pfrsqrt		mm4, mm4									// half = 1/sqrt(x*x+y+y+z*z)
		movd		mm5, [ebx]TL_K3DTMP.cn.z					// 0			cn.z
		pfmul		mm2, mm4									// half.y		half.x
		pfmul		mm3, mm4									// -			half.z
		// mm1=att, mm2,mm3 = half, mm6=diff, mm7=spec
		//dot = DotProduct( &pTlkTmp->cn, &half );
		pfmul		mm0, mm2									// cn.y*half.y	cn.x*half.x
		pfmul		mm5, mm3									// -			cn.z*half.z
		mov			eax, [ecx]RC.tl.lighting.specThreshold
		pfacc		mm0, mm0
		movq		mm4, mm1									// power function modifies mm1!!!
		pfadd		mm0, mm5									// dot = DOT(cn, half)

		//if(dot >= pRc->tl.lighting.specThreshold)
		movd		dword ptr [dwK3dCmpTmp], mm0
		cmp			eax, dword ptr [dwK3dCmpTmp]				// early out
		jg			Done				// THIS BRANCH STAYS, EVEN W/SPECIAL CASE
#if DO_COLORVERTEX
		movq		mm5, [ebx]TL_K3DTMP.p8SpecularSrc_U16pt0	// SpecSrc
#endif	//DO_COLORVERTEX

		//coeff = pow(dot, material.power) * att;
		call		[ecx]RC.tl.lighting.K3dPowerFunction
		pfmul		mm0, mm4									// att * pow

#if DO_COLORVERTEX
		pmulhuw		mm5, [edx]TLLIGHT.p8Ls_U12pt0	// Ls * SpecSrc
		paddd		mm0, [exp_X4K]		// coeff << 12
		pf2iw		mm0, mm0			// (int)coeff
		pshufw		mm0, mm0, 0			// broadcast
		pmulhuw		mm0, mm5			// spec = coef * Ls * SpecSrc
#else
		//spec += coef*Ms_Ls
		paddd		mm0, [exp_X4K]		// coeff << 12
		pf2iw		mm0, mm0			// (int)coeff
		pshufw		mm0, mm0, 0			// broadcast
		pmulhuw		mm0, [edx]TLLIGHT.p8Ms_Ls_U12pt4	// spec = coef * Ms_Ls
#endif	//DO_COLORVERTEX
		paddusw		mm7, mm0			// Specular += spec

#endif	// DO_SPECULAR
#endif // HAS_NORMALS
	Done:
	}
}
#endif //defined(POINT) || defined(SPOT)

#undef NAME
#undef DIRECTIONAL
#undef POINT
#undef SPOT
#undef DO_QUAD
#undef FALLOFF
#undef DO_DIFFUSE_AMBIENT
#undef HAS_NORMALS
#undef DO_DIFFUSE
#undef DO_SPECULAR
#undef DO_LOCALVIEWER
#undef DO_COLORVERTEX

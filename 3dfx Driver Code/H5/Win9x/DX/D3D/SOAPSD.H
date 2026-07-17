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
** File name: soapsd.h
**
** Description: Point and Spot Light code for the diffuse component of lighting
**
** $Revision: 10$
** $Date: 10/11/00 8:50:32 PM$
**
** $Log: 
**  10   3dfx      1.3.1.5     10/11/00 Brent           Forced check in to enforce
**       branching.
**  9    3dfx      1.3.1.4     09/04/00 Allen Hansen    colorvertex bug, caught in
**       whql stateblock tests
**  8    3dfx      1.3.1.3     09/02/00 Allen Hansen    added colorvertex support
**  7    3dfx      1.3.1.2     08/29/00 Allen Hansen    changed fSOADiffuse to
**       fSOADiffuseOut
**  6    3dfx      1.3.1.1     08/21/00 Allen Hansen    whql lighting bug fixes
**  5    3dfx      1.3.1.0     06/25/00 Allen Hansen    fixed bug with distSquared,
**       added support for falloff other than 1.0, updated TL_SOATMP variable name
**  4    Napalm    1.3         06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  3    Napalm    1.2         05/17/00 Scott Kephart   Improved scheduling in
**       spotlights
**  2    Napalm    1.1         05/12/00 Scott Kephart   Removed rcpps instruction
**       -- it was only needed for attenuation, and it appeared in an #if
**       !ATTENUATE ... Thanks Allen!
**  1    Napalm    1.0         05/11/00 Scott Kephart   
** $
*/

// Diffuse Point and Spot Lights

/*-------------------------------------------------------------------
Function Name:  POINT_DIFF_XXXXX
                SPOT_DIFF_XXXXX
Description:    Compute diffuse color for a point or spot light 
                based on various options
Parameters:   
                RC *pRC -- pointer to the current rendering context
                TLLIGHT *pL -- pointer to the current light

                xmm5 - RED component of the diffuse portion of the vertex
                xmm6 - GREEN component of the diffuse portion of the vertex
                xmm7 - BLUE component of the diffuse portion of the vertex
Information:    
                This function is included in soalitfn.c. It will handle 
                complete calculation for the diffuse component of lighting
                for a vertex. The red, green, and blue color compoenents are
                accumulated in xmm5, xmm6, and xmm7. 

                The following defines control which portions of the lighting
                equation are handled by this function

                NAME - the name of the generated directional diffuse lighting 
                function
                DO_AR - Ambient.red is non-zero
                DO_AG - Ambient.green is non-zero
                DO_AB - Ambient.blue is non-zero
                ATTENUATE - is attenuation used in this vertex at all
                FALLOFF - is the exponent for spotlight falloff != 1.0?
                SPOT - is this a spot light or a point light?
                NORMALS - this vertex has a normal
                DO_R - Diffuse.red is non-zero
                DO_G - Diffuse.green is non-zero
                DO_B - Diffuse.blue is non-zero

                
Return:         
                xmm5 - RED component of the diffuse portion of the vertex
                xmm6 - GREEN component of the diffuse portion of the vertex
                xmm7 - BLUE component of the diffuse portion of the vertex

                pL->SOAwritemask = which SOA groups have non-zero diffuse lighting 
                components (used by specular pass, later)

                pL->dwNonZeroDot = 0 when all 4 dot products are zero. 
                (Allowing the remainder of lighting for this light
                on this vertext to be skipped.

                pL->SOAatt = attenuation for the light
-------------------------------------------------------------------*/

ALIGN32 void NAME(
    RC *pRc, 
    TLLIGHT *pL) 
{
  TL_SOATMP *pTL = pRc->tl.pTL;

  __asm 
  {
    mov     eax, pL           // eax = pointer to the light
    mov     edx, pTL          // edx = pointer to the TL_SOATMP



    //  d = pL->poistion_in_eye - pTL->SOAcv
    //distSquared = d.x * d.x + d.y * d.y + d.z * d.z
    movaps  xmm0, [eax]_LIT.SOAposition_in_eye.x
    movaps  xmm1, [eax]_LIT.SOAposition_in_eye.y
    movaps  xmm2, [eax]_LIT.SOAposition_in_eye.z
    subps   xmm0, [edx]_STL.SOAcv.x
    subps   xmm1, [edx]_STL.SOAcv.y
    subps   xmm2, [edx]_STL.SOAcv.z
    movaps  xmm3, xmm0
    movaps  xmm4, xmm1
    movaps  [eax]_LIT.SOAd.x, xmm0         // pTL->d = SOAposition_in_eye - cv
    movaps  [eax]_LIT.SOAd.y, xmm1
    movaps  [eax]_LIT.SOAd.z, xmm2
    //distSquared = d.x * d.x + d.y * d.y + d.z * d.z
    mulps   xmm3, xmm3
    mulps   xmm4, xmm4
    addps   xmm3, xmm4
    movaps  xmm4, xmm2
    mulps   xmm4, xmm4
    addps   xmm3, xmm4                //xmm3 = distSquared

    // if (FLOAT_CMP_POS(distSquared, >=, pL->SOArange_squared.f[0]) ||
    //     FLOAT_EQZ(distSquared))
    // {
    //     return;
    // }

    // All 4 distances == 0?

    xorps   xmm4, xmm4
//    cmpneqps xmm4, xmm0 			// AllenH bugfix 6/15:  xmm0=d.x, xmm3=distSquared
    cmpneqps xmm4, xmm3 			// AllenH bugfix 6/15:  xmm0=d.x, xmm3=distSquared
    movaps  [eax]_LIT.SOAwritemask, xmm4	// (distSquared != 0) ? -1 : 0

    movaps  xmm4, [eax]_LIT.SOArange_squared;
    cmpleps xmm4, xmm3						// (range_squared <= distSquared) ? -1 : 0
    andnps  xmm4, [eax]_LIT.SOAwritemask	// (distSquared < range_squared) && (distSquared != 0) ? -1 : 0
    movaps  [eax]_LIT.SOAwritemask, xmm4
    
    rsqrtps  xmm4, xmm3       //xmm4 = 1/dist = 1/sqrt(distSquard)
#if ATTENUATE
    rcpps    xmm1, xmm4       //xmm1 = dist
#endif
    movaps  [eax]_LIT.SOAdist, xmm4

#if ATTENUATE
    // Calculate Attenuation
    // Att = 1 / Attenuation0 + Attenuation1 * dist + Attentuation2 * (dist**2)
    mulps   xmm3, [eax]_LIT.SOAAttenuation2
    mulps   xmm1, [eax]_LIT.SOAAttenuation1
    addps   xmm3, [eax]_LIT.SOAAttenuation0
    addps   xmm3, xmm1                      //xmm0 = att
    rcpps   xmm3, xmm3                      //xmm0 = 1/att
    movaps  [eax]_LIT.SOAatt, xmm3
#else
    movaps  xmm3, TL_soa_1
    movaps  [eax]_LIT.SOAatt, xmm3
#endif //ATTENUATE    

#if SPOT
  // compute the spot-light factor
  // Calc dot product of direction to light with light direction to
  // be compared against the cone angles to see if we are in the
  // light.
  // Note that cone_dot is still scaled by dist
  // D3DVALUE cone_dot = DotProductToSOA2(&d, &pL->direction_in_eye) * dist;
  
  // if (FLOAT_CMP_POS(cone_dot, <=, pL->SOAcos_phi_by_2.f[0]))
  // {
  //     return;
  // }

//  DotProduct3SOA(&pL->d, &pL->direction_in_eye);

#if ATTENUATE
//    movaps  xmm0, [eax]_LIT.d.x
    movaps  xmm1, [eax]_LIT.SOAd.y
//    movaps  xmm2, [eax]_LIT.d.z
#endif //ATTENUATE
    mulps   xmm0, [eax]_LIT.SOAdirection_in_eye.x
    mulps   xmm1, [eax]_LIT.SOAdirection_in_eye.y
    addps   xmm0, xmm1
    mulps   xmm2, [eax]_LIT.SOAdirection_in_eye.z
    addps   xmm0, xmm2


    mulps   xmm0, xmm4          //xmm0 = cone-dot
    movaps  xmm1, xmm0          //xmm1 = cone-dot

    cmpleps xmm1, [eax]_LIT.SOAcos_phi_by_2    //xmm1 = write mask
    andnps  xmm1, [eax]_LIT.SOAwritemask
    movmskps ecx, xmm1
    test    ecx, ecx
    jz      Spot_Diff_Bail

    // modify att if in the region between phi and theta
    // if (FLOAT_CMP_POS(cone_dot, <, pL->SOAcos_theta_by_2.f[0]))

    movaps  [eax]_LIT.SOAwritemask, xmm1
    movaps  xmm1, xmm0                              //xmm1 = cone-dot
    cmpltps   xmm1, [eax]_LIT.SOAcos_theta_by_2     //xmm1 = write mask for 


//     D3DVALUE val = (cone_dot - pL->SOAcos_phi_by_2.f[0]) *
//          pL->SOAinv_theta_minus_phi.f[0];
    subps   xmm0, [eax]_LIT.SOAcos_phi_by_2
    mulps   xmm0, [eax]_LIT.SOAinv_theta_minus_phi

#if 1 //FALLOFF
    // Need to handle case where Falloff != 1.0....
    // val = pow(val, pL->Falloff);
	mov		ecx, [eax]_LIT.Falloff
	cmp		ecx, INT32_FLOAT_ONE			// 1.0f
	je		FallOffNot1

//    movaps  [eax]_LIT.SOAcmpres, xmm1
}  
    taylorpowSSE(&pL->Falloff);
    
__asm  {
    mov     eax, pL           // eax = pointer to the light
    mov     edx, pTL          // edx = pointer to the TL_SOATMP

//    movaps  xmm1, [eax]_LIT.SOAcmpres		// writemask = cone-dot < SOAcos_theta_by_2

FallOffNot1:
#endif // FALLOFF

    movaps  xmm3, TL_soa_1                  //xmm3 = 1.0

    movaps  xmm2, xmm1                      //xmm2 = writemask
    andps   xmm2, xmm0                      //xmm2 = val & writemask
    andnps  xmm1, xmm3                      //xmm1 = ~writemask & 1.0
    orps    xmm2, xmm1                      //xmm2 = (val & writemask) | (~writemask & 1.0)
#if ATTENUATE
    mulps   xmm2, [eax]_LIT.SOAatt			//xmm2 = cone-dot < SOAcos_theta_by_2 ? att*val : att
#endif //ATTENUATE
    movaps  [eax]_LIT.SOAatt, xmm2
#else
	movaps	xmm2, xmm3
#endif //SPOT

	mov		ecx, pRc
#if DO_AR || DO_AG || DO_AB
    test	[ecx]RC.tl.dwTLState, TLPV_COLORVERTEXAMB
	jz		LoadAmbientFromMaterial

    // amb += dot * La * AmbSrc
	mov		edx, [ecx]RC.tl.lighting.pSOACvAmbientSrc
#if DO_AR
	movaps	xmm1, [eax]_LIT.SOALa.red
#endif //DO_AR
#if DO_AG
	movaps	xmm3, [eax]_LIT.SOALa.green
#endif //DO_AG
#if DO_AB
	movaps	xmm4, [eax]_LIT.SOALa.blue
#endif //DO_AB
#if DO_AR                                            
	mulps	xmm1, [edx]SOA_RGB.red
#endif //DO_AR
#if DO_AG
	mulps	xmm3, [edx]SOA_RGB.green
#endif //DO_AG
#if DO_AB
	mulps	xmm4, [edx]SOA_RGB.blue
#endif //DO_AB
    mov     edx, pTL          // edx = pointer to the TL_SOATMP (reload)
	jmp		AmbientLoaded

LoadAmbientFromMaterial:
#if DO_AR
    movaps  xmm1, [eax]_LIT.SOAMa_La.red       
#endif //DO_AR
#if DO_AG
    movaps  xmm3, [eax]_LIT.SOAMa_La.green
#endif  //DO_AG
#if DO_AB
    movaps  xmm4, [eax]_LIT.SOAMa_La.blue
#endif //DO_AB

AmbientLoaded:
#endif //DO_AR || DO_AG || DO_AB

#if ATTENUATE
    andps   xmm2, [eax]_LIT.SOAwritemask
#if DO_AR
    mulps   xmm1, xmm2                      //xmm1 = Ma_La.red   * att
#endif  //DO_AR
#if DO_AG
    mulps   xmm3, xmm2                      //xmm3 = Ma_La.green * att
#endif  //DO_AG
#if DO_AB
    mulps   xmm4, xmm2                      //xmm4 = Ma_La.blue  * att
#endif  //DO_AB
#endif	//ATTENUATE

#if DO_AR
    addps   xmm5, xmm1                      //fSOADiffuseOut.red   += Ma_La.red   * att
#endif //DO_AR
#if DO_AG
    addps   xmm6, xmm3                      //fSOADiffuseOut.green += Ma_La.green * att
#endif //DO_AG
#if DO_AB
    addps   xmm7, xmm4                      //fSOADiffuseOut.blue  += Ma_La.blue  * att
#endif //DO_AB
        
  //
  // If no normals are present, bail out since we cannot perform the
  // normal-dependent computations
  //

//  DotProduct3SOA(&pL->d, &pTL->SOAcn);

  // if ((dwFVFIn & D3DFVF_NORMAL) == 0) return;


#if NORMALS
    movaps  xmm0, [eax]_LIT.SOAd.x
    movaps  xmm1, [eax]_LIT.SOAd.y
    movaps  xmm2, [eax]_LIT.SOAd.z
    mulps   xmm0, [edx]_STL.SOAcn.x
    mulps   xmm1, [edx]_STL.SOAcn.y
    mulps   xmm2, [edx]_STL.SOAcn.z
    addps   xmm0, xmm1
    addps   xmm0, xmm2				// xmm0 = dot(d, cn)


    mulps   xmm0, [eax]_LIT.SOAdist	// dot = dot * dist
    xorps   xmm1, xmm1
#if(SPOT == 0 && ATTENUATE == 0)
#else
    mulps   xmm0, [eax]_LIT.SOAatt   //dot = dot * dist * att
#endif

#if DO_R || DO_G || DO_B
    test	[ecx]RC.tl.dwTLState, TLPV_COLORVERTEXDIFF
#endif
    cmpltps xmm1, xmm0               // form mask -- is dot > 0?
    andps   xmm1, [eax]_LIT.SOAwritemask

#if DO_R || DO_G || DO_B
	jz		LoadDiffuseFromMaterial

	mov		edx, [ecx]RC.tl.lighting.pSOACvDiffuseSrc
#if DO_R
	movaps	xmm2, [eax]_LIT.SOALd.red
#endif //DO_R
#if DO_G
	movaps	xmm3, [eax]_LIT.SOALd.green
#endif //DO_G
#if DO_B
	movaps	xmm4, [eax]_LIT.SOALd.blue
#endif //DO_B

#if DO_R
	mulps	xmm2, [edx]SOA_RGB.red
#endif //DO_R
#if DO_G
	mulps	xmm3, [edx]SOA_RGB.green
#endif //DO_G
#if DO_B
	mulps	xmm4, [edx]SOA_RGB.blue
#endif //DO_G

    andps   xmm0, xmm1
    movmskps  ecx, xmm1       // see if all 4 dot products were < 0

#if DO_R
	mulps	xmm2, xmm0
    addps   xmm5, xmm2
#endif //DO_R
#if DO_G
	mulps	xmm3, xmm0
    addps   xmm6, xmm3
#endif //DO_G
#if DO_B
	mulps	xmm4, xmm0
    addps   xmm7, xmm4
#endif //DO_B
	jmp		DiffuseLoaded

LoadDiffuseFromMaterial:
#if DO_R
    movaps  xmm2, [eax]_LIT.SOAMd_Ld.red
#endif  //DO_R
#if DO_G
    movaps  xmm3, [eax]_LIT.SOAMd_Ld.green
#endif  //DO_G
#ifdef DO_B
    movaps  xmm4, [eax]_LIT.SOAMd_Ld.blue
#endif //DO_B

    andps   xmm0, xmm1
    movmskps  ecx, xmm1       // see if all 4 dot products were < 0

#if DO_R
    // if (dot > 0) pTL->fSOADiffuseOut.red   += pL->Md_Ld.red   * dot
    mulps   xmm2, xmm0          
    addps   xmm5, xmm2
#endif//DO_R

#if DO_G
    // if (dot > 0) pTL->fSOADiffuseOut.green += pL->Md_Ld.green * dot
    mulps   xmm3, xmm0
    addps   xmm6, xmm3
#endif //DO_G

#if DO_B
    // if (dot > 0) pTL->fSOADiffuseOut.blue  += pL->Md_Ld.blue  * dot
    mulps   xmm4, xmm0
    addps   xmm7, xmm4
#endif DO_B

DiffuseLoaded:
#endif //DO_R || DO_G || DO_B

#endif //NORMALS

    // Save off fSOADiffuseOut
#if SPOT
Spot_Diff_Bail:
#endif
    movaps  [eax]_LIT.SOAwritemask, xmm1   
    mov     [eax]_LIT.dwNonZeroDot, ecx 
  }
}


#undef NAME 
#undef DO_AR
#undef DO_AG
#undef DO_AB
#undef DO_R
#undef DO_G
#undef DO_B
#undef NORMALS
#undef SPOT
#undef ATTENUATE
#undef FALLOFF




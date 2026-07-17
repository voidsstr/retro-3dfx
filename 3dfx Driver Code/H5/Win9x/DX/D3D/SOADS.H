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
** File name: soads.h
**
** Description: SOA Lighting code -- Directional Specular
**
** $Revision: 5$
** $Date: 10/11/00 8:50:31 PM$
**
** $Log: 
**  5    3dfx      1.1.1.2     10/11/00 Brent           Forced check in to enforce
**       branching.
**  4    3dfx      1.1.1.1     08/29/00 Allen Hansen    added colorvertex support
**  3    3dfx      1.1.1.0     06/25/00 Allen Hansen    updated TL_SOATMP variable
**       name
**  2    Napalm    1.1         06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  1    Napalm    1.0         05/11/00 Scott Kephart   
** $
*/


// ***************************************************************************
// Directional Light - Specular
// ***************************************************************************
/*-------------------------------------------------------------------
Function Name:  DIR_SPEC_XXXX
Description:    Directional Lights -- Specular components
Parameters:   
                RC *pRC -- pointer to the current rendering context
                TLLIGHT *pL -- pointer to the current light

                xmm5 - RED component of the specular portion of the vertex
                xmm6 - GREEN component of the specular portion of the vertex
                xmm7 - BLUE component of the specular portion of the vertex

                pL->SOAwritemask = which SOA groups have non-zero diffuse lighting 
                components (used by specular pass, later)

                pL->dwNonZeroDot = 0 when all 4 dot products are zero. 
                (Allowing the remainder of lighting for this light
                on this vertext to be skipped.
Information:    
                This function is included in soalitfn.c. It will handle 
                complete calculation for the specular component of lighting
                for a vertex. The red, green, and blue color compoenents are
                accumulated in xmm5, xmm6, and xmm7. 

                The following defines control which portions of the lighting
                equation are handled by this function

                NAME - the name of the generated directional specular lighting 
                LOCAL_VIEWER - 1 = local lighting, 0 = infinite lighting
                DO_SR - Specular.red is non-zero
                DO_SG - Specular.green is non-zero
                DO_SB - Specular.blue is non-zero
                EXP0 - the specular exponent is == 0
                EXP_BIG - the specular exponent is very large, so it's worthwhile 
                to check the dot product to see if it's bigger than some threshold. 
                When the exponent is very large, this becomes a VERY probable occurrence.


Return:       
                xmm5 - RED component of the specular portion of the vertex
                xmm6 - GREEN component of the specular portion of the vertex
                xmm7 - BLUE component of the specular portion of the vertex

                pL->SOAwritemask = which SOA groups have non-zero diffuse lighting 
                components (used by specular pass, later)

                pL->dwNonZeroDot = 0 when all 4 dot products are zero. 
                (Allowing the remainder of lighting for this light
                on this vertext to be skipped.
-------------------------------------------------------------------*/

ALIGN32 void NAME(
    RC *pRc, 
    TLLIGHT *pL) 
{
  __asm  {
    mov     ecx, pRc
    mov     eax, pL             // eax = pointer to the light
	mov     edx, [ecx]RC.tl.pTL // edx = pointer to the TL_SOATMP

    // Assume non-local
    // eye = <0, 0, -1>
#if !LOCAL_VIEWER
    movaps  xmm2, TL_soa_neg_1
    xorps   xmm0, xmm0
    xorps   xmm1, xmm1
#else // not LOCAL_VIEWER
//  if (bDoLocalViewer) 

    // calc vector from vertex to the eye
    // SubtractVector( &LData->eye_in_eye, &in->dvPosition, &eye );
    // Normalize( &eye );
    movaps  xmm3, TL_hi_bits	// 80000000 (x4)
    movaps  xmm0, [edx]_STL.SOAcv.x
    movaps  xmm1, [edx]_STL.SOAcv.y
    movaps  xmm2, [edx]_STL.SOAcv.z

    xorps   xmm0, xmm3          // eye_in_eye is 0, so we flip sign bits
    xorps   xmm1, xmm3
    xorps   xmm2, xmm3

//  Normalize3SOA_reg();
    movaps  xmm3, xmm0
    movaps  xmm4, xmm1
    mulps   xmm3, xmm3          //x*x
    mulps   xmm4, xmm4          //y*y
    addps   xmm3, xmm4
    movaps  xmm4, xmm2
    mulps   xmm4, xmm4          //z*z
    addps   xmm3, xmm4          //x*x + y*y + z*z
    rsqrtps xmm3, xmm3          // 1.0 / sqrt(x*x + y*y + z*z)
    mulps   xmm0, xmm3          // normalize X
    mulps   xmm1, xmm3          // normalize Y
    mulps   xmm2, xmm3          // normalize Z
#endif //LOCAL_VIWER

    // calc halfway vector
    // AddVectorFromSOA( &pL->direction_in_eye, &eye, &h );
    // xmm0 - 2 = normalized eye vector
    addps  xmm0, [eax]_LIT.SOAdirection_in_eye.x
    addps  xmm1, [eax]_LIT.SOAdirection_in_eye.y
    addps  xmm2, [eax]_LIT.SOAdirection_in_eye.z

    // dot = DotProduct( &h, &in->dvNormal );
    // normalize
    // Normalize( &h );

    movaps  xmm3, xmm0
    movaps  xmm4, xmm1
    mulps   xmm3, xmm3          //x*x
    mulps   xmm4, xmm4          //y*y
    addps   xmm3, xmm4
    movaps  xmm4, xmm2
    mulps   xmm4, xmm4          //z*z
    addps   xmm3, xmm4          //x*x + y*y + z*z
    rsqrtps xmm3, xmm3          // 1.0 / sqrt(x*x + y*y + z*z)
    mulps   xmm0, xmm3          // normalize X
    mulps   xmm1, xmm3          // normalize Y
    mulps   xmm2, xmm3          // normalize Z

    // dot = DotProduct( &h, &in->dvNormal );
    
    mulps   xmm0, [edx]_STL.SOAcn.x
    mulps   xmm1, [edx]_STL.SOAcn.y
    mulps   xmm2, [edx]_STL.SOAcn.z
    addps   xmm0, xmm1
    addps   xmm0, xmm2        // xmm0 = dot

    xorps   xmm1, xmm1
    cmpltps xmm1, xmm0        // form mask -- is dot > 0?
    andps   xmm1, [eax]_LIT.SOAwritemask
    movmskps  edx, xmm1

    andps   xmm0, xmm1          // zero out dot products when they are <= 0
    test    edx, edx
    jz      NoDirectionalSpecular
    //D3DVALUE coeff = (D3DVALUE) pow( dot, LData->material.power );

#if EXP_BIG
    movaps  xmm2, xmm0
    cmpleps xmm2, [eax]_LIT.SOASpecThresh
    movmskps  ecx, xmm2
    cmp     ecx, 0xf
    jz      NoDirectionalSpecular

#endif
    mov     ecx, pRc

#if !EXP0
    mov     edx, [ecx]RC.tl.lighting.KniPowerFunction
    call    edx                 // call the pow() function
    mov     eax, pL             // reload eax = pointer to the light
#endif //!EXP0


    test	[ecx]RC.tl.dwTLState, TLPV_COLORVERTEXSPEC
	jz		LoadSpecularFromMaterial

    // LData->fSpecular.r += coeff * srcSpec.r * pL->Ls.red.f[0];
    // LData->fSpecular.g += coeff * srcSpec.g * pL->Ls.green.f[0];
    // LData->fSpecular.b += coeff * srcSpec.b * pL->Ls.blue.f[0];
	mov		edx, [ecx]RC.tl.lighting.pSOACvSpecularSrc
  #if DO_SR
	movaps	xmm1, [eax]_LIT.SOALs.red
  #endif //DO_SR
  #if DO_SG
	movaps	xmm2, [eax]_LIT.SOALs.green
  #endif //DO_SG
  #if DO_SB
	movaps	xmm3, [eax]_LIT.SOALs.blue
  #endif //DO_SB
  #if DO_SR
	mulps	xmm1, [edx]SOA_RGB.red
  #endif //DO_SR
  #if DO_SG
	mulps	xmm2, [edx]SOA_RGB.red
  #endif //DO_SG
  #if DO_SB
	mulps	xmm3, [edx]SOA_RGB.red
  #endif //DO_SB
#if (EXP0)
  #if DO_SR
    mulps   xmm1, xmm0
  #endif //DO_SR
  #if DO_SG
    mulps   xmm2, xmm0
  #endif //DO_SG
  #if DO_SB
    mulps   xmm3, xmm0
  #endif //DO_SB
#endif //!EXP0
  #if DO_SR
    addps   xmm5, xmm1
  #endif //DO_SR
  #if DO_SG
    addps   xmm6, xmm2
  #endif //DO_SG
  #if DO_SB
    addps   xmm7, xmm3
  #endif //DO_SB
	jmp		SpecularLoaded

LoadSpecularFromMaterial:
    // LData->fSpecular.r += coeff * pL->SOAMs_Ls.red.f[0];
    // LData->fSpecular.g += coeff * pL->SOAMs_Ls.green.f[0];
    // LData->fSpecular.b += coeff * pL->SOAMs_Ls.blue.f[0];
#if (EXP0)
  #if DO_SR
    addps   xmm5, [eax]_LIT.SOAMs_Ls.red
  #endif //DO_SR
  #if DO_SG
    addps   xmm6, [eax]_LIT.SOAMs_Ls.green
  #endif //DO_SG
  #if DO_SB
    addps   xmm7, [eax]_LIT.SOAMs_Ls.blue
  #endif //DO_SB
#else //EXP0
  #if DO_SR
    movaps  xmm1, [eax]_LIT.SOAMs_Ls.red
  #endif //DO_SR
  #if DO_SG
    movaps  xmm2, [eax]_LIT.SOAMs_Ls.green
  #endif //DO_SG
  #if DO_SB
    movaps  xmm3, [eax]_LIT.SOAMs_Ls.blue
  #endif //DO_SB

  #if DO_SR
    mulps   xmm1, xmm0
  #endif //DO_SR
  #if DO_SG
    mulps   xmm2, xmm0
  #endif //DO_SG
  #if DO_SB
    mulps   xmm3, xmm0
  #endif //DO_SB

  #if DO_SR
    addps   xmm5, xmm1
  #endif //DO_SR
  #if DO_SG
    addps   xmm6, xmm2
  #endif //DO_SG
  #if DO_SB
    addps   xmm7, xmm3
  #endif //DO_SB
#endif //!EXP0
SpecularLoaded:

NoDirectionalSpecular:
  }
}


#undef NAME
#undef DO_SR
#undef DO_SG
#undef DO_SB
#undef LOCAL_VIEWER
#undef EXP0
#undef EXP_BIG
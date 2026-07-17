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
** File name: soadd.h
**
** Description: SOA Lighting code -- Directional Diffuse 
**
** $Revision: 6$
** $Date: 10/11/00 8:50:31 PM$
**
** $Log: 
**  6    3dfx      1.1.1.3     10/11/00 Brent           Forced check in to enforce
**       branching.
**  5    3dfx      1.1.1.2     09/04/00 Allen Hansen    colorvertex bug, caught in
**       whql stateblock tests
**  4    3dfx      1.1.1.1     08/29/00 Allen Hansen    added colorvertex support
**  3    3dfx      1.1.1.0     06/25/00 Allen Hansen    updated TL_SOATMP variable
**       name
**  2    Napalm    1.1         06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  1    Napalm    1.0         05/11/00 Scott Kephart   
** $
*/


// ***************************************************************************
// Directional Light - Diffuse
// ***************************************************************************
/*-------------------------------------------------------------------
Function Name:  DIR_DIFF_XXXX
Description:    Directional Lights -- Diffuse components
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
-------------------------------------------------------------------*/


ALIGN32 void NAME(
    RC *pRc, 
    TLLIGHT *pL) 
{
  __asm
  {
    mov     ecx, pRc
    mov     eax, pL           // eax = pointer to the light
#if DO_AR || DO_AG || DO_AB
    test	[ecx]RC.tl.dwTLState, TLPV_COLORVERTEXAMB
#endif

#if DO_AR || DO_AG || DO_AB
	jz		LoadAmbientFromMaterial
	mov		edx, [ecx]RC.tl.lighting.pSOACvAmbientSrc
#if DO_AR                                            
	movaps	xmm0, [eax]_LIT.SOALa.red
#endif //DO_AR
#if DO_AG
	movaps	xmm1, [eax]_LIT.SOALa.green
#endif //DO_AG
#if DO_AB
	movaps	xmm2, [eax]_LIT.SOALa.blue
#endif //DO_AB
#if DO_AR                                            
	mulps	xmm0, [edx]SOA_RGB.red
#endif //DO_AR
#if DO_AG
	mulps	xmm1, [edx]SOA_RGB.green
#endif //DO_AG
#if DO_AB
	mulps	xmm2, [edx]SOA_RGB.blue
#endif //DO_AB
#if DO_AR                                            
	addps	xmm5, xmm0						   // pTL->fSOADiffuseOut.red   += pTL->SOALa.red * ambsrc.red
#endif //DO_AR
#if DO_AG
	addps	xmm6, xmm1						   // pTL->fSOADiffuseOut.green += pTL->SOALa.red * ambsrc.green
#endif //DO_AG
#if DO_AB
	addps	xmm7, xmm2						   // pTL->fSOADiffuseOut.blue  += pTL->SOALa.red * ambsrc.blue
#endif //DO_AB

	jmp		AmbientLoaded
LoadAmbientFromMaterial:
#if DO_AR                                            
    addps   xmm5, [eax]_LIT.SOAMa_La.red       // pTL->fSOADiffuseOut.red   += pL->Ma_La.red
#endif //DO_AR
#if DO_AG
    addps   xmm6, [eax]_LIT.SOAMa_La.green     // pTL->fSOADiffuseOut.green += pL->Ma_La.green
#endif //DO_AG
#if DO_AB
    addps   xmm7, [eax]_LIT.SOAMa_La.blue      // pTL->fSOADiffuseOut.blue  += pL->Ma_La.blue
#endif //DO_AB
AmbientLoaded:

#endif // DO_AR || DO_AG || DO_AB

	mov     edx, [ecx]RC.tl.pTL // edx = pointer to the TL_SOATMP
  //
  // If no normals are present, bail out since we cannot perform the
  // normal-dependent computations
  //
  // if ((dwFVFIn & D3DFVF_NORMAL) == 0) return;

#if NORMALS
//  DotProduct3SOA(&pL->direction_in_eye, &pTL->SOAcn);
    movaps  xmm0, [eax]_LIT.SOAdirection_in_eye.x
    movaps  xmm1, [eax]_LIT.SOAdirection_in_eye.y
    movaps  xmm2, [eax]_LIT.SOAdirection_in_eye.z
    mulps   xmm0, [edx]_STL.SOAcn.x
    mulps   xmm1, [edx]_STL.SOAcn.y
    addps   xmm0, xmm1
    mulps   xmm2, [edx]_STL.SOAcn.z
    addps   xmm0, xmm2
#if DO_R || DO_G || DO_B
    test	[ecx]RC.tl.dwTLState, TLPV_COLORVERTEXDIFF
#endif

    xorps   xmm1, xmm1
    cmpltps xmm1, xmm0     // form mask -- is dot > 0?

#if DO_R || DO_G || DO_B
	jz		LoadDiffuseFromMaterial

    // diff += dot * Ld * DiffSrc
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

    andps   xmm0, xmm1

#if DO_R
	mulps	xmm2, [edx]SOA_RGB.red
#endif //DO_R
#if DO_G
	mulps	xmm3, [edx]SOA_RGB.green
#endif //DO_G
#if DO_B
	mulps	xmm4, [edx]SOA_RGB.blue
#endif //DO_G
#if DO_R
	mulps	xmm2, xmm0
#endif //DO_R
#if DO_G
	mulps	xmm3, xmm0
#endif //DO_G
#if DO_B
	mulps	xmm4, xmm0
#endif //DO_B

#endif //DO_R || DO_G || DO_B
    movaps  [eax]_LIT.SOAwritemask, xmm1
    movmskps  ecx, xmm1       // see if all 4 dot products were < 0
#if DO_R || DO_G || DO_B

#if DO_R
	addps	xmm5, xmm2						   // pTL->fSOADiffuseOut.red   += dot * pTL->SOALd.red * diffsrc.red
#endif //DO_R
#if DO_G
	addps	xmm6, xmm3						   // pTL->fSOADiffuseOut.green += dot * pTL->SOALd.red * diffsrc.green
#endif //DO_G
#if DO_B
	addps	xmm7, xmm4						   // pTL->fSOADiffuseOut.blue  += dot * pTL->SOALd.red * diffsrc.blue
#endif //DO_B
	jmp		DiffuseLoaded

LoadDiffuseFromMaterial:
#if DO_R
    movaps  xmm2, [eax]_LIT.SOAMd_Ld.red
#endif //DO_R
#if DO_G
    movaps  xmm3, [eax]_LIT.SOAMd_Ld.green
#endif //DO_G
#if DO_B
    movaps  xmm4, [eax]_LIT.SOAMd_Ld.blue
#endif //DO_B

    andps   xmm0, xmm1

#endif //DO_R || DO_G || DO_B
    movaps  [eax]_LIT.SOAwritemask, xmm1
    movmskps  ecx, xmm1       // see if all 4 dot products were < 0
#if DO_R || DO_G || DO_B

#if DO_R
    // if (dot > 0) pTL->fSOADiffuseOut.red += pL->Md_Ld.red * dot
    mulps   xmm2, xmm0          
    addps   xmm5, xmm2
#endif //DO_R

#if DO_G
    // if (dot > 0) pTL->fSOADiffuseOut.green += pL->Md_Ld.green * dot
    mulps   xmm3, xmm0
    addps   xmm6, xmm3
#endif

#if DO_B
    // if (dot > 0) pTL->fSOADiffuseOut.blue += pL->Md_Ld.blue * dot
    mulps   xmm4, xmm0
    addps   xmm7, xmm4
#endif //DO_B

DiffuseLoaded:
#endif // DO_R || DO_G || DO_B
    mov     [eax]_LIT.dwNonZeroDot, ecx

    
#endif  //NORMALS
  }
}

#undef NAME
#undef NORMALS
#undef DO_AR
#undef DO_AG
#undef DO_AB
#undef DO_R
#undef DO_G
#undef DO_B
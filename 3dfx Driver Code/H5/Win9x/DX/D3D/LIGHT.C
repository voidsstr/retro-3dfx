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
** File name: light.c
**
** Description: Main body of lighting code for T&L
**
** $Revision: 38$
** $Date: 10/18/00 5:43:20 PM$
**
** $Log: 
**  38   3dfx      1.17.1.2.1.1610/18/00 Allen Hansen    Optimized when T&L
**       parameters (matricies, material, lights, etc) are dirty, now we don't
**       reload these unless they actually change.  This saves the processing of
**       the dirty params.
**  37   3dfx      1.17.1.2.1.1510/11/00 Brent           Forced check in to enforce
**       branching.
**  36   3dfx      1.17.1.2.1.1410/03/00 Allen Hansen    optimization - change
**       unsigned to signed before doing float to int conversions, changed a few
**       doubles to floats
**  35   3dfx      1.17.1.2.1.1309/23/00 Allen Hansen    went from 16 to 32 max
**       active lights so match Sage
**  34   3dfx      1.17.1.2.1.1209/23/00 Allen Hansen    general code cleanup -
**       deleted unused variables and long-unused functions
**  33   3dfx      1.17.1.2.1.1109/13/00 Allen Hansen    fixed bug in the 3DNow
**       path where we only precalculated p8AmbEmissPlusDiffAlpha if the material
**       was dirty, this must also be done anytime any light changes
**  32   3dfx      1.17.1.2.1.1009/02/00 Allen Hansen    added SSE2 path to
**       converting lights from SOA_FLOATS to packed SOA_DWORDS
**  31   3dfx      1.17.1.2.1.908/31/00 Allen Hansen    check for colorvertex
**       before loading colorvertex values, loaded K3D immediate light values with
**       3dnow instuctions
**  30   3dfx      1.17.1.2.1.808/29/00 Allen Hansen    added colorvertex support
**       for sse path, code cleanup in UpdateLightingData()
**  29   3dfx      1.17.1.2.1.708/27/00 Allen Hansen    added colorvertex support
**       for 3dnow path, added 8 more lights to the slow path (now 16 like the fast
**       path)
**  28   3dfx      1.17.1.2.1.608/23/00 Allen Hansen    fastpath colorvertex work
** 
**  27   3dfx      1.17.1.2.1.508/21/00 Allen Hansen    always clear out ambient
**       alpha component (K3D path), added check for negitive cos_phi_by_2 and
**       cos_theta_by_2 > cos_phi_by_2 (refrast didn't do this, caused a whql
**       failure), support up to TLMAX_ACTIVE_LIGHTS, beyond that we drop lights
**       instead of crashing
**  26   3dfx      1.17.1.2.1.407/24/00 Allen Hansen    whql bug fixes (T&L tests)
**  25   3dfx      1.17.1.2.1.307/21/00 Allen Hansen    fixed whql bug, spec alpha
**       comes from diff alpha (K3D only)
**  24   3dfx      1.17.1.2.1.207/03/00 Allen Hansen    added support for 3dnow
**       special cased asm functions
**  23   3dfx      1.17.1.2.1.106/29/00 Allen Hansen    enabled K3dPowerFunction
**  22   3dfx      1.17.1.2.1.006/25/00 Allen Hansen    slow path now uses correct
**       variables instead of SOA variables, load 3dnow path variables
**  21   3dfx      1.17.1.2    06/04/00 Allen Hansen    Made changes so this will
**       work on non-kni cpu's, mainly disabled SqrtSS and put checks before
**       setting up lighting functions.  Also made a change in UpdateLightingData()
**       where the material power function is setup ... the branches looked
**       backwards and I'm still think it's wrong ... need to figure out what Scott
**       was trying to do here (I documented the change and commented out the old
**       code)
**  20   3dfx      1.17.1.1    06/01/00 Allen Hansen    added "SOA" to all SOA
**       variables
**  19   3dfx      1.17.1.0    05/11/00 Scott Kephart   More optimizations for
**       lighting
**  18   Napalm Shared1.17        04/19/00 Scott Kephart   Begining of optimized
**       spot lights
**  17   Napalm Shared1.16        04/19/00 Scott Kephart   Big lighting change -
**       Part I
**       Lighting is now split into two parts, diffuse and specular. 
**  16   Napalm Shared1.15        03/23/00 Bob Johnston    Scott and Bob's changes
**       to split up the tranformation and lighting in the vertex processing loop
**       for improved VB primitive perfromance.
**  15   Napalm Shared1.14        03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  14   Napalm Shared1.13        03/14/00 Scott Kephart   Fixed numerous specular
**       alpha bugs
**  13   Napalm Shared1.12        02/23/00 Scott Kephart   Specular now works for
**       directional and point lights
**  12   Napalm Shared1.11        02/14/00 Scott Kephart   Cleanup of SOA light
**       structures
**  11   Napalm Shared1.10        02/14/00 Scott Kephart   Changes for fastpath
**       lighting. Totally restructured the TLLIGHT structure so that the slowpath
**       and fastpath could share the same structure. The slowpath uses the zeroth
**       element in the SOA array.
**  10   Napalm Shared1.9         02/10/00 Scott Kephart   Data structure cleanup
**       for SOA.H -- we're unionized now!
**  9    Napalm Shared1.8         02/07/00 Scott Kephart   Fixed SOA light function
**       initialization bug that caused 3D WB 2K to crash
**  8    Napalm Shared1.7         02/01/00 Scott Kephart   More lighting changes.
**       Better SSE matrix multiply code.
**  7    Napalm Shared1.6         01/28/00 Scott Kephart   Big T&L Merge: Changes
**       for SSE lighting code
**  6    Napalm Shared1.5         01/10/00 Scott Kephart   Updates to lighting.
**       Split point and spot light functions. Only allow 8 active lights to reduce
**       branch mispredictions. Cleanup in lighting.
**  5    Napalm Shared1.4         12/14/99 Scott Kephart   Partial fix for spot
**       light problems
**  4    Napalm Shared1.3         12/13/99 Scott Kephart   Big T&L Update:
**       1. Improved T&L profiling code
**       2. Optimizations to scalar transformation and lighting code
**       3. Changes to the vertex buffer code to allow functionality under Windows
**       2000.
**  3    Napalm Shared1.2         11/10/99 Scott Kephart   Another tweak for
**       handling zero attenuation in point and spot lights
**  2    Napalm Shared1.1         11/10/99 Scott Kephart   Fix for 3D Winbench 2000
**       Lighting quality test -- the reference rasterizer clamped all attenuation
**       < 1.0 to 1.0 for point and spot lights. The attenuation should have been
**       clamped to 1.0 only if it's zero...
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
** 
** 5     3/10/00 4:38p Skephart
** Create float version of material alpha for SOA
** 
** 4     3/08/00 9:24p Skephart
 * 
 * 7     1/25/00 10:16p Skephart
 * BobJ Merge
 * 
 * 5     1/24/00 4:13p Skephart
 * Beginnings of the fast path
 * 
 * 4     1/19/00 9:50p Skephart
 * Add and Init SOA lighting function stubs. Allocate TL_SOATMP structure
 * 
 * 3     1/19/00 2:24p Skephart
 * Bob's FVF_COMP changes part II
** 
** 11    10/26/99 12:51a Skephart
** Added #ifdef TnL_HAL
** 
** 10    9/27/99 8:32p Skephart
** Move render context variables for TL HAL out of d3global.h, 
** and into tlglobal.h. All TL related variables are gathered under 
** pRc->tl.<variable>
** 
** 9     9/27/99 12:03p Skephart
** Fixed call to pfnLightVertex function -- it was passing in the
** glighting struct, when it needed to pass in &glighting.
** 
** 8     9/16/99 10:57a Skephart
** Minor cleanup
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

#ifdef __MAT_PROFILE
    static float power[200] = 
    {
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, 
      -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0 
    };
    static int count[200] = 
    {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    };
    static int num_pow = 0;
    static int printpow = 0;
#endif


__inline BOOL LightNeedsProcessing(TLLIGHT *pL) {
    return (pL->dwFlags & TLLIGHT_NEEDSPROCESSING);
}

///////////////////////////////////////////////////////////////////////////////
// Vertex Lighting function implementations
///////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------
ALIGN32 void TLLV_Directional(
    RC *pRc, 
    TLLIGHT *pL, 
    D3DLIGHTINGELEMENT *in)
{
    // BUGBUG: Need to heed the specular flag set per light here!!
    TLLIGHTING *LData = &pRc->tl.lighting;
    DWORD dwFlags = pRc->tl.dwTLState;
    DWORD dwFVFIn = (DWORD)pRc->tl.InFVF.dwFVFType;
    BOOL bDoSpecular = dwFlags & TLPV_DOSPECULAR;
    BOOL bDoLocalViewer = dwFlags & TLPV_LOCALVIEWER;
    BOOL bDoColVertexAmbient = dwFlags & TLPV_COLORVERTEXAMB;
    BOOL bDoColVertexDiffuse = dwFlags & TLPV_COLORVERTEXDIFF;
    BOOL bDoColVertexSpecular = dwFlags & TLPV_COLORVERTEXSPEC;
    ALIGN16 D3DVALUE dot;

    //
    // Add the material's ambient component
    //
    if (!bDoColVertexAmbient)
    {
        LData->fDiffuse.r += pL->Ma_La.r;
        LData->fDiffuse.g += pL->Ma_La.g;
        LData->fDiffuse.b += pL->Ma_La.b;
    }
    else
    {
        //
        // Note:
        // In case ColorVertexAmbient is enabled, note that it uses
        // VertexSpecular instead of VertexDiffuse
        //
        LData->fDiffuse.r += pL->La.r * LData->pAmbientSrc->r;
        LData->fDiffuse.g += pL->La.g * LData->pAmbientSrc->g;
        LData->fDiffuse.b += pL->La.b * LData->pAmbientSrc->b;
    }

    //
    // If no normals are present, bail out since we cannot perform the
    // normal-dependent computations
    //
    if( (dwFVFIn & D3DFVF_NORMAL) == 0 )
    {
        return;
    }

    dot = DotProductToSOA( &pL->SOAdirection_in_eye, &in->dvNormal );
    if (FLOAT_GTZ(dot))
    {
        if (!bDoColVertexDiffuse)
        {
            LData->fDiffuse.r += pL->Md_Ld.r * dot;
            LData->fDiffuse.g += pL->Md_Ld.g * dot;
            LData->fDiffuse.b += pL->Md_Ld.b * dot;
        }
        else
        {
            LData->fDiffuse.r += pL->Ld.r * LData->pDiffuseSrc->r * dot;
            LData->fDiffuse.g += pL->Ld.g * LData->pDiffuseSrc->g * dot;
            LData->fDiffuse.b += pL->Ld.b * LData->pDiffuseSrc->b * dot;
        }

        if (bDoSpecular)
        {
            D3DVECTOR h;      // halfway vector
            D3DVECTOR eye;    // incident vector ie vector from eye

            if (bDoLocalViewer)
            {
                // calc vector from vertex to the eye
                SubtractVector( &LData->eye_in_eye, &in->dvPosition, &eye );

                // normalize
                Normalize( &eye );
            }
            else
            {
                eye.x = (D3DVALUE) 0.0 ;
                eye.y = (D3DVALUE) 0.0 ;
                eye.z = (D3DVALUE) -1.0 ;
            }

            // calc halfway vector
            AddVectorFromSOA( &pL->SOAdirection_in_eye, &eye, &h );

            // normalize
            Normalize( &h );

            dot = DotProduct( &h, &in->dvNormal );

            if (FLOAT_GTZ(dot))
            {
                if (FLOAT_CMP_POS(dot, >=, LData->specThreshold))
                {
                    //D3DVALUE coeff = dot/( pRc->tl.lighting.Material.power - (pRc->tl.lighting.Material.power*dot) +dot);
                    D3DVALUE coeff = (D3DVALUE) pow( dot, pRc->tl.lighting.Material.power );
                    if (!bDoColVertexSpecular)
                    {
                        LData->fSpecular.r += pL->Ms_Ls.r * coeff;
                        LData->fSpecular.g += pL->Ms_Ls.g * coeff;
                        LData->fSpecular.b += pL->Ms_Ls.b * coeff;
                    }
                    else
                    {
                        LData->fSpecular.r += (pL->Ls.r * LData->pSpecularSrc->r * coeff);
                        LData->fSpecular.g += (pL->Ls.g * LData->pSpecularSrc->g * coeff);
                        LData->fSpecular.b += (pL->Ls.b * LData->pSpecularSrc->b * coeff);
                    }
                }
            }
        }
    }
    return;
}

ALIGN32 void TLLV_Point(
    RC *pRc, 
    TLLIGHT *pL, 
    D3DLIGHTINGELEMENT *in)
{
    // BUGBUG: Need to heed the specular flag set per light here!!
    TLLIGHTING *LData = &pRc->tl.lighting;
    DWORD dwFlags = pRc->tl.dwTLState;
    DWORD dwFVFIn = (DWORD)pRc->tl.InFVF.dwFVFType;
    BOOL bDoSpecular = dwFlags & TLPV_DOSPECULAR;
    BOOL bDoLocalViewer = dwFlags & TLPV_LOCALVIEWER;
    BOOL bDoColVertexAmbient = dwFlags & TLPV_COLORVERTEXAMB;
    BOOL bDoColVertexDiffuse = dwFlags & TLPV_COLORVERTEXDIFF;
    BOOL bDoColVertexSpecular = dwFlags & TLPV_COLORVERTEXSPEC;
    ALIGN16 D3DVECTOR d;    // Direction to light
    D3DVALUE att;
    D3DVALUE dist;
    D3DVALUE dot;
    D3DVALUE distSquared;
    
    SubtractVectorFromSOA( &pL->SOAposition_in_eye, &in->dvPosition, (D3DVECTOR *)&d );

    // early out if out of range or exactly on the vertex
    distSquared = SquareMagnitude( &d );
    if (FLOAT_CMP_POS(distSquared, >=, pL->range_squared) ||
        FLOAT_EQZ(distSquared))
    {
        return;
    }

    //
    // Compute the attenuation
    //
    dist = SQRTF( distSquared );

    att = pL->Attenuation0 + pL->Attenuation1 * dist + pL->Attenuation2 * distSquared;

    if (FLOAT_EQZ(att))
      att = (D3DVALUE) FLT_MAX;
    else
      att = (D3DVALUE)1.0/att;
    
    dist = D3DVAL(1)/dist;

    //
    // Add the material's ambient component
    //
    if (!bDoColVertexAmbient)
    {
        LData->fDiffuse.r += att*pL->Ma_La.r;
        LData->fDiffuse.g += att*pL->Ma_La.g;
        LData->fDiffuse.b += att*pL->Ma_La.b;
    }
    else
    {
        //
        // Note:
        // In case ColorVertexAmbient is enabled, note that it uses
        // VertexSpecular instead of VertexDiffuse
        //
        LData->fDiffuse.r += att*pL->La.r * LData->pAmbientSrc->r;
        LData->fDiffuse.g += att*pL->La.g * LData->pAmbientSrc->g;
        LData->fDiffuse.b += att*pL->La.b * LData->pAmbientSrc->b;
    }

    // Calc dot product of light dir with normal.  Note that since we
    // didn't normalize the direction the result is scaled by the distance.
    if( (dwFVFIn & D3DFVF_NORMAL) == 0)
    {
        // If no normals are present, bail out since we cannot perform the
        // normal-dependent computations
        return;
    }
    else
    {
        dot = DotProduct(&d, &in->dvNormal );
    }

    if (FLOAT_GTZ( dot ))
    {
        dot *= dist*att;

        if (!bDoColVertexDiffuse)
        {
            LData->fDiffuse.r += pL->Md_Ld.r * dot;
            LData->fDiffuse.g += pL->Md_Ld.g * dot;
            LData->fDiffuse.b += pL->Md_Ld.b * dot;
        }
        else
        {
            LData->fDiffuse.r += pL->Ld.r * LData->pDiffuseSrc->r * dot;
            LData->fDiffuse.g += pL->Ld.g * LData->pDiffuseSrc->g * dot;
            LData->fDiffuse.b += pL->Ld.b * LData->pDiffuseSrc->b * dot;
        }

        if (bDoSpecular)
        {
            D3DVECTOR h;      // halfway vector
            D3DVECTOR eye;    // incident vector ie vector from eye

            // normalize light direction
            d.x *= dist;
            d.y *= dist;
            d.z *= dist;

            if (bDoLocalViewer)
            {
                // calc vector from vertex to the eye
                SubtractVector( &LData->eye_in_eye, &in->dvPosition, &eye );

                // normalize
                Normalize( &eye );
            }
            else
            {
                eye.x = (D3DVALUE) 0.0 ;
                eye.y = (D3DVALUE) 0.0 ;
                eye.z = (D3DVALUE)-1.0 ;
            }

            // calc halfway vector
            AddVector( &d, &eye, &h );
            Normalize( &h );

            dot = DotProduct( &h, &in->dvNormal );

            if (FLOAT_CMP_POS(dot, >=, LData->specThreshold))
            {
                //D3DVALUE coeff =  dot/(pRc->tl.lighting.Material.power - (pRc->tl.lighting.Material.power*dot)+dot ) * att;
                D3DVALUE coeff = (D3DVALUE) pow( dot, pRc->tl.lighting.Material.power ) * att;
                if (!bDoColVertexSpecular)
                {
                    LData->fSpecular.r += pL->Ms_Ls.r * coeff;
                    LData->fSpecular.g += pL->Ms_Ls.g * coeff;
                    LData->fSpecular.b += pL->Ms_Ls.b * coeff;
                }
                else
                {
                    LData->fSpecular.r += (pL->Ls.r * LData->pSpecularSrc->r * coeff);
                    LData->fSpecular.g += (pL->Ls.g * LData->pSpecularSrc->g * coeff);
                    LData->fSpecular.b += (pL->Ls.b * LData->pSpecularSrc->b * coeff);
                }
            }
        }
    }
    return;
}

ALIGN32 void TLLV_Spot(
    RC *pRc, 
    TLLIGHT *pL, 
    D3DLIGHTINGELEMENT *in)
{
    // BUGBUG: Need to heed the specular flag set per light here!!
    TLLIGHTING *LData = &pRc->tl.lighting;
    DWORD dwFlags = pRc->tl.dwTLState;
    DWORD dwFVFIn = (DWORD)pRc->tl.InFVF.dwFVFType;
    BOOL bDoSpecular = dwFlags & TLPV_DOSPECULAR;
    BOOL bDoLocalViewer = dwFlags & TLPV_LOCALVIEWER;
    BOOL bDoColVertexAmbient = dwFlags & TLPV_COLORVERTEXAMB;
    BOOL bDoColVertexDiffuse = dwFlags & TLPV_COLORVERTEXDIFF;
    BOOL bDoColVertexSpecular = dwFlags & TLPV_COLORVERTEXSPEC;
    ALIGN16 D3DVECTOR d;    // Direction to light
    D3DVALUE att;
    D3DVALUE dist;
    D3DVALUE dot;
    D3DVALUE distSquared;
    D3DVALUE cone_dot;    

    SubtractVectorFromSOA( &pL->SOAposition_in_eye, &in->dvPosition, (D3DVECTOR *)&d );

    // early out if out of range or exactly on the vertex
    distSquared = SquareMagnitude( &d );
    if (FLOAT_CMP_POS(distSquared, >=, pL->range_squared) ||
        FLOAT_EQZ(distSquared))
    {
        return;
    }

    //
    // Compute the attenuation
    //
    dist = SQRTF( distSquared );

    att = pL->Attenuation0 + pL->Attenuation1 * dist + pL->Attenuation2 * distSquared;

    if (FLOAT_EQZ(att))
      att = (D3DVALUE) FLT_MAX;
    else
      att = (D3DVALUE)1.0/att;
    
    dist = D3DVAL(1)/dist;
    
    // Calc dot product of direction to light with light direction to
    // be compared anganst the cone angles to see if we are in the
    // light.
    // Note that cone_dot is still scaled by dist
    cone_dot = DotProductToSOA2(&d, &pL->SOAdirection_in_eye) * dist;
    
    if (FLOAT_CMP_POS(cone_dot, <=, pL->cos_phi_by_2))
    {
        return;
    }
    
    // modify att if in the region between phi and theta
    if (FLOAT_CMP_POS(cone_dot, <, pL->cos_theta_by_2))
    {
        D3DVALUE val = (cone_dot - pL->cos_phi_by_2) * pL->inv_theta_minus_phi;
        
        if (!FLOAT_EQZ( pL->Falloff - (float) 1.0 ))
        {
            //val = val/(pLight->dvFalloff - (pLight->dvFalloff*val) + val );
			val = POWF( val, pL->Falloff );
        }
        att *= val;
    }

    //
    // Add the material's ambient component
    //
    if (!bDoColVertexAmbient)
    {
        LData->fDiffuse.r += att*pL->Ma_La.r;
        LData->fDiffuse.g += att*pL->Ma_La.g;
        LData->fDiffuse.b += att*pL->Ma_La.b;
    }
    else
    {
        //
        // Note:
        // In case ColorVertexAmbient is enabled, note that it uses
        // VertexSpecular instead of VertexDiffuse
        //
        LData->fDiffuse.r += att*pL->La.r * LData->pAmbientSrc->r;
        LData->fDiffuse.g += att*pL->La.g * LData->pAmbientSrc->g;
        LData->fDiffuse.b += att*pL->La.b * LData->pAmbientSrc->b;
    }

    // Calc dot product of light dir with normal.  Note that since we
    // didn't normalize the direction the result is scaled by the distance.
    if( (dwFVFIn & D3DFVF_NORMAL) == 0)
    {
        // If no normals are present, bail out since we cannot perform the
        // normal-dependent computations
        return;
    }
    else
    {
        dot = DotProduct(&d, &in->dvNormal );
    }

    if (FLOAT_GTZ( dot ))
    {
        dot *= dist*att;

        if (!bDoColVertexDiffuse)
        {
            LData->fDiffuse.r += pL->Md_Ld.r * dot;
            LData->fDiffuse.g += pL->Md_Ld.g * dot;
            LData->fDiffuse.b += pL->Md_Ld.b * dot;
        }
        else
        {
            LData->fDiffuse.r += pL->Ld.r * LData->pDiffuseSrc->r * dot;
            LData->fDiffuse.g += pL->Ld.g * LData->pDiffuseSrc->g * dot;
            LData->fDiffuse.b += pL->Ld.b * LData->pDiffuseSrc->b * dot;
        }

        if (bDoSpecular)
        {
            D3DVECTOR h;      // halfway vector
            D3DVECTOR eye;    // incident vector ie vector from eye

            // normalize light direction
            d.x *= dist;
            d.y *= dist;
            d.z *= dist;

            if (bDoLocalViewer)
            {
                // calc vector from vertex to the eye
                SubtractVector( &LData->eye_in_eye, &in->dvPosition, &eye );

                // normalize
                Normalize( &eye );
            }
            else
            {
                eye.x = (D3DVALUE) 0.0 ;
                eye.y = (D3DVALUE) 0.0 ;
                eye.z = (D3DVALUE)-1.0 ;
            }

            // calc halfway vector
            AddVector( &d, &eye, &h );
            Normalize( &h );

            dot = DotProduct( &h, &in->dvNormal );

            if (FLOAT_CMP_POS(dot, >=, LData->specThreshold))
            {
                //D3DVALUE coeff =  dot/(pRc->tl.lighting.Material.power - (pRc->tl.lighting.Material.power*dot)+dot ) * att;
                D3DVALUE coeff = (D3DVALUE) pow( dot, pRc->tl.lighting.Material.power ) * att;
                if (!bDoColVertexSpecular)
                {
                    LData->fSpecular.r += pL->Ms_Ls.r * coeff;
                    LData->fSpecular.g += pL->Ms_Ls.g * coeff;
                    LData->fSpecular.b += pL->Ms_Ls.b * coeff;
                }
                else
                {
                    LData->fSpecular.r += (pL->Ls.r * LData->pSpecularSrc->r * coeff);
                    LData->fSpecular.g += (pL->Ls.g * LData->pSpecularSrc->g * coeff);
                    LData->fSpecular.b += (pL->Ls.b * LData->pSpecularSrc->b * coeff);
                }
            }
        }
    }
    return;
}

#if 0 //unused
void TLLV_PointAndSpot(
    RC *pRc, 
    TLLIGHT *pL, 
    D3DLIGHTINGELEMENT *in)
{
    // BUGBUG: Need to heed the specular flag set per light here!!
    TLLIGHTING *LData = &pRc->tl.lighting;
    DWORD dwFlags = pRc->tl.dwTLState;
    DWORD dwFVFIn = (DWORD)pRc->tl.InFVF.dwFVFType;
    BOOL bDoSpecular = dwFlags & TLPV_DOSPECULAR;
    BOOL bDoLocalViewer = dwFlags & TLPV_LOCALVIEWER;
    BOOL bDoColVertexAmbient = dwFlags & TLPV_COLORVERTEXAMB;
    BOOL bDoColVertexDiffuse = dwFlags & TLPV_COLORVERTEXDIFF;
    BOOL bDoColVertexSpecular = dwFlags & TLPV_COLORVERTEXSPEC;
#ifdef VCPP
    __declspec(align(16)) D3DVECTOR d;    // Direction to light
#else
    D3DVECTOR d;    // Direction to light
#endif
    D3DVALUE att;
    D3DVALUE dist;
    D3DVALUE dot;
    D3DVALUE distSquared;
    
    SubtractVectorFromSOA( &pL->SOAposition_in_eye, &in->dvPosition, (D3DVECTOR *)&d );  // d = SOAposition_in_eye - dvPosition

    // early out if out of range or exactly on the vertex

    distSquared = SquareMagnitude( &d );
    if (FLOAT_CMP_POS(distSquared, >=, pL->range_squared) ||
        FLOAT_EQZ(distSquared))
    {
        return;
    }

    //
    // Compute the attenuation
    //
#ifdef VCPP
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
      SqrtSS( &dist, &distSquared );
    else
#endif
      dist = SQRTF( distSquared );

    att = pL->Attenuation0 + pL->Attenuation1 * dist + pL->Attenuation2 * distSquared;

    if (FLOAT_EQZ(att))
      att = (D3DVALUE) FLT_MAX;
    else
      att = (D3DVALUE)1.0/att;
    
    dist = D3DVAL(1)/dist;
    
    //
    // If the light is a spotlight compute the spot-light factor
    //
    if (pL->dltType == D3DLIGHT_SPOT)
    {
        // Calc dot product of direction to light with light direction to
        // be compared anganst the cone angles to see if we are in the
        // light.
        // Note that cone_dot is still scaled by dist
        D3DVALUE cone_dot = DotProductToSOA2(&d, &pL->SOAdirection_in_eye) * dist;
        
        if (FLOAT_CMP_POS(cone_dot, <=, pL->cos_phi_by_2))
        {
            return;
        }
        
        // modify att if in the region between phi and theta
        if (FLOAT_CMP_POS(cone_dot, <, pL->cos_theta_by_2))
        {
            D3DVALUE val = (cone_dot - pL->cos_phi_by_2) * pL->inv_theta_minus_phi;
            
            if (!FLOAT_EQZ( pL->Falloff - (float) 1.0 ))
            {
                //val = val/(pLight->dvFalloff - (pLight->dvFalloff*val) + val );
                val = POWF( val, pL->Falloff );
            }
            att *= val;
        }
    }

    //
    // Add the material's ambient component
    //
    if (!bDoColVertexAmbient)
    {
        LData->fDiffuse.r += att*pL->Ma_La.r;
        LData->fDiffuse.g += att*pL->Ma_La.g;
        LData->fDiffuse.b += att*pL->Ma_La.b;
    }
    else
    {
        //
        // Note:
        // In case ColorVertexAmbient is enabled, note that it uses
        // VertexSpecular instead of VertexDiffuse
        //
        LData->fDiffuse.r += att*pL->La.r * LData->pAmbientSrc->r;
        LData->fDiffuse.g += att*pL->La.g * LData->pAmbientSrc->g;
        LData->fDiffuse.b += att*pL->La.b * LData->pAmbientSrc->b;
    }

    // Calc dot product of light dir with normal.  Note that since we
    // didn't normalize the direction the result is scaled by the distance.
    if( (dwFVFIn & D3DFVF_NORMAL) == 0)
    {
        // If no normals are present, bail out since we cannot perform the
        // normal-dependent computations
        return;
    }
    else
    {
        dot = DotProduct(&d, &in->dvNormal );
    }

    if (FLOAT_GTZ( dot ))
    {
        dot *= dist*att;

        if (!bDoColVertexDiffuse)
        {
            LData->fDiffuse.r += pL->Md_Ld.r * dot;
            LData->fDiffuse.g += pL->Md_Ld.g * dot;
            LData->fDiffuse.b += pL->Md_Ld.b * dot;
        }
        else
        {
            LData->fDiffuse.r += pL->Ld.r * LData->pDiffuseSrc->r * dot;
            LData->fDiffuse.g += pL->Ld.g * LData->pDiffuseSrc->g * dot;
            LData->fDiffuse.b += pL->Ld.b * LData->pDiffuseSrc->b * dot;
        }

        if (bDoSpecular)
        {
            D3DVECTOR h;      // halfway vector
            D3DVECTOR eye;    // incident vector ie vector from eye

            // normalize light direction
            d.x *= dist;
            d.y *= dist;
            d.z *= dist;

            if (bDoLocalViewer)
            {
                // calc vector from vertex to the eye
                SubtractVector( &LData->eye_in_eye, &in->dvPosition, &eye );

                // normalize
                Normalize( &eye );
            }
            else
            {
                eye.x = (D3DVALUE) 0.0 ;
                eye.y = (D3DVALUE) 0.0 ;
                eye.z = (D3DVALUE)-1.0 ;
            }

            // calc halfway vector
            AddVector( &d, &eye, &h );
            Normalize( &h );

            dot = DotProduct( &h, &in->dvNormal );

            if (FLOAT_CMP_POS(dot, >=, LData->specThreshold))
            {
                //D3DVALUE coeff =  dot/(pRc->tl.lighting.Material.power - (pRc->tl.lighting.Material.power*dot)+dot ) * att;
                D3DVALUE coeff = (D3DVALUE) pow( dot, pRc->tl.lighting.Material.power ) * att;
                if (!bDoColVertexSpecular)
                {
                    LData->fSpecular.r += pL->Ms_Ls.r * coeff;
                    LData->fSpecular.g += pL->Ms_Ls.g * coeff;
                    LData->fSpecular.b += pL->Ms_Ls.b * coeff;
                }
                else
                {
                    LData->fSpecular.r += (pL->Ls.r * LData->pSpecularSrc->r * coeff);
                    LData->fSpecular.g += (pL->Ls.g * LData->pSpecularSrc->g * coeff);
                    LData->fSpecular.b += (pL->Ls.b * LData->pSpecularSrc->b * coeff);
                }
            }
        }
    }
    return;
}
#endif //0-unused


/*----------------------------------------------------------------------**
**                                                                      **
**                                                                      **
**             Lighting Setup, Maintenance, and Processing              **
**                                                                      **
**                                                                      **
**----------------------------------------------------------------------*/

/*-------------------------------------------------------------------
Function Name:  InitializeLight
Description:    Clear all of the fields in a light array element to 
                sane, safe, values
Parameters:     
                TLLIGHT *pL -- pointer to the light to Initialize
Information:    Called "RRLight::RRLight()" in reference rasterizer
Return:         
-------------------------------------------------------------------*/
void InitializeLight(TLLIGHT *pL)
{
  pL->dwFlags = TLLIGHT_NEEDSPROCESSING;
  pL->Next = NULL;

  ZeroMemory(&pL->SOAposition_in_eye, sizeof(pL->SOAposition_in_eye));
  ZeroMemory(&pL->SOAdirection_in_eye, sizeof(pL->SOAdirection_in_eye));
  ZeroMemory(&pL->La, sizeof(pL->La));
  ZeroMemory(&pL->Ld, sizeof(pL->Ld));
  ZeroMemory(&pL->Ls, sizeof(pL->Ls));
  ZeroMemory(&pL->SOAMa_La, sizeof(pL->SOAMa_La));
  ZeroMemory(&pL->SOAMd_Ld, sizeof(pL->SOAMd_Ld));
  ZeroMemory(&pL->SOAMs_Ls, sizeof(pL->SOAMs_Ls));
  ZeroMemory(&pL->SOAAttenuation0, sizeof(pL->SOAAttenuation0));
  ZeroMemory(&pL->SOAAttenuation1, sizeof(pL->SOAAttenuation1));
  ZeroMemory(&pL->SOAAttenuation2, sizeof(pL->SOAAttenuation2));
  ZeroMemory(&pL->Position, sizeof(pL->Position));
  ZeroMemory(&pL->Direction, sizeof(pL->Direction));
  pL->Range = 0.0;
  pL->Falloff = 0.0;
  pL->Theta = 0.0;
  pL->Phi = 0.0;

  // Initialize the light to some default values
  pL->dltType        = D3DLIGHT_DIRECTIONAL;

  pL->La.r   = 1;
  pL->La.g   = 1;
  pL->La.b   = 1;
  pL->La.a   = 0;
 
  pL->Direction.x  = 0;
  pL->Direction.y  = 0;
  pL->Direction.z  = 1;

  return;
}


/*-------------------------------------------------------------------
Function Name:  SetLight
Description:    Copy lighting data from the app into a light element 
                in the light array
Parameters:   
                RC *pRc -- 
                TLLIGHT *pL -- pointer to the light
                LPD3DLIGHT7 pLight -- Light data to set
Information:    
Return:         
-------------------------------------------------------------------*/

HRESULT SetLight(RC *pRc, TLLIGHT *pL, LPD3DLIGHT7 pLight)
{
  // Validate the parameters passed
  switch (pLight->dltType)
  {
  case D3DLIGHT_POINT:
  case D3DLIGHT_SPOT:
  case D3DLIGHT_DIRECTIONAL:
    break;
  default:
    // No other light types are allowed
    //        DPFRR(0, "Invalid light type passed");
    return DDERR_INVALIDPARAMS;
  }

  if (pLight)
  {
    pL->dltType = pLight->dltType;
    pL->La = pLight->dcvAmbient;
    pL->Ld = pLight->dcvDiffuse;
    pL->Ls = pLight->dcvSpecular;
    pL->Position = pLight->dvPosition;
    pL->Direction = pLight->dvDirection;
    pL->Range = pLight->dvRange;
    pL->Falloff = pLight->dvFalloff;
    pL->Theta = pLight->dvTheta;
    pL->Phi = pLight->dvPhi;
    pL->Attenuation0 = pLight->dvAttenuation0;
    pL->Attenuation1 = pLight->dvAttenuation1;
    pL->Attenuation2 = pLight->dvAttenuation2;
  }

  // Mark it for processing later
  pL->dwFlags |= TLLIGHT_NEEDSPROCESSING;
  pL->dwFlags &= ~TLLIGHT_READY;
  return DD_OK;
}


/*-------------------------------------------------------------------
Function Name:  ProcessLight
Description:    Pre-process a light so that it's ready for the lighting 
                loop
Parameters:     
                TLLIGHT *pL -- Pointer to an element in the light array
                D3DMATERIAL7 *mat -- Pointer to the material
                TLLIGHTVERTEX_FUNC_TABLE *pTbl -- Pointer to the Lighting
                function table
Information:    
                This routine sets the TLLIGHT_READY bit after processing
                the light, meaning that this light is valid and ready 
                to be used.
Return:         
-------------------------------------------------------------------*/
//void ProcessLight(RC *pRc, TLLIGHT *pL, D3DMATERIAL7 *mat, TLLIGHTVERTEX_FUNC_TABLE *pTbl)
void ProcessLight(RC *pRc, TLLIGHT *pL, D3DMATERIAL7 *mat)
{
  extern TLLIGHT_SOA_FUNC_TABLE SOA_Light_fns;
  DWORD diff, spec;

  /* Range of lights is from 0.0 to 1.0, scale to 0.0 to 255.0 */
  // Material Ambient times Light Ambient
  pL->Ma_La.r = mat->ambient.r * pL->La.r * 255.0f;
  pL->Ma_La.g = mat->ambient.g * pL->La.g * 255.0f;
  pL->Ma_La.b = mat->ambient.b * pL->La.b * 255.0f;

  // Material Diffuse times Light Diffuse
  pL->Md_Ld.r = mat->diffuse.r * pL->Ld.r * 255.0f;
  pL->Md_Ld.g = mat->diffuse.g * pL->Ld.g * 255.0f;
  pL->Md_Ld.b = mat->diffuse.b * pL->Ld.b * 255.0f;

  // Material Specular times Light Specular
  pL->Ms_Ls.r = mat->specular.r * pL->Ls.r * 255.0f;
  pL->Ms_Ls.g = mat->specular.g * pL->Ls.g * 255.0f;
  pL->Ms_Ls.b = mat->specular.b * pL->Ls.b * 255.0f;

  if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
  {
    SwizzleScaler2SOAFLOAT(pL->Ma_La.r, &pL->SOAMa_La.red);
    SwizzleScaler2SOAFLOAT(pL->Ma_La.g, &pL->SOAMa_La.green);
    SwizzleScaler2SOAFLOAT(pL->Ma_La.b, &pL->SOAMa_La.blue);
    SwizzleScaler2SOAFLOAT(pL->Md_Ld.r, &pL->SOAMd_Ld.red);
    SwizzleScaler2SOAFLOAT(pL->Md_Ld.g, &pL->SOAMd_Ld.green);
    SwizzleScaler2SOAFLOAT(pL->Md_Ld.b, &pL->SOAMd_Ld.blue);
    SwizzleScaler2SOAFLOAT(pL->Ms_Ls.r, &pL->SOAMs_Ls.red);
    SwizzleScaler2SOAFLOAT(pL->Ms_Ls.g, &pL->SOAMs_Ls.green);
    SwizzleScaler2SOAFLOAT(pL->Ms_Ls.b, &pL->SOAMs_Ls.blue);
    SwizzleScaler2SOAFLOAT(pRc->tl.lighting.specThreshold, &pL->SOASpecThresh);
    if( pRc->tl.dwTLState & TLPV_COLORVERTEXFLAGS)
	{ // these are used for colorvertex
	  SwizzleScaler2SOAFLOAT(pL->La.r, &pL->SOALa.red);
	  SwizzleScaler2SOAFLOAT(pL->La.g, &pL->SOALa.green);
 	  SwizzleScaler2SOAFLOAT(pL->La.b, &pL->SOALa.blue);
	  SwizzleScaler2SOAFLOAT(pL->Ld.r, &pL->SOALd.red);
	  SwizzleScaler2SOAFLOAT(pL->Ld.g, &pL->SOALd.green);
	  SwizzleScaler2SOAFLOAT(pL->Ld.b, &pL->SOALd.blue);
	  SwizzleScaler2SOAFLOAT(pL->Ls.r, &pL->SOALs.red);
	  SwizzleScaler2SOAFLOAT(pL->Ls.g, &pL->SOALs.green);
	  SwizzleScaler2SOAFLOAT(pL->Ls.b, &pL->SOALs.blue);
	}
  } 
  else if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
  {
	/* 
	* This is for the mmx lighting ... mmx has a 16-bit signed multiply 
	* that we're stuck with.  The P3 and K7 have a 16-bit unsigned mul 
	* but that only gains us one more bit.  So we have to do some scaling.
	* The previous scale by 255 makes the value from 0.0-255.0.  Rampage 
	* will support overbright colors up to 16.0 so (255*16) means we 
	* need 12 bits of magnitude.  I have seen diffuse colors at least 
	* as high as 3.5 in 3dMark.  I *really* don't know if 16 is enough 
	* though.  Maybe we'll have to clamp colors that are too bright.
	*/

#if 0
	// special Ma_La for directional, value is a 16.0 so we can just add it to the diffuse
    pL->p8Ma_La_U16pt0.uw.b = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Ma_La.b) + (0 << 23)))));
    pL->p8Ma_La_U16pt0.uw.g = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Ma_La.g) + (0 << 23)))));
    pL->p8Ma_La_U16pt0.uw.r = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Ma_La.r) + (0 << 23)))));
    pL->p8Ma_La_U16pt0.uw.a = 0;
    // drop 4 more precision bits, change to short, now they're U12.4
    pL->p8Ma_La_U12pt4.uw.b = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Ma_La.b) + (4 << 23)))));
    pL->p8Ma_La_U12pt4.uw.g = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Ma_La.g) + (4 << 23)))));
    pL->p8Ma_La_U12pt4.uw.r = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Ma_La.r) + (4 << 23)))));
    pL->p8Ma_La_U12pt4.uw.a = 0;
    pL->p8Md_Ld_U12pt4.uw.b = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Md_Ld.b) + (4 << 23)))));
    pL->p8Md_Ld_U12pt4.uw.g = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Md_Ld.g) + (4 << 23)))));
    pL->p8Md_Ld_U12pt4.uw.r = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Md_Ld.r) + (4 << 23)))));
    pL->p8Md_Ld_U12pt4.uw.a = 0;
    pL->p8Ms_Ls_U12pt4.uw.b = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Ms_Ls.b) + (4 << 23)))));
    pL->p8Ms_Ls_U12pt4.uw.g = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Ms_Ls.g) + (4 << 23)))));
    pL->p8Ms_Ls_U12pt4.uw.r = (unsigned short)FTOI((AS_FLOAT((AS_UINT32(pL->Ms_Ls.r) + (4 << 23)))));
    pL->p8Ms_Ls_U12pt4.uw.a = 0;
#else
	_asm
	{
	  // Move from D3DCOLORVALUE (r,g,b,a) to P8RGBA (b,g,r,a) 
	  mov		eax, [pL]
	  movd		mm0, [eax]TLLIGHT.Ma_La.b	// 0		Ma_La.b
	  punpckldq	mm0, [eax]TLLIGHT.Ma_La.g	// Ma_La.g	Ma_La.b
	  movd		mm1, [eax]TLLIGHT.Ma_La.r	// 0		Ma_La.r
	  movq		mm2, mm0					// Ma_La.g	Ma_La.b
	  movq		mm3, mm1					// 0		Ma_La.r
	  movd		mm4, [eax]TLLIGHT.Md_Ld.b	// 0		Md_Ld.b
	  punpckldq	mm4, [eax]TLLIGHT.Md_Ld.g	// Md_Ld.g	Md_Ld.b
	  movd		mm5, [eax]TLLIGHT.Md_Ld.r	// 0		Md_Ld.r
	  movd		mm6, [eax]TLLIGHT.Ms_Ls.b	// 0		Ms_Ls.b
	  punpckldq	mm6, [eax]TLLIGHT.Ms_Ls.g	// Ms_Ls.g	Ms_Ls.b
	  movd		mm7, [eax]TLLIGHT.Ms_Ls.r	// 0		Ms_Ls.r
	  pf2iw		mm0, mm0
	  paddd		mm2, [exp_X16]				// 4 << 23
	  pf2iw		mm1, mm1
	  paddd		mm3, [exp_X16]
	  paddd		mm4, [exp_X16]
	  pf2iw		mm2, mm2
	  paddd		mm5, [exp_X16]
	  pf2iw		mm3, mm3
	  paddd		mm6, [exp_X16]
	  packssdw	mm0, mm1					// Ma_La.0rgb * 0
	  pf2iw		mm4, mm4
	  paddd		mm7, [exp_X16]
	  pf2iw		mm5, mm5
	  packssdw	mm2, mm3					// Ma_La.0rgb * 16
	  pf2iw		mm6, mm6
	  movq		[eax]TLLIGHT.p8Ma_La_U16pt0.uw, mm0
	  pf2iw		mm7, mm7
	  packssdw	mm4, mm5					// Md_Ld.0rgb * 16
	  movq		[eax]TLLIGHT.p8Ma_La_U12pt4.uw, mm2
	  packssdw	mm6, mm7					// Ms_Ls.0rgb * 16
	  movq		[eax]TLLIGHT.p8Md_Ld_U12pt4.uw, mm4
	  movq		[eax]TLLIGHT.p8Ms_Ls_U12pt4.uw, mm6
	}
#endif
    if( pRc->tl.dwTLState & TLPV_COLORVERTEXFLAGS)
	{ // these are used for colorvertex, the lights are range 0-1 and we need 0-12K
#if 0
      pL->p8La_U12pt0.uw.b = (unsigned short)FTOI(pL->La.b * 255.0f * 16.0f);
      pL->p8La_U12pt0.uw.g = (unsigned short)FTOI(pL->La.g * 255.0f * 16.0f);
      pL->p8La_U12pt0.uw.r = (unsigned short)FTOI(pL->La.r * 255.0f * 16.0f);
      pL->p8La_U12pt0.uw.a = 0;
      pL->p8Ld_U12pt0.uw.b = (unsigned short)FTOI(pL->Ld.b * 255.0f * 16.0f);
      pL->p8Ld_U12pt0.uw.g = (unsigned short)FTOI(pL->Ld.g * 255.0f * 16.0f);
      pL->p8Ld_U12pt0.uw.r = (unsigned short)FTOI(pL->Ld.r * 255.0f * 16.0f);
      pL->p8Ld_U12pt0.uw.a = 0;
      pL->p8Ls_U12pt0.uw.b = (unsigned short)FTOI(pL->Ls.b * 255.0f * 16.0f);
      pL->p8Ls_U12pt0.uw.g = (unsigned short)FTOI(pL->Ls.g * 255.0f * 16.0f);
      pL->p8Ls_U12pt0.uw.r = (unsigned short)FTOI(pL->Ls.r * 255.0f * 16.0f);
      pL->p8Ls_U12pt0.uw.a = 0;
#else
	  _asm
	  {
	    mov			eax, [pL]
	    movq		mm2, [exp_X4K]
	    movd		mm0, [eax]TLLIGHT.La.b	// 0		La.b
	    punpckldq	mm0, [eax]TLLIGHT.La.g	// La.g		La.b
	    movd		mm1, [eax]TLLIGHT.La.r	// 0		La.r
	    movd		mm4, [eax]TLLIGHT.Ld.b	// 0		Ld.b
	    punpckldq	mm4, [eax]TLLIGHT.Ld.g	// Ld.g		Ld.b
	    movd		mm5, [eax]TLLIGHT.Ld.r	// 0		Ld.r
	    movd		mm6, [eax]TLLIGHT.Ls.b	// 0		Ls.b
	    punpckldq	mm6, [eax]TLLIGHT.Ls.g	// Ls.g		Ls.b
	    movd		mm7, [eax]TLLIGHT.Ls.r	// 0		Ls.r
	    paddd		mm0, mm2
	    paddd		mm1, mm2
	    paddd		mm4, mm2
	    paddd		mm5, mm2
	    paddd		mm6, mm2
	    paddd		mm7, mm2
	    pf2iw		mm0, mm0
	    pf2iw		mm1, mm1
	    pf2iw		mm4, mm4
	    pf2iw		mm5, mm5
	    pf2iw		mm6, mm6
	    pf2iw		mm7, mm7
	    packssdw	mm0, mm1				// La.0rgb * 4k
	    packssdw	mm4, mm5				// Ld.0rgb * 4k
	    packssdw	mm6, mm7				// Ls.0rgb * 4k
	    movq		[eax]TLLIGHT.p8La_U12pt0.uw, mm0
	    movq		[eax]TLLIGHT.p8Ld_U12pt0.uw, mm4
	    movq		[eax]TLLIGHT.p8Ls_U12pt0.uw, mm6
	  }
#endif
    }
	_asm femms
  }


  //
  // Assign the actual lighting function pointer, in addition to
  // performing some precomputation of light-type specific data
  //
  pL->pfnLightVertex = NULL;
  switch (pL->dltType)
  {
  case D3DLIGHT_DIRECTIONAL:
    pL->pfnLightVertex = pRc->tl.lighting.LightVertexTable.pfnDirectional;	// fallback path

#if defined(VCPP)
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
	{
      diff = spec = 0;
      if( pRc->tl.dwTLState & TLPV_COLORVERTEXAMB)
	  {
        if (pL->La.r > 0) diff |= DIFF_AR;
        if (pL->La.g > 0) diff |= DIFF_AG;
        if (pL->La.b > 0) diff |= DIFF_AB;
	  }
	  else
	  {
        if (pL->Ma_La.r > 0) diff |= DIFF_AR;
        if (pL->Ma_La.g > 0) diff |= DIFF_AG;
        if (pL->Ma_La.b > 0) diff |= DIFF_AB;
	  }
      if (pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL)  
      {
        diff |= DIFF_N;
        if( pRc->tl.dwTLState & TLPV_COLORVERTEXDIFF)
    	{
          if (pL->Ld.r > 0) diff |= DIFF_R;
          if (pL->Ld.g > 0) diff |= DIFF_G;
          if (pL->Ld.b > 0) diff |= DIFF_B;
		}
		else
		{
          if (pL->Md_Ld.r > 0) diff |= DIFF_R;
          if (pL->Md_Ld.g > 0) diff |= DIFF_G;
          if (pL->Md_Ld.b > 0) diff |= DIFF_B;
		}
        if( pRc->tl.dwTLState & TLPV_COLORVERTEXSPEC)
    	{
          if (pL->Ls.r > 0) spec |= SPEC_R;
          if (pL->Ls.g > 0) spec |= SPEC_G;
          if (pL->Ls.b > 0) spec |= SPEC_B;
		}
		else
		{
          if (pL->Ms_Ls.r > 0) spec |= SPEC_R;
          if (pL->Ms_Ls.g > 0) spec |= SPEC_G;
          if (pL->Ms_Ls.b > 0) spec |= SPEC_B;
		}
        if (pRc->tl.dwTLState & TLPV_LOCALVIEWER) spec |= SPEC_L;
        if (mat->power == 0.0f) spec |= SPEC_E0;
        if (mat->power > 25.0f) spec |= SPEC_EB;
      }
      pL->pfnLightVertexSOA_Diff = SOA_Light_fns.pfnDirectionalSOA_D[diff];
      pL->pfnLightVertexSOA_Spec = SOA_Light_fns.pfnDirectionalSOA_S[spec];
	}
	else
#endif //defined(VCPP)
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
    {
	  DWORD k3dlight = 0;
	  //if ((pL->p8Ma_La_U12pt4.uw.r | pL->p8Ma_La_U12pt4.uw.g | pL->p8Ma_La_U12pt4.uw.b) != 0)		
	    k3dlight |= K3DL_AMBIENT;	// always do diffuse ambient on directional (because it's just 1 add)
      if (pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL)  
      {
        k3dlight |= K3DL_NORMAL;
	    if ((pL->p8Md_Ld_U12pt4.uw.r | pL->p8Md_Ld_U12pt4.uw.g | pL->p8Md_Ld_U12pt4.uw.b) != 0)		
	  	  k3dlight |= K3DL_DIFF;
	    if ( (pRc->tl.dwTLState & TLPV_DOSPECULAR) &&
	       ((pL->p8Ms_Ls_U12pt4.uw.r | pL->p8Ms_Ls_U12pt4.uw.g | pL->p8Ms_Ls_U12pt4.uw.b) != 0) )
		{
  	  	  k3dlight |= K3DL_SPECULAR;
          if (pRc->tl.dwTLState & TLPV_LOCALVIEWER) 
            k3dlight |= K3DL_LOCAL_VIEWER;
		}
	  }
	  // check any colorvertex but emmissive because that's handled 
	  // outside of the individual lighting functions
      if( pRc->tl.dwTLState & (TLPV_COLORVERTEXAMB | TLPV_COLORVERTEXDIFF | TLPV_COLORVERTEXSPEC) )
		k3dlight |= K3DL_COLORVERTEX;

      pL->pfnLightVertexK3D = K3D_DirectionalLight_fns[k3dlight];
    }
//    else if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
//    {
//	  pL->pfnLightVertexK3D = pRc->tl.lighting.LightVertexTable.pfnDirectionalK3d;
//    }

    break;

  case D3DLIGHT_POINT:
    pL->pfnLightVertex = pRc->tl.lighting.LightVertexTable.pfnPoint;	// fallback path

    pL->range_squared = pL->Range * pL->Range;
    pL->inv_theta_minus_phi = 1.0f;

#if defined(VCPP)
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
	{
      SwizzleScaler2SOAFLOAT(pL->range_squared, &pL->SOArange_squared);
      SwizzleScaler2SOAFLOAT(pL->inv_theta_minus_phi, &pL->SOAinv_theta_minus_phi);
      SwizzleScaler2SOAFLOAT(pL->Attenuation0, &pL->SOAAttenuation0);
      SwizzleScaler2SOAFLOAT(pL->Attenuation1, &pL->SOAAttenuation1);
      SwizzleScaler2SOAFLOAT(pL->Attenuation2, &pL->SOAAttenuation2);

      // Compute which diffuse case to use.
      diff = spec = 0;
      if( pRc->tl.dwTLState & TLPV_COLORVERTEXAMB)
	  {
        if (pL->La.r > 0) diff |= DIFF_AR;
        if (pL->La.g > 0) diff |= DIFF_AG;
        if (pL->La.b > 0) diff |= DIFF_AB;
	  }
	  else
	  {
        if (pL->Ma_La.r > 0) diff |= DIFF_AR;
        if (pL->Ma_La.g > 0) diff |= DIFF_AG;
        if (pL->Ma_La.b > 0) diff |= DIFF_AB;
	  }
      if (pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL)  
      {
        diff |= DIFF_N;
        if( pRc->tl.dwTLState & TLPV_COLORVERTEXDIFF)
    	{
          if (pL->Ld.r > 0) diff |= DIFF_R;
          if (pL->Ld.g > 0) diff |= DIFF_G;
          if (pL->Ld.b > 0) diff |= DIFF_B;
		}
		else
		{
          if (pL->Md_Ld.r > 0) diff |= DIFF_R;
          if (pL->Md_Ld.g > 0) diff |= DIFF_G;
          if (pL->Md_Ld.b > 0) diff |= DIFF_B;
		}
        if (!(pL->Attenuation0 == 1.0f && FLOAT_EQZ(pL->Attenuation1) && FLOAT_EQZ(pL->Attenuation2)))  
        {
          diff |= DIFF_A;
          spec |= SPEC_A;        
        }
        if( pRc->tl.dwTLState & TLPV_COLORVERTEXSPEC)
    	{
          if (pL->Ls.r > 0) spec |= SPEC_R;
          if (pL->Ls.g > 0) spec |= SPEC_G;
          if (pL->Ls.b > 0) spec |= SPEC_B;
		}
		else
		{
          if (pL->Ms_Ls.r > 0) spec |= SPEC_R;
          if (pL->Ms_Ls.g > 0) spec |= SPEC_G;
          if (pL->Ms_Ls.b > 0) spec |= SPEC_B;
		}
        if (pRc->tl.dwTLState & TLPV_LOCALVIEWER) spec |= SPEC_L;
        if (mat->power == 0.0f) spec |= SPEC_E0;
        if (mat->power > 25.0f) spec |= SPEC_EB;
      }

      pL->pfnLightVertexSOA_Diff = SOA_Light_fns.pfnPointSOA_D[diff];
      pL->pfnLightVertexSOA_Spec = SOA_Light_fns.pfnPointSOA_S[spec];
    }
	else
#endif //defined(VCPP)
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
    {
	  DWORD k3dlight = 0;
      if (!(pL->Attenuation0 == 1.0f && FLOAT_EQZ(pL->Attenuation1) && FLOAT_EQZ(pL->Attenuation2)))  
		k3dlight |= K3DL_QUAD1;
	  if ((pL->p8Ma_La_U12pt4.uw.r | pL->p8Ma_La_U12pt4.uw.g | pL->p8Ma_La_U12pt4.uw.b) != 0)		
	  	k3dlight |= K3DL_AMBIENT;
      if (pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL)  
      {
        k3dlight |= K3DL_NORMAL;
	    if ((pL->p8Md_Ld_U12pt4.uw.r | pL->p8Md_Ld_U12pt4.uw.g | pL->p8Md_Ld_U12pt4.uw.b) != 0)		
	  	  k3dlight |= K3DL_DIFF;
	    if ( (pRc->tl.dwTLState & TLPV_DOSPECULAR) &&
	       ((pL->p8Ms_Ls_U12pt4.uw.r | pL->p8Ms_Ls_U12pt4.uw.g | pL->p8Ms_Ls_U12pt4.uw.b) != 0) )
		{
  	  	  k3dlight |= K3DL_SPECULAR;
          if (pRc->tl.dwTLState & TLPV_LOCALVIEWER) 
            k3dlight |= K3DL_LOCAL_VIEWER;
		}
	  }
	  // check any colorvertex but emmissive because that's handled 
	  // outside of the individual lighting functions
      if( pRc->tl.dwTLState & (TLPV_COLORVERTEXAMB | TLPV_COLORVERTEXDIFF | TLPV_COLORVERTEXSPEC) )
		k3dlight |= K3DL_COLORVERTEX;

      pL->pfnLightVertexK3D = K3D_PointLight_fns[k3dlight];
    }
//    else if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
//    {
//		pL->pfnLightVertexK3D = pRc->tl.lighting.LightVertexTable.pfnPointK3d;
//    }
    break;

  case D3DLIGHT_SPOT:
    pL->pfnLightVertex = pRc->tl.lighting.LightVertexTable.pfnSpot;	// fallback path

    pL->range_squared = pL->Range * pL->Range;
    pL->cos_theta_by_2 = (float)cos(pL->Theta / 2.0f);
   	pL->cos_phi_by_2 = (float)cos(pL->Phi / 2.0f);

	// I added some error checking that the refrast left out
	// First make sure that cos_phi_by_2 is positive, sometimes if Phi=pi it will be slightly negetive
	pL->cos_phi_by_2 = MAX( pL->cos_phi_by_2, 0.0f );
	// Make sure the inner cone is smaller than the outer, which cos(Theta) must be larger than cos(Phi)
    pL->cos_theta_by_2 = MAX( pL->cos_theta_by_2, pL->cos_phi_by_2 );

    pL->inv_theta_minus_phi = pL->cos_theta_by_2 - pL->cos_phi_by_2;
   	if (pL->inv_theta_minus_phi != 0.0f)
		pL->inv_theta_minus_phi = 1.0f/pL->inv_theta_minus_phi;
	else
		pL->inv_theta_minus_phi = 1.0f;

#if defined(VCPP)
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
	{
      SwizzleScaler2SOAFLOAT(pL->range_squared, &pL->SOArange_squared);
      SwizzleScaler2SOAFLOAT(pL->cos_theta_by_2, &pL->SOAcos_theta_by_2);
      SwizzleScaler2SOAFLOAT(pL->cos_phi_by_2, &pL->SOAcos_phi_by_2);
      SwizzleScaler2SOAFLOAT(pL->inv_theta_minus_phi, &pL->SOAinv_theta_minus_phi);
      SwizzleScaler2SOAFLOAT(pL->Attenuation0, &pL->SOAAttenuation0);
      SwizzleScaler2SOAFLOAT(pL->Attenuation1, &pL->SOAAttenuation1);
      SwizzleScaler2SOAFLOAT(pL->Attenuation2, &pL->SOAAttenuation2);

      diff = spec = 0;
      if( pRc->tl.dwTLState & TLPV_COLORVERTEXAMB)
	  {
        if (pL->La.r > 0) diff |= DIFF_AR;
        if (pL->La.g > 0) diff |= DIFF_AG;
        if (pL->La.b > 0) diff |= DIFF_AB;
	  }
	  else
	  {
        if (pL->Ma_La.r > 0) diff |= DIFF_AR;
        if (pL->Ma_La.g > 0) diff |= DIFF_AG;
        if (pL->Ma_La.b > 0) diff |= DIFF_AB;
	  }
      if (pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL)  
      {
        diff |= DIFF_N;
        if( pRc->tl.dwTLState & TLPV_COLORVERTEXDIFF)
    	{
          if (pL->Ld.r > 0) diff |= DIFF_R;
          if (pL->Ld.g > 0) diff |= DIFF_G;
          if (pL->Ld.b > 0) diff |= DIFF_B;
		}
		else
		{
          if (pL->Md_Ld.r > 0) diff |= DIFF_R;
          if (pL->Md_Ld.g > 0) diff |= DIFF_G;
          if (pL->Md_Ld.b > 0) diff |= DIFF_B;
		}
        if (!(pL->Attenuation0 == 1.0f && FLOAT_EQZ(pL->Attenuation1) && FLOAT_EQZ(pL->Attenuation2)))  
        {
          diff |= DIFF_A;
          spec |= SPEC_A;        
        }
        if( pRc->tl.dwTLState & TLPV_COLORVERTEXSPEC)
    	{
          if (pL->Ls.r > 0) spec |= SPEC_R;
          if (pL->Ls.g > 0) spec |= SPEC_G;
          if (pL->Ls.b > 0) spec |= SPEC_B;
		}
		else
		{
          if (pL->Ms_Ls.r > 0) spec |= SPEC_R;
          if (pL->Ms_Ls.g > 0) spec |= SPEC_G;
          if (pL->Ms_Ls.b > 0) spec |= SPEC_B;
		}
        if (pRc->tl.dwTLState & TLPV_LOCALVIEWER) spec |= SPEC_L;
        if (mat->power == 0.0f) spec |= SPEC_E0;
        if (mat->power > 25.0f) spec |= SPEC_EB;
      }

      pL->pfnLightVertexSOA_Diff = SOA_Light_fns.pfnSpotSOA_D[diff];
      pL->pfnLightVertexSOA_Spec = SOA_Light_fns.pfnSpotSOA_S[spec];
	}
	else
#endif //defined(VCPP)
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
    {
	  DWORD k3dlight = 0;
      if (!(pL->Attenuation0 == 1.0f && FLOAT_EQZ(pL->Attenuation1) && FLOAT_EQZ(pL->Attenuation2)))  
		k3dlight |= K3DL_QUAD1;
	  if (pL->Falloff != 1.0f)
	  	k3dlight |= K3DL_FALLOFF1;
	  if ((pL->p8Ma_La_U12pt4.uw.r | pL->p8Ma_La_U12pt4.uw.g | pL->p8Ma_La_U12pt4.uw.b) != 0)		
	  	k3dlight |= K3DL_AMBIENT;
      if (pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL)  
      {
        k3dlight |= K3DL_NORMAL;
 	    if ((pL->p8Md_Ld_U12pt4.uw.r | pL->p8Md_Ld_U12pt4.uw.g | pL->p8Md_Ld_U12pt4.uw.b) != 0)
	  	  k3dlight |= K3DL_DIFF;
	    if ( (pRc->tl.dwTLState & TLPV_DOSPECULAR) &&
	       ((pL->p8Ms_Ls_U12pt4.uw.r | pL->p8Ms_Ls_U12pt4.uw.g | pL->p8Ms_Ls_U12pt4.uw.b) != 0) )
		{
  	  	  k3dlight |= K3DL_SPECULAR;
          if (pRc->tl.dwTLState & TLPV_LOCALVIEWER) 
            k3dlight |= K3DL_LOCAL_VIEWER;
		}
	  }
	  // check any colorvertex but emmissive because that's handled 
	  // outside of the individual lighting functions
      if( pRc->tl.dwTLState & (TLPV_COLORVERTEXAMB | TLPV_COLORVERTEXDIFF | TLPV_COLORVERTEXSPEC) )
		k3dlight |= K3DL_COLORVERTEX;

      pL->pfnLightVertexK3D = K3D_SpotLight_fns[k3dlight];
    }
//    else if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
//    {
//		pL->pfnLightVertexK3D = pRc->tl.lighting.LightVertexTable.pfnSpotK3d;
//    }
    break;

  default:
    //        DPFRR( 0, "Cannot process light of unknown type" );
    break;
  }

  // Mark it as been processed
  pL->dwFlags &= ~TLLIGHT_NEEDSPROCESSING;
  pL->dwFlags |= TLLIGHT_READY;
  return;
}


/*-------------------------------------------------------------------
Function Name:  LightEnable
Description:    Enable a light in the light array
Parameters:   
                TLLIGHT *pL -- Pointer to the light to enable
Information:    
                Enable a light that's already been created in the 
                light array. There can be any number of lights created.
                The driver is responsible for managing these. However,
                at any given time when rendering occurs, only the 
                number of lights defined in the device caps may be active.
                However, during LightEnable, it's possible for any number
                of lights to be enabled.

                The Enabled lights are stored in a linked list.


Return:         TRUE if lightstate changed, FALSE if it was already enabled
-------------------------------------------------------------------*/
BOOL LightEnable(TLLIGHT *pL, RC *pRc)
{
  TLLIGHT *pTmp;

  // Assert that it is not already enabled
  if (LightIsEnabled(pL)) 
  	return FALSE;


  // It seems like you should check to see that the maximum
  // number of active lights isn't exceeded here. But doing
  // so will break apps. 

  pRc->tl.lighting.dwNumActiveLights++;

  pTmp = pRc->tl.lighting.pActiveLights;
  pRc->tl.lighting.pActiveLights = pL;
  pL->Next = pTmp;
  pL->dwFlags |= TLLIGHT_ENABLED;
  if (!(pL->dwFlags & TLLIGHT_NEEDSPROCESSING))
    pL->dwFlags |= TLLIGHT_READY;

  return TRUE;
}


/*-------------------------------------------------------------------
Function Name:  LightDisable
Description:    Disable a light in the light array
Parameters:   
                TLLIGHT *pL -- Pointer to the light to enable
Information:    
                Disable a light. This clears the enable bit, and moves
                the light out of the active light list.

Return:         TRUE if lightstate changed, FALSE if it was already disabled
-------------------------------------------------------------------*/
BOOL LightDisable(TLLIGHT *pL, RC *pRc)
{
  TLLIGHT *pLightPrev;
  // Assert that the light is enabled
  if (!LightIsEnabled(pL)) 
  	return FALSE;

  pLightPrev = pRc->tl.lighting.pActiveLights;

  // If this is the first light in the active list
  if (pLightPrev == pL)
  {
    pRc->tl.lighting.pActiveLights = pL->Next;
    pL->dwFlags &= ~TLLIGHT_ENABLED;
    pRc->tl.lighting.dwNumActiveLights--;
    return TRUE;
  }

  while (pLightPrev->Next != pL)
  {
    // Though this light was marked as enabled, it is not on
    // the active list. Assert this.
    if (pLightPrev->Next == NULL)
    {
      pL->dwFlags &= ~TLLIGHT_ENABLED;
      return FALSE;
    }

    // Else get the next pointer
    pLightPrev = pLightPrev->Next;
  }

  pLightPrev->Next = pL->Next;
  pRc->tl.lighting.dwNumActiveLights--;
  pL->dwFlags &= ~TLLIGHT_ENABLED;
  return TRUE;
}


/*-------------------------------------------------------------------
Function Name:  XformLight
Description:    Transform Lights to the camera space
Parameters:   
                TLLIGHT *pL -- pointer to the light in question
                TLMATRIX *mView -- The VIEW matrix
Information:    
Return:         
-------------------------------------------------------------------*/
void XformLight( TLLIGHT *pL, TLMATRIX *mView )
{
  // If the light is not a directional light,
  // tranform its position to camera space
  if (pL->dltType != D3DLIGHT_DIRECTIONAL)
  {
    XformBy4x3(&pL->Position, mView, &pL->position_in_eye);
    SwizzleScaler2SOAFLOAT(pL->position_in_eye.x, &pL->SOAposition_in_eye.x);
    SwizzleScaler2SOAFLOAT(pL->position_in_eye.y, &pL->SOAposition_in_eye.y);
    SwizzleScaler2SOAFLOAT(pL->position_in_eye.z, &pL->SOAposition_in_eye.z);
  }

  if (pL->dltType != D3DLIGHT_POINT)
  {
    // Transform light direction to the eye space
    Xform3VecBy3x3( &pL->Direction, mView, &pL->direction_in_eye );

    // Normalize it
    Normalize( &pL->direction_in_eye );

    // Reverse it such that the direction is to the light
    ReverseVector( &pL->direction_in_eye, &pL->direction_in_eye );
    SwizzleScaler2SOAFLOAT(pL->direction_in_eye.x, &pL->SOAdirection_in_eye.x);
    SwizzleScaler2SOAFLOAT(pL->direction_in_eye.y, &pL->SOAdirection_in_eye.y);
    SwizzleScaler2SOAFLOAT(pL->direction_in_eye.z, &pL->SOAdirection_in_eye.z);
  }

  return;
}

//---------------------------------------------------------------------
// ScaleRGBColorTo255: Scales colors from 0-1 range to 0-255 range
//---------------------------------------------------------------------
void ScaleRGBColorTo255( const D3DCOLORVALUE *src, TLCOLOR *dest )
{
  dest->r = (D3DVALUE)(255.0) * src->r;
  dest->g = (D3DVALUE)(255.0) * src->g;
  dest->b = (D3DVALUE)(255.0) * src->b;
}

/*-------------------------------------------------------------------
Function Name:  UpdateLightingData
Description:    Update all lights after the material, light, or 
                other parameters have changed
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/
HRESULT UpdateLightingData(RC *pRc)
{
  extern TLLIGHTFN pLight_DS[TLMAX_ACTIVE_LIGHTS+1][2];
  extern TLLIGHTFN pLight_D[TLMAX_ACTIVE_LIGHTS+1][2];
  HRESULT hr = D3D_OK;
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TLLIGHT *pActiveLight = pRc->tl.lighting.pActiveLights;
  TL_SOATMP *pTL = pRc->tl.pTL;
  TLLIGHT **pActiveLightArray;	// array of pointers
  DWORD dwNumActiveLights;
  DWORD i;

  //
  // Eye in eye space
  //
  Ldata->eye_in_eye.x = (D3DVALUE)0;
  Ldata->eye_in_eye.y = (D3DVALUE)0;
  Ldata->eye_in_eye.z = (D3DVALUE)0;

  // BUGBUG: Colorvertex may have changed the values of the
  // material alphas
  if (pRc->tl.dwDirtyFlags & TLPV_DIRTY_MATERIAL)
  {
    //
    // Save the material to be used to light vertices
    //
    ScaleRGBColorTo255( &pRc->tl.lighting.Material.ambient, &Ldata->matAmb );
    ScaleRGBColorTo255( &pRc->tl.lighting.Material.diffuse, &Ldata->matDiff );
    ScaleRGBColorTo255( &pRc->tl.lighting.Material.specular, &Ldata->matSpec );
    ScaleRGBColorTo255( &pRc->tl.lighting.Material.emissive, &Ldata->matEmis );
    if (pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
	{
	  SwizzleScaler2SOAFLOAT( Ldata->matAmb.r, &pTL->fSOAmatAmbient.red );
	  SwizzleScaler2SOAFLOAT( Ldata->matAmb.g, &pTL->fSOAmatAmbient.green );
	  SwizzleScaler2SOAFLOAT( Ldata->matAmb.b, &pTL->fSOAmatAmbient.blue );
	  SwizzleScaler2SOAFLOAT( Ldata->matDiff.r, &pTL->fSOAmatDiffuse.red );
	  SwizzleScaler2SOAFLOAT( Ldata->matDiff.g, &pTL->fSOAmatDiffuse.green );
	  SwizzleScaler2SOAFLOAT( Ldata->matDiff.b, &pTL->fSOAmatDiffuse.blue );
	  SwizzleScaler2SOAFLOAT( Ldata->matSpec.r, &pTL->fSOAmatSpecular.red );
	  SwizzleScaler2SOAFLOAT( Ldata->matSpec.g, &pTL->fSOAmatSpecular.green );
	  SwizzleScaler2SOAFLOAT( Ldata->matSpec.b, &pTL->fSOAmatSpecular.blue );
	  SwizzleScaler2SOAFLOAT( Ldata->matEmis.r, &pTL->fSOAmatEmissive.red );
	  SwizzleScaler2SOAFLOAT( Ldata->matEmis.g, &pTL->fSOAmatEmissive.green );
	  SwizzleScaler2SOAFLOAT( Ldata->matEmis.b, &pTL->fSOAmatEmissive.blue );
	}


#ifdef __MAT_PROFILE
    {
      // Really horrible, quick material specular exp. change profiling code
      int i = 0;
      if (!printpow)
      {
        for (i = 0; i < num_pow; i++)
        {
          if (pRc->tl.lighting.Material.power == power[i])
          {
            count[i]++;
            goto done_count;
          }            
        }
        count[num_pow] = 1;
        power[num_pow] = pRc->tl.lighting.Material.power;
        num_pow++;

      }
      else
      {
        for (i = 0; i < num_pow; i++)
        {
          D3DPRINT(0,"Power = %s, count = %d", float2String(power[i]), count[i]);

        }
        for (i = 0; i < num_pow; i++)
        {
          count[i] = 0;
          power[i] = 0.0;
        }
        num_pow = 0;
      }
      done_count:;
    }
#endif //__MAT_PROFILE

    //
    // Compute the Material Diffuse Alpha
    //
    Ldata->materialDiffAlpha = FTOI(pRc->tl.lighting.Material.diffuse.a * 255.0f);
    if (Ldata->materialDiffAlpha < 0)
      Ldata->materialDiffAlpha = 0;
    else if (Ldata->materialDiffAlpha > 255)
      Ldata->materialDiffAlpha = 255 << 24;
    else Ldata->materialDiffAlpha <<= 24;

    //
    // Compute the Material Specular Alpha
    //
	// Also pre-compute Napalm Device Specific 255-materialSpecAlpha
    Ldata->materialSpecAlpha = FTOI(pRc->tl.lighting.Material.specular.a * 255.0f);
    if (Ldata->materialSpecAlpha < 0)
	{
      Ldata->fMatSpecAlpha = 0;
      Ldata->fMatSpecAlphaDev = (255.0f - 0.0f);
      Ldata->materialSpecAlpha = 0;
	}
    else if (Ldata->materialSpecAlpha > 255)
	{
	  Ldata->fMatSpecAlpha = 255.0f;
      Ldata->fMatSpecAlphaDev = (255.0f - 255.0f);
      Ldata->materialSpecAlpha = 255 << 24;
	}
    else
	{
	  Ldata->fMatSpecAlpha = (D3DVALUE)((int)Ldata->materialSpecAlpha);
      Ldata->fMatSpecAlphaDev = (D3DVALUE)((int)(255 - Ldata->materialSpecAlpha));
	  Ldata->materialSpecAlpha <<= 24;
	}

    //
    // Precompute the ambient and emissive components that are
    // not dependent on any contribution by the lights themselves
	// We also need to clamp this because sometimes we'll see some really crazy
	// values come in, and the float->int->byte conversions (particularily the
	// packuswb instruction) can't handle really large values.
    //
    Ldata->ambEmiss.r = MIN(255.0f, (Ldata->ambient_red   * Ldata->matAmb.r + Ldata->matEmis.r));
    Ldata->ambEmiss.g = MIN(255.0f, (Ldata->ambient_green * Ldata->matAmb.g + Ldata->matEmis.g));
    Ldata->ambEmiss.b = MIN(255.0f, (Ldata->ambient_blue  * Ldata->matAmb.b + Ldata->matEmis.b));


#if defined(VCPP)
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
	{
	  SwizzleDWORD2SOADWORD(Ldata->materialDiffAlpha, &pTL->dSOAMatDiffAlpha );
	  SwizzleDWORD2SOADWORD(Ldata->materialSpecAlpha, &pTL->dSOAMatSpecAlpha );
	  SwizzleScaler2SOAFLOAT(Ldata->fMatSpecAlpha, &pTL->fSOAMatSpecAlpha );
      // Create a copy of the ambEmiss components for use by the SOA code
	  SwizzleScaler2SOAFLOAT(Ldata->ambEmiss.r, &pTL->fSOAambEmiss.red );
	  SwizzleScaler2SOAFLOAT(Ldata->ambEmiss.g, &pTL->fSOAambEmiss.green );
	  SwizzleScaler2SOAFLOAT(Ldata->ambEmiss.b, &pTL->fSOAambEmiss.blue );
	  SwizzleScaler2SOAFLOAT(                0, &pTL->fSOAambEmiss.alpha );
	} 
#endif

    // If the dot product is less than this value, specular factor is zero
    if (pRc->tl.lighting.Material.power > 0.001f)
	{
      if (pRc->tl.lighting.Material.power >= 25.0f)
	  {
        Ldata->specThreshold = (D3DVALUE)pow (0.001f, (1.0f/pRc->tl.lighting.Material.power));
	  }
      else 
	  {
        Ldata->specThreshold = 0.0f;
	  }
	}

    
    // Update the Power function for specular components of lighting
#if defined(VCPP)
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
       Ldata->KniPowerFunction = UpdateKniPowerFunctionPtr(pRc->tl.lighting.Material.power);
	else
#endif
    if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
       Ldata->K3dPowerFunction = UpdateK3dPowerFunctionPtr(pRc->tl.lighting.Material.power);
    
  }	//if TLPV_DIRTY_MATERIAL

  if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
  {
	//clamp to 255 (ambEmmis gets pretty big sometimes)
    Ldata->p8AmbEmissPlusDiffAlpha.uw.r = (unsigned short) FTOI(Ldata->ambEmiss.r);
    Ldata->p8AmbEmissPlusDiffAlpha.uw.g = (unsigned short) FTOI(Ldata->ambEmiss.g);
    Ldata->p8AmbEmissPlusDiffAlpha.uw.b = (unsigned short) FTOI(Ldata->ambEmiss.b);
    Ldata->p8AmbEmissPlusDiffAlpha.uw.a = (unsigned short) (*pRc->tl.lighting.pDiffuseAlphaSrc >> 24);
	// specular is confusing, it doesn't have emmisive and the diffuse comes from the alpha
	// If the input vertex has specular, then the alpha component is actually fog
	Ldata->p8AmbEmissPlusSpecAlpha.uw.r = 0;	
	Ldata->p8AmbEmissPlusSpecAlpha.uw.g = 0;	
	Ldata->p8AmbEmissPlusSpecAlpha.uw.b = 0;
	Ldata->p8AmbEmissPlusSpecAlpha.uw.a = Ldata->p8AmbEmissPlusDiffAlpha.uw.a;
  }


  while (pActiveLight)
  {
    if ((pRc->tl.dwDirtyFlags & TLPV_DIRTY_MATERIAL) ||
        LightNeedsProcessing(pActiveLight))
    {
      // If the material is dirty, light needs processing, regardless
      if (pRc->tl.dwDirtyFlags & TLPV_DIRTY_MATERIAL)
      {
        pActiveLight->dwFlags |= TLLIGHT_NEEDSPROCESSING;
        pActiveLight->dwFlags &= ~TLLIGHT_READY;
      }

      // If the light has been set, or some material paramenters
      // changed, re-process the light.
      ProcessLight( pRc, pActiveLight, &pRc->tl.lighting.Material );

      // Transform the light to Eye space
      // Lights are defined in world space, so simply apply the
      // Viewing transform
      XformLight(pActiveLight, &pRc->tl.xfmView );

    }
    else if (pRc->tl.dwDirtyFlags & TLPV_DIRTY_NEEDXFMLIGHT)
    {
      XformLight( pActiveLight, &pRc->tl.xfmView );
    }

    pActiveLight = pActiveLight->Next;
  }

  // Support for array of lighting functions, load the array from the linked list
  pActiveLightArray = pRc->tl.lighting.pActiveLightArray;
  pActiveLight = pRc->tl.lighting.pActiveLights;
  dwNumActiveLights = MIN(TLMAX_ACTIVE_LIGHTS, Ldata->dwNumActiveLights);
  i = 0;
  while (pActiveLight && (i < dwNumActiveLights))  
  {
	if (pActiveLight->dwFlags & TLLIGHT_READY)  
      pActiveLightArray[i++] = pActiveLight;
    pActiveLight = pActiveLight->Next;
  }

  // Choose between Diffuse Only or Diffuse + Specular
#if defined(VCPP)
  if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
  {
    DWORD sse2 = (pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE2) ? 1 : 0;
    if ((pRc->tl.dwTLState & TLPV_DOSPECULAR) && (pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL))
      Ldata->pfnLightVertex = pLight_DS[dwNumActiveLights][sse2];
    else
      Ldata->pfnLightVertex = pLight_D[dwNumActiveLights][sse2];
  }
  else
#endif //defined(VCPP)
  if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOWEXT)
     Ldata->pfnLightVertex = pK3dLight[dwNumActiveLights];
//  else if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
//     Ldata->pfnLightVertex = &LightVertexK3D_C;


  // Clear Lighting dirty flags
  pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_LIGHTING;
  return hr;
}

/*-------------------------------------------------------------------
Function Name:  UpdateFogData
Description:    Process fog parameters after the fog state has changed
Parameters:   
                RC *pRC -- pointer to the rendering context
Information:    
Return:         
-------------------------------------------------------------------*/

HRESULT UpdateFogData(RC *pRc)
{
  HRESULT hr = D3D_OK;

  if (pRc->tl.lighting.fog_end == pRc->tl.lighting.fog_start)
    pRc->tl.lighting.fog_factor = D3DVAL(0.0);
  else
    pRc->tl.lighting.fog_factor = D3DVAL(255) / (pRc->tl.lighting.fog_end - pRc->tl.lighting.fog_start);

  // Clear Fog dirty flags
  pRc->tl.dwDirtyFlags &= ~TLPV_DIRTY_FOG;
  return hr;
}


/*-------------------------------------------------------------------
Function Name:  LightVertex
Description:    Apply all lights to a vertex
Parameters:     
                RC *pRC -- pointer to the rendering context
                D3DLIGHTINGELEMENT *pLE -- pointer to the vertex data
                in camera space, ready for lighting
Information:    
                Currently a maximum of 8 lights are supported. This is
                hard-wired into the code to reduce the number of 
                expensive branch mispredictions in the code.

                The diffuse and specular components for the vertex are
                accumluated here, in 
                pRc->tl.lighting.fDiffuse and pRc->tl.lighting.fSpecular,
                and stored in 
                pRc->tl.lighting.dwDiffuse and pRc->tl.lighting.dwSpecular
Return:         
-------------------------------------------------------------------*/
void LightVertex(RC* pRc, D3DLIGHTINGELEMENT *pLE)
{
    TLLIGHTING *Ldata = &pRc->tl.lighting;
    TLLIGHT  *pLight;
    int r, g, b;
    DWORD a;

    // Initialize Diffuse color with the Ambient and Emissive component
    // independent of the light (Ma*La + Me)
    if (pRc->tl.dwTLState & (TLPV_COLORVERTEXEMIS | TLPV_COLORVERTEXAMB))
    {
        // If the material values need to be replaced, compute
        Ldata->fDiffuse.r = Ldata->ambient_red   * Ldata->pAmbientSrc->r + Ldata->pEmissiveSrc->r;
        Ldata->fDiffuse.g = Ldata->ambient_green * Ldata->pAmbientSrc->g + Ldata->pEmissiveSrc->g;
        Ldata->fDiffuse.b = Ldata->ambient_blue  * Ldata->pAmbientSrc->b + Ldata->pEmissiveSrc->b;
    }
    else
    {
        // If none of the material values needs to be replaced
        Ldata->fDiffuse = Ldata->ambEmiss;
    }

    // Initialize the Specular to Zero
    Ldata->fSpecular.r = (D3DVALUE)(0);
    Ldata->fSpecular.g = (D3DVALUE)(0);
    Ldata->fSpecular.b = (D3DVALUE)(0);

    // In a loop accumulate color from the activated lights
	// The loop really sucks because the indirect branches get mispredicted
    pLight = Ldata->pActiveLights;

    // Light 0
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 1
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 2
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 3
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;


    // Light 4
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 5
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 6
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 7
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 8
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 9
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 10
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 11
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 12
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 13
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 14
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 15
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 16
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 17
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 18
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 19
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 20
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 21
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 22
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 23
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 24
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 25
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 26
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 27
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 28
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 29
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 30
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 31
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }
    else 
      goto LightFinished;

    // Light 32
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc, pLight, pLE);
      pLight = pLight->Next;
    }


LightFinished:

//     while (pLight)
//     {
//         if (pLight->pfnLightVertex)
//             (*pLight->pfnLightVertex)(Ldata,
//                                         &pLight->Light,
//                                         &pLight->LightI,
//                                         pLE,
//                                         pRc->tl.dwTLState,
//                                         pRc->tl.dwFVFIn);
//         pLight = pLight->Next;
//     }

    //
    // Compute the diffuse color of the vertex
    //
    r = FTOI(Ldata->fDiffuse.r);
    g = FTOI(Ldata->fDiffuse.g);
    b = FTOI(Ldata->fDiffuse.b);
    a = *Ldata->pDiffuseAlphaSrc;

    //
    // Clamp the r, g, b, components
    //
    if (r < 0) r = 0; else if (r > 255) r = 255;
    if (g < 0) g = 0; else if (g > 255) g = 255;
    if (b < 0) b = 0; else if (b > 255) b = 255;

    Ldata->dwDiffuse =  a + (r<<16) + (g<<8) + b;


    //
    // Obtain the specular Alpha
    //
    a = *(Ldata->pSpecularAlphaSrc);
    
    //
    // Compute the RGB part of the specular color
    //
    if (pRc->tl.dwTLState & TLPV_DOSPECULAR)
    {
        r = FTOI(Ldata->fSpecular.r);
        g = FTOI(Ldata->fSpecular.g);
        b = FTOI(Ldata->fSpecular.b);

        //
        // Clamp the r, g, b, components
        //
        if (r < 0) r = 0; else if (r > 255) r = 255;
        if (g < 0) g = 0; else if (g > 255) g = 255;
        if (b < 0) b = 0; else if (b > 255) b = 255;
    }
    //
    // If SPECULAR is not enabled but the specular color
    // had been provided in the input vertex, simply copy.
    //
    else if ( pRc->tl.TLFVF.dwFVFType & D3DFVF_SPECULAR )
    {
        r = FTOI(Ldata->vertexSpecular.r);
        g = FTOI(Ldata->vertexSpecular.g);
        b = FTOI(Ldata->vertexSpecular.b);
        a = Ldata->vertexSpecAlpha;
    }
    //
    // If SpecularColor is not enabled
    //
    else
    {
        r = g = b = 0;
    }

    Ldata->dwSpecular = a + (r<<16) + (g<<8) + b;

}

/*-------------------------------------------------------------------
Function Name:  FogVertex
Description:    Performs fogging calculation on the input vertex
                Alpha component of pv->lighting.dwSpecular is set
Parameters:   
                RC *pRC -- pointer to the rendering context
                D3DVECTOR *v -- input vertex in the model space
                D3DLIGHTINGELEMENT *pLE -- vertex, transformed to the 
                camera space
Information:    
Return:         
-------------------------------------------------------------------*/

#define RRPV_SET_ALPHA(color, a) ((char*)&color)[3] = (unsigned char)a;

void FogVertex(RC* pRc, D3DVECTOR *v,
               D3DLIGHTINGELEMENT *pLE,
               int numVertexBlends,
               float *pBlendFactors,
               BOOL bVertexInEyeSpace)
{
    int j;
    D3DVALUE dist = 0.0f;

    //
    // Calculate the distance
    //
    if (bVertexInEyeSpace)
    {
        // Vertex is already transformed to the camera space
        if (pRc->tl.dwTLState & TLPV_RANGEFOG)
        {
			D3DVALUE tmpSquare;
			tmpSquare = (pLE->dvPosition.x*pLE->dvPosition.x + pLE->dvPosition.y*pLE->dvPosition.y + pLE->dvPosition.z*pLE->dvPosition.z); 

			#ifdef VCPP
			if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
				SqrtSS( &dist, &tmpSquare);
			else
			#endif
				dist = SQRTF(tmpSquare);
        }
        else
        {
            dist = pLE->dvPosition.z;
        }
    }
    else if (pRc->tl.dwTLState & TLPV_RANGEFOG)
    {
        D3DVALUE x = 0, y = 0, z = 0;
        float cumulBlend = 0.0f;
        D3DVALUE tmpSquare;

        for( j=0; j<=numVertexBlends; j++)
        {
            float blend;
            
            if( numVertexBlends == 0 )	      blend = 1.0f;
            else if( j == numVertexBlends )   blend = 1.0f - cumulBlend;
            else						      blend = pBlendFactors[j];
                
            cumulBlend += pBlendFactors[j];

            x += (v->x*pRc->tl.lpxfmToEye[j]->_11 + v->y*pRc->tl.lpxfmToEye[j]->_21 + v->z*pRc->tl.lpxfmToEye[j]->_31 + pRc->tl.lpxfmToEye[j]->_41) * blend;
            y += (v->x*pRc->tl.lpxfmToEye[j]->_12 + v->y*pRc->tl.lpxfmToEye[j]->_22 + v->z*pRc->tl.lpxfmToEye[j]->_32 + pRc->tl.lpxfmToEye[j]->_42) * blend;
            z += (v->x*pRc->tl.lpxfmToEye[j]->_13 + v->y*pRc->tl.lpxfmToEye[j]->_23 + v->z*pRc->tl.lpxfmToEye[j]->_33 + pRc->tl.lpxfmToEye[j]->_43) * blend;
        } 
        
        tmpSquare = (x*x + y*y + z*z);
		#ifdef VCPP
		if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
			SqrtSS( &dist, &tmpSquare );
		else
		#endif
			dist = SQRTF(tmpSquare);
    }
    else
    {
        float cumulBlend = 0.0f;

        for( j=0; j<=numVertexBlends; j++)
        {
            float blend;

            if( numVertexBlends == 0 )	      blend = 1.0f;
            else if( j == numVertexBlends )   blend = 1.0f - cumulBlend;
            else						      blend = pBlendFactors[j];
                
            cumulBlend += pBlendFactors[j];

            dist += (v->x*pRc->tl.lpxfmToEye[j]->_13 + v->y*pRc->tl.lpxfmToEye[j]->_23 + v->z*pRc->tl.lpxfmToEye[j]->_33 + pRc->tl.lpxfmToEye[j]->_43) * blend;
        } 
    }

    if (pRc->tl.lighting.fog_mode == D3DFOG_LINEAR)
    {
        if (dist < pRc->tl.lighting.fog_start)
		{
            RRPV_SET_ALPHA(pRc->tl.lighting.dwSpecular, 255);
		}
        else if (dist >= pRc->tl.lighting.fog_end)
		{
            RRPV_SET_ALPHA(pRc->tl.lighting.dwSpecular, 0);
		}
        else
        {
            D3DVALUE v = (pRc->tl.lighting.fog_end - dist) * pRc->tl.lighting.fog_factor;
            int f = FTOI(v);
            RRPV_SET_ALPHA(pRc->tl.lighting.dwSpecular, f);
        }
    }
    else
    {
        int f;
        D3DVALUE tmp = dist * pRc->tl.lighting.fog_density;
        if (pRc->tl.lighting.fog_mode == D3DFOG_EXP2)
            tmp *= tmp;
        tmp = (D3DVALUE)exp(-tmp) * 255.0f;
        f = FTOI(tmp);
        RRPV_SET_ALPHA( pRc->tl.lighting.dwSpecular, f )
    }

    return;
}
#endif
#endif













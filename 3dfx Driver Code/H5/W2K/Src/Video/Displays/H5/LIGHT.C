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
** $Revision: 19$
** $Date: 10/11/00 8:45:11 PM$
**
** $Log: 
**  19   3dfx      1.17.2.0    10/11/00 Brent           Forced check in to enforce
**       branching.
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
 * Add and Init SOA lighting function stubs. Allocate TL_TMP structure
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


BOOL LightIsEnabled(TLLIGHT *pL) {
    return (pL->dwFlags & TLLIGHT_ENABLED);
}

BOOL LightNeedsProcessing(TLLIGHT *pL) {
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
        LData->diffuse.r += pL->Ma_La.red.f[0];
        LData->diffuse.g += pL->Ma_La.green.f[0];
        LData->diffuse.b += pL->Ma_La.blue.f[0];
    }
    else
    {
        //
        // Note:
        // In case ColorVertexAmbient is enabled, note that it uses
        // VertexSpecular instead of VertexDiffuse
        //
        LData->diffuse.r += pL->La.r * LData->pAmbientSrc->r;
        LData->diffuse.g += pL->La.g * LData->pAmbientSrc->g;
        LData->diffuse.b += pL->La.b * LData->pAmbientSrc->b;
    }

    //
    // If no normals are present, bail out since we cannot perform the
    // normal-dependent computations
    //
    if( (dwFVFIn & D3DFVF_NORMAL) == 0 )
    {
        return;
    }

    dot = DotProductToSOA( &pL->direction_in_eye, &in->dvNormal );
    if (FLOAT_GTZ(dot))
    {
        if (!bDoColVertexDiffuse)
        {
            LData->diffuse.r += pL->Md_Ld.red.f[0] * dot;
            LData->diffuse.g += pL->Md_Ld.green.f[0] * dot;
            LData->diffuse.b += pL->Md_Ld.blue.f[0] * dot;
        }
        else
        {
            LData->diffuse.r += pL->Ld.r * LData->pDiffuseSrc->r * dot;
            LData->diffuse.g += pL->Ld.g * LData->pDiffuseSrc->g * dot;
            LData->diffuse.b += pL->Ld.b * LData->pDiffuseSrc->b * dot;
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
            AddVectorFromSOA( &pL->direction_in_eye, &eye, &h );

            // normalize
            Normalize( &h );

            dot = DotProduct( &h, &in->dvNormal );

            if (FLOAT_GTZ(dot))
            {
                if (FLOAT_CMP_POS(dot, >=, LData->specThreshold))
                {
                   #ifdef POWSS
                    D3DVALUE coeff;
                    PowSS( &coeff, &dot, &LData->material.power );
                   #else
                    D3DVALUE coeff = (D3DVALUE) pow( dot, LData->material.power );
                    //D3DVALUE coeff = dot/( LData->material.power - (LData->material.power*dot) +dot);
                   #endif
                    if (!bDoColVertexSpecular)
                    {
                        LData->specular.r += pL->Ms_Ls.red.f[0] * coeff;
                        LData->specular.g += pL->Ms_Ls.green.f[0] * coeff;
                        LData->specular.b += pL->Ms_Ls.blue.f[0] * coeff;
                    }
                    else
                    {
                        LData->specular.r += (pL->Ls.r *
                                             LData->pSpecularSrc->r * coeff);
                        LData->specular.g += (pL->Ls.g *
                                             LData->pSpecularSrc->g * coeff);
                        LData->specular.b += (pL->Ls.b *
                                             LData->pSpecularSrc->b * coeff);
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
    
    SubtractVectorFromSOA( &pL->position_in_eye, &in->dvPosition, (D3DVECTOR *)&d );

    // early out if out of range or exactly on the vertex
#if 0    // The VCPP complier won't aligne locals for ASM use, don't call for now
//#ifdef SSECPP
    SquareMagnitude_m128( &distSquared, &d );
#else
    distSquared = SquareMagnitude( &d );
#endif
    if (FLOAT_CMP_POS(distSquared, >=, pL->range_squared.f[0]) ||
        FLOAT_EQZ(distSquared))
    {
        return;
    }

    //
    // Compute the attenuation
    //
#ifdef SSECPP
    SqrtSS( &dist, &distSquared );
#else
    dist = SQRTF( distSquared );
#endif
    att = pL->Attenuation0.f[0] + pL->Attenuation1.f[0] * dist +
        pL->Attenuation2.f[0] * distSquared;

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
        LData->diffuse.r += att*pL->Ma_La.red.f[0];
        LData->diffuse.g += att*pL->Ma_La.green.f[0];
        LData->diffuse.b += att*pL->Ma_La.blue.f[0];
    }
    else
    {
        //
        // Note:
        // In case ColorVertexAmbient is enabled, note that it uses
        // VertexSpecular instead of VertexDiffuse
        //
        LData->diffuse.r += att*pL->La.r * LData->pAmbientSrc->r;
        LData->diffuse.g += att*pL->La.g * LData->pAmbientSrc->g;
        LData->diffuse.b += att*pL->La.b * LData->pAmbientSrc->b;
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
            LData->diffuse.r += pL->Md_Ld.red.f[0] * dot;
            LData->diffuse.g += pL->Md_Ld.green.f[0] * dot;
            LData->diffuse.b += pL->Md_Ld.blue.f[0] * dot;
        }
        else
        {
            LData->diffuse.r += pL->Ld.r * LData->pDiffuseSrc->r * dot;
            LData->diffuse.g += pL->Ld.g * LData->pDiffuseSrc->g * dot;
            LData->diffuse.b += pL->Ld.b * LData->pDiffuseSrc->b * dot;
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
               #ifdef POWSS
                D3DVALUE coeff;
                PowSS( &coeff, &dot, &LData->material.power );
                coeff *= att;
               #else
                D3DVALUE coeff = (D3DVALUE) pow( dot, LData->material.power ) * att;
                //D3DVALUE coeff =  dot/(LData->material.power - (LData->material.power*dot)+dot ) * att;
               #endif
                if (!bDoColVertexSpecular)
                {
                    LData->specular.r += pL->Ms_Ls.red.f[0] * coeff;
                    LData->specular.g += pL->Ms_Ls.green.f[0] * coeff;
                    LData->specular.b += pL->Ms_Ls.blue.f[0] * coeff;
                }
                else
                {
                    LData->specular.r += (pL->Ls.r *
                                         LData->pSpecularSrc->r * coeff);
                    LData->specular.g += (pL->Ls.g *
                                         LData->pSpecularSrc->g * coeff);
                    LData->specular.b += (pL->Ls.b *
                                         LData->pSpecularSrc->b * coeff);
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
    SubtractVectorFromSOA( &pL->position_in_eye, &in->dvPosition, (D3DVECTOR *)&d );

    // early out if out of range or exactly on the vertex
#if 0 // The VCPP compiler won't align locals for ASM, don't call for now 
//#ifdef SSECPP 
    SquareMagnitude_m128( &distSquared, &d );
#else
    distSquared = SquareMagnitude( &d );
#endif
    if (FLOAT_CMP_POS(distSquared, >=, pL->range_squared.f[0]) ||
        FLOAT_EQZ(distSquared))
    {
        return;
    }

    //
    // Compute the attenuation
    //
#ifdef SSECPP
    SqrtSS( &dist, &distSquared );
#else
    dist = SQRTF( distSquared );
#endif
    att = pL->Attenuation0.f[0] + pL->Attenuation1.f[0] * dist +
        pL->Attenuation2.f[0] * distSquared;

    if (FLOAT_EQZ(att))
      att = (D3DVALUE) FLT_MAX;
    else
      att = (D3DVALUE)1.0/att;
    
    dist = D3DVAL(1)/dist;
    
    // Calc dot product of direction to light with light direction to
    // be compared anganst the cone angles to see if we are in the
    // light.
    // Note that cone_dot is still scaled by dist
    cone_dot = DotProductToSOA2(&d, &pL->direction_in_eye) * dist;
    
    if (FLOAT_CMP_POS(cone_dot, <=, pL->cos_phi_by_2.f[0]))
    {
        return;
    }
    
    // modify att if in the region between phi and theta
    if (FLOAT_CMP_POS(cone_dot, <, pL->cos_theta_by_2.f[0]))
    {
        D3DVALUE val = (cone_dot - pL->cos_phi_by_2.f[0]) *
            pL->inv_theta_minus_phi.f[0];
        
        if (!FLOAT_EQZ( pL->Falloff - (float) 1.0 ))
        {
         #ifdef POWSS
            PowSS( &val, &val, &pL->dvFalloff );
         #else
            //val = val/(pLight->dvFalloff - (pLight->dvFalloff*val) + val );
            val = POWF( val, pL->Falloff );
         #endif
        }
        att *= val;
    }

    //
    // Add the material's ambient component
    //
    if (!bDoColVertexAmbient)
    {
        LData->diffuse.r += att*pL->Ma_La.red.f[0];
        LData->diffuse.g += att*pL->Ma_La.green.f[0];
        LData->diffuse.b += att*pL->Ma_La.blue.f[0];
    }
    else
    {
        //
        // Note:
        // In case ColorVertexAmbient is enabled, note that it uses
        // VertexSpecular instead of VertexDiffuse
        //
        LData->diffuse.r += att*pL->La.r * LData->pAmbientSrc->r;
        LData->diffuse.g += att*pL->La.g * LData->pAmbientSrc->g;
        LData->diffuse.b += att*pL->La.b * LData->pAmbientSrc->b;
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
            LData->diffuse.r += pL->Md_Ld.red.f[0] * dot;
            LData->diffuse.g += pL->Md_Ld.green.f[0] * dot;
            LData->diffuse.b += pL->Md_Ld.blue.f[0] * dot;
        }
        else
        {
            LData->diffuse.r += pL->Ld.r * LData->pDiffuseSrc->r * dot;
            LData->diffuse.g += pL->Ld.g * LData->pDiffuseSrc->g * dot;
            LData->diffuse.b += pL->Ld.b * LData->pDiffuseSrc->b * dot;
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
               #ifdef POWSS
                D3DVALUE coeff;
                PowSS( &coeff, &dot, &LData->material.power );
                coeff *= att;
               #else
                D3DVALUE coeff = (D3DVALUE) pow( dot, LData->material.power ) * att;
                //D3DVALUE coeff =  dot/(LData->material.power - (LData->material.power*dot)+dot ) * att;
               #endif
                if (!bDoColVertexSpecular)
                {
                    LData->specular.r += pL->Ms_Ls.red.f[0] * coeff;
                    LData->specular.g += pL->Ms_Ls.green.f[0] * coeff;
                    LData->specular.b += pL->Ms_Ls.blue.f[0] * coeff;
                }
                else
                {
                    LData->specular.r += (pL->Ls.r *
                                         LData->pSpecularSrc->r * coeff);
                    LData->specular.g += (pL->Ls.g *
                                         LData->pSpecularSrc->g * coeff);
                    LData->specular.b += (pL->Ls.b *
                                         LData->pSpecularSrc->b * coeff);
                }
            }
        }
    }
    return;
}

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
#ifdef SSECPP
    __declspec(align(16)) D3DVECTOR d;    // Direction to light
#else
    D3DVECTOR d;    // Direction to light
#endif
    D3DVALUE att;
    D3DVALUE dist;
    D3DVALUE dot;
    D3DVALUE distSquared;
    
    SubtractVectorFromSOA( &pL->position_in_eye, &in->dvPosition, (D3DVECTOR *)&d );

    // early out if out of range or exactly on the vertex

#if 0  // The VCPP compiler won't align locals for ASM use, for now, don't call this code
//#ifdef SSECPP
    SquareMagnitude_m128( &distSquared, &d );
#else
    distSquared = SquareMagnitude( &d );
#endif
    if (FLOAT_CMP_POS(distSquared, >=, pL->range_squared.f[0]) ||
        FLOAT_EQZ(distSquared))
    {
        return;
    }

    //
    // Compute the attenuation
    //
#ifdef SSECPP
    SqrtSS( &dist, &distSquared );
#else
    dist = SQRTF( distSquared );
#endif
    att = pL->Attenuation0.f[0] + pL->Attenuation1.f[0] * dist +
        pL->Attenuation2.f[0] * distSquared;

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
        D3DVALUE cone_dot = DotProductToSOA2(&d, &pL->direction_in_eye) * dist;
        
        if (FLOAT_CMP_POS(cone_dot, <=, pL->cos_phi_by_2.f[0]))
        {
            return;
        }
        
        // modify att if in the region between phi and theta
        if (FLOAT_CMP_POS(cone_dot, <, pL->cos_theta_by_2.f[0]))
        {
            D3DVALUE val = (cone_dot - pL->cos_phi_by_2.f[0]) *
                pL->inv_theta_minus_phi.f[0];
            
            if (!FLOAT_EQZ( pL->Falloff - (float) 1.0 ))
            {
               #ifdef POWSS
                PowSS( &val, &val, &pL->Falloff );
               #else
                //val = val/(pLight->dvFalloff - (pLight->dvFalloff*val) + val );
                val = POWF( val, pL->Falloff );
               #endif
            }
            att *= val;
        }
    }

    //
    // Add the material's ambient component
    //
    if (!bDoColVertexAmbient)
    {
        LData->diffuse.r += att*pL->Ma_La.red.f[0];
        LData->diffuse.g += att*pL->Ma_La.green.f[0];
        LData->diffuse.b += att*pL->Ma_La.blue.f[0];
    }
    else
    {
        //
        // Note:
        // In case ColorVertexAmbient is enabled, note that it uses
        // VertexSpecular instead of VertexDiffuse
        //
        LData->diffuse.r += att*pL->La.r * LData->pAmbientSrc->r;
        LData->diffuse.g += att*pL->La.g * LData->pAmbientSrc->g;
        LData->diffuse.b += att*pL->La.b * LData->pAmbientSrc->b;
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
            LData->diffuse.r += pL->Md_Ld.red.f[0] * dot;
            LData->diffuse.g += pL->Md_Ld.green.f[0] * dot;
            LData->diffuse.b += pL->Md_Ld.blue.f[0] * dot;
        }
        else
        {
            LData->diffuse.r += pL->Ld.r * LData->pDiffuseSrc->r * dot;
            LData->diffuse.g += pL->Ld.g * LData->pDiffuseSrc->g * dot;
            LData->diffuse.b += pL->Ld.b * LData->pDiffuseSrc->b * dot;
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
               #ifdef POWSS
                D3DVALUE coeff;
                PowSS( &coeff, &dot, &LData->material.power );
                coeff *= att;
               #else
                D3DVALUE coeff = (D3DVALUE) pow( dot, LData->material.power ) * att;
                //D3DVALUE coeff =  dot/(LData->material.power - (LData->material.power*dot)+dot ) * att;
               #endif
                if (!bDoColVertexSpecular)
                {
                    LData->specular.r += pL->Ms_Ls.red.f[0] * coeff;
                    LData->specular.g += pL->Ms_Ls.green.f[0] * coeff;
                    LData->specular.b += pL->Ms_Ls.blue.f[0] * coeff;
                }
                else
                {
                    LData->specular.r += (pL->Ls.r *
                                         LData->pSpecularSrc->r * coeff);
                    LData->specular.g += (pL->Ls.g *
                                         LData->pSpecularSrc->g * coeff);
                    LData->specular.b += (pL->Ls.b *
                                         LData->pSpecularSrc->b * coeff);
                }
            }
        }
    }
    return;
}

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
Information:    
Return:         
-------------------------------------------------------------------*/

void InitializeLight(TLLIGHT *pL)
{
  pL->dwFlags = TLLIGHT_NEEDSPROCESSING;
  pL->Next = NULL;

  ZeroMemory(&pL->position_in_eye, sizeof(pL->position_in_eye));
  ZeroMemory(&pL->direction_in_eye, sizeof(pL->direction_in_eye));
  ZeroMemory(&pL->La, sizeof(pL->La));
  ZeroMemory(&pL->Ld, sizeof(pL->Ld));
  ZeroMemory(&pL->Ls, sizeof(pL->Ls));
  ZeroMemory(&pL->Ma_La, sizeof(pL->Ma_La));
  ZeroMemory(&pL->Md_Ld, sizeof(pL->Md_Ld));
  ZeroMemory(&pL->Ms_Ls, sizeof(pL->Ms_Ls));
  ZeroMemory(&pL->Attenuation0, sizeof(pL->Attenuation0));
  ZeroMemory(&pL->Attenuation1, sizeof(pL->Attenuation1));
  ZeroMemory(&pL->Attenuation2, sizeof(pL->Attenuation2));
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
                RC *pRC -- 
                TLLIGHT *pL -- pointer to the light
                LPD3DLIGHT7 pLight -- Light data to set
Information:    
Return:         
-------------------------------------------------------------------*/

HRESULT SetLight(RC *pRC, TLLIGHT *pL, LPD3DLIGHT7 pLight)
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
    pL->Attenuation0.f[0] = pLight->dvAttenuation0;
    pL->Attenuation1.f[0] = pLight->dvAttenuation1;
    pL->Attenuation2.f[0] = pLight->dvAttenuation2;

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
void ProcessLight(TLLIGHT *pL, D3DMATERIAL7 *mat, TLLIGHTVERTEX_FUNC_TABLE *pTbl)
{
  extern TLLIGHT_SOA_FUNC_TABLE SOA_Light_fns;
  DWORD diff;
  //
  // If it is already processed, return
  //
  if (!LightNeedsProcessing(pL)) return;

  //
  // Material Ambient times Light Ambient
  //
  pL->Ma_La.red.f[0] = pL->La.r * mat->ambient.r * (D3DVALUE)(255.0);
  pL->Ma_La.green.f[0] = pL->La.g * mat->ambient.g * (D3DVALUE)(255.0);
  pL->Ma_La.blue.f[0] = pL->La.b * mat->ambient.b * (D3DVALUE)(255.0);

  SwizzleSOAFLOAT(&pL->Ma_La.red);
  SwizzleSOAFLOAT(&pL->Ma_La.green);
  SwizzleSOAFLOAT(&pL->Ma_La.blue);

  // Material Diffuse times Light Diffuse
  //
  pL->Md_Ld.red.f[0] = pL->Ld.r * mat->diffuse.r * (D3DVALUE)(255.0);
  pL->Md_Ld.green.f[0] = pL->Ld.g * mat->diffuse.g * (D3DVALUE)(255.0);
  pL->Md_Ld.blue.f[0] = pL->Ld.b * mat->diffuse.b * (D3DVALUE)(255.0);

  SwizzleSOAFLOAT(&pL->Md_Ld.red);
  SwizzleSOAFLOAT(&pL->Md_Ld.green);
  SwizzleSOAFLOAT(&pL->Md_Ld.blue);

  //
  // Material Specular times Light Specular
  //
  pL->Ms_Ls.red.f[0] = pL->Ls.r * mat->specular.r * (D3DVALUE)(255.0);
  pL->Ms_Ls.green.f[0] = pL->Ls.g * mat->specular.g * (D3DVALUE)(255.0);
  pL->Ms_Ls.blue.f[0] = pL->Ls.b * mat->specular.b * (D3DVALUE)(255.0);

  SwizzleSOAFLOAT(&pL->Ms_Ls.red);
  SwizzleSOAFLOAT(&pL->Ms_Ls.green);
  SwizzleSOAFLOAT(&pL->Ms_Ls.blue);
  SwizzleSOAFLOAT(&pL->Attenuation0);
  SwizzleSOAFLOAT(&pL->Attenuation1);
  SwizzleSOAFLOAT(&pL->Attenuation2);

  //
  // Assign the actual lighting function pointer, in addition to
  // performing some precomputation of light-type specific data
  //
  pL->pfnLightVertex = NULL;
  switch (pL->dltType)
  {
  case D3DLIGHT_DIRECTIONAL:
    pL->pfnLightVertex = pTbl->pfnDirectional;
    pL->pfnLightVertexSOA = pTbl->pfnDirectionalSOA;
    pL->pfnLightVertexSOA_Diff = SOA_Light_fns.pfnDirectionalSOA_D[0];
    pL->pfnLightVertexSOA_Spec = SOA_Light_fns.pfnDirectionalSOA_S[0];
    break;
  case D3DLIGHT_POINT:
    pL->range_squared.f[0] = pL->Range * pL->Range;
    SwizzleSOAFLOAT(&pL->range_squared);

    pL->inv_theta_minus_phi.f[0] = 1.0f;
    SwizzleSOAFLOAT(&pL->inv_theta_minus_phi);

    pL->pfnLightVertex = pTbl->pfnPoint;
    pL->pfnLightVertexSOA = pTbl->pfnPointSOA;
    pL->pfnLightVertexSOA_Diff = SOA_Light_fns.pfnPointSOA_D[0];
    pL->pfnLightVertexSOA_Spec = SOA_Light_fns.pfnPointSOA_S[0];
    break;
  case D3DLIGHT_SPOT:
    pL->range_squared.f[0] = pL->Range * pL->Range;
    SwizzleSOAFLOAT(&pL->range_squared);

    pL->cos_theta_by_2.f[0] = (float)cos(pL->Theta / 2.0);
    SwizzleSOAFLOAT(&pL->cos_theta_by_2);

    pL->cos_phi_by_2.f[0] = (float)cos(pL->Phi / 2.0);
    SwizzleSOAFLOAT(&pL->cos_phi_by_2);

    pL->inv_theta_minus_phi.f[0] = pL->cos_theta_by_2.f[0] -
        pL->cos_phi_by_2.f[0];
    if (pL->inv_theta_minus_phi.f[0] != 0.0)
    {
      pL->inv_theta_minus_phi.f[0] = 1.0f/pL->inv_theta_minus_phi.f[0];
    }
    else
    {
      pL->inv_theta_minus_phi.f[0] = 1.0f;
    }
    SwizzleSOAFLOAT(&pL->inv_theta_minus_phi);
    pL->pfnLightVertex = pTbl->pfnSpot;
    pL->pfnLightVertexSOA = pTbl->pfnSpotSOA;

    // Compute which diffuse case to use.
    if (pL->Attenuation0.f[0] == 1.0 && pL->Attenuation1.f[0] == 0.0 && pL->Attenuation2.f[0] == 0.0 && pL->Falloff == 1.0) {
      diff = 0;      
    }
    else  {
      if (pL->Falloff == 1.0)
        diff = 1;
      else 
        diff = 2;
    }

    pL->pfnLightVertexSOA_Diff = SOA_Light_fns.pfnSpotSOA_D[diff];
    pL->pfnLightVertexSOA_Spec = SOA_Light_fns.pfnSpotSOA_S[0];
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


Return:         
-------------------------------------------------------------------*/

void LightEnable(TLLIGHT *pL, RC *pRc)
{
  TLLIGHT *pTmp;

  // Assert that it is not already enabled
  if (LightIsEnabled(pL)) return;


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

  return;
}

/*-------------------------------------------------------------------
Function Name:  LightDisable
Description:    Disable a light in the light array
Parameters:   
                TLLIGHT *pL -- Pointer to the light to enable
Information:    
                Disable a light. This clears the enable bit, and moves
                the light out of the active light list.
Return:         
-------------------------------------------------------------------*/

void LightDisable(TLLIGHT *pL, RC *pRc)
{
  TLLIGHT *pLightPrev;
  // Assert that the light is enabled
  if (!LightIsEnabled(pL)) return;

  pLightPrev = pRc->tl.lighting.pActiveLights;

  // If this is the first light in the active list
  if (pLightPrev == pL)
  {
    pRc->tl.lighting.pActiveLights = pL->Next;
    pL->dwFlags &= ~TLLIGHT_ENABLED;
    pRc->tl.lighting.dwNumActiveLights--;
    return;
  }

  while (pLightPrev->Next != pL)
  {
    // Though this light was marked as enabled, it is not on
    // the active list. Assert this.
    if (pLightPrev->Next == NULL)
    {
      pL->dwFlags &= ~TLLIGHT_ENABLED;
      return;
    }

    // Else get the next pointer
    pLightPrev = pLightPrev->Next;
  }

  pLightPrev->Next = pL->Next;
  pRc->tl.lighting.dwNumActiveLights--;
  pL->dwFlags &= ~TLLIGHT_ENABLED;
  return;
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
    XformBy4x3ToSOA(&pL->Position, mView, &pL->position_in_eye);
    SwizzleVec3(&pL->position_in_eye);
  }

  if (pL->dltType != D3DLIGHT_POINT)
  {
    // Transform light direction to the eye space
    Xform3VecBy3x3ToSOA( &pL->Direction, mView,
        &pL->direction_in_eye );
    // Normalize it
    NormalizeToSOA( &pL->direction_in_eye );

    // Reverse it such that the direction is to the light
    ReverseVectorToSOA( &pL->direction_in_eye, &pL->direction_in_eye );
    SwizzleVec3(&pL->direction_in_eye);
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
  extern TLLIGHTFN pLight_DS[TLMAX_LIGHTS];
  extern TLLIGHTFN pLight_D[TLMAX_LIGHTS];
  extern TLLIGHTFN pLight_Old[TLMAX_LIGHTS];
  HRESULT hr = D3D_OK;
  TLLIGHTING *Ldata = &pRc->tl.lighting;
  TLLIGHT *pLight = pRc->tl.lighting.pActiveLights;
  TL_TMP *pTL = pRc->tl.pTL;
  D3DMATERIAL7 *mat = &pRc->tl.Material;
  TLLIGHT **pLA = pRc->tl.lighting.pLightArray;
  int i;

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
    Ldata->material = *mat;
    ScaleRGBColorTo255( &mat->ambient, &Ldata->matAmb );
    ScaleRGBColorTo255( &mat->diffuse, &Ldata->matDiff );
    ScaleRGBColorTo255( &mat->specular, &Ldata->matSpec );
    ScaleRGBColorTo255( &mat->emissive, &Ldata->matEmis );


#ifdef __MAT_PROFILE
    {

      // Really horrible, quick material specular exp. change profiling code
      int i = 0;
      if (!printpow)
      {
        for (i = 0; i < num_pow; i++)
        {
          if (mat->power == power[i])
          {
            count[i]++;
            goto done_count;
          }            
        }
        count[num_pow] = 1;
        power[num_pow] = mat->power;
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

#endif

    //
    // Compute the Material Diffuse Alpha
    //
    Ldata->materialDiffAlpha = (DWORD) (mat->diffuse.a * (D3DVALUE)(255));
    if (Ldata->materialDiffAlpha < 0)
      Ldata->materialDiffAlpha = 0;
    else if (Ldata->materialDiffAlpha > 255)
      Ldata->materialDiffAlpha = 255 << 24;
    else Ldata->materialDiffAlpha <<= 24;

    pRc->tl.pTL->dMatDiffAlpha.d[0] = Ldata->materialDiffAlpha;
    SwizzleSOADWORD(&pRc->tl.pTL->dMatDiffAlpha);
      
    //
    // Compute the Material Specular Alpha
    //
    Ldata->materialSpecAlpha = (DWORD) (mat->specular.a * (D3DVALUE)(255));
    if (Ldata->materialSpecAlpha < 0)
      Ldata->materialSpecAlpha = 0;
    else if (Ldata->materialSpecAlpha > 255)
      Ldata->materialSpecAlpha = 255 << 24;
    else Ldata->materialSpecAlpha <<= 24;

    pRc->tl.pTL->dMatSpecAlpha.d[0] = Ldata->materialSpecAlpha;
    SwizzleSOADWORD(&pRc->tl.pTL->dMatSpecAlpha);
      

    pRc->tl.pTL->fMatSpecAlpha.f[0] = (float)(Ldata->materialSpecAlpha >> 24);
    SwizzleSOAFLOAT(&pRc->tl.pTL->fMatSpecAlpha);


    //
    // Precompute the ambient and emissive components that are
    // not dependent on any contribution by the lights themselves
    //
    Ldata->ambEmiss.r = Ldata->ambient_red   * Ldata->matAmb.r +
        Ldata->matEmis.r;
    Ldata->ambEmiss.g = Ldata->ambient_green * Ldata->matAmb.g +
        Ldata->matEmis.g;
    Ldata->ambEmiss.b = Ldata->ambient_blue  * Ldata->matAmb.b +
        Ldata->matEmis.b;

    // Create a copy of the ambEmiss components for use by the SOA code
    for (i = 0; i < SOA_SIZE; i++ )
    {
      pTL->fambEmiss.red.f[i] = Ldata->ambEmiss.r;
      pTL->fambEmiss.green.f[i] = Ldata->ambEmiss.g;
      pTL->fambEmiss.blue.f[i] = Ldata->ambEmiss.b;
      pTL->fambEmiss.alpha.f[i] = 0.0;
    }

    //
    // If the dot product is less than this
    // value, specular factor is zero
    //
    if (mat->power > (D3DVALUE)0.001)
    {
      Ldata->specThreshold = (D3DVALUE)pow(0.001, 1.0/mat->power);
    }

    // Update the Power function for specular components of lighting
    Ldata->KniPowerFunction = UpdateKniPowerFunctionPtr(mat->power);
  }

  while (pLight)
  {
    if ((pRc->tl.dwDirtyFlags & TLPV_DIRTY_MATERIAL) ||
        LightNeedsProcessing(pLight))
    {
      // If the material is dirty, light needs processing, regardless
      if (pRc->tl.dwDirtyFlags & TLPV_DIRTY_MATERIAL)
      {
        pLight->dwFlags |= TLLIGHT_NEEDSPROCESSING;
        pLight->dwFlags &= ~TLLIGHT_READY;
      }

      // If the light has been set, or some material paramenters
      // changed, re-process the light.
      ProcessLight( pLight, &pRc->tl.Material, &pRc->tl.LightVertexTable );

      // Transform the light to Eye space
      // Lights are defined in world space, so simply apply the
      // Viewing transform
      XformLight(pLight, &pRc->tl.xfmView );

    }
    else if (pRc->tl.dwDirtyFlags & TLPV_DIRTY_NEEDXFMLIGHT)
    {
      XformLight( pLight, &pRc->tl.xfmView );
    }

    pLight = pLight->Next;
  }

  // Support for array of lighting functions
  pLight = pRc->tl.lighting.pActiveLights;

  while (pLight)  {
    if (pLight->dwFlags & TLLIGHT_READY)  {
      *pLA = pLight;
      pLA++;
    }
    pLight = pLight->Next;
  }

  // Choose between Diffuse Only or Diffuse + Specular
#define SOA_NEW_LIGHT 1
#if SOA_NEW_LIGHT
  if ((pRc->tl.dwTLState & TLPV_DOSPECULAR) && (pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL))
    Ldata->pfnLightVertexSOA = pLight_DS[Ldata->dwNumActiveLights];
  else
    Ldata->pfnLightVertexSOA = pLight_D[Ldata->dwNumActiveLights];
#else
    Ldata->pfnLightVertexSOA = pLight_Old[Ldata->dwNumActiveLights];
#endif

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
    pRc->tl.lighting.fog_factor = D3DVAL(255) / (pRc->tl.lighting.fog_end -
    pRc->tl.lighting.fog_start);

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
                pRc->tl.lighting.diffuse and pRc->tl.lighting.specular,
                and stored in 
                pRc->tl.lighting.outDiffuse and pRc->tl.lighting.outSpecular
Return:         
-------------------------------------------------------------------*/

void LightVertex(RC* pRc, D3DLIGHTINGELEMENT *pLE)
{
    TLLIGHTING *Ldata = &pRc->tl.lighting;
    TLLIGHT  *pLight;
    int r, g, b;
    DWORD a;

    //
    // Initialize Diffuse color with the Ambient and Emissive component
    // independent of the light (Ma*La + Me)
    //

    if (pRc->tl.dwTLState & (TLPV_COLORVERTEXEMIS | TLPV_COLORVERTEXAMB))
    {
        // If the material values need to be replaced, compute

        Ldata->diffuse.r = Ldata->ambient_red * Ldata->pAmbientSrc->r +
            Ldata->pEmissiveSrc->r;
        Ldata->diffuse.g = Ldata->ambient_green * Ldata->pAmbientSrc->g +
            Ldata->pEmissiveSrc->g;
        Ldata->diffuse.b = Ldata->ambient_blue  * Ldata->pAmbientSrc->b +
            Ldata->pEmissiveSrc->b;
    }
    else
    {
        // If none of the material values needs to be replaced

        Ldata->diffuse = Ldata->ambEmiss;
    }


    //
    // Initialize the Specular to Zero
    //
    Ldata->specular.r = (D3DVALUE)(0);
    Ldata->specular.g = (D3DVALUE)(0);
    Ldata->specular.b = (D3DVALUE)(0);

    //
    // In a loop accumulate color from the activated lights
    //
    pLight = Ldata->pActiveLights;

    // Light 0
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc,
            pLight,
            pLE);

      
      pLight = pLight->Next;
    }
    else 
    {
      goto LightFinished;
    }


    // Light 1
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc,
          pLight,
          pLE);

      
      pLight = pLight->Next;
    }
    else 
    {
      goto LightFinished;
    }

    // Light 2
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc,
          pLight,
          pLE);

      
      pLight = pLight->Next;
    }
    else 
    {
      goto LightFinished;
    }

    // Light 3
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc,
            pLight,
            pLE);

      
      pLight = pLight->Next;
    }
    else 
    {
      goto LightFinished;
    }


    // Light 4
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc,
            pLight,
            pLE);

      
      pLight = pLight->Next;
    }
    else 
    {
      goto LightFinished;
    }

    // Light 5
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc,
            pLight,
            pLE);

      
      pLight = pLight->Next;
    }
    else 
    {
      goto LightFinished;
    }

    // Light 6
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc,
            pLight,
            pLE);

      
      pLight = pLight->Next;
    }
    else 
    {
      goto LightFinished;
    }

    // Light 7
    if (pLight) 
    {
      if (pLight->dwFlags & TLLIGHT_READY)
        pLight->pfnLightVertex(pRc,
            pLight,
            pLE);

      
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
    r = FTOI(Ldata->diffuse.r);
    g = FTOI(Ldata->diffuse.g);
    b = FTOI(Ldata->diffuse.b);
    a = *Ldata->pDiffuseAlphaSrc;

    //
    // Clamp the r, g, b, components
    //
    if (r < 0) r = 0; else if (r > 255) r = 255;
    if (g < 0) g = 0; else if (g > 255) g = 255;
    if (b < 0) b = 0; else if (b > 255) b = 255;

    Ldata->outDiffuse =  a + (r<<16) + (g<<8) + b;

    //
    // Obtain the specular Alpha
    //
    a = *(Ldata->pSpecularAlphaSrc);
    
    //
    // Compute the RGB part of the specular color
    //
    if (pRc->tl.dwTLState & TLPV_DOSPECULAR)
    {
        r = FTOI(Ldata->specular.r);
        g = FTOI(Ldata->specular.g);
        b = FTOI(Ldata->specular.b);

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

    Ldata->outSpecular =  a + (r<<16) + (g<<8) + b;

    return;
}

/*-------------------------------------------------------------------
Function Name:  FogVertex
Description:    Performs fogging calculation on the input vertex
                Alpha component of pv->lighting.outSpecular is set
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
           #ifdef SSECPP
           D3DVALUE tmpSquare;
            tmpSquare = (pLE->dvPosition.x*pLE->dvPosition.x +
                         pLE->dvPosition.y*pLE->dvPosition.y +
                         pLE->dvPosition.z*pLE->dvPosition.z); 
            SqrtSS( &dist, &tmpSquare);
           #else
            dist = SQRTF(pLE->dvPosition.x*pLE->dvPosition.x +
                         pLE->dvPosition.y*pLE->dvPosition.y +
                         pLE->dvPosition.z*pLE->dvPosition.z);
           #endif
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
      #ifdef SSECPP
        D3DVALUE tmpSquare;
      #endif

        for( j=0; j<=numVertexBlends; j++)
        {
            float blend;
            
            if( numVertexBlends == 0 )
            {
                blend = 1.0f;
            }
            else if( j == numVertexBlends )
            {
                blend = 1.0f - cumulBlend;
            }
            else
            {
                blend = pBlendFactors[j];
            }
                
            cumulBlend += pBlendFactors[j];

            x += (v->x*pRc->tl.lpxfmToEye[j]->_11 + 
                  v->y*pRc->tl.lpxfmToEye[j]->_21 + 
                  v->z*pRc->tl.lpxfmToEye[j]->_31 + 
                  pRc->tl.lpxfmToEye[j]->_41) * blend;
            y += (v->x*pRc->tl.lpxfmToEye[j]->_12 + 
                  v->y*pRc->tl.lpxfmToEye[j]->_22 + 
                  v->z*pRc->tl.lpxfmToEye[j]->_32 + 
                  pRc->tl.lpxfmToEye[j]->_42) * blend;
            z += (v->x*pRc->tl.lpxfmToEye[j]->_13 + 
                  v->y*pRc->tl.lpxfmToEye[j]->_23 + 
                  v->z*pRc->tl.lpxfmToEye[j]->_33 + 
                  pRc->tl.lpxfmToEye[j]->_43) * blend;
        } 
        
       #ifdef SSECPP
        tmpSquare = (x*x + y*y + z*z);
        SqrtSS( &dist, &tmpSquare );
       #else
        dist = SQRTF(x*x + y*y + z*z);
       #endif
    }
    else
    {
        float cumulBlend = 0.0f;

        for( j=0; j<=numVertexBlends; j++)
        {
            float blend;
            
            if( numVertexBlends == 0 )
            {
                blend = 1.0f;
            }
            else if( j == numVertexBlends )
            {
                blend = 1.0f - cumulBlend;
            }
            else
            {
                blend = pBlendFactors[j];
            }
                
            cumulBlend += pBlendFactors[j];

            dist += (v->x*pRc->tl.lpxfmToEye[j]->_13 + 
                  v->y*pRc->tl.lpxfmToEye[j]->_23 + 
                  v->z*pRc->tl.lpxfmToEye[j]->_33 + 
                  pRc->tl.lpxfmToEye[j]->_43) * blend;
        } 
    }

    if (pRc->tl.lighting.fog_mode == D3DFOG_LINEAR)
    {
        if (dist < pRc->tl.lighting.fog_start)
        {
            RRPV_SET_ALPHA(pRc->tl.lighting.outSpecular, 255);
        }
        else if (dist >= pRc->tl.lighting.fog_end)
        {
            RRPV_SET_ALPHA(pRc->tl.lighting.outSpecular, 0);
        }
        else
        {
            D3DVALUE v = (pRc->tl.lighting.fog_end - dist) * pRc->tl.lighting.fog_factor;
            int f = FTOI(v);
            RRPV_SET_ALPHA(pRc->tl.lighting.outSpecular, f);
        }
    }
    else
    {
        int f;
        D3DVALUE tmp = dist * pRc->tl.lighting.fog_density;
        if (pRc->tl.lighting.fog_mode == D3DFOG_EXP2)
        {
            tmp *= tmp;
        }
        tmp = (D3DVALUE)exp(-tmp) * 255.0f;
        f = FTOI(tmp);
        RRPV_SET_ALPHA( pRc->tl.lighting.outSpecular, f )
    }

    return;
}
#endif
#endif













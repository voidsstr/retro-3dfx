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
** File name: tnldp2.c
**
** Description: DrawPrimitives2 function for Software T&L HAL
**
** $Revision: 11$
** $Date: 10/11/00 8:45:17 PM$
**
** $Log: 
**  11   3dfx      1.9.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  10   Napalm Shared1.9         03/31/00 Scott Kephart   Fix for 3D Mark game 2
**       bug - color vertex mode was inadvertantly enabled.
**  9    Napalm Shared1.8         03/17/00 Scott Kephart   Added support for Visual
**       C++ processor pack Beta
**  8    Napalm Shared1.7         03/01/00 Allen Hansen    Fixed vertex fog for T&L
**  7    Napalm Shared1.6         02/15/00 Scott Kephart   Updates to align TLLIGHT
**       structures on 32 byte boundaries
**  6    Napalm Shared1.5         01/28/00 Scott Kephart   Big T&L Merge: added T&L
**       renderstate code
** 
**  5    Napalm Shared1.4         01/10/00 Scott Kephart   Updates to lighting.
**       Split point and spot light functions. Only allow 8 active lights to reduce
**       branch mispredictions. Cleanup in lighting.
**  4    Napalm Shared1.3         12/21/99 Scott Kephart   Fix for bug in
**       D3DDP2OP_CREATELIGHT -- new light array entries were not initialized,
**       resulting in bad things happening after multiple runs of 3D Winbench 2000.
**  3    Napalm Shared1.2         10/27/99 Russ Lind       added defines of
**       D3DHAL_SETLIGHT_ ENABLE, DISABLE & DATA if they aren't already defined
**       cast changes, etc so the TnL_HAL code builds for w2k
**  2    Napalm Shared1.1         10/26/99 Scott Kephart   removed #include
**       d7fvfext.h
**  1    Napalm Shared1.0         10/25/99 Scott Kephart   
** $
 * 
 * 1     1/19/00 8:44a Skephart
 * H5TOT Before the Napalm code freeze
** 
** 27    10/26/99 12:47a Skephart
** Remove extraneous command parsing code
** 
** 26    10/08/99 1:02a Skephart
** Kill tnldp2.c!
** 
** 25    10/07/99 4:50p Skephart
** Merged in more clip changes from Bob
** 
** 24    10/06/99 11:47p Skephart
** Merged in Bob's clipping framework
** 
** 23    10/01/99 2:33p Skephart
** 
** 22    9/30/99 1:25p Skephart
** Fixes for DirectX 7 multibuffering problem PRS #8782
** 
** 21    9/29/99 9:19p Skephart
** Trivial accept / reject logic from BobJ
** 
** 19    9/28/99 5:30p Skephart
** 
** 18    9/28/99 3:13p Skephart
** Fixes from BobJ
** 
** 17    9/27/99 8:32p Skephart
** Move render context variables for TL HAL out of d3global.h, 
** and into tlglobal.h. All TL related variables are gathered under 
** pRc->tl.<variable>
** 
** 16    9/27/99 7:36p Skephart
** Updated header
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

// The w2k headers are missing these!
#ifndef D3DHAL_SETLIGHT_ENABLE
#define D3DHAL_SETLIGHT_ENABLE      0
#endif
#ifndef D3DHAL_SETLIGHT_DISABLE
#define D3DHAL_SETLIGHT_DISABLE     1
#endif
#ifndef D3DHAL_SETLIGHT_DATA
#define D3DHAL_SETLIGHT_DATA        2
#endif

/*-------------------------------------------------------------------
Function Name:  DP2TL_SetLight
Description:    Perform the Draw Primitives 2 Set Light function
Parameters:     
                RC *pRc -- pointer to the current rendering context
                LPD3DHAL_DP2COMMAND pCmd -- command stream pointer
                LPDWORD extra -- pointer to the number of extra bytes used
                in the command. The SETLIGHT_ENABLE and SETLIGHT_DISABLE
                commands use a struct D3DHAL_DP2SETLIGHT as data. The SET_LIGHT_DATA
                command also includes a D3DLIGHT7 structure. extra returns the number of bytes
                these additional D3DLIGHT7 structures take in the command stream.
Information:    
Return:         
-------------------------------------------------------------------*/
HRESULT DP2TL_SetLight(RC *pRc, LPD3DHAL_DP2COMMAND pCmd, LPDWORD extra)
{
  HRESULT hr = D3D_OK;
  WORD wNumSetLight = pCmd->wStateCount;
  D3DHAL_DP2SETLIGHT *pSetLight = (D3DHAL_DP2SETLIGHT *)(pCmd + 1);
  D3DLIGHT7 *pLightData = NULL;
  int i;

//  _ASSERT( pdwStride != NULL, "pdwStride is Null" );

  for (i = 0; i < wNumSetLight; i++ )
  {
    DWORD dwStride = sizeof(D3DHAL_DP2SETLIGHT);
    DWORD dwIndex = pSetLight->dwIndex;

    // Assert that create was not called here
//    _ASSERTf(dwIndex < m_dwLightArraySize,
//            ( "Create was not called prior to the SetLight for light %d",
//            dwIndex ));

#ifdef WINNT
    switch (pSetLight->lightData)
#else
    switch (pSetLight->dwDataType)
#endif
    {
    case D3DHAL_SETLIGHT_ENABLE:
        LightEnable(&pRc->tl.pLightArray[dwIndex], pRc);
        pRc->tl.dwDirtyFlags |= TLPV_DIRTY_SETLIGHT;
        break;
    case D3DHAL_SETLIGHT_DISABLE:
        LightDisable(&pRc->tl.pLightArray[dwIndex], pRc);
        break;
    case D3DHAL_SETLIGHT_DATA:
        pLightData = (D3DLIGHT7 *)((LPBYTE)pSetLight + dwStride);
        dwStride += sizeof(D3DLIGHT7);
        *extra += sizeof(D3DLIGHT7);
        HR_RET(SetLight(pRc, &pRc->tl.pLightArray[pSetLight->dwIndex], pLightData));
        pRc->tl.dwDirtyFlags |= TLPV_DIRTY_SETLIGHT;
        break;
    default:
//        DPFM(0,TNL,("Unknown SetLight command"));
        hr = DDERR_INVALIDPARAMS;
    }

    // Update the command buffer pointer
    pSetLight = (D3DHAL_DP2SETLIGHT *)((LPBYTE)pSetLight +
                                     dwStride);

  }

  return hr;
}


HRESULT DP2TL_CreateLight(RC *pRc, LPD3DHAL_DP2COMMAND pCmd)
{
    WORD wNumCreateLight = pCmd->wStateCount;
    D3DHAL_DP2CREATELIGHT *pCreateLight = (D3DHAL_DP2CREATELIGHT *)(pCmd + 1);
    HRESULT hr = D3D_OK;
    int i;

    for (i = 0; i < wNumCreateLight; i++, pCreateLight++)
    {
        // If the index is not already allocated, grow the light array
        // by REF_LIGHTARRAY_GROWTH_SIZE
        if (pCreateLight->dwIndex >= pRc->tl.dwLightArraySize)
        {
            HR_RET(GrowLightArray(pRc, pCreateLight->dwIndex));
        }
    }

    return hr;
}

#define ARRAYGROW_DELTA     32 // Should be a power of 2

HRESULT GrowLightArray(RC *pRc, const DWORD dwIndex)
{
    NT9XDEVICEDATA *ppdev = pRc->ppdev;
    // Allocate a few extra in anticipation of more light being used in the
    // future
    DWORD dwNewArraySize = dwIndex+16;
    TLLIGHT *pTmpActiveLights = NULL;
    TLLIGHT *pTmpLightArray = NULL;
    TLLIGHT *pTmpLightArrayAlloc = NULL;
    DWORD i;

    pTmpLightArrayAlloc = DXMALLOC(sizeof(TLLIGHT)*dwNewArraySize+0x1f);

    if (pTmpLightArrayAlloc == NULL)
        return DDERR_OUTOFMEMORY;

    pTmpLightArray = (TLLIGHT *) ((((DWORD)pTmpLightArrayAlloc) + 0x1f) & ~0x1f);

    for (i = 0; i < dwNewArraySize ; i++)
    {
      InitializeLight(pTmpLightArray + i);
    }

    // Save all the created lights
    for (i=0; i < pRc->tl.dwLightArraySize; i++)
    {
        // If it is a valid, i.e. a light that has been set,
        // then save it in the new array
        pTmpLightArray[i] = pRc->tl.pLightArray[i];

        // If the light is enabled, update the ActiveList pointer
        if (LightIsEnabled(&pRc->tl.pLightArray[i]))
        {
            pTmpLightArray[i].Next = pTmpActiveLights;
            pTmpActiveLights = &pTmpLightArray[i];
        }
    }

    DXFREE(pRc->tl.pLightAlloc);
    pRc->tl.lighting.pActiveLights = pTmpActiveLights;
    pRc->tl.pLightArray = pTmpLightArray;
    pRc->tl.pLightAlloc = (unsigned char *) pTmpLightArrayAlloc;
    pRc->tl.dwLightArraySize = dwNewArraySize;
    return D3D_OK;
}


/* ScottK -- Light Changes */

RENDERFXN_RETVAL __stdcall StateClipping(RC *pRc, ULONG state)
{
  if(state)
    pRc->tl.dwTLState |= TLPV_DOCLIPPING;
  else
    pRc->tl.dwTLState &= ~TLPV_DOCLIPPING;

  // BobJ 01/19/00 
  // We have to mark the FVFOUT as dirty on 
  // a clipping renderstate change. We
  // need this because we have two different
  // vertex structures depending on this
  // state.
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FVFOUT;

  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateLighting(RC *pRc, ULONG state)
{
  if(state)
    pRc->tl.dwTLState |= TLPV_DOLIGHTING;
  else
    pRc->tl.dwTLState &= ~TLPV_DOLIGHTING;
  RENDERFXN_OK;
}

/*
// Currently not supported.
RENDERFXN_RETVAL __stdcall StateExtents(RC *pRc, ULONG state)
{
  RENDERFXN_OK;
}
*/

RENDERFXN_RETVAL __stdcall StateAmbient(RC *pRc, ULONG state)
{

  pRc->tl.lighting.ambient_red   = D3DVAL(RGBA_GETRED(state))/(D3DVALUE)(255);
  pRc->tl.lighting.ambient_green = D3DVAL(RGBA_GETGREEN(state))/(D3DVALUE)(255);
  pRc->tl.lighting.ambient_blue  = D3DVAL(RGBA_GETBLUE(state))/(D3DVALUE)(255);
  pRc->tl.lighting.ambient_save  = state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_MATERIAL;
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateFogRangeEnable(RC *pRc, ULONG state)
{
	//BUGBUG!!!

	// this isn't what the reference rasterizer uses
  pRc->tl.lighting.fog_range_enable = state;

	// AllenH added to get range-based fog working
  if(state)
    pRc->tl.dwTLState |= TLPV_RANGEFOG;
  else
    pRc->tl.dwTLState &= ~TLPV_RANGEFOG;

  RENDERFXN_OK;
}  

RENDERFXN_RETVAL __stdcall StateFogVertexMode(RC *pRc, ULONG state)
{
  pRc->tl.lighting.fog_mode = state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FOG;
  
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateColorVertex(RC *pRc, ULONG state)
{
//  pRc->tl.ColorVertex = state;
  // State is either 0 or 1. So we use it to set the appropriate bit in our render state
  pRc->tl.dwTLState &= ~TLPV_COLORVERTEXNEEDED;
  state <<= TLPV_COLORVERTEXNEEDED_SHIFT;
  state &= TLPV_COLORVERTEXNEEDED;
  pRc->tl.dwTLState |= state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateLocalViewer(RC *pRc, ULONG state)
{
  if(state)
    pRc->tl.dwTLState |= TLPV_LOCALVIEWER;
  else
    pRc->tl.dwTLState &= ~TLPV_LOCALVIEWER;
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateNormNormals(RC *pRc, ULONG state)
{
  if(state)
    pRc->tl.dwTLState |= TLPV_NORMALIZENORMALS;
  else
    pRc->tl.dwTLState &= ~TLPV_NORMALIZENORMALS;
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateDfusMaterialSrc(RC *pRc, ULONG state)
{
  pRc->tl.DfusMaterialSrc = state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateSpecMaterialSrc(RC *pRc, ULONG state)
{
  pRc->tl.SpecMaterialSrc = state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateAmbMaterialSrc(RC *pRc, ULONG state)
{
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  pRc->tl.AmbMaterialSrc = state;
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateEmisMaterialSrc(RC *pRc, ULONG state)
{
  pRc->tl.EmisMaterialSrc = state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateClipPlanEnable(RC *pRc, ULONG state)
{
  pRc->tl.clipPlaneEnable = state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_CLIPPLANES;
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateVertexBlends(RC *pRc, ULONG state)
{
  pRc->tl.VertexBlends = state;
  if (state == 0)
    pRc->tl.dwTLState &= ~TLPV_VERTEXBLENDNEEDED;
  else
    pRc->tl.dwTLState |= TLPV_VERTEXBLENDNEEDED;

  RENDERFXN_OK;
}

#endif //TnL_HAL
#endif



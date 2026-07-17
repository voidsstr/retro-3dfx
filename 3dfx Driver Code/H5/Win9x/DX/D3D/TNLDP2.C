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
** $Revision: 20$
** $Date: 10/26/00 7:57:30 AM$
**
** $Log: 
**  20   3dfx      1.9.1.9     10/26/00 Johnny Trainor  Changes due to modifation
**       of StateBlock structure.
**  19   3dfx      1.9.1.8     10/18/00 Allen Hansen    Optimized when T&L
**       parameters (matricies, material, lights, etc) are dirty, now we don't
**       reload these unless they actually change.  This saves the processing of
**       the dirty params.
**  18   3dfx      1.9.1.7     10/11/00 Brent           Forced check in to enforce
**       branching.
**  17   3dfx      1.9.1.6     10/03/00 Allen Hansen    optimized StateAmbient()
**  16   3dfx      1.9.1.5     09/28/00 Allen Hansen    started vertex blend
**       support in fastpath (still ifdef'd out)
**  15   3dfx      1.9.1.4     09/13/00 Allen Hansen    Fixed bug when using
**       colorvertex; if diffuse comes from the vertex, then the alpha must come
**       from that source.  Was failing the D3DIM sample app "Shadow Volume 2"
**  14   3dfx      1.9.1.3     08/29/00 Allen Hansen    added colorvertex support
**       for sse path
**  13   3dfx      1.9.1.2     08/27/00 Allen Hansen    added colorvertex support
**       for 3dnow path
**  12   3dfx      1.9.1.1     08/23/00 Allen Hansen    cleanup colorvertex state
**  11   3dfx      1.9.1.0     08/21/00 Allen Hansen    added stateblock support
**       for T&L, added debug print statements
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

  for (i = 0; i < wNumSetLight; i++ )
  {
    DWORD dwStride = sizeof(D3DHAL_DP2SETLIGHT);
    DWORD dwIndex = pSetLight->dwIndex;


#ifdef WINNT
    switch (pSetLight->lightData)
#else
    switch (pSetLight->dwDataType)
#endif
    {
    case D3DHAL_SETLIGHT_ENABLE:
	  {
        // if we're recording a state block just go save the data
        if (FALSE == pRc->bSBRecMode)
        {
	        D3DPRINT(D3DDBGLVL," enableing light %d", dwIndex);
        	if ( LightEnable(GetIndexedLightPtr(pRc->tl.lighting.pLightArray, dwIndex), pRc) )
			{
        		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_SETLIGHT;
        		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VB;
			}
        }
        else
        {
			if (NULL != pRc->pCurrSB)
			{
				D3DPRINT(D3DDBGLVL,"Storing light %d state = ENABLED", dwIndex );
	    		pRc->pCurrSB->uc.LightStates[dwIndex].enabled = TRUE;
				SET_STATEBLOCK_LIGHTSTATE_FLAG( pRc->pCurrSB, dwIndex, 1 );
			}
        }
      break;
	  }

    case D3DHAL_SETLIGHT_DISABLE:
	  {
        // if we're recording a state block just go save the data
        if (FALSE == pRc->bSBRecMode)
        {
	        D3DPRINT(D3DDBGLVL," disableing light %d", dwIndex);
        	if ( LightDisable(GetIndexedLightPtr(pRc->tl.lighting.pLightArray, dwIndex), pRc) )
        		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VB;
        }
        else
        {
			if (NULL != pRc->pCurrSB)
			{
				D3DPRINT(D3DDBGLVL,"Storing light %d state = DISABLED", dwIndex );
	    		pRc->pCurrSB->uc.LightStates[dwIndex].enabled = FALSE;
				SET_STATEBLOCK_LIGHTSTATE_FLAG( pRc->pCurrSB, dwIndex, 1 );
			}
        }
      break;
	  }

    case D3DHAL_SETLIGHT_DATA:
	  {
        pLightData = (D3DLIGHT7 *)((LPBYTE)pSetLight + dwStride);
        dwStride += sizeof(D3DLIGHT7);
        *extra += sizeof(D3DLIGHT7);

        // if we're recording a state block just go save the data
        if (FALSE == pRc->bSBRecMode)
        {
			TLLIGHT *pTlLight = GetIndexedLightPtr(pRc->tl.lighting.pLightArray, dwIndex);
			if( memcmp(pTlLight, pLightData, sizeof(LPD3DLIGHT7)) != 0 )	// only update if it really changed
			{
		        D3DPRINT(D3DDBGLVL," updating data for light %d", dwIndex);
    	    	HR_RET(SetLight(pRc, pTlLight, pLightData));
	    	    pRc->tl.dwDirtyFlags |= TLPV_DIRTY_SETLIGHT;
        		pRc->tl.dwDirtyFlags |= TLPV_DIRTY_VB;
			}
        }
        else
        {
			if (NULL != pRc->pCurrSB)
			{
				D3DPRINT(D3DDBGLVL,"Storing light %d from %lXh", dwIndex, (DWORD)pLightData );
				memcpy(	&pRc->pCurrSB->uc.LightStates[dwIndex].lData, pLightData, sizeof(D3DLIGHT7) );
				SET_STATEBLOCK_LIGHTSTATE_FLAG( pRc->pCurrSB, dwIndex, 2 );
			}
        }
      break;
	  }

    default:
        hr = DDERR_INVALIDPARAMS;
    }

    // Update the command buffer pointer
    pSetLight = (D3DHAL_DP2SETLIGHT *)((LPBYTE)pSetLight + dwStride);

  }

  return hr;
}


HRESULT DP2TL_CreateLight(RC *pRc, LPD3DHAL_DP2COMMAND pCmd)
{
    WORD wNumCreateLight = pCmd->wStateCount;
    D3DHAL_DP2CREATELIGHT *pCreateLight = (D3DHAL_DP2CREATELIGHT *)(pCmd + 1);
    HRESULT hr = D3D_OK;
    int i;

    // if we're recording a state block just go save the data
    if (FALSE == pRc->bSBRecMode)
    {
    	for (i = 0; i < wNumCreateLight; i++, pCreateLight++)
      	{
        	// If the index is not already allocated, grow the light array
        	// by REF_LIGHTARRAY_GROWTH_SIZE
	        D3DPRINT(D3DDBGLVL," creating light %d", pCreateLight->dwIndex);
        	if (pCreateLight->dwIndex >= pRc->tl.lighting.dwLightArraySize)
            	HR_RET(GrowLightArray(pRc, pCreateLight->dwIndex));
		}
	}
	else
	{
		if (NULL != pRc->pCurrSB)
		{
			// Add any light to the end of the list ... this could easily blow up
			// if this state block is executed many times!!!
    		for (i = 0; i < wNumCreateLight; i++, pCreateLight++)
			{
				DWORD dwIndex = pCreateLight->dwIndex;
				if(dwIndex < TLMAX_LIGHTS)
			    {
			    	pRc->pCurrSB->uc.LightStates[dwIndex].create = TRUE;
					D3DPRINT(D3DDBGLVL,"Storing CREATELIGHT %d", dwIndex );
					SET_STATEBLOCK_LIGHTSTATE_FLAG( pRc->pCurrSB, dwIndex, 0 );
				}
				else
				{
					D3DPRINT(D3DDBGLVL,"ERROR: CREATELIGHT %d overflowed max lignes in StateBlock! (%d)", dwIndex, TLMAX_LIGHTS );
				}
			}
        }
	}

    return hr;
}


HRESULT GrowLightArray(RC *pRc, const DWORD dwIndex)
{
    NT9XDEVICEDATA *ppdev = pRc->ppdev;
    // Allocate a few extra in anticipation of more light being used in the future
    DWORD dwNewArraySize = dwIndex+16;
    TLLIGHT *pTmpActiveLights = NULL;		// linked list of active lights
    TLLIGHT *pTmpLightArray = NULL;			// array of all lights
    TLLIGHT *pTmpLightArrayAlloc = NULL;	// ptr to where we allocated the light (for de-allocating)
    DWORD i;

	// Allocate the memory, then 32-byte align it
    pTmpLightArrayAlloc = DXMALLOC(SZ_TLLIGHT*dwNewArraySize+0x1f);
    if (pTmpLightArrayAlloc == NULL)
        return DDERR_OUTOFMEMORY;
    pTmpLightArray = (TLLIGHT*) ((((DWORD)pTmpLightArrayAlloc) + 0x1f) & ~0x1f);

    for (i = 0; i < dwNewArraySize ; i++)
    {
		InitializeLight(GetIndexedLightPtr(pTmpLightArray, i));
    }

    // Save all the created lights
	// copy the data (not that efficient but we shouldn't be doing this very often
    memcpy( pTmpLightArray, pRc->tl.lighting.pLightArray, pRc->tl.lighting.dwLightArraySize*SZ_TLLIGHT );

    for (i=0; i < pRc->tl.lighting.dwLightArraySize; i++)
	{
        // If the light is enabled, update the ActiveList pointer
		TLLIGHT *pL = GetIndexedLightPtr(pTmpLightArray, i);
		if(LightIsEnabled(pL))
		{
			pL->Next = pTmpActiveLights;
			pTmpActiveLights = pL;
		}
	}


    DXFREE(pRc->tl.lighting.pLightAlloc);
    pRc->tl.lighting.pActiveLights = pTmpActiveLights;
    pRc->tl.lighting.pLightArray = pTmpLightArray;
    pRc->tl.lighting.pLightAlloc = (unsigned char *) pTmpLightArrayAlloc;
    pRc->tl.lighting.dwLightArraySize = dwNewArraySize;
    return D3D_OK;
}


/* ScottK -- Light Changes */

RENDERFXN_RETVAL __stdcall StateClipping(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "Clipping Renderstate %d", state);
  pRc->tl.StateClipping = state;
  if(state)
    pRc->tl.dwTLState |= TLPV_DOCLIPPING;
  else
    pRc->tl.dwTLState &= ~TLPV_DOCLIPPING;

  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateLighting(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "Lighting Renderstate %d", state);
  pRc->tl.StateLighting = state;
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
  // Make sure this really changed.  This is a big deal
  // because we have to mark the material as dirty, which 
  // makes all the lights dirty
  if( pRc->tl.lighting.ambient_save != state)
  {
	int iRed   = RGBA_GETRED(state);
	int iGreen = RGBA_GETGREEN(state);
	int iBlue  = RGBA_GETBLUE(state);
	D3DVALUE fRed   = (D3DVALUE)iRed   / 255.0f;
	D3DVALUE fGreen = (D3DVALUE)iGreen / 255.0f;
	D3DVALUE fBlue  = (D3DVALUE)iBlue  / 255.0f;

	D3DPRINT( RSTATE_DBG_LVL, "ambient Renderstate %08lXh", state);
	pRc->tl.lighting.ambient_save  = state;

	pRc->tl.lighting.ambient_red   = fRed;
	pRc->tl.lighting.ambient_green = fGreen;
	pRc->tl.lighting.ambient_blue  = fBlue;

	if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_SSE)
	{
      TL_SOATMP *pTL = pRc->tl.pTL;
	  SwizzleScaler2SOAFLOAT( fRed,   &pTL->fSOAAmbientIn.red );
	  SwizzleScaler2SOAFLOAT( fGreen, &pTL->fSOAAmbientIn.green );
	  SwizzleScaler2SOAFLOAT( fBlue,  &pTL->fSOAAmbientIn.blue );
	}
	else if(pRc->tl.dwTLCpuCodePath & TL_CODEPATH_3DNOW)
	{
      // load into a qword and scale by 16,
      // p8Ambient values range 256-64K, FP values range 0-1
	  P8RGBA *p8Ambient_U12pt0 = (P8RGBA*)&pRc->tl.pTLK->p8Ambient_U12pt0;
	  p8Ambient_U12pt0->uw.b = (unsigned short)iRed   << 4;
	  p8Ambient_U12pt0->uw.g = (unsigned short)iGreen << 4;
	  p8Ambient_U12pt0->uw.r = (unsigned short)iBlue  << 4;
	  p8Ambient_U12pt0->uw.a = 0;
	}

	pRc->tl.dwDirtyFlags |= TLPV_DIRTY_MATERIAL;
  }

  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateFogRangeEnable(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "FogRange Renderstate %d", state);
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
  D3DPRINT( RSTATE_DBG_LVL, "FogVertexMode Renderstate %lX", state);
  pRc->tl.lighting.fog_mode = state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FOG;
  
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateColorVertex(RC *pRc, ULONG state)
{
  if (pRc->tl.StateColorVertex != state)
  {
    D3DPRINT( RSTATE_DBG_LVL, "ColorVertex Renderstate %lX", state);
	pRc->tl.StateColorVertex = state;
	if(state)
	  pRc->tl.dwTLState |= TLPV_COLORVERTEXNEEDED;
	else
	  pRc->tl.dwTLState &= ~TLPV_COLORVERTEXNEEDED;
	pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  }
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateLocalViewer(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "LocalViewer Renderstate %d", state);
  pRc->tl.StateLocalViewer = state;
  if(state)
    pRc->tl.dwTLState |= TLPV_LOCALVIEWER;
  else
    pRc->tl.dwTLState &= ~TLPV_LOCALVIEWER;
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateNormNormals(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "normalize normals Renderstate %d", state);
  pRc->tl.StateLocalViewer = state;
  if(state)
    pRc->tl.dwTLState |= TLPV_NORMALIZENORMALS;
  else
    pRc->tl.dwTLState &= ~TLPV_NORMALIZENORMALS;
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateDfusMaterialSrc(RC *pRc, ULONG state)
{
  if (pRc->tl.lighting.DfusMaterialSrc != state)
  {
	D3DPRINT( RSTATE_DBG_LVL, "diffuse material source Renderstate %d", state);
	pRc->tl.lighting.DfusMaterialSrc = state;
	pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  }
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateSpecMaterialSrc(RC *pRc, ULONG state)
{
  if (pRc->tl.lighting.SpecMaterialSrc != state)
  {
  D3DPRINT( RSTATE_DBG_LVL, "specular material source Renderstate %d", state);
  pRc->tl.lighting.SpecMaterialSrc = state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  }
  RENDERFXN_OK;
}


RENDERFXN_RETVAL __stdcall StateAmbMaterialSrc(RC *pRc, ULONG state)
{
  if (pRc->tl.lighting.AmbMaterialSrc != state)
  {
	D3DPRINT( RSTATE_DBG_LVL, "ambient material source Renderstate %d", state);
	pRc->tl.lighting.AmbMaterialSrc = state;
	pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  }
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateEmisMaterialSrc(RC *pRc, ULONG state)
{
  if (pRc->tl.lighting.EmisMaterialSrc != state)
  {
	D3DPRINT( RSTATE_DBG_LVL, "emmissive material source Renderstate %d", state);
	pRc->tl.lighting.EmisMaterialSrc = state;
	pRc->tl.dwDirtyFlags |= TLPV_DIRTY_COLORVTX;
  }
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateClipPlaneEnable(RC *pRc, ULONG state)
{
  if (pRc->tl.clipPlaneEnable != state)
  {
	D3DPRINT( RSTATE_DBG_LVL, "clip plane enable Renderstate %d", state);
	  pRc->tl.clipPlaneEnable = state;
	pRc->tl.dwDirtyFlags |= TLPV_DIRTY_CLIPPLANES;
  }
  RENDERFXN_OK;
}

RENDERFXN_RETVAL __stdcall StateVertexBlends(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "vertex blends Renderstate %d", state);
  if (state > D3DVBLEND_3WEIGHTS)
    state = D3DVBLEND_3WEIGHTS;

  pRc->tl.StateVertexBlends = state;

  if (state == D3DVBLEND_DISABLE)
    pRc->tl.dwTLState &= ~TLPV_VERTEXBLENDNEEDED;
  else
    pRc->tl.dwTLState |= TLPV_VERTEXBLENDNEEDED;

  RENDERFXN_OK;
}


#endif //TnL_HAL
#endif



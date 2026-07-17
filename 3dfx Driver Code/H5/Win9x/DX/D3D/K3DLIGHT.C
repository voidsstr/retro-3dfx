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
** File name: k3dlight.c
**
** Description: Lighting Code for the 3dnow format Vertex Buffers
**
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

#pragma warning(disable : 4799)  /* this shuts up the "no emms" warning */


_inline DWORD CLAMP_FLOAT_TO_INT(float f, int min, int max)
{								
	int i = FTOI(f);
	return (i>max) ? max : ((i<min) ? min : i);
}

#define EMULATE_K3D_MMX_COLORPATH

_inline signed short FTOSS(float f)
{
	signed short retVal;
	_asm	fld		dword ptr [f];
	_asm	fistp	word ptr [retVal];
	return	retVal;
}

// Colors are U16.0
__inline void AddP8COLORTOTLColor(P8RGBA *src, TLCOLOR *dst)
{
	dst->r += (D3DVALUE) (((int)src->uw.r) >> 0);
	dst->g += (D3DVALUE) (((int)src->uw.g) >> 0);
	dst->b += (D3DVALUE) (((int)src->uw.b) >> 0);
}

__inline void SetP8COLORToTLColor(P8RGBA *src, TLCOLOR *dst)
{
	dst->r = (D3DVALUE) (((int)src->uw.r) >> 0);
	dst->g = (D3DVALUE) (((int)src->uw.g) >> 0);
	dst->b = (D3DVALUE) (((int)src->uw.b) >> 0);
}
__inline void SetTLColorToP8COLOR(TLCOLOR *src, P8RGBA *dst)
{
	dst->uw.r = (unsigned short) (src->r * ((float)(1 << 0)));
	dst->uw.g = (unsigned short) (src->g * ((float)(1 << 0)));
	dst->uw.b = (unsigned short) (src->b * ((float)(1 << 0)));
}

// emulation of the mmx unsigned multiply (for proof of concept)
// Can run on K7 or SSE machines only (not regular mmx)
// val is a U12.4
// f is turned into m, an U12.4
// Do the umul, the result is result is 24.8 (actual 18.8), dump the lower 
// 16 bits (result is 8.8)

__inline void EM_MMX_MUL(P8RGBA *res, P8RGBA *val, float f)
{
	unsigned short m = (unsigned short) FTOSS(f * (float)(1 << 12));	// set to S11.4
	res->uw.a = (unsigned short) (((unsigned int)val->uw.a * (unsigned int)m) >> 16);
	res->uw.r = (unsigned short) (((unsigned int)val->uw.r * (unsigned int)m) >> 16);
	res->uw.g = (unsigned short) (((unsigned int)val->uw.g * (unsigned int)m) >> 16);
	res->uw.b = (unsigned short) (((unsigned int)val->uw.b * (unsigned int)m) >> 16);
}

__inline void EM_MMX_MAC(P8RGBA *res, P8RGBA *val, float f)
{
	unsigned short m = (unsigned short) FTOSS(f * (float)(1 << 12));	// set to S11.4
	res->uw.a += (unsigned short) (((unsigned int)val->uw.a * (unsigned int)m) >> 16);
	res->uw.r += (unsigned short) (((unsigned int)val->uw.r * (unsigned int)m) >> 16);
	res->uw.g += (unsigned short) (((unsigned int)val->uw.g * (unsigned int)m) >> 16);
	res->uw.b += (unsigned short) (((unsigned int)val->uw.b * (unsigned int)m) >> 16);
}


/*-------------------------------------------------------------------
Function Name:  LightVertexK3D_C
Description:    Apply all lights to a vertex
Parameters:     RC *pRC -- pointer to the rendering context
Information:    Currently a maximum of 8 lights are supported. This is
                hard-wired into the code to reduce the number of 
                expensive branch mispredictions in the code.
Return:         
-------------------------------------------------------------------*/
ALIGN32 void LightVertexK3D_C(RC* pRc)
{
	TLLIGHTING *Ldata = &pRc->tl.lighting;
	TLLIGHT  *pLight = Ldata->pActiveLights;
	DWORD diffuseAlpha = *pRc->tl.lighting.pDiffuseAlphaSrc;
	DWORD specularAlpha = *pRc->tl.lighting.pSpecularAlphaSrc;

	// Light 0
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 1
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 2
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 3
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 4
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 5
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 6
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 7
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 8
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 9
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 10
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 11
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 12
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 13
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 14
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	    pLight = pLight->Next;
	}
	else
    	goto LightFinished;

	// Light 15
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 16
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 17
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 18
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 19
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 20
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 21
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 22
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 23
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 24
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 25
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 26
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 27
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 28
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 29
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 30
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 31
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}
	else
    	goto LightFinished;

	// Light 32
	if (pLight)
	{
    	if (pLight->dwFlags & TLLIGHT_READY)
      		pLight->pfnLightVertexK3D(pRc, pLight);
	}

LightFinished:
	return;
}




/*******************************************************************************
	Special case light functions
*******************************************************************************/
ALIGN32 void LightVertexK3D0(RC* pRc)
{
  //TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

  // Accumulate color from the activated lights 
  // since there aren't any, this function is just a ret
  _asm nop		// Athlon's don't like to branch to a ret
}


ALIGN32 void LightVertexK3D1(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
}


ALIGN32 void LightVertexK3D2(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
}


ALIGN32 void LightVertexK3D3(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
}


ALIGN32 void LightVertexK3D4(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
}

ALIGN32 void LightVertexK3D5(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
}

ALIGN32 void LightVertexK3D6(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
}

ALIGN32 void LightVertexK3D7(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
}

ALIGN32 void LightVertexK3D8(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
}

ALIGN32 void LightVertexK3D9(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
}

ALIGN32 void LightVertexK3D10(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
}

ALIGN32 void LightVertexK3D11(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
}

ALIGN32 void LightVertexK3D12(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
}

ALIGN32 void LightVertexK3D13(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
}

ALIGN32 void LightVertexK3D14(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
}

ALIGN32 void LightVertexK3D15(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
}

ALIGN32 void LightVertexK3D16(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
}

ALIGN32 void LightVertexK3D17(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
}

ALIGN32 void LightVertexK3D18(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
}

ALIGN32 void LightVertexK3D19(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
}

ALIGN32 void LightVertexK3D20(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
}

ALIGN32 void LightVertexK3D21(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
}

ALIGN32 void LightVertexK3D22(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
}

ALIGN32 void LightVertexK3D23(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
}

ALIGN32 void LightVertexK3D24(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
	(pLA[23]->pfnLightVertexK3D)(pRc, pLA[23]);
}

ALIGN32 void LightVertexK3D25(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
	(pLA[23]->pfnLightVertexK3D)(pRc, pLA[23]);
	(pLA[24]->pfnLightVertexK3D)(pRc, pLA[24]);
}

ALIGN32 void LightVertexK3D26(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
	(pLA[23]->pfnLightVertexK3D)(pRc, pLA[23]);
	(pLA[24]->pfnLightVertexK3D)(pRc, pLA[24]);
	(pLA[25]->pfnLightVertexK3D)(pRc, pLA[25]);
}

ALIGN32 void LightVertexK3D27(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
	(pLA[23]->pfnLightVertexK3D)(pRc, pLA[23]);
	(pLA[24]->pfnLightVertexK3D)(pRc, pLA[24]);
	(pLA[25]->pfnLightVertexK3D)(pRc, pLA[25]);
	(pLA[26]->pfnLightVertexK3D)(pRc, pLA[26]);
}

ALIGN32 void LightVertexK3D28(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
	(pLA[23]->pfnLightVertexK3D)(pRc, pLA[23]);
	(pLA[24]->pfnLightVertexK3D)(pRc, pLA[24]);
	(pLA[25]->pfnLightVertexK3D)(pRc, pLA[25]);
	(pLA[26]->pfnLightVertexK3D)(pRc, pLA[26]);
	(pLA[27]->pfnLightVertexK3D)(pRc, pLA[27]);
}

ALIGN32 void LightVertexK3D29(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
	(pLA[23]->pfnLightVertexK3D)(pRc, pLA[23]);
	(pLA[24]->pfnLightVertexK3D)(pRc, pLA[24]);
	(pLA[25]->pfnLightVertexK3D)(pRc, pLA[25]);
	(pLA[26]->pfnLightVertexK3D)(pRc, pLA[26]);
	(pLA[27]->pfnLightVertexK3D)(pRc, pLA[27]);
	(pLA[28]->pfnLightVertexK3D)(pRc, pLA[28]);
}

ALIGN32 void LightVertexK3D30(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
	(pLA[23]->pfnLightVertexK3D)(pRc, pLA[23]);
	(pLA[24]->pfnLightVertexK3D)(pRc, pLA[24]);
	(pLA[25]->pfnLightVertexK3D)(pRc, pLA[25]);
	(pLA[26]->pfnLightVertexK3D)(pRc, pLA[26]);
	(pLA[27]->pfnLightVertexK3D)(pRc, pLA[27]);
	(pLA[28]->pfnLightVertexK3D)(pRc, pLA[28]);
	(pLA[29]->pfnLightVertexK3D)(pRc, pLA[29]);
}

ALIGN32 void LightVertexK3D31(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
	(pLA[23]->pfnLightVertexK3D)(pRc, pLA[23]);
	(pLA[24]->pfnLightVertexK3D)(pRc, pLA[24]);
	(pLA[25]->pfnLightVertexK3D)(pRc, pLA[25]);
	(pLA[26]->pfnLightVertexK3D)(pRc, pLA[26]);
	(pLA[27]->pfnLightVertexK3D)(pRc, pLA[27]);
	(pLA[28]->pfnLightVertexK3D)(pRc, pLA[28]);
	(pLA[29]->pfnLightVertexK3D)(pRc, pLA[29]);
	(pLA[30]->pfnLightVertexK3D)(pRc, pLA[30]);
}

ALIGN32 void LightVertexK3D32(RC* pRc)
{
	TLLIGHT **pLA = pRc->tl.lighting.pActiveLightArray;

	// Accumulate color from the activated lights
	(pLA[0]->pfnLightVertexK3D)(pRc, pLA[0]);
	(pLA[1]->pfnLightVertexK3D)(pRc, pLA[1]);
	(pLA[2]->pfnLightVertexK3D)(pRc, pLA[2]);
	(pLA[3]->pfnLightVertexK3D)(pRc, pLA[3]);
	(pLA[4]->pfnLightVertexK3D)(pRc, pLA[4]);
	(pLA[5]->pfnLightVertexK3D)(pRc, pLA[5]);
	(pLA[6]->pfnLightVertexK3D)(pRc, pLA[6]);
	(pLA[7]->pfnLightVertexK3D)(pRc, pLA[7]);
	(pLA[8]->pfnLightVertexK3D)(pRc, pLA[8]);
	(pLA[9]->pfnLightVertexK3D)(pRc, pLA[9]);
	(pLA[10]->pfnLightVertexK3D)(pRc, pLA[10]);
	(pLA[11]->pfnLightVertexK3D)(pRc, pLA[11]);
	(pLA[12]->pfnLightVertexK3D)(pRc, pLA[12]);
	(pLA[13]->pfnLightVertexK3D)(pRc, pLA[13]);
	(pLA[14]->pfnLightVertexK3D)(pRc, pLA[14]);
	(pLA[15]->pfnLightVertexK3D)(pRc, pLA[15]);
	(pLA[16]->pfnLightVertexK3D)(pRc, pLA[16]);
	(pLA[17]->pfnLightVertexK3D)(pRc, pLA[17]);
	(pLA[18]->pfnLightVertexK3D)(pRc, pLA[18]);
	(pLA[19]->pfnLightVertexK3D)(pRc, pLA[19]);
	(pLA[20]->pfnLightVertexK3D)(pRc, pLA[20]);
	(pLA[21]->pfnLightVertexK3D)(pRc, pLA[21]);
	(pLA[22]->pfnLightVertexK3D)(pRc, pLA[22]);
	(pLA[23]->pfnLightVertexK3D)(pRc, pLA[23]);
	(pLA[24]->pfnLightVertexK3D)(pRc, pLA[24]);
	(pLA[25]->pfnLightVertexK3D)(pRc, pLA[25]);
	(pLA[26]->pfnLightVertexK3D)(pRc, pLA[26]);
	(pLA[27]->pfnLightVertexK3D)(pRc, pLA[27]);
	(pLA[28]->pfnLightVertexK3D)(pRc, pLA[28]);
	(pLA[29]->pfnLightVertexK3D)(pRc, pLA[29]);
	(pLA[30]->pfnLightVertexK3D)(pRc, pLA[30]);
	(pLA[31]->pfnLightVertexK3D)(pRc, pLA[31]);
}

const TLLIGHTFN pK3dLight[TLMAX_ACTIVE_LIGHTS+1] =  {
	&LightVertexK3D0,
	&LightVertexK3D1,
	&LightVertexK3D2,
	&LightVertexK3D3,
	&LightVertexK3D4,
	&LightVertexK3D5,
	&LightVertexK3D6,
	&LightVertexK3D7,
	&LightVertexK3D8,
	&LightVertexK3D9,
	&LightVertexK3D10,
	&LightVertexK3D11,
	&LightVertexK3D12,
	&LightVertexK3D13,
	&LightVertexK3D14,
	&LightVertexK3D15,
	&LightVertexK3D16,
	&LightVertexK3D17,
	&LightVertexK3D18,
	&LightVertexK3D19,
	&LightVertexK3D20,
	&LightVertexK3D21,
	&LightVertexK3D22,
	&LightVertexK3D23,
	&LightVertexK3D24,
	&LightVertexK3D25,
	&LightVertexK3D26,
	&LightVertexK3D27,
	&LightVertexK3D28,
	&LightVertexK3D29,
	&LightVertexK3D30,
	&LightVertexK3D31,
	&LightVertexK3D32
};
/*************************************************************************************************************************************/
/*************************************************************************************************************************************/




/*------------------------------------------------------**
**         K3D Vertex Buffer Lighting Functions         **
**------------------------------------------------------*/
ALIGN32 void TLLV_DirectionalK3D_C( RC *pRc, TLLIGHT *pL) 
{
	TL_K3DTMP *pTlkTmp = (TL_K3DTMP *)pRc->tl.pTLK;
	TLLIGHTING *Ldata = &pRc->tl.lighting;
	D3DVALUE dot;
	DWORD dwTLState = pRc->tl.dwTLState;


	if (! (dwTLState & TLPV_COLORVERTEXAMB))
	{
#ifdef EMULATE_K3D_MMX_COLORPATH
		Ldata->p8Diffuse.w.b += pL->p8Ma_La_U16pt0.w.b;
		Ldata->p8Diffuse.w.g += pL->p8Ma_La_U16pt0.w.g;
		Ldata->p8Diffuse.w.r += pL->p8Ma_La_U16pt0.w.r;
		Ldata->p8Diffuse.w.a += pL->p8Ma_La_U16pt0.w.a;
#else
		Ldata->fDiffuse.b += pL->Ma_La.b;
		Ldata->fDiffuse.g += pL->Ma_La.g;
		Ldata->fDiffuse.r += pL->Ma_La.r;
		Ldata->fDiffuse.a += pL->Ma_La.a;
#endif
	}
	else
	{
		DWORD dwAmbientSrc = pRc->tl.lighting.dwAmbientSrc;
#ifdef EMULATE_K3D_MMX_COLORPATH
/*		Ldata->p8Diffuse.w.b += (unsigned short)(((DWORD)(pL->La.b * 255.0f*16.0f) * ((DWORD)RGBA_GETBLUE(dwAmbientSrc)  * 16)) >> 16);
		Ldata->p8Diffuse.w.g += (unsigned short)(((DWORD)(pL->La.g * 255.0f*16.0f) * ((DWORD)RGBA_GETGREEN(dwAmbientSrc) * 16)) >> 16);
		Ldata->p8Diffuse.w.r += (unsigned short)(((DWORD)(pL->La.r * 255.0f*16.0f) * ((DWORD)RGBA_GETRED(dwAmbientSrc)   * 16)) >> 16);
		Ldata->p8Diffuse.w.a += (unsigned short)(((DWORD)(pL->La.b * 255.0f*16.0f) * ((DWORD)RGBA_GETALPHA(dwAmbientSrc) * 16)) >> 16); */
		Ldata->p8Diffuse.w.b += (unsigned short)(((DWORD)pL->p8La_U12pt0.uw.b * (DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.b) >> 16);
		Ldata->p8Diffuse.w.g += (unsigned short)(((DWORD)pL->p8La_U12pt0.uw.g * (DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.g) >> 16);
		Ldata->p8Diffuse.w.r += (unsigned short)(((DWORD)pL->p8La_U12pt0.uw.b * (DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.r) >> 16);
		Ldata->p8Diffuse.w.a += (unsigned short)(((DWORD)pL->p8La_U12pt0.uw.a * (DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.a) >> 16);
#else
		Ldata->fDiffuse.b += pL->La.b * (D3DVALUE)RGBA_GETBLUE(dwAmbientSrc);
		Ldata->fDiffuse.g += pL->La.g * (D3DVALUE)RGBA_GETGREEN(dwAmbientSrc);
		Ldata->fDiffuse.r += pL->La.r * (D3DVALUE)RGBA_GETRED(dwAmbientSrc);
		Ldata->fDiffuse.a += pL->La.a * (D3DVALUE)RGBA_GETALPHA(dwAmbientSrc);
#endif
	}

	// If no normals are present, bail out since we cannot perform the
	// normal-dependent computations
	if ((pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL) == 0)
		return;

	// Dot the light vector with the normal, if it's negative then
	// the light source is behind the surface, so we're done
	dot = DotProduct( &pTlkTmp->cn, &pL->direction_in_eye );

	if (FLOAT_GTZ(dot))
	{
		if (! (dwTLState & TLPV_COLORVERTEXDIFF))
		{
#ifdef EMULATE_K3D_MMX_COLORPATH
			EM_MMX_MAC( &Ldata->p8Diffuse, &pL->p8Md_Ld_U12pt4, dot );
#else
			Ldata->fDiffuse.b += dot * pL->Md_Ld.b;
			Ldata->fDiffuse.g += dot * pL->Md_Ld.g;
			Ldata->fDiffuse.r += dot * pL->Md_Ld.r;
			Ldata->fDiffuse.a += dot * pL->Md_Ld.a;
#endif
		}
		else
		{
#ifdef EMULATE_K3D_MMX_COLORPATH
			DWORD dwDot_U16pt0 = (DWORD)(dot*255.0f*256.0f);		// clamp this to 64K
			Ldata->p8Diffuse.w.b += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.b * (DWORD)pL->p8Ld_U12pt0.uw.b) >> 16) * dwDot_U16pt0) >> 16);
			Ldata->p8Diffuse.w.g += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.g * (DWORD)pL->p8Ld_U12pt0.uw.g) >> 16) * dwDot_U16pt0) >> 16);
			Ldata->p8Diffuse.w.r += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.r * (DWORD)pL->p8Ld_U12pt0.uw.r) >> 16) * dwDot_U16pt0) >> 16);
			Ldata->p8Diffuse.w.a += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.a * (DWORD)pL->p8Ld_U12pt0.uw.a) >> 16) * dwDot_U16pt0) >> 16);
#else
			DWORD dwDiffuseSrc = pRc->tl.lighting.dwDiffuseSrc;
			Ldata->fDiffuse.b += dot * pL->Ld.b * (D3DVALUE)RGBA_GETBLUE(dwDiffuseSrc);
			Ldata->fDiffuse.g += dot * pL->Ld.g * (D3DVALUE)RGBA_GETGREEN(dwDiffuseSrc);
			Ldata->fDiffuse.r += dot * pL->Ld.r * (D3DVALUE)RGBA_GETRED(dwDiffuseSrc);
			Ldata->fDiffuse.a += dot * pL->Ld.a * (D3DVALUE)RGBA_GETALPHA(dwDiffuseSrc);
#endif
		}


		if (dwTLState & TLPV_DOSPECULAR)
		{
			D3DVECTOR h;      // halfway vector
			D3DVECTOR eye;    // incident vector ie vector from eye

			if (dwTLState & TLPV_LOCALVIEWER)
			{
				// calc vector from vertex to the eye
				SubtractVector( &Ldata->eye_in_eye, &pTlkTmp->cv, &eye);
				Normalize( &eye );
			}
			else
			{
				eye.x = (D3DVALUE) 0.0;
				eye.y = (D3DVALUE) 0.0;
				eye.z = (D3DVALUE)-1.0;
			}

            // calc halfway vector
			AddVector( &pL->direction_in_eye, &eye, &h );
			Normalize( &h );
			dot = DotProduct( &pTlkTmp->cn, &h );

			if ( (FLOAT_GTZ(dot)) && (FLOAT_CMP_POS(dot, >=, pRc->tl.lighting.specThreshold)) )
			{
				D3DVALUE coeff;
//				coeff = (D3DVALUE) pow( (double)dot, (double)pRc->tl.lighting.Material.power );
				_asm {
					mov		ecx, pRc
					movd	mm0, [dot]
					mov		eax, [ecx]RC.tl.lighting.K3dPowerFunction
					call	eax
					movd	[coeff], mm0
					femms
				}

				if (! (dwTLState & TLPV_COLORVERTEXSPEC))
				{
#ifdef EMULATE_K3D_MMX_COLORPATH
					EM_MMX_MAC( &Ldata->p8Specular, &pL->p8Ms_Ls_U12pt4, coeff );
#else
					Ldata->fSpecular.r += coeff * pL->Ms_Ls.r;
					Ldata->fSpecular.g += coeff * pL->Ms_Ls.g;
					Ldata->fSpecular.b += coeff * pL->Ms_Ls.b;
#endif
				}
				else
				{
#ifdef EMULATE_K3D_MMX_COLORPATH
					// Since coeff can be greater than one we'll make it a 12.0, which will
					// allow us some headroom.  This way we'll be okay as long as it's less
					// than 16.  To make the math work we need to make the specularSrc 16.0 
					// instead of 12.0
					DWORD dwCoeff_U12pt0 = (DWORD)(coeff*255.0f*16.0f);
					Ldata->p8Specular.w.b += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.b * (DWORD)pL->p8Ls_U12pt0.uw.b) >> 16) * dwCoeff_U12pt0) >> 16);
					Ldata->p8Specular.w.g += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.g * (DWORD)pL->p8Ls_U12pt0.uw.g) >> 16) * dwCoeff_U12pt0) >> 16);
					Ldata->p8Specular.w.r += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.r * (DWORD)pL->p8Ls_U12pt0.uw.r) >> 16) * dwCoeff_U12pt0) >> 16);
					Ldata->p8Specular.w.a += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.a * (DWORD)pL->p8Ls_U12pt0.uw.a) >> 16) * dwCoeff_U12pt0) >> 16);
#else
					DWORD dwSpecularSrc = pRc->tl.lighting.dwSpecularSrc;
					Ldata->fSpecular.r += coeff * pL->Ls.r * (D3DVALUE)RGBA_GETRED(dwSpecularSrc);
					Ldata->fSpecular.g += coeff * pL->Ls.g * (D3DVALUE)RGBA_GETGREEN(dwSpecularSrc);
					Ldata->fSpecular.b += coeff * pL->Ls.b * (D3DVALUE)RGBA_GETBLUE(dwSpecularSrc);
#endif
				}

			}

        } //TLPV_DOSPECULAR
	} // dot

}

ALIGN32 void TLLV_PointK3D_C( RC *pRc, TLLIGHT *pL)
{
	TL_K3DTMP *pTlkTmp = (TL_K3DTMP *)pRc->tl.pTLK;
	TLLIGHTING *Ldata = &pRc->tl.lighting;
	D3DVECTOR dirL;    // Direction to light
	D3DVALUE att;
	D3DVALUE dist;
	D3DVALUE dot;
	D3DVALUE distSquared;
	DWORD dwTLState = pRc->tl.dwTLState;


	//  d = pL->poistion_in_eye - pTL->cv
	SubtractVector( &pL->position_in_eye, &pTlkTmp->cv, &dirL);

	// early out if out of range or exactly on the vertex
	distSquared = SquareMagnitude( &dirL );
	if (FLOAT_CMP_POS(distSquared, >=, pL->range_squared) || FLOAT_EQZ(distSquared))
		return;

	// Compute the attenuation
	dist = SQRTF( distSquared );
	att = pL->Attenuation0 + pL->Attenuation1 * dist + pL->Attenuation2 * distSquared;

	if (FLOAT_EQZ(att))	att = (D3DVALUE) FLT_MAX;
   	else				att = (D3DVALUE) 1.0/att;

	dist = 1.0f/dist;

	// D3DLIGHT_SPOT was here


	// Add the material's ambient component
	if (! (dwTLState & TLPV_COLORVERTEXAMB))
	{
#ifdef EMULATE_K3D_MMX_COLORPATH
		EM_MMX_MAC( &Ldata->p8Diffuse, &pL->p8Ma_La_U12pt4, att );
#else
		Ldata->fDiffuse.r += att * pL->Ma_La.r;
		Ldata->fDiffuse.g += att * pL->Ma_La.g;
		Ldata->fDiffuse.b += att * pL->Ma_La.b;
		Ldata->fDiffuse.a += att * pL->Ma_La.a;
#endif
	}
	else
	{
#ifdef EMULATE_K3D_MMX_COLORPATH
		DWORD dwAtt_U16pt0 = (DWORD)(att*255.0f*256.0f);		// clamp this to 64K
		Ldata->p8Diffuse.w.b += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.b * (DWORD)pL->p8La_U12pt0.uw.b) >> 16) * dwAtt_U16pt0) >> 16);
		Ldata->p8Diffuse.w.g += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.g * (DWORD)pL->p8La_U12pt0.uw.g) >> 16) * dwAtt_U16pt0) >> 16);
		Ldata->p8Diffuse.w.r += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.r * (DWORD)pL->p8La_U12pt0.uw.r) >> 16) * dwAtt_U16pt0) >> 16);
		Ldata->p8Diffuse.w.a += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.a * (DWORD)pL->p8La_U12pt0.uw.a) >> 16) * dwAtt_U16pt0) >> 16);
#else
		DWORD dwAmbientSrc = pRc->tl.lighting.dwAmbientSrc;
		Ldata->fDiffuse.r += att * pL->La.r * (D3DVALUE)RGBA_GETRED(dwAmbientSrc);
		Ldata->fDiffuse.g += att * pL->La.g * (D3DVALUE)RGBA_GETGREEN(dwAmbientSrc);
		Ldata->fDiffuse.b += att * pL->La.b * (D3DVALUE)RGBA_GETBLUE(dwAmbientSrc);
		Ldata->fDiffuse.a += att * pL->La.a * (D3DVALUE)RGBA_GETALPHA(dwAmbientSrc);
#endif
	}


	// Calc dot product of light dir with normal.  Note that since we
	// didn't normalize the direction the result is scaled by the distance.
	// If no normals are present, bail out since we cannot perform the
	// normal-dependent computations
	if ((pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL) == 0) 
		return;

	// Dot the light vector with the normal, if it's negative then
	// the light source is behind the surface, so we're done
	dot = DotProduct( &pTlkTmp->cn, &dirL );

	if (FLOAT_GTZ( dot ))
	{
		dot = dot*dist*att;

		if (! (dwTLState & TLPV_COLORVERTEXDIFF))
		{
#ifdef EMULATE_K3D_MMX_COLORPATH
			EM_MMX_MAC( &Ldata->p8Diffuse, &pL->p8Md_Ld_U12pt4, dot );
#else
			Ldata->fDiffuse.r += dot * pL->Md_Ld.r;
			Ldata->fDiffuse.g += dot * pL->Md_Ld.g;
			Ldata->fDiffuse.b += dot * pL->Md_Ld.b;
			Ldata->fDiffuse.a += dot * pL->Md_Ld.a;
#endif
		}
		else
		{
#ifdef EMULATE_K3D_MMX_COLORPATH
			DWORD dwDot_U16pt0 = (DWORD)(dot*255.0f*256.0f);		// clamp this to 64K
			Ldata->p8Diffuse.w.b += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.b * (DWORD)pL->p8Ld_U12pt0.uw.b) >> 16) * dwDot_U16pt0) >> 16);
			Ldata->p8Diffuse.w.g += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.g * (DWORD)pL->p8Ld_U12pt0.uw.g) >> 16) * dwDot_U16pt0) >> 16);
			Ldata->p8Diffuse.w.r += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.r * (DWORD)pL->p8Ld_U12pt0.uw.r) >> 16) * dwDot_U16pt0) >> 16);
			Ldata->p8Diffuse.w.a += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.a * (DWORD)pL->p8Ld_U12pt0.uw.a) >> 16) * dwDot_U16pt0) >> 16);
#else
			DWORD dwDiffuseSrc = pRc->tl.lighting.dwDiffuseSrc;
			Ldata->fDiffuse.r += dot * pL->Ld.r * (D3DVALUE)RGBA_GETRED(dwDiffuseSrc);
			Ldata->fDiffuse.g += dot * pL->Ld.g * (D3DVALUE)RGBA_GETGREEN(dwDiffuseSrc);
			Ldata->fDiffuse.b += dot * pL->Ld.b * (D3DVALUE)RGBA_GETBLUE(dwDiffuseSrc);
			Ldata->fDiffuse.a += dot * pL->Ld.a * (D3DVALUE)RGBA_GETALPHA(dwDiffuseSrc);
#endif
		}


		if (dwTLState & TLPV_DOSPECULAR)
		{
			D3DVECTOR h;      // halfway vector
			D3DVECTOR eye;    // incident vector ie vector from eye

			// normalize light direction
			dirL.x *= dist;
			dirL.y *= dist;
			dirL.z *= dist;

			if (dwTLState & TLPV_LOCALVIEWER)
			{
				// calc vector from vertex to the eye
				SubtractVector( &Ldata->eye_in_eye, &pTlkTmp->cv, &eye);
				Normalize( &eye );
			}
			else
			{
				eye.x = (D3DVALUE) 0.0 ;
				eye.y = (D3DVALUE) 0.0 ;
				eye.z = (D3DVALUE)-1.0 ;
			}

            // calc halfway vector
   	        AddVector( &dirL, &eye, &h );
       	    Normalize( &h );
			dot = DotProduct( &pTlkTmp->cn, &h );

			if (FLOAT_CMP_POS(dot, >=, pRc->tl.lighting.specThreshold))
			{
				D3DVALUE coeff;
//				coeff = (D3DVALUE) pow( (double)dot, (double)pRc->tl.lighting.Material.power ) * att;
				_asm {
					mov		ecx, pRc
					movd	mm0, [dot]
					mov		eax, [ecx]RC.tl.lighting.K3dPowerFunction
					call	eax
					pfmul	mm0, [att]
					movd	[coeff], mm0
					femms
				}

				if (! (dwTLState & TLPV_COLORVERTEXSPEC))
				{
#ifdef EMULATE_K3D_MMX_COLORPATH
					EM_MMX_MAC( &Ldata->p8Specular, &pL->p8Ms_Ls_U12pt4, coeff );
#else
					Ldata->fSpecular.r += coeff * pL->Ms_Ls.r;
					Ldata->fSpecular.g += coeff * pL->Ms_Ls.g;
					Ldata->fSpecular.b += coeff * pL->Ms_Ls.b;
#endif
				}
				else
				{
#ifdef EMULATE_K3D_MMX_COLORPATH
					DWORD dwCoeff_U12pt0 = (DWORD)(coeff*255.0f*16.0f);
					Ldata->p8Specular.w.b += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.b * (DWORD)pL->p8Ls_U12pt0.uw.b) >> 16) * dwCoeff_U12pt0) >> 16);
					Ldata->p8Specular.w.g += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.g * (DWORD)pL->p8Ls_U12pt0.uw.g) >> 16) * dwCoeff_U12pt0) >> 16);
					Ldata->p8Specular.w.r += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.r * (DWORD)pL->p8Ls_U12pt0.uw.r) >> 16) * dwCoeff_U12pt0) >> 16);
					Ldata->p8Specular.w.a += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.a * (DWORD)pL->p8Ls_U12pt0.uw.a) >> 16) * dwCoeff_U12pt0) >> 16);
#else
					DWORD dwSpecularSrc = pRc->tl.lighting.dwSpecularSrc;
					Ldata->fSpecular.r += coeff * pL->Ls.r * (D3DVALUE)RGBA_GETRED(dwSpecularSrc);
					Ldata->fSpecular.g += coeff * pL->Ls.g * (D3DVALUE)RGBA_GETGREEN(dwSpecularSrc);
					Ldata->fSpecular.b += coeff * pL->Ls.b * (D3DVALUE)RGBA_GETBLUE(dwSpecularSrc);
#endif
				}

			}
		}//	TLPV_DOSPECULAR
	}//	dot

}


ALIGN32 void TLLV_SpotK3D_C( RC *pRc, TLLIGHT *pL)
{
	TL_K3DTMP *pTlkTmp = (TL_K3DTMP *)pRc->tl.pTLK;
	TLLIGHTING *Ldata = &pRc->tl.lighting;
	D3DVECTOR dirL;    // Direction to light
	D3DVALUE att;
	D3DVALUE dist;
	D3DVALUE dot;
	D3DVALUE distSquared;
	DWORD dwTLState = pRc->tl.dwTLState;


	//  d = pL->poistion_in_eye - pTL->cv
	SubtractVector( &pL->position_in_eye, &pTlkTmp->cv, &dirL);

	// early out if out of range or exactly on the vertex
	distSquared = SquareMagnitude( &dirL );
	if (FLOAT_CMP_POS(distSquared, >=, pL->range_squared) || FLOAT_EQZ(distSquared))
		return;

	// Compute the attenuation
	dist = SQRTF( distSquared );
	att = pL->Attenuation0 + pL->Attenuation1 * dist + pL->Attenuation2 * distSquared;

	if (FLOAT_EQZ(att))	att = (D3DVALUE) FLT_MAX;
   	else				att = (D3DVALUE) 1.0/att;

	dist = 1.0f/dist;

	// if D3DLIGHT_SPOT 
	{
		// Calc dot product of direction to light with light direction to
		// be compared anganst the cone angles to see if we are in the light.
		// Note that cone_dot is still scaled by dist
		D3DVALUE cone_dot = DotProduct(&dirL, &pL->direction_in_eye) * dist;
    
		if (FLOAT_CMP_POS(cone_dot, <=, pL->cos_phi_by_2))
			return;
    
		// modify att if in the region between phi and theta
		if (FLOAT_CMP_POS(cone_dot, <, pL->cos_theta_by_2))
		{
			D3DVALUE val = (cone_dot - pL->cos_phi_by_2) * pL->inv_theta_minus_phi;
       
			if (!FLOAT_EQZ( pL->Falloff - (float) 1.0 ))
			{
//				val = (D3DVALUE) pow( (double)val, (double)pL->Falloff );
				_asm {
					mov			eax, pL
					movd		mm0, [val]
					movd		mm1, [eax]TLLIGHT.Falloff
					call		k3dpow_amd
					movd		[val], mm0
					femms
				}
			}
			att *= val;
		}
	} // endif D3DLIGHT_SPOT


	// Add the material's ambient component
	if (! (dwTLState & TLPV_COLORVERTEXAMB))
	{
#ifdef EMULATE_K3D_MMX_COLORPATH
		EM_MMX_MAC( &Ldata->p8Diffuse, &pL->p8Ma_La_U12pt4, att );
#else
		Ldata->fDiffuse.r += att * pL->Ma_La.r;
		Ldata->fDiffuse.g += att * pL->Ma_La.g;
		Ldata->fDiffuse.b += att * pL->Ma_La.b;
		Ldata->fDiffuse.a += att * pL->Ma_La.a;
#endif
	}
	else
	{
#ifdef EMULATE_K3D_MMX_COLORPATH
		DWORD dwAtt_U16pt0 = (DWORD)(att*255.0f*256.0f);		// clamp this to 64K
		Ldata->p8Diffuse.w.b += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.b * (DWORD)pL->p8La_U12pt0.uw.b) >> 16) * dwAtt_U16pt0) >> 16);
		Ldata->p8Diffuse.w.g += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.g * (DWORD)pL->p8La_U12pt0.uw.g) >> 16) * dwAtt_U16pt0) >> 16);
		Ldata->p8Diffuse.w.r += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.r * (DWORD)pL->p8La_U12pt0.uw.r) >> 16) * dwAtt_U16pt0) >> 16);
		Ldata->p8Diffuse.w.a += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8AmbientSrc_U12pt0.uw.a * (DWORD)pL->p8La_U12pt0.uw.a) >> 16) * dwAtt_U16pt0) >> 16);
#else
		DWORD dwAmbientSrc = pRc->tl.lighting.dwAmbientSrc;
		Ldata->fDiffuse.r += att * pL->La.r * (D3DVALUE)RGBA_GETRED(dwAmbientSrc);
		Ldata->fDiffuse.g += att * pL->La.g * (D3DVALUE)RGBA_GETGREEN(dwAmbientSrc);
		Ldata->fDiffuse.b += att * pL->La.b * (D3DVALUE)RGBA_GETBLUE(dwAmbientSrc);
		Ldata->fDiffuse.a += att * pL->La.a * (D3DVALUE)RGBA_GETALPHA(dwAmbientSrc);
#endif
	}


	// Calc dot product of light dir with normal.  Note that since we
	// didn't normalize the direction the result is scaled by the distance.
	// If no normals are present, bail out since we cannot perform the
	// normal-dependent computations
	if ((pRc->tl.InFVF.dwFVFType & D3DFVF_NORMAL) == 0) 
	  return;

	// Dot the light vector with the normal, if it's negative then
	// the light source is behind the surface, so we're done
	dot = DotProduct( &pTlkTmp->cn, &dirL );

	if (FLOAT_GTZ( dot ))
	{
		dot = dot*dist*att;

		if (! (dwTLState & TLPV_COLORVERTEXDIFF))
		{
#ifdef EMULATE_K3D_MMX_COLORPATH
			EM_MMX_MAC( &Ldata->p8Diffuse, &pL->p8Md_Ld_U12pt4, dot );
#else
			Ldata->fDiffuse.r += dot * pL->Md_Ld.r;
			Ldata->fDiffuse.g += dot * pL->Md_Ld.g;
			Ldata->fDiffuse.b += dot * pL->Md_Ld.b;
			Ldata->fDiffuse.a += dot * pL->Md_Ld.a;
#endif
		}
		else
		{
#ifdef EMULATE_K3D_MMX_COLORPATH
			DWORD dwDot_U16pt0 = (DWORD)(dot*255.0f*256.0f);		// clamp this to 64K
			Ldata->p8Diffuse.w.b += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.b * (DWORD)pL->p8Ld_U12pt0.uw.b) >> 16) * dwDot_U16pt0) >> 16);
			Ldata->p8Diffuse.w.g += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.g * (DWORD)pL->p8Ld_U12pt0.uw.g) >> 16) * dwDot_U16pt0) >> 16);
			Ldata->p8Diffuse.w.r += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.r * (DWORD)pL->p8Ld_U12pt0.uw.r) >> 16) * dwDot_U16pt0) >> 16);
			Ldata->p8Diffuse.w.a += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8DiffuseSrc_U12pt0.uw.a * (DWORD)pL->p8Ld_U12pt0.uw.a) >> 16) * dwDot_U16pt0) >> 16);
#else
			DWORD dwDiffuseSrc = pRc->tl.lighting.dwDiffuseSrc;
			Ldata->fDiffuse.r += dot * pL->Ld.r * (D3DVALUE)RGBA_GETRED(dwDiffuseSrc);
			Ldata->fDiffuse.g += dot * pL->Ld.g * (D3DVALUE)RGBA_GETGREEN(dwDiffuseSrc);
			Ldata->fDiffuse.b += dot * pL->Ld.b * (D3DVALUE)RGBA_GETBLUE(dwDiffuseSrc);
			Ldata->fDiffuse.a += dot * pL->Ld.a * (D3DVALUE)RGBA_GETALPHA(dwDiffuseSrc);
#endif
		}


		if (dwTLState & TLPV_DOSPECULAR)
		{
			D3DVECTOR h;      // halfway vector
			D3DVECTOR eye;    // incident vector ie vector from eye

			// normalize light direction
			dirL.x *= dist;
			dirL.y *= dist;
			dirL.z *= dist;

			if (dwTLState & TLPV_LOCALVIEWER)
			{
				// calc vector from vertex to the eye
				SubtractVector( &Ldata->eye_in_eye, &pTlkTmp->cv, &eye);
				Normalize( &eye );
			}
			else
			{
				eye.x = (D3DVALUE) 0.0 ;
				eye.y = (D3DVALUE) 0.0 ;
				eye.z = (D3DVALUE)-1.0 ;
			}

            // calc halfway vector
   	        AddVector( &dirL, &eye, &h );
       	    Normalize( &h );
			dot = DotProduct( &pTlkTmp->cn, &h );

			if (FLOAT_CMP_POS(dot, >=, pRc->tl.lighting.specThreshold))
			{
				D3DVALUE coeff;
//				coeff = (D3DVALUE) pow( (double)dot, (double)pRc->tl.lighting.Material.power ) * att;
				_asm {
					mov		ecx, pRc
					movd	mm0, [dot]
					mov		eax, [ecx]RC.tl.lighting.K3dPowerFunction
					call	eax
					pfmul	mm0, [att]
					movd	[coeff], mm0
					femms
				}

				if (! (dwTLState & TLPV_COLORVERTEXSPEC))
				{
#ifdef EMULATE_K3D_MMX_COLORPATH
					EM_MMX_MAC( &Ldata->p8Specular, &pL->p8Ms_Ls_U12pt4, coeff );
#else
					Ldata->fSpecular.r += coeff * pL->Ms_Ls.r;
					Ldata->fSpecular.g += coeff * pL->Ms_Ls.g;
					Ldata->fSpecular.b += coeff * pL->Ms_Ls.b;
#endif
				}
				else
				{
#ifdef EMULATE_K3D_MMX_COLORPATH
					DWORD dwCoeff_U12pt0 = (DWORD)(coeff*255.0f*16.0f);
					Ldata->p8Specular.w.b += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.b * (DWORD)pL->p8Ls_U12pt0.uw.b) >> 16) * dwCoeff_U12pt0) >> 16);
					Ldata->p8Specular.w.g += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.g * (DWORD)pL->p8Ls_U12pt0.uw.g) >> 16) * dwCoeff_U12pt0) >> 16);
					Ldata->p8Specular.w.r += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.r * (DWORD)pL->p8Ls_U12pt0.uw.r) >> 16) * dwCoeff_U12pt0) >> 16);
					Ldata->p8Specular.w.a += (unsigned short)(((((DWORD)pRc->tl.pTLK->p8SpecularSrc_U16pt0.uw.a * (DWORD)pL->p8Ls_U12pt0.uw.a) >> 16) * dwCoeff_U12pt0) >> 16);
#else
					DWORD dwSpecularSrc = pRc->tl.lighting.dwSpecularSrc;
					Ldata->fSpecular.r += coeff * pL->Ls.r * (D3DVALUE)RGBA_GETRED(dwSpecularSrc);
					Ldata->fSpecular.g += coeff * pL->Ls.g * (D3DVALUE)RGBA_GETGREEN(dwSpecularSrc);
					Ldata->fSpecular.b += coeff * pL->Ls.b * (D3DVALUE)RGBA_GETBLUE(dwSpecularSrc);
#endif
				}

			}
		}//	TLPV_DOSPECULAR
	}//	dot

}





/*************************************************************************************************************************************/
/*************************************************************************************************************************************/
/*-------------------------------------------------------------------
Function Name:  FogVertexK3D_C
Description:    Performs fogging calculation on the input vertex
Parameters:     RC *pRC -- pointer to the rendering context
Information:    
Return:         fog value as a float
-------------------------------------------------------------------*/
#if 0	//unused but don't delete yet
float FogVertexK3D_C(RC* pRc)
{
	D3DVALUE fog, dist;
	TL_K3DTMP *pTlkTmp = (TL_K3DTMP *)pRc->tl.pTLK;

    // THIS DOESN'T HANDLE VERTEX BLENDS!!!

    // Vertex is already transformed to the camera space
	if ( pRc->tl.dwTLState & TLPV_RANGEFOG)
		dist = SQRTF( pTlkTmp->cv.x*pTlkTmp->cv.x + pTlkTmp->cv.y*pTlkTmp->cv.y + pTlkTmp->cv.z*pTlkTmp->cv.z );
	else
		dist = pTlkTmp->cv.z;

	if (pRc->tl.lighting.fog_mode == D3DFOG_LINEAR)
	{
		if (dist < pRc->tl.lighting.fog_start)
			fog = 255.0f;
		else if (dist >= pRc->tl.lighting.fog_end)
			fog = 0;
		else
			fog = (pRc->tl.lighting.fog_end - dist) * pRc->tl.lighting.fog_factor;
	}
    else
    {
       	D3DVALUE tmp = dist * pRc->tl.lighting.fog_density;
		if (pRc->tl.lighting.fog_mode == D3DFOG_EXP2)
			tmp *= tmp;
		fog = (D3DVALUE)exp(-tmp) * 255.0f;
    }

	return fog;
}
#endif //0

/*-------------------------------------------------------------------
Function Name:  FogVertexK3D_Asm
Description:    Performs fogging calculation on the input vertex
Parameters:     RC *pRC -- pointer to the rendering context
Information:    
Return:         fog value as a float in mm0
-------------------------------------------------------------------*/
#pragma warning( push )
#pragma warning(disable : 4799)  /* this shuts up the "no emms" warning */
void FogVertexK3D_Asm(RC* pRc)
{
	_asm {
		mov		ebx, [pRc]
		mov     eax, [ebx]RC.tl.pTLK	// TL_K3DTMP

	    // Vertex is already transformed to the camera space

		test	[ebx]RC.tl.dwTLState, TLPV_RANGEFOG

		// dist = SQRTF( pTlkTmp->cv.x*pTlkTmp->cv.x + pTlkTmp->cv.y*pTlkTmp->cv.y + pTlkTmp->cv.z*pTlkTmp->cv.z );
		movd	mm0, [eax]TL_K3DTMP.cv.z	//	0		z
		je		NoRangeFog
		movq	mm1, [eax]TL_K3DTMP.cv.x	//	y		x
		pfmul	mm0, mm0					//	0		z*z
		pfmul	mm1, mm1					//	y*y		x*x
		pfadd	mm0, mm1
		pfacc	mm0, mm0					//	x*x+y*y+z*z
		pfrsqrt	mm0, mm0
		pfrcp	mm0, mm0					// sqrt(x*x+y*y+z*z)
		jmp		CheckFogMode

	NoRangeFog:
		// dist = pTlkTmp->cv.z;
		// dist is already loaded

	CheckFogMode:
		cmp		[ebx]RC.tl.lighting.fog_mode, D3DFOG_LINEAR
		jne		ExponentialFog

		movd	mm2, [ebx]RC.tl.lighting.fog_end
		movq	mm1, mm0
		pfcmpge	mm0, [ebx]RC.tl.lighting.fog_start	// (dist < start) ? 0 : -1
		movd	mm3, [ebx]RC.tl.lighting.fog_factor
		movq	mm4, mm2
		pfcmpgt	mm2, mm1						 	// (dist >= end) ? 0 : -1
		pand	mm3, mm0							// (dist < start) ? 0 : fog_factor
		pand	mm3, mm2							// ((dist < start) || (dist >= end)) ? 0 : fog_factor

		// if fog is in range, fog = (end - dist) * fog_factor (else this result will be zero)
		pfsub	mm4, mm1							// end - dist
		pcmpeqd	mm2, mm2							// -1
		pfmul	mm4, mm3							// ((dist < start) || (dist >= end)) ? 0 : fog_factor * (end - dist)

		// if (dist < start) fog = 255
		pxor	mm0, mm2							// (dist < start) ? -1 : 0
		pand	mm0, [TL_soa_255]					// (dist < start) ? 255 : 0
		por		mm0, mm4							// (dist < start) ? 255 : (dist >= end) ? 0 : fog_factor * (end - dist)
		jmp		FogDone

	ExponentialFog:
		pfmul	mm0, [ebx]RC.tl.lighting.fog_density
		test	[ebx]RC.tl.lighting.fog_mode, D3DFOG_EXP2
		je		ExponentialFog_GetExponent
		pfmul	mm0, mm0
	ExponentialFog_GetExponent:
		pxor	mm0, [TL_hi_bits]					// 80000000 (flip sign bit)
		call	K3dExp
		pfmul	mm0, [TL_soa_255]

	FogDone:
	}
}
#pragma warning( pop ) /* restore the compiler warnings */

/*************************************************************************************************************************************/


#endif //TnL_HAL
#endif //DX >= 7
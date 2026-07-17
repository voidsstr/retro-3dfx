/* -*-c++-*- */
/* $Header: ddsli2d.h, 9, 10/11/00 8:45:38 PM, Brent$ */
/*
** Copyright (c) 1995, 3Dfx Interactive, Inc.
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
** File Name: 	ddsli2d.c
**
** Description: This file has all of the 2D routines to work on distributed
**  surfaces aka SLI surfaces.
**
** $Revision: 9$
** $Date: 10/11/00 8:45:38 PM$
**
** $History: $
**
*/

#ifndef __DDSLI2D_H__
#define __DDSLI2D_H__

#ifdef SLI_AA

#ifndef WINNT
#include "d3contxt.h"
#include "ddglobal.h"
#include "d3global.h"
#include "d3txtr.h"
#include "fifomgr.h"
#endif

// Blended Interface
typedef struct solidcolorparams {
   long dstLeft;
   long dstRight;
   long dstTop;
   long dstBottom;
   DWORD fillData;
   DWORD bltDstFormat;
   DWORD bltRop;
   DWORD bltDstBaseAddr;
   DWORD dwSliSurface;
   DWORD Pitch;                  
   DWORD fpVidMem;
   DWORD linearPitch; // following fields are used by sli_PuntColorFillZ
   DWORD isTiled;
   DWORD isZClear;
   DWORD BytesPerPel;
   } SOLIDCOLORPARAMS, * PSOLIDCOLORPARAMS;

// 3D Interface
HRESULT SliClear(RC *, DWORD, DWORD, D3DVALUE, DWORD, RECT *, DWORD);

// 2D Interface
DWORD DdSliColorFill(NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD);

// Common Interface
int DdSli2DSolidColor(NT9XDEVICEDATA *, PSOLIDCOLORPARAMS);

// Rest of the Functions
int DdSliBltNoSP(NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD);
DWORD DdSli2DScn2Scn(NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD, DWORD);
DWORD DdSli2DSystem2Scn(NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD, DWORD, DWORD);

//2 Sample Functions
DWORD Dd2SampleColorFill(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD dstPixelFormat);
DWORD Dd2SampleBltNoSP(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD dstPixelFormat);
DWORD Dd2SampleBltS(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD srcPixelFormat, DWORD dstPixelFormat);
DWORD Dd2SampleSystemToVideo(NT9XDEVICEDATA *ppdev, LPDDHAL_BLTDATA pbd, DWORD rop3, DWORD srcPixelFormat, DWORD pixelByteDepth, DWORD dstPixelFormat);

/*-------------------------------------------------------------------------
 *  ddsliblt.c
 *-------------------------------------------------------------------------
 */
void fillScanAND(BYTE *, BYTE *, DWORD);
void fillScanOR (BYTE *, BYTE *, DWORD);
void fillScanSRC(BYTE *, BYTE *, DWORD);
void initBuffer (BYTE *, DWORD, DWORD, DWORD);

DWORD __stdcall sli_ColorFill(NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD);
DWORD __stdcall sli_ColorFillZ(NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD);
DWORD __stdcall sli_DoBltNoSP( NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD);
DWORD __stdcall sli_DoBltS( NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD, DWORD, BLT_TYPE);
DWORD __stdcall sli_PuntColorFill( NT9XDEVICEDATA *, PSOLIDCOLORPARAMS);
DWORD __stdcall sli_PuntColorFillZ( NT9XDEVICEDATA *, PSOLIDCOLORPARAMS);

DWORD __stdcall sli_3DDoBltS( NT9XDEVICEDATA *, LPDDHAL_BLTDATA, DWORD, DWORD, DWORD);
DWORD __stdcall PromoteSrcToPower2Width( NT9XDEVICEDATA *, LPDDHAL_BLTDATA);
#endif

#endif  // __DDSLI2D_H__


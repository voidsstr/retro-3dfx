/*
** Copyright (c) 1998, 3Dfx Interactive, Inc.
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
** File name: d6global.h
**
** Description: Triangle rendering function declairations and wrap Macro
**
** $Revision: 9$
** $Date: 10/31/00 1:58:36 AM$
**
**
** $Log: 
**  9    3dfx      1.6.2.1     10/31/00 Johnny Trainor  Added new Triangle
**       rendering function declarations.
**  8    3dfx      1.6.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  7    3dfx      1.6         02/04/00 Steve Houston   Added new asm triangle
**       routines by Matt McClure and Allen Hanson to W2K driver.
**       Added NEWASMTRI, STBKNI and K6_2 defines to W2K\build\stbperf.inc to
**       enable this code.
** 
**  6    3dfx      1.5         10/29/99 Chris W. Shaw   Fix for PRS 11150.  Don't
**       do autostripped triangles when using specular.
**  5    3dfx      1.4         10/29/99 Chris W. Shaw   Allow IGX_SPECULAR_FIX for
**       WINNT.
**  4    3dfx      1.3         10/28/99 Chris W. Shaw   Temp fix for Verdict tex
**       coordinate problems associated with IGX_SPECULAR_FIX.
**  3    3dfx      1.2         10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
**  2    3dfx      1.1         10/22/99 Chris W. Shaw   Add IGX_SPECULAR_FIX(2) to
**       Napalm source.  Now DCT250-BetaPreview-DriverScenario passes 3200 and
**       fails 1153 tests (instead of passing only 2753 and failing 1601).
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 2     6/21/99 5:26p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 1     6/02/99 6:45a Michael
** Branch from H3
**
** 14    1/29/99 10:49a Cshaw
** Added unified headers.
**
** 13    12/16/98 4:03p Sreid
** K6 merge - 12-16-98 drop from contractor
**
** 12    12/09/98 6:38p Sreid
** Merged latest K6 optimization code from Metabyte
**
** 11    12/06/98 1:53p Adrians
** Optimisations for WinBench99.
**
** 10    11/04/98 5:42p Adrians
** Updated changes to K6-2.
**
** 7     9/12/98 12:57a Adrians
** Clean up renderstate trace code and add new DX6 states.
** Add trace support for DX6 texture stage states.
** General DX6 code tidyup.
**
** 6     9/05/98 5:02p Adrians
** Added AA support to DX6.
** Validate will now fail arguments with COMPLEMENT or ALPHAREPLICATE.
**
** 5     9/02/98 12:34p Adrians
** DX6 multitexture change.
**
** 4     8/28/98 1:33p Adrians
** Support for new multi-texture scheme.
**
** 3     8/15/98 12:15a Adrians
** Take Flat Shaded color from correct vertex for strips and fans.
** Use the same alpha component for all 3 vertices when Flat Shading.
** Add a Flat Shaded vertex component parameter to the line and point
** drawing.
**
** 2     8/14/98 8:11p Adrians
** Add texture wrapping support.
**
** 1     8/10/98 3:20p Adrians
**
** 1     7/30/98 4:37p Adrians
** DX6 line drawing support.
*/

#if ( DX >= 6 )

#ifndef IGX_SPECULAR_FIX    // This applies to a bunch of changes that correctly applies
#define IGX_SPECULAR_FIX    // alpha blending to specular on a second pass.
#endif // IGX_SPECULAR_FIX

#ifdef IGX_SPECULAR_FIX
#ifndef IGX_SPECULAR_FIX2    // This applies to a bunch of changes that correctly applies
#define IGX_SPECULAR_FIX2    // alpha blending to specular on a second pass for the second stage.
#endif // IGX_SPECULAR_FIX2
#endif // IGX_SPECULAR_FIX

#include "d6fvf.h"

#ifndef _D6GLOBAL_H
#define _D6GLOBAL_H

//-------------------------------------------------------------------

extern void  dp2Line( RC *pRc, LPDWORD pF, LPDWORD pA, LPDWORD pB, DWORD vertexType );
#ifdef TnL_HAL
extern void  dp2LineClipped( RC *pRc, LPDWORD pF, LPDWORD pA, LPDWORD pB, DWORD c0, DWORD c1, DWORD vertexType );
#endif
extern void  dp2Point( RC *pRc, LPDWORD pF, LPDWORD pV, DWORD vertexType );

extern void  dp2TriangleAllFill( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriangleAllFill( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriangle2AllFill( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16TriangleAllFill( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16Triangle2AllFill( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32TriangleAllFill( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32Triangle2AllFill( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2FanAllFill( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxFanAllFill( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16FanAllFill( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32FanAllFill( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2StripAllFill( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxStripAllFill( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16StripAllFill( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32StripAllFill( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

#ifndef WINNT  // shouston - this isn't implemented in WinNT.
#ifdef K6_2
// Strip
extern void  dp2StripAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2StripAll_K62O( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2StripAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

// Indexed Strip
extern void  dp2IdxStripAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxStripAll_K62O( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16StripAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32StripAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2IdxStripAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

// Fan
extern void  dp2FanAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2FanAll_K62O( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2FanAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

// Indexed Fan
extern void  dp2IdxFanAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxFanAll_K62O( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16FanAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32FanAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2IdxFanAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

#ifdef _3DNOWDP2AAFUNCS
extern void  dp2AAFanAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAFanAll_K62O( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2AAFanAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxFanAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxFanAll_K62O( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2AAIdxFanAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAStripAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAStripAll_K62O( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2AAStripAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxStripAll_Orig( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxStripAll_K62O( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2AAIdxStripAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AATriAll_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AATriAll_K62O( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2AATriAll)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTriAll_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTriAll_K62O( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2AAIdxTriAll)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTri2All_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTri2All_K62O( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2AAIdxTri2All)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AADrawLine_Orig( RC *pRc, LPDWORD pA, LPDWORD pB, LPDWORD pZ );
extern void  dp2AADrawLine_K62O( RC *pRc, LPDWORD pA, LPDWORD pB, LPDWORD pZ );
extern void  (*dp2AADrawLine)( RC *pRc, LPDWORD pA, LPDWORD pB, LPDWORD pZ );
#else
extern void  dp2AAFanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxFanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAStripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxStripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AATriAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTriAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTri2All( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AADrawLine( RC *pRc, LPDWORD pA, LPDWORD pB, LPDWORD pZ );

#endif // _3DNOWDP2AAFUNCS

// Triangle
extern void  dp2TriangleAll_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2TriangleAll_K62O( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2TriangleAll)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );

// IndexedTriangle
extern void  dp2IdxTriangleAll_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriangleAll_K62O( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16TriangleAll_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32TriangleAll_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2IdxTriangleAll)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );

// IndexedTriangle2
extern void  dp2IdxTriangle2All_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriangle2All_K62O( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16Triangle2All_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32Triangle2All_Orig( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  (*dp2IdxTriangle2All)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2TriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTri2All_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16TriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16Tri2All_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32TriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32Tri2All_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxFanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
#else // K6_2

extern void  dp2StripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxStripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16StripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16StripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2FanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxFanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16FanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32FanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2TriangleAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriangleAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriangle2All( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16TriangleAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16Triangle2All( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32TriangleAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32Triangle2All( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2TriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTri2All_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16TriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16Tri2All_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32TriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32Tri2All_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxFanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

// Antialiasing render functions
extern void  dp2AATriAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTriAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTri2All( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAFanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxFanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAStripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxStripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AADrawLine( RC *pRc, LPDWORD pA, LPDWORD pB, LPDWORD pZ );
#endif // K6_2

#else  // WINNT

extern void  dp2StripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxStripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16StripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
/* V56K-DX8-PORT: was a duplicate of the 16-bit line above -- the 32-bit
** indexed-strip renderer was never declared for NT, though D6STRIP.C:476
** does instantiate it and the Fan/Triangle groups below both carry their
** full 16/32 pair.  D8DP2.C is the first caller to need it. */
extern void  dp2Idx32StripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2FanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxFanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16FanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32FanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2TriangleAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriangleAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriangle2All( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16TriangleAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16Triangle2All( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32TriangleAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32Triangle2All( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2TriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxTri2All_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16TriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16Tri2All_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32TriAll_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32Tri2All_SM3( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );

extern void  dp2FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2IdxFanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx16FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2Idx32FanAll_SM3( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );

// Antialiasing render functions
extern void  dp2AATriAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTriAll( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxTri2All( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAFanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxFanAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAStripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AAIdxStripAll( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType );
extern void  dp2AADrawLine( RC *pRc, LPDWORD pA, LPDWORD pB, LPDWORD pZ );
#endif // WINNT

// Multi-texture register setup function
extern void setupTexturing( RC* pRc );

// Trace functions
#ifdef FXTRACE
  extern void printTextureStageState( DWORD stage, DWORD state, DWORD value );
#else
  #define printTextureStageState  1 ? (void)0 : (void)
#endif

// Wrap macros
#define DX6_WRAP(st, wrap)                                                  \
{                                                                           \
    if (wrap) {                                                             \
        /*                                                                  \
         * Calculate the three edge diffs to determine whether this         \
         * triangle crosses the boundary.  If it does, exactly two edge     \
         * diffs should have magnitude greater than 0.5.                    \
         */                                                                 \
        float  el, er, em;                                                  \
        el = d3absval(st##c - st##a);                                       \
        er = d3absval(st##b - st##c);                                       \
        em = d3absval(st##a - st##b);                                       \
        /*                                                                  \
         * If we have two edges crossing the boundary, then we can adjust   \
         * the one vertex which is shared by those two edges by +-1 to      \
         * perform the wrapping operation.                                  \
         */                                                                 \
        if (el > DTOVALP(0.5, 24) && er > DTOVALP(0.5, 24)) {               \
            /*                                                              \
             * Shared vertex is uv##3.                                      \
             */                                                             \
            if (st##c < st##b)                                              \
                st##c += ITOVALP(1, 24);                                    \
            else                                                            \
                { st##b += ITOVALP(1, 24); st##a += ITOVALP(1, 24); }       \
        } else if (er > DTOVALP(0.5, 24) && em > DTOVALP(0.5, 24)) {        \
            /*                                                              \
             * Shared vertex is uv##2.                                      \
             */                                                             \
            if (st##b < st##a)                                              \
                st##b += ITOVALP(1, 24);                                    \
            else                                                            \
                { st##a += ITOVALP(1, 24); st##c += ITOVALP(1, 24); }       \
        } else if (em > DTOVALP(0.5, 24) && el > DTOVALP(0.5, 24)) {        \
            /*                                                              \
             * Shared vertex is uv##1.                                      \
             */                                                             \
            if (st##a < st##c)                                              \
                st##a += ITOVALP(1, 24);                                    \
            else                                                            \
                { st##c += ITOVALP(1, 24); st##b += ITOVALP(1, 24); }       \
        }                                                                   \
    }                                                                       \
}

//-------------------------------------------------------------------

#define TS pRc->textureStage

#endif // D6GLOBAL_H

#endif // DX >= 6

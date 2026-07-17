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
** File name:   d6tri2.c
**
** Description: defines macros used in geration of triangle handling code via
**              inclusion of d6tri2i.c or d6tri2k.c (for K6-3D processors)
**
**              d6tri2i.c/d6tri2k.c is included multiple times.  Each time
**              it is used as a template to generate a function to process
**              DrawPrimitive triangle lists, indexed triangle lists, or
**              indexed triangle list 2's.
**
**              See d6tri2i.c/d6tri2k.c for more information
**
** $Revision: 7$
** $Date: 10/26/00 7:58:48 AM$
**
** $Log: 
**  7    3dfx      1.2.3.3     10/26/00 Johnny Trainor  We now use
**       pRc->dwVerticesStride rather that the ahrd coded FVFO_SIZE.
**  6    3dfx      1.2.3.2     10/19/00 Johnny Trainor  Fixed build error with Dx8
**       builds.
**  5    3dfx      1.2.3.1     10/12/00 Johnny Trainor  Added some defines for
**       indexed triangle list support under DX8.
**  4    3dfx      1.2.3.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    3dfx      1.2         02/04/00 Steve Houston   Inserted #undef K6_2 under
**       WinNT builds to remove 3DNow optimizations that are not supported under
**       NT. Related to new triangle asm port from W9X.
**  2    3dfx      1.1         10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 2     6/21/99 5:31p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 1     6/02/99 6:45a Michael
** Branch from H3
**
** 14    5/24/99 5:07p Bseitsin
** Removal of Antialiasing code.
**
** 13    4/13/99 8:49a Russ
** for NT direct write builds, redefine SET macro to DirectX style since
** it clashes with the NT gdi SET macro
**
** 12    1/25/99 4:53p Peterm
** added unified header information
**
** 11    12/09/98 6:38p Sreid
** Merged latest K6 optimization code from Metabyte
**
** 10    12/09/98 6:51a Russ
** NT5 D3D changes for Banshee
**
** 9     12/06/98 1:53p Adrians
** Optimisations for WinBench99.
**
** 8     11/22/98 9:13p Andrew
** Changes to support multi-monitor
**
** 7     11/04/98 4:09p Adrians
** Updated changes to K6-2.
**
** 5     9/05/98 5:03p Adrians
** Added AA support to DX6.
** Validate will now fail arguments with COMPLEMENT or ALPHAREPLICATE.
**
** 4     9/02/98 12:35p Adrians
** DX6 multitexture change.
**
** 3     7/30/98 4:36p Adrians
** DX6 line drawing support.
**
** 2     7/24/98 1:37p Hohn
**
** 1     5/06/98 6:23p Adrians
** New DX6 files.
**
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
**
**
**
** 1     4/29/98 6:31p Adrians
** Created
*/

#include "precomp.h"

#if( DX >= 6 )

#if defined(WINNT) && !defined(CMDFIFO)
#undef SET
#define SET(hwPtr,hwRegister,data)    SETDW((hwRegister),(data))
#endif

#ifndef WINNT
#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d6fvf.h"
#include "d6global.h"
#include "d3contxt.h"
#else // ifdef WINNT
// shouston 1-29-00 : The K6-2 optimizations in this file are not yet
// implemented under WinNT.
#undef K6_2
#endif

#ifdef TnL_HAL
#include "cliprend.h"
#endif

//--------------
// Triangle list
//--------------

#define TRI_TYPE LPVOID

#ifdef TnL_HAL
#define TRI_NEXT_WITH_CLIP() \
          pA = vertices; \
          c0 = *clipcodes++; \
          vertices += pRc->dwVerticesStride; \
          pB = vertices; \
          c1 = *clipcodes++; \
          vertices += pRc->dwVerticesStride; \
          pC = vertices; \
          c2 = *clipcodes++; \
          vertices += pRc->dwVerticesStride  

#endif // TnL_HAL

#define TRI_NEXT() \
          pA = vertices; \
          vertices += pRc->dwVerticesStride; \
          pB = vertices; \
          vertices += pRc->dwVerticesStride; \
          pC = vertices; \
          vertices += pRc->dwVerticesStride

#ifdef K6_2 
  #define dp2TriangleAll        dp2TriangleAll_Orig
#else
  #define dp2TriangleAll        dp2TriangleAll
#endif // K6_2

#define dp2TriangleAllFill    dp2TriangleAllFill

// Special multi-texture modes
#define dp2TriAll_SM3         dp2TriAll_SM3

#include "d6tri2i.c"

#undef dp2TriangleAll
#undef dp2TriangleAllFill

// Special multi-texture modes
#undef  dp2TriAll_SM3

#ifdef K6_2
  #define dp2TriangleAll_K62O        dp2TriangleAll_K62O
  #include "d6tri2k.c"
  #undef  dp2TriangleAll_K62O

  void (*dp2TriangleAll)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType )
    = dp2TriangleAll_Orig;
#endif // K6_2


#undef TRI_TYPE
#undef TRI_NEXT
#ifdef TnL_HAL
#undef TRI_NEXT_WITH_CLIP
#endif

//----------------------
// Indexed triangle list
//----------------------

#ifdef TnL_HAL
#define TRI_NEXT_WITH_CLIP() \
          pA = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST)idx)->wV1 * pRc->dwVerticesStride]; \
          c0 = clipcodes[((LPD3DHAL_DP2INDEXEDTRIANGLELIST)idx)->wV1]; \
          pB = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST)idx)->wV2 * pRc->dwVerticesStride]; \
          c1 = clipcodes[((LPD3DHAL_DP2INDEXEDTRIANGLELIST)idx)->wV2]; \
          pC = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST)idx)->wV3 * pRc->dwVerticesStride]; \
          c2 = clipcodes[((LPD3DHAL_DP2INDEXEDTRIANGLELIST)idx)->wV3]; \
          idx += sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST) 
#endif

#define TRI_NEXT() \
          pA = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST)idx)->wV1 * pRc->dwVerticesStride]; \
          pB = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST)idx)->wV2 * pRc->dwVerticesStride]; \
          pC = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST)idx)->wV3 * pRc->dwVerticesStride]; \
          idx += sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST)

#ifdef K6_2
 #define dp2TriangleAll        dp2IdxTriangleAll_Orig
#else
 #define dp2TriangleAll        dp2IdxTriangleAll
#endif

#define dp2TriangleAllFill    dp2IdxTriangleAllFill

// Special multi-texture modes
#define dp2TriAll_SM3         dp2IdxTriAll_SM3

#include "d6tri2i.c"

#undef dp2TriangleAll
#undef dp2TriangleAllFill

// Special multi-texture modes
#undef  dp2TriAll_SM3

#ifdef K6_2
  #define dp2TriangleAll_K62O        dp2IdxTriangleAll_K62O
  #include "d6tri2k.c"
  #undef  dp2TriangleAll_K62O

  void  (*dp2IdxTriangleAll)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType )
    = dp2IdxTriangleAll_Orig;
#endif

#undef TRI_TYPE
#undef TRI_NEXT
#ifdef TnL_HAL
#undef TRI_NEXT_WITH_CLIP
#endif

//-----------------------
// Indexed triangle list2
//-----------------------

#ifdef TnL_HAL
#define TRI_NEXT_WITH_CLIP() \
          pA = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST2)idx)->wV1 * pRc->dwVerticesStride]; \
          c0 = clipcodes[((LPD3DHAL_DP2INDEXEDTRIANGLELIST2)idx)->wV1]; \
          pB = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST2)idx)->wV2 * pRc->dwVerticesStride]; \
          c1 = clipcodes[((LPD3DHAL_DP2INDEXEDTRIANGLELIST2)idx)->wV2]; \
          pC = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST2)idx)->wV3 * pRc->dwVerticesStride]; \
          c2 = clipcodes[((LPD3DHAL_DP2INDEXEDTRIANGLELIST2)idx)->wV3]; \
          idx += sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST2)  
#endif

#define TRI_NEXT() \
          pA = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST2)idx)->wV1 * pRc->dwVerticesStride]; \
          pB = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST2)idx)->wV2 * pRc->dwVerticesStride]; \
          pC = &vertices[((LPD3DHAL_DP2INDEXEDTRIANGLELIST2)idx)->wV3 * pRc->dwVerticesStride]; \
          idx += sizeof(D3DHAL_DP2INDEXEDTRIANGLELIST2)

#ifdef K6_2
#define dp2TriangleAll        dp2IdxTriangle2All_Orig
#else
  #define dp2TriangleAll        dp2IdxTriangle2All
#endif

#define dp2TriangleAllFill    dp2IdxTriangle2AllFill

// Special multi-texture modes
#define dp2TriAll_SM3         dp2IdxTri2All_SM3

#include "d6tri2i.c"

#undef dp2TriangleAll
#undef dp2TriangleAllFill

// Special multi-texture modes
#undef  dp2TriAll_SM3

#ifdef K6_2
  #define dp2TriangleAll_K62O        dp2IdxTriangle2All_K62O
  #include "d6tri2k.c"
  #undef dp2TriangleAll_K62O

  void  (*dp2IdxTriangle2All)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType )
    = dp2IdxTriangle2All_Orig;
#endif // K6_2

#undef TRI_TYPE
#undef TRI_NEXT
#ifdef TnL_HAL
#undef TRI_NEXT_WITH_CLIP
#endif

#if (DX >= 8)

//----------------------
// Indexed triangle list (16-Bit Index)
//----------------------

#ifdef TnL_HAL
#define TRI_NEXT_WITH_CLIP() \
          pA = &vertices[(*(((WORD *) idx))) * pRc->dwVerticesStride]; \
          c0 = clipcodes[(*(((WORD *) idx)++))]; \
          pB = &vertices[(*(((WORD *) idx))) * pRc->dwVerticesStride]; \
          c1 = clipcodes[(*(((WORD *) idx)++))]; \
          pC = &vertices[(*(((WORD *) idx))) * pRc->dwVerticesStride]; \
          c2 = clipcodes[(*(((WORD *) idx)++))];
#endif

#define TRI_NEXT() \
          pA = &vertices[(*(((WORD *) idx)++)) * pRc->dwVerticesStride]; \
          pB = &vertices[(*(((WORD *) idx)++)) * pRc->dwVerticesStride]; \
          pC = &vertices[(*(((WORD *) idx)++)) * pRc->dwVerticesStride];

#ifdef K6_2
 #define dp2TriangleAll        dp2Idx16TriangleAll_Orig
#else
 #define dp2TriangleAll        dp2Idx16TriangleAll
#endif

#define dp2TriangleAllFill    dp2Idx16TriangleAllFill

// Special multi-texture modes
#define dp2TriAll_SM3         dp2Idx16TriAll_SM3

#include "d6tri2i.c"

#undef dp2TriangleAll
#undef dp2TriangleAllFill

// Special multi-texture modes
#undef  dp2TriAll_SM3

#ifdef K6_2
  #define dp2TriangleAll_K62O        dp2Idx16Triangle2All_K62O
  #include "d6tri2k.c"
  #undef dp2TriangleAll_K62O

  void  (*dp2Idx16TriangleAll)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType )
    = dp2Idx16TriangleAll_Orig;
#endif

#undef TRI_TYPE
#undef TRI_NEXT
#ifdef TnL_HAL
#undef TRI_NEXT_WITH_CLIP
#endif

//----------------------
// Indexed triangle list (32-Bit Index)
//----------------------

#ifdef TnL_HAL
#define TRI_NEXT_WITH_CLIP() \
          pA = &vertices[(*(((DWORD *) idx))) * pRc->dwVerticesStride]; \
          c0 = clipcodes[(*(((DWORD *) idx)++))]; \
          pB = &vertices[(*(((DWORD *) idx))) * pRc->dwVerticesStride]; \
          c1 = clipcodes[(*(((DWORD *) idx)++))]; \
          pC = &vertices[(*(((DWORD *) idx))) * pRc->dwVerticesStride]; \
          c2 = clipcodes[(*(((DWORD *) idx)++))];
#endif

#define TRI_NEXT() \
          pA = &vertices[(*(((DWORD *) idx)++)) * pRc->dwVerticesStride]; \
          pB = &vertices[(*(((DWORD *) idx)++)) * pRc->dwVerticesStride]; \
          pC = &vertices[(*(((DWORD *) idx)++)) * pRc->dwVerticesStride];

#ifdef K6_2
 #define dp2TriangleAll        dp2Idx32TriangleAll_Orig
#else
 #define dp2TriangleAll        dp2Idx32TriangleAll
#endif

#define dp2TriangleAllFill    dp2Idx32TriangleAllFill

// Special multi-texture modes
#define dp2TriAll_SM3         dp2Idx32TriAll_SM3

#include "d6tri2i.c"

#undef dp2TriangleAll
#undef dp2TriangleAllFill

// Special multi-texture modes
#undef  dp2TriAll_SM3

#ifdef K6_2
  #define dp2TriangleAll_K62O        dp2Idx32Triangle2All_K62O
  #include "d6tri2k.c"
  #undef dp2TriangleAll_K62O

  void  (*dp2Idx32TriangleAll)( RC *pRc, DWORD count, LPBYTE idx, LPDWORD vertices, DWORD vertexType )
    = dp2Idx32TriangleAll_Orig;
#endif

#undef TRI_TYPE
#undef TRI_NEXT
#ifdef TnL_HAL
#undef TRI_NEXT_WITH_CLIP
#endif

#endif // DX8

#endif // DX6

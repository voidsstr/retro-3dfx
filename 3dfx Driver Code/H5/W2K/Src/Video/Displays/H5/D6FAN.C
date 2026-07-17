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
** File name:   d6fan.c
**
** Description: defines macros used in geration of fan handling code via
**              inclusion of d6fani.c or d6fank.c (for 3DNow processors)
**
**              d6fani.c/d6fank.c is included multiple times.  The first
**              time the generated functions are for standard DrawPrimitive
**              calls.  The second time the generated functions are for
**              indexed DrawPrimitive calls.
**
**              See d6fani.c/d6fank.c for more information
**
** $Revision: 5$
** $Date: 10/31/00 1:59:33 AM$
**
** $Log: 
**  5    3dfx      1.2.2.1     10/31/00 Johnny Trainor  New Indexed Triangle fan
**       code. We now use pRc->dwVerticesStride rather that the hard coded
**       FVFO_SIZE.
** 
**  4    3dfx      1.2.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    3dfx      1.2         02/04/00 Steve Houston   Inserted #undef K6_2 under
**       WinNT builds to remove 3DNow optimizations that are not supported under
**       NT. Related to new triangle asm port from W9X.
**  2    3dfx      1.1         10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 2     6/21/99 5:25p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 1     6/02/99 6:45a Michael
** Branch from H3
**
** 14    5/24/99 5:06p Bseitsin
** Removal of Antialiasing code.
**
** 13    4/13/99 8:46a Russ
** for NT direct write builds, redefine SET macro to DirectX style since
** it clashes with the NT gdi SET macro
**
** 12    2/03/99 3:38p Sreid
** K6/K7 WC enable from contractor
**
** 11    1/25/99 4:45p Peterm
** added unified header information
**
** 10    12/09/98 6:38p Sreid
** Merged latest K6 optimization code from Metabyte
**
** 9     12/09/98 6:45a Russ
** NT5 D3D changes for Banshee
**
** 8     12/06/98 1:53p Adrians
** Optimisations for WinBench99.
**
** 7     11/22/98 9:10p Andrew
** Changes to support multi-monitor
**
** 6     11/04/98 4:09p Adrians
** Updated changes to K6-2.
**
** 5     10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
**
** 4     9/05/98 5:02p Adrians
** Added AA support to DX6.
** Validate will now fail arguments with COMPLEMENT or ALPHAREPLICATE.
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

//-------------
// Triangle fan
//-------------

#define TRI_INIT \
          pA = vertices; \
          vertices += pRc->dwVerticesStride; \
          pC = vertices; \
          vertices += pRc->dwVerticesStride

#define TRI_NEXT  \
          pB = pC; \
          pC = vertices; \
          vertices += pRc->dwVerticesStride

#ifdef TnL_HAL

#define TRI_INIT_WITH_CLIP \
          pA = vertices; \
          c0 = *clipcodes++; \
          vertices += pRc->dwVerticesStride; \
          pC = vertices; \
          c2 = *clipcodes++; \
          vertices += pRc->dwVerticesStride

#define TRI_NEXT_WITH_CLIP  \
          pB = pC; \
          c1 = c2; \
          pC = vertices; \
          c2 = *clipcodes++; \
          vertices += pRc->dwVerticesStride

          // Check if ve just clipped a triangle
          // If so, the original Vertex pointers 
          // need to be replaced
#define TRI_CLIP_RESTORE \
          if (RestoreVertexPointers) \
          {                       \
            pA = pAOrig;          \
            pB = pBOrig;          \
            pC = pCOrig;          \
            RestoreVertexPointers = 0; \
          }                       

          // Save off our vertex pointers so that they may be restored
          // again so that we can restart strips.
#define TRI_CLIP_SAVE \
          RestoreVertexPointers = 1; \
          pAOrig = pA; \
          pBOrig = pB; \
          pCOrig = pC


#endif // TnL_HAL

#ifdef K6_2
  #define dp2FanAll       dp2FanAll_Orig
#else
  #define dp2FanAll       dp2FanAll
#endif // K6_2

#define dp2FanAllFill   dp2FanAllFill

// Special multi-texture modes
#define dp2FanAll_SM3   dp2FanAll_SM3

#include "d6fani.c"

#ifdef K6_2
  #include "d6fank.c"
#endif

// Special multi-texture modes
#undef dp2FanAll_SM3
#undef dp2FanAll
#undef dp2FanAllFill

#ifdef K6_2
  void  (*dp2FanAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType )
   = dp2FanAll_Orig;
#endif // K6_2

#undef TRI_INIT
#undef TRI_NEXT

#ifdef TnL_HAL
#undef TRI_INIT_WITH_CLIP
#undef TRI_NEXT_WITH_CLIP
#endif // TnL_HAL

//---------------------
// Indexed Triangle fan
//---------------------

#define TRI_INIT \
          pA = &vertices[idx[0] * pRc->dwVerticesStride]; \
          pC = &vertices[idx[1] * pRc->dwVerticesStride]; \
          idx += 2

#define TRI_NEXT \
          pB = pC; \
          pC = &vertices[idx[0] * pRc->dwVerticesStride]; \
          idx += 1

#ifdef TnL_HAL

#define TRI_INIT_WITH_CLIP \
          pA = &vertices[idx[0] * pRc->dwVerticesStride]; \
          c0 = clipcodes[idx[0]]; \
          pC = &vertices[idx[1] * pRc->dwVerticesStride]; \
          c2 = clipcodes[idx[1]]; \
          idx += 2

#define TRI_NEXT_WITH_CLIP \
          pB = pC; \
          c1 = c2; \
          pC = &vertices[idx[0] * pRc->dwVerticesStride]; \
          c2 = clipcodes[idx[0]]; \
          idx += 1

#endif // TnL_HAL

#ifdef K6_2
#define dp2FanAll       dp2IdxFanAll_Orig
#else  // K6_2
#define dp2FanAll       dp2IdxFanAll
#endif // K6_2

#define dp2FanAllFill   dp2IdxFanAllFill

// Special multi-texture modes
#define dp2FanAll_SM3   dp2IdxFanAll_SM3

#include "d6fani.c"

// Special multi-texture modes
#undef dp2FanAll_SM3
#undef dp2FanAll
#undef dp2FanAllFill

#ifdef K6_2
  #define dp2FanAll_K62O       dp2IdxFanAll_K62O
  #include "d6fank.c"
  #undef dp2FanAll_K62O
#endif


#ifdef K6_2
  void  (*dp2IdxFanAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType )
   = dp2IdxFanAll_Orig;
#endif // K6_2

#undef TRI_INIT
#undef TRI_NEXT

#ifdef TnL_HAL
#undef TRI_INIT_WITH_CLIP
#undef TRI_NEXT_WITH_CLIP
#endif // TnL_HAL

#if (DX >= 8)

//---------------------
// Indexed Triangle fan (16-bit)
//---------------------

#define TRI_INIT \
          pA = &vertices[(*(((WORD *) idx)++)) * pRc->dwVerticesStride]; \
          pC = &vertices[(*(((WORD *) idx)++)) * pRc->dwVerticesStride]; \

#define TRI_NEXT \
          pB = pC; \
          pC = &vertices[(*(((WORD *) idx)++)) * pRc->dwVerticesStride]; \

#ifdef TnL_HAL

#define TRI_INIT_WITH_CLIP \
          pA = &vertices[(*(((WORD *) idx))) * pRc->dwVerticesStride]; \
          c0 = clipcodes[(*(((WORD *) idx)++))]; \
          pC = &vertices[(*(((WORD *) idx))) * pRc->dwVerticesStride]; \
          c2 = clipcodes[(*(((WORD *) idx)++))]; \

#define TRI_NEXT_WITH_CLIP \
          pB = pC; \
          c1 = c2; \
          pC = &vertices[(*(((WORD *) idx))) * pRc->dwVerticesStride]; \
          c2 = clipcodes[(*(((WORD *) idx)++))]; \

#endif // TnL_HAL

#ifdef K6_2
#define dp2FanAll       dp2Idx16FanAll_Orig
#else  // K6_2
#define dp2FanAll       dp2Idx16FanAll
#endif // K6_2

#define dp2FanAllFill   dp2Idx16FanAllFill

// Special multi-texture modes
#define dp2FanAll_SM3   dp2Idx16FanAll_SM3

#include "d6fani.c"

// Special multi-texture modes
#undef dp2FanAll_SM3
#undef dp2FanAll
#undef dp2FanAllFill

#ifdef K6_2
  #define dp2FanAll_K62O       dp2Idx16FanAll_K62O
  #include "d6fank.c"
  #undef dp2FanAll_K62O
#endif

#ifdef K6_2
  void  (*dp2Idx16FanAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType )
   = dp2Idx16FanAll_Orig;
#endif // K6_2

#undef TRI_INIT
#undef TRI_NEXT

#ifdef TnL_HAL
#undef TRI_INIT_WITH_CLIP
#undef TRI_NEXT_WITH_CLIP
#endif // TnL_HAL

//---------------------
// Indexed Triangle fan (32-bit)
//---------------------

#define TRI_INIT \
          pA = &vertices[(*(((DWORD *) idx)++)) * pRc->dwVerticesStride]; \
          pC = &vertices[(*(((DWORD *) idx)++)) * pRc->dwVerticesStride]; \

#define TRI_NEXT \
          pB = pC; \
          pC = &vertices[(*(((DWORD *) idx)++)) * pRc->dwVerticesStride]; \

#ifdef TnL_HAL

#define TRI_INIT_WITH_CLIP \
          pA = &vertices[(*(((DWORD *) idx))) * pRc->dwVerticesStride]; \
          c0 = clipcodes[(*(((DWORD *) idx)++))]; \
          pC = &vertices[(*(((DWORD *) idx))) * pRc->dwVerticesStride]; \
          c2 = clipcodes[(*(((DWORD *) idx)++))]; \

#define TRI_NEXT_WITH_CLIP \
          pB = pC; \
          c1 = c2; \
          pC = &vertices[(*(((DWORD *) idx))) * pRc->dwVerticesStride]; \
          c2 = clipcodes[(*(((DWORD *) idx)++))]; \

#endif // TnL_HAL

#ifdef K6_2
#define dp2FanAll       dp2Idx32FanAll_Orig
#else  // K6_2
#define dp2FanAll       dp2Idx32FanAll
#endif // K6_2

#define dp2FanAllFill   dp2Idx32FanAllFill

// Special multi-texture modes
#define dp2FanAll_SM3   dp2Idx32FanAll_SM3

#include "d6fani.c"

// Special multi-texture modes
#undef dp2FanAll_SM3
#undef dp2FanAll
#undef dp2FanAllFill

#ifdef K6_2
  #define dp2FanAll_K62O       dp2Idx32FanAll_K62O
  #include "d6fank.c"
  #undef dp2FanAll_K62O
#endif

#ifdef K6_2
  void  (*dp2Idx32FanAll)( RC *pRc, DWORD count, LPWORD idx, LPDWORD vertices, DWORD vertexType )
   = dp2Idx32FanAll_Orig;
#endif // K6_2

#undef TRI_INIT
#undef TRI_NEXT

#ifdef TnL_HAL
#undef TRI_INIT_WITH_CLIP
#undef TRI_NEXT_WITH_CLIP
#endif // TnL_HAL

#endif // DX 8

#endif // DX 6

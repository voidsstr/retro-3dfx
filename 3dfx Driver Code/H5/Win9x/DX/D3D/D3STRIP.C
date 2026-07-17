/*
** Copyright (c) 1997, 3Dfx Interactive, Inc.
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
** $Log: 
**  3    3dfx      1.0.1.1     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.0.1.0     09/22/00 Johnny Trainor  Preparation for DX8 support
**       in the driver. Modifications so that we can build the Win9x driver using
**       the Win98 DDK and the DX8 DDK. 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 1     6/02/99 6:44a Michael
** Branch from H3
** 
** 5     5/25/99 11:31a Bseitsin
** Remove Antialiasing baggage.
** 
** 4     11/22/98 9:03p Andrew
** Changes to support multi-monitor
** 
** 3     7/24/98 1:37p Hohn
** 
** 2     6/01/98 12:20p Adrians
** Added aa to DrawPrimitives.
** 
** 1     3/27/98 11:19a Adrians
** New DrawPrimitive code.
*/

#include "precomp.h"

// Fix for building with Windows 98 DDK (Must include DDrawI first)
#include "ddrawi.h"
#include "d3dhal.h"
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h" 
#include "fifomgr.h"
#include "d3contxt.h"

// Strip
#define TRIALL_INIT() \
          pA = vertices++; \
          pC = vertices++
                              
#define TRIALL_NEXT() \
          if( (flip = !flip) ) \
          { \
            pA = pA; \
            pB = pC; \
            pF = pA; \
          } \
          else \
          { \
            pA = pC; \
            pB = pB; \
            pF = pB; \
          } \
          pC = vertices++
                              
#define TRI_INIT() \
          pA = vertices++; \
          pB = vertices++; \
          pC = vertices++
          
#define TRI_NEXT() \
          pA = pB; \
          pB = pC; \
          pC = vertices++

#define dpDrawStripAll   dpDrawStripAll
#define dpDrawStripIZ    dpDrawStripIZ
#define dpDrawStripIZT   dpDrawStripIZT
#define dpDrawStripIZTH  dpDrawStripIZTH

#include "d3stripi.c"

#undef dpDrawStripAll
#undef dpDrawStripIZ
#undef dpDrawStripIZT
#undef dpDrawStripIZTH
#undef TRIALL_INIT
#undef TRIALL_NEXT
#undef TRI_INIT
#undef TRI_NEXT


// Indexed Strip
#define TRIALL_INIT() \
          pA = &vertices[*triIndex]; \
          triIndex++; \
          pC = &vertices[*triIndex]; \
          triIndex++
          
#define TRIALL_NEXT() \
          if( (flip = !flip) ) \
          { \
            pA = pA; \
            pB = pC; \
            pF = pA; \
          } \
          else \
          { \
            pA = pC; \
            pB = pB; \
            pF = pB; \
          } \
          pC = &vertices[*triIndex]; \
          triIndex++; \
          
#define TRI_INIT() \
          pA = &vertices[*triIndex]; \
          triIndex++; \
          pB = &vertices[*triIndex]; \
          triIndex++; \
          pC = &vertices[*triIndex]; \
          triIndex++
          
#define TRI_NEXT() \
          pA = pB; \
          pB = pC; \
          pC = &vertices[*triIndex]; \
          triIndex++
          
#define dpDrawStripAll   dpDrawIndexedStripAll
#define dpDrawStripIZ    dpDrawIndexedStripIZ
#define dpDrawStripIZT   dpDrawIndexedStripIZT
#define dpDrawStripIZTH  dpDrawIndexedStripIZTH

#include "d3stripi.c"

#undef dpDrawStripAll
#undef dpDrawStripIZ
#undef dpDrawStripIZT
#undef dpDrawStripIZTH
#undef TRIALL_INIT
#undef TRIALL_NEXT
#undef TRI_INIT
#undef TRI_NEXT


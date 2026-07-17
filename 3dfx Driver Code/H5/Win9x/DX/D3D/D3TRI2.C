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
** 21    5/25/99 11:31a Bseitsin
** Remove Antialiasing baggage.
** 
** 20    11/22/98 9:06p Andrew
** Changes to support multi-monitor
** 
** 19    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
** 
** 18    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
** 
** No more conditional compilation of AA.
** 
** 17    7/24/98 1:37p Hohn
** 
** 16    6/01/98 12:20p Adrians
** Added aa to DrawPrimitives.
** 
** 15    3/27/98 11:18a Adrians
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

//--------------
// Triangle list
//--------------

#define TRI_NEXT() \
          pA = vertices++; \
          pB = vertices++; \
          pC = vertices++

#define dpDrawTriangleAll   dpDrawTriangleAll
#define dpDrawTriangleIZT   dpDrawTriangleIZT
#define dpDrawTriangleIZ    dpDrawTriangleIZ
#define dpDrawTriangleIZTH  dpDrawTriangleIZTH

#include "d3tri2i.c"

#undef dpDrawTriangleAll
#undef dpDrawTriangleIZT
#undef dpDrawTriangleIZ
#undef dpDrawTriangleIZTH
#undef TRI_NEXT

//----------------------
// Indexed triangle list 
//----------------------

#define TRI_NEXT() \
          pA = &vertices[*triIndex]; \
          triIndex++; \
          pB = &vertices[*triIndex]; \
          triIndex++; \
          pC = &vertices[*triIndex]; \
          triIndex++

#define dpDrawTriangleAll   dpDrawIndexedTriangleAll
#define dpDrawTriangleIZT   dpDrawIndexedTriangleIZT
#define dpDrawTriangleIZ    dpDrawIndexedTriangleIZ
#define dpDrawTriangleIZTH  dpDrawIndexedTriangleIZTH

#include "d3tri2i.c"

#undef dpDrawTriangleAll
#undef dpDrawTriangleIZT
#undef dpDrawTriangleIZ
#undef dpDrawTriangleIZTH
#undef TRI_NEXT

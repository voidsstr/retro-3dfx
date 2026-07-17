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
** File name:   d6fvf.h
**
** Description: DirectX 6.0 Flexible Vertex Format ddi header
**
** $Revision: 4$
** $Date: 10/23/00 2:11:31 AM$
**
** $Log: 
**  4    3dfx      1.1.2.1     10/23/00 Johnny Trainor  Updated so we no longer use
**       the global variable vertexType.
**  3    3dfx      1.1.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         11/17/99 Russ Lind       added FVFOT_XYZWST1 to
**       FVFOTTYPE enum for the 0x184 vertex type
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 2     6/21/99 5:26p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 1     6/02/99 6:45a Michael
** Branch from H3
**
** 10    1/26/99 5:28p Peterm
** Added unified header information
**
** 9     12/09/98 6:38p Sreid
** Merged latest K6 optimization code from Metabyte
**
** 8     11/06/98 5:56p Adrians
** Add 2 new FVF formats for WinBench99.
**
** 7     10/17/98 10:59a Russ
** include d3dnthal.h for NT instead of d3dhal.h
**
** 6     10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
**
** 5     9/05/98 5:02p Adrians
** Added AA support to DX6.
** Validate will now fail arguments with COMPLEMENT or ALPHAREPLICATE.
**
** 4     8/14/98 8:11p Adrians
** Add support for up to 8 texture coordinates.
**
** 1     8/07/98 3:51p Adrians
**
** 3     7/29/98 7:25p Adrians
** DX6 changes.
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

#if( DX >= 6 )

#ifndef _D6FVF_H
#define _D6FVF_H

#if defined(WINNT) && (_WIN32_WINNT >= 0x0500)
#include <d3dnthal.h>
#else
#include <d3dhal.h>
#endif
#include "d3global.h"

#define FVFO_SIZE     ( fvfOffsetTable[pRc->dwVertexType].size )
#define FVFO_SX       ( fvfOffsetTable[pRc->dwVertexType].sx )
#define FVFO_SY       ( fvfOffsetTable[pRc->dwVertexType].sy )
#define FVFO_SZ       ( fvfOffsetTable[pRc->dwVertexType].sz )
#define FVFO_RHW      ( fvfOffsetTable[pRc->dwVertexType].rhw )
#define FVFO_COLOR    ( fvfOffsetTable[pRc->dwVertexType].color )
#define FVFO_SPECULAR ( fvfOffsetTable[pRc->dwVertexType].specular )
#define FVFO_TU       ( fvfOffsetTable[pRc->dwVertexType].tu )
#define FVFO_TV       ( fvfOffsetTable[pRc->dwVertexType].tv )
#define FVFO_TU1      ( fvfOffsetTable[pRc->dwVertexType].tu1 )
#define FVFO_TV1      ( fvfOffsetTable[pRc->dwVertexType].tv1 )
#define FVFO_TU2      ( fvfOffsetTable[pRc->dwVertexType].tu2 )
#define FVFO_TV2      ( fvfOffsetTable[pRc->dwVertexType].tv2 )
#define FVFO_TU3      ( fvfOffsetTable[pRc->dwVertexType].tu3 )
#define FVFO_TV3      ( fvfOffsetTable[pRc->dwVertexType].tv3 )
#define FVFO_TU4      ( fvfOffsetTable[pRc->dwVertexType].tu4 )
#define FVFO_TV4      ( fvfOffsetTable[pRc->dwVertexType].tv4 )
#define FVFO_TU5      ( fvfOffsetTable[pRc->dwVertexType].tu5 )
#define FVFO_TV5      ( fvfOffsetTable[pRc->dwVertexType].tv5 )
#define FVFO_TU6      ( fvfOffsetTable[pRc->dwVertexType].tu6 )
#define FVFO_TV6      ( fvfOffsetTable[pRc->dwVertexType].tv6 )
#define FVFO_TU7      ( fvfOffsetTable[pRc->dwVertexType].tu7 )
#define FVFO_TV7      ( fvfOffsetTable[pRc->dwVertexType].tv7 )

typedef struct FVFOFFSETTABLE {
  DWORD size; // This is specified in DWORD's
  DWORD sx;
  DWORD sy;
  DWORD sz;
  DWORD rhw;
  DWORD color;
  DWORD specular;
  DWORD tu;
  DWORD tv;
  DWORD tu1;
  DWORD tv1;
  DWORD tu2;
  DWORD tv2;
  DWORD tu3;
  DWORD tv3;
  DWORD tu4;
  DWORD tv4;
  DWORD tu5;
  DWORD tv5;
  DWORD tu6;
  DWORD tv6;
  DWORD tu7;
  DWORD tv7;
} FVFOFFSETTABLE, *LPFVFOFFSETTABLE;

typedef enum {
  FVFOT_TLVERTEX,
  FVFOT_XYZWDT1,
  FVFOT_XYZWDT2,
  FVFOT_XYZWDST2,
  FVFOT_XYZWST2,
  FVFOT_XYZWDS,
  FVFOT_XYZWT2,
  FVFOT_XYZWD,
  FVFOT_XYZWT1,
  FVFOT_XYZWDST3,
  FVFOT_XYZWDT3,
  FVFOT_XYZWST1,
  FVFOT_CUSTOM
} FVFOTTYPE;

extern FVFOFFSETTABLE fvfOffsetTable[];

#define FLTP(pa)  ((float*)pa)
#define INTP(pa)  ((int*)pa)

#endif  // _D6FVF_H

#endif  // DIRECT3d_VERSION >= 0x0600

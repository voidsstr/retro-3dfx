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
** File name:   d6fvf.c
**
** Description: DirectX 6.0 Flexible Vertex Format table
**
** $Revision: 3$
** $Date: 10/11/00 8:43:13 PM$
**
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         11/17/99 Russ Lind       added XYZWST1 to
**       fvfOffsetTable for the 0x184 vertex type
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
** 9     12/09/98 6:47a Russ
** NT5 D3D changes for Banshee
**
** 8     11/06/98 5:56p Adrians
** Add 2 new FVF formats for WinBench99.
**
** 7     10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
**
** 6     9/24/98 11:51a Hanson
** Dx6 Optimizations
**
** 5     8/14/98 8:11p Adrians
** Add support for up to 8 texture coordinates.
**
** 1     8/07/98 3:51p Adrians
**
** 4     8/04/98 10:21a Adrians
** Mark unused offsets in FVF table with 128.
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
//------------------------------------------------------------------------------

#include "precomp.h"

#ifndef WINNT
#include <d3dhal.h>
#include "d3global.h"
#include "d6fvf.h"
#endif

#if( DX >= 6 )

FVFOFFSETTABLE fvfOffsetTable[] = {
  //  size,  x,   y,   z,   w,   d,   s,   u,   v, u1, v1, u2, v2, u3, v3, u4, v4, u5, v5, u6, v6, u7, v7

  // TLVERTEX format
     {  8,   0,   1,   2,   3,   4,   5,   6,   7, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWDT1
     {  7,   0,   1,   2,   3,   4, 128,   5,   6, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWDT2
     {  9,   0,   1,   2,   3,   4, 128,   5,   6,   7,   8, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWDST2
     {  10,  0,   1,   2,   3,   4,   5,   6,   7,   8,   9, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWST2
     {  9,   0,   1,   2,   3, 128,   4,   5,   6,   7,   8, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWDS
     {  6,   0,   1,   2,   3,   4,   5,   0,   0, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWT2
     {  8,   0,   1,   2,   3, 128, 128,   4,   5,   6,   7, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWD
     {  5,   0,   1,   2,   3,   4, 128,   0,   0, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWT1
     {  6,   0,   1,   2,   3, 128, 128,   4,   5, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWDST3
     {  12,  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWDT3
     {  11,  0,   1,   2,   3,   4, 128,   5,   6,   7,   8,   9,  10, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // XYZWST1
     {  7,   0,   1,   2,   3, 128,   4,   5,   6, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
  // CUSTOM
     {  0, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128 },
};

#endif // DX >= 6

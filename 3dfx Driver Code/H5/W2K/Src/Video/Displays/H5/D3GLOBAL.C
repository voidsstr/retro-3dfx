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
**  4    3dfx      1.2.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    3dfx      1.2         01/11/00 Russ Lind       removed CMDFIFOBUMP global
**       var
**  2    3dfx      1.1         01/05/00 Geoff Bullard   Added CMDFIFO_CHECKBUMP
**       macro to CMDFIFO_CHECKROOM.  Rec'd from NVH 1/5/99.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 4     7/20/99 11:15a Russ
** changes for V3/Napalm runtime check
**
** 3     6/21/99 5:20p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 1     12/09/98 7:34a Russ
** NT5 D3D changes for NT
**
** 13    11/22/98 8:58p Andrew
** Changes to support multi-monitor
**
** 12    11/08/98 3:22p Hanson
** Improvements for Winbench 99
**
** 11    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
**
** 10    10/08/98 2:34a Hanson
** Continued K6 3D cleanup
**
** 9     10/08/98 1:47a Hanson
** K6 3D cleanup
**
** 8     10/04/98 5:39p Hanson
** Added check and flags setting for non P6's
**
** 7     7/24/98 1:37p Hohn
**
** 6     5/06/98 6:08p Adrians
** Changes for DX6 into DX5 driver.
**
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
**
**
**
** 1     4/29/98 6:31p Adrians
** Created
 *
 * 5     11/25/97 4:23p Suninn
 * #ifdef out d3Global declaration for H3
 *
 * 4     9/15/97 3:44p Adrians
 * Tidyup Fan and Strip code.
 * Removed some int to float routines
 * Changed lines and points to send wstz with all vertices
 * Converted tabs to spaces in some files
 *
 * 3     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#include "precomp.h"

FxU32 _cpu_type=0;


#if( DX >= 6 )

DrawTriFanImm_Type*  DrawTriFanImmAsm  =DrawTriFanImmAsm_P5;

DrawTriEdge_Type*    DrawTriEdge2Asm   =DrawTriEdge2Asm_P5;
DrawTriEdge_Type*    DrawTriEdge2GAsm  =DrawTriEdge2GAsm_P5;
DrawTriEdge_Type*    DrawTriEdge2FAsm  =DrawTriEdge2FAsm_P5;

Prefetch_TriIdx2_Type*   PrefetchTriIdx2_12_Asm =PrefetchTriIdx2_12_Asm_P5;
Prefetch_TriIdx2_Type*   PrefetchTriIdx2_11_Asm =PrefetchTriIdx2_11_Asm_P5;
Prefetch_TriIdx2_Type*   PrefetchTriIdx2_10_Asm =PrefetchTriIdx2_10_Asm_P5;
Prefetch_TriIdx2_Type*   PrefetchTriIdx2_9_Asm  =PrefetchTriIdx2_9_Asm_P5;
Prefetch_TriIdx2_Type*   PrefetchTriIdx2_8_Asm  =PrefetchTriIdx2_8_Asm_P5;
Prefetch_TriIdx2_Type*   PrefetchTriIdx2_7_Asm  =PrefetchTriIdx2_7_Asm_P5;
Prefetch_TriIdx2_Type*   PrefetchTriIdx2_6G_Asm =PrefetchTriIdx2_6G_Asm_P5;
Prefetch_TriIdx2_Type*   PrefetchTriIdx2_6T_Asm =PrefetchTriIdx2_6T_Asm_P5;
Prefetch_TriIdx2_Type*   PrefetchTriIdx2_5_Asm  =PrefetchTriIdx2_5_Asm_P5;

#endif   /* (DX >= 6) */

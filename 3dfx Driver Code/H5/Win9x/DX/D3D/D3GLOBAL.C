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
** File name: d3global.c
**
** Description: defines global data available across all processes
**
** $Revision: 4$
** $Date: 10/11/00 8:47:43 PM$
**
** $Log: 
**  4    3dfx      1.0.1.2     10/11/00 Brent           Forced check in to enforce
**       branching.
**  3    3dfx      1.0.1.1     08/21/00 Allen Hansen    added constants used by
**       WRAP_3DNow macro
**  2    3dfx      1.0.1.0     07/24/00 Allen Hansen    Rewrote cpu detection code,
**       cpuType is now defined in cpu.h, all new code should be protected by
**       #ifdef WINNT
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 3     8/14/99 10:02a Cwilcox
** Removed obsolete Voodoo2 code.
** 
** 2     7/28/99 11:59p Bseitsin
** Changes to enable DX7 for W9x.
** 
** 1     6/02/99 6:40a Michael
** Branch from H3
** 
** 16    5/30/99 5:42p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
** 
** 15    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
** 
** 14    1/25/99 4:46p Peterm
** added unified header information
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
#define INITGUID

#include "d3global.h"

#ifdef WINNT	// OLD CPUID CODE
FxU32 _cpu_type=0;
#endif


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

// These get used by the WRAP_3DNow macro
const float const_m1=-1.0f;
const float const_5=0.5f;
const float const_1=1.0f;

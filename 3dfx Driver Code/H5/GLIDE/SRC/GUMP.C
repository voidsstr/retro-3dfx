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
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished -
** rights reserved under the Copyright Laws of the United States.
**
** $Header: gump.c, 2, 10/11/00 8:22:01 PM, Brent$
** $Log: 
**  2    3dfx      1.0.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
 * 
 * 10    12/09/97 12:20p Peter
 * mac glide port
 * 
 * 9     11/12/97 5:12p Pgj
 * stubs for evil guFb{Read|Write}Region() calls
 * 
 * 8     5/27/97 1:16p Peter
 * Basic cvg, w/o cmd fifo stuff. 
 * 
 * 7     5/21/97 6:05a Peter
 * 
 * 6     3/09/97 10:31a Dow
 * Added GR_DIENTRY for di glide functions
 * 
 * 5     12/23/96 1:37p Dow
 * chagnes for multiplatform glide
**
*/

/* Implements multipass drawing */

#include <3dfx.h>
#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"
#include "gump.h"

/* CHD BUG this belongs in fxglide.h */
void FX_CSTYLE
_gumpTexCombineFunction( int virtual_tmu );

GrMPState _gumpState;

GR_DIENTRY(guMPInit, void, ( void ))
{
  int i;

  GDBG_INFO(98,"guMPInit()\n");
  for (i = 0; i < GLIDE_NUM_VIRTUAL_TMU; i += 1)
  {
    _gumpState.mmid[i] = (GrMipMapId_t) GR_NULL_MIPMAP_HANDLE;
  }
  _gumpState.tc_fnc = GR_MPTEXTURECOMBINE_ADD;
}

GR_DIENTRY(guMPTexCombineFunction, void, 
           ( GrMPTextureCombineFnc_t tc_fnc ))
{
  GDBG_INFO(98,"guMPTexCombineFunction(%d)\n",tc_fnc);
  _gumpState.tc_fnc = tc_fnc;
}

GR_DIENTRY(guMPTexSource, void, 
           ( GrChipID_t virtual_tmu, GrMipMapId_t mmid ))
{
  GR_DCL_GC;
  FXUNUSED( gc );
  GDBG_INFO(98,"guMPTexSource(%d,%d)\n",virtual_tmu,mmid);
  GR_CHECK_TMU("guMPTexSource",virtual_tmu);
  _gumpState.mmid[virtual_tmu] = mmid;
}

extern FX_ENTRY void FX_CALL
guFbReadRegion( const int srcX, const int srcY, const int w, const int h, const void *dst, const int strideInBytes );

extern FX_ENTRY void FX_CALL
guFbWriteRegion( const int dstX, const int dstY, const int w, const int h, const void *src, const int strideInBytes);


/*---------------------------------------------------------------------------
**  guFbReadRegion XXX obsolete
*/
GR_ENTRY(guFbReadRegion, void, ( const int srcX, const int srcY, const int w, const int h, const void *dst, const int strideInBytes ) )
{
}

/*---------------------------------------------------------------------------
**  guFbWriteRegion XXX obsolete
*/
GR_ENTRY(guFbWriteRegion, void, ( const int dstX, const int dstY, const int w, const int h, const void *src, const int strideInBytes))
{
}

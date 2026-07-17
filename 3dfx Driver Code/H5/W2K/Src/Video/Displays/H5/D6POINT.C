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
** File name:   d6point.c
**
** Description: DrawPrimitive point function implementation
**
** $Revision: 3$
** $Date: 10/11/00 8:43:25 PM$
**
** $Log: 
**  3    3dfx      1.1.1.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         10/01/99 Christopher Wilcox Removed P6FENCE macros,
**       which are no longer necessary even when !CMDFIFO, since register space is
**       not write combined.
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 4     8/19/99 4:28p Msmith
** adding opt for int to float conversion using lookup table
** 
** 3     8/03/99 2:50p Msmith
** fixes for perspective correct DCT failure from Bob J.
** 
** 2     6/21/99 5:29p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 1     6/02/99 6:45a Michael
** Branch from H3
**
** 14    4/13/99 8:48a Russ
** for NT direct write builds, redefine SET macro to DirectX style since
** it clashes with the NT gdi SET macro
**
** 13    1/25/99 4:52p Peterm
** added unified header information
**
** 12    12/09/98 6:50a Russ
** NT5 D3D changes for Banshee
**
** 11    11/22/98 9:12p Andrew
** Changes to support multi-monitor
**
** 10    10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
**
** 9     10/04/98 9:39p Adrians
** Add supporrt for Wbuffer+fog to lines and points.
**
** 8     9/02/98 12:34p Adrians
** DX6 multitexture change.
**
** 7     8/28/98 1:33p Adrians
** Change pixel coverage for lines and points.
**
** 6     8/15/98 12:15a Adrians
** Take Flat Shaded color from correct vertex for strips and fans.
** Use the same alpha component for all 3 vertices when Flat Shading.
** Add a Flat Shaded vertex component parameter to the line and point
** drawing.
**
** 5     8/04/98 10:20a Adrians
** Added support for texture0 coordIndex.
**
** 4     7/30/98 4:36p Adrians
** DX6 line drawing support.
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

#include "precomp.h"

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
#include "d3contxt.h"
#endif

#if( DX >= 6 )
/*-------------------------------------------------------------------
Function Name:  dp2Point

Description:    This function handles drawing DrawPrimitive points.

Return:         void
-------------------------------------------------------------------*/

void dp2Point( RC *pRc, LPDWORD pF, LPDWORD pV, DWORD vertexType )
{
  SETUP_PPDEV(pRc)
  D3DCOLOR    aColor;
  FxU32       setupFlag = pRc->sst.sSetupMode & ~SST_SETUP_EN_CULLING;
  float       s1, t1;
#if (NUMTEXTUREUNITS > 1)
  float       s1a, t1a;
#endif
  float       z1, w1;
#ifdef DCT_FIX
  float 	  wb1;
#endif
  CMDFIFO_PROLOG(cmdFifo);

  if ( pRc->shadeMode == D3DSHADE_FLAT )
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
      CLAMP888( aColor, pF[FVFO_COLOR], pF[FVFO_SPECULAR] );
    else
      aColor = pF[FVFO_COLOR];
  }
  else
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
      CLAMP888(aColor, pV[FVFO_COLOR], pV[FVFO_SPECULAR]);
    else
      aColor = pV[FVFO_COLOR];
  }

#ifdef DCT_FIX
  w1 = FLTP(pV)[FVFO_RHW] * pRc->scaleW;
#endif

  if( pRc->state & STATE_REQUIRES_WBUFFER )
  {
#ifdef DCT_FIX
	wb1 = w1;
#else
    w1 = WSCALE( FLTP(pV)[FVFO_RHW] );
#endif

    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
      z1 = (float)((255 - RGBA_GETALPHA(pV[FVFO_SPECULAR])) << 8);
  }
  else
  {
    z1 = ZSCALE( FLTP(pV)[FVFO_SZ] );

    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
#ifdef DCT_FIX
      wb1 = (float)(255 - RGBA_GETALPHA(pV[FVFO_SPECULAR]));
#else
#ifdef PERF_USE_255_LOOKUP_TABLE
//  faster here to use a lookup vs. letting fpu do conversion -mls
      w1 = f_255_reverse_lookup[RGBA_GETALPHA(pV[FVFO_SPECULAR])];
#else
#pragma message("note - NOT compiling with performance opt for int to float conversion")
      w1 = (float)(255 - RGBA_GETALPHA(pV[FVFO_SPECULAR]));
#endif // perf_use_255_lookup_table
#endif // dct_fix
    else if (pRc->state & STATE_REQUIRES_HWFOG)
#ifdef DCT_FIX
      wb1 = w1;
#else
      w1 = FLTP(pV)[FVFO_RHW];
#endif
  }

  // First texture stage
  if( pRc->state & STATE_REQUIRES_ST_TMU0 )
  {
    // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
    if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
    {
#ifdef DCT_FIX
      s1 = ((FLTP(pV)[FVFO_TU + pRc->t0CoordIndex] * pRc->sst.scaleS) + TEXEL_SOFFSET) * w1;
      t1 = ((FLTP(pV)[FVFO_TV + pRc->t0CoordIndex] * pRc->sst.scaleT) + TEXEL_TOFFSET) * w1;
#else
      s1 = ((FLTP(pV)[FVFO_TU + pRc->t0CoordIndex] * pRc->sst.scaleS) + TEXEL_SOFFSET) * FLTP(pV)[FVFO_RHW];
      t1 = ((FLTP(pV)[FVFO_TV + pRc->t0CoordIndex] * pRc->sst.scaleT) + TEXEL_TOFFSET) * FLTP(pV)[FVFO_RHW];
#endif
    }
    // if the texture is not prespective correct then the w is effectively equal to 1
    else
    {
#ifdef DCT_FIX
      s1 = (FLTP(pV)[FVFO_TU + pRc->t0CoordIndex] * pRc->sst.scaleS) + TEXEL_SOFFSET;
      t1 = (FLTP(pV)[FVFO_TV + pRc->t0CoordIndex] * pRc->sst.scaleT) + TEXEL_TOFFSET;
#else
      s1 = (FLTP(pV)[FVFO_TU + pRc->t0CoordIndex] * pRc->sst.scaleS) + TEXEL_SOFFSET;
      t1 = (FLTP(pV)[FVFO_TV + pRc->t0CoordIndex] * pRc->sst.scaleT) + TEXEL_TOFFSET;
#endif
    }
  } // texture

#if (NUMTEXTUREUNITS > 1)
  // Second texture stage
  if( pRc->state & STATE_REQUIRES_ST_TMU1 )
  {
    // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
    if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
    {
#ifdef DCT_FIX
      s1a = ((FLTP(pV)[FVFO_TU + pRc->t1CoordIndex] * pRc->sst.scaleS1) + TEXEL_S1OFFSET) * w1;
      t1a = ((FLTP(pV)[FVFO_TV + pRc->t1CoordIndex] * pRc->sst.scaleT1) + TEXEL_T1OFFSET) * w1;
#else
      s1a = ((FLTP(pV)[FVFO_TU + pRc->t1CoordIndex] * pRc->sst.scaleS1) + TEXEL_S1OFFSET) * FLTP(pV)[FVFO_RHW];
      t1a = ((FLTP(pV)[FVFO_TV + pRc->t1CoordIndex] * pRc->sst.scaleT1) + TEXEL_T1OFFSET) * FLTP(pV)[FVFO_RHW];
#endif
    }
    // if the texture is not prespective correct then the w is effectively equal to 1
    else
    {
      s1a = (FLTP(pV)[FVFO_TU + pRc->t1CoordIndex] * pRc->sst.scaleS1) + TEXEL_S1OFFSET;
      t1a = (FLTP(pV)[FVFO_TV + pRc->t1CoordIndex] * pRc->sst.scaleT1) + TEXEL_T1OFFSET;
    }
  } // texture
#endif

#if (NUMTEXTUREUNITS > 1)
  CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (10 * 4) );
#else
  CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (8 * 4) );
#endif

#ifdef CMDFIFO
  SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 4, setupFlag, 1 ) );
#else
  SET( cmdFifo, ghw0->sSetupMode, setupFlag);
#endif

  SETFPD( cmdFifo, ghw0->sVx, FLTP(pV)[FVFO_SX] - 0.5f + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, FLTP(pV)[FVFO_SY] - 0.5f + PIXEL_OFFSET );
  if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
    SETPD( cmdFifo, ghw0->sARGB, aColor );
  if(setupFlag & SST_SETUP_Z)
    SETFPD( cmdFifo, ghw0->sVz, z1 );
  if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
    SETFPD( cmdFifo, ghw0->sOowfbi, wb1 );
#else
    SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
#endif
  if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
    SETFPD( cmdFifo, ghw0->sOow0, w1 );
#else
    SETFPD( cmdFifo, ghw0->sOow0, FLTP(pV)[FVFO_RHW] );
#endif
  if(setupFlag & SST_SETUP_ST0)
  {
    SETFPD( cmdFifo, ghw0->sSow0, s1 );
    SETFPD( cmdFifo, ghw0->sTow0, t1 );
  }
#if (NUMTEXTUREUNITS > 1)
  if(setupFlag & SST_SETUP_ST1)
  {
    SETFPD( cmdFifo, ghw0->sSow1, s1a );
    SETFPD( cmdFifo, ghw0->sTow1, t1a );
  }
#endif
#ifndef CMDFIFO
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
#endif

  SETFPD( cmdFifo, ghw0->sVx, FLTP(pV)[FVFO_SX] + 0.5f + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, FLTP(pV)[FVFO_SY] - 0.5f + PIXEL_OFFSET );
  if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
    SETPD( cmdFifo, ghw0->sARGB, aColor );
  if(setupFlag & SST_SETUP_Z)
    SETFPD( cmdFifo, ghw0->sVz, z1 );
  if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
    SETFPD( cmdFifo, ghw0->sOowfbi, wb1 );
#else
    SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
#endif
  if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
    SETFPD( cmdFifo, ghw0->sOow0, w1 );
#else
    SETFPD( cmdFifo, ghw0->sOow0, FLTP(pV)[FVFO_RHW] );
#endif
  if(setupFlag & SST_SETUP_ST0)
  {
    SETFPD( cmdFifo, ghw0->sSow0, s1 );
    SETFPD( cmdFifo, ghw0->sTow0, t1 );
  }
#if (NUMTEXTUREUNITS > 1)
  if(setupFlag & SST_SETUP_ST1)
  {
    SETFPD( cmdFifo, ghw0->sSow1, s1a );
    SETFPD( cmdFifo, ghw0->sTow1, t1a );
  }
#endif
#ifndef CMDFIFO
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
#endif

  SETFPD( cmdFifo, ghw0->sVx, FLTP(pV)[FVFO_SX] - 0.5f + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, FLTP(pV)[FVFO_SY] + 0.5f + PIXEL_OFFSET );
  if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
    SETPD( cmdFifo, ghw0->sARGB, aColor );
  if(setupFlag & SST_SETUP_Z)
    SETFPD( cmdFifo, ghw0->sVz, z1 );
  if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
    SETFPD( cmdFifo, ghw0->sOowfbi, wb1 );
#else
    SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
#endif
  if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
    SETFPD( cmdFifo, ghw0->sOow0, w1 );
#else
    SETFPD( cmdFifo, ghw0->sOow0, FLTP(pV)[FVFO_RHW] );
#endif
  if(setupFlag & SST_SETUP_ST0)
  {
    SETFPD( cmdFifo, ghw0->sSow0, s1 );
    SETFPD( cmdFifo, ghw0->sTow0, t1 );
  }
#if (NUMTEXTUREUNITS > 1)
  if(setupFlag & SST_SETUP_ST1)
  {
    SETFPD( cmdFifo, ghw0->sSow1, s1a );
    SETFPD( cmdFifo, ghw0->sTow1, t1a );
  }
#endif
#ifndef CMDFIFO
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
#endif

  SETFPD( cmdFifo, ghw0->sVx, FLTP(pV)[FVFO_SX] + 0.5f + PIXEL_OFFSET );
  SETFPD( cmdFifo, ghw0->sVy, FLTP(pV)[FVFO_SY] + 0.5f + PIXEL_OFFSET );
  if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
    SETPD( cmdFifo, ghw0->sARGB, aColor );
  if(setupFlag & SST_SETUP_Z)
    SETFPD( cmdFifo, ghw0->sVz, z1 );
  if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
    SETFPD( cmdFifo, ghw0->sOowfbi, wb1 );
#else
    SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
#endif
  if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
    SETFPD( cmdFifo, ghw0->sOow0, w1 );
#else
    SETFPD( cmdFifo, ghw0->sOow0, FLTP(pV)[FVFO_RHW] );
#endif
  if(setupFlag & SST_SETUP_ST0)
  {
    SETFPD( cmdFifo, ghw0->sSow0, s1 );
    SETFPD( cmdFifo, ghw0->sTow0, t1 );
  }
#if (NUMTEXTUREUNITS > 1)
  if(setupFlag & SST_SETUP_ST1)
  {
    SETFPD( cmdFifo, ghw0->sSow1, s1a );
    SETFPD( cmdFifo, ghw0->sTow1, t1a );
  }
#endif
#ifndef CMDFIFO
  if(_MM(drawGlobal))
    SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
#endif

  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------
#endif

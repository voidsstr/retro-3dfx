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
**  3    3dfx      1.1.2.0     10/11/00 Brent           Forced check in to enforce
**       branching.
**  2    3dfx      1.1         10/01/99 Christopher Wilcox Removed P6FENCE macros,
**       which are no longer necessary even when !CMDFIFO, since register space is
**       not write combined.
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 2     8/19/99 4:14p Msmith
** adding opt for int to float conversion using lookup table
** 
** 1     6/02/99 6:41a Michael
** Branch from H3
** 
** 19    4/13/99 8:44a Russ
** for NT direct write builds, redefine SET macro to DirectX style since
** it clashes with the NT gdi SET macro
**
** 18    12/09/98 7:27a Russ
** NT5 D3D changes for Banshee
**
** 17    11/22/98 9:01p Andrew
** Changes to support multi-monitor
**
** 16    10/05/98 4:50p Adrians
** Fix for HWTable fog in lines and points.
**
** 15    7/30/98 4:34p Adrians
** Fix for DX5 line drawing.
**
** 14    7/24/98 1:37p Hohn
**
** 13    3/27/98 1:44p Adrians
** Code tidyup.
** Removed palette and texture clamp compile options.
 *
 * 12    11/09/97 2:50p Adrians
 * Single pk1's use an increment of 0.
 * Code added to find the triangle flavours used by apps.
 * Optimisation of triangle flavours.
 * Support for strips and fans in execute buffers.
 * Bug fix to wrapU and wrapV modes.
 *
 *
 * 11    10/31/97 4:54p Adrians
 * Now reads environment variables from the Registry.
 * Gamma is now read from the environment variables.
 * Fixed ddtof that was very buggy.
 *
 * 10    10/27/97 5:39p Adrians
 * Added packet 3 to DrawPrimitives, Fans, Strips & Tri's.
 * Added packet 3 to lines and points.
 * Added texture clamping to stw (build option tc=1).
 * Fix to DrawPrimitive fog.
 *
 *
 *
 * 9     10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 *
 * 8     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 *
 * 7     9/16/97 5:13p Adrians
 * Flat shading now uses RGB iterator rather than c0/c1.
 *
 * 5     9/12/97 3:46p Adrians
 * Multiple register pointers implemented for different chip fields.
 *
 * 4     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/

#include "precomp.h"

#if defined(WINNT) && !defined(CMDFIFO)
#undef SET
#define SET(hwPtr,hwRegister,data)    SETDW((hwRegister),(data))
#endif

#ifndef WINNT
#include "d3dhal.h"
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
//#include "fifomgr.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"
#endif

//------------------------------------------------------------------------------
//
//  Draw a single point
//
//------------------------------------------------------------------------------
void __stdcall fpDrawPoint( double dx, double dy, D3DTLVERTEX *pV, RC *pRc, D3DCOLOR flat )
{
  SETUP_PPDEV(pRc)
  D3DCOLOR    aColor;
  FxU32       setupFlag = pRc->sst.sSetupMode & ~SST_SETUP_EN_CULLING;
  float       s1, t1;
  float       w1;
  CMDFIFO_PROLOG(cmdFifo);

  if ( pRc->shadeMode == D3DSHADE_FLAT )
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
      CLAMP888( aColor, pV->color, pV->specular );
    else
      aColor = pV->color;
  }
  else
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
      CLAMP888(aColor, pV->color, pV->specular);
    else
      aColor = pV->color;
  }

  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
#ifdef PERF_USE_255_LOOKUP_TABLE
//  faster here to use a lookup vs. letting fpu do conversion -mls
    w1 = f_255_reverse_lookup[RGBA_GETALPHA(pV->specular)];
#else
#pragma message("note - NOT compiling with performance opt for int to float conversion")
    w1 = (float)(255 - RGBA_GETALPHA(pV->specular));
#endif
  else if (pRc->state & STATE_REQUIRES_HWFOG)
    w1 = pV->rhw;

  //----------------
  //
  // Texture Mapping
  //
  //----------------
  // NOTE: texture processing must occur after we process fog, chroma etc. because
  //       trilinear processing draws two triangles and assumes everything is setup
  //       by now.
  if ( pRc->texture != 0 )
  {
    // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
    if (pRc->texturePerspective)
    {
      s1 = D3DVAL(pV->tu) * D3DVAL(pV->rhw) * pRc->sst.scaleS;
      t1 = D3DVAL(pV->tv) * D3DVAL(pV->rhw) * pRc->sst.scaleT;
    }
    // if the texture is not prespective correct then the w is effectively equal to 1
    else
    {
      s1 = D3DVAL(pV->tu) * pRc->sst.scaleS;
      t1 = D3DVAL(pV->tv) * pRc->sst.scaleT;
    }
  } // texture

    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (8 * 4) );
  #ifdef CMDFIFO
    SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 4, setupFlag, 1 ) );
  #else
    SET( cmdFifo, ghw0->sSetupMode, setupFlag);
  #endif

  SETFPD( cmdFifo, ghw0->sVx, pV->sx );
  SETFPD( cmdFifo, ghw0->sVy, pV->sy );
  if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
    SETPD( cmdFifo, ghw0->sARGB, aColor );
  if(setupFlag & SST_SETUP_Z)
    SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pV->sz) );
  if(setupFlag & SST_SETUP_Wfbi)
    SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
  if(setupFlag & SST_SETUP_W0)
    SETFPD( cmdFifo, ghw0->sOow0, pV->rhw );
  if(setupFlag & SST_SETUP_ST0)
  {
    SETFPD( cmdFifo, ghw0->sSow0, s1 );
    SETFPD( cmdFifo, ghw0->sTow0, t1 );
  }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
  #endif

  SETFPD( cmdFifo, ghw0->sVx, pV->sx + 1.f);
  SETFPD( cmdFifo, ghw0->sVy, pV->sy );
  if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
    SETPD( cmdFifo, ghw0->sARGB, aColor );
  if(setupFlag & SST_SETUP_Z)
    SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pV->sz) );
  if(setupFlag & SST_SETUP_Wfbi)
    SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
  if(setupFlag & SST_SETUP_W0)
    SETFPD( cmdFifo, ghw0->sOow0, pV->rhw );
  if(setupFlag & SST_SETUP_ST0)
  {
    SETFPD( cmdFifo, ghw0->sSow0, s1 );
    SETFPD( cmdFifo, ghw0->sTow0, t1 );
  }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

  SETFPD( cmdFifo, ghw0->sVx, pV->sx );
  SETFPD( cmdFifo, ghw0->sVy, pV->sy + 1.f );
  if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
    SETPD( cmdFifo, ghw0->sARGB, aColor );
  if(setupFlag & SST_SETUP_Z)
    SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pV->sz) );
  if(setupFlag & SST_SETUP_Wfbi)
    SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
  if(setupFlag & SST_SETUP_W0)
    SETFPD( cmdFifo, ghw0->sOow0, pV->rhw );
  if(setupFlag & SST_SETUP_ST0)
  {
    SETFPD( cmdFifo, ghw0->sSow0, s1 );
    SETFPD( cmdFifo, ghw0->sTow0, t1 );
  }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

  SETFPD( cmdFifo, ghw0->sVx, pV->sx + 1.f );
  SETFPD( cmdFifo, ghw0->sVy, pV->sy + 1.f );
  if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
    SETPD( cmdFifo, ghw0->sARGB, aColor );
  if(setupFlag & SST_SETUP_Z)
    SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pV->sz) );
  if(setupFlag & SST_SETUP_Wfbi)
    SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
  if(setupFlag & SST_SETUP_W0)
    SETFPD( cmdFifo, ghw0->sOow0, pV->rhw );
  if(setupFlag & SST_SETUP_ST0)
  {
    SETFPD( cmdFifo, ghw0->sSow0, s1 );
    SETFPD( cmdFifo, ghw0->sTow0, t1 );
  }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

  CMDFIFO_EPILOG( cmdFifo );
}

//-----------------------------------------------------------
//
//  Point Primitive - parse the point list and call drawPoint
//
//-----------------------------------------------------------
void __stdcall fpPoint( RC *pRc, WORD count, LPD3DPOINT pts, LPD3DTLVERTEX vertices )
{
  // every point list
  for (; count > 0; --count)
  {
    int          ptCount;
    D3DTLVERTEX *vert;

    vert    = &vertices[pts->wFirst];
    ptCount = pts->wCount;

    // every pt in list
    for (ptCount = pts->wCount; ptCount > 0; --ptCount)
    {
      fpDrawPoint(vert->sx, vert->sy, vert, pRc, vert->color);
      ++vert;
    }

    // next pt list
    ++pts ;

  } // every pt list
}

//-------------------------------------------------------------------

// draw primitive point list
void __stdcall fpPointList(RC *pRc, DWORD count, LPD3DTLVERTEX vertices )
{
  for (; count > 0; --count)
  {
    D3DTLVERTEX *vert;

    vert    = vertices++;
    fpDrawPoint(vert->sx, vert->sy, vert, pRc, vert->color);
  }
}

//-------------------------------------------------------------------

// draw primitive indexed point list
void __stdcall fpIPointList(RC *pRc, DWORD count, WORD *indices, LPD3DTLVERTEX vertices)
{
  for (; count > 0; --count)
  {
    D3DTLVERTEX *vert;

    vert    = &vertices[*indices]; ++indices;
    fpDrawPoint(vert->sx, vert->sy, vert, pRc, vert->color);
  }
}

//-------------------------------------------------------------------

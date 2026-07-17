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
** 4     8/19/99 4:09p Msmith
** End of file got chopped off some how. Its back now...
** 
** 3     8/19/99 3:34p Msmith
** 
** 1     6/02/99 6:41a Michael
** Branch from H3
** 
** 26    5/24/99 5:05p Bseitsin
** Removal of Antialiasing code.
** 
** 25    4/13/99 8:43a Russ
** for NT direct write builds, redefine SET macro to DirectX style since
** it clashes with the NT gdi SET macro
**
** 24    12/09/98 7:26a Russ
** NT5 D3D changes for Banshee
**
** 23    11/22/98 9:00p Andrew
** Changes to support multi-monitor
**
** 22    10/05/98 4:50p Adrians
** Fix for HWTable fog in lines and points.
**
** 21    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
**
** No more conditional compilation of AA.
**
** 20    8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
**
** 19    7/30/98 4:34p Adrians
** Fix for DX5 line drawing.
**
** 18    7/24/98 1:37p Hohn
**
** 17    7/02/98 12:52p Adrians
** Fix for AA hang.
**
** 16    5/29/98 2:02p Adrians
** Enhancements to aa.
**
** 15    5/26/98 7:23p Adrians
** Added antialiasing.
**
** 14    5/06/98 6:09p Adrians
** Changes for DX6 into DX5 driver.
**
** 1     4/29/98 6:31p Adrians
** Created
**
** 13    3/27/98 1:44p Adrians
** Code tidyup.
** Removed palette and texture clamp compile options.
**
** 12    2/21/98 10:20p Adrians
** Fix to lines for MDK and fxLine test case.
 *
 * 11    11/09/97 2:50p Adrians
 * Single pk1's use an increment of 0.
 * Code added to find the triangle flavours used by apps.
 * Optimisation of triangle flavours.
 * Support for strips and fans in execute buffers.
 * Bug fix to wrapU and wrapV modes.
 *
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
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"
#endif

#define LINEWRAP(uv, wrap)          \
  {                                 \
  if (wrap)                         \
  {                                 \
    float  el;                      \
    el = d3absval(uv##2 - uv##1);   \
    if (el > DTOVALP(0.5, 24))      \
    {                               \
      if (uv##2 < uv##1)            \
        uv##2 += ITOVALP(1, 24);    \
      else                          \
        uv##1 += ITOVALP(1, 24);    \
    }                               \
  }                                 \
  }

#define WRAPST(s, t, wrap_s, wrap_t)  \
  do                                  \
  {                                   \
      LINEWRAP(s, wrap_s);            \
      LINEWRAP(t, wrap_t);            \
  } while (0)

//-------------------------------------------------------------------

void __stdcall fpDrawLine( D3DTLVERTEX *pA, D3DTLVERTEX *pB, RC *pRc, D3DCOLOR flat )
{
  SETUP_PPDEV(pRc)
  float         ax, ay, bx, by;
  float         dy, dx;
  D3DCOLOR    aColor, bColor;
  FxU32       setupFlag = pRc->sst.sSetupMode & ~SST_SETUP_EN_CULLING;
  float       s1, t1, s2, t2;
  float       w1, w2;
  D3DTLVERTEX *pFirst = pA, *pT;
  BOOL        xMajor;
  CMDFIFO_PROLOG(cmdFifo);

  ax = pA->sx;
  ay = pA->sy;

  bx = pB->sx;
  by = pB->sy;

  if((dx = bx - ax) < 0.f) dx = -dx;
  if((dy = by - ay) < 0.f) dy = -dy;

  if(dx >= dy)
  {
    // X Major
    xMajor = TRUE;

    if(ax > bx)
    {
      pT = pA;
      pA = pB;
      pB = pT;

      ax = pA->sx;
      ay = pA->sy;

      bx = pB->sx;
      by = pB->sy;
    }
  }
  else
  {
    // Y Major
    xMajor = FALSE;

    if(ay > by)
    {
      pT = pA;
      pA = pB;
      pB = pT;

      ax = pA->sx;
      ay = pA->sy;

      bx = pB->sx;
      by = pB->sy;
    }
  }

  if ( pRc->shadeMode == D3DSHADE_FLAT )
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
      CLAMP888( aColor, pFirst->color, pFirst->specular );
    else
      aColor = (pFirst->color & 0x00FFFFFF) | (pA->color & 0xFF000000);

    bColor = (pA->color & 0x00FFFFFF) | (pB->color & 0xFF000000);
  }
  else
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
    {
      CLAMP888(aColor, pA->color, pA->specular);
      CLAMP888(bColor, pB->color, pB->specular);
    }
    else
    {
      aColor = pA->color;
      bColor = pB->color;
    }
  }

  // Vertex Fog

  if (pRc->state & STATE_REQUIRES_VERTEXFOG)
  {
#ifdef PERF_USE_255_LOOKUP_TABLE
//  faster here to use a lookup vs. letting fpu do conversion -mls
    w1 = f_255_reverse_lookup[RGBA_GETALPHA(pA->specular)];
    w2 = f_255_reverse_lookup[RGBA_GETALPHA(pB->specular)];
#else
#pragma message("note - NOT compiling with performance opt for int to float conversion")
    w1 = (float)(255 - RGBA_GETALPHA(pA->specular));
    w2 = (float)(255 - RGBA_GETALPHA(pB->specular));
#endif
  }
  else if (pRc->state & STATE_REQUIRES_HWFOG)
  {
    w1 = pA->rhw;
    w2 = pB->rhw;
  }

  // Texture Mapping

  if ( pRc->texture != 0 )
  {
    float lscaleS, lscaleT;

    s1 = pA->tu;
    t1 = pA->tv;

    s2 = pB->tu;
    t2 = pB->tv;

    lscaleS = pRc->sst.scaleS;
    lscaleT = pRc->sst.scaleT;

    // two ways to texture. D3D wraps its textures going around the other direction
    // so we need to adjust S and T in order to get the correct result
    WRAPST(s, t, pRc->wrapU, pRc->wrapV);

    // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
    if (pRc->texturePerspective)
    {
      s1 = ( (s1 * lscaleS) + TEXEL_SOFFSET ) * pA->rhw;
      s2 = ( (s2 * lscaleS) + TEXEL_SOFFSET ) * pB->rhw;

      t1 = ( (t1 * lscaleT) + TEXEL_TOFFSET ) * pA->rhw;
      t2 = ( (t2 * lscaleT) + TEXEL_TOFFSET ) * pB->rhw;
    }
    // if the texture is not prespective correct then the w is effectively equal to 1
    else
    {
      s1 = (s1 * lscaleS) + TEXEL_SOFFSET;
      s2 = (s2 * lscaleS) + TEXEL_SOFFSET;

      t1 = (t1 * lscaleT) + TEXEL_TOFFSET;
      t2 = (t2 * lscaleT) + TEXEL_TOFFSET;
    }
  } // texture

  if(xMajor)
  {
    // X Major Axis

    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (8 * 4) );
  #ifdef CMDFIFO
    SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 4, setupFlag, 1 ) );
  #else
    SET( cmdFifo, ghw0->sSetupMode, setupFlag);
  #endif

    SETFPD( cmdFifo, ghw0->sVx, ax + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, aColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
    if(setupFlag & SST_SETUP_Wfbi)
      SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
    if(setupFlag & SST_SETUP_W0)
      SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s1 );
      SETFPD( cmdFifo, ghw0->sTow0, t1 );
    }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
  #endif

    SETFPD( cmdFifo, ghw0->sVx, ax + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay - 1.0f + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, aColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
    if(setupFlag & SST_SETUP_Wfbi)
      SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
    if(setupFlag & SST_SETUP_W0)
      SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s1 );
      SETFPD( cmdFifo, ghw0->sTow0, t1 );
    }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

    SETFPD( cmdFifo, ghw0->sVx, bx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
    if(setupFlag & SST_SETUP_Wfbi)
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
    if(setupFlag & SST_SETUP_W0)
      SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

    SETFPD( cmdFifo, ghw0->sVx, bx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by - 1.0f + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
    if(setupFlag & SST_SETUP_Wfbi)
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
    if(setupFlag & SST_SETUP_W0)
      SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif
  }
  else
  {
    // Y Major

    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (8 * 4) );
  #ifdef CMDFIFO
    SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 4, setupFlag, 1 ) );
  #else
    SET( cmdFifo, ghw0->sSetupMode, setupFlag);
  #endif

    SETFPD( cmdFifo, ghw0->sVx, ax - 1.0f + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, aColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
    if(setupFlag & SST_SETUP_Wfbi)
      SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
    if(setupFlag & SST_SETUP_W0)
      SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s1 );
      SETFPD( cmdFifo, ghw0->sTow0, t1 );
    }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sBeginTriCMD, 0 );
  #endif

    SETFPD( cmdFifo, ghw0->sVx, ax + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, aColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pA->sz) );
    if(setupFlag & SST_SETUP_Wfbi)
      SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
    if(setupFlag & SST_SETUP_W0)
      SETFPD( cmdFifo, ghw0->sOow0, pA->rhw );
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s1 );
      SETFPD( cmdFifo, ghw0->sTow0, t1 );
    }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

    SETFPD( cmdFifo, ghw0->sVx, bx - 1.0f + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
    if(setupFlag & SST_SETUP_Wfbi)
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
    if(setupFlag & SST_SETUP_W0)
      SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

    SETFPD( cmdFifo, ghw0->sVx, bx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, ZSCALE(pB->sz) );
    if(setupFlag & SST_SETUP_Wfbi)
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
    if(setupFlag & SST_SETUP_W0)
      SETFPD( cmdFifo, ghw0->sOow0, pB->rhw );
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #ifndef CMDFIFO
    if(_MM(drawGlobal)) SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif
  }

  CMDFIFO_EPILOG( cmdFifo );

  return ;
}

//-------------------------------------------------------------------

// execute buffer style lines

void __stdcall fpLine(RC *pRc, WORD count, LPD3DLINE line, LPD3DTLVERTEX vertices )
{
  SETUP_PPDEV(pRc)
  D3DTLVERTEX *pA, *pB;

  //---------------
  //
  // every line
  //
  //---------------
  for (; count > 0; --count)
  {
    pA = &vertices[line->v1];
    pB = &vertices[line->v2];

    fpDrawLine(pA, pB, pRc, pA->color);
    ++line;
  }

  return;
}

//-------------------------------------------------------------------

// draw primitive lines
void __stdcall fpLineList( RC *pRc, DWORD count, LPD3DTLVERTEX vertices )
{
  D3DTLVERTEX   *pA, *pB;

  for (; count > 0; count-=2)
  {
    pA    = vertices++;
    pB    = vertices++;

    fpDrawLine(pA, pB, pRc, pA->color);
  }

  return ;
}

//-------------------------------------------------------------------

// draw primitive indexed lines
void __stdcall fpILineList(RC *pRc, DWORD count, WORD *indices, LPD3DTLVERTEX vertices )
{
  D3DTLVERTEX   *pA, *pB;
  DWORD         index = 0;

  for (; count > 0; count-=2)
  {
    pA = vertices + indices[index++];
    //D3DPRINT( 255,"ILineList %d %d",indices[index-1], indices[index] );
    pB = vertices + indices[index++];

    fpDrawLine(pA, pB, pRc, pA->color);
  }

  return ;
}

//-------------------------------------------------------------------

// draw primitive line strip
void __stdcall fpLineStrip( RC *pRc, DWORD count, LPD3DTLVERTEX vertices )
{
  D3DTLVERTEX   *pA, *pB;

  pB = vertices++;

  for (; count > 0; --count)
  {
    pA = pB;
    pB = vertices++;

    fpDrawLine(pA, pB, pRc, pA->color);
  }

  return ;
}

//-------------------------------------------------------------------

// draw primitive indexed line strip
void __stdcall fpILineStrip( RC *pRc, DWORD count, WORD *indices, LPD3DTLVERTEX vertices )
{
  D3DTLVERTEX   *pA, *pB;
  DWORD         index = 0;

  pB = vertices + indices[index++];

  for (; count > 0; --count)
  {
    pA = pB;
    pB = vertices + indices[index++];

    //D3DPRINT( 255,"ILineStrip %d %d",indices[index-2], indices[index-1] );
    fpDrawLine(pA, pB, pRc, pA->color);
  }

  return ;
}

//-------------------------------------------------------------------

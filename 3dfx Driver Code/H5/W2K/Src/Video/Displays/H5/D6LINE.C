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
** File name:   d6line.c
**
** Description: line drawing functions for DrawPrimitive calls
**
** $Revision: 8$
** $Date: 10/11/00 8:43:17 PM$
**
** $Log: 
**  8    3dfx      1.5.3.0.1.0 10/11/00 Brent           Forced check in to enforce
**       branching.
**  7    3dfx      1.5.3.0     05/19/00 Russ Lind       merge from w9x, fix
**       NUMTEXTUREUNITS typo
**  6    3dfx      1.5         02/04/00 Steve Houston   Inserted #undef K6_2 under
**       WinNT builds to remove 3DNow optimizations that are not supported under
**       NT. Related to new triangle asm port from W9X.
**  5    3dfx      1.4         01/28/00 Scott Kephart   Big T&L Merge: FVF handling
**       changes
**  4    3dfx      1.3         12/06/99 Abraham Campbell Pulled forward from V3 my
**       changes for AA lines that makes the PC99 #28 AntiAlias DCT test pass. 
**       However, it is disabled by default.  To enable it, uncomment the following
**       line:  //#define SUBPIXELADJ	1
**  3    3dfx      1.2         10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
**  2    3dfx      1.1         10/01/99 Christopher Wilcox Removed P6FENCE macros,
**       which are no longer necessary even when !CMDFIFO, since register space is
**       not write combined.
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 4     8/19/99 4:27p Msmith
** adding opt for int to float conversion using lookup table
** 
** 3     8/03/99 2:46p Msmith
** fixes for perspective correct DCT failure from Bob J.
** 
** 2     6/21/99 5:27p Russ
** changed DX == 6 to DX >= 6 && DX7 to DX >= 7
**
** 1     6/02/99 6:45a Michael
** Branch from H3
**
** 19    5/24/99 5:06p Bseitsin
** Removal of Antialiasing code.
**
** 18    4/13/99 8:47a Russ
** for NT direct write builds, redefine SET macro to DirectX style since
** it clashes with the NT gdi SET macro
**
** 17    1/25/99 4:52p Peterm
** added unified header information
**
** 16    12/16/98 4:03p Sreid
** K6 merge - 12-16-98 drop from contractor
**
** 15    12/09/98 6:47a Russ
** NT5 D3D changes for Banshee
**
** 14    11/30/98 9:06p Adrians
** Earlier checkin for MDK fix was wrong. This is the correct fix.
** (PRS#726)
**
** 13    11/30/98 3:41p Adrians
** Removed the pixel coverage offsets added to lines because they caused a
** problem with MDK (PRS#726).
**
** 12    11/22/98 9:10p Andrew
** Changes to support multi-monitor
**
** 11    10/15/98 6:35p Artg
** changed ifdef h3 to account for h4
** ifdef h3  --> if defined(h3) || defined(h4)
**
** 10    10/04/98 9:39p Adrians
** Add supporrt for Wbuffer+fog to lines and points.
**
** 9     9/05/98 5:02p Adrians
** Added AA support to DX6.
** Validate will now fail arguments with COMPLEMENT or ALPHAREPLICATE.
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

#if( DX >= 6 )

#ifndef WINNT
#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d6fvf.h"
#include "d3contxt.h"
#else // ifdef WINNT
// shouston 1-29-00 : The K6-2 optimizations in this file are not yet
// implemented under WinNT.
#undef K6_2
#endif

#ifdef K6_2
#include "k6_2.h"
#include "3dnow_vc.h"
#pragma warning (disable : 4799 )
#endif

#ifdef TnL_HAL
#include "cliprend.h"
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

/*--- ABCamp 12-6-99   Port from V3 TOT  -----*/
//-----------------------------
// ABCamp 8-24-99  Added ability to generate subpixel
//                 vertices from original line end points in order
//                 to draw the 3D lines with 2 triangles,
//                 that more closely matches reference lines.
//                 
//#define SUBPIXELADJ	1

int ndxXAdj = 0x0E;
int ndxYAdj = 0x0E;
float fSubPixAdj[]=
{
  0.1f,  // 0
  0.15f, // 1
  0.2f,  // 2
  0.25f, // 3
  0.3f,  // 4
  0.35f, // 5
  0.4f,  // 6
  0.45f, // 7
  0.5f,  // 8
  0.515f,// 9
  0.525f,// A
  0.535f,// B
  0.545f,// C
  0.55f, // D
  0.565f,// E
  0.575f,// F
  0.585f,// 10
  0.595f,// 11
  0.6f,  // 12
  0.615f,// 13
  0.625f,// 14
  0.635f,// 15
  0.645f,// 16
  0.65f, // 17
  0.7f,  // 18
  0.8f,  // 19
  0.9f,  // 20
  1.0f,  // 21
  1.1f,  // 22
  1.2f,  // 23
  1.3f,  // 24
  1.4f,  // 25
  1.5f,  // 26
  1.6f,  // 27
  1.7f,  // 28
  1.8f,  // 29
  2.0f   // 2A
};


/*-------------------------------------------------------------------
Function Name:  calcSubPixelAdj

Description:    Calculates subpixel adjustments, to be used with the
                the original pixels to create the 4 verticies needed
                to draw the 3D "line" with 2 triangles.

Return:         void
-------------------------------------------------------------------*/
void calcSubPixelAdj ( 
	float  ax,    float  ay,    float bx,     float by, 
	float* xAdj1, float* yAdj1, float* xAdj2, float* yAdj2 )
{
  DWORD quad;
  float xAdj, yAdj;
  float dy, dx;
  float xa, ya;
  float tan;

#if 0
  // Adjust on the fly--for debugging...
  xa = fSubPixAdj[ ndxXAdj ];  // Get X Adjust value
  ya = fSubPixAdj[ ndxYAdj ];  // Get Y Adjust value
#endif

  // Base values that give the closest match.
  //
  xa = 0.565f;  // Get X Adjust value
  ya = 0.565f;  // Get X Adjust value

  dx = bx - ax;
  dy = by - ay;

  // Quad 3   |  Quad 2
  // ---------+---------
  // Quad 4   |  Quad 1
  //
  if ( dx >= 0.0f )  // dx is positive
  {
    if ( dy >= 0.0f ) quad = 1;
    else              quad = 2;
  }
  else // dx is negative
  {
    if ( dy >= 0.0f ) quad = 4;
    else              quad = 3;
  }

  // Make absolute values..
  if (dx < 0.f) dx = -dx;
  if (dy < 0.f) dy = -dy;

  if ( dx == 0 )	// vertical line
  {
    xAdj = xa;
    yAdj = 0;
    goto SignFixup;
  }
  else if ( dy == 0 )	// horizontal line
  {
    xAdj = 0;
    yAdj = ya;
    goto SignFixup;
  }

  if (dx > dy)
  { 
    // X Major - This is the region of a quadarant from the 45 degree
    //           down to the X axis, i.e. as we move away from the 45
    //           degree angle, Y diminishes to 0.

    // Percentage of the height (Y) w.r.t the width (X)
    //
    tan = dy / dx; // Goes from 1.0 ---> 0.0 as Y decreases

    // At the 45 degree angle of the quadrant, i.e. tan == 1.0, and we return
    // XAdj/2, YAdj/2.  As we move from the 45 degree angle
    // (tan == 1.0) towards the X axis, XAdj decreases to zero and 
    // YAdj increases to the full value, in order make a T cross bar perpendicular
    // to the X axis.
    xAdj = (float)(( xa * tan ) / 2.0f);
    yAdj = (float) ( ya - ((ya  / 2.0f) * tan ));
  }
  else if ( dy > dx )
  {
    // Y Major - This is the region of a quadarant from the 45 degree
    //           over to the Y axis, i.e. as we move away from the 45
    //           degree angle, X diminishes to 0.

    // Percentage of the width (X) w.r.t the height (Y)
    //
    tan = dx / dy;

    // At the 45 degree angle of the quadrant, i.e. tan == 1.0, and we return
    // XAdj/2, YAdj/2.  As we move from the 45 degree angle
    // (tan == 1.0) towards the Y axis, XAdj increases to full value and
    // YAdj decreases to zero, , in order make a T cross bar perpendicular
    // to the Y axis.
    xAdj = (float) ( xa - ((xa  / 2.0f) * tan ));
    yAdj = (float)(( ya * tan ) / 2.0f);
  }
  else // dx == dy  --> 45 degree angle
  { 
    // At the 45 degree angle of the quadrant, tan == 1.0, we return
    // XAdj/2, YAdj/2 , since they will form the sides
    // of a square angle.
    //
    xAdj = xa / 2.0f;
    yAdj = ya / 2.0f;
  } 

SignFixup:
  if ( quad == 1 || quad == 3 )
  {
    *xAdj1 = -xAdj;
    *yAdj1 = +yAdj;
    *xAdj2 = +xAdj;
    *yAdj2 = -yAdj;
  }
  else if ( quad == 2 || quad == 4 )
  {
    *xAdj1 = -xAdj;
    *yAdj1 = -yAdj;
    *xAdj2 = +xAdj;
    *yAdj2 = +yAdj;
  }
  return;
}

/*--- Port end ----*/



/*-------------------------------------------------------------------
Function Name:  dp2Line

Description:    Implementation of lines for DrawPrimitive


Return:         void
-------------------------------------------------------------------*/
void dp2Line( RC *pRc, LPDWORD pF, LPDWORD pA, LPDWORD pB, DWORD vertexType )
{
  SETUP_PPDEV(pRc)
  float       ax, ay, bx, by;
  float       dy, dx;
  D3DCOLOR    aColor, bColor;
  FxU32       setupFlag = pRc->sst.sSetupMode & ~SST_SETUP_EN_CULLING;
  float       s1, t1, s2, t2;
#if (NUMTEXTUREUNITS > 1)
  float       s1a, t1a, s1b, t1b;
#endif
  float       z1, z2, w1, w2;
#ifdef DCT_FIX
  float 	  wb1, wb2;
#endif
  LPDWORD     pT;
  BOOL        xMajor;
#ifdef SUBPIXELADJ
  float       fXAdj1 = 0.0f, fXAdj2 = 0.0f;
  float       fYAdj1 = 0.0f, fYAdj2 = 0.0f;
#endif
  CMDFIFO_PROLOG(cmdFifo);

  ax = FLTP(pA)[FVFO_SX];
  ay = FLTP(pA)[FVFO_SY];

  bx = FLTP(pB)[FVFO_SX];
  by = FLTP(pB)[FVFO_SY];

#ifdef SUBPIXELADJ
  calcSubPixelAdj (
     ax,ay, bx,by, &fXAdj1,&fYAdj1, &fXAdj2,&fYAdj2 );
#endif

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

      ax = FLTP(pA)[FVFO_SX];
      ay = FLTP(pA)[FVFO_SY];

      bx = FLTP(pB)[FVFO_SX];
      by = FLTP(pB)[FVFO_SY];
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

      ax = FLTP(pA)[FVFO_SX];
      ay = FLTP(pA)[FVFO_SY];

      bx = FLTP(pB)[FVFO_SX];
      by = FLTP(pB)[FVFO_SY];
    }
  }

  if ( pRc->shadeMode == D3DSHADE_FLAT )
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
      CLAMP888( aColor, pF[FVFO_COLOR], pF[FVFO_SPECULAR] );
    else
      aColor = pF[FVFO_COLOR];

    bColor = aColor;
  }
  else
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
    {
      CLAMP888(aColor, pA[FVFO_COLOR], pA[FVFO_SPECULAR]);
      CLAMP888(bColor, pB[FVFO_COLOR], pB[FVFO_SPECULAR]);
    }
    else
    {
      aColor = pA[FVFO_COLOR];
      bColor = pB[FVFO_COLOR];
    }
  }

#ifdef DCT_FIX
  w1 = FLTP(pA)[FVFO_RHW] * pRc->scaleW;
  w2 = FLTP(pB)[FVFO_RHW] * pRc->scaleW;
#endif

  // Process Fog and zBuffering.
  if( pRc->state & STATE_REQUIRES_WBUFFER )
  {
#ifdef DCT_FIX
    wb1 = w1;
    wb2 = w2;
#else
    w1 = WSCALE( FLTP(pA)[FVFO_RHW] );
    w2 = WSCALE( FLTP(pB)[FVFO_RHW] );
#endif

    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
    {
      z1 = (float)((255 - RGBA_GETALPHA(pA[FVFO_SPECULAR])) << 8);
      z2 = (float)((255 - RGBA_GETALPHA(pB[FVFO_SPECULAR])) << 8);
    }
  }
  else
  {
    z1 = ZSCALE( FLTP(pA)[FVFO_SZ] );
    z2 = ZSCALE( FLTP(pB)[FVFO_SZ] );

    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
    {
#ifdef DCT_FIX
      wb1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR]));
      wb2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR]));
#else
#ifdef PERF_USE_255_LOOKUP_TABLE
//  faster here to use a lookup vs. letting fpu do conversion -mls
      w1 = f_255_reverse_lookup[RGBA_GETALPHA(pA[FVFO_SPECULAR])];
      w2 = f_255_reverse_lookup[RGBA_GETALPHA(pB[FVFO_SPECULAR])];
#else
#pragma message("note - NOT compiling with performance opt for int to float conversion")
      w1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR]));
      w2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR]));
#endif // perf_use_255_lookup_table
#endif
    }
    else if (pRc->state & STATE_REQUIRES_HWFOG)
    {
#ifdef DCT_FIX
	  wb1 = w1;
	  wb2 = w2;
#else
      w1 = FLTP(pA)[FVFO_RHW];
      w2 = FLTP(pB)[FVFO_RHW];
#endif
    }
  }

  // First texture stage
  if( pRc->state & STATE_REQUIRES_ST_TMU0 )
  {
    float lscaleS, lscaleT;

    s1 = FLTP(pA)[FVFO_TU + pRc->t0CoordIndex];
    t1 = FLTP(pA)[FVFO_TV + pRc->t0CoordIndex];

    s2 = FLTP(pB)[FVFO_TU + pRc->t0CoordIndex];
    t2 = FLTP(pB)[FVFO_TV + pRc->t0CoordIndex];

    lscaleS = pRc->sst.scaleS;
    lscaleT = pRc->sst.scaleT;

    // two ways to texture. D3D wraps its textures going around the other direction
    // so we need to adjust S and T in order to get the correct result
    WRAPST(s, t, pRc->wrapU, pRc->wrapV);

    // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
      if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
    {
#ifdef DCT_FIX
      s1 = ((s1 * lscaleS) + TEXEL_SOFFSET) * w1;
      s2 = ((s2 * lscaleS) + TEXEL_SOFFSET) * w2;

      t1 = ((t1 * lscaleT) + TEXEL_TOFFSET) * w1;
      t2 = ((t2 * lscaleT) + TEXEL_TOFFSET) * w2;
#else
      s1 = ((s1 * lscaleS) + TEXEL_SOFFSET) * FLTP(pA)[FVFO_RHW];
      s2 = ((s2 * lscaleS) + TEXEL_SOFFSET) * FLTP(pB)[FVFO_RHW];

      t1 = ((t1 * lscaleT) + TEXEL_TOFFSET) * FLTP(pA)[FVFO_RHW];
      t2 = ((t2 * lscaleT) + TEXEL_TOFFSET) * FLTP(pB)[FVFO_RHW];
#endif
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

#if (NUMTEXTUREUNITS > 1)
  // Second texture stage
  if( pRc->state & STATE_REQUIRES_ST_TMU1 )
  {
    float lscaleS, lscaleT;

    s1a = FLTP(pA)[FVFO_TU + pRc->t1CoordIndex];
    t1a = FLTP(pA)[FVFO_TV + pRc->t1CoordIndex];

    s1b = FLTP(pB)[FVFO_TU + pRc->t1CoordIndex];
    t1b = FLTP(pB)[FVFO_TV + pRc->t1CoordIndex];

    lscaleS = pRc->sst.scaleS1;
    lscaleT = pRc->sst.scaleT1;

    // two ways to texture. D3D wraps its textures going around the other direction
    // so we need to adjust S and T in order to get the correct result
//    WRAPST(s, t, pRc->wrapU, pRc->wrapV);

    // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
      if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
    {
#ifdef DCT_FIX
      s1a = ((s1a * lscaleS) + TEXEL_S1OFFSET) * w1;
      s1b = ((s1b * lscaleS) + TEXEL_S1OFFSET) * w2;

      t1a = ((t1a * lscaleT) + TEXEL_T1OFFSET) * w1;
      t1b = ((t1b * lscaleT) + TEXEL_T1OFFSET) * w2;
#else
      s1a = ((s1a * lscaleS) + TEXEL_S1OFFSET) * FLTP(pA)[FVFO_RHW];
      s1b = ((s1b * lscaleS) + TEXEL_S1OFFSET) * FLTP(pB)[FVFO_RHW];

      t1a = ((t1a * lscaleT) + TEXEL_T1OFFSET) * FLTP(pA)[FVFO_RHW];
      t1b = ((t1b * lscaleT) + TEXEL_T1OFFSET) * FLTP(pB)[FVFO_RHW];
#endif
    }
    // if the texture is not prespective correct then the w is effectively equal to 1
    else
    {
      s1a = (s1a * lscaleS) + TEXEL_S1OFFSET;
      s1b = (s1b * lscaleS) + TEXEL_S1OFFSET;

      t1a = (t1a * lscaleT) + TEXEL_T1OFFSET;
      t1b = (t1b * lscaleT) + TEXEL_T1OFFSET;
    }
  } // texture
#endif

  if(xMajor)
  {
    // X Major Axis

  #if( NUMTEXTUREUNITS > 1)
    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (10 * 4) );
  #else
    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (8 * 4) );
  #endif

  #ifdef CMDFIFO
    SETPH( cmdFifo, CMDFIFO_BUILD_PK3( CMD_START, 4, setupFlag, 1 ) );
  #else
    SET( cmdFifo, ghw0->sSetupMode, setupFlag);
  #endif

#ifdef SUBPIXELADJ
    SETFPD( cmdFifo, ghw0->sVx, ax +fXAdj1 + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay +fYAdj1 + PIXEL_OFFSET );
#else
    SETFPD( cmdFifo, ghw0->sVx, ax + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay + PIXEL_OFFSET );
#endif
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
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
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

#ifdef SUBPIXELADJ
    SETFPD( cmdFifo, ghw0->sVx, ax +fXAdj2 + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay +fYAdj2 + PIXEL_OFFSET );
#else
    SETFPD( cmdFifo, ghw0->sVx, ax + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay - 1.0f + PIXEL_OFFSET );
#endif
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, aColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z1 );
    if(setupFlag & SST_SETUP_Wfbi)
#if DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb1 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
#endif
    if(setupFlag & SST_SETUP_W0)
#if DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w1 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
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

#ifdef SUBPIXELADJ
    SETFPD( cmdFifo, ghw0->sVx, bx +fXAdj1 + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by +fYAdj1 + PIXEL_OFFSET );
#else
    SETFPD( cmdFifo, ghw0->sVx, bx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by + PIXEL_OFFSET );
#endif
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z2 );
    if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb2 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
#endif
    if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w2 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
#endif
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #if (NUMTEXTUREUNITS > 1)
    if(setupFlag & SST_SETUP_ST1)
    {
      SETFPD( cmdFifo, ghw0->sSow1, s1b );
      SETFPD( cmdFifo, ghw0->sTow1, t1b );
    }
  #endif
  #ifndef CMDFIFO
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

#ifdef SUBPIXELADJ
    SETFPD( cmdFifo, ghw0->sVx, bx +fXAdj2 + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by +fYAdj2 + PIXEL_OFFSET );
#else
    SETFPD( cmdFifo, ghw0->sVx, bx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by - 1.0f + PIXEL_OFFSET );
#endif
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z2 );
    if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb2 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
#endif
    if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w2 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
#endif
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #if (NUMTEXTUREUNITS > 1)
    if(setupFlag & SST_SETUP_ST1)
    {
      SETFPD( cmdFifo, ghw0->sSow1, s1b );
      SETFPD( cmdFifo, ghw0->sTow1, t1b );
    }
  #endif
  #ifndef CMDFIFO
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif
  }
  else
  {
    // Y Major
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

#ifdef SUBPIXELADJ
    SETFPD( cmdFifo, ghw0->sVx, ax +fXAdj1 + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay +fYAdj1 + PIXEL_OFFSET );
#else
    SETFPD( cmdFifo, ghw0->sVx, ax - 1.0f + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay + PIXEL_OFFSET );
#endif
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
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
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

#ifdef SUBPIXELADJ
    SETFPD( cmdFifo, ghw0->sVx, ax +fXAdj2 + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay +fYAdj2 + PIXEL_OFFSET );
#else
    SETFPD( cmdFifo, ghw0->sVx, ax + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay + PIXEL_OFFSET );
#endif
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
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
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

#ifdef SUBPIXELADJ
    SETFPD( cmdFifo, ghw0->sVx, bx +fXAdj1 + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by +fYAdj1 + PIXEL_OFFSET );
#else
    SETFPD( cmdFifo, ghw0->sVx, bx - 1.0f + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by + PIXEL_OFFSET );
#endif
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z2 );
    if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb2 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
#endif
    if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w2 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
#endif
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #if (NUMTEXTUREUNITS > 1)
    if(setupFlag & SST_SETUP_ST1)
    {
      SETFPD( cmdFifo, ghw0->sSow1, s1b );
      SETFPD( cmdFifo, ghw0->sTow1, t1b );
    }
  #endif
  #ifndef CMDFIFO
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

#ifdef SUBPIXELADJ
    SETFPD( cmdFifo, ghw0->sVx, bx +fXAdj2 + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by +fYAdj2 + PIXEL_OFFSET );
#else
    SETFPD( cmdFifo, ghw0->sVx, bx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by + PIXEL_OFFSET );
#endif
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z2 );
    if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb2 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
#endif
    if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w2 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
#endif
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #if (NUMTEXTUREUNITS > 1)
    if(setupFlag & SST_SETUP_ST1)
    {
      SETFPD( cmdFifo, ghw0->sSow1, s1b );
      SETFPD( cmdFifo, ghw0->sTow1, t1b );
    }
  #endif
  #ifndef CMDFIFO
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif
  }

  CMDFIFO_EPILOG( cmdFifo );
}

#ifdef TnL_HAL
/*-------------------------------------------------------------------
Function Name:  dp2LineClipped

Description:    Implementation of Clipped lines for DrawPrimitive


Return:         void
-------------------------------------------------------------------*/
void dp2LineClipped( RC *pRc, LPDWORD pF, LPDWORD pA, LPDWORD pB, DWORD c0, DWORD c1, DWORD vertexType )
{
  SETUP_PPDEV(pRc)
  float       ax, ay, bx, by;
  float       dy, dx;
  D3DCOLOR    aColor, bColor;
  FxU32       setupFlag = pRc->sst.sSetupMode & ~SST_SETUP_EN_CULLING;
  float       s1, t1, s2, t2;
  DWORD dwUnion;
  DWORD dwMask;
  LPBYTE pTLV = (LPVOID) pRc->tl.clipping.ClipBuf.alignedBuf;

#if (NUMTEXTUREUNITS > 1)
  float       s1a, t1a, s1b, t1b;
#endif
  float       z1, z2, w1, w2;
#ifdef DCT_FIX
  float       wb1, wb2;
#endif
  LPDWORD     pT;
  BOOL        xMajor;
  CMDFIFO_PROLOG(cmdFifo);

  // Get the OR flags and clip mask to see if we can trivial 
  // accept it
  dwUnion = (c0 | c1);
  dwMask = TLCLIP_LEFT  | TLCLIP_RIGHT | TLCLIP_TOP | TLCLIP_BOTTOM  |         
           TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL; 
   
  if (pRc->tl.dwTLState & TLPV_GUARDBAND) 
  {
     dwMask = TLCLIPGB_LEFT | TLCLIPGB_RIGHT | TLCLIPGB_TOP | TLCLIPGB_BOTTOM |      
                        TLCLIP_FRONT | TLCLIP_BACK | TLCLIP_USERPLANES_ALL; 
  }

  // If all the vertices are in, 
  // No clipping is neeed! 
  // Let it pass down to the FIFO stuffing
  if ((dwUnion & dwMask) != 0)
  {
     if (ClipFVFLine( pRc, pA, pB, c0, c1, dwMask))
	 {
        pA = (LPDWORD) (pTLV); 
        pB = (LPDWORD) (pTLV + pRc->tl.TLFVF.dwStride); 
	 }
	 else
	   return;

  } // if clipping on this line


  ax = FLTP(pA)[FVFO_SX];
  ay = FLTP(pA)[FVFO_SY];

  bx = FLTP(pB)[FVFO_SX];
  by = FLTP(pB)[FVFO_SY];

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

      ax = FLTP(pA)[FVFO_SX];
      ay = FLTP(pA)[FVFO_SY];

      bx = FLTP(pB)[FVFO_SX];
      by = FLTP(pB)[FVFO_SY];
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

      ax = FLTP(pA)[FVFO_SX];
      ay = FLTP(pA)[FVFO_SY];

      bx = FLTP(pB)[FVFO_SX];
      by = FLTP(pB)[FVFO_SY];
    }
  }

  if ( pRc->shadeMode == D3DSHADE_FLAT )
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
      CLAMP888( aColor, pF[FVFO_COLOR], pF[FVFO_SPECULAR] );
    else
      aColor = pF[FVFO_COLOR];

    bColor = aColor;
  }
  else
  {
    if ( (pRc->specular) && (pRc->texture == 0) )
    {
      CLAMP888(aColor, pA[FVFO_COLOR], pA[FVFO_SPECULAR]);
      CLAMP888(bColor, pB[FVFO_COLOR], pB[FVFO_SPECULAR]);
    }
    else
    {
      aColor = pA[FVFO_COLOR];
      bColor = pB[FVFO_COLOR];
    }
  }

#ifdef DCT_FIX
  w1 = FLTP(pA)[FVFO_RHW] * pRc->scaleW;
  w2 = FLTP(pB)[FVFO_RHW] * pRc->scaleW;
#endif

  // Process Fog and zBuffering.
  if( pRc->state & STATE_REQUIRES_WBUFFER )
  {
#ifdef DCT_FIX
    wb1 = w1;
    wb2 = w2;
#else
    w1 = WSCALE( FLTP(pA)[FVFO_RHW] );
    w2 = WSCALE( FLTP(pB)[FVFO_RHW] );
#endif

    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
    {
      z1 = (float)((255 - RGBA_GETALPHA(pA[FVFO_SPECULAR])) << 8);
      z2 = (float)((255 - RGBA_GETALPHA(pB[FVFO_SPECULAR])) << 8);
    }
  }
  else
  {
    z1 = ZSCALE( FLTP(pA)[FVFO_SZ] );
    z2 = ZSCALE( FLTP(pB)[FVFO_SZ] );

    if (pRc->state & STATE_REQUIRES_VERTEXFOG)
    {
#ifdef DCT_FIX
      wb1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR]));
      wb2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR]));
#else
#ifdef PERF_USE_255_LOOKUP_TABLE
//  faster here to use a lookup vs. letting fpu do conversion -mls
      w1 = f_255_reverse_lookup[RGBA_GETALPHA(pA[FVFO_SPECULAR])];
      w2 = f_255_reverse_lookup[RGBA_GETALPHA(pB[FVFO_SPECULAR])];
#else
#pragma message("note - NOT compiling with performance opt for int to float conversion")
      w1 = (float)(255 - RGBA_GETALPHA(pA[FVFO_SPECULAR]));
      w2 = (float)(255 - RGBA_GETALPHA(pB[FVFO_SPECULAR]));
#endif // perf_use_255_lookup_table
#endif
    }
    else if (pRc->state & STATE_REQUIRES_HWFOG)
    {
#ifdef DCT_FIX
      wb1 = w1;
      wb2 = w2;
#else
      w1 = FLTP(pA)[FVFO_RHW];
      w2 = FLTP(pB)[FVFO_RHW];
#endif
    }
  }

  // First texture stage
  if( pRc->state & STATE_REQUIRES_ST_TMU0 )
  {
    float lscaleS, lscaleT;

    s1 = FLTP(pA)[FVFO_TU + pRc->t0CoordIndex];
    t1 = FLTP(pA)[FVFO_TV + pRc->t0CoordIndex];

    s2 = FLTP(pB)[FVFO_TU + pRc->t0CoordIndex];
    t2 = FLTP(pB)[FVFO_TV + pRc->t0CoordIndex];

    lscaleS = pRc->sst.scaleS;
    lscaleT = pRc->sst.scaleT;

    // two ways to texture. D3D wraps its textures going around the other direction
    // so we need to adjust S and T in order to get the correct result
    WRAPST(s, t, pRc->wrapU, pRc->wrapV);

    // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
      if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
    {
#ifdef DCT_FIX
      s1 = ((s1 * lscaleS) + TEXEL_SOFFSET) * w1;
      s2 = ((s2 * lscaleS) + TEXEL_SOFFSET) * w2;

      t1 = ((t1 * lscaleT) + TEXEL_TOFFSET) * w1;
      t2 = ((t2 * lscaleT) + TEXEL_TOFFSET) * w2;
#else
      s1 = ((s1 * lscaleS) + TEXEL_SOFFSET) * FLTP(pA)[FVFO_RHW];
      s2 = ((s2 * lscaleS) + TEXEL_SOFFSET) * FLTP(pB)[FVFO_RHW];

      t1 = ((t1 * lscaleT) + TEXEL_TOFFSET) * FLTP(pA)[FVFO_RHW];
      t2 = ((t2 * lscaleT) + TEXEL_TOFFSET) * FLTP(pB)[FVFO_RHW];
#endif
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

#if (NUMTEXTUREUNITS > 1)
  // Second texture stage
  if( pRc->state & STATE_REQUIRES_ST_TMU1 )
  {
    float lscaleS, lscaleT;

    s1a = FLTP(pA)[FVFO_TU + pRc->t1CoordIndex];
    t1a = FLTP(pA)[FVFO_TV + pRc->t1CoordIndex];

    s1b = FLTP(pB)[FVFO_TU + pRc->t1CoordIndex];
    t1b = FLTP(pB)[FVFO_TV + pRc->t1CoordIndex];

    lscaleS = pRc->sst.scaleS1;
    lscaleT = pRc->sst.scaleT1;

    // two ways to texture. D3D wraps its textures going around the other direction
    // so we need to adjust S and T in order to get the correct result
//    WRAPST(s, t, pRc->wrapU, pRc->wrapV);

    // NOTE: D3D passes in S, T and 1/W and we need S/W, T/W and 1/W
      if( pRc->state & STATE_REQUIRES_PERSPECTIVE )
    {
#ifdef DCT_FIX
      s1a = ((s1a * lscaleS) + TEXEL_S1OFFSET) * w1;
      s1b = ((s1b * lscaleS) + TEXEL_S1OFFSET) * w2;

      t1a = ((t1a * lscaleT) + TEXEL_T1OFFSET) * w1;
      t1b = ((t1b * lscaleT) + TEXEL_T1OFFSET) * w2;
#else
      s1a = ((s1a * lscaleS) + TEXEL_S1OFFSET) * FLTP(pA)[FVFO_RHW];
      s1b = ((s1b * lscaleS) + TEXEL_S1OFFSET) * FLTP(pB)[FVFO_RHW];

      t1a = ((t1a * lscaleT) + TEXEL_T1OFFSET) * FLTP(pA)[FVFO_RHW];
      t1b = ((t1b * lscaleT) + TEXEL_T1OFFSET) * FLTP(pB)[FVFO_RHW];
#endif
    }
    // if the texture is not prespective correct then the w is effectively equal to 1
    else
    {
      s1a = (s1a * lscaleS) + TEXEL_S1OFFSET;
      s1b = (s1b * lscaleS) + TEXEL_S1OFFSET;

      t1a = (t1a * lscaleT) + TEXEL_T1OFFSET;
      t1b = (t1b * lscaleT) + TEXEL_T1OFFSET;
    }
  } // texture
#endif

  if(xMajor)
  {
    // X Major Axis

  #if( NUMTEXTUREUNITS > 1)
    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (10 * 4) );
  #else
    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (8 * 4) );
  #endif

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
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
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

    SETFPD( cmdFifo, ghw0->sVx, ax + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay - 1.0f + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, aColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z1 );
    if(setupFlag & SST_SETUP_Wfbi)
#if DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb1 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w1 );
#endif
    if(setupFlag & SST_SETUP_W0)
#if DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w1 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
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

    SETFPD( cmdFifo, ghw0->sVx, bx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z2 );
    if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb2 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
#endif
    if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w2 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
#endif
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #if (NUMTEXTUREUNITS > 1)
    if(setupFlag & SST_SETUP_ST1)
    {
      SETFPD( cmdFifo, ghw0->sSow1, s1b );
      SETFPD( cmdFifo, ghw0->sTow1, t1b );
    }
  #endif
  #ifndef CMDFIFO
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

    SETFPD( cmdFifo, ghw0->sVx, bx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by - 1.0f + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z2 );
    if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb2 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
#endif
    if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w2 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
#endif
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #if (NUMTEXTUREUNITS > 1)
    if(setupFlag & SST_SETUP_ST1)
    {
      SETFPD( cmdFifo, ghw0->sSow1, s1b );
      SETFPD( cmdFifo, ghw0->sTow1, t1b );
    }
  #endif
  #ifndef CMDFIFO
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif
  }
  else
  {
    // Y Major
  #if (NUMTEXTURES > 1)
    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (10 * 4) );
  #else
    CMDFIFO_CHECKROOM( cmdFifo, PH3_SIZE + (8 * 4) );
  #endif

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
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
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

    SETFPD( cmdFifo, ghw0->sVx, ax + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, ay + PIXEL_OFFSET );
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
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pA)[FVFO_RHW] );
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

    SETFPD( cmdFifo, ghw0->sVx, bx - 1.0f + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z2 );
    if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb2 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
#endif
    if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w2 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
#endif
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #if (NUMTEXTUREUNITS > 1)
    if(setupFlag & SST_SETUP_ST1)
    {
      SETFPD( cmdFifo, ghw0->sSow1, s1b );
      SETFPD( cmdFifo, ghw0->sTow1, t1b );
    }
  #endif
  #ifndef CMDFIFO
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif

    SETFPD( cmdFifo, ghw0->sVx, bx + PIXEL_OFFSET );
    SETFPD( cmdFifo, ghw0->sVy, by + PIXEL_OFFSET );
    if(setupFlag & (SST_SETUP_RGB | SST_SETUP_A))
      SETPD( cmdFifo, ghw0->sARGB, bColor );
    if(setupFlag & SST_SETUP_Z)
      SETFPD( cmdFifo, ghw0->sVz, z2 );
    if(setupFlag & SST_SETUP_Wfbi)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOowfbi, wb2 );
#else
      SETFPD( cmdFifo, ghw0->sOowfbi, w2 );
#endif
    if(setupFlag & SST_SETUP_W0)
#ifdef DCT_FIX
      SETFPD( cmdFifo, ghw0->sOow0, w2 );
#else
      SETFPD( cmdFifo, ghw0->sOow0, FLTP(pB)[FVFO_RHW] );
#endif
    if(setupFlag & SST_SETUP_ST0)
    {
      SETFPD( cmdFifo, ghw0->sSow0, s2 );
      SETFPD( cmdFifo, ghw0->sTow0, t2 );
    }
  #if (NUMTEXTUREUNITS > 1)
    if(setupFlag & SST_SETUP_ST1)
    {
      SETFPD( cmdFifo, ghw0->sSow1, s1b );
      SETFPD( cmdFifo, ghw0->sTow1, t1b );
    }
  #endif
  #ifndef CMDFIFO
    if(_MM(drawGlobal))
      SET( cmdFifo, ghw0->sDrawTriCMD, 0 );
  #endif
  }

  CMDFIFO_EPILOG( cmdFifo );
}
#endif // TnL_HAL

//-------------------------------------------------------------------

#endif // DX >= 6

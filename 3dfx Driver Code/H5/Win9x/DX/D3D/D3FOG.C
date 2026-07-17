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
** File name:   d3fog.c
**
** Description: fog related functions
**
** $Revision: 9$
** $Date: 10/11/00 8:48:17 PM$
**
** $Log: 
**  9    3dfx      1.6.2.0.1.0 10/11/00 Brent           Forced check in to enforce
**       branching.
**  8    3dfx      1.6.2.0     06/05/00 Sam Hanna       Scaled the hw-fog-table
**       entries (linear formula only) if needed when w buffering is required.
**       Fixes PRS 14121.
** 
**  7    3dfx      1.6         04/06/00 Bob Seitsinger  Remove #ifdef WINNT in
**       PXLFOGLIN code for when to zero fog table. W98 needs same condition as
**       w2k.
**  6    3dfx      1.5         04/04/00 Russ Lind       Fix for W Fog - Linear
**       failure on w2k
**       Modified workaround from previous rev to zero out fog table only if nearZ
**       < 1.0F and farZ <= 1.0F, otherwise the W Fog - Linear test where nearZ =
**       1.0F and farZ = 1.0F fails.  This change is ifdef'd for winnt.
**  5    3dfx      1.4         04/03/00 Sam Hanna       Implemented a work around
**       for z-based-linear table fog. This fixes PRS 13517 - V5: Centipede, no
**       textures - only overlays appear once game starts (D3D only)
** 
**  4    3dfx      1.3         02/04/00 Steve Houston   Inserted #undef K6_2 under
**       WinNT builds to remove 3DNow optimizations that are not supported under
**       NT. Related to new triangle asm port from W9X.
**  3    3dfx      1.2         11/04/99 Edwin Wong      Replace fxExp() with exp()
**       from the math library. This fixed DCT 250 fog exponential square bug. MFC
**       fog sample also shows smoother transition for exp, and exp2 fog profile.
**  2    3dfx      1.1         10/26/99 Scott Kephart   Added initial support for
**       software T&L HAL
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 2     7/28/99 11:59p Bseitsin
** Changes to enable DX7 for W9x.
** 
** 1     6/02/99 6:40a Michael
** Branch from H3
** 
** 30    5/30/99 5:36p Edwin
** Remove ifdef MM, multi-monitor support is always enabled.
** 
** 29    3/27/99 2:03p Russ
** additional debug output
**
** 28    2/18/99 8:19a Russ
** add D3DPRINT's to fog renderstate functions
**
** 27    2/03/99 9:33p Russ
** for NT5, added return value from functions in _renderFuncs table.  This
** is needed in order to pass a new DCT test that throw garbage data at
** DP2
**
** 26    1/30/99 10:57p Adrians
** Code tidyup including removing some obsolete code in FogLinearTable
** generation.
**
** 25    1/26/99 5:27p Peterm
** Added unified header information
**
** 24    12/09/98 7:22a Russ
** NT5 D3D changes for Banshee
**
** 23    11/22/98 8:58p Andrew
** Changes to support multi-monitor
**
** 22    11/06/98 1:36p Adrians
** Updated K6_2 changes.
**
** 21    10/01/98 10:52p Adrians
** Re-coded Fog.  This fixes a problem when Wbuffering with Vertex Fog.
**
** 20    9/21/98 3:11p Adrians
** Optimisation to Fog HW setup.
** Added ZBIAS state change.
** Fix for multiple context's with different fog color.
** Set lodMin & lodMax to 1x1 when flat shading.
**
** 19    7/24/98 1:37p Hohn
**
** 18    6/30/98 5:29p Miriam
** Performance optimizations.
**
** 17    6/02/98 8:48p Adrians
** Exponential table fog is now back to how it was.
**
** 16    5/15/98 9:34a Miriam
** Need fog workaround for A2.
**
** 15    5/06/98 6:08p Adrians
** Changes for DX6 into DX5 driver.
**
** 1     4/29/98 6:31p Adrians
** Created
**
** 14    3/19/98 1:12p Miriam
** Banshee workaround for A0& A1 hw.
 *
 * 13    1/15/98 10:22a Adrians
 * Enabled Fog Dither.
 *
 * 12    1/12/98 5:39p Adrians
 * Fix for Exponential Fog (Not exact but as close as we can get).
 * Added an IndexToW conversion table.
 *
 * 11    11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 *
 * 10    11/09/97 2:50p Adrians
 * Single pk1's use an increment of 0.
 * Code added to find the triangle flavours used by apps.
 * Optimisation of triangle flavours.
 * Support for strips and fans in execute buffers.
 * Bug fix to wrapU and wrapV modes.
 *
 *
 * 9     10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 *
 * 8     10/09/97 10:23a Adrians
 * Now have a single SETPH macro.  Tidy-up of macro code.
 *
 * 7     10/03/97 5:23p Adrians
 * Fixed Fog Table.
 *
 * 6     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 *
 * 5     9/26/97 11:45a Adrians
 * Now supports proper cmdfifo packet 3 in these modules.
 *
 * 4     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/

#include "precomp.h"

#ifndef WINNT
#include "d3dhal.h"
#include "hw.h"
#include "d3global.h"
#include "ddglobal.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "math.h"
#include "d3contxt.h"
#else // ifdef WINNT
// shouston 1-29-00 : The K6-2 optimizations in this file are not yet
// implemented under WinNT.
#undef K6_2
#endif


#ifdef K6_2
#include "k6_2.h"
#endif

#define PXLFOGLIN // Define this variable to enable the linear z-based-hw-fog-tbl wk around when endZ <= 1.0

#ifdef REALLIBC
extern double fxPow(double x, double y);
#endif
extern int    float2int(float f);

#define FOG_TABLE_SIZE 64

#define INDEXTOW( i ) (*(float*)&indexToW[i])

#ifndef K6_2
static
#endif  // need this array in fogGenerateLinear_K62O
long indexToW[FOG_TABLE_SIZE] ={
0x3f800000, 0x3f924925, 0x3faaaaab, 0x3fcccccd, 0x40000000, 0x40124925, 0x402aaaab, 0x404ccccd,
0x40800000, 0x40924925, 0x40aaaaab, 0x40cccccd, 0x41000000, 0x41124925, 0x412aaaab, 0x414ccccd,
0x41800000, 0x41924925, 0x41aaaaab ,0x41cccccd, 0x42000000, 0x42124925, 0x422aaaab, 0x424ccccd,
0x42800000, 0x42924925, 0x42aaaaab, 0x42cccccd, 0x43000000, 0x43124925, 0x432aaaab, 0x434ccccd,
0x43800000, 0x43924925, 0x43aaaaab, 0x43cccccd, 0x44000000, 0x44124925, 0x442aaaab, 0x444ccccd,
0x44800000, 0x44924925, 0x44aaaaab, 0x44cccccd, 0x45000000, 0x45124925, 0x452aaaab, 0x454ccccd,
0x45800000, 0x45924925, 0x45aaaaab, 0x45cccccd, 0x46000000, 0x46124925, 0x462aaaab, 0x464ccccd,
0x46800000, 0x46924925, 0x46aaaaab, 0x46cccccd, 0x47000000, 0x47124925, 0x472aaaab, 0x474ccccd };

DWORD FnZTable[9][3] = {
  // 0. Fog - none, ZBuffer - none
  { 0,
    0,
    0 },
  // 1. Fog - none, ZBuffer - Zbuffer
  { STATE_REQUIRES_ZBUFFER | STATE_REQUIRES_OOZ,
    SST_SETUP_Z,
    0 },
  // 2. Fog - none, ZBuffer - Wbuffer
  { STATE_REQUIRES_WBUFFER | STATE_REQUIRES_W_FBI,
    SST_SETUP_Wfbi,
    0 },
  // 3. Fog - Vertex, ZBuffer - none
  { STATE_REQUIRES_W_FBI | STATE_REQUIRES_VERTEXFOG,
    SST_SETUP_Wfbi,
    SST_ENFOGGING | SST_FOG_Z | SST_FOG_ALPHA | SST_FOG_DITHER },
  // 4. Fog - Vertex, ZBuffer - ZBuffer
  { STATE_REQUIRES_OOZ | STATE_REQUIRES_ZBUFFER | STATE_REQUIRES_W_FBI | STATE_REQUIRES_VERTEXFOG,
    SST_SETUP_Z | SST_SETUP_Wfbi,
    SST_ENFOGGING | SST_FOG_Z | SST_FOG_ALPHA | SST_FOG_DITHER },
  // 5. Fog - Vertex, ZBuffer - Wbuffer
  { STATE_REQUIRES_W_FBI | STATE_REQUIRES_WBUFFER | STATE_REQUIRES_OOZ | STATE_REQUIRES_VERTEXFOG,
    SST_SETUP_Z | SST_SETUP_Wfbi,
    SST_ENFOGGING | SST_FOG_Z | SST_FOG_DITHER },
  // 6. Fog - HWTable, ZBuffer - none
  { STATE_REQUIRES_W_FBI | STATE_REQUIRES_HWFOG,
    SST_SETUP_Wfbi,
    SST_ENFOGGING | SST_FOG_DITHER },
  // 7. Fog - HWTable, ZBuffer - Zbuffer
  { STATE_REQUIRES_OOZ | STATE_REQUIRES_ZBUFFER | STATE_REQUIRES_W_FBI | STATE_REQUIRES_HWFOG,
    SST_SETUP_Z | SST_SETUP_Wfbi,
    SST_ENFOGGING | SST_FOG_DITHER },
  // 8. Fog - HWTable, ZBuffer - Wbuffer
  { STATE_REQUIRES_W_FBI | STATE_REQUIRES_WBUFFER | STATE_REQUIRES_HWFOG,
    SST_SETUP_Wfbi,
    SST_ENFOGGING | SST_FOG_DITHER },
};

/*-------------------------------------------------------------------
Function Name:  getFnZOffset

Description:    Utility function that takes fog enables and generates
                an index into the FnZTable

Return:         DWORD
-------------------------------------------------------------------*/
DWORD getFnZOffset( RC *pRc )
{
  if( pRc->fogEnable )
  {
    if( pRc->fogTableMode == D3DFOG_NONE )
      return( 3 + pRc->zEnable );
    else
      return( 6 + pRc->zEnable );
  }
  else
    return( pRc->zEnable );
}

//------------------------------------------------------------------------
//
// Fogtable support.
//
// So, How is D3D fog defined? Hmmm. Not much documentation so we will use
// the OpenGL definition.
//
// linear                = (fogend - z) / (fogend - fogstart)
// exponential           = e ** (-density x z)
// exponential squared   = e ** ((-density * z) ** 2)
//
// f = blending factor and is clamped to range [0.1]
// Cout = f*Cin + (1-f)Cfog
//
// This driver will fill in the hardware fog table with blending factors
// computed based on the type (linear...). SST will use W to index into
// the table and compute f. Then f will be blended into the color.
//
//
//------------------------------------------------------------------------

/*-------------------------------------------------------------------
Function Name:  fogGenerateExp

Description:    Calculates an exponential fog table for given params
                See D3D ddk docs for description of fog models.

Return:         void
-------------------------------------------------------------------*/
void fogGenerateExp( unsigned char fogtable[FOG_TABLE_SIZE], float density )
{
  int   i;
  float f;
  float scale;
  float dp;

  dp = density * INDEXTOW( FOG_TABLE_SIZE - 1 );
  scale = 1.0F / ( 1.0F - ( float ) exp( -dp ) );

  for ( i = 0; i < FOG_TABLE_SIZE; i++ )
  {
     dp = density * INDEXTOW( i );
     f = ( 1.0F - ( float ) exp( -dp ) ) * scale;

     if ( f > 1.0F )
        f = 1.0F;
     else if ( f < 0.0F )
        f = 0.0F;

     f *= 255.0F;

    fogtable[i] = ( unsigned char ) float2int( f );
    //D3DPRINT( 255, "Index [%d] =%d", i, fogtable[i] );
  }
}

/*-------------------------------------------------------------------
Function Name:  fogGenerateExp2

Description:    Calculates an exponential2 fog table for given params.
                See D3D ddk docs for description of fog models.

Return:         void
-------------------------------------------------------------------*/
void fogGenerateExp2( unsigned char fogtable[FOG_TABLE_SIZE], float density )
{
  int   i;
  float f;
  float scale;
  float dp;

  dp = density * INDEXTOW( FOG_TABLE_SIZE - 1 );
  scale = 1.0F / ( 1.0F - ( float ) exp( -( dp * dp ) ) );

  for ( i = 0; i < FOG_TABLE_SIZE; i++ )
  {
     dp = density * INDEXTOW( i );
     f = ( 1.0F - ( float ) exp( -( dp * dp ) ) ) * scale;

     if ( f > 1.0F )
        f = 1.0F;
     else if ( f < 0.0F )
        f = 0.0F;

     f *= 255.0F;

    fogtable[i] = ( unsigned char ) float2int( f );
    //D3DPRINT( 255, "Index [%d] =%d", i, fogtable[i] );
  }
} /* guFogGenerateExp2 */

/*-------------------------------------------------------------------
Function Name:  fogGenerateLinear

Description:    Calculates an exponential2 fog table for given params.
                See D3D ddk docs for description of fog models.

Return:         void
-------------------------------------------------------------------*/
#ifdef K6_2
void fogGenerateLinear_Orig( unsigned char fogtable[FOG_TABLE_SIZE], RC *pRc )
#else
void fogGenerateLinear( unsigned char fogtable[FOG_TABLE_SIZE], RC *pRc )
#endif // K6_2
{
  int i;
  float world_w, nearZ, farZ;
  float f;

  SETUP_PPDEV(pRc)

#ifdef PXLFOGLIN
// SH - 
// Since the hw doesn't support z-based table fog, we cannot support a table which
// starts and ends with values less than 1.0 (all entries end up being 255 and all objects are
// fogged out). Thus, in this case we set all the table entries to zero. This means that we are
// eliminating fogging for this case which is wrong but until we come up with a better work around,
// this is the easiest and lowest risk solution (??).
// Fixes PRS 13517 - V5: Centipede, no textures - only overlays appear once game starts (D3D only)

#pragma message(__FILELINE__ " undefine PXLFOGLIN to disable wk around for z-based hw fog tbl")

  nearZ = pRc->fogTableStart;
  farZ  = pRc->fogTableEnd;

  if ((nearZ < 1.0F) && (farZ <= 1.0F))
  {
	for ( i = 0; i < FOG_TABLE_SIZE; i++ )
	{
	  fogtable[i] = 0;
	}
  }
  else
#endif // PXLFOGLIN
  {
    for ( i = 0; i < FOG_TABLE_SIZE; i++ )
    {
      world_w = INDEXTOW( i ); //guFogTableIndexToW( i );
      //D3DPRINT( 255,"fogGenerateLinear index=%d world_w=0x%x",i,*(int *)&world_w );

      if( pRc->state & STATE_REQUIRES_WBUFFER )
	  {
	  	world_w = WSCALE(world_w);
	  }
      
      f = 255.f * (( world_w - nearZ ) / ( farZ - nearZ ));
      //D3DPRINT( 255,"fogGenerateLinear fog=0x%x nearZ=0x%x farZ=0x%x",
      //           *(int *)&f, *(int *)&nearZ, *(int *)&farZ );

      if ( f > 255.0F )
        f = 255.0F;
      else if ( f < 0.0F )
        f = 0.0F;

      //D3DPRINT( 255,"fogGenerateLinear f*255=0x%x ",*(int *)&f );
      fogtable[i] = ( unsigned char ) float2int(f);
      //D3DPRINT( 255,"FogGenerateLinear  entry[%d] = 0x%x",i,fogtable[i] );
    }
  }
}

/*-------------------------------------------------------------------
Function Name:  fogTable

Description:    Sets hardware fog table to passed in table

Return:         void
-------------------------------------------------------------------*/
void __stdcall fogTable(NT9XDEVICEDATA *ppdev, unsigned char fogtable[FOG_TABLE_SIZE] )
{
  int i;
  unsigned char *locTable = &fogtable[0];
  CMDFIFO_PROLOG(cmdFifo);

  CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + (FOG_TABLE_SIZE / 2) );
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( (FOG_TABLE_SIZE / 2), 1, fogTable, 0xF ) );

  D3DPRINT(D3DDBGLVL+1, "hwFogTable");

  for ( i = 0; i < (FOG_TABLE_SIZE / 2); i++ )
  {
    unsigned long e0, e1, d0, d1;

    e0 = locTable[0];                     /* lower entry */
    e1 = locTable[1];                     /* upper entry */
    d0 = (e1 - e0) << 2;                  /* delta0 in .2 format */
    d1 = (i == 31) ? e1 : locTable[2];    /* don't access beyond end of table */
    d1 = (d1 - e1) << 2;                  /* delta1 in .2 format */

    #if defined(H3_A0) || defined(H3_A1) || defined(H3_A2)
    SETPD( cmdFifo, ghw->fogTable[i], ~((e1 << 24) | (d1 << 16) | (e0 << 8) | d0 ));
    D3DPRINT(D3DDBGLVL+1, "  %08lX", ~(e1 << 24) | (d1 << 16) | (e0 << 8) | d0);
    #else
    SETPD( cmdFifo, ghw->fogTable[i], (e1 << 24) | (d1 << 16) | (e0 << 8) | d0 );
    D3DPRINT(D3DDBGLVL+1, "  %08lX", (e1 << 24) | (d1 << 16) | (e0 << 8) | d0);
    #endif

    locTable += 2;
  }
  CMDFIFO_EPILOG( cmdFifo );
}

/*-------------------------------------------------------------------
Function Name:  createTableAndLoad

Description:    Create a fog table for a given param set and load it
                into hardware.

Return:         void
-------------------------------------------------------------------*/
void createTableAndLoad(RC *pRc)
{
  SETUP_PPDEV(pRc)
  unsigned char localFogTable[FOG_TABLE_SIZE];

  switch(pRc->fogTableMode)
  {
    case  D3DFOG_LINEAR :
      D3DPRINT(D3DDBGLVL+1, "fogGenerateLinear: nearZ=%08lX  farZ = %08lX",
               pRc->fogTableStart, pRc->fogTableEnd);
      fogGenerateLinear(localFogTable, pRc ) ;
      break;

    case  D3DFOG_EXP :
      D3DPRINT(D3DDBGLVL+1, "fogGenerateExp: density=%08lX", pRc->fogDensity);
      fogGenerateExp(localFogTable, pRc->fogDensity) ;
      break;

    case  D3DFOG_EXP2 :
      D3DPRINT(D3DDBGLVL+1, "fogGenerateExp2: density=%08lX", pRc->fogDensity);
      fogGenerateExp2(localFogTable, pRc->fogDensity) ;
      break;

    default:
      D3DPRINT( 255, "WARNING: trying to set invalid fog type" ) ;
      break;

  } // fogtype

#ifdef DBG
  {
    int i;

    D3DPRINT(D3DDBGLVL+1, "localFogTable");
    for (i = 0; i < FOG_TABLE_SIZE; i+=8)
    {
      D3DPRINT(D3DDBGLVL+1, "  %02X : %02X %02X %02X %02X %02X %02X %02X %02X",
               i,
               localFogTable[i  ], localFogTable[i+1],
               localFogTable[i+2], localFogTable[i+3],
               localFogTable[i+4], localFogTable[i+5],
               localFogTable[i+6], localFogTable[i+7]);
    }
  }
#endif

  fogTable(ppdev, localFogTable);
}

/*-------------------------------------------------------------------
Function Name:  setFogMode

Description:    Set the fogging mode

Return:         void
-------------------------------------------------------------------*/
void setFogMode(RC *pRc)
{
  DWORD offset = getFnZOffset( pRc );

  pRc->state = (pRc->state & ~FNZ_STATE_MASK) | FnZTable[ offset ][ FNZ_STATE ];
  pRc->sst.sSetupMode = (pRc->sst.sSetupMode & ~FNZ_SETUP_MASK) | FnZTable[ offset ][ FNZ_SETUP ];
  pRc->sst.fogMode = (pRc->sst.fogMode & ~FNZ_FOGMODE_MASK) | FnZTable[ offset ][ FNZ_FOGMODE ];

  UPDATE_HW_STATE( SC_SOMETHING | SC_TRIANGLE_FLAVOUR );
}

/*-------------------------------------------------------------------
Function Name:  fogEnable

Description:    Enable/disable fog

Return:         void
-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall fogEnable(RC *pRc, ULONG state)
{
  pRc->fogEnable = state;

  D3DPRINT( RSTATE_DBG_LVL, "fogEnable Renderstate %d", state );

  setFogMode(pRc);

  if( state )
    pRc->useFog = FOG_ENABLED;
  else
    pRc->useFog = FOG_DISABLED;
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  fogColor

Description:    Set the fog color for when fogging is enabled

Return:         void
-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall fogColor(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "fogColor Renderstate %08lX", state );

  pRc->fogColor     = (D3DCOLOR) state;

  pRc->sst.fogColor = (ULONG)(   (RGBA_GETRED(pRc->fogColor)   << 16)
                               | (RGBA_GETGREEN(pRc->fogColor) << 8 )
                               | (RGBA_GETBLUE(pRc->fogColor)       ));

#ifdef TnL_HAL
  pRc->tl.lighting.fog_color = (D3DCOLOR) state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FOG;
#endif                              

  UPDATE_FOG_STATE( SC_FOGCOLOR );
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  fogTableMode

Description:    Sets the fog table mode

Return:         void
-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall fogTableMode(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "fogTableMode Renderstate %d", state );

  pRc->fogTableMode  = state;

  setFogMode( pRc );

  if( state != D3DFOG_NONE )
  {
    pRc->useFogTable = FOGTABLE_ENABLED;
    UPDATE_FOG_STATE( SC_FOGTABLE );
  }
  else
    pRc->useFogTable = FOGTABLE_DISABLED;
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  fogTableStart

Description:    Sets the fog table start distance

Return:         void
-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall fogTableStart(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "fogTableStart Renderstate %08lX", state );

  pRc->fogTableStart = *(float *)&state;
#ifdef TnL_HAL   
  pRc->tl.lighting.fog_start = *(float *)&state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FOG;
#endif


  UPDATE_FOG_STATE( SC_FOGTABLE );
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  fogTableEnd

Description:    Sets the fog table end distance

Return:         void
-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall fogTableEnd(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "fogTableEnd Renderstate %08lX", state );

  pRc->fogTableEnd   = *(float *)&state;
#ifdef TnL_HAL   
  pRc->tl.lighting.fog_end = *(float *)&state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FOG;
#endif

  UPDATE_FOG_STATE( SC_FOGTABLE );
  RENDERFXN_OK;
}

/*-------------------------------------------------------------------
Function Name:  fogDensity

Description:    Sets the fog table density

Return:         void
-------------------------------------------------------------------*/
RENDERFXN_RETVAL __stdcall fogDensity(RC *pRc, ULONG state)
{
  D3DPRINT( RSTATE_DBG_LVL, "fogDensity Renderstate %08lX", state );

  pRc->fogDensity    = *(float *)&state;
#ifdef TnL_HAL
  pRc->tl.lighting.fog_density = *(float *)&state;
  pRc->tl.dwDirtyFlags |= TLPV_DIRTY_FOG;
#endif

  UPDATE_FOG_STATE( SC_FOGTABLE );
  RENDERFXN_OK;
}

//-------------------------------------------------------------------

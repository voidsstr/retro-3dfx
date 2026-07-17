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
**  10   3dfx      1.6.1.2     10/23/00 Johnny Trainor  Updated so we no longer use
**       surface local pointers.
**  9    3dfx      1.6.1.1     10/11/00 Brent           Forced check in to enforce
**       branching.
**  8    3dfx      1.6.1.0     09/22/00 Johnny Trainor  Preparation for DX8 support
**       in the driver. Modifications so that we can build the Win9x driver using
**       the Win98 DDK and the DX8 DDK. 
**  7    3dfx      1.6         01/24/00 Matt McClure    Modifications to copy Scale
**       and Center 'S' and 'T' to support new 3DNow and KNI Triangle Rendering
**       routines.  Ifdef'd NEWASMTRI==1
**  6    3dfx      1.5         01/18/00 Bob Seitsinger  Changes to correctly write
**       to selected chip components (fbi,tmu0,tmu1). Chip component selection
**       occurred correctly for command packet header  writes, but needed to also
**       be done for data packet writes.
**  5    3dfx      1.4         01/11/00 Scott Kephart   Reverted back to source
**       prior to new triangle assembly code
**  4    3dfx      1.3         01/10/00 Matt McClure    Added support for new
**       assembler triangle routines, #ifdef NEWASMTRI==1 surrounds all changes.
**  3    3dfx      1.2         10/28/99 Christopher Wilcox Hardware definition
**       changes to merge divergent h3defs.h.
**  2    3dfx      1.1         09/20/99 Steve Rogers    Porting my fix for PRS 6247
**       from V3 OEM tree:  Reload texture pointers if context is switched.
** 
**  1    3dfx      1.0         09/11/99 StarTeam VTS Administrator 
** $
** 
** 8     7/29/99 3:43p Bseitsin
** Remove an #ifdef WINNT - not required, since NT won't call D3D DX5/DX6
** stuff.
** 
** 7     7/28/99 11:59p Bseitsin
** Changes to enable DX7 for W9x.
** 
** 6     7/13/99 2:27p Cshaw
** Added runtime support for Napalm's multitexturing (NAPALM_CU).
** 
** 5     7/09/99 4:33p Bseitsin
** Backwards compatability changes.
** 
** 4     6/21/99 12:42p Andrew
** removed HAL_HW
** 
** 3     6/14/99 3:17p Bseitsin
** Triangle Iterator Column Band Control support.
** 
** 2     6/04/99 8:33a Cshaw
** Added combineMode regs to the list of regs updated upon state change.
** 
** 1     6/02/99 6:40a Michael
** Branch from H3
** 
** 65    5/24/99 5:04p Bseitsin
** Removal of Antialiasing code.
** 
** 64    4/26/99 11:36a Stb_bseitsin
** 32bit rendering mods. New SETSURFACEPIXELDEPTH macro.
** 
** 63    4/23/99 2:06p Stb_bseitsin
** 32-bit rendering mods
** 
** 62    4/09/99 12:35p Stb_bseitsin
** Added Napalm registers. Added ifdef H5.
** 
** 61    1/20/99 1:49p Adrians
** Changes to DX6 hardware setup.
** 
** 60    1/18/99 2:08p Adrians
** Removed obsolete variables and associated code.
** 
** 59    1/12/99 8:04a Cshaw
** Added runtime switches to the performance analysis code (use SIce to
** modify).
** 
** 58    1/07/99 12:01p Cshaw
** Added #defs for doing wb99 performance analysis.
** To disable areas of the driver use the following environment vars:
** nd=1 (manditory - enables NULLDRIVER)
** OND_TEXD=1 (optional - OverridesNullDriver by executing
** TextureDownloads)
** OND_STATE=1 (optional - OverridesNullDriver by executing
** D3DStateChanges)
** OND_HWSU=1 (optional - OverridesNullDriver by executing Hardware setup)
** OND_ZB=1 (optional - OverridesNullDriver by executing ZBuffer clears)
** OND_TRI=1 (optional - OverridesNullDriver by executing Triangle
** Rendering)
** 
** 57    12/23/98 1:16p Martin
** Set auxBufferAddr when zWriteEnable is on.  If an app (MFCtex), enables
** writes to the zbuffer, but doesn't enable zbuffer compares, then we
** still need to set up the zbuffer address.
** 
** 56    12/02/98 8:24a Martin
** Turn off anti-aliasing by default and rename some variables for
** super-sampling AA.
** 
** 55    11/30/98 8:14p Adrians
** Removed some compiler warnings.
** 
** 54    11/22/98 8:56p Andrew
** Changes to support multi-monitor
** 
** 53    11/13/98 2:52p Miriam
** Support for DX6 triangle flavor & texture flavor tracing. Just compile
** with debug & fp=1 or tp=1.
** 
** 52    10/16/98 4:33p Artg
** change ifdef h3 to if defned(h3) || defined (H4)
** 
** 51    10/06/98 11:58p Adrians
** Migrate the aaTestFlags from the DX6 driver to the DX5 driver.
** 
** 50    10/04/98 9:44p Adrians
** When flat shading set texturing units lodmin/max to 1x1.
** 
** 49    10/02/98 11:16a Adrians
** Set maximum DX5 renderstate to D3DRENDERSTATE_STIPPLEPATTERN31.  This
** fixes a problem with Zbias not functioning under DX5.
** 
** 48    9/24/98 4:22p Adrians
** Added initial Instrumentation support.
** 
** 47    9/21/98 3:10p Adrians
** Optimisation to Fog HW setup.
** Added ZBIAS state change.
** Fix for multiple context's with different fog color.
** Set lodMin & lodMax to 1x1 when flat shading.
** 
** 46    9/12/98 12:56a Adrians
** Clean up renderstate trace code and add new DX6 states.
** Add trace support for DX6 texture stage states.
** General DX6 code tidyup.
** 
** 45    8/28/98 4:39p Martin
** Copy execute-buffer super-sampling AA implementation into
** drawPrimitive.
** 
** 44    8/28/98 3:20p Martin
** Reload palette when texture handle changes. Old fix caused performance
** degradation. New fix should only download palette at texture handle
** change.
** 
** 43    8/28/98 12:37p Martin
** Do super-sampling AA when the registry key is set to 2.  Do edge AA if
** registry key == 1 OR we are not running at the 2 magical resolutions of
** 640x480 or 800x600.
** 
** No more conditional compilation of AA.
** 
** 42    8/26/98 10:39a Adrians
** Add previous typo fix to execute buffers and DX6 setup.
** 
** 41    8/23/98 12:49a Adrians
** Added support for AA SuperSampling.
** 
** 40    7/31/98 10:34p Miriam
** D3D & Glide cooperation. Allow each API to set/reset state to indicate
** that  the HW state has been changed.
** 
** 39    7/29/98 3:30p Adrians
** DX6 Changes.
** 
** 38    7/24/98 1:37p Hohn
** 
** 37    7/10/98 4:50p Martin
** check if palettized texture and if palette has changed.
** If so then reload the palette.
** 
** 36    7/02/98 12:52p Adrians
** Fix for AA hang.
** 
** 35    7/01/98 3:49p Miriam
** Clip registers are now set for drawing surfaces.
** 
** 34    6/30/98 5:29p Miriam
** Performance optimizations.
** 
** 33    5/29/98 2:02p Adrians
** Enhancements to aa.
** 
** 32    5/26/98 7:22p Adrians
** Added antialiasing.
** 
** 30    5/18/98 3:20p Adrians
** Add some missing semi colon's.
** 
** 29    5/14/98 6:04p Miriam
** Merge in H3 support.
** 
** 28    5/06/98 6:08p Adrians
** Changes for DX6 into DX5 driver.
** 
** 2     5/01/98 4:11p Adrians
** Compile options for dx5 and dx6.
** Removed redundent returns.
** 
** 1     4/29/98 6:31p Adrians
** Created
** 
** 27    3/27/98 5:50p Adrians
** Broadcast palette changes to both TMU's.
** 
** 26    3/27/98 1:44p Adrians
** Code tidyup.
** Removed palette and texture clamp compile options.
** 
** 25    3/27/98 11:16a Adrians
** Code added for triangle profiling.
** 
** 24    2/22/98 5:32p Adrians
** In DEBUG displays warning if Lock active when rendering.
** 
** 23    2/21/98 10:18p Adrians
** Added FRONT_BUFFER_RENDER compile option.
 * 
 * 22    1/15/98 10:23a Adrians
 * Added ZBIAS support.
 * 
 * 21    1/13/98 5:31p Adrians
 * Fix for Lego Island.  Moved CMDFIFO_EPILOG before ZVisibility.
 * 
 * 20    11/23/97 3:56p Suninn
 * replay _d3Global with _D3 & D3G macros
 * 
 * 19    11/21/97 5:29p Adrians
 * Restore 3D registers after 2D has used them.
 * Added context change code for DrawPrimitives.
 * 
 * 18    11/09/97 2:50p Adrians
 * Single pk1's use an increment of 0.
 * Code added to find the triangle flavours used by apps.
 * Optimisation of triangle flavours.
 * Support for strips and fans in execute buffers.
 * Bug fix to wrapU and wrapV modes.
 * 
 * 17    10/27/97 2:56p Adrians
 * Fixed problem with z-visibility, causing some RM apps to hang.
 * 
 * 16    10/27/97 2:25p Adrians
 * Fixed 8 bit texture download.
 * 8 bit palettes are now built as default (p8=1).
 * Incorporated software triangle setup. Added build option (default
 * ss=0).
 * Command fifo debug build option added (default fd=0).
 * Texture clamping build option (default tc=1).
 * 
 * 15    10/24/97 7:14p Miriam
 * Support for 2 tmu and trilinear. Will see 2x the memory on a 2 tmu
 * system. Trilinear is 1 pass on 2 tmu & lodDither for 1 tmu. 
 * Fixed command fifo problem when using save/restore.
 * 
 * 14    10/14/97 2:31p Suninn
 * prepare for h3
 * 
 * 13    10/14/97 11:22a Adrians
 * Added Miriam's 4M texture support.  Added build environment for H3.
 * 
 * 12    10/10/97 10:42a Adrians
 * Removed all references to DIRECTX5.
 * 
 * 11    10/09/97 10:23a Adrians
 * Now have a single SETPH macro.  Tidy-up of macro code.
 * 
 * 10    10/07/97 1:39p Adrians
 * Optimisation to myRenderPrimitive setup.
 * New Caching enable call changes.
 * 
 * 8     10/02/97 8:38p Adrians
 * Include init code into build. Enable Write Combining.  Inline system
 * functions.  Change optimisations.  Some code tidy up.
 * 
 * 7     9/26/97 11:45a Adrians
 * Now supports proper cmdfifo packet 3 in these modules.
 * 
 * 6     9/16/97 5:13p Adrians
 * Flat shading now uses RGB iterator rather than c0/c1.
 * 
 * 4     9/03/97 5:52p Adrians
 * Updated File Header Comment.
 * Now includes LOG of SourceSafe changes.
*/
#include "precomp.h"

// Fix for building with Windows 98 DDK (Must include DDrawI first)
#include "ddrawi.h"
#include <d3dhal.h>
#include "hw.h"
#include "d3global.h"
#include "d3tri.h"
#include "fxglobal.h"
#include "fifomgr.h"
#include "d3contxt.h"
#include "d3txtr.h"
#include "sst1init.h"

#include "dxins.h"

//-------------
//
// Render state
//
//-------------
DWORD __stdcall ddiRenderState(LPD3DHAL_RENDERSTATEDATA prd)
{
  SETUP_PPDEV(prd->dwhContext)
  RC                  *pRc;
  LPBYTE              lpData;
  LPD3DSTATE          lpState;
  int                 i;

  D3D_ENTRY( "ddiRenderState" );
  INS_ENTRY( INSC_D3DRENDERSTATE );
  
#if defined( NULLDRIVER ) 
  if (!_D3(ondrtState)) {
	prd->ddrval = DD_OK;
	INS_EXIT( );
	D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
  
#ifdef FXTRACE
  D3DPRINT( 255,"RenderState, prd->dwhContext =%08lx", prd->dwhContext);
  if (CONTEXT_VALIDATE(prd->dwhContext) ) 
  {
    D3DPRINT( 255,"RenderState, bad context =0x08lx", prd->dwhContext );
    prd->ddrval = D3DHAL_CONTEXT_BAD;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif

  pRc    = CONTEXT_PTR(prd->dwhContext);
  lpData = (LPBYTE)(((LPDDRAWI_DDRAWSURFACE_INT)prd->lpExeBuf)->lpLcl->lpGbl->fpVidMem);
  i      = prd->dwCount;
  lpState = (LPD3DSTATE) (lpData + prd->dwOffset);
                      
  // for every renderstate                           
  while (i > 0)
  {
    DWORD type = (DWORD) lpState->drstRenderStateType;

    printRenderState(type, lpState->dwArg[0]) ;
  
    if (IS_OVERRIDE(type)) 
    {
        DWORD override = GET_OVERRIDE(type);
        if (lpState->dwArg[0]) 
        {
            //D3DPRINT( 255,"RenderState, setting override for state %d", override );
            STATESET_SET(pRc->overrides, override);
        } 
        else 
        {
            //D3DPRINT( 255,"RenderState, clearing override for state %d", override );
            STATESET_CLEAR(pRc->overrides, override);
        }
        goto nextstate;
    }

    if (STATESET_ISSET(pRc->overrides, type)) 
    {
        D3DPRINT( 255, "in ddiRenderState, state %d is overridden, ignoring", type );
        goto nextstate;
    }

    D3DPRINT( 255, "ddiRenderState (%d, %d)", type, lpState->dwArg[0] );

    // most common renderstate is texture handle so inline to enhance performance
    if (type == D3DRENDERSTATE_TEXTUREHANDLE)
    {

      ULONG state = lpState->dwArg[0];  
      DWORD oldTexture = pRc->texture;
      DWORD oldVertexColorType = pRc->vertexColorType;
      extern ULONG tLodT1[6][5]; 
      extern ULONG tLodT0[6][5]; 
      extern ULONG textureModeCombineT1[6][5];
      extern ULONG textureModeCombineT0[6][5];
      extern void __stdcall setupColorPath(RC *pRc, ULONG state);

      pRc->texture = (D3DTEXTUREHANDLE)state; 

    #if( DX >= 6 )
      // Disable texture stages
      pRc->textureStage[0].changed = 0;
      pRc->textureStage[1].changed = 0;
    #endif
      
      // when a texture is loaded the tLOD register is setup and so is some of the
      // texture mode
      if (pRc->texture != 0)
      {
        TXTRDESC *txtr;
        ULONG    home;
        txtr     = TXTRDESC_FROM_HNDL(state);

        // don't use mipmaps even if they are downloaded
        if (   (pRc->texMin == D3DFILTER_NEAREST)
            || (pRc->texMin == D3DFILTER_LINEAR))
        {
          pRc->sst.tLODT0 = txtr->tLODnoMipMaps[TREX0] ; 
          pRc->sst.tLODT1 = txtr->tLODnoMipMaps[TREX1] ;
          home = txtr->noMipMapsHome - 1; // largest mipmaplevel is on this tmu       
          pRc->sst.baseAddr  = txtr->noMipMapsBaseAddr[0] ;
          pRc->sst.baseAddr1 = txtr->noMipMapsBaseAddr[1] ;
        }
        else 
        {
          pRc->sst.tLODT0 = txtr->tLOD[TREX0] ;
          pRc->sst.tLODT1 = txtr->tLOD[TREX1] ;
          home = txtr->home - 1;
          pRc->sst.baseAddr  = txtr->baseAddr[0] ;
          pRc->sst.baseAddr1 = txtr->baseAddr[1] ;
        }
        
        pRc->sst.tLODT0  |= tLodT0[pRc->texMin - 1][home];
        pRc->sst.tLODT1  |= tLodT1[pRc->texMin - 1][home];

        pRc->sst.textureModeT0 = textureModeCombineT0[pRc->texMin - 1][home];
        pRc->sst.textureModeT1 = textureModeCombineT1[pRc->texMin - 1][home];

        // reset the fields in texture that are set when texture is loaded and then
        // merge in the new flags
        pRc->sst.textureMode &= ~(SST_TFORMAT) ;
        
        if( TEXFMTFLG_PALETTIZED & txtr->formatFlags)
        {
        #if( DX >= 6 )
          if( DDRAWIPAL_ALPHA & PALETTEGBL(pRc->texture)->dwFlags )
          {
            pRc->sst.textureMode |= ( TEXFMT_P8_RGBA << SST_TFORMAT_SHIFT );
            txtr->formatFlags |= TEXFMTFLG_PALETTIZED_ALPHA;
          }
          else
        #endif
          {
            pRc->sst.textureMode |= ( TEXFMT_P8_RGB << SST_TFORMAT_SHIFT );
            txtr->formatFlags &= ~TEXFMTFLG_PALETTIZED_ALPHA;
          }
        }
        else
          pRc->sst.textureMode |= txtr->format;
          
        // the texture blend is dependant on the format of the texture. Textures
        // with alpha are treated differently than textures without alpha. see setupcolorpath.
        if( txtr->formatFlags & (TEXFMTFLG_ALPHA | TEXFMTFLG_PALETTIZED_ALPHA))
          pRc->vertexColorType = RX_VERTEX_COLOR_RGBA;
        else
          pRc->vertexColorType = RX_VERTEX_COLOR_RGB;

        pRc->state |= (STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_W_TMU0);
        pRc->sst.sSetupMode |= (SST_SETUP_W0 | SST_SETUP_ST0);
       
        if (PALETTIZEDHANDLE(pRc->texture))
          TXTRNEWPALETTE(ppdev,PALETTEGBL(pRc->texture));
      }
      else
      {
        pRc->state &= ~(STATE_REQUIRES_ST_TMU0 | STATE_REQUIRES_W_TMU0 | STATE_REQUIRES_PERSPECTIVE);
        pRc->sst.sSetupMode &= ~(SST_SETUP_W0 | SST_SETUP_ST0);
        
        UPDATE_HW_STATE( SC_TLOD );
      }

      // FBI pipe must be flushed when transitioning between non-textured triangles 
      // and textured triangles 
      if( ((oldTexture == 0) && (state != 0)) ||
          ((oldTexture != 0) && (state == 0)) )
      {
        if ((state != 0) && (pRc->texturePerspective))
          pRc->state |= (STATE_REQUIRES_PERSPECTIVE);

        setupColorPath(pRc, pRc->texMapBlend) ;       

        // tri routine only changes on a transition
        UPDATE_HW_STATE(SC_TRIANGLE_FLAVOUR | SC_NEED_NOP);
      }
      else if (oldVertexColorType != pRc->vertexColorType)
        setupColorPath(pRc, pRc->texMapBlend) ;       
      
      UPDATE_HW_STATE(SC_SOMETHING);
    }

    else if (type <= D3DRENDERSTATE_STIPPLEPATTERN31)
      _renderFuncs[type](pRc, lpState->dwArg[0]) ;
nextstate:    
    i--;
    lpState++;
  }
  
  prd->ddrval = DD_OK;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
} 

//-----------------
//
// Render primitive
//
//-----------------
DWORD __stdcall ddiRenderPrimitive(LPD3DHAL_RENDERPRIMITIVEDATA prd)
{
  SETUP_PPDEV(prd->dwhContext)
  RC                 *pRc;
  LPD3DINSTRUCTION    lpIns;
  LPD3DTLVERTEX       lpVertices;
  LPBYTE              lpData, prim;
  CMDFIFO_PROLOG(cmdFifo);

  D3D_ENTRY( "ddiRenderPrimitive" );
  INS_ENTRY( INSC_D3DRENDERPRIMITIVE );

#if defined( NULLDRIVER )  
  if (!_D3(ondrtHWSU)) {//if this RenderPrimitive is actually called, I need to separate HWupdate and triangle rendering.
	prd->ddrval = DD_OK;
	INS_EXIT( );
	D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif

#if defined( DEBUG )  
  if( _FX( openLockCount ) )
    D3DPRINT( 0, "WARNING - RenderPrimitive during lock !!" );
#endif
    
#ifdef FXTRACE
  D3DPRINT( 255, "RenderPrimitive, prd->dwhContext =%08lx", prd->dwhContext );
  if (CONTEXT_VALIDATE(prd->dwhContext))
  {
    D3DPRINT( 255,"RenderPrimitive, bad context =0x08lx", prd->dwhContext );
    prd->ddrval = D3DHAL_CONTEXT_BAD;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
#endif
   
  pRc        = CONTEXT_PTR(prd->dwhContext);
  lpData     = (LPBYTE)(((LPDDRAWI_DDRAWSURFACE_INT)prd->lpExeBuf)->lpLcl->lpGbl->fpVidMem);
  lpIns      = &prd->diInstruction;
  prim       = lpData + prd->dwOffset;
  lpVertices = (LPD3DTLVERTEX)((LPBYTE)((LPDDRAWI_DDRAWSURFACE_INT)prd->lpTLBuf)->lpLcl->lpGbl->fpVidMem
                               + prd->dwTLOffset);
  
  if (_D3(last).colBufferAddr != GET_HW_ADDR(pRc->lpDDS))
  {
    CMDFIFO_CHECKROOM( cmdFifo, 2 * (PH1_SIZE + 2) );

    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, colBufferAddr, 0xf ) );
    if(IS_TILED(GET_HW_ADDR(pRc->lpDDS)))
    { 
      DWORD colBufferStride;
    
      colBufferStride = (_DS(ddTileStride) & 0x3FFFL) | 0x8000L;
      SETPD( cmdFifo, ghw0->colBufferAddr,   (GET_HW_ADDR(pRc->lpDDS) & 0x7FFFFFFFL));
      SETPD( cmdFifo, ghw0->colBufferStride, colBufferStride);
    }
    else
    {
      SETPD( cmdFifo, ghw0->colBufferAddr,   GET_HW_ADDR(pRc->lpDDS));
      SETPD( cmdFifo, ghw0->colBufferStride, pRc->lpDDS->lpGbl->lPitch);
    }

    // Reset the clipping registers
    SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, clipLeftRight, 0xf ) );
    SETPD( cmdFifo, ghw0->clipLeftRight, (DWORD)pRc->lpDDS->lpGbl->wWidth);
    SETPD( cmdFifo, ghw0->clipBottomTop, (DWORD)pRc->lpDDS->lpGbl->wHeight);

    _D3(last).colBufferAddr = GET_HW_ADDR(pRc->lpDDS);
  }

  if (pRc->zEnable || pRc->zWriteEnable)
  {
    if (_D3(last).auxBufferAddr != GET_HW_ADDR(pRc->lpDDSZ))
    {
      CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 2 );

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 2, 1, auxBufferAddr, 0xf ) );
      if(IS_TILED(GET_HW_ADDR(pRc->lpDDSZ)))
      {
        DWORD auxBufferStride;
      
        auxBufferStride = (_DS(ddTileStride) & SST_BUFFER_TILE_STRIDE) | SST_BUFFER_MEMORY_TILED;
        SETPD( cmdFifo, ghw->auxBufferAddr,   (GET_HW_ADDR(pRc->lpDDSZ) & 0x7fffffffL));
        SETPD( cmdFifo, ghw->auxBufferStride, auxBufferStride);
      }
      else
      {
        SETPD( cmdFifo, ghw->auxBufferAddr,   GET_HW_ADDR(pRc->lpDDSZ));
        SETPD( cmdFifo, ghw->auxBufferStride, pRc->lpDDSZ->lpGbl->lPitch);
      } 
      _D3(last).auxBufferAddr = GET_HW_ADDR(pRc->lpDDSZ);
    }
  }

  //------------------------------------------------------------
  // Update Context
  // Either we just changed contexts or something in the context
  // changed and we need to update the hardware state. Must update
  // context before process the Z visibility (calls triangle code)
  //------------------------------------------------------------
  // This optimization can not be enabled until the appropriate changes to
  // DDraw have been made.
  if( prd->dwhContext != _D3( lastContext ) )
  {
    UPDATE_HW_STATE( SC_STATE );
    UPDATE_FOG_STATE( SC_FOGALL );

	// srogers 8/17/99 - Reload textures if context is switched
	// Fixes PRS 6247
    pRc->textureStage[0].changed = 1;
    pRc->textureStage[1].changed = 1;
  }
  
  if( HW_STATE_CHANGED )
  {
    _D3(lastContext) = prd->dwhContext;    

    if( HW_STATE_CHANGED & SC_TRIANGLE_FLAVOUR )
      setDrawTriangle(pRc);
        
    triangleFlavour( pRc, pRc->state, 0, lpIns->wCount );

    pRc->sst.fbzMode &= ~(SST_ENCHROMAKEY);

    //------------------------------------------------------------------------
    //
    // Set attributes one time for all triangles  
    // NOTE: we are setting attributes here instead of when the attribute is             
    //       set for several reasons: 
    //       1. Direct Draw may change states because DD calls are interleaved
    //       2. Processes have just changed and a process swap occured.
    //       3. May have just swapped back from a glide application.
    //       4. This should be optimized later!!!!!!!!
    //------------------------------------------------------------------------
  
    //----------------
    //
    // Texture mapping
    //
    //----------------
    //if you change from texture mapping to non-texture mapping then must reset texturemode

    if ((pRc->texture != 0))
    {  
      TXTRDESC *txtr = TXTRDESC_FROM_HNDL(pRc->texture);
    
      CMDFIFO_CHECKROOM( cmdFifo, 2 * (PH4_SIZE + 3) );
      
      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R3, textureMode, TMU2CHIP(TREX0)) );
      SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->textureMode, pRc->sst.textureMode | pRc->sst.textureModeT0);
      SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->tLOD, pRc->sst.tLODT0 );
      SETPD( cmdFifo, SST_TREX(ghw0,TREX0)->texBaseAddr, pRc->sst.baseAddr );

#if (NEWASMTRI==1)
  #if (STBKNI==1)
      // Remove the involvement of Floating Point Code
      // Store off the texture scale components
      *((int*)&(_asm_data.kni_array[SCALE_S2])) = *((int*)&(_asm_data.scale_s)) = *((int*)&(pRc->sst.scaleS)) = *((int*)&(txtr->scaleS));
      *((int*)&(_asm_data.kni_array[SCALE_T2])) = *((int*)&(_asm_data.scale_t)) = *((int*)&(pRc->sst.scaleT))  = *((int*)&(txtr->scaleT));
      // Store off the texture pixel center offset
      *((int*)&(_asm_data.kni_array[TEX_OFF_S2])) = *((int*)&(_asm_data.tex_offset_s)) = *((int*)&(pRc->sst.centerS)) = *((int*)&(txtr->centerS));
      *((int*)&(_asm_data.kni_array[TEX_OFF_T2])) = *((int*)&(_asm_data.tex_offset_t)) = *((int*)&(pRc->sst.centerT)) = *((int*)&(txtr->centerT));
  #else // Not STBKNI
      // Remove the involvement of Floating Point Code
      // Store off the texture scale components
      *((int*)&(_asm_data.scale_s)) = *((int*)&(pRc->sst.scaleS)) = *((int*)&(txtr->scaleS));
      *((int*)&(_asm_data.scale_t)) = *((int*)&(pRc->sst.scaleT))  = *((int*)&(txtr->scaleT));
      // Store off the texture pixel center offset
      *((int*)&(_asm_data.tex_offset_s)) = *((int*)&(pRc->sst.centerS)) = *((int*)&(txtr->centerS));
      *((int*)&(_asm_data.tex_offset_t)) = *((int*)&(pRc->sst.centerT)) = *((int*)&(txtr->centerT));
  #endif // STBKNI
#else // Not NEWASMTRI
      pRc->sst.scaleS    = txtr->scaleS ;
      pRc->sst.scaleT    = txtr->scaleT ;

      pRc->sst.centerS   = txtr->centerS;
      pRc->sst.centerT   = txtr->centerT;
#endif // NEWASMTRI

      // if palettized texture and it changed then redownload it.
      // why download now instead of when texture id changes - this is the conservative
      // approach and the palette can change for numerous reasons so lets make sure
      // that it is download before each mesh (can optimize later)

#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      if (PALETTIZEDHANDLE(pRc->texture) && (PALETTECHANGED & _D3(flags)))
      {
        CMDFIFO_SAVE( cmdFifo );

        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ),
                            PALETTEGBL(pRc->texture),
                            txtr->formatFlags );

        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);
        CMDFIFO_RELOAD( cmdFifo );
      }
#else
      if(PALETTIZEDHANDLE(pRc->texture) && 
        ((_D3(flags) & PALETTECHANGED) || 
          (_D3(prevContentStamp) != PALETTEGBL(pRc->texture)->dwContentsStamp)))
      {
          CMDFIFO_SAVE( cmdFifo );
      
        // save off current value of dwContentStamp of palette associated
        // with texture.  When a palette changes, we don't get notified of
        // this event, so we will compare the previous one with the current
        // one, and reload the palette if different.
        _D3(prevContentStamp) = PALETTEGBL(pRc->texture)->dwContentsStamp;
      
      #if( DX >= 6 )      
          TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
            (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable,
            txtr->formatFlags );
      #else
          TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
            (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable );
      #endif
        
        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);
        CMDFIFO_RELOAD( cmdFifo );
      }
#endif

      //----------------
      //
      // Chroma Keying
      //
      //----------------
      // The chroma key value is stored in the texture surface. This is the only attribute
      // that is stored in the surface and not the rendering context. If the source chroma
      // key value is set for the texture surface then use the color. SST-1 chroma key
      // tests after the bi-linear filter and this is not correct. D3D performs the chroma
      // test before the filter. Scott/Gary said they would fix it one day...
      if (
            (pRc->colorKeyEnable) &&
            (((LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(pRc->texture)->surfLcl))->dwFlags & DDRAWISURF_HASCKEYSRCBLT))
      { 
        int r, g, b, xrgb;
      
        r=g=b=((LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(pRc->texture)->surfLcl))->ddckCKSrcBlt.dwColorSpaceLowValue ;

        // format of the chroma key value is the format of the texture
        // need to convert from texture format to xRGB-x888
        switch (txtr->format)
        {
          case (TEXFMT_ARGB_4444 << SST_TFORMAT_SHIFT): 
            r = ( r >> 8  ) & 0x0F;
            r = _imgMSBReplicate( r, 4, 0 );
            g = ( g >> 4  ) & 0x0F;
            g = _imgMSBReplicate( g, 4, 0 );
            b = ( b >> 0  ) & 0x0F;
            b = _imgMSBReplicate( b, 4, 0 );
            break;
          case (TEXFMT_ARGB_1555 << SST_TFORMAT_SHIFT):
            r = ( r >> 10 ) & 0x1F;
            r = _imgMSBReplicate( r, 3, 2);
            g = ( g >> 5  ) & 0x1F;
            g = _imgMSBReplicate( g, 3, 2);
            b =   b         & 0x1F;
            b = _imgMSBReplicate( b, 3, 2);
            break;
          default:
          case (TEXFMT_RGB_565 << SST_TFORMAT_SHIFT):
            r = ( r >> 11 ) & 0x1F;
            r = _imgMSBReplicate( r, 3, 2);
            g = ( g >> 5 ) &  0x3F;
            g = _imgMSBReplicate( g, 2, 4); 
            b = ( b >> 0 ) &  0x1F;
            b = _imgMSBReplicate( b, 3, 2 );
            break;
          case (TEXFMT_RGB_332 << SST_TFORMAT_SHIFT):
            r = ( r >> 5 ) & 0x07;
            r = (r  << 5 ) | (r << 2) | (r >> 1) ;   
            g = ( g >> 2 ) & 0x07;
            g = (g  << 5 ) | (g << 2) | (g >> 1) ;   
            b =   b        & 0x03;
            b = (b << 6) | (b << 4) | (b << 2) | b ;
            break;
          case (TEXFMT_P8_RGB << SST_TFORMAT_SHIFT):
          case (TEXFMT_P8_RGBA << SST_TFORMAT_SHIFT):
            {
              LPDDRAWI_DDRAWSURFACE_LCL lcl = (LPDDRAWI_DDRAWSURFACE_LCL)(TXTRHNDL_PTR(pRc->texture)->surfLcl);
              if (lcl->lpDDPalette != NULL)
              {
                g = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peGreen;   
                b = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peBlue;   
                r = lcl->lpDDPalette->lpLcl->lpGbl->lpColorTable[r].peRed;   
              }
            }
        } 
        xrgb =  (r << 16) | (g << 8) | b;
      
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chromaKey, 0xF ) );
        SETPD( cmdFifo, ghw->chromaKey, xrgb );
      
        _D3(last).chromaKey = xrgb;
        pRc->sst.fbzMode |= SST_ENCHROMAKEY;
      }
      else
      {
        pRc->sst.fbzMode &= ~SST_ENCHROMAKEY;
      }
    }
    else // pRc->texture != 0
    {
      if( HW_STATE_CHANGED & SC_TLOD )
      {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, tLOD, 0xF ) );
        SETPD( cmdFifo, ghw->tLOD, (8 << SST_LODMIN_SHIFT) | (8 << SST_LODMAX_SHIFT) );
      }
    }
  
    // Deal with fog.
    if( pRc->useFog & FOG_STATE_CHANGED )
    {
      if( FOG_STATE_CHANGED & SC_FOGCOLOR )
      {
        CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, fogColor, 0xF ) );
        SETPD( cmdFifo, ghw->fogColor, pRc->fogColor );
      }

      // Hardware table fog
      if( pRc->useFogTable & FOG_STATE_CHANGED )
      {
        CMDFIFO_SAVE( cmdFifo );
        createTableAndLoad(pRc);
        CMDFIFO_RELOAD( cmdFifo );
      }

      RESET_FOG_STATE;
    }

    if (IS_NAPALM)
    {
        // set target surface pixel depth.
        // this macro modifies the renderMode and fbzMode registers.
        SETSURFACEPIXELDEPTH(pRc, pRc->lpDDS);

        // Set the triangle iterator column band control.
        // This value is taken from a registry key, or is zero if no key exists.
        pRc->sst.fbzColorPath |= (( _D3(columnBandControl) << SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL_SHIFT ) 
                                  & SST_TRIANGLE_ITERATOR_COLUMN_BAND_CONTROL );

        if (IS_NAPALM) { //NAPALM_CU
            CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE + 5 + ((PH1_SIZE + 1) * 5) );
        }
        else {
            CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE + 5 + ((PH1_SIZE + 1) * 2) );
        }
    }
    else
    {
        CMDFIFO_CHECKROOM( cmdFifo, PH4_SIZE + 5 + PH1_SIZE + 1 );
    }

    //----------------------  
    //
    // Context switch restore 
    //
    //------------------------


    if ((HW_STATE_CHANGED) & SC_NEED_NOP)
    {
      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3|R7, fbzColorPath, 0 ) );
      SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
      SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
      SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
      SETPD( cmdFifo, ghw->nopCMD, 0 );
    }
    else
    {
      SETPH( cmdFifo, CMDFIFO_BUILD_PK4(R0|R1|R2|R3, fbzColorPath, 0 ) );
      SETPD( cmdFifo, ghw->fbzColorPath, pRc->sst.fbzColorPath );
      SETPD( cmdFifo, ghw->fogMode, pRc->sst.fogMode );
      SETPD( cmdFifo, ghw->alphaMode, pRc->sst.alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, pRc->sst.fbzMode );
    }

    if (IS_NAPALM)
    {
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 1 ) );//NAPALM_CU
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, pRc->sst.combineModeFBI );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 2 ) );
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU0)->combineMode, pRc->sst.combineModeT0 );
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 4 ) );
        SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU1)->combineMode, pRc->sst.combineModeT1 ); //NAPALM_CU

        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, renderMode, 0 ) );
        SETPD( cmdFifo, ghw->renderMode, pRc->sst.renderMode );
    }

    // If Z buffer is enabled then we need to send the current
    // ZBIAS value. - AS
    if( HW_STATE_CHANGED & SC_ZBIAS )
    {
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, zaColor, 0 ) );
      SETPD( cmdFifo, ghw->zaColor, pRc->sst.zaColor );
    }
  
    _D3(last).fbzColorPath  = pRc->sst.fbzColorPath;
    _D3(last).fogMode       = pRc->sst.fogMode;
    _D3(last).alphaMode     = pRc->sst.alphaMode;
    _D3(last).fbzMode       = pRc->sst.fbzMode;
    _D3(last).zaColor       = pRc->sst.zaColor;    

    if (IS_NAPALM)
    {
        _D3(last).combineModeFBI   = pRc->sst.combineModeFBI;//NAPALM_CU
        _D3(last).combineModeT0    = pRc->sst.combineModeT0 ;
        _D3(last).combineModeT1    = pRc->sst.combineModeT1 ;//NAPALM_CU
        _D3(last).renderMode       = pRc->sst.renderMode;
    }

    RESET_HW_STATE;
    
  } // context update
  else // HW_STATE_CHANGED
  {
    if (pRc->texture != 0)
	{
#if (DIRECT3D_VERSION >= 0x0700) && (DX >= 7)
      TXTRDESC *txtr = TXTRDESC_FROM_HNDL(pRc->texture);

      if (PALETTIZEDHANDLE(pRc->texture) && (PALETTECHANGED & _D3(flags)))
      {
        CMDFIFO_SAVE( cmdFifo );

        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ),
                            PALETTEGBL(pRc->texture),
                            txtr->formatFlags );

        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);
        CMDFIFO_RELOAD( cmdFifo );
      }
#else
      if(PALETTIZEDHANDLE(pRc->texture) && 
         ((_D3(flags) & PALETTECHANGED) || 
          (_D3(prevContentStamp) != PALETTEGBL(pRc->texture)->dwContentsStamp)))
	  {
        CMDFIFO_SAVE( cmdFifo );
      
        _D3(prevContentStamp) = PALETTEGBL(pRc->texture)->dwContentsStamp;

  #if( DX >= 6 )      
        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
          (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable,
          TXTRDESC_FROM_HNDL(pRc->texture)->formatFlags );
  #else
        TXTRDOWNLOADPALETTE(ppdev, ( CHIP_TMU0 ), ((LPDDRAWI_DDRAWSURFACE_LCL)
          (TXTRHNDL_PTR(pRc->texture)->surfLcl))->lpDDPalette->lpLcl->lpGbl->lpColorTable );
  #endif

        _D3(currentPalette) = (void *) PALETTEGBL(pRc->texture);
        CMDFIFO_RELOAD( cmdFifo );
	  }
#endif
    }

    if( _D3(last).changed )
    {
        if (IS_NAPALM)
        {
            CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 7) + 10 );//NAPALM_CU
        }
        else
        {
            CMDFIFO_CHECKROOM( cmdFifo, (PH1_SIZE * 3) + 6 );
        }

      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, chromaKey, 0xF ) );
      SETPD( cmdFifo, ghw->fbzColorPath, _D3(last).chromaKey );
      
      SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 4, 1, fbzColorPath, 0xF ) );
      SETPD( cmdFifo, ghw->fbzColorPath, _D3(last).fbzColorPath );
      SETPD( cmdFifo, ghw->fogMode, _D3(last).fogMode );
      SETPD( cmdFifo, ghw->alphaMode, _D3(last).alphaMode );
      SETPD( cmdFifo, ghw->fbzMode, _D3(last).fbzMode );

       if (IS_NAPALM)
       {
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 1 ) ); //NAPALM_CU
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_FBI)->combineMode, _D3(last).combineModeFBI);
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 2 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU0)->combineMode, _D3(last).combineModeT0);
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1CHIP( 1, 1, combineMode, 4 ) );
            SETPD( cmdFifo, SST_CHIP(ghw,CHIP_TMU1)->combineMode, _D3(last).combineModeT1); //NAPALM_CU
            SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, renderMode, 0 ) );
            SETPD( cmdFifo, ghw->renderMode, _D3(last).renderMode );
       }

      // If Z buffer is enabled then we need to send the current
      // ZBIAS value. - AS
      if(pRc->zEnable)
      {
        SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 1, zaColor, 0xF ) );
        SETPD( cmdFifo, ghw->zaColor, _D3(last).zaColor );
      }
      
      _D3(last).changed = FALSE;
    } // _D3(last).changed
  } // HW_STATE_CHANGED

  CMDFIFO_EPILOG( cmdFifo );
  
  if (pRc->zVisible)
  {
    prd->dwStatus &= ~D3DSTATUS_ZNOTVISIBLE;
    prd->ddrval = DD_OK;
    INS_EXIT( );
    D3D_EXIT( DDHAL_DRIVER_HANDLED );
  }
  
  //-------------------
  //
  // Parse instructions
  //
  //-------------------
  switch (lpIns->bOpcode)
  {
    case D3DOP_TRIANGLE:
        // I am ignoring the size field in the instruction, I am assuming that we
        // always get transformed and lit triangles so the size is always size of
        // D3DTLVERTEX.
        D3DPRINT( 255, "RenderPrimitive, D3DOP_TRIANGLELIST instruction count = %d",lpIns->wCount);
        pRc->drawTriangle(pRc, lpIns->wCount,(LPD3DTRIANGLE) prim, lpVertices) ;
        break;

    case D3DOP_POINT:
        D3DPRINT( 255, "RenderPrimitive, D3DOP_POINT instruction count = %d",lpIns->wCount);
        fpPoint(pRc, lpIns->wCount, (LPD3DPOINT)prim, lpVertices) ;
        break;
        
    case D3DOP_LINE:
        D3DPRINT( 255, "RenderPrimitive, D3DOP_LINE instruction count = %d",lpIns->wCount);
        fpLine(pRc, lpIns->wCount, (LPD3DLINE)prim, lpVertices) ;
        break;
        
    default:
        D3DPRINT( 255, "RenderPrimitive, Unhandled instruction opcode = %d", lpIns->bOpcode);
  }
   
  prd->ddrval = DD_OK;
  INS_EXIT( );
  D3D_EXIT( DDHAL_DRIVER_HANDLED );
} 

//-------------------------------------------------------------------

void FXNOPCMD(NT9XDEVICEDATA * ppdev)
{
  CMDFIFO_PROLOG(cmdFifo);

  CMDFIFO_CHECKROOM( cmdFifo, PH1_SIZE + 1 );
  SETPH( cmdFifo, CMDFIFO_BUILD_PK1( 1, 0, nopCMD, 0xF ) );
  SETPD( cmdFifo, ghw->nopCMD, 0 );
  CMDFIFO_EPILOG( cmdFifo );
}

//-------------------------------------------------------------------

